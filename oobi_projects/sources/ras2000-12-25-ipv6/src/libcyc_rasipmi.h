/*****************************************************************
 * File: ipmi.h
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
 * Description: The ras interface to IPMI for the hotkey menu
 *
 ****************************************************************/

#ifndef LIBCYC_RASIPMI_H
#define LIBCYC_RASIMPI_H 1

#include "server.h"

#define MAX_OPT_SIZE	50

void ipmi_login(CY_SESS *,int,char,
		void write_line(char *,int));
void ipmi_logout(CY_SESS *);
void ipmi_treat_input(CY_SESS *, void write_data(char *,int));

#endif
