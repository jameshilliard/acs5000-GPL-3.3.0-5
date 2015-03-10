/* -*- c++ -*- */
/*****************************************************************
 * File: libcyc++_main.cc
 *
 * Copyright (C) 2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc++_main
 *
 ****************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include <unistd.h>

#include <cyc++.h>
#include <cyc.h>

using namespace std;

const string libcycpp_info::LIBCYCPP_NAME = "libcyc++";
const string libcycpp_info::LIBCYCPP_VERSION = "0.0";
const string libcyc_app::LIBCYC_INIT_ERR = "Could not initialize libcyc";
int libcyc_app::secs_toquit = 10;

libcyc_app::libcyc_app(int f, string syslog_name,
		       string app_version, string lck_filepath,
		       string tmpdir)
{
	while ((cyc = libcyc_init(static_cast<enum libcyc_flags>(f),
			     syslog_name.c_str(), app_version.c_str(),
			     lck_filepath.c_str(), tmpdir.c_str())) == NULL) {
		if (f & CYC_DEBUG) cerr << syslog_name << ": waiting for lock" << endl;
		sleep(1);
		secs_toquit--;
		if (secs_toquit <= 0) throw app_error(LIBCYC_INIT_ERR);
	}
}

libcyc_app::~libcyc_app()
{
	if (cyc) {
		libcyc_destroy(cyc);
		cyc = NULL;
	}
}

libcyc_util::libcyc_util(int f, string syslog_name,
			 string app_version,
			 string lck_filepath,
			 string tmpdir) :
	libcyc_app(f, syslog_name, app_version, lck_filepath, tmpdir) {
	if (cyc == NULL) throw app_error(LIBCYC_INIT_ERR);
}

libcyc_daemon::libcyc_daemon(int f, string syslog_name,
			     string app_version,
			     string lck_filepath,
			     string tmpdir, int sig_ftok) :
	libcyc_app(f, syslog_name, app_version, lck_filepath, tmpdir) {
	if (cyc == NULL)
		throw app_error(LIBCYC_INIT_ERR);
	
	sig_sem = new cyc_sem(cyc->lock_filepath, sig_ftok);
	exit_status = 0;
}

libcyc_daemon::~libcyc_daemon()
{
	if (sig_sem) {
		delete sig_sem;
		sig_sem = NULL;
	}
}

void libcyc_daemon::quit(int status)
{
	exit_status = status;

	sig_th.kill(SIGQUIT);
	sig_th.join();
}

const unsigned int cyc_help::TAB_COLS = 4;
const unsigned int cyc_help::COL1_SIZE = 18;
const unsigned int cyc_help::COL2_SIZE = 19;
const unsigned int cyc_help::COL3_SIZE = 38;

void cyc_help::indent()
{
	indent(TAB_COLS);
}

void cyc_help::indent(int hspace)
{
	for (int i = 0; i < hspace; i++) help_message << " ";
}

void cyc_help::break_line(string &line)
{
        unsigned int spos, pos;
	bool first = true;
	bool last = false;

        for (pos = line.rfind(" ", COL3_SIZE - 1), spos = 0;
             (pos != string::npos) && (spos < line.size() && (spos < pos));
             spos = ++pos, pos = line.rfind(" ", pos + (COL3_SIZE - 1))) {
		int len;

		// We must indent lines that aren't the first to line
		// up the text
		if (! first) indent(TAB_COLS + COL1_SIZE + COL2_SIZE);

		if ((line.size() - spos) < COL3_SIZE) {
			// Last line
			len = line.size() - spos;
			last = true;
		} else {
			len = pos - spos;
		}
		help_message << line.substr(spos, len);
		if (! last) help_message << endl;
		first = false;
        }
}

void cyc_help::add_opt(string opt, string arg, string desc)
{
	if ((opt.size() > COL1_SIZE) ||
	    (arg.size() > COL2_SIZE)) {
		throw app_error("Help option/argument too long");
	}

	indent();
	help_message << opt;
	indent(COL1_SIZE - opt.size());
	help_message << arg;
	indent(COL2_SIZE - arg.size());

	if (desc.size() <= COL3_SIZE) {
		help_message << desc;
	} else {
		break_line(desc);
	}

	help_message << endl;
}

void cyc_help::add_opt(string opt, string desc)
{
	indent();
	help_message << opt;
	indent(COL1_SIZE - opt.size() + COL2_SIZE);
	help_message << desc << endl;
}

void cyc_help::add_opt()
{
	help_message << endl;
}

void cyc_help::print()
{
	cout << help_message.str();
}
