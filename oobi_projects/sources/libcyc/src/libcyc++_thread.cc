/* -*- c++ -*- */
/*****************************************************************
 * File: libcyc++_thread.cc
 *
 * Copyright (C) 2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc++_thread
 *
 ****************************************************************/

#include <cstdio>
#include <iostream>
#include <string>
#include <cstring>

#include <pthread.h>

#include <cyc++.h>
#include <cyc.h>

using namespace std;

cyc_mutex::cyc_mutex()
{
	pthread_mutexattr_t attr;
	mutex = new pthread_mutex_t;

	if (pthread_mutexattr_init(&attr)) perror("cyc_mutex");
	if (pthread_mutexattr_setpshared(&attr, 1)) perror("cyc_mutex");
	if (pthread_mutex_init(mutex, &attr)) perror("cyc_mutex");
}

cyc_mutex::~cyc_mutex()
{
	if (mutex) {
		pthread_mutex_destroy(mutex);
		delete mutex;
		mutex = NULL;
	} else {
		cerr << __func__ << ": mutex does not exist" << endl;
	}
}

void cyc_mutex::lock()
{
	if (mutex) {
		if (pthread_mutex_lock(mutex)) {
			perror("cyc_mutex::lock");
			throw app_error("pthread_mutex_lock failed");
		}
	} else {
		cerr << __func__ << ": mutex does not exist" << endl;
	}
}

void cyc_mutex::unlock()
{
	if (mutex) {
		if (pthread_mutex_unlock(mutex)) {
			perror("cyc_mutex::unlock");
			throw app_error("pthread_mutex_unlock failed");
		}
	} else {
		cerr << __func__ << ": mutex does not exist" << endl;
	}
}

void cyc_mutex::trylock()
{
	if (mutex) {
		if (pthread_mutex_trylock(mutex)) {
			perror("cyc_mutex::trylock");
			throw app_error("pthread_mutex_trylock failed");
		}
	} else {
		cerr << __func__ << ": mutex does not exist" << endl;
	}
}

cyc_thread::cyc_thread()
{
	if (pthread_attr_init(&attr)) {
		throw app_error("pthread_attr_init failed");
	}
}

void cyc_thread::create(void *(*start_routine)(void *))
{
	if (pthread_create(&thread, &attr, start_routine, this)) {
		throw app_error("pthread_create failed");
	}
}

void cyc_thread::create(void *(*start_routine)(void *), void *arg)
{
	if (pthread_create(&thread, &attr, start_routine, arg)) {
		throw app_error("pthread_create failed");
	}
}

void cyc_thread::exit(int retval)
{
	pthread_exit(&retval);
}

void cyc_thread::kill(int sig)
{
	pthread_kill(thread, sig);
}

int cyc_thread::cancel()
{
	return(pthread_cancel(thread));
}

void *cyc_thread::join()
{
	void *ret;

	pthread_join(thread, &ret);

	return(ret);
}

void cyc_thread::testcancel()
{
	pthread_testcancel();
}

bool cyc_thread::equal(pthread_t tid)
{
	return(pthread_equal(get_id(), tid) ? true : false);
}
