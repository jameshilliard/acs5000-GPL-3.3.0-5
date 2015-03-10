/*****************************************************************
* File: pmc_sendcmd.c
*
* Copyright (C) 2006 Avocent Corporation
*
* www.avocent.com
*
* Description:
*     This file has the send command routines of the PM Client
*     Library.
*
*****************************************************************/
#define PMCLIB_SRC
#define SHM_FREE_STR
#define SHM_MALLOC
#include <pmclib.h>
#include <math.h>
#include <pmc_show.h>

char *warn_pending = "[WARN] Outlet operations submitted but the response is pending. Check the status of the outlets.";
char *warn_upgrade_pending = "[WARN]Upgrade successfully started, but the response is pending.";

#define PMC_CAP_OUTLET              1
#define PMC_CAP_ELECMON             2
#define PMC_CAP_ELECMON_THRESHOLDS  3

// global variables:
static char * ctrl_user = NULL;
static int ctrl_cap;
static int ctrl_capgroup;
static int ctrl_ltype;
static int ctrl_cmd;
static int total_delay_outcmd;

// ctrl_capgroup =  PMC_CAP_ELECMON_THRESHOLDS, 
//		    PMC_CAP_ELECMON,  PMC_CAP_OUTLET
// ctrl_ltype = SINGLE_OUTLET, OUTLET_GROUP, OUTLET_LIST,  
//              OUTLIST_IPDU, OUTLIST_CHAIN, OUTLIST_ALL

void (*glb_treatAnswer)(char*);

int get_pid()
{
	FILE *fp;
	int ret=0;

	if ((fp = fopen(PMDPIDFILE, "r")) != NULL) {
		fscanf(fp, "%d", &ret);
		fclose(fp);
	}
	return(ret);
}


/* power_send_command_req 
 * copy from gsp-power-api: shm-power-request.c
 * merge with old pmc_cmdreq_pmd
 *
 * Send the command request to PMD core and wait the answer.
 * return : 1 - if SUCCESS
 *          0 - the request fail.
 * Parameters :
 *  command - the command code
 *  command_args - buffer with the command mounted (protocol).
 */

#define OUTLET_CMD_TO    30 // Timeout for outlet commands is 30 sec
#define UPGRADE_CMD_TO  100 // Timeout for upgrade command

int power_send_command_req (int admin_power_cmd, char *command_args)
{
	time_t time_ini, now, time_outlet, time_upgrade;
	answer * pans, * pnext, * pqueue = NULL;
	request *new_req, *prev;
	int oldPMDpid = 0, PMDpid = 0, subjectsize, altsize=0;
	char param = 0, cmdout = 0;
	static char altbuf[PMC_ARGSIZE];

	// Create a new request on shm
	new_req = (request *) pmshm_malloc(sizeof(request), TYPE_SHM_STRING);

	// Fill out the request on shared memory
	new_req->command_id = admin_power_cmd;
	new_req->slot = -1;
	memcpy(new_req->command_args, command_args, PMC_ARGSIZE);

	param = command_args[0];
	if ((admin_power_cmd == COMMAND_OUTLET_OPERATION) &&
	    (param == PROTO_ELEMENT_OUTLET_CONTROL_ID)) {
		cmdout = command_args[2];
	}

//syslog(4,"%s: command[%d] proto[0x%x]",__func__,admin_power_cmd,param);

	if (ctrl_user != NULL) {
		strncpy(new_req->username, ctrl_user, USERNAMESIZE);
	} else {
		strncpy(new_req->username, "(admin user)", USERNAMESIZE);
	}
	
	// Add request to the end of the queue
	WAIT_SHM;

	if (admin_power_cmd == COMMAND_UPGRADE) {
		time_upgrade = time(NULL);
	} else {
		time_upgrade = 0;
	}
	time_outlet = 0;
	switch (cmdout) {    //Immediate ON/OFF are not timed commands.
		case OUTLET_CONTROL_ON:
		case OUTLET_CONTROL_OFF:
		case OUTLET_CONTROL_CYCLE:
		case OUTLET_CONTROL_LOCK:
		case OUTLET_CONTROL_UNLOCK:
			time_outlet = time(NULL);
			break;
	}

	new_req->next = NULL;
	new_req->sequence = shmp->sequence++;

	if (time_outlet) {    //Using it as flag for outlet commands
		if (!shmp->req_outcmd) {
			shmp->req_outcmd = new_req;
		} else {
			prev = shmp->req_outcmd;
			while (prev->next)
				prev = prev->next;
			prev->next = new_req;
		}
		shmp->num_req_outcmd++;
		shmp->total_delay_outcmd += total_delay_outcmd;
	} else {    // Any other command.
		if (!shmp->requests) {
			shmp->requests = new_req;
		} else {
			prev = shmp->requests;
			while (prev->next)
				prev = prev->next;
			prev->next = new_req;
		}
		shmp->num_requests++;
	}

/*  for TEST... */
//	printf("----->> ");
//	int i, j;
//	printf("cmd_id:%d cmd_args:", new_req->command_id);
//	for (i=0; new_req->command_args[i]; i+=new_req->command_args[i+1])
//		for (j=0; j < new_req->command_args[i+1]; j++)
//			printf("%02x ", new_req->command_args[i+j]);
//	printf("\n");

	// Save the command sent since we may need to produce
	// an answer based on that (see below...)
	altbuf[0] = 0x00;
	for (subjectsize = 0; new_req->command_args[subjectsize] &&
			 new_req->command_args[subjectsize] != PROTO_ELEMENT_CHAIN_NUMBER;
		 subjectsize += new_req->command_args[subjectsize+1]) {
	}
	if (new_req->command_args[subjectsize]) {
		for (altsize = subjectsize;
			 new_req->command_args[altsize];
			 altsize+=new_req->command_args[altsize+1]) {
		}
		memcpy(altbuf, new_req->command_args+subjectsize, PMC_ARGSIZE-subjectsize);
		memcpy(altbuf+altsize-subjectsize, new_req->command_args, subjectsize);
	}

	POST_SHM;

	// Signal the thread pool

	POST_STA;

	if(admin_power_cmd == COMMAND_REBOOT){
		POST_SYNC(new_req->slot);
		return 1;
	}
	sched_yield();

	oldPMDpid = get_pid();
	time_ini = time(NULL);

	if (time_outlet && (shmp->total_delay_outcmd/10 >= OUTLET_CMD_TO)) {
		// call routine to send error warning.pending 
		if (glb_treatAnswer) glb_treatAnswer(warn_pending);
		return 1;
	}

	// wait PMD to get the command from the queue
	while (new_req->slot == -1) {
		now = time(NULL);
		if ((now < time_ini) || (now - time_ini >= 5)) {
			time_ini = now;
			// When no answer is received for 5 seconds 
			// check if PMD process is still active 
			PMDpid = get_pid();
			if ((PMDpid == -1) || (PMDpid != oldPMDpid)) {
				// PMD changed
				pmshm_free_str((char *)new_req);
				sleep(20); // some time for the new PMD to start
				return 0;
			}
		}
		if (time_upgrade) {
			int cur_time_upgrade;
			cur_time_upgrade = time(NULL);
			if ((cur_time_upgrade < time_upgrade) || 
			    (cur_time_upgrade - time_upgrade > UPGRADE_CMD_TO)){
				if (glb_treatAnswer) glb_treatAnswer(warn_upgrade_pending);
                                return 1;
                        }
                }

		if (time_outlet) {
			if ((now < time_outlet) || 
                	    (now - time_outlet >= OUTLET_CMD_TO)) {
				//Outlet command timed out. Warning only, don't set err.
				// send warning.pending ---
				if (glb_treatAnswer) glb_treatAnswer(warn_pending);
				return 1;
			}
		}

		sched_yield();
	}

	// wait to receive all responses from PMD core
	while (1) {

		time_ini = time(NULL);

		// wait to receive one response from PMD core
		while (1) {
			if (TRYLOCK_THREAD(new_req->slot) == 0){
				// receive one response
				break;
			}
			now = time(NULL);
			/* When no answer is received for 5 seconds, check if PMD process is still active */
			if ((now < time_ini) || (now - time_ini >= 5)) {
				time_ini = time(NULL);
		                PMDpid = get_pid();
				if ((PMDpid == -1) || (PMDpid != oldPMDpid)) {
					// PMD changed
					pmshm_free_str((char *)new_req);
					sleep(20); // some time for the new PMD to start
					return 0;
				}
			}

			if (time_upgrade) {
				int cur_time_upgrade;
				cur_time_upgrade = time(NULL);
				if ((cur_time_upgrade < time_upgrade) || 
				    (cur_time_upgrade - time_upgrade > UPGRADE_CMD_TO)){
					if (glb_treatAnswer) glb_treatAnswer(warn_upgrade_pending);
					return 1;
                        	}
                	}
			if (time_outlet) {
				if ((now < time_outlet) || (now - time_outlet >= OUTLET_CMD_TO)) {
					//Outlet command timed out. Warning only, don't set err.
					if (glb_treatAnswer) glb_treatAnswer(warn_pending);
					return 1;
				}
			}
			sched_yield();
		}

		if (! pqueue) {
			pans = new_req->answer_b;
		} else {
			pans = pqueue->next;
		}
		if (! pans) {
			//we need to try to produce some answer...
			if (altbuf[0])
				pmc_show_answer(altbuf, altsize, glb_treatAnswer, param);
			break;
		}
		pnext = pans->next;

		// process message
		//if (glb_treatAnswer != NULL) {
			pmc_show_answer(pans->buf, pans->size, glb_treatAnswer, param);
			altbuf[0] = 0x00;
		//}
		pqueue = pans;
		pans = pnext;
	}

	/* Free answer */
	for (pans = new_req->answer_b; pans != NULL ;) {
		pnext = pans->next;
		pmshm_free_str((char *)pans->buf);
		pmshm_free_str((char *)pans);
		pans = pnext;
	}
	new_req->answer_b = NULL;

	POST_SYNC(new_req->slot);

	return 1;
}

// poutlet: ptr outlet
// return: 1 - OK - outlet has the capability
//         0 - NOK - outlet does not have capability or user does not have permission
static int pmc_check_outcap(oData *pout)
{
	int err = 0, capmax = 0;
	char *argerr = NULL;
	
	if (!pout) return 0;

	if (ctrl_user && !pmc_check_userperm(pout, ctrl_user)) {
                err = OUTLERR_NOPERM;
		goto _CHECK_OUTCAP_ERROR;
        }

	if (ctrl_capgroup == 0) return 1;

	if (ctrl_capgroup == PMC_CAP_OUTLET) {
		if (!(pout->sflag & ctrl_cap)) {
			err = OUTLERR_CAP_NOTSUPPORTED;
			goto _CHECK_OUTCAP_ERROR;
		}
	
		if ((ctrl_cap == OUTLET_CAP_STATUS) &&
		    (pout->ipdu->sflag & IPDU_CAP_NOT_SWITCHABLE)) {
			err = OUTLERR_CAP_NOTSUPPORTED;
			goto _CHECK_OUTCAP_ERROR;
		}

		if ((ctrl_cap == OUTLET_CAP_STATUS) &&
		    ((ctrl_cmd == OUTLET_CONTROL_LOCK) || 
		     (ctrl_cmd == OUTLET_CONTROL_UNLOCK)) &&
		    (pout->ipdu->pmInfo->ipdu_type != IPDU_TYPE_CYCLADES)) {
			err = OUTLERR_CAP_NOTSUPPORTED;
			goto _CHECK_OUTCAP_ERROR;
		}
		if ( (ctrl_cap == OUTLET_CAP_WAKEUP) &&
		     (pout->ipdu->pmInfo->ipdu_type!=IPDU_TYPE_SERVERTECH) && 
		     (ctrl_cmd == WAKEUP_STATE_LAST)) {
			err = OUTLERR_CAP_NOTSUPPORTED;
			goto _CHECK_OUTCAP_ERROR;
                }
		if (ctrl_cap == OUTLET_CAP_STATUS) {
			// check cmd:ON/OFF/CYCLE --> lock/pend
			if (pout->pending) {
				err = OUTLERR_BUSY;
				goto _CHECK_OUTCAP_ERROR;
			}
			if ((pout->ipdu->pmInfo->ipdu_type==IPDU_TYPE_CYCLADES) &&
			    (ctrl_cmd != OUTLET_CONTROL_UNLOCK)) {
				if ((pout->status == OUTLET_STATUS_LOCK_OFF)||
				    (pout->status ==  OUTLET_STATUS_LOCK_ON)) {
					err = OUTLERR_LOCK;
					goto _CHECK_OUTCAP_ERROR;
				}
			}
			
		}
		return 1;
	}
	if (ctrl_capgroup == PMC_CAP_ELECMON) {
		switch (ctrl_cap) {
		case ELECMON_CAP_CURRENT:
			capmax = ELECMON_CAP_MAX_CURRENT;
			argerr = current_str;
			break;
		case ELECMON_CAP_VOLTAGE:
			capmax = ELECMON_CAP_MAX_VOLTAGE;
			argerr = voltage_str;
			break;
		case ELECMON_CAP_POWER:
			capmax = ELECMON_CAP_MAX_POWER;
			argerr = power_str;
			break;
		case ELECMON_CAP_POWER_FACTOR:
			capmax = ELECMON_CAP_MAX_POWER_FACTOR;
			argerr = powerFactor_str;
			break;
		default:
			err = OUTLERR_UNKNOWN_MEASUREMENT;
			goto _CHECK_OUTCAP_ERROR;
		}
		if (!(pout->elecMon.sflag & ctrl_cap)) {
			err = OUTLERR_MEASUREMENT_NOTSUPPORTED;
			goto _CHECK_OUTCAP_ERROR;
		}
		if (!(pout->elecMon.sflag & capmax)) {
			err = OUTLERR_MAXCAP_NOTSUPPORTED;
			goto _CHECK_OUTCAP_ERROR;
		}
		return 1;
	}	
	
	if (ctrl_capgroup == PMC_CAP_ELECMON_THRESHOLDS) {
		switch (ctrl_cap) {
		case ELECMON_CAP_CUR_CRIT_THRES:
			argerr = critHigh_str;
			break;
		case ELECMON_CAP_CUR_WARN_THRES:
			argerr = warnHigh_str;
			break;
		case ELECMON_CAP_CUR_LOW_THRES:
			argerr = warnLow_str;
			break;
		case ELECMON_CAP_CUR_LOWCRIT_THRES:
			argerr = critLow_str;
			break;
		default:
			err = OUTLERR_INVALID_THRESHOLD_ALARM;
			goto _CHECK_OUTCAP_ERROR;
		}
		if (!(pout->elecMon.sflag & ctrl_cap)) {
			err = OUTLERR_NO_SPEC_THRESHOLD_ALARM;
		} else {
			if (ctrl_cmd > pout->ipdu->max_current){
				err = OUTLERR_INVALID_THRESHOLD_ALARM;
			} else {
				return 1;
			}
		}
	}
_CHECK_OUTCAP_ERROR:
//syslog(4,"%s: capgrp[%d] cap[%x] err[%d] argerr[%s]",__func__,ctrl_capgroup,ctrl_cap,err,argerr);
	if ((ctrl_ltype < OUTLIST_IPDU) && (glb_treatAnswer != NULL)) {
		pmc_answer_outlet_error(pout, err, argerr, glb_treatAnswer);
	}
	return 0;
}

static void outlet_set_pending(oData * outl)
{
	switch (ctrl_cmd) {
	case OUTLET_CONTROL_ON:
		if (outl->status == OUTLET_STATUS_OFF ||
		    outl->status == OUTLET_STATUS_IDLE_OFF ||
		    outl->status == OUTLET_STATUS_WAKE_OFF) {
			outl->pending = outl->status;
			outl->status = OUTLET_STATUS_PEND_ON;
		}
		total_delay_outcmd += outl->post_power_on;
		break;
	case OUTLET_CONTROL_OFF:
		if (outl->status == OUTLET_STATUS_ON ||
		    outl->status == OUTLET_STATUS_IDLE_ON ||
		    outl->status == OUTLET_STATUS_WAKE_ON) {
			outl->pending = outl->status;
			outl->status = OUTLET_STATUS_PEND_OFF;
		}
		total_delay_outcmd += outl->post_power_off;
		break;
	case OUTLET_CONTROL_CYCLE:
		if (outl->status == OUTLET_STATUS_OFF ||
		    outl->status == OUTLET_STATUS_IDLE_OFF ||
		    outl->status == OUTLET_STATUS_WAKE_OFF) {
			outl->pending = outl->status;
			outl->status = OUTLET_STATUS_CYCLE;
		} else if (outl->status == OUTLET_STATUS_ON ||
			   outl->status == OUTLET_STATUS_IDLE_ON ||
			   outl->status == OUTLET_STATUS_WAKE_ON) {
				outl->pending = outl->status;
				outl->status = OUTLET_STATUS_PEND_CYCLE;
			}
		total_delay_outcmd += outl->post_power_on + outl->post_power_off;
		break;
	}
}

// copy from old pmc_buildcmd.c
// pbuf = buffer to add outlets in the protocol
// outlet = ptr outlets
static char *pmc_add_outlets (char *pbuf, void * outlet)
{
        pmInfo *pminfo = NULL;
        ipduInfo *pipdu = NULL, *ipdu = NULL;
        oData * pout=NULL, **outl=NULL;
        int index=0;
        int outlet_cur, outlet_ini = 0, outlet_end = 0;
        int port = -1;
        char *pini = pbuf;
        int outlets = 0, noutlet = 0;
	int flg_err = 0;

	total_delay_outcmd = 0;
	// pout: pointer first outlet
	switch (ctrl_ltype) {
		case SINGLE_OUTLET: /* a single outlet */
			pout = (oData *) outlet;
			break;
		case OUTLET_GROUP: // outlet group
		case OUTLET_LIST: /* a list of outlets */
			outl = (oData **) outlet;
			pout = outl[index++];
			break;
		case OUTLIST_IPDU: /* all outlets in the IPDU */
			ipdu = (ipduInfo *) outlet;
			if ((ipdu->num_outlets == 0) ||((ctrl_cap == OUTLET_CAP_STATUS) &&
			    (ipdu->sflag & IPDU_CAP_NOT_SWITCHABLE))) {
				pmc_answer_ipdu_error(ipdu, IPDUERR_IPDU_FIXEDON,glb_treatAnswer);
				return NULL;
			} else {
				pout = ipdu->outlet;
				noutlet = 0;
			}
			break;
		case OUTLIST_CHAIN: /* all outlets in the chain */
			pminfo = (pmInfo *) outlet;
			pout = NULL;
			for (ipdu = pminfo->ipdu; ipdu; ipdu = ipdu->next) {
				if (ipdu->ready && ipdu->license) {
					if ((ipdu->num_outlets == 0) || 
					    ((ctrl_cap == OUTLET_CAP_STATUS) &&
				    	     (ipdu->sflag & IPDU_CAP_NOT_SWITCHABLE))) {
						pmc_answer_ipdu_error(ipdu, IPDUERR_IPDU_FIXEDON,glb_treatAnswer);
						continue;
					}
					pout = ipdu->outlet;
					noutlet = 0;
					break;
				}
			}
			if (! pout) {
				return NULL;
			}
			break;
		case OUTLIST_ALL:
			pout = NULL;
			for (pminfo = shmp->pmInfo; pminfo; pminfo = pminfo->next) {
				for (ipdu = pminfo->ipdu; ipdu; ipdu = ipdu->next) {
					if (ipdu->ready && ipdu->license) {
						if ((ipdu->num_outlets == 0) || 
						    ((ctrl_cap == OUTLET_CAP_STATUS) &&
				    		     (ipdu->sflag & IPDU_CAP_NOT_SWITCHABLE))) {
							pmc_answer_ipdu_error(ipdu, IPDUERR_IPDU_FIXEDON,glb_treatAnswer);
							continue;
						}
						pout = ipdu->outlet;
						noutlet = 0;
						break;
					}
				}
				if (pout) {
					break;
				}
			}
			if (! pout) {
				return NULL;
			}
			break;
		default:
                	return NULL;
        }

        while (pout) {

		if (!pmc_check_outcap(pout)) {
			if (ctrl_ltype < OUTLET_LIST) {
				flg_err = 1;
			}
			goto _ADDOUT_NEXT;
		}
		
		if ((ctrl_ltype >= OUTLET_LIST) &&
		    (ctrl_capgroup == PMC_CAP_OUTLET) &&
		    (ctrl_cap == OUTLET_CAP_STATUS)) {
			// set pending
			outlet_set_pending(pout);
		}
		    
                outlets ++;

                /* add IPDU ID only when there is a new IPDU ID */
                if (pipdu != pout->ipdu) {
                        if (outlet_ini) {
                                if (outlet_ini == outlet_end) {
                                        *pbuf++ = PROTO_ELEMENT_OUTLET_NUMBER;
                                        *pbuf++ = 0x03;
                                        *pbuf++ = outlet_ini;
                                } else {
                                        *pbuf++ = PROTO_ELEMENT_OUTLET_NUMBER;
                                        *pbuf++ = 0x04;
                                        *pbuf++ = outlet_ini;
                                        *pbuf++ = outlet_end;
                                }
                        }
                        if (pout->ipdu->pmInfo->port != port) {
                                *pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
                                *pbuf++ = 0x03;
                                *pbuf++ = pout->ipdu->pmInfo->port;
                                port = pout->ipdu->pmInfo->port;
                        }
                        *pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
                        *pbuf++ = 0x03;
                        *pbuf++ = pout->ipdu->number;
                        pipdu = pout->ipdu;
                        outlet_ini = 0;
                }
                /* Try to consolidate outlets with ranges */
                outlet_cur = pout->onumber;
                if (! outlet_ini) {
                        outlet_ini = outlet_end = outlet_cur;
                } else if (outlet_end <= outlet_ini && outlet_cur == outlet_end - 1) {
                        outlet_end = outlet_cur;
                } else if (outlet_end >= outlet_ini && outlet_cur == outlet_end + 1) {
                        outlet_end = outlet_cur;
                } else {
                        if (outlet_ini == outlet_end) {
                                *pbuf++ = PROTO_ELEMENT_OUTLET_NUMBER;
                                *pbuf++ = 0x03;
                                *pbuf++ = outlet_ini;
                        } else {
                                *pbuf++ = PROTO_ELEMENT_OUTLET_NUMBER;
                                *pbuf++ = 0x04;
                                *pbuf++ = outlet_ini;
                                *pbuf++ = outlet_end;
                        }
                        outlet_ini = outlet_end = outlet_cur;
                }
                if ((int)(pbuf - pini) > (PMC_ARGSIZE-10)) {
                        syslog(4, "%s: COMMAND OVERFLOW - OUTLET OPERATION", __func__);
                        if (glb_treatAnswer) {
                                glb_treatAnswer("\nWARNING: Outlet list is incomplete due to command overflow.\n");
                        }
                        break;
                }

_ADDOUT_NEXT:
                if (ctrl_ltype == SINGLE_OUTLET) {
                        break;
                } else if ((ctrl_ltype == OUTLET_LIST) || (ctrl_ltype == OUTLET_GROUP)) {
                        pout = outl[index++];
                } else {
                        noutlet ++;
                        if (noutlet < ipdu->num_outlets) {
                                pout = ipdu->outlet + noutlet;
                                continue;
                        }
                        if (ctrl_ltype == OUTLIST_IPDU) {
                                break;
                        }
                        pout = NULL;
                        for  (ipdu = ipdu->next; ; ipdu = ipdu->next) {
                                if (ipdu == NULL) {
                                        if (ctrl_ltype == OUTLIST_CHAIN) {
                                                break;
                                        }
                                        for (pminfo = pminfo->next; pminfo;
                                                pminfo = pminfo->next) {
                                                ipdu = pminfo->ipdu;
                                                if (ipdu) {
                                                        break;
                                                }
                                        }
                                        if (ipdu == NULL) {
                                                break;
                                        }
                                }
                                if (ipdu->ready && ipdu->license) {
					if ((ipdu->num_outlets == 0) ||
					    ((ctrl_cap == OUTLET_CAP_STATUS) &&
				    	     (ipdu->sflag & IPDU_CAP_NOT_SWITCHABLE))) {
						pmc_answer_ipdu_error(ipdu, IPDUERR_IPDU_FIXEDON,glb_treatAnswer);
						continue;
					}
                                        pout = ipdu->outlet;
                                        noutlet = 0;
                                        break;
                                }
                        }
                        if (! pout) {
                                break;
                        }
                }
        }

        if (outlet_ini && (int)(pbuf - pini) <= (PMC_ARGSIZE-10)) {
                if (outlet_ini == outlet_end) {
                        *pbuf++ = PROTO_ELEMENT_OUTLET_NUMBER;
                        *pbuf++ = 0x03;
                        *pbuf++ = outlet_ini;
                } else {
                        *pbuf++ = PROTO_ELEMENT_OUTLET_NUMBER;
                        *pbuf++ = 0x04;
                        *pbuf++ = outlet_ini;
                        *pbuf++ = outlet_end;
                }
        }
        if ((outlets == 0) || flg_err) {
			return NULL;
        } else {
			return pbuf;
        }
}

/* pmc_sendcmd_outlet
 *     Send outlet command.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *       command - command to be sent:
 *               OUTLET_CONTROL_ON
 *               OUTLET_CONTROL_OFF
 *               OUTLET_CONTROL_CYCLE
 *		 OUTLET_CONTROL_LOCK
 *		 OUTLET_CONTROL_UNLOCK
 *  multout_flag - indicates type of outlet list:
 *               SINGLE_OUTLET
 *		 OUTLET_GROUP
 *               OUTLET_LIST
 *               OUTLIST_IPDU
 *               OUTLIST_CHAIN
 *               OUTLIST_ALL
 *        outlet - depending on multout_flag:
 *               outlet data (oData *) for SINGLE_OUTLET
 *               outlet list (oData **) for OUTLET_LIST and OUTLET_GROUP
 *               ipdu data (ipduInfo *) for OUTLIST_IPDU
 *               chain data (pmInfo *) for OUTLIST_CHAIN
 *               ignored, if OUTLIST_ALL
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_outlet(int command, char multout_flag, void *outlet,
			   char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf;
	int ret=0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	// init global variables
	ctrl_user = username;
	ctrl_ltype = multout_flag;
	ctrl_cap = OUTLET_CAP_STATUS;
	ctrl_capgroup = PMC_CAP_OUTLET;
	ctrl_cmd = command;
	glb_treatAnswer = treatAnswer;

	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL)
		return 0;

	pbuf = bufcmd;

	*pbuf++ = PROTO_ELEMENT_OUTLET_CONTROL_ID;
	*pbuf++ = 0x03;
	*pbuf++ = (char) command;

	pbuf = pmc_add_outlets (pbuf, outlet);

	if (pbuf) {
		if (ctrl_ltype == SINGLE_OUTLET) {
			// set outlet pending
			outlet_set_pending(outlet);
		} else {
			if (ctrl_ltype == OUTLET_GROUP) {
				// set outlet pending for the list
				oData **laux = (oData **)outlet;
				int i=0;
				for (; laux[i] ; i++){
					outlet_set_pending(laux[i]);
				}
			}
		}
		ret = power_send_command_req(COMMAND_OUTLET_OPERATION,bufcmd);
	}
	if (bufcmd) free(bufcmd);

	return ret;
}

/* pmc_sendcmd_outcfg_interval
 *     Send command to configure outlet interval.
 *     Validation of the interval is done by application
 * return: 1 on success; 0 otherwise
 * Parameters:
 *      interval - the value of the interval to be configured (seconds);
 *                 a value of -1.0 is used to represent an infinite interval
 *  multout_flag - indicates type of outlet list:
 *               SINGLE_OUTLET
 *               OUTLET_GROUP
 *               OUTLET_LIST
 *               OUTLIST_IPDU
 *               OUTLIST_CHAIN
 *               OUTLIST_ALL
 *        outlet - depending on multout_flag:
 *               outlet data (oData *) for SINGLE_OUTLET
 *               outlet list (oData **) for OUTLET_LIST and OUTLET_GROUP
 *               ipdu data (ipduInfo *) for OUTLIST_IPDU
 *               chain data (pmInfo *) for OUTLIST_CHAIN
 *               ignored, if OUTLIST_ALL
 *           cap - interval to be changed
 *               OUTLET_CAP_MINIMUMON
 *               OUTLET_CAP_MINIMUMOFF
 *               OUTLET_CAP_POSTPOWERON
 *               OUTLET_CAP_POSTPOWEROFF
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_outcfg_interval(double interval, char multout_flag, 
	void *outlet, unsigned int cap,
	char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf;
	int proto, time, ret=0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	//  Limit the interval to non-negative numbers up to 6500.0s 
	//  (roughly 1.8 hours), which should be more than enough. 
	if (interval < 0.0 || interval > 6500.0)
		return 0;

	// init global variables
	ctrl_user = username;
	ctrl_ltype = multout_flag;
	ctrl_cap = cap;
	ctrl_capgroup = PMC_CAP_OUTLET;
	ctrl_cmd = 0;
	glb_treatAnswer = treatAnswer;

	switch (cap) {
	case OUTLET_CAP_MINIMUMON:
		proto = PROTO_ELEMENT_OUTLET_MINIMUM_ON_TIME;
		time = (int) interval;
		break;
	case OUTLET_CAP_MINIMUMOFF:
		proto = PROTO_ELEMENT_OUTLET_MINIMUM_OFF_TIME;
		time = (int) interval;
		break;
	case OUTLET_CAP_POSTPOWERON:
		proto = PROTO_ELEMENT_OUTLET_POST_ON_INTERVAL;
		time = (int) (interval * 10);
		break;
	case OUTLET_CAP_POSTPOWEROFF:
		proto = PROTO_ELEMENT_OUTLET_POST_OFF_INTERVAL;
		time = (int) (interval * 10);
		break;
	default:
		return 0;
	}

	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL)
		return 0;

	pbuf = bufcmd;

	*pbuf++ = (char) proto;
	*pbuf++ = 0x04;
	*pbuf++ = (char) (time / 0x100);
	*pbuf++ = (char) (time % 0x100);

	pbuf = pmc_add_outlets(pbuf, outlet);

	if (pbuf) {
		ret = power_send_command_req(COMMAND_OUTLET_OPERATION,bufcmd);
	}
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_outcfg_name
 *     Send command to configure outlet name.
 *     Validation of the new name is done by application
 * return: 1 on success; 0 otherwise
 * Parameters:
 *        outlet - pointer to outlet data
 *          name - new outlet name
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_outcfg_name(oData *outlet, char *name,
			char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf;
	int ret = 0, size = 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "port[%d] pdu[%d] outlet[%d] name[%s]",
		  outlet->ipdu->pmInfo->port, outlet->ipdu->number, outlet->onumber, name);

	// init global variables
	ctrl_user = username;
	ctrl_ltype = SINGLE_OUTLET;
	ctrl_cap = OUTLET_CAP_NAME;
	ctrl_capgroup = 0;
	ctrl_cmd = 0;
	glb_treatAnswer = treatAnswer;

	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL)
		return 0;

	pbuf = bufcmd;

	pbuf = pmc_add_outlets (pbuf, outlet);

	if (pbuf) {
		// outlet name
		*pbuf++ = PROTO_ELEMENT_OUTLET_NAME;
		size = strlen(name);
		*pbuf++ = (char) size + 2;
		strncpy(pbuf, name, size);

		ret = power_send_command_req(COMMAND_OUTLET_NAME,bufcmd);
	}
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_outcfg_wakeup
 *     Send command to configure outlet wake up state.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *  multout_flag - indicates type of outlet list:
 *               SINGLE_OUTLET
 *               OUTLET_GROUP
 *               OUTLET_LIST
 *               OUTLIST_IPDU
 *               OUTLIST_CHAIN
 *               OUTLIST_ALL
 *        outlet - depending on multout_flag:
 *               outlet data (oData *) for SINGLE_OUTLET
 *               outlet list (oData **) for OUTLET_LIST
 *               ipdu data (ipduInfo *) for OUTLIST_IPDU
 *               chain data (pmInfo *) for OUTLIST_CHAIN
 *               ignored, if OUTLIST_ALL
 *         state - outlet wake up state
 *               WAKEUP_STATE_ON
 *               WAKEUP_STATE_OFF
 *               WAKEUP_STATE_LAST
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_outcfg_wakeup(char multout_flag, void *outlet, int state,
		  char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf;
	int ret=0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	// init global variables
	ctrl_user = username;
	ctrl_ltype = multout_flag;
	ctrl_cap = OUTLET_CAP_WAKEUP;
	ctrl_capgroup = PMC_CAP_OUTLET;
	ctrl_cmd = state;
	glb_treatAnswer = treatAnswer;

	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL)
		return 0;

	pbuf = bufcmd;

	*pbuf++ = PROTO_ELEMENT_OUTLET_WAKEUP;
	*pbuf++ = 0x03;
	*pbuf++ = (char) state;

	pbuf = pmc_add_outlets (pbuf, outlet);

	if (pbuf) {
		ret = power_send_command_req(COMMAND_OUTLET_OPERATION,bufcmd);
	}
	if (bufcmd) free(bufcmd);

	return ret;
}

/* pmc_sendcmd_outlet_reset
 *     Send command to reset the maximum value of:
 *         CURRENT, VOLTAGE, POWER and POWER_FACTOR
 *     for outlets.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *   multout_flag - indicates type of outlet list:
 *                SINGLE_OUTLET
 *                OUTLET_GROUP
 *                OUTLET_LIST
 *                OUTLIST_IPDU
 *                OUTLIST_CHAIN
 *                OUTLIST_ALL
 *         outlet - depending on multout_flag:
 *                outlet data (oData *) for SINGLE_OUTLET
 *                outlet list (oData **) for OUTLET_LIST and OUTLET_GROUP
 *                ipdu data (ipduInfo *) for OUTLIST_IPDU
 *                chain data (pmInfo *) for OUTLIST_CHAIN
 *                ignored, if OUTLIST_ALL
 *            cap - which type of measurement
 *                ELECMON_CAP_CURRENT
 *                ELECMON_CAP_VOLTAGE
 *                ELECMON_CAP_POWER
 *                ELECMON_CAP_POWER_FACTOR
 *                ELECMON_CAP_ENERGY
 *       username - user performing the action (NULL for 'root')
 *    treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_outlet_reset(char multout_flag, void *outlet, unsigned int cap,
			 char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf;
	int ret=0,proto;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	// init global variables
	ctrl_user = username;
	ctrl_ltype = multout_flag;
	ctrl_cap = cap;
	ctrl_capgroup = PMC_CAP_ELECMON;
	ctrl_cmd = 0;
	glb_treatAnswer = treatAnswer;

	switch (cap) {
		case ELECMON_CAP_CURRENT: 
			proto = PROTO_ELEMENT_ELECMON_RESET_CURRENT;
			break;
		case ELECMON_CAP_VOLTAGE:
			proto = PROTO_ELEMENT_ELECMON_RESET_VOLTAGE;
			break;
		case ELECMON_CAP_POWER:
			proto = PROTO_ELEMENT_ELECMON_RESET_POWER;
			break;
		case ELECMON_CAP_POWER_FACTOR:
			proto = PROTO_ELEMENT_ELECMON_RESET_POWER_FACTOR;
			break;
		case ELECMON_CAP_ENERGY:
			proto = PROTO_ELEMENT_ELECMON_RESET_ENERGY;
			break;
		default:
			return 0;
	}
	
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL)
		return 0;

	pbuf = bufcmd;

	*pbuf++ = proto;
	*pbuf++ = 0x02;

	pbuf = pmc_add_outlets (pbuf, outlet);

	if (pbuf) {
		ret = power_send_command_req(COMMAND_OUTLET_OPERATION,bufcmd);
	}
	if (bufcmd) free(bufcmd);

	return ret;
}

/* pmc_sendcmd_outcfg_alarm
 *     Send command to configure current threshold alarms for outlets.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *   multout_flag - indicates type of outlet list:
 *                SINGLE_OUTLET
 *                OUTLET_GROUP
 *                OUTLET_LIST
 *                OUTLIST_IPDU
 *                OUTLIST_CHAIN
 *                OUTLIST_ALL
 *         outlet - depending on multout_flag:
 *                outlet data (oData *) for SINGLE_OUTLET
 *                outlet list (oData **) for OUTLET_LIST and OUTLET_GROUP
 *                ipdu data (ipduInfo *) for OUTLIST_IPDU
 *                chain data (pmInfo *) for OUTLIST_CHAIN
 *                ignored, if OUTLIST_ALL
 *          alarm - alarm type
 *                ELECMON_CAP_CUR_CRIT_THRES
 *                ELECMON_CAP_CUR_WARN_THRES
 *                ELECMON_CAP_CUR_LOW_THRES
 *                ELECMON_CAP_CUR_LOWCRIT_THRES
 *          level - threashold value (in deciAmperes)
 *       username - user performing the action (NULL for 'root')
 *    treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_outcfg_alarm(char multout_flag, void *outlet, 
		unsigned int alarm, int level,
		char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf;
	int ret=0,proto;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	// init global variables
	ctrl_user = username;
	ctrl_ltype = multout_flag;
	ctrl_cap = alarm;
	ctrl_capgroup = PMC_CAP_ELECMON_THRESHOLDS;
	ctrl_cmd = level;
	glb_treatAnswer = treatAnswer;

	switch (alarm) {
		case ELECMON_CAP_CUR_CRIT_THRES:
			proto = PROTO_ELEMENT_ELECMON_CUR_CRIT_THRES;
			break;
		case ELECMON_CAP_CUR_WARN_THRES:
			proto = PROTO_ELEMENT_ELECMON_CUR_WARN_THRES;
			break;
		case ELECMON_CAP_CUR_LOW_THRES:
			proto = PROTO_ELEMENT_ELECMON_CUR_LOW_THRES;
			break;
		case ELECMON_CAP_CUR_LOWCRIT_THRES:
			proto =  PROTO_ELEMENT_ELECMON_CUR_LOWCRIT_THRES;
			break;
		default:
			return 0;
	}

	if (level < 0) return 0;

	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL)
		return 0;

	pbuf = bufcmd;

	*pbuf++ = proto;
	*pbuf++ = 0x04;
	*pbuf++ = (char)(level / 0x100);
	*pbuf++ = (char) (level % 0x0100);

	pbuf = pmc_add_outlets (pbuf, outlet);

	if (pbuf) {
		ret = power_send_command_req(COMMAND_OUTLET_OPERATION,bufcmd);
	}
	if (bufcmd) free(bufcmd);

	return ret;
}

//===========================================================
//  IPDUs send commands
//===========================================================

/* pmc_sendcmd_ipducfg_id
 *     Send command to configure IPDU ID.
 *     Validation of new id is done by application
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU data
 *            id - new IPDU ID
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipducfg_id(ipduInfo *ipdu, char *id,
		   char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0, size = 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "port[%d] pdu[%d] id[%s]", ipdu->pmInfo->port, ipdu->number, id);

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;

	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}

	pbuf = bufcmd;

	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 3;
	*pbuf++ = ipdu->number;
	*pbuf++ = PROTO_ELEMENT_IPDU_ID;
	size = strlen(id);
	*pbuf++ = (char) size + 2;
	strncpy(pbuf, id, size);
	pbuf += size;

	ret = power_send_command_req(COMMAND_IPDU_ID,bufcmd);

	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_ipducfg_onoff
 *     Send command to configure "boolean" IPDU capabilities.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU list
 *           len - number of IPDU list elements
 *           cap - the feature whose state will be configured:
 *               IPDU_CAP_SYSLOG
 *               IPDU_CAP_BUZZER
 *               IPDU_CAP_OVERCURRENT
 *         onoff - desired feature state
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipducfg_onoff(ipduInfo **ipdu, int len, unsigned int cap, 
		PMC_Boolean onoff,
		char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 1, i,proto,port=-1;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	switch (cap) {
		case IPDU_CAP_SYSLOG:
			proto = PROTO_ELEMENT_IPDU_SYSLOG;
			break;
		case IPDU_CAP_BUZZER:
			proto = PROTO_ELEMENT_IPDU_BUZZER;
            		break;
		case IPDU_CAP_OVERCURRENT:
			proto = PROTO_ELEMENT_IPDU_OVERCURRENT;
            		break;
		default:
			pmc_answer_ipdu_error(ipdu[0], IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
			return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)proto;
	*pbuf++ = 0x3;
	*pbuf++ = (char)onoff;
	for (i = 0; i < len; i ++) {
		debug(PMCLIB_SEND, "port[%d] pdu[%d] cap[%d] state[%d]",
			  ipdu[i]->pmInfo->port, ipdu[i]->number, cap, onoff);
		if (!(ipdu[i]->sflag & cap)) {
			pmc_answer_ipdu_error(ipdu[i], IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
			continue;
		}
		if (port !=  ipdu[i]->pmInfo->port) {
			*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
			*pbuf++ = 0x3;
			port = *pbuf++ = ipdu[i]->pmInfo->port;
		}
		*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
		*pbuf++ = 0x3;
		*pbuf++ = ipdu[i]->number;
	}
	if (port != -1) {
		ret = power_send_command_req(COMMAND_IPDU_OPERATION,bufcmd);
	}
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_ipdu
 *     Send IPDU commands.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU list
 *           len - number of IPDU list elements
 *       command - IPDU command to be sent:
 *               COMMAND_SAVE
 *               COMMAND_RESTORE
 *               COMMAND_DEFAULTS
 *               COMMAND_REBOOT
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipdu(ipduInfo **ipdu, int len, int command,
		 char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf;
	int ret = 1, i, port = -1, size;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	for (i=0; i < len; i++) {

		debug(PMCLIB_SEND, "port[%d] pdu[%d] command[%d]",
			  ipdu[i]->pmInfo->port, ipdu[i]->number, command);

		if (command == COMMAND_RESTORE) {
			if (ipdu[i]->pmInfo->ipdu_type != IPDU_TYPE_CYCLADES) {
				pmc_answer_ipdu_error(ipdu[i], 
					IPDUERR_IPDUCAP_NOTSUPPORTED, 
					treatAnswer);
					continue;
			}
		}
		if (command == COMMAND_REBOOT) {
			if (ipdu[i]->pmInfo->ipdu_type == IPDU_TYPE_SPC) {
				pmc_answer_ipdu_error(ipdu[i], 
					IPDUERR_IPDUCAP_NOTSUPPORTED, 
					treatAnswer);
					continue;
			}
		}
		if (port != ipdu[i]->pmInfo->port) {
			*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
			*pbuf++ = 0x3;
			port = *pbuf++ = ipdu[i]->pmInfo->port;
		}
		*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
		*pbuf++ = 0x3;
		*pbuf++ = ipdu[i]->number;
	}
	if (port != -1) {
		if ((ret = power_send_command_req(command,bufcmd))) {
			if (treatAnswer && command==COMMAND_REBOOT) {
				for (i=0; i < len; i++) {
					if (pmcOldFormat) {
						size = sprintf(localBuffer, pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu[i]->id));
					} else if (ipdu[i]->id[0]) {
						size = sprintf(localBuffer, pmc_answer_ipdupre, ipdu[i]->id);
					} else {
						size = sprintf(localBuffer, pmc_answer_ipdulocpre, ipdu[i]->pmInfo->device, pmc_get_old_ipdu(ipdu[i]->id) + 'A' - 1);
					}
					sprintf(localBuffer+size, pmc_errormsg[31-1]); //"Unit is rebooting."
					treatAnswer(localBuffer);
				}
			}
		}
	}
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_ipducfg_display
 *     Send command to configure IPDU display.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU data
 *          type - display type mode
 *               IPDU_DISPLAY_NORMAL_CURRENT
 *               IPDU_DISPLAY_INVERTED_CURRENT
 *               IPDU_DISPLAY_NORMAL_VOLTAGE
 *               IPDU_DISPLAY_INVERTED_VOLTAGE
 *         cycle - delay for auto mode (0-60 seconds)
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipducfg_display(ipduInfo *ipdu, int display_mode, int cycle,
			char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "port[%d] pdu[%d] mode[%d] cycle[%d]",
		  ipdu->pmInfo->port, ipdu->number, display_mode, cycle);

	if (!(ipdu->sflag & IPDU_CAP_DISPLAY) || 
	    (!(ipdu->sflag & IPDU_CAP_DISPLAY_CYCLE) && (cycle != 0)) ||
	    (cycle < 0) || (cycle > 60)) {
		pmc_answer_ipdu_error(ipdu, IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
		return 0;
	}
	if (strstr(ipdu->model, "PM45") == NULL && strstr(ipdu->model, "PM42") == NULL &&
	    ((display_mode == IPDU_DISPLAY_NORMAL_VOLTAGE) ||
	     (display_mode == IPDU_DISPLAY_INVERTED_VOLTAGE))) {
		pmc_answer_ipdu_error(ipdu, IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)PROTO_ELEMENT_IPDU_DISPLAY;
	*pbuf++ = 0x3;
	*pbuf++ = (char)display_mode;
	if (cycle) {
		*(pbuf-2) = 0x4;
		*pbuf++ = (char)cycle;
	}
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->number;
	ret = power_send_command_req(COMMAND_IPDU_OPERATION,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_ipducfg_interval
 *     Send command to configure IPDU intervals.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU data
 *           cap - the feature to be configured:
 *               IPDU_CAP_REBOOTDELAY
 *               IPDU_CAP_SEQINTERVAL
 *               IPDU_CAP_COLDSTARTDELAY
 *      interval - desired interval (seconds)
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipducfg_interval(ipduInfo *ipdu, unsigned int cap, int interval,
				 char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0,proto;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "port[%d] pdu[%d] cap[%d] interval[%d]",
		  ipdu->pmInfo->port, ipdu->number, cap, interval);

	if (interval < 0 || interval > 65000) {
		pmc_answer_ipdu_error(ipdu, IPDUERR_INVRANGE, treatAnswer);
		return 0;
	}

	switch (cap) {
	case IPDU_CAP_REBOOTDELAY:
		proto = PROTO_ELEMENT_IPDU_REBOOT_DELAY;
		break;
	case IPDU_CAP_SEQINTERVAL:
		proto = PROTO_ELEMENT_IPDU_SEQ_INTERVAL;
		break;
	case IPDU_CAP_COLDSTARTDELAY:
		proto = PROTO_ELEMENT_IPDU_COLDSTART_DELAY;
		break;
	default:
		pmc_answer_ipdu_error(ipdu, IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
		return 0;
	}

	if (!(ipdu->sflag & cap)) {
		pmc_answer_ipdu_error(ipdu, IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)proto;
	*pbuf++ = 0x4;
	*pbuf++ = (char)(interval/0x100);
	*pbuf++ = (char)(interval%0x100);
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->number;
	ret = power_send_command_req(COMMAND_IPDU_OPERATION,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_ipdu_fwupgrade
 *     Send command to upgrade IPDU firmware by chain.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to first IPDU
 *      listpdus - pdu# or pdu#_ini-pdu#_end[,]
 *          file - firmware file name in /tmp directory
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipdu_fwupgrade(ipduInfo *ipdu, char * listpdus, 
		char *file, char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0, size;

	if (file && strcmp(file,"/tmp/pmfirmware")) {
		char syscmd[100];
		sprintf(syscmd,"ln -sf %s /tmp/pmfirmware",file);
		system(syscmd);
	}

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;
	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->number;
	*pbuf++ = (char) PROTO_ELEMENT_UPGRADE_IPDUS;
	*pbuf++ = (char) (strlen(listpdus) + 2);
	size = (int) (pbuf - bufcmd);
	strncpy(pbuf, listpdus, PMC_ARGSIZE - size);
	ret = power_send_command_req(COMMAND_UPGRADE,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_ipdu_hwocp
 *     Send HW over current protection reset command.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU data
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipdu_hwocp(ipduInfo *ipdu, char *username, 
		void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "port[%d] pdu[%d]", ipdu->pmInfo->port, ipdu->number);

	if (! (ipdu->elecMon.sflag & ELECMON_CAP_HWOCP)) {
		pmc_answer_ipdu_error(ipdu, IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)PROTO_ELEMENT_ELECMON_HWOCP;
	*pbuf++ = 0x2;
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->number;
	ret = power_send_command_req(COMMAND_IPDU_OPERATION,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

// ELECMON && ENVMON commands

/* pmc_sendcmd_ipdu_reset_old
 *     Send commands to reset the maximum value of:
 *         CURRENT, VOLTAGE, TEMPERATURE and HUMIDITY
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU list
 *           len - number of IPDU list elements
 *        sensor - sensor whose maximum value will be reset:
 *               PROTO_ELEMENT_ELECMON_CURRENT
 *               PROTO_ELEMENT_ELECMON_VOLTAGE
 *               PROTO_ELEMENT_ELECMON_POWER
 *               PROTO_ELEMENT_ELECMON_POWER_FACTOR
 *               PROTO_ELEMENT_ELECMON_ENERGY
 *               ENVMON_SENSOR_TEMPERATURE
 *               ENVMON_SENSOR_HUMIDITY
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipdu_reset_old(ipduInfo **ipdu, int len, int sensor,
		   char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int i, j, ret=1, cap, proto, env=0;
	int elemcount, nelem=0;
	envMonInfo *penv=NULL;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	switch (sensor) {
	case PROTO_ELEMENT_ELECMON_CURRENT:
		cap = ELECMON_CAP_MAX_CURRENT;
		proto = PROTO_ELEMENT_ELECMON_RESET_CURRENT;
		break;
	case PROTO_ELEMENT_ELECMON_VOLTAGE:
		cap = ELECMON_CAP_MAX_VOLTAGE;
		proto = PROTO_ELEMENT_ELECMON_RESET_VOLTAGE;
		break;
	case PROTO_ELEMENT_ELECMON_POWER:
		cap = ELECMON_CAP_MAX_POWER;
		proto = PROTO_ELEMENT_ELECMON_RESET_POWER;
		break;
	case PROTO_ELEMENT_ELECMON_POWER_FACTOR:
		cap = ELECMON_CAP_MAX_POWER_FACTOR;
		proto = PROTO_ELEMENT_ELECMON_RESET_POWER_FACTOR;
		break;
	case PROTO_ELEMENT_ELECMON_ENERGY:
		cap = ELECMON_CAP_ENERGY;
		proto = PROTO_ELEMENT_ELECMON_RESET_ENERGY;
		break;
	case ENVMON_SENSOR_TEMPERATURE:
	case ENVMON_SENSOR_HUMIDITY:
		env = -1;
		proto = PROTO_ELEMENT_ENVMON_RESET_VALUE;
		cap = ENVMON_CAP_MAX_VALUE;
		break;
	default:
		pmc_answer_ipdu_error(ipdu[0], IPDUERR_IPDUCAP_NOSENSOR, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;

	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}

	for (i=0; i < len; i++) {
		if (env != -1) { // ELECMON RESET
			debug(PMCLIB_SEND, "port[%d] pdu[%d] sensor[%x]",
				  ipdu[i]->pmInfo->port, ipdu[i]->number, sensor);
			
			for (elemcount = 0; elemcount < ipdu[i]->num_phases; elemcount ++) { // set all the phases
				if (((ipdu[i]->phase[elemcount].sflag & PHASE_CAP_ELECMON) == 0) ||
						((ipdu[i]->phase[elemcount].elecMon.sflag & cap) == 0))
					continue;
				nelem = 1;
				pbuf = bufcmd;
				*pbuf++ = (char)proto;
				*pbuf++ = 0x2;
				*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->pmInfo->port;
				*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->number;
				*pbuf++ = PROTO_ELEMENT_PHASE_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->phase[elemcount].number;
				ret = power_send_command_req(COMMAND_PHASE_OPERATION,bufcmd);
				memset(bufcmd,0,PMC_ARGSIZE);
			}
			
			for (elemcount = 0; elemcount < ipdu[i]->num_circuits; elemcount ++) { // set all the circuits
				if (((ipdu[i]->circuit[elemcount].sflag & CIRCUIT_CAP_ELECMON) == 0) ||
						((ipdu[i]->circuit[elemcount].elecMon.sflag & cap) == 0))
					continue;
				nelem = 1;
				pbuf = bufcmd;
				*pbuf++ = (char)proto;
				*pbuf++ = 0x2;
				*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->pmInfo->port;
				*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->number;
				*pbuf++ = PROTO_ELEMENT_CIRCUIT_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->circuit[elemcount].number;
				ret = power_send_command_req(COMMAND_CIRCUIT_OPERATION,bufcmd);
				memset(bufcmd,0,PMC_ARGSIZE);
			}

			for (elemcount = 0; elemcount < ipdu[i]->num_outlets; elemcount ++) { // set all the outlets
				if (((ipdu[i]->outlet[elemcount].sflag & OUTLET_CAP_ELECMON) == 0) ||
						((ipdu[i]->outlet[elemcount].elecMon.sflag & cap) == 0))
					continue;
				nelem = 1;
				pbuf = bufcmd;
				*pbuf++ = (char)proto;
				*pbuf++ = 0x2;
				*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->pmInfo->port;
				*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->number;
				*pbuf++ = PROTO_ELEMENT_OUTLET_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->outlet[elemcount].onumber;
				ret = power_send_command_req(COMMAND_OUTLET_OPERATION,bufcmd);
				memset(bufcmd,0,PMC_ARGSIZE);
			}

			// elecmon cap
			if (ipdu[i]->elecMon.sflag & cap) {
				nelem = 1;
				pbuf = bufcmd;
				*pbuf++ = (char)proto;
				*pbuf++ = 0x2;
				*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->pmInfo->port;
				*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->number;
				ret = power_send_command_req(COMMAND_IPDU_OPERATION,bufcmd);
			}
			if (!nelem) {
				debug(PMCLIB_SEND, "NOT SUPPORTED port[%d] pdu[%d] sflag[%x] cap[%x]",
					  ipdu[i]->pmInfo->port, ipdu[i]->number,ipdu[i]->elecMon.sflag,cap);
				pmc_answer_ipdu_error(ipdu[i], 
					IPDUERR_IPDUCAP_NOTSUPPORTED, 
					treatAnswer);
			}
		} else { // ENVMON RESET
			for (j=0; j < ipdu[i]->num_envMons; j++) {
				penv = NULL;
				switch (ipdu[i]->envMon[j].sensor_type) {
				case ENVMON_SENSOR_TEMPERATURE:
				case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
				case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
					if ((sensor == ENVMON_SENSOR_TEMPERATURE) &&
					    (ipdu[i]->envMon[j].sflag & cap)) {
						penv = &ipdu[i]->envMon[j];
					}
					break;
				case ENVMON_SENSOR_HUMIDITY:
				case ENVMON_SENSOR_AVOCENT_HUM_INTERNAL:
				case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
					if ((sensor == ENVMON_SENSOR_HUMIDITY) &&
					    (ipdu[i]->envMon[j].sflag & cap)) {
						penv = &ipdu[i]->envMon[j];
					}
					break;
				}
				if (penv == NULL) {
					continue;
				}
				env = j;
				pbuf = bufcmd;
				*pbuf++ = (char)proto;
				*pbuf++ = 0x2;
				*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->pmInfo->port;
				*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = ipdu[i]->number;
				*pbuf++ = PROTO_ELEMENT_ENVMON_NUMBER;
				*pbuf++ = 0x3;
				*pbuf++ = penv->number;
				ret = power_send_command_req(COMMAND_ENVMON_OPERATION,bufcmd);
				memset(bufcmd,0,PMC_ARGSIZE);
			}
			if (env == -1){
				pmc_answer_ipdu_error(ipdu[i], IPDUERR_IPDUCAP_NOSENSOR, treatAnswer);
			}
			env = -1;
		}
	}
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_ipducfg_alarm_ex
 *     Send command to configure the current threshold alarms.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU data
 *         phase - phase number
 *         alarm - alarm type
 *               ELECMON_CAP_CUR_CRIT_THRES
 *               ELECMON_CAP_CUR_WARN_THRES
 *               ELECMON_CAP_CUR_LOW_THRES
 *               ELECMON_CAP_CUR_LOWCRIT_THRES
 *         level - threashold value (in deciAmperes)
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_ipducfg_alarm_ex(ipduInfo *ipdu, int phase, 
				unsigned int alarm, int level,
				char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0,proto=0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "port[%d] pdu[%d] phase[%d] alarm[%d] level[%.1f]",
		  ipdu->pmInfo->port, ipdu->number, phase, alarm, (double)level/10);

	if (!(ipdu->phase[phase - 1].elecMon.sflag & alarm)) {
		pmc_answer_ipdu_error(ipdu, IPDUERR_IPDUCAP_NOTSUPPORTED, treatAnswer);
		return 0;
	}

	switch (alarm) {
	case ELECMON_CAP_CUR_CRIT_THRES:
		proto = PROTO_ELEMENT_ELECMON_CUR_CRIT_THRES;
		break;
	case ELECMON_CAP_CUR_WARN_THRES:
		proto = PROTO_ELEMENT_ELECMON_CUR_WARN_THRES;
		break;
	case ELECMON_CAP_CUR_LOW_THRES:
		proto = PROTO_ELEMENT_ELECMON_CUR_LOW_THRES;
		break;
	case ELECMON_CAP_CUR_LOWCRIT_THRES:
		proto = PROTO_ELEMENT_ELECMON_CUR_LOWCRIT_THRES;
		break;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)proto;
	*pbuf++ = 0x4;
	*pbuf++ = (char) (level / 0x100);
	*pbuf++ = (char) (level % 0x100);
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->number;
	*pbuf++ = PROTO_ELEMENT_PHASE_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->phase[phase - 1].number;
	ret = power_send_command_req(COMMAND_PHASE_OPERATION,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_elecmon_reset
 *     Send command to reset the maximum value of:
 *         CURRENT, VOLTAGE, POWER and POWER_FACTOR
 *     for a given element.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *        element - pointer to referred element
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 *            cap - which type of measurement
 *                ELECMON_CAP_CURRENT
 *                ELECMON_CAP_VOLTAGE
 *                ELECMON_CAP_POWER
 *                ELECMON_CAP_POWER_FACTOR
 *       username - user performing the action (NULL for 'root')
 *    treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_elecmon_reset(void *element, int element_type, unsigned int cap,
			  char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL, *sysname, *measurement;
	int ret = 0,capmax;
	elecMonInfo *elecMon;
	int cmd = 0,proto, proto1, elen=0;
	ipduInfo *ipdu;

	if (!element) return 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "element[%08x] element_type[%d] cap[%d]", (int)element, element_type, cap);

	switch (element_type) {
	case ELEMENT_IPDU:
		sysname = "";
		ipdu = (ipduInfo *)element;
		elecMon = &(ipdu->elecMon);
		cmd = COMMAND_IPDU_OPERATION;
		proto1 = 0;
		break;
	case ELEMENT_PHASE:
		sysname = ((phaseInfo *)element)->sysname;
		ipdu = ((phaseInfo *)element)->ipdu;
		elecMon = &(((phaseInfo *)element)->elecMon);
		elen = ((phaseInfo *)element)->number;
		cmd = COMMAND_PHASE_OPERATION;
		proto1 = PROTO_ELEMENT_PHASE_NUMBER;
		break;
	case ELEMENT_CIRCUIT:
		sysname = ((circuitInfo *)element)->sysname;
		ipdu = ((circuitInfo *)element)->ipdu;
		elecMon = &(((circuitInfo *)element)->elecMon);
		elen = ((circuitInfo *)element)->number;
		cmd = COMMAND_CIRCUIT_OPERATION;
		proto1 = PROTO_ELEMENT_CIRCUIT_NUMBER;
		break;
	default:
		if (treatAnswer) treatAnswer(pmca_elemerr_unknown_element);
		return 0;
	}

	switch (cap) {
	case ELECMON_CAP_CURRENT:
		capmax = ELECMON_CAP_MAX_CURRENT;
		proto = PROTO_ELEMENT_ELECMON_RESET_CURRENT;
		measurement = current_str;
		break;
	case ELECMON_CAP_VOLTAGE:
		capmax = ELECMON_CAP_MAX_VOLTAGE;
		proto = PROTO_ELEMENT_ELECMON_RESET_VOLTAGE;
		measurement = voltage_str;
		break;
	case ELECMON_CAP_POWER:
		capmax = ELECMON_CAP_MAX_POWER;
		proto = PROTO_ELEMENT_ELECMON_RESET_POWER;
		measurement = power_str;
		break;
	case ELECMON_CAP_POWER_FACTOR:
		capmax = ELECMON_CAP_MAX_POWER_FACTOR;
		proto = PROTO_ELEMENT_ELECMON_RESET_POWER_FACTOR;
		measurement = powerFactor_str;
		break;
	case ELECMON_CAP_ENERGY:
		capmax = ELECMON_CAP_ENERGY;
		proto = PROTO_ELEMENT_ELECMON_RESET_ENERGY;
		measurement = energy_str;
		break;
	default:
		pmc_answer_element_error(ipdu, element_type, sysname, NULL, ELEMERR_UNKNOWN_MEASUREMENT, treatAnswer);
		return 0;
	}

	if (!(elecMon->sflag & cap)) {
		pmc_answer_element_error(ipdu, element_type, sysname, measurement, ELEMERR_MEASUREMENT_NOTSUPPORTED, treatAnswer);
		return 0;
	}
	if (!(elecMon->sflag & capmax)) {
		pmc_answer_element_error(ipdu, element_type, sysname, measurement, ELEMERR_MAXCAP_NOTSUPPORTED, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)proto;
	*pbuf++ = 0x2;
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->number;
	if (proto1) {
		*pbuf++ = proto1;
		*pbuf++ = 0x3;
		*pbuf++ = elen;
	}
	ret = power_send_command_req(cmd,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_eleccfg_alarm
 *     Send command to configure current threshold alarms for a given element.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *        element - pointer to referred element
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 *          alarm - alarm type
 *                ELECMON_CAP_CUR_CRIT_THRES
 *                ELECMON_CAP_CUR_WARN_THRES
 *                ELECMON_CAP_CUR_LOW_THRES
 *                ELECMON_CAP_CUR_LOWCRIT_THRES
 *          level - threashold value (in deciAmperes)
 *       username - user performing the action (NULL for 'root')
 *    treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_eleccfg_alarm(void *element, int element_type, unsigned int alarm, 
			int level, char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL, *sysname, *alarmstr;
	int ret = 0;
	elecMonInfo *elecMon;
	int cmd = 0,proto=0, proto1=0, elen=0;
	ipduInfo *ipdu;

	if (!element) return 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "element[%08x] element_type[%d] alarm[%d] level[%.1f]",
		  (int)element, element_type, alarm, (double)level/10);

	switch (element_type) {
	case ELEMENT_IPDU:
		sysname = "";
		ipdu = (ipduInfo *)element;
		elecMon = &(ipdu->elecMon);
		cmd = COMMAND_IPDU_OPERATION;
		proto1 = 0;
		break;
	case ELEMENT_PHASE:
		sysname = ((phaseInfo *)element)->sysname;
		ipdu = ((phaseInfo *)element)->ipdu;
		elecMon = &(((phaseInfo *)element)->elecMon);
		elen = ((phaseInfo *)element)->number;
		cmd = COMMAND_PHASE_OPERATION;
		proto1 = PROTO_ELEMENT_PHASE_NUMBER;
		break;
	case ELEMENT_CIRCUIT:
		sysname = ((circuitInfo *)element)->sysname;
		ipdu = ((circuitInfo *)element)->ipdu;
		elecMon = &(((circuitInfo *)element)->elecMon);
		elen = ((circuitInfo *)element)->number;
		cmd = COMMAND_CIRCUIT_OPERATION;
		proto1 = PROTO_ELEMENT_CIRCUIT_NUMBER;
		break;
	default:
		if (treatAnswer) treatAnswer(pmca_elemerr_unknown_element);
		return 0;
	}

	switch(alarm) {
	case ELECMON_CAP_CUR_CRIT_THRES:
		alarmstr = critHigh_str;
		proto = PROTO_ELEMENT_ELECMON_CUR_CRIT_THRES;
		break;
	case ELECMON_CAP_CUR_WARN_THRES:
		alarmstr = warnHigh_str;
		proto = PROTO_ELEMENT_ELECMON_CUR_WARN_THRES;
		break;
	case ELECMON_CAP_CUR_LOW_THRES:
		alarmstr = warnLow_str;
		proto = PROTO_ELEMENT_ELECMON_CUR_LOW_THRES;
		break;
	case ELECMON_CAP_CUR_LOWCRIT_THRES:
		alarmstr = critLow_str;
		proto = PROTO_ELEMENT_ELECMON_CUR_LOWCRIT_THRES;
		break;
	default:
		pmc_answer_element_error(ipdu, element_type, sysname, NULL, ELEMERR_INVALID_THRESHOLD_ALARM, treatAnswer);
		return 0;
	}

	if (!(elecMon->sflag & alarm)) {
		pmc_answer_element_error(ipdu, element_type, sysname, alarmstr, ELEMERR_NO_SPEC_THRESHOLD_ALARM, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)proto;
	*pbuf++ = 0x4;
	*pbuf++ = (char)(level / 0x100);
	*pbuf++ = (char)(level % 0x100);
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = ipdu->number;
	if (proto1) {
		*pbuf++ = proto1;
		*pbuf++ = 0x3;
		*pbuf++ = elen;
	}

	ret = power_send_command_req(cmd,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_envmon_reset
 *     Send command to reset the maximum measured value of environmental monitors.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *        envmon - pointer to environmental monitor data
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_envmon_reset(envMonInfo *envmon, char *username,
 				void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "ipdu[%s] sensor_type[%d]", envmon->ipdu->id, envmon->sensor_type);

	if (!(envmon->sflag & ENVMON_CAP_MAX_VALUE)) {
		if (treatAnswer) treatAnswer(pmca_envmerr_maxcap_notsupported);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)PROTO_ELEMENT_ENVMON_RESET_VALUE;
	*pbuf++ = 0x2;
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = envmon->ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = envmon->ipdu->number;
	*pbuf++ = PROTO_ELEMENT_ENVMON_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = envmon->number;
	
	ret = power_send_command_req(COMMAND_ENVMON_OPERATION,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_envmon_reset_all
 *     Send command to reset the maximum measured values of all environmental
 *     monitors on a given IPDU.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer IPDU data
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_envmon_reset_all(ipduInfo *ipdu, char *username, 
		void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0, i;

	if ( (!ipdu->num_envMons)||
			( (ipdu->num_envMons == 1)&&(ipdu->envMon[0].cap & ENVMON_CAP_NOT_PRESENT) )) {
		pmc_answer_envmon_error(ipdu, NULL, ENVMERR_SENSORS_NOTFOUND, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}

	for (i=0; i < ipdu->num_envMons; i++) {
//syslog(4,"%s: env[%d/%d] enumber[%d] sflag[0x%x]",__func__,i,ipdu->num_envMons,ipdu->envMon[i].number,ipdu->envMon[i].sflag);
		if (ipdu->envMon[i].sflag & ENVMON_CAP_MAX_VALUE) {
			pbuf = bufcmd;
			*pbuf++ = (char)PROTO_ELEMENT_ENVMON_RESET_VALUE;
			*pbuf++ = 0x2;
			*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
			*pbuf++ = 0x3;
			*pbuf++ = ipdu->pmInfo->port;
			*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
			*pbuf++ = 0x3;
			*pbuf++ = ipdu->number;
			*pbuf++ = PROTO_ELEMENT_ENVMON_NUMBER;
			*pbuf++ = 0x3;
			*pbuf++ = ipdu->envMon[i].number;
			ret = power_send_command_req(COMMAND_ENVMON_OPERATION,bufcmd);
			memset(bufcmd,0,PMC_ARGSIZE);
		}
		else{
			pmc_answer_envmon_error(ipdu, ipdu->envMon[i].sysname, ENVMERR_MAXCAP_NOTSUPPORTED, treatAnswer);
		}
	}

	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_envcfg_alarm
 *     Send command to configure threshold alarms for environmental monitors.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *        envmon - pointer to environmental monitor data
 *         alarm - alarm type
 *               ENVMON_CAP_CRIT_THRES
 *               ENVMON_CAP_CRIT_WARN_THRES
 *               ENVMON_CAP_LOW_WARN_THRES
 *               ENVMON_CAP_LOW_THRES
 *         level - threashold value (in deci"Units")
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
double iround(double val) {
        int fl;
        fl = floor(val);
        if (val - fl > 0.5) {
                return (double) (fl+1);
        }
        return (double) fl;
}

int pmc_sendcmd_envcfg_alarm(envMonInfo *envmon, unsigned int alarm, int level,
				 char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0,proto=0;
	int min=0,max=0,value=0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "ipdu[%s] sensor_type[%d] alarm[%d] level[%.1f]",
		  envmon->ipdu->id, envmon->sensor_type, alarm, (double)level/10);

	if (!(envmon->sflag & alarm)) {
		pmc_answer_envmon_error(envmon->ipdu, NULL, ENVMERR_NO_THRESHOLD_ALARM, treatAnswer);
		return 0;
	}

	switch (envmon->sensor_type) {
		case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
		case ENVMON_SENSOR_TEMPERATURE:
			switch (envmon->unit) {
			case TempScaleF:
				value = (int)iround((level + 4596.7) * 5 / 9);
				break;
			case TempScaleC:
			default:
				value = (int)iround(level + 2731.5);
			}
			min = 2730;     //273-373K, 0-100C, 32-212F
			max = 3730;
			break;
		case ENVMON_SENSOR_AVOCENT_HUM_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
		case ENVMON_SENSOR_HUMIDITY:
		case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
			value = (int) level;
			min = 0;        //0-100%
			max = 1000;
			break;
	}

	if ((value < min) || (value > max)) {
		pmc_answer_envmon_error(envmon->ipdu, NULL, ENVMERR_THRESHOLD_VALUE, treatAnswer);
		return 0;
	}

	switch (alarm) {
	case ENVMON_CAP_CRIT_THRES:
		proto = PROTO_ELEMENT_ENVMON_CRIT_THRESHOLD;
		break;
	case ENVMON_CAP_CRIT_WARN_THRES:
		proto = PROTO_ELEMENT_ENVMON_CRIT_WARN_THRESHOLD;
		break;
	case ENVMON_CAP_LOW_WARN_THRES:
		proto = PROTO_ELEMENT_ENVMON_LOW_WARN_THRESHOLD;
		break;
	case ENVMON_CAP_LOW_THRES:
		proto = PROTO_ELEMENT_ENVMON_LOW_THRESHOLD;
		break;
	}
	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	pbuf = bufcmd;
	*pbuf++ = (char)proto;
	*pbuf++ = 0x4;
	*pbuf++ = (char)(value/0x100);
	*pbuf++ = (char) (value % 0x100);
	*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = envmon->ipdu->pmInfo->port;
	*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = envmon->ipdu->number;
	*pbuf++ = PROTO_ELEMENT_ENVMON_NUMBER;
	*pbuf++ = 0x3;
	*pbuf++ = envmon->number;
	
	ret = power_send_command_req(COMMAND_ENVMON_OPERATION,bufcmd);
	if (bufcmd) free(bufcmd);
	return ret;
}

/* pmc_sendcmd_envcfg_unit
 *     Send command to configure the unit used by environmental monitors
 *     on a given IPDU.
 * return: 1 on success; 0 otherwise
 * Parameters:
 *          ipdu - pointer to IPDU data
 *        sensor - which type of environmental monitor
 *               ENVMON_SENSOR_TEMPERATURE
 *          unit - 'C' or 'F' or 'K'
 *      username - user performing the action (NULL for 'root')
 *   treatAnswer - pointer to client's print function
 */
int pmc_sendcmd_envcfg_unit(ipduInfo *ipdu, int sensor_type, char *unit,
			char *username, void (*treatAnswer)(char *))
{
	char *bufcmd, *pbuf=NULL;
	int ret = 0, i, sensorfound = 0, capok = 0;

	if ((shmp == NULL) || (pmc_pmd_status() == -1)) return 0;

	debug(PMCLIB_SEND, "ipdu[%s] sensor_type[%d] unit[%s]", ipdu->id, sensor_type, unit);

	if (!ipdu->num_envMons) {
		pmc_answer_envmon_error(ipdu, temperature_str, ENVMERR_SENSOR_NOTFOUND, treatAnswer);
		return 0;
	}

	ctrl_user = username;
	glb_treatAnswer = treatAnswer;
	if ((bufcmd = calloc(1, PMC_ARGSIZE)) == NULL) {
		return 0;
	}
	
	for (i=0; i < ipdu->num_envMons; i++) {
		if ((ipdu->envMon[i].sensor_type == ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL) ||
		    (ipdu->envMon[i].sensor_type == ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL) ||
		    (ipdu->envMon[i].sensor_type == ENVMON_SENSOR_TEMPERATURE)) {
			sensorfound = 1;
			if (!(ipdu->envMon[i].sflag & ENVMON_CAP_UNIT)) continue;
			capok = 1;
			pbuf = bufcmd;
			*pbuf++ = (char)PROTO_ELEMENT_ENVMON_UNIT;
			*pbuf++ = 0x3;
			*pbuf++ = (unit[0]=='F')?TempScaleF:TempScaleC;
			*pbuf++ = PROTO_ELEMENT_CHAIN_NUMBER;
			*pbuf++ = 0x3;
			*pbuf++ = ipdu->pmInfo->port;
			*pbuf++ = PROTO_ELEMENT_IPDU_NUMBER;
			*pbuf++ = 0x3;
			*pbuf++ = ipdu->number;
			*pbuf++ = PROTO_ELEMENT_ENVMON_NUMBER;
			*pbuf++ = 0x3;
			*pbuf++ = ipdu->envMon[i].number;
			ret = power_send_command_req(COMMAND_ENVMON_OPERATION,bufcmd);
			memset(bufcmd,0,PMC_ARGSIZE);
		}
	}

	if (!sensorfound) {
		pmc_answer_envmon_error(ipdu, temperature_str, ENVMERR_SENSOR_NOTFOUND, treatAnswer);
	} else if (!capok) {
		pmc_answer_envmon_error(ipdu, temperature_str, ENVMERR_CAPUNIT_NOTSUPPORTED, treatAnswer);
	}

	if (bufcmd) free(bufcmd);
	return ret;
}
