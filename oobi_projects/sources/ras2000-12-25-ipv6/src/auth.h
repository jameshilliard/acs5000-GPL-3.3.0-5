/*
 * auth.h	Header file for the authentication stuff.
 *		We only use radius, but anyway this is
 *		a try to hide the authentication protocol
 *		from the rest of the system.
 *
 * Version:	@(#)auth.h  1.00  05-Jul-1996  miquels@cistron.nl
 *
 */

#ifndef __auth_h
#define __auth_h

/*
 *	Types of connection.
 */
#define P_UNKNOWN	0
#define P_LOCAL		'L'
#define P_RLOGIN	'R'
#define P_SLIP		'S'
#define P_CSLIP		'C'
#define P_PPP		'P'
#define P_PPP_ONLY	'O'
#define P_AUTOPPP	'A'
#define P_TELNET	'E'
#ifdef BIDIRECT
#define P_TELNET_BIDIRECT	'F'
#endif
#define P_TCPCLEAR      'T'
#define P_TCPLOGIN      'U'
#define P_CONSOLE	'!'
#define P_SHELL		'X'
#define P_SSH		'H'
#define P_SSH2		'2'
#define P_SOCKET_SERVER		'W'
#define P_SOCKET_SERVER_RAW	'V'
#define P_SOCKET_CLIENT		'I'
#define P_SOCKET_SSH		'Y'
#define P_SOCKET_SERVER_SSH	'Q'
#ifdef PMD
#define P_IPDU              'i'
#endif
#define P_MODBUS            'M'
#define P_GENDIAL           'G'  //[GY]2006/Mar/17
#define P_BILLING           'B'
#define P_LPD               'D'
#define P_MODEM             'Z'
#define P_2WRAW             'N'  //[GY]2006/Mar/17

/*
 *	Types of authentication
*/

#define AUTH_NONE							0x0000
#define AUTH_REMOTE							0x0010
#define AUTH_SAME_AS_UNIT					0x0100

#define AUTH_RADIUS							0x1000
#define AUTH_RADIUS_LOCAL					0x1001
#define AUTH_RADIUS_DOWN_LOCAL				0x1002

#define AUTH_TacacsPlus						0x2000
#define AUTH_TacacsPlus_LOCAL				0x2001
#define AUTH_TacacsPlus_DOWN_LOCAL			0x2002

#ifdef KRBenable
#define AUTH_KERBEROS						0x3000
#define AUTH_KERBEROS_LOCAL					0x3001
#define AUTH_KERBEROS_DOWN_LOCAL			0x3002
#endif

#define AUTH_LDAP							0x4000
#define AUTH_LDAP_LOCAL						0x4001
#define AUTH_LDAP_DOWN_LOCAL				0x4002
#ifdef RADIUS2FACTORenable
#define AUTH_LDAP_DOWN_LOCAL_AND_RADIUS		0x4003
#endif

#ifdef NISenable
#define AUTH_NIS							0x5000
#define AUTH_NIS_LOCAL						0x5001
#define AUTH_NIS_DOWN_LOCAL					0x5002
#endif

#ifdef SMBenable
#define AUTH_SMB							0x6000
#define AUTH_SMB_DOWN_LOCAL					0x6001
#endif

//[RK]Jan/18/06 - 
#ifdef OTPenable
#define AUTH_OTP							0X0006
#define AUTH_OTP_LOCAL						0X0007
#endif

#ifdef ADSAP2
#define AUTH_DSVIEW							0x7000
#define AUTH_DSVIEW_LOCAL					0x7001
#define AUTH_DSVIEW_DOWN_LOCAL				0x7002
#endif

#define AUTH_LOCAL							0x0080
#define AUTH_LOCAL_RADIUS					0x8001
#define AUTH_LOCAL_TacacsPlus				0x8002
#define AUTH_LOCAL_NIS						0x8005


#ifdef CYCLADES
#define AUTH_BIOMETRIC						0x0070
#endif

#define AUTH_SERVER							0xF000

/*
 *	Authentication info. This struct
 *	contains all info we get back from
 *	the authentication protocol.
 */
struct auth {
	/* Input */
#ifdef CYCLADES
	short  next_token;	// next token mode
	short  st_sess;		// session status 0=None 1=Read/Write, 2=Read/Only 
	char login[64];
	char passwd[64];
	CY_SESS *sess;
#else
	char login[256];
	char passwd[256];
#endif
	time_t start;
	/* Output */
	char session[32];
	short msn;			//message num
	short fln;			//filterid num
	short frn;			//framed_router num
	short len_state;	//lenght of state_info field
#ifdef CYCLADES
	char message[4][128];		//Up to 4 messages. Good?
	char filterid[16][32];		//Up to 16 filters. Good?
	char framed_route[16][48];	//Up to 16 routes. Good?
#else
	char message[16][256];		//Up to 17 messages. Good?
	char filterid[16][256];		//Up to 17 filters. Good?
	char framed_route[16][256];	//Up to 17 routes. Good?
#endif
	char state_info[128];
	char conn_info[128];
	int  proto;
	int  nasport;			/* NAS-Port attribute value	    */
	int  loginport;			/* Login-TCP-Port attribute value   */
	short authenticated;
	short  do_acct;
	struct sockaddr_storage host;
	struct sockaddr_storage address;/* Address is in network byte order */
	unsigned int localip;
	unsigned int netmask;
#ifdef IPv6enable
//	struct sockaddr_storage host6;
//	struct sockaddr_storage address6;/* Address is in network byte order */
	struct sockaddr_storage localip6;
	unsigned int prefix6;
#endif
	int mtu;
	int mru;
	int idletime;
	int sessiontime;
	int porttype;
	int sent_bytes;
	int recv_bytes;
#ifdef CYCLADES
	unsigned char sess_type;	/* 0 - normal session, 1-callback session */
	unsigned char callback_number[64]; /* set by Radius Server */
	//[RK]May/24/05 - Group Authorization Support
	unsigned char flag_admin_user;  // 0 - no admin user, 1 - admin user
#endif
	char termsh[128];  // shell command. 
};

int login_local(struct auth *pai);
int do_local_or_server_authentication(struct auth *pai, int ppp);
int rad_client(struct auth *ai, int ppp);
int rad_acct(struct auth *ai, int islogin);
unsigned int rad_sessionid(char *s);
#ifdef IPv6enable
int rad_init(int port, struct auth *ai, char *tty, int fproto, struct sockaddr * ipaddr);
#else
int rad_init(int port, struct auth *ai, char *tty, int fproto);
#endif
char *dotted(unsigned int ipno);
struct realm_def *ckrealm(struct auth *ai);
int do_pam_close(void);

#endif	/* __auth_h */
