/* Framework code: (c) 2001 Art & Logic, Inc. All Rights Reserved. */
/* Project Specific code: (c) 2003 Cyclades */

#ifndef h_UserFunc
#define h_UserFunc

#include "Dmf.h"
#include "webApi.h" // ACS/TS-specific data
#if defined(KVM) || defined(ONS)
#include "KVMFunc.h"
#endif

/*
 * UserFunc.h -- This file should be the home for any user-supplied
 * functions required for your project. First we declare some functions 
 * for initialization and shutdown for you to hook in any behavior that 
 * the stock framework does not provide.
 * The remainder of the file is for project-specific functions
 */

/*
 * FRAMEWORK FUNCTIONS
 */
extern const char* kUserFuncVersion;

   /*
    * dmfUserPreInit will be called from 'dmfInit()' before it's done 
    * initiailzation.
    */
extern void dmfUserPreInit(void);

   /*
    * dmfUserPostInit will be called from 'dmfInit()' just before it 
    * returns. This is a good place to set/initialize any 'local variables' 
    * that will be accessed using the 'dmfSetLocal'/'dmfGetLocal'
    * functions.
    */
extern void dmfUserPostInit(void);

   /*
    * dmfUserPreClose will be called from 'dmfClose()' before it does 
    * any of its cleanup. All of the dmframe framework code is still 
    * available at this point.
    */
extern void dmfUserPreClose(void);

   /*
    * dmfUserPostClose will be called from 'dmfClose()' just before it 
    * returns. None of the dmframe framework code can be called -- this
    * is your last opportunity to do any cleanup that may be needed for 
    * your system.
    */
extern void dmfUserPostClose(void);


/*
 * AcsWeb-SPECIFIC FUNCTIONS/VARIABLES FOLLOW
 */


/*--------------------------------------------------------------------------
Request - this is for exchanging request-level (ultra-transient) information
See the comment on "req" in the param tree (Param.c)!
--------------------------------------------------------------------------*/
typedef struct
{
   CycSetAction action;
   UInt32 sid;
   char_t bogus[31]; // for a bogus field in the form, if necessary
   char_t logout[31]; // for a logout field in the form, if necessary
} ReqData; /* note, this is only to be used transiently! */
extern ReqData gReq;

/*--------------------------------------------------------------------------
Login (ultra-transient) data
--------------------------------------------------------------------------*/
typedef struct
{
   char_t userName[kUserNameLength];
   char_t password[kPasswordLength];
   char_t userLogin[kUserNameLength];
#if defined(KVM) || defined(ONS)
   char_t KVMPort[MAX_PORT_FIELD_LEN];
   UInt32 directacc;
#endif
   UInt8 confirm;
} LoginData; /* note, this is only to be used transiently! */
extern LoginData gLogin;

#if defined(KVM) || defined(ONS)
struct _stinfo {
	unsigned char portname[32];
	unsigned char physname[32];
	unsigned char username[64];
	unsigned char status;
	unsigned char conn[32];
	unsigned char perm[5];
	unsigned char orig_perm[5];
	unsigned char bright;
	unsigned char contr;
	unsigned char outlet[256];
	unsigned char cycle;
	unsigned char message[256];
	KVMportlist list;
};
#endif

/*--------------------------------------------------------------------------
Cache store of all system data
--------------------------------------------------------------------------*/
#include "SystemData.h"
extern SystemData gCache;
extern void includeNbsp(unsigned char *dest,unsigned char *buf, int maxsize);

/*------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------
"Commit" handlers
--------------------------------------------------------------------------*/
extern int Login(Param* param, int request, char_t* urlQuery);
extern int LoginConfirm(Param* param, int request, char_t* urlQuery);
extern int fakeCommit(Param* param, int request, char_t* urlQuery);
extern int Logout(Param* param, int request, char_t* urlQuery);
extern int wizardCommit(Param* param, int request, char_t* urlQuery);
extern int expertCommit(Param* param, int request, char_t* urlQuery);
extern int netSettingsCommit(Param* param, int request, char_t* urlQuery);
extern int autoDiscoveryCommit(Param* param, int request, char_t* urlQuery);
extern int accessCommit(Param* param, int request, char_t* urlQuery);
extern int SystemLogCommit(Param* param, int request, char_t* urlQuery);
extern int MultiOutletDevCommit(Param* param, int request, char_t* urlQuery);
extern int IpduOutManCommit(Param* param, int request, char_t* urlQuery);
extern int IpduOutGrpCtrlCommit(Param* param, int request, char_t* urlQuery);
extern int ViewIpduInfoCommit(Param* param, int request, char_t* urlQuery);
extern int IpduConfigCommit(Param* param, int request, char_t* urlQuery);
extern int IpduSwUpgradeCommit(Param* param, int request, char_t* urlQuery);
extern int pmdGenCommit(Param* param, int request, char_t* urlQuery);
extern int pmdOutGrpCommit(Param* param, int request, char_t* urlQuery);
extern int pmdUserManCommit(Param* param, int request, char_t* urlQuery);
extern int OutletManCommit(Param* param, int request, char_t* urlQuery);
extern int IPDUinfoCommit(Param* param, int request, char_t* urlQuery);
extern int PMUserManCommit(Param* param, int request, char_t* urlQuery);
extern int PMConfigCommit(Param* param, int request, char_t* urlQuery);
extern int PMSwUpgradeCommit(Param* param, int request, char_t* urlQuery);
extern int PmIpmiCommit(Param* param, int request, char_t* urlQuery);
extern int NotificationsCommit(Param* param, int request, char_t* urlQuery);
extern int SetEventsNotif(Param* param, int request, char_t* urlQuery);
extern int SNMPdaemonCommit(Param* param, int request, char_t* urlQuery);
extern int netServicesCommit(Param* param, int request, char_t* urlQuery);
extern int HostTableCommit(Param* param, int request, char_t* urlQuery);
extern int VPNTableCommit(Param* param, int request, char_t* urlQuery);
extern int VPNEntryCommit (Param* param, int request, char_t* urlQuery);
extern int GetVPNEntry(Param* param, int request);
extern int StaticRoutesCommit(Param* param, int request, char_t* urlQuery);
extern int BootConfCommit(Param* param, int request, char_t* urlQuery);
extern int UpgradeCommit(Param* param, int request, char_t* urlQuery);
extern int TimeDateCommit(Param* param, int request, char_t* urlQuery);
extern int RebootCommit(Param* param, int request, char_t* urlQuery);
extern int VirtualPortsInfoCommit(Param* param, int request, char_t* urlQuery);
extern int SetVirtualPort(Param* param, int request, char_t* urlQuery);
extern int PhysActionCommit(Param* param, int request, char_t* urlQuery);
extern int PhysPortCommit(Param* param, int request, char_t* urlQuery);
extern int ActiveSessionCommit(Param* param, int request, char_t* urlQuery);
extern int backupConfigCommit(Param* param, int request, char_t* urlQuery);
#ifdef PCMCIA
extern int AccessMethodCommit(Param* param, int request, char_t* urlQuery);
#endif
extern int MenuShellCommit(Param* param, int request, char_t* urlQuery);
extern int tablePageCommit(Param* param, int request, char_t* urlQuery);
extern int SecProfileCommit(Param* param, int request, char_t* urlQuery);

extern int IpTableChainsCommit(Param* param, int request, char_t* urlQuery);
extern int IpTableRulesCommit(Param* param, int request, char_t* urlQuery);
extern int IpTableRuleSpecCommit(Param* param, int request, char_t* urlQuery);

//[RK]Jun/14/04 - Access Privilege
extern int RegUserSecurityCommit(Param* param, int request, char_t* urlQuery);

#if defined(KVM) || defined(ONS)
extern int KVMDConfCommit(Param* param, int request, char_t* urlQuery);
extern int KVMUserCommit(Param* param, int request, char_t* urlQuery);
extern int KVMAccListCommit(Param* param, int request, char_t* urlQuery);
extern int KVMDevicePermissionsCommit(Param* param, int request, char_t* urlQuery);
extern int KVMDeviceCommit(Param* param, int request, char_t* urlQuery);
extern int KVMDeviceTermCommit(Param* param, int request, char_t* urlQuery);
extern int KVMuCupdateCommit(Param* param, int request, char_t* urlQuery);
extern int KVMuCresetCommit(Param* param, int request, char_t* urlQuery);
extern int KVMauxPortCommit(Param* param, int request, char_t* urlQuery);
extern int KVMPMGetNumPorts(Param* param, int request);
#endif
extern int AuthTypeCommit(Param* param, int request, char_t* urlQuery);
extern int RadiusConfCommit(Param* param, int request, char_t* urlQuery);
extern int TacplusConfCommit(Param* param, int request, char_t* urlQuery);
extern int LdapConfCommit(Param* param, int request, char_t* urlQuery);
extern int KrbConfCommit(Param* param, int request, char_t* urlQuery);
extern int SmbConfCommit(Param* param, int request, char_t* urlQuery);
extern int NisConfCommit(Param* param, int request, char_t* urlQuery);
//extern int AuthConfCommit(Param* param, int request, char_t* urlQuery);
/*--------------------------------------------------------------------------
"Query" handlers
--------------------------------------------------------------------------*/
extern int GetDevice(Param* param, int request);
extern int GetNetSettings(Param* param, int request);
extern int GetAutoDiscovery(Param* param, int request);
extern int GetCasProSettings(Param* param, int request);
extern int GetAccessUser(Param* param, int request);
extern int GetAccessGroup(Param* param, int request);
extern int GetUserSelected(Param* param, int request);
extern int GetGroupSelected(Param* param, int request);
extern int GetDataBuffering(Param* param, int request);
extern int GetSystemLog(Param* param, int request);
extern int GetConnectPortsHtml(Param* param, int request);
extern int GetMultiOutletDev(Param* param, int request);
extern int GetIpduOutMan(Param* param, int request);
extern int GetIpduOutGrpCtrl(Param* param, int request);
extern int GetViewIpduInfo(Param* param, int request);
extern int GetIpduConfig(Param* param, int request);
extern int GetIpduSwUpgrade(Param* param, int request);
extern int GetpmdGen(Param* param, int request);
extern int GetpmdOutGrp(Param* param, int request);
extern int GetpmdOutGrpEntry(Param* param, int request);
extern int GetpmdUserMan(Param* param, int request);
extern int GetpmdUserManEntry(Param* param, int request);
extern int GetOutletMan(Param* param, int request);
extern int GetIPDUinfo(Param* param, int request);
extern int GetPMUserMan(Param* param, int request);
extern int GetPMUserManEntry(Param* param, int request);
extern int GetPMConfig(Param* param, int request);
extern int GetPMSwUpgrade(Param* param, int request);
extern int GetPmIpmi(Param* param, int request);
extern int GetMenuShellInfo(Param* param, int request);
extern int GetMenuShAddModOption(Param* param, int request);
extern int GetUnitInfo(Param* param, int request);
extern int GetUnitKVMPortStatus(Param* param, int request);
extern int GetUnitFanStatus(Param* param, int request);
extern int GetUnitVersionInfo(Param* param, int request);
extern int GetUnitPwrSupplyInfo(Param* param, int request);
extern int GetUnitCpuInfo(Param* param, int request);
extern int GetUnitMemInfo(Param* param, int request);
#ifdef PCMCIA
extern int GetUnitPcmciaInfo(Param* param, int request);
#endif
extern int GetUnitRamDiskInfo(Param* param, int request);
extern int GetUnitBootConfInfo(Param* param, int request);
extern int GetTimeDate(Param* param, int request);
extern int GetNotifications(Param* param, int request);
extern int GetEventsNotif(Param* param, int request);
extern int GetSNMPdaemon(Param* param, int request);
extern int GetSNMPv12Entry(Param* param, int request);
extern int GetSNMPv3Entry(Param* param, int request);
extern int GetNetServices(Param* param, int request);
extern int GetHostTable(Param* param, int request);
extern int GetVPNTable(Param* param, int request);
extern int GetStaticRoutes(Param* param, int request);
extern int GetStaticRouteEntry(Param* param, int request);
extern int GetVirtualPortsInfo(Param* param, int request);
extern int GetVirtualPort(Param* param, int request);
extern int GetPhysTableHtml(Param* param, int request);
extern int GetPhysPConf(Param* param, int request);
extern int GetPhysPmIpduNameHtml(Param* param, int request);
extern int GetPhysPmOutletHtml(Param* param, int request);
extern int GetActiveSessions(Param* param, int request);
#ifdef PCMCIA
extern int GetAccessMethodInfo(Param* param, int request);
#endif
extern int GetSecProfile(Param* param, int request);

extern int GetIpTableChainsHtml(Param* param, int request);
extern int GetIpTableChains(Param* param, int request);
extern int GetIpTableRules(Param* param, int request);
extern int GetIpTableRuleSpec(Param* param, int request);
extern int GetNumIpdus(Param* param, int request);

//[RK]Jun/14/04 - Access Privilege
extern int GetRegUserAuthLocal(Param* param, int request);

//[RK]Sep/14/04 - Ports Status
extern int GetPortsStatusHtml(Param* param, int request);
//Jan/31/05 - Ports Statistics
extern int GetPortsStatisticsHtml(Param* param, int request);

#if defined(KVM) || defined(ONS)
extern int GetUnitKVMIPSecurityStatus(Param* param, int request);
extern int GetKVMDConf(Param* param, int request);
extern int GetKVMDirectAccConf(Param* param, int request);
extern int GetLoginDirectAccConf(Param* param, int request);
extern int GetKVMAccList(Param* param, int request);
extern int GetKVMDevicePermissions(Param *param, int request);
extern int GetKVMDevice(Param* param, int request);
extern int GetKVMDeviceOutlets(Param* param, int request);
extern int GetKVMDeviceSel(Param* param, int request);
extern int GetKVMDeviceTerm(Param* param, int request);
extern int GetKVMDeviceTermSel(Param* param, int request);
extern int GetKVMuCupdate(Param* param, int request);
extern int GetKVMuCreset(Param* param, int request);
extern int GetKVMauxPort(Param* param, int request);
extern int GetMaxOutlet(Param* param, int request);
/* to be removed later .. */
extern int GetKVMConf(Param* param, int request);
#endif
extern int GetAuthType(Param* param, int request);
extern int GetRadiusConf(Param* param, int request);
extern int GetTacplusConf(Param* param, int request);
extern int GetLdapConf(Param* param, int request);
extern int GetKrbConf(Param* param, int request);
extern int GetSmbConf(Param* param, int request);
extern int GetNisConf(Param* param, int request);
#endif /* this must be the last line in this file. */

#ifdef RDPenable
extern int GetRDPDevice(Param* param, int request);
extern int GetRDPDeviceSel(Param* param, int request);
extern int RDPDeviceCommit(Param* param, int request, char_t* urlQuery);
#endif	//RDPenable

/*warning: implicit declaration of function `handle_VPN_delete'*/
extern void handle_VPN_delete(void);

// Config Online Help
extern int GetOnlineHelpConf(Param* param, int request);
extern int OnlineHelpCommit(Param* param, int request, char_t* urlQuery);
