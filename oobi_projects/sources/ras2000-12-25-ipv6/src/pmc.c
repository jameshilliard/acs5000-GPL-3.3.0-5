/*
 * Module: pmc.c
 *
*/

// Linux headers
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

// Cyclades Headers
# include "server.h"	// Global definitions for cy_buffering shared memory

# define PMDCLIENT
#include <pmclib.h>
#include "pmc.h"

#ifndef KVMMAX
#include <userinfo.h>
#else
#define ADMIN_GROUP		"admin"
#define ADMIN_USER		"admin"
#endif

#define XDEBUG

#define MAX_MENUBUF	2048
#define MAX_SHOWBUF	2304
#define MAX_OPTSIZE	100
#define MAX_OUT_GRP	20

static char *otherStr=NULL; 	// string with the other input
static char *menuBuf=NULL;   // buffer with the menu
static char *showBuffer=NULL; // buffer used to mount the answer
static char *username=NULL;   // the username
static int flag_admin = 0;
static int flag_all = 0;
static oData * grptty[MAX_OUT_GRP];  // outlets of the device group
static oData * grpother[MAX_OUT_GRP];  // outlets of the other input
static int numOther = 0;		// number of outlets in the grpother
static char message[64]; // error message
static char optionBuf[80];
static int state = 0;	// 0 - normal , 1 - enter outlet list

void (*pmcas_write)(char *line,int cnt);

static void pmcas_error_write(char *error, int cnt) 
{
	pmcas_write(pmcerrorbegin, cnt);
	pmcas_write(error, cnt);
	pmcas_write(pmcerrorend, cnt);
}

static void pmcas_include_outlet(oData *po)
{
	int i;
	for  (i = 0; i < numOther; i ++) {
		if (po == grpother[i]) {
			break;
		}
	}
	if (i == numOther) {
		grpother[numOther++] = po;
	}
}

static int pmcas_treat_outlist()
{
	char * paux,aux = 0x00, *p1, *p2, *p3, *p4, *outl;
	oData ** poList, *po;
	int outN, outP, i, ret, dontcheck;
	char buf[64];
	ipduInfo *ipduinfo;
	
	numOther = 0;

	paux = optionBuf;
	
	ret = 0;
	while (*paux) {
		/* Find out each element of the list */
		dontcheck = 0;
		for (p1 = paux; ; p1 ++) {
			if (*p1 == 0) {
				break;
			} else if (*p1 == '[') {
				dontcheck = 1;
			} else if (*p1 == ']') {
				if (dontcheck) {
					dontcheck = 0;
				}
			} else if (*p1 == ',') {
				if (dontcheck == 0) {
					break;
				}
			}
		}
		if (*p1) {
			*p1++ = 0;
		}
		outl = strdup(paux);
		paux = p1;

		/* Analyse the outlet */
		for (p1 = outl; isspace(*p1); p1 ++);
		if (*p1 == '$') {
			/* outlet group */
			poList = pmc_validate_multout_name(p1, (flag_admin)?NULL:username);
			if (poList && *poList) {
				while (*poList) {
					pmcas_include_outlet(*poList);
					poList ++;
				}
			} else {
				snprintf(buf, 63, "\r\nOutlet group not found [%s].", p1);
				pmcas_write(buf, 0);
				ret = 1;
			}
		} else if ((p2 = strchr(p1, '[')) != NULL && strchr(p2, ']') != NULL) {
			/* format IPDU ID[outlet numbers]*/
			*p2++ = 0;
			if ((ipduinfo = pmc_validate_ipdu(p1)) == NULL) {
				snprintf(buf, 64, "\r\nIPDU ID not found [%s].", p1);
				pmcas_write(buf, 0);
				ret = 1;
			} else {
				while (*p2 != ']') {
					for (p3 = p2; *p3 != ']' && *p3 != ','; p3 ++);
					aux = *p3;
					*p3 = 0;
					p4 = p2;
					outN = strtol(p4, &p4, 10);
					if (p4 == NULL || outN < 1 || outN > ipduinfo->num_outlets) {
						snprintf(buf, 64, "\r\nInvalid outlet number [%s[%s]].", p1, p2);
						pmcas_write(buf, 0);
						ret = 1;
					} else {
						while (isspace(*p4)) p4 ++;
						if (! *p4) {
							po = pmc_validate_outlet_ipdu(p1, outN, 
								(flag_admin)?NULL:username);
							if (po) {
								pmcas_include_outlet(po);
							} else {
								snprintf(buf, 64, "\r\nNo permission for outlet [%s[%d]].", p1, outN);
								pmcas_write(buf, 0);
								ret = 1;
							}
						} else if (*p4 == '-') {
							p4 ++;
							outP = strtol(p4, &p4, 10);
							if (p4 == NULL || outP < 1 || outP > ipduinfo->num_outlets) {
								snprintf(buf, 64, "\r\nInvalid outlet number [%s[%s]].", p1, p2);
								pmcas_write(buf, 0);
								ret = 1;
							} else {
								while (isspace(*p4)) p4 ++;
								if (! *p4) {
									if (outP < outN) {
										for (i = outN; i >= outP; i --) {
											po = pmc_validate_outlet_ipdu(p1, i, 
												(flag_admin)?NULL:username);
											if (po) {
												pmcas_include_outlet(po);
											} else {
												snprintf(buf, 64, "\r\nNo permission for outlet [%s[%d]].", p1, i);
												pmcas_write(buf, 0);
												ret = 1;
											}
										}
									} else {
										for (i = outN; i <= outP; i ++) {
											po = pmc_validate_outlet_ipdu(p1, i, 
												(flag_admin)?NULL:username);
											if (po) {
												pmcas_include_outlet(po);
											} else {
												snprintf(buf, 64, "\r\nNo permission for outlet [%s[%d]].", p1, i);
												pmcas_write(buf, 0);
												ret = 1;
											}
										}
									}
								} else {
									snprintf(buf, 64, "\r\nInvalid outlet number [%s[%s]].", p1, p2);
									pmcas_write(buf, 0);
									ret = 1;
								}
							}
						} else {
							snprintf(buf, 64, "\r\nInvalid outlet number [%s[%s]].", p1, p2);
							pmcas_write(buf, 0);
							ret = 1;
						}
					}

					*p3 = aux;
					if (aux != ']') {
						*p3 ++;
					}
					p2 = p3; 
				}
			}
		} else {
			/* outlet name */
			po = pmc_validate_outlet_name(p1, (flag_admin)?NULL:username);
			if (po) {
				pmcas_include_outlet(po);
			} else {
				snprintf(buf, 64, "\r\nOutlet name not found [%s].", p1);
				pmcas_write(buf, 0);
				ret = 1;
			}
		}

		free(outl);
	}
	if (ret) {
		pmcas_write("\r\n\n",0);
		return 0;
	}

	if (otherStr) free(otherStr);
	if (numOther) {
		grpother[numOther] = NULL;
		otherStr = malloc(numOther * 30);
		for (outN=0,i=0; outN < numOther; outN++) {
			i += sprintf(otherStr+i," %s[%d]",
					grpother[outN]->ipdu->id, 
					grpother[outN]->onumber);	
		}
		pmcas_mount_menu();
	}
	return 1;
}

static void treatAnswer(char *answ)
{
	pmcas_write(answ, 0);
}

static void pmcas_send_cmd(int code)
{
	if (numOther) {
		pmc_sendcmd_outlet(code, OUTLET_LIST, grpother, username, treatAnswer);
	} else {
		pmc_sendcmd_outlet(code, OUTLET_GROUP, grptty, (flag_admin || flag_all)?NULL:username, treatAnswer);
	}

	pmcas_write(menuBuf,0);
}

/*****************************************************************
 * This routines checks if the user is part of the admin group,
 * giving them acess to all ports.  If so, they do not need to
 * be authenticated by pmd.
 ****************************************************************/
static int pmcas_user_in_admin_group(char * username)
{
        struct group *grpa;
        char *pname,**pmem; 

        if ((grpa = getgrnam(ADMIN_GROUP)) == NULL) {
                return 0;
        }
	
        pmem = grpa->gr_mem;
        while((pname = *pmem++)) {
                if (!strcmp(pname, username))
                        return 1;
        }

        return 0;
}

/*
 * This routine treats the logout of the PM session
 */
void pmcas_logout(CY_SESS *sess) 
{
	if (otherStr) {
		free(otherStr);
		otherStr = NULL;
	}
	if (menuBuf) {
		free(menuBuf);
		menuBuf = NULL;
	}
	if (showBuffer) {
		free(showBuffer);
		showBuffer = NULL;
	}
	if (username) {
		free(username);
		username = NULL;
	}
	sess->sess_flag &= ~(CY_SESS_FLAG_PMD);
}

/*****************************************************************
 * This routine is called whenever a user presses pmhotkey on a
 * telnet/ssh session. It receives as parameter:
 *   -  *sess: pointer to the user session;
 *   -  write_line: pointer to the routine that sends data
 *                  to the user;
 ****************************************************************/
void pmcas_login(CY_SESS * sess, char * device, void write_line(char * line,int cnt)) 
{
	int i;
	char group[100];
	oData ** poList;

	// reset global variables
	for (i=0; i < MAX_OUT_GRP ; i++) {
		grptty[i] = grpother[i] = NULL;
	}
	memset(optionBuf,0,80);
	numOther = flag_admin = flag_all = 0;

	// 0 - The the pointer to the routine that sends that to the user
	pmcas_write = write_line;
	
	// 1 - Access the PM shared memory
	if (pmc_shm_init_safe() == -1) {
		pmcas_error_write(servernotready,0);
		return;
	}

	// 2 - Checking if there is another session (??)
// remove - Juniper
//	if (cy_shm->sess_list[sess->port] != sess) {
//		pmcas_error_write(anothersession,0);
//		return;
//	}

	// 3 - reset global variables and check the user
	flag_admin = pmcas_user_in_admin_group(sess->tty_user);
	if (!flag_admin) {
		if (cy_shm->line_conf[sess->port].pmusers && *cy_shm->line_conf[sess->port].pmusers) {
			if (!strcmp(cy_shm->line_conf[sess->port].pmusers,"all")) {
				flag_all = 1;
			}
		}
	}
	username = strdup(sess->tty_user);

	// 3 - verify the device has one associated group.
	strcpy(group,"$");
	strcat(group,device);
	poList = pmc_validate_multout_name (group,(flag_admin || flag_all)? NULL:username);
	if (poList && *poList) {
		for (i=0; *poList; i++, poList++) {
			grptty[i] = *poList;
		}	
	} else {
		snprintf(message,64,"There is no outlet associated with this port");
		pmcas_error_write(message,0);
		pmcas_logout(sess);
		return;
	}

	// 4.1 - Mount menu
	pmcas_mount_menu();
	// 4.2 - Send menu
	pmcas_write(menuBuf,0);
	// Update sess->sess_flag
	sess->sess_flag |= CY_SESS_FLAG_PMD;
	return;
}

/**************************************************************************
 * This routine mount the PM session menu and save it on menuBuf 
 *************************************************************************/
static void pmcas_mount_menu() 
{
	char *menu, *label;
	int idx, i;

	if (menuBuf) free(menuBuf);
	menuBuf = calloc(1,MAX_MENUBUF);
	menu = menuBuf;

	idx = 0;
	idx += sprintf(&menu[idx], "\r\n%s%s", menuHeaderLine, menuHeader);
	
	if (numOther) {
		idx += sprintf(&menu[idx],"Outlets: %s\r\n",otherStr);
	}

	idx += sprintf(&menu[idx],"%s", menuHeaderLine);
	// Inserting the commands
	for (i = 0; i < NUMBER_OF_MENU_ITEMS ; i++) {
		label = pmcas_menu[i].menuName;
		if ((i%3) == 0) {
			idx += sprintf(&menu[idx], "       ");
		}
		idx += sprintf(&menu[idx]," %2d - %-8s", i + 1, label);
		if ((i%3) == 2) {
			idx += sprintf(&menu[idx], "\r\n");
		}
	}
	if ((i%3) != 0) {
		idx += sprintf(&menu[idx], "\r\n");
	}

	idx += sprintf(&menu[idx],"\r\n%s", chooseAnOption);
}

/*
 * This routine treats the data from socket connection to PM session
 */
void pmcas_treat_input(CY_SESS * sess, void write_data(char *data,int cnt)) 
{
	char echo_str[MAX_OPTSIZE], aux = 0, *popt, *pecho;
	int  option,size,i;

	pmcas_write = write_data;

	if (sess->InBuffer.RdPos == sess->InBuffer.WrPos) {
		// there is nothing to read
		return;
	}
	
	if (grptty[0] == NULL) { // not have PM session
		pmcas_error_write(noserversess,0);
		sess->sess_flag &= ~(CY_SESS_FLAG_PMD);
		return;
	}
	
	size = strlen(optionBuf);
	popt = optionBuf + size;

	memset(echo_str,0,MAX_OPTSIZE);
	pecho = echo_str;
	while (sess->InBuffer.RdPos != sess->InBuffer.WrPos) {
       		aux = sess->InBuffer.Buffer[sess->InBuffer.RdPos];
		sess->InBuffer.RdPos = (sess->InBuffer.RdPos + 1) % BufferSize;

		if ((aux == 0x08) || (aux == 0x7f)) {
			if (size) {
				popt--; // backspace
				*popt = 0x00;
				size --;
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
		if (size < MAX_OPTSIZE) {
			size++;
			*popt++ = aux;
			*pecho++ = aux;
		}
	}

	pmcas_write(echo_str,strlen(echo_str));	// echoing the data

	if (((aux != '\n') && (aux != '\r')) ||
		(!state && !strlen(optionBuf))) {
		return; // wait the ended line char
	}
	
	popt = optionBuf;
	if (state) { // enter outlet
		if ((*popt == '?') && (*(popt+1) == 0x00)) { // help line
			pmcas_write(outletHelp,0);
			pmcas_write(outletquestion,0);
			*popt = 0x00;
			memset(optionBuf,0,sizeof(optionBuf));
			return;
		}
		if (!strlen(optionBuf)) { // return to menu
			for (i=0; i < numOther; i++) {
				grpother[i] = NULL;
			}
			state = numOther = 0;
			pmcas_mount_menu();
			pmcas_write(menuBuf,0);
			return;
		}
		// treat outlet list
		if (! pmcas_treat_outlist()) {
			pmcas_write(outletquestion,0);
			*popt = 0x00;
			memset(optionBuf,0,sizeof(optionBuf));
			return;
		}
		pmcas_write(menuBuf,0);
		state = 0;
		memset(optionBuf,0,sizeof(optionBuf));
		return;
	}
	option = strtol(optionBuf,&popt,10); // convert option
	if (popt == optionBuf) {
		memset(optionBuf,0,sizeof(optionBuf));
		pmcas_write(invalidOption,0);
		pmcas_write(menuBuf,0);
		return;
	}
		
	memset(optionBuf,0,sizeof(optionBuf));
	if (option == OPT_EXIT) { // EXIT ==> Logout
		pmcas_logout(sess);
		pmcas_write(endpmsession,0);
	} else {
		pmcas_treat_option(option);		
	}
}

/*
 * This routine sends commands to PM server
 */
static void pmcas_treat_option(int option) 
{
	int i;
	char buffer[96];

	// is option valid
	if ((option == 0) || (option > NUMBER_OF_MENU_ITEMS)) {
		pmcas_write(invalidOption,0);
		pmcas_write(menuBuf,0);
		return;
	}

	// Look for the command	
	switch (option) {
		
		case OPT_HELP :  // help
			pmcas_write("\r\n", 0);
			for (i = 0; i < NUMBER_OF_MENU_ITEMS; i++) {
				snprintf(buffer, 96, "%-16s - %s\r\n", 
					pmcas_menu[i].menuName, pmcas_menu[i].commandHelp);
				pmcas_write(buffer,0);
			}
			pmcas_write(menuBuf,0);
			break;
	
		case OPT_OTHER :  // OTHER option
			state = 1; // enter outlet
			for (i=0; i < numOther; i++) {
				grpother[i] = NULL;
			}
			numOther = 0;
			pmcas_write(outletquestion,0);
			break;
		
		case OPT_ON : 
			pmcas_send_cmd(OUTLET_CONTROL_ON);
			break;

		case OPT_OFF :
			pmcas_send_cmd(OUTLET_CONTROL_OFF);
			break;

		case OPT_CYCLE :
			pmcas_send_cmd(OUTLET_CONTROL_CYCLE);
			break;

		case OPT_LOCK :
			pmcas_send_cmd(OUTLET_CONTROL_LOCK);
			break;

		case OPT_UNLOCK :
			pmcas_send_cmd(OUTLET_CONTROL_UNLOCK);
			break;

		case OPT_STATUS: // status
			if (!showBuffer) {
				showBuffer = malloc(MAX_SHOWBUF);
			}
			memset(showBuffer,0,MAX_SHOWBUF);
			pmc_show_outlet_data (1, (numOther)?grpother:grptty,showBuffer);
			pmcas_write(showBuffer,0);
			pmcas_write(menuBuf,0);
			break;

		case OPT_DELAY: // value of delay
			if (!showBuffer) {
				showBuffer = malloc(MAX_SHOWBUF);
			}
			memset(showBuffer,0,MAX_SHOWBUF);
			pmc_show_outlet_delay(1,(numOther)?grpother:grptty,OUTLET_CAP_POSTPOWERON,
					showBuffer);	
			pmcas_write(showBuffer,0);
			pmcas_write(menuBuf,0);
			break;
	}
}
