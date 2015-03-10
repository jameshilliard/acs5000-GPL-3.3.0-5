/*****************************************************************
* File: rot_shm.c
*
* Copyright (C) 2003 Cyclades Corporation
*
* www.cyclades.com
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version
* 2 of the License, or (at your option) any later version.
*
* Description: Shared memory API.
*
****************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAIN_SHM

#include "server.h"

static void shm_group_str(CY_SHM_STR *psh)
{
	CY_SHM_STR *pnext;

	if (psh->next == NULL) {
		return;
	}

	pnext = (CY_SHM_STR *)((char *)psh + psh->size);

	if (pnext == psh->next) {
		psh->size += pnext->size;
		psh->next = pnext->next;
	}
}

void shm_inc_count(char *p)
{
	CY_SHM_STR *psh = (CY_SHM_STR *)(p - sizeof(CY_SHM_STR));
	psh->count++;
}

void shm_free_str(char *p)
{
	CY_SHM_STR *psh = (CY_SHM_STR *)(p - sizeof(CY_SHM_STR));
	CY_SHM_STR *plist, *pold;

	pold = NULL;
	plist = (CY_SHM_STR *)cy_shm->alloc_str;

	XPRINTF8("shm_free_str %x  size %d str %s\r\n", (int)psh, psh->size,
		(char *)psh + sizeof(CY_SHM_STR));

	if (--psh->count > 0) {
		return;
	}

	// remove from used list
	while (plist) {
		if (plist == psh) {
			if (pold == NULL) {
				cy_shm->alloc_str = (char *)plist->next;
			} else {
				pold->next = plist->next;
			}
			break;
		}
		pold = plist;
		plist = plist->next;
	}

	if (plist == NULL) {
		return;
	}

	// put into free list
	plist = (CY_SHM_STR *)cy_shm->free_str;
	pold = NULL;

	while (plist != NULL && plist < psh) {
		pold = plist;
		plist = plist->next;
	}

	// at the beginning
	if (pold == NULL) {
		psh->next = (CY_SHM_STR *)cy_shm->free_str;
		cy_shm->free_str = (char *)psh;
		shm_group_str(psh);
		return;
	}

	// In the middle

	psh->next = pold->next;
	pold->next = psh;
	shm_group_str(psh);
	shm_group_str(pold);
}

char *shm_malloc(int size, int flag)
{
	CY_SHM_STR *pmin = NULL, *pold = NULL, *pnew, *pant = NULL;
	CY_SHM_STR *psh = (CY_SHM_STR *)cy_shm->free_str;

	if (psh == NULL) {
		return(NULL);
	}

	if (!size) ++size;

	size = (size - 1 + sizeof(CY_SHM_STR) * 2) & 0xfffffff0;

	while (psh) {
		if (psh->size >= size) {
			// save previous
			if (!pmin) {
				pant = pold;
				pmin = psh;
			} else {
				if (pmin->size > psh->size) {
					pant = pold;
					pmin = psh;
				}
			}
		}
		pold = psh;
		psh = psh->next;
	}

	if (pmin == NULL) {
		return(NULL);
	}
	// previous
	psh = pant;

	if (pmin->size <= (size + sizeof(CY_SHM_STR) + 8)) {
		// remove the entry from free list
		pnew = pmin;
		if (psh == NULL) {
			cy_shm->free_str = (char *)pmin->next;
		} else {
			psh->next = pmin->next;
		}
	} else {
		// allocate from the end 
		XPRINTF8("shm_malloc break %x  size %d \r\n", (int)pmin, pmin->size);
		pnew = (CY_SHM_STR *)((char *)pmin + pmin->size - size);
		pmin->size -= size;
		pnew->size = size;
	}

	// build used list
	pnew->next = (CY_SHM_STR *)cy_shm->alloc_str;
	cy_shm->alloc_str = (char *)pnew;

	// clean the memory allocated
	memset(((char *)pnew + sizeof(CY_SHM_STR)), 0,
		pnew->size - sizeof(CY_SHM_STR));

	XPRINTF8("shm_malloc %x  size %d \r\n", (int)pnew, pnew->size);

	pnew->flag = flag;
	pnew->count = 1;

	return(((char *)pnew + sizeof(CY_SHM_STR)));
}

char *shm_strdup(char *s)
{
	char *p;
	int len;

	len = strlen(s);

	if ((p = shm_malloc(len, TYPE_SHM_STR)) == NULL) {
		return(NULL);
	}

	XPRINTF8("shm_strdup %x  str %s\r\n", (int)p, s);
	if (len) {
    	memcpy(p, s, len);
	}
	*(p+len) = 0;
	return(p);
}

int shm_init(int shm_id)
{
	int size, shmflg;

	if (shm_id >= 0) {
		size = sizeof(CY_SHM);
		size += (shm_id * 1024); 
		shmflg = IPC_CREAT|IPC_EXCL|SHM_PERM;
	} else {
		size = 0;
		shmflg = SHM_PERM;
	}
		
	if ((shm_id = shmget(CY_ID_SHM, size, shmflg)) < 0) {
		XPRINTF("shm_init: %s\r\n", strerror(errno));
		return(-1);
	}

	if ((cy_shm != (CY_SHM *) -1) && (cy_shm == (CY_SHM *) CY_ADDR_SHM)) {
		if (cy_shm->id_shm != CY_ID_SHM) {
			XPRINTF("shm_init invalid shm_key : %x\r\n", cy_shm->id_shm);
			return(-1);
		}
		return(shm_id);
	}

	if ((cy_shm = (CY_SHM *) shmat(shm_id, (void *) CY_ADDR_SHM, 0)) < 0) {
		XPRINTF("shm_attach: %s\r\n", strerror(errno));
		return(-1);
	}
	if (cy_shm != (CY_SHM *) CY_ADDR_SHM) {
		XPRINTF("shm_init: Memory attached at an invalid address: %08lx\r\n",
					   	cy_shm);
		return (-1);
	}

	if (!(shmflg & IPC_CREAT)) {
		if (cy_shm->id_shm != CY_ID_SHM) {
			XPRINTF("shm_init invalid shm_key : %x\r\n", cy_shm->id_shm);
			return(-1);
		}
	} else {
		XPRINTF("Allocated shmid : %d  at : 0x%x\r\n", shm_id, (int)cy_shm);
	}
	return(shm_id);
}

void shm_set_st_data_buffering(int port, int st)
{
	if (!(st == DB_ST_RESTART &&
		cy_shm->st_data_buffering[port] != DB_ST_STOPPED))
		cy_shm->st_data_buffering[port] = st;
}

int shm_get_st_data_buffering(int port)
{
	return cy_shm->st_data_buffering[port];
}


int shm_get_set_val(int val, int *pval, int flag, int sem_num)
{
	struct sembuf sops;

	sops.sem_num = sem_num;  /* semaphore number: 0 = first */
	sops.sem_op = -1;   /* semaphore operation */
	//[RK]Oct/07/2005 - if the process exits operation is undone
	sops.sem_flg = SEM_UNDO;  /* operation flags */

	if (semop(cy_shm->sem_id, &sops, 1) < 0) {
		perror("shm_get_set_val semop -1:");
	}

	if (flag) {
		*pval = val;
	} else {
		val = *pval;
	}

	sops.sem_op = 1;   /* semaphore operation */
	if (semop(cy_shm->sem_id, &sops, 1) < 0) {
		perror("shm_get_set_val semop 1:");
	}

	return(val);
}

void shm_signal(int sem_num)
{
	struct sembuf sops;

SHM_SIGNAL_BACK:
	sops.sem_num = sem_num;  /* semaphore number: 0 = first */
	sops.sem_op = -1;   /* semaphore operation */
	//[RK]Nov/26/2003 Bug#1149 - if the process exits operation is undone
	sops.sem_flg = SEM_UNDO;  /* operation flags */
	if (semop(cy_shm->sem_id, &sops, 1) < 0) {
		if (errno == EINTR) goto SHM_SIGNAL_BACK;
		perror("shm_signal semop -1:");
	}
}

void shm_unsignal(int sem_num)
{
	struct sembuf sops;
	
SHM_UNSIGNAL_BACK:
	sops.sem_num = sem_num;  /* semaphore number: 0 = first */
	sops.sem_op = 1;   /* semaphore operation */
	//[RK]Nov/26/2003 Bug#1149 - if the process exits operation is undone
	sops.sem_flg = SEM_UNDO;  /* operation flags */
	if (semop(cy_shm->sem_id, &sops, 1) < 0) {
		if (errno == EINTR) goto SHM_UNSIGNAL_BACK;
		perror("shm_unsignal semop 1:");
	}
}
