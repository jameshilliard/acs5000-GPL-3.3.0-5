/*****************************************************************
 * File: config_lock.h			
 *
 * Copyright (C) 2003 Cyclades Corporation
 *
 * Maintainer: www.cyclades.com
 *	
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version. 
 *
 * Description: Lock/Unlock configuration process.
 *
 ****************************************************************/

#define SIGUSRKILL 33
#define SIGUSRACK  34

void unlock_config_session(int sid);
int lock_config_session(int appl, int sid, char *username);
int cancel_config_session(int wait, volatile int *sig_int, CY_CONFIG_SESSION *sess);
char *get_cnf_session_text(CY_CNF_APPL appl);
void change_config_session(int appl, int sid, char *username);
void send_signal_admin(void);

