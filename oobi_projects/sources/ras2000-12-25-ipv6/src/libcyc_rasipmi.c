/*****************************************************************
 * File: libcyc_rasipmi.c
 *
 * Copyright (C) 2003 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Description: The ras interface to IPMI for the hotkey menu
 *
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <linux/shm.h>
#include <linux/sem.h>
#include "libcyc_rasipmi.h"
#include "server.h"

static void ipmi_treat_option(CY_SESS *,int,void write_data(char *,int));
static void ipmi_disp_menu(void);
void (*ipmi_write)(char *line,int cnt);

/* Strings for the IPMI menu */
static char *hline="\r\n-------------------------------------------------------";
#ifdef OEM3
static char *htext="\r\n  American Power Conversion - IPMI Power Management";
#else
static char *htext="\r\n      Avocent Corporation - IPMI Power Management";
#endif
static char *line1="\r\n\t1 - Exit\t2 - Help\t3 - On\r\n";
#ifdef OEM3
static char *line2="\t4 - Off\t\t5 - Reboot\t6 - Status\r\n";
#else
static char *line2="\t4 - Off\t\t5 - Cycle\t6 - Status\r\n";
#endif
static char *prompt="\r\nPlease choose an option: ";
static char *exitstr="Exit from IPMI Session\r\n";

char *input_cmd;
int g_serverid=0;

void ipmi_login(CY_SESS *sess,int serverid,char other,
		void write_line(char *line,int cnt))
{
	ipmi_write = write_line;

	ipmi_disp_menu();

	input_cmd=malloc(MAX_OPT_SIZE);
	memset(input_cmd,0,MAX_OPT_SIZE);

	g_serverid=serverid;

	/* Update sess->sess_flag */
	sess->sess_flag |= CY_SESS_FLAG_IPMI;
}

static void ipmi_disp_menu(void)
{
	ipmi_write(hline,0);
	ipmi_write(htext,0);
	ipmi_write(hline,0);
	ipmi_write(line1,0);
	ipmi_write(line2,0);
	ipmi_write(prompt,0);
}

void ipmi_logout(CY_SESS *sess)
{
	g_serverid=0;
	sess->sess_flag &= ~(CY_SESS_FLAG_IPMI);
}

void ipmi_treat_input(CY_SESS *sess, void write_data(char *data,int cnt)) {
	char echo_str[MAX_OPT_SIZE], aux = 0, *pecho,*popt;
	int option,size;

	if (sess->InBuffer.RdPos == sess->InBuffer.WrPos) {
	  /* there is nothing to read */
		return;
	}

	memset(echo_str,0,MAX_OPT_SIZE);
	pecho=echo_str;
	size=strlen(input_cmd);
	popt=input_cmd + size;  //[GY]2006/Jan/04  BUG#5681
	while (sess->InBuffer.RdPos != sess->InBuffer.WrPos) {
       		aux = sess->InBuffer.Buffer[sess->InBuffer.RdPos];
		sess->InBuffer.RdPos = (sess->InBuffer.RdPos + 1) % BufferSize;

		if ((aux == 0x08) || (aux == 0x7f)) {
			if (size) {
				popt--;
				*popt=0x00;
				size--;
				*pecho++ = 0x08;
				*pecho++ = ' ';
				*pecho++ = 0x08;
			}
			continue;
		}
		if (aux == '\r') {
			char lix;
			*pecho++ = aux;
       			lix=sess->InBuffer.Buffer[sess->InBuffer.RdPos];
			if (lix == '\n') {
				sess->InBuffer.RdPos = 
					(sess->InBuffer.RdPos + 1) % BufferSize;
			} 
			*pecho++ = '\n';
			break;
		}
		if (aux == '\n') {
			*pecho++ = aux;
			break;
		}
		if (size < MAX_OPT_SIZE) {
			size++;
			*popt++ = aux;
			*pecho++ = aux;
		}
	}

	ipmi_write(echo_str,strlen(echo_str)); /* Echo data */

	if (((aux != '\n') && (aux != '\r'))) {
		return; /* Wait for line end character */
	}

  	option=atoi(input_cmd);
        memset(input_cmd,0,MAX_OPT_SIZE);  //[GY]2006/Jan/03  BUG#5681

	if ((option <= 0) || (option > 6)) {
		ipmi_write("Invalid option.\r\n",0);
		ipmi_disp_menu();
		return;
	}

	ipmi_treat_option(sess,option,ipmi_write);
}

static void ipmi_treat_option(CY_SESS * sess,int option,
			      void write_data(char *data,int cnt))
{
	char buffer[129];
	FILE *fp;
	int cmd = -1;

	ipmi_write = write_data;

	switch (option) {
		case 1 :  /* Logout */
			ipmi_logout(sess);
			ipmi_write(exitstr,0);
			break;

		case 2 :  /* Help */
			ipmi_write("\r\n", 0);
			snprintf(buffer, 96, "%-16s - %s\r\n", 
				"Exit", "Exits the IPMI Management Session");
			ipmi_write(buffer,0);
			snprintf(buffer, 96, "%-16s - %s\r\n", 
				"Help", "Shows this message");
			ipmi_write(buffer,0);
			snprintf(buffer, 96, "%-16s - %s\r\n", 
				"On", "Turns the server On");
			ipmi_write(buffer,0);
			snprintf(buffer, 96, "%-16s - %s\r\n", 
				"Off", "Turns the server Off");
			ipmi_write(buffer,0);
			snprintf(buffer, 96, "%-16s - %s\r\n", 
#ifdef OEM3
				"Reboot", "Reboots the server (off/on)");
#else
				"Cycle", "Power cycles the server (off/on)");
#endif
			ipmi_write(buffer,0);
			snprintf(buffer, 96, "%-16s - %s\r\n", 
				"Status", "Shows power status of the server");
			ipmi_write(buffer,0);
			ipmi_write(prompt,0);
			break;

		case 3 : /* Power On */
			cmd = 1; 
			break;

		case 4 : /* Power Off */
			cmd = 0;
			break;

		case 5 : /* Power Cycle */
			cmd = 3;
			break;

		case 6 : /* Power Status */
			cmd = 2;
			break;
		
		default : // invalid option
			sprintf(buffer, "\r\n Invalid option");
			ipmi_write(buffer,0);
			ipmi_write(prompt,0);
			break;
	}

	if (cmd == -1) {
                return;
	}

	sprintf(buffer,"cyc_ipmicmd %d %d >/tmp/ipmicmd.ret 2>&1",g_serverid,cmd);
	system(buffer);

	if ((fp = fopen("/tmp/ipmicmd.ret", "r")) != NULL) {
		while (fgets(buffer,128,fp) != NULL) {
                        buffer[128] = 0x00;
			ipmi_write(buffer,0);
                        memset(buffer,0,129);
                }
                fclose(fp);
        }
	ipmi_disp_menu();
        system("rm -f /tmp/ipmicmd.ret");
}
