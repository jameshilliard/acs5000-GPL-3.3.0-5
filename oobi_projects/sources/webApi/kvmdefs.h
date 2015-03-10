/***************************************************************** 
 * File: kvmdefs.h 
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
 * Description: These are the definitions on the KVM daemon
 ****************************************************************/ 
#ifndef h_KVMdefs
#define h_KVMdefs

#define MAXKVMTERMS		48
#define MAXKVMPORTS		(MAXKVMTERMS * (1 + MAXKVMTERMS))
#define MAXKVMSLAVES		64
#define MAXKVMSTATIONS		6	
#define MAXKVMSTATIONSIDX	6
#define MAX_CASC_PORTS		2
#define MAXOUTLS		1024
#define OUTLETSPERSERVER	8	/* changed maximum number of power outlets per server from 2 to 8 */ 

#define MAXVIEWERPORTLEN	25
#define MAXVIEWERPORTS		4

// Also defined in kvmnet_utils/src/include/kvmnet_util.h	
#define MIN_VIEWER_PORT 	1024
#define MAX_VIEWER_PORT 	65535


#ifdef RDPenable
#define MAXRDPSERVERS		(MAXKVMTERMS * (MAXKVMSLAVES + 1))
#define MAXRDPCONNECT		8
#define MAXRDPLIST			MAXRDPCONNECT*8
#define MAXRDPPORTLEN		25
#define RDP_PORT_DFL 		3389
#define MIN_RDP_PORT 		1024
#define MAX_RDP_PORT 		65535
#endif	//RDPenable

/* KVM states */
enum {KVM_IDLE, KVM_OSDFORM, KVM_CONNECTED, KVM_CONNECTING, KVM_WAITCONN, 
	KVM_ADMDOWN, KVM_PROBING, KVM_CHECK_POWER, KVM_WARNING, KVM_RESETTING};

/* KVM modes */
enum {MODE_MASTER, MODE_SLAVE, MODE_NONE};

/* KVM command error codes */
enum {errSlotFull = 100, errNoPort, errUplKill, errInvalidDev, errUplConflict,
	errnoFile, errFileNotOpened, errDeviceError, errDeviceRebootError, 
	errTransfCancel, errFileIncompatible, errOldVersion, errInconsistentRequest, errTimeout, errNoSlot};

/* KVM board types */
enum {TYPE_ANALOG, TYPE_KVMNET, TYPE_ONS, TYPE_ANALOGP, TYPE_KVMNETP};

#define	MAX_CONN_TRIES		2
/* KVM permissions */
#define PERM_NOACCESS		0
#define PERM_READ       	1
#define PERM_WRITE      	2
#define PERM_POWER	     	4
#define PERM_RW		     	(PERM_READ | PERM_WRITE)
#define PERM_FULL	     	(PERM_READ | PERM_WRITE | PERM_POWER)
#define PERM_DEFAULT		0x80

#ifdef RDPenable
#define PERM_RDP	     	8
#endif	//RDPenable

/* CAT5 protocol commands */
#define	CAT5_KEYBOARD		0xc0		/* a keyboard package */
#define	CAT5_MOUSE		0xa0		/* a mouse package */
#define	CAT5_RESET		0x9f		/* reset request */
#define	CAT5_GETINFO		0x9e		/* information required */
#define	CAT5_SHOWINFO		0x9d		/* send information */
#define	CAT5_ECHO		0x9c		/* echo request */
#define CAT5_BOOT		0x9b		/* device booting */
#define	CAT5_BOOTANS		0x9a		/* boot response (skip or 
						perform download) */
#define	CAT5_CONNECTPORT	0x99		/* command to connect a 
						specific port */
#define	CAT5_TERMSTATUS		0x98		/* information about a 
						specific terminator port */
#define	CAT5_XMODEM		0x97		/* encapsulation for xmodem */
#define	CAT5_RPSWITCH		0x96		/* command for the RP to 
						switch from local to remote 
						management */
#define	CAT5_SWITCHED		0x95		/* signals that the server has 
						been switched and the keyboard/
						mouse must be reset */
#define CAT5_PMCOMMAND		0x94		/* command sent to a slave KVM 
						switch to perform a power 
						management command */
#define CAT5_PMBUFFER		0x93		/* command sent to a slave KVM 
						switch to perform a power 
						management command */
#define CAT5_PMCOMMANDBUFFER		0x92		/* command sent to a slave KVM 
						switch to perform a power 
						management command */
#define CAT5_ACK		0x91		/* command sent to a slave KVM 
						switch to perform a power 
						management command */
#define CAT5_PMREQUEST          0x90
#define CAT5_SETINFO		0X8f

/* Values used by the GETINFO and SHOWINFO messages */
#define CAT5_INFO_DEVICE		0
#define CAT5_INFO_MODEL			1
#define	CAT5_INFO_VERSION		2
#define	CAT5_INFO_VERSION2		3
#define CAT5_INFO_ECHO			7	

#define CAT5_INFO_KBD			8
#define CAT5_INFO_KBD_ALL		8
#define CAT5_INFO_KBD_SCANCODE		9
#define CAT5_INFO_KBD_TYPEMATIC		10
#define	CAT5_INFO_KBD_LEDS		11

#define CAT5_INFO_MOUSE			16
#define CAT5_INFO_MOUSE_ALL		16
#define CAT5_INFO_MOUSE_SCALING		17
#define CAT5_INFO_MOUSE_MODE		18
#define CAT5_INFO_MOUSE_DATAREPORTING	19
#define CAT5_INFO_MOUSE_RESOLUTION	20
#define CAT5_INFO_MOUSE_SAMPLERATE	21

#define CAT5_INFO_RP			24
#define CAT5_INFO_RP_ALL		24
#define CAT5_INFO_RP_BRIGHT		25
#define CAT5_INFO_RP_CONTR		26
#define	CAT5_INFO_RP_POS		27

//Oct 2003 [DB]
#define CAT5_INFO_POLARITY              32
#define CAT5_INFO_POLARITY_ALL          32
#define CAT5_INFO_POLARITY_H            33
#define CAT5_INFO_POLARITY_V            34
#define CAT5_INFO_AGC			35
#define CAT5_INFO_BRIGHTNESS		36
#define CAT5_INFO_CONTRAST		37

#define CAT5_INFO_COMMAND		40
#define CAT5_CMD_RESETACK		41

#define	CAT5_INFO_KBD_LAYOUT		63
/* Devices */
#define DEVICE_TERMINATOR		0
#define DEVICE_SWITCH_UC		1	
#define DEVICE_RPMAIN			2
#define DEVICE_RP			DEVICE_RPMAIN	
#define DEVICE_RPLOCAL			3
#define DEVICE_SWITCH_SRV		4	
#define DEVICE_SWITCH_STA		5
#define DEVICE_EXPANDER_SRV		6	
#define DEVICE_EXPANDER_STA		7

#define MODEL_PEM8			7
#define MODEL_PEM16			6
/* Values used by the BOOT messages */
#define BOOT_SKIP			0
#define BOOT_DOWNLOAD			1

/* Values used by the TERMSTAT messages */
#define TERMSTATUS_TO			0
#define TERMSTATUS_OK			1
#define TERMSTATUS_NOK			2
#define TERMSTATUS_UNDEF		0xff

/* Values used by the PMCOMMAND messages */
#define	PMCOMMAND_STATUS		0
#define	PMCOMMAND_ON			1
#define	PMCOMMAND_OFF			2
#define	PMCOMMAND_CYCLE			3
#define	PMCOMMAND_LOCK			4
#define	PMCOMMAND_UNLOCK		5	

#define PMCOMMAND_BUFFER		6	
#define PMCOMMAND_GETINFO		7

#define	PMCOMMAND_LOCSTATUS		8

#define ACK_OK				1
#define ACK_RETRY			2	
#define ACK_ABORT			3
#define ACK_NOK				9


#define NEWB				0
#define APPENDB				1
#define	ENDB				2

#ifndef TRUE
#define TRUE				1
#endif

#ifndef FALSE
#define	FALSE				0
#endif

#define	MSGSIZE		256
#define ENDMSG		"[end]"
#define	SYNTAXERR	"[synerr]"
#define	MSGBRK		"[break]"

#define KVMDPIDFILE	"/var/run/kvmd.pid"
#define KVMTEMPFILE     "/tmp/kvmtemp"

#define ACT_BRIGHTNESS			1
#define ACT_CONTRAST			2
#define ACT_RGB				3

#define PM_REQ_STATUS 0
#define PM_REQ_OFF    3
#define PM_REQ_ON     2
#define PM_REQ_CYCLE  1
#endif

#if defined(OEM3) && defined(KVM)
#define PMLOCK_disabled 1
#endif

#ifdef OEM3
#define IPDU "rPDU"
#define CYCLE "Reboot"
#else
#define IPDU "IPDU"
#define CYCLE "Cycle"
#endif