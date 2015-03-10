/*
 * Module: sconf.c (support for serial port configuration through hot key)
 *
 */

#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include "server.h"	// Global definitions for cy_buffering shared memory
#define SCONFMOD
#include "sconf.h"

//#define XDEBUG

#define MAX_MENUBUF   2048
#define MAX_OPTSIZE   100

static char optionBuf[MAX_OPTSIZE];
static char *menuBuf=NULL;

static _sconfstate sconfstate;

void (*sconf_write)(char *line, int cnt);

char *parityValues[3] = {"none", "odd", "even"};
char *flowCtrlValues[3] = {"none", "hard", "soft"};

static int save_newconfig(struct line_cfg *pconf);


/***************************************************
 * This routine finishes serial configuration mode:
 *   - sess: pointer to the user session;
 ***************************************************/
void sconf_logout(CY_SESS *sess) 
{
	if (menuBuf) {
		free(menuBuf);
		menuBuf = NULL;
	}

	sess->sess_flag &= ~(CY_SESS_FLAG_SCONF);
}

/*******************************************
 * This routine loads 1st level menu values
 *******************************************/
static void load_sconf_menu_values(struct line_cfg *pconf)
{
	int i;

	sconf_menu_values[0].value = pconf->speed;
	sconf_menu_values[1].value = pconf->DataSize;
	sconf_menu_values[2].value = pconf->Parity;
	sconf_menu_values[3].value = pconf->StopBits;
	sconf_menu_values[4].value = pconf->flow;

	//prepare for saving later...
	for (i=0; i < SCONF_MENU_ITEMS-1; i++) {
		sprintf(sconf_menu_values[i].pslaveitem, "s%d.%s", atoi(&pconf->tty[4]), sconf_menu[i].pslavepar);
	}
}

/**********************************************************************
 * This routine is called whenever a user presses serial configuration 
 * hotkey on a telnet/ssh session. It receives as parameter:
 *   - sess: pointer to the user session;
 *   - pconf: pointer to serial port configuration;
 *   - write_data: pointer to the routine that sends data to the user;
 **********************************************************************/
void sconf_login(CY_SESS *sess, struct line_cfg *pconf, void write_data(char *data, int cnt))
{

	if (cy_shm->sess_list[sess->port] != sess) {
		write_data(notPrimaryUser, 0);
		return;
	}

	memset(optionBuf, 0, sizeof(optionBuf));
	sconfstate = SCONFSTATE_NORMAL;
	load_sconf_menu_values(pconf);

	sconf_mount_menu(pconf);
	write_data(menuBuf, 0);

	sess->sess_flag |= CY_SESS_FLAG_SCONF;
}

/****************************************************************************
 * This routine mounts the serial configuration menu and saves it in menuBuf
 ****************************************************************************/
static void sconf_mount_menu(struct line_cfg *pconf)
{
	char *menu;
	int idx=0, i;

	if (menuBuf) free(menuBuf);
	menuBuf = calloc(1, MAX_MENUBUF);
	menu = menuBuf;

	idx += sprintf(&menu[idx], "\r\n%s%s%s\r\n%s",
				   sconfmenuHeaderLine, sconfmenuHeader, pconf->tty, sconfmenuHeaderLine);

	// Updates strings for the 1st level menu
	sprintf(sconf_menu_values[0].strval, "%d", sconf_menu_values[0].value);
	sprintf(sconf_menu_values[1].strval, "%d", sconf_menu_values[1].value);
	sprintf(sconf_menu_values[2].strval, "%s", parityValues[sconf_menu_values[2].value-1]);
	sprintf(sconf_menu_values[3].strval, "%d", sconf_menu_values[3].value);
	sprintf(sconf_menu_values[4].strval, "%s", flowCtrlValues[sconf_menu_values[4].value]);

	for (i=0; i < SCONF_MENU_ITEMS-1 ; i++) {
		idx += sprintf(&menu[idx], "      %d - %-14s[%s]\r\n", i + 1,
					   sconf_menu[i].menuName, sconf_menu_values[i].strval);
	}

	sprintf(&menu[idx], "      %d - %-s\r\n%s",
			SCONF_MENU_ITEMS, sconf_menu[SCONF_MENU_ITEMS-1].menuName, chooseOption);
}

/***********************************************
 * This routine explains 1st level menu options
 ***********************************************/
void sconf_explain_options() {
	int i;
	char buffer[96];

	sconf_write(wrongOption, 0);
	for (i = 0; i < SCONF_MENU_ITEMS; i++) {
		snprintf(buffer, 96, "%-16s - %s\r\n", 
				 sconf_menu[i].menuName, sconf_menu[i].commandHelp);
		sconf_write(buffer,0);
	}
	sconf_write(cancelOption, 0);

	sconf_write(menuBuf, 0);
}

/********************************************************************************
 * This routine treats data from socket connection in serial configuration mode:
 *   - sess: pointer to the user session;
 *   - pconf: pointer to serial port configuration;
 *   - write_data: pointer to the routine that sends data to the user;
 ********************************************************************************/
void sconf_treat_input(CY_SESS *sess, struct line_cfg *pconf, void write_data(char *data, int cnt)) 
{
	char echo_str[MAX_OPTSIZE], aux=0, *popt, *pecho;
	int  option, size;

	sconf_write = write_data;

	if (sess->InBuffer.RdPos == sess->InBuffer.WrPos) {
		//there is nothing to read
		return;
	}
	
	size = strlen(optionBuf);
	popt = optionBuf + size;

	memset(echo_str, 0, MAX_OPTSIZE);
	pecho = echo_str;

	while (sess->InBuffer.RdPos != sess->InBuffer.WrPos) {
		aux = sess->InBuffer.Buffer[sess->InBuffer.RdPos];
		sess->InBuffer.RdPos = (sess->InBuffer.RdPos + 1) % BufferSize;

		if (sconfstate == SCONFSTATE_SPEED2) { //means the user is typing...
			if (aux == 0x08 || aux == 0x7f) { //backspace or del
				if (size) {
					popt--;
					*popt = 0x00;
					size --;
					*pecho++ = 0x08;
					*pecho++ = ' ';
					*pecho++ = 0x08;
				}
				continue;
			}
			if (aux == '\r') {
				*pecho++ = aux;
				if (sess->InBuffer.Buffer[sess->InBuffer.RdPos] == '\n') {
					sess->InBuffer.RdPos = (sess->InBuffer.RdPos + 1) % BufferSize;
				} 
				*pecho++ = '\n';
				break;
			}
			if (aux == '\n') {
				*pecho++ = aux;
				break;
			}
		}
		if (size < MAX_OPTSIZE) {
			size++;
			*popt++ = aux;
			*pecho++ = aux;
		}
	}

	if (aux == 0x1b) { //escape
		if (sconfstate == SCONFSTATE_NORMAL) {
			// Exit
			sconf_write(endsconfmode, 0);
			sconf_logout(sess);
		} else {
			// a second level menu was cancelled
			memset(optionBuf, 0, sizeof(optionBuf)); //reset option buffer for next time
			sconfstate = SCONFSTATE_NORMAL;
			sconf_mount_menu(pconf);
			sconf_write(menuBuf, 0);
		}
		return;
	}

	sconf_write(echo_str, strlen(echo_str)); //echoing the data

	if (sconfstate == SCONFSTATE_SPEED2) { //means the user is typing...
		if (aux != '\r' && aux != '\n' &&  aux != '?') {
			//wait for the end of the line
			return;
		}
	}

	popt = optionBuf;
	option = strtol(optionBuf, &popt, 10); //convert option
	memset(optionBuf, 0, sizeof(optionBuf)); //reset option buffer for next time

	switch (sconfstate) {
	case SCONFSTATE_NORMAL:
		if (popt == optionBuf || option < 1 || option > SCONF_MENU_ITEMS) {
			sconf_explain_options();
			return;
		}
		sconf_treat_option(option, sess, pconf);
		return;
	case SCONFSTATE_SPEED:
		if (popt == optionBuf || option < 1 || option > SCONF_SPEED_MENU_ITEMS) {
			sconf_write(menuBuf, 0);
			return;
		}
		if (option == SCONF_OTHER_SPEED) {
			//user wants to manually enter speed
			sconfstate = SCONFSTATE_SPEED2;
			sconf_write(speedquestion, 0);
			return;
		}
		sconf_menu_values[sconfstate].value = sconf_speed_menu[option-1].optval;
		break;
	case SCONFSTATE_SPEED2:
		if (popt == optionBuf) {
			if (aux == '?')
				sconf_write("\r\n", 0);
			goto speed_options;
		}
		switch (option) {
		case 300:
		case 1200:
		case 2400:
		case 4800:
		case 9600:
		case 14400:
		case 19200:
		case 28800:
		case 38400:
		case 57600:
		case 76800:
		case 115200:
		case 230400:
			sconf_menu_values[SCONFSTATE_SPEED].value = option;
			break;
		default:
speed_options:
			sconf_write(listofspeeds, 0);
			sconf_write(speedquestion, 0);
			return;
		}
		break;
	case SCONFSTATE_DATASIZE:
		if (popt == optionBuf || option < 5 || option-4/* skipping first 4... */ > SCONF_DATASIZE_MENU_ITEMS) {
			sconf_write(menuBuf, 0);
			return;
		}
		sconf_menu_values[sconfstate].value = sconf_datasize_menu[option-1 - 4].optval;
		break;
	case SCONFSTATE_PARITY:
		if (popt == optionBuf || option < 1 || option > SCONF_PARITY_MENU_ITEMS) {
			sconf_write(menuBuf, 0);
			return;
		}
		sconf_menu_values[sconfstate].value = sconf_parity_menu[option-1].optval;
		break;
	case SCONFSTATE_STOPBITS:
		if (popt == optionBuf || option < 1 || option > SCONF_STOPBITS_MENU_ITEMS) {
			sconf_write(menuBuf, 0);
			return;
		}
		sconf_menu_values[sconfstate].value = sconf_stopbits_menu[option-1].optval;
		break;
	case SCONFSTATE_FLOWCTRL:
		if (popt == optionBuf || option < 1 || option > SCONF_FLOWCTRL_MENU_ITEMS) {
			sconf_write(menuBuf, 0);
			return;
		}
		sconf_menu_values[sconfstate].value = sconf_flowctrl_menu[option-1].optval;
		break;
	}

	sconfstate = SCONFSTATE_NORMAL;

	sconf_mount_menu(pconf);
	sconf_write(menuBuf, 0);
}

/************************************************************************
 * This routine treats serial configuration options at first menu level
 * and change state accordingly, mounting the appropriate secondary menu
 ************************************************************************/
static void sconf_treat_option(int option, CY_SESS *sess, struct line_cfg *pconf)
{

	switch (option) {
	case OPT_SPEED:
		sconfstate = SCONFSTATE_SPEED;
		sconf_mount_speed_menu();
		break;
	case OPT_DATASIZE:
		sconfstate = SCONFSTATE_DATASIZE;
		sconf_mount_secondary_menu(sconfDatasizeMenuHeader,
								   SCONF_DATASIZE_MENU_ITEMS,
								   sconf_datasize_menu, 5);
		break;
	case OPT_PARITY:
		sconfstate = SCONFSTATE_PARITY;
		sconf_mount_secondary_menu(sconfParityMenuHeader,
								   SCONF_PARITY_MENU_ITEMS,
								   sconf_parity_menu, 1);
		break;
	case OPT_STOPBITS:
		sconfstate = SCONFSTATE_STOPBITS;
		sconf_mount_secondary_menu(sconfStopbitsMenuHeader,
								   SCONF_STOPBITS_MENU_ITEMS,
								   sconf_stopbits_menu, 1);
		break;
	case OPT_FLOWCTRL:
		sconfstate = SCONFSTATE_FLOWCTRL;
		sconf_mount_secondary_menu(sconfFlowctrlMenuHeader,
								   SCONF_FLOWCTRL_MENU_ITEMS,
								   sconf_flowctrl_menu, 1);
		break;
	case OPT_APPLY:
		sconf_write(applyingConfig, 0);
		if (save_newconfig(pconf) != 0) {
			sconf_write(configFileError, 0);
		} else {
			sess->sess_flag |= CY_SESS_FLAG_SCONF_APPLY;
			sconf_write(configApplied, 0);
			sconf_logout(sess);
			return;
		}
	}

	sconf_write(menuBuf, 0);
}

/***************************************************************************
 * This routine mounts the speed configuration menu and saves it in menuBuf
 ***************************************************************************/
static void sconf_mount_speed_menu()
{
	char *menu;
	int idx=0, i;

	if (menuBuf) free(menuBuf);
	menuBuf = calloc(1, MAX_MENUBUF);
	menu = menuBuf;

	idx += sprintf(&menu[idx], "\r\n%s%s\r\n%s",
				   sconfmenuHeaderLine, sconfSpeedMenuHeader, sconfmenuHeaderLine);

	for (i=0; i < SCONF_SPEED_MENU_ITEMS/3 ; i++) {
		idx += sprintf(&menu[idx], "      %d - %-7s",   i + 1, sconf_speed_menu[i].stropt);
		idx += sprintf(&menu[idx],       "%d - %-7s",   i + 4, sconf_speed_menu[i+3].stropt);
		idx += sprintf(&menu[idx],       "%d - %s\r\n", i + 7, sconf_speed_menu[i+6].stropt);
	}

	sprintf(&menu[idx],"%s", chooseOptionOrRet);
}

/**************************************************************
 * This routine mounts datasize, parity, stopbits and flowctrl
 * configuratuin menus and saves them in menuBuf
 **************************************************************/
static void sconf_mount_secondary_menu(char *menuheader, int num_of_items, struct _sconf_option_menu *menulist, int startno)
{
	char *menu;
	int idx=0, i;

	if (menuBuf) free(menuBuf);
	menuBuf = calloc(1, MAX_MENUBUF);
	menu = menuBuf;

	idx += sprintf(&menu[idx], "\r\n%s%s\r\n%s", sconfmenuHeaderLine, menuheader, sconfmenuHeaderLine);

	for (i=0; i < num_of_items ; i++)
		idx += sprintf(&menu[idx], "      %d - %s\r\n", i + startno, menulist[i].stropt);

	sprintf(&menu[idx],"%s", chooseOptionOrRet);
}

/**********************************************************************
 * This routine saves the currently changed port configuration to disk
 **********************************************************************/
static int save_newconfig(struct line_cfg *pconf) {
	int i;
	struct {
		char pslaveline[80];
		int done;
	} pslavectrl[SCONF_MENU_ITEMS-1];
	FILE *fr, *fw;
	char linebuf[256], *lp;

	for (i=0; i < SCONF_MENU_ITEMS-1; i++) {
		sprintf(pslavectrl[i].pslaveline, "%s\t%s\n", sconf_menu_values[i].pslaveitem, sconf_menu_values[i].strval);
		pslavectrl[i].done = 0;
	}

	if (!(fr = fopen("/etc/portslave/pslave.conf", "r"))) {
		return 1;
	}
	if (!(fw = fopen("/etc/portslave/pslave.conf.tmp", "w"))) {
		fclose(fr);
		return 1;
	}

	//read pslave.conf lines
	while (fgets(linebuf, 256, fr)) {
		lp = linebuf;

		while (isblank(*lp)) lp++;
		if (*lp == '#') {
			fprintf(fw, "%s", linebuf);
			continue;
		}
		for (i=0; i < SCONF_MENU_ITEMS-1; i++) {
			if (!strncmp(lp, sconf_menu_values[i].pslaveitem, strlen(sconf_menu_values[i].pslaveitem))) {
				//configuration item found
				*lp = '\0';
				fprintf(fw, "%s", linebuf);
				fprintf(fw, "%s", pslavectrl[i].pslaveline);
				pslavectrl[i].done = 1;
				break;
			}
		}
		if (i == SCONF_MENU_ITEMS-1) {
			//just copy entire line to temporary file
			fprintf(fw, "%s", linebuf);
		}
	}

	//check if we need to add any new line
	for (i=0; i < SCONF_MENU_ITEMS-1; i++) {
		if (!pslavectrl[i].done) {
			fprintf(fw, "%s", pslavectrl[i].pslaveline);
		}
	}

	fclose(fr);
	fclose(fw);

	if ((rename("/etc/portslave/pslave.conf.tmp", "/etc/portslave/pslave.conf") != 0) ||
		(system("updatefiles /etc/portslave/pslave.conf >/dev/null 2>&1") != 0)) {
		return 1;
	}

	//update information on shared memory
	pconf->speed = sconf_menu_values[0].value;
	pconf->DataSize = sconf_menu_values[1].value;
	pconf->Parity = sconf_menu_values[2].value;
	pconf->StopBits = sconf_menu_values[3].value;
	pconf->flow = sconf_menu_values[4].value;

	return 0;
}

/*************************************************************
 * This routine, used by cy_buffering module, makes effective
 * the currently changed port configuration
 *************************************************************/
int apply_newconfig(int pfd, struct line_cfg *pconf)
{
	struct termios  term;
	int             aux;

	// Getting port configuration
	if (tcgetattr(pfd, &term) != 0) {
		return 1;
	}

	// Setting speed
	switch (pconf->speed) {
	case 300:
		aux = B300;
		break;
	case 1200:
		aux = B1200;
		break;
	case 2400:
		aux = B2400;
		break;
	case 4800:
		aux = B4800;
		break;
	case 14400:
		aux = B14400;
		break;
	case 19200:
		aux = B19200;
		break;
	case 28800:
		aux = B28800;
		break;
	case 38400:
		aux = B38400;
		break;
	case 57600:
		aux = B57600;
		break;
	case 76800:
		aux = B76800;
		break;
	case 115200:
		aux = B115200;
		break;
	case 230400:
		aux = B230400;
		break;
	default:
		aux = B9600;
	}
	cfsetospeed(&term, aux);
	cfsetispeed(&term, aux);

	// Setting data size
	term.c_cflag &= ~CSIZE;
	switch (pconf->DataSize) {
	case 5:
		term.c_cflag |= CS5;
		break;
	case 6:
		term.c_cflag |= CS6;
		break;
	case 7:
		term.c_cflag |= CS7;
		break;
	default:
		term.c_cflag |= CS8;
	}

	// Setting parity
	switch (pconf->Parity) {
	case PARITY_ODD:
		term.c_cflag |= (PARENB | PARODD);
		break;
	case PARITY_EVEN:
		term.c_cflag |= (PARENB);
		term.c_cflag &= ~(PARODD);
		break;
	default:
		term.c_cflag &= ~(PARENB); //none
	}

	// Setting number of stop bits
	if (pconf->StopBits == 2)
		term.c_cflag |= (CSTOPB);
	else
		term.c_cflag &= ~(CSTOPB);

	// Setting flow control
	switch (pconf->flow) {
	case FLOW_HARD:
		term.c_iflag &= ~(IXON | IXOFF);
		term.c_cflag |= (CRTSCTS);
		break;
	case FLOW_SOFT:
		term.c_iflag |= (IXON | IXOFF);
		term.c_cflag &= ~(CRTSCTS);
		break;
	default:
		term.c_iflag &= ~(IXON | IXOFF); //none
		term.c_cflag &= ~(CRTSCTS);
	}

	// Setting port configuration
	if (tcsetattr(pfd, TCSANOW, &term) != 0) {
		return 1;
	}

	return 0;
}
