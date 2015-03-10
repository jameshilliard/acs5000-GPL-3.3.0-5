/* -*- c++ -*- */
/*****************************************************************
 * File: libcyc++_ipc.cc
 *
 * Copyright (C) 2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc++_ipc
 *
 ****************************************************************/

#include <cstdio>
#include <string>
#include <cstring>
#include <sstream>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <unistd.h>

#include <cyc++.h>
#include <cyc.h>

using namespace std;

cyc_ipc::cyc_ipc(string filepath, int proj_id)
{
	const char *fp = filepath.c_str();

	if ((ipc_key = ftok(fp, proj_id)) == -1) {
		throw app_error("ftok failed");
	}
}

cyc_ipc::cyc_ipc(int key)
{
	ipc_key = key;
}

cyc_shmem::cyc_shmem(string filepath, int proj_id) : cyc_ipc(filepath,
								proj_id)
{
	shmid = ERROR;
	mem = NULL;
}

cyc_shmem::cyc_shmem(int id) : cyc_ipc(0)
{
	shmid = id;
	mem = NULL;
}

void cyc_shmem::create(size_t size)
{
	create(size, 0);
}

void cyc_shmem::create(size_t size, int shmflg)
{
	if (size <= 0) throw app_error("Invalid size specified");

	if ((shmid = shmget(ipc_key, size, IPC_CREAT | 0660 | shmflg)) == -1) {
		throw app_error("shmget failed");
	}
}

void cyc_shmem::destroy()
{
	struct shmid_ds ds;

	if (shmctl(shmid, IPC_RMID, &ds) == -1) {
		throw app_error("shmctl failed");
	}
}

void *cyc_shmem::attach(const void *shmaddr, int shmflags)
{
	if (shmid == ERROR) throw app_error("Invalid shmid");

        if ((mem=shmat(shmid, shmaddr, shmflags)) == (void *)-1) {
		perror("cyc_shmem::attach");
		ostringstream err_msg;
		err_msg << "shmat failed: Could not attach to " << shmaddr;
		throw app_error(err_msg.str());
        }

	return(mem);
}

void cyc_shmem::detach()
{
	if (mem) {
		if (shmdt(mem) == -1) {
			throw app_error("shmdt failed");
		}
		mem=NULL;
	}
}

cyc_shmem::~cyc_shmem()
{
	detach();
}

cyc_sem::cyc_sem(string filepath, int proj_id) : cyc_ipc(filepath,
							    proj_id)
{
	semid = ERROR;
	nsems = 0;
}

cyc_sem::cyc_sem(int key) : cyc_ipc(key)
{
	semid = ERROR;
	nsems = 0;
}

cyc_sem::~cyc_sem()
{
	if (semid != ERROR) destroy();
}

void cyc_sem::create(int num_sems, int init_val)
{
	if (init_val < 0) throw app_error("Value to be set is out of range.");

	create(num_sems);
	set_all(init_val);
}

void cyc_sem::create(int num_sems)
{
	if (num_sems <= 0) throw app_error("Invalid value for nsems");

	nsems = num_sems;
	
	if ((semid = semget(ipc_key, nsems, IPC_CREAT | 0660)) == -1) {
		perror("cyc_sem::create: semget");
		throw app_error("semget failed");
	}
}

void cyc_sem::setval(int sem_num, int val)
{
	if (semid == ERROR) return;
	if ((sem_num < 0) || (sem_num >= nsems))
		throw app_error("Invalid semaphore number");

	union semun arg;
	arg.val = val;
	if (semctl(semid, sem_num, SETVAL, arg) == -1) {
		perror("cyc_sem::create: semctl");
		throw app_error("semctl failed");
	}
}

void cyc_sem::set_all(int val)
{
	if (semid == ERROR) throw app_error("Invalid semid");
	if (nsems <= 0) throw app_error("Invalid number of sems in set");

	for (int i=0; i < nsems; i++) {
		setval(i, val);
	}
}

void cyc_sem::destroy()
{
	union semun arg;

	if (semid == ERROR) return;

	if (semctl(semid, 0, IPC_RMID, arg) == -1) {
		perror("cyc_sem::destroy: semctl");
	}

	semid = ERROR;
}

int cyc_sem::op(int sem_num, int sem_op, int sem_flg)
{
	struct sembuf sops = { sem_num, sem_op, sem_flg };

	if (semid == ERROR) return(ERROR);

        return(semop(semid, &sops, 1));
}

int cyc_sem::op(int sem_num, int sem_op)
{
	return(op(sem_num, sem_op, 0));
}

int cyc_sem::timed_op(int sem_num, int sem_op, long int msecs)
{
	return(timed_op(sem_num, sem_op, msecs, 0));
}

#ifdef HAS_SEMTIMEDOP
int cyc_sem::timed_op(int sem_num, int sem_op,
		      long int msecs, int sem_flg)
{
	struct sembuf sops = { sem_num, sem_op, sem_flg };
	struct timespec timeout = { 0, msecs * 1000000  };

	if (semid == ERROR) return(ERROR);

	return(semtimedop(semid, &sops, 1, &timeout));
}
#else
int cyc_sem::timed_op(int sem_num, int sem_op,
		      long int msecs, int sem_flg)
{
	int ret = 0;

	if (semid == ERROR) return(ERROR);

	if (msecs < 100) msecs = 100;

	while (msecs >= 0) {
		usleep(100000);
		msecs -= 100;
		ret = op(sem_num, sem_op, IPC_NOWAIT | sem_flg);
		if (ret == -1) {
			// Not successful yet, try again
			continue;
		} else {
			// semop succeeded
			return(0);
		}
	}

	// Timeout expired without performing a successful semop
	return(ERROR);
}
#endif

int cyc_sem::getval(int sem_num)
{
	if (semid == ERROR) return(ERROR);

	return(semctl(semid, sem_num, GETVAL));
}
