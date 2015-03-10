/*
 * <security/pam_appdata.h>
 * 
 * This file redefines the field appdata_ptr from the conversation structure
 * (conv).  --- that is,
 * public interface between the PAM library and an application program
 * that wishes to use it.
 *
 * Created: Jun/12/2002 EAS (seabra@cyclades.com).
 *
 */

#ifndef PAM_APPDATA_H
#define PAM_APPDATA_H

#ifndef INPUTSIZE
#define INPUTSIZE PAM_MAX_MSG_SIZE           /* maximum length of input+1 */
#define CONV_ECHO_ON  1                            /* types of echo state */
#define CONV_ECHO_OFF 0
#endif

struct pam_appl_data {
	int id;
	char *data;	//[RK]Jun/02/05 Group Authorization Support
    char ppp;
    char sock;
    char flag;
    char caller;
    int  result;
	int  conv_once;		/* nastiness used to detect local login */
    char name[INPUTSIZE];
    char reply[256];
    char state[256];
	unsigned char callback_number[64];
	int port;
	//[RK]May/19 - Group Authorization 
	unsigned char *group_name;
};

#endif /* PAM_APPDATA_H */
