/* -*- c++ -*- */
/*****************************************************************
 * File: libcyc++_signals.cc
 *
 * Copyright (C) 2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc++_signals
 *
 ****************************************************************/

#include <string>
#include <cstring>
#include <stack>
#include <csignal>
#include <cerrno>

#include <pthread.h>

#include <cyc++.h>
#include <cyc.h>

using namespace std;

const string signals::SIG_FN_INVALID = "Invalid signal function";
const string signals::SIG_INIT_ERR = "Error initializing signals";
const string signals::SIG_ADD_ERR = "Error adding signals";
const string signals::SIG_HANDLE_ERR = "Error activating signal handling";

void csignals::add(int signum, void (*sig_fn)(int))
{
	if (sig_fn == NULL) throw app_error(SIG_FN_INVALID);

	struct signal_data *sigdata = new signal_data;

	sigdata->signum = signum;
	sigdata->sig_fn = sig_fn; 
	
	sigs.push(sigdata);
}

void csignals::handle()
{
	struct signal_data *sigdata = NULL;

	while (! sigs.empty()) {
		sigdata = sigs.top();
		if (signal(sigdata->signum, sigdata->sig_fn) == SIG_ERR) {
			delete sigdata;
			throw app_error(strerror(errno));
		}
		sigs.pop();
		delete sigdata;
		sigdata = NULL;
	}
}

psignals::psignals()
{
	if (sigemptyset(&sigset) == -1) throw app_error(strerror(errno));
}

void psignals::add(int signum, void (*sig_fn)(int))
{
	if (sig_fn == NULL) throw app_error(SIG_FN_INVALID);

	struct signal_data *sigdata = new signal_data;

	sigdata->signum = signum;
	sigdata->sig_fn = sig_fn; 
	
	sigs.push(sigdata);
}

void psignals::handle()
{
	struct signal_data *sigdata = NULL;

	while (! sigs.empty()) {
		sigdata = sigs.top();

		struct sigaction sigact;
		sigset_t sigfullset;

		sigfillset(&sigfullset);

		sigact.sa_handler = sigdata->sig_fn;
		sigact.sa_mask = sigfullset;
		sigact.sa_flags = 0;
		
		if (sigaction(sigdata->signum, &sigact, NULL) == -1) {
			delete sigdata;
			throw app_error(strerror(errno));
		}
		
		sigs.pop();
		delete sigdata;
		sigdata = NULL;
	}
}

void psignals::add(int signum)
{
	if (sigaddset(&sigset, signum) == -1) throw app_error(strerror(errno));
}

void psignals::clear()
{
	if (sigemptyset(&sigset) == -1) throw app_error(strerror(errno));
}

void psignals::mask(int how)
{
	if (sigprocmask(how, &sigset, NULL) == -1)
		throw app_error(strerror(errno));
}

int psignals::wait()
{
	int sig_caught;

	if (sigwait(&sigset, &sig_caught)) throw app_error(strerror(errno));

	return(sig_caught);
}

void psignals::ignore()
{
	struct signal_data *sigdata = NULL;
		
	while (! sigs.empty()) {
		sigdata = sigs.top();

		struct sigaction sigact;
		sigset_t sigfullset;

		sigfillset(&sigfullset);

		sigact.sa_handler = SIG_IGN;
		sigact.sa_mask = sigfullset;
		sigact.sa_flags = SA_NOCLDSTOP;
		
		if (sigaction(sigdata->signum, &sigact, NULL) == -1) {
			delete sigdata;
			throw app_error(strerror(errno));
		}
		
		sigs.pop();
		delete sigdata;
		sigdata = NULL;
	}
}

void psignals::ignore(int sig)
{
	struct sigaction sigact;
	sigset_t sigfullset;
	
	sigfillset(&sigfullset);
	
	sigact.sa_handler = SIG_IGN;
	sigact.sa_mask = sigfullset;
	sigact.sa_flags = SA_NOCLDSTOP;
	
	if (sigaction(sig, &sigact, NULL) == -1) {
		throw app_error(strerror(errno));
	}
}

void psignals_th::mask(int how)
{
	if (pthread_sigmask(how, &sigset, NULL))
		throw app_error(strerror(errno));
}
