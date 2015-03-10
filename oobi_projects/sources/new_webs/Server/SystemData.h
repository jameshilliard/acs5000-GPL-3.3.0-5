/* (c) 2003 Cyclades. All Rights Reserved. */
/* $Id: SystemData.h,v 1.37 2010/07/19 23:59:11 regina Exp $ */

#ifndef h_SystemData
#define h_SystemData

#include "webApi.h" // ACS/TS-specific data

#if defined(KVM) || defined(ONS)
#include "kvmconfig.h"
#endif

typedef struct
{
   CycDeviceClass klass;
   char_t model[kModelLength];
   char_t picture[kPictureLength];
   UInt32 number;
#ifdef ONS
   UInt32 number2; /* used for # of serial port in ONS */
   UInt32 numberSA; /* used for # of aux port plus offset in ONS */
#endif
   char_t swVersion[kSwVersionLength];
   char_t olhVersion[kSwVersionLength];
   /* The following 3 are duplicates of NetSettings hostName, ipAddress1,
   and gateway, but we need to keep separate copies in case an admin is
   changing the one in NetSettings; all users can view these */
   char_t hostName[kHostNameLength];
   char_t ip[kIpLength];
#ifdef IPv6enable
   char_t    ip6[kIp6Length];
   UInt32    prefix6;
#endif
} Device;

typedef struct
{
   UInt32    mode;
   UInt32    dhcp;
   char_t    hostName[kHostNameLength];
   char_t    ipAddress1[kIpLength];
   char_t    netMask1[kIpLength];
   DnsService dnsService;
   char_t    gateway[kIpLength];
   //members below are for normal mode only
   char_t    consBanner[kConsoleBannerLength];
   char_t    ipAddress2[kIpLength];
   char_t    netMask2[kIpLength];
   UInt32    mtu;
#ifdef BONDING_FEATURE
   BondingService bond;
#endif
#ifdef IPv6enable
   UInt32    enableIPv4;
   UInt32    enableIPv6;
   UInt32    IPv6method;
   char_t    ethIp6[kIp6Length];
   UInt32    ethPrefix6;
   UInt32    DHCPv6Opts;
   char_t    gateway6[kIp6Length];
#endif
} NetSettings;

typedef struct
{
   int protocol;
   UInt32 baudRate;
   UInt32 dataSize;
   UInt32 stopBits;
   UInt32 parity;
   UInt32 flowControl;
   UInt32 authReq;
   UInt32 enableAll;
} CasProSettings;

typedef struct
{
   UInt32 mode;
   char_t *usersHtml; /* _Derived_ data */
   char_t usersCs[kListLength]; /* _Derived_ data */
   char_t *groupsHtml; /* _Derived_ data */
   char_t groupsCs[kListLength]; /* _Derived_ data */
   UInt32 action;
   UInt32 selected;
   ConfGroups addModGroup;
   Users addModUser;
   UInt32 changedUser;
   UInt32 numUsers;
   Users* users;
   UInt32 changedGrp;
   UInt32 numGroups;
   ConfGroups* groups;
} CycAccess;

typedef struct
{
   char_t users[kUserListLength];
   char_t groups[kUserListLength];
} AuthSecurity;

typedef struct
{
   UInt32 mode;
   UInt32 fileSize;
   char_t nfsPath[kDirPathLength];
   UInt32 showMenu;
   UInt32 timeStamp;
} DataBuffering;

typedef struct
{
   UInt32 mode;
   UInt32 facilityNumber;
#if defined(KVM) || defined(ONS)
   UInt32 KVMfacility;
#endif
   UInt32 dest;
   UInt32 dummy;
   FilterLevels filter;
   /* the following is _derived_ data */
   char_t *serversHtml;
   char_t serversCs[kListLength];
   /* WebApi struct */
   UInt32 numServers; /* somewhat redundant */
   SyslogServerInfo *servers;
} SystemLog;


// APPLICATIONS
// Power Management

// Outlets Manager
#if defined(PMD) || defined(IPMI)
typedef enum {
	no_Action,         // 0
	Turn_ON,     // 1
	Turn_OFF,    // 2
	Lock,        // 3
	Unlock,      // 4
	Cycle,       // 5
    Save,        // 6
	Edit,        // 7
	Clear_Current,     // 8
	Clear_EnvMon, // 9
	Refresh_LatestSw,       //10
	Upgrade_UnitFw,         //11
	Msg_to_WEB, // 12
	Add, // 13
	Delete, //14
	Show, //15
	Done, //16
	Page_reload, //17
	Automatic_reload, //18
	Clear_Power, //19
	Clear_Voltage, // 20
	Clear_PowerFactor, // 21
	Reset_HWOvercurrent, // 22
} PMActions;
#endif

#ifdef PMDNG
typedef struct
{
	UInt32             numPorts;
	UInt32             numIpdus;
	char_t            *portInfoHtml;
	char_t            *ipduInfoHtml;
	char_t            *outletInfoHtml;
	char_t            *outcapInfoHtml;
	UInt32             showIpdu; //indicates 1st or 2nd level web page
	char_t             ipduName_str[kPMDipduIDLength+kPmStringLength]; //name & serial port (2nd level)
	char_t             ipduModel_str[kPmModelLength+kPmVendorLength]; //vendor & model (2nd level)
	UInt32             isServerTech; //used in 2nd level web page
	UInt32             ipduNofOut; //used in 2nd level web page
	PMActions          action; //a communication channel from Web Space
	char_t             sysMsg[kInfoLength]; //system messages to and from Web Space
	UInt32             memSignature;
	//fields below are not part of the corresponding structure in Param.c
	PmIpduPortInfo    *portInfo;
	PmIpduOutletsInfo  ipduOutInfo;
} ipduOutMan;

typedef struct
{
	UInt32              numIpdus;
	UInt32	            numGroups;
	char_t             *groupInfoHtml;
	char_t             *outletInfoHtml;
	PMActions           action; //a communication channel from Web Space
	char_t              sysMsg[kInfoLength]; //system messages to and from Web Space
	UInt32             memSignature;
	//fields below are not part of the corresponding structure in Param.c
	PmIpduOutGrpInfo   *groupInfo;
	UInt32              totalGroups; //to be able to correctly free memory later...
	PMDOutGrpsDef      *groupDefs;
} ipduOutGrpCtrl;

typedef struct
{
	UInt32	           numIpdus;
	char_t            *ipduInfoHtml;
	char_t            *confcapHtml;
	PMActions          action; //a communication channel from Web Space
	char_t             sysMsg[kInfoLength]; //system messages to and from Web Space
	UInt32             memSignature;
	//fields below are not part of the corresponding structure in Param.c
	UInt32	           numPorts;
	PmIpduPortInfo    *portInfo;
} viewConfIpdu;

typedef struct
{
	UInt32	           numIpdus;
	UInt32	           nValidIpdus;
	char_t            *swupInfoHtml;
	char_t             latest_sw[kPmModelLength];
	PMActions          action; //a communication channel from Web Space
	char_t             sysMsg[kInfoLength]; //system messages to and from Web Space
	UInt32             memSignature;
	//fields below are not part of the corresponding structure in Param.c
	UInt32	           numPorts;
	PmIpduPortInfo    *portInfo;
} ipduSwUpgrade;

typedef struct
{
	UInt32              numVendors;
	char_t             *confInfoHtml;
	char_t              sysMsg[kInfoLength]; //system messages to and from Web Space
	//fields below are not part of the corresponding structure in Param.c
	PMDGeneralInfo      genInfo[3]; //Cyclades, SPC and ServerTech
} PMDGen;

typedef struct
{
	char_t             *groupsListHtml;
	char_t              entries[kPMOutGrpTableEntriesLength];
	char_t              sysMsg[kInfoLength]; //system messages to and from Web Space
	//fields below are not part of the corresponding structure in Param.c
	PMDOutGrpsDef       groups[kPmMaxGroups];
	PMDOutGrpsDef       singleGroup;
	UInt32	            numGroups;
	UInt32	            changed;
	Action              addEditEntry;
	int	                savedIndex;
} PMDOutGrp;

typedef struct
{
	char_t             *usersListHtml;
	char_t              entries[kPMUserManTableEntriesLength];
	char_t              sysMsg[kInfoLength]; //system messages to and from Web Space
	//fields below are not part of the corresponding structure in Param.c
	PMDUsersMgm         usersMgm[kPmMaxUsers];
	PMDUsersMgm         singleUser;
	UInt32	            numUsers;
	UInt32	            changed;
	Action              addEditEntry;
	int                 savedIndex;
} PMDUserMan;
#endif

// IPMI
#ifdef IPMI
typedef struct
{
	UInt32		numIpmiDev;
	char_t		*ipmiInfoHtml;
	IPMIDeviceInfo	*ipmiInfo;
	PMActions	action; //a communication channel from Web Space
	char_t		sysMsg[kInfoLength]; //system messages to and from Web Space
	IPMIDeviceInfo  ipmidev;  // add/edit information
	UInt32		numActiveIpmiDev;
} PMIPMI;
#endif


// PORTS
// Physical Ports ID-25

typedef struct
{
	char_t *physTableHtml;	
	UInt32	action;		// 0-None, 1-Disable, 2-Enable, 3-Modify
	char_t	selected[kUserListLength]; // 0-all or port list ,<number>,
	UInt32	numPortsel;	// number of selected ports
	PhysPortsInfo infoP;    // configuration of the first selected port
	UInt32	physPortAction; // 0-None, 1-Cancel 2-Done
#ifdef PMD
	UInt32	numPowerMgm;	// number of serial ports configured as IPDU
#endif
#ifdef IPMI
	char_t  *ipmiDeviceListHtml; // alias to used in the portsPhysPowerManEntry
#endif
	// webApi struct
	PhysPortsInfo *physList;
	char_t	ADProbeString[2*kADStringLength+1];
	char_t	ADAnswerString[2*kADStringLength+1];
} PhysPorts;

typedef struct
{
	UInt32		unitauthtype;
	UInt32		pppauthtype;
#ifdef NISenable
	NisInfo		nis;
#endif
	LdapInfo	ldap;
#ifdef KRBenable
	KrbInfo		krb;
#endif
	RadiusInfo	radius;
	TacplusInfo	tacplus;
#ifdef SMBenable
	SmbInfo		smb;
#endif
} AuthUnit_t;
//typedef struct
//{
//	UInt32 type;
//	UInt32 radius;
//	UInt32 tacplus;
//#ifdef KRBenable
//	UInt32 krb;
//#endif
//	UInt32 ldap;
//#ifdef SMBenable
//	UInt32 smb;
//#endif
//#ifdef NISenable
//	UInt32 nis;
//#endif
//} AuthAction_t;

// Virtual Ports ID-107
typedef struct
{
   char_t virtualInfoSet[kUserListLength];
   char_t   *virtualInfoHtml;
   UInt32   action;
   UInt32   nports;
   char_t remoteIp[kIpLength];
   char_t clusteringIp[kIpLength];
   VirtualPortsInfo virtualPort;
   UInt32   numVirtual;
   VirtualPortsInfo *virtualInfo;
} VirtualPorts;


// NETWORK

// SNMP Daemon Settings: IDs-23...
typedef struct
{
	char_t         sysContact[kInfoLength];
	char_t         sysLocation[kInfoLength];
	UInt32	       editIndex;
	char_t         webMsg[20]; //a communication channel from Web Space

	UInt32	       numSNMPv12;
	CommunityInfo *SNMPv12p;
	char_t        *SNMPv12Html;
	char_t         SNMPv12Entries[kSNMPdaemonListLength]; //up to 100 entries: "01 02 03...98 99"
	UInt32	       SNMPv12Changed;
	CommunityInfo  SNMPv12;

	UInt32	       numSNMPv3;
	Snmpv3Info    *SNMPv3p;
	char_t        *SNMPv3Html;
	char_t         SNMPv3Entries[kSNMPdaemonListLength]; //up to 100 entries: "01 02 03...98 99"
	UInt32	       SNMPv3Changed;
	Snmpv3Info     SNMPv3;
} SNMPdaemonSettings;

// Services ID-24
/*typedef struct
{
	UInt32     telnet;
	UInt32     ssh;
	UInt32     snmp;
#ifdef IPSEC
	UInt32     ipsec;
#endif
	UInt32     sunrpc;
	WebService webService;
} NetServices;*/

/* [LMT] Security Enhancements */
typedef enum
{
        kNoSecWarning,
        kSecAdvisory,
        kSecWebsRestart,
        kSecPortsWarning,
        kSecPortsWebs
} SecWarnings;


/* [LMT] Security Enhancements */
typedef struct
{
        UInt32       mode;
        UInt32       telnet;
        SshdOptions  ssh;
        HttpOptions  webUI;
        UInt32       snmp;
        UInt32       rpc;
#ifdef KRBenable
        UInt32       ftp;
#endif
#ifdef ONS
        UInt32       tftp;
#endif
#ifdef IPSEC
        UInt32       ipsec;
#endif
        UInt32       icmp;
        SecOptions   options;
        //the fields below are used for the WebUI only
        SecWarnings       showWarning;
} SecProfile;

// Host Table ID-55
typedef struct
{
   UInt32   hostsChanged;
   char_t *hostTableHtml;
   UInt32   numHostsSet;
   char_t hostTableSet[kUserListLength];
   UInt32   numHosts;
   Hosts   *hosts;
} HostTable_st;

#ifdef IPSEC
// VPN Table ID-101
typedef struct
{
	UInt32	VPNChanged;
	char_t *VPNTableHtml;
	char_t  VPNTableDel[kUserListLength];
	UInt32	numVPN;
	UInt32	currEntry;
	char_t pubRsaKey[kRsaKeyLength];
	IpSecInfo	*VPN;
} VPNTable_st;
#endif

// Static Routes: IDs-59/60
typedef struct
{
	UInt32	      numRoutes;
	StRoutesInfo *routes;
	char_t       *stRoutesHtml;
	UInt32	      routesChanged;
	char_t        entriesList[kStRoutesEntriesListLength]; //up to 100 entries: "01 02 03...98 99"
	StRoutesInfo  route;
	UInt32	      editIndex;
} StaticRoutes;


// ADMINISTRATION

typedef struct
{
	char_t command[kFtpCmdLength];
	char_t ServerName[kHostNameLength];
	char_t bkptype[kSDoption];
	char_t SDDefaultoption[kSDoption];
	char_t SDReplaceoption[kSDoption];
	char_t PathName[kPathFilenameLength]; //[RK]Sep/13/04 - increase from 50 to 100
	char_t UserName[kUserNameLength];
	char_t Password[kPasswordLength];
	char_t last_command[kFtpCmdLength];
	char_t *Message;
} backupConfig_st;

typedef struct
{
  char_t *versionHtml; /* _Derived_ data */
   UnitVersionInfo version;
   char_t mac_address[kMacAddressLength];
   char_t date[kDateLength];
   char_t upTime[kUpTimeLength];
   PwrSupplyInfo pwrSupply;   
   char_t *pwrSupplyHtml;   
   CpuInfo cpuInfo;
	UInt32 numPcmcia;
#ifdef PCMCIA
   PcmciaInfo *pcmcia;
   char_t *pcmciaHtml;   
#endif
   MemInfo memory;
   InterfaceInfo ifInfo;
   char_t *memHtml;   
	UInt32 numRamDisk;
   RamDiskInfo *ram;
   char_t *ramHtml;
   UInt32 reboot;
#if defined(KVM) || defined(ONS)
   UInt32 kvmanalog;            /* 1 if analog KVM, 0 if kvmnet */
   UInt32 numkvmports;          /* number of KVM ports (16/32) */
   UInt32 numkvmports_1;        /* number of KVM ports - 1 for index */
   UInt32 numkvmstations;       /* number of KVM stations (2) */
   UInt32 numkvmstations_1;     /* number of KVM stations - 1 for index */
   UInt32 maxkvmports;		/* maximum of KVM ports */
   char_t *kvmPortStatus[kMaxKVMStationsDef+1]; /* Add one for termination */
   char_t kvmIPSecurityStatus[256];
   UInt32 fanStatus[3];
#endif
   char_t serialNumber[32];
} UnitInfo;

// Notifications ID-110
typedef struct
{
   UInt32 alarm;
   char_t *notifHtml;
   char_t notifTable[kUserListLength];
   char_t *triggerHtml;
   SInt32 triggerId;
   SInt32 notifType;
   char_t trigger[kTriggerLength];
   Events event;
   /* WebApi struct */
   UInt32 numNotif; 
   NotificationInfo *notif;
} Notifications;

typedef struct
{
	UInt32 ntp;
	char_t ntpServer[kServerLength]; //[RK]Sep/17/04 - accept host name
	char_t timezone[kTimezoneLength];
	DateTime dt;
	TimezoneData tz; //mp[Dec/2005] Adding stuff for Timezone Enhancement
} TimeDate;

typedef struct
{
   MenuShInfo menuShInfo;
   MenuShOpt addModOption;
   char_t *menuOptionHtml;
   UInt32 selectedOption;
   UInt32 menuShellChanged;
   char_t optionsCs[kHtmlListLength];
	UInt32 action;
} MenuShell;

typedef struct
{
	// LogTcpOptions
	UInt32 logTcpSeq;
	UInt32 logTcpOpt;
	UInt32 logIpOpt;
	// Tcp Options
	UInt32 fin;
	UInt32 syn;
	UInt32 rst;
	UInt32 psh;
	UInt32 ack;
	UInt32 urg;
} TcpOptions;

typedef struct
{
	UInt32 numChains;
	UInt32 confNumChains;
#ifdef IPv6enable
	UInt32 confNumChains6;
#endif
	char_t *ipTableHtml;
	char_t *rulesHtml;
	char_t chainsCs[kUserListLength];
	UInt32 chainSelected;
	char_t chainName[kChainNameLength];
#ifdef IPv6enable
	char_t chainType[kChainTypeLength];
#endif
	char_t targetHtml[kUserListLength];
	UInt32 numRules;
	char_t rulesCs[kUserListLength];
	UInt32 ruleSelected;
	UInt32 actionRule;
	IpTablesRules rule;
	TcpOptions tcpOpt;
	UInt32 chainChanged;
	ChainInfo *chainTable;
#ifdef IPv6enable
	ChainInfo *chainTable6;
#endif
} IpTable;

typedef struct
{
	UInt32 selectedOption;
	UInt32 action;
	char_t *sessionHtml;	
	UInt32 numSessions;
	UInt32 totalSessions;
	ActiveSessionInfo *sess;
} ActivePortSession;

// General Data Structure

#ifdef PCMCIA
typedef struct
{
	UInt32 pcmConfigChanged;
	char_t ident[2][kIdentLength];
	PcmciaType pcmType;
	UInt32 selectedOption;
	UInt32 action;
} PcmciaAccessMethod;
#endif

// Loadkey

typedef struct _LoadKeyInfo {
    int     numKeys;
	char_t  UserName[kUserNameLength];
	char_t  URL[kURLLength];
    LoadKey *users_key;
} LoadKeyInfo;

// regular User - change password
typedef struct {
	int	authLocal; // 1 - user was authenticated in local database
	int	operResult; // 0 - NONE, 1 - OK, 2 - Old passwd wrong 
	char_t  oldPasswd[kPasswordLength];
	char_t  newPasswd[kPasswordLength];
} RegUserSecurity;

#if defined(KVM) || defined(ONS)
typedef struct {
	UInt32 kvmdChanged;             /* set to TRUE when the table was changed*/
	KVMDConfig kvmd;                /* KVMD configuration */
} KVMConfig;

typedef struct {
	UInt32 numOutlets;             /**/ 
} KVMtemp;

typedef struct {
	/* This part is common with the webAPI */
	KVMUserAccessConfig useracc;
	/* This part is only used in the middleware */
	char_t *KVMPermHtml;
	char_t *KVMDefPermHtml;
	char_t entryName[kUserNameLength];
	char_t currName[kUserNameLength];/* current user or group name to be configured */
	UInt32 currUserGroup;           /* 0 if user, 1 if group */
	UInt32 currDefault;             /* 1 if default access list for the current user/group */
	KVMUserPermConfig userperm;     /* access list being edited */
	UInt32 nalloc;			/* number of entries allocated */
	UInt32 numports;		/* number of ports of the current device */
	UInt32 devicenum;		/* number of the device in the list */
	char_t currDevice[kCascNameLength];/* current device */
	char_t ports_none[kPortRangeLength];
	char_t ports_ro[kPortRangeLength];
	char_t ports_rw[kPortRangeLength];
	char_t ports_full[kPortRangeLength];
	UInt32 from;
	UInt32 cancel;
	UInt32 tableChanged;            /* set to TRUE when the table was changed */
} KVMUserAccList;

// Devices : 
typedef struct {
	char_t *deviceTableHtml;
	char_t *deviceTable;
	char_t deviceMsgError[kInfoLength];
	char_t deviceNameList[kUserListLength];
	char_t deviceNTermsList[kUserListLength];
	char_t deviceCs[kUserListLength];
	UInt32 deviceAction;
	UInt32 deviceSelected;
	KVMCascConfig confDevice;
	// terms of cascading
	char_t *termTableHtml;
	UInt32	action;	
	char_t	selected[kUserListLength]; // 0-all or port list ,<number>,
	KVMTermConfig confTerm;    // configuration of the selected port
	// webApi struct
	UInt32  changed;
	UInt32	numDevices;
	UInt32	maxOutlet;
	KVMCascConfig *deviceList;
	char_t	  *outletsHtml;
	char_t    *portsUsedHtml;
	char_t    *pmoutletHtml;
	//char_t *devicePMStatus[32];
} KVMDevice;

#endif // KVM

#ifdef PMDNG
typedef struct {
	// PMD configuration
	int		numGeneral;
	PMDGeneralInfo	*general;
	PMDPortInfo	oneport;
	int		numports;
	PMDPortInfo	*ports;
	PMDIpduInfo	oneipdu;
	int		numipdus;
	PMDIpduInfo	*ipdus;
	PMDOutGrpsDef	onegrp;
	int		numgrps;
	PMDOutGrpsDef	*grps;
	PMDUsersMgm	oneuser;
	int		numusers;
	PMDUsersMgm	*users;
} PMDConfig;
#endif

//[AP][2007-08-02] hostname discovery
typedef struct {
	int ADStringChange;
	char_t ADProbeString[2*kADStringLength+1]; // doubling the size for the escape sequence expansion
	char_t ADAnswerString[2*kADStringLength+1]; // doubling the size for the escape sequence expansion
} AutoDiscovery;

typedef struct
{
    Device device;
    NetSettings netSettings;
    CasProSettings casProSettings;
    CycAccess access;
    AuthSecurity authSecurity;
    DataBuffering dataBuffering;
    SystemLog systemLog;
    UnitInfo unitInfo;
    BootConf bootInfo;
    UpGradeInfo upGradeInfo;
    TimeDate timeDate;
    Notifications notifications;
	SNMPdaemonSettings SNMPdaemon;
	HostTable_st hostTable;
#ifdef IPSEC
	VPNTable_st VPNTable;
	IpSecInfo	VPNEntry;
#endif
	StaticRoutes staticRoutes;
	backupConfig_st backupConfig;
	MenuShell menuShell;
	VirtualPorts virtualP;
	IpTable ipTable;
	char_t *connectPortsHtml;
	char_t *portsStatusHtml;	//[RK]Sep/14/04 - Ports Status
	char_t *portsStatisticsHtml;	//[RK]Sep/14/04 - Ports Status
	UInt32	numIpdus;
	PhysPorts	physP;
	SyslogBufServerInfo sysBufServer; // one Syslog Remote Server to sysbuf
	AuthUnit_t	authU;
//	AuthAction_t authAction;
	ActivePortSession session;
#ifdef PCMCIA
	PcmciaAccessMethod accessMethod;
#endif
	LoadKeyInfo	   LoadedKeys;
	RegUserSecurity	   regUser;
	SecProfile     secProfile; /* [LMT] Security Enhancements */
#ifdef IPMI
	PMIPMI		pmipmi;
#endif
#if defined(KVM) || defined(ONS)
	char_t	*KVMconnectPortsHtml;
	KVMConfig      kvmconf;
	KVMUserAccList kvmuser;

	KVMDevice	kvmdevice;
	KVMMicrocode   kvmuCup;
	KVMMicrocode   kvmuCres;
	KVMAuxPort     kvmAuxPort[MAX_AUX];
	KVMtemp		temp;
	/* to be deleted later */
	struct _kvm_conf kvmConf;
#endif // KVM

#ifdef RDPenable
	RDPDevice	rdpdevice;
#endif	//RDPenable

    OnlineHelpConf onlineHelp;

#ifdef PMDNG
	ipduOutMan      IpduOutMan;
	ipduOutGrpCtrl  IpduOutGrpCtrl;
	viewConfIpdu    ViewConfIpdu;
	ipduSwUpgrade   IpduSwUpgrade;
	// PMD configuration
	PMDConfig	pmdConf;
	// PMD configuration (WebUI)
	PMDGen      pmdGen;
	PMDOutGrp   pmdOutGrp;
	PMDUserMan  pmdUserMan;
#endif
	//[AP][2007-08-02] hostname discovery
	AutoDiscovery	autoDiscovery;
} SystemData;

#endif
