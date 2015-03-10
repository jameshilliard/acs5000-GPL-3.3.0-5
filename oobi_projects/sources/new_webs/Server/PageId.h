#ifndef h_PageId
#define h_PageId

typedef enum
{
   // Wizard
   kLoadDevice,
   kNetSettings,    // network/host settings too
   kCasProSettings,
   kAccessUsers,
   kAccessGroups,
   kDataBuffering,
   kSystemLog,      // network/syslog too
   // Applications
   kOutletMan,
   kIPDUinfo,
   kPMUserMan,
   kPMConfig,
   kPMSwUpgrade,
   kMultiOutletDev,
#ifdef PMDNG
   kIPDUOutletsManager,
   kIPDUOutletGroupsCtrl,
   kViewConfIPDUs,
   kIPDUSoftwareUpgrade,
   kPMDGeneral,
   kPMDOutletGroups,
   kPMDUsersManagement,
   kPMDPortsInfo,
#endif
#ifdef IPMI
   kPmIpmiPage,
#endif
   // Ports
   kPortPhysTable,
   kPortPhysConf,
   kVirtualPorts,
   kSysBufServer,
   // Administration
   kUnit,
   kBootInfo,
#ifdef PCMCIA
   kUnitPcmcia,
#endif
   kUnitRam,
   kTimeDate,
   kNotifications,
   kMenuShell,
   kOnlineHelpPage,
   // Network
   kSNMPdaemonSettings,
   kSnmpv1v2,
   kSnmpv3,
   kServices,
   kHostTablePage,
   kVPNTablePage,
   kStaticRoutes,
   kIpTablesPage,
#ifdef PCMCIA
   kAccessMethod,
#endif
   // Security
   kActiveSess,
   kSecProfile,   /* [LMT] Security Enhancements */
   // Others here...
   kPortsTableLoaded,
#if defined(KVM) || defined(ONS)
   kKVMD,
   kKVMDAccList,
   kKVMPStatus,
   kKVMPorts,
   kKVMDevices,
   kKVMuCupdate,
   kKVMuCreset,
   kKVMauxPort,
#endif
   ksecAuth,
   ksecRadius,
   ksecTacplus,
   ksecLdap,
   ksecKrb,
   ksecSmb,
   ksecNis,
#ifdef RDPenable
   kRDPServers,
#endif	//RDPenable
   kPortsPowerMgmt,
   //[AP][2007-08-02] hostname discovery
   kAutoDiscovery,
   kNumConfigPages,	//This must be the last one in the enum.
} ConfigPageId;

#endif
