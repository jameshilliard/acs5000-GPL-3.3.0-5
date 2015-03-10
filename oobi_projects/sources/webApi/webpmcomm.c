# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <errno.h>
# include <unistd.h>
# include <termios.h>
# include <linux/ioctl.h>
# include <stdlib.h>
# include <signal.h>
# include <string.h>
# include <ctype.h>

#include <webApi.h>
#include <src/server.h>
#include "webpmcomm.h"
#include "webpm.h"

#ifdef PMDNG
#define HAVE_SEMUN_UNION
#include <pmclib.h>
#include <SystemData.h>
#endif

char *pmcCommand[] = {PMCOMM_OFF, PMCOMM_ON, PMCOMM_UNLOCK, PMCOMM_LOCK, 
	PMCOMM_CYCLE};

static int cycAnsError = 0;

void debugTreatAnswer(char *answ)
{
	//syslog(4,"%s: answer[%s]",__func__,answ);
}

void cycPMDngTreatAnswerUpg(char *answ)
{
//syslog(4,"%s: answ[%s]",__func__,answ);
	if (strstr(answ,"[Error]")) {
		cycAnsError=1;
	} else {
		cycAnsError=0;
	}
}

void cycPMDngTreatAnswerName(char *answ)
{
//syslog(4,"%s: answ[%s]",__func__,answ);
	if (strstr(answ,"Outlet now named")) {
		cycAnsError=0;
	} else {
		cycAnsError=1;
	}
}

int cycPMDngcheck_SPC(void * out)
{
//syslog(4,"%s: out",__func__);
	return (pmc_check_outlet_SPC_ServerTech(out));
}

int cycPMDngcheckGroup_SPC(PmIpduOutGrpInfo * groupInfo)
{
        int ret = 0, i;
        PmOutletInfo *outletInfo = groupInfo->outletInfo;
//syslog(4,"%s: called",__func__);

        for (i=0; i < groupInfo->numOutlets ; i++, outletInfo++) {
                if (pmc_check_outlet_SPC_ServerTech(outletInfo->shmRef))
                        return 1;
        }

        return (ret);
}

int cycPMDngSaveCmd(char_t *username, void *ipduRef)
{

//syslog(4,"%s: called",__func__);
	if (pmc_sendcmd_ipdu((ipduInfo**)&ipduRef, 1, 
		COMMAND_SAVE,username[0] ? username : NULL,
		NULL) == 0)
		return 1;
	return 0;
}

int cycPMDngSaveCmd2(char_t *username, PmIpduInfo *info )
{
//syslog(4,"%s: called",__func__);
	if (pmc_sendcmd_ipdu((ipduInfo**)&info->shmRef, 1, 
		COMMAND_SAVE,username[0] ? username : NULL,
		NULL) == 0)
                return 1;
        return 0;
}

void * cycPMDngOutletPtr(char_t *username, char * ipduId, int outN)
{
	oData * outRef = NULL;

//syslog(4,"%s: user[%s] ipduid[%s] outN[%d]",__func__,username,ipduId,outN);
	outRef = pmc_validate_outlet_ipdu(ipduId,outN,username[0]?username:NULL);

	return outRef;
}

int cycPMDngChangeState(char_t *username, void * outRef, CycPmOutletCommand command)
{
	int action;

//syslog(4,"%s: called",__func__);
	switch(command) {
	case kPmOn:
		action = OUTLET_CONTROL_ON;
		break;
	case kPmOff:
		action = OUTLET_CONTROL_OFF;
		break;
	case kPmLock:
		action = OUTLET_CONTROL_LOCK;
		break;
	case kPmUnlock:
		action = OUTLET_CONTROL_UNLOCK;
		break;
	case kPmCycle:
		action = OUTLET_CONTROL_CYCLE;
		break;
	default:
		return 1;
	}

	if (pmc_sendcmd_outlet(action, SINGLE_OUTLET, outRef, 
		username[0] ? username : NULL,
		NULL) == 0)
		return 1;
	return 0;
}

int cycPMDngConfOutName(char_t *username, void *outRef, char_t *outletName)
{
//syslog(4,"%s: called",__func__);
	if (!strcmp(((oData*)outRef)->name,outletName)) return 0;

	if ((strlen(outletName) > 8) && 
	    ((oData*)outRef)->ipdu->pmInfo->ipdu_type == IPDU_TYPE_SPC) {
		return 1; // erro size of name
	}

	if (pmc_validate_outlet_name(outletName,NULL)) {
		return 2; // name duplicated
	}

	cycAnsError = 0;  //[RK]Jan/31/07
	if (pmc_sendcmd_outcfg_name(outRef, outletName,
		username[0] ? username : NULL,
		cycPMDngTreatAnswerName) == 0)
		return 1;
	return cycAnsError;
}

int cycPMDngConfThreOut(char_t *username, void * out, double thre_hc,double thre_hw, double thre_lw,double thre_lc)
{
	oData *outRef = (oData *) out;
	//syslog(4, "%s: user[%s] hc[%.1f] lc[%.1f] hw[%.1f] lw[%.1f]", __FUNCTION__,username,thre_hc,thre_lc,thre_hw,thre_lw);

	if (thre_hc != outRef->elecMon.crit_threshold) {
		pmc_sendcmd_outcfg_alarm(SINGLE_OUTLET,outRef, ELECMON_CAP_CUR_CRIT_THRES,
					thre_hc,
					(username[0])?username:NULL,NULL);
	} 
	if (thre_lc != outRef->elecMon.lowcrit_threshold) {
		pmc_sendcmd_outcfg_alarm(SINGLE_OUTLET,outRef, ELECMON_CAP_CUR_LOWCRIT_THRES,
					thre_lc,
					(username[0])?username:NULL,NULL);
	} 
	if (thre_hw != outRef->elecMon.warn_threshold) {
		pmc_sendcmd_outcfg_alarm(SINGLE_OUTLET,outRef, ELECMON_CAP_CUR_WARN_THRES,
					thre_hw,
					(username[0])?username:NULL,NULL);
	} 
	if (thre_lw != outRef->elecMon.low_threshold) {
		pmc_sendcmd_outcfg_alarm(SINGLE_OUTLET,outRef, ELECMON_CAP_CUR_LOW_THRES,
					thre_lw,
					(username[0])?username:NULL,NULL);
	} 
	return 0;
}

int cycPMDngConfIntervals(char_t *username, void *outRef, double interval, int type)
{

//syslog(4,"%s: interval[%.1f] type[%d]",__func__,interval,type);
	if (!((oData*)outRef)->cap & type) {
		return 0;
	}

	switch (type) {
		case OUTLET_CAP_MINIMUMON :
			if (((oData *)outRef)->minimum_on == (int) interval) {
				return 0;
			}
			break;
		case OUTLET_CAP_MINIMUMOFF :
			if (((oData *)outRef)->minimum_off == (int) interval) {
				return 0;
			}
			break;
		case OUTLET_CAP_POSTPOWERON :
			if (((oData *)outRef)->post_power_on == interval*10) {
				return 0;
			}
			break;
		case OUTLET_CAP_POSTPOWEROFF : 
			if (((oData *)outRef)->post_power_off == interval*10) {
				return 0;
			}
			break;
	}

	if (pmc_sendcmd_outcfg_interval(
		interval, SINGLE_OUTLET, outRef, type,
		username[0] ? username : NULL,
		NULL) == 0)
		return 1;

	return 0;
}

int cycPMDngConfWakeUpState(char_t *username, void *outRef, PmWakeUpState state)
{
//syslog(4,"%s: called",__func__);
	if (!((oData*)outRef)->cap & OUTLET_CAP_WAKEUP) {
		return 0;
	}

	if (((oData *)outRef)->wakeup == (char) state) {
		return 0;
	}

	switch(state) {
	case PmWuStateLast:
	case PmWuStateOff:
	case PmWuStateOn:
		break;
	default:
		return 1;
	}

	if (pmc_sendcmd_outcfg_wakeup(
			SINGLE_OUTLET, outRef, state, 
			username[0] ? username : NULL,
			NULL) == 0)
		return 1;
	return 0;
}

int cycPMDngOutGrpChangeState(char_t *username, char_t *groupname, CycPmOutletCommand command)
{
	int action;
	char grp[30];
	oData **outlist;

//syslog(4,"%s: called",__func__);
	memset(grp,0,30);
	grp[0]='$';
	strncat(grp,groupname,28);	

	switch(command) {
	case kPmOn:
		action = OUTLET_CONTROL_ON;
		break;
	case kPmOff:
		action = OUTLET_CONTROL_OFF;
		break;
	case kPmLock:
		action = OUTLET_CONTROL_LOCK;
		break;
	case kPmUnlock:
		action = OUTLET_CONTROL_UNLOCK;
		break;
	case kPmCycle:
		action = OUTLET_CONTROL_CYCLE;
		break;
	default:
		return 0;
	}

	if (!(outlist = pmc_validate_multout_name(grp,
				  username[0] ? username : NULL)))
		return 0;

	if (pmc_sendcmd_outlet(
		action, OUTLET_GROUP, outlist, 
		username[0] ? username : NULL,
		NULL) == 0)
		return 0;
	while (*outlist) {
		if (((*outlist)->ipdu->pmInfo->ipdu_type == IPDU_TYPE_SPC) ||
		    ((*outlist)->ipdu->pmInfo->ipdu_type == IPDU_TYPE_SERVERTECH))
			return 2;
		outlist++;
	}

	return 1;
}

int cycPMDngClearMaxDetected(char* username, void *ipduRef, int action)
{
	int sensor=0;

//syslog(4,"%s: action[%d]",__func__,action);
	switch(action) {
	case Clear_Current:
		sensor = PROTO_ELEMENT_ELECMON_CURRENT;
		break;
	case Clear_EnvMon:
		if (pmc_sendcmd_envmon_reset_all((ipduInfo*)ipduRef,username[0] ? username : NULL,NULL)==0) 
			return 1;	
		break;
	case Clear_Power:
		sensor =  PROTO_ELEMENT_ELECMON_POWER; 
		break;
	case Clear_Voltage:
		sensor = PROTO_ELEMENT_ELECMON_VOLTAGE; 
		break;
	case Clear_PowerFactor:
		sensor =  PROTO_ELEMENT_ELECMON_POWER_FACTOR; 
		break;
	case Reset_HWOvercurrent:
		if (pmc_sendcmd_ipdu_hwocp((ipduInfo*)ipduRef,username[0] ? username : NULL,NULL)==0) 
			return 1;	
		break;
	default:
		return 1;
	}

	if (sensor) {
		if (pmc_sendcmd_ipdu_reset_old((ipduInfo**)&ipduRef, 1, sensor,
				   username[0] ? username : NULL,
				   NULL) == 0) {
			return 1;
		}
	}
	return 0;
}

char *cycPMDngGetIpduName(PmIpduInfo *info)
{
//syslog(4,"%s: called",__func__);
	return ((ipduInfo *)(info->shmRef))->id;
}

int cycPMDngConfIpduName(char *username, PmIpduInfo *info)
{
//syslog(4,"%s: called",__func__);
	info->name[16] = '\0';
	if (pmc_sendcmd_ipducfg_id(info->shmRef, info->name,
			username[0] ? username : NULL,
			NULL) == 0)
		return 1;
	return 0;
}

int cycPMDngConfIpduOnOffparams(char *username, PmIpduInfo *info)
{
//syslog(4,"%s: called",__func__);
	if (info->changed & IPDU_CHG_OCP) {
		if (pmc_sendcmd_ipducfg_onoff((ipduInfo**)&info->shmRef, 1,
				IPDU_CAP_OVERCURRENT,
				(PMC_Boolean)info->overcurrent,
				username[0] ? username : NULL,
				NULL) == 0)
			return 1;
	}
	if (info->changed & IPDU_CHG_SYSLOG) {
		if (pmc_sendcmd_ipducfg_onoff((ipduInfo**)&info->shmRef, 1,
				IPDU_CAP_SYSLOG,
				(PMC_Boolean)info->syslog,
				username[0] ? username : NULL,
				NULL) == 0)
			return 1;
	}
	if (info->changed & IPDU_CHG_BUZZER) {
		if (pmc_sendcmd_ipducfg_onoff((ipduInfo**)&info->shmRef, 1,
				IPDU_CAP_BUZZER,
				(PMC_Boolean)info->buzzer,
				username[0] ? username : NULL,
				NULL) == 0) 
			return 1;
	}

	return 0;
}

unsigned char *ReadFile(unsigned char *filename, int write);

int cycPMDngConfIpduParams(PmIpduPortInfo *port, int nports)
{
	char * buff, *aux;
	FILE *fd;
	PmIpduInfo *pis;
	int i, j, k, newentry=0,count;
	PmElecMonInfo *pelec;

//syslog(4,"%s: called",__func__);
	buff = ReadFile(PMDCONF,0);

	if (!(fd = fopen(PMDCONF,"w"))) return 1;
	aux = strstr(buff,"[IPDU]");
	if (aux == NULL) {
		newentry = 1;
		aux = buff;
	}
	// copy the first sessions
	fwrite(buff,aux-buff,1,fd);
	fprintf(fd,"[IPDU]\n");
	for (i=0; i<nports; i++, port++) {
		pis = port->ipdu;
		for (j=0; j<port->numIpdu; j++, pis++) {
			if (pis->poll_rate >= 500) {
				fprintf(fd,"%s.pollrate %d\n",pis->name,pis->poll_rate);
			}
			if (pis->def_voltage > 0) {
				fprintf(fd,"%s.defvoltage %d\n",pis->name,(int)(pis->def_voltage/10));
			}
			if (pis->pwr_factor >= 0 && pis->pwr_factor <= 100) {
				fprintf(fd,"%s.pwrfactor %f\n",pis->name,pis->pwr_factor/100);
			}
			if (pis->type != Vendor_cyclades) {
				if (pis->num_inlets == 3) {
					if (pis->num_phases == 1 || pis->num_phases == 3) {
						fprintf(fd,"%s.numphases %d\n",pis->name,pis->num_phases);
					}
					if (pis->firstPhaseInfo != NULL) {
						pelec = pis->firstPhaseInfo;
					} else {
						pelec = pis->firstBankInfo;
					}
					count = 3;
				} else {
					if (pis->num_phases == 3) {
						pelec = pis->firstPhaseInfo;
						count = 3;
					} else {
						pelec = pis->firstBankInfo;
						count = (pis->num_banks)?pis->num_banks:1;
					}
				}
				if (count > 1) {
					for (k=0; k<count; k++,pelec++) {
						if (pelec->current.thre_hc != 999999)
							fprintf(fd,"%s[%d].loadmax %f\n",pis->name,k+1,pelec->current.thre_hc/10);
						if (pelec->current.thre_lc != 999999)
							fprintf(fd,"%s[%d].loadmin %f\n",pis->name,k+1,pelec->current.thre_lc/10);
					}
				} else {
					fprintf(fd,"%s.loadmax %.1f\n",pis->name,pis->elecPduInfo.current.thre_hc/10);
					fprintf(fd,"%s.loadmin %.1f\n",pis->name,pis->elecPduInfo.current.thre_lc/10);
				}
			}
		}
	}
	if (!newentry) { 
		aux+=6;
		aux = strstr(aux,"\n[");
	}
	if (aux != NULL) {
		//fprintf(fd,"%s",++aux);
		fprintf(fd,"%s",aux);
	}	
	free(buff);
	fclose(fd);

	system("/bin/daemon.sh restart PMD"); // restart PMD to get the new file

	sleep(5); // wait 5 sec for PMD restart

	return 0;
}

int cycPMDngConfIpduIntervals(char *username, PmIpduInfo *info)
{
//syslog(4,"%s: called",__func__);
	if (info->changed & IPDU_CHG_CYCLEINT) {
		if (pmc_sendcmd_ipducfg_interval(info->shmRef,
				IPDU_CAP_REBOOTDELAY,
				info->cycle_int,
				username[0] ? username : NULL,
				NULL) == 0)
			return 1;
	}
	if (info->changed & IPDU_CHG_SEQINT) {
		if (pmc_sendcmd_ipducfg_interval(info->shmRef,
				  IPDU_CAP_SEQINTERVAL,
				  info->seq_int,
				  username[0] ? username : NULL,
				  NULL) == 0)
			return 1;
	}
	if (info->changed & IPDU_CHG_COLDSTART) {
		if (pmc_sendcmd_ipducfg_interval(info->shmRef,
				  IPDU_CAP_COLDSTARTDELAY,
				  info->cold_start,
				  username[0] ? username : NULL,
				  NULL) == 0)
			return 1;
	}
	if ((info->changed & IPDU_CHG_DISPLAY) ||
	    (info->changed & IPDU_CHG_DISPLAY_CYCLE))  {
		if (pmc_sendcmd_ipducfg_display(info->shmRef,
				  info->display,
				  info->display_cycle,
				  username[0] ? username : NULL,
				  NULL) == 0)
			return 1;
	}
	
	return 0;
}

int cycPMDngConfIpduPduThre(char *username, PmIpduInfo *info)
{
	PmCurrInfo * pcur=&info->elecPduInfo.current;
	ipduInfo *pdu = (ipduInfo *)info->shmRef;
	
	if (pcur->changed == 0) return 0;
	if (pcur->changed & 0x0001) {
		pmc_sendcmd_eleccfg_alarm( pdu, ELEMENT_IPDU,
			ELECMON_CAP_CUR_CRIT_THRES,
			pcur->thre_hc,
			NULL,
			NULL);
	}
	if (pcur->changed & 0x0002) {
		pmc_sendcmd_eleccfg_alarm( pdu, ELEMENT_IPDU,
			ELECMON_CAP_CUR_LOWCRIT_THRES,
			pcur->thre_lc,
			NULL,
			NULL);
	}
	if (pcur->changed & 0x0004) {
		pmc_sendcmd_eleccfg_alarm( pdu, ELEMENT_IPDU,
			ELECMON_CAP_CUR_WARN_THRES,
			pcur->thre_hw,
			NULL,
			NULL);
	}
	if (pcur->changed & 0x0008) {
		pmc_sendcmd_eleccfg_alarm( pdu, ELEMENT_IPDU,
			ELECMON_CAP_CUR_LOW_THRES,
			pcur->thre_lw,
			NULL,
			NULL);
	}
	return 0;
}

int cycPMDngConfIpduPhaseThre(char *username, PmIpduInfo *info)
{
	ipduInfo *pdu = (ipduInfo *)info->shmRef;
	phaseInfo *phase;
	PmCurrInfo * pcur;
	int i;
	
//syslog(4,"%s: called",__func__);
	for (i=0; i < pdu->num_phases ; i++) {
		pcur =&info->firstPhaseInfo[i].current;
//syslog(4,"%s: phase[%d] changed[%x]",__func__,i,pcur->changed);
		if (pcur->changed == 0) continue;
		phase = &pdu->phase[i];
		if (pcur->changed & 0x0001) {
			pmc_sendcmd_eleccfg_alarm( phase, ELEMENT_PHASE,
				ELECMON_CAP_CUR_CRIT_THRES,
				pcur->thre_hc,
				NULL,
				NULL);
		}
		if (pcur->changed & 0x0002) {
			pmc_sendcmd_eleccfg_alarm( phase, ELEMENT_PHASE,
				ELECMON_CAP_CUR_LOWCRIT_THRES,
				pcur->thre_lc,
				NULL,
				NULL);
		}
		if (pcur->changed & 0x0004) {
			pmc_sendcmd_eleccfg_alarm( phase, ELEMENT_PHASE,
				ELECMON_CAP_CUR_WARN_THRES,
				pcur->thre_hw,
				NULL,
				NULL);
		}
		if (pcur->changed & 0x0008) {
			pmc_sendcmd_eleccfg_alarm( phase, ELEMENT_PHASE,
				ELECMON_CAP_CUR_LOW_THRES,
				pcur->thre_lw,
				NULL,
				NULL);
		}
	}
	return 0;
}

int cycPMDngConfIpduBankThre(char *username, PmIpduInfo *info)
{
	ipduInfo *pdu = (ipduInfo *)info->shmRef;
	circuitInfo *circ;
	PmCurrInfo * pcur;
	int i;
	
//syslog(4,"%s: called",__func__);
	for (i=0; i < pdu->num_circuits ; i++) {
		pcur =&info->firstBankInfo[i].current;
//syslog(4,"%s: circuit[%d] changed[%x]",__func__,i,pcur->changed);
		if (pcur->changed == 0) continue;
		circ = &pdu->circuit[i];
		if (pcur->changed & 0x0001) {
			pmc_sendcmd_eleccfg_alarm( circ, ELEMENT_CIRCUIT,
				ELECMON_CAP_CUR_CRIT_THRES,
				pcur->thre_hc,
				NULL,
				NULL);
		}
		if (pcur->changed & 0x0002) {
			pmc_sendcmd_eleccfg_alarm( circ, ELEMENT_CIRCUIT,
				ELECMON_CAP_CUR_LOWCRIT_THRES,
				pcur->thre_lc,
				NULL,
				NULL);
		}
		if (pcur->changed & 0x0004) {
			pmc_sendcmd_eleccfg_alarm( circ, ELEMENT_CIRCUIT,
				ELECMON_CAP_CUR_WARN_THRES,
				pcur->thre_hw,
				NULL,
				NULL);
		}
		if (pcur->changed & 0x0008) {
			pmc_sendcmd_eleccfg_alarm( circ, ELEMENT_CIRCUIT,
				ELECMON_CAP_CUR_LOW_THRES,
				pcur->thre_lw,
				NULL,
				NULL);
		}
	}
	return 0;
}

int cycPMDngConfIpduEnvMonThre(char *username, PmIpduInfo *info)
{
	ipduInfo *pdu = (ipduInfo *)info->shmRef;
	envMonInfo *env;
	PmEnvMonInfo *penv;
	int i;
	
//syslog(4,"%s: called",__func__);
	for (i=0; i < pdu->num_envMons ; i++) {
		penv =&info->firstEnvMonSensor[i];
//syslog(4,"%s: envmon[%d] changed[%x]",__func__,i,penv->changed);
		if (penv->changed == 0) continue;
		env = &pdu->envMon[i];
		if (penv->changed & 0x0001) {
			pmc_sendcmd_envcfg_alarm( env, 
				ENVMON_CAP_CRIT_THRES,
				penv->thre_hc,
				NULL,
				NULL);
		}
		if (penv->changed & 0x0002) {
			pmc_sendcmd_envcfg_alarm( env,
				ENVMON_CAP_LOW_THRES,
				penv->thre_lc,
				NULL,
				NULL);
		}
		if (penv->changed & 0x0004) {
			pmc_sendcmd_envcfg_alarm( env,
				ENVMON_CAP_CRIT_WARN_THRES,
				penv->thre_hw,
				NULL,
				NULL);
		}
		if (penv->changed & 0x0008) {
			pmc_sendcmd_envcfg_alarm( env,
				ENVMON_CAP_LOW_WARN_THRES,
				penv->thre_lw,
				NULL,
				NULL);
		}
	}
	return 0;
}

int cycPMDngUpgradeFirmware(char* username, UInt32 serialPort, UInt32 ipduNumber,
			char * filever, char *ipduver)
{
	ipduInfo *pdu;
	char str_pdu[10];

//syslog(4,"%s: called",__func__);
	if (shmp->fw_lock) {
		return 1;
	}
	
	cycAnsError=0;  //[RK]Jan/31/07

	//[RK]Nov/06/06 - event notification
        //pmc_writeevt(32,"uiiss",serialPort,ipduNumber,ipduver,filever);

	pdu = pmc_validate_ipdu_old(serialPort,ipduNumber,NULL);

	if (!pdu) return 1;

	sprintf(str_pdu,"%d",ipduNumber);
	
	if (pmc_sendcmd_ipdu_fwupgrade(pdu,
				str_pdu,
				NULL,
				username[0] ? username : NULL,
				cycPMDngTreatAnswerUpg) == 0) {
		//[RK]Nov/06/06 - event notification
         //       pmc_writeevt(33,"iiis", serialPort,ipduNumber,2,filever);
		return 1;
	}
	//[RK]Jan/31/07 - check upgrade error
//	if (cycAnsError) { // error
 //               pmc_writeevt(33,"iiis", serialPort,ipduNumber,2,filever);
//	} else {
//		//[RK]Nov/06/06 - event notification
 //       	pmc_writeevt(33,"iiis",serialPort, ipduNumber,1,filever);
//	}
	return cycAnsError;
}

char_t *cycPMDngGetOutName (void *outRef)
{
//syslog(4,"%s: called",__func__);
	return (((oData *)outRef)->name);
}
