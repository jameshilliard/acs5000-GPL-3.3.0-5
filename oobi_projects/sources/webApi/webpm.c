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

# include <linux/shm.h>
# include <linux/sem.h>

#include <webApi.h>
#include "WebApi2.h"
#include <src/server.h>

#if defined(KVM)
#define MAX_DEV         cy_shm->max_devA
#elif defined(ONS)
#define MAX_DEV			AUX_OFFSET+cy_shm->max_devA
#else
#define MAX_DEV         cy_shm->max_devS
#endif

pmcInfo *pmcWeb[kMaxPhysicalPorts] = {(pmcInfo *)0xffff};

void freeAllWebPmc()
{
	pmcInfo *pmc;
	int i, flag = 0;

	if (pmcWeb[0] == (pmcInfo *)0xffff) return;

try_again:

	for(i=0; i < kMaxPhysicalPorts; i++) {
		if ((pmc = pmcWeb[i]) == NULL) continue;

		if (pmc->pmc_status != PMC_IDLE && flag != 2) {
			flag = 1;
			continue;
		}
		if (pmc->pmc_answerBuf) shm_free_str(pmc->pmc_answerBuf);
		cy_shm->pmc_list[pmc->idx_pmclist] = NULL;
		shm_free_str((char *)pmc);
	}

	if (flag == 1) {
		flag = 2;
		sleep(3);
		goto try_again;
	}
}

/* 
 *  getPmcInfoEntry() - This routine looks for an empty entry in cy_shm->pmc_list.
 *  If found, a pmcInfo structure is allocated, initialized and inserted in 
 *  this entry.
 **/
static pmcInfo *getPmcInfoEntry(int serialPort,int pid,int size)
{
	pmcInfo *pmc;
	char * buf;
	int index;
	
	// Try to find an empty PMC entry in the shared memory
	for (index = 0; index < MAXPMDACCESS; index++) {
		if (cy_shm->pmc_list[index] == 0)
		break;
	}

	// If not found, return error
	if (index == MAXPMDACCESS) {
		return NULL;
	}

	// Initialize the session: allocate the entry and set initial
	// values.
	pmc = (pmcInfo *)shm_malloc(sizeof(pmcInfo), 1);
	buf = shm_malloc(size,1);

	if (!pmc || !buf) {
		if (buf) shm_free_str((char *)buf);
		if (pmc) shm_free_str((char *)pmc);
		return NULL;
	}
	shm_signal(CY_SEM_SHM);
	pmc->idx_pmclist = index;
	pmc->pmc_pid = pid;
#ifdef ONS
	pmc->pmc_aux = cy_shm->line_conf[serialPort-1].auxflg;
#endif
	pmc->pmc_ipdu = serialPort-1;
	pmc->pmc_status = PMC_IDLE;
	pmc->pmc_sizeAnswerBuf = size;
	pmc->pmc_answerBuf = buf; 
	pmc->next = NULL;
	cy_shm->pmc_list[index] = pmc;
	shm_unsignal(CY_SEM_SHM);
        return pmc;
}

static void cycPmcGetBuf(pmcInfo *pmc, int size)
{
	char * buf;
	buf = shm_malloc(size,1);
	if (pmc->pmc_answerBuf) {
		shm_signal(CY_SEM_SHM);
		shm_free_str(pmc->pmc_answerBuf);
		shm_unsignal(CY_SEM_SHM);
	}
	pmc->pmc_sizeAnswerBuf = size;
	pmc->pmc_answerBuf = buf; 
}

// mount the command in pmcInfo
void cycPmcMountCommand(pmcInfo *pmc,int outlet, char *user, char *command)
{
	if (user && *user) {
		strcpy(pmc->pmc_username, user);
		strcpy(pmc->pmc_cmd,command);
		pmc->pmc_outlet[0] = outlet;
		pmc->pmc_outlet[1] = 0;
	} else {
		pmc->pmc_username[0] = 0x00;
		strcpy(pmc->pmc_cmd,command);
	}
}

/* Which software interrupts were detected */
static int got_sig = -1;
# define GOT_SIGUSR1	1	/* It is signalled by PMD, indicating that the
				   PMD is exiting */
# define GOT_SIGUSR2	2	/* It is signalled by PMD, indicating that 
				   there is data to be read from the shared
				   memory. */

// treatSignals : set got_sig with corret signal received SIGUSR1 or SIGUSR2
void treatSignals( int sig ) {
        struct sigaction act={{0}};
	switch (sig) {
		case SIGUSR1:
			got_sig |= GOT_SIGUSR1;
			break;
		case SIGUSR2:
			got_sig |= GOT_SIGUSR2;
			break;
	}
        act.sa_handler = treatSignals;
        sigaction(sig, &act, 0);
        unblock(sig);
}

void setSignals() 
{
        struct sigaction act={{0}};

	got_sig = 0;
	
	// Setting up the act structure
	act.sa_handler = treatSignals;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	
	sigaction(SIGUSR1, &act, 0);
	sigaction(SIGUSR2, &act, 0);
	unblock(SIGUSR1);
	unblock(SIGUSR2);
}

int cycWaitAnswer (pmcInfo *pmc)
{
	if (got_sig == -1) setSignals();

	got_sig = 0;

	if (pmc->pmc_username[0]) {
		pmc->pmc_status = PMC_REQUEST;
	} else {
		strcpy(pmc->pmc_username,"WebUser");
		pmc->pmc_status = PMC_REQUEST | PMD_NOAUTH;
	}

	kill(cy_shm->PMD_pid, SIGIO);

	while(true) {
		if (got_sig & GOT_SIGUSR1) {
		    	got_sig &= ~GOT_SIGUSR1;
			return false;
		}
		if (got_sig & GOT_SIGUSR2) {
		    	got_sig &= ~GOT_SIGUSR2;
			if (pmc->pmc_status == PMC_ANSOK) {
				if (!strncmp(pmc->pmc_answerBuf,"Ipdu is not",11)) {
					pmc->pmc_answerBuf[0] = 0x00;
				}
			}	
			return true;
		}
		sleep(10);
	}
}

pmcInfo * cycGetPmc(UInt32 serialPort,int size)
{
	pmcInfo *pmc;
	int i;
	
	if (pmcWeb[0] == (pmcInfo *)0xffff) {
		memset(pmcWeb,0,4 * kMaxPhysicalPorts);
		for (i=0;i<MAXPMDACCESS; i++) {
			if (cy_shm->pmc_list[i] == 0) continue;
			pmc = cy_shm->pmc_list[i];
			if (kill(pmc->pmc_pid, 0) < 0) {
				if (errno == ESRCH) {
					cy_shm->pmc_list[i] = NULL;
					if (pmc->pmc_answerBuf) {
						shm_free_str(pmc->pmc_answerBuf);
					}
					shm_free_str((char *)pmc);
				}
			}
		}
	}

	pmc = pmcWeb[serialPort - 1];
	
	if (pmc) {
		if (pmc->pmc_sizeAnswerBuf != size) {
			cycPmcGetBuf(pmc,size);
			if (!pmc->pmc_answerBuf) {
				shm_free_str((char *)pmc);
				pmcWeb[serialPort - 1] = NULL;
				return NULL; //mp: like in getPmcInfoEntry()...
			}
		} else {
			memset(pmc->pmc_answerBuf,0,pmc->pmc_sizeAnswerBuf);
		}
	} else {
		pmc = getPmcInfoEntry(serialPort,getpid(),size);
		pmcWeb[serialPort - 1] = pmc;
	}
	return pmc; //mp: also returns FALSE if getPmcInfoEntry() fails...
}

#ifdef IPMI
// return the status of the IPMI device
int cycIPMIcmd(int deviceId,CycIPMIcmds cmd, char *msg)
{
	char cmdIpmi[129], *p;
	int st=0;
	FILE *fp;
	
	sprintf(cmdIpmi,"cyc_ipmicmd %d %d >/tmp/ipmicmd.ret 2>&1",deviceId,cmd);

	system(cmdIpmi);

	memset(cmdIpmi,0,129);
	if ((fp = fopen("/tmp/ipmicmd.ret", "r")) != NULL) {
		while (fgets(cmdIpmi,128,fp) != NULL) {
			cmdIpmi[128] = 0x00;
			if (strstr(cmdIpmi,"Chassis Power")) {
				st = 1;
				break;
			}
			memset(cmdIpmi,0,129);
		}
		fclose(fp);
	}
	system("rm -f /tmp/ipmicmd.ret");
	if (st) {
		if (strstr(cmdIpmi, "failed:")) { 
			if ((p=strchr(cmdIpmi, '\n'))) *p = 0;
			if (msg) strcpy(msg, cmdIpmi);
			return(kIpmiCycle); // error
		}
		if (strstr(cmdIpmi, "is on") || strstr(cmdIpmi,"On")) { 
			if (msg) strcpy(msg, "Turn ON command was sent.");
			return(kIpmiOn);
		}
		if (strstr(cmdIpmi, "is off") || strstr(cmdIpmi,"Off")) { 
			if (msg) strcpy(msg, "Turn OFF command was sent.");
			return(kIpmiOff);
		}
		if (strstr(cmdIpmi, "Cycle")) { 
			if (msg) strcpy(msg,
#ifdef OEM3 
			"Reboot"
#else
			"Cycle"
#endif
			" command was sent.");
			return(kIpmiOn);
		}
	} 
	if (msg) {
		if (cmdIpmi[0]) {
			if ((p=strchr(cmdIpmi, '\n'))) *p = 0;
			strcpy(msg, cmdIpmi);
		} else {
			strcpy(msg, "Error : the IPMI server does not answer.");
		}
	}
	return(kIpmiCycle); // error
}
#endif
