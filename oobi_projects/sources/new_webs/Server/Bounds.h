#ifndef h_Bounds
#define h_Bounds

#define MAXVIRTUALPORTS    100
#ifdef ONS
#define kMaxPhysicalPorts 64
#else
#define kMaxPhysicalPorts 48
#endif
#define kMaxOutletsSP	129
#define kMaxIpdusSP	16
#define kMaxPorts 1024
#define kMaxUsersOnline 1
#define kMaxRegularUsers 60
#define kMaxSysMemInfo 26  //[RK]Feb/15/05
#define kMaxDigitalPorts 8
#define kMaxAnalogPorts 2
#define kMaxIdleTimeout 2147483648 /* ?!!! */
#define kMaxSshBreakInterval 2147483648 /* ?!!! */
#define kMaxChainOutlets 128
#define kMaxMenuShOptions 20
#define kMaxPcmciaSlots 2

#define kModelLength 31
#define kPictureLength 31
#define kSwVersionLength 31
#define kSensorNameLength 50
#define kSensorValuesLength 15
#define kUserNameLength 31
#define kPasswordLength 31
#define kConsoleBannerLength 50
#define kLdapDomainNameLength 70 //[RK]Aug/12/05 - increased the length
#define kSttyCommandLength 65
#define kServerFarmLength 31
#define kUserListLength 512
#define kInitChatLength 1024
#define kAutoPPPLength 1024
//[AP][2007-08-01] hostname discovery
#define kADStringLength 1024
#ifdef IPv6enable
#define kHostNameLength 53
#define kSubnetLength 51 /* ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128 */
#define kDomainNameLength 51
#define kDnsLength 52
#define kIpVirtualLength 53 /* [ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255]:9999 */
#define kIp6Length 51 /* ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128 */
#define kIpLength 51 /* ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255/128 */
#else
#define kHostNameLength 31
#define kSubnetLength 19 /* 255.255.255.255/24 */
#define kDomainNameLength 40
#define kDnsLength 20
#define kIpVirtualLength 21 /* 255.255.255.255:9999 */
#define kIpLength 16 /* 255.255.255.255 */
#endif

#define kPathFilenameLength	101 //[RK]Sep/13/04 - increase the parameter Path/Filename

#define kIntLength 4
#define kHotKeyLength 3

#define kGroupNameLength 21 
#define kPMDGroupNameLength 31
#define kPMDipduIDLength 31
#define kPMDpfLength 21
#define kIdLength 4 
#define kCommentLength 50
#define kDirPathLength 100
#define kSDoption 40
#define kBootPathLength 60 
#define kFileNameLength 40 
#define kRsaKeyLength 256 
#define kInterfaceLength 10
#define kSessionLength 80
#define kVersionLength 100  

#define kCpuLength 4
#define kClockLength 6
#define kRevisionLength 20
#define kBogomipsLength 6
#define kZeroPagesLength 60 

#define kBlockLength  16
#define kUsageLength 16

#define kStatusLength 512
#define kInfoLength 512
#define kIdentLength 150
#define kTargetPortLength 128

#define kPromptLength 15
#define kTermTypeLength 10
#define kListLength 2048
#define kBreakSeqLength 15
#define kTtyLength 21
#define kAutoAnswerStrLength 50

#define kSysMemInfoNameLength 15
#define kSysMemInfoValueLength 15 //[RK]Feb/15/05

#define kRoutingTableMiscLength 3

#define kLoginTimeLength 9
#define kIdleTimeLength 7
#define kCpuTimeLength 7
#define kWhatLength 50

#define kMetricLength 11 //must cover a decimal notation for 4 bytes (BGP) : 4294967296
#define kStRoutesEntriesListLength 300
#define kSNMPdaemonListLength 300
//[RK]Jan/05/06 - Fixed problem with 128 users
//#define kPMUserManTableEntriesLength 300
#define kPMUserManTableEntriesLength 2048
#define kPMOutGrpTableEntriesLength 2048
#define kPmPortUserListLength	2048
#define kPmUserListLength	2048	//[RK]Jan/30/06 - 128 users

#define kAnalogStatusLength 5

#define kMaxOutletServer	10 	//[RK]Jul/01/04 
#define kPmNameLength		30
#define kPmModelLength		40
#define kPmVendorLength		40
#define kPmStringLength		40
//#define kPmOutletsStringLength 60
#define kPmOutletsStringLength 128
//[RK]Jan/05/06 - Fixed problem with 128 users
#define kPmMaxUsers		128 
#define kPmMaxGroups	128 

#define kCommunityLength  31
#ifdef IPv6enable
#define kCommunitySourceLength 56
#else
#define kCommunitySourceLength 32
#endif
#define kOidLength 40 

#define kServerLength 100
#define kTriggerLength 100
#define kEmailListLength 300
#define kEmailSubjectLength 200
#define kEmailBodyLength 500
#define kPhoneNumLength 20
#define kPagerTextLength 160
#define kSnmpTrapBodyLength 250

#define kNetworkNameLength 30
#define kPinLength 10
#define kAddinitLength 15

#define kZeroLength 0

// IP Tables defines
#define kIpTablesNumberLength	20
#define kChainNameLength  20
#define kTargetLength  kChainNameLength
#define kProtocolLength 6
#define kInterfaceNameLength 20
#define kICMPTypeLength	40
#define kLogPrefixLength 30
#define kRejectWithLength 30
#define kPortLength  10
#define kChainTypeLength  5

#define kTitleLength 50
#define kCommandLength 50

#define kHtmlUserListLength 1024
#define kHtmlListLength 2048

#define kMacAddressLength 40
#define kDateLength 40
#define kUpTimeLength 10
#define kFtpCmdLength 16

#define kPidLength 6

#define kPubKeyLength 1024
#define kURLLength 100

#define kDevicePermLength 64

#define kTimezoneLength 10
#define kTzAcronymLength 7
#define kTzLabelLength 16

#endif
