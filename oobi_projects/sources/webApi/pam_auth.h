/**
 *  pam_auth.h  -  Prototypes for pam_auth.c
 *  Copyright (C), 2004 Cyclades Corporation
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  This file is part of webApi
 */

#ifndef PAM_AUTH_H
#define PAM_AUTH_H

/**
 * global include files
 */

#define PAM_WEB 0
#if defined(KVM) || defined(ONS)
#define PAM_KVM 1
#define PAM_OSD 2
#endif

int PamAuthenticate(const char *user, const char *passw, int type);
#ifdef GRPAUTHenable
void PAM_get_group_name(char ** group_name);
#endif
#endif /* PAM_AUTH_H */
