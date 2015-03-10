/* (c) 2003 Cyclades. All Rights Reserved. */
/* $Id: Acs.c,v 1.1.1.1 2005/09/01 02:36:38 edson Exp $ */

/* This is the simulator implementation.
Note that this filename, "Acs", is a bit of a misnomer since this simulator
represents both ACS and TS devices.  However, the project name is ACS-Web and
various titles have stemmed from that. */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "Dmf.h" /* A&L Device Management Framework definitions */
#include "webApi.h" /* Cyclades ACS/TS API definition */
#include "SystemData.h" /* Data structures representing all system data */

/* Simulation of the real system (underlying) data, which is actually
(generally) stored in *nix files on the real system */
SystemData gData;

/* Logged-in users: */
Sid gCurrentUser[60]; // array of up to 60 (hokey limitation) logged-in test users
Sid gCurrentAdmin = 0;
Sid gWannabeAdmin = 0;
Sid gBumpedAdmin = 0;

/* Test username / password combos: */
const UInt8 kNumCurrentUsers = sizeof(gCurrentUser) / sizeof(gCurrentUser[0]);
const char_t* gAdmin[] = {"qw", "as", "zx"};
const UInt8 kNumAdmins = sizeof(gAdmin) / sizeof(gAdmin[0]);
const char_t* gAdminPassword[] = {"wq", "sa", "xz"}; // must be same size as gAdmin
const UInt8 kNumAdminPasswords = sizeof(gAdminPassword) / sizeof(gAdminPassword[0]);
const char_t* gUser[] = {"rt", "fg", "vb"};
const UInt8 kNumUsrs = sizeof(gUser) / sizeof(gUser[0]);
const char_t* gPassword[] = {"tr", "gf", "bv"}; // must be same size as gUser
const UInt8 kNumPasswords = sizeof(gPassword) / sizeof(gPassword[0]);

/* API Implementation */
/* ------------------------------------------------------------------------- */

Result CycLogIn(const char_t* user, const char_t* password, Sid* sid)
{
   UInt8 x = 0;
   CycUserType userType = kNone;
   Result result = kCycSuccess;

   /* Authorize: */
   for (x = 0; x < kNumUsrs; ++x)
   {
      if(0 == strcmp(user, gUser[x]) && 0 == strcmp(password, gPassword[x]))
      {
         userType = kRegularUser;
         break;
      }
   }
   if (kNone == userType)
   {
      for (x = 0; x < kNumAdmins; ++x)
      {
         if(0 == strcmp(user, gAdmin[x]) && 0 == strcmp(password, gAdminPassword[x]))
         {
            userType = kAdminUser;
            break;
         }
      }
   }

   /* Assign SID: */
   if (kNone == userType)
      /* None: */
      result = kInvalidLogin;
   else
   {
      srand((unsigned)time(NULL));
      *sid = (UInt16)(rand());
      /* Admin: */
      if (kAdminUser == userType)
      {
         if (0 != gCurrentAdmin)
         {
            result = kAnotherAdminIsOn;
            gWannabeAdmin = *sid;
         }
         else
            gCurrentAdmin = *sid;
      }
      /* Non-admin: */
      else
      {
         for (x = 0; x < kNumCurrentUsers; ++x)
         {
            if (0 == gCurrentUser[x])
            {
               gCurrentUser[x] = *sid;
               break;
            }
         }
         if (kNumCurrentUsers == x) /* max # logged-in; we ran out of spots */
            result = kSystemError;
      }
   }

   return result;
}

Result CycLogOut(Sid sid)
{
   UInt8 x = 0;
   if (sid == gCurrentAdmin)
      /* Admin: */
      gCurrentAdmin = 0;
   else
   {
      /* Non-admin: */
      for (x = 0; x < kNumCurrentUsers; ++x)
      {
         if (sid == gCurrentUser[x])
            gCurrentUser[x] = 0;
      }
   }
   return kCycSuccess;
}

Result CycGetUserType(Sid sid, CycUserType* userType)
{
   if (0 != sid && (sid == gCurrentAdmin || sid == gWannabeAdmin))
      /* Admin: */
      *userType = kAdminUser;
   else
   {
      /* Non-admin: */
      UInt8 x = 0;
      for (x = 0; x < kNumCurrentUsers; ++x)
      {
         if (0 != sid && sid == gCurrentUser[x])
            *userType = kRegularUser;
      }
      if (kNumCurrentUsers == x) /* never found! theif! */
         *userType = kNone;
   }
   return kNone;
}

Result CycBecomeAdminBoss(Sid sid)
{
   /* Assumption: sid really represents an admin!  Must trust caller for this,
   which is generally safe since the only sids anyone has are those we
   provided. */
   gBumpedAdmin = gCurrentAdmin;
   gCurrentAdmin = sid;
   return kCycSuccess;
}

Result CycGet(CycParam* param, UInt32 collectionSize, CycUserAccess* access, Sid sid)
{
   UInt32 x;
   Result result = kNone;
   CycUserType userType = kNone;

   /* Determine what kind of user this is (will help in setting 'access' and 'result': */
   if (0 != sid && sid == gBumpedAdmin)
      result = kAnotherAdminIsOn;
   else
      CycGetUserType(sid, &userType);

   // Walk the 'param' collection, assigning each values as requested:
   if (kCycSuccess == result)
   {
      *access = kReadWrite; // if this is reduced to kReadOnly at any time, it must stay there!
      if (1 > collectionSize)
         collectionSize = 1; // not technically a collection, just a single item
      for (x = 0; x < collectionSize && kCycSuccess == result; ++x)
      {
         switch (param[x].category)
         {
            /* Device */
            case kDevice: // assume param[x].collectionSize = 0 or 1 for following keys!
            *access = kReadOnly;
            switch (param[x].key)
            {
               case kClass:
               *((int*)(param[x].value)) = kCyclades;
               break;

               case kModel:
               #ifdef qTs110
                  *((int*)(param[x].value)) = kTs;
               #else
                  *((int*)(param[x].value)) = kAcs;
               #endif
               break;

               case kNumber:
               #ifdef qTs110
                  *((int*)(param[x].value)) = 110;
               #else
                  *((int*)(param[x].value)) = 48;
               #endif
               break;

               case kSwVersion:
               strncpy(param[x].value, "1.0.1a3 (Aug/28/03)", kSwVersionLength);
               break;

               default:
               result = kInvalidKeyForCategory;
            }
            break;

            /* HostnameSetting */
            case kHostnameSetting: // assume param[x].collectionSize = 0 or 1 for following keys!
            if (kAdminUser != userType)
               *access = kReadOnly;
            if (kHostName != param[x].key)
               result = kInvalidKeyForCategory;
            else
               strncpy(param[x].value, gData.netSettings.hostName, kHostNameLength);
            break;

            /* Pslave */
            case kPslave: // assume param[x].collectionSize = 0 or 1 for following keys!
            if (kNone == userType)
               result = kLoginRequired;
            else if (kAdminUser != userType)
               result = kCannotAccess;
            else
            {
               switch (param[x].key)
               {
                  case kSttyCommand:
                  strncpy((char_t*)(param[x].value), gData.casProSettings.sttyCommand, kIpLength);
                  break;

                  case kDhcpClient:
                  *((UInt8*)(param[x].value)) = gData.netSettings.dhcp;
                  break;

                  case kIpAddress:
                  strncpy(param[x].value, gData.netSettings.ipAddress1, kIpLength);
                  break;

                  case kNetMask:
                  strncpy(param[x].value, gData.netSettings.netMask1, kIpLength);
                  break;

                  case kProtocol:
                  *((int*)(param[x].value)) = gData.casProSettings.protocol;
                  break;

                  case kIdleTimeOut:
                  *((UInt32*)(param[x].value)) = gData.casProSettings.timeout;
                  break;

                  case kLineFeedSuppress:
                  *((UInt8*)(param[x].value)) = gData.casProSettings.lineFeedSuppression;
                  break;

                  case kBrkSeq:
                  strncpy(param[x].value, gData.casProSettings.breakSequence, kBreakSeqLength);
                  break;

                  case kBrkInterval:
                  *((UInt32*)(param[x].value)) = gData.casProSettings.breakInterval;
                  break;

                  case kServerFarm:
                  strncpy(param[x].value, gData.casProSettings.serverFarm, kServerFarmLength);
                  break;

                  case kNumEntries:
                  *((UInt32*)(param[x].value)) = 0;
                  break;

                  case kUsers:
                  //!!!
                  break;

                  case kNumConfGroups:
                  //!!!
                  *((UInt32*)(param[x].value)) = 2;
                  break;

                  case kGroups:
                  strncpy((((ConfGroups*)(param[x].value))[0]).oldGrpName,
                   //gData.access.groups[0!!!].oldGrpName, kGroupNameLength);
                   "old1", kGroupNameLength);
                  strncpy((((ConfGroups*)(param[x].value))[0]).newGrpName,
                   "new1", kGroupNameLength);
                  strncpy((((ConfGroups*)(param[x].value))[0]).usersInGrp,
                   "test1,test2,test3", kGroupNameLength);
                  strncpy((((ConfGroups*)(param[x].value))[1]).oldGrpName,
                   //gData.access.groups[0!!!].oldGrpName, kGroupNameLength);
                   "old2", kGroupNameLength);
                  strncpy((((ConfGroups*)(param[x].value))[1]).newGrpName,
                   "new2", kGroupNameLength);
                  strncpy((((ConfGroups*)(param[x].value))[1]).usersInGrp,
                   "test2,test5", kGroupNameLength);
                  break;

                  case kPortUsers:
                  strncpy(param[x].value, gData.authSecurity.users, kUserListLength);
                  break;

                  case kConfigGroup:
                  strncpy(param[x].value, gData.authSecurity.groups, kUserListLength);
                  break;

                  case kDbEnable:
                  *((UInt8*)(param[x].value)) = gData.dataBuffering.enable;
                  break;

                  case kDbDestination:
                  *((UInt8*)(param[x].value)) = gData.dataBuffering.destination;
                  break;

                  case kDbMode:
                  *((UInt8*)(param[x].value)) = gData.dataBuffering.mode;
                  break;

                  case kDbFileSize:
                  *((UInt32*)(param[x].value)) = gData.dataBuffering.fileSize;
                  break;

                  case kNfsFilePath:
                  strncpy(param[x].value, gData.dataBuffering.nfsPath, kDirPathLength);
                  break;

                  case kShowMenu:
                  *((UInt8*)(param[x].value)) = gData.dataBuffering.showMenu;
                  break;

                  case kConfFacilityNum:
                  *((UInt32*)(param[x].value)) = gData.systemLog.facilityNumber;
                  break;

                  case kTimeStamp:
                  *((UInt8*)(param[x].value)) = gData.systemLog.timestamp;
                  break;

                  case kSyslogBuffSess:
                  *((UInt8*)(param[x].value)) = gData.systemLog.bufferSysLog;
                  break;

                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* ResolvConf */
            case kResolvConf: // assume param[x].collectionSize = 0 or 1 for following keys!
            if (kNone == userType)
               result = kLoginRequired;
            else if (kAdminUser != userType)
               result = kCannotAccess;
            else
            {
               switch (param[x].key)
               {
                  case kDnsService:
                  strncpy(((DnsService*)(param[x].value))->domainName,
                   gData.netSettings.dnsService.domainName, kDomainNameLength);
                  strncpy(((DnsService*)(param[x].value))->dns1,
                   gData.netSettings.dnsService.dns1, kDnsLength);
                  strncpy(((DnsService*)(param[x].value))->dns2,
                   gData.netSettings.dnsService.dns2, kDnsLength);
                  break;

                  /* Invalid */
                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* StRoutes */
            case kStRoutes:
            if (kNone == userType)
               result = kLoginRequired;
            else if (kAdminUser != userType)
               result = kCannotAccess;
            else
            {
               switch (param[x].key)
               {
                  case kGateWay:
                  strncpy(param[x].value, gData.netSettings.gateway, kIpLength);
                  break;

                  /* Invalid */
                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* Syslog */
            case kSyslogNg:
            if (kAdminUser != userType)
               result = kCannotAccess;
            else
            {
               switch (param[x].key)
               {
                  case kSyslogServer:
                  strncpy((char_t*)(param[x].value), gData.systemLog.serversCs, kListLength);
                  break;

                  case kNumEntries:
                  *((UInt8*)(param[x].value)) = gData.systemLog.numServers;
                  break;

                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* Invalid */
            default:
            result = kInvalidCategory;
         }
      }
   }

   return result;
}

Result CycSet(CycParam* param, UInt32 collectionSize, CycSetAction action, Sid sid)
{
   UInt32 x;
   Result result = kCycSuccess;
   CycUserType userType = kNone;

   // Determine what kind of user this is (will help in setting 'access' and 'result':
   if (sid == gBumpedAdmin)
      result = kAnotherAdminIsOn;
   else
      CycGetUserType(sid, &userType);

   // Walk the 'param' collection, assigning each value as requested:
   if (kCycSuccess == result)
   {

      if (1 > collectionSize)
         collectionSize = 1; // not technically a collection, just a single item
      for (x = 0; x < collectionSize && kCycSuccess == result; ++x)
      {
         switch (param[x].category)
         {
            /* Device */
            case kDevice: // assume param[x].collectionSize = 0 or 1 for following keys!
            result = kCannotSetRoField;

            /* HostnameSetting */
            case kHostnameSetting: // assume param[x].collectionSize = 0 or 1 for following keys!
            if (kAdminUser != userType)
               result = kCannotSet;
            if (kHostName != param[x].key)
               result = kInvalidKeyForCategory;
            else
               strncpy(gData.netSettings.hostName, param[x].value, kHostNameLength);
            break;

            /* Pslave */
            case kPslave: // assume param[x].collectionSize = 0 or 1 for following keys!
            if (kAdminUser != userType)
               result = kCannotSet;
            else
            {
               switch (param[x].key)
               {
                  case kSttyCommand:
                  strncpy(gData.casProSettings.sttyCommand, (char_t*)(param[x].value), kIpLength);
                  break;

                  case kDhcpClient:
                  gData.netSettings.dhcp = *((UInt8*)(param[x].value));
                  break;

                  case kIpAddress:
                  strncpy(gData.netSettings.ipAddress1, (char_t*)(param[x].value), kIpLength);
                  break;

                  case kNetMask:
                  strncpy(gData.netSettings.netMask1, (char_t*)(param[x].value), kIpLength);
                  break;

                  case kProtocol:
                  gData.casProSettings.protocol = *((int*)(param[x].value));
                  break;

                  case kIdleTimeOut:
                  gData.casProSettings.timeout = *((UInt32*)(param[x].value));
                  break;

                  case kLineFeedSuppress:
                  gData.casProSettings.lineFeedSuppression = *((UInt8*)(param[x].value));
                  break;

                  case kBrkSeq:
                  strncpy(gData.casProSettings.breakSequence, param[x].value, kBreakSeqLength);
                  break;

                  case kBrkInterval:
                  gData.casProSettings.breakInterval = *((UInt32*)(param[x].value));
                  break;

                  case kServerFarm:
                  strncpy(gData.casProSettings.serverFarm, param[x].value, kServerFarmLength);
                  break;

                  case kNumEntries:
                  result = kCannotSetRoField;
                  break;

                  case kUsers:
                  //!!!strncpy(gData.access.users, param[x].value, kUserListLength);
                  break;

                  case kNumConfGroups:
                  result = kCannotSetRoField;
                  break;

                  case kGroups:
                  //!!!strncpy(gData.access.groups, param[x].value, kUserListLength);
                  break;

                  case kPortUsers:
                  strncpy(gData.authSecurity.users, param[x].value, kUserListLength);
                  break;

                  case kConfigGroup:
                  strncpy(gData.authSecurity.groups, param[x].value, kUserListLength);
                  break;

                  case kDbEnable:
                  gData.dataBuffering.enable = *((UInt8*)(param[x].value));
                  break;

                  case kDbDestination:
                  gData.dataBuffering.destination = *((UInt8*)(param[x].value));
                  break;

                  case kDbMode:
                  gData.dataBuffering.mode = *((UInt8*)(param[x].value));
                  break;

                  case kDbFileSize:
                  gData.dataBuffering.fileSize = *((UInt32*)(param[x].value));
                  break;

                  case kNfsFilePath:
                  strncpy(gData.dataBuffering.nfsPath, param[x].value, kDirPathLength);
                  break;

                  case kShowMenu:
                  gData.dataBuffering.showMenu = *((UInt8*)(param[x].value));
                  break;

                  case kConfFacilityNum:
                  gData.systemLog.facilityNumber = *((UInt32*)(param[x].value));
                  break;

                  case kTimeStamp:
                  gData.systemLog.timestamp = *((UInt8*)(param[x].value));
                  break;

                  case kSyslogBuffSess:
                  gData.systemLog.bufferSysLog = *((UInt8*)(param[x].value));
                  break;

                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* ResolvConf */
            case kResolvConf: // assume param[x].collectionSize = 0 or 1 for following keys!
            if (kAdminUser != userType)
               result = kCannotSet;
            else
            {
               switch (param[x].key)
               {
                  case kDnsService:
                  strncpy(gData.netSettings.dnsService.domainName,
                   ((DnsService*)(param[x].value))->domainName, kDomainNameLength);
                  strncpy(gData.netSettings.dnsService.dns1, 
                   ((DnsService*)(param[x].value))->dns1, kDnsLength);
                  strncpy(gData.netSettings.dnsService.dns2, 
                   ((DnsService*)(param[x].value))->dns2, kDnsLength);
                  break;

                  /* Invalid */
                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* StRoutes */
            case kStRoutes:
            if (kAdminUser != userType)
               result = kCannotSet;
            else
            {
               switch (param[x].key)
               {
                  case kGateWay:
                  strncpy(gData.netSettings.gateway, (char_t*)(param[x].value), kIpLength);
                  break;

                  /* Invalid */
                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* Syslog */
            case kSyslogNg:
            if (kAdminUser != userType)
               result = kCannotSet;
            else
            {
               switch (param[x].key)
               {
                  case kSyslogServer:
                  strncpy(gData.systemLog.serversCs, (char_t*)(param[x].value), kListLength);
                  break;

                  case kNumEntries:
                  gData.systemLog.numServers = *((UInt8*)(param[x].value));
                  break;

                  default:
                  result = kInvalidKeyForCategory;
               }
            }
            break;

            /* Invalid */
            default:
            result = kInvalidCategory;
         }
      }
   }

   return result;
}
