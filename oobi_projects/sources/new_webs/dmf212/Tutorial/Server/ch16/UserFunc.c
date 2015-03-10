/* (c) 2001 Art & Logic, Inc. All Rights Rerverved. */
/* $Id: UserFunc.c,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $ */


#include "webs.h"

#include "Access.h"
#include "GetSet.h"
#include "Localize.h"
#include "Locals.h"
#include "UserFunc.h"

//!!!!!!!!!!!!!!!!!!!!!!
typedef unsigned int UInt32;
typedef unsigned char UInt8;
typedef UInt8 bool;
#define false 0
#define true 1
typedef UInt32 IpAddress;

enum { kNone = 0 };
typedef enum
{
   //kNone = 0, // use this to indicate successful call
   kSystemError = 1, // generic, last-resort error (try not to use)
   kAnotherAdminIsOn, // double-use: return this upon login attempt, if another
                      // admin is on, or in response to any CycSet(<configParam>)
                      // call if another admin stole the current admin's
                      // configuration control.
   kTimeout, // user's "session" has timed out
   kCannotSetRoField, // cannot call CycSet() on a read-only field
   kCannotGetWoField, // cannot call CycGet() on a write-only field
   kCannotCancel, // cannot cancel a CycSet(...,kSet) call, only a kTry call
   kInvalidKeyForCategory, // some keys don't belong to some categories
   // other errors here
} Error;

// This enum is intended to identify the categories of data that need
// management.  Here they're shown roughly along the lines of the
// web pages, but I recommend that we define them based on the files
// which are modified.  E.g., one category may correspond to /etc/hosts
// and another to snmpd.conf.  Below, you'll find the enum for CycKey,
// which identifies keys for values that go in a given file.  
typedef enum
{
   // Global:
   kDevice, // R/O
   // The following two are for the likes of HP Open View or Tivoli:
   kElementManagementSystems, // R/O
   kNetworkManagementSystems, // R/O

   // Terminal:
   kPowerManagement,
   kProcess,
   kPortsSpec,
   kPortsAuthentication,
   kPortsDataBuffering,
   kPortsSystemLog,
   //...
   kCasProfile,
   kTsProfile,
   kDialInProfile,
   //...

   // Information (all R/O):
   kInterfaceStats,
   kIpStats,
   kIcmpStats,
   //...

   // Notification:
   kEmail,
   kPager,
   //...

   // Network:
   kHostSettings, // aka, "Network Settings" in Wizard mode
   kAccessMethod,
   kVpnSettings,
   kSnmpDaemonSettings,
   kNetworkServices,
   //...

   // Access:
   kUsersAndGroups,
   kAuthentication,
   //...

   // Other:
   //...

   // etc.
} CycCategory;

// This enum identifies keys for values that may be set or exposed through the
// web interface.  A key may be meaningful in the context of one or more
// categories (above), but meaningless in the context of other categories (see
// kInvalidKeyForCategory). Shown here are a few keys for demonstration purposes.
// A tree of valid category-key groupings should be drawn up for documentation
// purposes, at least, and valid pairings should be enforced by the server code
// (see kInvalidKeyForCategory)
typedef enum
{
   kClass,
   kProduct,
   kNumber,
   kDhcpIsEnabled,
   kHostName,
   kIpAddress,
   kDomainName,
   kDnsServer,
   kGatewayIp,
   kPorts,
   kType,
   kFileSize,
   // ...
} CycKey;

// This enum identifies generic types for values that are passed through the API.
typedef enum
{
   //kNone = 0,
   tUInt8 = 1, tUInt32, // etc.
   tIpAddress, // corresponds to IpAddress type, currently typedef'd to UInt32
   tString, // char_t array, actually
   tBool,
   // etc.
   tCycPortAuthenticationType, // See CycPortAuthenticationType, below
} Type;

// Values of this struct are what is actually passed through the API.
typedef struct
{
   CycCategory category;
   CycKey key;
   Type type;
   void* value;
   UInt32 collectionSize; // non-zero if array (incl. char-array/string)
   /* Note that a collectionSize of 0 or 1 means the same thing: only
   one value can be found at the pointer.*/
} CycParam;

typedef enum
{
   kSet, // also known as "Apply", for config params - permanently applies the change
   kTry, // this is only valid for config params - temporarily applies the change
} CycSetAction;

typedef enum
{
   kLocal,
   kLdap,
   kRadius,
   kTacacs,
   kSecureId,
   kNis
} CycPortAuthenticationType;

Error CycGet(CycParam* param, UInt32 collectionSize)
{
   return kNone;
}

Error CycSet(CycParam* param, UInt32 collectionSize, CycSetAction action)
{
   return kNone;
}

Error CycCancel() // only effective on a recently-called CycSet(..., kTry)
{
   return kNone;
}

void Foo()
{
   char hostName[31];
   IpAddress ipAddress;
   CycParam parameters[] = 
   {
      {
         kHostSettings,
         kHostName,
         tString,
         hostName, // an already-allocated char-array of n char_t's
         strlen(hostName)
      },
      {
         kHostSettings,
         kIpAddress,
         tIpAddress,
         &ipAddress, // an already-allocated IpAddress-type variable
         0
      }
      // etc. - there may be others; every param array is potentially unique
   };
   Error e = kNone;
   e = CycGet(parameters, 2);
   // now, parameters[0].value may be something like "myhost.com"
   strcpy(parameters[0].value, "myChangedHost.com");
   e = CycSet(parameters, 2, kSet); // though typically <parameters> would be reconstructed, and may only contain kCycHostName this time
}

void Bar()
{
   UInt8 portsToModify[] = {1, 2, 5, 7, 8}; // this will, of course, be dynamically built
   CycPortAuthenticationType paType = kLocal; // likewise, this is merely an example
   IpAddress ipAddress = 0xFFFFFFFF; // 255.255.255.255; again, merely an example
   UInt32 fileSize = 1024; // data buffering file size; again, merely an example
   CycParam parameters[] = 
   {
      {
         kPortsSpec,
         kPorts,
         tUInt8,
         portsToModify, // this tells you that the next params of kPorts* category
         5              // will be modifications for ports 1, 2, 5, 7, 8
      },
      {
         kPortsAuthentication,
         kType,
         tCycPortAuthenticationType,
         &paType, // this sets ports 1, 2, 5, 7, and 8 to authenticate via "Local" method
         0
      },
      {
         kPortsAuthentication,
         kIpAddress,
         tIpAddress, // this is, for instance, the "Server IP Address"
         &ipAddress, // for Radius authentication (on ports 1, 2, 5, 7, and 8)
         0
      },
      {
         kPortsDataBuffering, // here's a different category / different web page
         kFileSize,
         tUInt32,
         &fileSize, // this sets the data buffering "file size" to 1024 bytes;
         0          // again, on ports 1, 2, 5, 7, and 8
      },
      // etc. - you may even get another kPortsSpec inline here telling you that
      // the next batch of params of kPorts* category will be modifications for
      // another set of ports, such as 41, 42, and 43, for example.
   };
   Error e = kNone;
   e = CycSet(parameters, 4, kSet);
}

#if 0
// Device:
typedef enum
{
   //kNone = 0,
   kAlterPath = 1,
   kCyclades,
   kCyclom
} CycDeviceClass;
typedef enum
{
   //kNone = 0,
   kCycAcs = 1, kCycPm, kCycSm, // AlterPath products
   kCycNl, kCycPc, kCycPr, kCycTs, kCycZ, // Cyclades products
   kCycY // Cyclom products
} CycDeviceProduct;

void Bar()
{
   CycDeviceClass deviceClass;
   CycDeviceProduct deviceProduct;
   UInt8 deviceNumber;
   CycParam parameters[] = 
   {
      {
         kCycDeviceClass,
         kUInt8,
         &deviceClass,
         0
      },
      {
         kCycDeviceProduct,
         kUInt8,
         &deviceProduct,
         0
      },
      {
         kCycDeviceNumber,
         kUInt8,
         &deviceNumber,
         0
      },
   };
   Error e = kNone;
   e = CycGet(parameters);
   // so for TS110, we'd have kCyclades, kTs, and 110.  For AlterPath ACS we'd have kAlterPath, kAcs, 0

   e = CycSet(parameters); // this is illegal, from your point of view, and you should return an error!
}

//!!!!!!!!!!!!!!!

   kNumUsers, // R/O
   kUsers, // R/O
   kUser,
   kNewUser, // W/O
   kChangedUser, // W/O
   kDeletedUser, // W/O
   kNumGroups, // R/O
   kGroups, // R/O
   kNewGroup, // W/O
   kChangedGroup, // W/O
   kDeletedGroup, // W/O

// API functions:

typedef UInt8 Sid; // session ID

/*////-------------------------------------------------------------------------
CycGet
Retrieves <data> for <key>.
<sid> identifies the session ID of the user attempting to access the data.
For the single-user paradigm, if <sid> does not correspond to the currently
logged-in user, kAnotherUserIsOn should be returned and <data> should be
considered invalid.  Note that some <key> values represent data that
does not require authentication, and thus a <sid> = 0 will work irrespective
of any login state.
<data> should be cast according to <key>, and should be filled in-place with
current values from the device, or, if present, uncommitted values which
were recently set by a call to CycSet().
/*////-------------------------------------------------------------------------
Error CycGet(CycKey key, void* data, Sid sid)
{
   return kNone;
}

/*////-------------------------------------------------------------------------
CycSet
Sets <data> for <key>.
<sid> identifies the session ID of the user attempting to access the data.
For the single-user paradigm, if <sid> does not correspond to the currently
logged-in user, kAnotherUserIsOn should be returned and no action should be
taken with respect to <data>.  Note that some <key> values represent data that
does not require authentication, and thus a <sid> = 0 will work irrespective
of any login state.
<data> should be cast according to <key>, and cached for a future Commit().
/*////-------------------------------------------------------------------------
Error CycSet(CycKey key, void* data, Sid sid)
{
   return kNone;
}

/*////-------------------------------------------------------------------------
CycCommit
Commits any modified data to the server and, if <restart> is true, sends
a HUP to the process to restart.  It is understood that this function will
never return if <restart> is true and the engine is running on the same
process as the web interface.
/*////-------------------------------------------------------------------------
Error CycCommit(bool restart)
{
   return kNone;
}

/*////-------------------------------------------------------------------------
CycLogIn
Attempts to log in <user>, if <password> is correct and no other user is
currently logged in.  If another user is logged in and <force> is false,
kAnotherUserIsOn is returned and <user> is not logged in.  If <force> is true,
<user> is logged in and assumes exclusive control of the device (thus forcing
the previously logged-in user offline).
<sid> is ths new session ID if login is successful, and should be non-zero
(unless login is unsuccessful).
/*////-------------------------------------------------------------------------
Error CycLogIn(const char* user, const char* password, Sid* sid, bool force)
{
   return kNone;
}

/*////-------------------------------------------------------------------------
CycLogOut
Logs out user on session identified by <sid>.
/*////-------------------------------------------------------------------------
Error CycLogOut(Sid sid)
{
   return kNone;
}

// Device:
typedef enum
{
   //kNone = 0,
   kAlterPath = 1,
   kCyclades,
   kCyclom
} DeviceClass;
typedef enum
{
   //kNone = 0,
   kCycAcs = 1, kCycPm, kCycSm, // AlterPath products
   kCycNl, kCycPc, kCycPr, kCycTs, kCycZ, // Cyclades products
   kCycY // Cyclom products
} DeviceProduct;
typedef struct
{
   DeviceClass klass;
   DeviceProduct product;
   UInt8 version;
} Device;


// Host (network) settings:
#define kServerLength 255
typedef struct
{
   char hostName[kServerLength];
   IpAddress ipAddress;
   char domainName[kServerLength];
   char dnsServer[kServerLength];
   IpAddress gatewayIp;
   IpAddress networkMask;
} HostSettings;


// Access method:
typedef enum
{
   kAmModem,
   kAmIsdn,
   kAmGsm,
   kAmEthernet,
   kAmFlashCard
} AccessMethod;

#define kUsernameLength 31
#define kPasswordLength 31
typedef UInt8 GroupId;
typedef struct
{
   char userName[kUsernameLength];
   char password[kPasswordLength];
   GroupId groupId;
   bool enabled;
} User;
typedef UInt8 NumUsers;
typedef struct
{
   NumUsers numUsers;
   User* users;
} UserList;

void Test()
{
   Error e = kNone;
   Device device;
   Sid sid = 0; // 0 will work for params like kDevice
   e = CycGet(kDevice, &device, sid);
    // Do for TS110, we'd have kCyclades, kTs, and 110. 
    // For AlterPath ACS we'd have kAlterPath, kAcs, 0.
   e = CycSet(kDevice, &device, sid);
    // This is illegal, from your point of view, and you should return an error!

   {
      e = CycLogIn("mcaine", "huckleberry", &sid, false);
   }

   // assuming e is kNone...
   {
      HostSettings hostSettings;
      e = CycGet(kHostSettings, &hostSettings, sid);
      strcpy(hostSettings.hostName, "newHostName.com");
      e = CycSet(kHostSettings, &hostSettings, sid);
   }

   {
      AccessMethod am = kAmEthernet;
      e = CycSet(kAccessMethod, &am, sid);
   }

   {
      NumUsers n = 0;
      e = CycGet(kNumUsers, &n, sid);
      {
         User* users = balloc(B_L, sizeof(User) * n);
          // Note that we're responsible for allocating (and deallocating) everything!
         e = CycGet(kUsers, users, sid);
         e = CycSet(kUsers, users, sid);
          // This is an error - R/O value, you should return kCannotSetRoField
         bfree(B_L, users);
      }
   }

   // I haven't hammered it out yet, but the "Terminal" ("ports") page with up
   // to 48 ports will look somewhat similar to the above userlist mechanism.
   // One difference is that it will be R/W rather than R/O (we'll be able to set
   // groups of ports all at once).

   CycLogOut(sid);
}

#endif

// Cyclades parameter keys:
/*typedef enum
{
   // Device:
   kCycDeviceClass,
   kCycDeviceProduct,
   kCycDeviceNumber,
   // Network settings:
   kCycDhcpIsEnabled,
   kCycHostName,
   kCycIpAddress,
   kCycDomainName,
   kCycDnsServer,
   kCycGatewayIp,
   // ...
   kCycAccessMethod
   // ... this list will be rather long
} CycKey;

typedef enum
{
   //kNone = 0,
   kUInt8 = 1, kUInt32, // etc.
   kIpAddress,
   kStr, // char_t array, actually
   kBool,
   // etc.
} Type;

typedef struct
{
   CycKey key;
   Type type;
   void* value;
   UInt32 collectionSize; // non-zero if array (incl. char-array/string)
} CycParam;

/*Error CycGet(CycParam* param)
{
   return kNone;
}

Error CycSet(CycParam* param)
{
   return kNone;
}
*//*
void Foo()
{
   char hostName[31];
   IpAddress ipAddress;
   CycParam parameters[] = 
   {
      {
         kCycHostName,
         kStr,
         hostName, // an already-allocated char-array of n char_t's
         strlen(hostName)
      },
      {
         kCycIpAddress,
         kIpAddress,
         &ipAddress, // an IpAddress-type variable already defined
         0
      }
      // etc. - there may be others; every param array is potentially unique
   };
   Error e = kNone;
   e = CycGet(parameters);
   // now, parameters[0].value may be something like "myhost.com"
   strcpy(parameters[0].value, "myChangedHost.com");
   e = CycSet(parameters); // though typically <parameters> would be reconstructed, and may only contain kCycHostName this time
}

// Device:
typedef enum
{
   //kNone = 0,
   kAlterPath = 1,
   kCyclades,
   kCyclom
} CycDeviceClass;
typedef enum
{
   //kNone = 0,
   kCycAcs = 1, kCycPm, kCycSm, // AlterPath products
   kCycNl, kCycPc, kCycPr, kCycTs, kCycZ, // Cyclades products
   kCycY // Cyclom products
} CycDeviceProduct;

void Bar()
{
   CycDeviceClass deviceClass;
   CycDeviceProduct deviceProduct;
   UInt8 deviceNumber;
   CycParam parameters[] = 
   {
      {
         kCycDeviceClass,
         kUInt8,
         &deviceClass,
         0
      },
      {
         kCycDeviceProduct,
         kUInt8,
         &deviceProduct,
         0
      },
      {
         kCycDeviceNumber,
         kUInt8,
         &deviceNumber,
         0
      },
   };
   Error e = kNone;
   e = CycGet(parameters);
   // so for TS110, we'd have kCyclades, kTs, and 110.  For AlterPath ACS we'd have kAlterPath, kAcs, 0

   e = CycSet(parameters); // this is illegal, from your point of view, and you should return an error!
}

*/
//!!!!!!!!!!!!!!!!!!!!!!

/*
 * In the dmfUserPostInit() function  for each chapter, we set a server
 * variable named '_chapter_' that ASP code may use to make sure that it
 * always uses the correct ASP files to correspond with the version of the
 * server executable that's actually running.
 */
#define  kChapterKey T("_chapter_")

/*
 * In each chapter's UserFunc.c file, set this to the current chapter number
 * (as a string: 'chXX')
 */
#define  kCurrentChapter   T("ch16")


static int sUptimeFormats = -1;


static void InitSystemInfo(void);

/* CH05 -- initialize the simulator's 'gValues' structure.*/
static void InitValues(void);


void dmfUserPreInit(void)
{
   /*
    * TODO: Things that you may want to change here for your project:
    * + location of web root directory
    * + name of default file
    * + server port
    */

   /*
    * Initialize the DMF's parameter tree.
    */
   dmfInitParams(parameterList, parameterCount);

   /*
    * CH03: Since this tutorial is not connected to a real device, we need to
    * initialize our SystemInfo data structure. In a real implementation, the
    * SystemInfo structure would be filled by executing the associated Query
    * function, which would retrieve the required data from the device and
    * store it in the gSystemInfo structure.
    */
   InitSystemInfo();

   /*
    * CH05: Initialize the simulator 'gValues' structure, since our tutorial
    * is not connected to a real device.
    */
   InitValues();

      
}

void dmfUserPostInit(void)
{

   char_t* newRootWeb = NULL;

#ifdef USER_MANAGEMENT_SUPPORT
   /* CH06: Open the user database...*/
   dmfInitAccess(T("users.txt"));


   /*
    * TODO: Create user groups that must be present on the server.
    * NOTE that we use the dmfAddGroup() function -- this allows us to
    * associate an integer access level with each group as we create it.
    */
   /* CH06: Add the three access control groups that our application will use:
    *    - users (lowest level of access, access level = 1)
    *    - tech  (medium access, access level = 5)
    *    - admin (full access, access level = 10)
    */

   dmfAddGroup(NULL, T("users"), PRIV_READ | PRIV_WRITE, 
    AM_BASIC, 1, 1, 1);
   
   dmfAddGroup(NULL, T("tech"), PRIV_READ | PRIV_WRITE, 
    AM_BASIC, 1, 1, 5);
   
   dmfAddGroup(NULL, T("admin"), PRIV_READ | PRIV_WRITE | PRIV_ADMIN, 
    AM_BASIC, 1, 1, 10);



   
   /*
    * TODO: Create a default admin account on the server. You may want to
    * choose a different name for the builtin admin account, and will
    * definitely want to change the password.
    */

   /* CH06: Add a predefined user with 'admin' access. This user will be able
    * to add regular user accounts to the system.
    */
   dmfAddUser(NULL, T("adminUser"), T("changeThisPassword"), 
    T("admin"), 1, 1);

#endif
   /*
    * TODO: Configure access permissions for paths on the device as needed.
    */
   
   /* CH06: Our server has four areas that use varying levels of access
    * control:
    *    - the root '/' directory (and any other directory not otherwise 
    *    controlled are accessible to any client.
    *    - the /user/ directory and its subdirectories are accessible by any
    *    logged-in user from the group 'users' or a group with a higher level
    *    of access control.
    *    - the /tech/ directory and its subdirectories are accessible by any
    *    logged-in user from the group 'tech' or 'admin'.
    *    - the /admin/ directory and its subdirectories are accessible by any
    *    logged-in user belonging to the 'admin' group.
    */
   umAddAccessLimit(T("/user/"), AM_BASIC, 0, T("users"));
   umAddAccessLimit(T("/tech/"), AM_BASIC, 0, T("tech"));
   umAddAccessLimit(T("/admin/"), AM_BASIC, 0, T("admin"));
   umAddAccessLimit(T("/goform/Dmf"), AM_BASIC, 0, T("users"));

   

   /*
    * set the ephemeral server variable that says what chapter we are...
    */
   dmfSetLocal(kChapterKey, NULL, kCurrentChapter, 0);

   fmtAlloc(&newRootWeb, BUF_MAX, "%s/%s", websGetDefaultDir(), kCurrentChapter);
   websSetDefaultDir(newRootWeb);
   bfreeSafe(B_L, newRootWeb);

   /*
    * CH12: Register the new 'textInput' ejScript function with the framework:
    */
   websAspDefine(T("textInput"), (AspFunc) TextInput);
   
}

void dmfUserPreClose(void)
{
   /*
    * by default, this function does nothing.
    */
   if (-1 != sUptimeFormats)
   {
      symClose(sUptimeFormats);
      sUptimeFormats = -1;
   }
         
}

void dmfUserPostClose(void)
{
   /*
    * by default, this function does nothing.
    */
}




/*
 * PROJECT-SPECIFIC CODE FOLLOWS....
 *
 */

/*
 * CH03: Adding system.info.* parameters...
 */

SystemInfo gSystemInfo;

#define kDefaultName T("DMF Tutorial")
#define kDefaultDescription T("Imaginary Pseudo Device")

static int sStartTime = 0;

static void InitSystemInfo(void)
{
   if (0 == sStartTime)
   {
      sStartTime = time(NULL);
   }
   gstrcpy(gSystemInfo.name, kDefaultName);
   gstrcpy(gSystemInfo.description, kDefaultDescription);
   gSystemInfo.uptime = 0;

}

extern int SystemInfoQuery(Param* param, int request)
{
   /*
    * CH03: In a real implementation, the Query function would retrieve
    * current data from the device using some sort of inter-process
    * communication. Since we're just simulating here, we leave the 'name' and
    * 'description' fields alone, and update the 'uptime' field.
    */
   gSystemInfo.uptime = time(NULL) - sStartTime;

   return kDmfSuccess;
}

extern int SystemInfoCommit(Param* param, int request)
{
   /*
    * CH04: This commit function will be called after any of the members of
    * the gSystemInfo structure have succesfully updated by a user. In a real
    * system, the new values would be sent to your device using the
    * appropriate IPC mechanism.
    *
    * In this simulation, we do nothing.
    */

   return kDmfSuccess;
}


/* CH05: */
Values   gValues;
const int kZeroIndex = 0;
#define kDefaultUptimeFormat  T("%d days %d hours %d minutes %d seconds")
static void InitValues(void)
{
   int i = 0;

   gValues.endIndex = kMaxValueIndex;

   for (i = 0; i < kMaxValueIndex; ++i)
   {
      /* 
       * create random values scaled between 0..120
       */
      gValues.value[i] = (int) (((rand() * 1.0) / RAND_MAX) * 120);
      gValues.lowerAlarm[i] = 0;
      gValues.upperAlarm[i] = 100;
      fmtStatic((gValues.label[i]), kMaxValueLabelLength, "Label %02d", i);      
   }


   /*
    * CH16:
    * Modified the custom get() function for the uptime parameter so that it
    * can use language-specific format strings. If the currently selected
    * language does not have an entry in this table, the DMf will display the
    * English string, otherwise it will use the format string found in this
    * table.
    */
   sUptimeFormats = symOpen(5);
   if (-1 != sUptimeFormats)
   {
      symEnter(sUptimeFormats, T("en"), 
       valueString(kDefaultUptimeFormat, VALUE_ALLOCATE), 0);
      symEnter(sUptimeFormats, T("es"), 
       valueString(T("%d días %d horas %d minutos %d segundos"), VALUE_ALLOCATE), 0);

      /* !!! Add any additional language/format mappings here...*/
   }

}


static int RandomWalker(int prevVal)
{
   /*
    * we create a random value with a triangle distribution centered around
    * zero ranging between -5..+5, and return the previous value + our new 
    * random noise value.
    */
   const int kRange = 3;

   /* 
    * create two random numbers between 0 and 1
    */
   double val1 = (rand() * 1.0) / RAND_MAX;
   double val2 = (rand() * 1.0) / RAND_MAX;
   /*
    * average the numbers to create a triangle distribution, then slide it down so the
    * distribution centers around zero.
    */
   double offset = ((val1 + val2) * 0.5) - 0.5;

   /*
    * convert to an integer 'noise' value scaled between -kRange and +kRange
    */
   int noise = (int) (kRange * 2 * offset);
    
   return prevVal + noise;

}

extern int ValuesQuery(Param* param, int request)
{
   /*
    * CH05: In a real implementation, the Query function would retrieve
    * current data from the device using some sort of inter-process
    * communication.  Here, we create fresh random data for each of the 
    * 'value' points in the structure.
    */
   int i = 0;
   for (i = 0; i < kMaxValueIndex; ++i)
   {
      gValues.value[i] = RandomWalker(gValues.value[i]);
   }

   return kDmfSuccess;
}

/*
 * Commit function to send current state of the 'gValues' structure to the
 * device.
 */
extern int ValuesCommit(Param* param, int request)
{

   return kDmfSuccess;
}


/*
 * CH11: Custom get() function.
 */

#define kSecondsPerMinute  60
#define kSecondsPerHour    (60 * kSecondsPerMinute)
#define kSecondsPerDay     (24 * kSecondsPerHour)


/*
 * Macro that takes a two-letter language code and generates an integer
 * from the characters. Make sure that the incoming string is lowercase before
 * calling this...
 */

#define mLangToInt(code) (((code[0] - 'a') << 16) | (code[1] - 'a'))
#define mLangInt(code) (mLangToInt(code))


extern int GetUptime(const char_t* key, const char_t* index, 
 char_t** value, void* rawData, struct ParamTAG* param, 
 int request)
{
   int retval = kDmfDataLowerRange;
   int days = 0;
   int hours = 0;
   int minutes = 0;
   int seconds = 0;
   int rawTime = *((int*)(rawData));
   char_t* format = kDefaultUptimeFormat;
   sym_t* symbol = NULL;

   /*
    * CH16:
    * Modified the custom get() function for the uptime parameter so that it
    * can use language-specific format strings. If the currently selected
    * language does not have an entry in this table, the DMf will display the
    * English string, otherwise it will use the format string found in this
    * table.
    */
   if (-1 != sUptimeFormats)
   {
      symbol = symLookup(sUptimeFormats, (char_t*) dmfGetLanguage());
      if (symbol != NULL)
      {
         format = symbol->content.value.string;
      }
   }
   

   if (rawTime >= 0)
   {
      retval = kDmfSuccess;

      days = rawTime / kSecondsPerDay;
      rawTime %= kSecondsPerDay;

      hours = rawTime / kSecondsPerHour;
      rawTime %= kSecondsPerHour;

      minutes = rawTime / kSecondsPerMinute;
      seconds = rawTime % kSecondsPerMinute;

      fmtAlloc(value, BUF_MAX, format, days, hours, minutes, seconds);
   }
   
   return retval;
}




/*
 * CH12: Adding a new ejScript function
 */
extern int TextInput(int eid, webs_t wp, int argc, char_t **argv)
{
   char_t* parameter = NULL;
   char_t* value = NULL;
   char_t* index = NULL;
   int size = 0;
   int retval = kDmfError;
   int argCount = 0;
   int accessLevel = dmfUserAccessLevel(wp->userName);

   /* 
    * the ejArgs() function pulls the individual arguments out of the 'argv'
    * array using scanf()-like syntax, returning the number of arguments that
    * were actually provided.
    */
   argCount = ejArgs(argc, argv, T("%d %s %s"), &size, &parameter, &index);
   /*
    * we need at least two parameters.
    */
   if (argCount >= 2)
   {
      /*
       * before we attempt to write anything to the browser, let's make sure
       * that the parameter is valid, and that the current user has adequate
       * access privileges...
       */
      retval = dmfGetParam(parameter, index, &value, accessLevel);
      if (kDmfSuccess == retval)
      {
         /* 
          * we successfully retrieved the parameter value, so let's write the
          * input control into the user's browser...
          */
         websWrite(wp, T("<input type=\"text\" size=\"%d\" name=\"%s"), size, parameter);
         if (3 == argCount)
         {
            /* an index was provided, so we must append it to the parameter
             * name to make the form setting code know how to handle the
             * parameter....*/
            websWrite(wp, T("_%s"), index);
         }
         /*
          * write the current value of this parameter into the input control
          */
         websWrite(wp, T("\" value=\"%s\">"), value);
      }
      /*
       * The dmfGetParam() call above returned allocated memory, so we must
       * free it before we leave the function.
       */
      bfreeSafe(B_L, value);
   }
   else
   {
      /* there weren't enough arguments made to the function call.
       */
      retval = kDmfArgCount;
   }
   /*
    * set the error code in the DMF, and return zero (success) to the ejScript
    * interpreter.
    */
   dmfSetErrorCode(eid, retval);
   return 0;
}
