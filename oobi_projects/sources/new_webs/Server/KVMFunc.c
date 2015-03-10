/* -*- linux-c -*- */
/*****************************************************************
 * File: KVMFunc.c
 *
 * Copyright (C) 2004 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Description: KVM Functions
 *
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <webs.h>
#include <Access.h>
#include <Error.h>
#include <GetSet.h>
#include <Locals.h>
#include <Localize.h>

#ifdef RDPenable
#include <time.h>
#endif	//RDPenable

#include "UserFunc.h"
#include "KVMFunc.h"
#include "PortsFunc.h"
#include "ResultCodes.h" /* define the Result enum */

#include <cyc.h>
#include <event_lib.h>
#include <names.h>

#include <src/server.h>
#include <src/mysyslog.h>

#include <userinfo.h>

extern struct cyc_info *cyc;

extern char_t kSidKey[]; // quasi-constant (DMF tolower()'s it, so it can't be a _real_ const)
extern WebUserInfo wUser;
extern char *websGetHTTPHost(void);

KVMportlist KVMport;
KVMReqData gKVMReq;

#ifdef RDPenable
RDPsrvlist  RDPsrv;
tRDPconnect  RDPconnect[MAXRDPCONNECT];
int numRDPconnect = 0;
unsigned char KVM_only[] = "(KVM)";
unsigned char RDP_only[] = "(Inband)";
unsigned char RDP_KVM[] =  "(Inband + KVM)";
#endif	//RDPenable

/***************************************************************** 
 ****************************************************************/ 
//void KVMGetUserInfo(char_t *username, KVMportlist *KVMport)
/*Christine Qiu
  Task: Enhanced Authorization
  doing: add new parameter char *authGroupName in KVMGetUserInfo
*/

void KVMGetUserInfo(char_t *username, KVMportlist *KVMport, char_t *authGroupName)
{
	char *tmpfile = "/tmp/.tmpinfo";
	char buf[128], cmd[128];
	FILE *fp;
	unsigned char *s, *t, perm;

	KVMport->user[0] = 0;
	if (username) {
		strncpy(KVMport->user, username, 32);
	}
	KVMport->nports = 0;
	KVMport->admin = 0;
	
	if(authGroupName == NULL){	
	snprintf(cmd, 128, "kvm getperms%s%s > %s",
		(username && *username) ? " -u " : "", 
		(username && *username) ? username : "", tmpfile);
	}
	else{
	snprintf(cmd, 128, "kvm getperms%s%s -X \"%s\" > %s",
		(username && *username) ? " -u " : "", 
		(username && *username) ? username : "",authGroupName, tmpfile);
	}
	system(cmd);

	if((fp = fopen(tmpfile, "r")) == NULL) {
		printf("Error opening pipe for command %s (NULL)", cmd);
		return;
	}
	if(fp == (FILE *)-1) {
		printf("Error opening pipe for command %s (-1)", cmd);
		return;
	}

	fcntl(fileno(fp), F_SETFL, fcntl(fp, F_GETFL) & ~O_NONBLOCK);

	if (! fgets(buf, 128, fp)) {
		fclose(fp);
		return;
	}

	if (strstr(buf, ADMIN_GROUP)) {
		KVMport->admin = 1;
	}
	else if (strstr(buf, "regular")) {
		
	}
	else {
		fclose(fp);
		return;
	}

	while (!feof(fp)){
		if(!fgets(buf, 128, fp))
			continue;
		if ((s = strchr(buf, '"')) == NULL) {
			continue;
		}
		t = s + 1;
		if ((s = strchr(t, '"')) == NULL) {
			continue;
		}
		*s = 0;
		strncpy(KVMport->port[KVMport->nports].name, t, 32);
		if ((s = strchr(s + 1, '"')) == NULL) {
			continue;
		}
		t = s + 1;
		if ((s = strchr(t, '"')) == NULL) {
			continue;
		}
		*s++ = 0;
		strncpy(KVMport->port[KVMport->nports].help, t, 32);
		if (! isspace(*s)) {
			continue;
		}
		s ++;
		for (; isspace(*s); s++);
		perm = 0;
		for (; *s; s++) {
			switch (*s) {
			case 'r':
				perm |= PERM_READ;
				break;
			case 'w':
				perm |= PERM_WRITE;
				break;
			case 'p':
				perm |= PERM_POWER;
				break;
#ifdef RDPenable
			case 'i':
				perm |= PERM_RDP;
				break;
#endif	//RDPenable
			}
		}
		KVMport->port[KVMport->nports].perm = perm;
		KVMport->nports ++;
	}
	fclose(fp);
	unlink(tmpfile);
}


#ifdef RDPenable
void RDPGetSrvInfo()
{
	char *tmpfile = "/tmp/.tmpinfo";
	char buf[128], cmd[128];
	FILE *fp;
	unsigned char *s, *t;

	RDPsrv.nservers = 0;

	snprintf(cmd, 128, "kvm getrdpsrv > %s", tmpfile);
	system(cmd);

	if((fp = fopen(tmpfile, "r")) == NULL) {
		printf("Error opening pipe for command %s (NULL)", cmd);
		return;
	}
	if(fp == (FILE *)-1) {
		printf("Error opening pipe for command %s (-1)", cmd);
		return;
	}

	fcntl(fileno(fp), F_SETFL, fcntl(fp, F_GETFL) & ~O_NONBLOCK);

	while (!feof(fp)){
		if(!fgets(buf, 128, fp))
			continue;
		if ((s = strchr(buf, '"')) == NULL) {
			continue;
		}
		t = s + 1;
		if ((s = strchr(t, '"')) == NULL) {
			continue;
		}
		*s = 0;
		strncpy(RDPsrv.server[RDPsrv.nservers].name, t, lRDP_name-1);
		if ((s = strchr(s + 1, '"')) == NULL) {
			continue;
		}
		t = s + 1;
		if ((s = strchr(t, '"')) == NULL) {
			continue;
		}
		*s = 0;
		strncpy(RDPsrv.server[RDPsrv.nservers].ip_port, t, lRDP_ipport-1);
		if ((s = strchr(s + 1, '"')) == NULL) {
			continue;
		}
		t = s + 1;
		if ((s = strchr(t, '"')) == NULL) {
			continue;
		}
		*s = 0;
		strncpy(RDPsrv.server[RDPsrv.nservers].kvmport, t, lRDP_kvmport-1);
		RDPsrv.nservers++;
	}

	fclose(fp);
	unlink(tmpfile);
}

int GetRDPConnectKVMPorts(Param* param, int request)
{
	int size;
	int i;
	char_t *bufHtml;
	Result result;

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	KVMGetUserInfo(wUser.uName, &KVMport, wUser.uGroupName);
	bfreeSafe(B_L, gCache.rdpdevice.RDPconnectKVMPorts);
	size = (MAXConnectLine * MAXKVMPORTS);
	// dynamic memory allocation
	bufHtml = gCache.rdpdevice.RDPconnectKVMPorts = balloc(B_L, size);
	if (! bufHtml) {
		return mAppError(kMemAllocationFailure);
	}
	// initialize the memory with NULL.
	memset(bufHtml, 0, size);
	gsprintf(bufHtml + strlen(bufHtml), 
	"<option value=\"None\"> None</option>\n");
	for (i = 0; i < KVMport.nports; i ++) {
		gsprintf(bufHtml + strlen(bufHtml), 
		"<option value=\"%s\" %s> %s</option>\n",
		KVMport.port[i].name,
		(!strcmp(KVMport.port[i].name, gCache.rdpdevice.confDevice.kvmport))?"selected":"",
		KVMport.port[i].name);
	}
	return mAppError(kCycSuccess);
}

#endif	//RDPenable

/***************************************************************** 
 ****************************************************************/ 
int GetKVMConnectPortsHtml(Param* param, int request)
{
	int size;
	int i;
	char_t *bufHtml;
	Result result;

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	KVMGetUserInfo(wUser.uName, &KVMport, wUser.uGroupName);
	bfreeSafe(B_L, gCache.KVMconnectPortsHtml);
	size = (MAXConnectLine * MAXKVMPORTS);
#ifdef RDPenable
	RDPGetSrvInfo();
	size += MAXRDPSERVERS;
#endif	//RDPenable

	// dynamic memory allocation
	bufHtml = gCache.KVMconnectPortsHtml = balloc(B_L, size);
	if (! bufHtml) {
		return mAppError(kMemAllocationFailure);
	}
	// initialize the memory with NULL.
	memset(bufHtml, 0, size);	
#ifndef RDPenable
	for (i = 0; i < KVMport.nports; i ++) {
		gsprintf(bufHtml + strlen(bufHtml), 
		"<option value=\"%s\"> %s</option>\n",
		KVMport.port[i].name, KVMport.port[i].name);
	}
#else	//RDPenable
	for (i = 0; i < KVMport.nports; i ++) {
		int jx;
		char *pip = NULL;

		for (jx=0; jx<RDPsrv.nservers; jx++) {
			if (!strcmp(KVMport.port[i].name, RDPsrv.server[jx].kvmport)) {
				pip = RDPsrv.server[jx].ip_port;
			}
		}
		if (KVMport.port[i].perm & PERM_RDP) {
			gsprintf(bufHtml + strlen(bufHtml), 
			"<option value=\"%s#KVM:%s\"> %s %s</option>\n",
			KVMport.port[i].name,
			(pip == NULL) ? "NULL" : pip,
			KVMport.port[i].name, RDP_KVM);
		} else {
			gsprintf(bufHtml + strlen(bufHtml), 
			"<option value=\"%s\"> %s %s</option>\n",
			KVMport.port[i].name,
			KVMport.port[i].name, KVM_only);
		}
	}

	for (i = 0; i < RDPsrv.nservers; i ++) {
		if (strcmp(RDPsrv.server[i].kvmport, "(null)")) continue;
		gsprintf(bufHtml + strlen(bufHtml), 
		"<option value=\"%s#RDP:%s\"> %s %s</option>\n",
		RDPsrv.server[i].name, RDPsrv.server[i].ip_port,
		RDPsrv.server[i].name, RDP_only);
	}
#endif	//RDPenable

	return mAppError(kCycSuccess);
}
/*Christine Qiu
  Task: Enhanced Authorization
  doing: add new parameter char *authGroupName for kvm_ipconnec()
*/
void kvm_ipconnect(char *portname, char *username,
		   char *client_ip, char *http_ip, char *station,
		   char *cmd_output, char *permission, int sid,
		   char *authGroupName) 
{

	char ssid[32];
	int max_len = MAX_RESULT_LEN;

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		syslog(LOG_DEBUG, "%s portname: %s username: %s client_ip: %s http_ip: %s station: %s permission: %s authGroupName: %s",
		       __func__, portname, username, client_ip, http_ip,
		       station, permission, authGroupName);
	}

	if (authGroupName != NULL) {
 		char *k_argv[]={ "kvm","ipconnect",
			 	"-u",username,
			 	"-X",authGroupName,
			 	"-d",portname,
			 	"-v",client_ip,
			 	"-S",ssid,
			 	"-P",permission,
                         	"-s",station,
			 	NULL };

		sprintf(ssid, "%d", sid);
		if (permission == NULL) {
			k_argv[11] = "full";
		}
		if (strstr(station, "-1")) {
			k_argv[14] = NULL;//Christine Qiu, Task:Enhanced Authorization,: add "-X", authGroupName, so here is 14
		}
		libcyc_system_str(cyc, KVM_COMMAND, k_argv,
				  cmd_output, &max_len);
		/* Replace the last character (newline) with a
	   	null terminator */
		cmd_output[strlen(cmd_output)-1]='\0';
	} else {
		char *k_argv[]={ "kvm","ipconnect",
			 	"-u",username,
			 	"-d",portname,
			 	"-v",client_ip,
			 	"-S",ssid,
			 	"-P",permission,
                         	"-s",station,
			 	NULL };

		sprintf(ssid, "%d", sid);
		if (permission == NULL) {
			k_argv[11] = "full";
		}
		if (strstr(station, "-1")) {
			k_argv[12] = NULL;
		}
		libcyc_system_str(cyc, KVM_COMMAND, k_argv,
				  cmd_output, &max_len);
		/* Replace the last character (newline) with a
	   	null terminator */
		cmd_output[strlen(cmd_output)-1]='\0';
	}
	
}

static unsigned char *pwebnotactive = "<p>Your web session is no longer "
	"active (timed out). If you wish to log in again, "
	"<a href=\"/login.asp\">click here</a>.</p>";
static unsigned char *pquitsession = "<p>Quitting the session ...</p>";
static unsigned char *pdenied = "<p>Access to this port was denied!</p>";
static unsigned char *pportinactive = "<p>This port is not active!</p>";
static unsigned char *pnouserstation = "<p>No user station was available for connection.</p>";
static unsigned char *pinvalidport = "<p>This port name is not valid!</p>";
static unsigned char *pinvalidviewer = "<p>No TCP Viewer port available to establish IP connection!</p>";
static unsigned char *plaunching = "<p>Launching the KVM Viewer...</p>\n";
static unsigned char *perroroccurred = "<p>An error has occurred launching the Viewer</p>";

void WriteSniffMenu(webs_t wp, unsigned char *id, unsigned char *portname, 
	       unsigned char type)
{
	websWrite(wp, 
	"<script type=\"text/javascript\">\n"
	"   sniff = 1;\n"
	"</script>\n");
	websWrite(wp, 
	"<form name=\"connectForm\">\n"
	"   <input type=\"hidden\" name=\"station\" value=\"%s\">\n"
	"   <table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" align=\"center\" width=\"100%%\" height=\"100%%\">\n"
	"   <tr><td align=\"center\" valign=\"center\">\n"
        "      <table>\n"
	"      <tr><td align=\"center\" valign=\"center\">\n"
	"         <b>", id);
	if (type) {
		websWrite(wp, "The server %s is being used in another "
			"session!", portname);
	} else {
		websWrite(wp, "%s - The port to the cascade device is being "
			"used by another session!", portname);
	}
	websWrite(wp, "</b>\n" 
	"      </td></tr>\n"
	"      <tr><td align=\"center\" valign=\"center\">\n"
	"         <table>\n");
	websWrite(wp, 
	"         <tr>\n"
	"            <td><input type=\"radio\" name=\"option\" value=\"1\" checked></td>\n"
	"            <td>Quit this session</td>\n"
	"         </tr>\n");
	if (type) {
	websWrite(wp, 
	"         <tr>\n"
	"            <td><input type=\"radio\" name=\"option\" value=\"2\"></td>\n"
	"            <td>Connect read only</td>\n"
	"         </tr>\n");
	websWrite(wp, 
	"         <tr>\n"
	"            <td><input type=\"radio\" name=\"option\" value=\"3\"></td>\n"
	"            <td>Connect read write</td>\n"
	"         </tr>\n");
	}
	websWrite(wp, 
	"         <tr>\n"
	"            <td><input type=\"radio\" name=\"option\" value=\"4\"></td>\n"
	"            <td>Kill other session</td>\n"
	"         </tr>\n");
	websWrite(wp, 
	"         </table>\n"
        "      </td></tr>\n"
	"      <tr><td align=\"center\">\n"
	"         <a href=\"javascript:connect();\">\n"
	"            <img src=\"/Images/OK_button.gif\" alt=\"\" width=\"35\" height=\"21\" border=\"0\">\n"
	"         </a>\n"
	"      </td></tr>\n"
	"      </table>\n"
	"   </td></tr>\n"
	"   </table>\n"
	"</form>");
}

static unsigned char *pcall_popup = "<script type=\"text/javascript\">\n"
	"newWindow('/normal/applications/accessConnectPopUp.asp?tabx=%s&SSID=%d', 'kvmcontrol', 600, 400, 'yes');"
	"</script>\n";

void handle_kvmipctl_output(struct cyc_info *cyc, char *buf, int buf_sz,
			    void *handle)
{
	webs_t *wp = NULL;

	if (handle == NULL) return;

	wp = (webs_t *)handle;

	websWriteBlock(*wp, buf, buf_sz);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		syslog(LOG_DEBUG,
		       "%s wrote %d bytes to websWriteBlock buf: %s",
		       __func__, buf_sz, buf);
	}
}

void portMountViewerHtml(char *portname, char *sniff, char *otherstation, 
			 webs_t wp)
{
	char_t *username=KVMport.user;
	char cmd_output[MAX_RESULT_LEN];
	char *http_host_str=websGetHTTPHost();
	char *kvmip_req=websGetRequestIpaddr(wp);
	char *permission = "full";
	char *prev_username = NULL;

	/* Return if there is no username specified; the user
	   timeout has been reached. */
	if (strcmp(username, "") == 0) {
		websWrite(wp, pwebnotactive);
		return;
	}

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		syslog(LOG_DEBUG,
		       "%s portname: %s sniff: %s otherstation: %s\n",
			__func__, portname, sniff, otherstation);
	}

	if (*sniff != '0') {
		char *s_argv[]={ "kvm","setperm","-s", otherstation,
				 "-P", NULL, NULL };

		*otherstation = libcyc_kvm_get_kvmipm_id_from_station(cyc, *otherstation);
		
		if ((prev_username = libcyc_kvm_get_username_from_station(cyc, atoi(otherstation))) == NULL) {
			prev_username = libcyc_strdup(cyc, "unknown");
		}

		if (*sniff == '1') {
			/* Quit this session */
			websWrite(wp, pquitsession);
			return;
		} else if (*sniff == '2') {
			/* Connect read only */
			permission = "ro";
		} else if (*sniff == '3') {
			char *physport_str = libcyc_kvm_get_physport_from_alias(cyc, portname);

			/* Connect read write; other session ro */
			s_argv[5] = "ro";
			if (physport_str) {
				writeevt(39, "ssss", username, prev_username,
					 portname, physport_str);
		
				libcyc_free(cyc, physport_str);
				physport_str = NULL;
			}
			libcyc_system(cyc, KVM_COMMAND, s_argv);
		} else if (*sniff == '4') {
			char *physport_str = libcyc_kvm_get_physport_from_alias(cyc, portname);

			/* Disconnect other session */
			s_argv[5] = "none";
			if (physport_str) {
				writeevt(38, "ssss", username, prev_username,
					 portname, physport_str);
		
				libcyc_free(cyc, physport_str);
				physport_str = NULL;
			}
			libcyc_system(cyc, KVM_COMMAND, s_argv);
		}
		libcyc_free(cyc, prev_username);
		prev_username = NULL;
	}

	/*Christine Qiu
  	Task: Enhanced Authorization
  	doing: add new parameter wUser.uGroupName
	*/
	kvm_ipconnect(portname, username, kvmip_req, http_host_str, "-1",
		      cmd_output, permission, GetWindowNumber(wp),
		      wUser.uGroupName);
	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		syslog(LOG_DEBUG, "%s after kvm_ipconnect() cmd_output: %s",
		       __func__, cmd_output);
	}
	if (strstr(cmd_output, "DENIED")) {
		websWrite(wp, pdenied);
	} else if (strstr(cmd_output, "INACTIVE PORT")) {
		websWrite(wp, pportinactive);
	} else if (strstr(cmd_output, "NO USER STATION")) {
		websWrite(wp, pnouserstation);
	} else if (strstr(cmd_output, "INVALID PORT NAME")) {
		websWrite(wp, pinvalidport);
	} else if (strstr(cmd_output, "INVALID VIEWER PORT")) {
		websWrite(wp, pinvalidviewer);
	} else if (strstr(cmd_output, "CONNECT")) {
		char *kvmip_ret=strdup(&cmd_output[13]);
		char *kvmip_id=kvmip_ret;
		char *kvmport_viewer=kvmip_ret+2;
		char viewer_title[MAX_STR_LEN];
		char monitor_runfile[MAX_STR_LEN];
		pid_t monitor_pid = ERROR;
		
		kvmip_id[1] = '\0';
		kvmip_id[0] = libcyc_kvm_get_kvmipm_id_from_station(cyc, kvmip_id[0]);

		libcyc_kvm_viewer_title_create(cyc, viewer_title, MAX_STR_LEN,
					       portname, atoi(kvmip_id));
		{
		char *e_argv[]={ "kvmipctl", "-l", "-i", kvmip_id,
				 "-n", http_host_str, "-p", kvmport_viewer,
				 "-t", viewer_title, NULL };
		char *m_argv[]={ "monitor", kvmip_id, username,
				 portname, kvmport_viewer, NULL };
		char *c_argv[]={ "kvm", "ipdisconnect","-s", kvmip_id, NULL };
		
		if (libcyc_get_flags(cyc) & CYC_DEBUG) {
			syslog(LOG_DEBUG,
				"%s: Initiating connection kvmip_id: %s\n",
				__func__,kvmip_id);
			syslog(LOG_DEBUG,"%s: websGetHTTPHost: %s\n",
				__func__,http_host_str);
		}
		
		websWrite(wp, plaunching);

		/* Output the HTML code to launch the viewer */
		if (libcyc_system_pipe(cyc, KVMIPCTL_COMMAND, e_argv,
				       handle_kvmipctl_output, &wp)) {
			websWrite(wp,perroroccurred);
			kvmip_id[0] = libcyc_kvm_get_station_from_kvmipm_id(cyc, kvmip_id[0]);
			/* Disconnect to free the port after an error */
			libcyc_system(cyc, KVM_COMMAND, c_argv);
			goto cleanup;
		}

		/* If there's a monitor process running on this port already,
		   kill it before we continue */
		snprintf(monitor_runfile, MAX_STR_LEN,
			 "/var/run/monitor-%s.pid", kvmport_viewer);
		monitor_runfile[MAX_STR_LEN-1] = '\0';
		monitor_pid = libcyc_get_pid_from_runfile(monitor_runfile);

		if (libcyc_get_flags(cyc) & CYC_DEBUG) {
			syslog(LOG_DEBUG, "%s monitor_runfile: %s monitor_pid: %d",
			       __func__, monitor_runfile, (int)monitor_pid);
		}
		if (monitor_pid > 0) {
			kill(monitor_pid, SIGTERM);
			waitpid(monitor_pid, NULL, 0);
		}

		/* Start the monitor application to begin monitoring
		   and logging the connection */
		libcyc_system_nowait(cyc, MONITOR_COMMAND, m_argv);
		if (libcyc_kvm_get_model(cyc) == MODEL_KVMNET_PLUS ||
			(libcyc_kvm_get_model(cyc) == MODEL_ONSITE &&
			 libcyc_kvm_get_bd_ver(cyc) == 2)) {
			websWrite(wp, pcall_popup, kvmip_id, gReq.sid);
		}
		
cleanup:
		/* Clean up */
		free(kvmip_ret);
		kvmip_id=NULL;
		bfree(B_L,http_host_str);
		http_host_str=NULL;
		}
	} else if (strstr(cmd_output, "BUSY")) {
		char *kvmip_ret=strdup(&cmd_output[10]);
		char *kvmip_id=kvmip_ret;

		kvmip_id[1] = '\0';
		kvmip_id[0] = libcyc_kvm_get_kvmipm_id_from_station(cyc, kvmip_id[0]);
		WriteSniffMenu(wp, kvmip_id, portname, 1);
		free(kvmip_ret);
	} else if (strstr(cmd_output, "NOT AVAILABLE")) {
		char *kvmip_ret=strdup(&cmd_output[19]);
		char *kvmip_id=kvmip_ret;

		kvmip_id[1] = '\0';
		kvmip_id[0] = libcyc_kvm_get_kvmipm_id_from_station(cyc, kvmip_id[0]);
		WriteSniffMenu(wp, kvmip_id, portname, 0);
		free(kvmip_ret);
#ifdef IPCASCADING
	} else if (strstr(cmd_output, "REDIRECT")) {
		char_t *temp = NULL;
		if (kDmfSuccess == dmfGetParam(kSidKey, NULL, &temp, -1)) {
			websWrite(wp, "<script type=\"text/javascript\">"
				"location.replace(\"%s&sid=%d&reqnum=%d\");"
				"</script>", cmd_output + 9, temp, 
				dmfGetRequestNumber());
			bfreeSafe(B_L, temp);
		}
#endif
	} else {
		websWrite(wp,"<p>Unknown result: %s</p>", cmd_output);
	}
}



int KVMConnectPort(Param* param, int request, char_t* urlQuery)
{
	return(0);
}

int KVMConnQuery(Param* param, int request)
{
	return(0);
}

#ifdef IPCASCADING
void KVMIPCascadeConnection(webs_t wp, UInt32 ipsecurity, char_t *username, 
	char_t *portname, char_t *port, char_t *master)
{
	char cmd_output[MAX_RESULT_LEN];
	char *kvmip_req=websGetRequestIpaddr(wp);
	char *k_argv[]={ "kvm","ipconnect",
			 "-u",username,
			 "-d",port,
			 "-v",kvmip_req,
			 NULL };
	char ipsecurity_str[2];

	/* write the header */
//printf("IPCascade pass 1\n");
	/* Make sure the ASP page loads this value first, otherwise
	  it won't exist */	
	snprintf(ipsecurity_str,2,"%d", ipsecurity);
	
	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		syslog(LOG_DEBUG,"%s entering\n",__func__);
		syslog(LOG_DEBUG,"%s: username: %s portname: %s\n",__func__,
			username,portname);
		syslog(LOG_DEBUG,"%s: ipsecurity: %s\n",__func__,
			ipsecurity_str);
	}

	cyc_system_pipe(cmd_output,MAX_RESULT_LEN,
			 KVM_COMMAND,k_argv);
	/* Replace the last character (newline) with a
	   null terminator */
	cmd_output[strlen(cmd_output)-1]='\0';

//printf("IPCascade pass 3 - output = %s\n", cmd_output);
	if (strstr(cmd_output, "DENIED")) {
		websWrite(wp,"Access to this port was denied");
	} else if (strstr(cmd_output, "INACTIVE PORT")) {
		websWrite(wp,"This port is not active!");
	} else if (strstr(cmd_output, "NO USER STATION")) {
		websWrite(wp,"No user station was available for connection");
	} else if (strstr(cmd_output, "INVALID PORT NAME")) {
		websWrite(wp,"This port name is not valid!");
	} else if (strstr(cmd_output, "CONNECT")) {
		char *http_host_str=websGetHTTPHost();
		char *kvmip_ret=strdup(&cmd_output[13]);
		char *kvmip_id=kvmip_ret;
		char *kvmport_viewer=kvmip_ret+2;
		kvmip_id[1] = '\0';

		{
        char *e_argv[]={ "proxy","launch-viewer",
                 http_host_str,kvmip_id,kvmport_viewer,NULL };
		char *s_argv[]={ "proxy","post-security-setting",
				 ipsecurity_str,kvmip_id,NULL };
		char *m_argv[]= { "monitor",kvmip_id,username,portname,NULL };

		if (libcyc_get_flags(cyc) & CYC_DEBUG) {
			syslog(LOG_DEBUG,
				"%s: Initiating connection kvmip_id: %s\n",
				__func__,kvmip_id);
		}

//printf("IPCascade pass 4\n");
		websWrite(wp,"Launching the KVM Viewer...\n");
		libcyc_system(cyc, PROXY_COMMAND,s_argv);
		webs_system_pipe(wp,PROXY_COMMAND,e_argv);
		libcyc_system_nowait(cyc, MONITOR_COMMAND, m_argv);

		free(kvmip_ret);
		kvmip_id=NULL;
		}
	} else {
		websWrite(wp,"UNKNOWN");
	}
//printf("IPCascade pass 5\n");
}
#endif

int checkPortStatusNet(char_t *station, char_t *ip, webs_t wp)
{
	char cmd_output[MAX_RESULT_LEN];
	char *portini, *portend;
	char *k_argv[]={ "kvm","status",
			 "-s",station,
			 NULL };
	int max_len = MAX_RESULT_LEN;

	libcyc_system_str(cyc, KVM_COMMAND, k_argv,
			  cmd_output, &max_len);

	/* Replace the last character (newline) with a
	   null terminator */
	cmd_output[strlen(cmd_output)-1]='\0';
	cmd_output[MAX_RESULT_LEN-1]='\0';

	if (strstr(cmd_output, ip)) {
		portini = strstr(cmd_output, "Current server: ");
		portend = strstr(cmd_output, "Connection status: ");
		if (portini && portend) {
			*(portend-1) = '\0';
			websWrite(wp, portini+16);
			return 1;
		}
	}
	return 0;
}

int KVMSwitchStation(webs_t wp, char_t *station, char_t *device, char_t *sniffopt, char_t *otherstation)
{
	char_t *username=KVMport.user;
	char cmd_output[MAX_RESULT_LEN];
	char *k_argv[]={ "kvm", "reconnect",
			 "-s", station,
			 "-d", device,
			 "-P", "full",
			 NULL };
	char *s_argv[]={ "kvm", "setperm",
			 "-s", otherstation,
			 "-P", NULL,
			 NULL };
	char *prev_username = NULL;
	int max_len = MAX_RESULT_LEN;

	*otherstation = libcyc_kvm_get_station_from_kvmipm_id(cyc, *otherstation);
	
	if ((prev_username = libcyc_kvm_get_username_from_station(cyc, atoi(otherstation))) == NULL) {
		prev_username = libcyc_strdup(cyc, "unknown");
	}

	if (*sniffopt == '1') {
		/* Quit the session */
		websWrite(wp, "sniff[%s-1]=\"\";", station);
		return 0;
	} else if (*sniffopt == '2') {
		/* Connect read only */
		k_argv[7] = "ro";
	} else if (*sniffopt == '3') {
		char *physport_str = libcyc_kvm_get_physport_from_alias(cyc, device);

		/* Connect read write */
		s_argv[5] = "ro";

		if (physport_str) {
			writeevt(39, "ssss", username, prev_username,
				 device, physport_str);
			
			libcyc_free(cyc, physport_str);
			physport_str = NULL;
		}
		libcyc_system(cyc, KVM_COMMAND, s_argv);
	} else if (*sniffopt == '4') {
		char *physport_str = libcyc_kvm_get_physport_from_alias(cyc, device);

		/* Disconnect other session */
		s_argv[5] = "none";

		if (physport_str) {
			writeevt(38, "ssss", username, prev_username,
				 device, physport_str);
			
			libcyc_free(cyc, physport_str);
			physport_str = NULL;
		}
		libcyc_system(cyc, KVM_COMMAND, s_argv);
	} else if (*sniffopt == '5') {
		/* Disconnect the session */
		s_argv[3] = station;
		s_argv[5] = "none";
		*station = libcyc_kvm_get_station_from_kvmipm_id(cyc, *station);
		libcyc_system(cyc, KVM_COMMAND, s_argv);
		websWrite(wp, "sniff[%s-1]=\"\";", station);
		return 0;
	}
	libcyc_free(cyc, prev_username);
	prev_username = NULL;

	*station = libcyc_kvm_get_station_from_kvmipm_id(cyc, *station);
	libcyc_system_str(cyc, KVM_COMMAND, k_argv,
			  cmd_output, &max_len);
	*station = libcyc_kvm_get_kvmipm_id_from_station(cyc, *station);

	if ((strstr(cmd_output, "BUSY")) != NULL) {
		char *kvmip_ret=strdup(&cmd_output[10]);
		char *kvmip_id=kvmip_ret;

		kvmip_id[1] = '\0';
		kvmip_id[0] = libcyc_kvm_get_kvmipm_id_from_station(cyc, kvmip_id[0]);
		websWrite(wp, "sniff[%s-1]=\"%s %s\";", station, kvmip_id, 
			device);
		free(kvmip_ret);
	} else {
		websWrite(wp, "sniff[%s-1]=\"\";", station);
	}
	return 0;
}

int KVMCycleStation(char_t *station)
{
	char *k_argv[]={ "kvm", "cycle",
			 "-s", station,
			 NULL };

	return(libcyc_system(cyc, KVM_COMMAND, k_argv));
}

unsigned int GetWindowNumber(webs_t wp)
{
	int ip1, ip2, ip3, ip4;
	sscanf(websGetRequestIpaddr(wp), "%d.%d.%d.%d", &ip1, &ip2, &ip3, &ip4);
	return ((ip1 << 24) + (ip2 << 16) + (ip3 << 8) + ip4);
}


#ifdef RDPenable
extern int RdpConnectionExist(unsigned short port);

static int check_rdpconnect(unsigned char *srvip, unsigned char *cliip,
			unsigned short srvport, unsigned short cliport)
{
	int ix, jx = -1;

	if (RdpConnectionExist(cliport)) return -1;
	for (ix=0; ix<MAXRDPCONNECT; ix++) {
		if (cliport == RDPconnect[ix].cliport) {
			memset ((unsigned char *)&RDPconnect[ix], 0, 
					sizeof(tRDPconnect));
			if (jx == -1) jx = ix;
			break;
		}
		if (RDPconnect[ix].cliport == 0 && jx == -1) jx = ix;
	}	

	if (jx >= 0) {
		strncpy(RDPconnect[jx].srvip, srvip, lRDP_ipport-1);
		strncpy(RDPconnect[jx].cliip, cliip, lRDP_ipport-1);
		strncpy(RDPconnect[jx].kvmuser, wUser.uName, lRDP_user-1);
		RDPconnect[jx].srvport = srvport;
		RDPconnect[jx].cliport = cliport;
		RDPconnect[jx].conn_time = (int)time(NULL);

		syslog(LOG_DEBUG, 
				"New RDP connection. User %s, Port %d, ID %d. Server: %s:%d",
				RDPconnect[jx].kvmuser, RDPconnect[jx].cliport, 
				RDPconnect[jx].conn_time, 
				RDPconnect[jx].srvip, RDPconnect[jx].srvport);

		return jx;
	}
	
	return -1;
}

void portMountRDPHtml(char *portname, char *connect, webs_t wp)
{
	char *kvmip=websGetIpaddr();
	char *cliip=websGetRequestIpaddr(wp);
	unsigned char srvip[lRDP_ipport];
	unsigned char srvname[kRDPGenLength];
	struct hostent *hp;
	unsigned short srvport = 0;
	unsigned char cports[MAXRDPLIST], *token;
	unsigned short cliports[MAXRDPCONNECT];
	unsigned char type = 0, code = 0;
	int ix, jx;
	char pcli[8];
	char psrv[8];

	*cports = 0;
	for (ix = 0; ix < MAXRDPCONNECT; ix++) {
		cliports[ix] = 0;
	}
	if (sscanf(connect, "KVM:%s %d %s", srvname, &ix, cports) >= 1) {
		srvport = (unsigned short)ix;
		type = 2;
		if (!strcmp(srvname, "NULL")) {
			code = 1;
			goto KVM_connection;
		}
	} else if (sscanf(connect, "RDP:%s %d %s", 
						srvname, &ix, cports) == 3) {
		srvport = (unsigned short)ix;
		type = 1;
	} else {
		goto KVM_connection;
	}

	memset(srvip, 0, lRDP_ipport);
	if ((hp = gethostbyname(srvname)) != NULL) {
		struct in_addr in;
		memcpy ((char *) &in, (char *)hp->h_addr, hp->h_length);
		strncpy (srvip, inet_ntoa(in), 16);
	} else {
        strncpy (srvip, srvname, kRDPGenLength);
	}

	if (srvport == 0 || !libcyc_kvm_rdpserver_probe(cyc, srvip, srvport)) {
		code = 2;
		goto KVM_connection;
	}

	for (ix = 0; ix < MAXRDPCONNECT; ix++) {
		if (!strcmp(srvip, RDPconnect[ix].srvip) && 
			!strcmp(cliip, RDPconnect[ix].cliip) &&
			RdpConnectionExist(RDPconnect[ix].cliport)) {
// Connection already exists.
			code = 3;
			goto KVM_connection;
		}
	}

	if (*cports) {
		for (ix = 0; ix < MAXRDPCONNECT; ix++) {
			int num;
			token = strtok((ix==0)?cports:NULL, "|");
			if (token == NULL) break;
			num = strtol(token, NULL, 10);
			cliports[ix] = (num < MIN_RDP_PORT || num > MAX_RDP_PORT)?0:num;
		}
	}
		
	for (ix = 0; ix < MAXRDPCONNECT; ix++) {
		if (cliports[ix] == 0) continue;
		jx = check_rdpconnect(srvip, cliip, srvport, cliports[ix]); 
		if (jx >= 0) break;
	}
	if (ix >= MAXRDPCONNECT) {
// No more RDP Connections 
		code = 4;
		goto KVM_connection;
	}

	sprintf(pcli, "%d", cliports[ix] );
	sprintf(psrv, "%d", srvport );
{
	char_t *username = KVMport.user;
	char *s_argv[]={ "rdp_conntrack", username,
			 pcli, cliip, kvmip, srvip, psrv, NULL };

	/* Return if there is no username specified; the user
	   timeout has been reached. */
	if (strcmp(username,"") == 0) {
		websWrite(wp, pwebnotactive);
		return;
	}

	libcyc_system(cyc, RDP_CONNTRACK_COMMAND, s_argv);
}

// Print web page with activex
	websWrite(wp, " onload=\"RDPconnect(\'%s\', \'%s\', \'%d\', \'%u\');\" ",
				portname, kvmip, cliports[ix], RDPconnect[jx].conn_time);
	return;

KVM_connection:

// Back to KVM connection
	websWrite(wp, " onload=\"KVMconnect(%d, %d);\" ", type, code);

}
#endif	//RDPenable
