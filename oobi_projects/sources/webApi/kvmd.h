/***************************************************************** 
 * File: kvmd.h 
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
 * Description: This routine contains the main kvm structure 
 *	definitions.
 ****************************************************************/ 

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <dirent.h>

typedef unsigned char 	uchar;

typedef struct _queue {
	unsigned char size;			/* number of allocated elements - Between 1 and MAXKVMSTATIONS */
	unsigned char rwstation;		/* station with rw permission in a shared connection */
	unsigned char port[MAXKVMSTATIONSIDX];	/* ports sharing the same port */
} queue;

struct keycodes {
	unsigned char normal;
	unsigned char shift;
	unsigned char altgr;
}; 

#define UserNameLenght 33

typedef struct _UserMenuCtl {
	char foundmatch;
	char group;
	char name[UserNameLenght+1];
	char menuItem[24];
} UserMenuCtl;


/*****************************************************************
 * _kvm_setting structure - indexed by the station number
 ****************************************************************/ 
#define IPADDRSTRLEN 16
#define XMITBUFLEN 256
struct _kvm_setting {
	/* General settings */
	unsigned char port;		/* station port */
	unsigned char status;		/* status in the state machine */
	unsigned char form;		/* type of form */
	unsigned char mode;		/* master or slave */
	unsigned char name[20];		/* station name */

	/* Serial port settings */
	int inp_fd;			/* file descriptor (station port) */
	int outp_fd;			/* file descriptor (server port) */
	char inp_buf[512];		/* buffer for the station port */
	char outp_buf[512];		/* buffer for the server port */
	int inp_bufidx;		/* buffer position (station port)*/
	int outp_bufidx;		/* buffer position (server port)*/
	char rcv_xmodem;
	char probing;
	char xmit_buf[XMITBUFLEN];
	unsigned int xmit_bufidx;
	
	/* Connection settings */
	unsigned int station_restarted;
	unsigned int physport;
	int index;			/* server number */
	unsigned int cycle;		/* cycle time; 0 if there is no 
					   cycling through ports; */
	unsigned int cycletimer;	/* time when the last cycling occured */
	unsigned char portconn;		/* CAT5 port connected; 0 if 
					   disconnected */
	unsigned char slave;		/* KVM slave CAT5 port if cascading 
					   set; 0 otherwise */
	unsigned char otherport;	/* another station which shares the 
					   same physical port, if appliable */
	queue *portSharingQueue;	/* queue to store all stations sharing the same port */

	unsigned char connstation;	/* the station which will be used to  
					   transmit/receive messages */
	unsigned char portfirst;	/* port to connected for probing 
					   purposes */
	unsigned char group;		/* index for the kvm_slave structure */
	unsigned char groupidx;		/* port index within the kvm_slave 
					   structure */
	unsigned short portlist[MAXKVMSTATIONSIDX]; /* list of ports which 
					   could be used for connection */
	unsigned char portlistindex;
	
	unsigned char probed;
	unsigned char pmprobed;
	unsigned int  probe_to;
	unsigned char pmreqsent;

	unsigned char perm;		/* permissions for the server */
	unsigned char orig_perm;	/* original permissions */
	unsigned char connectcheck;	/* was the port probed yet? */
	unsigned char srvport;          /* physical port currently connected */
	unsigned char servername[64];	/* server name */
	unsigned char station[MAXKVMSTATIONSIDX]; /* other stations connecting to
					   the same target */
	unsigned char rwstation;	/* station which the writing 
					   permission was taken from */
	unsigned char mainport;	
	unsigned char commcycle;	/* cycle by command (= 1) */
	unsigned char testedports[MAXKVMSTATIONSIDX]; /* list of cascaded ports
					already tested for connection */
	/* Probing settings */
	unsigned char rppos;		/* RP mode (remote/local) */
	unsigned char connect;		/* station port connected ? */
	unsigned char device;		/* device connected to the station */
	unsigned char model;		/* model of the device */
	unsigned char connmsgfrom;	/* port where the CONNECTPORT message
					   came from */
	int version;			/* FW version of the device */
	int version2;			/* FW version of the device (local) */
	unsigned int echotime;	/* time when the last ECHO was sent to
					the station port */
	unsigned int inputtime;
	unsigned char connecttries;	
	unsigned char echorx;

	/* Authentication settings */
	unsigned char username[32];	/* username logged in the session */
	unsigned char passwd[32];	/* password for the user */
	unsigned char passwd2[32];	/* password for the user (confirmation) */
	int logintime;			/* time when the user was logged in */

	/* keyboard settings */
	unsigned short keybuf[16];	/* buffer of key messages not sent 
					   to the server yet */
	unsigned char checkseq;		/* sequence check status */
	unsigned long kbdbuf;		/* buffer of the last 4 keys pressed */
	unsigned short keypress;	/* keys pressed (Shift, Ctrl, Alt...) */
	unsigned char leds;		/* keyboard leds */
	unsigned char sunkey;
	unsigned char sunqual;
	int exit_osd; 			/* exiting OSD, waiting to release 
					   the ENTER key*/
	struct keycodes keycode[255];	/* keyboard map */
	unsigned char ASCIIKey[256];

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

	/* power management settings */
	unsigned int pmtimer;		/* time when the last cascaded outlet
					   status was asked */
	unsigned char pmstatus[OUTLETSPERSERVER]; /* status of each outlet */

	/* FW upgrade settings */
	char download;			/* set when in downloading process */
	unsigned int uploadtime;	/* time since last upload answer */
	
	unsigned int connecttime;
	unsigned int connecttime2;
	unsigned int conninfotime;	/* time for the connection info */
	unsigned int quittime;
	unsigned int loginerrtime;

	unsigned char idle;		/* whether the session is idle or not */

	/* OSD user interface settings - these are used to store the option/
	 * value/string set by the user */
	unsigned int option1;		/* integer value */
	unsigned int option2;		/* integer value */
	char option3[2048];		/* string value */
	void *option4;			/* string value */
	unsigned int option5;		/* integer value */
	unsigned int option6;		/* integer value */
	char option3_128[128];		/* string value */

	int step;			/* configuration state (used for network settings) */
	int prevstep;			/* configuration state (used for network settings) */
	int nextstep;			/* configuration state (used for network settings) */
	int steps[32];
	uchar stepidx;

	/* Parameter configuration settings */
	unsigned char *confmenu2[4];	/* Horizontal menu items names */
	unsigned char confmenupos[4];	/* Horizontal menu X positions */
	short cfgmenuidx[4];		/* Horizontal menu values */

	int configtype;			/* Configuration type (general, user,
					   port) */
	unsigned char configidx;	/* index in the config list */
	
	char *confori;			/* Configuration vector to be changed */
	char *confbak;			/* Backup of the last configuration */
	int confsize;			/* size of the configuration vector */
	struct kvm_conf_def *cfglist;		/* List of the configuration 
					   parameters */
	unsigned char *listmenu[30];	/* Menu items for the configuration 
					   type list*/
	short listmenuidx[30];		/* Menu indexes for the configuration 
					   type list*/

	/* PM Menu */
	int pmmenuidx[MAXOUTLS];
	unsigned char *pmmenu[MAXOUTLS];
	unsigned int pmmenusize;
	unsigned int pmindex;

	
	/* ========= Dynamic menu settings ========== */

	/* list of ports */
	short portmenuidx[MAXKVMPORTS + 3]; /* values of the items (slave 
					   names or ports)*/
	unsigned char *portmenu[MAXKVMPORTS + 3]; /* names of the items */
	unsigned char *portmenuhelp[MAXKVMPORTS + 3]; /* names of the items */
	unsigned int portmenusize;	/* number of items */

	/* connection menu */
	short numconnports;		/* number of connectable ports */
	
	/* Sniff menu */
	unsigned char *sniffmenu[4];
	short sniffmenuidx[4];		/* values of the items */

	/* Slave access ports menu */
	unsigned char cfg_group[MAX_CASC_PORTS]; /* list of the ports 
					   connected to the slave */
	unsigned char cfg_grpidx;	/* index of cfg_group */

	/* User access list configuration menus */
	unsigned char **usermenu;	/* list of users in the menu */
	UserMenuCtl *usermenuctl;	/* list of users in the menu */
	unsigned int usermenu_entries;	/* list of users in the menu */
	unsigned int usermenusize;	/* number of items in the menu */
	struct dirent **userlist;	/* structure of the directory 
					   /etc/kvm */
	unsigned int userlistsize;	/* number of files in the directory */
	unsigned char *usernamecfg;	/* username selected/added */
	void *usercfg;			/* pointer to the user configuration 
					   structure */
	unsigned int userport;		/* server number for permission 
					   configuration */
	unsigned char multiport_list[MAXKVMPORTS]; /* List of servers set in 
					   the Multiple Servers option */
	/* Miscellaneous */
	unsigned char old_status;	/* KVM status before the warning 
					   screen */
	unsigned char logged;
	unsigned char msgslot;
	unsigned char net;		/* 1 if it is used by a network */
	int ext_pid;
	int sessid;			/* Web Session ID that connected the 
					   port*/
	unsigned short viewerport;      /* TCP viewer port for Network connection */
	unsigned short viewnatport;     /* TCP viewer port used for nat */
	unsigned char client_viewer[IPADDRSTRLEN];    /* Viewer client IP address */
	unsigned char ack;
	/*Christine: add *authGroupName  09.09.2005*/
	char *authGroupName; //to store the group information come from authentication server
	unsigned int netdisctime;	/* time since the the network started 
					to be disconnected in IP connections */
	unsigned int agc;		/* AGC on/off */
	unsigned int brightness;	/* Brightness */
	unsigned int contrast;		/* Contrast */
	unsigned char slavevideo;	/* Video adjustment mode */
	unsigned int red;
	unsigned int blue;
	unsigned int green;
	unsigned char rgbset;
};

/*****************************************************************
 * _kvm_term structure - indexed by the physical CAT5 port
 ****************************************************************/
struct _kvm_casc_term {
	unsigned char active;
	unsigned char port;
	unsigned char model;		/* device model */
	unsigned char connecttries;	
	int version;			/* FW version of the device */
};

struct _kvm_term {
	unsigned char connect;		/* server port connected? */
	unsigned char device;		/* device connected */
	unsigned char model;		/* device model */
	unsigned char numports;		/* number of server ports in the 
					device connected */
	unsigned char kbdlayout;	/* keyboard layout */
        unsigned char vid_polarity;     /* video polarity //Oct 2003 [DB] */
	unsigned char connecttries;	
	unsigned char name[10];		/* name of the port */
	int version;			/* FW version of the device */
	unsigned int echotime;		/* time when the last ECHO was sent to
					   the server port */
	struct _kvm_casc_term casc;	/* info about the cascade devices */
};


/*****************************************************************
 * _kvm_global structure - not indexed.
 ****************************************************************/ 
struct _kvm_global {
	char prod_name[32];
	unsigned char model;		/* my model */
	unsigned char numports;		/* number of server ports of my 
					device */
	unsigned char numportsS;	/* number of serial ports of my 
					device */
	unsigned char numstations;	/* number of station ports on my 
					device */
	int num_outlets;
	unsigned char type;		/* Board type */
	int version;			/* FW version of the device */
	char version_new[32];           /* FW version of the device including suffix*/
	int cpuclock;
	int ramsize;
	int flashsize;
	unsigned char serial_number[32];
	int fpga_version;
	int board_version;
	unsigned char ipboard[MAXKVMSTATIONS];
	int msgbusy;			/* message buffer busy? */
	int msgqid;			/* message queue ID */
	int msgpid;			/* message sender PID */
	struct msgbuf *msg;		/* receive message buffer */
	unsigned long debuglevel;
	unsigned char pmstatus[MAXOUTLS];	/* status of local PM outlets */
	unsigned char pmlockstatus[MAXOUTLS];
#ifdef RDPenable
	int num_rdpserver;		/* number of RDP servers */
	unsigned short rdp_client[MAXRDPCONNECT];
#endif	//RDPenable
};

/*****************************************************************
 * _kvm_messageglobal structure - not indexed.
 ****************************************************************/ 
struct _kvm_message {
	int msgpid;			/* message sender PID */
	int msgbusy;			/* is kvm handling a message? */
	unsigned char cmdcode;		/* code of the command received */
	unsigned char cmdstation;	/* station to which the command was 
					   directed */
	struct msgbuf *msg;		/* send message buffer */
};


struct _gen_buffer {
	unsigned int  idx;
	unsigned int from;
	unsigned int to;
	unsigned char *buffer;
	unsigned char *bufferstart;
};


struct _outl_info {
	unsigned char port;
	unsigned char device;
	unsigned int  outl;
	unsigned char name[17];/* outlet names are maximum 8 characters long */
	unsigned char onOff;
	unsigned char loUnl;
};

/* Function prototypes */
void kvm_copy_station_config(unsigned char, unsigned char);

#ifdef RDPenable
int g_rdpenabled;
#endif

static inline int GetIpboardFromStation(int station, int type, int bd_ver)
{
    switch (type) {
    case  TYPE_KVMNETP:
        return(station - 2);
        break;
    case TYPE_ONS:
        if (bd_ver == 2) return(station - 1);
        break;
    }
    return(station);
}

static inline int GetStationFromIpboard(int ipBoard, int type, int bd_ver)
{
    switch (type) {
    case  TYPE_KVMNETP:
        return(ipBoard + 2);
        break;
    case TYPE_ONS:
        if (bd_ver == 2) return(ipBoard + 1);
        break;
    }
    return(ipBoard);
}

