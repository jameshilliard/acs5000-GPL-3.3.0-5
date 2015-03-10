#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
#include <webpmcomm.h>

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

unsigned char debug_flag = 0;

#define itoa(sid, temp, len) snprintf(temp, len, "%d", sid)

static int  KVMChangeTermStatus(char_t * selected,KVMTermConfig *termList, int numPorts, 
		int action)
{
	int change = 0;
	int i,num;
	char *psel;
	
	psel = selected;
	
	for (i=0; (i < numPorts) && *psel; i++) {
		num = 0;
		sscanf(psel,",%d,",&num);
		if (!num) break;
		psel++;
		while (*psel && (*psel != ',')) psel++;
		if (num <= numPorts) {
			if (termList[num-1].status != action) {
				termList[num-1].status = action;
				change = 1;
			}
		}
	}
	return (change);
}

static int KVMCheckDeviceConnect(KVMCascConfig *devConf, int action, int oldIndex)
{
	int conn = 0;
	int num=0;
	KVMTermConfig *termList=0;
	KVMCascConfig *dev,*old;

	if (devConf->connectionType == 1) { // network
		return 1;
	}

	// found the device 
	for (num=0, dev=gCache.kvmdevice.deviceList; num < gCache.kvmdevice.numDevices; 
			num++, dev++) 
	{
		if ((dev->action == kDelete) || (dev->action == kActionIgnore)) 
		       continue;	
		if (!strcmp(dev->name, devConf->device)) {
			termList = dev->terms;
			break;
		}
	}
	
	if (!termList) return 0;
	
	if (action == kDelete) {
		// removed the terms from the list
		conn = devConf->connectTerms[0];
		(termList + conn - 1)->status = kTermEnable;
		(termList + conn - 1)->action = kEdit;
		conn = devConf->connectTerms[1];
		if (conn) {
			(termList + conn - 1)->status = kTermEnable;
			(termList + conn - 1)->action = kEdit;
		}
		return 1;
	}
	
	// action kEdit
	if (action == kEdit) {
		old = &gCache.kvmdevice.deviceList[oldIndex];
		// removed the terms from the list
		conn = old->connectTerms[0];
		(termList + conn - 1)->status = kTermEnable;
		(termList + conn - 1)->action = kEdit;
		conn = old->connectTerms[1];
		if (conn) {
			(termList + conn - 1)->status = kTermEnable;
			(termList + conn - 1)->action = kEdit;
		}
	}
	
	conn = devConf->connectTerms[0];
	if ((termList + conn - 1)->status != kTermCascading) {
		num = devConf->connectTerms[1];
		if (num) {
			if ((termList + num - 1)->status != kTermCascading) {
				(termList+conn-1)->status = kTermCascading;
				(termList+num-1)->status = kTermCascading;
				(termList + conn - 1)->action = kEdit;
				(termList + num - 1)->action = kEdit;
				return 1;
			} else {
				return 0;
			}
		} else {
			(termList + conn - 1)->action = kEdit;
			(termList+conn-1)->status = kTermCascading;
			return 1;
		}
	}
	return 0;
}

static int KVMCheckDeviceName(char *name)
{
	int num=0;
	KVMCascConfig *dev;

	dev = gCache.kvmdevice.deviceList;

	for (num=0; num < gCache.kvmdevice.numDevices; num++, dev++) {
		if ((dev->action == kDelete) || (dev->action == kActionIgnore)) 
		       continue;	
		if (!strcmp(dev->name, name)) {
			return 1;
		}
	}
	return 0;
}

static int KVMCheckTermAlias(char *alias)
{
	int num,nterm;
	KVMCascConfig *dev;
	KVMTermConfig *term;


	dev = &gCache.kvmdevice.confDevice;
	term = dev->terms;
	for (nterm=0; nterm < dev->numTerms; nterm++, term++) {
		if (term->status == 2) {
			continue;
		}
//printf("alias1 %s\n", term->alias);
		if (!strcmp(term->alias, alias)) {
			return 1;
		}
	}

	dev = gCache.kvmdevice.deviceList;
	for (num=0; num < gCache.kvmdevice.numDevices; num++, dev++) {
		if (num == gCache.kvmdevice.deviceSelected - 1) {
			continue;
		}
		if ((dev->action == kDelete) || (dev->action == kActionIgnore)) 
			continue;
		term = dev->terms;
		for (nterm=0; nterm < dev->numTerms; nterm++, term++) {
			if (term->status == 2) {
				continue;
			}
//printf("alias2 %s\n", term->alias);
			if (!strcmp(term->alias, alias)) {
				return 1;
			}
		}
	}
	return 0;
}

static int KVMCheckTermOutlet(char *device, UInt32 outlet, UInt32 idx)
{
	int num,nterm,out;
	KVMCascConfig *dev;
	KVMTermConfig *term;

	if (outlet == 0) {
		return 0;
	}
	//Point to the right structure
	term = &gCache.kvmdevice.confTerm;

	for (out = 0; out < 2; out ++) {
		if (out == idx) {
			continue;
		}
		if (term->pmoutlet[out] == 0) {
			continue;
		}
		if (! strcmp(term->pmdevice[out], device) && 
			term->pmoutlet[out] == outlet) {
			return 1;
		}
	}

	//point to the right device
	dev = &gCache.kvmdevice.confDevice;
	term = dev->terms;
	for (nterm = 0; nterm < dev->numTerms; nterm++, term++) {
		if (term->status == 2) {
			continue;
		}
		if (nterm == gCache.kvmdevice.selected[0] - 1) {
			continue;
		}
		for (out = 0; out < 2; out ++) {
			if (term->pmoutlet[out] == 0) {
				continue;
			}
			if (! strcmp(term->pmdevice[out], device) && 
				term->pmoutlet[out] == outlet) {
				return 1;
			}
		}
	}

	dev = gCache.kvmdevice.deviceList;
	for (num=0; num < gCache.kvmdevice.numDevices; num++, dev++) {
		if (num == gCache.kvmdevice.deviceSelected - 1) {
			continue;
		}
		if ((dev->action == kDelete) || (dev->action == kActionIgnore)) 
		       continue;	
		term = dev->terms;
		for (nterm=0; nterm < dev->numTerms; nterm++, term++) {
			if (term->status == 2) {
				continue;
			}
			for (out = 0; out < 2; out ++) {
				if (term->pmoutlet[out] == 0) {
					continue;
				}
				if (! strcmp(term->pmdevice[out], device) && 
					term->pmoutlet[out] == outlet) {
					return 1;
				}
			}
		}
	}
	return 0;
}

static kvmSetTermDefault(KVMCascConfig *dev,int fport)
{
	KVMTermConfig *term;
	int nterm;
	int i;
	
	term = &dev->terms[fport-1];
	for (nterm=fport-1; nterm < dev->numTerms; nterm++, term++) {
		term->action = kNone;
		term->status = 1;
		sprintf(term->alias,"%s.%d",dev->name,nterm+1);
		strcpy(term->pmdevice[0],"master");
		strcpy(term->pmdevice[1],"master");
		term->pmoutlet[0] = term->pmoutlet[1] = 0;
		term->lclagc = term->remagc = 1;
		term->lclbright = term->rembright = 128;
		term->lclcontr = term->remcontr = 96;
		for (i = 0; i < 4; i ++) {
			term->ipbright[i] = 128;
			term->ipcontr[i] = 96;
		}
	}
}

static KVMGetOutletStr()
{
	int numEntries;
	int i, j;
	char buffer[100] = {0};
	KVMCascConfig *dev;

	numEntries = 0;
	for (i = 0; i < gCache.kvmdevice.numDevices; i++) {
		numEntries += gCache.kvmdevice.deviceList[i].numTerms;
	}
	bfreeSafe(B_L,gCache.kvmdevice.pmoutletHtml);
	gCache.kvmdevice.pmoutletHtml = balloc(B_L, 100 * numEntries);
	if (!gCache.kvmdevice.pmoutletHtml) {
		return mAppError(kMemAllocationFailure); 
	}
	strcpy(gCache.kvmdevice.pmoutletHtml, "[");
	for (i = 0; i < gCache.kvmdevice.numDevices; i++) {

		dev = &gCache.kvmdevice.deviceList[i];
		for (j = 0; j < dev->numTerms; j++) {
			if ((dev->terms[j].pmoutletstr) && (*dev->terms[j].pmoutletstr)) {
				sprintf(buffer, "[%d, \"%s\",",
					i, gCache.kvmdevice.deviceList[i].name); 
				gstrcat(gCache.kvmdevice.pmoutletHtml, buffer);
				sprintf(buffer, " \"%s\", \"%s\"],",
					dev->terms[j].alias, dev->terms[j].pmoutletstr);
				gstrcat(gCache.kvmdevice.pmoutletHtml, buffer);
			}
		}
	}

	// insert indication of the last one
	sprintf(buffer, "[-1, \"\", \"\", \"\"]]");
	gstrcat(gCache.kvmdevice.pmoutletHtml, buffer);
	return 1;
}
// Commit routines
extern int KVMDeviceCommit(Param * param, int request, char_t *urlQuery)
{
	int num=0, change=0, ign=0,size;
	char devCs[10],buf[100];
	Result result = kCycSuccess;
	KVMCascConfig *dev;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	if (checkCancelRequest(request, kKVMDevices))
		return mAppError(result);

	if (!gCacheLoaded[kKVMDevices]) return(0);

	strcpy(gCache.kvmdevice.deviceMsgError,"OK");

	for (num=0; num < gCache.kvmdevice.numDevices; num++) {
		sprintf(devCs,",%d,",num+1);
		if (!strstr(gCache.kvmdevice.deviceCs,devCs)) {
			// the device was deleted
			change = 1;
			KVMCheckDeviceConnect(&gCache.kvmdevice.deviceList[num], kDelete,0);
			if (gCache.kvmdevice.deviceList[num].action == kAdd) {
				gCache.kvmdevice.deviceList[num].action = kActionIgnore;
				ign = 1;
			} else {
				gCache.kvmdevice.deviceList[num].action = kDelete;
			}
		}
	}

	switch (gCache.kvmdevice.deviceAction) {
		case kAdd : // add operation
			if (KVMCheckDeviceName(gCache.kvmdevice.confDevice.name)) {
				strcpy(gCache.kvmdevice.deviceMsgError,"Name is already in use");
				break;
			}

			if ((gCache.kvmdevice.confDevice.connectionType == 1) ||  // network
			    (KVMCheckDeviceConnect(&gCache.kvmdevice.confDevice,kAdd,0))) { // physical
				dev = gCache.kvmdevice.deviceList;
				size = (gCache.kvmdevice.numDevices+1) * sizeof(KVMCascConfig);
				gCache.kvmdevice.deviceList = balloc(B_L, size);
				if (! gCache.kvmdevice.deviceList) {
					return mAppError(kMemAllocationFailure);
				}
				memcpy((char*)gCache.kvmdevice.deviceList, (char *)dev,size-sizeof(KVMCascConfig));
				bfreeSafe(B_L,dev);
				dev = &gCache.kvmdevice.deviceList[gCache.kvmdevice.numDevices];
				gCache.kvmdevice.numDevices++;
				memcpy((char*)dev, (char*)&gCache.kvmdevice.confDevice,sizeof(KVMCascConfig));
				// set TERMS default : ENABLE
				kvmSetTermDefault(dev,1);
				change = 1;
			} else {
				strcpy(gCache.kvmdevice.deviceMsgError,"Unavailable Port was selected");
			}
			break;

		case kEdit : // edit operation
			num = gCache.kvmdevice.deviceSelected - 1;
			dev = &gCache.kvmdevice.deviceList[num];
			change = 1;
			if (strcmp(dev->name, gCache.kvmdevice.confDevice.name)) { // changed the device
				if (KVMCheckDeviceName(gCache.kvmdevice.confDevice.name)) {
					strcpy(gCache.kvmdevice.deviceMsgError,"Name is already in use");
					break;
				}
			}

			if (gCache.kvmdevice.confDevice.connectionType == 1) { // network
				if (dev->connectionType == 0) { // old - physical
					KVMCheckDeviceConnect(&gCache.kvmdevice.deviceList[num], kDelete,0);
				}
				// set TERMS default : ENABLE
				if (dev->numTerms < gCache.kvmdevice.confDevice.numTerms) {
					size = dev->numTerms + 1;
					memcpy((char*)dev, (char*)&gCache.kvmdevice.confDevice,sizeof(KVMCascConfig));
					kvmSetTermDefault(dev,size);
				} else {
					memcpy((char*)dev, (char*)&gCache.kvmdevice.confDevice,sizeof(KVMCascConfig));
				}
				break;
			}
			//physical - check terms	
			if (strcmp(dev->device, gCache.kvmdevice.confDevice.device)) { // changed the device
				if (KVMCheckDeviceConnect(&gCache.kvmdevice.confDevice,kAdd,0)) {
					KVMCheckDeviceConnect(&gCache.kvmdevice.deviceList[num], kDelete,0);
					// set TERMS default : ENABLE
					if (dev->numTerms < gCache.kvmdevice.confDevice.numTerms) {
						size = dev->numTerms + 1;
						memcpy((char*)dev, (char*)&gCache.kvmdevice.confDevice,sizeof(KVMCascConfig));
						kvmSetTermDefault(dev,size);
					} else {
						memcpy((char*)dev, (char*)&gCache.kvmdevice.confDevice,sizeof(KVMCascConfig));
					}
				} else {
					strcpy(gCache.kvmdevice.deviceMsgError,"Unavailable Port was selected");
				}
			} else {
				if (KVMCheckDeviceConnect(&gCache.kvmdevice.confDevice,kEdit,num)) {
					// set TERMS default : ENABLE
					if (dev->numTerms < gCache.kvmdevice.confDevice.numTerms) {
						size = dev->numTerms + 1;
						memcpy((char*)dev, (char*)&gCache.kvmdevice.confDevice,sizeof(KVMCascConfig));
						kvmSetTermDefault(dev,size);
					} else {
						memcpy((char*)dev, (char*)&gCache.kvmdevice.confDevice,sizeof(KVMCascConfig));
					}
				} else {
					strcpy(gCache.kvmdevice.deviceMsgError,"Unavailable Port was selected");
				}
			}
			break;

		case kActionIgnore : // ports
			dev = &gCache.kvmdevice.deviceList[gCache.kvmdevice.deviceSelected - 1];
			memcpy((char*)&gCache.kvmdevice.confDevice,(char*)dev,sizeof(KVMCascConfig));	
			break;
	}
	
	if (ign) {
		for (num=0; num < gCache.kvmdevice.numDevices; num++) {
			if (gCache.kvmdevice.deviceList[num].action == kActionIgnore) {
				if ((num+1) != gCache.kvmdevice.numDevices) {
					dev = &gCache.kvmdevice.deviceList[gCache.kvmdevice.numDevices-1];
					memcpy((char*)&gCache.kvmdevice.deviceList[num],(char*)dev, sizeof(KVMCascConfig));
					memset(dev,0, sizeof(KVMCascConfig));
					if (gCache.kvmdevice.deviceSelected == gCache.kvmdevice.numDevices) {
						gCache.kvmdevice.deviceSelected = num+1;
					}
				}
				gCache.kvmdevice.numDevices--;
			}
		}
	}
	
	dev = &gCache.kvmdevice.deviceList[0];

	memset(gCache.kvmdevice.deviceNameList,0,kUserListLength);
	memset(gCache.kvmdevice.deviceNTermsList,0,kUserListLength);

	for (num=0 ; num < gCache.kvmdevice.numDevices; num++, dev++) {
		
		if (dev->action == kDelete) continue;

		sprintf(buf,"%s ",dev->name);
		gstrcat(gCache.kvmdevice.deviceNameList,buf);

		sprintf(buf,"%d ",dev->numTerms);
		gstrcat(gCache.kvmdevice.deviceNTermsList,buf);
	}
	gCache.kvmdevice.changed |= change;
	
	gLedState |= change;
	
	if (gReq.action == kNone) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}
	
	return expertCommit(param, request, urlQuery);
}

extern int KVMDeviceTermCommit(Param * param, int request, char_t *urlQuery)
{
	int num=0, change=0, out, ok;
	Result result = kCycSuccess;
	KVMTermConfig *term;

//	printf("pmoutletstr: [%s]\n", gCache.kvmdevice.confTerm.pmoutletstr);

        if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess) {
		return mAppError(result);
	}

	if (checkCancelRequest(request, -1)) {
		return mAppError(result);
	}

	if (!gCacheLoaded[kKVMDevices]) {
		return(0);
	}
	
	if (gCache.kvmdevice.deviceSelected > gCache.kvmdevice.numDevices) {
		return (0);
	}
	   	
	strcpy(gCache.kvmdevice.deviceMsgError, "OK");
	if (gCache.kvmdevice.action == kEdit) {
		sscanf(gCache.kvmdevice.selected,",%d,",&num);
		if (num) {
			num--;
			term = &gCache.kvmdevice.confDevice.terms[num];
			ok = 1;
			
			if (memcmp((char*)&gCache.kvmdevice.confTerm, (char*)term, sizeof(KVMTermConfig))) {
				if (strcmp(gCache.kvmdevice.confTerm.alias, term->alias)) {
					if (KVMCheckTermAlias(gCache.kvmdevice.confTerm.alias)) {
						sprintf(gCache.kvmdevice.deviceMsgError,"Alias [%s] is already in use", gCache.kvmdevice.confTerm.alias);
						ok = 0;
						if (!term->alias[0]) {
							gCache.kvmdevice.confTerm.alias[0] = 0;
						}
					}
				}

				if (ok) {
					change = 1;
					gCache.kvmdevice.confDevice.terms[num] = gCache.kvmdevice.confTerm;
					gCache.kvmdevice.confDevice.terms[num].action = kEdit;
				}
			}
		}
	} else {
		if (gCache.kvmdevice.action != 0) { // disable or enable
			change = KVMChangeTermStatus(gCache.kvmdevice.selected,gCache.kvmdevice.confDevice.terms,
					gCache.kvmdevice.confDevice.numTerms, gCache.kvmdevice.action-1);
	 	}		
	}
	
	gCache.kvmdevice.changed |= change;
	
	gLedState |= change;
	
	//if (gCache.kvmdevice.action == 0) {
		gCache.kvmdevice.deviceList[gCache.kvmdevice.deviceSelected-1] = gCache.kvmdevice.confDevice;
	//}		//Christine Qiu comments for bug 5664

	if (gReq.action == kNone) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}
	
	return expertCommit(param, request, urlQuery);
}

// Load functions
Result LoadKVMDevices(void)
{
	int size;
	Result result = kCycSuccess;
	CycParam counts[] =
	{
		{ kKVMCascading, kKVMCascNumDevices, &(gCache.kvmdevice.numDevices),0},
	};

	GetCycParamValues(counts, 1);

	bfreeSafe(B_L,gCache.kvmdevice.deviceList);
	
	if (gCache.kvmdevice.numDevices > 0) {
		size = gCache.kvmdevice.numDevices * sizeof(KVMCascConfig);
		gCache.kvmdevice.deviceList = balloc(B_L, size);
		if (! gCache.kvmdevice.deviceList) {
			return kMemAllocationFailure;
		}
		memset((char*)gCache.kvmdevice.deviceList,0, size);
		{
			CycParam values[] = 
			{
				{ kKVMCascading, kKVMCascConf, gCache.kvmdevice.deviceList,
						gCache.kvmdevice.numDevices},
			};
			result = GetCycParamValues(values,1);
		}
	}

	strcpy(gCache.kvmdevice.deviceMsgError,"OK");
	return result;
}

// Get functions

int KVMMountTermTable()
{
	char_t temp[]="<option value=\"%d\">";
	char_t buf[300],*bufHtml,physId[20];
	int x;
	KVMTermConfig *term;

	bfreeSafe(B_L,gCache.kvmdevice.termTableHtml);
	
	x = gCache.kvmdevice.confDevice.numTerms * 300;
	bufHtml = gCache.kvmdevice.termTableHtml = balloc(B_L, x);
	if (! bufHtml) {
		return 0;
	}
	memset(bufHtml, 0, x);
	term = gCache.kvmdevice.confDevice.terms;

	if (strcmp(gCache.kvmdevice.confDevice.name,"master")) {
		strcpy(physId,gCache.kvmdevice.confDevice.name);
		strcat(physId,".%d");
	} else {
		strcpy(physId,"Port_%d");
	}
	for (x=0 ; x < gCache.kvmdevice.confDevice.numTerms ; x++, term++) {
		if (term->status == kTermCascading) continue;
		
		gsprintf(buf,temp,x+1);
		gstrcat(bufHtml,buf);
		
		// alias
		if (term->alias[0]) 
			includeNbsp(buf,term->alias,20); 
		else 
			includeNbsp(buf,0,20); 

		gstrcat(bufHtml,buf);

		gstrcat(bufHtml,"&nbsp;&nbsp;&nbsp;");

		gsprintf(buf,physId,x+1);
		includeNbsp(buf,buf,20);
		gstrcat(bufHtml,buf);
		
		gstrcat(bufHtml,"&nbsp;&nbsp;&nbsp;");
		// disable
		if (term->status == kTermDisable) {
			gstrcat(bufHtml,"Yes");
		} else {
			gstrcat(bufHtml,"&nbsp;&nbsp;&nbsp;");
		}
		gstrcat(bufHtml,"&nbsp;&nbsp;&nbsp;");
		gstrcat(bufHtml,"</option>\n");
	}
	return 1;
}


int KVMMountDeviceHTMLTable()
{
	char_t temp[]="<option value=\"%d\">";
	char_t buf[250],*bufHtml, *bufHtml2, ports[32];
	int x;
	KVMCascConfig *dev;

	bfreeSafe(B_L,gCache.kvmdevice.deviceTableHtml);
	x = gCache.kvmdevice.numDevices * 250;
	bufHtml = gCache.kvmdevice.deviceTableHtml = balloc(B_L, x);
	if (! bufHtml) {
		return 0;
	}
	bfreeSafe(B_L,gCache.kvmdevice.portsUsedHtml);
	bufHtml2 = gCache.kvmdevice.portsUsedHtml = balloc(B_L, 100);
	if (! bufHtml2) {
		return 0;
	}
	memset(bufHtml,0,x);
	memset(ports,0,32);

	dev = &gCache.kvmdevice.deviceList[0];

	// Master Line
	gsprintf(buf,temp,1);
	gstrcat(bufHtml,buf);
	includeNbsp(buf,dev->name,17); 
	gstrcat(bufHtml,buf);
	includeNbsp(buf, NULL, 23);
	gstrcat(bufHtml,buf);
	if (dev->numTerms < 9) {
		strcat(bufHtml,"&nbsp;");
	}
	gsprintf(buf,"%d&nbsp;",dev->numTerms);
	gstrcat(bufHtml,buf);
	gstrcat(bufHtml,"</option>\n");

	sprintf(buf,"%s ",dev->name);
	gstrcpy(gCache.kvmdevice.deviceNameList,buf);

	sprintf(buf,"%d ",dev->numTerms);
	gstrcpy(gCache.kvmdevice.deviceNTermsList,buf);

	for (x=1 ; x < gCache.kvmdevice.numDevices; x++) {
		dev = &gCache.kvmdevice.deviceList[x];
		if (dev->action == kDelete) continue;

		gsprintf(buf,temp,x+1);
		gstrcat(bufHtml,buf);
		
		// device name
		if (dev->name[0]) 
			includeNbsp(buf,dev->name,17); 
		else 
			includeNbsp(buf,0,17); 
		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"&nbsp;&nbsp;&nbsp;");
		
		// Physical ID
		if (dev->connectionType == 1) { // network
			includeNbsp(buf, dev->hostName, 17);
		} else { // physical
			if (! strcmp(dev->device, "master")) {
				if (dev->connectTerms[0] > 0 && dev->connectTerms[0] <= 32) {
					ports[dev->connectTerms[0] - 1] = 1;
				}
				if (dev->connectTerms[1] > 0 && dev->connectTerms[1] <= 32) {
					ports[dev->connectTerms[1] - 1] = 1;
				}
			}
			strcpy(buf,dev->device);
			strcat(buf,":");
			if (dev->connectTerms[1]) {
				sprintf(&buf[strlen(buf)],"%d,%d",dev->connectTerms[0],dev->connectTerms[1]);
			} else {
				sprintf(&buf[strlen(buf)],"%d",dev->connectTerms[0]);
			}
			includeNbsp(buf,buf,17);	
		}
		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"&nbsp;&nbsp;&nbsp;");

		if (dev->numTerms < 9) {
			strcat(bufHtml,"&nbsp;");
		}
		gsprintf(buf,"%d&nbsp;",dev->numTerms);
		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"</option>\n");

		sprintf(buf,"%s ",dev->name);
		gstrcat(gCache.kvmdevice.deviceNameList,buf);

		sprintf(buf,"%d ",dev->numTerms);
		gstrcat(gCache.kvmdevice.deviceNTermsList,buf);
	}
	if (debug_flag) printf("devicebuf size %d [%s]\n", strlen(bufHtml), bufHtml);

	sprintf(bufHtml2, "%d", ports[0]);
	for (x = 1; x < 32; x ++) {
		sprintf(bufHtml2 + strlen(bufHtml2), ",%d", ports[x]);
	}
	return 1;
}

/******************************************************************************************
	Function KVMMountDeviceTable 
	Arguments: None
	Description: 
		Gather information from slave devices to mount a text table with
		devices names in the following format:
		master|slave1|slave2|...|slaven
		Fill the relate struct in Param.c
	Return: TRUE except when get a memory allocation error 
	See Also: KVMPMGetSlavePortStatus @ Server/KVMPorts.c
		Param.c
*******************************************************************************************/
int KVMMountDeviceTable()
{
	char_t *bufHtml;
	int x;
	KVMCascConfig *dev;

	bfreeSafe(B_L,gCache.kvmdevice.deviceTable);
	x = gCache.kvmdevice.numDevices * 250;
	bufHtml = gCache.kvmdevice.deviceTable = balloc(B_L, x);
	if (! bufHtml) {
		return 0;
	}
	memset(bufHtml,0,x);

	dev = &gCache.kvmdevice.deviceList[0];

	// Master Line
	gstrcat(bufHtml,dev->name);

	for (x=1 ; x < gCache.kvmdevice.numDevices; x++) {
		dev = &gCache.kvmdevice.deviceList[x];
		gstrcat(bufHtml, "|");
		gstrcat(bufHtml,dev->name);
	}
	if (debug_flag) printf("devicebuf size %d [%s]\n", strlen(bufHtml), bufHtml);
	return 1;
}

extern int KVMPMSendSlaveCommand(int device, int port, int pmCmd)
{
        unsigned char command[128];
        unsigned char tempcmd[7];

	switch(pmCmd) {
		case Turn_ON:
			sprintf(tempcmd, "%s", "on");
			break;
		case Turn_OFF:
			sprintf(tempcmd, "%s", "off");
			break;
		case Cycle:
			sprintf(tempcmd, "%s", "cycle");
			break;
		case Lock:
			sprintf(tempcmd, "%s", "lock");
			break;
		case Unlock:
			sprintf(tempcmd, "%s", "unlock");
			break;
		default:
			sprintf(tempcmd, "%s", "status");
			break;
	}


        /*
        Issue the command -- Station 1 (-s1) looks good, but may we have to try something dynamic
        Execute (-xcommand) command to get the outlet status
        for given outlet (-o<outlet>)
        */
        snprintf(command, 128, "/bin/kvm pmcommand -p%d -x%s -o%d", device, tempcmd, port);
        system(command);
#ifdef DEBUG
	printf("Issuing command %s\n", command);
#endif
	usleep(500000);

	return 1;
}


/******************************************************************************************
	Function KVMPMGetSlavePortStatus 
	Arguments: None
	Description:
		This function issues pmcommands to the cascaded boxes to read the PM
		status from the AUX port. The command is sent thru kvmd, and the 
		result is stored in /tmp/pmtemp file. 
		The file is readed and parsed, and the results are inserted in the
		proper structures.
	Return: TRUE if data is correctly loaded
	See Also: GetKVMDevice @ Server/KVMPorts.c  
		  Server/Param.c
		  Web/normal/applications/appsPMOutletsManMod.asp
*******************************************************************************************/
int KVMPMGetSlavePortStatus()
{
	unsigned char command[128], *tmpfile1 = "/tmp/kvmtemp";
	FILE *fp;
	struct stat st;
	unsigned char s[256], pt[256];
	char_t *bufHtml; 
	char_t bufport[256];
	int x, errflag=0;
	int number;
	char *name=NULL, *pwron; 
	int onoff, lock;

	/* Allocate the buffers, reserving 1024 for each PM device */
	x = gCache.kvmdevice.numDevices;
	bfreeSafe(B_L,gCache.kvmdevice.outletsHtml);
	bufHtml = gCache.kvmdevice.outletsHtml = balloc(B_L, 60 + 1124 * x);
	if (! bufHtml) {
		return mAppError(kMemAllocationFailure);
	}
	memset(bufHtml, 0, 60 + 1124 * x);

	strcpy(bufport, "var cascade_portinfo = [");
	gstrcat(bufHtml, "var cascade_outlinfo = [[");

	/* Read the PM info from each cascaded device */
	for (x=1 ; x < gCache.kvmdevice.numDevices; x++) {
		/* 
		Execute (-xstatus) command to get the outlet status
		for all outlets (-o0) 
		*/
		snprintf(command, 128, "/bin/kvm pmcommand -p%d -xstatus -o0> %s", x, tmpfile1);
		system(command);

		stat((const char *)tmpfile1, &st);
		if ((fp = fopen(tmpfile1, "r")) == NULL) {
			printf("Error opening %s\n", tmpfile1);
			return 0;
		}

		/* Parse outlet information */
		while(fgets(command, 128, fp)) {
			if (debug_flag) printf("Line: %s\n", command);
			if ( command[0] == '\n' || command[1] == '\n')  continue; 	/* means empty line */
			if ( !command || (strstr(command, "[Error]") || strstr(command, "not available"))) {
				errflag = 1; goto dealerr; } 	/* means IPDU error */
			if (!parse_pmstatus_line(command, &number, &name, &onoff, &lock, &pwron)) {
				continue;
			}
			if (pwron == NULL) {
				pwron = "";
			}
			sprintf(s, "[%d, \"%s\", \"%s\", \"%s\", \"%s\", %d],\n",
				number, convertOutletNameForWeb(name), 
				(onoff==PM_ON)?"on":"off", 
				(lock==PM_LOCKED)?"locked":"unlocked", 
				pwron, x);

			gstrcat(bufHtml, s);
		}
		/* Close file and release memory/structs */
dealerr:
		fclose(fp);
		unlink(tmpfile1);

		/* Fill in portInfoHtml */
		if (errflag) {
			/* Maybe some syslog info here */
			if (debug_flag) printf("IPDU Error at Cascaded device #%d\n", x);
			sprintf(s, "[%d, \"%s\", \"%s\", \"%s\", \"%s\", %d],\n",
					0, "", "", "", "", 0);
			gstrcat(bufHtml, s);
			errflag = 0;
			number = 0;
		}
		sprintf(pt, "[%d, %d, \"%s\", %d],",
			(int)number, x, "Cascaded", 1);
		gstrcat(bufport, pt);
	}
	gstrcat(bufHtml, "],];\n");
 	gstrcat(bufport, "];\n");
	gstrcat(bufHtml, bufport);
	return 1;
}


extern int GetKVMDevice(Param *param, int request)
{
	Result result;
	
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

	result = LoadFirst(LoadKVMDevices, kKVMDevices, &gCache.kvmdevice,NULL,sizeof(KVMDevice));

	if (result != kCycSuccess) return mAppError(result);

	if (gCache.kvmdevice.numDevices) {
		if (! KVMMountDeviceHTMLTable()) {
			return mAppError(kMemAllocationFailure);
		}
		if (! KVMMountDeviceTable()) {
			return mAppError(kMemAllocationFailure);
		}
	}

	return mAppError(result);
}

extern int GetKVMDeviceOutlets(Param *param, int request)
{
	Result result;
	
	if ((result = GetCycUserType(1)) != kCycSuccess)
		return mAppError(result);

	result = LoadFirst(LoadKVMDevices, kKVMDevices, &gCache.kvmdevice,
		NULL, sizeof(KVMDevice));

	if (result != kCycSuccess) return mAppError(result);

	if (gCache.kvmdevice.numDevices) {
		if (! KVMPMGetSlavePortStatus()) {
			return mAppError(kMemAllocationFailure);
		}
	}
	return mAppError(result);
}

extern int GetKVMDeviceSel(Param *param, int request)
{
	Result result = kCycSuccess;
	KVMCascConfig *dev;

	if (gCacheLoaded[kKVMDevices] == false) {
		return(0); 	// devices configuration did not load
	}
	if (gCache.kvmdevice.deviceSelected == 0) {
		memset(&gCache.kvmdevice.confDevice,0,sizeof(KVMCascConfig));
		return(0); 	// device not selected
	}
	
	if (gCache.kvmdevice.deviceSelected > gCache.kvmdevice.numDevices) {
		return(0);
	}

	
	dev = &gCache.kvmdevice.deviceList[gCache.kvmdevice.deviceSelected - 1];
	
	if (dev->action == kDelete) {
		return (0);
	}

	memcpy((char*)&gCache.kvmdevice.confDevice,(char*)dev,sizeof(KVMCascConfig));	

	return mAppError(result);
}

extern int GetKVMDeviceTerm(Param *param, int request)
{
	Result result = kCycSuccess;
	
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

	if (gCacheLoaded[kKVMDevices] == false) {
		return(0); 	// devices configuration did not load
	}

	if (gCache.kvmdevice.deviceSelected == 0) {
		return(0); 	// device not selected
	}
	//strcpy(gCache.kvmdevice.deviceMsgError,"OK");
	if (! KVMMountTermTable()) {
		return mAppError(kMemAllocationFailure);
	}
	
	if (! KVMGetOutletStr()) {
		return mAppError(kMemAllocationFailure);
	}
	return mAppError(result);
}

extern int GetKVMDeviceTermSel(Param *param, int request)
{
	UInt32 num;
	Result result = kCycSuccess;

	if (gCacheLoaded[kKVMDevices] == false) {
		return(0); 	// devices configuration did not load
	}

	if (gCache.kvmdevice.deviceSelected == 0) {
		return(0); 	// device not selected
	}
	
	sscanf(gCache.kvmdevice.selected,",%d,",&num);

	memcpy((char*)&gCache.kvmdevice.confTerm,
		(char*)&gCache.kvmdevice.confDevice.terms[num-1],
		sizeof(KVMTermConfig));

	return mAppError(result);
}

// Set functions
int SetKVMDevices(void)
{
	if (gCacheLoaded[kKVMDevices] == false) return(0);

	if (gCache.kvmdevice.changed == 1) {
		if (gReq.action != kNone) {
			RegisterParamChange(0,gCache.kvmdevice.deviceList,kKVMCascading,kKVMCascConf, gCache.kvmdevice.numDevices);
		}
		gLedState = 1;
	}
	return (0);
}


#ifdef RDPenable
/****************************************************************
** Routines to configure RDP servers
****************************************************************/

static int RDPCheckDeviceName(char *name)
{
	int num=0;
	RDPDevConfig *dev;

	dev = gCache.rdpdevice.deviceList;

	for (num=0; num < gCache.rdpdevice.numDevices; num++, dev++) {
		if (!strcmp(dev->name, name)) {
			return 1;
		}
	}
	return 0;
}
int RDPMountDeviceHTMLTable()
{
	char_t temp[]="<option value=\"%d\">";
	char_t buf[250],*bufHtml;
	int x = 2;
	RDPDevConfig *dev;

	bfreeSafe(B_L,gCache.rdpdevice.deviceTableHtml);
	x += gCache.rdpdevice.numDevices * 1024;
	bufHtml = gCache.rdpdevice.deviceTableHtml = balloc(B_L, x);
	if (! bufHtml) {
		return 0;
	}
	memset(bufHtml,0,x);

	for (x=0 ; x < gCache.rdpdevice.numDevices; x++) {
		dev = &gCache.rdpdevice.deviceList[x];

		gsprintf(buf,temp,x+1);
		gstrcat(bufHtml,buf);
		
		// server name
		if (dev->name[0]) 
			includeNbsp(buf,dev->name,19); 
		else 
			includeNbsp(buf,0,19); 
		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"&nbsp;");
		
		// server IP address
		if (dev->ipaddr[0]) {
			int iplen = strlen(dev->ipaddr);
			if (iplen > 15) {
				strncpy(buf,dev->ipaddr,7);
				strncpy(buf+7, "...", 3);
				strncpy(buf+10,dev->ipaddr+(iplen-5),6);
			} else {
				includeNbsp(buf,dev->ipaddr,15);
			}
		} else {
			includeNbsp(buf,0,15);
		}
		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"&nbsp;");
		
		// server port
		if (dev->srvport[0]) 
			includeNbsp(buf,dev->srvport,5); 
		else 
			includeNbsp(buf,0,5); 
		gstrcat(bufHtml,buf);
		gstrcat(bufHtml,"&nbsp;");
		
		// kvm port
		if (dev->kvmport[0]) 
			includeNbsp(buf,dev->kvmport,19); 
		else 
			includeNbsp(buf,0,19); 
		gstrcat(bufHtml,buf);
		
		gstrcat(bufHtml,"</option>\n");
	}
	if (debug_flag) printf("rdpdevbuf size %d [%s]\n", strlen(bufHtml), bufHtml);
	return 1;
}

int RDPMountDeviceTable()
{
	char_t *bufHtml;
	int x = 2;
	RDPDevConfig *dev;

	bfreeSafe(B_L,gCache.rdpdevice.deviceTable);
	x += gCache.rdpdevice.numDevices * (kRDPGenLength+1);
	bufHtml = gCache.rdpdevice.deviceTable = balloc(B_L, x);
	if (! bufHtml) {
		return 0;
	}
	memset(bufHtml,0,x);

	for (x=0 ; x < gCache.rdpdevice.numDevices; x++) {
		dev = &gCache.rdpdevice.deviceList[x];
		gstrcat(bufHtml, "|");
		gstrcat(bufHtml,dev->name);
	}
	if (debug_flag) printf("rdpdevbuf size %d [%s]\n", strlen(bufHtml), bufHtml);
	return 1;
}

Result LoadRDPDevices(void)
{
	int size;
	Result result = kCycSuccess;

	bfreeSafe(B_L,gCache.rdpdevice.deviceList);
	
	size = kMaxRDPServers * sizeof(RDPDevConfig);
	gCache.rdpdevice.deviceList = balloc(B_L, size);
	if (! gCache.rdpdevice.deviceList) {
		return kMemAllocationFailure;
	}
	memset((char*)gCache.rdpdevice.deviceList,0, size);
	{
		CycParam values[] = 
		{
			{ kRDPDevices, kRDPDevConf, &(gCache.rdpdevice), 0}
		};
		result = GetCycParamValues(values,1);
	}

	strcpy(gCache.rdpdevice.deviceMsgError,"OK");
	return result;
}

extern int GetRDPDevice(Param *param, int request)
{
	Result result;
	
    if ((result = GetCycUserType(1)) != kCycSuccess)
        return mAppError(result);

	result = LoadFirst(LoadRDPDevices, kRDPServers, &gCache.rdpdevice,NULL,sizeof(RDPDevice));

	if (result != kCycSuccess) return mAppError(result);

	if (! RDPMountDeviceHTMLTable()) {
		return mAppError(kMemAllocationFailure);
	}
	if (! RDPMountDeviceTable()) {
		return mAppError(kMemAllocationFailure);
	}

	return mAppError(result);
}

extern int GetRDPDeviceSel(Param *param, int request)
{
	Result result = kCycSuccess;
	RDPDevConfig *dev;
	
	if (gCacheLoaded[kRDPServers] == false) {
		return(0); 	// devices configuration did not load
	}
	if (gCache.rdpdevice.deviceSelected == 0) {
		memset(&gCache.rdpdevice.confDevice,0,sizeof(RDPDevConfig));
		return(0); 	// device not selected
	}
	
	if (gCache.rdpdevice.deviceSelected > gCache.rdpdevice.numDevices) {
		return(0);
	}
	
	dev = &gCache.rdpdevice.deviceList[gCache.rdpdevice.deviceSelected - 1];
	memcpy((char*)&gCache.rdpdevice.confDevice,(char*)dev,sizeof(RDPDevConfig));	
	return mAppError(result);
}

extern int RDPDeviceCommit(Param * param, int request, char_t *urlQuery)
{
	int num=0, change=0, ix;
	char devCs[10];
	Result result = kCycSuccess;
	RDPDevConfig *dev;

	if ((result = CycCheckUser(&gReq.sid, &wUser, 1)) != kCycSuccess)
		return mAppError(result);

	if (checkCancelRequest(request, -1))
		return mAppError(result);

	if (!gCacheLoaded[kRDPServers]) return(0);

	strcpy(gCache.rdpdevice.deviceMsgError,"OK");

	if (strncmp(gCache.rdpdevice.rdppfile, gCache.rdpdevice.rdpport, kRDPPortLength)) {
		strncpy(gCache.rdpdevice.rdppfile, gCache.rdpdevice.rdpport, kRDPPortLength);
		change = 1;
	}

	for (ix=0, num=1; ix < gCache.rdpdevice.numDevices; ix++, num++) {
		sprintf(devCs,",%d,",num);
		if (!strstr(gCache.rdpdevice.deviceCs,devCs)) {
		// the device was deleted
			change = 1;
			dev = &gCache.rdpdevice.deviceList[ix];
			if (ix < (gCache.rdpdevice.numDevices-1)) {
				memcpy((char*)dev, (char*)(dev+1),
				sizeof(RDPDevConfig) * (gCache.rdpdevice.numDevices - ix -1));
				ix--;
			}
			if (gCache.rdpdevice.deviceSelected == gCache.rdpdevice.numDevices)
				gCache.rdpdevice.deviceSelected--;

			gCache.rdpdevice.numDevices--;
			memset(&gCache.rdpdevice.deviceList[gCache.rdpdevice.numDevices],
					0, sizeof(RDPDevConfig));
		}
	}

	switch (gCache.rdpdevice.deviceAction) {
		case kAdd : // add operation
			if (RDPCheckDeviceName(gCache.rdpdevice.confDevice.name)) {
				strcpy(gCache.rdpdevice.deviceMsgError,"Name is already in use");
				break;
			}
			if (gCache.rdpdevice.numDevices < kMaxRDPServers) {
				dev = &gCache.rdpdevice.deviceList[gCache.rdpdevice.numDevices];
				gCache.rdpdevice.numDevices++;
				memcpy((char*)dev, (char*)&gCache.rdpdevice.confDevice,sizeof(RDPDevConfig));
				change = 1;
			} else {
				strcpy(gCache.rdpdevice.deviceMsgError,"Cannot add more servers. Limit exceeded.");
			}
			break;

		case kEdit : // edit operation
			num = gCache.rdpdevice.deviceSelected - 1;
			dev = &gCache.rdpdevice.deviceList[num];
			change = 1;
			if (strcmp(dev->name, gCache.rdpdevice.confDevice.name)) { // changed the device
				if (RDPCheckDeviceName(gCache.rdpdevice.confDevice.name)) {
					strcpy(gCache.rdpdevice.deviceMsgError,"Name is already in use");
					break;
				}
			}
			memcpy((char*)dev, (char*)&gCache.rdpdevice.confDevice,sizeof(RDPDevConfig));
			break;

	}
	
	gCache.rdpdevice.changed |= change;
	gLedState |= change;

	if (gReq.action == kNone) {
		SetLedChanges(request, gLedState);
		ResetSidEtc(request);
		return mAppError(result);
	}
	
	return expertCommit(param, request, urlQuery);
}

int SetRDPservers(void)
{
	if (gCacheLoaded[kRDPServers] == false) return(0);

	if (gCache.rdpdevice.changed == 1) {
		if (gReq.action != kNone) {
			RegisterParamChange(0,&gCache.rdpdevice,kRDPDevices,kRDPDevConf, 0);
		}
		gLedState = 1;
	}
	return (0);
}

#endif	//RDPenable
extern int GetMaxOutlet(Param *param, int request)
{
    Result result;
    CycParam counts[] =
    {
        { kKVM, kKVMMaxOutlet, &(gCache.kvmdevice.maxOutlet), 0 },
    };

    if ((result = GetCycUserType(0)) != kCycSuccess)
        return mAppError(result);

    result = GetCycParamValues(counts, mArrayLength(counts));
    return result;
}
