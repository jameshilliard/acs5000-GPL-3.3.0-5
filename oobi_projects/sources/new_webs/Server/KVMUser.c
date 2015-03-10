#include "webs.h"

#include "Access.h"
#include "Error.h"
#include "GetSet.h"
#include "Locals.h"
#include "Localize.h"
#include "UserFunc.h"
#include "KVMFunc.h"
#include "PageId.h"

#include "ResultCodes.h" /* define the Result enum */

extern ReqData gReq;
extern SystemData gCache;
extern SystemData gCacheSave;

extern WebUserInfo wUser;
extern int gLedState;
extern bool gCacheLoaded[];

extern int checkCancelRequest(int request, int page);
extern void ResetSidEtc(int request);
extern void SetLedChanges(int request, int val);

typedef Result (*LoadFunc)(void);
extern Result LoadFirst(LoadFunc loadFunc, ConfigPageId page, void *orig, void *save, int len);
extern Result GetCycParamValues(CycParam* values, UInt32 collectionSize);
extern void includeNbsp(unsigned char *dest, unsigned char *buf, int maxsize);
extern Result LoadKVMDevices(void);
extern void RegisterParamChange(int kPorts, void* updatedValue, CycCategory category, CycKey key, UInt32 collectionSize);
extern char_t kSel1Key[]; 

static void SetAccListHtml(KVMUserPermConfig *puser, char_t **bufptr);


#define itoa(sid, temp, len) snprintf(temp, len, "%d", sid)
int KVMUserCommit(Param* param, int request, char_t* urlQuery)
{
	char_t *s, *t;
	int user;
//printf("entry name = %s\n", gCache.kvmuser.entryName);
	s = gCache.kvmuser.entryName;
	if (!strcmp(s, "-1")) {
		return kCycSuccess;
	}
	if (!strncmp(s, "user=", 5)) {
		gCache.kvmuser.currUserGroup = 0;
		s += 5;

	} else if (!strncmp(gCache.kvmuser.entryName, "group=", 6)) {
		gCache.kvmuser.currUserGroup = 1;
		s += 6;
	} 
	if (gCache.kvmuser.currUserGroup) {
		user = gatoi(s);
//printf("group index %d name %s\n", user, gCache.access.groups[user - 1].newGrpName);
		strncpy(gCache.kvmuser.currName, 
			gCache.access.groups[user - 1].newGrpName, 
			kUserNameLength); 
	} else {
		if ((t = strchr(s, '!')) != NULL) {
			*t = 0;
		}
		if(*s == '0'){ // generic user
			strcpy(gCache.kvmuser.currName, "Generic User");
		}else{
			user = gatoi(s);
//printf("user index %d name %s\n", user, gCache.access.users[user - 1].userName);
			strncpy(gCache.kvmuser.currName,
				gCache.access.users[user - 1].userName,
					kUserNameLength);
		}
	}
//printf("KVMUserCommit - end\n");
	gCache.kvmuser.from = 1;
	return kCycSuccess;
}

static int copyUserEntry(KVMUserPermConfig *to,  KVMUserPermConfig *from)
{
	if (to->perms)
		bfreeSafe(B_L, to->perms);
	to->perms = NULL;
	to->defperm = from->defperm;
	to->nperms = from->nperms;
	to->npermsalloc= from->npermsalloc;
	if (to->npermsalloc) {
		to->perms = balloc(B_L, to->npermsalloc * sizeof(KVMPortPerm));
		if (! to->perms) {
			printf("copyUserEntry - Allocation Error\n");
			to->nperms = to->npermsalloc =  0;
			return 0;
		}
		memset(to->perms, 0, to->npermsalloc * sizeof(KVMPortPerm));
		if (to->nperms)
			memcpy(to->perms, from->perms, to->nperms * sizeof(KVMPortPerm));
	}
	return 1;
}

static int cmpUserEntry(KVMUserPermConfig *to,  KVMUserPermConfig *from)
{
	int i;
	if (to->defperm != from->defperm) return 1;
	if (to->nperms != from->nperms) return 1;
	for (i = 0; i < to->nperms; i ++) {
		if (strcmp(to->perms[i].device, from->perms[i].device)) {
			return 1;
		}
		if (to->perms[i].port !=  from->perms[i].port) {
			return 1;
		}
		if (to->perms[i].value !=  from->perms[i].value) {
			return 1;
		}
	}
	return 0;
}

static void delAccListEntry(int i)
{
	int j;
	for (j = i + 1; j < gCache.kvmuser.useracc.nacclists; j ++) {
		gCache.kvmuser.useracc.acclistList[j - 1] =
			gCache.kvmuser.useracc.acclistList[j];
	}
	gCache.kvmuser.useracc.nacclists--;
}

static int addAccListEntry()
{
	int i, idx;
	/* Add an entry in the list. Seek first an empty one.  */
	for (idx = 0; idx < kMaxAccLists; idx ++) {
		for (i = 0; i < gCache.kvmuser.useracc.nacclists; i++) {
			if (idx + 1 == gCache.kvmuser.useracc.acclistList[i]) {
				break;
			}
		}
		if (i == gCache.kvmuser.useracc.nacclists) {
			break;
		}
	}
	if (idx >= kMaxAccLists) {
		return 0;
	}
	/* Reallocates the userperms entry if the space allocated is
	not enough. */
	if (idx + 1 > gCache.kvmuser.nalloc) {
		int size = sizeof(KVMUserPermConfig) * (idx + 1);
		 void *buf = balloc(B_L, size);
		 if (! buf) {
			return 0;
		 }
		 memset(buf, 0, size);
		 if (gCache.kvmuser.nalloc) {
			memcpy(buf, gCache.kvmuser.useracc.userperms,
				sizeof(KVMUserPermConfig) * gCache.kvmuser.nalloc);
			bfreeSafe(B_L, gCache.kvmuser.useracc.userperms);
		 }
		 gCache.kvmuser.useracc.userperms = buf;
		 gCache.kvmuser.nalloc = idx + 1;
	}
	gCache.kvmuser.useracc.acclistList[gCache.kvmuser.useracc.nacclists] = idx + 1;
		
	copyUserEntry(&gCache.kvmuser.useracc.userperms[idx],
		&gCache.kvmuser.userperm);
	strncpy(gCache.kvmuser.useracc.userperms[idx].name,
		gCache.kvmuser.currName, kUserNameLength);
	gCache.kvmuser.useracc.userperms[idx].usergroup =
		gCache.kvmuser.currUserGroup;
	gCache.kvmuser.useracc.nacclists ++;
	return 1;
}

static void handleUserEntry()
{
	int i, idx;

	if(strcmp("Generic User", gCache.kvmuser.currName) == 0){

		if (cmpUserEntry(&gCache.kvmuser.useracc.defuserperm,
				&gCache.kvmuser.userperm)) {
			gLedState = 1;
		}

		copyUserEntry(&gCache.kvmuser.useracc.defuserperm,
			&gCache.kvmuser.userperm);
	}else{
		for (i = 0; i < gCache.kvmuser.useracc.nacclists; i++) {
			idx = gCache.kvmuser.useracc.acclistList[i];
			if (idx == 0) {
				continue;
			} else {
				idx --;
			}
			if (strcmp(gCache.kvmuser.useracc.userperms[idx].name, gCache.kvmuser.currName)) {
				continue;
			}
			if (gCache.kvmuser.useracc.userperms[idx].usergroup !=
					gCache.kvmuser.currUserGroup) {
				continue;
			}
//printf("AccListCommit - username found\n");
			/* The username was found. Check for the status*/
			if (gCache.kvmuser.currDefault) {
				/* Remove the user */
				gLedState = 1;
				delAccListEntry(i);
			} else {
				if (cmpUserEntry(&gCache.kvmuser.useracc.userperms[idx],
					&gCache.kvmuser.userperm)) {
					gLedState = 1;
				}
				copyUserEntry(&gCache.kvmuser.useracc.userperms[idx],
					&gCache.kvmuser.userperm);
			}
			break;
		}
	}

	if (i == gCache.kvmuser.useracc.nacclists && ! gCache.kvmuser.currDefault) {
		gLedState = 1;
		addAccListEntry();
	}
}

int KVMAccListCommit(Param* param, int request, char_t* urlQuery)
{
	Result result = kCycSuccess;

//printf("AccListCommit - start\n");
    if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

//printf("AccListCommit - pass2\n");

	// if cancel was chosen just discard the changes
	if (checkCancelRequest(request,kKVMDAccList)) return mAppError(result);

//printf("AccListCommit - user = %s\n", gCache.kvmuser.currName);
//printf("AccListCommit - defuser = %d\n", gCache.kvmuser.currDefault);
	if (! gCache.kvmuser.cancel) {
		handleUserEntry();
	}
	
	if (gReq.action == kNone) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}
	result = expertCommit(param, request, urlQuery);
	return result;
}

static unsigned char validperms[4] = {0, PERM_READ, PERM_READ | PERM_WRITE, 
   PERM_READ | PERM_WRITE | PERM_POWER};
static unsigned char *permname[4] = {"none", "ro", "rw", "full"};
int KVMDevicePermissionsCommit(Param* param, int request, char_t* urlQuery)
{
	Result result = kCycSuccess;
	char_t *permbuf[4] = {gCache.kvmuser.ports_none, 
		gCache.kvmuser.ports_ro, gCache.kvmuser.ports_rw, 
		gCache.kvmuser.ports_full};
	int i, j, max, min;
	unsigned char *s, *t, *u, perm[kMaxKVMTerms];
	KVMCascConfig *pdev;
	KVMUserPermConfig *puser = &gCache.kvmuser.userperm;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	pdev = &gCache.kvmdevice.deviceList[gCache.kvmuser.devicenum];
	for (i = 0; i < pdev->numTerms; i ++) {
		perm[i] = PERM_DEFAULT;
	}
	for (i = 0; i < 4; i ++) {
		s = permbuf[i];
		while (*s) {
			if ((t = strchr(s, ',')) != NULL) {
				*t ++ = 0;
			} else {
				t = s + strlen(s);
			}
			if ((u = strchr(s, '-')) != NULL) {
				*u ++ = 0;
				if (sscanf(u, "%d", &max) < 1 || max == 0 ||
					max > pdev->numTerms) {
					/* parsing error */
				}
			} else {
				max = 0;
			}
			if (sscanf(s, "%d", &min) < 1 || min == 0 || 
				min > pdev->numTerms) {
				/* parsing error */
			}
			if (max == 0) {
				max = min;
			}
			if (max < min) {
				/* parsing error */
			}
			for (j = min; j <= max; j ++) {
				if (perm[j - 1] != PERM_DEFAULT) {
					/* parsing error - overlap */
				}
				perm[j - 1] = validperms[i];
			}
			s = t;
		}
	}
	/* Remove all the permissions set for that device */
	for (i = 0; i < puser->nperms; i++) {
		if (((puser->perms[i].device[0] != 0) &&
			strcmp(puser->perms[i].device, gCache.kvmuser.currDevice))
			|| ((puser->perms[i].device[0] == 0) && 
			strcmp(gCache.kvmuser.currDevice, "master"))) {
			continue;
		}
		for (j = i + 1; j < puser->nperms; j ++) {
			puser->perms[j - 1] = puser->perms[j];
		}
		i --;
		puser->nperms --;
	}
	/* Count the number of permissions set for this device; reallocate the
	buffer if necessary. */
	j = puser->nperms;
	for (i = 0; i < pdev->numTerms; i ++) {
		if (perm[i] != PERM_DEFAULT) {
			j ++;
		}
	}
	if (j > puser->npermsalloc) {
		void *auxbuf = balloc(B_L, j * sizeof(KVMPortPerm));
		if (! auxbuf) {
			return mAppError(kMemAllocationFailure);
		}
		memset(auxbuf, 0, j * sizeof(KVMPortPerm));
		if (puser->nperms && puser->perms) {
			memcpy(auxbuf, puser->perms, puser->npermsalloc * sizeof(KVMPortPerm));
			bfreeSafe(B_L, puser->perms);
		}
		puser->perms = auxbuf;
		puser->npermsalloc = j;
	}
	/* Add all the permissions set */
	for (i = 0; i < pdev->numTerms; i ++) {
		if (perm[i] == PERM_DEFAULT) {
			continue;
		}
		if (!puser->perms) continue;
		if (! strcmp(gCache.kvmuser.currDevice, "master")) {
			*(puser->perms[puser->nperms].device) = 0; 
		} else {
			strcpy(puser->perms[puser->nperms].device, 
				gCache.kvmuser.currDevice);
		}
		puser->perms[puser->nperms].port = i + 1;
		puser->perms[puser->nperms].value = perm[i];
		puser->nperms ++;
	}
	gCache.kvmuser.currDefault = 0;
	gCache.kvmuser.from = 0;
	/* this fixes bug #2694: */
	if(strcmp(gCache.kvmuser.currName, "Generic User") != 0)
		SetAccListHtml(puser, &gCache.kvmuser.KVMPermHtml);
	else
		SetAccListHtml(puser, &gCache.kvmuser.KVMDefPermHtml);
	if (gReq.action == kNone) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}

	result = expertCommit(param, request, urlQuery);
	return result;
}

Result LoadKVMAccList(void)
{
	int size, i;
	Result result;
	char_t *device;
	int j, k, l;
	KVMCascConfig *pdev;
	KVMUserPermConfig *puser;
	KVMUserAccessConfig *puseracc;
	result = LoadFirst(LoadKVMDevices, kKVMDevices, &gCache.kvmdevice,
		NULL, sizeof(KVMDevice));
	if (kCycSuccess != result) {
		return mAppError(result);
	}

	{
		CycParam counts[] = {
			{ kKVM, kKVMAccEntries, &(gCache.kvmuser.useracc), 0 },
		};
		result = GetCycParamValues(counts, mArrayLength(counts)); /* fetch sizes */
		if (result != kCycSuccess) {
			return result;
		}
	}
	if (gCache.kvmuser.useracc.userperms) {
		for (i = 0; i < gCache.kvmuser.nalloc; i ++) {
			bfreeSafe(B_L, 
				gCache.kvmuser.useracc.userperms[i].perms);
			gCache.kvmuser.useracc.userperms[i].perms = NULL;
		}
		bfreeSafe(B_L, gCache.kvmuser.useracc.userperms);
		gCache.kvmuser.useracc.userperms = NULL;
		gCache.kvmuser.nalloc = 0;
	}
	if (gCache.kvmuser.useracc.nacclists) {
		size = sizeof(KVMUserPermConfig) * 
			(gCache.kvmuser.useracc.nacclists);
		gCache.kvmuser.useracc.userperms = balloc(B_L, size);
		if (! gCache.kvmuser.useracc.userperms) {
			return mAppError(kMemAllocationFailure);
		}
		gCache.kvmuser.nalloc = gCache.kvmuser.useracc.nacclists;
		memset(gCache.kvmuser.useracc.userperms, 0, size);
	}
	{
		CycParam counts[] = {
			{ kKVM, kKVMPermEntries, &(gCache.kvmuser.useracc), 0 },
		};
		result = GetCycParamValues(counts, mArrayLength(counts));
		if (result != kCycSuccess) {
			return result;
		}
	}
	if (gCache.kvmuser.useracc.defuserperm.perms) {
		bfreeSafe(B_L, gCache.kvmuser.useracc.defuserperm.perms);
		gCache.kvmuser.useracc.defuserperm.perms = NULL;
	}
	if (gCache.kvmuser.useracc.defuserperm.nperms) {
		gCache.kvmuser.useracc.defuserperm.perms = 
			balloc(B_L, gCache.kvmuser.useracc.defuserperm.nperms * sizeof(KVMPortPerm));
		if (! gCache.kvmuser.useracc.defuserperm.perms) {
			return kMemAllocationFailure;
		}
		memset(gCache.kvmuser.useracc.defuserperm.perms, 0, 
			gCache.kvmuser.useracc.defuserperm.nperms * sizeof(KVMPortPerm));
	}
	gCache.kvmuser.useracc.defuserperm.npermsalloc = 
		gCache.kvmuser.useracc.defuserperm.nperms;
	for (i = 0; i < gCache.kvmuser.useracc.nacclists; i ++) {
		if (gCache.kvmuser.useracc.userperms[i].nperms) {
			gCache.kvmuser.useracc.userperms[i].perms = balloc(B_L, gCache.kvmuser.useracc.userperms[i].nperms * sizeof(KVMPortPerm));
			if (! gCache.kvmuser.useracc.userperms[i].perms) {
				return mAppError(kMemAllocationFailure);
			}
			memset(gCache.kvmuser.useracc.userperms[i].perms, 0, gCache.kvmuser.useracc.userperms[i].nperms * sizeof(KVMPortPerm));
		}
		gCache.kvmuser.useracc.userperms[i].npermsalloc = gCache.kvmuser.useracc.userperms[i].nperms;
	}
	{
		CycParam values[] =
		{
			{ kKVM, kKVMAccList, &(gCache.kvmuser.useracc), 0 },
		};
		result = GetCycParamValues(values, mArrayLength(values));
		if (kCycSuccess != result) {
			return mAppError(result);
		}
	}
	puseracc = &gCache.kvmuser.useracc;
   /* We should check if the devices configured in the access list are valid;
   otherwise, we should delete the entries */
	for (i = 0; i < puseracc->nacclists; i++) {
		puser = &puseracc->userperms[i];
		for (j = 0; j < puser->nperms; j ++) {
			device = puser->perms[j].device;
			if (*device == 0) continue;
			for (k = 0; k < gCache.kvmdevice.numDevices; k ++) {
				pdev = &gCache.kvmdevice.deviceList[k];
				if (! strcmp(device, pdev->name)) {
					break;
				}
			}
			if (k == gCache.kvmdevice.numDevices || 
				puser->perms[j].port > pdev->numTerms) {
				/* Remove this entry */
				for (l = j + 1; l < puser->nperms; l ++) {
					puser->perms[l - 1] = puser->perms[l];
				}
				puser->nperms --;
				j --;
			}
		}
	}
	return result;
}


static void SetAccListRanges(KVMUserPermConfig *puser, UInt32 devnum, 
	unsigned char **buf)
{
   int j, k, min, max;
   unsigned char perm[kMaxKVMTerms];
   KVMCascConfig *pdev = &gCache.kvmdevice.deviceList[devnum];
   KVMPortPerm *pperm;
   /* Grab the permissions for all the ports */
   for (j = 0; j < pdev->numTerms; j ++) {
      perm[j] = PERM_DEFAULT;
   }
   for (j = 0; j < puser->nperms; j ++) {
      pperm = &puser->perms[j];
      if (devnum == 0 && *pperm->device) {
         continue;
      }
      if (devnum > 0 && strcmp(pdev->name, pperm->device)) {
         continue;
      }
      if (pperm->port == 0 || pperm->port > pdev->numTerms) {
         continue;
      }
      perm[pperm->port - 1] = pperm->value;
   }
   /* Check for the ranges */
   for (j = 0; j < 4; j ++) {
      *buf[j] = 0;
      min = max = 0;
      for (k = 0; k < pdev->numTerms; k ++) {
         if (perm[k] == validperms[j]) {
             if (k == max && max != 0) {
                max = k + 1;
             } else {
                if (min != 0) {
                   if (min == max) {
                       sprintf(buf[j] + strlen(buf[j]), "%d,", min);
                   } else {
                       sprintf(buf[j] + strlen(buf[j]), "%d-%d,", min, max);
                   }
                }
                min = max = k + 1;
             }
         }
      }
      if (min != 0) {
         if (min == max) {
            sprintf(buf[j] + strlen(buf[j]), "%d", min);
         } else {
            sprintf(buf[j] + strlen(buf[j]), "%d-%d", min, max);
         }
      }
   }
}

static void SetAccListHtml(KVMUserPermConfig *puser, char_t **bufptr)
{
   char_t *s, *buf = *bufptr;
   int i, j, k;

   bfreeSafe(B_L, buf);
   buf = balloc(B_L, gCache.kvmdevice.numDevices * kDevicePermLength);
   if (buf == NULL) {
      printf("SetAccListHtml - could not allocate buffer for device permissions\n");
      return;
   }
   *buf = 0;

   for (i = 0; i < gCache.kvmdevice.numDevices; i ++) {
      unsigned char permaux[4 * kPortRangeLength];
      unsigned char *permbuf[4] = {permaux, permaux + kPortRangeLength, 
         permaux + 2 * kPortRangeLength, permaux + 3 * kPortRangeLength};
      KVMCascConfig *pdev = &gCache.kvmdevice.deviceList[i];
      gsprintf(buf + strlen(buf), "'");
      s = buf + strlen(buf);
      gsprintf(s, "%s", pdev->name);
      if (strlen(s) < 16) {
         for (j = strlen(s); j < 16; j ++) {
            gsprintf(s + strlen(s), " ");
         }
      } else {
         *(s + 16) = 0;
      }
      strcat(s, "   ");
      SetAccListRanges(puser, i, permbuf);
      k = 0;
      s = buf + strlen(buf);
      for (j = 0; j < 4; j ++) {
         if (*permbuf[j]) {
            gsprintf(s + strlen(s), "%s:%s/", permbuf[j], permname[j]);
            k ++;
         }
      }
      if (k) {
         *(s + strlen(s) - 1) = 0;
      } else {
         gsprintf(s + strlen(s), "-");
      }
      if (strlen(s) > 36) {
         *(s + 34) = '.';
         *(s + 35) = '.';
         *(s + 36) = 0;
      }
      gsprintf(s + strlen(s), "',");
   }
   if (gCache.kvmdevice.numDevices) {
      *(s + strlen(s) - 1) = 0;
   }
   *bufptr = buf;
}

int GetKVMAccList(Param* param, int request)
{
	char_t *device;
	int i, idx;
	Result result;
	KVMUserPermConfig *puser;
	KVMUserAccessConfig *puseracc;


	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

/*
	if (GetCycUserType() != kAdminUser) {
		if (sessTimeout)
			return mAppError(kTimeout);
		return mAppError(kInvalidLogin);
	}
*/

	result = LoadFirst(LoadKVMAccList, kKVMDAccList, &gCache.kvmuser.useracc, 
		&gCacheSave.kvmuser.useracc, sizeof(gCache.kvmuser.useracc));

	if (kCycSuccess != result) {
		return mAppError(result);
	}

	puseracc = &gCache.kvmuser.useracc;

	/* Get the access list for the user/group selected */
	if (gCache.kvmuser.from) {
		gCache.kvmuser.currDefault = 1;

		if(strcmp(gCache.kvmuser.currName, "Generic User") == 0){
			puser = &(puseracc->defuserperm);
			gCache.kvmuser.currDefault = 0;
			copyUserEntry(&gCache.kvmuser.userperm, puser);

			/* Mount the options */
			SetAccListHtml(&puseracc->defuserperm, &gCache.kvmuser.KVMPermHtml);
			SetAccListHtml(&puseracc->defuserperm, &gCache.kvmuser.KVMDefPermHtml);
		}else{
			for (i = 0; i < puseracc->nacclists; i++) {
				idx = puseracc->acclistList[i];
				if (idx == 0) {
					continue;
				} else {
					idx --;
				}
				puser = &puseracc->userperms[idx];
				if (strcmp(puser->name, gCache.kvmuser.currName)) {
					continue;
				}
				if (puser->usergroup != gCache.kvmuser.currUserGroup) {
					continue;
				}
				/* access list for the user found. disable default */
				gCache.kvmuser.currDefault = 0;
				copyUserEntry(&gCache.kvmuser.userperm, puser);
				break;
			}
			/* Mount the options */
			if (i < puseracc->nacclists) {
				SetAccListHtml(&gCache.kvmuser.userperm, &gCache.kvmuser.KVMPermHtml);
				SetAccListHtml(&puseracc->defuserperm, &gCache.kvmuser.KVMDefPermHtml);
			} else {
				puser = &(puseracc->defuserperm);
				copyUserEntry(&gCache.kvmuser.userperm, puser);

				/* Mount the options */
				SetAccListHtml(&puseracc->defuserperm, &gCache.kvmuser.KVMPermHtml);
				SetAccListHtml(&puseracc->defuserperm, &gCache.kvmuser.KVMDefPermHtml);
			}
		}
	}

	return mAppError(result);
}

int GetKVMDevicePermissions(Param *param, int request)
{
   char_t *temp = NULL;
   unsigned char *permbuf[4] = {gCache.kvmuser.ports_none, 
      gCache.kvmuser.ports_ro, gCache.kvmuser.ports_rw, 
      gCache.kvmuser.ports_full}; 
   int i = 0;
   KVMCascConfig *pdev;

//printf("GetKVMDevicePermissions - start\n");
   if (kDmfSuccess == dmfGetParam(kSel1Key, NULL, &temp, -1)) {
//printf("GetKVMDevicePermissions - temp = %s\n", temp);
      i = gatoi(temp);
      bfreeSafe(B_L, temp);
   }
   if (i < 0 || i >= gCache.kvmdevice.numDevices) {
      i = 0;
   }
   pdev = &gCache.kvmdevice.deviceList[i];
   gCache.kvmuser.devicenum = i;
   if (i == 0) {
      strcpy(gCache.kvmuser.currDevice, "master");
   } else {
      strcpy(gCache.kvmuser.currDevice, pdev->name);
   }
//printf("GetKVMDevicePermissions - device %s\n", gCache.kvmuser.currDevice);
   gCache.kvmuser.numports = pdev->numTerms;
//printf("GetKVMDevicePermissions - pass1\n");
   SetAccListRanges(&gCache.kvmuser.userperm, i, permbuf);
//printf("GetKVMDevicePermissions - pass2\n");
   
   return mAppError(kCycSuccess);
}

int SetKVMAccList(void)
{
	if (!gCacheLoaded[kKVMDAccList]) return(0); // The page was even loaded
	RegisterParamChange(0, &gCache.kvmuser.useracc, kKVM, kKVMAccList, 0);
	return 0;
}

