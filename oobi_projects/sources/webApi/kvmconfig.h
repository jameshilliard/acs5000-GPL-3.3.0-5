/***************************************************************** 
 * File: kvmconfig.h 
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
 * Description: This part contains the definitions and prototypes 
 * for the kvmconfig.c module
 ****************************************************************/ 
#include <time.h>
#ifndef h_KVMConfig
#define h_KVMConfig

/* These following defines will be unsed for the CLI configuration */
#ifdef KVM_CLI_CFG
/* General configuration */
enum {
	CLI_KVM_AUTHTYPE, 
	CLI_KVM_AUTHDOWNLOCAL,
	CLI_KVM_FACILITY,
	CLI_KVM_AUTHHOST1,
	CLI_KVM_AUTHHOST2,
	CLI_KVM_ACCTHOST1,
	CLI_KVM_ACCTHOST2,
	CLI_KVM_AUTHTIMEOUT,
	CLI_KVM_AUTHRETRIES,
	CLI_KVM_AUTHSECRET,
	CLI_KVM_SECURITY,
};

/* Server configuration */
enum {
	CLI_KVM_PORT,
	CLI_KVM_ALIAS,
	CLI_KVM_LCLAGC,
	CLI_KVM_LCLBRIGHT,
	CLI_KVM_LCLCONTRAST,
	CLI_KVM_REMAGC,
	CLI_KVM_REMBRIGHT,
	CLI_KVM_REMCONTRAST,
	CLI_KVM_PMOUTLET
};

/* Station configuration */
enum {
	CLI_KVM_IDLETIMEOUT,
	CLI_KVM_SCREENSAVERTIME,
	CLI_KVM_CYCLETIME,
	CLI_KVM_KBDTYPE,
        CLI_KVM_ESCAPESEQ,
        CLI_KVM_POWERSEQ,
	CLI_KVM_RPSWITCHSEQ,
	CLI_KVM_SRVSYNCSEQ,
	CLI_KVM_VIDEOCONFSEQ,
	CLI_KVM_SWITCHNEXTSEQ,
	CLI_KVM_SWITCHPREVSEQ,
	CLI_KVM_PORTINFOSEQ
};

/* Slave list */
enum {
	CLI_KVM_SLAVENAME,
	CLI_KVM_SLAVEREMOTE,
	CLI_KVM_SLAVELOCAL,
	CLI_KVM_SLAVENPORTS
};

#endif

/* Configuration structure definitions */

struct escape_seq{
        unsigned char qualifier;
        unsigned char key;
};
struct lockoutmacro_seq{
        unsigned char qualifier;
        unsigned char key[15];
};

struct prevPortUser{
	char username[32];
	struct lockoutmacro_seq lockoutmacro;
	unsigned int station;
};
/* General configuration */
struct _kvm_conf {
    int authtype;
	int facility;
	int auxp_facility;
	unsigned char escapeseq[4];
	struct escape_seq esc_seq;
	struct escape_seq sunmodifier;
	int ipsecurity;
	int des;
	int directacc;
	unsigned char *viewerport;
#ifdef RDPenable
	unsigned char *rdpport;
#endif	//RDPenable
};

/* Server configuration */
struct _kvm_term_conf {
	int active;
	unsigned int port;
	unsigned char *alias;
	//unsigned char *lockoutmacro;	
	//struct escape_seq  lockoutmacro;
	struct lockoutmacro_seq lockoutmacro;
	unsigned short pmoutlet[OUTLETSPERSERVER + 1];
	int tcpport;
	int lclagc;
	int remagc;
	/* Don't change this order ! */
	int lclbright;
	int rembright;
	int ip1bright;
	int ip2bright;
	int ip3bright;
	int ip4bright;
	int lclcontr;
	int remcontr;
	int ip1contr;
	int ip2contr;
	int ip3contr;
	int ip4contr;
	/* */
	unsigned short portmap;
	unsigned char name[21];
	unsigned char help[21];
};

/* Station configuration */
struct _kvm_station_conf {
	unsigned int idletimeout;
	unsigned int screensavertime;
	unsigned int cycletime;
	unsigned int kbdtype;

	struct escape_seq quitseq;
	struct escape_seq powerseq;
	struct escape_seq srvsyncseq;
	struct escape_seq videoconfseq;
	struct escape_seq switchnextseq;
	struct escape_seq switchprevseq;
	struct escape_seq portinfoseq;
};

struct pmstate_status {
	int noutlets;
	time_t timer;
	unsigned char *status;
};

/* Slave list */
struct _kvm_slaves {
	unsigned char nports;			/* number of ports of the 
						 * device */
	unsigned char name[21];			/* name of the slave */
	unsigned char type;			/* type of the slave (switch=0, 
						 * pem=1) */
	unsigned char net;			/* 1 if network, 0 if 
						 * physical */
	unsigned char host[64];			/* host name / IP of the net 
						 * cascaded device */
	unsigned char device;			/* Device to whom the slave is 
						 * connected to */
	unsigned char ports[MAXKVMSTATIONSIDX];	/* Ports in the device to 
						 * connect */
	int  pm_index;
	struct _kvm_term_conf termconf[MAXKVMTERMS];
	unsigned char next;
	unsigned char prev;
	struct pmstate_status pmstate_cache; 
};

#ifdef NISenable
typedef struct {
	unsigned char *domainName;
	unsigned char *serverIp;
} nis_conf;
#endif

typedef struct {
	unsigned char *ldapServer;
	unsigned char *ldapBase;
	unsigned char *ldapUser;
	unsigned char *ldapPassword;
	unsigned char *ldapLoginAttribute;
	unsigned int secure;
} ldap_conf;

#ifdef KRBenable
typedef struct {
	unsigned char *krbServer;
	unsigned char *krbDomainName;
} krb_conf;
#endif

typedef struct {
    unsigned char *authServer;
	unsigned char *auth2Server;
	unsigned char *accServer;
	unsigned char *acc2Server;
	unsigned char *secret;
	unsigned int raccess; //Christine support raccess. 09.10.2005
	unsigned int timeout;
	unsigned int retries;
} tac_conf;

typedef struct {
	unsigned char *authServer;
	unsigned char *auth2Server;
	unsigned char *accServer;
	unsigned char *acc2Server;
	unsigned char *secret;
	unsigned int timeout;
	unsigned int retries;
} rad_conf;

#ifdef SMBenable
typedef struct {
	unsigned char *domain;
	unsigned char *server;
	unsigned char *server2;
} smb_conf;
#endif

struct _auth_conf {
	unsigned int authtype;
#ifdef NISenable
	nis_conf nis;
#endif
	ldap_conf ldap;
#ifdef KRBenable
	krb_conf krb;
#endif
	rad_conf radius;
	tac_conf tacplus;
#ifdef SMBenable
	smb_conf smb;
#endif
};

#ifdef RDPenable
struct _rdp_server_conf {
	unsigned char *alias;	// Alias for the server
	unsigned char *ipaddr;	// IP addr or FQDN
	unsigned int  serverport; // TCP port where server listen to RDP
	unsigned char *kvmport;	// KVM port name for KVM connection
};
#endif	//RDPenable

/*****************************************************************
 * _kvm_server structure - indexed by the server number
 ****************************************************************/ 
struct _kvm_server {
	unsigned char device;
	unsigned char port;
};

/* Keyboard types */
#define	KBD_US		0
#define	KBD_ABNT	1
#define	KBD_ABNT2	2
#define	KBD_JP		3
#define	KBD_DE		4
#define	KBD_IT		5
#define	KBD_FR		6
#define	KBD_ES		7

/* several configuration file names */
#define LDAP_CONF_FILE       "/etc/ldap.conf"
#ifdef KRBenable
#define KRB5_CONF_FILE       "/etc/krb5.conf"
#endif
#define RADIUS_CONF_FILE        "/etc/raddb/server"
#define TACPLUS_CONF_FILE   "/etc/tacplus.conf"
#ifdef NISenable
#define DOMAINNAME_CONF_FILE "/etc/domainname.conf"
#define YP_CONF_FILE         "/etc/yp.conf"
#endif
#ifdef SMBenable
#define SMB_CONF_FILE           "/etc/pam_smb.conf"
#endif
#define PSLAVE_FILE          "/etc/portslave/pslave.conf"
#define PMDCONF_FILE         "/etc/pmd.conf"
#ifdef NISenable
#define NIS_SHELL			"/etc/daemon.d/ypbind.conf"
#endif

/* web station */
#define	WEB_STATION	0

/* Configuration structure variables */
#ifdef KVMCONFIG_C
	struct _kvm_conf kvmconf;
	struct _kvm_station_conf kvmstationconf[MAXKVMSTATIONSIDX + 1];
	struct _kvm_slaves kvm_slave[MAXKVMSLAVES];
	struct _kvm_server kvmserver[MAXKVMPORTS];
	unsigned char kvm_ndevices;
	unsigned int kvm_availports;
	struct _auth_conf authconf;
#ifdef RDPenable
	struct _rdp_server_conf rdpserverconf[MAXRDPSERVERS];
#endif	//RDPenable
#else
	extern struct _kvm_conf kvmconf;
	extern struct _kvm_station_conf kvmstationconf[MAXKVMSTATIONSIDX + 1];
	extern struct _kvm_slaves kvm_slave[MAXKVMSLAVES];
	extern struct _kvm_server kvmserver[MAXKVMPORTS];
	extern unsigned char kvm_ndevices;
	extern unsigned int kvm_availports;
	extern struct _auth_conf authconf;
#ifdef RDPenable
	extern struct _rdp_server_conf rdpserverconf[MAXRDPSERVERS];
#endif	//RDPenable
#endif

/* Definitions of number of stations and number of ports */
#ifdef KVMD
#define NUMSTATIONS		kvmgbl.numstations
#define NUMPORTS		kvmgbl.numports
#else
#ifdef KVM_WEBS_CFG
#define NUMSTATIONS		kvmnumstations
#define NUMPORTS		kvmnumports
#ifdef KVMCONFIG_C
unsigned char kvmnumports, kvmnumstations;
#else
extern unsigned char kvmnumports, kvmnumstations;
#endif
#define CONF_CFG_SIZE	16
#define TERM_CFG_SIZE	16
#define USER_CFG_SIZE	16
#else
#define NUMSTATIONS	kvmgbl.numstations	
#define NUMPORTS		MAXKVMTERMS
#endif
#endif

#define get_termconf(i) (&(kvm_slave[kvmserver[i].device].termconf[kvmserver[i].port]))


/* Prototypes */
int kvmreadcfg(void);
void kvmdefaultcfg(void);
int kvmwritecfg(void);
void kvmreadslavelist(void);
void kvmwriteslavelist(void);
int kvmaddslaveentry(unsigned char *, unsigned char *, 
	unsigned char, unsigned char, unsigned char,
	unsigned char *, unsigned char);
int kvmdelslaveentry(int);
int kvmchgslaveentry(int, unsigned char *, unsigned char, 
	unsigned char, unsigned char, unsigned char *, unsigned char);

int kvmreadauth(void);
int kvmwriteauth(void);
#ifdef KRBenable
void kvm_write_auth_krb(void);
#endif
void kvm_write_auth_ldap(void);
void kvm_write_auth_radius(void);
#ifdef SMBenable
void kvm_write_auth_smb(void);
#endif
int kvm_rw_auth_type(int wrt_flg);
void kvm_rw_auth_tacplus(int wrt_flg);
void kvm_rw_facility(int wrt_flg);
#ifdef NISenable
void kvm_rw_auth_nis(int wrt_flg);
#endif
//#ifdef KVMD
#define DEFAULTVIEWERPORT 5900
unsigned int parse_viewerport(char *ptr, unsigned short *portlist);
int check_viewerport (char *ptr);
//#endif

#ifdef RDPenable
#define DEFAULTRDPPORT 3389
unsigned int parse_rdpport(char *ptr, unsigned short *portlist);
int check_rdpport (char *ptr);
#endif	//RDPenable

#endif
