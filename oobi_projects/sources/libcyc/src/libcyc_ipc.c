/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_ipc.c
 *
 * Copyright (C) 2003-2005 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_ipc
 *
 ****************************************************************/
/*  Some code:  
 *  Copyright 2002,2003 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/select.h>

#include <cyc.h>

static int create_named_socket(struct cyc_info *);

static int create_named_socket(struct cyc_info *cyc)
{
        struct sockaddr_un name;
        int sock;
        size_t size;
	char *filename=cyc->sock_filepath;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

        /* Create the socket. */
        sock=socket(PF_LOCAL,SOCK_DGRAM,0);
        if (sock < 0) {
                perror ("socket");
		return(ERROR);
        }
        
        /* Bind a name to the socket. */
        name.sun_family=AF_LOCAL;
        strncpy(name.sun_path,filename,sizeof(name.sun_path));
        
	size=SUN_LEN(&name);
        
        if (bind(sock,(struct sockaddr *)&name,size) < 0) {
                perror ("bind");
		return(ERROR);
        }
        
        return(sock);
}

int libcyc_sock_init(struct cyc_info *cyc)
{
	int sock;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	
	/* Remove the filename first; it's ok if the call fails */
	unlink(cyc->sock_filepath);
	
	/* Make the socket */
	sock=create_named_socket(cyc);

	return(sock);
}

int libcyc_get_sock_from_sockfile(struct cyc_info *cyc)
{
        struct sockaddr_un name;
        int sock=ERROR;
        size_t size;
        char *filename=cyc->sock_filepath;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

        /* Create the socket. */
        sock=socket(PF_LOCAL,SOCK_DGRAM,0);
        if (sock < 0) {
                perror ("socket");
                return(ERROR);
        }

        /* Bind a name to the socket. */
        name.sun_family=AF_LOCAL;
        strncpy(name.sun_path,filename,sizeof(name.sun_path));

        size=SUN_LEN(&name);

        if (connect(sock,(struct sockaddr *)&name,size) < 0) {
                perror ("bind");
                return(ERROR);
        }

	return(sock);
}

void libcyc_sock_destroy(struct cyc_info *cyc,int sock)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return;
	
	close(sock);
	unlink(cyc->sock_filepath);
}

void *libcyc_sock_recv(struct cyc_info *cyc,int sock)

{
	struct sockaddr_un name;
	size_t size;
	int nbytes;
	fd_set rfds;
	struct timeval tv;
	void *data=NULL;

	if ((cyc == NULL) ||
	    (! (cyc->flags & CYC_INIT)) ||
	    (sock == ERROR)) {
		return(NULL);
	}

	/* Watch the socket to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(sock, &rfds);
	/* No timeout for polling */
	tv.tv_sec=0;
	tv.tv_usec=0;

	if (select(sock+1,&rfds,NULL,NULL,&tv)) {
		data=libcyc_malloc(cyc, MAX_MSG_LEN);
		/* Wait for a datagram. */
		size=sizeof(name);
		nbytes=recvfrom(sock,data,MAX_MSG_LEN,0,
				(struct sockaddr *) &name,&size);
		if (nbytes < 0)	{
			perror ("recvfrom (server)");
		}
		return(data);
	}
	
	return(NULL);
}

int libcyc_sock_send(struct cyc_info *cyc,int sock,void *data)
{
	struct sockaddr_un name;
	size_t size;
	int nbytes;

	if ((cyc == NULL) ||
	    (! (cyc->flags & CYC_INIT)) ||
	    (data == NULL) ||
	    (sock == ERROR)) {
		return(ERROR);
	}	
	
	/* Initialize the server socket address. */
	name.sun_family=AF_LOCAL;
	strcpy(name.sun_path,cyc->sock_filepath);
	size=SUN_LEN(&name);
	/* Send the datagram. */
	nbytes=sendto(sock,data,MAX_MSG_LEN, 0,
		      (struct sockaddr *) &name, size);
	if (nbytes < 0)	{
		perror("sendto (client)");
	}

	return(nbytes);
}

int libcyc_ipc_key_get(struct cyc_info *cyc,
		       const char *filepath, int proj_id)
{
	key_t ipc_key = ERROR;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

/*
	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];
		snprintf(debug_str, MAX_STR_LEN,
			 "%s filepath: %s proj_id: %d", __func__,
			 filepath, proj_id);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/
	
	if ((ipc_key = ftok(filepath, proj_id)) == -1) {
		perror("ftok");
		return(ERROR);
	}

	return(ipc_key);
}

int libcyc_ipc_key_get_private(void)
{
	return(IPC_PRIVATE);
}

int libcyc_ipc_shmem_create(struct cyc_info *cyc,
			    key_t ipc_key, size_t size)
{
	return(libcyc_ipc_shmem_get(cyc, ipc_key, size, IPC_CREAT));
}

int libcyc_ipc_shmem_get(struct cyc_info *cyc,
			 key_t ipc_key, size_t size, int flags)
{
	struct shmem_info *shm = libcyc_malloc(cyc, sizeof(struct shmem_info));

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (size <= 0) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s ipc_key: %d size: 0x%x",
			 __func__, ipc_key, size);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	shm->id = ERROR;
	shm->mem = NULL;

	if ((shm->id = shmget(ipc_key, size, flags | 0660)) == -1) {
		perror("shmget");
		return(ERROR);
	}

	cyc->shmem_ids = libcyc_ll_append(cyc->shmem_ids, shm);

	return(shm->id);
}

void *libcyc_ipc_shmem_attach(struct cyc_info *cyc, int id,
			      void *shmaddr, int shmflg)
{
	void *mem = NULL;
	struct shmem_info *tmpshm = NULL, *shm = NULL;
	int found = 0;
	char debug_str[MAX_STR_LEN];

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d", __func__, id);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	while ((tmpshm = libcyc_ll_iterate(cyc->shmem_ids))) {
		if (tmpshm->id == id) {
			shm = tmpshm;
			found++;
		}
	}

	if (! found) return(NULL);

	if ((mem = shmat(id, shmaddr, shmflg)) == (void *)-1) {
		perror("shmat");
		mem = NULL;
		return(NULL);
	}
	shm->mem = mem;

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d attaching to: %p", __func__, id, mem);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	return(mem);
}

void libcyc_ipc_shmem_detach(struct cyc_info *cyc, int id)
{
	void *mem = NULL;
	struct shmem_info *shm = NULL;
	int found = 0;
	char debug_str[MAX_STR_LEN];

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return;

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d", __func__, id);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	while ((shm = libcyc_ll_iterate(cyc->shmem_ids))) {
		if (shm->id == id) {
			found++;
			mem = shm->mem;
		}
	}
	
	if (! found) return;
	
	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s detaching from: %p", __func__, mem);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	if (shmdt(mem) == -1) {
		perror("shmdt");
	}
}

int libcyc_ipc_shmem_destroy(struct cyc_info *cyc, int id)
{
	struct shmid_ds ds;
	struct shmem_info *shm = NULL;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d", __func__, id);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	while ((shm = libcyc_ll_iterate(cyc->shmem_ids))) {
		if (id && (shm->id == id)) {
			if (shmctl(shm->id, IPC_RMID, &ds) == -1) {
				perror("shmctl");
				return(ERROR);
			}
			shm->id = ERROR;
			shm->mem = NULL;
			cyc->shmem_ids = libcyc_ll_remove(cyc->shmem_ids, shm);
			libcyc_free(cyc, shm);
		} else if (id == 0) {
			if (shmctl(shm->id, IPC_RMID, &ds) == -1) {
				perror("shmctl");
				return(ERROR);
			}
			shm->id = ERROR;
			shm->mem = NULL;
			cyc->shmem_ids = libcyc_ll_remove(cyc->shmem_ids, shm);
			libcyc_free(cyc, shm);
		}
	}

	return(0);
}

int libcyc_ipc_sem_get(struct cyc_info *cyc,
			  key_t ipc_key, int nsems, int semflg)
{
	struct sem_info *new_sem = NULL, *sem = NULL, *tmp = NULL;
	int found = 0;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

/*
	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN, "%s ipc_key: %d nsems: %d",
			 __func__, ipc_key, nsems);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/

	new_sem = libcyc_malloc(cyc, sizeof(struct sem_info));
	new_sem->id = ERROR;
	new_sem->nsems = nsems;

	if ((new_sem->id = semget(ipc_key, new_sem->nsems, semflg | 0660)) == -1) {
		libcyc_free(cyc, new_sem);
		new_sem = NULL;
		return(ERROR);
	}

	while ((tmp = (struct sem_info *)libcyc_ll_iterate(cyc->sem_ids))) {
		if (tmp->id == new_sem->id) {
			found++;
			sem = tmp;
		}
	}
	
	if (found) {
		libcyc_free(cyc, new_sem);
		new_sem = NULL;
	} else {
		cyc->sem_ids = libcyc_ll_append(cyc->sem_ids, new_sem);
		sem = new_sem;
	}

	return(sem->id);
}

int libcyc_ipc_sem_create(struct cyc_info *cyc,
			  key_t ipc_key, int nsems)
{
	return(libcyc_ipc_sem_get(cyc, ipc_key, nsems, IPC_CREAT));
}

int libcyc_ipc_sem_set_all(struct cyc_info *cyc, int id, int init_val)
{
	struct sem_info *tmpsem = NULL, *sem = NULL;
	int found = 0, i;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s semid: %d init_val: %d",
			 __func__, id, init_val);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	while ((tmpsem = libcyc_ll_iterate(cyc->sem_ids))) {
		if (tmpsem->id == id) {
			sem = tmpsem;
			found++;
		}
	}

	if (! found) return(ERROR);

	for (i = 0; i < sem->nsems; i++) {
		libcyc_ipc_sem_set(cyc, id, i, init_val);
	}

	return(0);
}

int libcyc_ipc_sem_set(struct cyc_info *cyc, int id,
		       int semnum, int init_val)
{
	union semun arg;
	
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];
		
		snprintf(debug_str, MAX_STR_LEN,
			 "%s semid: %d semnum: %d init_val: %d",
			 __func__, id, semnum, init_val);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	arg.val = init_val;
	if (semctl(id, semnum, SETVAL, arg) == -1) {
		perror("semctl");
		return(ERROR);
	}

	return(0);
}

int libcyc_ipc_sem_op(struct cyc_info *cyc, int id, int num, int op)
{
	return(libcyc_ipc_sem_op_flag(cyc, id, num, op, 0));
}

int libcyc_ipc_sem_op_flag(struct cyc_info *cyc, int id, int num,
			   int op, int sem_flg)
{
	struct sem_info *sem = NULL;
	int found = 0;
	struct sembuf sops;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

/*
	if (cyc->flags & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d num: %d op: %d",
			 __func__, id, num, op);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/

	while ((sem = (struct sem_info *)libcyc_ll_iterate(cyc->sem_ids))) {
		if (sem->id == id) {
			found++;
		}
	}
	
	if (! found) return(ERROR);

	sops.sem_num = num;
	sops.sem_op = op;
	sops.sem_flg = sem_flg;

	if (semop(id, &sops, 1) == -1) {
		if (cyc->flags & CYC_DEBUG) {
			perror("libcyc_ipc_sem_op");
		}
		return(ERROR);
	}

	return(0);
}

int libcyc_ipc_sem_timed_op(struct cyc_info *cyc, int id, int num,
			    int op, long int milli_secs)
{
	return(libcyc_ipc_sem_timed_op_flag(cyc, id, num, op,
					    milli_secs, 0));
}

#ifdef HAS_SEMTIMEDOP
int libcyc_ipc_sem_timed_op_flag(struct cyc_info *cyc, int id, int num,
			    int op, long int milli_secs, int sem_flg)
{
	struct sem_info *sem = NULL;
	int found = 0;
	struct sembuf sops;
	struct timespec timeout;

/*
	if (cyc->flags & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d num: %d op: %d",
			 __func__, id, num, op);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/

	while ((sem = (struct sem_info *)libcyc_ll_iterate(cyc->sem_ids))) {
		if (sem->id == id) found++;
	}
	
	if (! found) return(ERROR);

	sops.sem_num = num;
	sops.sem_op = op;
	sops.sem_flg = sem_flg;
	timeout.tv_sec = 0;
	timeout.tv_nsec = milli_secs * 1000000;

	if (semtimedop(id, &sops, 1, &timeout) == -1) {
		perror("libcyc_ipc_sem_timed_op_flag");
		return(ERROR);
	}

	return(0);
}
#else
int libcyc_ipc_sem_timed_op_flag(struct cyc_info *cyc, int id, int num,
			    int op, long int msecs, int sem_flg)
{
	struct sem_info *sem = NULL;
	int found = 0;
	int tmp_flag = 0;
	int ret = 0;

/*
	if (cyc->flags & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d num: %d op: %d",
			 __func__, id, num, op);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/

	while ((sem = (struct sem_info *)libcyc_ll_iterate(cyc->sem_ids))) {
		if (sem->id == id) found++;
	}
	
	if (! found) return(ERROR);

	if (msecs < 100) msecs = 100;

	while (msecs >= 0) {
		usleep(100000);
		msecs -= 100;
		/* Temp. disable debug mode if active */
		tmp_flag = cyc->flags;
		cyc->flags &= ~CYC_DEBUG;
		ret = libcyc_ipc_sem_op_flag(cyc, id, num,
					     op, IPC_NOWAIT | sem_flg);
		/* Restore libcyc flags */
		cyc->flags = tmp_flag;
		if (ret == -1) {
			/* Not successful yet, try again */
			continue;
		} else {
			/* semop succeeded */
			return(0);
		}
	}

	/* Timeout expired without performing a
	   successful semop */
	return(ERROR);
}
#endif

int libcyc_ipc_sem_getval(struct cyc_info *cyc, int id, int num)
{
	int semval = ERROR;
	union semun arg;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	semval = semctl(id, num, GETVAL, arg);

/*
	if (cyc->flags & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d num: %d semval: %d",
			 __func__, id, num, semval);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/

	return(semval);
}

int libcyc_ipc_sem_destroy(struct cyc_info *cyc, int id)
{
	struct sem_info *sem = NULL;
	union semun arg;

	/* The semctl() man page lies.  We actually need four args for semctl
	   always even if the fourth argument is ignored.  In this case,
	   IPC_RMID doesn't use the fourth arg, but we have to include it
	   anyway to avoid a segfault. */

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (id == ERROR) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];
		
		snprintf(debug_str, MAX_STR_LEN, "%s id: %d", __func__, id);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	while ((sem = (struct sem_info *)libcyc_ll_iterate(cyc->sem_ids))) {
		if (id && (sem->id == id)) {
			if (semctl(sem->id, 0, IPC_RMID, arg) == -1) {
				perror("semctl");
				return(ERROR);
			}
			cyc->sem_ids = libcyc_ll_remove(cyc->sem_ids, sem);
			libcyc_free(cyc, sem);
			sem = NULL;
		} else if (id == 0) {
			if (semctl(sem->id, 0, IPC_RMID, arg) == -1) {
				perror("semctl");
				return(ERROR);
			}
			cyc->sem_ids = libcyc_ll_remove(cyc->sem_ids, sem);
			libcyc_free(cyc, sem);
			sem = NULL;
		}
	}

	return(0);
}

int libcyc_ipc_sem_nsems_get(struct cyc_info *cyc, int id)
{
	struct sem_info *tmpsem = NULL, *sem = NULL;
	int found = 0;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];

		snprintf(debug_str, MAX_STR_LEN,
			 "%s id: %d",__func__, id);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	while ((tmpsem = libcyc_ll_iterate(cyc->sem_ids))) {
		if (tmpsem->id == id) {
			sem = tmpsem;
			found++;
		}
	}

	if (! found) return(ERROR);

	return(sem->nsems);
}
