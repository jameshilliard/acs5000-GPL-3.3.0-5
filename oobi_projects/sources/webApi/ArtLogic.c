/*****************************************************************
 * File: artLogic.c
 *
 * Copyright (C) 2003 Cyclades Corporation
 *
 * Maintainer: www.cyclades.com
 *
 * Description: Contains functions only called by Art and Logic.
 * Any immediate API calls are in this file. (ie.  eject a pcmcia 
 * slot. Also functions such as Login, Logout.
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>
#include <linux/version.h>
#include <asm/cyc_config.h>
#include <asm/flash.h>
#include <asm/prxk.h>
#include <sys/ioctl.h>
#include <src/server.h>
#include <src/config_lock.h>
#include <src/mysyslog.h>
#include <webApi.h>
#include "WebApi2.h"
#include <crypt.h>
#ifdef SHADOWPASSWDS
#include <shadow.h>
#endif
#include "pam_auth.h"
#include "userinfo.h"

#include <cyc.h>

extern struct cyc_info *cyc;

extern int EncryptString(char *key, char* textString, int def);
extern char *HexEncode(char* textString, int xorLen);
extern void writeevt(int, char*, ...); //[RK]May/11/06 - event notification

int web_idle_timeout = MAX_SESS_IDLE_TIME;

static void adjust_web_sessions(void);

//========================================================================
// getSidKey - generates a new sid and check for repetition
//========================================================================
static Sid getSidKey(void)
{
	int i;
	Sid sid;

	do {
		srand((unsigned)time(NULL));
		sid = (UInt32)(rand());
		for (i=0; i<MAX_WEB_USERS; ++i) {
			if (webUsers[i].sid == sid) {
				sid = 0;
				break;
			}
		}
	} while (!sid);

	return(sid);
}
	
//========================================================================
// CycLogIn - API that handles logging user into the web
//========================================================================
Result CycLogIn(const char* user, const char* password, Sid *sid, WebUserInfo *uInfo, int noAuth, int portauth)
{
	CycUserType userType = kRegularUser;
	struct group *webGroup = NULL;
	int i, ix_empty, ix_admin;
	int  now, xorLen;
	char temp[12] = {0};
	char *pass;
	struct passwd *pw;

	dolog("CycLogIn()");

	if (cy_shm->web_date_set) adjust_web_sessions();

#if defined(KVM) || defined(ONS)
	if (!(user && *user)) {
		return (kInvalidLogin);
	}

	if (strcmp(user, "root") == 0) {
		return (kInvalidRoot);
	}
#endif

#if defined(OEM3)
	if (strcmp(user, "root") == 0) {
		if (getgrnam(ADMIN_GROUP)) {
			//If group "apc" exists, avoid root login
			return (kInvalidRoot);
		} else {
         		userType = kAdminUser;
		}
	}
#endif

	if (portauth) {
#if defined(KVM) || defined(ONS)
	    dolog("Portauth");
		if (PamAuthenticate(user, password, PAM_KVM)) {
			return kInvalidLogin;
	    }
#endif
        } else {
		dolog("Unitauth");
		// use unit authentication
		//Check if user and password is allowed
		// Call some pam function
		//If not, then return kInvalidLogin
		if (PamAuthenticate(user, password, PAM_WEB)) {
			//[RK]May/11/06 - event notification
			writeevt(4,"s",user);
			//syslog(LOG_ALERT, "%s: User [%s] login failed", TAG_AUTH, user);
			//[RK]Sep/16/04 - Web Improvement
			if (strcmp(user,"root")) {
				return kInvalidLogin;
			} else {
				return kInvalidRoot;
			}
		}
	}

	dolog("User authenticated");

	//If it is, randomly generate an Sid
	*sid = getSidKey();

	/* Check if this user is part of the webAdmin group. */
	// Get info regarding group, admin
	setgrent();
	while (1) {
		webGroup = getgrent();
		if (webGroup == NULL) {
			break;
		}
		if (strcmp(webGroup->gr_name, ADMIN_GROUP)) {
			continue;
		}
		// check if user is in group admin
		for(i=0; webGroup->gr_mem[i]; i++) {
	   		if (!strcmp(webGroup->gr_mem[i], user)) {
         		userType = kAdminUser;
		 		break;
	   		}
		}

		// if could not find in group member, do one more check
		if ( userType == kRegularUser) {
			pw = getpwnam(user);
			if (pw) {
				if (pw->pw_gid == webGroup->gr_gid) {
					userType = kAdminUser;
				}
			}
		}
		if ( userType == kAdminUser) {
			break;
		}
	}
	endgrent();
	dolog("User: %s is %s user", user, 
		(userType == kAdminUser ? "an admin" : "a regular"));

found_an_empty:

	for (ix_empty=-1, ix_admin=-1, i=0; i<MAX_WEB_USERS; ++i) {
		//find the first empty cell in array and put user sid in it
		if (webUsers[i].sid == 0) {
			if (ix_empty == -1) ix_empty = i;
			continue;
		}
		if (webUsers[i].status == kSessAdminOk) {
			ix_admin = i;
		}
	}

	if (ix_empty >= 0) {
		webUsers[ix_empty].sid = *sid;
		strcpy(webUsers[ix_empty].userName,user);

#ifdef GRPAUTHenable
		//[RK]Jun/02/05 Group Authorization Support
		webUsers[ix_empty].group_name = NULL;
#endif

		// Encrypt password and store encrypted pass in hex format
		// This logic goes with logic for java applet
		sprintf(temp, "%d", *sid);
		/* ArtLogic.c:172: warning: passing arg 2 of `EncryptString' 
		discards qualifiers from pointer target type
		xorLen = EncryptString(temp, password, 0);*/
		xorLen = EncryptString(temp, (char *) password, 0);
		dolog("Encrypted pass: [%s]", password);
		/*ArtLogic.c:174: warning: passing arg 1 of `HexEncode' 
		discards qualifiers from pointer target type
		pass = HexEncode(password, xorLen);*/
		pass = HexEncode((char *) password, xorLen);
		dolog("Encrypted pass in hex: [%s]", pass);
		strcpy(webUsers[ix_empty].pass,pass);
		// save the Key to decrypt the pass
		webUsers[ix_empty].passK = *sid;

		strcpy(uInfo->uName, user);
		webUsers[ix_empty].lastAccess = time(0); //mp: register login time
#ifdef GRPAUTHenable
                //[GY]2006/Jan/27  Group_Auth_Box
                PAM_get_group_name(&webUsers[ix_empty].group_name);
                if( webUsers[ix_empty].group_name != NULL )
                   if( strstr(webUsers[ix_empty].group_name, "admin") )
                      userType = kAdminUser;
#endif

#ifdef PMD
		webUsers[ix_empty].showIpdu = 0; //[RK]Nov/03/06
#endif

		if (userType == kAdminUser) {
			uInfo->uAccess = kReadWrite;
			uInfo->uType = kAdminUser;
			webUsers[ix_empty].ix_admin = -1;
			if (ix_admin >= 0) {
				webUsers[ix_empty].ix_admin = ix_admin;
				webUsers[ix_empty].status = kWannaBeAdmin;
				syslog(LOG_ALERT, "%s: User [%s] login failed. There exists another admin session", TAG_AUTH, user);
				return(kAnotherAdminIsOn);
			}
			if (lock_config_session(CNF_APPL_WEB, webUsers[ix_empty].sid,
            	webUsers[ix_empty].userName)) {
				webUsers[ix_empty].status = kWannaBeAdmin;
				syslog(LOG_ALERT, "%s: User [%s] login failed. There exists another admin session", TAG_AUTH, user);
				return(kAnotherAdminIsOn);
			}
			//[RK]May/11/06 - event notification
			writeevt(2,"si",user,3);

			webUsers[ix_empty].status = kSessAdminOk;
			return(kCycSuccess);
		} else { 
#ifdef GRPAUTHenable
			//[RK]Jun/02/05 - Group Authorization Support
                        //[GY]2006/Jan/27  Group_Auth_Box
//			PAM_get_group_name(&webUsers[ix_empty].group_name);
                        if(webUsers[ix_empty].group_name != NULL){
                                uInfo->uGroupName = strdup(webUsers[ix_empty].group_name);
				syslog(LOG_DEBUG, "Group Authorization: the group name from Auth Server is %s \n", webUsers[ix_empty].group_name);
                        }
                        else{
				syslog(LOG_DEBUG, "Group Authorization: the group name from Auth Server is NULL \n");
                        }
#endif

		}
		webUsers[ix_empty].status = kSessUserOk;
		webUsers[ix_empty].ix_admin = -1;
		uInfo->uType = kRegularUser;
		uInfo->uAccess = kReadOnly;
		//[RK]May/11/06 - event notification
		writeevt(2,"si",user,3);

		return(kCycSuccess);
	}


	if (web_idle_timeout) {
		now = time(0) - web_idle_timeout;
		for (i=0; i<MAX_WEB_USERS; ++i) {
			if (now > webUsers[i].lastAccess) {
				webUsers[i].sid = 0;
				goto found_an_empty;
			}
		}
	}
	syslog(LOG_DEBUG, "%s: User [%s] login failed. Maximum number of connected users reached", TAG_AUTH, user);
	return (kSystemError);
}

static void adjust_web_sessions(void)
{
	time_t tm;
	int i;

	if (!web_idle_timeout) return;

	tm = cy_shm->web_date_set - cy_shm->web_start_time;
	cy_shm->web_start_time = cy_shm->web_date_set;
	cy_shm->web_date_set = 0;

	for (i=0; i<MAX_WEB_USERS; ++i) {
		if (webUsers[i].sid) {
			webUsers[i].lastAccess += tm;
		}
	}
}

//========================================================================
// CycCheckUser - API that validates the user's session
//========================================================================

static int ix_user;

#ifdef PMD
//[RK]Nov/03/06
Result CycSetshowIpdu(Sid *sid, int showIpdu)
{
	UInt8 i;

	for (i=0; i<MAX_WEB_USERS; ++i) {
		if(*sid == webUsers[i].sid) {
			webUsers[i].showIpdu = showIpdu; 
			break;
		}
	}
	return(kCycSuccess);	
}
#endif

Result CycCheckUser(Sid *sid, WebUserInfo *wUser, int chkAdmin)
{
	UInt8 i;
	int  now;
	
	if (cy_shm->web_date_set) adjust_web_sessions();

	now = time(0);
	wUser->uAccess = kReadOnly;
	wUser->uType = kUserNone;
	wUser->uName[0] = 0;

	if (!(*sid)) {
		return (kInvalidLogin);
	}
 
	for (i=0; i<MAX_WEB_USERS; ++i) {
		if(*sid == webUsers[i].sid) {
			if (web_idle_timeout &&
				(now - webUsers[i].lastAccess) > web_idle_timeout) {
				unlock_config_session(webUsers[i].sid);
				webUsers[i].sid = 0;
				webUsers[i].status = 0;
#ifdef PMD
				webUsers[i].showIpdu = 0; //[RK]Nov/03/06
#endif
#ifdef GRPAUTHenable
				//[RK]Jun/03/05 - Group Authorization Support
				if (webUsers[i].group_name) {
					free(webUsers[i].group_name);
					webUsers[i].group_name = NULL;
				}
#endif

				*sid = 0;
				/* ArtLogic.c:285: warning: int format, pointer arg (arg 4)
				syslog(LOG_INFO, "%s: Session [%d] timed out", TAG_AUTH, sid);*/
				//[RK]May/11/06 - event notification
				writeevt(2,"si",webUsers[i].userName,3);
				//syslog(LOG_INFO, "%s: Session [%d] timed out", TAG_AUTH, (Sid)sid);
				return(kTimeout);
			}
			ix_user = i;
			strcpy(wUser->uName, webUsers[i].userName);
			strcpy(wUser->uPass, webUsers[i].pass);
			setenv("LOGNAME", wUser->uName, 1);
			
			wUser->uPassK = webUsers[i].passK;//[RK]Jul/29/05 

#ifdef PMD
			wUser->showIpdu = webUsers[i].showIpdu; //[RK]Nov/03/06
#endif

#ifdef GRPAUTHenable
			//[RK]Jun/03/05 Group Authorization Support
			if (webUsers[i].group_name) {
				wUser->uGroupName = strdup(webUsers[i].group_name);
			} else 
#endif
			{
				wUser->uGroupName = NULL;
			}
			webUsers[i].lastAccess = now;
			switch (webUsers[i].status) {
			case kSessUserOk:
				wUser->uType = kRegularUser;
				if (chkAdmin) return(kEditPermissionDenied);
				break;
			case kSessBumped:
				return(kAnotherAdminIsOn);
			default:
				wUser->uAccess = kReadWrite;
				wUser->uType = kAdminUser;
				break;
			}
			return(kCycSuccess);	
		}
	}
	return (kInvalidLogin);
}

//========================================================================
// CycBecomeAdminBoss - API that can allow current user logged in to 
// take over control if another admin user is logged in.
//========================================================================
Result CycBecomeAdminBoss(Sid sid)
{
	int i;
	Result result;
	WebUserInfo wInfo;
	CY_CONFIG_SESSION sess;
	char *old_username;
	extern int signal_usr_flag;

	if ((result = CycCheckUser(&sid, &wInfo, 1)) != kCycSuccess) {
		return(result);
	}

	// status must be WannabeAdmin always
	if (webUsers[ix_user].status != kWannaBeAdmin) {
		return(kLoginRequired);
	}
	
	if ((i = webUsers[ix_user].ix_admin) != -1) {
		if (webUsers[i].status != kSessAdminOk) {
			return(kLoginRequired);
		}
		old_username = webUsers[i].userName;
		webUsers[i].status = kSessBumped;
		change_config_session(CNF_APPL_WEB, webUsers[ix_user].sid,
			webUsers[ix_user].userName);
	} else {
		if (cancel_config_session(10, &signal_usr_flag, &sess)) {
			return(kLoginRequired);
		}
		old_username = sess.username;
		if (lock_config_session(CNF_APPL_WEB, webUsers[ix_user].sid,
            webUsers[ix_user].userName)) {
			return(kLoginRequired);
		}
	}

	//[RK]May/11/06 - event notification
	writeevt(13,"ss",webUsers[ix_user].userName,old_username);
	syslog(LOG_ALERT, "%s: Previous admin session terminated by new admin [%s] login", 
		TAG_AUTH, webUsers[ix_user].userName);
	webUsers[ix_user].status = kSessAdminOk;

	return result;
}

//========================================================================
// Web Admin kicked off by another configuration method
//========================================================================
void CycKickAdmin(void)
{
	int i;

	for (i=0; i<MAX_WEB_USERS; ++i) {
		if (webUsers[i].status == kSessAdminOk) {
			send_signal_admin();
			unlock_config_session(webUsers[i].sid);
			webUsers[i].sid = 0;
			webUsers[i].status = 0;
			break;
		}
	}
}

//========================================================================
// CycLogOut - API that handles logging out user from the web
//========================================================================
Result CycLogOut(Sid sid)
{
	Result result;
	WebUserInfo wInfo;

	if ((result = CycCheckUser(&sid, &wInfo, 0)) != kCycSuccess) {
		return(result);
	}

	if (webUsers[ix_user].status != kSessUserOk) {
		unlock_config_session(sid);
	}

	//[RK]May/11/06 - event notification
	writeevt(3,"si",webUsers[ix_user].userName,3);
	//syslog(LOG_INFO, "%s: User [%s] for session [%d] logged out", 
	//	TAG_AUTH, webUsers[ix_user].userName, webUsers[ix_user].sid);
	webUsers[ix_user].sid = 0;
	webUsers[ix_user].status = 0;

#ifdef GRPAUTHenable
	//[RK]Jun/03/05 - Group Authorization Support
	if (webUsers[ix_user].group_name) {
		free(webUsers[ix_user].group_name);
		webUsers[ix_user].group_name = NULL;
	}
#endif
	return kCycSuccess;
}

//========================================================================
// CycReboot - Immediate API that reboots the unit
//========================================================================
void CycReboot()
{
	writeevt(1,"U"); // write the EVT1 - param : username
	system("halt");
}

//========================================================================
// CycInsertPcmciaCard - Immediate API that doesn't physically insert
// the card, but loads or reloads the network drivers with the new 
// configurations.
//========================================================================
#ifdef PCMCIA
Result CycInsertPcmciaCard(CycSlotLocationOption slot)
{
  unsigned char command[50];

	sprintf(command, "/sbin/cardctl insert %d", slot);
	system(command);
	return (kCycSuccess);
}
#endif

//========================================================================
// CycEjectPcmciaCard - Immediate API that doesn't physically eject 
// the card, but unloads the client drivers.
//========================================================================
#ifdef PCMCIA
Result CycEjectPcmciaCard(CycSlotLocationOption slot)
{
  unsigned char command[50];

	sprintf(command, "/sbin/cardctl eject %d", slot);
	system(command);
	return (kCycSuccess);
}
#endif

//========================================================================
// CycKillActiveSession - Immediate API kills an active session
//========================================================================
Result CycKillActiveSession(ActiveSessionInfo *pActiveSess)
{
	DIR *dir;
	struct dirent *direntry;
	struct stat sbuf;
	char statpath[32], buffer[512];
	char *s, *t;
	int tty, ttys;
	int i, fd;
	//struct ActiveSessionInfo *pActiveSess = paramList->value;

	dolog("KillActiveSession()");
	
	if (*pActiveSess->pid)
	{
		snprintf (statpath, 32, "kill -int %s", pActiveSess->pid);
		system(statpath);
		sleep(1);
#if defined(KVM) || defined(ONS)
	} else if (strncmp(pActiveSess->fromIp, "user", 4) == 0){	
		int station = atoi(&(pActiveSess->fromIp[4]));
		if(station > 0){
			sprintf(buffer, "kvm setperm -s %d -P none", station);
			printf("CycKillActiveSession() calling %s\n", buffer);
			system(buffer);
		}
#endif
	} else {
		sprintf(statpath, "%s%s", "/dev/", pActiveSess->tty);
		if(stat(statpath, &sbuf) >= 0) 
		{
			ttys = sbuf.st_rdev;
			if ((dir = opendir("/proc")) != NULL) 
			{
				while ((direntry = readdir(dir)) != NULL) 
				{
					if (!isdigit(*direntry->d_name)) 
					{
						continue;
					}
					sprintf(statpath, "/proc/%s/stat", direntry->d_name);
					if ((fd = open(statpath, O_RDONLY)) != -1) 
					{
						if ((i = read(fd, buffer, 511)) > 0) 
						{
							buffer[i] = 0;
							for (s=buffer, i=0; i<6; i++) 
							{
								while (*s && !isspace(*s)) s++;
								while (isspace(*s)) s++;
							}
							t=s;
							while (*t && !isspace(*t)) t++;
							*t=0;
							if (sscanf(s, "%d", &tty)) 
							{
								if (tty == ttys) 
								{
									snprintf (statpath, 32, "kill -9 %s", direntry->d_name);
									dolog("StatPath: %s", statpath);
									system (statpath);
									sleep(1);
								}
							}
						}
						close(fd);
					}
				}
				closedir(dir);
			}
		}
	}
	return (kCycSuccess);
}

//========================================================================
// CycNewSid - returns a new sid for security sake
//========================================================================
int CycGetNewSid(Sid sid)
{
	WebUserInfo wInfo;

	if (CycCheckUser(&sid, &wInfo, 0) != kCycSuccess) {
		return(0);
	}

	sid = getSidKey();

	webUsers[ix_user].sid = sid;
	if (webUsers[ix_user].status == kSessAdminOk) {
		cy_shm->cnf_session.sid = sid;
	}
	return(sid);
}

//========================================================================
// CycUserBio - returns whether user is bio user or not
//========================================================================
int CycUserBio(Sid sid)
{
	struct group *bioGroup;
	int i;
	WebUserInfo wUser;

	if (CycCheckUser(&sid, &wUser, 0) != kCycSuccess) return(1);

	if ((bioGroup = getgrnam("biouser")) == NULL) {
		dolog("Group: %s not found", "biouser");
		return (1);
	}

	for(i = 0; bioGroup->gr_mem[i]; i++) {
		if (!strcmp(bioGroup->gr_mem[i], wUser.uName)) return (0);
	}

	return (1);
}

//========================================================================
// CycUserBioName - returns whether user is bio user or not
//========================================================================
int CycUserBioName(char *username)
{
	struct group *bioGroup;
	int i;

	if ((bioGroup = getgrnam("biouser")) == NULL) {
		dolog("Group: %s not found", "biouser");
		return (1);
	}

	for(i = 0; bioGroup->gr_mem[i]; i++) {
		if (!strcmp(bioGroup->gr_mem[i], username)) return (0);
	}

	return (1);
}

extern struct MachineType mt;
extern int numPowerSupply;
#ifdef PMD
extern void setSignals();
#endif

//=============================================================================
// CycInitApi - Performs general WebApi Initialization Must be called before
// any call to CycGet, CycSet, etc
//=============================================================================
Result CycInitApi(void)
{
	int fd;

	get_machine_type(&mt);

	//printf("cycinitapii\n");
#if defined(KVM) || defined(ONS)
	switch (mt.board_type) {
	case BOARD_KVM16:
	case BOARD_KVM32:
	case BOARD_KVMNET16:
	case BOARD_KVMNET32:
		kMaxKVMStations = 2;
		break;
	case BOARD_ONS442:
	case BOARD_ONS482:
	case BOARD_ONS842:
	case BOARD_ONS882:
		kMaxKVMStations = 2;
		//if (mt.board_version == 2) kMaxKVMStations++;
		if (libcyc_kvm_get_bd_ver(cyc) == 2) kMaxKVMStations++;
		break;
	case BOARD_ONS441:
	case BOARD_ONS481:
	case BOARD_ONS841:
	case BOARD_ONS881:
		kMaxKVMStations = 1;
		//if (mt.board_version == 2) kMaxKVMStations++;
		if (libcyc_kvm_get_bd_ver(cyc) == 2) kMaxKVMStations++;
		break;
	case BOARD_KVMP16:
	case BOARD_KVMP32:
		kMaxKVMStations = 6;
		break;
	default:
		kMaxKVMStations = 2;
	}
	//printf("kMaxKVMStations = %d\n", kMaxKVMStations);
#endif

	// Initialize share memory
	if (shm_init(-1) < 0) {
		return(kSystemError);
	}

	cy_shm->web_start_time = time(NULL);

#if LINUX_VERSION_CODE != KERNEL_VERSION(2,2,14)
	if ((fd = open("/dev/led",O_RDWR)) > 0) {
		ioctl(fd,0,&numPowerSupply);
		close(fd);
	} else {
		return(kSystemError);
	}
#endif

	get_picture();

#ifdef PMD
	setSignals();
#endif

	return(kCycSuccess);
}

void CycEndApi(void)
{
#ifdef PMD
	freeAllWebPmc();
#endif
}

//[RK]Jun/14/04 - Access Privilege
int CycRegUserAuthLocal(char * username)
{
	struct passwd *pw;

	pw = getpwnam(username);

	if (!pw || !strcmp(pw->pw_passwd,"!") ||
        !strcmp(pw->pw_passwd,"*")) {
		return 0; // authenticated in remote database
	} else {
		return 1; // authenticated in local database
	}	
}

int CycRegUserPasswd(char * username, char * oldpwd, char * newpwd)
{
	struct passwd *pw;
	char buffer[256];
	char *cp;
	FILE *fp;
    int shadow_file = 0;
	int i;
#ifdef SHADOWPASSWDS
	struct spwd *spwd = NULL;
#endif

	if (!(pw = getpwnam(username))) {
		syslog(LOG_ALERT, "%s: Change password by user [%s] was unsuccessful", 
			TAG_CONFIG, username); 
		return 2;
	}

	if (!strcmp(pw->pw_passwd,"!") ||
		!strcmp(pw->pw_passwd,"*")) {
		syslog(LOG_ALERT, "%s: Change password by user [%s] was unsuccessful", 
			TAG_CONFIG, username); 
		return 2;
	}

#ifdef SHADOWPASSWDS
	if (!strcmp(pw->pw_passwd,"x")) {
		setspent();
		spwd = getspnam(username);
		if (spwd) {
			pw->pw_passwd = spwd->sp_pwdp;
            shadow_file = 1;
		}
		endspent();
		if (!strcmp(pw->pw_passwd,"!") ||
			!strcmp(pw->pw_passwd,"*")) {
			syslog(LOG_ALERT, "%s: Change password by user [%s] was unsuccessful", 
				   TAG_CONFIG, username); 
			return 2;
		}
	}
#endif

	strcpy(buffer,pw->pw_passwd);

	cp = (char *) crypt(oldpwd,buffer);
// Bugfix 5211
//	if (strlen(cp) > 13) { // max encrypted password == CRYPT_EP_SIZE
//		cp[14] = 0x00;
//	}

	if (strcmp(cp,buffer)) {
		syslog(LOG_ALERT, "%s: Change password by user [%s] was unsuccessful", 
			TAG_CONFIG, username); 
		return 2; // old passwd is wrong
	} 
	
	fp = fopen("/tmp/webtemp", "w");
	fprintf(fp, "%s\n%s\n", newpwd, newpwd);
	fclose(fp);
	chmod("/tmp/webtemp",0600);
	sprintf(buffer, "passwd -p %s</tmp/webtemp >/tmp/webtemp2 2>/tmp/webtemp2", username);
	system(buffer);
	system("rm -f /tmp/webtemp");

	// check oper result
	buffer[0] = 0x00;
	if ((fp = fopen("/tmp/webtemp2","r")) != 0) {
		buffer[fread(buffer,1,256,fp)] = 0;
		fclose(fp);
	}
	system("rm -f /tmp/webtemp2");

	if (buffer[0]) {
		syslog(LOG_ALERT, "%s: Change password by user [%s] was unsuccessful", 
			TAG_CONFIG, username); 
		return 3; // problem with passwd 
	}

	syslog(LOG_INFO, "%s: Password changed for user [%s] by user [%s]", 
		TAG_CONFIG, username, username); 

    system("updatefiles /etc/passwd >/dev/null");
    if (shadow_file)
        system("updatefiles /etc/shadow >/dev/null");

    for (i = 0; i < MAX_WEB_USERS; i ++) {
        if (webUsers[i].sid == 0) {
            continue;
        }
        if (! strcmp(webUsers[i].userName, username)) {
            int  xorLen;
            char temp[12] = {0};
            char *pass;
            sprintf(temp, "%d", webUsers[i].sid);
            xorLen = EncryptString(temp, (char *) newpwd, 0);
            dolog("Encrypted pass: [%s]", newpwd);
            pass = HexEncode((char *)newpwd, xorLen);
            dolog("Encrypted pass in hex: [%s]", pass);
            strcpy(webUsers[i].pass, pass);
            webUsers[i].passK = webUsers[i].sid;
        }
        
    }
    return 1; // success
}
