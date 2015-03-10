/* -*- c++ -*- */
/*****************************************************************
 * File: cyc++.h
 *
 * Copyright (C) 2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc++ header
 *      
 ****************************************************************/

#ifndef LIBCYCPP_H
#define LIBCYCPP_H 1

#include <iostream>
#include <string>
#include <stack>
#include <csignal>
#include <sstream>

#include <pthread.h>

#include <cyc.h>

// Info about this library
class libcycpp_info {
	static const std::string LIBCYCPP_NAME;
	static const std::string LIBCYCPP_VERSION;
public:
	static const std::string get_name() { return(LIBCYCPP_NAME); }
	static const std::string get_version() { return(LIBCYCPP_VERSION); }
	static const std::string get_license() { return(LICENSE); }
};

// Exception handlers
class app_exception {
protected:
	std::string str;
public:
	virtual ~app_exception() {}
	virtual void set(std::string s) { str = s; }
	virtual std::string get_str() { return(str); }
	virtual void print() { std::cerr << str << std::endl; }
};

class app_error : public app_exception {
public:
	app_error(std::string s) { str = s; }
	void print() const { std::cerr << "Error: " << str << std::endl; }
};

class app_notice : public app_exception {
public:
	app_notice(std::string s) { str = s; }
	void print() const { std::cout << "Notice: " << str << std::endl; }
};

// Signal handling base class
class signals {
protected:
	static const std::string SIG_FN_INVALID;
	static const std::string SIG_INIT_ERR;
	static const std::string SIG_ADD_ERR;
	static const std::string SIG_HANDLE_ERR;
public:
	virtual ~signals() {}
	virtual void add(int, void (*sig_fn)(int)) = 0;
	virtual void handle() = 0;
};

struct signal_data {
	int signum;
	void (*sig_fn)(int);
};

// C signal handling
// The operation is simple: Add signals (by number) along with the
// desired handler function using add(); call "handle" when you want
// the association to be made with the "signal()" function call
class csignals: public signals {
	std::stack<struct signal_data *> sigs;
public:
	void add(int, void (*sig_fn)(int));
	void handle();
};

// POSIX signal handling
// The basic use, defined in the signals base class, is the same as
// the use for csignals except that sigaddset() and sigaction() are
// used instead of signal().  This class also offers the additional
// features that POSIX signalling provides beyond basic C-style signal
// handling.
class psignals : public signals {
	std::stack<struct signal_data *> sigs;
protected:
	sigset_t sigset;
public:
	psignals();

	// Virtual functions inherited from signals class
	void add(int, void (*sig_fn)(int));
	void handle();

	// POSIX-specific signal handling
	void add(int); // Adds the signal using sigaddset to sigset
	void clear(); // Clears sigset
	void mask(int); // Set signal mask; uses sigprocmask()
	int wait(); // Wait for sigs in set; return sig caught
	void ignore(); // Ignore sigs in the set
	void ignore(int); // Ignore one signal
};

// POSIX signal handling when using threads
class psignals_th : public psignals {
public:
	void mask(int); // Set signal mask; uses pthread_sigmask()
};

class cyc_ipc {
 protected:
	key_t ipc_key;
 public:
	cyc_ipc(std::string, int);
	cyc_ipc(int);
};

// Shared memory using Sys V
class cyc_shmem : cyc_ipc {
	int shmid;
	void *mem;
 public:
	cyc_shmem(std::string, int);
	cyc_shmem(int);
	~cyc_shmem();
	void create(size_t);
	void create(size_t, int);
	void destroy();
	void *attach() { return(attach(NULL, 0)); }
	void *attach(const void *, int);
	void detach();
	void *getmem() { return(mem); }
	int get_id() { return(shmid); }
};

// Semaphores using Sys V
class cyc_sem : cyc_ipc {
	int semid;
	int nsems;
 public:
	cyc_sem(std::string, int);
	cyc_sem(int);
	~cyc_sem();
	void create(int, int);
	void create(int);
	void destroy();
	int op(int, int, int);
	int op(int, int);
	int timed_op(int, int, long int, int);
	int timed_op(int, int, long int);
	int getval(int);
	void setval(int, int);
	void set_all(int);
	int get_id() { return(semid); }
	int get_nsems() { return(nsems); }
};

// Mutex using POSIX
class cyc_mutex {
	pthread_mutex_t *mutex;
 public:
	cyc_mutex();
	~cyc_mutex();
	void lock();
	void unlock();
	void trylock();
};

// Threads using POSIX
class cyc_thread {
	pthread_t thread;
	pthread_attr_t attr;
 public:
	cyc_thread();
	void create(void *(*)(void *));
	void create(void *(*)(void *), void *);
	void exit(int);
	void kill(int);
	int cancel();
	void *join();
	void testcancel();
	pthread_t get_id() { return(pthread_self()); }
	bool equal(pthread_t);
};

// Application classes
class app {
 public:
	virtual ~app() {}
	virtual void print_title() = 0;
	virtual void print_help() = 0;
};

// Application that will use libcyc
class libcyc_app : app {
protected:
	static const std::string LIBCYC_INIT_ERR;
	static int libcyc_app::secs_toquit;
	struct cyc_info *cyc;
public:
	libcyc_app(int, std::string, std::string,
		   std::string, std::string);
	~libcyc_app();
	cyc_info *get_cyc() { return(cyc); }
};

// Command-line utility using libcyc
class libcyc_util : public libcyc_app {
public:
	csignals sighandler;

	libcyc_util(int, std::string, std::string, std::string, std::string);
};

// Base class for utility operations
class libcyc_util_ops {
protected:
	struct cyc_info *cyc;
public:
	virtual ~libcyc_util_ops() {}
	libcyc_util_ops(struct cyc_info *c) {
		if (c == NULL)
			throw app_error("libcyc unavailable; cannot proceed");
		cyc = c;
	};
	virtual void help();
	virtual void print();
};

// Daemon using libcyc
class libcyc_daemon : public libcyc_app {
	int exit_status;
public:
	psignals_th sighandler;
	cyc_thread sig_th;
	cyc_sem *sig_sem;

	libcyc_daemon(int, std::string, std::string, std::string,
		      std::string, int);
	~libcyc_daemon();
	void quit(int);
	int get_exit_status() { return(exit_status); }
};

// Class to create help screens in command-line programs
class cyc_help {
	static const unsigned int TAB_COLS;
	static const unsigned int COL1_SIZE;
	static const unsigned int COL2_SIZE;
	static const unsigned int COL3_SIZE;

	std::ostringstream help_message;

	void indent();
	void indent(int);
	void break_line(std::string &);
public:
	void add_opt(std::string, std::string, std::string);
	void add_opt(std::string, std::string);
	void add_opt();
	void print();
};

#endif /* LIBCYCPP_H */
