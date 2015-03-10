/*****************************************************************
* File: pmclib.c
* 
* Copyright (C) 2006 Avocent Corporation
* 
* www.avocent.com
* 
* Description:
*    This file implements the Communication, Request Information
*    and Special routines of the PM Client Library.
* 
*****************************************************************/
#define PMCLIB_SRC
#include <pmclib.h>
#include <sched.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>

#ifdef DEBUG
unsigned long debugLevel = PMCLIB_SEND;
#endif

pthread_mutex_t shm_mtx = PTHREAD_MUTEX_INITIALIZER;
SHARED_DATA *shmp = NULL;
char *gsp_power_api_username = NULL;
int shmid = 0;
static unsigned char def_device[]=DEF_DEVICE;

// variables related with /var/run/snmp_pipe
int pmc_EventPipe = -1;	// fd 


/* pmc_shm_init
 *     Open the PM shared memory and attach it to address space of 
 *     the calling process.
 * return: 0 when the PM shared memory is attached to shmp global pointer
 *        -1 otherwise
 */
int pmc_shm_init(void)
{
	key_t myKey;

	if ((myKey = ftok("/bin/pmd_ng", 42)) == -1) {
		return -1;
	}
	if ((shmid = shmget(myKey, 0, IPC_OPEN_FLAG)) < 0) {
		return -1;
	}

	// Attach shared memory segment to calling process's address space
	shmp = (SHARED_DATA *) shmat(shmid, (void*)PM_ADDR_SHM, 0);

	if ((shmp == (void *) -1) || (shmp != (SHARED_DATA *)PM_ADDR_SHM)) {
		shmp = NULL;
		return -1;
	}

	return (0);
}

/* pmc_shm_init_safe
 */
int pmc_shm_init_safe(void)
{
	key_t myKey;

	if (shmp) {
		if (shmdt(shmp))
			return -1;
		else
			shmp = NULL;
	}

	if ((myKey = ftok("/bin/pmd_ng", 42)) == -1) {
		return -1;
	}
	if ((shmid = shmget(myKey, 0, IPC_OPEN_FLAG)) < 0) {
		return -1;
	}

	// Attach shared memory segment to calling process's address space
	shmp = (SHARED_DATA *) shmat(shmid, (void*)PM_ADDR_SHM, 0);

	if ((shmp == (void *) -1) || (shmp != (SHARED_DATA *)PM_ADDR_SHM)) {
		shmp = NULL;
		return -1;
	}

	return (0);
}

/* pmc_pmd_status
 * return: 0 - when the PMD core is running
 *        -1 - otherwise
 */
int pmc_pmd_status(void)
{
	FILE *fp;
	int pid;
	int status;
	int ret = -1;

	if ((fp = fopen(PMDPIDFILE, "r")) != NULL) {
		if (fscanf(fp, "%d", &pid) == 1) {
			status = kill(pid, 0);
			if (status != -1 || errno == EPERM) {
				if (shmp->corestatus & INIT_READY) {
					ret=0;
				}
			}
		}
		fclose(fp);
	}

	return ret;
}

/* pmc_cmdreq_pmd
 * Send the command request to PMD core and wait the answer.
 * return : the address of the allocated memory with the answer 
 *          received from PMD core. Or NULL – the request fail.
 * Parameters :
 *  username – the name of the regular user
 *  command - the command code 
 *  command_args – the pointer of the command mounted by one build 
 *                 command routine.
 */
#if 0
int pmc_cmdreq_pmd(char * username, unsigned char command, char * command_args, void (*treatAnswer)(char *))
{
	char *usern;
	answer * pans, * pnext, * pqueue = NULL;
	request *new_req, *prev;
	int upd_verbose = 0;
	FILE *fp;
	int interval, oldPMDpid = 0, PMDpid = 0;
	union semun sem_ctl;

	// verify the status of PMD core
	if ((shmp == NULL) || (pmc_pmd_status() == -1)) {
		free(command_args);
		return 0;
	}

	debug(PMCLIB,"username[%s] command[%d]",username, command);
	dotrace("PMCLIB-CMDREQ","request",command_args,100);

	if (username == NULL) {
		usern = user_root;
	} else {
		usern = username;
	}

	if (command == COMMAND_UPGRADE_FN) {
		command = COMMAND_UPGRADE;
		upd_verbose = 1;
	}

	if ((command == COMMAND_OUTLET_OPERATION) &&
	    (command_args[2] <= OUTLET_CONTROL_CYCLE)) {
		pmc_build_out_evt(usern,command_args);
	}

	debug(PMCLIB,"creating new request");
	/// Create a new request on shm
	new_req = (request *) pmshm_malloc(sizeof(request), TYPE_SHM_STRING);
	
	debug(PMCLIB,"mutexes and semaphores set, filling request");
	/// Fill out the request on shared memory
	new_req->command_id = command;
	new_req->slot = -1;
	strncpy(new_req->username, usern, USERNAMESIZE);
	memcpy(new_req->command_args, command_args, PMC_ARGSIZE);

	debug(PMCLIB,"adding request to the queue");

	/// Add request to the queue
	WAIT_SHM;

	prev = shmp->requests;
	shmp->requests = new_req;
	shmp->requests->next = prev;
	shmp->num_requests++;

	POST_SHM;

	debug(PMCLIB,"signaling thread pool");
		
	/// Signal the thread pool
	POST_STA;
	sched_yield();

	do {
		usleep(50000);
	} while (new_req->slot == -1);
	
	debug(PMCLIB, "Semaphore values shm0=%d shm1=%d shm2=%d", 
		semctl(shmp->cmdbuff[new_req->slot].buf_sem_id, 0, GETVAL, sem_ctl),
		semctl(shmp->cmdbuff[new_req->slot].buf_sem_id, 1, GETVAL, sem_ctl),
		semctl(shmp->cmdbuff[new_req->slot].buf_sem_id, 2, GETVAL, sem_ctl));

	while (1) {

		if ((fp = fopen(PMDPIDFILE, "r")) != NULL) {
			fscanf(fp, "%d", &oldPMDpid);
			fclose(fp);
		}
		interval = 5; // 5s

		if (upd_verbose) {
			int start = 0;
			printf("\r\n");
			while (1) {
				int prog = (shmp->fw_progress * 100) / shmp->fw_size;
				int i;

				if (start == 0 && prog == 0) {
					start = 1;
				}
				if (start) {
					char buf[128];
					strcpy(buf, "\rProgress: |");
					for (i = 0; i < 100; i += 2) {
						buf[12 + i/2] = (prog > i)? '=' : ' ';
					}
					sprintf(buf + 62, "| %d %% ", prog);
					treatAnswer(buf);
					if (TRYLOCK_THREAD(new_req->slot) == 0) {
						break;
					}
				}
				sleep(1);
				//[mp]
				//Code below is to produce a return when PMD crashes
				//I'm not sure if the clean up is correct, but it works for now...
				if (!interval--) {
					interval = 5;
					if ((fp = fopen(PMDPIDFILE, "r")) != NULL) {
						fscanf(fp, "%d", &PMDpid);
						fclose(fp);
					}
					if (!fp || (PMDpid != oldPMDpid)) {
						// PMD changed
						pmshm_free_str((char *)new_req);
						free(command_args);
						return 0;
					}
				}
			}
			printf("\r\n");
			upd_verbose = 0;

		} else {

//			WAIT_THREAD(new_req->slot);

			//[mp]
			//Code below is to produce a return when PMD crashes
			//I'm not sure if the clean up is correct, but it works for now...
			while (1) {
				if (TRYLOCK_THREAD(new_req->slot) == 0)
					break;
				sleep(1);
				if (!interval--) {
					interval = 5;
					if ((fp = fopen(PMDPIDFILE, "r")) != NULL) {
						fscanf(fp, "%d", &PMDpid);
						fclose(fp);
					}
					if (!fp || (PMDpid != oldPMDpid)) {
						// PMD changed
						pmshm_free_str((char *)new_req);
						free(command_args);
						sleep(20); // some time for the new PMD to start
						return 0;  // (WebUI needs it right away...)
					}
				}
			}
		}

		if (! pqueue) {
			pans = new_req->answer_b;
		} else {
			pans = pqueue->next;
		}
		if (! pans) {
			break;
		}
		pnext = pans->next;
		// process message 
		dotrace("PMCLIB-CMDREQ","answer",pans->buf, pans->size);
		pmc_show_answer (pans->buf, pans->size, treatAnswer);
		pqueue = pans;
		pans = pnext;
	}

	/* Free answer */
	for (pans = new_req->answer_b; pans != NULL ;) {
		pnext = pans->next;
		pmshm_free_str(pans->buf);
		pmshm_free_str((char *)pans);
		pans = pnext;
	}
	new_req->answer_b = NULL;

	POST_SYNC(new_req->slot);

	free(command_args);

	return 1;
}
#endif

/* pmc_isServerTechAllowed
 * return: 1 if ServerTech Ipdu use is allowed, 0 otherwise.
 * Parameter :
 *  device - the device to be checked: ttySxx or ttyAxx
 */
int pmc_isServerTechAllowed (int sp)
{
	int i;

	if ((shmp == NULL) || (shmp->license == NULL)) return (0);

	for (i=0;shmp->license[i].port; i++) {
		if (shmp->license[i].port == sp) {
			return 1;
		}
	}

	return 0;
}

/* pmc_numPhysicalPorts
 * return: the number of physical ports which have IPDUs
 *         connected to. 
 * Parameter :
 *   device - the device type of the physical port: ttyS or ttyA
 *            Default: ttyS, when device is NULL.
 */
int pmc_numPhysicalPorts(char *device)
{
	int count=0 ,size;
	char *dev;
	pmInfo *pminfo;

	if (shmp == NULL) return 0;

	// when device is NULL, uses the default: ttyS
	if (device == NULL) {
		dev = def_device;
	} else {
		dev = device;
	}

	size = strlen(dev);

	pminfo = shmp->pmInfo;

	while (pminfo != NULL) {
		if (!strncmp(dev, pminfo->device, size) && pminfo->num_ipdus && pminfo->ipdu && pminfo->ipdu->ready) {
			count++;
		}
		pminfo = pminfo->next;
	}

	debug(PMCLIB, "device[%s] numports[%d]", device, count);

	return count;
}

/* pmc_first_pmInfo
 * return : address of the first pmInfo struct, otherwise NULL 
 *          when there is none IPDUs connected.
 */
pmInfo *pmc_first_pmInfo(void)
{
	pmInfo *pm;

	if (shmp) {
		for (pm = shmp->pmInfo; pm; pm = pm->next) {
			if (pm->num_ipdus && pm->ipdu)
				return pm;
		}
	}

	return NULL;
}

/* pmc_numOutlets_pmInfo
 * Count the number of outlets of the chain connected to 
 * the physical port that the pmInfo is addressed by physport.
 * return: number of outlets of the chain.
 * Parameter :
 *   physport – address of the register of the physical port 
 *              whose outlets will be counted.
 */
int pmc_numOutlets_pmInfo ( pmInfo * pminfo)
{
	int count=0;
	ipduInfo * ipdu;

	if ((shmp == NULL) || (pminfo == NULL))  
		return (0);

	// first IPDU of the chain	
	ipdu = pminfo->ipdu;

	while (ipdu != NULL) {
		count += ipdu->num_outlets;
		ipdu = ipdu->next;
	}

	debug(PMCLIB,"pminfo[%x] device[%s] numOuts[%d]",(int)pminfo,pminfo->device,count);
	return (count);
}

/* pmc_numOutlets_port
 * Count the number of outlets of the chain connected to 
 * the physical port that the pmInfo is addressed by physport.
 * return: number of outlets of the chain.
 * Parameter :
 *  port_number - the number of the physical port
 *  device - the device of the physical port : ttyS or ttyA
 *           Default value : ttyS, used when device is NULL.
 */
int pmc_numOutlets_port (int port_number, char * device)
{
	int count=0;
	pmInfo * pminfo;
	ipduInfo * ipdu;

	if (shmp == NULL) return (0);

	pminfo = pmc_validate_port (port_number,device);

	if (pminfo == NULL) return (0); // not found the port

	// first IPDU of the chain	
	ipdu = pminfo->ipdu;

	while (ipdu != NULL) {
		if (ipdu->ready && ipdu->license) {
			count += ipdu->num_outlets;
		}
		ipdu = ipdu->next;
	}
	debug(PMCLIB,"portN[%d] device[%s] pminfo[%x] numOuts[%d]",port_number,device,(int)pminfo,count);
	return (count);
}

/* pmc_servername_out
 * verify if the outlet belongs to one CAS-Server
 * return: the number of serial port of the CAS-Server, otherwise 0
 * Parameters :
 *  outlet - the outlet data 
 */
int pmc_servername_out (oData * outlet)
{
	char * ngrp = NULL;
	int nsp = 0;

	ngrp = pmc_validate_multout_out(outlet);

	if (ngrp && (strncmp(ngrp,"$ttyS",5) == 0)) {
		nsp = atoi(ngrp+5);
	}
	return (nsp);
}

/**
 *  Find the pmInfo structure for a certain port.
 */
pmInfo *pmc_get_pmInfo_by_port(int port)
{
	pmInfo *pm;

	if (shmp) {
		for (pm = shmp->pmInfo; pm; pm = pm->next) {
			if (port == pm->number) {
				return pm;
			}
		}
	}

	return NULL;
}

/**
 *  Find the pmInfo structure for a certain logical port.
 */
pmInfo *pmc_get_pmInfo_by_logical_port(int port)
{
	pmInfo *pm;

	if (shmp) {
		for (pm = shmp->pmInfo; pm; pm = pm->next) {
			if (port == pm->port) {
				return pm;
			}
		}
	}

	return NULL;
}

/**
 *  Find the pmInfo structure given a device name.
 */
pmInfo *pmc_get_pmInfo_by_device(char *device)
{
	pmInfo *pm;
	int size = strlen(device);

	if (shmp) {
		for (pm = shmp->pmInfo; pm; pm = pm->next) {
			if (!strncmp(device, pm->device, size)) {
				return pm;
			}
		}
	}

	return NULL;
}

/**
 *  Find the pmInfo structure related to the given IPDU.
 *  NOTE: Only considers the first IPDU in the chain...
 */
pmInfo *pmc_get_pmInfo_by_id(const char *ipdu_id)
{
	pmInfo *pm;

	if (shmp) {
		for (pm = shmp->pmInfo; pm; pm = pm->next) {
			if (pm->ipdu && !strcmp(pm->ipdu->id, ipdu_id))
				return pm;
		}
	}

	return NULL;
}

/**
 *  Find the ipduInfo structure related to the given IPDU number in the chain.
 */
ipduInfo *pmc_get_ipduInfo_by_number(pmInfo *pm, int number)
{
	ipduInfo *ipdu;

	if (pm) {
		for (ipdu = pm->ipdu; ipdu; ipdu = ipdu->next) {
			if (number == ipdu->number) {
				return ipdu;
			}
		}
	}

	return NULL;
}

/**
 *  Check if a given outlet belongs to an SPC or ServerTech chain.
 */
int pmc_check_outlet_SPC_ServerTech(void *outp)
{
	if ((((oData*)outp)->ipdu->pmInfo->ipdu_type == IPDU_TYPE_SPC) ||
	    (((oData*)outp)->ipdu->pmInfo->ipdu_type == IPDU_TYPE_SERVERTECH)) {
		return 1;
	}
	return 0;
}

/*
- writeevt  - send messages to the event notification (event_gen) 
  daemon.
- writeevt generates a event notification message, which
  will be written to the fifo device.
  Parameters :
	. event_number : number of the event
	. format : type of the parameters : 'i' = interger
                                            's' = string
					    'u' = username
                   Ex. "iss" : interger, string, string
        . va_list : list of the parameters for the evnet
*/

static int pmc_openevt(void)
{
	if (pmc_EventPipe != -1) {
		return (0); // OK
	}

	pmc_EventPipe = open("/var/run/snmp_pipe",
                        O_RDWR | O_NONBLOCK | O_NOCTTY );
	if (pmc_EventPipe == -1) {
		syslog(4,"Event Library: Open error[%d] : %m",getpid());
		return(-1); // error
	}
	return(0); // OK
}

void pmc_writeevt(int evtn, char * format,...)
{
	va_list params;
	int pard;	// parameter type digit
	char *pars;	// parameter type string
	int size;	// size of the message	

	struct passwd *pusr; 

	char * buffer;	// buffer allocated to store the event

	if (pmc_EventPipe == -1) { // open the connect
		if (pmc_openevt() == -1) 
			return;
	}
	
	if ((buffer = calloc((MAXMSGSIZE+2),1)) == NULL) {
		syslog(4,"Event Library: allocat buffer error[%d] : %m",getpid());
		return; 
	}

	va_start(params,format);

	// mount the first part of the message
	size = sprintf(buffer,"EVT%d %ld ",evtn, time(NULL));
	
	// add the parameters
	while (*format) {
		switch(*format++) {
			case 's': // string
				pars = va_arg(params,char *);
				size += snprintf(buffer+size,MAXMSGSIZE-size,
						"%d %s ",strlen(pars),pars);
				break;
			case 'i': // integer
				pard = va_arg(params, int);
				size += snprintf(buffer+size,MAXMSGSIZE-size,
					"%d ",pard);
				break;		
			case 'u': // user name 
				pusr = getpwuid(getuid());
				if (pusr == NULL) {
					pars = strdup("unknow");
				} else {
					pars = strdup(pusr->pw_name);
				}			
				size += snprintf(buffer+size,MAXMSGSIZE-size,
						"%d %s ",strlen(pars),pars);
				free(pars);
		}
		if (size >= MAXMSGSIZE) {
			break;
		}
	}
	va_end(params);
	buffer[size++] = '\n';
	size=write(pmc_EventPipe, buffer, size);
	fsync(pmc_EventPipe);
}

static int check_ready_port(int port)
{
	pmInfo *pm;
	int nready = 0;;
	ipduInfo *ipdu;

	/* For each chain */
	for (pm = shmp->pmInfo; pm; pm = pm->next) {
		if (pm->number == port) {
            
           if(!pm->got_num_ipdus) {
               break;
            }
            
            for (ipdu = pm->ipdu; ipdu; ipdu = ipdu->next) {
                if (ipdu->ready)
                    nready++;
            }
            /* Check if not all ipdus in the chain are ready */
            if (nready < pm->num_ipdus)
                nready = 0;
            
            break;
        }
	}

	return nready;
}

static int check_ready_all()
{
	pmInfo *pm;
	ipduInfo *ipdu;
	int nready;

	/* For each chain */
	for (pm = shmp->pmInfo; pm; pm = pm->next) {
		if (!pm->got_num_ipdus)
			break;
		nready = 0;
		/* For each ipdu in the chain */
		for (ipdu = pm->ipdu; ipdu; ipdu = ipdu->next) {
			if (ipdu->ready)
				nready++;
		}
		/* Check if not all ipdus in the chain are ready */
		if (nready < pm->num_ipdus)
			break;
	}

	return !pm;
}

void pmc_wait_detection_all(int timeout)
{
	time_t ready_timeout = 4;	/* Ready confirmation timeout. */
	time_t start_time;

	if (shmp == NULL) return;

	start_time = time(NULL);

	/* Possibly wait for some chain to come up */
	while (shmp->pmInfo == NULL) {
		if ((time(NULL) - start_time) > timeout)
			break;
		usleep(500000);
	}

	if (shmp->pmInfo == NULL)
		return;

	if (check_ready_all()) {
		/*
		 * All devices are already ready: it is possible that we
		 * haven't had enough time to load a new configuration yet, so
		 * let's wait a little.
		 */
		while ((time(NULL) - start_time) < ready_timeout) {
			usleep(250000);
			if (!check_ready_all())
				break;
		}
	}

	while ((time(NULL) - start_time) < (time_t) timeout) {
		if (check_ready_all())
			break;
		usleep(500000);
	}
}

void pmc_wait_detection_port(int timeout, int port)
{
	time_t ready_timeout = 4;	/* Ready confirmation timeout. */
	time_t start_time;

	if (shmp == NULL) return;

	start_time = time(NULL);

	/* Possibly wait for some chain to come up */
	while (shmp->pmInfo == NULL) {
		if ((time(NULL) - start_time) > timeout)
			break;
		usleep(500000);
	}

	if (shmp->pmInfo == NULL)
		return;

	if (check_ready_all()) {
		/*
		 * All devices are already ready: it is possible that we
		 * haven't had enough time to load a new configuration yet, so
		 * let's wait a little.
		 */
		while ((time(NULL) - start_time) < ready_timeout) {
			usleep(250000);
			if (!check_ready_all())
				break;
		}
	}

	while ((time(NULL) - start_time) < (time_t) timeout) {
		if (check_ready_port(port))
			break;
		usleep(500000);
	}
}
