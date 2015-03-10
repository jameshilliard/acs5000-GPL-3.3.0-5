#ifndef h_WebApi
#define h_WebApi

#include "AITypes.h"
#include "Bounds.h"
#include "ResultCodes.h"

extern int pmdTimer;

#define PMDGUARDTIME 5 //20sec

#define setPMDtimer(t) pmdTimer=(t)
#define checkPMDguardtime(t) \
{\
	if (pmdTimer) {\
int elipsed = (t)-pmdTimer;\
		if (elipsed < PMDGUARDTIME) {\
			sleep(PMDGUARDTIME-elipsed);\
		}\
		pmdTimer = 0;\
	}\
}

// PM firmware version
#define PMVERSION110	0x110
#define PMVERSION122	0x120
#define PMVERSION160	0x160    //[GY]2005/Dec/20  BUG#4726
#define PMNGVERSION200	0x200

#if defined (KVM) && !defined (KVMNETP) && !defined(ONS)
#define MAX_AUX	1
#elif defined(KVMNETP)
#define MAX_AUX 2	
#elif defined(ONS)
#define MAX_AUX 3
#endif

// Session Inactivity Control
#define MAX_SESS_IDLE_TIME   10 * 60 //mp: maximum idle time (s) a user is still accept as "logged in"

// Total max number of users (regular + admin)
#define MAX_WEB_USERS		60

#ifdef PMDNG
#define PMDCONF "/etc/pmd.conf"
#define PMDUSER "/etc/pmd.users"
#define PMDGRPS "/etc/pmdgrp.conf"
#endif

typedef UInt32 Sid;

typedef enum {
	kSessAdminOk = 1,
	kSessUserOk,
	kSessBumped,
	kWannaBeAdmin
} WebUserSessStatus;

typedef struct _WebUserEntry {
	Sid	sid;
	Sid	passK;		//[RK]Jul/29/05 - Key to decryp pass
	int	lastAccess;
	short	ix_admin;
	WebUserSessStatus	status;
	char	userName[32];
	char_t	pass[kPasswordLength*2]; //2x because pass is stored in its hex val
#ifdef GRPAUTHenable
	char 	*group_name;	//[RK]Jun/02/05 - Group Authorization Support
#endif
#ifdef PMD
	int 	showIpdu;	//[RK]Nov/03/06
#endif
} WebUserEntry;

/* Category List */
/* ------------------------------------------------------------------------- */
typedef enum
{
   kSecProfiles,  /* [LMT] Security Enhancements */
   kActiveSessions,
   kBootManagement,
   kCycDateTime,
   kDevice,		//R/O
   kFirmWare = kDevice,
   kHostConf,		//Host.conf
   kBonding,            //bonding
   kHostnameSetting,	//Hostname
   kHostTable,		//Hosts
   kInetd,
   kIpChains,		//IPChains-network/firewall-TS
#ifdef IPSEC
   kIpSec,
#endif
   kIpTables,		//IPTables-network/firewall-ACS
#ifdef IPv6enable
   kIp6Tables,
#endif
   kIssueSetting,	//Issue
#ifdef KRBenable
   kKrb5Conf,
#endif
   kLdapConf,
   kRadiusConf,
   kTacplusConf,
#ifdef SMBenable
   kSmbConf,
#endif
   kMenuShCfg,
#ifdef NISenable
   kNis,
#endif
   kNtp,
   kPasswd,		//Passwd
#ifdef PCMCIA
   kPcmcia,
#endif
   kPortsSpec,		//Signals specific ports
	kPortMap,
   kPslave,		//Pslave.conf
   kResolvConf,		//Resolv.conf
   kRoutingTable,
   kSnmpdSettings,
   kSshd,
   kStRoutes,		//St_Routes
   kSyslogNg,		//Syslog-ng.conf
   kUnitInfo,
#ifdef PMD
   kPowerMgm,		// Applications -> Power Management
#endif
   //kUpGrade,
   kLoadKey,
	kWebUI,
	kKVM,
	kKVMCascading, /* Configuration of the cascade devices in kvmd.slaves */
	kKVMGroup,
	kIPMIDevice,		//[RK]Dec/22/04 - IPMI config
	kRDPDevices,
   kOnlineHelpConf,
#ifdef PMDNG
   kPMDConfig,   // Applications -> PMD Configuration
#endif
} CycCategory;


/* Key List */
/* ------------------------------------------------------------------------- */
typedef enum
{
   /* 0 */ k2ndIpAddress,
   k2ndNetMask,
   kAcctHost1,
   kAcctHost2,
   kActiveSessionInfo,
   kAdminUsers,
   kAlarm, 
   kAnalogStatus,
   kAuth,
   kAuthHost1,
   /*10*/ kAuthHost2,
   kBaudRate,
	 kBiometric,
   kBits,
   kBootConf,
   kBrkInterval,
   kBrkSeq,
   kClusteringIp,
   kCommonConfigFile,
   kCommunityInfo, 
   /* 20 */ kConfFacilityNum,
   kConfGroup,
   kConsoleBanner,
   kCpuInfo, 
   kDate, 
   kDbFacilityNum,
   kDbFileSize,
   kDbMode, 
   kDcdSensitive,
   kDefaultUserName, 
   /* 30 */ kDhcpClient,
   kDigitalStatus,
   kDnsService,
   kDtrOffTimeInterval,
#ifdef WINEMSenable
   kEmsTranslation,
   kEmsWebAccess,
   kEmsXmlMonitor,
#endif
   kEscapeChar,
   kFanStatus,
   kFanStatus2,
   /* 40 */ kFanStatus3,
   kFlow,
#ifdef KRBenable
   kFtpEnable, /* [LMT] Security Enhancements */
#endif
#ifdef ONS
   kTftpEnable, /* [LMT] Security Enhancements */
#endif
   kRpcEnable, /* [LMT] Security Enhancements */
   kIcmpEnable, /* [LMT] Security Enhancements */
   kWebUIOpt, /* [LMT] Security Enhancements */
   kSshdOpt, /* [LMT] Security Enhancements */
   kSecOpt, /* [LMT] Security Enhancements */
   kIpAddress,
   kHostName,
   /* 50 */ kHosts,
   kIdleTimeOut,
   kInitChat,
   kInputStrAutoAns,
   kIpChainsInfo,
   kIpChainsNum,
   kIpChainsRuleInfo,
#ifdef IPSEC
   kIpSecEnable,
   kIpSecInfo,
#endif
   kIpTablesFilterChainInfo,
   /* 60 */ kIpTablesFilterChainNum,
   kIpTablesFilterChainRuleInfo,
#ifdef IPv6enable
   kIp6TablesFilterChainInfo,
   kIp6TablesFilterChainNum,
   kIp6TablesFilterChainRuleInfo,
#endif
   kIpTablesNatChainInfo,
   kIpTablesNatChainNum,
   kIpTablesNatChainRuleInfo,
   kIssue,   
#ifdef KRBenable
   kKrbInfo,
#endif
   kLdapInfo,
   kRadiusInfo,
   kTacplusInfo,
#ifdef SMBenable
   /* 70 */ kSmbInfo,
#endif
   kLineFeedSuppress,
   kLoginPrompt,     
   kMemInfo, 
   kMenuShInfo,
   kMenuShNum,
   kMenuShTitle,
#ifdef MODBUS
   kModbusSmode,
#endif
   kModel,
   kPicture,
   /* 80 */ kNumber,
   kMthDayYrHrMinSec, 
   kTimezone,
   kMtu,
   kMultiSess,
   kMultiUserNotif,
   kNameServiceAccess,
   kNetMask,
   kNfsFilePath,
#ifdef NISenable
   kNisEnable,
   /* 90 */ kNisInfo,
#endif
   kNotificationInfo,
   kNotificationType, 		// Do nothing
   kNtpEnable,
   kNtpServer,
   kNumCommunityEntries, 
   kNumConfGroups,
   kNumEntries,
#ifdef PMD
   kNumIpduEntries,
#endif
#ifdef IPSEC
   kNumIpSec, 
#endif
   /* 100 */ kNumMenuShOpt,
   kNumNotificationEntries,
#ifdef PCMCIA
   kNumPcmciaSlots,
#endif
   kNumSnmpv3Entries,
   kOutputStrAutoAns,
   kParity,
#ifdef PCMCIA
   kPcmciaInfo,
#endif
   kPollInterval,
   kPoolIpno,
   kPoolServerFarm,
   /* 110 */ kPoolSocketPort,
   kPortBundle,
   kPortIpAddress,
   kPortIpno,
   kPortUsers,
	kPppopt,
   kProtocol, 
   kPwrSupplyInfo,
   kRadRetries,
   kRadTimeOut,
   /* 120 */ kRamDiskInfo,
   kRoutingTableEntry, 
   kSecret,
   kServerFarm,
   kShowMenu,
   kSniffMode, 
   kSnmpdEnable,
   kSnmpv3Info,
   kSocketPort,
   kSshdEnable,
   /* 130 */ kStopBits,
   kStRoutesInfo,
   kSttyCommand, 
	kSunrpc,
   kSwVersion,
   kSysContact,
   kSysLocation,
   kSyslogBuffEnable,
   kSyslogBuffSess,
   kSyslogBuffSize,
   /* 140 */ kSyslogBufServer,
   kSyslogServer,
   kTelnetClientMode,
   kTelnetEnable,
   kTerminalType,     
   kTimeStamp,
   kTransInterval,
   kTs110,
   kTsHost,
   kTty,
#ifdef PCMCIA
   /* 150 */ kUnitPcmciaInfo, 
#endif
   kUnitVersionInfo, 
   kUpTime, 
   kUsers,
   kVirtualPortsInfo,
   kPhysPortsConf,
   kWriteLoginRecordsUtmp,
   kWriteLoginRecordsWtmp,
   kDeleteEntry,
#ifdef PMD
	kNumPorts,	// number of serial ports that are configured as IPDU/Cyclades
/* 160 */	kPortInfo,
	kOutletInfo,
	kIpduSerPortNumber,
	kIpduConfSyslog,
	kIpduConfBuzzer,
	kIpduConfOverCurrent,
	kIpduUnitNumber,
 	kIpduUnitInfo,
	kIpduUnitAlarm,
	kIpduUnitCurrent,
/* 170 */	kIpduUnitTemperature,
	kIpduUserMgm,
	kFirmUpgradeVersion,
#endif
	kLoadPubKey,
	kUnitAuth,
	kPPPAuth,
	kLogDestEnable,
   	kLogFilterlevel,
	kOnlineHelp,
	kTzParams,
/* 180 */   kNumberS,
   kNumberSA,
	kKVMAuxPort,
	kKVMMaxOutlet,
	kKVMBoard,
	kKVMIPCardInfo,
	kKVMPortStatus,
	kKVMIPSecurityStatus,
	kKVMDConf,      /* configuration in kvmd.conf */
	kKVMAccEntries,
	kKVMPermEntries,
	kKVMAccList, /* Configuration of the user access list */
	kKVMMicrocode,
	kKVMCascNumDevices,
	kKVMGroups,
	kKVMPhysPorts,		// get/set physical ports of Master device
	kKVMCascConf,		// get/set configuration of slave devices
	/* to be deleted later */
	kKVMConf,
	kKVMAuthType,
	kIPMIDeviceInfo,		//[RK]Dec/22/04 - IPMI config
	kRDPDevConf,
   kBondingEnable, //bonding
   kBondingMiimon,
   kBondingUpdelay,
#ifdef IPSEC
   kIpSecPubKey,
#endif 
   kInterfacesInfo,
   kMacAddress,
#ifdef PMDNG   // Applications -> PMD Configuration
   kPMDNumGeneralInfo,
   kPMDGeneralInfo,
   kPMDConfnumPorts,
   kPMDPortInfo,
   kPMDIpduInfo,
   kPMDConfnumOutGrps,
   kPMDOutGrpsDef,
   kPMDConfnumUsers,
   kPMDUsersMgm,
   kOutGrpsInfo,
   kIpduUnitConfig,
#endif
   kSSHexit,
   kSConf,
   //[AP][2007-07-30] hostname discovery
   kADEnable,
   kADTimeOut,
   kADProbeStrings,
   kADAnswerStrings,
#ifdef IPv6enable
   kEnableIPv4,
   kEnableIPv6,
   kIPv6method,
   kEthIp6,
   kEthPrefix6,
   kDHCPv6Opts,
#endif
   kOlhVersion,
   kSerialNumber,
} CycKey;


/* General type definitions */
/* ------------------------------------------------------------------------- */

// define LOG - tcp options
#define	klogTcpSequence	0x01
#define klogTcpOptions	0x02
#define	klogIpOptions	0x04

// define TCP flags :
#define kTCPFlagsFin	0x01
#define kTCPFlagsSyn	0x02
#define kTCPFlagsRST	0x04
#define kTCPFlagsPSH	0x08
#define kTCPFlagsAck	0x10
#define kTCPFlagsUrg	0x20

// Pcmcia type flags
#ifdef PCMCIA
#define kPcmciaFlagsModem	    0x01
#define kPcmciaFlagsIsdn	    0x02
#define kPcmciaFlagsGsm	      0x04
#define kPcmciaFlagsEthernet	0x08
#define kPcmciaFlagsFlash	    0x10
#define kPcmciaFlagsWireless	0x20
#define kPcmciaFlagsModemCallback  0x40
#define kPcmciaFlagsGsmSpeed        0x80
#define kPcmciaFlagsGsmAddinit      0x100
#endif

enum { kNone = 0}; // Used in many enums, below 
typedef struct
{
  CycCategory category;
  CycKey key;
  void* value;
  UInt32 collectionSize; // non-zero if array (incl. char_t-array/string)
  UInt32 pSize;
  UInt32 flag;
} CycParam;

/* PortBundle is used with kPortsSpec to specify a CycGet or CycSet stream
of CycParam objects. In other words, to set the baud rate to apply to ports
4, 6, and 7, kPortsSpec/kPortBundle with PortBundle { [4, 6, 7, 0, ...], 1 }
and follow with a single kPslave/kBaudRate.  To get the best "least common
denominator" (single) value for the same ports, set it up the same way but
use CycGet() rather than CycSet.  For baud rate and parity, PortBundle would
have a count value of 2, rather than 1: { [4, 6, 7, 0, ...], 1 } */
typedef struct
{
   SInt8 ports[kMaxPorts]; /* array of port id's; 1 is first, -1 means "all", 0 is end of spec */
   UInt16 count;    /* number of CycParam-struct objects that will follow for these ports */
} PortBundle;

typedef enum
{
  kSet = 1, /* also known as "Apply", for config params - permanently applies the change */
  kTry,     /* this is only valid for config params - temporarily applies the change */
  kCancel,
  kWrite,
  kTryForced,
  kSetForced,
  kCheckChange,
  kReloadPage,
} CycSetAction;

typedef enum
{
  /*kNone = 0*/
  kDelete = 1,
  kAdd,
  kEdit,
  kActionIgnore,
} Action; /* List actions */

typedef enum
{
  kAllP,
  kSpec,
} PhysAction; /* List Physical Configuration actions */

typedef enum
{
	kReadOnly,
	kReadWrite
} CycUserAccess;

typedef enum
{
	kUserNone,
	kAdminUser,
	kRegularUser,
	kBioUser
} CycUserType;

typedef enum
{
	kNo,
	kYes,
} CycYesNoOption;

typedef enum
{
	kOpen,
	kClosed,
} CycOpenClosedOption;

typedef enum
{
	kOff,
	kOn,
	kSinglePower, //used to indicate there is only one power supply
} CycOnOffOption;

/* Page-specific data */
typedef enum
{
   kCyclades
} CycDeviceClass;

typedef enum
{
	kDsInactive,
	kDsActive,
	kDsActivateRestore
} CycDhcpState;

typedef enum
{
	kRtDefault,
	kRtNetwork,
	kRtHost
} CycRouteType;

typedef enum
{
	kRgtGateway,
	kRgtInterface,
} CycRouteGoToOption;

typedef enum 
{
	kAoNone,
	kAoHosts,
	kAoDns,
	kAoHostsDns,
	kAoDnsHosts
} CycAccessOrder;

typedef enum
{
	kInActive,
	kActive
} CycBootConfWatchDogTimer;

typedef enum
{
	kFlash,
	kNetwork
} CycBootConfUnitBoot;

typedef enum
{
	kBootp = 1,
	kTftp,
	kBoth
} CycBootConfBootType;

typedef enum
{
	k4800,
	k9600,
	k19200,
	k38400,
	k57600, 
	k115200
} CycBootConfConsoleSpeed;

typedef enum
{
	kSkipFlash,
	kFullFlash
} CycBootConfFlashTest;

typedef enum
{
	kSkipRam,
	kQuick, 
	kFullRam
} CycBootConfRamTest;

typedef enum
{
	kAutoNegate,
	k100BaseTHalf,
	k10BaseTFull, 
	k100BaseTFull,
	k10BaseTHalf
} CycBootConfFastEthernet;

#ifdef IPSEC
typedef enum
{
	kEsp,
	kAh
} CycIpSecAuthProtocol;

typedef enum
{
	kRsa,
	kIpSecSecret
} CycIpSecAuthMethod;

typedef enum
{
	kIgnore=1,
	kIpSecAdd, 
	kStart
} CycIpSecBootAction;
#endif

typedef enum
{
	kLower,
	kUpper,
} CycSlotLocationOption;

// Types of authentication.
typedef enum
{
	kPatNone = 0, 			// None	
#ifdef OTPenable
	kPatOtp = 6,			// otp [RK]Jan/18/06
	kPatOtpLocal = 7,		// otplocal [RK]Jan/18/06
#endif
	kPatRadius = 4096,		// Radius
	kPatTacacs = 8192,		// TacacsPlus
#ifdef KRBenable
	kPatKerberos = 12288,		// Kerberos
#endif
	kPatLdap = 16384,		// Ldap
	kPatRemote = 16,		// Remote
	kPatLocal = 128,		// Local
	kPatRadiusLocal = 4097,		// Radius/Local
	kPatTacacsLocal = 8193,		// TacacsPlus/Local
	kPatLocalRadius = 32769,	// Local/TacacsPlus
	kPatLocalTacacs = 32770,	// RadiusDownLocal
	kPatRadiusDownLocal = 4098,	// TacacsPlusDownLocal
	kPatTacacsDownLocal = 8194,	// TacacsPlusDownLocal
#ifdef KRBenable
	kPatKerberosDownLocal = 12290,	// KerberosDownLocal 
#endif
	kPatLdapDownLocal = 16386,	// LdapDownLocal
#ifdef RADIUS2FACTORenable
	kPatLdapDownLocalandRadius = 16387,	// LdapDownLocal-Radius
#endif
#ifdef NISenable
	kPatNis = 20480,
	kPatNisLocal = 20481,
	kPatNisDownLocal = 20482,
	kPatLocalNis = 32773,
#endif
#ifdef SMBenable
	kPatSmb = 24576,
	kPatSmbDownLocal = 24577,
#endif
} CycPortAuthType;

typedef enum
{
	//kPfnLocalNone = -1,
	kPfnLocal0None,
	kPfnLocal1,
	kPfnLocal2,
	kPfnLocal3,
	kPfnLocal4,
	kPfnLocal5,
	kPfnLocal6,
	kPfnLocal7,
} CycPortFacilityNum;

typedef enum
{
	kPbr1200 = 1200, 
	kPbr2400 = 2400, 
	kPbr4800 = 4800,
	kPbr9600 = 9600,
	kPbr14400 = 14400,
	kPbr19200 = 19200,
	kPbr28800 = 28800,
	kPbr38400 = 38400,
	kPbr57600 = 57600,
	kPbr76800 = 76800,
	kPbr115200 = 115200,
	kPbr230400 = 230400,
} CycPortBaudRate;

typedef enum
{
	kPtcmText,
	kPtcmBinary
} CycPortTelnetClientMode;

/*typedef enum
{
  kPtXterm
} CycPortTranslation;*/

typedef enum
{
	kPb5 = 5,
	kPb6,
	kPb7,
	kPb8
} CycPortBits;

typedef enum
{
	kPsb1 = 1,
	kPsb2
} CycPortStopBits;

// Types of flow control.
typedef enum
{
	kPftNone,			// None					
	kPftHard,			// Hardware
	kPftSoft			// Software
} CycPortFlowType;

// Types of sniff session mode.
typedef enum
{
	kPstOut,			// Out
	kPstIn,				// In
	kPstInOut,		// In/Out
	kPstNo				// No
} CycPortSniffType;

typedef enum
{
	kPdmShowAllMenu,	// Show all options
	kPdmShowNoMenu,		// Show nothing
	kPdmShowNoMenuShowFile, // Show db file only
	kPdmShowPartMenu,	// Show w/o erase options
	kPdmShowNoMenuShowEraseFile, // Show and erase db file
} CycPortDbMenu;

// Types of data buffering mode.
typedef enum
{
	kPdbmCir,			// Circular
	kPdbmLin			// Linear
#ifdef DLSenable
	,kPdbmDls		// DLS [RK]May/23/06
	,kPdbmDlsCir
#endif
} CycPortDbModeType;

// Types of Multiple sniff sessions 
typedef enum
{
	kPmsNo,				 // No
	kPmsYesMenu,	 // Yes or All
	kPmsYesNoMenu, // ReadWrite 
	kPmsRo = 4		 // ReadOnly
} CycPortMultiSessType;

// Types of parity.
typedef enum
{
	kPptNone=1,	 		// None
	kPptOdd,			// Odd
	kPptEven			// Even
} CycPortParityType;

#ifdef PMDNG
typedef enum {
	Vendor_none = 0,
	Vendor_auto = 1,
	Vendor_cyclades,
	Vendor_spc,
	Vendor_servertech,
} IPDUvendor;
#endif

typedef enum
{
	kStColdStart,
	kStWarmStart, 
	kStLinkDown,
	kStLinkUp,
	kStAuthenticationFail,
	kStEgpNeighborLoss,
	kStEnterpriseSpecific,
	kStUserLogin,
	kStUserLogoff,
	kStAuthenticationSuccessful
} CycSnmpdTrapType;

typedef enum
{
  kNtEmail,
  kNtPager, 
  kNtSnmpTrap
} CycNotificationType;

typedef enum
{
	kLsLock,
	kLsUnlock 
} CycLockStatus;

typedef enum
{
	kPtModem,
	kPtEthernet, 
	kPtWireless,
	kPtIsdn,
	kPtGsm,
	kPtFlashCard
} CycPcmciaType;

typedef enum 
{   
	kAll     = 0,
	kIcmp    = 1,
	kTcp     = 6,
	kUdp     = 17,
#ifdef IPv6enable
	kIcmp6   = 58,
#endif
	kNumeric = 255
} CycIpTableChainProtocolType;

typedef enum 
{
	knotInverted,
	kInverted
} InvertedFlag;

typedef enum 
{
	kallPackets,
	konlyFragments,
	knoFragFirstFrag
} RuleFragments;

typedef enum 
{
	klogEmerg,
	klogAlert,
	klogCrit,
	klogErr,
	klogWarning,
	klogNotice,
	klogInfo,
	klogDebug
} LogLevelEnum;

// Type of modem cards
typedef enum
{
        kModem,
        kGsm,
        kCdma,
} CycModemCardType;

// General struct for ftp data 
typedef struct
{
	char_t ftpserver[kHostNameLength];	/* FTP server */
	char_t username[kUserNameLength];	/* User name to log in */
	char_t password[kPasswordLength];	/* User password */
	char_t directory[kDirPathLength];	/* Directory specification */ 
	char_t filename[kDirPathLength];	/* File name */
} ftpInfo;

typedef enum
{
        kSslVdefault,     // default (v2 & v3)
        kSslV3 = 3,               // v3
} CycSslOptionType;

#ifdef SECPROFILE
/*[LMT] Security enhancement*/
typedef enum
{
        kSecNone,
        kSecCustom,
        kSecOpen,
        kSecModerate,
        kSecSecured
} CycSecProfType;
#endif

#ifdef IPv6enable
typedef enum {
	//kNone    = 0x0000,
	kDns       = 0x0001,
	kDomain    = 0x0002,
	kDnsDomain = 0x0003 // kDns | kDomain
} CycDHCPv6Opts;
#endif

/* Parameter structures */
/* ------------------------------------------------------------------------- */
/*[LMT] Security enhancement*/
typedef struct
{
#ifdef SECPROFILE
        CycSecProfType    profile;
#endif
        CycYesNoOption    ssh2sport;
        CycYesNoOption    telnet2sport;
        CycYesNoOption    raw2sport;
        CycYesNoOption    auth2sport;
#ifdef BIDIRECT
        CycYesNoOption    bidirect;
#endif
        CycYesNoOption    directaccess;
} SecOptions;

/*[LMT] Security enhancement*/
typedef struct
{
        CycYesNoOption    http;
        CycYesNoOption    https;
#ifdef SECPROFILE
        UInt32            http_port;
        UInt32            https_port;
#endif
        CycSslOptionType  ssl_version;
        CycYesNoOption    redirect;
} HttpOptions;

/*[LMT] Security enhancement*/
typedef struct
{
        CycYesNoOption    sshv1;
        CycYesNoOption    sshv2;
#ifdef SECPROFILE
        UInt32            port;
#endif
        CycYesNoOption    rootAccess;
} SshdOptions;

typedef struct 
{
	char_t dns1[kDnsLength];
	char_t dns2[kDnsLength];
	char_t domainName[kDomainNameLength];
} DnsService;

#ifdef BONDING_FEATURE
typedef struct
{
        UInt32 enabled;
        UInt32 miimon;
        UInt32 updelay;
} BondingService;
#endif

typedef struct 
{
	CycOnOffOption multiIp;
	CycAccessOrder accessOrder;
} NameServiceAccess;

typedef struct 
{
	Action action;
	char_t userName[kUserNameLength];
	char_t passWord[kPasswordLength];
	char_t passWord2[kPasswordLength];
	CycUserType groupAdminReg;	
	CycUserType groupBio;	
	char_t comments[kCommentLength];
	char_t homeDir[kDirPathLength];
	char_t shell[kDirPathLength];
	char_t sysMsg[kInfoLength];
} Users;

typedef struct 
{
	char_t grpName[kGroupNameLength];
	char_t grpId[kIdLength];
} Groups;

typedef struct 
{
	char_t newGrpName[kGroupNameLength];
	char_t usersInGrp[kListLength];
} ConfGroups;

typedef struct 
{
	char_t hostIp[kIpLength];
	char_t hostName[kHostNameLength];
	char_t hostAlias[kHostNameLength];
} Hosts;

typedef struct 
{
	int month;
	int day;
	int year;
	int hour;
	int min;
	int sec;
	char_t tzacronym[kTzAcronymLength];	
	char_t sysMsg[kInfoLength];
} DateTime;

typedef struct
{
	int hour; //must be signed for GMToff and DSTsave
	unsigned int min;
} TzTime;

typedef struct
{
	unsigned int month;
	unsigned int week; //added for ACS 3.1.0: [1..5]
	unsigned int day;  //changed meaning for ACS 3.1.0: [1..7]
} TzDate;

typedef struct
{
	char_t zlabel[kTzLabelLength];
	char_t acronym[kTzAcronymLength];
	TzTime GMToff;
	int    DSTactive;
	char_t DSTacronym[kTzAcronymLength];
	TzTime DSTsave;
	TzDate DSTstartDate;
	TzTime DSTstartTime;
	TzDate DSTendDate;
	TzTime DSTendTime;
} TimezoneData;

typedef struct 
{
	char_t fsName[kDirPathLength];
	char_t blocks[kBlockLength];
	char_t used[kBlockLength];
	char_t avail[kBlockLength];
	char_t usage[kUsageLength];
	char_t mountedOn[kDirPathLength];
} RamDiskInfo;

typedef struct
{
	char_t ipAddr[kIpLength];
	CycBootConfWatchDogTimer wdTimer;
	CycBootConfUnitBoot uBoot;
	CycBootConfBootType bType;
	char_t bootFilename[kBootPathLength+1];
	char_t serverIpAddr[kIpLength+1];
	CycBootConfConsoleSpeed speed;
	CycBootConfFlashTest flashTest;
	CycBootConfRamTest ramTest;
	CycBootConfFastEthernet fastEthernet;
	int maxEvent;	
// Must fit default patch versions: imageN:vmlinux.UBoot.V_X.Y.Z-N-MMM_DD_AA
	char_t image1Filename[kFileNameLength+1];
	char_t image2Filename[kFileNameLength+1];
	char_t uBootHtml[512];
	char_t currentImage;
} BootConf;

#ifdef IPSEC
typedef struct
{
	Action action;
	char_t oldConnectionName[kFileNameLength];
	char_t connectionName[kFileNameLength];
	char_t leftId[kDomainNameLength];
	char_t leftIp[kIpLength];
	char_t leftSubnet[kSubnetLength];
	char_t leftNextHop[kIpLength];
	char_t leftRsaKey[kRsaKeyLength];
	char_t rightId[kDomainNameLength];
	char_t rightIp[kIpLength];
	char_t rightSubnet[kSubnetLength];
	char_t rightNextHop[kIpLength];
	char_t rightRsaKey[kRsaKeyLength];
	int bAction;
	int aProtocol;
	int aMethod;
	char_t secret[kPasswordLength];
} IpSecInfo;
#endif

typedef struct
{
	char_t ftpSite[kDomainNameLength];
	char_t userName[kUserNameLength];
	char_t passWd[kPasswordLength];
	char_t filePathName[kPathFilenameLength]; //[RK]Sep/13/04 - increase from 50 to 100
	CycYesNoOption checkSum;
	char_t sysMsg[kInfoLength];
} UpGradeInfo; 

typedef struct
{
	CycRouteType routeType;
	char_t destinationIp[kIpLength];	
	char_t destinationMask[kIpLength];	
	CycRouteGoToOption goTo;
	char_t gateWay[kIpLength];	
	char_t routeInterface[kInterfaceLength];	
	char_t metric[kMetricLength];	
	char_t sysMsg[kInfoLength];  // saves the error message to be displayed to user
} StRoutesInfo;

typedef struct
{
	char_t destinationIp[kIpLength];	
	char_t gateWay[kIpLength];	
	char_t genMask[kIpLength];	
	char_t flags[kRoutingTableMiscLength];
	char_t metric[kMetricLength];
	char_t mss[kRoutingTableMiscLength];
  char_t window[kRoutingTableMiscLength];
	char_t irtt[kRoutingTableMiscLength];
	char_t routeInterface[kInterfaceLength];	
} RoutingTableInfo;

// MAY REMOVE LATER
typedef struct
{
	char_t destinationIp[kIpLength];
	char_t destinationMask[kIpLength];	
	char_t hostIp[kIpLength];
	char_t gateWay[kIpLength];	
	char_t routeInterface[kInterfaceLength];	
	char_t sysMsg[kInfoLength];  // saves the error message to be displayed to user
} RoutingTableEntry;

typedef struct
{
	char_t user[kUserNameLength];
	char_t tty[kTtyLength];
	char_t fromIp[kDomainNameLength];
	char_t loginTime[kLoginTimeLength];
	char_t idle[kIdleTimeLength];
	char_t jcpu[kCpuTimeLength]; 
	char_t pcpu[kCpuTimeLength];
	char_t what[kWhatLength];
	char_t pid[kPidLength];
	char_t sysMsg[kInfoLength];
} ActiveSessionInfo;

typedef struct
{
	char_t linuxVersion[kVersionLength];
	char_t compilerVersion[kVersionLength];
	char_t dateOfVersionGeneration[kVersionLength];
	char_t firmwareVersion[kVersionLength];
} UnitVersionInfo;

typedef struct
{
	CycOnOffOption pwrSupply1;
	CycOnOffOption pwrSupply2;
} PwrSupplyInfo;

typedef struct
{
	int processor;
	char_t cpu[kCpuLength];
	char_t clock[kClockLength];
	char_t busClock[kClockLength];
	char_t revision[kRevisionLength];
	char_t bogomips[kBogomipsLength];
	char_t zeroPages[kZeroPagesLength];
} CpuInfo ;

typedef struct
{
	int numEntries;
	char_t name[kMaxSysMemInfo][kSysMemInfoNameLength];
	char_t value[kMaxSysMemInfo][kSysMemInfoValueLength];
} MemInfo;


typedef struct
{
	int numEntries;
	char_t name[64][kHostNameLength];   // alias of the interface
	char_t allif[64*(kHostNameLength+1)]; 
} InterfaceInfo;

typedef struct
{
	char_t communityName[kCommunityLength];
	char_t source[kCommunitySourceLength];
	char_t oid[kOidLength];
	CycUserAccess permission;
} CommunityInfo;

typedef struct
{
	char_t userName[kUserNameLength];
	char_t password[kPasswordLength];
	CycUserAccess permission;
	char_t oid[kOidLength];
} Snmpv3Info;

typedef struct
{
	char_t toAddress[kEmailListLength];
	char_t fromAddress[kEmailListLength];
	char_t subject[kEmailSubjectLength];
	char_t body[kEmailBodyLength];
	char_t serverIp[kServerLength]; //[RK]Sep/17/04 - accept host name
	int serverPort;
} EmailEntry;

typedef struct
{
	char_t phoneNum[kPhoneNumLength];
	char_t text[kPagerTextLength];
	char_t smsUserName[kUserNameLength];
	char_t smsServerIp[kServerLength]; //[RK]Sep/17/04 - accept host name
	int smsPort;
} Pager;

typedef struct
{
	CycSnmpdTrapType trapNumber;
	char_t community[kCommunityLength];
	char_t serverIp[kServerLength]; //[RK]Sep/17/04 - accept host name
	char_t oid[kOidLength];
	char_t body[kSnmpTrapBodyLength];
} SnmpTrap;

typedef char_t PortName[kHostNameLength];

typedef struct
{
  	Action action; 
	char_t remoteIp[kIpLength];
	int firstRemoteTcpPort;
	int localTcpPort;
	int protocol;
	int numOfPorts;
	int firstLocalPort;
	PortName name[kMaxPhysicalPorts];
} VirtualPortsInfo;

typedef struct
{
  	PhysAction action; 
	CycYesNoOption	enable;	
	int protocol;
	char_t alias[kHostNameLength];
#ifdef PMDNG
	IPDUvendor pmVendor;
#endif
#ifdef PMD
	int  pmsessions;
#endif
	int  stallowed;
} PhysGeneralInfo;

typedef struct
{
  	PhysAction action; 
	CycPortBaudRate	baudRate;
	CycPortBits 	dataSize;
	CycPortStopBits stopBits;
	CycPortParityType   parity;
	CycPortFlowType	flowControl;
	UInt32	dcdState;
} PhysSerialInfo;

typedef struct
{
  	PhysAction action; 
	char_t users[kListLength];
	CycPortAuthType authtype;
#ifdef BIDIRECT
	UInt32 lgtimeout;
	char_t termsh[kDirPathLength];
#endif
} PhysAccessInfo;

typedef struct
{
  	PhysAction 		action; 
	UInt32			destType;
	CycPortDbModeType	mode;
	UInt32			fileSize;
	char_t			nfsFile[kDirPathLength];
	CycYesNoOption		timeStamp;
	CycPortDbMenu		showMenu;
	CycYesNoOption		allTime;  //[RK]Feb/16/06
	//[AP][2007-07-30] hostname discovery
	CycYesNoOption		ADEnable;
	UInt32				ADTimeOut;
} PhysDataBufInfo;

typedef struct
{
  	PhysAction action; 
	CycYesNoOption	enable;
	CycPortFacilityNum facility;
	UInt32	bufferSize;
	CycYesNoOption	allTime;
} PhysSysBufInfo;

typedef struct
{
  	PhysAction action; 
	CycPortMultiSessType	multSess;
	CycPortSniffType	sniffMode;
	char_t			adminUsers[kListLength];
	char_t			hotKey[kHotKeyLength];
	CycOnOffOption		notifyUsers;
} PhysMusInfo;

#if defined(PMD) || defined(IPMI)
typedef struct
{
  	PhysAction action;
#ifdef PMD
	CycYesNoOption	enable;
	char_t		pmKey[kHotKeyLength];
	char_t		numOutlets;
	char_t 		outletList[kPmOutletsStringLength];
	UInt32		allUsers;
	char_t		userList[kPmPortUserListLength];
#endif
//[RK]Dec/22/04 - WebUI IPMI conf
#ifdef IPMI
	CycYesNoOption	enableIPMI;
	char_t		ipmiKey[kHotKeyLength];
	UInt32		ipmiDeviceId;
#endif
} PhysPowerMgmInfo;
#endif

typedef struct
{
  	PhysAction action; 
	char_t	portIpAlias[kIpLength];
#ifdef IPv6enable
	char_t	portIp6Alias[kIpLength];
#endif
#ifdef WINEMSenable
	CycYesNoOption winEms;
#endif
	CycYesNoOption authBio;
	UInt32	tcpKeepAlive;
	UInt32	tcpIdleTmo;
} PhysCASInfo;

typedef struct
{
  	PhysAction action; 
	char_t host[kServerLength];
	char_t	terminalType[kTermTypeLength];
} PhysTSInfo;

typedef struct
{
  	PhysAction action; 
	char_t	initChat[kInitChatLength];
	char_t	pppOpt[kAutoPPPLength]; //[RK]Feb/02/07
} PhysDialInInfo;

typedef struct
{
  	PhysAction action; 
	UInt32	socketPort;
	char_t	stty[kSttyCommandLength];
	char_t loginBanner[kInfoLength];
	UInt32	breakInterval;
	char_t breakSequence[kBreakSeqLength];
	char_t SSHexit[kHotKeyLength];
	char_t sconf[kHotKeyLength];
} PhysOtherInfo;

typedef struct
{
	Action action;
	PhysGeneralInfo general;
	PhysSerialInfo serial;
	PhysAccessInfo access;
	PhysDataBufInfo dataBuf;
	PhysSysBufInfo sysBuf;
	PhysMusInfo mus;
#if defined(PMD) || defined(IPMI)
	PhysPowerMgmInfo powerMgm;
#endif
	PhysCASInfo cas;
	PhysTSInfo ts;
	PhysDialInInfo dialIn;
	PhysOtherInfo other;
} PhysPortsInfo;

typedef union _Events
{     
	 EmailEntry email;     
	 Pager pager;     
	 SnmpTrap snmptrap;   
} Events;

typedef struct
{
  Action action; 
  CycNotificationType type;
  int triggerId; 
  char trigger[kTriggerLength]; 
  Events event;
} NotificationInfo;

typedef struct
{
  Action action; 
  int ServerId; 
  char SyslogServer[kServerLength]; 
} SyslogServerInfo;

typedef struct
{
  //Action action;
  //int ServerId;
  char SyslogBufServer[kServerLength];
	CycPortFacilityNum dbFacilityNum;
} SyslogBufServerInfo;

typedef struct {
	UInt32 CASlevel;
	UInt32 KVMlevel;
	UInt32 Buflevel;
	UInt32 Weblevel;
	UInt32 Syslevel;
	UInt32 CASfacility;
} FilterLevels;

#ifdef PCMCIA
typedef struct
{ 
	int both;
	char_t essid[kMaxPcmciaSlots][kNetworkNameLength];
	int channel[kMaxPcmciaSlots];
	CycYesNoOption encryption[kMaxPcmciaSlots];
	char_t key[kMaxPcmciaSlots][kListLength];
} PcmciaWirelessConfig;

typedef struct
{ 
	int both;
	int pinBoth;
	CycYesNoOption ppp[kMaxPcmciaSlots];
	CycYesNoOption callBack;
#ifdef OTPenable
	CycYesNoOption otp;	//[RK]Jan/20/06
#endif
	CycYesNoOption version2;	//[RK]Nov/10/06
	char_t callBackNum[kPhoneNumLength];
	char_t localIp[kMaxPcmciaSlots][kIpLength];
	char_t remoteIp[kMaxPcmciaSlots][kIpLength];
	char_t pin[kMaxPcmciaSlots][kPinLength];
        int speedBoth;
        int addinitBoth;
        int speed[kMaxPcmciaSlots];
        char_t addinit[kMaxPcmciaSlots][kAddinitLength];
        CycModemCardType modemCardtype;
} PcmciaModemGsmConfig;

typedef struct
{ 
	char_t localIp[kIpLength];
	char_t remoteIp[kIpLength];
	CycYesNoOption callBack;
	char_t callBackNum[kPhoneNumLength];
} PcmciaIsdnConfig;

typedef struct
{ 
	int both;
	CycYesNoOption enable[kMaxPcmciaSlots];
	CycYesNoOption dataBuf;
} PcmciaFlashConfig;

typedef struct
{ 
	char_t ipAddr[kIpLength];
	char_t netWkAddr[kIpLength];
	char_t netMask[kIpLength];
	char_t broadCast[kIpLength];
	char_t gateWay[kIpLength];
} PcmciaNetwkConfig;

typedef struct
{
	PcmciaNetwkConfig netWkConfig;
	PcmciaWirelessConfig wireless;
	PcmciaModemGsmConfig modemGsm;
	PcmciaIsdnConfig isdn;
	PcmciaFlashConfig flash;		 		
} PcmciaType;

typedef struct 
{
	Action action;
	CycSlotLocationOption slot;
	char_t status[kStatusLength];
	char_t identInfo[kInfoLength];
	char_t configInfo[kInfoLength];
	int pcmciaTypeFlag; 
	PcmciaType pcmciaType;
} PcmciaInfo;
#endif

#ifdef NISenable
typedef struct
{
	char domainName[kDomainNameLength];
	char serverIp[kDomainNameLength]; // can be IP or name
	//CycNisAuthType authType;
} NisInfo;
#endif

typedef struct
{
	char title[kTitleLength]; 
	char command[kCommandLength]; 
} MenuShOpt;

typedef struct
{
	char menuTitle[kTitleLength];
	int numOptions;
	MenuShOpt *optInfo;
} MenuShInfo;


// Table Filter or NAT - rules
// TARGET : ACCEPT, DROP, RETURN, LOG or REJECT
//          (chain==POSTROUTING) SNAT or MASQUERADE
//          (chain!=POSTROUTING) DNAT or REDIRECT
typedef struct {
	char_t packets[kIpTablesNumberLength];
	char_t bytes[kIpTablesNumberLength];
	char_t target[kTargetLength];
	char_t sourceIp[kIpLength];
	char_t sourceIpMask[kIpLength];
	InvertedFlag sourceIpInvert;
	char_t destinationIp[kIpLength];
	char_t destinationIpMask[kIpLength];
	InvertedFlag destinationIpInvert;
	CycIpTableChainProtocolType protocol; 
	int	protocolNumber;
	InvertedFlag protocolInvert;
	char_t 	inInterface[kInterfaceNameLength];
	InvertedFlag inInterfaceInvert;
	char_t 	outInterface[kInterfaceNameLength];
	InvertedFlag outInterfaceInvert;
	RuleFragments fragment;
	// extension to TCP and UDP protocol
	char_t 	SourcePort[kPortLength];
	char_t 	LastSourcePort[kPortLength];
	InvertedFlag SourcePortInvert;
	char_t 	DestinationPort[kPortLength];
	char_t 	LastDestinationPort[kPortLength];
	InvertedFlag DestinationPortInvert;
	// extension to TCP protocol
	int  TcpFlags;
	int  TcpFlagsSet;
	InvertedFlag TcpFlagsInvert;
	// extension to ICMP/ICMPv6 protocol
	char_t  ICMPType[kICMPTypeLength];  
	InvertedFlag ICMPTypeInvert;
	// extension to LOG target
	LogLevelEnum	LogLevel;
	char_t	LogPrefix[kLogPrefixLength];
	int	LogTcpOptions;
	// extension to REJECT target
	char_t RejectWith[kRejectWithLength];
	// extension to SNAT/DNAT target
	char_t NatToIp[kIpLength];
	char_t NatToLastIp[kIpLength];
	// extension to SNAT/DNAT/Masquerade/Redirect target and TCP protocol
	char_t NatToPort[kPortLength];
	char_t NatToLastPort[kPortLength];
} IpTablesRules;

// Action : edit - only target
//          add  - only name (user chain does not have target
//          delete - only user chain
// Policy : ACCEPT or DROP
// 
typedef struct {
	Action action;	
	char_t name[kChainNameLength];
	char_t policy[kTargetLength];
	char_t packets[kIpTablesNumberLength];
	char_t bytes[kIpTablesNumberLength];
	int numrules;
	IpTablesRules *rules;
} ChainInfo;

typedef struct 
{
	char ldapServer[kDomainNameLength];
	char ldapBase[kLdapDomainNameLength]; //[RK]Aug/12/05 - increased the length
	CycYesNoOption secure;
	char ldapUser[kLdapDomainNameLength]; //[LMT] increased the length (CyBTS Bug 4974)
	char ldapPassword[kPasswordLength];
	char ldapLoginAttribute[kDomainNameLength];
} LdapInfo;

#ifdef KRBenable
typedef struct 
{
	char krbServer[kDomainNameLength];
	char krbDomainName[kDomainNameLength];
//	CycYesNoOption secure;
} KrbInfo;
#endif

typedef struct {
   unsigned char *optionName; /* List of strings as shown in the file */
   int optionEnum;
} TranslateOptions;

typedef struct
{
	char authServer[kHostNameLength];
	char accServer[kHostNameLength];
	char auth2Server[kHostNameLength];
	char acc2Server[kHostNameLength];
	char secret[kPasswordLength];
/*
#if defined(KVM) || defined(ONS)
	char domain[kServerLength];
	UInt32 secure; 
#endif
*/
	UInt32	timeout;
	UInt32	retries;
	CycYesNoOption serviceType;
} RadiusInfo;

typedef struct
{
	char authServer[kHostNameLength];
	char accServer[kHostNameLength];
	char auth2Server[kHostNameLength];
	char acc2Server[kHostNameLength];
	char secret[kPasswordLength];
        CycYesNoOption raccess;
	UInt32	timeout;
	UInt32	retries;
} TacplusInfo;

#ifdef SMBenable
typedef struct
{
	char domain[kServerLength];
	char server[kServerLength];
	char server2[kServerLength];
} SmbInfo;
#endif

#ifdef PMD
/*-------------------------------------------------------------------------
 * Applications -> Power Management
 *------------------------------------------------------------------------*/
typedef enum   {
	kOutNameServer,     // 0 - name is alias of the server
	kOutNamePM,         // 1 - name is outlet name in PM configuration
} CycOutNameFlag;

typedef enum {
	kPmStateOFF,
	kPmStateON,
} CycPmStOnOff;
  
typedef enum {
	kOutStateUnlock,
	kOutStateLock,
} CycOutStUnLock;

typedef enum {
	kPmOff,
	kPmOn,
	kPmUnlock,
	kPmLock,
	kPmCycle,
} CycPmOutletCommand;

typedef enum {
	kPmUpgNotAvail,		// not available version = v.1.1.0
	kPmUpgAvail,           // available to Master Unit v1.1.0 - v.1.2.1
	kPmUpgAvailDaisyChain, // available to daisy-chain > v.1.2.2
} CycPmUpgrade;    

typedef enum {
	PmTime0s,
	PmTime15s,
	PmTime30s,
	PmTime45s,
	PmTime1min,
	PmTime1min15s,
	PmTime1min30s,
	PmTime1min45s,
	PmTime2min,
	PmTime3min,
	PmTime4min,
	PmTime5min,
	PmTime10min,
	PmTime15min,
	PmTime30min,
	PmTime1h,
} PmMinOnOfftime;

typedef enum {
	PmWuStateOff,
	PmWuStateOn,
	PmWuStateLast,
} PmWakeUpState;

typedef char_t  PmAlarmStr[kPmStringLength];

typedef struct {
	UInt32 number;         // outlet number
	CycOutNameFlag flagName;     
	char_t name[kHostNameLength];   // outlet name
	CycPmStOnOff  flgOnOff;
	CycOutStUnLock flgLock;
	double	 rebootDuration;               //APC [GY]07/Jun/09  Reboot Duration
	double	 postOnDelay;
	double	 powerDownInterval;            //APC [GY]07/Jun/09  PwrOff Delay
	double   powerUpInterval;              //APC [GY]07/Jun/09  PwrOn Delay
	PmMinOnOfftime minOnTime;
	PmMinOnOfftime minOffTime;
	PmWakeUpState  wakeupState;
	UInt32   cap;
	UInt32	elec_cap;
	UInt32   serialPort;                //Ipdu port
	void    *shmRef;
	//fields below are used for groups
	char_t   ipduID[kPMDipduIDLength];    //Ipdu name
	//[RK]May/2009 - Zeus
	double	 postOffDelay;	
	double	 current;
	double	power;
	double  threshold_hc;
	double 	threshold_lc;
	double	threshold_hw;
	double	threshold_lw;
	PmAlarmStr       alarm;
} PmOutletInfo;

typedef struct {
	Action   action;	
	char_t   userName[kUserNameLength];
	char_t   outlets[kPmOutletsStringLength];
} PmIpduUsers;
		   
typedef struct {
	UInt32   serialPort;
	UInt32   numUsers;
	PmIpduUsers user[kPmMaxUsers];
	// the field below isn't used by WebApi itself
	UInt32   changed;
} PmIpduUserMgm;


typedef struct {
	UInt32   serialPort;
	UInt32   ipduNumber;      // 0 - all
} PmIpduUnitNumber;

typedef struct {
	double    value;
	double    maxval;
	double    thre_hc;	// high critical
	double 	  thre_lc;	// low critical                    
	double    thre_hw;	// high warn
	double	  thre_lw;	// low warn
	int 	  changed;
	PmAlarmStr       alarm;
} PmCurrInfo;

typedef struct {
	double    value;
	double    maxval;
	int	  type; // 0 - measured 1-estimated
} PmElecInfo;

typedef struct {
	char_t    name[kSensorNameLength];
	PmCurrInfo current;
	PmElecInfo power;
	PmElecInfo voltage;
	PmElecInfo pf;
	UInt32    overcurrent;               
	int 	  changed;
	int 	  cap;		// capabilities of the sensor
} PmElecMonInfo;

#define PMENVMON_TEMP 	1	// temperature sensor
#define PMENVMON_HUM	2	// humidity
#define PMENVMON_AIRF	3	// air-flow
#define PMENVMON_SMOKE	4	// smoke
#define PMENVMON_DOOR	5	// door
#define PMENVMON_WATER	6	// water level
#define PMENVMON_MOTION 7	// motion

#define PMENVMON_CAP_MAXVAL 0x0001
#define PMEVNMON_CAP_THRE   0x0002

typedef struct {
	char_t    name[kSensorNameLength];
	int    	  type;		// type of env sensor
	double    value;
	double    maxval;
	double    thre_hc;	// high critical
	double 	  thre_lc;	// low critical                    
	double    thre_hw;	// high warn
	double	  thre_lw;	// low warn
	int 	  changed;
	int 	  cap;		// capabilities of the sensor
	char	  unit[10];    // unit: &deg;C or &deg;F or %% or afu
} PmEnvMonInfo;

#define IPDU_CHG_NAME        0x0001
#define IPDU_CHG_OCP         0x0002
#define IPDU_CHG_SYSLOG      0x0004
#define IPDU_CHG_BUZZER      0x0008
#define IPDU_CHG_POLLRATE    0x0010
#define IPDU_CHG_CYCLEINT    0x0020
#define IPDU_CHG_SEQINT      0x0040
#define IPDU_CHG_PDUTHRE     0x0080
#define IPDU_CHG_PHASETHRE   0x0100
#define IPDU_CHG_BANKTHRE    0x0200
#define IPDU_CHG_ENVMONTHRE  0x0400
#define IPDU_CHG_COLDSTART   0x0800
#define IPDU_CHG_CONFIGFILE  0x1000
#define IPDU_CHG_DISPLAY     0x2000
#define IPDU_CHG_DISPLAY_CYCLE 0x4000

typedef struct {
	char_t 		name[kPmNameLength];
	char_t 		vendor[kPmVendorLength];
	char_t 		model[kPmModelLength];          
	char_t 		version[kPmModelLength];       
	UInt32          serialPort; //number of the serial port to which this IPDU is connected
	UInt32          ipduNumber; //IPDU number in the chain
	UInt32          cap;
	UInt32          type;
	UInt32          num_phases;	
	UInt32          num_banks;	
	UInt32		num_inlets;
	UInt32		max_current; 
	CycPmUpgrade    upgradeFlag;
	UInt32          outlets;
	UInt32          overcurrent;
	UInt32          syslog;
	UInt32          buzzer;
	UInt32          cycle_int;
	UInt32          seq_int;
	UInt32		cold_start;
	UInt32          poll_rate;
	UInt32          max_flag;
	PmElecMonInfo	elecPduInfo;
	PmElecMonInfo   *firstPhaseInfo;
	PmElecMonInfo   *firstBankInfo;
	UInt32          envmon_sensors;
	PmEnvMonInfo    *firstEnvMonSensor;
	UInt32          changed;
	UInt32          totalLoad_max;
	UInt32          totalLoad_min;
	int             def_voltage;
	float           pwr_factor;
	void            *shmRef;
	char		display;
	char		display_cycle;
	PmOutletInfo    *firstOutlet;
	PmIpduUnitNumber query;
	PmAlarmStr       alarm;
        char_t username[kUserNameLength]; 
} PmIpduInfo;

typedef struct {
	UInt32         serialPort;     // number of the serial port
	UInt32         numIpdu;        // number of IPDUs connected
	// the fields below aren't used by WebApi itself
	PmOutletInfo  *outletInfo;
	PmIpduUserMgm *users;
	UInt32	       syslog;
	UInt32	       buzzer;
	UInt32	       overcurr;
	PmIpduInfo    *ipdu;
	UInt32         changed;
	SInt8		   gport[2];
	UInt32         lastNumIpdu;
} PmIpduPortInfo;

typedef struct {
        void  *ipduRef;                     // input data
        char_t username[kUserNameLength];   // input data
        int numOutlets;                     // number of outlets
        PmOutletInfo *outletInfo;           // information about the outlet
} PmIpduOutletsInfo;

typedef struct {
	char_t         username[kUserNameLength]; // input data
	char_t         name[kPMDGroupNameLength]; // input data
	UInt32         numOutlets;
	PmOutletInfo  *outletInfo;
} PmIpduOutGrpInfo;
#endif

typedef struct _LoadKey {
	int	   size;
	char_t filePathName[kDirPathLength];
	char_t PubKey[kPubKeyLength];
} LoadKey;

typedef struct _WebUserInfo {
	CycUserAccess	uAccess;
	CycUserType	uType;
	Sid		uPassK;	//[RK]Jul/29/05 - key to decryp the pass
	char_t		uName[32];
	char_t		uPass[kPasswordLength*2]; //2x because pass is stored in its hex val
	char_t		*uGroupName;
#ifdef PMD
	int		showIpdu;	// set for PM page for this user.
#endif
} WebUserInfo;

#ifdef IPMI //[RK]Dec/22/04 - IPMI config
typedef enum {
	kIpmiOff = 0,
	kIpmiOn,
	kIpmiStatus,
	kIpmiCycle,
} CycIPMIcmds;

typedef struct {
	Action  action;	
	UInt32	deviceId;
	char_t	alias[kUserNameLength];
	char_t  serverIP[kIpLength];
	char_t  authType[kUserNameLength];
	char_t  userAccessLevel[kUserNameLength];
	char_t	username[kUserNameLength];
	char_t	password[kPasswordLength];
} IPMIDeviceInfo;

#endif

#ifdef RDPenable
#define kMaxRDPServers	MAXRDPSERVERS
#endif	//RDPenable

/* Function declarations */
/* ------------------------------------------------------------------------- 
main.c:602: warning: implicit declaration of function `dmfSetParam'
------------------------------------------------------------------------------*/
int dmfSetParam(char_t*, char_t*, char_t*, int, int);


/* ------------------------------------------------------------------------- 
main.c:606: warning: implicit declaration of function `GetCycUserType' 
------------------------------------------------------------------------------*/
Result GetCycUserType(int);


/*----------------------------------------------------------------------------
warning: implicit declaration of function `CycSetCurrentAdmin'
------------------------------------------------------------------------------*/
void CycSetCurrentAdmin(char_t *user);

/*----------------------------------------------------------------------------
 Security Enhancement
------------------------------------------------------------------------------*/
//static int SetSecProfile(void); warning: `SetSecProfile' declared `static' but never defined

/*-----------------------------------------------------------------------------
CycInitApi
Performs the WebApi initialization
-----------------------------------------------------------------------------*/

Result CycInitApi(void);

/*-----------------------------------------------------------------------------
CycLogIn
Logs in <user> if <password> is correct.
<sid> is ths new session ID if login is successful, and should be non-zero
(unless login is unsuccessful).
-----------------------------------------------------------------------------*/
Result CycLogIn(const char_t* user, const char_t* password, Sid *sid, WebUserInfo *uInfo, int noAuth, int portauth);

/*-----------------------------------------------------------------------------
CycLogOut
Logs out user on session identified by <sid>.
-----------------------------------------------------------------------------*/
Result CycLogOut(Sid sid);

/*-----------------------------------------------------------------------------
CycCheckUser
Checks timeout and if the user is admin
-----------------------------------------------------------------------------*/
Result CycCheckUser(Sid *sid, WebUserInfo *wUser, int chkAdmin);

#ifdef PMD
// set the showIpdu in the WebUserInfo struct.
Result CycSetshowIpdu(Sid *sid, int showIpdu);
#endif

/*-----------------------------------------------------------------------------
CycBecomeAdminBoss
If another admin user is logged in and has the singleton config control (that
is, s/he is "boss"), then the admin user logged in as <sid> can take over
control via this call.
-----------------------------------------------------------------------------*/
Result CycBecomeAdminBoss(Sid sid);

/*-----------------------------------------------------------------------------
CycGet
Retrieves <collectionSize> values for <param>s.
<access> is set (from within) to indicate to the caller whether the data
returned in <param> is only readable or is read-writable.
<sid> identifies the session ID of the user attempting to access the data.
For config data, if <sid> does not correspond to the currently logged-in admin
who has config rights, kAnotherAdminIsOn should be returned and <param> should
be considered read-only.  Note that some parameters represent data that does
not require authentication; for these, a <sid> = 0 will work irrespective of
any login state.
<param> will be filled in-place (caller will be responsible for all allocation
and deallocation) with current values from the device.
-----------------------------------------------------------------------------*/
Result CycGet(CycParam *paramList, UInt32 collectionSize);

/*-----------------------------------------------------------------------------
CycSet
Commits <param>s to the device.
<sid> identifies the session ID of the user attempting to access the data.
For config data, if <sid> does not correspond to the currently logged-in admin
who has config rights, kAnotherAdminIsOn should be returned and no action will
be taken with respect to <param>.  Note that some parameters represent data
that does not require authentication; for these, a <sid> = 0 will work
irrespective of any login state.
-----------------------------------------------------------------------------*/
Result CycSet(CycParam *paramList, UInt32 collectionSize, CycSetAction action);
void CycReboot();
void CycEndApi();
void get_picture();
char *CycGetBoardName(void);
char *CycGetFamilyModel(void);
char *CycGetFamilyExt(void);
char *CycGetRDPconnect(void);
int  CycGetNewSid(Sid sid);
int  CycUserBio(Sid sid);
int  CycUserBioName(char * username);
#ifdef PCMCIA
Result CycInsertPcmciaCard(CycSlotLocationOption slot);
Result CycEjectPcmciaCard(CycSlotLocationOption slot);
#endif
Result CycKillActiveSession(ActiveSessionInfo *pActiveSess);
Result CycCancel(void);
void CycKickAdmin(void);
extern int UpGradeFirmware(UpGradeInfo *pUpgrade);

#ifdef PMD
/*--------------------------------------------------------------------------
 * Power Management functions 
 *--------------------------------------------------------------------------
 */
int cycPMDngSaveCmd(char_t *username, void *ipduRef);
int cycPMDngChangeState(char_t *username, void *outRef, CycPmOutletCommand command);
int cycPMDngConfOutName(char_t *username, void *outRef, char_t *outletName);
int cycPMDngConfIntervals(char_t *username, void *outRef, double interval, int type);
int cycPMDngConfWakeUpState(char_t *username, void *outRef, PmWakeUpState state);
int cycPMDngOutGrpChangeState(char_t *username, char_t *groupname, CycPmOutletCommand command);
int cycPMDngClearMaxDetected(char *username, void *ipduRef, int action);
int cycPMDngConfIpduName(char *username, PmIpduInfo *info);
char *cycPMDngGetIpduName(PmIpduInfo *info);
int cycPMDngConfIpduOnOffparams(char *username, PmIpduInfo *info);
int cycPMDngConfIpduParams(PmIpduPortInfo *port, int nports);
int cycPMDngConfIpduIntervals(char *username, PmIpduInfo *info);
int cycPMDngConfIpduPduThre(char *username, PmIpduInfo *info);
int cycPMDngConfIpduPhaseThre(char *username, PmIpduInfo *info);
int cycPMDngConfIpduBankThre(char *username, PmIpduInfo *info);
int cycPMDngConfIpduEnvMonThre(char *username, PmIpduInfo *info);
int cycPMDngUpgradeFirmware(char *username, UInt32 serialPort,UInt32 ipduNumber,char* filever, char *ipduver);
int cycPMDngcheckGroup_SPC(PmIpduOutGrpInfo * groupInfo);
int cycPMDngcheck_SPC(void *);
void * cycPMDngOutletPtr(char_t *username, char * ipduId, int outN);
char_t *cycPMDngGetOutName (void *outRef);
int cycPMDngConfThreOut(char_t *username, void * outRef,double thre_hc,double thre_lc, double thre_hw,double thre_lw);
#endif

#ifdef IPMI //[RK]Dec/22/04 - WebUI IPMI config
// return the status of the IPMI device
int cycIPMIcmd(int deviceId,CycIPMIcmds cmd, char *msg);
#endif

//[RK]Jun/14/04 - Access Privileges
// Regular User Security  Functions
int CycRegUserAuthLocal(char * username);
int CycRegUserPasswd(char * username, char * oldpwd, char * newpwd);

/*--------------------------------------------------------------------------
 * Other functions used by web and cli
 *--------------------------------------------------------------------------
 */

extern char *read_file_return_buf(char *fn);

extern WebUserEntry webUsers[MAX_WEB_USERS];

#ifdef RDPenable

#define kRDPGenLength 41
#define kRDPListLength (kRDPGenLength * kMaxRDPServers)
#define kRDPPortLength (kRDPGenLength)

typedef struct {
	UInt32    index;                    // index of server in kvm config
	char_t    name[kRDPGenLength];      // server name
	char_t    ipaddr[kRDPGenLength];    // server ipaddr
	char_t    srvport[kRDPGenLength];   // server port
	char_t    kvmport[kRDPGenLength];   // kvm port assigned
} RDPDevConfig; 

typedef struct {
	char_t *deviceTableHtml;
	char_t *deviceTable;
	char_t deviceMsgError[kInfoLength];
	char_t deviceCs[kRDPListLength];
	UInt32 deviceAction;
	UInt32 deviceSelected;
	RDPDevConfig confDevice;
	// webApi struct
	UInt32  changed;
	UInt32	numDevices;
	RDPDevConfig *deviceList;
	char_t rdppfile[kRDPPortLength];
	char_t rdpport[kRDPPortLength];
	char_t *RDPconnectKVMPorts;
} RDPDevice;

#endif	//RDPenable

#define ONLINE_HELP_FILE		"/etc/online_help"
#define kOnlineHelpPathLength	121
typedef struct
{
	char_t path[kOnlineHelpPathLength];
} OnlineHelpConf;

//------------------------------------------------------
//   new Power Management Daemon architecture
//------------------------------------------------------
#ifdef PMDNG

typedef struct {
	Action      action;//kDelete, kActionIgnore
	IPDUvendor  vendor;
	char_t      user[kUserNameLength];
	char_t      passwd[kPasswordLength];
} PMDGeneralInfo;

typedef enum {
	PmdAuto,
	Pmd1200 = 1200,
	Pmd2400 = 2400,
	Pmd4800 = 4800,
	Pmd9600 = 9600,
	Pmd14400 = 14400,
	Pmd19200 = 19200,
	Pmd28800 = 28800,
	Pmd38400 = 38400,
	Pmd57600 = 57600,
	Pmd76800 = 76800,
	Pmd115200 = 115200,
	Pmd230400 = 230400,
} PMDPortSpeed;

typedef struct {
	Action             action;//kDelete, kActionIgnore
	UInt32             serialPort;
	PMDPortSpeed       speed;
	CycPortBits        dataSize;
	CycPortParityType  parity;
	CycPortStopBits    stopBits;
	CycPortFlowType    flowCtrl;
	IPDUvendor         type;
	UInt32             stallowed;
} PMDPortInfo;

typedef struct {
	Action       action;//kDelete, kActionIgnore
	char_t       ipdu[kPMDipduIDLength];
	UInt32       pollRate;
	UInt32		 infeed;
	UInt32		 total_min;
	UInt32		 total_max;
	UInt32       def_voltage;
	float        pwr_factor;
	UInt32       num_phases;	//[RK]May/07/08
} PMDIpduInfo;

typedef struct {
	Action         action;//kDelete, kActionIgnore
	char_t         group[kPMDGroupNameLength];
	char_t		outletList[kPmOutletsStringLength];
} PMDOutGrpsDef;

typedef struct {
	Action         action;//kDelete, kActionIgnore
	char_t         user[kUserNameLength];
	char_t		outletList[kPmOutletsStringLength];
} PMDUsersMgm;

#endif

#endif // h_WebApi
