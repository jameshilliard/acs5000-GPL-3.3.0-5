/* -*- linux-c -*- */
/*****************************************************************
 * File: KVMFunc.h
 *
 * Copyright (C) 2004 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * Description: Header for the KVM Functions
 *
 ****************************************************************/
#ifndef KVMFUNC_H
#define KVMFUNC_H

#include <webs.h>

#define MAX_PORT_FIELD_LEN 32
#define MAX_RESULT_LEN WEBS_BUFSIZE

#define MAXConnectLine  100

typedef struct {
   char_t kvmconnport[MAX_PORT_FIELD_LEN];    /* for the KVM connect port
						 field in the form, 
						 if necessary */
   char_t kvmconnresult[128]; /* for the KVM connect port field 
				 in the form, if necessary */
} KVMReqData; /* note, this is only to be used transiently! */

typedef struct {
	unsigned char user[32];
	unsigned char admin;
	int nports;
	struct _KVMport {
		unsigned char name[32];
		unsigned char help[32];
		unsigned char perm;
	} port[MAXKVMPORTS];
} KVMportlist;

#ifdef RDPenable
extern void RDPGetSrvInfo(void);
extern int GetRDPConnectKVMPorts(Param *,int);
extern void portMountRDPHtml(char *portname, char *connect, webs_t wp);

#define lRDP_name 21
#define lRDP_ipport 80
#define lRDP_kvmport 21
#define lRDP_user 33
#define lRDP_time 65

typedef struct {
	int nservers;
	struct _RDPserver {
		unsigned char name[lRDP_name];
		unsigned char ip_port[lRDP_ipport];
		unsigned char  kvmport[lRDP_kvmport];
	} server[MAXRDPSERVERS];
} RDPsrvlist;


typedef struct {
		unsigned char srvip[lRDP_ipport];
		unsigned char cliip[lRDP_ipport];
		unsigned short srvport;
		unsigned short cliport;
		unsigned char kvmuser[lRDP_user];
		unsigned char rdpkill[lRDP_user];
		unsigned int conn_time;
		unsigned int rdpasix;
} tRDPconnect;

#endif	//RDPenable

void KVMGetUserInfo(char_t *, KVMportlist *, char_t *);//Christine add the third parameter
int GetKVMConnectPortsHtml(Param *,int);
void portMountViewerHtml(char *, char *, char *, webs_t);
int ShowPortList(int, webs_t, int, char_t **);
int KVMConnectPort(Param *param, int, char_t *);
int KVMConnQuery(Param *, int);
void KVMIPCascadeConnection(webs_t,UInt32,char_t *,char_t *,char_t *, char_t *);

int checkPortStatusNet(char_t *, char_t *, webs_t);
unsigned int GetWindowNumber(webs_t);

#endif /* KVMFUNC_H */
