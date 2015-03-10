/*
 * rwconf.h	Definitions for the configuration stuff in conf.c.
 *		You cannot set any compile-time directives here.
 *
 * Version:	@(#)rwconf.h  1.10  14-Oct-1996  miquels@cistron.nl
 *
 */

#ifndef __rwconf_h
#define __rwconf_h

#ifdef IPv6enable
#include <sys/types.h>
#include <sys/socket.h>
#endif

#define C_CHR   0
#define C_INT   1
#define C_STR   2
#define C_HOST  3
#define C_LIST  4
#define C_IPNO  5
#define C_IPDY  6
#define C_CHAT  7
#define C_REALM 8
#define C_INDY  9
#define C_MSTR  10
#define C_LSTR  11
#define C_HCNF  12
#define C_CHAR  13
#define C_IPNO6 14
#define C_IPDY6 15
#define C_XSTR  200
#define C_XIPNO 201
#define C_XINT  202

//  PA8, PA9 and PA10
//  
// media & 0x1 = 1 ==> RS485
// media & 0x1 = 0 ==> RS232
// media & 0x2 = 2 ==> Half Duplex
// media & 0x9 = 1 ==> RS485 terminator
// 
#define RS232                 0x00      // 0000
#define RS232_HALF            0x02      // 0010
#define RS232_HALF_CTS        0x0a      // 1010
#define RS485                 0x01      // 0001
#define RS485_HALF            0x0b      // 1011
#define RS485_FULL            0x09      // 1001
#define RS485_HALF_TERM       0x03      // 0011
#define RS485_FULL_TERM       0x01      // 0001
#define RS_QUERY_MEDIA        0x10000
 

/*
 * Multiple sessions definitions
 */
#define SNIFF_SESSIONS_ONLY             4
#define RW_SESSIONS_ONLY                2
#define MULTIPLE_RW_AND_SNIFF_SESSIONS  1
#define NO_MULTIPLE_SESSIONS            0

// PM Sessions allowed
#define PMSESS_NONE	0
#define PMSESS_SSH	1
#define PMSESS_TELNET	2

// nsswitch.conf
#define NSSWITCH_FILE		"/etc/nsswitch.conf"
/*
 *	For option lists.
 */

/* Bonding definitions file */
#define BONDING_FILE	"/etc/bonding.opts"
/* Bonding enabled flag: */
#define BD_BONDING_ENABLED		(1<<0)
/* Bonding enabled flag was changed: */
#define BD_BONDING_ENA_CHANGED		(1<<1)
/* Bonding options changed: */
#define BD_BONDING_OPTS_CHANGED		(1<<2)

//[GY]2006/Apr/24
//Ports Alerts strings
#define ALERT_STRINGS_QUANTITY 10   //Up to 10 strings/port
#define ALERT_STRINGS_MIN_CHAR 3    // 3 characters/string minimum
#define ALERT_STRINGS_MAX_CHAR 32   //32 characters/string maximum

//some DLS definitions
#define CIR_mode       0
#define LIN_mode       1
#ifdef DLSenable
#define DLS_mode       2
#define DLS_LOCAL_mode 3
#endif

//IPv6 Definitions
#ifdef IPv6enable

//IPv6 config method
#define IPv6_STATELESS_ONLY 0
#define IPv6_STATIC         1
#define IPv6_DHCP           2

//DHCPv6 options (bitmask)
#define DHCPv6_NONE   0x0000
#define DHCPv6_DNS    0x0001
#define DHCPv6_DOMAIN 0x0002

#endif

struct lst {
	char *name;
	int value;
};

/*
 *	Define type of config variables.
 */
struct conf {
	char *name;
	int type;
	struct lst *opts;
	int offs;
	int (*check)(char *);
};

struct bonding_cfg {
        unsigned char  bonding; //bonding enabled and/or config updated
        int  miimon;
        int  updelay;
};

/*
 *	General system configuration.
 */
struct main_cfg {
	char *hostname;
	char *hostname1;
	char *nfs_data_buffering;
	int  dhcpcd;
	unsigned int host_ipno;
	unsigned int mtu;
	unsigned int netmask;
	char *macaddress;
	unsigned int ipno;
	unsigned int eth_ip;
	unsigned int eth_ip_alias;
	unsigned int eth_mask_alias;
	char *lockdir;
	char *rlogin;
	char *telnet;
	char *ssh;
	char *pppd;
	int locallogins;
	unsigned int syslog;
	int facility;
	int DB_facility;
	char *filterdir;
	char *st_routes;
	char *groups;
	char include[128];
	int host_config;
	int nat_clustering_ip;
	int authtype;
	int pppauthtype;
	int dla;
	int alert;
	char *alert_strings;
	char *dla_file;
	// [AP][2007-07-24] Hostname auto discovery
	char *ad_probe_str;
	char *ad_answer_str;
#ifdef IPv6enable
	// IPv6 parameters
	int enableIPv4; 
	int enableIPv6;
	int IPv6method; // How to set eth addr: static/dhcp/stateless_only
	struct sockaddr_storage eth_ip6; // Static IPv6 address for eth
	int eth_prefix6; // prefix length for eth_ip6
	struct sockaddr_storage ipno6;
	int prefix6; // prefix length for ipno6
	int dhcpv6_opts; //what can be obtained from a DHCPv6 server
#endif
};

/*
 *	Realm structure.
 */
struct realm_def {
	char *name;
	int prefix;
	int strip;
	unsigned int authhost1;
	unsigned int authhost2;
	unsigned int accthost1;
	unsigned int accthost2;
	struct realm_def *next;
};

/*
 *	Configuration, per line.
 */
struct line_cfg {
	int this;
	int auxflg;
	int virtual;
	char *translation;
	int web_WinEMS;
	char *alias;
	char *pool_alias;
	char *tty;
	int protocol;
	int authtype;
	int biometric; /* [EL] Biometric support 8/27/2003 */
	int socket_port;
	int pool_socket_port;
	int poll_interval;
	int tx_interval;
	int idletimeout;
	/* Modem options */
	int DTR_reset;
	int flow;
	int dcd;
	int speed;
	int media;
	int StopBits;
	int Parity;
	int DataSize;
	int syslog_level;
	int console_level;
	int DB_timestamp;
	int DB_mode;
	int DB_user_logs;           //Data Buffer User Logs
	int data_buffering;
	int data_buffering_sess;   // data buffer/connection established [RK]Feb/13/06 
	int syslog_buffering;
	int syslog_sess;
	int dont_show_db_op;
	int sniff_mode;
	int multiple_sessions;      //Multiple sniffers allowed?
	int multiuser_notif;        //Multiuser Notification
	int escape_char;            //Show Multiple sessions/sniffers menu
	char *users;
	char *admin_users;
	char *userauto;
	char *break_seq;

	/* Networking options */
	char *secret;
	unsigned int radnullpass;
	unsigned int authhost1;
	unsigned int authhost2;
	unsigned int accthost1;
	unsigned int accthost2;
	unsigned int timeout;
	unsigned int retries;
	char *utmp_from;
	char *wtmp_from;
#ifdef IPv6enable
	struct sockaddr_storage host;
#else
	unsigned int host;
#endif
	int host_port;
	int host_idletime;
	unsigned int ipno;
	unsigned int pool_ipno;
	unsigned int netmask;
#ifdef IPv6enable
	struct sockaddr_storage ipno6;
	struct sockaddr_storage pool_ipno6;
	unsigned int prefix6;
#endif
	int mtu, mru;
	char *autoppp;
	char *pppopt;
	/* Other options */
	char *issue;
	char *sttyCmd;
	char *prompt;
	char *term;
	int lf_suppress;
	char *auto_answer_input;
	char *auto_answer_output;
	int sysutmp;
	int syswtmp;
	struct realm_def *realm;
// [Fuji] Nov/27/02 #ifndef CYCLADES
//	int debug;
//#endif
	int emumodem;
	int porttype;
	char *initchat;
// [Fuji] Nov/27/02 #ifdef CYCLADES
	int alarm;
	int xml_monitor;	// Support for Windows 2003 EMS xml tags
	int telnet_client_mode; //[RK]Nov/20/02 - ES DST830
        char *port_alert_strings;//match strings for Ports Alerts  [GY]2006/Apr/24
//#endif
	int modbus_smode;
	char * pmtype;		// the IPDU manufacturer
	char * pmusers;		// The users allowed to access this IPDU
	char * pmoutlet;	// The outlets where this server is connected
	int sconfkey;          	// The key that identifies serial configuration command
	int pmkey;          	// The key that identifies a pm command
	int IPMIkey;		// The key that identifies an IPMI command
	int SSH_exitkey;	// The key that closes an SSH session
	int pmNumOfOutlets;     // Number of outlets on this ipdu
        int IPMIServer; /* IPMI Server ID [EL] 11/25/2003 */
//#ifdef PMD
//	char * pmadmpasswd;  // Administrative password
//#endif
	int  billing_records;   //mp.Jan2003: new parameter
	int  billing_timeout;   //mp.Jan2003: new parameter
	char *billing_eor;      //mp.Aug2003: new parameter
	char *billing_dir;      //mp.Jan2004: new parameter
	int  mr_ip_timeout; //mp.Jun2004: new parameter
	char *mr_telno;     //mp.Jun2004: new parameter
	int  mr_modemtries; //mp.Jun2004: new parameter
	int  mr_idletime;   //mp.Jun2004: new parameter
	int  break_interval;	//[RK]May/05/03 - 

	int  fast;		//[RK]May/22/03 - Performance Improvement

	int  pmsessions;	// ssh/telnet access is allowed
	int lgtimeout;		// terminal session timeout for bidirect telnet
	char *termsh;		// terminal shell for bidirect telnet
	int conntype;		// connection type for bidirect telnet
	// [AP][2007-07-24] Hostname auto discovery
	int ad;
	int ad_timeout;
};

/*
 *	Flow control definitions.
 */
#define FLOW_NONE	0
#define FLOW_HARD	1
#define FLOW_SOFT	2

extern struct conf line_cfg[];
extern struct conf main_cfg[];
extern struct conf realm_cfg[];

/*
 *	Functions.
 */
void initcfg(struct main_cfg *mainconf,
	     struct line_cfg *allconf,
	     struct line_cfg *lineconf);

int readcfg(struct main_cfg *mainconf,
	    struct line_cfg *allconf,
	    struct line_cfg *lineconf);

#ifdef BONDING_FEATURE
/* reads the bonding.opts file */
int read_bondcfg(struct bonding_cfg *bondcfg, int first_time); /* BUG#6602 */
#endif

#endif	/* __rwconf_h */
