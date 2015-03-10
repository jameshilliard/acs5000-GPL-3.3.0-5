/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_thread.c
 *
 * Copyright (C) 2005 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_thread
 *
 *****************************************************************
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>

#include <cyc.h>

pthread_t libcyc_thread_create(struct cyc_info *cyc,
			 void *(*start_routine)(void *),
			 void *arg)
{
	pthread_t tid;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT)) ||
	    (start_routine == NULL)) return(0);

	if ((pthread_create(&tid, NULL, start_routine, arg)) < 0) {
		perror("libcyc_thread_create");
		return(0);
	}

	return(tid);
}

int libcyc_thread_exit(struct cyc_info *cyc)
{
	int retval;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	pthread_exit(&retval);

	return(retval);
}

int libcyc_thread_cancel(struct cyc_info *cyc, pthread_t tid)
{
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(pthread_cancel(tid));
}

void *libcyc_thread_join(struct cyc_info *cyc, pthread_t tid)
{
	void *ret;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	pthread_join(tid, &ret);

	return(ret);
}

int libcyc_thread_detach(struct cyc_info *cyc, pthread_t tid)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(pthread_detach(tid));
}

int libcyc_thread_kill(struct cyc_info *cyc, pthread_t tid, int sig)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(pthread_kill(tid, sig));
}

void libcyc_thread_testcancel()
{
	pthread_testcancel();
}

pthread_mutex_t *libcyc_mutex_create(struct cyc_info *cyc)
{
	pthread_mutex_t *mutex = libcyc_malloc(cyc, sizeof(pthread_mutex_t));

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	pthread_mutex_init(mutex, NULL);
	cyc->mutexes = libcyc_ll_append(cyc->mutexes, mutex);

	return(mutex);
}

int libcyc_mutex_lock(struct cyc_info *cyc, pthread_mutex_t *mutex)
{
	pthread_mutex_t *mtx = NULL;
	int found = 0;
	
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	while ((mtx = (pthread_mutex_t *)libcyc_ll_iterate(cyc->mutexes))) {
		if (mtx == mutex) {
			found++;
		}
	}
	
	if (! found) return(ERROR);

	return(pthread_mutex_lock(mutex));
}

int libcyc_mutex_unlock(struct cyc_info *cyc, pthread_mutex_t *mutex)
{
	pthread_mutex_t *mtx = NULL;
	int found = 0;
	
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	while ((mtx = (pthread_mutex_t *)libcyc_ll_iterate(cyc->mutexes))) {
		if (mtx == mutex) {
			found++;
		}
	}
	
	if (! found) return(ERROR);

	return(pthread_mutex_unlock(mutex));
}

int libcyc_mutex_trylock(struct cyc_info *cyc, pthread_mutex_t *mutex)
{
	pthread_mutex_t *mtx = NULL;
	int found = 0;
	
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	while ((mtx = (pthread_mutex_t *)libcyc_ll_iterate(cyc->mutexes))) {
		if (mtx == mutex) {
			found++;
		}
	}
	
	if (! found) return(ERROR);

	return(pthread_mutex_trylock(mutex));
}

void libcyc_mutex_destroy(struct cyc_info *cyc, pthread_mutex_t *mutex)
{
	pthread_mutex_t *mtx = NULL;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return;

	while ((mtx = libcyc_ll_iterate(cyc->mutexes))) {
		if (mutex && (mtx == mutex)) {
			pthread_mutex_destroy(mtx);
			cyc->mutexes = libcyc_ll_remove(cyc->mutexes, mtx);
			libcyc_free(cyc, mtx);
			mtx = NULL;
		} else if (mutex == NULL) {
			pthread_mutex_destroy(mtx);
			cyc->mutexes = libcyc_ll_remove(cyc->mutexes, mtx);
			libcyc_free(cyc, mtx);
			mtx = NULL;
		}
	}
}
