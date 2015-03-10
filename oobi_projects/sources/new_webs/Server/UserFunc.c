/* Framework code: (c) 2001 Art & Logic, Inc. All Rights Reserved. */
/* Project Specific code: (c) 2003 Cyclades */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>

#include "webs.h"

#include "Access.h"
#include "Error.h"
#include "GetSet.h"
#include "Locals.h"
#include "Localize.h"
#include "UserFunc.h"
#if defined(KVM) || defined(ONS)
#include "KVMFunc.h"
#endif
#include "PortsFunc.h"

#include <src/server.h>
#include <src/mysyslog.h>
#include "ResultCodes.h" /* define the Result enum */
#define qErrorStringTable
#include "ResultCodes.h" /* re-include, now defining the kResultStrings array */
#undef qErrorStringTable
#ifdef PMDNG
#include <pmclib.h>
#endif

#include "PageId.h"

#include <cyc.h>
#include <time.h>

#include <userinfo.h>

extern struct cyc_info *cyc;

#define itoa(sid, temp, len) snprintf(temp, len, "%d", sid)

//already defined in /opt/montavista/pro/devkit/ppc/8xx/target/usr/include/signal.h...
//typedef void (*sighandler_t)(int);

/* Constants (and pseudo-constants) */
char_t kSidKey[15]; // quasi-constant (DMF tolower()'s it, so it can't be a _real_ const)
static char_t kChangesLedKey[15]; // another quasi-constant, for the LED that signals changes
char_t kSel1Key[15]; // another quasi-constant, for a transient selection in page A related to a modification page B
static char_t kSel2Key[15]; // quasi-constant, for another transient selection (page C <- page B)
static char_t kSel3Key[15]; // quasi-constant, for another transient selection (page C <- page B)
char_t kCurrentAdminName[32];

//WARNING MESSAGES
//implicit declaration of function `dmfSetUrlOk'
extern void dmfSetUrlOk(char_t *, char_t *);
//warning: implicit declaration of function `EncryptString'
int EncryptString(char *, char *, int);
//warning: implicit declaration of function `HexEncode'
char *HexEncode(char *, int);
//warning: implicit declaration of function `portMountPortsStatusHtml'
void portMountPortsStatusHtml(char *);
//warning: implicit declaration of function `RegisterParamChange'
void RegisterParamChange(int, void*, CycCategory, CycKey, UInt32);
//warning: implicit declaration of function `portMountPortsStatisticsHtml'
void portMountPortsStatisticsHtml(char *);

/* Data */
ReqData gReq; /* this (HTTP) request (extern (UserFunc.h) definition) */
LoginData gLogin; /* login request (extern (UserFunc.h) definition) */
SystemData gCacheSave;
SystemData gCache; /* cached system data (extern (UserFunc.h) definition) */
/* Note that for all of the calls to CycGet() and CycSet(), herein, we pass
the addresses of the values in gCache, on the stack.  This is somewhat
dangerous if we can't fully trust the engine.  This approach could be replaced
with a temporary created (heap or stack) and passed to each CycGet()/CycSet() -
then the engine doesn't have access to our master cache datastore. */
static CycParam* gChanges = NULL; /* collection of CycParam changes to send to CycSet */
/* Note that the change queue is maintained as a contiguous array rather than
a dynamic list because that's how we've defined the API.  A list would give us
more flexibility, but would be more complicated to handle if we ever had to
speak through IPCs. */
static UInt16 gCurrentChangePos = 0; /* position in gChanges array */
static const UInt16 kChangesGrowthSize = 10; /* growth factor for enlarging gChanges */

WebUserInfo wUser;

static int WebNeedsRestart;

static int gWizard = 0;
/* Fake kPortSpecs for Expert mode */
SInt8 gPorts[kMaxPhysicalPorts+2] = {0};
static SInt8 allPorts[] = {-1, 0};
int gLedState = 0;
static int gKportSpec = 0;      // If kPortsSpec was put into vector changes
bool gCacheLoaded[kNumConfigPages]; /* Single-admin model: the first
time the admin (only one logged in at a time) sees a "config" page, that page
is flagged as "loaded", using gCacheLoaded.  In the future, the system data is
not loaded "over the top of" the cache because (1) it's assumed that the 
system config won't be changed underneath (this admin is the only one who can
change it), and (2) we don't want to obliterate changes made in this cache by
the admin who's making them.  Another way to handle this, especially if we
had to support multiple admins, would be to send the data back and forth in
cookies, as "session" data is often maintained.  However, it's possible that
the cookie payload may become rather large, and since we are enforcing one
admin only at a time (which is realistic for the userbase), there's no need
to increase our static data size: we still need one bucket for one copy of all
the data in the system.
*/

int checkCancelRequest(int request,int page);
static int isEven(UInt16 x);
static char *TransOptGetStr(TranslateOptions *Op, int nOp, int value);
static int SetNetSettings(void);
static int SetAutoDiscovery(void);
static int SetCasProSettings(void);
static int SetAccess(void);
static int SetDataBuffering(void);
static int SetSystemLog(void);
#ifdef PMD
static int SetIpduOutMan(void);
static int SetIpduOutGrpCtrl(void);
static int SetViewIpduInfo(void);
static int SetIpduConfig(void);
static int SetIpduSwUpgrade(void);
static int SetpmdGen(void);
static int SetpmdOutGrp(void);
static int SetpmdUserMan(void);
#endif
#ifdef IPMI
static int SetPmIpmi(void);
#endif
static int SetNotifications(void);
static int SetSNMPdaemon(void);
//static int SetNetServices(void); warning: `SetNetServices' declared `static' but never defined
static int SetHostTable(void);
#ifdef IPSEC
static int SetVPNTable(void);
#endif
static int SetStaticRoutes(void);
static int SetBootConf(void);
static int SetTimeDate(void);
static int SetMenuShell(void);
static int SetVirtualPortsInfo(void);
static int SetPhysPortConf(void);
#ifdef PCMCIA
static int SetAccessMethod(void);
#endif
static int SetIpTables(void);
static int SetSecProfile(void);

static int SetOnlineHelp(void);
int GetOnlineHelp(int,webs_t,int,char_t **);

#if defined(KVM) || defined(ONS)
extern Result LoadKVMDevices(void);
int ShowViewerHtml(int,webs_t,int,char_t **);
int ShowIPCascadeHtml(int,webs_t,int,char_t **);
static void setCommand(webs_t, char_t *, char_t *);
static int SetKVMDConf(void);
extern int SetKVMAccList(void);
static int SetKVMauxPort(void);
extern int SetKVMDevices(void);
int CheckStationRequest(int,webs_t,int,char_t **);
int SwitchStation(int,webs_t,int,char_t **);
int CycleStation(int,webs_t,int,char_t **);
int StationStatus(int,webs_t,int,char_t **);
int StartUserInfo(struct _stinfo *); 
int UserInfo(int,webs_t,int,char_t **);
int ReconnectStation(int,webs_t,int,char_t **);
int StationVideoConf(int,webs_t,int,char_t **);
int PMSwitch(int,webs_t,int,char_t **);
int ResyncServer(int,webs_t,int,char_t **);
Result LoadKVMDConf(void);
struct _stinfo old_stinfo[4];

#ifdef RDPenable
int ShowRDPHtml(int,webs_t,int,char_t **);
int CheckRDPconn(int,webs_t,int,char_t **);
extern int SetRDPservers(void);
#endif	//RDPenable

#endif
static int SetAuthType(void);
static int SetRadius(void);
static int SetTacplus(void);
#ifdef SMBenable
static int SetSmb(void);
#endif
#ifdef NISenable
static int SetNis(void);
#endif
static int SetLdap(void);
#ifdef KRBenable
static int SetKrb(void);
#endif
extern int ShowAppletHtml();
extern int ShowGrapherAppletHtml();
static void WriteProfileFile();
static void DeleteProfile();
static void ReadDirList();

//[AP][2007-08-03] hostname discovery
void expandADStrings(void);

#ifdef PMDNG
static char_t PMDngUserName[kUserNameLength]; //[RK]Nov/03/06
#endif

TranslateOptions IpChainProtocolOptions[] =
{
  {"All", kAll},
  {"ICMP", kIcmp},
  {"TCP", kTcp},
  {"UDP", kUdp},
#ifdef IPv6enable
  {"ICMPv6", kIcmp6},
#endif
  {"Numeric", kNumeric}
};

TranslateOptions IpChainFragmentOptions[] =
{
  {"All packets", kallPackets},
  {"2nd, 3rd... fragmented packets", konlyFragments},
  {"Non-fragmented and 1st fragmented packets", knoFragFirstFrag}
};

void ConvTextArea(char *bufConf, char *bufWeb, int to)
{
	unsigned char *d, *s;
	unsigned char *dest;
	int size;

	if (to) {
		s = bufWeb;
	} else {
		s = bufConf;
	}
	size = strlen(s);
	dest = malloc(size * 2);
	memset(dest, 0, size*2);
	d = dest;
	
	while (*s) {
		if (to && (*s == '\r') && (*(s+1) == '\n')) {
			*d++ = '\\';
			*d++ = 'r';
			*d++ = '\\';
			*d++ = 'n';
			s+=2;
			continue;
		}
		if (to && (*s == '\n') && (*(s+1))) {
			*d++ = '\\';
			*d++ = 'n';
			s++;
			continue;
		}
		if (!to && (*s == '\\') && (*(s+1) == 'r')) {
			*d++ = '\r';
			s+=2;
			continue;
		}
		if (!to && (*s == '\\') && (*(s+1) == 'n')) {
			*d++ = '\n';
			s+=2;
			continue;
		}
		*d++ = *s++;
	}
	*d=0x00;
	if (to) {
		strcpy(bufConf,dest);
	} else {
		strcpy(bufWeb,dest);
	}

	free(dest);//mp
}

void ConvGroups(char *bufConf, char *bufWeb)
{
	unsigned char *d, *s;

	d = bufConf;
	s = bufWeb;

	while (*s) {
		if ((*s == '\r') && (*(s+1) == '\n')) {
			s+=2;
			continue;
		}
		*d++ = *s++;
	}
	*d=0x00;
}

/*
 * ReplaceSPwithNBSP - Reads the sptr string and copies it into dptr string while
 * replacing all occurences of space character with "&nbsp;" group of characters. 
 */
void ReplaceSPwithNBSP(char *dptr, char *sptr)
{
	char * ptr;

	for(ptr = sptr; *ptr != '\0'; ptr++)
		if (*ptr != ' ') {
			*dptr = *ptr;
			dptr++;
		}
		else {
			strncpy(dptr, "&nbsp;", 6);
			dptr += 6;
		}
	*dptr = '\0';
}

void includeNbsp(unsigned char *dest,unsigned char *buf, int maxsize)
{
   int count=0;

	if (buf) {
		strncpy(dest,buf,maxsize);
		*(dest+maxsize) = 0x00;
		count = strlen(dest);
	} else {
	       *dest = 0x00;
	}	       
	
	for (; count < maxsize ; count++) { 
		strcat(dest,"&nbsp;");
	}
}

/* Standard DMF initialization hook implementations */
/* ------------------------------------------------------------------------- */

void dmfUserPreInit(void)
{
   char_t temp[10];
   char_t* webLangDir;

   /* Set the system-wide "constants" */
   #ifdef qDebug
   dmfSetParam(T("_kDebug_"), NULL, T("1"), 0, -1);
   #else
   dmfSetParam(T("_kDebug_"), NULL, T("0"), 0, -1);
   #endif

   gdtoa(temp, kSet);
   dmfSetParam(T("_kSet_"), NULL, temp, 0, -1);
   
   gdtoa(temp, kTry);
   dmfSetParam(T("_kTry_"), NULL, temp, 0, -1);

   gdtoa(temp, kCancel);
   dmfSetParam(T("_kCancel_"), NULL, temp, 0, -1);

   gdtoa(temp, kReloadPage);
   dmfSetParam(T("_kReloadPage_"), NULL, temp, 0, -1);

   gstrcpy(kSidKey, T("_sid_")); /* must strcpy due to the tolower() call inside of dmfGetParam! */
   dmfSetParam(kSidKey, NULL, T("0"), 0, -1);

   gstrcpy(kChangesLedKey, T("_changesLed_"));
   dmfSetParam(kChangesLedKey, NULL, T("0"), 0, -1);

   gstrcpy(kSel1Key, T("_sel1_"));
   dmfSetParam(kSel1Key, NULL, T("-1"), 0, -1);

   gstrcpy(kSel2Key, T("_sel2_"));
   dmfSetParam(kSel2Key, NULL, T("-1"), 0, -1);

   gstrcpy(kSel3Key, T("_sel3_"));
   dmfSetParam(kSel3Key, NULL, T("-1"), 0, -1);

   gstrcpy(kCurrentAdminName, T("_currentAdminName_"));
   dmfSetParam(kCurrentAdminName, NULL, T("none"), 0, -1);

   /* Set the language dir: */
   fmtAlloc(&webLangDir, BUF_MAX, T("%s/Locale"), websGetDefaultDir());
   dmfSetLanguagePath(webLangDir);
   bfreeSafe(B_L, webLangDir);

   /* Initialize the DMF's parameter tree. */
   dmfInitParams(parameterList, parameterCount);
}

void dmfUserPostInit(void)
{
   /* Set the web root dir */
   //char_t* webRootDir = NULL;
   //fmtAlloc(&webRootDir, BUF_MAX, T("%s/../../Web"), websGetDefaultDir());
   //websSetDefaultDir(webRootDir);
   //bfreeSafe(B_L, webRootDir);

   /* Set up our error list */
   dmfSetAppErrors(kResultStrings, kCycLastError);

   /* Set up the WebApi environment */
   CycInitApi();

#ifdef UBOOT_PARAM
   dmfSetParam("_ubootparam_", NULL, "1", 0, -1);
#else
   dmfSetParam("_ubootparam_", NULL, "0", 0, -1);
#endif
   dmfSetParam("_boardname_", NULL, CycGetBoardName(), 0, -1);
   dmfSetParam("_familymodel_", NULL, CycGetFamilyModel(), 0, -1);
   dmfSetParam("_familyext_", NULL, CycGetFamilyExt(), 0, -1);
   dmfSetParam("_rdpconnect_", NULL, CycGetRDPconnect(), 0, -1);

#ifdef SHOW_DSVIEW_OPTIONS
   dmfSetParam("_showDSVIEWoptions_", NULL, "1", 0, -1); 
#else
   dmfSetParam("_showDSVIEWoptions_", NULL, "0", 0, -1);
#endif

#ifdef TARGET_ACS5K
   dmfSetParam("_acs5k_", NULL, "1", 0, -1);
#else
   dmfSetParam("_acs5k_", NULL, "0", 0, -1);
#endif

#ifdef IPv6enable
   dmfSetParam("_IPv6_", NULL, "1", 0, -1);
#else
   dmfSetParam("_IPv6_", NULL, "0", 0, -1);
#endif

    /* Register the new ejScript functions with the framework */
#if (defined(KVM) || defined(ONS))
   websAspDefine(T("showViewerHtml"), ShowViewerHtml);
#ifdef IPCASCADING
   websAspDefine(T("showIPCascadeHtml"), ShowIPCascadeHtml);
   websFormDefine(T("SetCommand"), setCommand);
#endif
   websAspDefine(T("checkStationRequest"), CheckStationRequest);
   websAspDefine(T("switchStation"), SwitchStation);
   websAspDefine(T("reconnectStation"), ReconnectStation);
   websAspDefine(T("cycleStation"), CycleStation);
   websAspDefine(T("stationStatus"), StationStatus);
   StartUserInfo(old_stinfo);
   websAspDefine(T("userInfo"), UserInfo);
   websAspDefine(T("stationVideoConf"), StationVideoConf);
   websAspDefine(T("pmSwitch"), PMSwitch);
   websAspDefine(T("resyncServer"), ResyncServer);

#ifdef RDPenable 
   websAspDefine(T("showRDPHtml"), ShowRDPHtml);
   websAspDefine(T("checkRDPconn"), CheckRDPconn);
#endif	//RDPenable

#endif
   /*passing arg 2 of `websAspDefine' from incompatible pointer type
   websAspDefine(T("showAppletHtml"), (AspFunc) ShowAppletHtml);
   */
   websAspDefine(T("showAppletHtml"), ShowAppletHtml);
   websAspDefine(T("getOnlineHelp"), GetOnlineHelp);

	if ((libcyc_kvm_get_model(cyc) == MODEL_KVMNET_PLUS) ||
	    (libcyc_kvm_get_model(cyc) == MODEL_ONSITE)) {
		/*passing arg 2 of `websAspDefine' from incompatible pointer type
		websAspDefine(T("showGrapherAppletHtml"), (AspFunc) ShowGrapherAppletHtml);
		*/
		websAspDefine(T("showGrapherAppletHtml"), ShowGrapherAppletHtml);
		websFormDefine(T("WriteProfileFile"), WriteProfileFile);
		websFormDefine(T("DeleteProfile"), DeleteProfile);
		websFormDefine(T("ReadDirList"), ReadDirList);
	}
}

void dmfUserPreClose(void)
{
}

void dmfUserPostClose(void)
{
}

/* Local shared util functions */
/* ------------------------------------------------------------------------- */

void ResetSidEtc(int request)
{
   if ((gReq.sid == 0) && (gReq.action == kNone)) {
	   return;
   }
   
   dmfSetParam(kSidKey, NULL, T("0"), request, -1);
   dmfSetParam(kSel1Key, NULL, T("-1"), request, -1);
   dmfSetParam(kSel2Key, NULL, T("-1"), request, -1);
   /* Clear ultra-transients */
   gReq.action = kNone;
   gReq.sid = 0;
}

/* Login, etc. special handler functions */
/* ------------------------------------------------------------------------- */

/* Clear gChanges list */
static void ClearChanges(void)
{
   bfreeSafe(B_L, gChanges);
   gChanges = NULL;
   gCurrentChangePos = 0;
   gKportSpec = 0;
}

/* Clear gCacheLoaded state */
static void ClearPages(void)
{
   int x;

   for (x = 0; x < kNumConfigPages; ++x) {
	if (gCacheLoaded[x] != false) {
		// Free allocated memory
		switch(x) {
			case kAccessUsers :
   				bfreeSafe(B_L, gCache.access.usersHtml);
   				gCache.access.usersHtml = 0;
				bfreeSafe(B_L, gCache.access.users);
				gCache.access.users = 0;
				gCache.access.changedUser = 0;
				break;
			case kAccessGroups :
				bfreeSafe(B_L, gCache.access.groupsHtml);
				gCache.access.groupsHtml = 0;
				bfreeSafe(B_L, gCache.access.groups);
				gCache.access.groups = 0;
				gCache.access.changedGrp = 0;
				break;
			case kSystemLog :
   				bfreeSafe(B_L, gCache.systemLog.serversHtml);
   				gCache.systemLog.serversHtml = 0;
   				bfreeSafe(B_L, gCache.systemLog.servers);
   				gCache.systemLog.servers = 0;
				break;
			case kUnit :
   				bfreeSafe(B_L, gCache.unitInfo.versionHtml);
   				gCache.unitInfo.versionHtml = 0;
   				bfreeSafe(B_L, gCache.unitInfo.pwrSupplyHtml);
   				gCache.unitInfo.pwrSupplyHtml = 0;
   				bfreeSafe(B_L, gCache.unitInfo.memHtml);
   				gCache.unitInfo.memHtml = 0;
				break;
#ifdef PCMCIA
			case kUnitPcmcia :
   				bfreeSafe(B_L, gCache.unitInfo.pcmcia);
   				gCache.unitInfo.pcmcia = 0;
   				bfreeSafe(B_L, gCache.unitInfo.pcmciaHtml);
   				gCache.unitInfo.pcmciaHtml = 0;
				break;
#endif
			case kUnitRam :
   				bfreeSafe(B_L, gCache.unitInfo.ram);
   				gCache.unitInfo.ram = 0;
   				bfreeSafe(B_L, gCache.unitInfo.ramHtml);
   				gCache.unitInfo.ramHtml = 0;
				break;
#ifdef PMD
			case kPMDGeneral :
				bfreeSafe(B_L, gCache.pmdGen.confInfoHtml);
				gCache.pmdGen.confInfoHtml = 0;
				memset(gCache.pmdGen.sysMsg,0,sizeof(gCache.pmdGen.sysMsg));
				break;
			case kPMDOutletGroups :
				bfreeSafe(B_L, gCache.pmdOutGrp.groupsListHtml);
				gCache.pmdOutGrp.groupsListHtml = 0;
				memset(gCache.pmdOutGrp.sysMsg,0,sizeof(gCache.pmdOutGrp.sysMsg));
				break;
			case kPMDUsersManagement :
				bfreeSafe(B_L, gCache.pmdUserMan.usersListHtml);
				gCache.pmdUserMan.usersListHtml = 0;
				memset(gCache.pmdUserMan.sysMsg,0,sizeof(gCache.pmdUserMan.sysMsg));
				break;
#endif

#ifdef IPMI
			case kPmIpmiPage :
   				bfreeSafe(B_L, gCache.pmipmi.ipmiInfoHtml);
   				gCache.pmipmi.ipmiInfoHtml = 0;
				bfreeSafe(B_L, gCache.pmipmi.ipmiInfo);
				gCache.pmipmi.ipmiInfo = 0;
				break;
#endif
			case kNotifications :
   				bfreeSafe(B_L, gCache.notifications.notif);
   				gCache.notifications.notif = 0;
   				bfreeSafe(B_L, gCache.notifications.notifHtml);
   				gCache.notifications.notifHtml = 0;
   				bfreeSafe(B_L, gCache.notifications.triggerHtml);
   				gCache.notifications.triggerHtml = 0;
				break;
			case kSNMPdaemonSettings :
   				bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv12p);
   				gCache.SNMPdaemon.SNMPv12p = 0;
   				bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv12Html);
   				gCache.SNMPdaemon.SNMPv12Html = 0;
   				bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv3p);
   				gCache.SNMPdaemon.SNMPv3p = 0;
   				bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv3Html);
   				gCache.SNMPdaemon.SNMPv3Html = 0;
				break;
#ifdef IPSEC
			case kVPNTablePage :
				bfreeSafe(B_L, gCache.VPNTable.VPNTableHtml);
				bfreeSafe(B_L, gCache.VPNTable.VPN);
				memset(&gCache.VPNTable, 0, sizeof(gCache.VPNTable));
				break;
#endif
			case kHostTablePage :
   				bfreeSafe(B_L, gCache.hostTable.hostTableHtml);
   				gCache.hostTable.hostTableHtml = 0;
   				bfreeSafe(B_L, gCache.hostTable.hosts);
   				gCache.hostTable.hosts = 0;
				break;
			case kStaticRoutes :
   				bfreeSafe(B_L, gCache.staticRoutes.routes);
   				gCache.staticRoutes.routes = 0;
   				bfreeSafe(B_L, gCache.staticRoutes.stRoutesHtml);
   				gCache.staticRoutes.stRoutesHtml = 0;
				break;
			case kMenuShell :
				bfreeSafe(B_L, gCache.menuShell.menuOptionHtml);
				gCache.menuShell.menuOptionHtml = 0;
				gCache.menuShell.menuShellChanged = 0;
				break;
	  		case kVirtualPorts :
				bfreeSafe(B_L,gCache.virtualP.virtualInfoHtml);
				gCache.virtualP.virtualInfoHtml = 0;
				bfreeSafe(B_L,gCache.virtualP.virtualInfo);
				gCache.virtualP.virtualInfo = 0;
				break;
			case kIpTablesPage: 
				bfreeSafe(B_L, gCache.ipTable.ipTableHtml);
				gCache.ipTable.ipTableHtml = 0;
				bfreeSafe(B_L, gCache.ipTable.rulesHtml);
				gCache.ipTable.rulesHtml = 0;
				bfreeSafe(B_L, gCache.ipTable.chainTable);
				gCache.ipTable.chainTable = 0;
#ifdef IPv6enable
				bfreeSafe(B_L, gCache.ipTable.chainTable6);
				gCache.ipTable.chainTable6 = 0;
#endif
				break;
			case kPortPhysTable :
				bfreeSafe(B_L,gCache.physP.physTableHtml);
				bfreeSafe(B_L,gCache.physP.physList);
#ifdef IPMI
				bfreeSafe(B_L,gCache.physP.ipmiDeviceListHtml);
#endif
				gCacheLoaded[kPortPhysConf] = false;
				memset((char*)&gCache.physP,0,sizeof(PhysPorts));
				break;
			case kActiveSess:
				bfreeSafe(B_L, gCache.session.sessionHtml);
				gCache.session.sessionHtml = 0;
				bfreeSafe(B_L,gCache.session.sess);
				gCache.session.sess = 0;
				break;
#ifdef PCMCIA
			case kAccessMethod:
				gCache.accessMethod.pcmConfigChanged = 0;
				memset(&gCache.accessMethod, 0, sizeof(PcmciaAccessMethod));
				break;
#endif
			case ksecAuth:
//				gCache.authAction.type = 0;
				gCache.authU.unitauthtype = 0;
				gCache.authU.pppauthtype = 0;
				break;

			case ksecRadius:
//				gCache.authAction.radius = 0;
				memset(&gCache.authU.radius, 0, sizeof(RadiusInfo));
				break;
#ifdef KRBenable
			case ksecKrb:
//				gCache.authAction.krb = 0;
				memset(&gCache.authU.krb, 0, sizeof(KrbInfo));
				break;
#endif
			case ksecLdap:
//				gCache.authAction.ldap = 0;
				memset(&gCache.authU.ldap, 0, sizeof(LdapInfo));
				break;
#ifdef NISenable
			case ksecNis:
//				gCache.authAction.nis = 0;
				memset(&gCache.authU.nis, 0, sizeof(NisInfo));
				break;
#endif
			case ksecTacplus:
//				gCache.authAction.tacplus = 0;
				memset(&gCache.authU.tacplus, 0, sizeof(TacplusInfo));
				break;
#ifdef SMBenable
			case ksecSmb:
//				gCache.authAction.smb = 0;
				memset(&gCache.authU.smb, 0, sizeof(SmbInfo));
				break;
#endif
#if defined(KVM) || defined(ONS)
           		case kKVMDAccList:
				while(gCache.kvmuser.nalloc--) {
					bfreeSafe(B_L, gCache.kvmuser.useracc.userperms[gCache.kvmuser.nalloc].perms);
				}
				bfreeSafe(B_L, gCache.kvmuser.useracc.defuserperm.perms);
				bfreeSafe(B_L, gCache.kvmuser.useracc.userperms);
				gCache.kvmuser.useracc.defuserperm.perms = NULL;
				gCache.kvmuser.useracc.defuserperm.nperms = 0;
				gCache.kvmuser.useracc.userperms = NULL;
				gCache.kvmuser.nalloc = gCache.kvmuser.useracc.nacclists = 0;
				gCache.kvmuser.KVMPermHtml = NULL;
				gCache.kvmuser.KVMDefPermHtml = NULL;
				break;
#endif
		}

	}

	gCacheLoaded[x] = false;
   }

   gLedState = 0;

}

/* Sets the LED change indicator */
void SetLedChanges(int request, int val)
{
   char str_val[8];

   snprintf(str_val, 7, "%d", val);

   dmfSetParam(kChangesLedKey, NULL, str_val, request, -1);
}

/* ------------------------------------------------------------------------- */
#/* COMMIT FUNCTIONS */ 
/* Page "commit" (POST/Set) (utils and) handler functions */
/* ------------------------------------------------------------------------- */
#define TMP_FILE "/tmp/webtemp2"

extern int backupConfigCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;
	char buffer[1024];
	backupConfig_st *pb = &gCache.backupConfig;
	int status;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	if (checkCancelRequest(request,-1))
		return mAppError(result);

	if (gReq.action != kNone) {
		return expertCommit(param, request, urlQuery);
	}

	if (!*pb->command) {
   		ResetSidEtc(request);
		return mAppError(result);
	}

	if (gLedState) {
   		ResetSidEtc(request);
		 return mAppError(kCommitPending);
	}

	//[RK]Feb/24/05 - save the iptables in /etc/network/firewall
	if (!strcmp(pb->command, "saveconf")) {
		system("iptables-save >/etc/network/firewall");
#ifdef IPv6enable
		system("ip6tables-save >/etc/network/firewall6");
#endif
	} 
	
	/* From/to FTP server */
	if(strcmp(pb->bkptype,"1")==0) {
	sprintf(buffer, "%s ftp %s %s %s %s 2>&1",
		pb->command, pb->PathName, pb->ServerName,
		pb->UserName, pb->Password);
	}
	else {
		if(strcmp(pb->SDDefaultoption,"1")==0) {
			if(strcmp(pb->SDReplaceoption,"1")==0) {
				sprintf(buffer,"%s sd default replace 2>&1",pb->command);
			}
			else sprintf(buffer,"%s sd default 2>&1",pb->command);
		}
		else if (strcmp(pb->SDReplaceoption, "1") == 0) sprintf(buffer,"%s sd replace 2>&1", pb->command);
		else sprintf(buffer,"%s sd 2>&1", pb->command);
	}

//	printf("command: %s\n", buffer);
	status = system(buffer);
//	printf("status: %d\n", WEXITSTATUS(status));

	bfreeSafe(B_L , pb->Message);
	pb->Message = balloc(pb->Message, 256);	

	switch (WEXITSTATUS(status)) {
	case 0:
		if (!strcmp(pb->command, "saveconf")) {
	    		strcpy(pb->Message,"Configuration was uploaded successfully.");
		} else { 
	    		strcpy(pb->Message,"Configuration was downloaded successfully.");
		}
	    	break;
	case 10:
	    strcpy(pb->Message, "configuration file list does not exist");
	    break;
	case 11:
	    strcpy(pb->Message, "configuration file list is empty");
	    break;
	case 12:
	    strcpy(pb->Message, "failed to create the tar file");
	    break;
	case 13:
	    strcpy(pb->Message, "failed to create compressed file");
	    break;
	case 14:
	    strcpy(pb->Message, "Invalid compressd config file");
	    break;
	case 15:
	    strcpy(pb->Message, "failed to exttract configuration files");
	    break;
	case 20:
	    strcpy(pb->Message, "Could not connect to ftp server");
	    break;
	case 21:
	    strcpy(pb->Message, "Authentication failed");
	    break;
	case 22:
	    strcpy(pb->Message, "Permission denied");
	    break;
	case 23:
	    strcpy(pb->Message, "file transfer failed");
	    break;
	case 24:
	    strcpy(pb->Message, "No such file or directory");
	    break;
	case 25:
	    strcpy(pb->Message, "File is a directory");
	    break;
	default:
	    *pb->Message = 0;	    
	}

	//[RK]Aug/16/05 - Load treatment
	if (!strcmp(pb->command, "restoreconf")) {
		system("iptables-restore </etc/network/firewall");
#ifdef IPv6enable
		system("ip6tables-restore </etc/network/firewall6");
#endif
		result = CycSet(NULL, 0, kTryForced);
		ClearPages();
		SetLedChanges(request, 0);
		ClearChanges(); 

		//[RK]Aug/18/05 - add message about the configuration
		if (WEXITSTATUS(status) == 0) {
		    strcat(pb->Message, " The loaded configuration is activated.\nClick in Apply Changes to save it");
		}
	}
	*pb->command = 0;
   	ResetSidEtc(request);
	return mAppError(result);
}

extern int Login(Param* param, int request, char_t* urlQuery)
{
   Sid sid = 0;
   char_t temp[128];
   Result result;
   struct stat statbuf;

#if defined(KVM) || defined(ONS)
   /* Bring this variable over from KVMFunc.c.  KVMFunc.c really
      needs accessor functions to avoid hacks like this. */
   extern KVMportlist KVMport;
#endif

   if (WebNeedsRestart)
	   return mAppError(kLoginMustWait);

#if defined(KVM) || defined(ONS)
	syslog(LOG_DEBUG, "Directacc = %d, user = [%s]\n", (gCache.kvmconf.kvmd.general.directacc), gLogin.userName);
	if (gLogin.directacc && gLogin.KVMPort && *gLogin.KVMPort) {
		// perform port authentication check
		result = CycLogIn(gLogin.userName, gLogin.password, &sid, &wUser, 0, 1);
	} else {
		// perform unit authentication check 
		result = CycLogIn(gLogin.userName, gLogin.password, &sid, &wUser, 0, 0);
	}
#else
   /* Log in, obtain SID, and get the user type */
   result = CycLogIn(gLogin.userName, gLogin.password, &sid, &wUser, 0, 0);
#endif

   if (kCycSuccess != result) {
   	    if (kAnotherAdminIsOn == result) {
        	strcpy(gLogin.userLogin, gLogin.userName);
                sprintf(temp, "?SSID=%d", sid);
			dmfSetUrlOk("/loginConfirm.asp", temp); 
	        result = kCycSuccess;
	    }
	    syslog(LOG_DEBUG, "Auth Not Successful\n");
	    goto exit_rot;
   }
        /*--------------------------------------------------------------------------------
        Author:         Christine Qiu(09.2005)
        Task:           Enhance Authorization
        Explain:        KVM use the groupname list which from the remote authentication server to
                        get the user's permission.
        Implementation: add a new parameter(char *authGroupName) in correspoding
                        function call.Usually the user's permission only from user name, now
                        every function call which used to get user's permission will take two
                        parameters. one if username and other is auth groupname.
        ----------------------------------------------------------------------------------*/

#if defined(KVM) || defined(ONS)
        /* Look up user info */
        /*Christine Qiu
          Task: Enhance Authorization
          doing: add "wUser.uGroupName" for KVMGetUserInfo
        */
        KVMGetUserInfo(gLogin.userName, &KVMport, wUser.uGroupName);
	//KVMPMGetNumPorts();

	/* Forward user to correct start page, depending on user type */
	if (gLogin.directacc && gLogin.KVMPort && *gLogin.KVMPort) {
		int i;

		for (i = 0; i < KVMport.nports; i++) {
			if (strcmp(KVMport.port[i].name, gLogin.KVMPort) == 0) {
	                	sprintf(temp, "?name=%s&SSID=%d", gLogin.KVMPort, sid);
				dmfSetUrlOk("/normal/applications/appl/accessConnectPortsDA.asp", temp); 
				syslog(LOG_DEBUG, "Launching direct access to KVM Port %s\n", gLogin.KVMPort);
				goto exit_rot;
			}
		}
		syslog(LOG_DEBUG, "Denied direct access to KVM Port %s\n", gLogin.KVMPort);
		result = kInvalidPort;
		CycLogOut(sid);
		goto exit_rot;
	}
#endif

//	ClearChanges(); // clear any residue from old session
//	ClearPages(); //mp: need this even for Regular User, due to IPDU Power Mgmt.
//	SetLedChanges(request, 0);

	if (kAdminUser == wUser.uType) {
		ClearChanges(); // clear any residue from old session
		ClearPages();
		SetLedChanges(request, 0);
		dmfSetParam(kCurrentAdminName, NULL, gLogin.userName, request, -1);
		CycSetCurrentAdmin(gLogin.userName);
		sprintf(temp, "?x=e&SSID=%d", sid);
		/* [LMT] Security Enhancements */
		if (stat("/etc/security.opts", &statbuf)){
			dmfSetUrlOk("/wizard/main.asp", temp); 
		}else{
			dmfSetUrlOk("/normal/main.asp", temp); 
		}
	} else {
		sprintf(temp, "?x=n&SSID=%d", sid);
		dmfSetUrlOk("/normal/main.asp", temp);
	}

	syslog(LOG_INFO,
	       "%s: User [%s] for session [%d] successfully authenticated", 
	       TAG_AUTH, gLogin.userName, sid) ;

exit_rot:

   /* Clear ultra-transients */
   strcpy(gLogin.userName, "");
   strcpy(gLogin.password, "");
   /* rem: no need to clear gReq.sid or .action b/c the login page
   doesn't actually set these as fields.  now that there is a sid,
   future pages will set gReq.sid as a field, so those commitFunc
   handlers will have to clear them. (Of course, this should all be
   done with cookies!!! */

   ResetSidEtc(request);
   return mAppError(result);
}

extern int fakeCommit(Param* param, int request, char_t* urlQuery)
{
   int result = kCycSuccess;

   return mAppError(result);
}

extern int LoginConfirm(Param* param, int request, char_t* urlQuery)
{
	int result;
	char_t temp[128];
	struct stat statbuf;
#if defined(KVM) || defined(ONS)
	/* Bring this variable over from KVMFunc.c.  KVMFunc.c really
	   needs accessor functions to avoid hacks like this. */
	extern KVMportlist KVMport;
#endif

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess) {
		ResetSidEtc(request);
		return mAppError(result);
	}

	if (!gLogin.confirm ||
		(result = CycBecomeAdminBoss(gReq.sid)) != kCycSuccess) {
		ResetSidEtc(request);
		syslog(LOG_INFO, "%s: Cancel new admin [%s] login", TAG_AUTH, gLogin.userLogin);
		dmfSetUrlOk("/login.asp", ""); 
		return mAppError(result);
	}

	// Clear record of old admin's un-applied changes:
	syslog(LOG_INFO, "%s: User [%s] for session [%d] successfully authenticated", 
		TAG_AUTH, gLogin.userLogin, gReq.sid);
#if defined(KVM) || defined(ONS)
	/* Look up user info */
        /*Christine Qiu
          Task: Enhance Authorization
          doing: add "wUser.uGroupName" for KVMGetUserInfo
        */
        KVMGetUserInfo(gLogin.userLogin, &KVMport, wUser.uGroupName);

	/* Forward user to correct start page, depending on user type */
	if (gLogin.directacc && gLogin.KVMPort && *gLogin.KVMPort) {
		int i;

		for (i = 0; i < KVMport.nports; i++) {
			if (strcmp(KVMport.port[i].name, gLogin.KVMPort) == 0) {
            	sprintf(temp, "?name=%s&SSID=%d", gLogin.KVMPort, gReq.sid);
				dmfSetUrlOk("/normal/applications/appl/accessConnectPortsDA.asp", temp); 
				goto exit_rot;
			}
		}
		syslog(LOG_DEBUG, "Denied direct access to KVM Port %s\n", gLogin.KVMPort);
		result = kInvalidPort;
		CycLogOut(gReq.sid);
		goto exit_rot;
	}
#endif
	ClearChanges();
	ClearPages();
	SetLedChanges(request, 0);
    sprintf(temp, "?x=e&SSID=%d", gReq.sid);
	/* [LMT] Security Enhancements */
	if (stat("/etc/security.opts", &statbuf)){
		dmfSetUrlOk("/wizard/main.asp", temp); 
	}else{
		dmfSetUrlOk("/normal/main.asp", temp); 
	}
	CycSetCurrentAdmin(gLogin.userLogin);
	dmfSetParam(kCurrentAdminName, NULL, gLogin.userLogin, request, -1);
	ResetSidEtc(request);
#if defined(KVM) || defined(ONS)
exit_rot:
#endif
	return mAppError(result);
}

extern int Logout(Param* param, int request, char_t* urlQuery)
{
  Result result;
  Sid sid = gReq.sid;

  if (*gReq.logout) {
      return (mAppError(kCycSuccess));
  }
  gReq.sid = 0;

  /* Log out the user associated with the specified sid */
  result = CycLogOut(sid);
  if (kCycSuccess == result) {
      gsprintf(urlQuery, "to=login");
  }

  WebNeedsRestart = FALSE;

  return mAppError(result);
}

extern int tablePageCommit(Param* param, int request, char_t* urlQuery)
{
	int result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

	if (gReq.action != kNone) {
		if (!strcmp(gReq.bogus,"wizLastPage"))
			return wizardCommit(param, request, urlQuery);
		else
			return expertCommit(param, request, urlQuery);
	}

	ResetSidEtc(request);
	return mAppError(result);
}

extern int expertCommit(Param* param, int request, char_t* urlQuery)
{
   Result result = kCycSuccess;

   gWizard = 0;
   gLedState = 0;

#if defined(KVM) || defined(ONS)
	SetAccess();
	SetKVMDConf();
	SetKVMAccList();
	SetKVMDevices();
	SetKVMauxPort();
#endif

	SetSNMPdaemon();
	//SetNetServices();
	SetNetSettings();
	SetSystemLog();
	SetHostTable();
#ifdef IPMI
	SetPmIpmi();
#endif
#ifdef IPSEC
	SetVPNTable();
#endif
	SetStaticRoutes();
#if !defined(KVM) && !defined(ONS)
	SetAccess();
#endif
	SetNotifications();
	SetBootConf();
	SetTimeDate();
	SetMenuShell();
	SetVirtualPortsInfo();
#ifdef PCMCIA
	SetAccessMethod();
#endif
	SetSecProfile();
	SetOnlineHelp(); //Set Online Help Config

	SetIpTables();

	if (gCacheLoaded[kPortPhysTable]) 
		SetPhysPortConf();
#ifdef PMD
	SetpmdGen();
	SetpmdOutGrp();
	SetpmdUserMan();
#endif

	SetAuthType();
	SetRadius();
	SetTacplus();
	SetLdap();
#ifdef KRBenable
	SetKrb();
#endif
#ifdef SMBenable
	SetSmb();
#endif
#ifdef NISenable
	SetNis();
#endif

#ifdef RDPenable
	SetRDPservers();
#endif	//RDPenable

	//[AP][2007-08-02] hostname discovery
	SetAutoDiscovery();

	if (gCurrentChangePos) {

		result = CycSet(gChanges, gCurrentChangePos, gReq.action);
		if (kCycSuccess == result) {
			// Success - start over gChanges:
		    ClearPages();
		    SetLedChanges(request, 0);
			//ClearChanges(); //mp: I understand that ClearPages() and ClearChanges()
			                //should go together...
		}
		ClearChanges();   //mp (see above)
	} else {

		if (gReq.action == kSet) //mp: we must call the webapi anyway to run "saveconf"
			CycSet(gChanges, gCurrentChangePos, gReq.action);
		    SetLedChanges(request, 0);

	}

	ResetSidEtc(request);
	return mAppError(result);
}

extern int wizardCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

   gWizard = 1;
   gLedState = 0;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,-1)) return mAppError(result);

#ifdef SECPROFILE
   SetSecProfile();
#endif
   SetNetSettings();
   SetCasProSettings();
   SetAccess();
   SetDataBuffering();
   SetSystemLog();

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      ResetSidEtc(request);
      SetLedChanges(request, gLedState);
      return mAppError(result);
   }

   if (gCurrentChangePos) {
      result = CycSet(gChanges, gCurrentChangePos, gReq.action);
      if (kCycSuccess == result) {
         // Success - start over gChanges:
          ClearPages();
          SetLedChanges(request, 0);
		  //ClearChanges(); //mp: I understand that ClearPages() and ClearChanges()
		                  //should go together...
	  }
	  ClearChanges();   //mp (see above)
   } else {
	   
	   if (gReq.action == kSet) //mp: we must call the webapi anyway to run "saveconf"
		   CycSet(gChanges, gCurrentChangePos, gReq.action);
	   
   }

   ResetSidEtc(request);
   return mAppError(result);
}

//[RK]Jun/14/04 - Access Privilege
extern int RegUserSecurityCommit(Param* param, int request, char_t* urlQuery)
{
	int operRes;
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

	if (gCache.regUser.oldPasswd[0]) {
		operRes = CycRegUserPasswd(wUser.uName,gCache.regUser.oldPasswd,
			gCache.regUser.newPasswd);

		gCache.regUser.operResult = operRes;
	}

      	ResetSidEtc(request);

	return (mAppError(kCycSuccess));
}

extern int netSettingsCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kNetSettings)) return mAppError(result);

   //gLedState = 0; mp: commented on Bug Fix 1863

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetNetSettings();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   if (gCache.netSettings.mode) { // Normal mode
      return expertCommit(param, request, urlQuery);
   } else {
      return wizardCommit(param, request, urlQuery);
   }   
}

//[AP][2007-08-02] hostname discovery
extern int autoDiscoveryCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kAutoDiscovery)) return mAppError(result);

   //gLedState = 0; mp: commented on Bug Fix 1863

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetAutoDiscovery();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }
   gCache.autoDiscovery.ADStringChange = 0;

   return expertCommit(param, request, urlQuery);
}

static int treatUserAdd(void)
{
	Users* pu, *old;
	int i;

	pu = gCache.access.users;

	// first check user name
#if defined(KVM) || defined(ONS)
	if (!(strcmp(gCache.access.addModUser.userName, "rpc"))
		|| !(strcmp(gCache.access.addModUser.userName, "nobody"))
		|| !(strcmp(gCache.access.addModUser.userName, "sshd"))) {
		return kAddUserFailed;
	}
#endif
	for (i=0; i < gCache.access.numUsers; i++, pu++) {
		if ((pu->action != kDelete) && (pu->action != kActionIgnore) &&
		    !(strcmp(pu->userName, gCache.access.addModUser.userName))) {
			// user name exist
			return kAddUserFailed;
		}
	}
	old = gCache.access.users;
	if (!(gCache.access.users = balloc(B_L, sizeof(Users) * (gCache.access.numUsers + 1))))
		return kMemAllocationFailure; //mp
	memcpy((char *)gCache.access.users, (char*)old, gCache.access.numUsers * sizeof(Users));
	bfreeSafe(B_L, old);
	pu = &gCache.access.users[gCache.access.numUsers];
	memcpy((char *)pu, &gCache.access.addModUser, sizeof(Users));
	gCache.access.numUsers++;
	pu->action = kAdd;
	return(kCycSuccess); 
}

static int treatGroup(int action)
{
	ConfGroups *pg, *old;
	int i;

	pg = gCache.access.groups;

	if (action && (gCache.access.selected > gCache.access.numGroups)) return 0;
	
	// check group name
	for (i=0; i < gCache.access.numGroups ; i++, pg++) {
		if (!strcmp(pg->newGrpName, gCache.access.addModGroup.newGrpName) &&
		    ((i+1) != gCache.access.selected)) {
			return 0;
		}
	}
	
	if (action) { // edit
		pg = &gCache.access.groups[gCache.access.selected-1];
		memcpy((char *)pg, (char*)&gCache.access.addModGroup,sizeof(ConfGroups));
		ConvGroups(pg->usersInGrp,pg->usersInGrp);
		return 1;
	}

	// ADD group
	if (gCache.access.addModGroup.newGrpName[0]) { //check if group name exists
		old = gCache.access.groups;
		if (!(gCache.access.groups = balloc(B_L, sizeof(ConfGroups) * (gCache.access.numGroups + 1))))
			return kMemAllocationFailure; //mp
		memcpy(gCache.access.groups, old, gCache.access.numGroups * sizeof(ConfGroups));
		bfreeSafe(B_L, old);
		pg = &gCache.access.groups[gCache.access.numGroups];
		memcpy((char *)pg, (char *)&gCache.access.addModGroup, sizeof(ConfGroups));
		ConvGroups(pg->usersInGrp,pg->usersInGrp);
		gCache.access.numGroups++;
		return 1;
	} else {
		return 0;
	}
}

extern int accessCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;
	Users *pu;
	ConfGroups *pg;
	int changeUsr = 0,changeGrp=0,num,usrdel=0,groupdel=0;
	char buf[20];

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	if (checkCancelRequest(request,kAccessUsers)) {
		gCacheLoaded[kAccessGroups] = 0;
	       	return mAppError(result);
	}

   //	gLedState = 0;

	// treat Users list
	for (num=0, pu=gCache.access.users; num < gCache.access.numUsers; num++, pu++) {
#if defined(KVM) || defined(ONS)
		sprintf(buf,",%d!",num+1);
#else
		sprintf(buf,",%d,",num+1);
#endif
		if (strstr(gCache.access.usersCs,buf)==NULL) { 
			// user was deleted
			if (pu->action == kAdd) {
				pu->action = kActionIgnore;
				usrdel++;
			} else {
				//[RK]Sep/22/05 - not delete root and admin
				if (strcmp(pu->userName,"root") &&
				    strcmp(pu->userName,ADMIN_USER)) {
					pu->action = kDelete;
					changeUsr = 1;
				}
			}
		}
	}
	
	if (gCache.access.mode) {
		// treat Group list
		for (num=0, pg=gCache.access.groups; num < gCache.access.numGroups; num++, pg++) {
			sprintf(buf,",%d,",num+1);
			if (strstr(gCache.access.groupsCs,buf)==NULL) { 
				// user was deleted
				changeGrp = groupdel++;
				memset((char*)pg,0, sizeof(ConfGroups));
			}
		}
	}
	
	switch(gCache.access.action) {
		case 0 : // none
			break;
		case 1 : // edit user ==> change password
			pu = &gCache.access.users[gCache.access.selected-1];
#if defined(KVM) || defined(ONS)
			if (!strcmp(pu->userName,"root")) break;
#endif
			strcpy(pu->passWord, gCache.access.addModUser.passWord);
			strcpy(pu->passWord2, gCache.access.addModUser.passWord2);
			if (pu->action == kNone)
				pu->action = kEdit;
			changeUsr = 1;
			break;
		case 2 : // add user
			if ((result=treatUserAdd()) == kCycSuccess)
				changeUsr = 1;
			break;
		case 3 : // edit group
			changeGrp = treatGroup(1); 
			break;
		case 4 : // add group
			changeGrp = treatGroup(0);
			break;	
	}
	
	if (usrdel) {
		for (num=1, pu=gCache.access.users; num < gCache.access.numUsers; num++, pu++) {
			if (pu->action == kActionIgnore) {
				memcpy((char*)pu, (char*)(pu+1), sizeof(Users)*(gCache.access.numUsers-num));
				gCache.access.numUsers--;			
			}
		}
	}

	if (groupdel) {
		for (num=0, pg=gCache.access.groups; num < gCache.access.numGroups;) {
			if (!pg->newGrpName[0]){
				gCache.access.numGroups--;
				memcpy((char*)pg,(char*)(pg+1),sizeof(ConfGroups) * (gCache.access.numGroups - num));
				memset((char*)(gCache.access.groups + gCache.access.numGroups), 0, sizeof(ConfGroups));
				changeGrp = 1;
			} else {
				pg++;
				num++;
			}
		}
	}

	gCache.access.changedUser |= changeUsr;
	gCache.access.changedGrp |= changeGrp;
	
	if (gReq.action == kNone) {
		if (gCache.access.changedUser || gCache.access.changedGrp) {
			gLedState++;
		}
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	if (gCache.access.mode) { // Normal mode
		return expertCommit(param, request, urlQuery);
	} else {
		return wizardCommit(param, request, urlQuery);
	}   
}

extern int SystemLogCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kSystemLog)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetSystemLog();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   if (gCache.systemLog.mode) { // Normal mode
      return expertCommit(param, request, urlQuery);
   } else {
      return wizardCommit(param, request, urlQuery);
   }   
}

#ifdef PMD
// Power Management - New Outlets Manager page
extern int IpduOutManCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kIPDUOutletsManager)) return mAppError(result);

	//[RK]Nov/03/06 - set the username
	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);

   // this is because there is nothing to be registered for commit action
   SetIpduOutMan(); //so we don't want expertCommit to call this function

   // if it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// Power Management - New Outlet Groups Ctrl page
extern int IpduOutGrpCtrlCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kIPDUOutletGroupsCtrl)) return mAppError(result);

	//[RK]Nov/03/06 - set the username
	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);

   // this is because there is nothing to be registered for commit action
   SetIpduOutGrpCtrl(); //so we don't want expertCommit to call this function

   // if it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// Power Management - New View IPDUs Info page
extern int ViewIpduInfoCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kViewConfIPDUs)) return mAppError(result);

   // this is because there is nothing to be registered for commit action
   SetViewIpduInfo(); //so we don't want expertCommit to call this function

   // if it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// Power Management - New Configuration page
extern int IpduConfigCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kViewConfIPDUs)) return mAppError(result);

   // this is because there is nothing to be registered for commit action
   SetIpduConfig();

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// Power Management - New Software Upgrade page
extern int IpduSwUpgradeCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kIPDUSoftwareUpgrade)) return mAppError(result);

   // this is because there is nothing to be registered for commit action
   SetIpduSwUpgrade(); //so we don't want expertCommit to call this function

   // if it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// Power Management - New PMD Config, General tab
extern int pmdGenCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kPMDGeneral)) return mAppError(result);

   // if it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetpmdGen();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// Power Management - New PMD Config, Outlet Groups tab
extern int pmdOutGrpCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kPMDOutletGroups)) return mAppError(result);

   // if it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetpmdOutGrp();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// Power Management - New PMD Config, Users Management tab
extern int pmdUserManCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kPMDUsersManagement)) return mAppError(result);

   // if it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetpmdUserMan();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

#endif

#ifdef IPMI
// Power Management - IPMI
extern int PmIpmiCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kPmIpmiPage)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetPmIpmi();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}
#endif

extern int NotificationsCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kNotifications)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetNotifications();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

extern int SetEventsNotif(Param* param, int request, char_t* urlQuery)
{
   int i;
   NotificationInfo * notif;
   Result result = kCycSuccess;

   if (!gCacheLoaded[kNotifications]) return(0); // The page was even loaded
	if (gCache.notifications.triggerId == -1) {
		// ADD operation
		notif = gCache.notifications.notif;
		if (!(gCache.notifications.notif = 
			  balloc(B_L, sizeof(NotificationInfo) * (gCache.notifications.numNotif + 1))))
			return mAppError(kMemAllocationFailure); //mp
		memcpy(gCache.notifications.notif, notif,
				sizeof(NotificationInfo) * gCache.notifications.numNotif);
		bfreeSafe(B_L, notif);
		notif = &gCache.notifications.notif[gCache.notifications.numNotif];
		gCache.notifications.numNotif++;
		notif->action = kAdd;
		notif->triggerId = 0;
		notif->type = gCache.notifications.notifType;
		memcpy(notif->trigger, gCache.notifications.trigger,kTriggerLength);
		notif->trigger[kTriggerLength-1] = 0x00;
		memcpy(&notif->event, &gCache.notifications.event,sizeof(Events));
	} else {
		// Edit operation
		notif = gCache.notifications.notif;
	   	for (i=0; i < gCache.notifications.numNotif ; i++,notif++) {
			if ((notif->action != kDelete) &&
			    (notif->triggerId == gCache.notifications.triggerId)) {
				// found
				if (notif->action == kNone) 
					notif->action = kEdit;
				memcpy(notif->trigger, gCache.notifications.trigger,kTriggerLength);
				notif->trigger[kTriggerLength-1] = 0x00;
				memcpy(&notif->event, &gCache.notifications.event,sizeof(Events));
				break;
			}
		}
	}

	switch (notif->type) {
		case kNtEmail :
			ConvTextArea(notif->event.email.body,gCache.notifications.event.email.body,1);
			break;
		case kNtPager :
			ConvTextArea(notif->event.pager.text,gCache.notifications.event.pager.text,1);
			break;
		case kNtSnmpTrap :
			ConvTextArea(notif->event.snmptrap.body,gCache.notifications.event.snmptrap.body,1);
			break;
	}

	gCache.notifications.triggerId = -1;
	gCache.notifications.notifType = -1;
	memset(&gCache.notifications.event,0,sizeof(Events));
	memset(gCache.notifications.trigger,0,kTriggerLength);

	gLedState++; //mp
	SetLedChanges(request, gLedState);
	ResetSidEtc(request);
	return mAppError(result);
}

// COMMIT - Network -> SNMP Daemon Settings - main page
extern int SNMPdaemonCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kSNMPdaemonSettings)) return mAppError(result);

	// If it was just a menu navigation does not commit anything
	if (gReq.action == kNone) {
		SetSNMPdaemon();
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	return expertCommit(param, request, urlQuery);
}

/*extern int netServicesCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kServices)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetNetServices();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);

}*/

/* [LMT] Security Enhancements */
extern int SecProfileCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;
    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kSecProfile)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetSecProfile();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);

      return mAppError(result);
   }

   if (gCache.secProfile.mode) { // Normal mode
      return expertCommit(param, request, urlQuery);
   } else {
      return wizardCommit(param, request, urlQuery);
   }   

}

extern int HostTableCommit(Param* param, int request, char_t* urlQuery)
{
	Hosts *new, *pold;
	char *pentry, *ptr;
	int i;
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request, kHostTablePage)) return mAppError(result);
	
	if (!gCacheLoaded[kHostTablePage]) return(0); // The page was even loaded
     	
	pold = gCache.hostTable.hosts;

	if (gCache.hostTable.numHostsSet) {
		int size;

		pentry = gCache.hostTable.hostTableSet;
		size = sizeof(Hosts) * gCache.hostTable.numHostsSet;
		if (!(gCache.hostTable.hosts = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
		memset(gCache.hostTable.hosts, 0, size);
		for (i = 0, new = gCache.hostTable.hosts; 
				i < gCache.hostTable.numHostsSet; i++, new++) {
			ptr = strstr(pentry, ",");
			*ptr = 0;
			if (sscanf(pentry, "%s %s %s",
					new->hostIp, new->hostName, new->hostAlias) < 2) {
				break;
			}
			pentry = ptr + 1;
			if(*pentry == 0) break;
		}	
	} else {
		gCache.hostTable.hosts = NULL;
	}

	if (gCache.hostTable.numHosts != gCache.hostTable.numHostsSet) {
		gCache.hostTable.hostsChanged = 1;
	} else {
		i = sizeof(Hosts) * gCache.hostTable.numHostsSet;
		if (memcmp(pold, gCache.hostTable.hosts, i)) {
			gCache.hostTable.hostsChanged = 1;
		}
	}
	
	gCache.hostTable.numHosts = gCache.hostTable.numHostsSet;
	bfreeSafe(B_L, pold);

	// If it was just a menu navigation does not commit anything
	if (gReq.action == kNone) {
		if (gCache.hostTable.hostsChanged)
			gLedState++;
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	return expertCommit(param, request, urlQuery);
}

// COMMIT - Network -> Static Routes - table
extern int StaticRoutesCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kStaticRoutes)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetStaticRoutes();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

extern int BootConfCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kBootInfo)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
#ifdef UBOOT_PARAM
   if (gCache.bootInfo.uBoot == 1 ||
      gCache.bootInfo.uBoot == 2) {
      gCache.bootInfo.currentImage = gCache.bootInfo.uBoot + '0';
      gCache.bootInfo.uBoot = kFlash;
   } else {
      gCache.bootInfo.uBoot = kNetwork;
   }
#endif
   if (gReq.action == kNone) {
      SetBootConf();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

extern int UpgradeCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,-1)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kSet) {
      result = UpGradeFirmware(&gCache.upGradeInfo);
   }
      ResetSidEtc(request);
      return mAppError(result);
}

extern int RebootCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,-1)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gCache.unitInfo.reboot == 1) {
		syslog(LOG_INFO, "%s: System rebooted by admin [%s] [%s] [%s]",
			TAG_SYSTEM, wUser.uName, gCache.netSettings.hostName, gCache.netSettings.ipAddress1);
		sleep(2);
      CycReboot();
   }
      ResetSidEtc(request);
   return mAppError(result);
}

extern int TimeDateCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kTimeDate)) return mAppError(result);

	// If it was just a menu navigation does not commit anything
	if (gReq.action == kNone) {
		SetTimeDate();
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	return expertCommit(param, request, urlQuery);
}

// COMMIT - Ports -> Virtual Ports -> New/Modify Port (pop up)
extern int SetVirtualPort(Param* param, int request, char_t* urlQuery)
{
   int i;
   VirtualPortsInfo * virtual;
   Result result = kCycSuccess;

   if (!gCacheLoaded[kVirtualPorts]) return(0); // The page was even loaded

   if (gCache.virtualP.action == kAdd) {
      // ADD operation
      virtual = &gCache.virtualP.virtualInfo[gCache.virtualP.numVirtual];
      gCache.virtualP.numVirtual++;
      memcpy(virtual, &gCache.virtualP.virtualPort,sizeof(VirtualPortsInfo));
      virtual->action = kAdd;
   	  gLedState++;
   } else {
      // Edit operation
      virtual = gCache.virtualP.virtualInfo;
         for (i=0; i < gCache.virtualP.numVirtual ; i++,virtual++) {
         if (!strcmp(virtual->remoteIp, gCache.virtualP.remoteIp)) {
            // found
            if (memcmp(virtual, &gCache.virtualP.virtualPort,sizeof(VirtualPortsInfo))) {
   	  			gLedState++;
            	memcpy(virtual, &gCache.virtualP.virtualPort,sizeof(VirtualPortsInfo));
            	if (virtual->action == kNone) virtual->action = kEdit;
            	break;
			}
         }
      }
   }
   memset(&gCache.virtualP.virtualPort,0,sizeof(VirtualPortsInfo));

   SetLedChanges(request, gLedState);
//[RK]Feb/24/05 - VirtualPortsInfoCommit will do -> ResetSidEtc(request);
   return mAppError(result);
}

// COMMIT Ports -> Virtual Ports - table 
extern int VirtualPortsInfoCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kVirtualPorts)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetVirtualPortsInfo();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

// COMMIT Ports -> Physical Ports - Action 
extern int PhysActionCommit(Param * param, int request, char_t *urlQuery)
{
	Result result;
	int i,num,gLedStateAux=0;
	char *psel,*pnum,pc;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,-2)) return mAppError(result);

	if (gReq.action != kNone) {
		return expertCommit(param, request, urlQuery);
	}
	
	if (!gCacheLoaded[kPortPhysTable]) {  // The page was even loaded
		return 0;
	}
	
#ifdef ONS
	for (i=0; i < (gCache.device.number2+1); i++) gPorts[i] = 0;
#else
	for (i=0; i < (gCache.device.number+1); i++) gPorts[i] = 0;
#endif

	gCache.physP.numPortsel = 0;
	psel = gCache.physP.selected;
#ifdef ONS
	for (i=0; (i < gCache.device.number2) && *psel; i++) {
#else
	for (i=0; (i < gCache.device.number) && *psel; i++) {
#endif
		pnum = psel;
		while (*psel && (*psel != ',')) psel++;
		pc = *psel;
		if (*psel) *psel = 0x00;
		num = atoi(pnum);
		if (pc) *psel++ = pc;
#ifdef ONS
		if (num <= gCache.device.number2) {
#else
		if (num <= gCache.device.number) {
#endif
			gPorts[gCache.physP.numPortsel++] = num;
		}
	}
	
	switch (gCache.physP.action) {
		case 1 : // disable selected ports
			gLedStateAux = portPhysChangeStatus(0);
			break;

		case 2 : // enable selected ports
			gLedStateAux = portPhysChangeStatus(1);
			break;

		case 3 : // modify selected ports
			break;
			
		case 0 : // none
			break;
			
	}
	if (gLedStateAux) gLedState++;
	SetLedChanges(request, gLedState);
	ResetSidEtc(request);
	return mAppError(result);
}

// COMMIT Ports -> Physical Ports -> DONE or CANCEL
static void PhysPortClearConf(int cancel)
{
	if (cancel) { // Cancel the configuration
		gCache.sysBufServer = gCacheSave.sysBufServer;
	}
	gCacheLoaded[kPortPhysConf] = false;
#ifdef PMD
	gCacheLoaded[kPMDPortsInfo] = false;
	gCacheLoaded[kPMDOutletGroups] = false;
	gCacheLoaded[kPMDUsersManagement] = false;
#endif
	memset((char*)&gCache.physP.infoP, 0, sizeof(PhysPortsInfo));
	memset((char*)&gCache.physP.selected,0, sizeof(gCache.physP.selected));
#ifdef IPMI
	bfreeSafe(B_L, gCache.physP.ipmiDeviceListHtml);
	gCache.physP.ipmiDeviceListHtml = 0;
#endif
	gCache.physP.action = 0;
	gCache.physP.physPortAction = 0;
}

extern int PhysPortCommit(Param * param, int request, char_t *urlQuery)
{
	int cont,gLedStateAux=0;
	Result result;

	//[RK]Mar/01/05 
	if (gCacheLoaded[kPortPhysConf] == false) {
		return mAppError(kCycSuccess);
	}

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (gReq.action == kCancel) {
		PhysPortClearConf(1);
	       	if (checkCancelRequest(request,-2)){
			return mAppError(result);
		}
	}
	if (gReq.action == kReloadPage) {
		// [JY] fix to 3774:
		// clean up, and reread from shared memory when page is loaded.
		bfreeSafe(B_L,gCache.physP.physTableHtml);
		bfreeSafe(B_L,gCache.physP.physList);
#ifdef IPMI
		bfreeSafe(B_L,gCache.physP.ipmiDeviceListHtml);
#endif
		gCacheLoaded[kPortPhysConf] = false;
		memset((char*)&gCache.physP,0,sizeof(PhysPorts));
		gCacheLoaded[kPortPhysTable] = false;
#ifdef PMD
		gCacheLoaded[kPMDPortsInfo] = false;
		gCacheLoaded[kPMDOutletGroups] = false;
		gCacheLoaded[kPMDUsersManagement] = false;
#endif
		if (checkCancelRequest(request,-2)){
			return mAppError(result);
		}
	}
	

#ifdef PMD
	//[RK]Jul/27/04 - Fixed Bug 2160 : SSH/Telnet to IPDU does not permit authtype NONE.
	if ((gCache.physP.infoP.general.protocol == P_IPDU) &&
	    (gCache.physP.infoP.general.pmsessions != 0) &&
            (gCache.physP.infoP.access.authtype == kPatNone)) {
            gCache.physP.infoP.access.authtype = kPatLocal;
    	}
#endif

	if (gCache.physP.physPortAction == 2) { // DONE 
		if (gPorts[0] == 0) { // modify ALL
			/*[LMT] CyBTS Bug 3360 - fixed LED problem*/
			gLedStateAux = portPhysChangeAll(); 
		} else {
			int res = kCycSuccess; //mp
			gLedStateAux = 0;
			gLedStateAux = portPhysChangeGeneral();
			gLedStateAux |= portPhysChangeSerial();
			gLedStateAux |= portPhysChangeAccess();
			gLedStateAux |= portPhysChangeDataBuf();
			gLedStateAux |= portPhysChangeSysBuf();
			gLedStateAux |= portPhysChangeMus();
#if defined(PMD) || defined(IPMI)
			if ((res = portPhysChangePowerMgm()) == kMemAllocationFailure) return mAppError(res);
#endif
			gLedStateAux |= res;
			gLedStateAux |= portPhysChangeCas();
			gLedStateAux |= portPhysChangeTs();
			gLedStateAux |= portPhysChangeDialIn();
			gLedStateAux |= portPhysChangeOther();
		}
		PhysPortClearConf(0);
	}		
	
	// check IPDU 
#ifdef PMD
	if (gCache.physP.physPortAction != kNone) { 
		gCache.physP.numPowerMgm = 0;
		for (cont = 0; cont < gCache.device.number; cont++) {
			if (gCache.physP.physList[cont].general.protocol == P_IPDU) {
				gCache.physP.numPowerMgm ++;
			}
		}
	}
#endif
	if (gLedStateAux) gLedState++;
	SetLedChanges(request, gLedState);
	
	if (gReq.action != kNone) {
		PhysPortClearConf(0);
		return expertCommit(param, request, urlQuery);
	}
	
	ResetSidEtc(request);
	return mAppError(result);
}

extern int ActiveSessionCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;
   ActiveSessionInfo *sess; 

   if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kActiveSess)) return mAppError(result);

   gCacheLoaded[kActiveSess] = false;
   
   if (gCache.session.action == kDelete) { 
	// Kill a session 
     if (gCache.session.selectedOption < gCache.session.numSessions) {
       // It's a local or KVM session
       sess = gCache.session.sess + gCache.session.selectedOption;	
       result = CycKillActiveSession(sess);
       ResetSidEtc(request);
       return mAppError(result);
     }
 
#ifdef RDPenable
     // It's an RDP session
	result = KillRdpConnection(gCache.session.selectedOption - gCache.session.numSessions);
	ResetSidEtc(request);
	return mAppError(result);
#endif	//RDPenable
   }

   if (gReq.action != kNone)
     return expertCommit(param, request, urlQuery);

   ResetSidEtc(request);
   return mAppError(result);
}

#ifdef PCMCIA
extern int AccessMethodCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	if (gCache.accessMethod.pcmConfigChanged)
		gLedState++;

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kAccessMethod)) return mAppError(result);

   gCacheLoaded[kUnitPcmcia] = false;
   //gCacheLoaded[kAccessMethod] = false;
   if (gCache.accessMethod.action == kAdd)
	{ // Insert
      result = CycInsertPcmciaCard(gCache.accessMethod.selectedOption);
   } 
	else if (gCache.accessMethod.action == kDelete)
	{ // Eject
      result = CycEjectPcmciaCard(gCache.accessMethod.selectedOption);
	}
	else 
	{
		if (gReq.action == kNone) {
			SetAccessMethod();
			SetLedChanges(request, gLedState);
			ResetSidEtc(request);
			return mAppError(kCycSuccess);
		} 

		return expertCommit(param, request, urlQuery);
	}

	ResetSidEtc(request);
   return mAppError(result);
}
#endif

extern int MenuShellCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	if (gCache.menuShell.menuShellChanged)
		gLedState++;

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kMenuShell)) return mAppError(result);

	if (gReq.action == kNone) {
		SetMenuShell();
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	} 

	return expertCommit(param, request, urlQuery);
}

// IP Tables Commit functions
extern int IpTableChainsCommit(Param* param, int request, char_t* urlQuery)
{
	int i,j,k;
	Result result = kCycSuccess;
	ChainInfo *chain;
	char *pcs,*ptr;
	int index,change,ignore,size;
	char name[kChainNameLength], policy[kTargetLength];
#ifdef IPv6enable
	ChainInfo *chain6, *chainx, *chainxo;
	char table[5]; // can be either IPv4 or IPv6
#define CHAINX(idx) (idx > gCache.ipTable.confNumChains)? &chain6[idx-gCache.ipTable.confNumChains-1] : &chain[idx-1]
#endif

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kIpTablesPage)) return mAppError(result);
	
	if (!gCacheLoaded[kIpTablesPage]) return(0); // The page was even loaded
     	
	if (gCache.ipTable.numChains == 0) {
		return (0);
	}
	
	chain = gCache.ipTable.chainTable;
#ifdef IPv6enable
	chain6 = gCache.ipTable.chainTable6;
#endif
	pcs = gCache.ipTable.chainsCs;
	
	change = ignore = 0;
	
	for (j=1, i=0; i < gCache.ipTable.numChains; i++) {
		index = -1;
		ptr = strstr(pcs,",");
		if (!ptr) break;
		*ptr++ = 0x00;
#ifdef IPv6enable
		if (sscanf(pcs,"%d %s %s %s",&index,name,policy,table) != 4) {
			break;
		}
#else
		if (sscanf(pcs,"%d %s %s",&index,name,policy) != 3) {
			break;
		}
#endif
		pcs = ptr;
		if (index == 0) { // chain was added
			break;
		}
		if (index > j) {
#ifdef IPv6enable
			for (k=j; k < index; k++) {
				chainx = CHAINX(k);	
				change = 1;
				if (chainx->action == kAdd) {
					ignore = k;
					chainx->action = kActionIgnore;
				} else {
					chainx->action = kDelete;
				}
			}
#else
			for (k=j; k < index; k++,chain++) {
				change = 1;
				if (chain->action == kAdd) {
					ignore = k;
					chain->action = kActionIgnore;
				} else {
					chain->action = kDelete;
				}
			}
#endif
			j = index;
		}
		
#ifdef IPv6enable
		chainx = CHAINX(index);

		if (chainx->action == kDelete) continue;
		
		if (strcmp(chainx->name, name)) {
			change = 1;
			strcpy(chainx->name,name);
			if (chainx->action != kAdd) {
				chainx->action = kEdit;
			}
		}
		
		if (strcmp(chainx->policy, policy)) {
			change = 1;
			strcpy(chainx->policy,policy);
			if (chainx->action != kAdd) {
				chainx->action = kEdit;
			}
		}
#else
		if (chain->action == kDelete) continue;
		
		if (strcmp(chain->name, name)) {
			change = 1;
			strcpy(chain->name,name);
			if (chain->action != kAdd) {
				chain->action = kEdit;
			}
		}
		
		if (strcmp(chain->policy, policy)) {
			change = 1;
			strcpy(chain->policy,policy);
			if (chain->action != kAdd) {
				chain->action = kEdit;
			}
		}
		chain++;
#endif	
		j = index+1;
	}

#ifdef IPv6enable
	if (j <= (gCache.ipTable.confNumChains+gCache.ipTable.confNumChains6)) {
		for (k=j; k <= (gCache.ipTable.confNumChains+gCache.ipTable.confNumChains6); k++) {
			chainx = CHAINX(k);
			change = 1;
			if (chainx->action == kAdd) {
				ignore = k;
				chainx->action = kActionIgnore;
			} else {
				chainx->action = kDelete;
			}
		}
	}
#else
	if (j <= gCache.ipTable.confNumChains) {
		for (k=j; k <= gCache.ipTable.confNumChains; k++, chain++) {
			change = 1;
			if (chain->action == kAdd) {
				ignore = k;
				chain->action = kActionIgnore;
			} else {
				chain->action = kDelete;
			}
		}
	}
#endif

	if (ignore) {
		ignore = 0;
		chain = gCache.ipTable.chainTable;
		for (j = 0; j < gCache.ipTable.confNumChains; j++) {
			if (chain->action == kActionIgnore) {
				bfreeSafe(B_L, chain->rules);
				k = gCache.ipTable.confNumChains - j;
				if ((j+1) < gCache.ipTable.confNumChains) { 
					memcpy((char*)chain,(char *)(chain+1),sizeof(ChainInfo)*k);
				}
				ignore++;
			} else {
				chain++;
			}
		}
		gCache.ipTable.confNumChains -= ignore;
#ifdef IPv6enable
		ignore = 0;
		chain6 = gCache.ipTable.chainTable6;
		for (j = 0; j < gCache.ipTable.confNumChains6; j++) {
			if (chain6->action == kActionIgnore) {
				bfreeSafe(B_L, chain6->rules);
				k = gCache.ipTable.confNumChains6 - j;
				if ((j+1) < gCache.ipTable.confNumChains6) { 
					memcpy((char*)chain6,(char *)(chain6+1),sizeof(ChainInfo)*k);
				}
				ignore++;
			} else {
				chain6++;
			}
		}
		gCache.ipTable.confNumChains6 -= ignore;
#endif
	}

	if (index == 0) { // ADD
#ifdef IPv6enable
		if (!strcmp(table,"IPv6")) {
			size = sizeof(ChainInfo) * (gCache.ipTable.confNumChains6+1);
			chainxo = gCache.ipTable.chainTable6;
			if (!(gCache.ipTable.chainTable6 = (ChainInfo *)balloc(B_L, size)))
				return mAppError(kMemAllocationFailure);
			chainx = gCache.ipTable.chainTable6;
			gCache.ipTable.confNumChains6++;
		} else {
			size = sizeof(ChainInfo) * (gCache.ipTable.confNumChains+1);
			chainxo = gCache.ipTable.chainTable;
			if (!(gCache.ipTable.chainTable = (ChainInfo *)balloc(B_L, size)))
				return mAppError(kMemAllocationFailure);
			chainx = gCache.ipTable.chainTable;
			gCache.ipTable.confNumChains++;
		}
		memset((ChainInfo*)chainx,0,size);
		size -= sizeof(ChainInfo);
		memcpy((ChainInfo*)chainx,(ChainInfo*)chainxo,size);
		bfreeSafe(B_L, (ChainInfo*)chainxo);
		chainxo = &chainx[size/sizeof(ChainInfo)];
		strcpy(chainxo->name, name);
		strcpy(chainxo->policy, policy);
		chainxo->action = kAdd;
#else
		size = sizeof(ChainInfo) * (gCache.ipTable.confNumChains+1);
		chain = gCache.ipTable.chainTable;
		if (!(gCache.ipTable.chainTable = (ChainInfo *)balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
		memset((char*)gCache.ipTable.chainTable,0,size);
		size -= sizeof(ChainInfo);
		memcpy((char*)gCache.ipTable.chainTable,(char*)chain,size);
		bfreeSafe(B_L, (char *)chain);
		chain = &gCache.ipTable.chainTable[gCache.ipTable.confNumChains];
		strcpy(chain->name, name);
		strcpy(chain->policy, policy);
		chain->action = kAdd;
		gCache.ipTable.confNumChains++;
#undef CHAINX(idx)
#endif
		change=1;
	}

	if (change) { //mp
		gCache.ipTable.chainChanged = 1;
	}
		
	// If it was just a menu navigation does not commit anything
	if (gReq.action == kNone) {
		if (gCache.ipTable.chainChanged)
			gLedState++;
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	return expertCommit(param, request, urlQuery);
}

extern int IpTableRulesCommit(Param* param, int request, char_t* urlQuery)
{
	int i,j;
	ChainInfo *chain;
	char *pcs;
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kIpTablesPage)) return mAppError(result);
	
	if (!gCacheLoaded[kIpTablesPage]) return(0); // The page was even loaded
     	
	if (gCache.ipTable.chainSelected) {
		IpTablesRules *rule,*old;
		int size;
		
#ifdef IPv6enable
		if (gCache.ipTable.chainSelected>gCache.ipTable.confNumChains) {
			chain = gCache.ipTable.chainTable6;
			chain += gCache.ipTable.chainSelected-gCache.ipTable.confNumChains-1;
		} else {
			chain = gCache.ipTable.chainTable;
			chain += gCache.ipTable.chainSelected-1;
		}
		old = chain->rules;
#else
		chain = gCache.ipTable.chainTable;
		chain += gCache.ipTable.chainSelected-1;
		old = chain->rules;
#endif
		size = sizeof(IpTablesRules) * (gCache.ipTable.numRules);
		if (!(chain->rules = (IpTablesRules *)balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
		memset((char *)chain->rules, 0, size);
		rule = chain->rules;
		
		pcs = gCache.ipTable.rulesCs;

		for (i=0; (i < gCache.ipTable.numRules) && *pcs; i++) {
			if (sscanf(pcs,",%d,",&j) != 1) break;
			if (gCache.ipTable.ruleSelected == j) {
				gCache.ipTable.ruleSelected = 0;
				gCache.ipTable.actionRule = 0;
				if (memcmp(old+j-1, (char *)&gCache.ipTable.rule,sizeof(IpTablesRules))){
					gCache.ipTable.chainChanged = 1;
				}
				memcpy((char *)rule,(char*)&gCache.ipTable.rule,sizeof(IpTablesRules));
			} else {
				memcpy((char *)rule,old+j-1,sizeof(IpTablesRules));
			}
			rule++;
			pcs++;
			while( *pcs && (*pcs != ',')) pcs++;
		}

		bfreeSafe(B_L,(char*)old);
		if (gCache.ipTable.actionRule == kAdd) {
			memcpy((char *)rule,(char*)&gCache.ipTable.rule,sizeof(IpTablesRules));
			gCache.ipTable.actionRule = 0;
			gCache.ipTable.chainChanged = 1;
		} else {
			if (chain->numrules != gCache.ipTable.numRules) {
				gCache.ipTable.chainChanged = 1;
			}
		}
		chain->numrules = gCache.ipTable.numRules;
	}

	if (gReq.action == kNone) {
		if (gCache.ipTable.chainChanged)
			gLedState++;
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	return expertCommit(param, request, urlQuery);
}

#if defined(KVM) || defined(ONS)
extern int KVMDConfCommit(Param* param, int request, char_t* urlQuery)
{
	Result result = kCycSuccess;

	/*printf("KVMDConfCommit()\n"); */

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kKVMD)) return mAppError(result);

	if (0 != memcmp(&gCache.kvmconf.kvmd, &gCacheSave.kvmconf.kvmd,
			sizeof(gCache.kvmconf.kvmd))) {
//		printf("gCache.kvmconf.kvmd has changed\n");
		if (gReq.action != kNone){
//			printf("gCache.kvmconf.kvmd has changed 1\n");
			RegisterParamChange(0, &gCache.kvmconf.kvmd, kKVM, kKVMDConf, 1);
		}else{
//			printf("gCache.kvmconf.kvmd has changed 2\n");
			gLedState++;
		}
	}

	if (gReq.action == kNone) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}
	result = expertCommit(param, request, urlQuery);
	return result;
}

extern int KVMuCupdateCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;
	error_vector kvm_result;
	error_vector kvm_port;
	char_t errorBuffer[80];
	int i;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen
	if (checkCancelRequest(request,kKVMuCupdate)) return mAppError(result);

	// It may be just a menu navigation. Do not commit anything,
	// but check if some action was asked
	if (gReq.action == kNone) {

		gCache.kvmuCup.sysMsg[0] = '\0';				

		if (gCache.kvmuCup.action != 99) { //otherwise it's only a page change

			if (gCache.kvmuCup.targetType ==  99) { //no Target selection
				strcpy(gCache.kvmuCup.sysMsg, "Please, select the desired Target.");
				goto end_uCupdate;
			}

			if (!gCache.kvmuCup.targetType && !gCache.kvmuCup.targetPort[0]) { //KVM Terminator && no Port
#ifdef OEM3
				strcpy(gCache.kvmuCup.sysMsg, "Please, select at least one Port for KVM Server Module.");
#else
				strcpy(gCache.kvmuCup.sysMsg, "Please, select at least one Port for KVM Terminator.");
#endif
				goto end_uCupdate;
			}

			if (gCache.kvmuCup.targetType == 4 && !gCache.kvmuCup.targetPort[0]) { //KVM Terminator && no Port
				strcpy(gCache.kvmuCup.sysMsg, "Please, select at least one Port for KVM Port Expander Module.");
				goto end_uCupdate;
			}

			UpdateKVMuC(&gCache.kvmuCup, &kvm_port, &kvm_result);

			//if (kvm_result[0] != NULL) 
			//		printf("ERROR [%s]\n", kvm_result[0]);

			if (kvm_result[0] == NULL) {
				strcpy(gCache.kvmuCup.sysMsg, "Microcontroller "
					"firmware upgrade was successful.");
			} else if (kvm_result[0] != NULL && strstr(kvm_result[0], "Download error") != NULL ) {
				//printf("Result is not NULL = [%s]\n", kvm_result[0]);
				strcpy(gCache.kvmuCup.sysMsg, kvm_result[0]);
			} else if (gCache.kvmuCup.targetType == 0 || 
				gCache.kvmuCup.targetType == 4) {
				for (i = 0; i < uCNumErrors; i++) {
					if (kvm_result[i] == NULL) {
						//printf("Result [%d] is NULL\n", i);
						break;
					}
					//printf("Result [%d] is not NULL = [%s]\n", i, kvm_result[i]);
#ifdef OEM3
					sprintf(errorBuffer, "%s, port %s: %s\\n", (gCache.kvmuCup.targetType)? "Port Expander Module" : "Server Module", kvm_port[i], kvm_result[i]);
#else
					sprintf(errorBuffer, "%s, port %s: %s\\n", (gCache.kvmuCup.targetType)? "Port Expander Module" : "Terminator", kvm_port[i], kvm_result[i]);
					//printf("ErrorBuf = [%s]\n", errorBuffer);
#endif
					strcat(gCache.kvmuCup.sysMsg, errorBuffer);
					//printf("SysMSG = [%s]\n", gCache.kvmuCup.sysMsg);
				}
			} else {
				strcpy(gCache.kvmuCup.sysMsg, kvm_result[0]);
			}

/* Other tests */
/* ----------- */
/*  				sprintf(gCache.kvmuCup.sysMsg, "BELEZA (action=%d)\\n\ */
/*  target = %d, \ */
/*  ports => %s,\\n\ */
/*  ftp server: %s,\\n\ */
/*  user: %s,\\n\ */
/*  password: %s,\\n\ */
/*  directory: %s,\\n\ */
/*  filename: %s", gCache.kvmuCup.action, */
/*  						gCache.kvmuCup.targetType, */
/*  						gCache.kvmuCup.targetPort[0] ? gCache.kvmuCup.targetPort : "no port needed", */
/*  						gCache.kvmuCup.ftpData.ftpserver, */
/*  						gCache.kvmuCup.ftpData.username, */
/*  						gCache.kvmuCup.ftpData.password, */
/*  						gCache.kvmuCup.ftpData.directory, */
/*  						gCache.kvmuCup.ftpData.filename);				 */
		}

end_uCupdate:
		ResetSidEtc(request);
		return mAppError(result);

	} else {

		return expertCommit(param, request, urlQuery);

	}
}

extern int KVMuCresetCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;
	error_vector kvm_result;
	error_vector kvm_port;
	char_t errorBuffer[80];
	int i;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen
	if (checkCancelRequest(request,kKVMuCreset)) return mAppError(result);

	// It may be just a menu navigation. Do not commit anything,
	// but check if some action was asked
	if (gReq.action == kNone) {

		gCache.kvmuCres.sysMsg[0] = '\0';				

		if (gCache.kvmuCres.action != 99) { //otherwise it's only a page change

			if (gCache.kvmuCres.targetType ==  99) { //no Target selection
				strcpy(gCache.kvmuCres.sysMsg, "Please, select the desired Target.");
				goto end_uCreset;
			}

			if (!gCache.kvmuCres.targetType && !gCache.kvmuCres.targetPort[0]) { //KVM Terminator && no Port
#ifdef OEM3
				strcpy(gCache.kvmuCres.sysMsg, "Please, select at least one Port for KVM Server Module.");
#else
				strcpy(gCache.kvmuCres.sysMsg, "Please, select at least one Port for KVM Terminator.");
#endif
				goto end_uCreset;
			}

			if (gCache.kvmuCres.targetType == 4 && !gCache.kvmuCres.targetPort[0]) { //KVM Terminator && no Port
				strcpy(gCache.kvmuCres.sysMsg, "Please, select at least one Port for KVM Port Expander Module.");
				goto end_uCreset;
			}

			ResetKVMuC(&gCache.kvmuCres, &kvm_port, &kvm_result);

			if (kvm_result[0] == NULL) {
				strcpy(gCache.kvmuCres.sysMsg, "Microcontroller "
					"reset was successful.");
			} else if (gCache.kvmuCres.targetType && gCache.kvmuCres.targetType != 4) { //Target is not KVM Terminator
				strcpy(gCache.kvmuCres.sysMsg, kvm_result[0]);
			} else {
				for (i = 0; i < uCNumErrors; i++) {
					if (kvm_result[i] == NULL) {
						break;
					}

#ifdef OEM3
					sprintf(errorBuffer, "%s port %s: %s\\n", (gCache.kvmuCres.targetType)?"Port Expander Module":"Server Module", kvm_port[i], kvm_result[i]);
#else
					sprintf(errorBuffer, "%s port %s: %s\\n", (gCache.kvmuCres.targetType)?"Port Expander Module":"Terminator", kvm_port[i], kvm_result[i]);
#endif
					strcat(gCache.kvmuCres.sysMsg, errorBuffer);
				}
			}

/* Other tests */
/* ----------- */
/*  				sprintf(gCache.kvmuCres.sysMsg, "BELEZA (action=%d)\\n\ */
/*  target = %d, \ */
/*  ports => %s", gCache.kvmuCres.action, */
/*  						gCache.kvmuCres.targetType, */
/*  						gCache.kvmuCres.targetPort[0] ? gCache.kvmuCres.targetPort : "no port needed"); */
		}

end_uCreset:
		ResetSidEtc(request);
		return mAppError(result);

	} else {

		return expertCommit(param, request, urlQuery);

	}
}

extern int KVMauxPortCommit(Param* param, int request, char_t* urlQuery)
{
   Result result;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

   // if cancel was chosen just discard the changes
   if (checkCancelRequest(request,kKVMauxPort)) return mAppError(result);

   // If it was just a menu navigation does not commit anything
   if (gReq.action == kNone) {
      SetKVMauxPort();
      SetLedChanges(request, gLedState);
      ResetSidEtc(request);
      return mAppError(result);
   }

   return expertCommit(param, request, urlQuery);
}

#endif

// COMMIT - Network -> Static Routes - table
/* ------------------------------------------------------------------------- */
/* Page "query" (Get) (utils and) handler functions (below) */
/* ie) Load functions. Loads the category and key that communicates with  */
/* backend */
/* ------------------------------------------------------------------------- */

typedef Result (*LoadFunc)(void); /* see load functions above */

Result LoadFirst(LoadFunc loadFunc, ConfigPageId page, void *orig, void *save, int len)
{
   Result result = kCycSuccess;
   /* Load page if it has not yet been cached */
   if (!gCacheLoaded[page])
   {
      result = loadFunc();
      if (kCycSuccess == result) {
         gCacheLoaded[page] = true;
       if (save != NULL)              
          memcpy(save, orig, len);
     }
   }
   return result;
}

Result GetCycUserType(int check_admin)
{
	char_t * temp;

	if (kDmfSuccess == dmfGetParam(kSidKey,NULL,&temp,-1)) {
		gReq.sid = gatoi(temp);
		bfreeSafe(B_L, temp);
	}
	return  CycCheckUser(&gReq.sid, &wUser, check_admin);
}

Result GetCycParamValues(CycParam* values, UInt32 collectionSize)
{
   // Perform the CycGet() and translate the result/error:
   return CycGet(values, collectionSize);
}

Result LoadDevice(void)
{
   CycParam values[] =
   {
      //{ kDevice, kClass, &(gCache.device.klass), 0 },
		{ kDevice, kModel, gCache.device.model, 0 },
		{ kDevice, kPicture, gCache.device.picture, 0 },
		{ kDevice, kNumber, &(gCache.device.number), 0 },
#ifdef ONS
		{ kDevice, kNumberS, &(gCache.device.number2), 0 },
		{ kDevice, kNumberSA, &(gCache.device.numberSA), 0 },
#endif
		{ kDevice, kSwVersion, gCache.device.swVersion, 0 },
		{ kDevice, kOlhVersion, gCache.device.olhVersion, 0 },
		{ kUnitInfo, kInterfacesInfo, &(gCache.unitInfo.ifInfo), 0 },
		{ kHostnameSetting, kHostName, gCache.device.hostName, 0 },
		{ kPslave, kIpAddress, gCache.device.ip, 0 },
#ifdef IPv6enable
		{ kPslave, kEthIp6,     &gCache.device.ip6,     0 },
		{ kPslave, kEthPrefix6, &gCache.device.prefix6, 0 },
#endif
#if defined(KVM) || defined(ONS)
		{ kUnitInfo, kKVMBoard, &gCache.unitInfo.kvmanalog, 0 },
#endif
	};
   return GetCycParamValues(values, mArrayLength(values));
}

Result LoadNetSettings(void) //we could have two functions,
{                            //to load only Wizard and all data (Normal)
   CycParam values[] =
   {
      { kPslave, kDhcpClient, &gCache.netSettings.dhcp, 0 },
      { kHostnameSetting, kHostName, gCache.netSettings.hostName, 0 },
      { kPslave, kIpAddress, gCache.netSettings.ipAddress1, 0 },
      { kPslave, kNetMask, gCache.netSettings.netMask1, 0 },
      { kResolvConf, kDnsService, &gCache.netSettings.dnsService, 0 },
      { kStRoutes, kIpAddress, gCache.netSettings.gateway, 0 },
      { kIssueSetting, kConsoleBanner, gCache.netSettings.consBanner, 0 },
      { kPslave, k2ndIpAddress, gCache.netSettings.ipAddress2, 0 },
      { kPslave, k2ndNetMask, gCache.netSettings.netMask2, 0 },
      { kPslave, kMtu, &gCache.netSettings.mtu, 0 },
#ifdef BONDING_FEATURE
      //bonding configuration parameters
      { kBonding, kBondingEnable, &gCache.netSettings.bond.enabled, 0},
      { kBonding, kBondingMiimon, &gCache.netSettings.bond.miimon, 0},
      { kBonding, kBondingUpdelay, &gCache.netSettings.bond.updelay, 0},
#endif
#ifdef IPv6enable
      { kPslave,   kEnableIPv4, &gCache.netSettings.enableIPv4, 0 },
      { kPslave,   kEnableIPv6, &gCache.netSettings.enableIPv6, 0 },
      { kPslave,   kIPv6method, &gCache.netSettings.IPv6method, 0 },
      { kPslave,   kEthIp6,     &gCache.netSettings.ethIp6,     0 },
      { kPslave,   kEthPrefix6, &gCache.netSettings.ethPrefix6, 0 },
      { kPslave,   kDHCPv6Opts, &gCache.netSettings.DHCPv6Opts, 0 },
      { kStRoutes, kEthIp6,     &gCache.netSettings.gateway6,   0 },
#endif
   };
   return GetCycParamValues(values, mArrayLength(values));
}

//[AP][2007-08-02] hostname discovery
void expandADStrings(void) {

   char *ptmps, tmps[2*kADStringLength+1];
   char *esc_seq[] = {"\n","\r","\t","\b","\f","\\","'",NULL};
   char *esc_txt[] = {"\\n","\\r","\\t","\\b","\\f","\\\\","\\'",NULL};
   int  i;

   for (i=0; esc_seq[i] != NULL; i++) {
      strncpy(tmps, gCache.autoDiscovery.ADAnswerString, 2*kADStringLength)[2*kADStringLength] = '\0';
   	  strcpy(gCache.autoDiscovery.ADAnswerString, "\0");
   	  ptmps = strtok(tmps,esc_seq[i]);
   	  while (ptmps != NULL) {
   	     strncat(gCache.autoDiscovery.ADAnswerString, ptmps, 2*kADStringLength)[2*kADStringLength] = '\0';
   	     if ((ptmps = strtok(NULL,esc_seq[i])) != NULL) {
   	        strncat(gCache.autoDiscovery.ADAnswerString, esc_txt[i], 2*kADStringLength)[2*kADStringLength] = '\0';
   	     }
   	  }
   }

   for (i=0; esc_seq[i] != NULL; i++) {
      strncpy(tmps, gCache.autoDiscovery.ADProbeString, 2*kADStringLength)[2*kADStringLength] = '\0';
   	  strcpy(gCache.autoDiscovery.ADProbeString, "\0");
   	  ptmps = strtok(tmps,esc_seq[i]);
   	  while (ptmps != NULL) {
   	     strncat(gCache.autoDiscovery.ADProbeString, ptmps, 2*kADStringLength)[2*kADStringLength] = '\0';
   	     if ((ptmps = strtok(NULL,esc_seq[i])) != NULL) {
   	        strncat(gCache.autoDiscovery.ADProbeString, esc_txt[i], 2*kADStringLength)[2*kADStringLength] = '\0';
   	     }
   	  }
   }

}

Result LoadAutoDiscovery(void)
{
   //char *ptmps, tmps[2*kADStringLength+1];
   //char *esc_seq[] = {"'","\n","\r","\t","\b","\f","\\", NULL};
   //char *esc_txt[] = {"\\'","\\n","\\r","\\t","\\b","\\f","\\\\", NULL};
   //char *ad_strp, *ad_str[3] = {gCache.autoDiscovery.ADProbeString, gCache.autoDiscovery.ADAnswerString, NULL};
   //int  i;
   
   CycParam values[] =
   {
      { kPslave, kADProbeStrings, gCache.autoDiscovery.ADProbeString, 0 },
      { kPslave, kADAnswerStrings, gCache.autoDiscovery.ADAnswerString, 0 },
   };

   memset(gCache.autoDiscovery.ADProbeString, 0, 2*kADStringLength+1);
   memset(gCache.autoDiscovery.ADAnswerString, 0, 2*kADStringLength+1);

   return GetCycParamValues(values, mArrayLength(values));
   //result = GetCycParamValues(values, mArrayLength(values));
   
   //expandADStrings();
   
   /*for (i=0; esc_seq[i] != NULL; i++) {
      strncpy(tmps, gCache.autoDiscovery.ADAnswerString, 2*kADStringLength)[2*kADStringLength] = '\0';
   	  strcpy(gCache.autoDiscovery.ADAnswerString, "\0");
   	  ptmps = strtok(tmps,esc_seq[i]);
   	  while (ptmps != NULL) {
   	     strncat(gCache.autoDiscovery.ADAnswerString, ptmps, 2*kADStringLength)[2*kADStringLength] = '\0';
   	     if ((ptmps = strtok(NULL,esc_seq[i])) != NULL) {
   	        strncat(gCache.autoDiscovery.ADAnswerString, esc_txt[i], 2*kADStringLength)[2*kADStringLength] = '\0';
   	     }
   	  }
   }
   for (i=0; esc_seq[i] != NULL; i++) {
      strncpy(tmps, gCache.autoDiscovery.ADProbeString, 2*kADStringLength)[2*kADStringLength] = '\0';
   	  strcpy(gCache.autoDiscovery.ADProbeString, "\0");
   	  ptmps = strtok(tmps,esc_seq[i]);
   	  while (ptmps != NULL) {
   	     strncat(gCache.autoDiscovery.ADProbeString, ptmps, 2*kADStringLength)[2*kADStringLength] = '\0';
   	     if ((ptmps = strtok(NULL,esc_seq[i])) != NULL) {
   	        strncat(gCache.autoDiscovery.ADProbeString, esc_txt[i], 2*kADStringLength)[2*kADStringLength] = '\0';
   	     }
   	  }
   }*/
   
   /*for (ad_strp=ad_str[0]; ad_strp != NULL; ad_strp++) {
      for (i=0; esc_seq[i] != NULL; i++) {
      	 strncpy(tmps, ad_strp, kADStringLength)[kADStringLength] = '\0';
   	     strcpy(ad_strp, "\0");
   	     ptmps = strtok(tmps,esc_seq[i]);
   	     while (ptmps != NULL) {
   	        strncat(ad_strp, ptmps, kADStringLength)[kADStringLength] = '\0';
   	        if ((ptmps = strtok(NULL,esc_seq[i])) != NULL) {
   	           strncat(ad_strp, esc_txt[i], kADStringLength)[kADStringLength] = '\0';
   	        }
   	     }
      }
   }*/
   
   //return result;
}

Result LoadCasProSettings(void)
{
   CycParam values[] =
   {
      { kPortsSpec, kPortBundle, allPorts, 0 },
      { kPslave, kProtocol, &(gCache.casProSettings.protocol), 0 },
      { kPslave, kBaudRate, &(gCache.casProSettings.baudRate), 0 },
      { kPslave, kBits, &(gCache.casProSettings.dataSize), 0 },
      { kPslave, kStopBits, &(gCache.casProSettings.stopBits), 0 },
      { kPslave, kParity, &(gCache.casProSettings.parity), 0 },
      { kPslave, kFlow, &(gCache.casProSettings.flowControl), 0 },
      { kPslave, kAuth, &(gCache.casProSettings.authReq), 0 },
   };
   return GetCycParamValues(values, mArrayLength(values));
}

Result LoadAccessUsers(void)
{
   /* Create the buckets the right size */
   Result result = kCycSuccess;
   static CycParam counts[] = {
		{kPasswd, kNumEntries, &(gCache.access.numUsers)},
   };

   if (gCacheLoaded[kAccessUsers]) return(result);

   if (!strcmp(wUser.uName, "root")) {
		counts[0].flag = 1;
   } else {
		counts[0].flag = 0;
   }

   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
   bfreeSafe(B_L, gCache.access.users);
   if (0 < gCache.access.numUsers)
   {
      gCache.access.users = balloc(B_L, sizeof(Users) * (gCache.access.numUsers));
	  if (!gCache.access.users) return kMemAllocationFailure; //mp
      {
         CycParam values[] = {
        	{kPasswd, kUsers, gCache.access.users, gCache.access.numUsers,0,0},
         };
	     if (!strcmp(wUser.uName, "root")) {
		   values[0].flag = 1;
	     } else {
		   values[0].flag = 0;
	     }
         result = GetCycParamValues(values, mArrayLength(values));
      }
   }
   return result;
}

Result LoadAccessGroups(void)
{
   /* Create the buckets the right size */
   Result result = kCycSuccess;
   static CycParam counts[] =
   {
#if defined (KVM) || defined (ONS)
      { kKVMGroup, kNumEntries, &(gCache.access.numGroups), 0 },
#else
      { kPslave, kNumConfGroups, &(gCache.access.numGroups), 0 },
#endif
   };

   if (gCacheLoaded[kAccessGroups]) return(result);

   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
   bfreeSafe(B_L, gCache.access.groups);
   if (0 < gCache.access.numGroups)
   {
      gCache.access.groups = balloc(B_L, sizeof(ConfGroups) * (gCache.access.numGroups));
	  if (!gCache.access.groups) return kMemAllocationFailure; //mp
      {
         CycParam values[] =
         {
#if defined (KVM) || defined (ONS)
            { kKVMGroup, kKVMGroups, gCache.access.groups, gCache.access.numGroups },
#else
            { kPslave, kConfGroup, gCache.access.groups, gCache.access.numGroups },
#endif
         };
         result = GetCycParamValues(values, mArrayLength(values));
      }
   }
   return result;
}

Result LoadDataBuffering(void)
{
   CycParam values[] =
   {
      { kPortsSpec, kPortBundle, allPorts, 0 },
      { kPslave, kDbMode, &(gCache.dataBuffering.mode), 0 },
      { kPslave, kDbFileSize, &(gCache.dataBuffering.fileSize), 0 },
      { kPslave, kNfsFilePath, gCache.dataBuffering.nfsPath, 0 },
      { kPslave, kShowMenu, &(gCache.dataBuffering.showMenu), 0 },
      { kPslave, kTimeStamp, &(gCache.dataBuffering.timeStamp), 0 },
   };
   return GetCycParamValues(values, mArrayLength(values));
}

Result LoadSystemLog(void)
{
   Result result = kCycSuccess;
   CycParam counts[] =
   {
      { kPslave, kConfFacilityNum, &(gCache.systemLog.facilityNumber), 0 },
      { kSyslogNg, kNumEntries, &(gCache.systemLog.numServers), 0 },
      { kSyslogNg, kLogDestEnable, &(gCache.systemLog.dest), 0 },
      { kSyslogNg, kLogFilterlevel, &(gCache.systemLog.filter), 0 },
   };
   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
   bfreeSafe(B_L, gCache.systemLog.servers);
   if (gCache.systemLog.numServers)
   {
      gCache.systemLog.servers = balloc(B_L, sizeof(SyslogServerInfo) * (gCache.systemLog.numServers));
	  if (!gCache.systemLog.servers) return kMemAllocationFailure; //mp
      {
         CycParam values[] =
         {
            { kSyslogNg, kSyslogServer, gCache.systemLog.servers, gCache.systemLog.numServers },
         };
         result = GetCycParamValues(values, mArrayLength(values));
      }
   }

#if defined(KVM) || defined(ONS)
   {
       Result ret;
       ret = LoadFirst(LoadKVMDConf, kKVMD, &gCache.kvmconf.kvmd, &gCacheSave.kvmconf.kvmd, sizeof(gCache.kvmconf.kvmd));
       if (ret == kCycSuccess) {
           gCache.systemLog.KVMfacility = gCache.kvmconf.kvmd.general.facility;
           gCacheSave.systemLog.KVMfacility = gCache.systemLog.KVMfacility;
       } else {
           return ret;
       }
   }
#endif

   return result;
}

#ifdef PCMCIA
Result LoadUnitPcmciaInfo(void)
{
   Result result = kCycSuccess;

   /*static CycParam counts[] =
   {
      { kPcmcia, kNumPcmciaSlots, &(gCache.unitInfo.numPcmcia), 0 },
   };

   GetCycParamValues(counts, mArrayLength(counts)); 
	*/
	// Always 2 slots
	gCache.unitInfo.numPcmcia = kMaxPcmciaSlots;
   bfreeSafe(B_L, gCache.unitInfo.pcmcia);
   if (gCache.unitInfo.numPcmcia)
   {
	   if (!(gCache.unitInfo.pcmcia = balloc(B_L, sizeof(PcmciaInfo) * (gCache.unitInfo.numPcmcia))))
		   return kMemAllocationFailure; //mp
	   memset(gCache.unitInfo.pcmcia, 0, sizeof(PcmciaInfo) * (gCache.unitInfo.numPcmcia));
	   {
		   CycParam values[] =
		   {
			   { kPcmcia, kUnitPcmciaInfo, gCache.unitInfo.pcmcia, gCache.unitInfo.numPcmcia },
		   };
		   result = GetCycParamValues(values, mArrayLength(values));
		   memcpy(&gCacheSave.unitInfo, &gCache.unitInfo, sizeof(UnitInfo));
		   if (!(gCacheSave.unitInfo.pcmcia = 
				 balloc(B_L, sizeof(PcmciaInfo) * (gCache.unitInfo.numPcmcia))))
			   return kMemAllocationFailure; //mp
		   memcpy(gCacheSave.unitInfo.pcmcia, gCache.unitInfo.pcmcia, sizeof(PcmciaInfo) * (gCache.unitInfo.numPcmcia));
	   }
   }
   return result;
}
#endif

Result LoadUnitRamDiskInfo(void)
{
  Result result = kCycSuccess;
   int size;

   static CycParam counts[] =
   {
      { kUnitInfo, kNumEntries, &(gCache.unitInfo.numRamDisk), 0 },
   };

//   if (gCacheLoaded[kUnitRam]) return(result);

   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
   bfreeSafe(B_L, gCache.unitInfo.ram);
   if (gCache.unitInfo.numRamDisk)
   {
	   size=sizeof(RamDiskInfo) * (gCache.unitInfo.numRamDisk);
	   if (!(gCache.unitInfo.ram = balloc(B_L, size)))
		   return kMemAllocationFailure; //mp
       memset(gCache.unitInfo.ram, 0, size);
	   {
		   CycParam values[] =
		   {
			   { kUnitInfo, kRamDiskInfo, gCache.unitInfo.ram, gCache.unitInfo.numRamDisk },
		   };
		   result = GetCycParamValues(values, mArrayLength(values));
	   }
   }
   return result;
}

Result LoadUnitInfo(void)
{
   CycParam values[] =
   {
      { kUnitInfo, kUnitVersionInfo, &(gCache.unitInfo.version), 1 },
      { kUnitInfo, kSerialNumber, gCache.unitInfo.serialNumber, 0 },
      { kUnitInfo, kDate, gCache.unitInfo.date, 0 },
      { kUnitInfo, kUpTime, gCache.unitInfo.upTime, 0 },
      { kUnitInfo, kPwrSupplyInfo, &(gCache.unitInfo.pwrSupply), 0 },
      { kUnitInfo, kCpuInfo, &(gCache.unitInfo.cpuInfo), 0 },
      { kUnitInfo, kMemInfo, &(gCache.unitInfo.memory), 0 },
      { kUnitInfo, kMacAddress, gCache.unitInfo.mac_address, 0 },
#if defined(KVM) || defined(ONS)
      { kUnitInfo, kFanStatus, &(gCache.unitInfo.fanStatus), 0 },
#endif
   };
   return GetCycParamValues(values, mArrayLength(values));
}

#if defined(KVM) || defined(ONS)
Result LoadKVMPortStatus(void)
{
	int i;
	const int num_stations = libcyc_kvm_get_num_stations(cyc);

	/* Add enough for the IP modules plus 2 local stations */
	for (i=0; i < num_stations ; i++) {
		bfreeSafe (B_L, gCache.unitInfo.kvmPortStatus[i]);
		gCache.unitInfo.kvmPortStatus[i] = balloc(B_L, kKVMPortInfoLength);
		if (! gCache.unitInfo.kvmPortStatus[i]) {
			return kMemAllocationFailure;
		}
	}
	gCache.unitInfo.kvmPortStatus[i] = 0;
	
	{
   		CycParam values[] =
   		{
      			{ kUnitInfo, kKVMPortStatus, &(gCache.unitInfo.kvmPortStatus), 0 },
   		};
		return GetCycParamValues(values, mArrayLength(values));
	}
}

#if 0
Result LoadKVMConf(void)
{
	kvmreadcfg();
	memcpy(&(gCache.kvmConf), &kvmconf, sizeof(kvmconf));
	return kCycSuccess;
}
#endif

Result LoadKVMDConf(void)
{
	CycParam values[] =
	{
		{ kKVM, kKVMDConf, &(gCache.kvmconf.kvmd), 0 },
	};
	return GetCycParamValues(values, mArrayLength(values));
}

Result LoadKVMuCupdate(void)
{
	memset(&(gCache.kvmuCup), 0, sizeof(KVMMicrocode));
	return kCycSuccess;
}

Result LoadKVMuCreset(void)
{
	memset(&(gCache.kvmuCres), 0, sizeof(KVMMicrocode));
	return kCycSuccess;
}

Result LoadKVMauxPort(void)
{
	CycParam values[] =
	{
		{ kKVM, kKVMAuxPort, &gCache.kvmAuxPort[0], 0 },
	};
	return GetCycParamValues(values, mArrayLength(values));
}
#endif // KVM

Result LoadBootInfo(void)
{
   CycParam values[] =
   {
      { kBootManagement, kBootConf, &(gCache.bootInfo), 1 },
   };
   return GetCycParamValues(values, mArrayLength(values));
}

// Notifications
Result LoadNotifications(void)
{
   int size;
   Result result = kCycSuccess;
   CycParam counts[] =
   {
      { kSyslogNg, kNumNotificationEntries, &(gCache.notifications.numNotif), 0 },
   };
   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
   bfreeSafe(B_L, gCache.notifications.notif);
   {
      CycParam values[] =
      {
         { kPortsSpec, kPortBundle, allPorts, 0 },
         { kPslave, kAlarm, &(gCache.notifications.alarm), 0 },
      };
      result = GetCycParamValues(values, mArrayLength(values));
   }
   if (gCache.notifications.numNotif) {
      size=sizeof(NotificationInfo) * (gCache.notifications.numNotif);
      if (!(gCache.notifications.notif = balloc(B_L, size)))
		  return kMemAllocationFailure; //mp
      memset(gCache.notifications.notif,0,size);
      {
         CycParam values[] =
         {
            { kSyslogNg, kNotificationInfo, gCache.notifications.notif, gCache.notifications.numNotif },
         };
         result = GetCycParamValues(values, mArrayLength(values));
      }
   }
   return result;
}

#ifdef PMD
extern int GetNumIpdus(Param* param, int request)
{
        Result result;
        CycParam counts[] =
        {
                { kPowerMgm, kNumPorts, &(gCache.numIpdus), 0 },
        };

        if ((result = GetCycUserType(0)) != kCycSuccess)
                return mAppError(result);

        result = GetCycParamValues(counts, mArrayLength(counts));
        return result;
}

// Power Management - New Outlets Manager (level 1)
Result LoadIpduOutMan(void)
{
	Result result;
	int size, i,count;
	PmIpduPortInfo  *port = gCache.IpduOutMan.portInfo;
	PmIpduInfo      *ipdu;
	CycParam values[] =
		{
			{ kPowerMgm, kNumPorts, &gCache.IpduOutMan.numPorts, 0 },
		};

	//free previously allocated memory
	while (gCache.IpduOutMan.numPorts) {
		ipdu = port->ipdu;
		while (port->numIpdu) {
			if (ipdu->firstPhaseInfo) free(ipdu->firstPhaseInfo);
			if (ipdu->firstBankInfo) free(ipdu->firstBankInfo);
			if (ipdu->firstEnvMonSensor) free(ipdu->firstEnvMonSensor);
			ipdu++;
			port->numIpdu--;
		}
		bfreeSafe(B_L, port->ipdu);
		port++;
		gCache.IpduOutMan.numPorts--;
	}
	bfreeSafe(B_L, gCache.IpduOutMan.portInfo);
	gCache.IpduOutMan.portInfo = NULL;
	gCache.IpduOutMan.numIpdus = 0;
#if (defined(PCMCIA) || defined(TARGET_ACS5K))
	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;
#else
#endif

	if (gCache.IpduOutMan.numPorts) {
		size = sizeof(PmIpduPortInfo) * gCache.IpduOutMan.numPorts;
		if (!(gCache.IpduOutMan.portInfo = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(gCache.IpduOutMan.portInfo,0,size);
		{
			CycParam values[] =
				{
					{ kPowerMgm, kPortInfo, gCache.IpduOutMan.portInfo, gCache.IpduOutMan.numPorts },
				};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}

		port = gCache.IpduOutMan.portInfo;
		for (i=0; i<gCache.IpduOutMan.numPorts; i++, port++) {
//syslog(4,"%s: i[%d] numports[%d] numipdus[%d]",__func__,i,gCache.IpduOutMan.numPorts,port->numIpdu);
			if (port->numIpdu) {
				size = sizeof(PmIpduInfo) * port->numIpdu;
				if (!(port->ipdu = balloc(B_L, size)))
					return kMemAllocationFailure; //mp
				memset(port->ipdu,0,size);
				{
					CycParam values[] =
						{
							{ kPowerMgm, kIpduUnitInfo, port->ipdu, port->numIpdu},
						};
					port->ipdu->serialPort = port->serialPort;
					port->ipdu->ipduNumber = 0; //All

					strcpy(port->ipdu->username, PMDngUserName);
					if ((result = GetCycParamValues(values, mArrayLength(values)))
						!= kCycSuccess) return result;
				}
				for (count=0; (count < port->numIpdu) && port->ipdu[count].ipduNumber; count++);
				gCache.IpduOutMan.numIpdus += count;
			}
		}
	}

	return result;
}

// Power Management - New Outlets Manager (level 2)
Result LoadIpduOutMan2nd(int portindex, int ipduindex)
{
	Result result;
	PmIpduInfo *ipdu = &(gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex]);
	CycParam values[] =
		{
			{ kPowerMgm, kOutletInfo, &(gCache.IpduOutMan.ipduOutInfo), 0 },
		};

//	printf("LoadIpduOutMan2nd\n");
	//free previously allocated memory
	if (gCache.IpduOutMan.ipduOutInfo.outletInfo)
		free(gCache.IpduOutMan.ipduOutInfo.outletInfo);
	gCache.IpduOutMan.ipduOutInfo.outletInfo = NULL;

	strcpy(gCache.IpduOutMan.ipduOutInfo.username, PMDngUserName);
	gCache.IpduOutMan.ipduOutInfo.ipduRef = ipdu->shmRef;

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

//	syslog(4,"%s: <<< %d >>> outlets\n", __func__,gCache.IpduOutMan.ipduOutInfo.numOutlets);

	return result;
}

// Power Management - New Outlet Groups Ctrl page
Result LoadIpduOutGrpCtrl(void)
{
	Result result;
	int numPorts=0, size, i;
	PmIpduPortInfo    *port;
	PmIpduOutGrpInfo  *group = gCache.IpduOutGrpCtrl.groupInfo;
	PMDOutGrpsDef     *grpdef;
	CycParam values[] =
		{
			{ kPowerMgm, kNumPorts, &numPorts, 0 },
			{ kPMDConfig, kPMDConfnumOutGrps, &gCache.IpduOutGrpCtrl.totalGroups, 0 },
		};

	//free previously allocated memory
	if (group != NULL) {
		while (gCache.IpduOutGrpCtrl.totalGroups) {
			free(group->outletInfo);
			group++;
			gCache.IpduOutGrpCtrl.totalGroups--;
		}
		bfreeSafe(B_L, gCache.IpduOutGrpCtrl.groupInfo);
	}
	if ( gCache.IpduOutGrpCtrl.groupDefs != NULL)
		bfreeSafe(B_L, gCache.IpduOutGrpCtrl.groupDefs);
	gCache.IpduOutGrpCtrl.groupInfo = 0;
	gCache.IpduOutGrpCtrl.groupDefs = 0;
	gCache.IpduOutGrpCtrl.numIpdus = 0;
	gCache.IpduOutGrpCtrl.numGroups = 0;

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	if (numPorts) {
		size = sizeof(PmIpduPortInfo) * numPorts;
		if (!(port = malloc(size)))
			return kMemAllocationFailure; //mp
		memset(port,0,size);
		{
			CycParam values[] =
				{
					{ kPowerMgm, kPortInfo, port, numPorts },
				};

			result = GetCycParamValues(values, mArrayLength(values));
			if (result != kCycSuccess) {
				free(port);
				return result;
			}
		}

		for (i = 0; i < numPorts; i++)
			gCache.IpduOutGrpCtrl.numIpdus += port[i].numIpdu;
		free(port);

		if (gCache.IpduOutGrpCtrl.totalGroups) {
			size = sizeof(PmIpduOutGrpInfo) * gCache.IpduOutGrpCtrl.totalGroups;
			if (!(gCache.IpduOutGrpCtrl.groupInfo = balloc(B_L, size)))
				return kMemAllocationFailure; //mp
			memset(gCache.IpduOutGrpCtrl.groupInfo,0,size);
			size = sizeof(PMDOutGrpsDef) * gCache.IpduOutGrpCtrl.totalGroups;
			if (!(gCache.IpduOutGrpCtrl.groupDefs = balloc(B_L, size)))
				return kMemAllocationFailure; //mp
			memset(gCache.IpduOutGrpCtrl.groupDefs,0,size);
			{
				CycParam values[] =
					{
						{ kPMDConfig, kPMDOutGrpsDef, gCache.IpduOutGrpCtrl.groupDefs, gCache.IpduOutGrpCtrl.totalGroups },
					};

				if ((result = GetCycParamValues(values, mArrayLength(values)))
					!= kCycSuccess) return result;
			}
			{
				CycParam values[] =
					{
						{ kPowerMgm, kOutGrpsInfo, gCache.IpduOutGrpCtrl.groupInfo, gCache.IpduOutGrpCtrl.totalGroups },
					};

				strcpy(gCache.IpduOutGrpCtrl.groupInfo->username, PMDngUserName);
				group = gCache.IpduOutGrpCtrl.groupInfo;
				grpdef = gCache.IpduOutGrpCtrl.groupDefs;
				for (i=0; i<gCache.IpduOutGrpCtrl.totalGroups; i++, group++, grpdef++) {
					strncpy(group->name+1, grpdef->group, kPMDGroupNameLength);
					group->name[0]='$';
				}

				if ((result = GetCycParamValues(values, mArrayLength(values)))
					!= kCycSuccess) return result;
			}

			group = gCache.IpduOutGrpCtrl.groupInfo;
			for (i=0; (i<gCache.IpduOutGrpCtrl.totalGroups); i++, group++)
				if (group->numOutlets)
					gCache.IpduOutGrpCtrl.numGroups++;
		}
	}

	return result;
}

// Power Management - New View IPDUs Info & Configuration pages
Result LoadViewConfIpdu(void)
{
	Result result;
	int size, i;
	PmIpduPortInfo  *port = gCache.ViewConfIpdu.portInfo;
	PmIpduInfo      *ipdu;
	CycParam values[] =
		{
			{ kPowerMgm, kNumPorts, &gCache.ViewConfIpdu.numPorts, 0 },
		};
	//free previously allocated memory
	while (gCache.ViewConfIpdu.numPorts) {
		ipdu = port->ipdu;
		while (port->numIpdu) {
			if (ipdu->firstPhaseInfo) free(ipdu->firstPhaseInfo);
			if (ipdu->firstBankInfo) free(ipdu->firstBankInfo);
			if (ipdu->firstEnvMonSensor) free(ipdu->firstEnvMonSensor);
			ipdu++;
			port->numIpdu--;
		}
		bfreeSafe(B_L, port->ipdu);
		port++;
		gCache.ViewConfIpdu.numPorts--;
	}
	bfreeSafe(B_L, gCache.ViewConfIpdu.portInfo);
	gCache.ViewConfIpdu.portInfo = NULL;
	gCache.ViewConfIpdu.numIpdus = 0;

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	if (gCache.ViewConfIpdu.numPorts) {
		size = sizeof(PmIpduPortInfo) * gCache.ViewConfIpdu.numPorts;
		if (!(gCache.ViewConfIpdu.portInfo = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(gCache.ViewConfIpdu.portInfo,0,size);
		{
			CycParam values[] =
				{
					{ kPowerMgm, kPortInfo, gCache.ViewConfIpdu.portInfo, gCache.ViewConfIpdu.numPorts },
				};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}

		port = gCache.ViewConfIpdu.portInfo;
		for (i=0; i<gCache.ViewConfIpdu.numPorts; i++, port++) {

			if ((port->serialPort == 0)||(port->numIpdu == 0)) {
				port->numIpdu = 0;
				port->ipdu = NULL;
				continue;
			}

			gCache.ViewConfIpdu.numIpdus += port->numIpdu;

			size = sizeof(PmIpduInfo) * port->numIpdu;
			if (!(port->ipdu = balloc(B_L, size)))
				return kMemAllocationFailure; //mp
			memset(port->ipdu,0,size);
			{
				CycParam values[] =
					{
						{ kPowerMgm, kIpduUnitInfo, port->ipdu, port->numIpdu},
						{ kPMDConfig, kPMDIpduInfo, port->ipdu, port->numIpdu},
					};
				port->ipdu->serialPort = port->serialPort;
				port->ipdu->ipduNumber = 0; //All
				strcpy(port->ipdu->username, PMDngUserName);

				if ((result = GetCycParamValues(values, mArrayLength(values)))
					!= kCycSuccess) return result;
			}
		}
	}

	return result;
}

// Power Management - New Software Upgrade page
Result LoadIpduSwUpgrade(void)
{
	Result result;
	int size, i;
	PmIpduPortInfo  *port = gCache.IpduSwUpgrade.portInfo;
	PmIpduInfo      *ipdu;
	CycParam values[] =
		{
			{ kPowerMgm, kNumPorts, &gCache.IpduSwUpgrade.numPorts, 0 },
			{ kPowerMgm, kFirmUpgradeVersion, gCache.IpduSwUpgrade.latest_sw, 0 },
		};

	//free previously allocated memory
	while (gCache.IpduSwUpgrade.numPorts) {
		ipdu = port->ipdu;
		while (port->numIpdu) {
			if (ipdu->firstPhaseInfo) free(ipdu->firstPhaseInfo);
			if (ipdu->firstBankInfo) free(ipdu->firstBankInfo);
			if (ipdu->firstEnvMonSensor) free(ipdu->firstEnvMonSensor);
			ipdu++;
			port->numIpdu--;
		}
		bfreeSafe(B_L, port->ipdu);
		port++;
		gCache.IpduSwUpgrade.numPorts--;
	}
	bfreeSafe(B_L, gCache.IpduSwUpgrade.portInfo);
	gCache.IpduSwUpgrade.portInfo = NULL;
	gCache.IpduSwUpgrade.numIpdus = 0;

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	if (gCache.IpduSwUpgrade.numPorts) {
		size = sizeof(PmIpduPortInfo) * gCache.IpduSwUpgrade.numPorts;
		if (!(gCache.IpduSwUpgrade.portInfo = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(gCache.IpduSwUpgrade.portInfo,0,size);
		{
			CycParam values[] =
				{
					{ kPowerMgm, kPortInfo, gCache.IpduSwUpgrade.portInfo, gCache.IpduSwUpgrade.numPorts },
				};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}

		port = gCache.IpduSwUpgrade.portInfo;
		for (i=0; i<gCache.IpduSwUpgrade.numPorts; i++, port++) {
			if ((port->serialPort == 0)||(port->numIpdu == 0)) {
				continue;
			}
			gCache.IpduSwUpgrade.numIpdus += port->numIpdu;

			size = sizeof(PmIpduInfo) * port->numIpdu;
			if (!(port->ipdu = balloc(B_L, size)))
				return kMemAllocationFailure; //mp
			memset(port->ipdu,0,size);
			{
				CycParam values[] =
					{
						{ kPowerMgm, kIpduUnitInfo, port->ipdu, port->numIpdu},
					};
				port->ipdu->serialPort = port->serialPort;
				port->ipdu->ipduNumber = 0; //All

				if ((result = GetCycParamValues(values, mArrayLength(values)))
					!= kCycSuccess) return result;
			}
		}
	}

	return result;
}

// Power Management - New PMD Config, General tab
Result LoadpmdGen(void)
{
	Result result;
	int i;
	int vendors=3;
	PMDGeneralInfo info[vendors];
	CycParam values[] =
		{
			{ kPMDConfig, kPMDGeneralInfo, info, vendors },
		};

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	memset(gCache.pmdGen.genInfo,0,vendors*sizeof(PMDGeneralInfo));
	//because we want it correctly initialized for gCacheSave...
	gCache.pmdGen.genInfo[0].action = kDelete;
	gCache.pmdGen.genInfo[1].action = kDelete;
	gCache.pmdGen.genInfo[2].action = kDelete;
	//put entries in the correct order (Cyclades, SPC and ServerTech)
	for (i=0; i<vendors; i++) {
		switch (info[i].vendor) {
		case Vendor_cyclades:
			gCache.pmdGen.genInfo[0] = info[i];
			gCache.pmdGen.genInfo[0].action = kActionIgnore;
			break;
		case Vendor_spc:
			gCache.pmdGen.genInfo[1] = info[i];
			gCache.pmdGen.genInfo[1].action = kActionIgnore;
			break;
		case Vendor_servertech:
			gCache.pmdGen.genInfo[2] = info[i];
			gCache.pmdGen.genInfo[2].action = kActionIgnore;
			break;
		default:
			break;
		}
	}
	//because we may have "empty positions"...
	gCache.pmdGen.genInfo[0].vendor = Vendor_cyclades;
	gCache.pmdGen.genInfo[1].vendor = Vendor_spc;
	gCache.pmdGen.genInfo[2].vendor = Vendor_servertech;

	return result;
}

// Power Management - New PMD Config, Ports Conf
Result LoadpmdPortInfo(void)
{
	Result result;
	CycParam values[] = {
		{ kPMDConfig, kPMDConfnumPorts, &gCache.pmdConf.numports, 0 },
	};

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	if (gCache.pmdConf.ports) {
		free(gCache.pmdConf.ports);
		gCache.pmdConf.ports = NULL;
	}
	
	if (gCache.pmdConf.numports) {
		gCache.pmdConf.ports = (PMDPortInfo *)calloc(1,sizeof(PMDPortInfo)*gCache.pmdConf.numports);
		{
			CycParam values[] =				{
				{ kPMDConfig, kPMDPortInfo, gCache.pmdConf.ports, gCache.pmdConf.numports },
			};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}
	}

	return result;
}

// Power Management - New PMD Config, Ports Conf
Result LoadpmdGroups(void)
{
	Result result;
	CycParam values[] = {
		{ kPMDConfig, kPMDConfnumOutGrps, &gCache.pmdConf.numgrps, 0 },
	};

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	if (gCache.pmdConf.grps) {
		free(gCache.pmdConf.grps);
		gCache.pmdConf.grps = NULL;
	}

	if (gCache.pmdConf.numgrps) {
		gCache.pmdConf.grps = (PMDOutGrpsDef *)calloc(1,sizeof(PMDOutGrpsDef)*gCache.pmdConf.numgrps);

		{
		CycParam values[] ={
			{ kPMDConfig, kPMDOutGrpsDef, gCache.pmdConf.grps, gCache.pmdConf.numgrps },
		};
		if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}
	}

	return result;
}

// Power Management - New PMD Config, Ports Conf
Result LoadpmdUsers(void)
{
	Result result;
	CycParam values[] = {
		{ kPMDConfig, kPMDConfnumUsers, &gCache.pmdConf.numusers, 0 },
	};

	if (gCache.pmdConf.users) {
		free(gCache.pmdConf.users);
		gCache.pmdConf.numusers = 0;
	}

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;


	if (gCache.pmdConf.numusers) {
		gCache.pmdConf.users = (PMDUsersMgm *)calloc(1,sizeof(PMDUsersMgm)*gCache.pmdConf.numusers);

		{
		CycParam values[] ={
			{ kPMDConfig, kPMDUsersMgm, gCache.pmdConf.users, gCache.pmdConf.numusers },
		};
		if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}
	}

	return result;
}

// Power Management - New PMD Config, Outlet Groups tab
Result LoadpmdOutGrp(void)
{
	Result result;
	CycParam values[] =
		{
			{ kPMDConfig, kPMDConfnumOutGrps, &gCache.pmdOutGrp.numGroups, 0 },
		};

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	if (gCache.pmdOutGrp.numGroups) {
		memset(gCache.pmdOutGrp.groups,0,sizeof(PMDOutGrpsDef)*kPmMaxGroups);
		{
			CycParam values[] =
				{
					{ kPMDConfig, kPMDOutGrpsDef, gCache.pmdOutGrp.groups, gCache.pmdOutGrp.numGroups },
				};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}
	}

	gCache.pmdOutGrp.changed = 0;

	return result;
}

// Power Management - New PMD Config, Users Management tab
Result LoadpmdUserMan(void)
{
	Result result;
	CycParam values[] =
		{
			{ kPMDConfig, kPMDConfnumUsers, &gCache.pmdUserMan.numUsers, 0 },
		};

	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	if (gCache.pmdUserMan.numUsers) {
		memset(gCache.pmdUserMan.usersMgm,0,sizeof(PMDUsersMgm)*kPmMaxUsers);
		{
			CycParam values[] =
				{
					{ kPMDConfig, kPMDUsersMgm, gCache.pmdUserMan.usersMgm, gCache.pmdUserMan.numUsers },
				};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}
	}

	gCache.pmdUserMan.changed = 0;

	return result;
}
#endif

#ifdef IPMI
// Power Management - IPMI
Result LoadPmIpmi(void)
{
	int size;
	Result result;

	CycParam counts[] =
	{
		{ kIPMIDevice, kNumEntries, &(gCache.pmipmi.numIpmiDev), 0 },
	};

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	if ((result = GetCycParamValues(counts, mArrayLength(counts)))
		!= kCycSuccess) return result;

	gCache.pmipmi.numActiveIpmiDev = gCache.pmipmi.numIpmiDev;

	bfreeSafe(B_L, gCache.pmipmi.ipmiInfo);
	if (gCache.pmipmi.numIpmiDev) {
		size = sizeof(IPMIDeviceInfo) * gCache.pmipmi.numIpmiDev;
		if (!(gCache.pmipmi.ipmiInfo = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(gCache.pmipmi.ipmiInfo,0,size);
		{
			CycParam values[] =
			{
				{ kIPMIDevice, kIPMIDeviceInfo, gCache.pmipmi.ipmiInfo, gCache.pmipmi.numIpmiDev },
			};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}

	}

	gCache.pmipmi.action = no_Action;

	return result;
}
#endif

// NETWORK
// SNMP Daemon Settings
Result LoadSNMPdaemon(void)
{
	int size;
	Result result;

	CycParam values[] =
	{
      { kSnmpdSettings, kSysContact, gCache.SNMPdaemon.sysContact, 0 },
      { kSnmpdSettings, kSysLocation, gCache.SNMPdaemon.sysLocation, 0 },
	};
	if ((result = GetCycParamValues(values, mArrayLength(values)))
		!= kCycSuccess) return result;

	memcpy(gCacheSave.SNMPdaemon.sysContact, gCache.SNMPdaemon.sysContact, kInfoLength);
	memcpy(gCacheSave.SNMPdaemon.sysLocation, gCache.SNMPdaemon.sysLocation, kInfoLength);

	{ // SNMPv12 section
		CycParam counts[] =
		{
			{ kSnmpdSettings, kNumCommunityEntries, &(gCache.SNMPdaemon.numSNMPv12), 0 },
		};
		if ((result = GetCycParamValues(counts, mArrayLength(counts))) /* fetch SNMPv12 size */
			!= kCycSuccess) return result;
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv12p);
		if (gCache.SNMPdaemon.numSNMPv12) {
			size = sizeof(CommunityInfo) * gCache.SNMPdaemon.numSNMPv12;
			if (!(gCache.SNMPdaemon.SNMPv12p = balloc(B_L, size)))
				return kMemAllocationFailure; //mp
			memset(gCache.SNMPdaemon.SNMPv12p,0,size);
			{
				CycParam values[] =
				{
					{ kSnmpdSettings, kCommunityInfo, gCache.SNMPdaemon.SNMPv12p, gCache.SNMPdaemon.numSNMPv12 },
				};
				if ((result = GetCycParamValues(values, mArrayLength(values)))
					!= kCycSuccess) return result;
			}
		}
		gCache.SNMPdaemon.SNMPv12Changed = FALSE;
	}

	{ // SNMPv3 section
        CycParam counts[] =
		{
			{ kSnmpdSettings, kNumSnmpv3Entries, &(gCache.SNMPdaemon.numSNMPv3), 0 },
		};
		if ((result = GetCycParamValues(counts, mArrayLength(counts))) /* fetch SNMPv3 size */
			!= kCycSuccess) return result;
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv3p);
		if (gCache.SNMPdaemon.numSNMPv3) {
			size = sizeof(Snmpv3Info) * gCache.SNMPdaemon.numSNMPv3;
            if (!(gCache.SNMPdaemon.SNMPv3p = balloc(B_L, size)))
				return kMemAllocationFailure; //mp
			memset(gCache.SNMPdaemon.SNMPv3p,0,size);
			{
				CycParam values[] =
				{
					{ kSnmpdSettings, kSnmpv3Info, gCache.SNMPdaemon.SNMPv3p, gCache.SNMPdaemon.numSNMPv3 },
				};
				if ((result = GetCycParamValues(values, mArrayLength(values)))
					!= kCycSuccess) return result;
			}
		}
		gCache.SNMPdaemon.SNMPv3Changed = FALSE;
	}

	return result;
}

/* [LMT] Security Enhancements */
static HttpOptions localSecWebUI;

// Security Profile
Result LoadSecProfile(void)
{
        struct stat statbuf;
        Result result;
        CycParam values[] =
        {
                { kInetd, kTelnetEnable, &(gCache.secProfile.telnet), 0},
                { kSshd, kSshdOpt, &(gCache.secProfile.ssh), 1}, //struct
                { kWebUI, kWebUIOpt, &(gCache.secProfile.webUI), 1}, //struct
                { kSnmpdSettings, kSnmpdEnable, &(gCache.secProfile.snmp), 0},
                { kPortMap, kRpcEnable, &(gCache.secProfile.rpc), 0},
#ifdef KRBenable
                { kInetd, kFtpEnable, &(gCache.secProfile.ftp), 0},
#endif
#ifdef ONS
                { kInetd, kTftpEnable, &(gCache.secProfile.tftp), 0},
#endif
#ifdef IPSEC
                { kIpSec, kIpSecEnable, &(gCache.secProfile.ipsec), 0},
#endif
                { kSecProfiles, kIcmpEnable, &(gCache.secProfile.icmp), 0},
                { kSecProfiles, kSecOpt, &(gCache.secProfile.options), 1}, //struct
        };

        if (stat("/etc/security.opts", &statbuf))
                gCache.secProfile.showWarning = kSecAdvisory;
        else
                gCache.secProfile.showWarning = kNoSecWarning;

        result = GetCycParamValues(values, mArrayLength(values));

        localSecWebUI = gCache.secProfile.webUI;
        WebNeedsRestart = FALSE;

	return result;
}

// Host Table
Result LoadHostTable(void)
{
   int size;
   Result result = kCycSuccess;
   CycParam counts[] =
   {
      { kHostTable, kNumEntries, &(gCache.hostTable.numHosts), 0 },
   };
   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
   bfreeSafe(B_L, gCache.hostTable.hosts);
   if (gCache.hostTable.numHosts) {
      size=sizeof(Hosts) * (gCache.hostTable.numHosts);
      if (!(gCache.hostTable.hosts = balloc(B_L, size)))
		  return kMemAllocationFailure; //mp
      memset(gCache.hostTable.hosts,0,size);
      {
         CycParam values[] =
         {
            { kHostTable, kHosts, gCache.hostTable.hosts, gCache.hostTable.numHosts},
         };
         result = GetCycParamValues(values, mArrayLength(values));
      }
   }
   return result;
}

// Static Routes
Result LoadStaticRoutes(void)
{
	int size;
	Result result;
	CycParam counts[] =
	{
		{ kStRoutes, kNumEntries, &(gCache.staticRoutes.numRoutes), 0 },
	};

	if ((result = GetCycParamValues(counts, mArrayLength(counts))) /* fetch size */
		!= kCycSuccess) return result;

	bfreeSafe(B_L, gCache.staticRoutes.routes);
	if (gCache.staticRoutes.numRoutes) {
		size = sizeof(StRoutesInfo) * gCache.staticRoutes.numRoutes;
		if (!(gCache.staticRoutes.routes = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(gCache.staticRoutes.routes,0,size);
		{
			CycParam values[] =
			{
				{ kStRoutes, kStRoutesInfo, gCache.staticRoutes.routes, gCache.staticRoutes.numRoutes },
			};

			if ((result = GetCycParamValues(values, mArrayLength(values)))
				!= kCycSuccess) return result;
		}
	}

   gCache.staticRoutes.routesChanged = FALSE;

   return result;
}

// PORTS -> Virtual Ports
Result LoadVirtualPorts(void)
{
   int size;
   Result result = kCycSuccess;
   
   bfreeSafe(B_L, gCache.virtualP.virtualInfo);
   size=sizeof(VirtualPortsInfo) * MAXVIRTUALPORTS;
   if (!(gCache.virtualP.virtualInfo = balloc(B_L, size)))
	   return kMemAllocationFailure; //mp
   memset(gCache.virtualP.virtualInfo,0,size);
   gCache.virtualP.numVirtual = MAXVIRTUALPORTS;
   {
      CycParam values[] =
      {
         { kPslave, kVirtualPortsInfo, gCache.virtualP.virtualInfo, (int)&gCache.virtualP.numVirtual },
         { kPslave, kClusteringIp, gCache.virtualP.clusteringIp, 1 },
      };
      result = GetCycParamValues(values, mArrayLength(values));
   } 
   return result;
}

// PORTS -> Physical Ports -> Auth. Server
Result LoadAuthPhysP(void)
{
	Result result = kCycSuccess;
	CycParam values[] =
	{
		{ kPslave, kUnitAuth, &gCache.authU.unitauthtype, 0 },
		{ kPslave, kPPPAuth, &gCache.authU.pppauthtype, 0 },
#ifdef NISenable
		{ kNis, kNisInfo, &gCache.authU.nis, 1 },
#endif
#ifndef KVM
		{ kLdapConf, kLdapInfo, &gCache.authU.ldap, 1 },
#ifdef KRBenable
		{ kKrb5Conf, kKrbInfo, &gCache.authU.krb, 1 },
#endif
#endif
		{ kRadiusConf, kRadiusInfo, &gCache.authU.radius, 1 },
		{ kTacplusConf, kTacplusInfo, &gCache.authU.tacplus, 1 },
#ifdef SMBenable
		{ kSmbConf, kSmbInfo, &gCache.authU.smb, 1 },
#endif
	};
	memset((char *)&gCache.authU,0,sizeof(AuthUnit_t));
	result = GetCycParamValues(values, mArrayLength(values));
	return result;
}

Result LoadTimeDate(void)
{
	CycParam values[] =
	{
		{ kNtp, kNtpEnable, &(gCache.timeDate.ntp), 0 },
		{ kNtp, kNtpServer, gCache.timeDate.ntpServer, 0 },
		{ kCycDateTime, kTimezone, gCache.timeDate.timezone, 0 },
		{ kCycDateTime, kMthDayYrHrMinSec, &(gCache.timeDate.dt), 0 },
		{ kCycDateTime, kTzParams, &(gCache.timeDate), 0 }
	};

	return GetCycParamValues(values, mArrayLength(values));
}

Result LoadMenuShellInfo(void)
{
   /* Create the buckets the right size */
   int size;
   Result result = kCycSuccess;

   CycParam counts[] =
   {
	   //BugFix #19551 [mp]
         { kMenuShCfg, kMenuShTitle, gCache.menuShell.menuShInfo.menuTitle, 0 },
         { kMenuShCfg, kNumMenuShOpt, &(gCache.menuShell.menuShInfo.numOptions), 0 },
   };
   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */

   if (gCache.menuShell.menuShInfo.optInfo) {
   		bfreeSafe(B_L, gCache.menuShell.menuShInfo.optInfo);
		gCache.menuShell.menuShInfo.optInfo = NULL;
	}
   size = sizeof(MenuShOpt) * (gCache.menuShell.menuShInfo.numOptions);
   if (size > 0) {
   		if (!(gCache.menuShell.menuShInfo.optInfo = balloc(B_L, size)))
	   		return kMemAllocationFailure; //mp
   		memset(gCache.menuShell.menuShInfo.optInfo, 0, size);
		{
   		CycParam values[] =
      	{
      		{ kMenuShCfg, kMenuShInfo, &gCache.menuShell.menuShInfo, gCache.menuShell.menuShInfo.numOptions },
      	};
      	result = GetCycParamValues(values, mArrayLength(values));
		}
	}
	
   return result;
}

Result LoadIpTable(void)
{
	Result result = kCycSuccess;
	int i, size;

	CycParam counts[] = {
		{ kIpTables, kIpTablesFilterChainNum, &(gCache.ipTable.confNumChains), 0 },
	};
	GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */

	bfreeSafe(B_L, gCache.ipTable.chainTable);

#ifdef IPv6enable
	CycParam counts6[] = {
		{ kIp6Tables, kIp6TablesFilterChainNum, &(gCache.ipTable.confNumChains6), 0 },
	};
	GetCycParamValues(counts6, mArrayLength(counts6)); /* fetch sizes */

	bfreeSafe(B_L, gCache.ipTable.chainTable6);
#endif

	gCache.ipTable.numChains = gCache.ipTable.confNumChains;
#ifdef IPv6enable
	gCache.ipTable.numChains += gCache.ipTable.confNumChains6;
#endif

	if (gCache.ipTable.confNumChains) {
		i = sizeof(ChainInfo) * (gCache.ipTable.confNumChains);
		if (!(gCache.ipTable.chainTable = (ChainInfo *)balloc(B_L, i)))
			return kMemAllocationFailure; //mp
		memset((char*)gCache.ipTable.chainTable, 0, i);
		{
			CycParam values[] = {
				{ kIpTables, kIpTablesFilterChainInfo, gCache.ipTable.chainTable, gCache.ipTable.confNumChains },
			};

			if (kCycSuccess == GetCycParamValues(values, mArrayLength(values))) {
				// Allocate space for rule array that is pointed by the rule pointer
				for (i=0; i<gCache.ipTable.confNumChains; i++) {
					if (gCache.ipTable.chainTable[i].numrules) {
						size = sizeof(IpTablesRules) * (gCache.ipTable.chainTable[i].numrules);
						if (!(gCache.ipTable.chainTable[i].rules = balloc(B_L, size)))
							return kMemAllocationFailure; //mp
						memset(gCache.ipTable.chainTable[i].rules, 0, size);
					}
				}
				// read the rules
				{
					CycParam values[] = {
						{ kIpTables, kIpTablesFilterChainRuleInfo, 
							gCache.ipTable.chainTable, gCache.ipTable.confNumChains },
					};
					result = GetCycParamValues(values, mArrayLength(values));
				}
			}
		}
	}
#ifdef IPv6enable
	if (gCache.ipTable.confNumChains6) {
		i = sizeof(ChainInfo) * (gCache.ipTable.confNumChains6);
		if (!(gCache.ipTable.chainTable6 = (ChainInfo *)balloc(B_L, i)))
			return kMemAllocationFailure; //mp
		memset((char*)gCache.ipTable.chainTable6, 0, i);

			CycParam values6[] = {
				{ kIp6Tables, kIp6TablesFilterChainInfo, gCache.ipTable.chainTable6, gCache.ipTable.confNumChains6 },
			};

		if (kCycSuccess == GetCycParamValues(values6, mArrayLength(values6))) {
			// Allocate space for rule array that is pointed by the rule pointer
			for (i=0; i<gCache.ipTable.confNumChains6; i++) {
				if (gCache.ipTable.chainTable6[i].numrules) {
					size = sizeof(IpTablesRules) * (gCache.ipTable.chainTable6[i].numrules);
					if (!(gCache.ipTable.chainTable6[i].rules = balloc(B_L, size)))
						return kMemAllocationFailure; //mp
					memset(gCache.ipTable.chainTable6[i].rules, 0, size);
				}
			}
			// read the rules
			{
				CycParam values6[] = {
					{ kIp6Tables, kIp6TablesFilterChainRuleInfo, 
						gCache.ipTable.chainTable6, gCache.ipTable.confNumChains6 },
				};
				result = GetCycParamValues(values6, mArrayLength(values6));
			}
		}
	}
#endif
	return result;
}

Result LoadActiveSession(void)
{
   Result result = kCycSuccess;
	int size;

   CycParam counts[] =
   {
      { kActiveSessions, kNumEntries, &(gCache.session.numSessions), 0 },
   };
   GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
   bfreeSafe(B_L, gCache.session.sess);
   if (gCache.session.numSessions)
   {
		size = sizeof(ActiveSessionInfo) * (gCache.session.numSessions);
		if (!(gCache.session.sess = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(gCache.session.sess, 0, size);
		memset(gCache.unitInfo.upTime, 0, kUpTimeLength);
      {
         CycParam values[] =
         {
            { kActiveSessions, kActiveSessionInfo, gCache.session.sess, gCache.session.numSessions },
		{ kUnitInfo, kUpTime, gCache.unitInfo.upTime, 0 }
         };
         result = GetCycParamValues(values, mArrayLength(values));
      }
   }
   return result;
}

#ifdef PCMCIA
Result LoadAccessMethod(void)
{
   Result result = kCycSuccess;
	
	memset(&gCache.accessMethod.pcmType, 0, sizeof(PcmciaType));
   {
		CycParam values[] =
      {
			{ kPcmcia, kPcmciaInfo, &gCache.accessMethod.pcmType, 1 }
      };
      result = GetCycParamValues(values, mArrayLength(values));
   }
   return result;
}
#endif

/* ------------------------------------------------------------------------- */
/* QUERY FUNCTIONS - Get info needed for web page */
/* ------------------------------------------------------------------------- */

extern int GetDevice(Param* param, int request)
{
   return mAppError(LoadFirst(LoadDevice, kLoadDevice, &gCache.device, &gCacheSave.device, sizeof(Device)));
}

//[RK]Jun/14/04 - Access Privilege
extern int GetRegUserAuthLocal(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	gCache.regUser.authLocal = CycRegUserAuthLocal(wUser.uName);

	return mAppError(result);
}

extern int GetNetSettings(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   return mAppError(LoadFirst(LoadNetSettings, kNetSettings, &gCache.netSettings, &gCacheSave.netSettings, sizeof(NetSettings)));
}

//[AP][2007-08-02] hostname discovery
extern int GetAutoDiscovery(Param* param, int request)
{
	Result result;

   if ((result = GetCycUserType(1)) != kCycSuccess)
      return mAppError(result);
	
   result = LoadFirst(LoadAutoDiscovery, kAutoDiscovery, &gCache.autoDiscovery, &gCacheSave.autoDiscovery, sizeof(AutoDiscovery));
   expandADStrings(); 

   return mAppError(result);
}

extern int GetCasProSettings(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   return mAppError(LoadFirst(LoadCasProSettings, kCasProSettings, &gCache.casProSettings, &gCacheSave.casProSettings, sizeof(CasProSettings)));
}

extern int GetDataBuffering(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   return mAppError(LoadFirst(LoadDataBuffering, kDataBuffering, &gCache.dataBuffering, &gCacheSave.dataBuffering, sizeof(DataBuffering)));
}

extern int GetUnitInfo(Param* param, int request)
{
	Result result;
    gCacheLoaded[kUnit] = true;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   result = LoadUnitInfo();

   if (kCycSuccess == result)
   {
	   GetUnitVersionInfo(param, request);
       GetUnitPwrSupplyInfo(param, request);
       GetUnitCpuInfo(param, request);
       GetUnitMemInfo(param, request);
   }

   return mAppError(result);
}

#if defined(KVM) || defined(ONS)
extern int GetUnitKVMPortStatus(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	gCacheLoaded[kKVMPStatus] = false;
   return mAppError(LoadFirst(LoadKVMPortStatus, kKVMPStatus, &gCache.unitInfo, &gCacheSave.unitInfo, sizeof(UnitInfo)));
}

extern int GetKVMDConf(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	return mAppError(LoadFirst(LoadKVMDConf, kKVMD, &gCache.kvmconf.kvmd, &gCacheSave.kvmconf.kvmd, sizeof(gCache.kvmconf.kvmd)));
}

extern int GetKVMDirectAccConf(Param* param, int request)
{
	return mAppError(LoadFirst(LoadKVMDConf, kKVMD, &gCache.kvmconf.kvmd, &gCacheSave.kvmconf.kvmd, sizeof(gCache.kvmconf.kvmd)));
}

extern int GetLoginDirectAccConf(Param* param, int request)
{
	Result result;
	gLogin.directacc = 0;
	if ((result = LoadSecProfile()) == kCycSuccess) {
#ifdef SECPROFILE
		if ((gCache.secProfile.options.profile == kSecOpen) ||
			((gCache.secProfile.options.profile == kSecCustom) &&
			 (gCache.secProfile.options.directaccess))) 
#else
		if (gCache.secProfile.options.directaccess)
#endif
		{
			if ((result = LoadKVMDConf()) == kCycSuccess) {
				if (gCache.kvmconf.kvmd.general.directacc) {
					if (gCache.kvmconf.kvmd.general.authtype) {
						gLogin.directacc = 1;
					} else { // Auth type NONE: can submit URL user and port
						gLogin.directacc = 2;
					}
				}
			}
		}
	}

	return result; 
}

extern int GetKVMCascConf(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   return mAppError(kCycSuccess);
}

extern int GetUnitFanStatus(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	return mAppError(LoadFirst(LoadUnitInfo, kUnit, &gCache.unitInfo, &gCacheSave.unitInfo, sizeof(UnitInfo)));
}

extern int GetKVMuCupdate(Param* param, int request)
{
	int i, j, nterms, ncasc;
	char_t *pHtml1, *pHtml2, *pHtml3, *pHtml4;
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	LoadFirst(LoadKVMuCupdate, kKVMuCupdate, NULL, NULL, 0);

	result = LoadFirst(LoadKVMDevices, kKVMDevices, &gCache.kvmdevice,
		NULL, sizeof(KVMDevice));
	if (kCycSuccess != result) {
		return mAppError(result);
	}

	/* Calculate number of ports and devices */
	nterms = 0;
	ncasc = 0;
	for (i = 0; i < gCache.kvmdevice.numDevices; i ++) {
		if (gCache.kvmdevice.deviceList[i].action == kDelete) {
			continue;
		}
		nterms += gCache.kvmdevice.deviceList[i].numTerms;
		for (j = 0; j < gCache.kvmdevice.deviceList[i].numTerms; j ++) {
			if (gCache.kvmdevice.deviceList[i].terms[j].status == 
				kTermCascading) {
				ncasc ++;
			}
		}
	}
	/* Allocate space for the list of ports and devices */
	bfreeSafe(B_L, gCache.kvmuCup.portsNameHtml);
	pHtml1 = gCache.kvmuCup.portsNameHtml = balloc(B_L, nterms * 23);
	if (pHtml1 == NULL) {
		return mAppError(kMemAllocationFailure);
	}
	bfreeSafe(B_L, gCache.kvmuCup.portsValHtml);
	pHtml2 = gCache.kvmuCup.portsValHtml = balloc(B_L, nterms * 23);
	if (pHtml2 == NULL) {
		return mAppError(kMemAllocationFailure);
	}
	bfreeSafe(B_L, gCache.kvmuCup.cascNameHtml);
	pHtml3 = gCache.kvmuCup.cascNameHtml = balloc(B_L, ncasc * 23);
	if (pHtml3 == NULL) {
		return mAppError(kMemAllocationFailure);
	}
	bfreeSafe(B_L, gCache.kvmuCup.cascValHtml);
	pHtml4 = gCache.kvmuCup.cascValHtml = balloc(B_L, ncasc * 23);
	if (pHtml4 == NULL) {
		return mAppError(kMemAllocationFailure);
	}
	/* Write the list of ports */
	*pHtml1 = 0;
	*pHtml2 = 0;
	*pHtml3 = 0;
	*pHtml4 = 0;
	for (i = 0; i < gCache.kvmdevice.numDevices; i ++) {
		if (gCache.kvmdevice.deviceList[i].action == kDelete) {
			continue;
		}
		for (j = 0; j < gCache.kvmdevice.deviceList[i].numTerms; j++) {
			if (i == 0) {
				sprintf (pHtml1 + strlen(pHtml1), 
					"\"Port %d\",", j + 1);
				sprintf (pHtml2 + strlen(pHtml2), 
					"\"%d\",", j + 1);
			} else {
				sprintf (pHtml1 + strlen(pHtml1), "\"%s.%d\",", 
					gCache.kvmdevice.deviceList[i].name, 
					j + 1);
				sprintf (pHtml2 + strlen(pHtml2), "\"%s.%d\",", 
					gCache.kvmdevice.deviceList[i].name, 
					j + 1);
			}
			if (gCache.kvmdevice.deviceList[i].terms[j].status == 
				kTermCascading) {
				if (i == 0) {
					sprintf (pHtml3 + strlen(pHtml3), 
						"\"Port %d\",", j + 1);
					sprintf (pHtml4 + strlen(pHtml4), 
						"\"%d\",", j + 1);
				} else {
					sprintf (pHtml3 + strlen(pHtml3), "\"%s.%d\",", 
						gCache.kvmdevice.deviceList[i].name, j);
					sprintf (pHtml4 + strlen(pHtml4), "\"%s.%d\",", 
						gCache.kvmdevice.deviceList[i].name, j);
				}
			}
		}
	}
	if (strlen(pHtml1)) {
		pHtml1[strlen(pHtml1) - 1] = 0;
	}
	if (strlen(pHtml2)) {
		pHtml2[strlen(pHtml2) - 1] = 0;
	}
	if (strlen(pHtml3)) {
		pHtml3[strlen(pHtml3) - 1] = 0;
	}
	if (strlen(pHtml4)) {
		pHtml4[strlen(pHtml4) - 1] = 0;
	}
	return mAppError(kCycSuccess);
}

extern int GetKVMuCreset(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	LoadFirst(LoadKVMuCreset, kKVMuCreset, NULL, NULL, 0);

	return mAppError(kCycSuccess);
}

extern int KVMPMGetSlaveNumPorts(Param* param, int request)
{
/*
        unsigned char command[128],  *tmpfile = "/tmp/.pmtemp", *tmpfile1 = "/tmp/pmtemp";
        FILE *fp;
        struct stat st;
        unsigned char *s;
*/
	return mAppError(kCycSuccess);
}

extern int KVMPMGetNumPorts(Param* param, int request)
{

        unsigned char command[128],  *tmpfile = "/tmp/.pmtemp", *tmpfile1 = "/tmp/pmtemp";
        FILE *fp;
        struct stat st;
        unsigned char *s;
        int number = 0, l, x, tmp;
	if (!gCacheLoaded[kKVMauxPort]) 
		LoadFirst(LoadKVMauxPort, kKVMauxPort, &gCache.kvmAuxPort[0], &gCacheSave.kvmAuxPort[0], MAX_AUX * sizeof(KVMAuxPort));
        //printf("gCache Loaded: [%d]\n", gCache.kvmAuxPort.protocol);
	if (gCache.kvmAuxPort[0].protocol == 0) {
	        snprintf(command, 128, "ps -ax | grep pmd | grep -v grep > %s", tmpfile1);
	        system(command);
	        stat((const char *)tmpfile1,&st);
	        if (st.st_size == 0) return 0;
	        //printf("String: [%d]\n", st.st_size);
	        snprintf(command, 128, "rm -f %s", tmpfile1);
	        system(command);

#ifdef ONS
	        snprintf(command, 128, "pmCommand a1 status all | tail -n 1 | cut -f1 > %s", tmpfile);
#else
	        snprintf(command, 128, "pmCommand 1 status all | tail -n 1 | cut -f1 > %s", tmpfile);
#endif
	        x = system(command);
	        if ((fp = fopen(tmpfile, "r")) == NULL) {
			return mAppError(kCycSuccess);
	        }
	        if (! fgets(command, 128, fp)) {
	                fclose(fp);
	                unlink(tmpfile);
			return mAppError(kCycSuccess);
	        }
		s = command;
	        l=strlen(s)-1;
	
	        for(;x <= l;x++)
	        {
	                if ((s[x]>='0')&&(s[x]<='9'))
	                {
	                        tmp=(s[x]-'0');//0 is the 0 index
	                        number=number*10+tmp; //to enlarge the number by *10 every time
	                }
	        }
		gCache.temp.numOutlets = number;
        	//printf("OUTLETS: [%d]\n", number);
	}	
	return mAppError(kCycSuccess);
}



extern int GetKVMauxPort(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	LoadFirst(LoadKVMauxPort, kKVMauxPort, &gCache.kvmAuxPort[0], &gCacheSave.kvmAuxPort[0], MAX_AUX * sizeof(KVMAuxPort));


/*  	printf("############### From WebApi:\n\ */
/*  protocol: %d,\n\ */
/*  baudRate: %d,\n\ */
/*  dataSize: %d,\n\ */
/*  stopBits: %d,\n\ */
/*  parity: %d,\n\ */
/*  flowControl: %d,\n\ */
/*  initChat: <%s>,\n\ */
/*  autoPPP: <%s>,\n", */
/*  		   gCache.kvmAuxPort.protocol, */
/*  		   gCache.kvmAuxPort.baudRate, */
/*  		   gCache.kvmAuxPort.dataSize, */
/*  		   gCache.kvmAuxPort.stopBits, */
/*  		   gCache.kvmAuxPort.parity, */
/*  		   gCache.kvmAuxPort.flowControl, */
/*  		   gCache.kvmAuxPort.initChat, */
/*  		   gCache.kvmAuxPort.autoPPP); */

	return mAppError(kCycSuccess);
}
#endif	// KVM

extern int GetUnitBootConfInfo(Param* param, int request)
{
	Result result;
#ifdef UBOOT_PARAM
	int len = 0;
#endif

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   result = LoadFirst(LoadBootInfo, kBootInfo, &gCache.bootInfo, &gCacheSave.bootInfo, sizeof(BootConf));

#ifdef UBOOT_PARAM
   if (result != kCycSuccess) return  mAppError(result);

   len += sprintf(&gCache.bootInfo.uBootHtml[len],
		"<option value='0' %s> Network </option>",
			gCache.bootInfo.uBoot == kNetwork ? "SELECTED" : "");

   len += sprintf(&gCache.bootInfo.uBootHtml[len],
		"<option value='1' %s> %s </option>",
		gCache.bootInfo.uBoot == kFlash && gCache.bootInfo.currentImage == '1' ?
		"SELECTED" : "", gCache.bootInfo.image1Filename);

   len += sprintf(&gCache.bootInfo.uBootHtml[len],
		"<option value='2' %s> %s </option>",
		gCache.bootInfo.uBoot == kFlash && gCache.bootInfo.currentImage == '2' ?
		"SELECTED" : "", gCache.bootInfo.image2Filename);
#endif
	return mAppError(result);
}

extern int GetTimeDate(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	if (gCacheLoaded[kTimeDate] == true)
	{ 
		//we only need to update time information...
		CycParam values[] =
		{
			{ kCycDateTime, kMthDayYrHrMinSec, &(gCache.timeDate.dt), 0 },
		};

		//...and save it to cache for a possible next get()
		if ((result = GetCycParamValues(values, mArrayLength(values))) == kCycSuccess)
			memcpy(&gCacheSave.timeDate.dt, &gCache.timeDate.dt, sizeof(gCache.timeDate.dt));

		return result;
	}

	return mAppError(LoadFirst(LoadTimeDate, kTimeDate, &gCache.timeDate, &gCacheSave.timeDate, sizeof(TimeDate)));
}

/*extern int GetNetServices(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   return mAppError(LoadFirst(LoadNetServices, kServices, &gCache.netServices, &gCacheSave.netServices, sizeof(NetServices)));
}*/

/****************************************************/
/* Special handling query functions. */
/* WIZARD -> ACCESS , SYSTEM LOG */
/* EXPERT -> SECURITY , NETWORK->SYSLOG */
/****************************************************/

/* Get the list of system users */
extern int GetAccessUser(Param* param, int request)
{
	UInt16 x = 0, i;
	Result result;
	Users *pu;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadAccessUsers, kAccessUsers, NULL, NULL, 0); 

	if (kCycSuccess == result) {
		/* Fill in usersHtml */
#if defined(KVM) || defined(ONS)
		char_t templateStr[] = "<option value=\"%d!%c\">%s</option>\n";
#else
		char_t templateStr[] = "<option value=\"%d\">%s</option>\n";
#endif
		char_t templateFilled[100] = {0};

		bfreeSafe(B_L, gCache.access.usersHtml);
		gCache.access.usersHtml = 0;
		if (gCache.access.numUsers) {
			i = sizeof(templateFilled) * (gCache.access.numUsers);
			gCache.access.usersHtml = balloc(B_L, i);
			if (! gCache.access.usersHtml) {
				return mAppError(kMemAllocationFailure);
			}
			memset(gCache.access.usersHtml, 0, i);

#if defined(KVM) || defined(ONS)
			gsprintf(templateFilled, "<option value=\"0!g\">Generic User</option>\n");
			gstrcat(gCache.access.usersHtml, templateFilled);
#endif

			for (x=0,pu=gCache.access.users; x < gCache.access.numUsers; x++,pu++) {
				if (pu->action != kDelete) {
#if defined(KVM) || defined(ONS)
					gsprintf(templateFilled, templateStr, x + 1,
						(pu->groupAdminReg == kRegularUser)?'r':'a',
						pu->userName);
#else
					gsprintf(templateFilled, templateStr, x+1, pu->userName);
#endif
					gstrcat(gCache.access.usersHtml, templateFilled);
				}
			}
		}
	}
	return mAppError(result);
}

/* Get the list of groups configured in pslave */
extern int GetAccessGroup(Param* param, int request)
{
	UInt16 x=0,i;
	Result result;
	ConfGroups *pg;
   
	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadAccessGroups, kAccessGroups, NULL, NULL, 0); 

	if (kCycSuccess == result){
		/* Fill in groupsHtml */
		char_t templateStr[] = "<option value=\"%d\">%s</option>\n";
		char_t templateFilled[100]= {0};
		
	  	bfreeSafe(B_L, gCache.access.groupsHtml);
		gCache.access.groupsHtml = 0;

		if (gCache.access.numGroups) {
			i= sizeof(templateFilled) * (gCache.access.numGroups);
			if (!(gCache.access.groupsHtml = balloc(B_L, i)))
				return mAppError(kMemAllocationFailure); //mp
			memset(gCache.access.groupsHtml, 0, i);
			for (x=0, pg=gCache.access.groups; x<gCache.access.numGroups; x++,pg++) {
            			gsprintf(templateFilled, templateStr, x+1,pg->newGrpName);
				gstrcat(gCache.access.groupsHtml, templateFilled);
			}
		}
	}
	return mAppError(result);
}

/* Get the info for a specific group chosen to be edited */
extern int GetGroupSelected(Param* param, int request)
{
	Result result = kCycSuccess;
	
	if (!gCacheLoaded[kAccessGroups])
		return mAppError(result);

	if ((gCache.access.action == 3)	&&
	    (gCache.access.selected > gCache.access.numGroups))
		return mAppError(result);

	if (gCache.access.action == 3) { // edit
		memcpy(&gCache.access.addModGroup,&gCache.access.groups[gCache.access.selected-1],
				sizeof(ConfGroups));
	} else {
		memset(&gCache.access.addModGroup,0,sizeof(ConfGroups));
	}

	return mAppError(result);
}

extern int GetSystemLog(Param* param, int request) {

	Result result;
	SyslogServerInfo * sys;
	char buffer[2*kIpLength] = {0};
	int i;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadSystemLog, kSystemLog, &gCache.systemLog,&gCacheSave.systemLog, sizeof(SystemLog));
	/* Fill in serversHtml */

	bfreeSafe(B_L, gCache.systemLog.serversHtml);
	gCache.systemLog.serversHtml = 0;
	
	i = gCache.systemLog.numServers? 120 * gCache.systemLog.numServers : 120;
	if (!(gCache.systemLog.serversHtml = balloc(B_L, i)))
		return mAppError(kMemAllocationFailure); //mp

	memset(gCache.systemLog.serversHtml,0,i);

	gstrcat(gCache.systemLog.serversHtml, "[");
	if (gCache.systemLog.numServers) {
		sys = gCache.systemLog.servers;
		for (i=0; i < gCache.systemLog.numServers; i++,sys++) {
			if (sys->action != kDelete) {
				sprintf(buffer, "[\"%s\"],", sys->SyslogServer);
				gstrcat(gCache.systemLog.serversHtml, buffer);
			}
		}
	}
	gstrcat(gCache.systemLog.serversHtml, "]");

	return mAppError(result);

}

/****************************************************/
/* EXPERT -> ADMINISTRATION */
/****************************************************/

// System (unit) Info
extern int GetUnitVersionInfo(Param* param, int request)
{
   UnitVersionInfo *uv;
   Result result = kCycSuccess;
   
   if (kCycSuccess == result)
    {
      /* Fill in versionHtml */
      char_t templateStr[] = "%s<br>\n%s<br>\n%s<br>\n%s<br>";
      uv = &gCache.unitInfo.version;
      bfreeSafe(B_L, gCache.unitInfo.versionHtml);
      if (!(gCache.unitInfo.versionHtml = balloc(B_L, kHtmlListLength)))
		  return mAppError(kMemAllocationFailure); //mp
      memset(gCache.unitInfo.versionHtml, 0, kHtmlListLength);
	//[RK]Feb/25/05 - removed the GenerationSess
      sprintf(gCache.unitInfo.versionHtml, templateStr, uv->linuxVersion, 
         uv->compilerVersion, uv->dateOfVersionGeneration, uv->firmwareVersion);
   }
   return mAppError(result);
}

extern int GetUnitPwrSupplyInfo(Param* param, int request)
{
   PwrSupplyInfo *pw;
   Result result = kCycSuccess;

   if (kCycSuccess == result)
   {
	   char_t templateStr[] = "Number 1 %s<br>Number 2 %s";
	   pw = &gCache.unitInfo.pwrSupply;
	   bfreeSafe(B_L, gCache.unitInfo.pwrSupplyHtml);
	   if (!(gCache.unitInfo.pwrSupplyHtml = balloc(B_L, kHtmlListLength)))
		   return mAppError(kMemAllocationFailure); //mp
	   memset(gCache.unitInfo.pwrSupplyHtml, 0, kHtmlListLength);
	   if (pw->pwrSupply1 == kSinglePower)
		   strcpy(gCache.unitInfo.pwrSupplyHtml, "SINGLE");
	   else
		   sprintf(gCache.unitInfo.pwrSupplyHtml, templateStr, pw->pwrSupply1 ? "ON" : "OFF", pw->pwrSupply2 ? "ON" : "OFF");
   }
   return mAppError(result);
}

extern int GetUnitCpuInfo(Param* param, int request)
{
   CpuInfo *cpu;
   Result result = kCycSuccess;
   
   if (kCycSuccess == result)
    {
      cpu = &gCache.unitInfo.cpuInfo;
      gCache.unitInfo.cpuInfo.processor = cpu->processor;
      strcpy(gCache.unitInfo.cpuInfo.cpu, cpu->cpu);
      strcpy(gCache.unitInfo.cpuInfo.clock, cpu->clock);
      strcpy(gCache.unitInfo.cpuInfo.busClock, cpu->busClock);
      strcpy(gCache.unitInfo.cpuInfo.revision, cpu->revision);
      strcpy(gCache.unitInfo.cpuInfo.bogomips, cpu->bogomips);
      strcpy(gCache.unitInfo.cpuInfo.zeroPages, cpu->zeroPages);
   }
   return mAppError(result);
}

#ifdef PCMCIA
extern int GetUnitPcmciaInfo(Param* param, int request)
{
   UInt16 x=0;
   Result result;
   gCacheLoaded[kUnitPcmcia] = true;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   result = LoadUnitPcmciaInfo();
   
   if (kCycSuccess == result)
    {
      /* Fill in pcmciaHtml */
      char_t templateStr[] = "<td class=\"%s\">Socket %d - Ident:</td>\n       <td class=\"%s\">%s</td>\n       </tr>\n      <tr class=\"%s\">\n         <td class=\"%s\">Socket %d - Config:</td>\n        <td class=\"%s\">%s</td>\n      </tr>\n      <tr class=\"%s\">\n       <td class=\"%s\">Socket %d - Status:</td>\n        <td class=\"%s\">%s</td></tr>";

      char_t templateFilled[kHtmlListLength] = {0};
      bfreeSafe(B_L, gCache.unitInfo.pcmciaHtml);
      if (!(gCache.unitInfo.pcmciaHtml = balloc(B_L, kHtmlListLength)))
		  return mAppError(kMemAllocationFailure); //mp
      memset(gCache.unitInfo.pcmciaHtml, 0, kHtmlListLength);
      for (x = 0; x < gCache.unitInfo.numPcmcia; x++)
      {
         PcmciaInfo pcm = gCache.unitInfo.pcmcia[x];
         gsprintf(templateFilled, templateStr, isEven(x)?"colorTRow":"colorTRowAlt", x, isEven(x)?"colorTRow":"colorTRowAlt", pcm.identInfo, isEven(x)?"colorTRow":"colorTRowAlt", isEven(x)?"colorTRow":"colorTRowAlt", x, isEven(x)?"colorTRow":"colorTRowAlt", pcm.configInfo, isEven(x)?"colorTRow":"colorTRowAlt", isEven(x)?"colorTRow":"colorTRowAlt", x, isEven(x)?"colorTRow":"colorTRowAlt", pcm.status);
         gstrcat(gCache.unitInfo.pcmciaHtml, templateFilled);
      }
   }
   return mAppError(result);
}
#endif

extern int GetUnitRamDiskInfo(Param* param, int request)
{
   UInt16 x=0, size;
   Result result;
   gCacheLoaded[kUnitRam] = true;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   result = LoadUnitRamDiskInfo();
   
   if (kCycSuccess == result)
   {
	   /* Fill in ramHtml */
	   char_t templateStr[] = "<tr class=\"%s\">\n          <td class=\"%s\">%s</td>\n          <td class=\"%s\">%s</td>\n          <td class=\"%s\">%s</td>\n          <td class=\"%s\">%s</td>\n          <td class=\"%s\">%s</td>\n          <td class=\"%s\">%s</td>\n         </tr>";
	   char_t templateFilled[500] = {0};
	   RamDiskInfo *ram = gCache.unitInfo.ram;
	   
	   bfreeSafe(B_L, gCache.unitInfo.ramHtml);
	   gCache.unitInfo.ramHtml = 0;
	   if (gCache.unitInfo.numRamDisk) {
		   size = sizeof(templateFilled) * (gCache.unitInfo.numRamDisk);
		   // dynamic memory allocation
		   if (!(gCache.unitInfo.ramHtml = balloc(B_L, size)))
			   return mAppError(kMemAllocationFailure); //mp
		   // initialize the memory with NULL.
		   memset(gCache.unitInfo.ramHtml, 0, size);   
		   
		   for (x = 0; x < gCache.unitInfo.numRamDisk; x++, ram++) 
		   {
			   gsprintf(templateFilled, templateStr, isEven(x)?"colorTRow":"colorTRowAlt", isEven(x)?"colorTRow":"colorTRowAlt", ram->fsName, isEven(x)?"colorTRow":"colorTRowAlt", ram->blocks, isEven(x)?"colorTRow":"colorTRowAlt", ram->used, isEven(x)?"colorTRow":"colorTRowAlt", ram->avail, isEven(x)?"colorTRow":"colorTRowAlt", ram->usage, isEven(x)?"colorTRow":"colorTRowAlt", ram->mountedOn);
			   gstrcat(gCache.unitInfo.ramHtml, templateFilled);
		   }
	   }
   }
   return mAppError(result);
}

extern int GetUnitMemInfo(Param* param, int request)
{
	MemInfo *mem;
	UInt16 x=0, size;
	Result result = kCycSuccess;
	
	if (kCycSuccess == result)
    {
		char_t templateStr[] = "\n      <tr class=\"%s\">\n        <td class=\"%s\">%s:</td>\n        <td class=\"%s\">%s</td>\n      </tr>";
		char_t templateFilled[500] = {0}; //[RK]Feb/15/05
		mem = &gCache.unitInfo.memory;
		
		if (mem->numEntries) {
			bfreeSafe(B_L, gCache.unitInfo.memHtml);
			size = sizeof(templateFilled) * mem->numEntries;
			
			// dynamic memory allocation
			if (!(gCache.unitInfo.memHtml = balloc(B_L, size)))
				return mAppError(kMemAllocationFailure); //mp
			
			// clean memory
			memset(gCache.unitInfo.memHtml, 0, size);   
			
			for (x = 0; x < mem->numEntries; x++)
			{
				gsprintf(templateFilled, templateStr, isEven(x)?"colorTRow":"colorTRowAlt", isEven(x)?"colorTRow":"colorTRowAlt", mem->name[x], isEven(x)?"colorTRow":"colorTRowAlt", mem->value[x]);
				gstrcat(gCache.unitInfo.memHtml, templateFilled);
			}
		}
	}
	
	return mAppError(result);
}

extern int GetConnectPortsHtml(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

#ifdef ONS
	if (gCache.device.number2) {
#else
	if (gCache.device.number) {
#endif
		bfreeSafe(B_L, gCache.connectPortsHtml);
		//[RK]Jun/03/05 - Group Authorization Support
		portMountConnectPortsHtml(wUser.uName, wUser.uType, wUser.uGroupName);
	}
	return mAppError(kCycSuccess);
}

extern int ShowAppletHtml(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* portValue = NULL;
   int portNum=0;
	Sid currSid = 0;
	char_t *id, nparam;
	Result result = kCycSuccess;
	char tempId[25] = {0};
	int xorLen;

	memset(&wUser, 0, sizeof(wUser));

	nparam=ejArgs(argc, argv, T("%s"), &id);
	if (nparam==1) {
		currSid = gatoi(id);
		if ((result = CycCheckUser(&currSid, &wUser, 0)) != kCycSuccess) {
			dmfSetErrorCode(eid,result); //[RK]Aug/01 - set the error
			return 0;
		}
	}
	portValue = websGetVar(wp, "port", "x");
	if (*portValue != 'x') {
		portNum = atoi(portValue);
	}
	
	//xorLen = EncryptString(NULL, id, 1);
	sprintf(tempId,"%d",wUser.uPassK); //[RK]Jul/29/05 
	xorLen = EncryptString(NULL, tempId, 1);
	id = HexEncode(tempId, xorLen);
	strcpy(tempId, id);

	if (*portValue == 'x') {
		boxMountAppletHtml(tempId, wUser.uName, wUser.uPass, wp);
	} else {
		portMountAppletHtml(portNum, tempId, wUser.uName, wUser.uPass, wp);
	}

	dmfSetErrorCode(eid,result); //[RK]Aug/01 - set the error
	return 0;
}

extern int ShowGrapherAppletHtml(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* portValue = NULL;
   int portNum;
	Sid currSid = 0;
	char_t *id, nparam;
	Result result = kCycSuccess;
	char tempId[25] = {0};
	int xorLen;

        if ((libcyc_kvm_get_model(cyc) != MODEL_KVMNET_PLUS) &&
            (libcyc_kvm_get_model(cyc) != MODEL_ONSITE)) return(result);

	memset(&wUser, 0, sizeof(wUser));

	nparam=ejArgs(argc, argv, T("%s"), &id);
	if (nparam==1) {
		currSid = gatoi(id);
		if ((result = CycCheckUser(&currSid, &wUser, 0)) != kCycSuccess)
			return mAppError(result);
	}
	portValue = websGetVar(wp, "port", "1");
	portNum = atoi(portValue);
	
	xorLen = EncryptString(NULL, id, 1);
	id = HexEncode(id, xorLen);
	strcpy(tempId, id);
	
	portMountGrapherAppletHtml(portNum, tempId, wUser.uName, wUser.uPass, wp);

	return (result);
}

static void  WriteProfileFile(webs_t wp, char_t *path, char_t *query)
{
	FILE *arq;
	unsigned char *filename = websGetVar(wp, T("filename"), "");
	unsigned char *filetext = websGetVar(wp, T("filetext"), "");
	unsigned char *filedir = "/new_web/normal/applications/appl/profiles/";
	unsigned char *filepath;
	unsigned char *s, *t;
	/* struct cyc_webs_header hdr; */
	unsigned char error=1, *message;
	int maxsize = (strlen(filedir) + strlen(filename))* sizeof(char);

	filepath = (char *) malloc(maxsize);
	strcpy(filepath, filedir);
	strcat(filepath, filename);
	/* Remove all the <CR> characters */
	for (s=filetext+strlen(filetext)-1; s>=filetext; s--) {
		if (*s == '\r') {
			for (t=s+1; *(t-1); t++) *(t-1)=*t;
		}
	}
	if ((arq = fopen(filepath, "w+")) != NULL) {
		setbuf(arq,NULL);
		if (fwrite(filetext, 1, strlen(filetext), arq) != strlen(filetext)) {
			message = "error";
		} else {
			message = "success";
			error = 0;
		}
		fclose(arq);
	} else {
		message = "error";
	}
	free(filepath);

	websHeader(wp);
	websWrite(wp, message);
	websFooter(wp);
	websDone(wp, 200);
}

static void  DeleteProfile(webs_t wp, char_t *path, char_t *query)
{
	unsigned char *filename = websGetVar(wp, T("filename"), "");
	unsigned char *filedir = "/new_web/normal/applications/appl/profiles/";
	unsigned char filepath[1024];

	strcpy(filepath, filedir);
	strcat(filepath, filename);
	unlink(filepath);

	websHeader(wp);
	websWrite(wp, "success");
	websFooter(wp);
	websDone(wp, 200);
}
static void  ReadDirList(webs_t wp, char_t *path, char_t *query)
{
	struct dirent *dentry;
	DIR *dpntr;
	int maxsize;
	char_t *message = NULL, *message1 = NULL;
	dpntr = opendir("/new_web/normal/applications/appl/profiles");
	if(dpntr == 0)
		fmtAlloc(&message, 10, T("error|"));
	else{
		dentry = readdir(dpntr);
		fmtAlloc(&message1, 10, T("success|"));
		while(dentry != 0)
		{
			if( (strcmp(dentry->d_name, ".") == 0) || (strcmp(dentry->d_name, "..") == 0) )
			{
				dentry = readdir(dpntr);
				continue;
			}
			message = NULL;
			maxsize = strlen(message1) + strlen(dentry->d_name) + 2;
			fmtAlloc(&message, maxsize, T("%s%s|"), message1, dentry->d_name);
			bfree(B_L, message1);
			message1 = message;
			dentry = readdir(dpntr);		
		}
	}
	websHeader(wp);
	if(message == NULL)
		websWrite(wp, message1);
	else
		websWrite(wp, message);
	websFooter(wp);
	websDone(wp, 200);
	if(message != NULL)
		bfree(B_L, message);
	bfree(B_L, message1);
}

//[RK]Sep/14/04 - Ports Status
extern int GetPortsStatusHtml(Param* param, int request)
{
	int size;
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess){
		return mAppError(result);
	}

#ifdef ONS
	if (gCache.device.number2) {
#else
	if (gCache.device.number) {
#endif
		bfreeSafe(B_L, gCache.portsStatusHtml);
#ifdef ONS
		size = 400 * (gCache.device.number2);
#else
		size = 400 * (gCache.device.number);
#endif
		// dynamic memory allocation
		if (!(gCache.portsStatusHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
		// initialize the memory with NULL.
		memset(gCache.portsStatusHtml, 0, size);	
		portMountPortsStatusHtml(gCache.portsStatusHtml);
	}
	return mAppError(kCycSuccess);
}

//[RK]Jan/28/01 - Ports Statistics
extern int GetPortsStatisticsHtml(Param* param, int request)
{
	int size;
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess){
		return mAppError(result);
	}

#ifdef ONS
	if (gCache.device.number2) {
#else
	if (gCache.device.number) {
#endif
		bfreeSafe(B_L, gCache.portsStatisticsHtml);
#ifdef ONS
		size = 400 * (gCache.device.number2);
#else
		size = 400 * (gCache.device.number);
#endif
		// dynamic memory allocation
		if (!(gCache.portsStatisticsHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
		// initialize the memory with NULL.
		memset(gCache.portsStatisticsHtml, 0, size);	
		portMountPortsStatisticsHtml(gCache.portsStatisticsHtml);
	}
	return mAppError(kCycSuccess);
} 


#if defined(KVM) || defined(ONS)
int ShowViewerHtml(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *portValue=NULL, *sniff=NULL, *otherstation=NULL;
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess) {
		return mAppError(result);
	}

	portValue = websGetVar(wp,"portName","0");
	sniff = websGetVar(wp,"sniffopt",NULL);
	otherstation = websGetVar(wp,"otherstation",NULL);
	portMountViewerHtml(portValue, sniff, otherstation, wp);

	return(kCycSuccess);
}

#ifdef IPCASCADING
int ShowIPCascadeHtml(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *pipsec = websGetVar(wp, "ipSec", NULL);
	char_t *username = websGetVar(wp, "username", NULL);
	char_t *portname = websGetVar(wp, "portname", NULL);
	char_t *port = websGetVar(wp, "portNum", NULL);
	char_t *master = websGetVar(wp, "master", NULL);
	UInt32 ipsecurity;
	if (master && pipsec && username && portname && port &&
		sscanf(pipsec, "%d", &ipsecurity) > 0) {
//printf("SetCommand - pass2 %s %d %s %s %s\n", master, ipsecurity, username, portname, port);
		KVMIPCascadeConnection(wp, ipsecurity, username, 
			portname, port, master);
	}
	return(kCycSuccess);
}

static void setCommand(webs_t wp, char_t *path, char_t *query)
{
#if 0
/* This is commented out because we don't have a proper HTTP Auth mechanism
in place to protect against users executing arbitrary commands as root. */
	unsigned char comm[256], *command;
	FILE *fp;

//printf("SetCommand - start\n");
	command = websGetVar(wp,"command", NULL);
	if (command == NULL) {
		return(kCycSuccess);
	}
//printf("SetCommand - command = %s\n", command);
	snprintf(comm, 256, "%s > %s", command, "/tmp/webtemp");
//printf("SetCommand - comm = %s\n", comm);
	system(comm);
	fp = fopen("/tmp/webtemp", "r");
	if (fp == NULL) {
		return(kCycSuccess);
	}
	websWrite(wp,"HTTP/1.0 200 OK\r\n");
	websWrite(wp,"Content-Type: text/plain\r\n\r\n");
//printf("SetCommand - pass1\n");
	while (fgets(comm, 256, fp) != NULL) {
		websWrite(wp, comm);
	}
//printf("SetCommand - end\n");
	fclose (fp);
	websDone(wp, 200);
#endif
	return(kCycSuccess);
}
#endif

#ifdef RDPenable

/* Returns 1 if RDP connection's file exist, otherwise returns zero */
int RdpConnectionExist(unsigned short port)
{
	char pid_file[MAX_STR_LEN];
	
	snprintf(pid_file, MAX_STR_LEN,
		 "/var/run/rdp_conntrack-%d.pid", (int)port);
	pid_file[MAX_STR_LEN-1] = '\0';
	
	return(libcyc_file_exist(pid_file));
}

extern tRDPconnect  RDPconnect[];
int KillRdpConnection(int n)
{
  int i;
  char buf[MAX_STR_LEN];
  pid_t rdp_pid;

  /* find the corresponding RDP connection */
	for (i = 0; i < MAXRDPCONNECT; i++) {
		if (RDPconnect[i].cliport && RDPconnect[i].rdpasix == n && 
			RdpConnectionExist(RDPconnect[i].cliport)) {

			strncpy(RDPconnect[i].rdpkill, wUser.uName, lRDP_user-1);

// Get the pid of rdp_conntrack process which is monitoring this RDP connection
			snprintf(buf, MAX_STR_LEN,
				 "/var/run/rdp_conntrack-%d.pid",
				 RDPconnect[i].cliport);
			buf[MAX_STR_LEN-1] = '\0';

/* Send the interrupt signal to the corresponding rdp_conntrack process */
			rdp_pid = libcyc_get_pid_from_runfile(buf);
			syslog(LOG_DEBUG, "Sending SIGTERM signal to the rdp_conntrack[pid=%d] process",rdp_pid);
			kill(rdp_pid, SIGTERM);
			break;
		}
	}
	return kCycSuccess;
}

int CheckRDPconn(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *rdpport=NULL, *start=NULL;
	char_t *ident=NULL, *user=NULL;
	char_t message[64];
	int ix, port = 0;

//Check if monitor keeps the connection
	rdpport = websGetVar(wp,"rdpport",NULL);
	start = websGetVar(wp,"start",NULL);
	ident = websGetVar(wp,"ident",NULL);
	port = atoi(rdpport);

	for (ix=0; ix < MAXRDPCONNECT; ix++) {
		if (RDPconnect[ix].cliport == port &&
			RDPconnect[ix].conn_time == atoi(ident)) {
			break;
		}
	}

	if (ix < MAXRDPCONNECT && RdpConnectionExist(port)) {
		websWrite(wp, " onload=\"launchRDP();\" ");
	}
	else {
		syslog(LOG_DEBUG, "RDP connection fail. Port (%d) and ID (%.16s) %s.", port, ident, (ix < MAXRDPCONNECT)?"exists":"mismatch");
		if (!strncmp(start, "yes", 3)) {
			snprintf(message, 63, "Connection mismatch. Invalid port %d or ID %.16s", port, ident );
		} else if (ix >= MAXRDPCONNECT) {
			snprintf(message, 63, "Connection lost.");
		} else {
			user = RDPconnect[ix].rdpkill;
			if (user && *user) {
				snprintf(message, 63, "Session killed by %s user.", user);
			} else {
				snprintf(message, 63, "Connection closed.");
			}
		}
		websWrite(wp, " onload=\"RDPerror(\'%s\');\" ", message);
	}
    return (kCycSuccess);       // connection still ON
}

int ShowRDPHtml(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *portValue=NULL;
	char_t *connect=NULL;
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess) {
		return mAppError(result);
	}

	portValue = websGetVar(wp,"portName","0");
	connect = websGetVar(wp,"connect",NULL);
	portMountRDPHtml(portValue, connect, wp);

	return(kCycSuccess);
}
#endif	//RDPenable

int CheckStationRequest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t nparam;
	char_t *ip = websGetRequestIpaddr(wp);
	char_t *station;

	nparam = ejArgs(argc, argv, T("%s"), &station);

	if (nparam != 1 || !checkPortStatusNet(station, ip, wp)) {
		websWrite(wp, "None");
	}
	return kCycSuccess;
}

int SwitchStation(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t nparam;
	char_t *device, *station, *sniffopt, *otherstation;

	nparam = ejArgs(argc, argv, T("%s %s %s %s"), &station, &device, 
		&sniffopt, &otherstation);
	if(nparam == 4){
		KVMSwitchStation(wp, station, device, sniffopt, otherstation);
	}
	return kCycSuccess;
}

int ReconnectStation(int eid, webs_t wp, int argc, char_t **argv)
{
	/* Bring this variable over from KVMFunc.c.  KVMFunc.c really
	needs accessor functions to avoid hacks like this. */
	char_t nparam;
	char_t *device;
	char_t *station;
	char_t *username;
	char cmd_output[MAX_RESULT_LEN];
	char *http_host_str = websGetHTTPHost();
	char *kvmip_req = websGetRequestIpaddr(wp);

	nparam = ejArgs(argc, argv, T("%s %s %s"), &station, &device, 
		&username);
	if(nparam == 3){
		*station = libcyc_kvm_get_station_from_kvmipm_id(cyc, *station);
                /*Christine Qiu
                  Task: Enhanced Authentication
                  doing: add wUser.uGroupName for kvm_ipconnect()
                */
                kvm_ipconnect(device, username, kvmip_req, http_host_str, station, cmd_output, "full", GetWindowNumber(wp), wUser.uGroupName);

	}
	return kCycSuccess;
}

int CycleStation(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t nparam;
	char_t *device;
	char_t *station;

	nparam = ejArgs(argc, argv, T("%s"), &station);
	if(nparam == 1){
		*station = libcyc_kvm_get_station_from_kvmipm_id(cyc, *station);
		KVMCycleStation(station);
	}
	return kCycSuccess;
}

int StationStatus(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t nparam;
	int station;
	char cmd[256], buf[256];
	FILE *fptr;

	nparam = ejArgs(argc, argv, T("%d"), &station);
	if(nparam == 1){
		station = libcyc_kvm_get_station_from_kvmipm_id(cyc, station);
		snprintf(cmd, 255, "/bin/kvm status -s %d", station);
		if((fptr = popen(cmd, "r"))){
			while(!feof(fptr)){
				if(fgets(buf, 255, fptr)){
					buf[strlen(buf)-1] = '|';
					websWrite(wp, buf);
				}
			}
			if((pclose(fptr) == -1)){
#if 0
/* always returns error... why? (setting signal SIGCHLD to SIG_DFL does not help) */
				websWrite(wp,
					"Error closing kvm status: %d %s",
					errno, strerror(errno));
#endif
			}
		}else{
			websWrite(wp, "Error calling kvm status: %d %s", errno, strerror(errno));
		}
	}else
		return mAppError(kWrongArgs);

	return kCycSuccess;
}

int StationVideoConf(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t nparam;
	int station, b, c;
	char_t buf[256], *a;
	FILE *fptr;

	memset(buf, 0, 256);

	nparam = ejArgs(argc, argv, T("%d %s %d %d"), &station, &a, &b, &c);
	if(nparam == 0 || station == 0) {
		snprintf(buf, 255, "/bin/kvm vidconf -S %d", 
			GetWindowNumber(wp));
	} else {
		station = libcyc_kvm_get_station_from_kvmipm_id(cyc, station);
		if(nparam < 4) {
			snprintf(buf, 255, "/bin/kvm vidconf -s %d", station);
		} else {
			snprintf(buf, 255, "/bin/kvm vidconf -s %d -A %s -B %d -C %d",
				station, a, b, c);
		}
	}
	if((fptr = popen(buf, "r"))){
		fread(buf, 255, 1, fptr);
		pclose(fptr);
	}
	ejSetResult(eid, buf);

	return kCycSuccess;
}


int PMSwitch(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t nparam;
	int station;
	char_t buf[256], *a;

	nparam = ejArgs(argc, argv, T("%d %s"), &station, &a);
	if(nparam == 2){
		station = libcyc_kvm_get_station_from_kvmipm_id(cyc, station);
		if(strcasecmp(a, "on") == 0 ||
				strcasecmp(a, "off") == 0 ||
				strcasecmp(a, "cycle") == 0){ 
			snprintf(buf, 255, "/bin/kvm power -s %d -a %s", station, a);
		}
		system(buf);
		sleep(4);
	}

	return kCycSuccess;
}

int StartUserInfo(struct _stinfo *stinfo) 
{
	int st = 0;
	for (st = 0; st < 4; st ++) {
		strcpy(stinfo[st].portname, "None");
		strcpy(stinfo[st].physname, "");
		strcpy(stinfo[st].username, "");
		stinfo[st].status = 0;
		strcpy(stinfo[st].conn, "");
		strcpy(stinfo[st].perm, "NONE");
		strcpy(stinfo[st].orig_perm, "NONE");
		stinfo[st].bright = 0;
		stinfo[st].contr = 0;
		strcpy(stinfo[st].outlet, "none");
		stinfo[st].cycle = 0;
		stinfo[st].list.nports = 0;
	}
}

int UserInfo(int eid, webs_t wp, int argc, char_t **argv)
{
	char *tmpfile = "/tmp/.tmpinfo";
	char_t cmd[256], buf[256], *t, *s;
	int st = 0, i;
	FILE *fptr;
	struct _stinfo stinfo[4];
	StartUserInfo(stinfo);

	st = 0;

	snprintf (cmd, 255, "/bin/kvm userinfo -S %d > %s", 
		GetWindowNumber(wp), tmpfile);
	system(cmd);
	if ((fptr = fopen(tmpfile, "r")) != NULL) {
		while (!feof(fptr)){
			if (! fgets(buf, 255, fptr)) {
				break;
			}
			if (strstr(buf, "[STATION] USER ")) {
				st = atoi(buf + 14) - 1;
				st = libcyc_kvm_get_kvmipm_id_from_station(cyc, st);
				if (st < 0) {
					st = 0;
				}
			} else if ((s = strstr(buf, "[PORT] ")) != NULL) {
				s += 7;
				t = s;
				while (*s && ! isspace(*s)) s++;
				if (*s) *s++ = 0;
				strncpy(stinfo[st].portname, t, 31);
			} else if ((s = strstr(buf, "[PHYS] ")) != NULL) {
				s += 7;
				t = s;
				while (*s && *s != '\n') s++;
				if (*s) *s++ = 0;
				strncpy(stinfo[st].physname, t, 31);
			} else if ((s = strstr(buf, "[USER] ")) != NULL) {
				s += 7;
				t = s;
				while (*s && *s != '\n') s++;
				if (*s) *s++ = 0;
				strncpy(stinfo[st].username, t, 63);
				for (i = 0; i < st; i ++) {
					if (!strcmp(stinfo[i].username, 
						stinfo[st].username)) {
						stinfo[st].list = 
							stinfo[i].list;
						break;
					}
				}
				if (i == st) {
                                       /*Christine Qiu
                                        Task: Enhanced Authentication
                                        doing: add wUser.uGroupName for KVMGetUserInfo()
                                        */
                                        KVMGetUserInfo(stinfo[st].username,
                                                &stinfo[st].list, wUser.uGroupName);

				}
			} else if ((s = strstr(buf, "[STATUS] ")) != NULL) {
				s += 9;
				t = s;
				while (*s && ! isspace(*s)) s++;
				if (*s) *s++ = 0;
				if (! strcmp(t, "NOK")) {
					stinfo[st].status = 0;
				} else if (! strcmp(t, "OK")) {
					stinfo[st].status = 1;
					t = s;
					while (*s && ! isspace(*s)) s++;
					if (*s) *s++ = 0;
					strncpy(stinfo[st].conn, t, 31);
				}
			} else if ((s = strstr(buf, "[PERM] ")) != NULL) {
				s += 7;
				t = s;
				while (*s && ! isspace(*s)) s++;
				if (*s) *s++ = 0;
				strncpy(stinfo[st].perm, t, 4);
				t = s;
				while (*s && ! isspace(*s)) s++;
				if (*s) *s++ = 0;
				strncpy(stinfo[st].orig_perm, t, 4);
			} else if ((s = strstr(buf, "[VIDEO] ")) != NULL) {
				s += 8;
				t = s;
				while (*s && ! isspace(*s)) s++;
				if (*s) *s++ = 0;
				stinfo[st].bright = atoi(t);
				t = s;
				while (*s && ! isspace(*s)) s++;
				if (*s) *s++ = 0;
				stinfo[st].contr = atoi(t);
			} else if ((s = strstr(buf, "[POWER] ")) != NULL) {
				s += 8;
				t = s;
				while (*s && *s != '\n') s++;
				if (*s) *s++ = 0;
				strncpy(stinfo[st].outlet, t, 255);
			} else if ((s = strstr(buf, "[CYCLE]")) != NULL) {
				stinfo[st].cycle = 1;
			}
		}
		fclose(fptr);
	} else {
		printf("Error opening pipe for command %s (NULL)\n", cmd);
	}
	unlink(tmpfile);

	/* Check for status changes */
	for (st = 0; st < 4; st ++) {
		if (strcmp(old_stinfo[st].portname, "None") && 
			! strcmp(stinfo[st].portname, "None")) {
			strcpy(stinfo[st].message, "Disconnected.");
		} else if (! strcmp(old_stinfo[st].portname, "None") && 
			strcmp(stinfo[st].portname, "None")) {
			strcpy(stinfo[st].message, "Connection started.");
		} else if (! old_stinfo[st].cycle && stinfo[st].cycle) {
			strcpy(stinfo[st].message, "Cycle started.");
		} else if (old_stinfo[st].cycle && !stinfo[st].cycle) {
			strcpy(stinfo[st].message, "Cycle stopped.");
		} else if (strcmp(old_stinfo[st].portname, 
			stinfo[st].portname)) {
			sprintf(stinfo[st].message, "Server changed (%s => %s).", old_stinfo[st].portname, stinfo[st].portname);
		} else if (old_stinfo[st].status && !stinfo[st].status) {
			strcpy(stinfo[st].message, "Connection lost.");
		} else if (!old_stinfo[st].status && stinfo[st].status) {
			strcpy(stinfo[st].message, "Connection established.");
		} else if (strcmp(old_stinfo[st].perm, stinfo[st].perm)) {
			sprintf(stinfo[st].message, "Permission changed (%s => %s).", old_stinfo[st].perm, stinfo[st].perm);
		} else if (strcmp(old_stinfo[st].outlet, stinfo[st].outlet)) {
			snprintf(stinfo[st].message, 256, "Power status changed (%s => %s)", 
				old_stinfo[st].outlet, stinfo[st].outlet);
		} else {
			strcpy(stinfo[st].message, "None");
		}
	}

	websWrite(wp, "var port = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].portname, stinfo[1].portname, 
		stinfo[2].portname, stinfo[3].portname);
	websWrite(wp, "var phys = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].physname, stinfo[1].physname, 
		stinfo[2].physname, stinfo[3].physname);
	websWrite(wp, "var username = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].username, stinfo[1].username, 
		stinfo[2].username, stinfo[3].username);
	websWrite(wp, "var status = new Array (%d, %d, %d, %d);\n",
		stinfo[0].status, stinfo[1].status, 
		stinfo[2].status, stinfo[3].status);
	websWrite(wp, "var conn = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].conn, stinfo[1].conn, 
		stinfo[2].conn, stinfo[3].conn);
	websWrite(wp, "var perm = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].perm, stinfo[1].perm, 
		stinfo[2].perm, stinfo[3].perm);
	websWrite(wp, "var orig_perm = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].orig_perm, stinfo[1].orig_perm, 
		stinfo[2].orig_perm, stinfo[3].orig_perm);
	websWrite(wp, "var bright = new Array (%d, %d, %d, %d);\n",
		stinfo[0].bright, stinfo[1].bright, 
		stinfo[2].bright, stinfo[3].bright);
	websWrite(wp, "var contr = new Array (%d, %d, %d, %d);\n",
		stinfo[0].contr, stinfo[1].contr, 
		stinfo[2].contr, stinfo[3].contr);
	websWrite(wp, "var outlet = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].outlet, stinfo[1].outlet, 
		stinfo[2].outlet, stinfo[3].outlet);
	websWrite(wp, "var cycle = new Array (%d, %d, %d, %d);\n",
		stinfo[0].cycle, stinfo[1].cycle, 
		stinfo[2].cycle, stinfo[3].cycle);
	websWrite(wp, "var msg = new Array ('%s', '%s', '%s', '%s');\n",
		stinfo[0].message, stinfo[1].message, 
		stinfo[2].message, stinfo[3].message);

	/* Check for port lists */
	websWrite(wp, "var portlist = new Array (4);\n");
	for (st = 0; st < 4; st ++) {
		websWrite(wp, "portlist[%d] = new Array (", st);
		for (i = 0; i < stinfo[st].list.nports; i ++) {
			websWrite(wp, "'%s'", stinfo[st].list.port[i].name);
			if (i < stinfo[st].list.nports - 1) {
				websWrite(wp, ",");
			}
		}
		websWrite(wp, ");\n");
	}
	for (st = 0; st < 4; st ++) {
		old_stinfo[st] = stinfo[st];
	}

	return kCycSuccess;
}

int ResyncServer(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t nparam;
	int station;
	char_t buf[256];
	nparam = ejArgs(argc, argv, T("%d"), &station);
	if(nparam >= 1){
		station = libcyc_kvm_get_station_from_kvmipm_id(cyc, station);
		snprintf (buf, 255, "/bin/kvm resync -s %d", station);
		system(buf);
	}
	return kCycSuccess;
}

#endif /* KVM */

extern int GetMenuShellInfo(Param* param, int request)
{
	UInt16 x = 0, size;
	Result result; 

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadMenuShellInfo, kMenuShell, &gCache.menuShell, &gCacheSave.menuShell, sizeof(MenuShell)); 

   if (kCycSuccess == result)
   {
	   /* Fill in menuShellHtml */
	   char_t templateStr[] = "<option value=\"%s!%s\">";
	   char_t templateFilled[300] = {0};
	   MenuShOpt *menuOpt = gCache.menuShell.menuShInfo.optInfo;
	   
	   //strcpy(gCache.menuShell.menuTitle, menu->menuTitle);
	   if (gCache.menuShell.menuOptionHtml) {
	   	bfreeSafe(B_L, gCache.menuShell.menuOptionHtml);
	   	gCache.menuShell.menuOptionHtml = 0;
	   }
	   if (gCache.menuShell.menuShInfo.numOptions) 
	   {
		   size = sizeof(templateFilled) * (gCache.menuShell.menuShInfo.numOptions);
		   if (!(gCache.menuShell.menuOptionHtml = balloc(B_L, size)))
			   return mAppError(kMemAllocationFailure); //mp
		   memset(gCache.menuShell.menuOptionHtml, 0, size);
		   for (x = 0; x < gCache.menuShell.menuShInfo.numOptions; x++, menuOpt++)
		   {
			   gsprintf(templateFilled, templateStr, menuOpt->title, menuOpt->command);
			   gstrcat(gCache.menuShell.menuOptionHtml, templateFilled);
			   includeNbsp(templateFilled, menuOpt->title, 18);
			   gstrcat(gCache.menuShell.menuOptionHtml, templateFilled);
			   gstrcat(gCache.menuShell.menuOptionHtml, "&nbsp;&nbsp;");
			   includeNbsp(templateFilled, menuOpt->command, 20);
			   gstrcat(gCache.menuShell.menuOptionHtml, templateFilled);
			   gstrcat(gCache.menuShell.menuOptionHtml, "&nbsp;&nbsp;</option>\n");
		   }
		   gstrcat(gCache.menuShell.menuOptionHtml, "<option value=\"-1\" selected></option>\n");
	   } else { 
		   // Put in blank spaces into Html pointer so that it will at least fill up option box
		   if (!(gCache.menuShell.menuOptionHtml = balloc(B_L, sizeof(templateFilled))))
			   return mAppError(kMemAllocationFailure); //mp
		   memset(gCache.menuShell.menuOptionHtml, 0, sizeof(templateFilled));
		   gstrcat(gCache.menuShell.menuOptionHtml, "<option value=\"-1\" selected>");
		   includeNbsp(templateFilled, " ", 38);
		   gstrcat(gCache.menuShell.menuOptionHtml, templateFilled);
		   gstrcat(gCache.menuShell.menuOptionHtml, "</option>\n");
	   }
   }
   return mAppError(result);
}

/* Get the info for a specific menuoption chosen to be edited */
extern int GetMenuShAddModOption(Param* param, int request)
{
   /* Get the group # */
   UInt32 option = -1;
   char_t* temp = NULL;

   if (kDmfSuccess == dmfGetParam(kSel1Key, NULL, &temp, -1))
   {
      option = gatoi(temp);
      bfreeSafe(B_L, temp);
   }
   /* Set the vars visibly for an edit (only) */
   memset(&gCache.menuShell.addModOption, 0, sizeof(MenuShOpt));

   if (0 <= option && option < gCache.menuShell.menuShInfo.numOptions)
   {
      MenuShOpt m = gCache.menuShell.menuShInfo.optInfo[option];
		memcpy(&gCache.menuShell.addModOption, &m, sizeof(MenuShOpt));
   }
   return mAppError(kCycSuccess);
}

extern int GetNotifications(Param* param, int request)
{
   int nadd = -2;

   Result result; 

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   result = LoadFirst(LoadNotifications, kNotifications,
            &gCache.notifications, &gCacheSave.notifications,
            sizeof(Notifications));
   if (kCycSuccess == result){
      /* Fill in serversHtml */
      char_t tmpStr[] = "<option value=\"%d\">%s&nbsp;&nbsp;%.20s </option>\n";
      char_t *notifStr[] = {"Email&nbsp;&nbsp;&nbsp;&nbsp;","Pager&nbsp;&nbsp;&nbsp;&nbsp;","SNMP Trap"};
      char_t buffer[100];
      NotificationInfo * notif = gCache.notifications.notif;
      int i ;

      bfreeSafe(B_L, gCache.notifications.notifHtml);
      bfreeSafe(B_L, gCache.notifications.triggerHtml);
      gCache.notifications.notifHtml = 0;
      gCache.notifications.triggerHtml = 0;
      if (gCache.notifications.numNotif) {
         i= sizeof(buffer) * (gCache.notifications.numNotif);
         if (!(gCache.notifications.notifHtml = balloc(B_L, i)))
			 return mAppError(kMemAllocationFailure); //mp
         memset(gCache.notifications.notifHtml,0,i);
         if (!(gCache.notifications.triggerHtml = balloc(B_L, i)))
			 return mAppError(kMemAllocationFailure); //mp
         memset(gCache.notifications.triggerHtml,0,i);
      
         for (i=0; i < gCache.notifications.numNotif; i++,notif++) {
            if (notif->action == kDelete) continue;
            if (notif->action == kAdd) {
               notif->triggerId = nadd;
               nadd -= 1;
            }
            sprintf(buffer,tmpStr,notif->triggerId,notifStr[notif->type],notif->trigger);
            gstrcat(gCache.notifications.notifHtml, buffer);
      
            gstrcat(gCache.notifications.triggerHtml, "<option>");
            gstrcat(gCache.notifications.triggerHtml, notif->trigger);
            gstrcat(gCache.notifications.triggerHtml, "</option>\n");
         }
      }
      gCache.notifications.triggerId = -1;
      gCache.notifications.notifType = -1;
   }
   return mAppError(result);
}

extern int GetEventsNotif(Param* param, int request)
{
   NotificationInfo * notif;
   char i ;
   Result result = LoadFirst(LoadNotifications, kNotifications,
            &gCache.notifications, &gCacheSave.notifications,
            sizeof(Notifications));

   if (kCycSuccess == result){
      if (gCache.notifications.triggerId == -1) { // add operation ?
         if (gCache.notifications.notifType == -1) { // error
            return kDmfParameterName;
         }
         memset(&gCache.notifications.event,0,sizeof(Events));
         memset(gCache.notifications.trigger,0,kTriggerLength);
         return mAppError(result);
      }
      notif = gCache.notifications.notif;
     for (i=0; i < gCache.notifications.numNotif; i++,notif++) {
     	if (notif->action == kDelete) continue;
        if ((gCache.notifications.triggerId == notif->triggerId)) {
            break;
         }
      }
      if (i == gCache.notifications.numNotif)
         return kDmfParameterName;

      gCache.notifications.notifType = notif->type;
      memcpy(gCache.notifications.trigger, notif->trigger,kTriggerLength);
      memcpy(&gCache.notifications.event, &notif->event,sizeof(Events));
      
	switch (notif->type) {
		case kNtEmail :
			ConvTextArea(notif->event.email.body,gCache.notifications.event.email.body,0);
			break;
		case kNtPager :
			ConvTextArea(notif->event.pager.text,gCache.notifications.event.pager.text,0);
			break;
		case kNtSnmpTrap :
			ConvTextArea(notif->event.snmptrap.body,gCache.notifications.event.snmptrap.body,0);
			break;
	}

   }
   return mAppError(result);
}

/****************************************************/
/* EXPERT -> PORTS  */
/****************************************************/

// Physical Ports -> Html Table
void GetSyslogBufServer()
{
	CycParam values[] =
	{
		{ kSyslogNg, kSyslogBufServer, &gCache.sysBufServer, 1 },
	};
	memset(gCache.sysBufServer.SyslogBufServer,0,kServerLength);
	GetCycParamValues(values, mArrayLength(values));
	gCacheSave.sysBufServer = gCache.sysBufServer;
}

#define MAXPhysLine	400
extern int GetPhysTableHtml(Param* param, int request)
{
	int size = MAXPhysLine * gCache.device.number;
	int i;
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	if (!gCacheLoaded[kPortPhysTable]) {  // The page was even loaded
#ifdef IPMI
		LoadFirst(LoadPmIpmi, kPmIpmiPage, &gCache.pmipmi, NULL, 0);
#endif
		bfreeSafe(B_L, gCache.physP.physList);
		i=  (gCache.device.number+1) * sizeof(PhysPortsInfo);
#ifdef PMDNG
		LoadpmdPortInfo();
		LoadpmdGroups();
		LoadpmdUsers();
#endif
		if (!(gCache.physP.physList = (PhysPortsInfo *)balloc(B_L, i)))
			return mAppError(kMemAllocationFailure); //mp
		memset((char *)gCache.physP.physList, 0, i);
		portGetPortConfiguration();
		gCacheLoaded[kPortPhysTable] = true;
		GetSyslogBufServer();
	}
	
	if (!gCache.physP.physTableHtml) {
		if (!(gCache.physP.physTableHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
	}
	memset(gCache.physP.physTableHtml, 0, size);
	portMountPhysPortHtml(gCache.physP.physTableHtml);
	return mAppError(kCycSuccess);
}

// Physical Ports -> Port Configuration 
extern int GetPhysPConf(Param* param, int request)
{
	PhysPortsInfo *phys = gCache.physP.physList;
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	if (!gCacheLoaded[kPortPhysTable]) 
		return mAppError(0);

	if (gCache.physP.action == 0) {
		return kCycSuccess;
	}

	if (gCacheLoaded[kPortPhysConf] == false)
	{
		phys += gPorts[0];
		memcpy((char*)&gCache.physP.infoP,(char*)phys,sizeof(PhysPortsInfo));
		gCacheLoaded[kPortPhysConf] = true;
		phys = &gCache.physP.infoP;
		if (phys->other.loginBanner[0]) {
			ConvTextArea(phys->other.loginBanner, phys->other.loginBanner,0);
		}
		if (phys->ts.host[0]) {
			ConvTextArea(phys->ts.host, phys->ts.host , 0);
		}
	}
	return mAppError(kCycSuccess);
}

#ifdef IPMI
// Physical Ports -> Power Management --> IPMI devices list 
static void GetPhysIpmiDeviceListHtml(void)
{
	char_t temp[]="<option value=%d> %s </option>\n";
	char_t temp1[]="<option value=%d selected> %s </option>\n";
	char_t buf[200];
	IPMIDeviceInfo *pdev;
	int i,size,id;

	if (gCache.pmipmi.numIpmiDev) {
		id = gCache.physP.infoP.powerMgm.ipmiDeviceId;
		size = gCache.pmipmi.numIpmiDev * sizeof(buf);
		/*warning: `return' with a value, in function returning void
		if (!(gCache.physP.ipmiDeviceListHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp*/
		if ((gCache.physP.ipmiDeviceListHtml = balloc(B_L, size)))
		{
			memset(gCache.physP.ipmiDeviceListHtml, 0, size);
			for (i=0,pdev=gCache.pmipmi.ipmiInfo; i < gCache.pmipmi.numIpmiDev ; i++,pdev++) 
			{
				if (pdev->action != kDelete) 
				{
					if (id == pdev->deviceId) 
						sprintf(buf,temp1,pdev->deviceId,pdev->alias);
					else 
						sprintf(buf,temp,pdev->deviceId,pdev->alias);

					gstrcat(gCache.physP.ipmiDeviceListHtml,buf);
				}
			}
		}
	} else {
		bfreeSafe(B_L, gCache.physP.ipmiDeviceListHtml);
		gCache.physP.ipmiDeviceListHtml = 0;
	}
	//return mAppError(kCycSuccess); warning: `return' with a value, in function returning void
}
#endif

// Physical Ports -> Outlets Html
extern int GetPhysPmOutletHtml(Param* param, int request)
{
#ifdef IPMI
	GetPhysIpmiDeviceListHtml();
#endif
	return mAppError(kCycSuccess);
}

// Virtual Ports Table
extern int GetVirtualPortsInfo(Param* param, int request)
{
   Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   result = LoadFirst(LoadVirtualPorts, kVirtualPorts,&gCache.virtualP,&gCacheSave.virtualP,sizeof(VirtualPorts));

   if (kCycSuccess == result){
      char_t tmpStr[] = "<option value=\"%s\">%s</option>\n";
      char_t buffer[200];
      VirtualPortsInfo *virtual = gCache.virtualP.virtualInfo;
      int i ;

      bfreeSafe(B_L, gCache.virtualP.virtualInfoHtml);
      gCache.virtualP.virtualInfoHtml = 0;
      if (gCache.virtualP.numVirtual) {
         i= sizeof(buffer) * (gCache.virtualP.numVirtual);
         if (!(gCache.virtualP.virtualInfoHtml = balloc(B_L, i)))
			 return mAppError(kMemAllocationFailure); //mp
         memset(gCache.virtualP.virtualInfoHtml,0,i);
         for (i=0; i < gCache.virtualP.numVirtual; i++,virtual++) {
            if (virtual->action != kDelete) {
               sprintf(buffer,tmpStr,virtual->remoteIp,virtual->remoteIp);
               gstrcat(gCache.virtualP.virtualInfoHtml, buffer);
            }
         }
      }
   }
   return mAppError(result);
}

// Virtual Port - POP UP EDIT
extern int GetVirtualPort(Param* param, int request)
{
   VirtualPortsInfo *virtual;
   char i ;
   Result result = LoadFirst(LoadVirtualPorts, kVirtualPorts,&gCache.virtualP,&gCacheSave.virtualP,sizeof(VirtualPorts));

   if (kCycSuccess == result){
      if (gCache.virtualP.action == kAdd) { // add operation ?
         gCache.virtualP.virtualPort.action = kAdd;
		memset(&gCache.virtualP.virtualPort,0,sizeof(VirtualPortsInfo));
                   return mAppError(result);
                }
      // edit operation
                virtual = gCache.virtualP.virtualInfo;
                for (i=0; i < gCache.virtualP.numVirtual; i++,virtual++) {
         if (!(strcmp(gCache.virtualP.remoteIp,virtual->remoteIp))) {
            break;
         }
      }
                if (i == gCache.virtualP.numVirtual)
         return kDmfParameterName;

      memcpy(&gCache.virtualP.virtualPort, virtual,sizeof(VirtualPortsInfo));
   }
   return mAppError(result);
}



/****************************************************/
/* EXPERT -> NETWORK */
/****************************************************/

#ifdef IPSEC
extern int GetVPNEntry(Param* param, int request)
{
	/* Get the group # */
	UInt32 Index = -1;
	char_t* temp = NULL;

	if (kDmfSuccess == dmfGetParam(kSel1Key, NULL, &temp, -1)) {
		Index = gatoi(temp);
		bfreeSafe(B_L, temp);
	}

	if (Index >= 0 && Index < gCache.VPNTable.numVPN) {
		gCache.VPNTable.currEntry = Index;
		memcpy(&gCache.VPNEntry, &gCache.VPNTable.VPN[Index], sizeof(IpSecInfo));
	} else {
		gCache.VPNTable.currEntry = -1;
		memset(&gCache.VPNEntry, 0, sizeof(IpSecInfo));
	}

// For new entries (or with RSA local not defined), show the pubkey available in ipsec.secrets on left (local), so the user need to config only the remote side.
	if (*gCache.VPNEntry.leftRsaKey == 0) {
		strncpy(gCache.VPNEntry.leftRsaKey, gCache.VPNTable.pubRsaKey, kRsaKeyLength-1);
	}
	return mAppError((Result)kCycSuccess);
}

static int SetVPNTable(void)
{
    /* Register changes */
	if (gCacheLoaded[kVPNTablePage] == false) {
		return(0);
	}

    if (gCache.VPNTable.VPNChanged == 1) {
        if (gReq.action != kNone) {
            RegisterParamChange(0, gCache.VPNTable.VPN, kIpSec, kIpSecInfo, gCache.VPNTable.numVPN);
        }
        gLedState++;
    }
    return (0);
}

extern int VPNEntryCommit (Param* param, int request, char_t* urlQuery)
{
	Result result;
	int index = gCache.VPNTable.currEntry;
	int size;
	char *new;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	handle_VPN_delete();

	if (index < 0) { // add
		size=sizeof(IpSecInfo) * (gCache.VPNTable.numVPN);
		new = balloc(B_L, size + sizeof(IpSecInfo));
		if (new == NULL) {
			return mAppError(kMemAllocationFailure);
		}

		memcpy(new, gCache.VPNTable.VPN,size);
		bfreeSafe(B_L, gCache.VPNTable.VPN);
		/* warning: assignment from incompatible pointer type
                gCache.VPNTable.VPN = new;
                */
                gCache.VPNTable.VPN = (IpSecInfo *)new;
		index = gCache.VPNTable.numVPN++;
		memcpy(&gCache.VPNTable.VPN[index], &gCache.VPNEntry, sizeof(IpSecInfo));
		gCache.VPNTable.VPN[index].action = kAdd;
    	gCache.VPNTable.VPNChanged = 1;
        gLedState++;
	} else {
		if (index >= 0 && index < gCache.VPNTable.numVPN) {
			if (memcmp(&gCache.VPNTable.VPN[index], &gCache.VPNEntry, sizeof(IpSecInfo))) {
				memcpy(&gCache.VPNTable.VPN[index], &gCache.VPNEntry, sizeof(IpSecInfo));
				gCache.VPNTable.VPN[index].action = kEdit;
    			gCache.VPNTable.VPNChanged = 1;
        		gLedState++;
			}
		} else {
			result = kSystemError;
		}
	}

	if (result == kCycSuccess) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
	}

	return mAppError(result);
}

Result LoadVPNTable(void)
{
	int size;
	Result result;
	CycParam counts[] =
	{
		{ kIpSec, kNumIpSec, &(gCache.VPNTable.numVPN), 0 },
		{ kIpSec, kIpSecPubKey, gCache.VPNTable.pubRsaKey, 0 },
	};

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
	bfreeSafe(B_L, gCache.VPNTable.VPN);
	if (gCache.VPNTable.numVPN) {
		size=sizeof(IpSecInfo) * (gCache.VPNTable.numVPN);
		gCache.VPNTable.VPN = balloc(B_L, size);
		if (gCache.VPNTable.VPN == NULL) {
			return kMemAllocationFailure;
		}

		memset(gCache.VPNTable.VPN,0,size);
		{
			CycParam values[] =
			{
				{ kIpSec, kIpSecInfo, gCache.VPNTable.VPN, gCache.VPNTable.numVPN},
			};
			result = GetCycParamValues(values, mArrayLength(values));
		}
	}
	return result;
}

#if 0
static int CBitMask(char *str_mask)
{
	int mask;
	int i, j;

	mask = inet_addr(str_mask);

	for(i=32, j=1; i > 0; i--, j<<=1) {
		if (j & mask) break;
	}

	return(i);
}
#endif

// VPN Table
extern int GetVPNTable(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadVPNTable, kVPNTablePage,&gCache.VPNTable,NULL,0);

	if (kCycSuccess == result){
		/* Fill in VPNHtml */
		//static char_t tmpStr[] = "<option value=\"%d\">";
		char_t buffer[500];
		IpSecInfo * pIp = gCache.VPNTable.VPN;
		int i ;

		bfreeSafe(B_L, gCache.VPNTable.VPNTableHtml);
		gCache.VPNTable.VPNTableHtml = 0;
		i =sizeof(buffer) * (gCache.VPNTable.numVPN + 1);
		if (!(gCache.VPNTable.VPNTableHtml = balloc(B_L, i)))
			return mAppError(kMemAllocationFailure); //mp
		memset(gCache.VPNTable.VPNTableHtml,0,i);
		gstrcat(gCache.VPNTable.VPNTableHtml, "[");
		for (i=0; i < gCache.VPNTable.numVPN; i++,pIp++) {
			if (pIp->action == kDelete) {
				continue;
			}
			sprintf(buffer, "[\"%s\",\"%s\",\"%s\"],", pIp->connectionName, pIp->rightSubnet, pIp->leftSubnet);
			gstrcat(gCache.VPNTable.VPNTableHtml, buffer);
			/*sprintf(buffer,tmpStr,i);
			gstrcat(gCache.VPNTable.VPNTableHtml, buffer);
			strncpy(buffer,pIp->connectionName, 24);
			buffer[24] = 0;
			includeNbsp(buffer,buffer,24);
			gstrcat(gCache.VPNTable.VPNTableHtml, buffer);
			strcpy(buffer,pIp->rightSubnet);
			includeNbsp(buffer,buffer,20);
			gstrcat(gCache.VPNTable.VPNTableHtml, buffer);
			strcpy(buffer,pIp->leftSubnet);
			gstrcat(gCache.VPNTable.VPNTableHtml, buffer);
			gstrcat(gCache.VPNTable.VPNTableHtml, "</option>\n");*/
		}
		gstrcat(gCache.VPNTable.VPNTableHtml, "]");
		/*gstrcat(gCache.VPNTable.VPNTableHtml, "<option value=\"-1\" selected>");
		includeNbsp(buffer, " ", 64);
		gstrcat(gCache.VPNTable.VPNTableHtml, buffer);
		gstrcat(gCache.VPNTable.VPNTableHtml, "</option>\n");*/
	}
	return mAppError(result);
}

extern void handle_VPN_delete(void)
{
	int index;
	char *pentry;

	if (memcmp(gCache.VPNTable.VPNTableDel, "Deleted=",8) == 0) {
		pentry = gCache.VPNTable.VPNTableDel + 8;
		while ((pentry=strtok(pentry, ";")) != NULL) {
			index = atoi(pentry);
			pentry = NULL;
			if (index < 0 || index > gCache.VPNTable.numVPN) {
				continue;
			}
			gCache.VPNTable.VPN[index].action = kDelete;
			gCache.VPNTable.VPNChanged = 1;
			gLedState++;
		}
		memset(gCache.VPNTable.VPNTableDel, 0, kUserListLength);
	}
}

extern int VPNTableCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;

	//[RK]Feb/23/05 
	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kVPNTablePage)) return mAppError(result);
	
	handle_VPN_delete();

	if (gReq.action == kNone) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	// If it was just a menu navigation does not commit anything

	return expertCommit(param, request, urlQuery);

}
#endif

#ifdef PMDNG
unsigned int getPortsIpdusSignature(int numPorts, PmIpduPortInfo *port)
{
	unsigned int signature = 0;
	PmIpduInfo *ipdu;
	int i, j, k;

	for (i=1; i<=numPorts; i++, port++) {
//syslog(4,"%s: i[%d] serial[%d] numI[%d]",__func__,i,port->serialPort,port->numIpdu);
		signature += (i * port->serialPort);
		signature += (i * port->numIpdu);
		ipdu = port->ipdu;
		for (j=1; ipdu && j<=port->numIpdu; j++, ipdu++) {
//syslog(4,"%s: i[%d] j[%d] pdu#[%d] pduI[%s] model[%s] outlets[%d]",__func__,i,j,ipdu->ipduNumber,ipdu->name,ipdu->model,ipdu->outlets);
			for (k=0; k<strlen(ipdu->name); k++)
				signature += ipdu->name[k];
			for (k=0; k<strlen(ipdu->model); k++)
				signature += ipdu->model[k];
			signature += (i * j * ipdu->ipduNumber);
			signature += (i * j * ipdu->outlets);
		}
	}

//syslog(4,"%s: signature[%d]",__func__,signature);
	return signature;
}

unsigned int getGroupsSignature(int numGroups, PmIpduOutGrpInfo *grpinfo, PMDOutGrpsDef *grpdef)
{
	unsigned int signature = 0;
	int i, j;

	if (grpinfo != NULL && grpdef != NULL) {
		for (i=1; i<=numGroups; i++, grpinfo++, grpdef++) {
			signature += (i * grpinfo->numOutlets);
			for (j=0; j<strlen(grpdef->group); j++)
				signature += grpdef->group[j];
			for (j=0; j<strlen(grpdef->outletList); j++)
				signature += grpdef->outletList[j];
		}
	}

	return signature;
}

unsigned int checkPortsIpdusSignature()
{
	unsigned int signature = 0;
	int size, i, numPorts;
	PmIpduPortInfo  *port, *auxport = NULL;
	PmIpduInfo      *ipdu;
	CycParam values[] =
		{
			{ kPowerMgm, kNumPorts, &numPorts, 0 },
		};

	if (GetCycParamValues(values, mArrayLength(values)) != kCycSuccess)
		return 0;

	if (numPorts) {
		size = sizeof(PmIpduPortInfo) * numPorts;
		if (!(port = malloc(size)))
			return 0; //mp
		auxport = port;
		memset(port,0,size);
		{
			CycParam values[] =
				{
					{ kPowerMgm, kPortInfo, port, numPorts },
				};

			if (GetCycParamValues(values, mArrayLength(values)) != kCycSuccess)
				goto ports_ipdus_free_end;
		}

		for (i=0; i<numPorts; i++, port++) {
			if (port->numIpdu) {
				size = sizeof(PmIpduInfo) * port->numIpdu;
				if (!(port->ipdu = malloc(size)))
					goto ports_ipdus_free_end;
				memset(port->ipdu,0,size);
				{
					CycParam values[] =
						{
							{ kPowerMgm, kIpduUnitInfo, port->ipdu, port->numIpdu},
						};
					port->ipdu->serialPort = port->serialPort;
					port->ipdu->ipduNumber = 0; //All

					strcpy(port->ipdu->username, PMDngUserName);
					if (GetCycParamValues(values, mArrayLength(values)) != kCycSuccess)
						goto ports_ipdus_free_end;
				}
			}
		}

		signature = getPortsIpdusSignature(numPorts, auxport);
	}

ports_ipdus_free_end:
	port = auxport;
	while (numPorts) {
		ipdu = port->ipdu;
		while (port->numIpdu) {
			if (ipdu->firstPhaseInfo) free(ipdu->firstPhaseInfo);
			if (ipdu->firstBankInfo) free(ipdu->firstBankInfo);
			if (ipdu->firstEnvMonSensor) free(ipdu->firstEnvMonSensor);
			ipdu++;
			port->numIpdu--;
		}
		free(port->ipdu);
		port++;
		numPorts--;
	}
	free(auxport);

	if (signature)
		return signature;
	else
		return 0;
}

unsigned int checkGroupsSignature()
{
	unsigned int signature = 0;
	int numPorts=0, totalGroups=0, size, i;
	PmIpduOutGrpInfo  *group, *auxgroup = NULL;
	PMDOutGrpsDef     *grpdef, *auxgrpdef = NULL;
	CycParam values[] =
		{
			{ kPowerMgm, kNumPorts, &numPorts, 0 },
			{ kPMDConfig, kPMDConfnumOutGrps, &totalGroups, 0 },
		};

	if (GetCycParamValues(values, mArrayLength(values)) != kCycSuccess)
		return 0;

	if (numPorts) {
		if (totalGroups) {
			size = sizeof(PmIpduOutGrpInfo) * totalGroups;
			if (!(group = malloc(size)))
				return 0;
			auxgroup = group;
			memset(group,0,size);
			size = sizeof(PMDOutGrpsDef) * totalGroups;
			if (!(grpdef = malloc(size)))
				goto groups_free_end;
			auxgrpdef = grpdef;
			memset(grpdef,0,size);
			{
				CycParam values[] =
					{
						{ kPMDConfig, kPMDOutGrpsDef, grpdef, totalGroups },
					};

				if (GetCycParamValues(values, mArrayLength(values)) != kCycSuccess)
					goto groups_free_end;
			}
			{
				CycParam values[] =
					{
						{ kPowerMgm, kOutGrpsInfo, group, totalGroups },
					};

				strcpy(group->username, PMDngUserName);
				for (i=0; i<totalGroups; i++, group++, grpdef++) {
					strncpy(group->name+1, grpdef->group, kPMDGroupNameLength);
					group->name[0]='$';
				}

				if (GetCycParamValues(values, mArrayLength(values)) != kCycSuccess)
					goto groups_free_end;
			}

			signature = getGroupsSignature(totalGroups, auxgroup, auxgrpdef);
		}
	}

groups_free_end:
	group = auxgroup;
	if ( group != NULL ) {
		while (totalGroups) {
			free(group->outletInfo);
			group++;
			totalGroups--;
		}
		free(auxgroup);
	}
	if ( auxgrpdef != NULL ) {
		free(auxgrpdef);
	}

	if (signature)
		return signature;
	else
		return 0;
}

#define PMD_SIZE_INFO_IPDU  1500
#define PMD_SIZE_OUTLET  150
#define PMD_SIZE_PORT  100

// Power Management - New Outlets Manager page
extern int GetIpduOutMan(Param* param, int request)
{
	Result result;
	char_t buffer[200];
	PmIpduPortInfo *port;
	PmIpduInfo     *ipdu;
	PmOutletInfo   *outlet;
	int size, i, j, totalOuts,nipdus;
	int flg_cyc;
	int portindex, ipduindex;
	
	gCacheLoaded[kIPDUOutletsManager] = true;
//	printf("-----> GetIpduOutMan()\n");

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess) 
		return result;

//syslog(4,"%s: showIpdu[%d]",__func__,wUser.showIpdu);
	//[RK]Nov/03/06 - set the username
	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);

	checkPMDguardtime(time(NULL));

	gCache.IpduOutMan.showIpdu = wUser.showIpdu;

	if (wUser.showIpdu) { // Second Level page
		if (gCache.IpduOutMan.memSignature != checkPortsIpdusSignature()) {
			gCache.IpduOutMan.action = no_Action;
			strcpy(gCache.IpduOutMan.sysMsg, "ACS detected changes on IPDU configuration.\\n"
				   "Please try the last operation again...");
			CycSetshowIpdu(&gReq.sid,0);
			return kCycSuccess;
		}
		portindex = wUser.showIpdu / 1000;
		ipduindex = wUser.showIpdu % 1000 - 1;
		
//syslog(4,"%s: showIpdu[%d] port[%d] ipdu[%d]",__func__,wUser.showIpdu,portindex,ipduindex);
		if ((result = LoadIpduOutMan2nd(portindex,ipduindex)) == kCycSuccess) {
			totalOuts = gCache.IpduOutMan.ipduOutInfo.numOutlets;
			ipdu = &(gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex]);

			gCache.IpduOutMan.ipduNofOut = totalOuts;
			sprintf(gCache.IpduOutMan.ipduName_str, "%s (%s port %d)",
					ipdu->name,
					"serial",
					gCache.IpduOutMan.portInfo[portindex].serialPort);
			sprintf(gCache.IpduOutMan.ipduModel_str, "%s %s",
					ipdu->vendor, ipdu->model);
			gCache.IpduOutMan.isServerTech = !strcmp(ipdu->vendor, "ServerTech");
			if ((ipdu->type == Vendor_cyclades) && !(ipdu->cap & IPDU_CAP_NOT_SWITCHABLE)) {
				flg_cyc=1;
			} else {
				flg_cyc=0;
			}

			bfreeSafe(B_L, gCache.IpduOutMan.outletInfoHtml);
			gCache.IpduOutMan.outletInfoHtml = NULL;
			bfreeSafe(B_L, gCache.IpduOutMan.outcapInfoHtml);
			gCache.IpduOutMan.outcapInfoHtml = NULL;

			//size = sizeof(buffer) * (totalOuts ? totalOuts : 1);
			size = PMD_SIZE_OUTLET * (totalOuts ? totalOuts : 1);
			// the calculation of the size to be allocated is approximate... 
			if (!(gCache.IpduOutMan.outletInfoHtml = balloc(B_L, size)))
				return mAppError(kMemAllocationFailure); //mp
			if (!(gCache.IpduOutMan.outcapInfoHtml = balloc(B_L, size)))
				return mAppError(kMemAllocationFailure); //mp

			// Fill in outletInfoHtml and outcapInfoHtml
			if (!totalOuts) {
				strcpy(gCache.IpduOutMan.outletInfoHtml, "'no-Outlets'");
				strcpy(gCache.IpduOutMan.outcapInfoHtml, "'no-Outlets'");
			} else {
				strcpy(gCache.IpduOutMan.outletInfoHtml, "[");
				strcpy(gCache.IpduOutMan.outcapInfoHtml, "[");
				outlet = gCache.IpduOutMan.ipduOutInfo.outletInfo;
				for (j=0; j<totalOuts; j++, outlet++) {
					sprintf(buffer, "[%d, \"%s\", %d, %d, %d, %d, \"%.1f\", %d, %d, \"%.1f\", \"%.1f\",  \"%s\",  \"%.1f\", [\"%.1f\",  \"%.1f\",  \"%.1f\", \"%.1f\"]],",
							outlet->number,
							outlet->name,
							outlet->flgOnOff,
							outlet->flgLock,
							outlet->minOnTime,
							outlet->minOffTime,
							outlet->postOnDelay,
							outlet->wakeupState,
							outlet->flagName,
							//[RK]May/2009 Zeus
							(double)outlet->current/10,
							(double)outlet->power/10,
							outlet->alarm,
							outlet->postOffDelay,
							(double)outlet->threshold_hc/10,
							(double)outlet->threshold_hw/10,
							(double)outlet->threshold_lw/10,
							(double)outlet->threshold_lc/10
						);             
					gstrcat(gCache.IpduOutMan.outletInfoHtml, buffer);
					sprintf(buffer, "[0,0,%d,%d,%d,%d,%d,%d,0,%d,%d,%d,%d,%d],", 
							(ipdu->cap & IPDU_CAP_NOT_SWITCHABLE)?0:1,	
							(flg_cyc)? 1:0,	// CHECK IPDU CAP - LOCK/UNLOCK
							(outlet->cap & OUTLET_CAP_MINIMUMON) ? 1 : 0,
							(outlet->cap & OUTLET_CAP_MINIMUMOFF) ? 1 : 0,
							(!(ipdu->cap & IPDU_CAP_NOT_SWITCHABLE) && (outlet->cap & OUTLET_CAP_POSTPOWERON)) ? 1 : 0,
							(outlet->cap & OUTLET_CAP_WAKEUP) ? 1 : 0,
							(outlet->elec_cap & ELECMON_CAP_CURRENT) ? 1 : 0,
							(outlet->elec_cap & ELECMON_CAP_POWER) ? 1 : 0,	
							(outlet->elec_cap & ELECMON_CAP_THRES_STATUS) ? 1 : 0,	
							(outlet->cap & OUTLET_CAP_POSTPOWEROFF) ? 1 : 0,
							(outlet->elec_cap & ELECMON_CAP_CUR_CRIT_THRES) ? 1 : 0	
							);  
					gstrcat(gCache.IpduOutMan.outcapInfoHtml, buffer);
				}
				gstrcat(gCache.IpduOutMan.outletInfoHtml, "]");
				gstrcat(gCache.IpduOutMan.outcapInfoHtml, "]");
			}

			if (gCache.IpduOutMan.action != Msg_to_WEB) {
				strcpy(gCache.IpduOutMan.sysMsg, "OK");
			}
			gCache.IpduOutMan.action = no_Action;
			//syslog(4,"outlet_info:{%s}\n", gCache.IpduOutMan.outletInfoHtml);
			//syslog(4,"out_cap:{%s}\n", gCache.IpduOutMan.outcapInfoHtml);
		}

	} else if ((result = LoadIpduOutMan()) == kCycSuccess) { // First Level page
		port = gCache.IpduOutMan.portInfo;

		gCache.IpduOutMan.memSignature = getPortsIpdusSignature(gCache.IpduOutMan.numPorts, port);

		bfreeSafe(B_L, gCache.IpduOutMan.portInfoHtml);
		gCache.IpduOutMan.portInfoHtml = NULL;
		bfreeSafe(B_L, gCache.IpduOutMan.ipduInfoHtml);
		gCache.IpduOutMan.ipduInfoHtml = NULL;

		//size = sizeof(buffer) * (gCache.IpduOutMan.numPorts ? gCache.IpduOutMan.numPorts : 1);
		size = PMD_SIZE_PORT * (gCache.IpduOutMan.numPorts ? gCache.IpduOutMan.numPorts : 1);
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.IpduOutMan.portInfoHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
		size = sizeof(buffer) * (gCache.IpduOutMan.numIpdus ? gCache.IpduOutMan.numIpdus : 1);
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.IpduOutMan.ipduInfoHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		// Fill in portInfoHtml and ipduInfoHtml
		strcpy(gCache.IpduOutMan.portInfoHtml, "[");
		strcpy(gCache.IpduOutMan.ipduInfoHtml, "[");
		for (j=0; j<gCache.IpduOutMan.numPorts; j++, port++) {
			// count ipdus before show port
			nipdus = 0;
			for (i=0; i < port->numIpdu; i++) {
				if (port->ipdu->ipduNumber)
					nipdus++;
			}
			sprintf(buffer, "[%d, %d],",
					port->serialPort,
					nipdus);
			gstrcat(gCache.IpduOutMan.portInfoHtml, buffer);
			if (nipdus == 0)  {
				gstrcat(gCache.IpduOutMan.ipduInfoHtml, "[0],");
				continue;
			}
			gstrcat(gCache.IpduOutMan.ipduInfoHtml, "[");
			ipdu = port->ipdu;
			for (i=0; i<port->numIpdu; i++, ipdu++) {
				if (ipdu->ipduNumber) {
					sprintf(buffer, "[\"%s\", \"%s\", \"%s\"],",
						ipdu->name,
						ipdu->model,
						ipdu->vendor);
					gstrcat(gCache.IpduOutMan.ipduInfoHtml, buffer);
				}
			}
			gstrcat(gCache.IpduOutMan.ipduInfoHtml, "],");
		}
		gstrcat(gCache.IpduOutMan.portInfoHtml, "]");
		gstrcat(gCache.IpduOutMan.ipduInfoHtml, "]");

		if (gCache.IpduOutMan.action != Msg_to_WEB) {
			strcpy(gCache.IpduOutMan.sysMsg, "OK");
		}
		gCache.IpduOutMan.action = no_Action;
		//syslog(4,"port_info:{%s}\n", gCache.IpduOutMan.portInfoHtml);
		//syslog(4,"ipdu_info:{%s}\n", gCache.IpduOutMan.ipduInfoHtml);
	}

	if (result != kCycSuccess) {
		result = kCycSuccess;
		gCache.IpduOutMan.numIpdus = 0;
		gCache.IpduOutMan.showIpdu = 0;
		CycSetshowIpdu(&gReq.sid,0);
	}

	return mAppError(result);
}

// Power Management - New Outlet Groups Ctrl page
extern int GetIpduOutGrpCtrl(Param* param, int request)
{
	Result result;
	char_t buffer[100];
	PmIpduOutGrpInfo *group;
	PmOutletInfo     *outlet;
	int size, i, j,flg;

	gCacheLoaded[kIPDUOutletGroupsCtrl] = true;
	//printf("-----> GetIpduOutGrpCtrl()\n");

	if ((result = CycCheckUser(&gReq.sid, &wUser, 0)) != kCycSuccess) 
		return result;

	//[RK]Nov/03/06 - set the username
	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);

	checkPMDguardtime(time(NULL));

	if ((result = LoadIpduOutGrpCtrl()) == kCycSuccess) {
//		gCache.IpduOutGrpCtrl.numIpdus = 10;
//		gCache.IpduOutGrpCtrl.numGroups = 3;
//		gCache.IpduOutGrpCtrl.groupInfo = fakeGroups;

		gCache.IpduOutGrpCtrl.memSignature = getGroupsSignature(gCache.IpduOutGrpCtrl.totalGroups,
									gCache.IpduOutGrpCtrl.groupInfo,
									gCache.IpduOutGrpCtrl.groupDefs);

		bfreeSafe(B_L, gCache.IpduOutGrpCtrl.groupInfoHtml);
		gCache.IpduOutGrpCtrl.groupInfoHtml = NULL;
		bfreeSafe(B_L, gCache.IpduOutGrpCtrl.outletInfoHtml);
		gCache.IpduOutGrpCtrl.outletInfoHtml = NULL;

		size = sizeof(buffer) * (gCache.IpduOutGrpCtrl.numGroups ? gCache.IpduOutGrpCtrl.numGroups : 1);
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.IpduOutGrpCtrl.groupInfoHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp
		if ( gCache.IpduOutGrpCtrl.groupInfo != NULL ) {
			group = gCache.IpduOutGrpCtrl.groupInfo;
			for (i=0, size=0; i<gCache.IpduOutGrpCtrl.totalGroups; i++, group++)
				size += group->numOutlets;
			size = sizeof(buffer) * (size ? size : 1);
			// the calculation of the size to be allocated is approximate... 
			if (!(gCache.IpduOutGrpCtrl.outletInfoHtml = balloc(B_L, size)))
				return mAppError(kMemAllocationFailure); //mp
	
			// Fill in groupInfoHtml and outletInfoHtml
			group = gCache.IpduOutGrpCtrl.groupInfo;
			strcpy(gCache.IpduOutGrpCtrl.groupInfoHtml, "[");
			strcpy(gCache.IpduOutGrpCtrl.outletInfoHtml, "[");
			for (j=0; j<gCache.IpduOutGrpCtrl.totalGroups; j++, group++) {
				if (!group->numOutlets) continue;
				sprintf(buffer, "[\"%s\", %d, %d],",
						group->name+1,
						group->numOutlets,
						j);
				gstrcat(gCache.IpduOutGrpCtrl.groupInfoHtml, buffer);
				gstrcat(gCache.IpduOutGrpCtrl.outletInfoHtml, "[");
				outlet = group->outletInfo;
				for (i=0; i<group->numOutlets; i++, outlet++) {
					flg = cycPMDngcheck_SPC(outlet->shmRef);
					sprintf(buffer, "[\"%s\", \"%s, %s %d\", %d, %d],",
							outlet->name,
							outlet->ipduID,
							"port",
							outlet->serialPort,
							outlet->flgOnOff,
							(flg)?10:outlet->flgLock);
					gstrcat(gCache.IpduOutGrpCtrl.outletInfoHtml, buffer);
				}
				gstrcat(gCache.IpduOutGrpCtrl.outletInfoHtml, "],");
			}
			gstrcat(gCache.IpduOutGrpCtrl.groupInfoHtml, "]");
			gstrcat(gCache.IpduOutGrpCtrl.outletInfoHtml, "]");
		}
		if (gCache.IpduOutGrpCtrl.action != Msg_to_WEB) {
			strcpy(gCache.IpduOutGrpCtrl.sysMsg, "OK");
		}
		gCache.IpduOutGrpCtrl.action = no_Action;
		//printf(" group_info:{%s}\n", gCache.IpduOutGrpCtrl.groupInfoHtml);
		//printf("outlet_info:{%s}\n", gCache.IpduOutGrpCtrl.outletInfoHtml);
	}

	if (result != kCycSuccess) {
		result = kCycSuccess;
		gCache.IpduOutGrpCtrl.numGroups = 0;
		gCache.IpduOutGrpCtrl.numIpdus = 0;
	}

	return mAppError(result);
}

// Power Management - New View IPDUs Info page
extern int GetViewIpduInfo(Param* param, int request)
{
	Result result;
	char_t buffer[250],env_type[40];
	PmIpduPortInfo *port;
	PmIpduInfo     *ipdu;
	PmElecMonInfo   *elecmon;
	PmEnvMonInfo	*envmon;
	int size, i, j, k;

	gCacheLoaded[kViewConfIPDUs] = true;
	//printf("-----> GetViewIpduInfo()\n");

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	checkPMDguardtime(time(NULL));

	if ((result = LoadViewConfIpdu()) == kCycSuccess) {
		port = gCache.ViewConfIpdu.portInfo;

		gCache.ViewConfIpdu.memSignature = getPortsIpdusSignature(gCache.ViewConfIpdu.numPorts, port);

		bfreeSafe(B_L, gCache.ViewConfIpdu.ipduInfoHtml);
		gCache.ViewConfIpdu.ipduInfoHtml = NULL;
		bfreeSafe(B_L, gCache.ViewConfIpdu.confcapHtml);
		gCache.ViewConfIpdu.confcapHtml = NULL;

		//size = sizeof(buffer) * (gCache.ViewConfIpdu.numIpdus ? gCache.ViewConfIpdu.numIpdus : 1);
		size = PMD_SIZE_INFO_IPDU * (gCache.ViewConfIpdu.numIpdus ? gCache.ViewConfIpdu.numIpdus : 1);
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.ViewConfIpdu.ipduInfoHtml = balloc(B_L, size * 2))) //4 is due to multiple buffers...
			return mAppError(kMemAllocationFailure); //mp
		size = sizeof(buffer) * (gCache.ViewConfIpdu.numIpdus ? gCache.ViewConfIpdu.numIpdus : 1);
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.ViewConfIpdu.confcapHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		// Fill in ipduInfoHtml and confcapHtml
		strcpy(gCache.ViewConfIpdu.ipduInfoHtml, "[");
		strcpy(gCache.ViewConfIpdu.confcapHtml, "[");
		for (i=0; i<gCache.ViewConfIpdu.numPorts; i++, port++) {
			ipdu = port->ipdu;
			for (j=0; j<port->numIpdu; j++, ipdu++) {

				if (ipdu->ipduNumber == 0) continue;

			// ipduInfo is [[ 0. name(str),
	                //                1. complement(str),
        	        //                2. vendor(str),
             		//                3. model(str),
	                //                4. num_outlets(int),
	                //                5. sw_ver(str),
	                //                6. num_phases(int),
	                //                7. num_banks(int),
	                //                8. nominal_voltage(int),
				sprintf(buffer, "[\"%s\",\"%s port %d\",\"%s\",\"%s\",%d,\"%s\",%d,%d,%d,[",
						ipdu->name,
						"serial",
						port->serialPort,
						ipdu->vendor,
						ipdu->model,
						ipdu->outlets,
						ipdu->version,
						ipdu->num_phases,
						ipdu->num_banks,
						ipdu->def_voltage/10
					);
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				// cap for 0-8
				sprintf(buffer, "[%d,%d,%d,%d,%d,%d,%d,%d,%d,[",
						(ipdu->max_flag & ELECMON_CAP_MAX_CURRENT)? 1:0,
						(ipdu->max_flag & ELECMON_CAP_MAX_POWER)? 1:0,
						((ipdu->cap & IPDU_CAP_ENVMON)&& ipdu->envmon_sensors)? 1:0,
						(ipdu->elecPduInfo.cap & ELECMON_CAP_HWOCP)? 1:0,
						(ipdu->max_flag & ELECMON_CAP_MAX_VOLTAGE)? 1:0,
						(ipdu->max_flag & ELECMON_CAP_MAX_POWER_FACTOR)? 1:0,
						(ipdu->num_phases)? 1:0,
						(ipdu->num_banks)? 1:0,
						(ipdu->def_voltage != -1)? 1:0
					);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);

				// PDU elec mon
				elecmon = &ipdu->elecPduInfo;
	                //      9. pdu_elecmon_info[] and CAP MAXVAL
			//          0. name
        	        //          1. value(str),
	                //          2. maxval(str),
				if (elecmon->cap & ELECMON_CAP_CURRENT) {
					sprintf(buffer, "[\"%s(%s)\",\"%.1f A\",\"%.1f A\"],",
							"Current",
							elecmon->current.alarm,
							elecmon->current.value/10,
							elecmon->current.maxval/10);
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_CURRENT)?1:0);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				} else {
					sprintf(buffer,"[0],");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				if (elecmon->cap & ELECMON_CAP_POWER) {
					sprintf(buffer, "[\"%s(%s)\",\"%.1f W\",\"%.1f W\"],",
							"Power Consumption",
							(elecmon->power.type==0)?"measured":"estimated",
							elecmon->power.value/10,
							elecmon->power.maxval/10);
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_POWER)?1:0);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				} else {
					sprintf(buffer,"[0],");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				if (elecmon->cap & ELECMON_CAP_VOLTAGE) {
					sprintf(buffer, "[\"%s(%s)\",\"%d V\",\"%d V\"],",
							"Voltage",
							(elecmon->voltage.type==0)?"measured":"estimated",
							(int)elecmon->voltage.value/10,
							(int)elecmon->voltage.maxval/10);
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_VOLTAGE)?1:0);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				} else {
					sprintf(buffer,"[0],");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				
				if (elecmon->cap & ELECMON_CAP_POWER_FACTOR) {
					sprintf(buffer, "[\"%s(%s)\",\"%.2f\",\"%.2f\"],",
							"Power Factor",
							(elecmon->pf.type==0)?"measured":"estimated",
							elecmon->pf.value/100,
							elecmon->pf.maxval/100);
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_POWER_FACTOR)?1:0);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				} else {
					sprintf(buffer,"[0],");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}

				sprintf(buffer, "],[");
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);

				elecmon = ipdu->firstBankInfo;
				for (k=0; k<ipdu->num_banks; k++, elecmon++) {
	                //               10. bank
	       	       //                      0. name(str),
			//			  [ 0.name
	                //                     	    1. value(str),
	                //                          2. maxval(str)],
					sprintf(buffer,"[\"%s\",", elecmon->name);
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					sprintf(buffer,"[");
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					if (elecmon->cap & ELECMON_CAP_CURRENT) {
						sprintf(buffer, "[\"%s(%s)\",\"%.1f A\",\"%.1f A\"],",
							"Current",
							elecmon->current.alarm,
							elecmon->current.value/10,
							elecmon->current.maxval/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_CURRENT)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					if (elecmon->cap & ELECMON_CAP_POWER) {
						sprintf(buffer, "[\"%s(%s)\",\"%.1f W\",\"%.1f W\"],",
							"Power Consumption",
							(elecmon->power.type==0)?"measured":"estimated",
							elecmon->power.value/10,
							elecmon->power.maxval/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_POWER)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					if (elecmon->cap & ELECMON_CAP_VOLTAGE) {
						sprintf(buffer, "[\"%s(%s)\",\"%d V\",\"%d V\"],",
							"Voltage",
							(elecmon->voltage.type)?"estimated":"measured",
							(int)elecmon->voltage.value/10,
							(int)elecmon->voltage.maxval/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_VOLTAGE)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					if (elecmon->cap & ELECMON_CAP_POWER_FACTOR) {
						sprintf(buffer, "[\"%s(%s)\",\"%.2f\",\"%.2f\"],",
							"Power Factor",
							(elecmon->pf.type)?"estimated":"measured",
							elecmon->pf.value/100,
							elecmon->pf.maxval/100);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_POWER_FACTOR)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					sprintf(buffer,"],");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				if (ipdu->num_banks == 0) {
					sprintf(buffer, " 0 ],[");
				} else {
					sprintf(buffer, "],[");
				}
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);

				elecmon = ipdu->firstPhaseInfo;
				for (k=0; k<ipdu->num_phases; k++, elecmon++) {
	                //               12. phases
	       	       //                      0. name(str),
			//			  [ 0.name
	                //                     	    1. value(str),
	                //                          2. maxval(str)],
					sprintf(buffer,"[\"%s\",", elecmon->name);
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					sprintf(buffer,"[");
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					if (elecmon->cap & ELECMON_CAP_CURRENT) {
						sprintf(buffer, "[\"%s(%s)\",\"%.1f A\",\"%.1f A\"],",
							"Current",
							elecmon->current.alarm,
							elecmon->current.value/10,
							elecmon->current.maxval/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_CURRENT)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					if (elecmon->cap & ELECMON_CAP_POWER) {
						sprintf(buffer, "[\"%s(%s)\",\"%.1f W\",\"%.1f W\"],",
							"Power Consumption",
							(elecmon->power.type)?"estimated":"measured",
							elecmon->power.value/10,
							elecmon->power.maxval/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_POWER)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					if (elecmon->cap & ELECMON_CAP_VOLTAGE) {
						sprintf(buffer, "[\"%s(%s)\",\"%d V\",\"%d V\"],",
							"Voltage",
							(elecmon->voltage.type)?"estimated":"measured",
							(int)elecmon->voltage.value/10,
							(int)elecmon->voltage.maxval/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_VOLTAGE)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					if (elecmon->cap & ELECMON_CAP_POWER_FACTOR) {
						sprintf(buffer, "[\"%s(%s)\",\"%.2f\",\"%.2f\"],",
							"Power Factor",
							(elecmon->pf.type)?"estimated":"measured",
							elecmon->pf.value/100,
							elecmon->pf.maxval/100);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d],",(elecmon->cap & ELECMON_CAP_MAX_POWER_FACTOR)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[0],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
					sprintf(buffer,"],");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				if (ipdu->num_phases == 0) {
					sprintf(buffer, " 0 ],%d, [", ipdu->envmon_sensors);
				} else {
	                //               13. envmon_sensors(int),
					sprintf(buffer, "],%d,[",
						ipdu->envmon_sensors);
				}
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);

				// cap for 13
				if (ipdu->num_phases == 0) {
					sprintf(buffer,"0],%d,", (ipdu->envmon_sensors)?1:0);
				} else {
					sprintf(buffer,"],%d,", (ipdu->envmon_sensors)?1:0);
				}
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);

				envmon = ipdu->firstEnvMonSensor;
				for (k=0; k<ipdu->envmon_sensors; k++, envmon++) {
	                //               14. envmon_sensor_info[][]
	                //                      0. type (str),
	                //                      1. name(str),
	                //                      2. value(str),
	                //                      3. maxval(str),
					buffer[0] = 0x00;
					switch (envmon->type) {
					case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
					case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
					case ENVMON_SENSOR_TEMPERATURE: // temperature sensor
						if (envmon->type == ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL) {
								strcpy(env_type,"Temperature-Internal");
						} else {
								strcpy(env_type,"Temperature");
						}
						if (envmon->maxval) {
							sprintf(buffer, "[\"%s\",\"%s\",\"%.1f&deg;C(%.1f&deg;F)\",\"%.1f&deg;C(%.1f&deg;F)\"],",
								env_type,
								envmon->name,
								((double)envmon->value-2731.5)/10,
								((double)envmon->value * 9 / 5 - 4596.7) / 10,
								((double)envmon->maxval-2731.5)/10,
								((double)envmon->maxval * 9 / 5 - 4596.7) / 10);
						} else {
							sprintf(buffer, "[\"%s\",\"%s\",\"%.1f&deg;C(%.1f&deg;F)\",\"\"],",
								env_type,
								envmon->name,
								((double)envmon->value-2731.5)/10,
								((double)envmon->value * 9 / 5 - 4596.7) / 10);
						}
						break;
					case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
					case ENVMON_SENSOR_HUMIDITY:
						if (envmon->maxval) {
							sprintf(buffer, "[\"%s\",\"%s\",\"%.1f%%\",\"%.1f%%\"],",
								"Humidity",
								envmon->name,
								(double)envmon->value/10,
								(double)envmon->maxval/10);
						} else {
							sprintf(buffer, "[\"%s\",\"%s\",\"%.1f%%\",\"\"],",
								"Humidity",
								envmon->name,
								(double)envmon->value/10);
						}
						break;
					case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
						sprintf(buffer, "[\"%s\",\"%s\",\"%.1fafu\",\"%.1fafu\"],",
							"Air-flow",
							envmon->name,
							(double)envmon->value/10,
							(double)envmon->maxval/10);
						break;
					case ENVMON_SENSOR_AVOCENT_SMOKE:
						sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
							"Smoke sensor",
							envmon->name,
							((int)envmon->value)?"triggered":"normal",
							"");
						break;
					case ENVMON_SENSOR_AVOCENT_DRY_CONTACT_OPEN:
					case ENVMON_SENSOR_DRY_CONTACT:
						sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
							"Dry contact",
							envmon->name,
							((int)envmon->value)?"opened":"closed",
							"");
						break;
					case ENVMON_SENSOR_AVOCENT_DRY_CONTACT_CLOSED:
						sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
							"Dry contact",
							envmon->name,
							((int)envmon->value)?"closed":"opened",
							"");
						break;
					case ENVMON_SENSOR_WATER_LEVEL:
					case ENVMON_SENSOR_AVOCENT_WATER_LEVEL:
						sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
							"Water level",
							envmon->name,
							((int)envmon->value)?"trigged":"normal",
							"");
						break;
					case ENVMON_SENSOR_AVOCENT_MOTION:
						sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
							"Motion sensor",
							envmon->name,
							((int)envmon->value)?"triggered":"normal",
							"");
						break;
					case ENVMON_SENSOR_UNDEFINED:
					default:
						sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
							"Unplugged",
							envmon->name,
							"n/a",
							"");
						break;
						
					}
					if (buffer[0]) gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				}
	                //               15. gindex
				if (ipdu->envmon_sensors) {
					sprintf(buffer, "],%d],",i*1000+j);
				} else {
					sprintf(buffer, " 0 ],%d],",i*1000+j);
				}
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer, "],");
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
			}
		}
		gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, "]");
		gstrcat(gCache.ViewConfIpdu.confcapHtml, "]");

//syslog(4,"%s: info {%s}",__func__,gCache.ViewConfIpdu.ipduInfoHtml);
//syslog(4,"%s: cap {%s}",__func__,gCache.ViewConfIpdu.confcapHtml);
		if (gCache.ViewConfIpdu.action != Msg_to_WEB) {
			strcpy(gCache.ViewConfIpdu.sysMsg, "OK");
		}
		gCache.ViewConfIpdu.action = no_Action;

	}

	if (result != kCycSuccess) {
		result = kCycSuccess;
		gCache.ViewConfIpdu.numIpdus = 0;
	}

	return mAppError(result);
}

// Power Management - New Configuration page
extern int GetIpduConfig(Param* param, int request)
{
	Result result;
	char_t buffer[200], envtype[50];
	PmIpduPortInfo *port;
	PmIpduInfo     *ipdu;
	PmElecMonInfo   *elec;

	PmEnvMonInfo	*env;
	int size, i, j, k,cap;

	gCacheLoaded[kViewConfIPDUs] = true;

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	checkPMDguardtime(time(NULL));

	if ((result = LoadViewConfIpdu()) == kCycSuccess) {
		port = gCache.ViewConfIpdu.portInfo;

//syslog(4,"%s: after load",__func__);

		gCache.ViewConfIpdu.memSignature = getPortsIpdusSignature(gCache.ViewConfIpdu.numPorts, port);

		bfreeSafe(B_L, gCache.ViewConfIpdu.ipduInfoHtml);
		gCache.ViewConfIpdu.ipduInfoHtml = NULL;
		bfreeSafe(B_L, gCache.ViewConfIpdu.confcapHtml);
		gCache.ViewConfIpdu.confcapHtml = NULL;

		//size = sizeof(buffer) * (gCache.ViewConfIpdu.numIpdus ? gCache.ViewConfIpdu.numIpdus : 1);
		size = PMD_SIZE_INFO_IPDU * (gCache.ViewConfIpdu.numIpdus ? gCache.ViewConfIpdu.numIpdus : 1);
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.ViewConfIpdu.ipduInfoHtml = balloc(B_L, size))) 
			return mAppError(kMemAllocationFailure); //mp
		// the calculation of the size to be allocated is approximate... 
		size = sizeof(buffer) * (gCache.ViewConfIpdu.numIpdus ? gCache.ViewConfIpdu.numIpdus : 1);
		if (!(gCache.ViewConfIpdu.confcapHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		// Fill in ipduInfoHtml and confcapHtml
		strcpy(gCache.ViewConfIpdu.ipduInfoHtml, "[");
		strcpy(gCache.ViewConfIpdu.confcapHtml, "[");
		for (i=0; i<gCache.ViewConfIpdu.numPorts; i++, port++) {
			ipdu = port->ipdu;
			for (j=0; j<port->numIpdu; j++, ipdu++) {
				// 1 - 11
//syslog(4,"%s: port[%d] pdu[%s]",__func__,port->serialPort,ipdu->name);
				sprintf(buffer, "[\"%s\",\"serial port %d\",\"%s\",\"%s\",%d,%d,%d,%d,%d,%d,%d,%.1f,",
						ipdu->name,
						port->serialPort,
						ipdu->vendor,
						ipdu->model,
						ipdu->overcurrent,
						ipdu->syslog,
						ipdu->buzzer,
						ipdu->cycle_int,
						ipdu->seq_int,
						ipdu->poll_rate,
						i*1000+j,
						(double)(ipdu->max_current)/10
						);
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer, "[0,0,0,0,%d,%d,%d,%d,%d,1,0,%d,",
						(ipdu->cap & IPDU_CAP_OVERCURRENT) ? 1 : 0,
						(ipdu->cap & IPDU_CAP_SYSLOG) ? 1 : 0,
						(ipdu->cap & IPDU_CAP_BUZZER) ? 1 : 0,
						(ipdu->cap & IPDU_CAP_REBOOTDELAY) ? 1 : 0,
						(ipdu->cap & IPDU_CAP_SEQINTERVAL) ? 1 : 0,
						(ipdu->cap & IPDU_CAP_MAXCURRENT) ? 1 : 0);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);

				// 12 - 14 - Nominal Voltage ...
				sprintf(buffer,"%d,%.2f,%d,",
						ipdu->def_voltage/10,
						ipdu->pwr_factor/100,
						ipdu->num_phases); // num phases for servertech
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer,"%d,%d,%d,",
						(ipdu->def_voltage==-1)?0:1,
						(ipdu->elecPduInfo.pf.type && (ipdu->pwr_factor!=-1))?1:0,
						(ipdu->num_inlets == 3)?1:0);	 
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				
				// 15 - PDU
				elec = &ipdu->elecPduInfo;
				cap = elec->cap;
				sprintf(buffer, "[%.1f,%.1f,%.1f,%.1f],",   
						(double)elec->current.thre_hc/10,
						(double)elec->current.thre_lc/10,
						(double)elec->current.thre_hw/10,
						(double)elec->current.thre_lw/10);
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer,"[%d,%d,%d,%d],",
					(cap & ELECMON_CAP_CUR_CRIT_THRES)?1:0,
					(cap & ELECMON_CAP_CUR_LOWCRIT_THRES)?1:0,
					(cap & ELECMON_CAP_CUR_WARN_THRES)?1:0,
					(cap & ELECMON_CAP_CUR_LOW_THRES)?1:0);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);

				// 16 - Banks 17 - banks info
				sprintf(buffer,"%d,[",ipdu->num_banks);
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer,"%d,[",(ipdu->num_banks)?1:0);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				elec = ipdu->firstBankInfo;
				for (k=0; k<ipdu->num_banks; k++, elec++) {
					cap = elec->cap & (ELECMON_CAP_CUR_CRIT_THRES|
							   ELECMON_CAP_CUR_LOWCRIT_THRES|
							   ELECMON_CAP_CUR_WARN_THRES|
							   ELECMON_CAP_CUR_LOW_THRES);
					if (cap) {	
						sprintf(buffer, "[\"%s\",%.1f,%.1f,%.1f,%.1f],",   
								elec->name,
								(double)elec->current.thre_hc/10,
								(double)elec->current.thre_lc/10,
								(double)elec->current.thre_hw/10,
								(double)elec->current.thre_lw/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d,%d,%d,%d],",
							(cap & ELECMON_CAP_CUR_CRIT_THRES)?1:0,
							(cap & ELECMON_CAP_CUR_LOWCRIT_THRES)?1:0,
							(cap & ELECMON_CAP_CUR_WARN_THRES)?1:0,
							(cap & ELECMON_CAP_CUR_LOW_THRES)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[ 0 ],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[ 0,0,0,0,0 ],");
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
				}
				if (ipdu->num_banks == 0) {
					sprintf(buffer," 0,");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				// 18 - Phases 19 - phases info
				sprintf(buffer,"],%d,[",ipdu->num_phases);
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer,"],%d,[",(ipdu->num_phases)?1:0);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				elec = ipdu->firstPhaseInfo;
				for (k=0; k<ipdu->num_phases; k++, elec++) {
					cap = elec->cap & (ELECMON_CAP_CUR_CRIT_THRES|
							   ELECMON_CAP_CUR_LOWCRIT_THRES|
							   ELECMON_CAP_CUR_WARN_THRES|
							   ELECMON_CAP_CUR_LOW_THRES);
					if (cap) {	
						sprintf(buffer, "[\"%s\",%.1f,%.1f,%.1f,%.1f],",   
								elec->name,
								(double)elec->current.thre_hc/10,
								(double)elec->current.thre_lc/10,
								(double)elec->current.thre_hw/10,
								(double)elec->current.thre_lw/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d,%d,%d,%d],",
							(cap & ELECMON_CAP_CUR_CRIT_THRES)?1:0,
							(cap & ELECMON_CAP_CUR_LOWCRIT_THRES)?1:0,
							(cap & ELECMON_CAP_CUR_WARN_THRES)?1:0,
							(cap & ELECMON_CAP_CUR_LOW_THRES)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[ 0 ],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[ 0,0,0,0,0 ],");
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
				}
				if (ipdu->num_phases == 0) {
					sprintf(buffer," 0,");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				// 20 - Envmon = 21 envmon info
				sprintf(buffer,"],%d,[",ipdu->envmon_sensors);
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer,"],%d,[",(ipdu->envmon_sensors)?1:0);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				env = ipdu->firstEnvMonSensor;
				for (k=0; k<ipdu->envmon_sensors; k++, env++) {
					cap = env->cap & (ENVMON_CAP_CRIT_THRES|
							   ENVMON_CAP_LOW_THRES|
							   ENVMON_CAP_CRIT_WARN_THRES|
							   ENVMON_CAP_LOW_WARN_THRES);
					if (cap) {
						switch (env->type) {
							case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
								strcpy(envtype,"Temp.Intern.");
								break;
							case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
							case ENVMON_SENSOR_TEMPERATURE:
								strcpy(envtype,"Temperature");
								break;
							case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
							case ENVMON_SENSOR_HUMIDITY:
								strcpy(envtype,"Humidity");
								break;
							case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
								strcpy(envtype,"Air flow");
								break;
							default:
								sprintf(envtype,"type=%d",env->type);
								break;
						}
						sprintf(buffer, "[\"%s(%s) [%s]\",%.1f,%.1f,%.1f,%.1f],",   
							envtype,
							env->name,
							env->unit,
							(double)env->thre_hc/10,
							(double)env->thre_lc/10,
							(double)env->thre_hw/10,
							(double)env->thre_lw/10);
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[1,%d,%d,%d,%d],",
							(cap & ENVMON_CAP_CRIT_THRES)?1:0,
							(cap & ENVMON_CAP_LOW_THRES)?1:0,
							(cap & ENVMON_CAP_CRIT_WARN_THRES)?1:0,
							(cap & ENVMON_CAP_LOW_WARN_THRES)?1:0);
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					} else {
						sprintf(buffer,"[ 0 ],");
						gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
						sprintf(buffer,"[ 0,0,0,0,0 ],");
						gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
					}
				}
				if (ipdu->envmon_sensors == 0) {
					sprintf(buffer," 0,");
					gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
					gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
				}
				// 22 cold start delay, 23 display, 24 display-cycle
				sprintf(buffer,"],%d,%d,%d],",ipdu->cold_start,ipdu->display,ipdu->display_cycle);
				gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, buffer);
				sprintf(buffer,"],%d,%d,%d],",
						(ipdu->cap & IPDU_CAP_COLDSTARTDELAY)?1:0,
						(ipdu->cap & IPDU_CAP_DISPLAY)?1:0,
						(ipdu->cap & IPDU_CAP_DISPLAY_CYCLE)?1:0);
				gstrcat(gCache.ViewConfIpdu.confcapHtml, buffer);
			}
		}
		gstrcat(gCache.ViewConfIpdu.ipduInfoHtml, "]");
		gstrcat(gCache.ViewConfIpdu.confcapHtml, "]");

//syslog(4,"%s: info {%s}",__func__,gCache.ViewConfIpdu.ipduInfoHtml);
//syslog(4,"%s: cap {%s}",__func__,gCache.ViewConfIpdu.confcapHtml);
		if (gCache.ViewConfIpdu.action != Msg_to_WEB) {
			strcpy(gCache.ViewConfIpdu.sysMsg, "OK");
		}
		gCache.ViewConfIpdu.action = no_Action;

	}

	return mAppError(result);
}

// Power Management - New Software Upgrade page
extern int GetIpduSwUpgrade(Param* param, int request)
{
	Result result;
	char_t buffer[100];
	PmIpduPortInfo *port;
	PmIpduInfo     *ipdu;
	int size, i, j;

	gCacheLoaded[kIPDUSoftwareUpgrade] = true;
	//printf("-----> GetIpduSwUpgrade()\n");

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	checkPMDguardtime(time(NULL));

	if (((result = LoadIpduSwUpgrade()) == kCycSuccess) &&
		gCache.IpduSwUpgrade.numPorts) { //[RK]Jab/31/07 - fixed bug#18689

//		gCache.IpduSwUpgrade.numPorts = 3;
//		gCache.IpduSwUpgrade.numIpdus = 3;
//		gCache.IpduSwUpgrade.portInfo = fakePorts;
//		strcpy(gCache.IpduSwUpgrade.latest_sw, "1.2.3");

		bfreeSafe(B_L, gCache.IpduSwUpgrade.swupInfoHtml);
		gCache.IpduSwUpgrade.swupInfoHtml = NULL;

		size = sizeof(buffer) * (gCache.IpduSwUpgrade.numIpdus ? gCache.IpduSwUpgrade.numIpdus : 1);
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.IpduSwUpgrade.swupInfoHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		port = gCache.IpduSwUpgrade.portInfo;
		gCache.IpduSwUpgrade.nValidIpdus = gCache.IpduSwUpgrade.numIpdus;

		gCache.IpduSwUpgrade.memSignature = getPortsIpdusSignature(gCache.IpduOutMan.numPorts, port);

		// Fill in swupInfoHtml
		strcpy(gCache.IpduSwUpgrade.swupInfoHtml, "[");
		for (i=0; i<gCache.IpduSwUpgrade.numPorts; i++, port++) {
			ipdu = port->ipdu;
			for (j=0; j<port->numIpdu; j++, ipdu++) {
				if ((ipdu->type == Vendor_cyclades) &&
				    (ipdu->upgradeFlag != kPmUpgNotAvail)) {
					sprintf(buffer, "[\"%s\",\"%s port %d\",\"%s\",\"%s\",\"%s\",%d],",
							ipdu->name,
							"serial",
							port->serialPort,
							ipdu->vendor,
							ipdu->model,
							ipdu->version,
							i*1000+j);
					gstrcat(gCache.IpduSwUpgrade.swupInfoHtml, buffer);
				} else {
					gCache.IpduSwUpgrade.nValidIpdus--;
				}
			}
		}
		gstrcat(gCache.IpduSwUpgrade.swupInfoHtml, "]");

		if (gCache.IpduSwUpgrade.action != Msg_to_WEB) {
			strcpy(gCache.IpduSwUpgrade.sysMsg, "OK");
		}
		gCache.IpduSwUpgrade.action = no_Action;

	}

	return mAppError(result);
}

// Power Management - New PMD Config, General tab
extern int GetpmdGen(Param* param, int request)
{
	Result result;
	char_t buffer[100];
	PMDGeneralInfo *genInfo;
	int size, i;
	int vendors = 3; //Cyclades, SPC and ServerTech

	//printf("---> GetpmdGen()\n");

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	if ((result = LoadFirst(LoadpmdGen, kPMDGeneral, &gCache.pmdGen, &gCacheSave.pmdGen, sizeof(PMDGen))) == kCycSuccess) {

		bfreeSafe(B_L, gCache.pmdGen.confInfoHtml);
		gCache.pmdGen.confInfoHtml = NULL;

		gCache.pmdGen.numVendors = vendors;

		size = sizeof(buffer) * vendors;
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.pmdGen.confInfoHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		genInfo = gCache.pmdGen.genInfo;

		// Fill in confInfoHtml
		strcpy(gCache.pmdGen.confInfoHtml, "[");
		for (i=0; i<vendors; i++, genInfo++) {
			switch (genInfo->vendor) {
			case Vendor_cyclades:
				sprintf(buffer, "[\"Cyclades\",");
				break;
			case Vendor_spc:
				sprintf(buffer, "[\"SPC\",");
				break;
			case Vendor_servertech:
				sprintf(buffer, "[\"ServerTech\",");
				break;
			default:
				break;
			}
			gstrcat(gCache.pmdGen.confInfoHtml, buffer);
			sprintf (buffer, "\"%s\",\"%s\"],",
					 genInfo->user,
					 genInfo->passwd);
			gstrcat(gCache.pmdGen.confInfoHtml, buffer);
		}
		gstrcat(gCache.pmdGen.confInfoHtml, "]");

		strcpy(gCache.pmdGen.sysMsg, "OK");
	}

	return mAppError(result);
}

// Power Management - New PMD Config, Outlet Groups tab
extern int GetpmdOutGrp(Param* param, int request)
{
	Result result;
	char buffer[200];
	PMDOutGrpsDef *groups;
	int size, i, groupsDeleted=0;

	//printf("---> GetpmdOutGrp()\n");

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	if ((result = LoadFirst(LoadpmdOutGrp, kPMDOutletGroups, &gCache.pmdOutGrp, NULL, 0)) == kCycSuccess) {

		bfreeSafe(B_L, gCache.pmdOutGrp.groupsListHtml);
		gCache.pmdOutGrp.groupsListHtml = NULL;

		size = sizeof(buffer) * gCache.pmdOutGrp.numGroups;
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.pmdOutGrp.groupsListHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		groups = gCache.pmdOutGrp.groups;
		gCache.pmdOutGrp.groupsListHtml[0] = '\0';

		// Fill in groupsListHtml
		for (i = 0; i < gCache.pmdOutGrp.numGroups; i++, groups++) {
			if (groups->action != kDelete) { //find out if it is a deleted entry
				// write HTML code
				sprintf(buffer, "<option value=\"%d \">", i);
				gstrcat(gCache.pmdOutGrp.groupsListHtml, buffer);

				includeNbsp(buffer,groups->group, 15);
				gstrcat(gCache.pmdOutGrp.groupsListHtml, buffer);
				gstrcat(gCache.pmdOutGrp.groupsListHtml, "&nbsp;&nbsp;");

				//includeNbsp(buffer,groups->outletList, 90);
				//gstrcat(gCache.pmdOutGrp.groupsListHtml, buffer);
				gstrcat(gCache.pmdOutGrp.groupsListHtml, groups->outletList);
				gstrcat(gCache.pmdOutGrp.groupsListHtml, "</option>");
			} else {
				groupsDeleted++;
			}
		}

		strcpy(gCache.pmdOutGrp.entries, "unchanged");

		if (!gCache.pmdOutGrp.sysMsg[0])
			strcpy(gCache.pmdOutGrp.sysMsg, "OK");

		gCache.pmdOutGrp.addEditEntry = kNone;
	}

	return mAppError(result);
}

extern int GetpmdOutGrpEntry(Param* param, int request)
{
	PMDOutGrpsDef *groups = gCache.pmdOutGrp.groups;

	//printf("---> GetpmdOutGrpEntry()\n");

	gCache.pmdOutGrp.savedIndex = atoi(gCache.pmdOutGrp.sysMsg);
	gCache.pmdOutGrp.sysMsg[0] = '\0';

	if (!gCacheLoaded[kPMDOutletGroups]) { 	// The page wasn't even loaded
		return mAppError((Result)kCommitPending);
	}

	if (gCache.pmdOutGrp.savedIndex >= 0) {// Edit call

		gCache.pmdOutGrp.addEditEntry = kEdit; //signal Edit Operation
		strcpy(gCache.pmdOutGrp.singleGroup.group, groups[gCache.pmdOutGrp.savedIndex].group); //load info
		strcpy(gCache.pmdOutGrp.singleGroup.outletList, groups[gCache.pmdOutGrp.savedIndex].outletList);

	} else { // Add call

		gCache.pmdOutGrp.addEditEntry = kAdd; //signal Add Operation
		strcpy(gCache.pmdOutGrp.singleGroup.group, "addaddadd"); //signal web
		strcpy(gCache.pmdOutGrp.singleGroup.outletList, "addaddadd");
	}

	return mAppError((Result)kCycSuccess);
}

// Power Management - New PMD Config, Users Management tab
extern int GetpmdUserMan(Param* param, int request)
{
	Result result;
	char_t buffer[200];
	PMDUsersMgm *users;
	int size, i, usersDeleted=0;

	//printf("---> GetpmdUserMan()\n");

	if ((result = GetCycUserType(0)) != kCycSuccess)
		return mAppError(result);

	if ((result = LoadFirst(LoadpmdUserMan, kPMDUsersManagement, &gCache.pmdUserMan, NULL, 0)) == kCycSuccess) {
		if (gCache.pmdUserMan.usersListHtml) {
			bfreeSafe(B_L, gCache.pmdUserMan.usersListHtml);
			gCache.pmdUserMan.usersListHtml = NULL;
		}
		size = sizeof(buffer) * gCache.pmdUserMan.numUsers;
		// the calculation of the size to be allocated is approximate... 
		if (!(gCache.pmdUserMan.usersListHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		users = gCache.pmdUserMan.usersMgm;
		gCache.pmdUserMan.usersListHtml[0] = '\0';

		// Fill in usersListHtml
		for (i = 0; i < gCache.pmdUserMan.numUsers; i++, users++) {
			if (users->action != kDelete) { //find out if it is a deleted entry
				// write HTML code
				sprintf(buffer, "<option value=\"%d \">", i);
				gstrcat(gCache.pmdUserMan.usersListHtml, buffer);

				includeNbsp(buffer,users->user, 15);
				gstrcat(gCache.pmdUserMan.usersListHtml, buffer);
				gstrcat(gCache.pmdUserMan.usersListHtml, "&nbsp;&nbsp;");

				//includeNbsp(buffer,users->outletList, 90);
				//gstrcat(gCache.pmdUserMan.usersListHtml, buffer);
				gstrcat(gCache.pmdUserMan.usersListHtml, users->outletList);
				gstrcat(gCache.pmdUserMan.usersListHtml, "</option>");
			} else {
				usersDeleted++;
			}
		}

		strcpy(gCache.pmdUserMan.entries, "unchanged");

		if (!gCache.pmdUserMan.sysMsg[0])
			strcpy(gCache.pmdUserMan.sysMsg, "OK");

		gCache.pmdUserMan.addEditEntry = kNone;
	}

	return mAppError(result);
}

extern int GetpmdUserManEntry(Param* param, int request)
{
	PMDUsersMgm *users = gCache.pmdUserMan.usersMgm;

	//printf("---> GetpmdUserManEntry()\n");

	gCache.pmdUserMan.savedIndex = atoi(gCache.pmdUserMan.sysMsg);
	gCache.pmdUserMan.sysMsg[0] = '\0';

	if (!gCacheLoaded[kPMDUsersManagement]) { 	// The page wasn't even loaded
		return mAppError((Result)kCommitPending);
	}

	if (gCache.pmdUserMan.savedIndex >= 0) {// Edit call

		gCache.pmdUserMan.addEditEntry = kEdit; //signal Edit Operation
		strcpy(gCache.pmdUserMan.singleUser.user, users[gCache.pmdUserMan.savedIndex].user); //load info
		strcpy(gCache.pmdUserMan.singleUser.outletList, users[gCache.pmdUserMan.savedIndex].outletList);

	} else { // Add call

		gCache.pmdUserMan.addEditEntry = kAdd; //signal Add Operation
		strcpy(gCache.pmdUserMan.singleUser.user, "addaddadd"); //signal web
		strcpy(gCache.pmdUserMan.singleUser.outletList, "addaddadd");
	}

	return mAppError((Result)kCycSuccess);
}
#endif

#ifdef IPMI
// Power Management - IPMI
extern int GetPmIpmi(Param* param, int request)
{
	Result result;
	int OnOffState;
	unsigned char st[5]; //[RK]Feb/11/05

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadPmIpmi, kPmIpmiPage, &gCache.pmipmi, NULL, 0);

	if (result == kCycSuccess) {
		char_t buffer[200];
		IPMIDeviceInfo *ipmi = gCache.pmipmi.ipmiInfo;
		int size, j;
/*  		IPMIDeviceInfo   testIpmi[] = { */
/*  			{0, 11, "alias01", "111.111.111.111", "None", "Operator", "", ""}, */
/*  			{0, 11, "alias02", "222.222.222.222", "MD5", "Operator", "Joao", "pass"}, */
/*  			{0, 11, "alias03", "333.333.333.333", "MD2", "Administrator", "Maria", "passs"}, */
/*  		}; */
/*  		IPMIDeviceInfo *ipmi = testIpmi; */
/*  		gCache.pmipmi.numIpmiDev = 3; */
/*  		gCache.pmipmi.numActiveIpmiDev = 3; */
/*  		gCache.pmipmi.action = no_Action; */

		bfreeSafe(B_L, gCache.pmipmi.ipmiInfoHtml);
		gCache.pmipmi.ipmiInfoHtml = NULL;

		if (gCache.pmipmi.numActiveIpmiDev) {
			size = sizeof(buffer) * gCache.pmipmi.numActiveIpmiDev;
			// the calculation of the size to be allocated is approximate... 
			if (!(gCache.pmipmi.ipmiInfoHtml = balloc(B_L, size)))
				return mAppError(kMemAllocationFailure); //mp
			memset((char *)gCache.pmipmi.ipmiInfoHtml,0,size);
			strcpy(gCache.pmipmi.ipmiInfoHtml, "[");

			// Fill in ipmiInfoHtml
			for (j=0; j<gCache.pmipmi.numIpmiDev; j++, ipmi++) {
				if (ipmi->action != kDelete) {
					if ((ipmi->action == kAdd) || (ipmi->action == kEdit))
						OnOffState = kIpmiCycle;
					else 
						OnOffState=cycIPMIcmd(ipmi->deviceId, kIpmiStatus,NULL);
					
					//[RK]Feb/11/05 - include the state UNKNOW
					switch (OnOffState) {
						case kIpmiOn : strcpy(st,"on"); break;
						case kIpmiOff : strcpy(st,"off"); break;
						case kIpmiCycle : strcpy(st,"uk"); break;
					}

					sprintf(buffer, "[\"%s\", \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\", \"%s\" ],",
							ipmi->alias,
							ipmi->serverIP,
							st, //[RK]Feb/11/05
							ipmi->deviceId,
							ipmi->authType,
							ipmi->userAccessLevel,
							ipmi->username,
							ipmi->password);
					gstrcat(gCache.pmipmi.ipmiInfoHtml, buffer);
				}
			}
			gstrcat(gCache.pmipmi.ipmiInfoHtml, "];");

		} else {
			if (!(gCache.pmipmi.ipmiInfoHtml = balloc(B_L, sizeof(buffer))))
				return mAppError(kMemAllocationFailure); //mp
			strcpy(gCache.pmipmi.ipmiInfoHtml, "'no-IPMIdevice'");
		}

		if (gCache.pmipmi.action != Msg_to_WEB) {
			strcpy(gCache.pmipmi.sysMsg, "OK");
		}
	}

	return mAppError(result);
}
#endif

// SNMP Daemon Settings
extern int GetSNMPdaemon(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadSNMPdaemon, kSNMPdaemonSettings, &gCache.SNMPdaemon, NULL, 0);
	
	if (kCycSuccess == result) {
		char_t tmpStr[] = "<option value=\"%d \">";
		char_t buffer[1024];
		CommunityInfo *info12 = gCache.SNMPdaemon.SNMPv12p;
		Snmpv3Info     *info3 = gCache.SNMPdaemon.SNMPv3p;
		int i, size;

		// Fill in SNMPv12Html
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv12Html);
		gCache.SNMPdaemon.SNMPv12Html = NULL;

		size = gCache.SNMPdaemon.numSNMPv12? sizeof(buffer) * gCache.SNMPdaemon.numSNMPv12 : sizeof(buffer); 
		if (!(gCache.SNMPdaemon.SNMPv12Html = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		memset(gCache.SNMPdaemon.SNMPv12Html, 0, size);

		gstrcat(gCache.SNMPdaemon.SNMPv12Html, "[");
		if (gCache.SNMPdaemon.numSNMPv12) {
			for (i = 0; i < gCache.SNMPdaemon.numSNMPv12; i++, info12++) {
				sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
					info12->communityName,
					info12->source,
					info12->oid,
					(info12->permission==kReadOnly)?
					"Read Only" : "Read Write");
				gstrcat(gCache.SNMPdaemon.SNMPv12Html, buffer);
			}
		}
		gstrcat(gCache.SNMPdaemon.SNMPv12Html, "]");

		// prepare list of entries for future control
		strcpy(gCache.SNMPdaemon.SNMPv12Entries, "unchanged");

		// Fill in SNMPv3Html
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv3Html);
		gCache.SNMPdaemon.SNMPv3Html = NULL;
		// prepare list of entries for future control
		strcpy(gCache.SNMPdaemon.SNMPv3Entries, "unchanged");
		if (gCache.SNMPdaemon.numSNMPv3) {
			i = sizeof(buffer) * 3/*columns*/ * (gCache.SNMPdaemon.numSNMPv3);
			// the calculation of the size to be allocated is only approximate... 
			if (!(gCache.SNMPdaemon.SNMPv3Html = balloc(B_L, i)))
				return mAppError(kMemAllocationFailure); //mp
			gCache.SNMPdaemon.SNMPv3Html[0] = '\0';
			for (i = 0; i < gCache.SNMPdaemon.numSNMPv3; i++, info3++) {
                // write HTML code
				sprintf(buffer, tmpStr, i);
				gstrcat(gCache.SNMPdaemon.SNMPv3Html, buffer);

				includeNbsp(buffer,info3->userName, 20); //as defined in the Web
				gstrcat(gCache.SNMPdaemon.SNMPv3Html, buffer);
				gstrcat(gCache.SNMPdaemon.SNMPv3Html, "&nbsp;&nbsp;");

				if (info3->permission == kReadOnly)
					gstrcat(gCache.SNMPdaemon.SNMPv3Html, "Read Only ");
				else
					gstrcat(gCache.SNMPdaemon.SNMPv3Html, "Read Write");
				gstrcat(gCache.SNMPdaemon.SNMPv3Html, "&nbsp;&nbsp;");

				includeNbsp(buffer,info3->oid, 15);
				gstrcat(gCache.SNMPdaemon.SNMPv3Html, buffer);

				gstrcat(gCache.SNMPdaemon.SNMPv3Html, "</option>\n");
			}
		}

	}
	return mAppError(result);
}

// SNMP Daemon Settings - POP UP (v12)
extern int GetSNMPv12Entry(Param* param, int request)
{
	// Fill in SNMPv12 data
	int index = atoi(gCache.SNMPdaemon.webMsg);

	if (index >= 0) {// Edit call
		gCache.SNMPdaemon.SNMPv12 = *(gCache.SNMPdaemon.SNMPv12p+index);
		gCache.SNMPdaemon.editIndex = index;
	}
	else { // Add call (signal "WEB space")
		gCache.SNMPdaemon.SNMPv12.permission = 99;
	}

	return mAppError((Result)kCycSuccess);
}

// SNMP Daemon Settings - POP UP (v3)
extern int GetSNMPv3Entry(Param* param, int request)
{
	// Fill in SNMPv3 data
	int index = atoi(gCache.SNMPdaemon.webMsg);

	if (index >= 0) {// Edit call
		gCache.SNMPdaemon.SNMPv3 = *(gCache.SNMPdaemon.SNMPv3p+index);
		gCache.SNMPdaemon.editIndex = index;
	}
	else { // Add call (signal "WEB space")
		gCache.SNMPdaemon.SNMPv3.permission = 99;
	}

	return mAppError((Result)kCycSuccess);
}

// Host Table
extern int GetHostTable(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadHostTable, kHostTablePage, &gCache.hostTable, NULL, sizeof(Hosts));
	if (kCycSuccess == result){
		/* Fill in serversHtml */
		char_t buffer[1024];
		Hosts *host = gCache.hostTable.hosts;
		int i, size;

		bfreeSafe(B_L, gCache.hostTable.hostTableHtml);
		gCache.hostTable.hostTableHtml = 0;

		size = gCache.hostTable.numHosts? sizeof(buffer) * gCache.hostTable.numHosts : sizeof(buffer); 
		if (!(gCache.hostTable.hostTableHtml = balloc(B_L, size)))
			return mAppError(kMemAllocationFailure); //mp

		memset(gCache.hostTable.hostTableHtml, 0, size);

		gstrcat(gCache.hostTable.hostTableHtml, "[");
		if (gCache.hostTable.numHosts) {
			for (i = 0; i < gCache.hostTable.numHosts; i++,host++) {
				sprintf(buffer, "[\"%s\",\"%s\",\"%s\"],",
					host->hostIp,
					host->hostName,
					host->hostAlias);
				gstrcat(gCache.hostTable.hostTableHtml, buffer);
			}
		}
		gstrcat(gCache.hostTable.hostTableHtml, "]");
		
	}
	return mAppError(result);
}

// Static Routes
extern int GetStaticRoutes(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadStaticRoutes, kStaticRoutes, &gCache.staticRoutes, NULL, 0);
	
	if (kCycSuccess == result) {

		char_t buffer[1024];
		StRoutesInfo *route = gCache.staticRoutes.routes;
		int i;
		// Fill in stRoutesHtml
		bfreeSafe(B_L, gCache.staticRoutes.stRoutesHtml);
		gCache.staticRoutes.stRoutesHtml = NULL;
		// prepare list of entries for future control
		strcpy(gCache.staticRoutes.entriesList, "unchanged");
		
		i = gCache.staticRoutes.numRoutes? sizeof(buffer) * gCache.staticRoutes.numRoutes : sizeof(buffer);
		if (!(gCache.staticRoutes.stRoutesHtml = balloc(B_L, i)))
			return mAppError(kMemAllocationFailure); //mp
		memset(gCache.staticRoutes.stRoutesHtml, 0, i);

		gstrcat(gCache.staticRoutes.stRoutesHtml, "[");
		if (gCache.staticRoutes.numRoutes) {
			for (i = 0; i < gCache.staticRoutes.numRoutes; i++, route++) {
                // write HTML code
				sprintf(buffer, "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"],",
					route->destinationIp,
					route->destinationMask,
					route->gateWay,
					route->routeInterface,
					route->metric
				);
				gstrcat(gCache.staticRoutes.stRoutesHtml, buffer);
			}
		}
		gstrcat(gCache.staticRoutes.stRoutesHtml, "]");

		gCache.staticRoutes.route.sysMsg[0] = '\0';

	}
	return mAppError(result);
}

// Static Routes - POP UP
extern int GetStaticRouteEntry(Param* param, int request)
{     
   // Fill in stRoute data
   int index = atoi(gCache.staticRoutes.route.sysMsg);
      
   if (index >= 0) {// Edit call
      gCache.staticRoutes.route = *(gCache.staticRoutes.routes+index);
      gCache.staticRoutes.editIndex = index;
   }  
   else { // Add call (signal "WEB space") 
      gCache.staticRoutes.route.routeType = 99;
      gCache.staticRoutes.route.goTo = 99;
   }     
         
   return mAppError((Result)kCycSuccess);
}          

// IpTable
extern int GetIpTableChainsHtml(Param* param, int request)
{
	/* Fill in ipTableHtml */
	ChainInfo * chain;
	char_t templateFilled[300] = {0};
	int i;
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = (LoadFirst(LoadIpTable, kIpTablesPage, &gCache.ipTable, NULL, 0));
	
	if (kCycSuccess != result) {
		return mAppError(result);
	}

	gCache.ipTable.chainSelected = 0;
	gCache.ipTable.ruleSelected = 0;
	gCache.ipTable.actionRule = 0;
	bfreeSafe(B_L, gCache.ipTable.ipTableHtml);
	gCache.ipTable.ipTableHtml = 0;
	
	i = gCache.ipTable.numChains? sizeof(templateFilled) * (gCache.ipTable.numChains) : sizeof(templateFilled);
	if (!(gCache.ipTable.ipTableHtml = balloc(B_L, i)))
		return mAppError(kMemAllocationFailure); //mp
	memset(gCache.ipTable.ipTableHtml, 0, i);
	
	gstrcat(gCache.ipTable.ipTableHtml, "[");
	if (gCache.ipTable.numChains) {
		chain = gCache.ipTable.chainTable;
		for (i=0; i < gCache.ipTable.confNumChains; chain++,i++) {
			if (chain->action != kDelete) {
#ifdef IPv6enable
				gsprintf(templateFilled, "[\"%s\",\"%s\",\"%s\",\"%s\",\"IPv4\"],",
					chain->name,
					chain->policy,
					chain->packets,
					chain->bytes);
#else
				gsprintf(templateFilled, "[\"%s\",\"%s\",\"%s\",\"%s\"],",
					chain->name,
					chain->policy,
					chain->packets,
					chain->bytes);
#endif
				gstrcat(gCache.ipTable.ipTableHtml, templateFilled);
			}
		}
#ifdef IPv6enable
		chain = gCache.ipTable.chainTable6;
		for (i=0; i < gCache.ipTable.confNumChains6; chain++,i++) {
			if (chain->action != kDelete) {
				gsprintf(templateFilled, "[\"%s\",\"%s\",\"%s\",\"%s\",\"IPv6\"],",
					chain->name,
					chain->policy,
					chain->packets,
					chain->bytes);
				gstrcat(gCache.ipTable.ipTableHtml, templateFilled);
			}
		}
#endif
	}   
	gstrcat(gCache.ipTable.ipTableHtml, "]");
	return mAppError(result);
}

extern int GetIpTableRules(Param* param, int request)
{
	char_t templateFilled[600] = {0};
	IpTablesRules *rule;
	int i;
	ChainInfo * chain;
	Result result;
#ifdef IPv6enable
	int type = 4;
#endif

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	if ((!gCacheLoaded[kIpTablesPage]) ||
		(gCache.ipTable.chainSelected == 0)) {
		return mAppError(0);
	}

	i = gCache.ipTable.chainSelected-1;
	if (gCache.ipTable.chainTable[i].action == kDelete) {
		return mAppError(0);
	}

#ifdef IPv6enable
	if (i < gCache.ipTable.confNumChains) {
		chain = &gCache.ipTable.chainTable[i];
		type = 4;
	} else {
		chain = &gCache.ipTable.chainTable6[i-gCache.ipTable.confNumChains];
		type = 6;
	}
	sprintf(gCache.ipTable.chainType,"IPv%d",type);
#else
	chain = &gCache.ipTable.chainTable[i];
#endif

	strcpy(gCache.ipTable.chainName,chain->name);

	bfreeSafe(B_L, gCache.ipTable.rulesHtml);
	gCache.ipTable.rulesHtml = 0;
	
	i = chain->numrules? sizeof(templateFilled) * (chain->numrules) : sizeof(templateFilled);
	if (!(gCache.ipTable.rulesHtml = balloc(B_L, i)))
		return mAppError(kMemAllocationFailure); //mp
	memset(gCache.ipTable.rulesHtml, 0, i);
	memset(&gCache.ipTable.tcpOpt, 0, sizeof(TcpOptions));

	gstrcat(gCache.ipTable.rulesHtml, "[");
	if (chain->numrules) {
		rule = chain->rules;
		for (i = 0; i < chain->numrules; i++, rule++) {
#ifdef IPv6enable
			if (type==4) {
				sprintf(templateFilled, "[\"%s\",\"%s\",\"%s\",\"%s%s\",\"%s%s\",\"%s%s\",\"%s\",\"%s%s\",\"%s%s\"],",
					rule->packets,
					rule->bytes,
					rule->target,
					rule->sourceIpInvert==kInverted? "!" : "",
					strcmp(rule->sourceIp, "0.0.0.0")? rule->sourceIp : "anywhere",
					rule->destinationIpInvert==kInverted? "!" : "",
					strcmp(rule->destinationIp, "0.0.0.0")? rule->destinationIp : "anywhere",
					rule->protocolInvert==kInverted? "!" : "",
					TransOptGetStr(IpChainProtocolOptions,
						sizeof(IpChainProtocolOptions)/sizeof(TranslateOptions),
						rule->protocol),
					TransOptGetStr(IpChainFragmentOptions, 
						sizeof(IpChainFragmentOptions)/sizeof(TranslateOptions),
						rule->fragment),
					rule->inInterfaceInvert==kInverted? "!" : "",
					rule->inInterface,
					rule->outInterfaceInvert==kInverted? "!" : "",
					rule->outInterface);
			} else {
				sprintf(templateFilled, "[\"%s\",\"%s\",\"%s\",\"%s%s\",\"%s%s\",\"%s%s\",\"%s%s\",\"%s%s\"],",
					rule->packets,
					rule->bytes,
					rule->target,
					rule->sourceIpInvert==kInverted? "!" : "",
					strcmp(rule->sourceIp, "::/128")? rule->sourceIp : "anywhere",
					rule->destinationIpInvert==kInverted? "!" : "",
					strcmp(rule->destinationIp, "::/128")? rule->destinationIp : "anywhere",
					rule->protocolInvert==kInverted? "!" : "",
					TransOptGetStr(IpChainProtocolOptions,
						sizeof(IpChainProtocolOptions)/sizeof(TranslateOptions),
						rule->protocol),
					rule->inInterfaceInvert==kInverted? "!" : "",
					rule->inInterface,
					rule->outInterfaceInvert==kInverted? "!" : "",
					rule->outInterface);
			}
#else
			sprintf(templateFilled, "[\"%s\",\"%s\",\"%s\",\"%s%s\",\"%s%s\",\"%s%s\",\"%s\",\"%s%s\",\"%s%s\"],",
				rule->packets,
				rule->bytes,
				rule->target,
				rule->sourceIpInvert==kInverted? "!" : "",
				strcmp(rule->sourceIp, "0.0.0.0")? rule->sourceIp : "anywhere",
				rule->destinationIpInvert==kInverted? "!" : "",
				strcmp(rule->destinationIp, "0.0.0.0")? rule->destinationIp : "anywhere",
				rule->protocolInvert==kInverted? ":" : "",
				TransOptGetStr(IpChainProtocolOptions,
					sizeof(IpChainProtocolOptions)/sizeof(TranslateOptions),
					rule->protocol),
				TransOptGetStr(IpChainFragmentOptions, 
					sizeof(IpChainFragmentOptions)/sizeof(TranslateOptions),
					rule->fragment),
				rule->inInterfaceInvert==kInverted? "!" : "",
				rule->inInterface,
				rule->outInterfaceInvert==kInverted? "!" : "",
				rule->outInterface);
#endif
			gstrcat(gCache.ipTable.rulesHtml, templateFilled);
		}
	}  
	gstrcat(gCache.ipTable.rulesHtml, "]");

	return mAppError(kCycSuccess);
}

extern int GetIpTableRuleSpec(Param* param, int request)
{
	int i,flg=0;
	ChainInfo * chain;
	char temp[]="<option value=\"%s\" %s>%s</option>";
	char temp1[100],sel[10];
#ifdef IPv6enable
	int type = 4;
#endif
	char defChains[] = 
		"[\"ACCEPT\",\"ACCEPT\"],"
		"[\"DROP\",\"DROP\"],"
		"[\"RETURN\",\"RETURN\"],"
		"[\"LOG\",\"LOG\"],"
		"[\"REJECT\",\"REJECT\"],";

	if ((!gCacheLoaded[kIpTablesPage]) ||
	    (gCache.ipTable.chainSelected == 0)) {
		return mAppError(0);
	}	
 	
	i = gCache.ipTable.chainSelected-1;
	if (gCache.ipTable.chainTable[i].action == kDelete) {
		return mAppError(0);
	}

#ifdef IPv6enable
	if (i < gCache.ipTable.confNumChains) {
		chain = &gCache.ipTable.chainTable[i];
		type = 4;
	} else {
		chain = &gCache.ipTable.chainTable6[i-gCache.ipTable.confNumChains];
		type = 6;
	}
	sprintf(gCache.ipTable.chainType,"IPv%d",type);
#else
	chain = &gCache.ipTable.chainTable[i];
#endif
	
	strcpy(gCache.ipTable.chainName,chain->name);

	if (gCache.ipTable.actionRule == kEdit) {
		i = gCache.ipTable.ruleSelected - 1;
		if (chain->numrules < i) {
			return mAppError(0);
		}
		memcpy((char *)&gCache.ipTable.rule,(char *)&chain->rules[i],sizeof(IpTablesRules));
	} else {
		memset((char*)&gCache.ipTable.rule,0,sizeof(IpTablesRules));
		strcpy(gCache.ipTable.rule.target,"ACCEPT");
	}
	// mount targetHtml
#ifdef IPv6enable
	if (type==6) {
		memset(gCache.ipTable.targetHtml,0,kUserListLength);
		gstrcat(gCache.ipTable.targetHtml, "[");
		// default chains
		gstrcat(gCache.ipTable.targetHtml, defChains);
		// user-defined chains
		if (gCache.ipTable.confNumChains6 > 3) {
			for (i=3; i < gCache.ipTable.confNumChains6; i++) {
				chain = &gCache.ipTable.chainTable6[i];
				if (chain->action == kDelete) {
					continue;
				}
				if (!strcmp(chain->name, gCache.ipTable.chainName)
					&& !strcmp(gCache.ipTable.chainType,"IPv6")) {
					continue;
				}
				sprintf(temp1,"[\"%s\",\"%s\"],",chain->name,chain->name);
				gstrcat(gCache.ipTable.targetHtml, temp1);
			}
		}
		gstrcat(gCache.ipTable.targetHtml, "]");
	} else
#endif
	{
		memset(gCache.ipTable.targetHtml,0,kUserListLength);
		sel[0] = 0x00;
		flg = 1;
		if (!strcmp(gCache.ipTable.rule.target,"ACCEPT")) {
			strcpy(sel,"selected");
			flg=0;
		}
		gsprintf(temp1, temp,"ACCEPT",sel,"ACCEPT" );
		gstrcat(gCache.ipTable.targetHtml, temp1);
		
		if (flg && !strcmp(gCache.ipTable.rule.target,"DROP")) {
			strcpy(sel,"selected");
			flg = 0;
		} else {
			sel[0] = 0x00;
		}
		gsprintf(temp1, temp,"DROP",sel,"DROP" );
		gstrcat(gCache.ipTable.targetHtml, temp1);
		
		if (flg && !strcmp(gCache.ipTable.rule.target,"RETURN")) {
			strcpy(sel,"selected");
			flg = 0;
		} else {
			sel[0] = 0x00;
		}
		gsprintf(temp1, temp,"RETURN",sel,"RETURN" );
		gstrcat(gCache.ipTable.targetHtml, temp1);
		
		if (flg && !strcmp(gCache.ipTable.rule.target,"LOG")) {
			strcpy(sel,"selected");
			flg = 0;
		} else {
			sel[0] = 0x00;
		}
		gsprintf(temp1, temp,"LOG",sel,"LOG" );
		gstrcat(gCache.ipTable.targetHtml, temp1);
		
		if (flg && !strcmp(gCache.ipTable.rule.target,"REJECT")) {
			strcpy(sel,"selected");
			flg = 0;
		} else {
			sel[0] =0x00;
		}
		gsprintf(temp1, temp,"REJECT",sel,"REJECT" );
		gstrcat(gCache.ipTable.targetHtml, temp1);
		
		// chain defined by user
		if (gCache.ipTable.confNumChains > 3) {
			for (i=3; i < gCache.ipTable.confNumChains; i++) {
				chain = &gCache.ipTable.chainTable[i];
				if (chain->action == kDelete) {
					continue;
				}
				if (!strncmp(chain->name, gCache.ipTable.chainName,
								strlen(gCache.ipTable.chainName))) {
					continue;
				}
				if (flg && !strcmp(gCache.ipTable.rule.target,chain->name)) {
					strcpy(sel,"selected");
					flg = 0;
				} else {
					sel[0] = 0x00;
				}
				gsprintf(temp1, temp,chain->name,sel,chain->name);
				gstrcat(gCache.ipTable.targetHtml, temp1);
			}
		}
	}

	return mAppError(kCycSuccess);
}

// Access Method
#ifdef PCMCIA
extern int GetAccessMethodInfo(Param* param, int request)
{
	int i;
	char *ptr, *str;
   Result result;
   gCacheLoaded[kUnitPcmcia] = true;

   result = LoadUnitPcmciaInfo();

	for (i=0; i<gCache.unitInfo.numPcmcia; i++)
	{
		if (strstr(gCache.unitInfo.pcmcia[i].status, "no card") == NULL) {
			if((str = ptr = strchr(gCache.unitInfo.pcmcia[i].identInfo, ':')) != NULL) {
				str++;
				while (ptr && *ptr != '\n') ptr++;
				if (ptr) *ptr = 0x00;
				sprintf(gCache.accessMethod.ident[i], "%s", str);
				//remove quotes
				str = ptr = gCache.accessMethod.ident[i];
				while(*ptr) {
					if (*ptr == '"') {
						str = ptr;
						do {
							*str = *(str+1);
							str++;
						} while (*str);
					} else {
						ptr++;
					}
				}
			}
		} else {
			str = ptr = gCache.unitInfo.pcmcia[i].status;
			while (ptr && *ptr != '\n') ptr++;
			if (ptr) *ptr = 0x00;
         sprintf(gCache.accessMethod.ident[i], "%s", str);
		}
	}

	result = LoadFirst(LoadAccessMethod, kAccessMethod, &gCache.accessMethod, &gCacheSave.accessMethod, sizeof(PcmciaAccessMethod));

   return mAppError(result);
}
#endif

/****************************************************/
/* EXPERT -> SECURITY -> ACTIVE PORT SESSION */
/****************************************************/
extern int GetActiveSessions(Param* param, int request)
{
   Result result;
   ActiveSessionInfo *sessptr, *endsessptr;
   char Template[128];
   char User[9], StartTime[8], Tty[12];
   int i, nsess;
   int cnt = 0;			/* number of listed connections */
#ifdef RDPenable
   tRDPconnect * rdpconnptr = RDPconnect;	/* pointer to RDP connection structures */
   tRDPconnect * const rdpendptr = RDPconnect + MAXRDPCONNECT;
   tRDPconnect * ActiveRdpConnections[MAXRDPCONNECT];
   int NumRdpConnections = 0;	/* number of active RDP connections */
#endif	//RDPenable

   if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

   result = LoadFirst(LoadActiveSession, kActiveSess, &gCache.session, &gCacheSave.session, sizeof(ActivePortSession));

#ifdef RDPenable
   for (; rdpconnptr < rdpendptr; rdpconnptr++)
     if (rdpconnptr->cliport && RdpConnectionExist(rdpconnptr->cliport)) {
       ActiveRdpConnections[NumRdpConnections] = rdpconnptr;
       rdpconnptr->rdpasix = NumRdpConnections;
       NumRdpConnections++;
     }

   gCache.session.totalSessions = nsess = gCache.session.numSessions +  NumRdpConnections;
#else
   gCache.session.totalSessions = nsess = gCache.session.numSessions;
#endif	//RDPenable

   /* free up the the allocated memory */
   bfreeSafe(B_L, gCache.session.sessionHtml);

   if (nsess) {
	/* Allocate memory for the html buffer initialize it as empty string */
	if ((gCache.session.sessionHtml = (char_t *) balloc(B_L, nsess * 512)) == NULL)
	  return mAppError(kMemAllocationFailure); //mp
	*gCache.session.sessionHtml = '\0';

	endsessptr = gCache.session.sess + gCache.session.numSessions;
	for (sessptr = gCache.session.sess; sessptr < endsessptr; sessptr++) {
		gsprintf(Template,  "   <option value=\"%d\" %s>", cnt, (cnt == 0) ? "SELECTED" : " ");
		gstrcat(gCache.session.sessionHtml, Template);
		*User = '\0'; strncat(User, sessptr->user, 8);
		*StartTime = '\0'; strncat(StartTime, sessptr->loginTime, 8);
		*Tty = '\0'; strncat(Tty, sessptr->tty, 11);
		/* Each string is left-justified(-) on the field boundary and padded with blanks on the right to field-width */
		gsprintf(Template, "%-8s  %-11s  %-15s  %-8s  %-7s  %-7s  %-7s  %-7s  </option>\n", 
			User, Tty, sessptr->fromIp, StartTime,
			sessptr->idle, sessptr->jcpu, sessptr->pcpu, sessptr->what);
		ReplaceSPwithNBSP(gCache.session.sessionHtml + strlen(gCache.session.sessionHtml), Template);
		cnt++;
	}

#ifdef RDPenable
	for (i = 0; i < NumRdpConnections; i++) {
		gsprintf(Template,  "   <option value=\"%d\" %s>", cnt, (cnt == 0) ? "SELECTED" : " ");
		gstrcat(gCache.session.sessionHtml, Template);
		*User = '\0'; strncat(User, ActiveRdpConnections[i]->kvmuser, 8);
		*StartTime = '\0';
		strftime(StartTime, 7, "%l:%M%P", localtime((time_t*)&ActiveRdpConnections[i]->conn_time));
		gsprintf(Template, "%-8s  %-11s  %-15s  %-35s  </option>\n", 
			User, "-", ActiveRdpConnections[i]->cliip, StartTime);
		ReplaceSPwithNBSP(gCache.session.sessionHtml + strlen(gCache.session.sessionHtml), Template);
		cnt++;
	}
#endif	//RDPenable
   }
   else {
     // Put in blank spaces into Html string so that it will at least fill up option box
     if (!(gCache.session.sessionHtml = balloc(B_L, 512)))
       return mAppError(kMemAllocationFailure); //mp
     *gCache.session.sessionHtml = '\0';
     gstrcat(gCache.session.sessionHtml, "   <option value=\"-1\" selected>");
     for (i = 0; i < 77; i++)
       gstrcat(gCache.session.sessionHtml, "&nbsp");
     gstrcat(gCache.session.sessionHtml, "</option>\n");
   }

   return mAppError(result);
}

/******************************************/
/* EXPERT -> SECURITY -> SECURITY PROFILE */
/******************************************/
/* [LMT] Security Enhancements */
extern int GetSecProfile(Param* param, int request)
{
        Result result;

        if ((result = GetCycUserType(1)) != kCycSuccess)
                return mAppError(result);

        return mAppError(LoadFirst(LoadSecProfile, kSecProfile, &gCache.secProfile, &gCacheSave.secProfile, sizeof(SecProfile)));
}

/* ------------------------------------------------------------------------- */
/* Register parameter changes functions */
/* ------------------------------------------------------------------------- */

void RegisterParamChange(int kPorts, void* updatedValue, CycCategory category, CycKey key, UInt32 collectionSize)
{
   if (0 == gCurrentChangePos % kChangesGrowthSize) {
      // Grow the gChanges array:
      CycParam* old = gChanges;
      if (!(gChanges = balloc(B_L, sizeof(CycParam) * (gCurrentChangePos + kChangesGrowthSize))))
		  return; //mp
      if (gCurrentChangePos)
         memcpy(gChanges, old, gCurrentChangePos * sizeof(CycParam));
      bfreeSafe(B_L, old);
   }

   if (kPorts != 0 && gKportSpec == 0) {
      gKportSpec = 1;
      if (gWizard) {
         /* register - Delete all specific parameters */
         RegisterParamChange(0, 0, kPortsSpec, kDeleteEntry, 0);
         /* register the portBundle == all ports */
         RegisterParamChange(0, allPorts, kPortsSpec, kPortBundle, 0);
      } else {
         // Fake kPorts Spec for expert mode (the final code must be done)
         RegisterParamChange(0, gPorts, kPortsSpec, kPortBundle, 0);
      }
   }

   gChanges[gCurrentChangePos].category = category;
   gChanges[gCurrentChangePos].key = key;
   gChanges[gCurrentChangePos].collectionSize = collectionSize;
   gChanges[gCurrentChangePos].value = updatedValue;
   ++gCurrentChangePos;
}

#define mRegisterParamChangeDT(branch, param, category, key, nElem) \
if (0 != memcmp(gCache.branch.param, gCacheSave.branch.param, nElem * sizeof(*gCache.branch.param))) { \
   if (gReq.action != kNone) \
       RegisterParamChange(0, gCache.branch.param, category, key, nElem); \
   else \
      gLedState++; \
}
#define mRegisterParamChangeT(branch, param, category, key, nElem) \
if (0 != memcmp(&gCache.branch.param, &gCacheSave.branch.param, nElem * sizeof(gCache.branch.param))) { \
   if (gReq.action != kNone) \
       RegisterParamChange(0, &gCache.branch.param, category, key, nElem); \
   else \
      gLedState++; \
}
#define mRegisterParamChangeT2(param, category, key, nElem) \
if (0 != memcmp(&gCache.param, &gCacheSave.param, nElem * sizeof(gCache.param))) { \
   if (gReq.action != kNone) \
       RegisterParamChange(0, &gCache.param, category, key, nElem); \
   else \
      gLedState++; \
}
#define mRegisterParamChangeS(branch, param, category, key, nElem) \
if (0 != gstrcmp(gCache.branch.param, gCacheSave.branch.param)) { \
   if (gReq.action != kNone) \
       RegisterParamChange(0, &gCache.branch.param, category, key, nElem); \
   else \
      gLedState++; \
}

#define mRegisterParamChangeI(branch, param, category, key, nElem) \
if (gCache.branch.param != gCacheSave.branch.param) { \
   if (gReq.action != kNone) \
       RegisterParamChange(0, &gCache.branch.param, category, key, nElem); \
   else \
      gLedState++; \
}

#define pRegisterParamChangeT(branch, param, category, key, nElem) \
if (0 != memcmp(&gCache.branch.param, &gCacheSave.branch.param, sizeof(gCache.branch.param))) { \
   if (gReq.action != kNone) \
       RegisterParamChange(1, &gCache.branch.param, category, key, nElem); \
   else \
      gLedState++; \
}

#define pRegisterParamChangeS(branch, param, category, key, nElem) \
if (0 != gstrcmp(gCache.branch.param, gCacheSave.branch.param)) { \
   if (gReq.action != kNone) \
       RegisterParamChange(1, &gCache.branch.param, category, key, nElem); \
   else \
      gLedState++; \
}

#define pRegisterParamChangeI(branch, param, category, key, nElem) \
if (gCache.branch.param != gCacheSave.branch.param) { \
   if (gReq.action != kNone) \
       RegisterParamChange(1, &gCache.branch.param, category, key, nElem); \
   else \
      gLedState++; \
}

/* ------------------------------------------------------------------------- */
/* Other functions called by Commit or Query functions */
/* ------------------------------------------------------------------------- */

static int SetNetSettings(void)
{
	int dhcpChanged = 0;

	if (!gCacheLoaded[kNetSettings]) return(0); // The page wasn't even loaded

	/* Register changes: */
	if (gCache.netSettings.dhcp != gCacheSave.netSettings.dhcp) {
		if (gReq.action != kNone) 
			RegisterParamChange(0, &gCache.netSettings.dhcp, kPslave, kDhcpClient,0); 
		else
			gLedState++; 
		dhcpChanged = 1;
	}
#ifdef IPv6enable
	mRegisterParamChangeS(netSettings, hostName, kHostnameSetting, kHostName, 0);
#else
	if (!gCache.netSettings.dhcp || gCache.netSettings.mode) {
		mRegisterParamChangeS(netSettings, hostName, kHostnameSetting, kHostName, 0);
	}
#endif
   
	if (!gCache.netSettings.dhcp) {
		if (dhcpChanged && (gReq.action != kNone)) {
			RegisterParamChange(0, &gCache.netSettings.ipAddress1, kPslave, kIpAddress, 0); 
		} else {		
			mRegisterParamChangeS(netSettings, ipAddress1, kPslave, kIpAddress, 0);
		}
		mRegisterParamChangeS(netSettings, netMask1, kPslave, kNetMask, 0);
		mRegisterParamChangeT(netSettings, dnsService, kResolvConf, kDnsService, 1);
#ifdef IPv6enable
		if (gCache.netSettings.enableIPv4) {
			mRegisterParamChangeS(netSettings, gateway, kStRoutes, kIpAddress, 0);
		}
#else
		mRegisterParamChangeS(netSettings, gateway, kStRoutes, kIpAddress, 0);
#endif
	}
	if (gCache.netSettings.mode) {
		mRegisterParamChangeS(netSettings, consBanner, kIssueSetting, kConsoleBanner, 0);
		if (!gCache.netSettings.dhcp) {
			mRegisterParamChangeS(netSettings, ipAddress2, kPslave, k2ndIpAddress, 0);
			mRegisterParamChangeS(netSettings, netMask2, kPslave, k2ndNetMask, 0);
			mRegisterParamChangeI(netSettings, mtu, kPslave, kMtu, 0);
		}
	}
#ifdef BONDING_FEATURE
   //germano
   //check configuration changes for bonding
   if (gCache.netSettings.bond.enabled != gCacheSave.netSettings.bond.enabled){
        mRegisterParamChangeI(netSettings, bond.enabled, kBonding, kBondingEnable, 1);
        gLedState++;
   }
   if(gCache.netSettings.bond.enabled == kYes){
                if(gCache.netSettings.bond.miimon != gCacheSave.netSettings.bond.miimon){
                        mRegisterParamChangeI(netSettings, bond.miimon, kBonding, \
                        kBondingMiimon, 1);
                        gLedState++;
                }

                if(gCache.netSettings.bond.updelay != gCacheSave.netSettings.bond.updelay){
                        mRegisterParamChangeI(netSettings, bond.updelay, kBonding, \
                        kBondingUpdelay, 1);
                        gLedState++;
                }
   }
#endif
#ifdef IPv6enable
	if (gCache.netSettings.DHCPv6Opts != gCacheSave.netSettings.DHCPv6Opts) {
		if (gReq.action != kNone) 
			RegisterParamChange(0, &gCache.netSettings.DHCPv6Opts, kPslave, kDHCPv6Opts, 0);
		else
			gLedState++;
	}
	if (gCache.netSettings.enableIPv4 != gCacheSave.netSettings.enableIPv4) {
		mRegisterParamChangeI(netSettings, enableIPv4, kPslave, kEnableIPv4, 0);
		gLedState++;
	}
	if (gCache.netSettings.enableIPv6 != gCacheSave.netSettings.enableIPv6) {
		mRegisterParamChangeI(netSettings, enableIPv6, kPslave, kEnableIPv6, 0);
		gLedState++;
	}
	if (gCache.netSettings.IPv6method != gCacheSave.netSettings.IPv6method) {
		mRegisterParamChangeI(netSettings, IPv6method, kPslave, kIPv6method, 0);
		gLedState++;
	}
	//if (gCache.netSettings.IPv6method == 1) {
		if (strcmp(gCache.netSettings.ethIp6, gCacheSave.netSettings.ethIp6)) {
			mRegisterParamChangeS(netSettings, ethIp6, kPslave, kEthIp6, 0);
			gLedState++;
		}
		if (gCache.netSettings.ethPrefix6 != gCacheSave.netSettings.ethPrefix6) {
			mRegisterParamChangeI(netSettings, ethPrefix6, kPslave, kEthPrefix6, 0);
			gLedState++;
		}
	//}
	if (gCache.netSettings.enableIPv6 && strcmp(gCache.netSettings.gateway6, gCacheSave.netSettings.gateway6)) {
		mRegisterParamChangeS(netSettings, gateway6, kStRoutes, kEthIp6, 0);
		gLedState++;
	}
#endif
   return (0);
}

//[AP][2007-02-08] hostname discovery
static int SetAutoDiscovery(void)
{
   if (!gCacheLoaded[kAutoDiscovery]) return(0);

   if (/*(gCache.autoDiscovery.ADStringChange & 1) != 0*/strcmp(gCache.autoDiscovery.ADProbeString, gCacheSave.autoDiscovery.ADProbeString)) {
	if (gReq.action != kNone) 
		RegisterParamChange(0, gCache.autoDiscovery.ADProbeString, kPslave, kADProbeStrings,0); 
	else 
		gLedState++; 
   }
   if (/*(gCache.autoDiscovery.ADStringChange & 1) != 0*/strcmp(gCache.autoDiscovery.ADAnswerString, gCacheSave.autoDiscovery.ADAnswerString)) {
	if (gReq.action != kNone) 
		RegisterParamChange(0, gCache.autoDiscovery.ADAnswerString, kPslave, kADAnswerStrings,0); 
	else 
		gLedState++; 
   }
   
   return (0);
}

static int SetCasProSettings(void)
{

   if (!gCacheLoaded[kCasProSettings]) return(0); // The page was even loaded

   /* Register changes: */
  //[RK]Dec/05/05 - implement the enableALL in the wizard 
   if (gCache.casProSettings.enableAll != 0) {
        RegisterParamChange(0, &gCache.casProSettings.enableAll, kPslave, kTty, 0);
	gLedState++;
   }


   pRegisterParamChangeI(casProSettings, protocol, kPslave, kProtocol, 0);
   pRegisterParamChangeI(casProSettings, baudRate, kPslave, kBaudRate, 0);
   pRegisterParamChangeI(casProSettings, dataSize, kPslave, kBits, 0);
   pRegisterParamChangeI(casProSettings, stopBits, kPslave, kStopBits, 0);
   pRegisterParamChangeI(casProSettings, parity, kPslave, kParity, 0);
   pRegisterParamChangeI(casProSettings, flowControl, kPslave, kFlow, 0);
   pRegisterParamChangeI(casProSettings, authReq, kPslave, kAuth, 0);

   return(0);
}

static int SetAccess(void)
{
	if (gReq.action != kNone) {
		if (gCacheLoaded[kAccessUsers] && gCache.access.changedUser) {
			RegisterParamChange(0,gCache.access.users,kPasswd,kUsers,gCache.access.numUsers);
		}
		if (gCacheLoaded[kAccessGroups] && gCache.access.changedGrp) {
#if defined (KVM) || defined (ONS)
			RegisterParamChange(0, gCache.access.groups, kKVMGroup, kKVMGroups, gCache.access.numGroups);
#else
			RegisterParamChange(0, gCache.access.groups, kPslave, kConfGroup, gCache.access.numGroups);
#endif
		}
	}
	return(0); 
}

static int SetMenuShell(void)
{
   char_t *Cs = NULL;
   int i, change=0;
   MenuShInfo *menu;
   MenuShOpt *newOpt, *oldOpt, *old;
   char *ptr, *str;

   if (!gCacheLoaded[kMenuShell]) return(0); 

   // The page was loaded before
   Cs = gCache.menuShell.optionsCs;
   menu = &gCache.menuShell.menuShInfo;

	if (gCache.menuShell.action == kDelete) change=1;
	if (gCache.menuShell.menuShInfo.numOptions &&(*Cs || gCache.menuShell.action == kDelete))
	{
		oldOpt = menu->optInfo;
		i = sizeof(MenuShOpt) * gCache.menuShell.menuShInfo.numOptions;
		if (!(menu->optInfo = balloc(B_L, i)))
			return kMemAllocationFailure; //mp
		memset(menu->optInfo, 0, i);

		for (i=0, newOpt = menu->optInfo, old=oldOpt; i < gCache.menuShell.menuShInfo.numOptions; i++, newOpt++) 
		{
			ptr = strstr(Cs,",");
			if (ptr) *ptr = 0x00;
			str = strstr(Cs, "!");
			if (str) *str++ = 0x00;
			if (Cs) strcpy(newOpt->title, Cs);
			if (str) strcpy(newOpt->command, str);
			Cs = ptr+1;
			if (old) {
				if ((strcmp(newOpt->title, old->title) || strcmp(newOpt->command, old->command))) {
					change = 1;
				} else {
					old++;
				}
			} else {
				change = 1;
			}
		}   
	}

   // Clear buffer and reset flags
   memset(gCache.menuShell.optionsCs, 0, sizeof(gCache.menuShell.optionsCs));
   gCache.menuShell.action = 0;

	if (memcmp(&gCache.menuShell.menuShInfo.menuTitle, 
		&gCacheSave.menuShell.menuShInfo.menuTitle, kTitleLength))
		change = 1;
	if (change || gCache.menuShell.menuShellChanged)	{
		if (gReq.action != kNone) {
				RegisterParamChange(0, &gCache.menuShell.menuShInfo, 
					kMenuShCfg, kMenuShInfo, gCache.menuShell.menuShInfo.numOptions);
		} else {
				gLedState++;
				gCache.menuShell.menuShellChanged = 1;
		}
   }
   return(0); 
}

static int SetDataBuffering(void)
{
   if (!gCacheLoaded[kDataBuffering]) return(0); // The page was even loaded

   /* Register changes: */
   pRegisterParamChangeI(dataBuffering, mode, kPslave, kDbMode, 0);
   pRegisterParamChangeI(dataBuffering, fileSize, kPslave, kDbFileSize, 0);
   mRegisterParamChangeS(dataBuffering, nfsPath, kPslave, kNfsFilePath, 0);
   pRegisterParamChangeI(dataBuffering, showMenu, kPslave, kShowMenu, 0);
   pRegisterParamChangeI(dataBuffering, timeStamp, kPslave, kTimeStamp, 0);
   return (0);
}

static int SetSystemLog(void)
{
   // verify servers 
   char_t *pi, *pf, *Cs, *pnew[30];
   SyslogServerInfo* pser;
   int i,news=0, change=0;
   
   if (!gCacheLoaded[kSystemLog]) return(0); // The page was even loaded

   memcpy(gCacheSave.systemLog.serversCs,gCache.systemLog.serversCs,kListLength);

   Cs = gCacheSave.systemLog.serversCs;   
   pser = gCache.systemLog.servers;
      
   for (i=0; i < gCache.systemLog.numServers ; i++,pser++) {
      if (!*Cs) {
         pser->action = kDelete;
         change = 1;
         continue;
      }

      if ((pi = strstr(Cs, pser->SyslogServer)) == NULL) {
         // server was deleted
         pser->action = kDelete;
         change = 1;
      } else { 
         if (pser->action != kAdd) {
            pser->action = 0; // None
         } else {
            change = 1;
         }
         pf = strstr(pi,",");
         pf++;
         strcpy(pi,pf);
      }
   }
   // adds
   pi = Cs;
   while (*pi) {
      while (*pi && *pi == ',') pi++;
      if (!*pi) break;
   
      if ((pf = strstr(pi,",")) == NULL) {
         break;
      }
      *pf = 0x00;
      pnew[news++]=pi;
      pi = ++pf;
   }
   if (news) {
      SyslogServerInfo* old = gCache.systemLog.servers;
      if (!(gCache.systemLog.servers = 
			balloc(B_L, sizeof(SyslogServerInfo) * (gCache.systemLog.numServers + news))))
		  return kMemAllocationFailure; //mp
      memcpy(gCache.systemLog.servers, old, gCache.systemLog.numServers * sizeof(SyslogServerInfo));
      bfreeSafe(B_L, old);
      pser = &gCache.systemLog.servers[gCache.systemLog.numServers];
      for (i=0; i < news; i++, pser++) {
         pser->action = kAdd;
         pser->ServerId = 0;
         strcpy(pser->SyslogServer,pnew[i]);
      }
      gCache.systemLog.numServers += news;
      change = 1;
   }
   
   /* Register changes:  all.facility */
   mRegisterParamChangeI(systemLog, facilityNumber, kPslave, kConfFacilityNum, 0);
   if (change) {
      if (gReq.action != kNone) {
         RegisterParamChange(0, gCache.systemLog.servers, kSyslogNg, kSyslogServer, gCache.systemLog.numServers);
      } else {
         gLedState++;
      }
   }

// Register filters by levels
   gCache.systemLog.filter.CASfacility = gCache.systemLog.facilityNumber;
   if (change
    || gCache.systemLog.filter.CASlevel != gCacheSave.systemLog.filter.CASlevel
    || gCache.systemLog.filter.KVMlevel != gCacheSave.systemLog.filter.KVMlevel
    || gCache.systemLog.filter.Buflevel != gCacheSave.systemLog.filter.Buflevel
    || gCache.systemLog.filter.Weblevel != gCacheSave.systemLog.filter.Weblevel
    || gCache.systemLog.filter.Syslevel != gCacheSave.systemLog.filter.Syslevel
    || gCache.systemLog.filter.CASfacility != gCacheSave.systemLog.filter.CASfacility) {
      if (gReq.action != kNone) {
         RegisterParamChange(0, &gCache.systemLog.filter, kSyslogNg, kLogFilterlevel, 0);
         change = 1;
      } else {
         gLedState++;
      }
   }

// register dest anyway in order to have consistent filter/dest/log.
// routine must run after filters and servers update.

   if (change || gCache.systemLog.dest != gCacheSave.systemLog.dest) {
      if (gReq.action != kNone) {
         RegisterParamChange(0, &gCache.systemLog.dest, kSyslogNg, kLogDestEnable, 0);
      } else {
         gLedState++;
      }
   }

   return (0);
}

#ifdef PMD
static int replaceName(char *old, char *new, int type)
{
	char *list, *t, *u, *v;
	int i;
        Result result;

	result = LoadFirst(LoadpmdOutGrp, kPMDOutletGroups, &gCache.pmdOutGrp, NULL, 0); 
	if (result != kCycSuccess) {
		return result;
	}
	result = LoadFirst(LoadpmdUserMan, kPMDUsersManagement, &gCache.pmdUserMan, NULL, 0);
	if (result != kCycSuccess) {
		return result;
	}
	for (i = 0; i < gCache.pmdOutGrp.numGroups + gCache.pmdUserMan.numUsers; i ++) {
		if (i < gCache.pmdOutGrp.numGroups) {
			list = gCache.pmdOutGrp.groups[i].outletList;
		} else {
			list = gCache.pmdUserMan.usersMgm[i - gCache.IpduOutGrpCtrl.totalGroups].outletList;
		}
		t = list;
		while (*t) {
			u = t;
			while (*u && *u != ',' && !isspace(*u)) u ++;
			if (type) {
				v = t;
				while (v < u && *v != '[') v ++;
			} else {
				v = u;
			}
			if (v - t == strlen(old) && ! strncmp(t, old, v - t)) {
				if (new && *new) {
					char *aux = strdup(v);
					strcpy(t, new);
					strcpy(t + strlen(new), aux);
					free(aux);
				} else {
					while (*u == ',' || isspace(*u)) u ++;
					if (*u) {
						strcpy(t, u);
					} else {
						u = t - 1;
						while (u >= list && (isspace(*u) || *u == ',')) u --;
						u ++;
						if (u == list) {
							if (i < gCache.pmdOutGrp.numGroups) {
								gCache.pmdOutGrp.groups[i].action = kDelete;
							} else {
								gCache.pmdUserMan.usersMgm[i - gCache.IpduOutGrpCtrl.totalGroups].action = kDelete;
							}
						}
						*u ++ = '\n';
						*u ++ = 0;
					}
				}
				if (i < gCache.pmdOutGrp.numGroups) {
					gCache.pmdOutGrp.changed = TRUE;
				} else {
					gCache.pmdUserMan.changed = TRUE;
				}
			}
			while (*u == ',' || isspace(*u)) u ++;
			t = u;
		}
	}
	if (gCache.pmdOutGrp.changed) {
		if (gReq.action != kNone) {
			RegisterParamChange(0, gCache.pmdOutGrp.groups, kPMDConfig, 
				kPMDOutGrpsDef, gCache.pmdOutGrp.numGroups );
			gCache.pmdOutGrp.changed = FALSE;
		} else {
			gLedState++;
		}
	}
	strcpy(gCache.pmdOutGrp.entries, "unchanged");
	gCache.pmdOutGrp.addEditEntry = kNone;

	if (gCache.pmdUserMan.changed) {
		if (gReq.action != kNone) {
			RegisterParamChange(0, gCache.pmdUserMan.usersMgm, kPMDConfig, 
				kPMDUsersMgm, gCache.pmdUserMan.numUsers );
			gCache.pmdUserMan.changed = FALSE;
		} else {
			gLedState++;
		}
	}
	strcpy(gCache.pmdUserMan.entries, "unchanged");
	gCache.pmdUserMan.addEditEntry = kNone;
	return result;
}

// Power Management - New Outlets Manager page
static int SetIpduOutMan(void)
{
	int  who, edit_error=0, spcop=0;
	char *aux;

	int portindex=0, ipduindex=0, outletN=0; 
	void *outRef;

//	if (!gCacheLoaded[kIPDUOutletsManager]) return(0); //The page wasn't even loaded

	//printf("<----- SetIpduOutMan()\n");
	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);

	switch (gCache.IpduOutMan.action) {
	case no_Action:
	case Done:
	case Automatic_reload:
	case Page_reload:
		break;
	default:
		if (gCache.IpduOutMan.memSignature != checkPortsIpdusSignature()) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "ACS detected changes on IPDU configuration.\\n"
				   "Please try the last operation again...");
			CycSetshowIpdu(&gReq.sid,0);
			return 0;
		}
	}

//syslog(4,"%s: action[%d]",__func__,gCache.IpduOutMan.action);

	switch (gCache.IpduOutMan.action) {
	case no_Action:
		CycSetshowIpdu(&gReq.sid,0);
		break;
	case Show:
		who = atoi(gCache.IpduOutMan.sysMsg) - 1;
		portindex = who / 1000;
		ipduindex = who % 1000;
		CycSetshowIpdu(&gReq.sid,who+1);
		////syslog(4,"%s: ##### portindex <%d> ######",__func__, portindex);
		//syslog(4,"%s: ###### ipduindex <%d> ######",__func__,ipduindex);
		break;
	case Done:
		//printf("###### 2nd level Done ######\n");
		CycSetshowIpdu(&gReq.sid,0);
		break;
	case Save:
		who = atoi(gCache.IpduOutMan.sysMsg) - 1;
		portindex = who / 1000;
		ipduindex = who % 1000;
		if (cycPMDngSaveCmd(PMDngUserName,
			gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].shmRef)) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Save command!");
		} else {
			if ((gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_spc) ||
			    (gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_servertech)) {
				CycSetshowIpdu(&gReq.sid,0);
				gCache.IpduOutMan.action = Msg_to_WEB;
				strcpy(gCache.IpduOutMan.sysMsg, "restart");
			}
		}
		break;
	case Turn_ON:
		who = atoi(gCache.IpduOutMan.sysMsg);
		outletN = who % 1000;
		portindex = (who / 1000);
		ipduindex = portindex % 1000;
		portindex /= 1000;

		if ((outRef = cycPMDngOutletPtr(PMDngUserName,gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].name, 
					outletN)) == NULL) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Turn ON command!");
			break;
		}

		if (cycPMDngChangeState(PMDngUserName,outRef, kPmOn)) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Turn ON command!");
		} else {
			if ((gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_spc) ||
			    (gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_servertech)) {
				spcop = 1;
			}
		}
		break;
	case Turn_OFF:
		who = atoi(gCache.IpduOutMan.sysMsg);
		outletN = who % 1000;
		portindex = who / 1000;
		ipduindex = portindex % 1000;
		portindex /= 1000;
		if ((outRef = cycPMDngOutletPtr(PMDngUserName,gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].name, 
					outletN)) == NULL) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Turn OFF command!");
			break;
		}

		if (cycPMDngChangeState(PMDngUserName,outRef,kPmOff)) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Turn OFF command!");
		} else {
			if ((gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_spc) ||
			    (gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_servertech)) {
				spcop = 1;
			}
		}
		break;
	case Lock:
		who = atoi(gCache.IpduOutMan.sysMsg);
		outletN = who % 1000;
		portindex = who / 1000;
		ipduindex = portindex % 1000;
		portindex /= 1000;
		if ((outRef = cycPMDngOutletPtr(PMDngUserName,gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].name, 
					outletN)) == NULL) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Lock command!");
			break;
		}

		if (cycPMDngChangeState(PMDngUserName,outRef,kPmLock)) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Lock command!");
		}
		break;
	case Unlock:
		who = atoi(gCache.IpduOutMan.sysMsg);
		outletN = who % 1000;
		portindex = who / 1000;
		ipduindex = portindex % 1000;
		portindex /= 1000;
		if ((outRef = cycPMDngOutletPtr(PMDngUserName,gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].name, 
					outletN)) == NULL) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Unlock command!");
			break;
		}

		if (cycPMDngChangeState(PMDngUserName,outRef,kPmUnlock)) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Unlock command!");
		}
		break;
	case Cycle:
		who = atoi(gCache.IpduOutMan.sysMsg);
		outletN = who % 1000;
		portindex = who / 1000;
		ipduindex = portindex % 1000;
		portindex /= 1000;
		if ((outRef = cycPMDngOutletPtr(PMDngUserName,gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].name, 
					outletN)) == NULL) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Reboot command!");
			break;
		}

		if (cycPMDngChangeState(PMDngUserName,outRef,kPmCycle)) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Reboot command!");
		} else {
			if ((gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_spc) ||
			    (gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].type == Vendor_servertech)) {
				spcop = 1;
			}
		}
		break;
	case Edit:
		aux = gCache.IpduOutMan.sysMsg;
		while (*aux != ' ') aux++;
		*aux++ = '\0';
		who = atoi(gCache.IpduOutMan.sysMsg);
		outletN = who % 1000;
		portindex = who / 1000;
		ipduindex = portindex % 1000;
		portindex /= 1000;
		if ((outRef = cycPMDngOutletPtr(PMDngUserName,gCache.IpduOutMan.portInfo[portindex].ipdu[ipduindex].name, 
					outletN)) == NULL) {
			gCache.IpduOutMan.action = Msg_to_WEB;
			strcpy(gCache.IpduOutMan.sysMsg, "Couldn't send Turn ON command!");
			break;
		}

		{
			char name[50];
			int minOnTime = 0, minOffTime=0,wakeupState=0;
			double postOnDelay =0;
			//[RK] May/2009 - ZEUS
			double postOffDelay =0;
			double thre_hc=0, thre_lc=0, thre_hw=0, thre_lw=0;
			sscanf(aux, "%s %d %d %lf %d %lf %lf %lf %lf %lf",
			   name, (int*)&minOnTime,(int*)&minOffTime,(double*)&postOnDelay,(int*)&wakeupState,
			   (double*)&postOffDelay,(double*)&thre_hc,(double*)&thre_lc,(double*)&thre_hw,(double*)&thre_lw);
	
			edit_error = 0;
			if (strcmp(name, "editedit")) { //check if name can be changed
				char *oldname = strdup(cycPMDngGetOutName(outRef));
				if (!strcmp(name, "blankblank")) name[0] = '\0';
				if ((edit_error=cycPMDngConfOutName(PMDngUserName, outRef, name))) {
					edit_error += 1;
				} else if (strcmp(oldname, cycPMDngGetOutName(outRef))) {
					replaceName(oldname, cycPMDngGetOutName(outRef), 0);
				}
				free(oldname);
			}
			if (!edit_error && cycPMDngConfIntervals(PMDngUserName, outRef,
					  (double)minOnTime, OUTLET_CAP_MINIMUMON))
				edit_error = 1;
			if (!edit_error && cycPMDngConfIntervals(PMDngUserName, outRef,
					  (double)minOffTime, OUTLET_CAP_MINIMUMOFF))
				edit_error = 1;
			if (!edit_error && cycPMDngConfIntervals(PMDngUserName, outRef,
						 (double) postOnDelay,
						  OUTLET_CAP_POSTPOWERON))
				edit_error = 1;
			if (!edit_error && cycPMDngConfIntervals(PMDngUserName, outRef,
						 (double) postOffDelay,
						  OUTLET_CAP_POSTPOWEROFF))
				edit_error = 1;
			if (!edit_error && cycPMDngConfThreOut(PMDngUserName, outRef,
						 (double)thre_hc*10,(double)thre_lc*10,
						 (double)thre_hw*10,(double)thre_lw*10))
				edit_error = 1;
			if (!edit_error && cycPMDngConfWakeUpState(PMDngUserName, outRef, wakeupState))
				edit_error = 1;
			if (edit_error) {
				gCache.IpduOutMan.action = Msg_to_WEB;
				if (edit_error == 1) {
					strcpy(gCache.IpduOutMan.sysMsg, "Couldn't Edit outlet!");
				} else {
					if (edit_error == 2) {
						strcpy(gCache.IpduOutMan.sysMsg, "The outlet name is invalid");
					} else {
						strcpy(gCache.IpduOutMan.sysMsg, "Duplicated outlet name");
					}
				}			
 			}
		}
		break;
	case Page_reload:
		gCache.IpduOutMan.action = Msg_to_WEB;
		strcpy(gCache.IpduOutMan.sysMsg, "This page has been successfully reloaded.");
		break;
	case Automatic_reload:
		break;
	default:
		//printf("###### Unknown Action <%d> ######\n", gCache.IpduOutMan.action);
		gCache.IpduOutMan.action = Msg_to_WEB;
		strcpy(gCache.IpduOutMan.sysMsg, "Unknown Action !!!");
	}

	if (spcop) {
		gCache.IpduOutMan.action = Msg_to_WEB;
		strcpy(gCache.IpduOutMan.sysMsg, "reload_outlets");
	}

	return 0;
}

// Power Management - New Outlet Groups Ctrl page
static int SetIpduOutGrpCtrl(void)
{
	int  group, spcop=0;
	char  *who = gCache.IpduOutGrpCtrl.sysMsg;

//	if (!gCacheLoaded[kIPDUOutletGroupsCtrl]) return(0); //The page wasn't even loaded
//	printf("<----- SetIpduOutGrpCtrl()\n");

	switch (gCache.IpduOutGrpCtrl.action) {
	case no_Action:
	case Automatic_reload:
		break;
	default:
		if (gCache.IpduOutGrpCtrl.memSignature != checkGroupsSignature()) {
			gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
			strcpy(gCache.IpduOutGrpCtrl.sysMsg, "ACS detected changes on IPDU configuration.\\n"
				   "Please try the last operation again...");
			return 0;
		}
	}
	
	group = (gCache.IpduOutGrpCtrl.groupInfo != NULL);
	printf("SetIpduOutGrpCtrl(), group: %d\n", group);

	switch (gCache.IpduOutGrpCtrl.action) {
	case Turn_ON:
		//printf("###### TURN ON %s group ######\n", who);
		if ( group ) {
			if ((spcop = cycPMDngOutGrpChangeState(PMDngUserName, who, kPmOn)) != 0)
				break;
		}
		gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
		strcpy(gCache.IpduOutGrpCtrl.sysMsg, "Couldn't send Group Turn ON command!");
		break;
	case Turn_OFF:
		//printf("###### TURN OF %s group ######\n", who);
		if ( group ) {
			if ((spcop = cycPMDngOutGrpChangeState(PMDngUserName, who, kPmOff)) != 0)
				break;
		}
		gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
		strcpy(gCache.IpduOutGrpCtrl.sysMsg, "Couldn't send Group Turn OFF command!");
		break;
	case Lock:
		//printf("###### LOCK %s group ######\n", who);
		if ( group ) {
			if ((spcop = cycPMDngOutGrpChangeState(PMDngUserName, who, kPmLock)) != 0)
				break;
		}
		gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
		strcpy(gCache.IpduOutGrpCtrl.sysMsg, "Couldn't send Group Lock command!");
		break;
	case Unlock:
		//printf("###### UNLOCK %s group ######\n", who);
		if ( group ) {
			if ((spcop = cycPMDngOutGrpChangeState(PMDngUserName, who, kPmUnlock)) != 0)
				break;
		}
		gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
		strcpy(gCache.IpduOutGrpCtrl.sysMsg, "Couldn't send Group Unlock command!");
		break;
	case Cycle:
		//printf("###### CYCLE %s group ######\n", who);
		if ( group ) {
			if ((spcop = cycPMDngOutGrpChangeState(PMDngUserName, who, kPmCycle)) != 0)
				break;
		}
		gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
		strcpy(gCache.IpduOutGrpCtrl.sysMsg, "Couldn't send Group Cycle command!");
		break;
	case Automatic_reload:
		break;
	case no_Action:
		break;
	default:
		//printf("###### Unknown Action <%d> ######\n", gCache.IpduOutGrpCtrl.action);
		gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
		strcpy(gCache.IpduOutGrpCtrl.sysMsg, "Unknown Action !!!");
	}

	if (spcop==2) {
		gCache.IpduOutGrpCtrl.action = Msg_to_WEB;
		strcpy(gCache.IpduOutGrpCtrl.sysMsg, "reload_outlets");
	}

	return 0;
}

// Power Management - New View IPDUs Info page
static int SetViewIpduInfo(void)
{
	int aux, portindex, ipduindex;
	PmIpduPortInfo *port = gCache.ViewConfIpdu.portInfo;

	if (!gCacheLoaded[kViewConfIPDUs]) return(0); //The page wasn't even loaded
	//printf("<----- SetViewIpduInfo()\n");
	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);

	if ((gCache.ViewConfIpdu.action != no_Action) &&
		(gCache.ViewConfIpdu.memSignature != checkPortsIpdusSignature())) {
		gCache.ViewConfIpdu.action = Msg_to_WEB;
		strcpy(gCache.ViewConfIpdu.sysMsg, "ACS detected changes on IPDU configuration.\\n"
						"Please try the last operation again...");
		return 0;
	}

	aux = atoi(gCache.ViewConfIpdu.sysMsg); //get general index
	//prepare port and ipdu indexes
	portindex = aux / 1000;
	ipduindex = aux % 1000;

//syslog(4,"%s: actio[%d] port/ipdu[%d/%d]",__func__,gCache.ViewConfIpdu.action,portindex, ipduindex);
	switch(gCache.ViewConfIpdu.action) {
	case Clear_Current:
		//printf("###### Reset Max. Curr. Detected for Ipdu <%s> ######\n", gCache.ViewConfIpdu.sysMsg);
		if (cycPMDngClearMaxDetected(PMDngUserName,
			 	port[portindex].ipdu[ipduindex].shmRef,
				gCache.ViewConfIpdu.action)) {
			gCache.ViewConfIpdu.action = Msg_to_WEB;
			strcpy(gCache.ViewConfIpdu.sysMsg, "Couldn't send Clear Max. Current command!");
		}
		break;
	case Clear_EnvMon:
		//printf("###### Reset Max. Temp. Detected for Ipdu <%s> ######\n", gCache.ViewConfIpdu.sysMsg);
		if (cycPMDngClearMaxDetected(PMDngUserName,
				 port[portindex].ipdu[ipduindex].shmRef,
				 gCache.ViewConfIpdu.action)) {
			gCache.ViewConfIpdu.action = Msg_to_WEB;
			strcpy(gCache.ViewConfIpdu.sysMsg, "Couldn't send Clear Max. EnvMon  command!");
		}
		break;
	case Clear_Power:
		if (cycPMDngClearMaxDetected(PMDngUserName,
				 port[portindex].ipdu[ipduindex].shmRef,
				 gCache.ViewConfIpdu.action)) {
			gCache.ViewConfIpdu.action = Msg_to_WEB;
			strcpy(gCache.ViewConfIpdu.sysMsg, "Couldn't send Clear Max. Power command!");
		}
		break;
	case Clear_Voltage:
		if (cycPMDngClearMaxDetected(PMDngUserName,
				 port[portindex].ipdu[ipduindex].shmRef,
				 gCache.ViewConfIpdu.action)) {
			gCache.ViewConfIpdu.action = Msg_to_WEB;
			strcpy(gCache.ViewConfIpdu.sysMsg, "Couldn't send Clear Max. Voltage command!");
		}
		break;
	case Clear_PowerFactor:
		if (cycPMDngClearMaxDetected(PMDngUserName,
				 port[portindex].ipdu[ipduindex].shmRef,
				 gCache.ViewConfIpdu.action)) {
			gCache.ViewConfIpdu.action = Msg_to_WEB;
			strcpy(gCache.ViewConfIpdu.sysMsg, "Couldn't send Clear Max. PowerFactor command!");
		}
		break;
	case Reset_HWOvercurrent:
		if (cycPMDngClearMaxDetected(PMDngUserName,
				 port[portindex].ipdu[ipduindex].shmRef,
				 gCache.ViewConfIpdu.action)) {
			gCache.ViewConfIpdu.action = Msg_to_WEB;
			strcpy(gCache.ViewConfIpdu.sysMsg, "Couldn't send Reset HW Overcurrent command!");
		}
		break;
	case no_Action:
		break;
	default:
		//printf("###### Unknown Action <%d> ######\n", gCache.ViewConfIpdu.action);
		gCache.ViewConfIpdu.action = Msg_to_WEB;
		strcpy(gCache.ViewConfIpdu.sysMsg, "Unknown Action !!!");
	}

	return 0;
}

// Power Management - New Configuration page
static int SetIpduConfig(void)
{
	PmIpduPortInfo *port = gCache.ViewConfIpdu.portInfo;
	PmIpduInfo *ipdu;
	PmCurrInfo *elec;
	PmEnvMonInfo *env;
	char *p, *e = gCache.ViewConfIpdu.sysMsg;
	int aux, portindex, ipduindex, i, j, confchg=0,save_flag=0;
	int old_numphases;
	float auxf;

	if (!gCacheLoaded[kViewConfIPDUs]) return(0); //The page wasn't even loaded
	//printf("<----- SetIpduConfig()\n");

	if (!e[0] || !strstr(e, "end") || !strcmp(e, "end"))
		return 0;

	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);
	if (gCache.ViewConfIpdu.memSignature != checkPortsIpdusSignature()) {
		gCache.ViewConfIpdu.action = Msg_to_WEB;
		strcpy(gCache.ViewConfIpdu.sysMsg, "ACS detected changes on IPDU configuration.\\n"
						"Please try the last operation again...");
		return 0;
	}

	gCache.ViewConfIpdu.action = no_Action;
	//printf("Web wrote <%s>\n", e);
//syslog(4,"%s: from web <%s>",__func__,e);

	while (strcmp(e, "end")) {   //process entries if not end
		aux = strtol(e, &p, 10); //get general index
		//prepare port and ipdu indexes
		portindex = aux / 1000;
		ipduindex = aux % 1000;

		//printf("##### portindex<%d>, ipduindex<%d>\n", portindex, ipduindex);

		e = p + 1;              //skip ':' (e points to ipdu name)
		while (*p != ' ') p++;  //p points to the blank after ipdu name
		*p++ = '\0';            //p points to over_curr_prot boolean
		if (strcmp(e, port[portindex].ipdu[ipduindex].name)) {
			strcpy(port[portindex].ipdu[ipduindex].name, e);
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_NAME;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_CONFIGFILE;
		}
		e = p;
		aux = strtol(e, &p, 10); //p points to the blank before syslog (boolean)
		if (aux != port[portindex].ipdu[ipduindex].overcurrent) {
			port[portindex].ipdu[ipduindex].overcurrent = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_OCP;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before buzzer (boolean)
		if (aux != port[portindex].ipdu[ipduindex].syslog) {
			port[portindex].ipdu[ipduindex].syslog = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_SYSLOG;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before poll_rate (integer)
		if (aux != port[portindex].ipdu[ipduindex].buzzer) {
			port[portindex].ipdu[ipduindex].buzzer = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_BUZZER;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before cycle_int (integer)
		if (aux != port[portindex].ipdu[ipduindex].poll_rate) {
			port[portindex].ipdu[ipduindex].poll_rate = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_CONFIGFILE;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before seq_int (integer)
		if (aux != port[portindex].ipdu[ipduindex].cycle_int) {
			port[portindex].ipdu[ipduindex].cycle_int = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_CYCLEINT;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before cold start (integer)
		if (aux != port[portindex].ipdu[ipduindex].seq_int) {
			port[portindex].ipdu[ipduindex].seq_int = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_SEQINT;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before display (integer)
		if (aux != port[portindex].ipdu[ipduindex].cold_start) {
			port[portindex].ipdu[ipduindex].cold_start = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_COLDSTART;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before displaycycle (integer)
		if (aux != port[portindex].ipdu[ipduindex].display) {
			port[portindex].ipdu[ipduindex].display = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_DISPLAY;
		}
		e = p+1;
		aux = strtol(e, &p, 10); //p points to the blank before voltage (integer)
		if (aux != port[portindex].ipdu[ipduindex].display_cycle) {
			port[portindex].ipdu[ipduindex].display_cycle = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_DISPLAY_CYCLE;
		}
		e = p+1;
		aux = 10 * strtol(e, &p, 10); //p points to the char before pwr_factor (float)
		if (aux != port[portindex].ipdu[ipduindex].def_voltage) {
			port[portindex].ipdu[ipduindex].def_voltage = aux;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_CONFIGFILE;
		}
		e = p+1;
		auxf = 100 * strtof(e, &p); //p points to the char after pwr_factor (float)
		if (auxf != port[portindex].ipdu[ipduindex].pwr_factor) {
			port[portindex].ipdu[ipduindex].pwr_factor = auxf;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_CONFIGFILE;
		}

		e = p+1;
		aux = strtol(e, &p, 10); //p points to the char after num phases (int)
		old_numphases = port[portindex].ipdu[ipduindex].num_phases;
		if (aux != port[portindex].ipdu[ipduindex].num_phases) {
			if (port[portindex].ipdu[ipduindex].type == Vendor_servertech) {
				port[portindex].ipdu[ipduindex].num_phases = aux;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_CONFIGFILE;
				//port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_NAME;
			}
		}

		// PDU threshold
		elec = &port[portindex].ipdu[ipduindex].elecPduInfo.current;
		e = p+1;
		auxf = strtof(e, &p)*10; //p points to the char before threshold_LC (double)
		if ((auxf != 999999) && (auxf != elec->thre_hc)) {
			elec->thre_hc = auxf;
			elec->changed |= 1;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PDUTHRE;
		}
		e = p+1;
		auxf = 10 * strtof(e, &p); //p points to the char before threshold_HW (double)
		if ((auxf != 999999) && (auxf != elec->thre_lc)) {
			elec->thre_lc = auxf;
			elec->changed |= 2;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PDUTHRE;
		}
		e = p+1;
		auxf = 10 * strtof(e, &p); //p points to the char before threshold_LW (double)
		if ((auxf != 999999) && (auxf != elec->thre_hw)) {
			elec->thre_hw = auxf;
			elec->changed |= 4;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PDUTHRE;
		}
		e = p+1;
		auxf = 10 * strtof(e, &p); //p points to the char after threshold_LW (double)
		if ((auxf != 999999) && (auxf != elec->thre_lw)) {
			elec->thre_lw = auxf;
			elec->changed |= 8;
			port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PDUTHRE;
		}
		
		for (i=0; i<port[portindex].ipdu[ipduindex].num_banks; i++) {
			elec = &port[portindex].ipdu[ipduindex].firstBankInfo[i].current;
			elec->changed = 0;
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char before threshold_LC (double)
			if ((auxf != 999999) && (auxf != elec->thre_hc)) {
				elec->thre_hc = auxf;
				elec->changed |= 1;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_BANKTHRE;
			}
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char after threshold_LC (double)
			if ((auxf != 999999) && (auxf != elec->thre_lc)) {
				elec->thre_lc = auxf;
				elec->changed |= 2;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_BANKTHRE;
			}
			if (elec->thre_lc > elec->thre_hc) {
				elec->thre_lc = elec->thre_hc;
			}
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char before threshold_LW (double)
			if ((auxf != 999999) && (auxf != elec->thre_hw)) {
				elec->thre_hw = auxf;
				elec->changed |= 4;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_BANKTHRE;
			}
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char after threshold_LW (double)
			if ((auxf != 999999) && (auxf != elec->thre_lw)) {
				elec->thre_lw = auxf;
				elec->changed |= 8;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_BANKTHRE;
			}
		}

		for (i=0; i<old_numphases; i++) {
			elec = &port[portindex].ipdu[ipduindex].firstPhaseInfo[i].current;
			elec->changed = 0;
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char before threshold_LC (double)
			if ((auxf != 999999) && (auxf != elec->thre_hc)) {
				elec->thre_hc = auxf;
				elec->changed |= 1;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PHASETHRE;
			}
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char after threshold_LC (double)
			if ((auxf != 999999) && (auxf != elec->thre_lc)) {
				elec->thre_lc = auxf;
				elec->changed |= 2;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PHASETHRE;
			}
			if (elec->thre_lc > elec->thre_hc) {
				elec->thre_lc = elec->thre_hc;
			}
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char before threshold_LW (double)
			if ((auxf != 999999) && (auxf != elec->thre_hw)) {
				elec->thre_hw = auxf;
				elec->changed |= 4;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PHASETHRE;
			}
			e = p+1;
			auxf = 10 * strtof(e, &p); //p points to the char after threshold_LW (double)
			if ((auxf != 999999) && (auxf != elec->thre_lw)) {
				elec->thre_lw = auxf;
				elec->changed |= 8;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_PHASETHRE;
			}
		}

		// EnvMon Sensor
		env = port[portindex].ipdu[ipduindex].firstEnvMonSensor;
		for (i=0; i<port[portindex].ipdu[ipduindex].envmon_sensors; i++,env++) {
			e = p+1;
			auxf = strtof(e, &p) * 10; //p points to the char before threshold_LC (double)
			if ((auxf != 999999) && (auxf != env->thre_hc)) {
				env->thre_hc = auxf;
				env->changed |= 1;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_ENVMONTHRE;
			}
			e = p+1;
			auxf = strtof(e, &p) * 10; //p points to the char after threshold_LC (double)
			if ((auxf != 999999) && (auxf != env->thre_lc)) {
				env->thre_lc = auxf;
				env->changed |= 2;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_ENVMONTHRE;
			}
			e = p+1;
			auxf = strtof(e, &p) * 10; //p points to the char before threshold_LW (double)
			if ((auxf != 999999) && (auxf != env->thre_hw)) {
				env->thre_hw = auxf;
				env->changed |= 4;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_ENVMONTHRE;
			}
			e = p+1;
			auxf = strtof(e, &p) * 10; //p points to the char after threshold_LW (double)
			if ((auxf != 999999) && (auxf != env->thre_lw)) {
				env->thre_lw = auxf;
				env->changed |= 8;
				port[portindex].ipdu[ipduindex].changed |= IPDU_CHG_ENVMONTHRE;
			}
		}

		e = p + 1; //p points to the next general index, if any
	}

	for (i=0; i<gCache.ViewConfIpdu.numPorts; i++, port++) {
		ipdu = port->ipdu;
		for (j=0; j<port->numIpdu; j++, ipdu++) {
			if (ipdu->changed) {
				if (ipdu->changed & IPDU_CHG_NAME) {
					char *oldname = strdup(cycPMDngGetIpduName(ipdu));
					if (cycPMDngConfIpduName(PMDngUserName, ipdu)) {
						free(oldname);
						goto ipdu_conf_error;
					}
					if (strcmp(oldname, cycPMDngGetIpduName(ipdu))) {
						replaceName(oldname, cycPMDngGetIpduName(ipdu), 1);
						save_flag=1;
					}
					free(oldname);
				}

				if ((ipdu->changed & IPDU_CHG_OCP) ||
					(ipdu->changed & IPDU_CHG_SYSLOG) ||
					(ipdu->changed & IPDU_CHG_BUZZER)) {
					if (cycPMDngConfIpduOnOffparams(PMDngUserName, ipdu)) {
						goto ipdu_conf_error;
					} else save_flag=1;
				}
				if ((ipdu->changed & IPDU_CHG_CYCLEINT) ||
					(ipdu->changed & IPDU_CHG_SEQINT) ||
					(ipdu->changed & IPDU_CHG_COLDSTART) ||
					(ipdu->changed & IPDU_CHG_DISPLAY_CYCLE) ||
					(ipdu->changed & IPDU_CHG_DISPLAY)){
					if (cycPMDngConfIpduIntervals(PMDngUserName, ipdu)) {
						goto ipdu_conf_error;
					} else save_flag=1;
				}
				if (ipdu->changed & IPDU_CHG_PDUTHRE) {
					if (ipdu->type == Vendor_cyclades) {
						if (cycPMDngConfIpduPduThre(PMDngUserName, ipdu)) {
							goto ipdu_conf_error;
						} else save_flag=1;
					} else {
						ipdu->changed |= IPDU_CHG_CONFIGFILE;
					}
				}
				if (ipdu->changed & IPDU_CHG_PHASETHRE) {
					if (ipdu->type == Vendor_cyclades) {
						if (cycPMDngConfIpduPhaseThre(PMDngUserName, ipdu)) {
							goto ipdu_conf_error;
						} else save_flag=1;
					} else {
						ipdu->changed |= IPDU_CHG_CONFIGFILE;
					}
				}
				if (ipdu->changed & IPDU_CHG_BANKTHRE) {
					if (ipdu->type == Vendor_cyclades) {
						if (cycPMDngConfIpduBankThre(PMDngUserName, ipdu)) {
							goto ipdu_conf_error;
						} else save_flag=1;
					} else {
						ipdu->changed |= IPDU_CHG_CONFIGFILE;
					}
				}
				if (ipdu->changed & IPDU_CHG_ENVMONTHRE) {
					if (ipdu->type == Vendor_cyclades) {
						if (cycPMDngConfIpduEnvMonThre(PMDngUserName, ipdu)) {
							goto ipdu_conf_error;
						} else save_flag=1;
					} else {
						ipdu->changed |= IPDU_CHG_CONFIGFILE;
					}
				}

				if (ipdu->changed & IPDU_CHG_CONFIGFILE)
					confchg = 1;

				if(save_flag){
        			        cycPMDngSaveCmd2(PMDngUserName,ipdu);
			        }

				continue;
ipdu_conf_error:
				gCache.ViewConfIpdu.action = Msg_to_WEB;
				sprintf(gCache.ViewConfIpdu.sysMsg, "Couldn't configure %s Ipdu.", ipdu->name);
				break;
			}
		}
		if (gCache.ViewConfIpdu.action == Msg_to_WEB)
			break;	
	}
	if ((confchg) && (gCache.ViewConfIpdu.action != Msg_to_WEB)) {
		if (cycPMDngConfIpduParams(gCache.ViewConfIpdu.portInfo, gCache.ViewConfIpdu.numPorts)) {
			gCache.ViewConfIpdu.action = Msg_to_WEB;
			sprintf(gCache.ViewConfIpdu.sysMsg, "Couldn't configure Ipdus!");
		}
	}

	gCacheLoaded[kViewConfIPDUs]=0;
	return 0;
}

// Power Management - New Software Upgrade page
static int SetIpduSwUpgrade(void)
{
	int  aux, portindex, ipduindex;

	if (!gCacheLoaded[kIPDUSoftwareUpgrade]) return(0); //The page wasn't even loaded
	//printf("<----- SetIpduSwUpgrade()\n");

	if (wUser.uType == kAdminUser)
		PMDngUserName[0] = '\0';
	else
		strcpy(PMDngUserName, wUser.uName);
	if ((gCache.IpduSwUpgrade.action != no_Action) &&
		(gCache.IpduSwUpgrade.memSignature != checkPortsIpdusSignature())) {
		gCache.IpduSwUpgrade.action = Msg_to_WEB;
		strcpy(gCache.IpduSwUpgrade.sysMsg, 
		       "ACS detected changes on IPDU"
               		" configuration.\\nPlease try the last operation again...");
		return 0;
	}

	switch(gCache.IpduSwUpgrade.action) {
	case Refresh_LatestSw:
		//printf("###### Getting Latest Sw for Cyclades IPDUs ######\n");
		{
			CycParam version[] =
				{
					{ kPowerMgm, kFirmUpgradeVersion, gCache.IpduSwUpgrade.latest_sw, 0 },
				};

			if (GetCycParamValues(version, mArrayLength(version)) != kCycSuccess) {
				gCache.IpduSwUpgrade.action = Msg_to_WEB;
				strcpy(gCache.IpduSwUpgrade.sysMsg, "Couldn't check latest "
				"Cyclades IPDU Sw available!");
			}
		}
		break;
	case Upgrade_UnitFw:
		aux = atoi(gCache.IpduSwUpgrade.sysMsg); //get general index
		//prepare port and ipdu indexes
		portindex = aux / 1000;
		ipduindex = aux % 1000;
		//printf("###### Upgrade firmware: portindex<%d>, ipduindex<%d>\n", portindex, ipduindex);
		if (cycPMDngUpgradeFirmware(PMDngUserName,
				gCache.IpduSwUpgrade.portInfo[portindex].serialPort,
				gCache.IpduSwUpgrade.portInfo[portindex].ipdu[ipduindex].ipduNumber,
				gCache.IpduSwUpgrade.latest_sw,
				gCache.IpduSwUpgrade.portInfo[portindex].ipdu[ipduindex].version)) {
			gCache.IpduSwUpgrade.action = Msg_to_WEB;
			strcpy(gCache.IpduSwUpgrade.sysMsg, "Couldn't send Firmware Upgrade command!");
			//[RK]Jan/31/07 - fixed bug#18689
			sleep(5);//some time to PMD finish updating internal structures
			break;
		}
		sleep(5);//some time to PMD finish updating internal structures
		gCache.IpduSwUpgrade.action = Msg_to_WEB;
		strcpy(gCache.IpduSwUpgrade.sysMsg, "Firmware Upgrade Done!\\n"
			   "You may have to reload this page\\n"
			   "to see updated information...");
		break;
	case no_Action:
		break;
	default:
		//printf("###### Unknown Action <%d> ######\n", gCache.IpduSwUpgrade.action);
		gCache.IpduSwUpgrade.action = Msg_to_WEB;
		strcpy(gCache.IpduSwUpgrade.sysMsg, "Unknown Action !!!");
	}

	return 0;
}

// Power Management - New PMD Config, General tab
static int SetpmdGen(void)
{
	PMDGeneralInfo *info = gCache.pmdGen.genInfo;
	int i;
	int vendors=3; //Cyclades, SPC & ServerTech

	if (!gCacheLoaded[kPMDGeneral]) return(0); //The page wasn't even loaded

	//printf("<--- SetpmdGen()\n");
	//printf("##### <%s>\n", gCache.pmdGen.sysMsg);

	sscanf(gCache.pmdGen.sysMsg, "%s %s %s %s %s %s ",
		   info[0].user, info[0].passwd,
		   info[1].user, info[1].passwd,
		   info[2].user, info[2].passwd);

	for (i=0; i<vendors; i++) {
		if (!strcmp(info[i].user, "not-defined")) {
			memset(info[i].user, 0, kUserNameLength);   //so we can successfully
			memset(info[i].passwd, 0, kPasswordLength); //compare to gCacheSave
			info[i].action = kDelete;
			continue;
		}
		if (!strcmp(info[i].passwd, "not-defined")) memset(info[i].passwd, 0, kPasswordLength);
		info[i].action = kActionIgnore;
	}

	mRegisterParamChangeDT(pmdGen, genInfo, kPMDConfig, kPMDGeneralInfo, vendors);

	return 0;
}

// Power Management - New PMD Config, Outlet Groups tab
static int SetpmdOutGrp(void)
{
	PMDOutGrpsDef *groups = gCache.pmdOutGrp.groups;
	char *p, *e = gCache.pmdOutGrp.entries;
	int aux=0, index, sysMsgSet = 0;

	if (!gCacheLoaded[kPMDOutletGroups]) return(0); // The page wasn't even loaded
	//printf("<--- SetpmdOutGrp()\n");

	// Check deleted entries
	if (strcmp(e, "unchanged")) {
		while (strcmp(e, "end")) { //process entries if not end
			index = strtol(e, &p, 10); //get first index
			while (*p && isspace(*p)) p++;
			e = p; //point to next index or end
			while (index != aux) {
				groups[aux].action = kDelete;
				gCache.pmdOutGrp.changed = TRUE;
				aux++;
			}
			aux++;
		}
		while (aux < gCache.pmdOutGrp.numGroups) {
			groups[aux].action = kDelete;
			gCache.pmdOutGrp.changed = TRUE;
			aux++;
		}
	}

	if (gCache.pmdOutGrp.addEditEntry == kAdd) { // Check Add or Edit
		// ADD action
		if (strcmp(gCache.pmdOutGrp.singleGroup.group, "addaddadd")) {
			//user didn't press Cancel when adding group
			for (index=0; index < gCache.pmdOutGrp.numGroups; index++) {
				if (groups[index].action == kDelete) 
					break;
			}
			if (index != kPmMaxGroups) {
				for (aux=0; aux < gCache.pmdOutGrp.numGroups; aux++) {
					if (!strncmp(groups[aux].group, gCache.pmdOutGrp.singleGroup.group, kPMDGroupNameLength)) {
						sprintf(gCache.pmdOutGrp.sysMsg,"ERROR: Group <%s> is duplicated.", gCache.pmdOutGrp.singleGroup.group);
						sysMsgSet = 1;
						
					}
				}
				if (!sysMsgSet) {
					strcpy(groups[index].group, gCache.pmdOutGrp.singleGroup.group);
					strcpy(groups[index].outletList, gCache.pmdOutGrp.singleGroup.outletList);
					groups[index].action = kActionIgnore;
					gCache.pmdOutGrp.changed = TRUE;
					if (index == gCache.pmdOutGrp.numGroups)
						gCache.pmdOutGrp.numGroups++;
				}
			} else {
				strcpy(gCache.pmdOutGrp.sysMsg,"ERROR: Maximun number of groups was reached.");
				sysMsgSet = 1;
			}
		}
	} else if (gCache.pmdOutGrp.addEditEntry == kEdit) {
		// EDIT action
		if (strcmp(groups[gCache.pmdOutGrp.savedIndex].group, gCache.pmdOutGrp.singleGroup.group) ||
			strcmp(groups[gCache.pmdOutGrp.savedIndex].outletList, gCache.pmdOutGrp.singleGroup.outletList)) {
			strcpy(groups[gCache.pmdOutGrp.savedIndex].group, gCache.pmdOutGrp.singleGroup.group);
			strcpy(groups[gCache.pmdOutGrp.savedIndex].outletList, gCache.pmdOutGrp.singleGroup.outletList);
			gCache.pmdOutGrp.changed = TRUE;
		}
	}

	gCache.pmdOutGrp.addEditEntry = kNone;

	//register changes
	if (gCache.pmdOutGrp.changed) {
		if (gReq.action != kNone) {
			RegisterParamChange(0, groups, kPMDConfig, kPMDOutGrpsDef, gCache.pmdOutGrp.numGroups );
			gCache.pmdOutGrp.changed = FALSE;
		} else {
			gLedState++;
		}
	}

	if (!sysMsgSet)
		gCache.pmdOutGrp.sysMsg[0] = '\0';

	return 0;
}

// Power Management - New PMD Config, Users Management tab
static int SetpmdUserMan(void)
{
	PMDUsersMgm *users = gCache.pmdUserMan.usersMgm;
	char *p, *e = gCache.pmdUserMan.entries;
	int aux=0, index, sysMsgSet = 0;

	if (!gCacheLoaded[kPMDUsersManagement]) return(0); // The page wasn't even loaded
	//printf("<--- SetpmdUserMan()\n");

	// Check deleted entries
	if (strcmp(e, "unchanged")) {
		while (strcmp(e, "end")) { //process entries if not end
			index = strtol(e, &p, 10); //get first index
			while (*p && isspace(*p)) p++;
			e = p; //point to next index or end
			while (index != aux) {
				users[aux].action = kDelete;
				gCache.pmdUserMan.changed = TRUE;
				aux++;
			}
			aux++;
		}
		while (aux < gCache.pmdUserMan.numUsers) {
			users[aux].action = kDelete;
			gCache.pmdUserMan.changed = TRUE;
			aux++;
		}
	}

	if (gCache.pmdUserMan.addEditEntry == kAdd) { // Check Add or Edit
		// ADD action
		if (strcmp(gCache.pmdUserMan.singleUser.user, "addaddadd")) {
			//user didn't press Cancel when adding user
			for (index=0; index < gCache.pmdUserMan.numUsers; index++) {
				if (users[index].action == kDelete) 
					break;
			}
			if (index != kPmMaxUsers) {
				strcpy(users[index].user, gCache.pmdUserMan.singleUser.user);
				strcpy(users[index].outletList, gCache.pmdUserMan.singleUser.outletList);
				users[index].action = kActionIgnore;
				gCache.pmdUserMan.changed = TRUE;
				if (index == gCache.pmdUserMan.numUsers)
					gCache.pmdUserMan.numUsers++;
			} else {
				strcpy(gCache.pmdUserMan.sysMsg,"ERROR : Maximun number of users was reached");
				sysMsgSet = 1;
			}
		}
	} else if (gCache.pmdUserMan.addEditEntry == kEdit) {
		// EDIT action
		if (strcmp(users[gCache.pmdUserMan.savedIndex].user, gCache.pmdUserMan.singleUser.user) ||
			strcmp(users[gCache.pmdUserMan.savedIndex].outletList, gCache.pmdUserMan.singleUser.outletList)) {
			strcpy(users[gCache.pmdUserMan.savedIndex].user, gCache.pmdUserMan.singleUser.user);
			strcpy(users[gCache.pmdUserMan.savedIndex].outletList, gCache.pmdUserMan.singleUser.outletList);
			gCache.pmdUserMan.changed = TRUE;
		}
	}

	gCache.pmdUserMan.addEditEntry = kNone;

	//register changes
	if (gCache.pmdUserMan.changed) {
		if (gReq.action != kNone) {
			RegisterParamChange(0, users, kPMDConfig, kPMDUsersMgm, gCache.pmdUserMan.numUsers );
			gCache.pmdUserMan.changed = FALSE;
		} else {
			gLedState++;
		}
	}

	if (!sysMsgSet)
		gCache.pmdUserMan.sysMsg[0] = '\0';

	return 0;
}

#endif

#ifdef IPMI
// Power Management - IPMI
static int SetPmIpmi(void)
{
	IPMIDeviceInfo *ipmi = &gCache.pmipmi.ipmidev;
	IPMIDeviceInfo *ipmiList = gCache.pmipmi.ipmiInfo;
	IPMIDeviceInfo *newList, *newIpmi;						
	int    i, entryAdded = 0, nextDeviceId, ret;

	if (!gCacheLoaded[kPmIpmiPage]) return(0); // The page wasn't even loaded
        
	if (gCache.pmipmi.action != no_Action)
	{
		if ((gCache.pmipmi.action != Edit) && (gCache.pmipmi.action != Add)) {
			ipmi->deviceId = atoi(gCache.pmipmi.sysMsg);
			for (i = 0; i < gCache.pmipmi.numIpmiDev; i++, ipmiList++) {
				if (ipmiList->deviceId == ipmi->deviceId) {
					ipmi->action = ipmiList->action;
					break;
				}
			}
			ipmiList = gCache.pmipmi.ipmiInfo;
		}
		switch(gCache.pmipmi.action) {
			case Turn_ON:
			case Turn_OFF:
			case Cycle:
				if (ipmi->action == kAdd) {
					gCache.pmipmi.action = Msg_to_WEB;
					strcpy(gCache.pmipmi.sysMsg, "This device was just created.\\n"
					                         "Please \"try\" or \"apply\" changes first,\\n"
                                       			      "then you will be able to operate it.");
					return 0;
				}
				break;
			default:
				break; /*warning: deprecated use of label at end of compound statement*/
		}
		switch(gCache.pmipmi.action) {
		case Turn_ON:
			ret=cycIPMIcmd(ipmi->deviceId, kIpmiOn, gCache.pmipmi.sysMsg);
			gCache.pmipmi.action = Msg_to_WEB;
			break;
		case Turn_OFF:
			ret=cycIPMIcmd(ipmi->deviceId, kIpmiOff, gCache.pmipmi.sysMsg);
			gCache.pmipmi.action = Msg_to_WEB;
			break;
		case Cycle:
			ret=cycIPMIcmd(ipmi->deviceId, kIpmiCycle, gCache.pmipmi.sysMsg);
			gCache.pmipmi.action = Msg_to_WEB;
			break;
		case Edit:
			sscanf(gCache.pmipmi.sysMsg, "%d %s %s %s %s %s %s",
				   &ipmi->deviceId,
				   ipmi->alias,
				   ipmi->serverIP,
				   ipmi->authType,
				   ipmi->userAccessLevel,
				   ipmi->username,
				   ipmi->password);
			ipmi->action = kEdit;
			if (!strcmp(ipmi->username, "blankblank")) ipmi->username[0] = '\0';
			if (!strcmp(ipmi->password, "blankblank")) ipmi->password[0] = '\0';

			for (i = 0; i < gCache.pmipmi.numIpmiDev; i++, ipmiList++) {
				if (ipmiList->deviceId == ipmi->deviceId) {
					*ipmiList = *ipmi;
					break;
				}
			}
			gLedState++;
			break;
		case Add:
			sscanf(gCache.pmipmi.sysMsg, "%s %s %s %s %s %s",
				   ipmi->alias,
				   ipmi->serverIP,
				   ipmi->authType,
				   ipmi->userAccessLevel,
				   ipmi->username,
				   ipmi->password);
			ipmi->action = kAdd;
			for (i = 0; i < gCache.pmipmi.numIpmiDev; i++, ipmiList++) {
				if (ipmiList->action == kDelete) {
					ipmi->deviceId = ipmiList->deviceId;
					*ipmiList = *ipmi;
					entryAdded = 1;
					break;
				}
			}
			newIpmi = ipmiList;
			nextDeviceId = gCache.pmipmi.numIpmiDev+1;
			while (!entryAdded) {
				ipmi->deviceId = nextDeviceId;
				ipmiList = gCache.pmipmi.ipmiInfo;
				for (i = 0; i < gCache.pmipmi.numIpmiDev; i++, ipmiList++) {
					if (ipmiList->deviceId == ipmi->deviceId) {
						ipmi->deviceId = 0;
						break;
					}
				}
				if (ipmi->deviceId) {
					i = sizeof(IPMIDeviceInfo) * (gCache.pmipmi.numIpmiDev+1);
					if (!(newList = balloc(B_L, i)))
						return kMemAllocationFailure; //mp
					memset((char *)newList,0,i);
					memcpy(newList, gCache.pmipmi.ipmiInfo, sizeof(IPMIDeviceInfo) * (gCache.pmipmi.numIpmiDev));
					bfreeSafe(B_L, gCache.pmipmi.ipmiInfo);
					gCache.pmipmi.ipmiInfo = newList;
					
					newIpmi = &newList[gCache.pmipmi.numIpmiDev];
					gCache.pmipmi.numIpmiDev++;
					*newIpmi = *ipmi;
					entryAdded = 1; 
				} else
					nextDeviceId--;
			}
			gCache.pmipmi.numActiveIpmiDev++;
			if (!strcmp(newIpmi->username, "blankblank")) newIpmi->username[0] = '\0';
			if (!strcmp(newIpmi->password, "blankblank")) newIpmi->password[0] = '\0';
			gLedState++;
			break;
		case Delete:
			for (i = 0; i < gCache.pmipmi.numIpmiDev; i++, ipmiList++) {
				if (ipmiList->deviceId == ipmi->deviceId) {
					ipmiList->action = kDelete;
					gCache.pmipmi.numActiveIpmiDev--;
					break;
				}
			}
			gLedState++;
			break;
		default:
			gCache.pmipmi.action = Msg_to_WEB;
			strcpy(gCache.pmipmi.sysMsg, "Unknown ACTION !!!");
		}
	}
	if (gReq.action != kNone) {
		RegisterParamChange(0, gCache.pmipmi.ipmiInfo, kIPMIDevice, kIPMIDeviceInfo, gCache.pmipmi.numIpmiDev);
	} 

	return 0;
}
#endif // IPMI

static int SetNotifications(void)
{
   char_t *table, pnew[30];
   int i, change=0;
   NotificationInfo * notif;
   
   if (!gCacheLoaded[kNotifications]) return(0); // The page was even loaded
   
   table = gCache.notifications.notifTable;
   notif = gCache.notifications.notif;
      
   for (i=0; i < gCache.notifications.numNotif ; i++,notif++) {
      if (!*table) {
         notif->action = kDelete;
         change = 1;
         continue;
      }
      sprintf(pnew,",%d,",notif->triggerId);
      if (!strstr(table, pnew)) {
         notif->action = kDelete;
         change = 1;
      }
      if (notif->action != kNone) change = 1;
   }
   
   /* Register changes:  all.alarm */
   if (gCache.notifications.alarm != gCacheSave.notifications.alarm) { 
      if (gReq.action != kNone) {
         RegisterParamChange(0, allPorts, kPortsSpec, kPortBundle, 0);
         RegisterParamChange(0, &gCache.notifications.alarm,kPslave, kAlarm, 0); 
      } else { 
         gLedState++; 
      }
   }
   if (change) {
      if (gReq.action != kNone) {
         RegisterParamChange(0, gCache.notifications.notif, kSyslogNg, kNotificationInfo, gCache.notifications.numNotif);
      } else {
         gLedState++;
      }
   }
   return (0);
}

// SET/COMMIT -- NETWORK->SNMP Daemon Settings and POP-UPs
static int SetSNMPdaemon(void)
{
	char *e, *p;
	int  i, j, size;
	int  listchanged, added, edited;
	CommunityInfo *nv12;
	Snmpv3Info *nv3;

	if (!gCacheLoaded[kSNMPdaemonSettings]) return(0); // The page wasn't even loaded!

	// Common Section
	mRegisterParamChangeS(SNMPdaemon, sysContact, kSnmpdSettings, kSysContact, 0);
	mRegisterParamChangeS(SNMPdaemon, sysLocation, kSnmpdSettings, kSysLocation, 0);

	// SNMP v1 and v2 Section
	e = gCache.SNMPdaemon.SNMPv12Entries;
	i=0; j=0; listchanged=FALSE; added=FALSE; edited=FALSE;
	if (strcmp(e, "unchanged")) { // check for changes on this table (delete, up, down)
		do { //counts number of entries
			strtol(e, &p, 10);
			if (p != e) {
				i++;
				e = p + 1;
			}
		} while (p != e);
		listchanged = TRUE;
		gCache.SNMPdaemon.numSNMPv12 = i;
		gCache.SNMPdaemon.SNMPv12Changed = TRUE;

	} else {
		i = gCache.SNMPdaemon.numSNMPv12;
	}

	if (!strcmp(gCache.SNMPdaemon.webMsg,"v12entryADDED")) {
		i++;
		added = TRUE;
	} else if (!strcmp(gCache.SNMPdaemon.webMsg,"v12entryEDITED")) {
		edited = TRUE;
	}

	if (i && (listchanged || added || edited)) {
		//allocate new memory for SNMP v1 and v2 entries
		size = sizeof(CommunityInfo) * i;
		if (!(nv12 = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(nv12,0,size);

		if (listchanged) {
			//copy to new area, correcting order, deleting or changing entries
			e = gCache.SNMPdaemon.SNMPv12Entries;
			do {
				i = strtol(e, &p, 10);
				if (p != e) {
					*(nv12+j) = *(gCache.SNMPdaemon.SNMPv12p+i);
					j++;
					e = p + 1;
				}
			} while (p != e);
			
		} else {
			//just copy to new area
			for (; j < (added ? (i-1) : i); j++)
				*(nv12+j) = *(gCache.SNMPdaemon.SNMPv12p+j);
		}

		if (added) {
			// add the new entry
			*(nv12+j) = gCache.SNMPdaemon.SNMPv12;
			gCache.SNMPdaemon.numSNMPv12++;
			gCache.SNMPdaemon.SNMPv12Changed = TRUE;
		} else if (edited) { // correct the old entry
			*(nv12 + gCache.SNMPdaemon.editIndex) = gCache.SNMPdaemon.SNMPv12;
			gCache.SNMPdaemon.SNMPv12Changed = TRUE;
		}

		// free old memory and point to the new one
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv12p);
		gCache.SNMPdaemon.SNMPv12p = nv12;
		
	} else if (!gCache.SNMPdaemon.numSNMPv12) {
		// free old memory and reset number of entries
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv12p);
		gCache.SNMPdaemon.SNMPv12p = NULL;
	}

	// SNMPv3 Section
	e = gCache.SNMPdaemon.SNMPv3Entries;
	i=0; j=0; listchanged=FALSE; added=FALSE; edited=FALSE;
	if (strcmp(e, "unchanged")) { // check for changes on this table (delete, up, down)
		do { //counts number of entries
			strtol(e, &p, 10);
			if (p != e) {
				i++;
				e = p + 1;
			}
		} while (p != e);
		listchanged = TRUE;
		gCache.SNMPdaemon.numSNMPv3 = i;
		gCache.SNMPdaemon.SNMPv3Changed = TRUE;

	} else {
		i = gCache.SNMPdaemon.numSNMPv3;
	}

	if (!strcmp(gCache.SNMPdaemon.webMsg,"v3entryADDED")) {
		i++;
		added = TRUE;
	} else if (!strcmp(gCache.SNMPdaemon.webMsg,"v3entryEDITED")) {
		edited = TRUE;
	}

	if (i && (listchanged || added || edited)) {
		//allocate new memory for SNMPv3 entries
		size = sizeof(CommunityInfo) * i;
		if (!(nv3 = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(nv3,0,size);

		if (listchanged) {
			//copy to new area, correcting order, deleting or changing entries
			e = gCache.SNMPdaemon.SNMPv3Entries;
			do {
				i = strtol(e, &p, 10);
				if (p != e) {
					*(nv3+j) = *(gCache.SNMPdaemon.SNMPv3p+i);
					j++;
					e = p + 1;
				}
			} while (p != e);
			
		} else {
			//just copy to new area
			for (; j < (added ? (i-1) : i); j++)
				*(nv3+j) = *(gCache.SNMPdaemon.SNMPv3p+j);
		}

		if (added) {
			// add the new entry
			*(nv3+j) = gCache.SNMPdaemon.SNMPv3;
			gCache.SNMPdaemon.numSNMPv3++;
			gCache.SNMPdaemon.SNMPv3Changed = TRUE;
		} else if (edited) { // correct the old entry
			*(nv3 + gCache.SNMPdaemon.editIndex) = gCache.SNMPdaemon.SNMPv3;
			gCache.SNMPdaemon.SNMPv3Changed = TRUE;
		}

		// free old memory and point to the new one
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv3p);
		gCache.SNMPdaemon.SNMPv3p = nv3;
		
	} else if (!gCache.SNMPdaemon.numSNMPv3) {
		// free old memory and reset number of entries
		bfreeSafe(B_L, gCache.SNMPdaemon.SNMPv3p);
		gCache.SNMPdaemon.SNMPv3p = NULL;
	}

	gCache.SNMPdaemon.webMsg[0] = '\0';

	// Register table changes
	if (gCache.SNMPdaemon.SNMPv12Changed) {
		if (gReq.action != kNone) {
			RegisterParamChange(0, gCache.SNMPdaemon.SNMPv12p, kSnmpdSettings, kCommunityInfo, gCache.SNMPdaemon.numSNMPv12);
			gCache.SNMPdaemon.SNMPv12Changed = FALSE;
		} else {
			gLedState++;
		}
	}
	if (gCache.SNMPdaemon.SNMPv3Changed) {
		if (gReq.action != kNone) {
			RegisterParamChange(0, gCache.SNMPdaemon.SNMPv3p, kSnmpdSettings, kSnmpv3Info, gCache.SNMPdaemon.numSNMPv3);
			gCache.SNMPdaemon.SNMPv3Changed = FALSE;
		} else {
			gLedState++;
		}
	}

	return (0);
}

// SET/COMMIT -- NETWORK->HOST TABLE
static int SetHostTable(void)
{
	if (!gCacheLoaded[kHostTablePage]) return(0); // The page was even loaded
	/* Register changes */
	if (gCache.hostTable.hostsChanged == 1) {
			RegisterParamChange(0, gCache.hostTable.hosts, kHostTable, kHosts, gCache.hostTable.numHosts);
	}
	return (0);
}

// SET/COMMIT -- NETWORK->STATIC ROUTES and POP-UP
static int SetStaticRoutes(void)
{
	char *e = gCache.staticRoutes.entriesList, *p;
	int  i = 0, j = 0, size;
	int  listchanged = FALSE, added = FALSE, edited = FALSE;
	StRoutesInfo *nr;

	if (!gCacheLoaded[kStaticRoutes]) return(0); // The page wasn't even loaded!

	if (strcmp(e, "unchanged")) { // check for changes on the table (delete, up, down)
		do { //counts number of entries
			strtol(e, &p, 10);
			if (p != e) {
				i++;
				e = p + 1;
			}
		} while (p != e);
		listchanged = TRUE;
		gCache.staticRoutes.numRoutes = i;
		gCache.staticRoutes.routesChanged = TRUE;

	} else {
		i = gCache.staticRoutes.numRoutes;
	}

	if (!strcmp(gCache.staticRoutes.route.sysMsg,"entryADDED")) {
		i++;
		added = TRUE;
	} else if (!strcmp(gCache.staticRoutes.route.sysMsg,"entryEDITED")) {
		edited = TRUE;
	}
	gCache.staticRoutes.route.sysMsg[0] = '\0';

	if (i && (listchanged || added || edited)) {
		//allocate new memory for routes
		size = sizeof(StRoutesInfo) * i;
		if (!(nr = balloc(B_L, size)))
			return kMemAllocationFailure; //mp
		memset(nr,0,size);

		if (listchanged) {
			//copy to new area, correcting order, deleting or changing entries
			e = gCache.staticRoutes.entriesList;
			do {
				i = strtol(e, &p, 10);
				if (p != e) {
					*(nr+j) = *(gCache.staticRoutes.routes+i);
					j++;
					e = p + 1;
				}
			} while (p != e);
			
		} else {
			//just copy to new area
			for (; j < (added ? (i-1) : i); j++)
				*(nr+j) = *(gCache.staticRoutes.routes+j);
		}

		if (added) {
			// add the new entry
			*(nr+j) = gCache.staticRoutes.route;
			gCache.staticRoutes.numRoutes++;
			gCache.staticRoutes.routesChanged = TRUE;
		} else if (edited) { // correct the old entry
			*(nr + gCache.staticRoutes.editIndex) = gCache.staticRoutes.route;
			gCache.staticRoutes.routesChanged = TRUE;
		}

		// free old memory and point to the new one
		bfreeSafe(B_L, gCache.staticRoutes.routes);
		gCache.staticRoutes.routes = nr;
		
	} else if (!gCache.staticRoutes.numRoutes) {
		// free old memory and reset number of routes
		bfreeSafe(B_L, gCache.staticRoutes.routes);
		gCache.staticRoutes.routes = NULL;
	}

	// Register changes
	if (gCache.staticRoutes.routesChanged) {
		if (gReq.action != kNone) {
			RegisterParamChange(0, gCache.staticRoutes.routes, kStRoutes, kStRoutesInfo, gCache.staticRoutes.numRoutes);
			gCache.staticRoutes.routesChanged = FALSE;
		} else {
			gLedState++;
		}
	}

   return (0);
}

/* [LMT] Security Enhancements */
static int SetSecProfile(void)
{
   if (!gCacheLoaded[kSecProfile]) return(0); // The page wasn't even loaded

    /* Register changes: */
#ifdef SECPROFILE
    if (gCache.secProfile.options.profile == kSecCustom) 
#endif
    {
        mRegisterParamChangeI(secProfile, telnet, kInetd, kTelnetEnable, 0);
        mRegisterParamChangeT(secProfile, ssh, kSshd, kSshdOpt, 1); //struct
        mRegisterParamChangeT(secProfile, webUI, kWebUI, kWebUIOpt, 1); //struct
        mRegisterParamChangeI(secProfile, snmp, kSnmpdSettings, kSnmpdEnable, 0);
        mRegisterParamChangeI(secProfile, rpc, kPortMap, kRpcEnable, 0);
#ifdef KRBenable
        mRegisterParamChangeI(secProfile, ftp, kInetd, kFtpEnable, 0);
#endif
#ifdef IPSEC
        mRegisterParamChangeI(secProfile, ipsec, kIpSec, kIpSecEnable, 0);
#endif
        mRegisterParamChangeI(secProfile, icmp, kSecProfiles, kIcmpEnable, 0);
    }
    mRegisterParamChangeT(secProfile, options, kSecProfiles, kSecOpt, 1); //struct
    
    //if changes in webs, we must register to use when Trying or Applying changes
#ifdef SECPROFILE
    if (gCache.secProfile.options.profile == kSecCustom) {
#endif
		if (memcmp(&gCache.secProfile.webUI, &localSecWebUI, sizeof(localSecWebUI))) {
			WebNeedsRestart = TRUE;
		}
#ifdef SECPROFILE
	} else if (gCache.secProfile.showWarning == kSecWebsRestart) {
		WebNeedsRestart = TRUE;
	}
#endif

	//reset control
	gCache.secProfile.showWarning = kNoSecWarning;

	if (gReq.action != kNone && WebNeedsRestart) {

		CycLogOut(gReq.sid);
		dmfSetUrlOk("/logoutWait.asp", "");

    } else {
        //save info for future use
        localSecWebUI = gCache.secProfile.webUI;

    }
    return (0);
}


static int SetBootConf(void)
{
   if (!gCacheLoaded[kBootInfo]) return(0); // The page was even loaded

   /* Register changes: */
   mRegisterParamChangeT2(bootInfo, kBootManagement, kBootConf, 1);
   return (0);
}

static int SetTimeDate(void)
{
	int tzCh = 0;

	if (!gCacheLoaded[kTimeDate]) return(0); // The page wasn't even loaded

	//Register changes...
	mRegisterParamChangeI(timeDate, ntp, kNtp, kNtpEnable, 0);
	mRegisterParamChangeS(timeDate, ntpServer, kNtp, kNtpServer, 0);

	//set timezone, if needed
	if ((tzCh = gstrcmp(gCache.timeDate.timezone, gCacheSave.timeDate.timezone)) ||
		memcmp(&gCache.timeDate.tz, &gCacheSave.timeDate.tz, sizeof(gCache.timeDate.tz))) {
		if (gReq.action != kNone) {
			if (tzCh || !gstrcmp(gCache.timeDate.timezone, "Custom")) {
				RegisterParamChange(0, &gCache.timeDate, kCycDateTime, kTzParams, 1);
			}
		}
		else
			gLedState++;
	}

	//set date & time
	if ((gCache.timeDate.ntp == kNo) && (gCache.timeDate.dt.year)) {
                                     // year==0 signals we shouldn't apply time change
		if (0 != memcmp(&gCache.timeDate.dt, &gCacheSave.timeDate.dt, sizeof(gCache.timeDate.dt))) {
			if (gReq.action != kNone)
				RegisterParamChange(0, &gCache.timeDate.dt, kCycDateTime, kMthDayYrHrMinSec, 1);
//attention: We don't set led state. If "kNone", next get() will update page info...
//			else
//				gLedState++;
		}
	}

	return (0);
}

// SET -- PORTS -> Virtual Ports Table

static int SetVirtualPortsInfo(void)
{
   VirtualPortsInfo *virtual;
   char *pentry;
   char remote[kIpLength+3];
   int i, change=0;
   
   if (!gCacheLoaded[kVirtualPorts]) return(0); // The page was even loaded
        
   pentry = gCache.virtualP.virtualInfoSet;
   
   virtual = gCache.virtualP.virtualInfo;
      
   for (i=0; i < gCache.virtualP.numVirtual ; i++,virtual++) {
      sprintf(remote,",%s,",virtual->remoteIp);
      if (!strstr(gCache.virtualP.virtualInfoSet,remote) &&
          (virtual->action != kAdd)) {
         virtual->action = kDelete;
         change = 1;
      } else {
         if (virtual->action != kNone) change = 1;
      }
   }

   /* Register changes:  clustering IP */
   if (strcmp(gCache.virtualP.clusteringIp,gCacheSave.virtualP.clusteringIp)) { 
      if (gReq.action != kNone) {
         RegisterParamChange(0, &gCache.virtualP.clusteringIp,kPslave, kClusteringIp, 0); 
      } else { 
         gLedState++; 
      }
   }
   /* Register changes */
   if (change) {
      if (gReq.action != kNone) {
         RegisterParamChange(0, gCache.virtualP.virtualInfo, kPslave, kVirtualPortsInfo, gCache.virtualP.numVirtual);
      } else {
         gLedState++;
      }
   }
   return (0);
}


static int SetIpTables(void) 
{
	if (!gCacheLoaded[kIpTablesPage]) return(0); // The page was even loaded
	
	/* Register changes */
	if ((gReq.action != kNone) && gCache.ipTable.chainChanged){
		RegisterParamChange(0, gCache.ipTable.chainTable, kIpTables, 
				kIpTablesFilterChainInfo, gCache.ipTable.confNumChains);
		RegisterParamChange(0, gCache.ipTable.chainTable, kIpTables, 
				kIpTablesFilterChainRuleInfo, gCache.ipTable.confNumChains);
#ifdef IPv6enable
		RegisterParamChange(0, gCache.ipTable.chainTable6, kIp6Tables, 
				kIp6TablesFilterChainInfo, gCache.ipTable.confNumChains6);
		RegisterParamChange(0, gCache.ipTable.chainTable6, kIp6Tables, 
				kIp6TablesFilterChainRuleInfo, gCache.ipTable.confNumChains6);
#endif
		gCache.ipTable.chainChanged = 0; //[RK]Feb/25/05 
	}
	return(0);
}

// SET -- Change Physical Port Configuration

static int SetPhysPortConf(void)
{
	int change=0, i;

#ifdef ONS
	for (i=0; i < (gCache.device.number2+1) ; i++) {
#else
	for (i=0; i < (gCache.device.number+1) ; i++) {
#endif
		if (gCache.physP.physList[i].action) {
			change = 1;
			break;
		}		
	}

	// register Syslog Server to syslog buffering
	if (memcmp(&gCache.sysBufServer,&gCacheSave.sysBufServer,
			sizeof(SyslogBufServerInfo))) {
		RegisterParamChange(0, &gCache.sysBufServer,
				kSyslogNg, kSyslogBufServer, 0); 
	}

#ifdef PMDNG
	if ((gCache.pmdConf.numgrps != gCacheSave.pmdConf.numgrps) ||
	    memcmp(gCache.pmdConf.grps,gCacheSave.pmdConf.grps,sizeof(PMDOutGrpsDef)*gCache.pmdConf.numgrps)) {
		RegisterParamChange(0, gCache.pmdConf.grps, 
			kPMDConfig, kPMDOutGrpsDef, gCache.pmdConf.numgrps);
	} 
	if ((gCache.pmdConf.numusers != gCacheSave.pmdConf.numusers) ||
	    memcmp(gCache.pmdConf.users,gCacheSave.pmdConf.users,sizeof(PMDUsersMgm)*gCache.pmdConf.numusers)) {
		RegisterParamChange(0, gCache.pmdConf.users, 
			kPMDConfig, kPMDUsersMgm,gCache.pmdConf.numusers);
	} 
#endif

	if (change) {
#ifdef ONS
		RegisterParamChange(0, gCache.physP.physList,kPslave, kPhysPortsConf, gCache.device.number2+1); 
#else
		RegisterParamChange(0, gCache.physP.physList,kPslave, kPhysPortsConf, gCache.device.number+1); 
#endif
	}
	return (0);
}

#ifdef PCMCIA
static int SetAccessMethod(void)
{
	PcmciaInfo *p = &gCache.unitInfo.pcmcia[0];

   if (!gCacheLoaded[kAccessMethod]) return(0); // The page wasn't even loaded

	p->pcmciaTypeFlag = 0;

	// See if networking parameters got changed
	if (memcmp(&gCache.accessMethod.pcmType.netWkConfig, 
		&gCacheSave.accessMethod.pcmType.netWkConfig, sizeof(PcmciaNetwkConfig)))
		p->pcmciaTypeFlag |= kPcmciaFlagsEthernet;

	// See if wireless parameters got changed
	if (memcmp(&gCache.accessMethod.pcmType.wireless, 
		&gCacheSave.accessMethod.pcmType.wireless, sizeof(PcmciaWirelessConfig))) 
	{
		p->pcmciaTypeFlag |= kPcmciaFlagsWireless;
		if (gCache.accessMethod.pcmType.wireless.encryption[0] == kNo)
			memset(gCache.accessMethod.pcmType.wireless.key[0], 0, kListLength);
		if (gCache.accessMethod.pcmType.wireless.encryption[1] == kNo)
			memset(gCache.accessMethod.pcmType.wireless.key[1], 0, kListLength);

		if ((memcmp(&gCache.accessMethod.pcmType.wireless.essid[0], 
			&gCacheSave.accessMethod.pcmType.wireless.essid[0], kNetworkNameLength)) ||
			 (memcmp(&gCache.accessMethod.pcmType.wireless.key[0],
         &gCacheSave.accessMethod.pcmType.wireless.key[0], kListLength)) ||
			(gCache.accessMethod.pcmType.wireless.channel[0] !=
         gCacheSave.accessMethod.pcmType.wireless.channel[0]) || 
			(gCache.accessMethod.pcmType.wireless.encryption[0] !=
         gCacheSave.accessMethod.pcmType.wireless.encryption[0]))
		{
				gCache.accessMethod.pcmType.wireless.both = 1; // 1st slot configured 
		}

		if ((memcmp(&gCache.accessMethod.pcmType.wireless.essid[1], 
			&gCacheSave.accessMethod.pcmType.wireless.essid[1], kNetworkNameLength)) ||
			 (memcmp(&gCache.accessMethod.pcmType.wireless.key[1],
         &gCacheSave.accessMethod.pcmType.wireless.key[1], kListLength)) ||
			(gCache.accessMethod.pcmType.wireless.channel[1] !=
         gCacheSave.accessMethod.pcmType.wireless.channel[1]) || 
			(gCache.accessMethod.pcmType.wireless.encryption[1] !=
         gCacheSave.accessMethod.pcmType.wireless.encryption[1]))
		{
			if (gCache.accessMethod.pcmType.wireless.both)
				gCache.accessMethod.pcmType.wireless.both = 3; //both slots configured
			else
				gCache.accessMethod.pcmType.wireless.both = 2; //only 2nd slot configured
		}
	}

	//[RK]Nov/10/06
	if (gCache.accessMethod.pcmType.modemGsm.version2 == kNo) {
	// See if ppp parameters got changed for slot 1
	if (gCache.accessMethod.pcmType.modemGsm.ppp[0] != gCacheSave.accessMethod.pcmType.modemGsm.ppp[0] 
		|| (memcmp(&gCache.accessMethod.pcmType.modemGsm.localIp[0], 
		&gCacheSave.accessMethod.pcmType.modemGsm.localIp[0], kIpLength)) || 
		(memcmp(&gCache.accessMethod.pcmType.modemGsm.remoteIp[0],
		&gCacheSave.accessMethod.pcmType.modemGsm.remoteIp[0], kIpLength)))
	{
		p->pcmciaTypeFlag |= kPcmciaFlagsModem;
		gCache.accessMethod.pcmType.modemGsm.both = 1; // 1st slot configured 
	}

	// See if ppp parameters got changed for slot 2
	if (gCache.accessMethod.pcmType.modemGsm.ppp[1] != gCacheSave.accessMethod.pcmType.modemGsm.ppp[1]
		|| (memcmp(&gCache.accessMethod.pcmType.modemGsm.localIp[1], 
		&gCacheSave.accessMethod.pcmType.modemGsm.localIp[1], kIpLength)) ||
		(memcmp(&gCache.accessMethod.pcmType.modemGsm.remoteIp[1],
      &gCacheSave.accessMethod.pcmType.modemGsm.remoteIp[1], kIpLength)))
	{
		p->pcmciaTypeFlag |= kPcmciaFlagsModem;
		if (gCache.accessMethod.pcmType.modemGsm.both)
			gCache.accessMethod.pcmType.modemGsm.both = 3; //both slots configured
		else
			gCache.accessMethod.pcmType.modemGsm.both = 2; //only 2nd slot configured
	}

	// See if modem callback parameter got changed 
	if ((gCache.accessMethod.pcmType.modemGsm.callBack != gCacheSave.accessMethod.pcmType.modemGsm.callBack)
		|| (memcmp(gCache.accessMethod.pcmType.modemGsm.callBackNum, 
			gCacheSave.accessMethod.pcmType.modemGsm.callBackNum, kPhoneNumLength)))
	{
		p->pcmciaTypeFlag |= kPcmciaFlagsModemCallback;
	}

#ifdef OTPenable
	// See if modem OTP parameter got changed //[RK]Jan/20/06
	if (gCache.accessMethod.pcmType.modemGsm.otp != gCacheSave.accessMethod.pcmType.modemGsm.otp)
	{
		p->pcmciaTypeFlag |= kPcmciaFlagsModemCallback;
	}
#endif

	// See if additional gsm parameter got changed 
	if (memcmp(&gCache.accessMethod.pcmType.modemGsm.pin[0], 
		&gCacheSave.accessMethod.pcmType.modemGsm.pin[0], kPinLength)) {
		p->pcmciaTypeFlag |= kPcmciaFlagsGsm;
		gCache.accessMethod.pcmType.modemGsm.pinBoth = 1; // 1st slot configured 
	}

	if (memcmp(&gCache.accessMethod.pcmType.modemGsm.pin[1], 
		&gCacheSave.accessMethod.pcmType.modemGsm.pin[1], kPinLength)) {
		p->pcmciaTypeFlag |= kPcmciaFlagsGsm;
		if (gCache.accessMethod.pcmType.modemGsm.pinBoth)
			gCache.accessMethod.pcmType.modemGsm.pinBoth = 3; //both slots configured
		else
			gCache.accessMethod.pcmType.modemGsm.pinBoth = 2; //only 2nd slot configured
	}

        // See if additional cdma parameter (speed, addinit) got changed
        if( gCache.accessMethod.pcmType.modemGsm.speed[0] !=
            gCacheSave.accessMethod.pcmType.modemGsm.speed[0] ) {
          p->pcmciaTypeFlag |= kPcmciaFlagsGsmSpeed;
          gCache.accessMethod.pcmType.modemGsm.speedBoth = 1; // 1st slot configured
        }

        if( gCache.accessMethod.pcmType.modemGsm.speed[1] !=
            gCacheSave.accessMethod.pcmType.modemGsm.speed[1] ) {
          p->pcmciaTypeFlag |= kPcmciaFlagsGsmSpeed;
          if( gCache.accessMethod.pcmType.modemGsm.speedBoth )
            gCache.accessMethod.pcmType.modemGsm.speedBoth = 3; //both slots configured
          else
            gCache.accessMethod.pcmType.modemGsm.speedBoth = 2; //only 2nd slot configured
        }

        if( memcmp(&gCache.accessMethod.pcmType.modemGsm.addinit[0],
            &gCacheSave.accessMethod.pcmType.modemGsm.addinit[0], kAddinitLength) ) {
          p->pcmciaTypeFlag |= kPcmciaFlagsGsmAddinit;
          gCache.accessMethod.pcmType.modemGsm.addinitBoth = 1; // 1st slot configured
        }

        if( memcmp(&gCache.accessMethod.pcmType.modemGsm.addinit[1],
            &gCacheSave.accessMethod.pcmType.modemGsm.addinit[1], kAddinitLength) ) {
          p->pcmciaTypeFlag |= kPcmciaFlagsGsmAddinit;
          if( gCache.accessMethod.pcmType.modemGsm.addinitBoth )
            gCache.accessMethod.pcmType.modemGsm.addinitBoth = 3; //both slots configured
          else
            gCache.accessMethod.pcmType.modemGsm.addinitBoth = 2; //only 2nd slot configured
        }
	} // end of version2
	
	// See if isdn parameters got changed 
	if (memcmp(&gCache.accessMethod.pcmType.isdn, 
		&gCacheSave.accessMethod.pcmType.isdn, sizeof(PcmciaIsdnConfig)))
		p->pcmciaTypeFlag |= kPcmciaFlagsIsdn;

	// See if CF parameters got changed
	if (memcmp(&gCache.accessMethod.pcmType.flash, 
		&gCacheSave.accessMethod.pcmType.flash, sizeof(PcmciaFlashConfig)))
		p->pcmciaTypeFlag |= kPcmciaFlagsFlash;

	// See if ppp parameters got changed for slot 1
	if (gCache.accessMethod.pcmType.flash.enable[0] 
		!= gCacheSave.accessMethod.pcmType.flash.enable[0])
	{
		gCache.accessMethod.pcmType.flash.both = 1; // 1st slot configured 
	}

	// See if ppp parameters got changed for slot 2
	if (gCache.accessMethod.pcmType.flash.enable[1] 
		!= gCacheSave.accessMethod.pcmType.flash.enable[1])
	{
		if (gCache.accessMethod.pcmType.flash.both)
			gCache.accessMethod.pcmType.flash.both = 3; //both slots configured
		else
			gCache.accessMethod.pcmType.flash.both = 2; //only 2nd slot configured
	}

	if (memcmp(&gCache.accessMethod.pcmType, 
		&gCacheSave.accessMethod.pcmType, sizeof(PcmciaType))) 
	{
		if (gReq.action != kNone) {
			memset(&(p->pcmciaType), 0, sizeof(PcmciaType));
			memcpy(&(p->pcmciaType), &gCache.accessMethod.pcmType, sizeof(PcmciaType));
			 RegisterParamChange(0, gCache.unitInfo.pcmcia, kPcmcia, kPcmciaInfo, 1);
		} else { 
			gCache.accessMethod.pcmConfigChanged = 1;
			gLedState++;
		}
	}

   return (0);
}
#endif

int checkCancelRequest(int request,int page)
{
	// Reload Page
	if (gReq.action == kReloadPage) {
		if (page != -1 && page != -2) {
			gCacheLoaded[page] = false;
		}
		if (page != -2) //mp: keep LED state for Physical Ports pages (Bug Fix 1863)
			if (page == -1 || gLedState == 1) { //mp: we should only clear LED if this is the only page changed
				SetLedChanges(request, 0);
				gLedState = 0;
			}
		ResetSidEtc(request);
		return(1);
	}
	
   // If TryChanges or ApplyChanges was chosen the commit must be done
   if (gReq.action != kCancel) {
		if (page >= 0 && !gCacheLoaded[page]) {
			return(2);
		}
		return(0);
	}

   // Clear record of old admin's un-applied changes:
   ClearChanges();
   ClearPages();
   //gLedState = 0; mp: it is cleared in ClearPages() now
   SetLedChanges(request, 0);
   // restore configuration
   CycCancel();
   // clear dmf controls
   ResetSidEtc(request);
   return(1);
}

static int isEven(UInt16 x) 
{
   if (x % 2 == 0) 
      return true;
   else    
      return false;
}

static char *TransOptGetStr(TranslateOptions *Op, int nOp, int value)
{
   TranslateOptions *p = Op;

   for( ; nOp--; p++) {
      if (p->optionEnum == value) {
         return(p->optionName);
      }
   }

   return(Op->optionName);
}
Result LoadUnitAuth(void)
{
    Result result = kCycSuccess;
    CycParam values[] =
    {
        { kPslave, kUnitAuth, &gCache.authU.unitauthtype, 0 },
        { kPslave, kPPPAuth, &gCache.authU.pppauthtype, 0 },
    };
    memset((char *)&gCache.authU.unitauthtype,0,sizeof(UInt32));
    memset((char *)&gCache.authU.pppauthtype,0,sizeof(UInt32));
    result = GetCycParamValues(values, mArrayLength(values));
    return result;
}
extern int GetAuthType(Param* param, int request)
{
	Result result;
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

    result = LoadFirst(LoadUnitAuth, ksecAuth, &gCache.authU.unitauthtype, &gCacheSave.authU.unitauthtype, 2*sizeof(UInt32));
	return mAppError(result);
}
Result LoadRadiusConf(void)
{
    Result result = kCycSuccess;
    CycParam values[] =
    {
        { kRadiusConf, kRadiusInfo, &gCache.authU.radius, 1 },
    };
    memset((char *)&gCache.authU.radius,0,sizeof(RadiusInfo));
    result = GetCycParamValues(values, mArrayLength(values));
    return result;
}
extern int GetRadiusConf(Param* param, int request)
{
	Result result;
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

    result = LoadFirst(LoadRadiusConf, ksecRadius, &gCache.authU.radius, &gCacheSave.authU.radius, sizeof(RadiusInfo));
	return mAppError(result);
}
Result LoadTacplusConf(void)
{
    Result result = kCycSuccess;
    CycParam values[] =
    {
        { kTacplusConf, kTacplusInfo, &gCache.authU.tacplus, 1 },
    };
    memset((char *)&gCache.authU.tacplus,0,sizeof(TacplusInfo));
    result = GetCycParamValues(values, mArrayLength(values));
    return result;
}
extern int GetTacplusConf(Param* param, int request)
{
	Result result;
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

    result = LoadFirst(LoadTacplusConf, ksecTacplus, &gCache.authU.tacplus, &gCacheSave.authU.tacplus, sizeof(TacplusInfo));
	return mAppError(result);
}
Result LoadLdapConf(void)
{
    Result result = kCycSuccess;
    CycParam values[] =
    {
        { kLdapConf, kLdapInfo, &gCache.authU.ldap, 1 },
    };
    memset((char *)&gCache.authU.ldap,0,sizeof(LdapInfo));
    result = GetCycParamValues(values, mArrayLength(values));
    return result;
}
extern int GetLdapConf(Param* param, int request)
{
	Result result;
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

    result = LoadFirst(LoadLdapConf, ksecLdap, &gCache.authU.ldap, &gCacheSave.authU.ldap, sizeof(LdapInfo));
	return mAppError(result);
}
#ifdef KRBenable
Result LoadKrbConf(void)
{
    Result result = kCycSuccess;
    CycParam values[] =
    {
        { kKrb5Conf, kKrbInfo, &gCache.authU.krb, 1 },
    };
    memset((char *)&gCache.authU.krb,0,sizeof(KrbInfo));
    result = GetCycParamValues(values, mArrayLength(values));
    return result;
}
extern int GetKrbConf(Param* param, int request)
{
	Result result;
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

    result = LoadFirst(LoadKrbConf, ksecKrb, &gCache.authU.krb, &gCacheSave.authU.krb, sizeof(KrbInfo));
	return mAppError(result);
}
#endif

#ifdef SMBenable
Result LoadSmbConf(void)
{
    Result result = kCycSuccess;
    CycParam values[] =
    {
        { kSmbConf, kSmbInfo, &gCache.authU.smb, 1 },
    };
    memset((char *)&gCache.authU.smb,0,sizeof(SmbInfo));
    result = GetCycParamValues(values, mArrayLength(values));
    return result;
}
extern int GetSmbConf(Param* param, int request)
{
    Result result;
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

    result = LoadFirst(LoadSmbConf, ksecSmb, &gCache.authU.smb, &gCacheSave.authU.smb, sizeof(SmbInfo));
    return mAppError(result);
}
#endif

#ifdef NISenable
Result LoadNisConf(void)
{
    Result result = kCycSuccess;
    CycParam values[] =
    {
        { kNis, kNisInfo, &gCache.authU.nis, 1 },
    };
    memset((char *)&gCache.authU.nis,0,sizeof(NisInfo));
    result = GetCycParamValues(values, mArrayLength(values));
    return result;
}
extern int GetNisConf(Param* param, int request)
{
    Result result;
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

    result = LoadFirst(LoadNisConf, ksecNis, &gCache.authU.nis, &gCacheSave.authU.nis, sizeof(NisInfo));
    return mAppError(result);
}
#endif
int SetAuthType(void)
{
	if (!gCacheLoaded[ksecAuth]) return(0); // The page was even loaded
	 mRegisterParamChangeT(authU,unitauthtype,kPslave,kUnitAuth,1);
	 mRegisterParamChangeT(authU,pppauthtype,kPslave,kPPPAuth,1);
	return (0);
}
int SetRadius(void)
{
	if (!gCacheLoaded[ksecRadius]) return(0); // The page was even loaded
	 mRegisterParamChangeT(authU,radius,kRadiusConf,kRadiusInfo,1);
	return (0);
}
int SetTacplus(void)
{
	if (!gCacheLoaded[ksecTacplus]) return(0); // The page was even loaded
	 mRegisterParamChangeT(authU,tacplus,kTacplusConf,kTacplusInfo,1);
	return (0);
}
int SetLdap(void)
{
	if (!gCacheLoaded[ksecLdap]) return(0); // The page was even loaded
	 mRegisterParamChangeT(authU,ldap,kLdapConf,kLdapInfo,1);
	return (0);
}
#ifdef KRBenable
int SetKrb(void)
{
	if (!gCacheLoaded[ksecKrb]) return(0); // The page was even loaded
	 mRegisterParamChangeT(authU,krb,kKrb5Conf,kKrbInfo,1);
	return (0);
}
#endif

#ifdef SMBenable
int SetSmb(void)
{
	if (!gCacheLoaded[ksecSmb]) return(0); // The page was even loaded
	 mRegisterParamChangeT(authU,smb,kSmbConf,kSmbInfo,1);
	return (0);
}
#endif

#ifdef NISenable
int SetNis(void)
{
	if (!gCacheLoaded[ksecNis]) return(0); // The page was even loaded
	 mRegisterParamChangeT(authU,nis,kNis,kNisInfo,1);
	return (0);
}
#endif

extern int AuthTypeCommit(Param* param, int request, char_t* urlQuery)
{
    Result result = kCycSuccess;
    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,ksecAuth)) return mAppError(result);

	if (!gCacheLoaded[ksecAuth]) return(0); // The page was even loaded

//    if ((gReq.action == kCancel) || (gReq.action == kReloadPage)){
//        gCache.authU.authtype = gCacheSave.authU.authtype;
//        gCacheLoaded[ksecAuth] = false;
//        gCache.authAction.type = 0;
//        if (checkCancelRequest(request,ksecAuth))
//            return mAppError(result);
//    }
//    if (gCache.authAction.type == 3) { //DONE
        if (gCache.authU.unitauthtype != gCacheSave.authU.unitauthtype
		|| gCache.authU.pppauthtype != gCacheSave.authU.pppauthtype
	) {
            gLedState++;
        }
//        gCache.authAction.type = 0;
//    }

	SetLedChanges(request, gLedState);
    if (gReq.action != kNone) {
    	return expertCommit(param, request, urlQuery);
    }
    ResetSidEtc(request);
    return mAppError(result);
}

extern int RadiusConfCommit(Param* param, int request, char_t* urlQuery)
{
    Result result = kCycSuccess;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,ksecRadius)) return mAppError(result);

	if (!gCacheLoaded[ksecRadius]) return(0); // The page was even loaded

//    if ((gReq.action == kCancel) || (gReq.action == kReloadPage)){
//        memcpy(&gCache.authU.radius, &gCacheSave.authU.radius, sizeof(RadiusInfo));
//        gCacheLoaded[ksecRadius] = false;
//        gCache.authAction.radius = 0;
//        if (checkCancelRequest(request,ksecRadius))
//            return mAppError(result);
//    }
//    if (gCache.authAction.radius == 3) { //DONE
        if (0 != memcmp(&gCache.authU.radius, &gCacheSave.authU.radius,
                        sizeof(RadiusInfo))) {
            gLedState++;
        }
//        gCache.authAction.radius = 0;
//    }

    SetLedChanges(request, gLedState);

    if (gReq.action != kNone) {
        return expertCommit(param, request, urlQuery);
    }
    ResetSidEtc(request);
    return mAppError(result);
}

extern int TacplusConfCommit(Param* param, int request, char_t* urlQuery)
{
    Result result = kCycSuccess;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,ksecTacplus)) return mAppError(result);

	if (!gCacheLoaded[ksecTacplus]) return(0); // The page was even loaded

//    if ((gReq.action == kCancel) || (gReq.action == kReloadPage)){
//        memcpy(&gCache.authU.tacplus, &gCacheSave.authU.tacplus, sizeof(TacplusInfo));
//        gCacheLoaded[ksecTacplus] = false;
//        gCache.authAction.tacplus = 0;
//        if (checkCancelRequest(request,ksecTacplus))
//            return mAppError(result);
//    }
//    if (gCache.authAction.tacplus == 3) { //DONE
        if (0 != memcmp(&gCache.authU.tacplus, &gCacheSave.authU.tacplus,
                        sizeof(TacplusInfo))) {
            gLedState++;
        }
//        gCache.authAction.tacplus = 0;
//    }

    SetLedChanges(request, gLedState);

    if (gReq.action != kNone) {
        return expertCommit(param, request, urlQuery);
    }
    ResetSidEtc(request);
    return mAppError(result);
}

extern int LdapConfCommit(Param* param, int request, char_t* urlQuery)
{
    Result result = kCycSuccess;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,ksecLdap)) return mAppError(result);

	if (!gCacheLoaded[ksecLdap]) return(0); // The page was even loaded

//    if ((gReq.action == kCancel) || (gReq.action == kReloadPage)){
//        memcpy(&gCache.authU.ldap, &gCacheSave.authU.ldap, sizeof(LdapInfo));
//        gCacheLoaded[ksecLdap] = false;
//        gCache.authAction.ldap = 0;
//        if (checkCancelRequest(request,ksecLdap))
//            return mAppError(result);
//    }
//    if (gCache.authAction.ldap == 3) { //DONE
        if (0 != memcmp(&gCache.authU.ldap, &gCacheSave.authU.ldap,
                        sizeof(LdapInfo))) {
            gLedState++;
        }
//        gCache.authAction.ldap = 0;
//    }

    SetLedChanges(request, gLedState);

    if (gReq.action != kNone) {
        return expertCommit(param, request, urlQuery);
    }
    ResetSidEtc(request);
    return mAppError(result);

}

#ifdef KRBenable
extern int KrbConfCommit(Param* param, int request, char_t* urlQuery)
{
    Result result = kCycSuccess;


    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,ksecKrb)) return mAppError(result);

	if (!gCacheLoaded[ksecKrb]) return(0); // The page was even loaded

//    if ((gReq.action == kCancel) || (gReq.action == kReloadPage)){
//        memcpy(&gCache.authU.krb, &gCacheSave.authU.krb, sizeof(KrbInfo));
//        gCacheLoaded[ksecKrb] = false;
//        gCache.authAction.krb = 0;
//        if (checkCancelRequest(request,ksecKrb))
//            return mAppError(result);
//    }
//    if (gCache.authAction.krb == 3) { //DONE
        if (0 != memcmp(&gCache.authU.krb, &gCacheSave.authU.krb,
                        sizeof(KrbInfo))) {
            gLedState++;
        }
//        gCache.authAction.krb = 0;
//    }

    SetLedChanges(request, gLedState);

    if (gReq.action != kNone) {
        return expertCommit(param, request, urlQuery);
    }
    ResetSidEtc(request);
    return mAppError(result);

}
#endif

#ifdef SMBenable
extern int SmbConfCommit(Param* param, int request, char_t* urlQuery)
{
    Result result = kCycSuccess;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,ksecSmb)) return mAppError(result);

	if (!gCacheLoaded[ksecSmb]) return(0); // The page was even loaded

//    if ((gReq.action == kCancel) || (gReq.action == kReloadPage)){
//        memcpy(&gCache.authU.smb, &gCacheSave.authU.smb, sizeof(SmbInfo));
//        gCacheLoaded[ksecSmb] = false;
//        gCache.authAction.smb = 0;
//        if (checkCancelRequest(request,ksecSmb))
//            return mAppError(result);
//    }
//    if (gCache.authAction.smb == 3) { //DONE
        if (0 != memcmp(&gCache.authU.smb, &gCacheSave.authU.smb,
                        sizeof(SmbInfo))) {
            gLedState++;
        }
//        gCache.authAction.smb = 0;
//    }

    SetLedChanges(request, gLedState);

    if (gReq.action != kNone) {
        return expertCommit(param, request, urlQuery);
    }
    ResetSidEtc(request);
    return mAppError(result);
}
#endif

#ifdef NISenable
extern int NisConfCommit(Param* param, int request, char_t* urlQuery)
{
    Result result = kCycSuccess;

    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
        return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,ksecNis)) return mAppError(result);

	if (!gCacheLoaded[ksecNis]) return(0); // The page was even loaded

//    if ((gReq.action == kCancel) || (gReq.action == kReloadPage)){
//        memcpy(&gCache.authU.nis, &gCacheSave.authU.nis, sizeof(NisInfo));
//        gCacheLoaded[ksecNis] = false;
//        gCache.authAction.nis = 0;
//        if (checkCancelRequest(request,ksecNis))
//            return mAppError(result);
//    }
//    if (gCache.authAction.nis == 3) { //DONE
        if (0 != memcmp(&gCache.authU.nis, &gCacheSave.authU.nis,
                        sizeof(NisInfo))) {
            gLedState++;
        }
//        gCache.authAction.nis = 0;
//    }

    SetLedChanges(request, gLedState);

    if (gReq.action != kNone) {
        return expertCommit(param, request, urlQuery);
    }
    ResetSidEtc(request);
    return mAppError(result);
}
#endif

//***** Online Help Configuration *******//
static 
Result LoadOnlineHelp(void)
{
	CycParam values[] =
	{
		{ kOnlineHelpConf, kOnlineHelp, &(gCache.onlineHelp), 0 },
	};
	return GetCycParamValues(values, mArrayLength(values));
}

static int SetOnlineHelp(void)
{
	if (!gCacheLoaded[kOnlineHelpPage]) return(0); // The page was even loaded

	/* Register changes: */
	mRegisterParamChangeT2(onlineHelp, kOnlineHelpConf, kOnlineHelp, 1);
	return (0);
}

int GetOnlineHelpConf(Param* param, int request)
{
	Result result;

	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadOnlineHelp, kOnlineHelpPage, &gCache.onlineHelp, 
						&gCacheSave.onlineHelp, sizeof(gCache.onlineHelp));

	return mAppError(result);
}

int OnlineHelpCommit(Param* param, int request, char_t* urlQuery)
{
	Result result;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kOnlineHelpPage)) return mAppError(result);

	// If it was just a menu navigation does not commit anything
	if (gReq.action == kNone) {
		SetOnlineHelp();
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}
	return expertCommit(param, request, urlQuery);
}

int GetOnlineHelp(int eid, webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	int len;
	char path[kOnlineHelpPathLength];

	fp = fopen(ONLINE_HELP_FILE, "r");
	if (fp == NULL) {
		websWrite(wp, "Online Help Undefined");
		return(kCycSuccess);
	}
	memset(path, 0, kOnlineHelpPathLength);
	fread(path, 1, kOnlineHelpPathLength-1, fp);
	len = strlen(path);
	if (len && path[len-1] == '\n') path[len-1] = 0;
	fclose(fp);

	websWrite(wp, path);
	return(kCycSuccess);
}
