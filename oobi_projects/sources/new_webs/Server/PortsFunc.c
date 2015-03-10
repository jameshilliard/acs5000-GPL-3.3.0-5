#include "webs.h"
#include <sys/ioctl.h>
#include <stdio.h>

#include "Access.h"
#include "Error.h"
#include "GetSet.h"
#include "Locals.h"
#include "Localize.h"
#include "UserFunc.h"
#include  <src/server.h>

#include "ResultCodes.h" /* define the Result enum */

extern SystemData gCache;
extern SInt8 gPorts[];

#define itoa(sid, temp, len) snprintf(temp, len, "%d", sid)

extern void ConvTextArea(char *bufConf, char *bufWeb, int to);

char *fields_statistics[]={" baud:"," tx:"," rx:"," fe:"," pe:"," brk:"," oe:"};

void ConvInitChat(char *bufConf, char *bufWeb)
{
	unsigned char *d, *s;

	d = bufConf;
	s = bufWeb;

	while (*s) {
		if ((*s == '\r') && (*(s+1) == '\n')) {
			*d++ = ' ';
			s+=2;
			continue;
		}
		*d++ = *s++;
	}
	*d=0x00;
}

//[RK]Jun/03/05 - Group Authorization Support
static int check_access(struct line_cfg *pconf ,char *user,char *group_name)
{

#ifdef ONS
#define NULL_NOT_VALID 1
#else
#define NULL_NOT_VALID 0
#endif

	if (group_name) {
		if (!check_group_name_access(user, group_name, pconf->users, pconf->admin_users, NULL_NOT_VALID)) {
			return(1);
		}
	} else {
		if (!check_users_access(pconf->users, user, NULL_NOT_VALID)) {
			return(1);
		} else {
			if (!check_users_access(pconf->admin_users, user, 1)) {
				return(1);
			}
		}
	}
	return 0;
}

void portMountConnectPortsHtml(char *username, CycUserType userType,char * group_name)
{
	/* Fill in connectPortsHtml */
	char_t templateStr1[] = "<option value=\"%d/%d\"> %s </option>\n";
	char_t templateStr2[] = "<option value=\"%d/%d\"> %s %d </option>\n";
	char_t templateFilled[100] = {0};
	int x;
	//[RK]Feb/24/05 - problem with clustering
	struct line_cfg* pconf;
	int size;

#ifdef ONS
	for (x=0, size=0, pconf=&lineconf[0]; x < gCache.device.number2; x++, pconf++) {
#else
	for (x=0, size=0, pconf=&lineconf[0]; x < MAXVLINES; x++, pconf++) {
#endif
		if ((pconf->this != x) || (!pconf->tty || !*pconf->tty)) {
			continue;
		}	

		//[RK]Jun/16/04 - SSH/Telnet to IPDU
		if ((pconf->protocol != P_SOCKET_SERVER) &&
			(pconf->protocol != P_SOCKET_SERVER_SSH) &&
#ifdef BIDIRECT
			(pconf->protocol != P_TELNET_BIDIRECT) &&
#endif
			(pconf->protocol != P_SOCKET_SERVER_RAW) &&
		    (pconf->protocol != P_SOCKET_SSH)) {
#ifdef PMD
			if ((pconf->protocol == P_IPDU) && 
			     (pconf->pmsessions & (PMSESS_TELNET | PMSESS_SSH))) {
			} else 
#endif
			{
				continue;
			}
		}
		size++; 
	}
	size *= sizeof(templateFilled);
	/* warning: `return' with a value, in function returning void 
	//dynamic memory allocation
	if (!(gCache.connectPortsHtml = balloc(B_L, size)))
		return mAppError(kMemAllocationFailure); //mp */
	if ((gCache.connectPortsHtml = balloc(B_L, size)))
	{
		// initialize the memory with NULL.
		memset(gCache.connectPortsHtml, 0, size);

#ifdef ONS
		for (x=0, pconf=&lineconf[0]; x < gCache.device.number2; x++, pconf++) {
#else
		for (x=0, pconf=&lineconf[0]; x < MAXVLINES; x++, pconf++) {
#endif
			if (pconf->this != x) {
				continue;
			}

			if (!pconf->tty || !*pconf->tty) {
				continue;
			}	

			//[RK]Jun/16/04 - SSH/Telnet to IPDU
			if ((pconf->protocol != P_SOCKET_SERVER) &&
				(pconf->protocol != P_SOCKET_SERVER_SSH) &&
#ifdef BIDIRECT
				(pconf->protocol != P_TELNET_BIDIRECT) &&
#endif
				(pconf->protocol != P_SOCKET_SERVER_RAW) &&
			    (pconf->protocol != P_SOCKET_SSH)) {
#ifdef PMD
				if ((pconf->protocol == P_IPDU) && 
				     (pconf->pmsessions & (PMSESS_TELNET | PMSESS_SSH))) {
				} else 
#endif
				{
					continue;
				}
			}

			if (userType!=kAdminUser) {
				//[RK]Jun/03/05 - Group Authorization Support
				if ( !check_access(pconf,username,group_name) ) {
					continue;
				}
			}

			if (pconf->biometric && (CycUserBioName(username) != kCycSuccess)) {
				continue;
			} 

			if (pconf->alias) {
				gsprintf(templateFilled, templateStr1, x, 
#ifdef WINEMSenable
					lineconf[x].web_WinEMS, 
#else
					0,
#endif
					pconf->alias); 
			} else {
				if (pconf->virtual)
					gsprintf(templateFilled, templateStr2, x, 
#ifdef WINEMSenable
						lineconf[x].web_WinEMS, 
#else
						0,
#endif
						"Remote Port", x+1); 
				else
					gsprintf(templateFilled, templateStr2, x, 
#ifdef WINEMSenable
						lineconf[x].web_WinEMS, 
#else
						0,
#endif
						"Port", x+1); 
			}
			gstrcat(gCache.connectPortsHtml, templateFilled);
		}
	}
}

void boxMountAppletHtml(char_t *mysid, char_t *myname, char_t *pass, webs_t wp)
{
 	/* To add more parameters, make sure you add them in two places. The first section
	   is for IE and the 2nd part is for Netscape syntax. */

	/*** IE Browser Syntax ***/
	// Applet info
	websWrite(wp, "<OBJECT classid=\"clsid:8AD9C840-044E-11D1-B3E9-00805F499D93\" "
   	"width=\"590\" height=\"470\" align=\"baseline \""
   	"codebase=\"http://java.sun.com/products/plugin/autodl/jinstall-1_4_2-windows-i586.cab#Version=1,4,2,mn\">\n");
#ifdef MINDTERM
   websWrite (wp, "<param name=\"code\" value=\"com.mindterm.application.MindTerm3.class\">\n"
#else
   websWrite (wp, "<param name=\"code\" value=\"de.mud.jta.Applet\">\n"
#endif
   	"<param name=\"codebase\" value=\".\">\n"
      "<param name=\"type\" value=\"application/x-java-applet;version=1.4.2\">\n"
      "<param name=\"cache_option\" value=\"Browser\">\n"
#ifdef MINDTERM
      "<param name=\"cache_archive\" value=\"mindterm.jar\">\n"
#else
      "<param name=\"cache_archive\" value=\"jta25.jar\">\n"
#endif
      "<param name=\"cache_version\" value=\"1.0.0.10\">\n"
#ifdef MINDTERM
	"<param name=\"term-type\" value=\"vt100\">\n"
	"<param name=\"port\" value=\"22\">\n"
	"<param name=\"exit-on-logout\" value=\"true\">\n");
#else
		"<param name=\"config\" value=\"applet.conf\">\n");
#endif
	// User name
	websWrite(wp,"<param name=\"username\" value=\"%s\">\n", myname);

#ifndef MINDTERM
	websWrite(wp, "<param name=\"plugins\" value=\"%s\">\n", "Status,Jsch,ButtonBar,Terminal");
#endif
	// Protocol 
	websWrite(wp, "<param name=\"proto\" value=\"89\">\n");

	// System type (ACS/APM)
	websWrite(wp, "<param name=\"unit\" value=\"%s\">\n", "acs");

	// Session ID
	websWrite(wp, "<param name=\"parma\" value=\"%s\">\n", mysid);
	websWrite(wp, "<param name=\"parmb\" value=\"%s\">\n", pass);
#ifdef MINDTERM
	websWrite(wp, "<param name=\"fg-color\" value=\"white\">\n"
		      "<param name=\"bg-color\" value=\"black\">\n"
		      "<param name=\"cursor-color\" value=\"white\">\n"
		      //"<param name=\"debug\" value=\"true\">\n"
		      "<param name=\"quiet\" value=\"true\">\n"
		      "<param name=\"skipPasswordMethod\" value=\"true\">\n"
		      "<param name=\"ems\" value=\"disable\">\n");
#endif
	/*** Netscape Browser Syntax ***/
	websWrite(wp, "<COMMENT>\n"
   	"<EMBED type=\"application/x-java-applet;version=1.4.2\" pluginspage=\"http://www.java.com/en/index.jsp\"\n"
   	"width=\"590\" height=\"470\" align=\"baseline\" "
#ifdef MINDTERM
   	"code=\"com.mindterm.application.MindTerm3.class\"\n");
#else
   	"align=\"baseline\" code=\"de.mud.jta.Applet\"\n");
#endif
	websWrite(wp, "codebase=\".\"\n"
		"cache_option=\"Browser\"\n"
#ifdef MINDTERM
		"cache_archive=\"mindterm.jar\"\n"
#else
		"cache_archive=\"jta25.jar\"\n"
#endif
		"cache_version=\"1.0.0.10\"\n"
#ifdef MINDTERM
		"term-type=\"vt100\"\n"
		"port=\"22\"\n"
		"exit-on-logout=\"true\"\n"
#else
		"config=\"applet.conf\"\n"
		"plugins=\"Status,Jsch,ButtonBar,Terminal\"\n"
#endif
		"proto=\"89\"\n"
		"username=\"%s\"\n"
		"unit=\"acs\"\n"
      "parma=\"%s\"\n", myname, mysid);
	websWrite(wp, "parmb=\"%s\"\n", pass);
#ifdef MINDTERM
	websWrite(wp, "fg-color=\"white\"\n"
		      "bg-color=\"black\"\n"
		      "cursor-color=\"white\"\n"
		      //"debug=\"true\"\n"
		      "quiet=\"true\"\n"
		      "skipPasswordMethod=\"true\"\n"
		      "ems=\"disable\">\n");
#endif
   websWrite(wp, "</EMBED>\n<NOEMBED>\n"
  		"No JDK 1.4.2 Support for APPLET!!\n"
      "</NOEMBED>\n</COMMENT>\n"
    	"</OBJECT>\n");
}

void portMountGrapherAppletHtml(int portNum, char_t *mysid, char_t *myname, char_t *pass, webs_t wp)
{
	int tcp_port = portNum + 600;

    /* To add more parameters, make sure you add them in two places. 
       The first section is for IE and the 2nd part is for Netscape syntax. */

    /*** IE Browser Syntax ***/

	websWrite(wp, "<OBJECT classid=\"clsid:8AD9C840-044E-11D1-B3E9-00805F499D93\" "
    "width=\"890\" height=\"600\" align=\"baseline \""
    "codebase=\"http://java.sun.com/products/plugin/autodl/jinstall-1_4_2-windows-i586.cab#Version=1,4,2,mn\">\n");
	websWrite(wp, "<param name=\"code\" value=\"GrapherApplet.class\">\n"
    "<param name=\"codebase\" value=\".\">\n"
    "<param name=\"type\" value=\"application/x-java-applet;version=1.4.2\">\n"
    "<param name=\"cache_option\" value=\"Browser\">\n"
    "<param name=\"cache_archive\" value=\"temp.jar\">\n"
    "<param name=\"cache_version\" value=\"1.0.0.0\">\n"
    "<param name=\"Port\" value=\"%d\">\n", tcp_port);

    /*** Netscape Browser Syntax ***/

	websWrite(wp, "<COMMENT>\n"
    "<EMBED type=\"application/x-java-applet;version=1.4.2\" pluginspage=\"http://www.java.com/en/index.jsp\"\n"
    "width=\"890\" height=\"600\" align=\"baseline\"\n"
    "code=\"GrapherApplet.class\" codebase=\".\"\n");
    websWrite(wp, "cache_option=\"Browser\"\n"
        "cache_archive=\"temp.jar\"\n"
        "cache_version=\"1.0.0.7\"\n"
        "Port=\"%d\"\n", tcp_port);
   websWrite(wp, "</EMBED>\n<NOEMBED>\n"
  		"No JDK 1.4.2 Support for APPLET!!\n"
      "</NOEMBED>\n</COMMENT>\n"
    	"</OBJECT>\n");

}

void portMountAppletHtml(int portNum, char_t *mysid, char_t *myname, char_t *pass, webs_t wp)
{
	int proto; //[RK]Jul/26/04 Bug#2166
#ifdef MINDTERM
	//[AP][2007-12-06] Bug #24943 - correct port number for NAT clustering
	int nat=0;
	char port[6] = "22", // temporary holder for the SSH port
		sport[6], // temporary holder for the converted socket port
		*lp, // the login-part SSH port
		*pp; // the SSH port

		pp = port;
		if (lineconf[portNum].virtual && mainconf.nat_clustering_ip) {
			char *p;
			if ((p=strstr(lineconf[portNum].tty, ":")) != NULL) {
				strncpy(port, (char*)(p+1), 5)[5] = '\0';
				nat=1;
				lp = port;
			}
		}
		
		if(!lineconf[portNum].socket_port) {
			if (nat) {
				pp = lineconf[portNum].alias? lineconf[portNum].alias : lineconf[portNum].tty;
			} else {
				lp = lineconf[portNum].alias? lineconf[portNum].alias : lineconf[portNum].tty;
			}
		} else {
			snprintf(sport, 5, "%d", lineconf[portNum].socket_port);
			if (nat) {
				pp = sport;
			} else {
				lp = sport;
			}
		}
#endif	
 	/* To add more parameters, make sure you add them in two places. The first section
	   is for IE and the 2nd part is for Netscape syntax. */

	/*** IE Browser Syntax ***/
	// Set up applet info
	websWrite(wp, "<OBJECT classid=\"clsid:8AD9C840-044E-11D1-B3E9-00805F499D93\" "
   	"width=\"%s\" height=\"%s\" align=\"baseline \""
   	"codebase=\"http://java.sun.com/products/plugin/autodl/jinstall-1_4_2-windows-i586.cab#Version=1,4,2,mn\">\n", 
#ifdef WINEMSenable
		lineconf[portNum].web_WinEMS ? "590" : "590",
		lineconf[portNum].web_WinEMS ? "670" : "470"
#else
		"590", "470"
#endif
	);
      websWrite (wp, "%s%s%s%s",
#ifdef MINDTERM
         //lineconf[portNum].web_WinEMS ?
         //"<param name=\"code\" VALUE=\"de.mud.jta.Applet\">\n" :
         "<param name=\"code\" VALUE=\"com.mindterm.application.MindTerm3.class\">\n",
#else
         "<param name=\"code\" VALUE=\"de.mud.jta.Applet\">\n",
#endif
   	  "<param name=\"codebase\" value=\".\">\n"
      "<param name=\"type\" value=\"application/x-java-applet;version=1.4.2\">\n"
      "<param name=\"cache_option\" value=\"Browser\">\n",
#ifdef MINDTERM
      "<param name=\"cache_archive\" value=\"mindterm.jar\">\n",
#else
      "<param name=\"cache_archive\" value=\"jta25.jar\">\n",
#endif
      "<param name=\"cache_version\" value=\"1.0.0.10\">\n");
#ifndef MINDTERM 
		websWrite (wp, "<param name=\"config\" value=\"%s\">\n",
#ifdef WINEMSenable
		lineconf[portNum].web_WinEMS ? "winApplet.conf" : "applet.conf"
#else
		"applet.conf"
#endif
	);
		
	// User name
	websWrite(wp,"<param name=\"username\" value=\"%s\">\n", myname);
#endif

	// Protocol number
	// [RK]Jul/26/04 - Bug#2166
#ifdef PMD
	if ((lineconf[portNum].protocol == P_IPDU) && 
	     (lineconf[portNum].pmsessions & (PMSESS_TELNET | PMSESS_SSH))) {
		if (lineconf[portNum].pmsessions & PMSESS_SSH) {
			proto = P_SOCKET_SSH;
		} else {
			proto = P_SOCKET_SERVER;
		}
	} else 
#endif
	{
		if (lineconf[portNum].protocol == P_SOCKET_SERVER_SSH) {
			proto = P_SOCKET_SSH;
#ifdef BIDIRECT
                }
		else if (lineconf[portNum].protocol == P_TELNET_BIDIRECT) {
			proto = P_SOCKET_SERVER;
#endif
		} else {
			proto = lineconf[portNum].protocol;
                }
	}
	
	websWrite(wp, "<param name=\"proto\" value=\"%d\">\n", proto); //[RK]Jul/26/04 Bug#2166
	// Protocol Socket SSH / Server_SSH
	if (proto == P_SOCKET_SSH) { //[RK]Jul/26/04 Bug#2166
#ifndef MINDTERM
		websWrite(wp, "<param name=\"plugins\" value=\"Status,Jsch,ButtonBar,Terminal\">\n");
#else
		if (lineconf[portNum].web_WinEMS)
		    websWrite(wp, "<param name=\"plugins\" value=\"Status,Jsch,ButtonBar,Terminal\">\n");
#endif
		// CHECK FOR BIOMETRIC
		if (CycUserBioName(myname) == kCycSuccess || lineconf[portNum].biometric) {
			websWrite(wp,"<param name=\"authtype\" value=\"bio\">\n");
		}

#ifdef MINDTERM
		websWrite(wp, "<param name=\"username\" value=\"%s:%s\">\n"
			"<param name=\"port\" value=\"%s\">\n", myname, lp, pp);
#else
		if(!lineconf[portNum].socket_port) {
			if(!lineconf[portNum].alias)
				websWrite(wp, "<param name=\"port\" value=\"%s\">\n", lineconf[portNum].tty);
			else
				websWrite(wp, "<param name=\"port\" value=\"%s\">\n", lineconf[portNum].alias);
		} else {
			websWrite(wp, "<param name=\"port\" value=\"%d\">\n", lineconf[portNum].socket_port);
		}
#endif
  	} // Protocol Socket Server/Raw
	else if (proto == P_SOCKET_SERVER || proto == P_SOCKET_SERVER_RAW) { //[RK]Jul/26/04 Bug#2166
#ifndef MINDTERM
		if (lineconf[portNum].authtype != AUTH_NONE) {
			websWrite(wp, "<param name=\"plugins\" value=\"Status,Socket,Telnet,Script,ButtonBar,Terminal\">\n");
		} else {
			websWrite(wp, "<param name=\"plugins\" value=\"Status,Socket,Telnet,ButtonBar,Terminal\">\n");
		}
		websWrite(wp, "<param name=\"port\" value=\"%d\">\n", lineconf[portNum].socket_port);
#else
	    websWrite(wp, "<param name=\"username\" value=\"%s\">\n"
                          "<param name=\"port\" value=\"%d\">\n", myname, lineconf[portNum].socket_port);
		if (lineconf[portNum].authtype != AUTH_NONE) 
			websWrite(wp, "<param name=\"phys-port-auth\" value=\"single\">\n");
		else
			websWrite(wp, "<param name=\"phys-port-auth\" value=\"none\">\n");
#endif
	}

	// Nat Clustering
#ifndef MINDTERM
	if (lineconf[portNum].virtual && mainconf.nat_clustering_ip) {
		//[RK]Mar/03/05 - pass the remote TCP port
		unsigned char *p;
		p = strstr(lineconf[portNum].tty, ":");
		if (p) {
			websWrite(wp, "<param name=\"NatClustering\" value=\"%s\">\n",(p+1));
		}
	}
#endif

	// Break Sequence
	if (lineconf[portNum].break_seq && (*lineconf[portNum].break_seq != 0)) 
		websWrite(wp, "<param name=\"breakseq\" value=\"%s\">\n", lineconf[portNum].break_seq);

	// System Type (ACS/APM)
	websWrite(wp, "<param name=\"unit\" value=\"acs\">\n");

	// Session ID
	websWrite(wp, "<param name=\"parma\" value=\"%s\">\n", mysid);
	websWrite(wp, "<param name=\"parmb\" value=\"%s\">\n", pass);

#ifdef MINDTERM
	websWrite(wp, "<param name=\"fg-color\" value=\"white\">\n"
		      "<param name=\"bg-color\" value=\"black\">\n"
		      "<param name=\"cursor-color\" value=\"white\">\n"
		      //"<param name=\"debug\" value=\"true\">\n"
		      "<param name=\"quiet\" value=\"true\">\n"
		      "<param name=\"skipPasswordMethod\" value=\"true\">\n"
		      "<param name=\"exit-on-logout\" value=\"true\">\n"
		      "<param name=\"ems\" value=\"%s\">\n",
		      lineconf[portNum].web_WinEMS? "enable":"disable");

	// Terminal Type
	if (lineconf[portNum].term && (*lineconf[portNum].term != 0)) {
		websWrite(wp,"<param name=\"term-type\" value=\"%s\">\n", lineconf[portNum].term);
	} else {
		websWrite(wp,"<param name=\"term-type\" value=\"vt100\">\n");
	}
#endif

	/*** Netscape Browser Syntax ***/
	// Applet info
	websWrite(wp, "<COMMENT>\n"
      "<EMBED type=\"application/x-java-applet;version=1.4.2\" pluginspage=\"http://www.java.com/en/index.jsp\"\n"
      "width=\"%s\" height=\"%s\" align=\"baseline\"\n",
#ifdef WINEMSenable
		lineconf[portNum].web_WinEMS ? "590" : "590",
        	lineconf[portNum].web_WinEMS ? "670" : "470"
#else
		"590", "470"
#endif
	);
   websWrite (wp, "codebase=\".\"\n"
      "cache_option=\"Browser\"\n%s%s",
#ifdef MINDTERM
      "code=\"com.mindterm.application.MindTerm3.class\"\n"
      "cache_archive=\"mindterm.jar\"\n",
#else
      "code=\"de.mud.jta.Applet\"\n"
      "cache_archive=\"jta25.jar\"\n",
#endif
      "cache_version=\"1.0.0.10\"\n");
#ifndef MINDTERM
    websWrite (wp, "config=\"%s\"\n",
#ifdef WINEMSenable
	lineconf[portNum].web_WinEMS ? "winApplet.conf" : "applet.conf"
#else
	"applet.conf"
#endif
	);

	// User name
	websWrite(wp,"username=\"%s\"\n", myname);
#endif
	// Protocol
	websWrite(wp, "proto=\"%d\"\n", proto); //[RK]Jul/26/04 Bug#2166
	
	// Protocol Socket SSH / Server_SSH
	if (proto == P_SOCKET_SSH) { //[RK]Jul/26/04 Bug#2166
#ifndef MINDTERM
		websWrite(wp, "plugins=\"Status,Jsch,ButtonBar,Terminal\"\n");
#endif
		// Biometric
		if (CycUserBioName(myname) == kCycSuccess || lineconf[portNum].biometric) {
			websWrite(wp,"authtype=\"bio\"\n");
		}

#ifdef MINDTERM
		websWrite(wp, "username=\"%s:%s\"\nport=\"%s\"\n", myname, lp, pp);
#else
		if(!lineconf[portNum].socket_port) {
			if(!lineconf[portNum].alias)
				websWrite(wp, "port=\"%s\"\n", lineconf[portNum].tty);
			else
				websWrite(wp, "port=\"%s\"\n", lineconf[portNum].alias);
		} else {
			websWrite(wp, "port=\"%d\"\n", lineconf[portNum].socket_port);
		}
#endif
	} // Protocol Socket Server/Raw
	else if (proto == P_SOCKET_SERVER || proto == P_SOCKET_SERVER_RAW)  //[RK]Jul/26/04 Bug#2166
	{
#ifndef MINDTERM
		if (lineconf[portNum].authtype != AUTH_NONE) {
			websWrite(wp, "plugins=\"Status,Socket,Telnet,Script,ButtonBar,Terminal\"\n");
		} else {
			websWrite(wp, "plugins=\"Status,Socket,Telnet,ButtonBar,Terminal\"\n");
		}
		websWrite(wp, "port=\"%d\"\n", lineconf[portNum].socket_port);
#else
		websWrite(wp, "username=\"%s\"\n"
   	                          "port=\"%d\"\n", myname, lineconf[portNum].socket_port);
		if (lineconf[portNum].authtype != AUTH_NONE) 
			websWrite(wp, "phys-port-auth=\"single\"\n");
		else
			websWrite(wp, "phys-port-auth=\"none\"\n");
#endif
	}

	// Nat Clustering
#ifndef MINDTERM
	if (lineconf[portNum].virtual && mainconf.nat_clustering_ip) {
		//[RK]Mar/03/05 - pass the remote TCP port
		unsigned char *p;
		p = strstr(lineconf[portNum].tty, ":");
		if (p) {
			websWrite(wp, "NatClustering=\"%s\"\n",(p+1));
		}
	}
#endif

	// Break Sequence
	if (lineconf[portNum].break_seq && (*lineconf[portNum].break_seq != 0)) 
		websWrite(wp, "breakseq=\"%s\"\n", lineconf[portNum].break_seq);

	// System Type (ACS/APM)
	websWrite(wp, "unit=\"acs\"\n");

	// Session ID
	websWrite(wp, "parma=\"%s\"\n", mysid);
	websWrite(wp, "parmb=\"%s\"", pass);
	
#ifdef MINDTERM
	websWrite(wp, "\nfg-color=\"white\"\n"
		      "bg-color=\"black\"\n"
		      "cursor-color=\"white\"\n"
		      //"debug=\"true\"\n"
		      "quiet=\"true\"\n"
		      "skipPasswordMethod=\"true\"\n"
		      "exit-on-logout=\"true\"\n"
		      "ems=\"%s\"\n",
			  lineconf[portNum].web_WinEMS? "enable":"disable");

	// Terminal Type
	if (lineconf[portNum].term && (*lineconf[portNum].term != 0)) {
		websWrite(wp,"term-type=\"%s\"", lineconf[portNum].term);
	} else {
		websWrite(wp, "term-type=\"vt100\"");
	}
#endif

	// Close up applet code
   websWrite(wp, ">\n</EMBED>\n<NOEMBED>\n"
  		"No JDK 1.4.2 Support for APPLET!!\n"
      "</NOEMBED>\n</COMMENT>\n"
    	"</OBJECT>\n");
}

void portMountPortsStatusHtml(char * bufHtml)
{
	int fd,signal_status;
	char_t status[30];
	CY_SESS *sess;
#ifdef PMD
	pmcInfo *pmc;
#endif
	char_t list[205];
	char_t buf[400],virg;
	int x,i;

	while (shm_get_set_val(0, &cy_shm->st_cy_ras, 0, CY_SEM_CONFIG) != 1) {
		sleep(1);
		continue;
	}
	  
	virg = ' ';

#ifdef ONS
	for (x=0 ; x < gCache.device.number2; x++) {
#else
	for (x=0 ; x < gCache.device.number; x++) {
#endif
		// get RS232 signal status
		sprintf(buf,"/dev/ttyS%d",x+1);
		memset(status,0,30);
		if ((fd = open(buf,O_RDONLY | O_NOCTTY | O_NONBLOCK)) > 0) {
			ioctl(fd,TIOCMGET,&signal_status);
			close(fd);
			if ((signal_status & TIOCM_RTS) && (signal_status & TIOCM_DTR)) {
				strcat(status,"RTS ");
			}
			if (signal_status & TIOCM_CTS) {
				strcat(status,"CTS ");
			}
			if (signal_status & TIOCM_DSR) {
				strcat(status,"DSR ");
			}
			if (signal_status & TIOCM_CD) {
				strcat(status,"DCD ");
			}
			if (signal_status & TIOCM_DTR) {
				strcat(status,"DTR");
			}
		}
		// get current user list
		list[0] = 0x00;
		if ((lineconf[x].protocol != P_SOCKET_SSH) &&
			(lineconf[x].protocol != P_SOCKET_SERVER) &&
		    (lineconf[x].protocol != P_SOCKET_SERVER_SSH) &&
#ifdef BIDIRECT
		    NOT_BIDIR_TCP(lineconf[x].protocol, lineconf[x].conntype, x) &&
#endif
		    (lineconf[x].protocol != P_SOCKET_SERVER_RAW) 
#ifdef PMD
		 && (lineconf[x].protocol != P_IPDU)
#endif
		) {
			strcpy(list,"Not CAS session");
		} else { 
#ifdef PMD
			if (lineconf[x].protocol == P_IPDU) { // PM
				if (lineconf[x].pmsessions != 0) {
					// look for user in pmclist
					for (i=0; i < MAXPMDACCESS ; i++) {
						if (cy_shm->pmc_list[i]) {
							pmc = cy_shm->pmc_list[i];
							if ((pmc->pmc_ipdu == x) && (pmc->pmc_ppid)) {
								strncat(list,pmc->pmc_username,200-strlen(list));
								strcat(list," ");
							} 
						}						    
					}
				}
			} else 
#endif
			{
				if (lineconf[x].fast) {		
					if (cy_shm->vet_pid[x]) { // there is one CAS session
						strcpy(list,cy_shm->tty_user[x]);
					}
				} else { // multiple users
					sess = cy_shm->sess_list[x];
					while (sess) {
						if (sess->tty_user) {
							strncat(list,sess->tty_user,200-strlen(list));
							strcat(list," ");
						}
						sess = sess->next;
					}	
				}
			}
		}

		sprintf(buf,"%c[\"%s\",\"%s\",\"%s\"]",virg,(lineconf[x].alias)? lineconf[x].alias : " ",status,list);
		strcat(bufHtml,buf);
		virg = ',';
	}
}

void portMountPortsStatisticsHtml(char * bufHtml)
{
    static int MAX_BUF=1024;
    FILE *fd;
    int posArr,
        x,
        i;
    char *ptr2,
         buff[MAX_BUF], 
	 line[MAX_BUF],
         *ptr,
         tmpbaud[10];

    char_t buf[400],virg;
    CY_SESS *sess;
#ifdef PMD
    pmcInfo *pmc;
#endif
    char_t list[205];

    struct SERIAL_INFO 
    {
       int tx, rx, tty, baud, frame, parity, overrun, brk;
    }; 
 

#ifdef ONS
      int skip_lines = 9;
      struct SERIAL_INFO serialinfo[gCache.device.number2+1];
#else
      int skip_lines = 1;
      struct SERIAL_INFO serialinfo[gCache.device.number+1];
#endif

	while (shm_get_set_val(0, &cy_shm->st_cy_ras, 0, CY_SEM_CONFIG) != 1) {
		sleep(1);
		continue;
	}
	  
	virg = ' ';


      //Check ACS model
#ifdef ONS
      if(gCache.device.number2 > 1)
#else
      if(gCache.device.number > 1)
#endif
      {
          sprintf(buf,"/proc/tty/driver/serial_ttyS"); //ACS-4,8,16,32,48...
      }
      else
          sprintf(buf,"/proc/tty/driver/serial"); //only ACS-1


        /* Get information about TTYs by /proc */
        if((fd = fopen(buf, "r")) != NULL)
        {
			for (i=0; i < skip_lines; i++)
            	fgets(line, MAX_BUF, fd); //skip first line

            x=0;
            while(fgets(line, MAX_BUF, fd))
            {
                ptr2=line;
                serialinfo[x].tty = 0;
                serialinfo[x].baud = 0;
                serialinfo[x].tx = 0;
                serialinfo[x].rx = 0;
                serialinfo[x].frame = 0;
                serialinfo[x].parity = 0;
                serialinfo[x].brk = 0;
                serialinfo[x].overrun = 0;

                if((ptr=strchr(ptr2, ':')) != NULL)
                {
                    strncpy(buff,ptr2,(ptr-ptr2));
                    buff[(ptr-ptr2)] = '\0';
                    serialinfo[x].tty = atoi(buff);
                }
                else
                    continue;

                //Get fields_statistics: BAUD,TX,RX,FRAME,PARITY,BREAK,OVERRUN
                for (posArr=0; posArr < (sizeof(fields_statistics)/4); posArr++)
                {
                    if((ptr2=strstr(ptr, fields_statistics[posArr])) != NULL)
                    {
                        ptr2+=strlen(fields_statistics[posArr]);
                        if((ptr=strchr(ptr2, ' ')) != NULL)
                        {
                            strncpy(buff,ptr2,(ptr-ptr2));
                            buff[(ptr-ptr2)] = '\0';
                            switch (posArr)
                            {
                                case 0:
                                    serialinfo[x].baud = atoi(buff);
                                    break;
                                case 1:
                                    serialinfo[x].tx = atoi(buff);
                                    break;
                                case 2:
                                    serialinfo[x].rx = atoi(buff);
                                    break;
                                case 3:
                                    serialinfo[x].frame = atoi(buff);
                                    break;
                                case 4:
                                    serialinfo[x].parity = atoi(buff);
                                    break;
                                case 5:
                                    serialinfo[x].brk = atoi(buff);
                                    break;
                                case 6:
                                    serialinfo[x].overrun = atoi(buff);
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
                x++;
            }
            fclose(fd);
        }

       virg = ' ';

#ifdef ONS
       for (x=0 ; x < gCache.device.number2; x++) 
#else
       for (x=0 ; x < gCache.device.number; x++) 
#endif
       {

		// get current user list
		list[0] = 0x00;
		if ((lineconf[x].protocol != P_SOCKET_SSH) &&
			(lineconf[x].protocol != P_SOCKET_SERVER) &&
			(lineconf[x].protocol != P_SOCKET_SERVER_SSH) &&
#ifdef BIDIRECT
			NOT_BIDIR_TCP(lineconf[x].protocol, lineconf[x].conntype, x) &&
#endif
		    (lineconf[x].protocol != P_SOCKET_SERVER_RAW) 
#ifdef PMD
		  && (lineconf[x].protocol != P_IPDU)
#endif
		) {
			strcpy(list,"Not CAS session");
		} else { 
#ifdef PMD
			if (lineconf[x].protocol == P_IPDU) { // PM
				if (lineconf[x].pmsessions != 0) {
					// look for user in pmclist
					for (i=0; i < MAXPMDACCESS ; i++) {
						if (cy_shm->pmc_list[i]) {
							pmc = cy_shm->pmc_list[i];
							if ((pmc->pmc_ipdu == x) && (pmc->pmc_ppid)) {
								strncat(list,pmc->pmc_username,200-strlen(list));
								strcat(list," ");
							} 
						}						    
					}
				}
			} else 
#endif
			{
				if (lineconf[x].fast) {		
					if (cy_shm->vet_pid[x]) { // there is one CAS session
						strcpy(list,cy_shm->tty_user[x]);
					}
				} else { // multiple users
					sess = cy_shm->sess_list[x];
					while (sess) {
						if (sess->tty_user) {
							strncat(list,sess->tty_user,200-strlen(list));
							strcat(list," ");
						}
						sess = sess->next;
					}	
				}
			}
		}

#ifdef ONS
      		if(gCache.device.number2 > 1)
#else
      		if(gCache.device.number > 1)
#endif
      		{
                    if(! serialinfo[x].baud)
                    {
                        strcpy(tmpbaud,"n/a");
                        tmpbaud[strlen("n/a")]='\0';
                    }
                    else
                        sprintf(tmpbaud,"%d",serialinfo[x].baud);
                
                    sprintf(buf,"%c[\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"]",virg
						                                 	     ,(lineconf[x].alias)? lineconf[x].alias : " "
						                                 	     ,tmpbaud
						                                 	     ,serialinfo[x].tx
						                                 	     ,serialinfo[x].rx
						                                 	     ,serialinfo[x].frame
						                                 	     ,serialinfo[x].parity
						                                 	     ,serialinfo[x].brk
						                                 	     ,serialinfo[x].overrun);
                }
      		else
      		{
                    if(! serialinfo[x+1].baud)
                    {
                        strcpy(tmpbaud,"n/a");
                        tmpbaud[strlen("n/a")]='\0';
                    }
                    else
                        sprintf(tmpbaud,"%d",serialinfo[x+1].baud);

          	    sprintf(buf,"%c[\"%s\",\"%s\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\"]",virg
						                         		     ,(lineconf[x].alias)? lineconf[x].alias : " "
						                         		     ,tmpbaud
						                         		     ,serialinfo[x+1].tx
						                         		     ,serialinfo[x+1].rx
						                         		     ,serialinfo[x+1].frame
						                         		     ,serialinfo[x+1].parity
						                         		     ,serialinfo[x+1].brk
						                         		     ,serialinfo[x+1].overrun);
      		}

	        strcat(bufHtml,buf);
	        virg = ',';
	}
}

void portMountPhysPortHtml(char * bufHtml)
{
	char_t temp[]="<option value=\"%d\">&nbsp;";
	char_t serialInfo[]="%d&nbsp;%1d%c%1d";
	char_t buf[200],parity='N';
	int x;
	PhysPortsInfo *phys;

	phys = &gCache.physP.physList[1];

#ifdef PMD
#ifndef ONS
	gCache.physP.numPowerMgm = 0;
#endif
#endif

#ifdef ONS
	for (x=0 ; x < gCache.device.number2; x++, phys++) {
#else
	for (x=0 ; x < gCache.device.number; x++, phys++) {
#endif
		gsprintf(buf,temp,x+1);
		gstrcat(bufHtml,buf);
		if (x < 9) {
			gstrcat(bufHtml,"&nbsp;");
		}
		gsprintf(buf,"%d&nbsp;",x+1);
		gstrcat(bufHtml,buf);
		
		// disable
		if (phys && !phys->general.enable) {
			gstrcat(bufHtml,"Yes&nbsp;&nbsp;&nbsp;");
		} else {
			gstrcat(bufHtml,"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
#ifdef PMD
#ifndef ONS
			if (phys->general.protocol == P_IPDU) { // IPDU
				gCache.physP.numPowerMgm++;
			}
#endif
#endif
		}

		// alias
		if (phys->general.alias[0]) 
			includeNbsp(buf,phys->general.alias,17); 
		else 
			includeNbsp(buf,0,17); 

		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"&nbsp;");

		// protocol
		switch (phys->general.protocol) {
			case P_SOCKET_SERVER : includeNbsp(buf,"Console (Telnet)",19);break;
			case P_SOCKET_SSH : includeNbsp(buf,"Console (SSH)",19);break;
                        case P_SOCKET_SERVER_SSH : includeNbsp(buf,"Console (TelnetSSH)",19);break;
			case P_SOCKET_SERVER_RAW : includeNbsp(buf,"Console (Raw)",19);break;
			case P_TELNET : includeNbsp(buf,"Telnet",19);break;
#ifdef BIDIRECT
			case P_TELNET_BIDIRECT : includeNbsp(buf,"BidirectionTelnet",19);break;
#endif
			case P_SSH : includeNbsp(buf,"SSHv1",19);break;
			case P_SSH2 : includeNbsp(buf,"SSHv2",19);break;
			case P_LOCAL : includeNbsp(buf,"Local Terminal",19);break;
			case P_SOCKET_CLIENT : includeNbsp(buf,"Raw Socket",19);break;
			case P_PPP_ONLY : includeNbsp(buf,"PPP-No Auth",19);break;
			case P_PPP : includeNbsp(buf,"PPP",19);break;
			case P_SLIP : includeNbsp(buf,"SLIP",19);break;
			case P_CSLIP : includeNbsp(buf,"CSLIP",19);break;
#ifdef PMD
			case P_IPDU : includeNbsp(buf,"Power Management",19);break;
#endif
			default : includeNbsp(buf,"Not defined",19);break;
		}		
		gstrcat(bufHtml,buf);

		// baudRate
		gsprintf(buf,"%d",phys->serial.baudRate);
		if (strlen(buf) < 7) {
			includeNbsp(buf,NULL,7-strlen(buf));	
		}
		gstrcat(bufHtml,buf);
		switch (phys->serial.parity) {
			case 1 : parity='N';break;
			case 2 : parity='O';break;
			case 3 : parity='E';break;
		}
		
		gsprintf(buf,serialInfo,phys->serial.baudRate,
				phys->serial.dataSize?phys->serial.dataSize:5,parity,
				phys->serial.stopBits?phys->serial.stopBits:1);
		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"</option>\n");
	}
}
#ifdef PMDNG
static void portGetPmConfig(int serialNumber, PhysPortsInfo *phys)
{
	PMDPortInfo *pinfo;
	int i;

	for (i=0,pinfo=gCache.pmdConf.ports; i < gCache.pmdConf.numports; i++,pinfo++) {
		if (pinfo->action != kDelete) {
			if (serialNumber) {
		    		if (pinfo->serialPort == serialNumber) {
					// serial info
					phys->serial.baudRate = pinfo->speed;
					phys->serial.dataSize = pinfo->dataSize;
					phys->serial.stopBits = pinfo->stopBits;
					phys->serial.parity = pinfo->parity;
					phys->serial.flowControl = pinfo->flowCtrl;
#ifdef PMDNG
					phys->general.pmVendor = pinfo->type;
#endif
					phys->general.stallowed = pinfo->stallowed;
					return;
				}
			} else { // ALL Ports
				phys->serial.baudRate = pinfo->speed;
				phys->serial.dataSize = pinfo->dataSize;
				phys->serial.stopBits = pinfo->stopBits;
				phys->serial.parity = pinfo->parity;
				phys->serial.flowControl = pinfo->flowCtrl;
#ifdef PMDNG
				phys->general.pmVendor = pinfo->type;
#endif
				phys->general.stallowed = 0; 
				return;
			}
		}
	}
	/* Assume default values */
	phys->serial.baudRate = kPbr9600;
	phys->serial.dataSize = kPb8;
	phys->serial.stopBits = kPsb1;
	phys->serial.parity = kPptNone;
	phys->serial.flowControl = kPftNone;
#ifdef PMDNG
	phys->general.pmVendor = Vendor_auto;
#endif
	phys->general.stallowed = 0;
}

static char *deviceInList(char *device, char *list)
{
	char *plist, *paux, *pbegin;
	char *ret = NULL;
	plist = strdup(list);
	paux = plist;
	while(isspace(*paux) || *paux == ',') paux ++;
	while (*paux) {
		pbegin = paux;
		while(*paux && !isspace(*paux) && *paux != ',') {
			if (*paux == '[') {
				while(*paux && *paux != ']') paux ++;
			} else {
				paux ++;
			}
		}
		if (*paux) *paux++ = 0;
		while(isspace(*paux) || *paux == ',') paux ++;
		if (*pbegin == '$' && ! strcmp(pbegin + 1, device)) {
			ret = list + (pbegin - plist);
			break;
		}
	}
	free(plist);
	return ret;
}

static void portGetPmUser(char *device, PhysPowerMgmInfo * power)
{
	PMDUsersMgm *puser;
	int i,len;
	char sep;

	if (gCache.pmdConf.numusers == 0) return;
	
	len = strlen(device);
	sep = ' ';
	power->userList[0] = 0x00;
	for (i=0,puser=gCache.pmdConf.users; i < gCache.pmdConf.numusers;
		i++,puser++) {
		if ((puser->action != kDelete) &&
			deviceInList(device, puser->outletList)) {
			sprintf(power->userList+strlen(power->userList),
				 "%c%s",sep,puser->user);
			sep = ',';
		}
	}
}

static void portSetPmUserList(char *device, char * userList)
{
	char *plist, *pusr, *paux, *p1, *p2, *pdev;
	int i;
	PMDUsersMgm *puser = NULL;

	for (i = 0; i < gCache.pmdConf.numusers; i++) {
		puser = gCache.pmdConf.users + i;
		/* Check for the device */
		if (puser->action == kDelete) {
			continue;
		}
		pdev = deviceInList(device, puser->outletList);
		if (pdev == NULL) {
			continue;
		}
		/* Check if the user is in the list */
		plist = strdup(userList);
		pusr = plist;
		while (*pusr) {
			paux = pusr;
			while (*paux && *paux!=',' && !isspace(*paux)) paux++;
			if (*paux) *paux++ = 0x00;
			if (*pusr && ! strcmp(puser->user, pusr)) {
				break;
			}
			pusr = paux;
		}
		if (*pusr) {
			free(plist);
			continue;
		}
		free(plist);

		/* Remove device in the user list */
		p1 = pdev - 1;
		while (p1 >= puser->outletList && (isspace(*p1) || *p1 == ',')) p1 --;
		p1 ++;
		p2 = pdev;
		while (*p2 && !isspace(*p2) && *p2 != ',') {
			if (*p2 == '[') {
				while (*p2 && *p2 != ']') p2 ++;
			} else {
				p2 ++;
			}
		}
		while (isspace(*p2) || *p2 == ',') p2 ++;
		if (p1 <= puser->outletList) {
			strcpy(puser->outletList, p2);
		} else if (*p2) {
			*p1 = ',';
			strcpy(p1 + 1, p2);
		} else {
			*p1 = 0;
		}

		/* If new outlet list is empty, remove user entry */
		for (p2 = puser->outletList; isspace(*p2) || *p2 == ','; p2 ++);
		if (*p2) {
			puser->action = kEdit;
		} else {
			puser->action = kDelete;
		}
	}

	plist = strdup(userList);
	pusr = plist;
	while (*pusr) {
		paux = pusr;
		while (*paux && *paux!=',' && !isspace(*paux)) paux++;
		if (*paux) *paux++ = 0x00;
		if (*pusr) {
			for (i = 0; i < gCache.pmdConf.numusers; i++) {
				puser = gCache.pmdConf.users + i;
				if ((puser->action != kDelete) && ! strcmp(puser->user, pusr)) {
					puser->action = kEdit;
					break;
				}
			}
	
			if (i == gCache.pmdConf.numusers) {
				// add new user
				puser = gCache.pmdConf.users;
				gCache.pmdConf.users = calloc(1,sizeof(PMDUsersMgm) * 
					(gCache.pmdConf.numusers+1));
				if (puser) {
					memcpy(gCache.pmdConf.users, puser, 
						gCache.pmdConf.numusers * sizeof(PMDUsersMgm));
					free(puser);
				}
				puser = &gCache.pmdConf.users[gCache.pmdConf.numusers++];
				strcpy(puser->user, pusr);
				sprintf(puser->outletList, "$%s", device);
				puser->action = kAdd;
			} else if (! deviceInList(device, puser->outletList)) {
				// add another entry in the outlet list
				strcat(puser->outletList, ",$");
				strcat(puser->outletList, device);
			}
		}
		pusr = paux;
	}
	free(plist);
}

static void portSetPmOutlets(int serial, PhysPowerMgmInfo * power)
{
	PMDOutGrpsDef *pgrp;
	char device[10];
	int i;

	sprintf(device,"ttyS%d",serial);

	for (i=0, pgrp=gCache.pmdConf.grps; i < gCache.pmdConf.numgrps; i++,pgrp++) {
		if ((pgrp->action != kDelete) && 
		     !strcmp(pgrp->group, device)) {
			if (power->enable == kNo) {
				pgrp->action = kDelete;
				return;
			}
			if (pgrp->action != kAdd) 
				pgrp->action = kEdit;
			break;
		}
	}

	if (power->enable == kNo) return;

	if ( i == gCache.pmdConf.numgrps) {
		// add a new group
		pgrp = gCache.pmdConf.grps;
		gCache.pmdConf.grps = calloc(1,sizeof(PMDOutGrpsDef) * 
					(gCache.pmdConf.numgrps+1));
		if (pgrp) {
			memcpy(gCache.pmdConf.grps, pgrp, 
				gCache.pmdConf.numgrps * sizeof(PMDOutGrpsDef));
			free(pgrp);
		}
		pgrp = &gCache.pmdConf.grps[gCache.pmdConf.numgrps++];
		pgrp->action = kAdd;
	}
	strcpy(pgrp->group,device);
	strcpy(pgrp->outletList, power->outletList);

	// check the userList
	if (!power->allUsers && power->userList[0]) {
		portSetPmUserList(device,power->userList);
	}
}

static void portGetPmOutlets(int nport, char * user, PhysPowerMgmInfo * power)
{
	PMDOutGrpsDef *pgrp;
	int i;
	char device[10];

	sprintf(device,"ttyS%d",nport);

	if (gCache.pmdConf.numgrps == 0) return;

	for (i=0, pgrp=gCache.pmdConf.grps; i < gCache.pmdConf.numgrps; i++,pgrp++) {
		if ((pgrp->action != kDelete) && 
		     !strcmp(pgrp->group, device)) {
			break;
		}
	}
	if ( i == gCache.pmdConf.numgrps) return;

	// get the outlets and fill the power struct
	power->numOutlets = 1;
	strcpy(power->outletList,pgrp->outletList);
	if (user && !strcmp(user,"all")) {
		power->allUsers = 1;	
	} else {
		portGetPmUser(device,power);
	}
}

#endif
#ifdef OLDPMD // OLD format
static int portPMcheckUserAccess(char *pmusers, char *user,int outlet)
{
	char *s, *t;
	int first, last;
	char outStr[50];
	
	for (s = pmusers; *s;) {
		while (*s && (*s == ' ')) s++;
		if (strncmp(s,user,strlen(user)) ||
		    *(s + strlen(user)) != ':') {
			//next
			while (*s && (*s != ';')) s++;
			if (*s) s++;
			continue;
		}
		break;
	}
	if (!*s) return(false);
	
	// user found
	while (*s && (*s != ':')) s++;	
	if (!*s) return(false);
	s++;
	while (*s && (*s == ' ')) s++;
	if (!*s) return(false);
	t = s;
	while (*s && (*s != ';')) s++;

	memset(outStr,0,50);
	strncpy(outStr,t,(s-t));
		
	t = outStr;
	while (*t) {
		while(*t && (*t == ' ')) t++;
		if (sscanf(t,"%d-%d",&first,&last) == 2) {
			if ((outlet >= first) && (outlet <= last)) {
				return(true);
			}
		} else {
			if (sscanf(t,"%d",&first)==1) {
				if (outlet == first) {
					return(true);
				}
			}
		}
		while(*t && (*t != ',')) t++;
		if (*t) t++;
	}
	return(false);
}

static int portPmChangeUserList(char *old, char *new, PhysPowerMgmInfo *power)
{      //mp: now returns 0=OK or kMemAllocationFailure
	int ipdu, outl,numl,first,last,found;
	int i,j,k;
	struct PMuserListAction { 
		int action;
		char username[kUserNameLength];
	} *list;
	char *po,*pn,user[kUserNameLength],outlet[20];
	char secOut[kMaxOutletsSP],aux[50];
	char *newpm,*plist,virg;

	i = sizeof(struct PMuserListAction) * 130; //[RK]Jan/30/06
	if (!(list = (struct PMuserListAction *) balloc(B_L,i))) return kMemAllocationFailure;
	memset(list,0,i);
	numl = 0;
	
	// Add users in the list
	po = new;
	while (*po) {
		memset(user,0,kUserNameLength);
		pn = po;
		while (*po && (*po != ',')) po++;
		memcpy(user,pn, (po-pn));
		if (*po) po++;
		sprintf(list[numl].username,"%s",user);
		list[numl++].action = kAdd;
	}

	po = old;
	while (*po) {
		memset(user,0,kUserNameLength);
		pn = po;
		while (*po && (*po != ',')) po++;
		memcpy(user,pn, (po-pn));
		if (*po) po++;
		pn=strstr(new,user);
		if (!pn || ((*(pn+strlen(user))) && (*(pn+strlen(user)) != ','))) {
			sprintf(list[numl].username,"%s",user);
			list[numl++].action = kDelete;
		}
	}

	if (numl == 0) return 0;
	
	if (!(newpm = balloc(B_L,kPmUserListLength))) return kMemAllocationFailure;
	ipdu = 0;
	for (i=0; i < power->numOutlets; i++) {
		if (ipdu && (ipdu == power->outlet[i].portNum)) continue;
		
		ipdu = power->outlet[i].portNum;
		memset(newpm,0,kPmUserListLength);
		plist = gCache.physP.physList[ipdu].general.pmUserList; 
		while (*plist) {
			while (*plist && (*plist == ' ')) plist++;
			pn = plist;
			while(*plist && (*plist!=':')) plist++;
			if (!*plist) break;
			*plist++=0x00;
			strcpy(user,pn);
			while (*plist && (*plist == ' ')) plist++;
			pn = plist;
			while (*plist && (*plist != ';')) plist++;
			if (*plist) *plist++=0x00;
			strcpy(outlet,pn);
			memset(secOut,0,kMaxOutletsSP);
			while (*pn) {
				while(*pn && (*pn == ' ')) pn++;
				if (sscanf(pn,"%d-%d",&first,&last) == 2) {
					for (j=first; j <=last ; j++) {
						secOut[j] = 1;
					}
				} else {
					sscanf(pn,"%d",&first);
					secOut[first] = 1;
				}
				while(*pn && (*pn != ',')) pn++;
				if (*pn) pn++;
			}	
			for (j=0;j<numl;j++) {
				if (!strcmp(user,list[j].username)) {
					found = 1;
					if (list[j].action == kDelete) {
						outl = 0;
					} else {
						list[j].action = kEdit;
						outl = 1;
					}
						
					for (k=i; k< power->numOutlets;k++) {
						if (power->outlet[k].portNum != ipdu) break;
						secOut[power->outlet[k].outNum] = outl;
					}
					break;
				}
			}
			if (found) {
				first = last = 0;
				virg=' ';
				for (j=0; j < kMaxOutletsSP ; j++) {
					if (secOut[j]) {
						if (!first) 
							first=last=j;
						else
							last = j;
					} else {
						if (last) {
							if (virg == ' ') {
								sprintf(aux," %s:",user);
							}
							if (last != first) 
								sprintf(user,"%c%d-%d",virg,first,last);
							else
								sprintf(user,"%c%d",virg,first);
							strcat(aux,user);
							virg =',';
							last = first = 0;
						}
					}
				}									
				if (virg == ',') {
				       	strcat(aux,";");
					strcat(newpm,aux);
				}
			} else {
				sprintf(aux," %s:%s;",user,outlet);
				strcat(newpm,aux);
			}
		}
		
		for (j=0;j<numl;j++) {
				if (list[j].action != kAdd) {
					if (list[j].action == kEdit){
						list[j].action = kAdd;
					}
					continue;
				}
				sprintf(aux," %s:%d",list[j].username,power->outlet[i].outNum);
				for (k=i+1; k< power->numOutlets;k++) {
					if (power->outlet[k].portNum != ipdu) break;
					sprintf(outlet,",%d",power->outlet[k].outNum);
					strcat(aux,outlet);
				}
				strcat(aux,";");
				strcat(newpm,aux);
		}
		strcpy(gCache.physP.physList[ipdu].general.pmUserList,newpm); 
		gCache.physP.physList[ipdu].general.action = kSpec;
		gCache.physP.physList[ipdu].action = kEdit;
	}

	bfreeSafe(B_L,list);
	bfreeSafe(B_L,newpm);
	return 0;	//[RK]Jul/03/04 - need to return 0 bug#2090
}

#ifdef OLDPMD
void portPmFindUserlist(PhysPowerMgmInfo *power)
{
	int ipdu,outl,i,access,ipdu1;
	char *paux,outStr[50], *ps;

	access = 0;
	ipdu = power->outlet[0].portNum;
	outl = power->outlet[0].outNum;
	if (gCache.physP.physList[ipdu].general.pmUserList[0]){
		for (paux = gCache.physP.physList[ipdu].general.pmUserList; *paux;) {
			while (*paux && (*paux == ' ')) paux++;
			ps = paux;
			while (*paux && (*paux!=':')) paux++;
			if (!*paux) break;
			memset(outStr,0,50);
			strncpy(outStr,ps,(paux-ps));

			access = true;
			for(i=0; i < power->numOutlets; i++) {
				ipdu1=power->outlet[i].portNum;
				if (gCache.physP.physList[ipdu1].general.pmUserList[0]){
					access = portPMcheckUserAccess(gCache.physP.physList[ipdu1].general.pmUserList,
						outStr,power->outlet[i].outNum);
					if (access == false) break;
				}
			}
			//[RK]Jul/01/04 - Fixed problem with ADD outlet after one TRY
			if (access == true) {
				if ((ps=strstr(power->userList,outStr))) {
					ps +=strlen(outStr);
					if ((*ps == 0) || (*ps == ',')) {
						access = false;
					}	
				} 	
				if (access == true) {	
					if (power->userList[0]) {
					strcat(power->userList,",");
					}
			       		strcat(power->userList,outStr);
				}
			}
			while (*paux && (*paux!=';')) paux++;
			if (*paux) paux++;
		}
	}	
}
#endif

static void portTreatPmOutlets(char *pmoutlet, PhysPowerMgmInfo * power)
{
	int ipdu,outl,nipdu,erro=0;
	char *paux;
#ifdef ONS
	int auxflg = 0;
#endif
	
	memset((char*)power->outlet,0,sizeof(PhysPMgmIpdu) * kMaxOutletServer);
	nipdu = 0;
	paux = pmoutlet;
	while (*paux && (nipdu < kMaxOutletServer)) {
#ifdef ONS
        if (*paux == 'a') {
            auxflg = 1;
        } else {
            auxflg = 0;
        }
		paux++;
#endif
		ipdu = strtol(paux,&paux,10);
#ifdef ONS
        if (auxflg) {
            ipdu +=AUX_OFFSET;
        }
        if (((!auxflg) && (ipdu > gCache.device.number2+1)) ||
            (auxflg && (ipdu > gCache.device.numberSA+1))) {
#else
		if (ipdu > gCache.device.number+1) {
#endif
			erro = 1;
			break;
		}
		if (gCache.physP.physList[ipdu].general.protocol != P_IPDU) {
			erro = 1;
			break;
		}
		if (*paux != '.') {
			erro = 1;
			break;
		}
		paux++;
		outl = strtol(paux,&paux,10);
		while (isspace(*paux)) paux++;
		power->outlet[nipdu].portNum = ipdu;
		power->outlet[nipdu++].outNum = outl;
	}
	power->numOutlets = nipdu;

	if (erro) {
		memset((char*)power->outlet,0,sizeof(PhysPMgmIpdu) * kMaxOutletServer);
		return;
	}
}

#endif // PMDNG - and old format

static void portConvIPtoStr(char * buf, int ip)
{
	char * aux;

	aux = (char*)&ip;
	sprintf(buf,"%d.%d.%d.%d",aux[0],aux[1],aux[2],aux[3]);
}

void portGetPortConfiguration(void)
{
	PhysPortsInfo *phys,*all;
	struct line_cfg *pconf;
	int port;
	char ad_temp[2*kADStringLength+1], *ad_tempp;

	while (shm_get_set_val(0, &cy_shm->st_cy_ras, 0, CY_SEM_CONFIG) != 1) {
		sleep(1);
		continue;
	}
	  
	phys = gCache.physP.physList;

	// general info
	phys->general.enable = kYes;
	phys->general.protocol = allconf.protocol;	

#ifdef PMDNG
	if (allconf.protocol == P_IPDU) {
		portGetPmConfig(0, phys);
	} else 
#endif
	{
		// serial info
		phys->serial.baudRate = allconf.speed;
		phys->serial.dataSize = allconf.DataSize;
		phys->serial.stopBits = allconf.StopBits;
		phys->serial.parity = allconf.Parity;
		phys->serial.flowControl = allconf.flow;
	}
	phys->serial.dcdState = allconf.dcd;

#ifdef PMD
	phys->general.pmsessions = allconf.pmsessions;
#endif

	// access info
	phys->access.authtype = allconf.authtype;
/*
#ifdef NISenable
    if (phys->access.authtype == kPatLocal) {
        switch (gCache.authU.nis.authType) {
            case kNatLocalNis :
                phys->access.authtype = kPatLocalNis;
                break;
            case kNatNisLocal :
                phys->access.authtype = kPatNisLocal;
                break;
            case kNatNisDownLocal :
                phys->access.authtype = kPatNisDownLocal;
                break;
            case kNatNisOnly :
                phys->access.authtype = kPatNis;
                break;
            case kNatLocalOnly :
                phys->access.authtype = kPatLocal;
                break;
        }
    }
#endif
*/
	if (allconf.users && *allconf.users)
		strcpy(phys->access.users, allconf.users);

#ifdef BIDIRECT
	phys->access.lgtimeout = allconf.lgtimeout; 
	if (allconf.termsh && *allconf.termsh) {
		strcpy(phys->access.termsh, allconf.termsh);
	}
#endif
	// data buffering info
	if (mainconf.nfs_data_buffering && *mainconf.nfs_data_buffering) {
		phys->dataBuf.destType = 1;
		strcpy(phys->dataBuf.nfsFile, mainconf.nfs_data_buffering);
	} else {
		phys->dataBuf.destType = 0;
		phys->dataBuf.mode = allconf.DB_mode;
	}
	phys->dataBuf.fileSize = allconf.data_buffering;
	phys->dataBuf.timeStamp = allconf.DB_timestamp;
	phys->dataBuf.showMenu = allconf.dont_show_db_op;
	phys->dataBuf.allTime = allconf.data_buffering_sess; //[RK]Feb/16/06
	//[AP][2007-07-30] hostname discovery
	phys->dataBuf.ADEnable = allconf.ad;
	phys->dataBuf.ADTimeOut = allconf.ad_timeout;
	if (mainconf.ad_probe_str && *mainconf.ad_probe_str) {
		strcpy(gCache.physP.ADProbeString,"\0");
		strcpy(ad_temp, mainconf.ad_probe_str);
		ad_tempp = strtok(ad_temp, "\n");
		while (ad_tempp != NULL) {
			strncat(gCache.physP.ADProbeString, ad_tempp, kADStringLength)[kADStringLength] = '\0';
			if ((ad_tempp = strtok(NULL, "\n")) != NULL) {
				strncat(gCache.physP.ADProbeString, "\\n", kADStringLength)[kADStringLength] = '\0';
			}
		}
		strcpy(ad_temp, gCache.physP.ADProbeString);
		strcpy(gCache.physP.ADProbeString,"\0");
		ad_tempp = strtok(ad_temp, "\t");
		while (ad_tempp != NULL) {
			strncat(gCache.physP.ADProbeString, ad_tempp, kADStringLength)[kADStringLength] = '\0';
			if ((ad_tempp = strtok(NULL, "\t")) != NULL) {
				strncat(gCache.physP.ADProbeString, "\\t", kADStringLength)[kADStringLength] = '\0';
			}
		}
		strcpy(ad_temp, gCache.physP.ADProbeString);
		strcpy(gCache.physP.ADProbeString,"\0");
		ad_tempp = strtok(ad_temp, "\\");
		while (ad_tempp != NULL) {
			strncat(gCache.physP.ADProbeString, ad_tempp, kADStringLength)[kADStringLength] = '\0';
			if ((ad_tempp = strtok(NULL, "\\")) != NULL) {
				strncat(gCache.physP.ADProbeString, "\\\\", kADStringLength)[kADStringLength] = '\0';
			}
		}
	}
	if (mainconf.ad_answer_str && *mainconf.ad_answer_str) {
		strcpy(gCache.physP.ADAnswerString,"\0");
		strcpy(ad_temp, mainconf.ad_answer_str);
		ad_tempp = strtok(ad_temp, "\\");
		while (ad_tempp != NULL) {
			strncat(gCache.physP.ADAnswerString, ad_tempp, kADStringLength)[kADStringLength] = '\0';
			if ((ad_tempp = strtok(NULL, "\\")) != NULL) {
				strncat(gCache.physP.ADAnswerString, "\\\\", kADStringLength)[kADStringLength] = '\0';
			}
		}
	}

	// syslog buffering info
	if (allconf.syslog_buffering) {
		phys->sysBuf.enable = kYes;
		phys->sysBuf.bufferSize = allconf.syslog_buffering;
		phys->sysBuf.facility = mainconf.DB_facility;
		phys->sysBuf.allTime = allconf.syslog_sess;
	}

	// multiple user session info
	phys->mus.multSess = allconf.multiple_sessions;
	phys->mus.sniffMode = allconf.sniff_mode;
	if (allconf.admin_users && *allconf.admin_users)
		strcpy(phys->mus.adminUsers,allconf.admin_users);
	if (allconf.escape_char) {
		if (allconf.escape_char > 32) 
			sprintf(phys->mus.hotKey,"%c",allconf.escape_char);
		else
			sprintf(phys->mus.hotKey,"^%c",'@'+allconf.escape_char);
	}
	phys->mus.notifyUsers = allconf.multiuser_notif;
	
#ifdef PMD
	// power management info
	if (allconf.pmkey) {
		phys->powerMgm.enable = kYes;
		if (allconf.pmkey > 32) 
			sprintf(phys->powerMgm.pmKey,"%c",allconf.pmkey);
		else
			sprintf(phys->powerMgm.pmKey,"^%c",'@'+allconf.pmkey);
	}
#endif
	
	// CAS info
	if (allconf.ipno) {
		portConvIPtoStr(phys->cas.portIpAlias,allconf.ipno);
	}
#ifdef IPv6enable
	// "ipno6" is a sockaddr_storage in ras-ipv6.
	// This code should be reviewed when all projects start using ras-ipv6.
	if (((struct sockaddr *)&allconf.ipno6)->sa_family) {
		dotted6((struct sockaddr *)&allconf.ipno6, phys->cas.portIp6Alias, sizeof(phys->cas.portIp6Alias));
	}
#endif

#ifdef WINEMSenable
	phys->cas.winEms = allconf.web_WinEMS;
#endif
	phys->cas.authBio = allconf.biometric;
	phys->cas.tcpKeepAlive = allconf.poll_interval;
	phys->cas.tcpIdleTmo = allconf.idletimeout;
	
	// TS info
#ifdef IPv6enable
	// "host" is a sockaddr_storage in ras-ipv6.
	// This code should be reviewed when all projects start using ras-ipv6.
	if (((struct sockaddr *)&allconf.host)->sa_family) {
		dotted6((struct sockaddr *)&allconf.host, phys->ts.host, sizeof(phys->ts.host));
	}
#else
	if (allconf.host) {
		portConvIPtoStr(phys->ts.host,allconf.host);
	}
#endif
	if (allconf.term && *allconf.term) {
		strcpy(phys->ts.terminalType, allconf.term);
	}

	// Dial In info
	if (allconf.initchat && *allconf.initchat)
		strcpy(phys->dialIn.initChat,allconf.initchat);
	if (allconf.pppopt && *allconf.pppopt) {
		strcpy(phys->dialIn.pppOpt, allconf.pppopt);
	} else {
		if (allconf.autoppp && *allconf.autoppp)
			strcpy(phys->dialIn.pppOpt, allconf.autoppp);
	}

	// Other info
	if (allconf.sttyCmd && *allconf.sttyCmd)
		strcpy(phys->other.stty, allconf.sttyCmd);
	if (allconf.issue && *allconf.issue){
		strcpy(phys->other.loginBanner,allconf.issue);
	}
	phys->other.socketPort = allconf.socket_port;
	phys->other.breakInterval = allconf.break_interval;
	//[RK]Feb/25/05	
	if (allconf.break_seq && *allconf.break_seq) {
		if (strlen(allconf.break_seq)==1 && allconf.break_seq[0] <= 32) {
			sprintf(phys->other.breakSequence,"^%c",'@'+allconf.break_seq[0]);
		} else {
			strcpy(phys->other.breakSequence,allconf.break_seq);
		}
	}
	if (allconf.SSH_exitkey) {
		if (allconf.SSH_exitkey > 32) 
			sprintf(phys->other.SSHexit,"%c",allconf.SSH_exitkey);
		else
			sprintf(phys->other.SSHexit,"^%c",'@'+allconf.SSH_exitkey);
	}
	if (allconf.sconfkey) {
		if (allconf.sconfkey > 32) 
			sprintf(phys->other.sconf,"%c",allconf.sconfkey);
		else
			sprintf(phys->other.sconf,"^%c",'@'+allconf.sconfkey);
	}

	all = phys;
	phys++;
	
#ifdef ONS
	for (port=0,pconf=&lineconf[0]; port < gCache.device.numberSA; port++,pconf++,phys++) {
#else
	for (port=0,pconf=&lineconf[0]; port < gCache.device.number; port++,pconf++,phys++) {
#endif
		// general info
		if ((pconf->this == port)) {
			phys->general.enable = 1;
		} else {
			phys->general.enable = 0;
		}
		phys->general.protocol = pconf->protocol;
		if (pconf->alias && *pconf->alias) {
			strcpy(phys->general.alias,pconf->alias);
		}

#ifdef PMD
		// Is IPDU ??
		if (pconf->protocol == P_IPDU) {
			gCache.physP.numPowerMgm++;
#ifdef PMDNG
			portGetPmConfig(port+1,phys);
#endif
#ifdef OLDPMD
			if (pconf->pmusers && *pconf->pmusers) {
				strcpy(phys->general.pmUserList,pconf->pmusers);
			}
#endif
			phys->general.pmsessions = pconf->pmsessions;
		}
#endif
				
		phys->general.action = kSpec;
	}

	phys = &gCache.physP.physList[1];

#ifdef ONS
	for (port=0,pconf=&lineconf[0]; port < gCache.device.numberSA; port++,pconf++,phys++) {
#else
	for (port=0,pconf=&lineconf[0]; port < gCache.device.number; port++,pconf++,phys++) {
#endif
		// serial info
		phys->serial.action = kAllP;
#ifdef PMDNG
		if (pconf->protocol != P_IPDU) 
#endif
		{
			phys->serial.baudRate = pconf->speed;
			phys->serial.dataSize = pconf->DataSize;
			phys->serial.stopBits = pconf->StopBits;
			phys->serial.parity = pconf->Parity;
			phys->serial.flowControl = pconf->flow;
		}
		phys->serial.dcdState = pconf->dcd;	//[RK]Bug#2588
		if (memcmp((char*)&phys->serial,(char*)&all->serial,sizeof(PhysSerialInfo)))
			phys->serial.action = kSpec;
	
		// access info
		phys->access.action = kAllP;
		phys->access.authtype = pconf->authtype;
/*
#ifdef NISenable
        if (phys->access.authtype == kPatLocal) {
            switch (gCache.authU.nis.authType) {
                case kNatLocalNis :
                    phys->access.authtype = kPatLocalNis;
                    break;
                case kNatNisLocal :
                    phys->access.authtype = kPatNisLocal;
                    break;
                case kNatNisDownLocal :
                    phys->access.authtype = kPatNisDownLocal;
                    break;
                case kNatNisOnly :
                    phys->access.authtype = kPatNis;
                    break;
                case kNatLocalOnly :
                    phys->access.authtype = kPatLocal;
                    break;
            }
        }
#endif
*/
		if (pconf->users && *pconf->users)
			strcpy(phys->access.users, pconf->users);
		else 
			memset(phys->access.users,0,sizeof(phys->access.users));

#ifdef BIDIRECT
		phys->access.lgtimeout = pconf->lgtimeout; 
		if (pconf->termsh && *pconf->termsh) {
			strcpy(phys->access.termsh, pconf->termsh);
		}
#endif
		if (memcmp((char*)&phys->access,(char*)&all->access,sizeof(PhysAccessInfo)))
			phys->access.action = kSpec;
	
		// data buffering info
		phys->dataBuf.action = kAllP;
		if (mainconf.nfs_data_buffering && *mainconf.nfs_data_buffering) {
			phys->dataBuf.destType = 1;
			strcpy(phys->dataBuf.nfsFile, mainconf.nfs_data_buffering);
		} else {
			phys->dataBuf.destType = 0;
			phys->dataBuf.mode = pconf->DB_mode;
		}
		phys->dataBuf.fileSize = pconf->data_buffering;
		phys->dataBuf.timeStamp = pconf->DB_timestamp;
		phys->dataBuf.showMenu = pconf->dont_show_db_op;
		phys->dataBuf.allTime = pconf->data_buffering_sess; //[RK]Feb/16/06
		//[AP][2007-07-30] hostname discovery
		phys->dataBuf.ADEnable = pconf->ad;
		phys->dataBuf.ADTimeOut = pconf->ad_timeout;
		if (memcmp((char*)&phys->dataBuf,(char*)&all->dataBuf,sizeof(PhysDataBufInfo)))
			phys->dataBuf.action = kSpec;

		// syslog buffering info
		phys->sysBuf.action = kAllP;
		if (pconf->syslog_buffering) {
			phys->sysBuf.enable = kYes;
			phys->sysBuf.bufferSize = pconf->syslog_buffering;
			phys->sysBuf.facility = mainconf.DB_facility;
			phys->sysBuf.allTime = pconf->syslog_sess;
		}
		if (memcmp((char*)&phys->sysBuf,(char*)&all->sysBuf,sizeof(PhysSysBufInfo)))
			phys->sysBuf.action = kSpec;

		// multiple user session info
		phys->mus.action = kAllP;
		phys->mus.multSess = pconf->multiple_sessions;
		phys->mus.sniffMode = pconf->sniff_mode;
		if (pconf->admin_users && *pconf->admin_users)
			strcpy(phys->mus.adminUsers,pconf->admin_users);
		if (pconf->escape_char) {
			if (pconf->escape_char > 32) 
				sprintf(phys->mus.hotKey,"%c",pconf->escape_char);
			else
				sprintf(phys->mus.hotKey,"^%c",'@'+pconf->escape_char);
		}
		phys->mus.notifyUsers = pconf->multiuser_notif;
		if (memcmp((char*)&phys->mus,(char*)&all->mus,sizeof(PhysMusInfo)))
			phys->mus.action = kSpec;
		
#if defined(PMD) || defined(IPMI)
		// power management info
		phys->powerMgm.action = kAllP;
#ifdef PMD
		if (pconf->pmkey) {
			phys->powerMgm.enable = kYes;
			if (pconf->pmkey > 32) 
				sprintf(phys->powerMgm.pmKey,"%c",pconf->pmkey);
			else
				sprintf(phys->powerMgm.pmKey,"^%c",'@'+pconf->pmkey);
#ifdef PMDNG
			portGetPmOutlets(port+1,pconf->pmusers,&phys->powerMgm);
#endif
#ifdef OLDPMD
			if (pconf->pmoutlet && *pconf->pmoutlet) {
				portTreatPmOutlets(pconf->pmoutlet, &phys->powerMgm);
				// find userList or all
				if (pconf->pmusers && *pconf->pmusers && !strcmp(pconf->pmusers,"all")) {
					phys->powerMgm.allUsers = 1;
				} else {
					portPmFindUserlist(&phys->powerMgm);
				}
			}
#endif
		}
#endif
#ifdef IPMI
		//[RK]Dec/22/04 - WebUI IPMI Conf
		if (pconf->IPMIkey) {
			phys->powerMgm.enableIPMI = kYes;
			phys->powerMgm.ipmiDeviceId = pconf->IPMIServer;
			if (pconf->IPMIkey > 32) 
				sprintf(phys->powerMgm.ipmiKey,"%c",pconf->IPMIkey);
			else
				sprintf(phys->powerMgm.ipmiKey,"^%c",'@'+pconf->IPMIkey);
		}
#endif
		if (memcmp((char*)&phys->powerMgm,(char*)&all->powerMgm,sizeof(PhysPowerMgmInfo)))
			phys->powerMgm.action = kSpec;
#endif
	
		// CAS info
		phys->cas.action = kAllP;
#ifdef WINEMSenable
		phys->cas.winEms = pconf->web_WinEMS;
#endif
		phys->cas.authBio = pconf->biometric;
		phys->cas.tcpKeepAlive = pconf->poll_interval;
		phys->cas.tcpIdleTmo = pconf->idletimeout;
		if (memcmp((char*)&phys->cas,(char*)&all->cas,sizeof(PhysCASInfo)))
			phys->cas.action = kSpec;
		if (pconf->ipno) {
			portConvIPtoStr(phys->cas.portIpAlias,pconf->ipno);
			//[RK]Sep/19/05 - fixed problem when port has IPNO configured.
			if (pconf->ipno != (allconf.ipno + port)) {
				phys->cas.action = kSpec;
			}
		}
#ifdef IPv6enable
		if (((struct sockaddr *)&pconf->ipno6)->sa_family) {
			dotted6((struct sockaddr *)&pconf->ipno6, phys->cas.portIp6Alias, sizeof(phys->cas.portIp6Alias));
			if (ntohl(((struct sockaddr_in6 *)&pconf->ipno6)->sin6_addr.s6_addr32[3]) != 
				ntohl(htonl(((struct sockaddr_in6 *)&allconf.ipno6)->sin6_addr.s6_addr32[3]) + port)) {
				phys->cas.action = kSpec;
			}
		}
#endif

		// TS info
		phys->ts.action = kAllP;
#ifdef IPv6enable
		// "host" is a sockaddr_storage in ras-ipv6.
		// This code should be reviewed when all projects start 
		// using ras-ipv6.
		if (((struct sockaddr *)&pconf->host)->sa_family) {
			dotted6((struct sockaddr *)&pconf->host, phys->ts.host, sizeof(phys->ts.host));
		}
#else
		if (pconf->host) 
			portConvIPtoStr(phys->ts.host,pconf->host);
#endif
		if (pconf->term && *pconf->term) {
			strcpy(phys->ts.terminalType, pconf->term);
		}
		if (memcmp((char*)&phys->ts,(char*)&all->ts,sizeof(PhysTSInfo)))
			phys->ts.action = kSpec;
	
		// Dial In info
		phys->dialIn.action = kAllP;
		if (pconf->initchat && *pconf->initchat)
			strcpy(phys->dialIn.initChat,pconf->initchat);
		if (pconf->pppopt && *pconf->pppopt)
			strcpy(phys->dialIn.pppOpt, pconf->pppopt);
		else {
			if (pconf->autoppp && *pconf->autoppp)
				strcpy(phys->dialIn.pppOpt, pconf->autoppp);
		}
		if (memcmp((char*)&phys->dialIn,(char*)&all->dialIn,sizeof(PhysDialInInfo)))
			phys->dialIn.action = kSpec;

		// Other info
		phys->other.action = kAllP;
		if (pconf->sttyCmd && *pconf->sttyCmd)
			strcpy(phys->other.stty, pconf->sttyCmd);
		if (pconf->issue && *pconf->issue){
			strcpy(phys->other.loginBanner,pconf->issue);
		}
		if (pconf->socket_port == (allconf.socket_port + port)) {
			phys->other.socketPort = allconf.socket_port;
		} else {
			phys->other.socketPort = pconf->socket_port;
		}
		phys->other.breakInterval = pconf->break_interval;
		//[RK]Feb/25/05 
		if (pconf->break_seq && *pconf->break_seq) {
			if (strlen(pconf->break_seq)==1 && pconf->break_seq[0] <= 32) {
				sprintf(phys->other.breakSequence,"^%c",'@'+pconf->break_seq[0]);
			} else {
				strcpy(phys->other.breakSequence,pconf->break_seq);
			}
		}
		if (pconf->SSH_exitkey) {
			if (pconf->SSH_exitkey > 32) 
				sprintf(phys->other.SSHexit,"%c",pconf->SSH_exitkey);
			else
				sprintf(phys->other.SSHexit,"^%c",'@'+pconf->SSH_exitkey);
		}
		if (pconf->sconfkey) {
			if (pconf->sconfkey > 32) 
				sprintf(phys->other.sconf,"%c",pconf->sconfkey);
			else
				sprintf(phys->other.sconf,"^%c",'@'+pconf->sconfkey);
		}
		if (memcmp((char*)&phys->other,(char*)&all->other,sizeof(PhysOtherInfo)))
			phys->other.action = kSpec;
		phys->other.socketPort = pconf->socket_port;
	}
}

int portPhysChangeStatus(int status)
{
	PhysPortsInfo *phys;
	int change=0;
	int i;
	for (i=0; i < gCache.physP.numPortsel; i++) {
		phys = &gCache.physP.physList[(int)gPorts[i]];
                //[GY]2006/Mar,13  BUG#6677
                ConvTextArea(phys->other.loginBanner,phys->other.loginBanner,1);
		if (phys->general.enable != status) {
			phys->general.enable = status;
			phys->general.action = kSpec;
#ifdef PMD
			if (phys->general.protocol == P_IPDU) { //IPDU
				if (status == kNo) {
					gCache.physP.numPowerMgm--;
					// remove the IPDU from the pmoutles ???
				} else {
					gCache.physP.numPowerMgm++;
				}
			}
#endif
			phys->action = kEdit;
			change = 1;
		}	
	}
	return(change);
}

//[RK]Sep/24/04 - Implement Bug#2544
/*[LMT] CyBTS Bug 3360 - This function MUST return 
just 0 or 1 because it is being used to define the 
led status on PhysPortCommit(...) function located
in UserFunc.c*/
int portPhysChangeAll(void)
{
	PhysPortsInfo *cnf,*all;
	PhysGeneralInfo *pgen,*pgall;
	PhysSerialInfo *pse,*psall;
	PhysAccessInfo *pacc,*paall;
	PhysDataBufInfo *pdb,*pdball;
	PhysSysBufInfo *psys,*psysall;
	PhysMusInfo *pmus,*pmall;
	PhysCASInfo *pcas,*pcasall;
	PhysTSInfo *pts,*ptsall;
	PhysDialInInfo *pdial,*pdialall;
	PhysOtherInfo *pot,*potall;
	struct in_addr b;
	int i,change,size;

	cnf = &gCache.physP.infoP;
	
	// modify loginBanner --> "\r" ==> "\r\n/"
	ConvTextArea(cnf->other.loginBanner, cnf->other.loginBanner,1);
	if (cnf->ts.host[0])
		ConvTextArea(cnf->ts.host,cnf->ts.host,1);
	if (cnf->dialIn.initChat[0]) 
		ConvInitChat(cnf->dialIn.initChat, cnf->dialIn.initChat);
	
	all = &gCache.physP.physList[0];

        /*[LMT] CyBTS Bug 3360 - Convert all to the same format of cnf */
        ConvTextArea(all->other.loginBanner,all->other.loginBanner,1);

	if (!memcmp((char*)all,(char*)cnf, sizeof(PhysPortsInfo))) {
		return 0; // nothing was changed
	}

	// General Information
	pgen = &gCache.physP.infoP.general;
	pgall = &gCache.physP.physList[0].general;

	if (memcmp((char*)pgall,(char*)pgen, sizeof(PhysGeneralInfo))) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].general.action == kAllP) {
				gCache.physP.physList[i].general.protocol = pgen->protocol;
#ifdef PMD
				gCache.physP.physList[i].general.pmsessions = pgen->pmsessions;
#endif
#ifdef PMDNG
				gCache.physP.physList[i].general.pmVendor = pgen->pmVendor;
#endif
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (pgall->protocol != pgen->protocol) {
				gCache.physP.physList[i].general.protocol = pgen->protocol;
				change = 1;
			}
#ifdef PMD
			if (pgall->pmsessions != pgen->pmsessions) {
				gCache.physP.physList[i].general.pmsessions = pgen->pmsessions;
				change = 1;
			}
#endif
#ifdef PMDNG
			if (pgall->pmVendor != pgen->pmVendor) {
				gCache.physP.physList[i].general.pmVendor = pgen->pmVendor;
				change = 1;
			}
#endif
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].general.protocol,
					   (char*)&pgen->protocol, sizeof(PhysGeneralInfo)-4)) {
					gCache.physP.physList[i].general.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) pgall, (char *)pgen, sizeof(PhysGeneralInfo));
		gCache.physP.physList[0].action = kEdit;
	}

	// Physical Ports 
	pse = &gCache.physP.infoP.serial;
	psall = &gCache.physP.physList[0].serial;
	size = sizeof(PhysSerialInfo) - sizeof(PhysAction);
	
	if (memcmp((char*)&pse->baudRate,(char*)&psall->baudRate, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].serial.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].serial, 
					(char *)pse,sizeof(PhysSerialInfo));
				gCache.physP.physList[i].serial.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (psall->baudRate != pse->baudRate) {
				gCache.physP.physList[i].serial.baudRate = pse->baudRate;
				change = 1;
			}
			if (psall->dataSize != pse->dataSize) {
				gCache.physP.physList[i].serial.dataSize = pse->dataSize;
				change = 1;
			}
			if (psall->stopBits != pse->stopBits) {
				gCache.physP.physList[i].serial.stopBits = pse->stopBits;
				change = 1;
			}
			if (psall->parity != pse->parity) {
				gCache.physP.physList[i].serial.parity = pse->parity;
				change = 1;
			}
			if (psall->flowControl != pse->flowControl) {
				gCache.physP.physList[i].serial.flowControl = pse->flowControl;
				change = 1;
			}
			if (psall->dcdState != pse->dcdState) {
				gCache.physP.physList[i].serial.dcdState = pse->dcdState;
				change = 1;
			}
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].serial.baudRate,
					   (char*)&pse->baudRate, size)) {
					gCache.physP.physList[i].serial.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) psall, (char *)pse, sizeof(PhysSerialInfo));
		gCache.physP.physList[0].action = kEdit;
	}

	// Access Information
	size = sizeof(PhysAccessInfo) - sizeof(PhysAction);
	pacc = &gCache.physP.infoP.access;
	paall = &gCache.physP.physList[0].access;

	if (memcmp((char*)&pacc->users,(char*)&paall->users, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].access.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].access, 
					(char *)pacc,sizeof(PhysAccessInfo));
				gCache.physP.physList[i].access.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (strcmp(pacc->users,paall->users)) {
				strcpy(gCache.physP.physList[i].access.users,pacc->users);
				change = 1;
			}
			if (pacc->authtype != paall->authtype) {
				gCache.physP.physList[i].access.authtype = pacc->authtype;
				change = 1;
			}
#ifdef BIDIRECT
			if (pacc->lgtimeout != paall->lgtimeout) {
				gCache.physP.physList[i].access.lgtimeout = pacc->lgtimeout;
				change = 1;
			}
			if (strcmp(pacc->termsh, paall->termsh)) {
				strcpy(gCache.physP.physList[i].access.termsh, pacc->termsh);
				change = 1;
			}
#endif
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].access.users,
					   (char*)&pacc->users, size)) {
					gCache.physP.physList[i].access.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
/*
#ifdef NISenable
        if (pacc->authtype != paall->authtype) {
            switch (pacc->authtype) {
                case kPatLocalNis :
                    gCache.authU.nis.authType = kNatLocalNis;
                    break;
                case kPatNisLocal :
                    gCache.authU.nis.authType = kNatNisLocal;
                    break;
                case kPatNisDownLocal :
                    gCache.authU.nis.authType = kNatNisDownLocal;
                    break;
                case kPatNis :
                    gCache.authU.nis.authType = kNatNisOnly;
                    break;
                case kPatLocal :
                    gCache.authU.nis.authType = kNatLocalOnly;
                    break;
            }
        }
#endif
*/
		memcpy((char*) paall, (char *)pacc, sizeof(PhysAccessInfo));
		gCache.physP.physList[0].action = kEdit;
	}

	// Data Buffering Info
	size = sizeof(PhysDataBufInfo) - sizeof(PhysAction);
	pdb = &gCache.physP.infoP.dataBuf;
	pdball = &gCache.physP.physList[0].dataBuf;

	if (memcmp((char*)&pdb->destType,(char*)&pdball->destType, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].dataBuf.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].dataBuf, 
					(char *)pdb,sizeof(PhysDataBufInfo));
				gCache.physP.physList[i].dataBuf.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			
			if (pdb->destType != pdball->destType) {
				gCache.physP.physList[i].dataBuf.destType = pdb->destType;
				change = 1;
			}
			if (pdb->mode != pdball->mode) {
				gCache.physP.physList[i].dataBuf.mode = pdb->mode;
				change = 1;
			}
			if (pdb->fileSize != pdball->fileSize) {
				gCache.physP.physList[i].dataBuf.fileSize = pdb->fileSize;
				change = 1;
			}
			if (strcmp(pdb->nfsFile,pdball->nfsFile)) {
				strcpy(gCache.physP.physList[i].dataBuf.nfsFile,pdball->nfsFile);
				change = 1;
			}
			if (pdb->timeStamp != pdball->timeStamp) {
				gCache.physP.physList[i].dataBuf.timeStamp = pdb->timeStamp;
				change = 1;
			}
			if (pdb->showMenu != pdball->showMenu) {
				gCache.physP.physList[i].dataBuf.showMenu = pdb->showMenu;
				change = 1;
			}
			//[RK]Feb/16/06
			if (pdb->allTime != pdball->allTime) {
				gCache.physP.physList[i].dataBuf.allTime = pdb->allTime;
				change = 1;
			}
			//[AD][2007-07-31] hostname discovery
			if (pdb->ADEnable != pdball->ADEnable) {
				gCache.physP.physList[i].dataBuf.ADEnable = pdb->ADEnable;
				change = 1;
			}
			if (pdb->ADTimeOut != pdball->ADTimeOut) {
				gCache.physP.physList[i].dataBuf.ADTimeOut = pdb->ADTimeOut;
				change = 1;
			}
			
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].dataBuf.destType,
					   (char*)&pdb->destType, size)) {
					gCache.physP.physList[i].dataBuf.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) pdball, (char *)pdb, sizeof(PhysDataBufInfo));
		gCache.physP.physList[0].action = kEdit;
	}
		
	// Syslog Buffering
	size = sizeof(PhysSysBufInfo) - sizeof(PhysAction);
	psys = &gCache.physP.infoP.sysBuf;
	psysall = &gCache.physP.physList[0].sysBuf;

	if (memcmp((char*)&psys->enable,(char*)&psysall->enable, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].sysBuf.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].sysBuf, 
					(char *)psys,sizeof(PhysSysBufInfo));
				gCache.physP.physList[i].sysBuf.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			
			if (psys->enable != psysall->enable) {
				gCache.physP.physList[i].sysBuf.enable = psys->enable;
				change = 1;
			}
			if (psys->facility != psysall->facility) {
				gCache.physP.physList[i].sysBuf.facility = psys->facility;
				change = 1;
			}
			if (psys->bufferSize != psysall->bufferSize) {
				gCache.physP.physList[i].sysBuf.bufferSize = psys->bufferSize;
				change = 1;
			}
			if (psys->allTime != psysall->allTime) {
				gCache.physP.physList[i].sysBuf.allTime = psys->allTime;
				change = 1;
			}
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].sysBuf.enable,
					   (char*)&psys->enable, size)) {
					gCache.physP.physList[i].sysBuf.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
		if (psysall->facility != psys->facility) {
			gCache.sysBufServer.dbFacilityNum = psys->facility;
		}
		memcpy((char*) psysall, (char *)psys, sizeof(PhysSysBufInfo));
		gCache.physP.physList[0].action = kEdit;
	}

	// Multiple sessions
	size = sizeof(PhysMusInfo) - sizeof(PhysAction);
	pmus = &gCache.physP.infoP.mus;
	pmall = &gCache.physP.physList[0].mus;

	if (memcmp((char*)&pmus->multSess,(char*)&pmall->multSess, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].mus.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].mus, 
					(char *)pmus,sizeof(PhysMusInfo));
				gCache.physP.physList[i].mus.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (pmus->multSess != pmall->multSess) {
				gCache.physP.physList[i].mus.multSess = pmus->multSess;
				change = 1;
			}
			if (pmus->sniffMode != pmall->sniffMode) {
				gCache.physP.physList[i].mus.sniffMode = pmus->sniffMode;
				change = 1;
			}
			if (strcmp(pmus->adminUsers, pmall->adminUsers)) {
				strcpy(gCache.physP.physList[i].mus.adminUsers,pmus->adminUsers);
				change = 1;
			}
			if (strcmp(pmus->hotKey, pmall->hotKey)) {
				strcpy(gCache.physP.physList[i].mus.hotKey,pmus->hotKey);
				change = 1;
			}
			if (pmus->notifyUsers != pmall->notifyUsers) {
				gCache.physP.physList[i].mus.notifyUsers = pmus->notifyUsers;
				change = 1;
			}
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].mus.multSess,
					   (char*)&pmus->multSess, size)) {
					gCache.physP.physList[i].mus.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) pmall, (char *)pmus, sizeof(PhysMusInfo));
		gCache.physP.physList[0].action = kEdit;
	}

	// CAS info
	size = sizeof(PhysCASInfo) - sizeof(PhysAction);
	pcas = &gCache.physP.infoP.cas;
	pcasall = &gCache.physP.physList[0].cas;

	if (memcmp((char*)&pcas->portIpAlias,(char*)&pcasall->portIpAlias, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].cas.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].cas, 
					(char *)pcas,sizeof(PhysCASInfo));
				gCache.physP.physList[i].cas.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (strcmp(pcas->portIpAlias,pcasall->portIpAlias)) {
				inet_aton(pcas->portIpAlias, &b);
				*((unsigned char *)(&b)+3) += i - 1;
				strcpy(gCache.physP.physList[i].cas.portIpAlias,inet_ntoa(b));
				change = 1;
			}
#ifdef IPv6enable
			if (strcmp(pcas->portIp6Alias,pcasall->portIp6Alias)) {
				struct sockaddr ip6addr;
				asc_to_binary6(pcas->portIp6Alias, &ip6addr);
				((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3] = htonl(
					ntohl(((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3]) + (i-1)
				);
				dotted6(&ip6addr, gCache.physP.physList[i].cas.portIp6Alias, sizeof(gCache.physP.physList[i].cas.portIpAlias));
				change = 1;
			}
#endif
#ifdef WINEMSenable
			if (pcas->winEms != pcasall->winEms) {
				gCache.physP.physList[i].cas.winEms = pcas->winEms;
				change = 1;
			}
#endif
			if (pcas->authBio != pcasall->authBio) {
				gCache.physP.physList[i].cas.authBio = pcas->authBio;
				change = 1;
			}
			if (pcas->tcpKeepAlive != pcasall->tcpKeepAlive) {
				gCache.physP.physList[i].cas.tcpKeepAlive = pcas->tcpKeepAlive;
				change = 1;
			}
			if (pcas->tcpIdleTmo != pcasall->tcpIdleTmo) {
				gCache.physP.physList[i].cas.tcpIdleTmo = pcas->tcpIdleTmo;
				change = 1;
			}
			if (change) {
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) pcasall, (char *)pcas, sizeof(PhysCASInfo));
		gCache.physP.physList[0].action = kEdit;
	}
			
	// TS info
	size = sizeof(PhysTSInfo) - sizeof(PhysAction);
	pts = &gCache.physP.infoP.ts;
	ptsall = &gCache.physP.physList[0].ts;

	if (memcmp((char*)&pts->host,(char*)&ptsall->host, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].ts.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].ts, 
					(char *)pts,sizeof(PhysTSInfo));
				gCache.physP.physList[i].ts.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (strcmp(pts->host,ptsall->host)) {
				strcpy(gCache.physP.physList[i].ts.host,pts->host);
				change = 1;
			}
			if (strcmp(pts->terminalType,ptsall->terminalType)) {
				strcpy(gCache.physP.physList[i].ts.terminalType,pts->terminalType);
				change = 1;
			}
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].ts.host,
					   (char*)&pts->host, size)) {
					gCache.physP.physList[i].ts.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) ptsall, (char *)pts, sizeof(PhysTSInfo));
		gCache.physP.physList[0].action = kEdit;
	}

	// Dial In info
	size = sizeof(PhysDialInInfo) - sizeof(PhysAction);
	pdial = &gCache.physP.infoP.dialIn;
	pdialall = &gCache.physP.physList[0].dialIn;
	if (memcmp((char*)&pdial->initChat,(char*)&pdialall->initChat, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].dialIn.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].dialIn, 
					(char *)pdial,sizeof(PhysDialInInfo));
				gCache.physP.physList[i].dialIn.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (strcmp(pdial->initChat,pdialall->initChat)) {
				strcpy(gCache.physP.physList[i].dialIn.initChat,pdial->initChat);
				change = 1;
			}
			if (strcmp(pdial->pppOpt,pdialall->pppOpt)) {
				strcpy(gCache.physP.physList[i].dialIn.pppOpt,pdial->pppOpt);
				change = 1;
			}
			if (change) {
				if (!memcmp((char*)&gCache.physP.physList[i].dialIn.initChat,
					   (char*)&pdial->initChat, size)) {
					gCache.physP.physList[i].dialIn.action = kAllP;
				}
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) pdialall, (char *)pdial, sizeof(PhysDialInInfo));
		gCache.physP.physList[0].action = kEdit;
	}

	// Other Info
	size = sizeof(PhysOtherInfo) - sizeof(PhysAction);
	pot = &gCache.physP.infoP.other;
	potall = &gCache.physP.physList[0].other;

	if (memcmp((char*)&pot->socketPort,(char*)&potall->socketPort, size)) {
#ifdef ONS
		for (i=1; i <= gCache.device.number2; i++) {
#else
		for (i=1; i <= gCache.device.number; i++) {
#endif
			if (gCache.physP.physList[i].other.action == kAllP) {
				memcpy((char*)&gCache.physP.physList[i].other, 
					(char *)pot,sizeof(PhysOtherInfo));
				gCache.physP.physList[i].other.action = kAllP;
				gCache.physP.physList[i].action = kEdit;
				continue;
			}	
			change = 0;
			if (pot->socketPort != potall->socketPort) {
				gCache.physP.physList[i].other.socketPort = pot->socketPort+i-1;
				change = 1;
			}
			if (strcmp(pot->stty,potall->stty)) {
				strcpy(gCache.physP.physList[i].other.stty,pot->stty);
				change = 1;
			}
			if (strcmp(pot->loginBanner,potall->loginBanner)) {
				strcpy(gCache.physP.physList[i].other.loginBanner,pot->loginBanner);
				change = 1;
			} else {
				ConvTextArea(gCache.physP.physList[i].other.loginBanner,
					gCache.physP.physList[i].other.loginBanner,1);
			}
			if (pot->breakInterval != potall->breakInterval) {
				gCache.physP.physList[i].other.breakInterval = pot->breakInterval;
				change = 1;
			}
			//[RK]Feb/25/05
			if (strcmp(pot->breakSequence,potall->breakSequence)) {
				strcpy(gCache.physP.physList[i].other.breakSequence,pot->breakSequence);
				change = 1;
			}
			if (strcmp(pot->SSHexit,potall->SSHexit)) {
				strcpy(gCache.physP.physList[i].other.SSHexit,pot->SSHexit);
				change = 1;
			}
			if (strcmp(pot->sconf,potall->sconf)) {
				strcpy(gCache.physP.physList[i].other.sconf,pot->sconf);
				change = 1;
			}
			if (change) {
				gCache.physP.physList[i].action = kEdit;
			}
		}
		memcpy((char*) potall, (char *)pot, sizeof(PhysOtherInfo));
		gCache.physP.physList[0].action = kEdit;
	}
	return(1);
}

int portPhysChangeGeneral(void)
{
	PhysGeneralInfo *pcnf,*psel;
	int change=0;
	int i;

	pcnf = &gCache.physP.infoP.general;
	psel = &gCache.physP.physList[(int)gPorts[0]].general;

	if (memcmp((char*)psel,(char*)pcnf, sizeof(PhysGeneralInfo))) {
		if (gCache.physP.numPortsel==1) {
			strcpy(psel->alias, pcnf->alias);
		}
		psel->protocol = pcnf->protocol;
#ifdef PMD
		psel->pmsessions = pcnf->pmsessions;
#endif
#ifdef PMDNG
		psel->pmVendor = pcnf->pmVendor;
#endif
		psel->action = kSpec;
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		change = 1;
	}
	
	for (i=1; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].general;
		//[RK]Sep/08/05 - fixed problem with changed
		if ((psel->protocol != pcnf->protocol) || 
#ifdef PMD
		    (psel->pmsessions != pcnf->pmsessions) ||
#endif
#ifdef PMDNG
		    (psel->pmVendor != pcnf->pmVendor) ||
#endif
		     (psel->enable != pcnf->enable)) {
			psel->protocol = pcnf->protocol;
#ifdef PMD
			psel->pmsessions = pcnf->pmsessions;
#endif
#ifdef PMDNG
			psel->pmVendor = pcnf->pmVendor;
#endif
			psel->enable = pcnf->enable;
			psel->action = kSpec;
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}

int portPhysChangeSerial(void)
{
	PhysSerialInfo *psel,*pcnf,*pall;
	int size, i,change=0;

	size = sizeof(PhysSerialInfo) - sizeof(PhysAction);
	
	pcnf = &gCache.physP.infoP.serial;
	pall = &gCache.physP.physList[0].serial;
	psel = &gCache.physP.physList[(int)gPorts[0]].serial;

	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->baudRate,(char*)&pall->baudRate, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysSerialInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].serial;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysSerialInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].serial;
		if (memcmp((char*)&pcnf->baudRate,(char*)&psel->baudRate, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysSerialInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}

int portPhysChangeAccess(void)
{
	PhysAccessInfo *psel,*pcnf,*pall;
	int i,size,change=0;

	size = sizeof(PhysAccessInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.access;
	pall = &gCache.physP.physList[0].access;
	psel = &gCache.physP.physList[(int)gPorts[0]].access;
/*
#ifdef NISenable
    switch (pcnf->authtype) {
        case kPatLocalNis :
            gCache.authU.nis.authType = kNatLocalNis;
            break;
        case kPatNisLocal :
            gCache.authU.nis.authType = kNatNisLocal;
            break;
        case kPatNisDownLocal :
            gCache.authU.nis.authType = kNatNisDownLocal;
            break;
        case kPatNis :
            gCache.authU.nis.authType = kNatNisOnly;
            break;
        default : //[RK]jul/01/04 - Fixed bug 2007 - changed the nsswitch.conf file
            gCache.authU.nis.authType = kNatLocalOnly;
            break;
    }
#endif
*/
	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->users,(char*)&pall->users, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysAccessInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].access;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysAccessInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].access;
		if (memcmp((char*)&pcnf->users,(char*)&psel->users, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysAccessInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}

	return(change);
}

int portPhysChangeDataBuf(void)
{
	PhysDataBufInfo *psel,*pcnf,*pall;
	int i,size,change=0;

	size = sizeof(PhysDataBufInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.dataBuf;
	pall = &gCache.physP.physList[0].dataBuf;
	psel = &gCache.physP.physList[(int)gPorts[0]].dataBuf;

	if (pcnf->nfsFile[0]) {
		strcpy(pall->nfsFile,pcnf->nfsFile);
		strcpy(psel->nfsFile,pcnf->nfsFile);
	} else {
		pall->nfsFile[0] = 0x00;
		psel->nfsFile[0] = 0x00;
	}	
		
	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->destType,(char*)&pall->destType, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysDataBufInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].dataBuf;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysDataBufInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].dataBuf;
		if (memcmp((char*)&pcnf->destType,(char*)&psel->destType, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysDataBufInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}

int portPhysChangeSysBuf(void)
{
	PhysSysBufInfo *psel,*pcnf,*pall;
	int i,size,change=0;

	size = sizeof(PhysSysBufInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.sysBuf;
	pall = &gCache.physP.physList[0].sysBuf;
	psel = &gCache.physP.physList[(int)gPorts[0]].sysBuf;

	if (pall->facility != pcnf->facility) {
		gCache.sysBufServer.dbFacilityNum = pcnf->facility;
		pall->facility = pcnf->facility;
	}

	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->enable,(char*)&pall->enable, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysSysBufInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].sysBuf;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysSysBufInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].sysBuf;
		if (memcmp((char*)&pcnf->enable,(char*)&psel->enable, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysSysBufInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}

int portPhysChangeMus(void)
{
	PhysMusInfo *psel,*pcnf,*pall;
	int i,size,change=0;

	size = sizeof(PhysMusInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.mus;
	pall = &gCache.physP.physList[0].mus;
	psel = &gCache.physP.physList[(int)gPorts[0]].mus;

	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->multSess,(char*)&pall->multSess, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysMusInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].mus;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysMusInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].mus;
		if (memcmp((char*)&pcnf->multSess,(char*)&psel->multSess, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysMusInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}

#if defined(PMD) || defined(IPMI)
int portPhysChangePowerMgm(void)
{      //mp: now can return kMemAllocationFailure
	PhysPowerMgmInfo *psel,*pcnf,*pall;
	int i,size,change=0;

	size = sizeof(PhysPowerMgmInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.powerMgm;
	pall = &gCache.physP.physList[0].powerMgm;
	psel = &gCache.physP.physList[(int)gPorts[0]].powerMgm;

	// If protocol is not socket, power management should be disabled.
	if (gCache.physP.infoP.general.protocol != P_SOCKET_SERVER &&
		gCache.physP.infoP.general.protocol != P_SOCKET_SERVER_RAW &&
		gCache.physP.infoP.general.protocol != P_SOCKET_SSH &&
		gCache.physP.infoP.general.protocol != P_SOCKET_SERVER_SSH) {
#ifdef PMD
		pcnf->enable = 0;
#endif
#ifdef IPMI
		pcnf->enableIPMI = 0;
#endif
	}

	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->enable,(char*)&pall->enable, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysPowerMgmInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].powerMgm;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysPowerMgmInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	// check old conf
	if (memcmp((char*)&pcnf->enable,(char*)&psel->enable, size)) {
#ifdef OLDPMD
		//[RK]Jul/01/04 - Fixed problem with delete or add Outlet after Try
		if (portPmChangeUserList(psel->userList,pcnf->userList,pcnf)) return kMemAllocationFailure;
#endif
#ifdef PMD
		if (pcnf->enable && !pcnf->pmKey[0]) {
			strcpy(pcnf->pmKey,"^p");
		}
#endif
#ifdef IPMI
		//[RK]Dec/22/04 - WebUi IPMI conf
		if (pcnf->enableIPMI && !pcnf->ipmiKey[0]) {
			strcpy(pcnf->ipmiKey,"^I");
		}
#endif
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysPowerMgmInfo));
		psel->action = kSpec; 
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		change = 1;
#ifdef PMDNG
		portSetPmOutlets((int)gPorts[0],psel);
#endif
	}

	for (i=1; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].powerMgm;
		if (memcmp((char*)&pcnf->enable,(char*)&psel->enable, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysPowerMgmInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}
#endif

int portPhysChangeCas(void)
{
	PhysCASInfo *psel,*pcnf,*pall;
	int i,size,change=0;
	//[RK]Sep/14/04 - fix problem with ipno increment
	struct in_addr b;

	size = sizeof(PhysCASInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.cas;
	pall = &gCache.physP.physList[0].cas;
	psel = &gCache.physP.physList[(int)gPorts[0]].cas;

	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->portIpAlias,(char*)&pall->portIpAlias, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysCASInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].cas;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysCASInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
				//[RK]Sep/14/04
				if (psel->portIpAlias[0]) { //[RK]Oct/07/04 
					inet_aton(psel->portIpAlias, &b);
					*((unsigned char *)(&b)+3) += i;
					strcpy(psel->portIpAlias,inet_ntoa(b));
				}
#ifdef IPv6enable
				if (psel->portIp6Alias[0]) {
					struct sockaddr ip6addr;
					asc_to_binary6(psel->portIp6Alias, &ip6addr);
					((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3] = htonl(
						ntohl(((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3]) + (i-1)
					);
					dotted6(&ip6addr, psel->portIp6Alias, sizeof(psel->portIp6Alias));
				}
#endif
			}
		}
		return (1);
	}


	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].cas;
		if (memcmp((char*)&pcnf->portIpAlias,(char*)&psel->portIpAlias, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysCASInfo));
			//[RK]Sep/14/04
			if (psel->portIpAlias[0]) { //[RK]Oct/07/04 
				inet_aton(psel->portIpAlias, &b);
				*((unsigned char *)(&b)+3) += i;
				strcpy(psel->portIpAlias,inet_ntoa(b));
			}
#ifdef IPv6enable
			if (psel->portIp6Alias[0]) {
				struct sockaddr ip6addr;
				asc_to_binary6(psel->portIp6Alias, &ip6addr);
				((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3] = htonl(
					ntohl(((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3]) + i
				);
				dotted6(&ip6addr, psel->portIp6Alias, sizeof(psel->portIp6Alias));
			}
#endif
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}

int portPhysChangeTs(void)
{
	PhysTSInfo *psel,*pcnf,*pall;
	int i,size,change=0;

	size = sizeof(PhysTSInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.ts;
	pall = &gCache.physP.physList[0].ts;
	psel = &gCache.physP.physList[(int)gPorts[0]].ts;

	ConvTextArea(pcnf->host, pcnf->host,1);

	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)pcnf->host,(char*)pall->host, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysTSInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].ts;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysTSInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].ts;
		if (memcmp((char*)pcnf->host,(char*)psel->host, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysTSInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}

int portPhysChangeDialIn(void)
{
	PhysDialInInfo *psel,*pcnf,*pall;
	int i,size,change=0;
	PhysCASInfo *pselip,*pcnfip;
	//[RK]Sep/14/04 - fix problem with ipno increment
#ifdef NOK
	struct in_addr b;
#endif

	size = sizeof(PhysDialInInfo) - sizeof(PhysAction);
	pcnf = &gCache.physP.infoP.dialIn;
	pall = &gCache.physP.physList[0].dialIn;
	psel = &gCache.physP.physList[(int)gPorts[0]].dialIn;
	pselip = &gCache.physP.physList[(int)gPorts[0]].cas;
	pcnfip = &gCache.physP.infoP.cas;

	ConvInitChat(pcnf->initChat, pcnf->initChat);

	// check ALL
	if ((psel->action != kAllP) &&
	    !memcmp((char*)pcnf->initChat,(char*)pall->initChat, size)) {
		pcnf->action = kAllP; // equal ALL
		memcpy((char*)psel, (char *)pcnf,sizeof(PhysDialInInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].dialIn;
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysDialInInfo));
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].dialIn;
		if (memcmp((char*)pcnf->initChat,(char*)psel->initChat, size)) {
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysDialInInfo));
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
#ifdef NOK
		pselip = &gCache.physP.physList[(int)gPorts[i]].cas;
		if (memcmp((char*)&pcnfip->portIpAlias,(char*)&pselip->portIpAlias, size)) {
			memcpy((char*)pselip, (char *)pcnfip,sizeof(PhysCASInfo));
			//[RK]Sep/14/04
			if (pselip->portIpAlias[0]) { //[RK]Oct/07/04 
				inet_aton(pselip->portIpAlias, &b);
				*((unsigned char *)(&b)+3) += i;
				strcpy(pselip->portIpAlias,inet_ntoa(b));
			}
#ifdef IPv6enable
			if (pselip->portIp6Alias[0]) {
				struct sockaddr ip6addr;
				asc_to_binary6(pselip->portIp6Alias, &ip6addr);
				((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3] = htonl(
					ntohl(((struct sockaddr_in6*)&ip6addr)->sin6_addr.s6_addr32[3]) + i
				);
				dotted6(&ip6addr, pselip->portIp6Alias, sizeof(pselip->portIp6Alias));
				change = 1;
			}
#endif
			psel->action = kSpec; 
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
#endif
	}
	return(change);
}

int portPhysChangeOther(void)
{
	PhysOtherInfo *psel,*pcnf,*pall;
	int i,size,change=0,spc=0,old;
	size = sizeof(PhysOtherInfo) - sizeof(PhysAction) - sizeof(int);
	pcnf = &gCache.physP.infoP.other;
	pall = &gCache.physP.physList[0].other;
	psel = &gCache.physP.physList[(int)gPorts[0]].other;

	// modify loginBanner --> '\r' ==> "\r\n"
	ConvTextArea(pcnf->loginBanner,pcnf->loginBanner,1);
	/*[LMT] CyBTS Bug 3360 - Convert psel to the same format of pcnf */
	ConvTextArea(psel->loginBanner,psel->loginBanner,1);
	
	// [RK]Feb/23/05 - check changes in the socket port value
	if (psel->socketPort != pcnf->socketPort) {
		spc = 1;
	}
	// check ALL
	//[RK]Sep/08/05 - fixed problem with changed
	if ((psel->action != kAllP) &&
	    !memcmp((char*)&pcnf->stty,(char*)&pall->stty, size)) {
		pcnf->action = kAllP; // equal ALL

		memcpy((char*)psel, (char *)pcnf,sizeof(PhysOtherInfo));
		gCache.physP.physList[(int)gPorts[0]].action = kEdit;
		for (i=1; i < gCache.physP.numPortsel; i++) {
			psel = &gCache.physP.physList[(int)gPorts[i]].other;
			old = psel->socketPort; //[RK]Feb/23/05
			if (psel->action != kAllP) {
				memcpy((char*)psel, (char *)pcnf,sizeof(PhysOtherInfo));
				if (spc) {   //[RK]Feb/23/05 
					psel->socketPort += (int)gPorts[i]; //[RK]Sep/14/04 - increase the socket_port
				} else {
					psel->socketPort = old; 
				}
				gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			}
		}
		return (1);
	}

	for (i=0; i < gCache.physP.numPortsel; i++) {
		psel = &gCache.physP.physList[(int)gPorts[i]].other;
		//[RK]Sep/08/05 - fixed problem with changed
		ConvTextArea(psel->loginBanner,psel->loginBanner,1);
		if (spc || memcmp((char*)&pcnf->stty,(char*)&psel->stty, size)) {
			old = psel->socketPort; //[RK]Feb/23/05 
			memcpy((char*)psel, (char *)pcnf,sizeof(PhysOtherInfo));
			psel->action = kSpec; 
			if (spc) {   //[RK]Feb/23/05 
				psel->socketPort += i; //[RK]Sep/14/04 - increase the socket_port
			} else {
				psel->socketPort = old;
			}
			gCache.physP.physList[(int)gPorts[i]].action = kEdit;
			change = 1;
		}
	}
	return(change);
}
