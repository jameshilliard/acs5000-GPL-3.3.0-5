#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "cySerialPortTable.h"
#include "cystrings.h"
#include "cycommon.h"
#include <sys/stat.h>
/*
 * cySSpec_variables_oid:
 */

oid cySerialPortTable_variables_oid[] = { PROD_OID, 2, 6, 2 ,1};
// include column  exclude index 
#define	CYSSPEC_OID_LEN		PROD_OID_SIZE+6
extern int cy_numports;

static unsigned char ipaddr[INET6_ADDRSTRLEN];
static struct in_addr aux;
static struct sockaddr_storage aux_server;
static int cy_index;
static int cy_column;
static unsigned char cy_prefix[10], *cy_paux;

/**
 * Returns NULL if there was no error or an error message.
 */
static const char *xinet_aton(const char *buffer, struct sockaddr_storage *server) {
	
	struct addrinfo *res = NULL;
	struct addrinfo hints;
	int n;
	memset(&hints,0,sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_flags = AI_NUMERICHOST;
	n = getaddrinfo(buffer,NULL,&hints,&res);
	
	if (n == 0) {
		memcpy(server,res->ai_addr,res->ai_addrlen);
		freeaddrinfo(res);
		return NULL;
	} else {
		const char *msg = gai_strerror(n);
		return msg;
	}
}

/**
 * The buffer parameter needs to have at least INET6_ADDRSTRLEN bytes.
 */
static char *xinet_ntoa(struct sockaddr *server, char *buffer, int b_len) {
	
	struct sockaddr_storage addr;
	int addr_len = sizeof(addr);
	int n;
	
	if (server->sa_family == AF_INET) {
		addr_len = sizeof(struct sockaddr_in);
		memcpy(&addr,server,addr_len);
		server = (struct sockaddr *)&addr;
	} else if (server->sa_family == AF_INET6) {
		addr_len = sizeof(struct sockaddr_in6);
		memcpy(&addr,server,addr_len);
		server = (struct sockaddr *)&addr;
	}
	
	n = getnameinfo(server,addr_len,
			buffer,b_len,NULL,0,NI_NUMERICHOST);
	
	if (n != 0) {
		//_pam_log(LOG_ERR, "error in %s getnameinfo - %d",__FUNCTION__, n);
	}
	
	return buffer;
}

/*
 * variable3 cySPortTable_variables:
 *   this variable defines function callbacks and type return information 
 *   for the  mib section 
 */

struct variable4 cySerialPortTable_variables[] = {
/*
 * magic number        , variable type , ro/rw , callback fn  , L, oidsuffix 
 */
#define CYSPORTNUMBER		1
{ CYSPORTNUMBER, ASN_INTEGER, RONLY, var_cySerialPortTable, 2, { 1, 1} } ,
#define CYSPORTTTY		2
{ CYSPORTTTY, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 2} } ,
#define CYSPORTNAME		3
{ CYSPORTNAME, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 3} } ,
#define CYSPORTSPEED		4
{ CYSPORTSPEED, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 4} } ,
#define CYSPORTDATASIZE		5
{ CYSPORTDATASIZE, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 5} } ,
#define CYSPORTSTOPBITS		6
{ CYSPORTSTOPBITS, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 6} } ,
#define CYSPORTPARITY		7
{ CYSPORTPARITY, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 7} } ,
#define CYSPORTFLOWCTRL		8
{ CYSPORTFLOWCTRL, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 8} } ,
#define CYSPORTDTRDELAY		9
{ CYSPORTDTRDELAY, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 9} } ,
#define CYSPORTDCDCTRL		10
{ CYSPORTDCDCTRL, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 10} } ,
#define CYSPORTLOGUTMP		11
{ CYSPORTLOGUTMP, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 11} } ,
#define CYSPORTLOGWTMP		12
{ CYSPORTLOGWTMP, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 12} } ,
#define CYSPORTLOGFORM		13
{ CYSPORTLOGFORM, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 13} } ,
#define CYSPORTAUTHTYPE		14
{ CYSPORTAUTHTYPE, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 14} } ,
#define CYSPORTAUTHSRV1		15
{ CYSPORTAUTHSRV1, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 15} } ,
#define CYSPORTACCSRV1		16
{ CYSPORTACCSRV1, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 16} } ,
#define CYSPORTAUTHTMO		17
{ CYSPORTAUTHTMO, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 17} } ,
#define CYSPORTAUTHRETR		18
{ CYSPORTAUTHRETR, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 18} } ,
#define CYSPORTAUTHSRV2		19
{ CYSPORTAUTHSRV2, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, {  1, 19} } ,
#define CYSPORTACCSRV2		20
{ CYSPORTACCSRV2, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 20} } ,
#define CYSPORTAUTHSECRET	21
{ CYSPORTAUTHSECRET, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 21} } ,
#define CYSPORTAUTHRADP		22
{ CYSPORTAUTHRADP, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 22} } ,
#define CYSPORTAUTHACC		23
{ CYSPORTAUTHACC, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 23} } ,
#define CYSPORTPROTOCOL		24
{ CYSPORTPROTOCOL, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 24} } ,
#define CYSPORTREMOTEIP		25
{ CYSPORTREMOTEIP, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 25} } ,
#define CYSPORTSOCKETPORT	26
{ CYSPORTSOCKETPORT, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 26} } ,
#define CYSPORTREMHOST		27
{ CYSPORTREMHOST, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 27} } ,
#define CYSPORTBANNER		28
{ CYSPORTBANNER, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 28} } ,
#define CYSPORTPROMPT		29
{ CYSPORTPROMPT, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 29} } ,
#define CYSPORTTERMTYPE		30
{ CYSPORTTERMTYPE, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 30} } ,
#define CYSPORTAUTOMUSR		31
{ CYSPORTAUTOMUSR, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 31} } ,
#define CYSPORTNETMASK		32
{ CYSPORTNETMASK, ASN_IPADDRESS, RWRITE, var_cySerialPortTable, 2, { 1, 32} } ,
#define CYSPORTPPPMTU		33
{ CYSPORTPPPMTU, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 33} } ,
#define CYSPORTPPPMRU		34
{ CYSPORTPPPMRU, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 34} } ,
#define CYSPORTPPPOPTIONS	35
{ CYSPORTPPPOPTIONS, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 35} } ,
#define CYSPORTPPPFOPTION	36
{ CYSPORTPPPFOPTION, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 36} } ,
#define CYSPORTMODEMCHAT	37
{ CYSPORTMODEMCHAT, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 37} } ,
#define CYSPORTSTTYCMD		38
{ CYSPORTSTTYCMD, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 38} } ,
#define CYSPORTSOCKTX		39
{ CYSPORTSOCKTX, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 39} } ,
#define CYSPORTSOCKPOLL		40
{ CYSPORTSOCKPOLL, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 40} } ,
#define CYSPORTSOCKIDLE		41
{ CYSPORTSOCKIDLE, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 41} } ,
#define CYSPORTDBSIZE		42
{ CYSPORTDBSIZE, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 42} } ,
#define CYSPORTDBTIME		43
{ CYSPORTDBTIME, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 43} } ,
#define CYSPORTDBMODE		44
{ CYSPORTDBMODE, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 44} } ,
#define CYSPORTDBSYSLOG		45
{ CYSPORTDBSYSLOG, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 45} } ,
#define CYSPORTDBMENU		46
{ CYSPORTDBMENU, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 46} } ,
#define CYSPORTDBALARM		47
{ CYSPORTDBALARM, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 47} } ,
#define CYSPORTSSHBREAK		48
{ CYSPORTSSHBREAK, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 48} } ,
#define CYSPORTSNIFFSESS	49
{ CYSPORTSNIFFSESS, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 49} } ,
#define CYSPORTSNIFFADM		50
{ CYSPORTSNIFFADM, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 50} } ,
#define CYSPORTSNIFFESC		51
{ CYSPORTSNIFFESC, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 51} } ,
#define CYSPORTSNIFFMSESS	52
{ CYSPORTSNIFFMSESS, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 52} } ,
//[RK]Nov/20/02 New PortSlave parameters - TS 1.3.5 and ACS 2.1.1
#define CYSPORTTELNETMODE	53
{ CYSPORTTELNETMODE, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 53} } ,
#define CYSPORTSYSBUFSESS	54
{ CYSPORTSYSBUFSESS, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 54} } ,
//[RK]Nov/26/02 New PortSlave parameters - TS 1.3.5 and ACS 2.1.1
#define CYSPORTLFSUP		55
{ CYSPORTLFSUP, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 55} } ,
#define CYSPORTAUTOIN		56
{ CYSPORTAUTOIN, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 56} } ,
#define CYSPORTAUTOOUT		57
{ CYSPORTAUTOOUT, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 57} },
//[RK]Jan/06/03 - PM8 parameters
#define CYSPORTPMTYPE		58
{ CYSPORTPMTYPE, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 58} } ,
#define CYSPORTPMUSERS		59
{ CYSPORTPMUSERS, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 59} } ,
#define CYSPORTPMOUTLET		60
{ CYSPORTPMOUTLET, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 60} } ,
#define CYSPORTPMKEY		61
{ CYSPORTPMKEY, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 61} }, 
#define CYSPORTPMNOUTLETS	62
{ CYSPORTPMNOUTLETS, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 62} },
#define CYSPORTBREAKINTERVAL	63
{ CYSPORTBREAKINTERVAL, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 63} }, 
#define CYSPORTREMOTEIP6	64
{ CYSPORTREMOTEIP6, ASN_OCTET_STR, RWRITE, var_cySerialPortTable, 2, { 1, 64} } ,
#define CYSPORTPREFIX6		65
{ CYSPORTPREFIX6, ASN_INTEGER, RWRITE, var_cySerialPortTable, 2, { 1, 65} }, 
 };


unsigned char *var_cySPT_name[] = {
	"tty",			// CYSPORTTTY
	"alias",		// CYSPORTNAME
	"speed",		// CYSPORTSPEED	
	"datasize",		// CYSPORTDATASIZE
	"stopbits",		// CYSPORTSTOPBITS
	"parity",		// CYSPORTPARITY
	"flow",			// CYSPORTFLOWCTRL
	"DTR_reset",		// CYSPORTDTRDELAY
	"dcd",			// CYSPORTDCDCTRL
	"sysutmp",		// CYSPORTLOGUTMP
	"syswtmp",		// CYSPORTLOGWTMP
	"utmpfrom",		// CYSPORTLOGFORM
	"authtype",		// CYSPORTAUTHTYPE
	"authhost1",		// CYSPORTAUTHSRV1
	"accthost1",		// CYSPORTACCSRV1
	"radtimeout",		// CYSPORTAUTHTMO
	"radretries",		// CYSPORTAUTHRETR
	"authhost2",		// CYSPORTAUTHSRV2
	"accthost2",		// CYSPORTACCSRV2
	"secret",		// CYSPORTAUTHSECRET
	"radnullpass",		// CYSPORTAUTHRADP
	"users",		// CYSPORTAUTHACC
	"protocol",		// CYSPORTPROTOCOL
	"ipno",			// CYSPORTREMOTEIP
	"socket_port",		// CYSPORTSOCKETPORT
	"host",			// CYSPORTREMHOST
	"issue",		// CYSPORTBANNER
	"prompt",		// CYSPORTPROMPT
	"term",			// CYSPORTTERMTYPE
	"userauto",		// CYSPORTAUTOMUSR
	"netmask",		// CYSPORTNETMASK
	"mtu",			// CYSPORTPPPMTU
	"mru",			// CYSPORTPPPMRU
	"autoppp",		// CYSPORTPPPOPTIONS
	"pppopt",		// CYSPORTPPPFOPTION
	"initchat",		// CYSPORTMODEMCHAT
	"sttyCmd",		// CYSPORTSTTYCMD
	"tx_interval",		// CYSPORTSOCKTX
	"poll_interval",	// CYSPORTSOCKPOLL
	"idletimeout",		// CYSPORTSOCKIDLE
	"data_buffering",	// CYSPORTDBSIZE
	"DB_timestamp",		// CYSPORTDBTIME
	"DB_mode",		// CYSPORTDBMODE
	"syslog_buffering",	// CYSPORTDBSYSLOG
	"dont_show_DBmenu",	// CYSPORTDBMENU
	"alarm",		// CYSPORTDBALARM
	"break_sequence",	// CYSPORTSSHBREAK
	"sniff_mode",		// CYSPORTSNIFFSESS
	"admin_users",		// CYSPORTSNIFFADM
	"escape_char",		// CYSPORTSNIFFESC
	"multiple_sessions",	// CYSPORTSNIFFMSESS
	"telnet_client_mode",	// CYSPORTTELNETMODE
	"syslog_sess",		// CYSPORTSYSBUFSESS
	"lf_suppress",		// CYSPORTLFSUP	
	"auto_answer_input",	// CYSPORTAUTOIN
	"auto_answer_output",	// CYSPORTAUTOOUT
	"pmtype",		// CYSPORTPMTYPE
	"pmusers",		// CYSPORTPMUSERS
	"pmoutlet",		// CYSPORTPMOUTLET
	"pmkey",		// CYSPORTPMKEY
	"pmNumOfOutlets",	// CYSPORTPMNOUTLETS
//	"pmadmpasswd",          	// CYSPORTPMADMPASSWD
	"break_interval",	// CYSPORTBREAKINTERVAL
	"ipno6",		// CYSPORTREMOTEIP6
	"prefix6",		// CYSPORTPREFIX6
};

/** Initializes the cySerialSpec module */
void init_cySerialPortTable(void)
{
	DEBUGMSGTL(("cySerialSpec", "Initializing\n"));

	/*
	 * register ourselves with the agent to handle our mib tree 
	 */
	REGISTER_MIB("cySerialPortTable", cySerialPortTable_variables, variable4,
		     cySerialPortTable_variables_oid);
}

/*
 * Compare 'name' to vp->name for the best match or an exact match (if
 *	requested).  Also check that 'name' is not longer than 'max' if
 *	max is greater-than/equal 0.
 * Store a successful match in 'name', and increment the OID instance if
 *	the match was not exact.  
 */
int
header_sp_table(struct variable *vp, oid * name, size_t * length,
                    int exact, size_t * var_len,
                    WriteMethod ** write_method, int max)
{
	int i, rtest;   /* Set to: -1 If name < vp->name,
                         *          1 If name > vp->name,
                         *          0 Otherwise.
                        */
	oid newname[MAX_OID_LEN];

	for (i = 0, rtest = 0;
	     (i < (int) vp->namelen) && (i < (int)(*length)) && !rtest; 
	     i++) {
		if (name[i] != vp->name[i]) {
			if (name[i] < vp->name[i])
				rtest = -1;
			else
				rtest = 1;
		}
	}

	if ( (rtest > 0) || 
	     ( (exact == 1) && 
               (rtest || 
                ( (int)*length != (int)(vp->namelen + 1) ) ) ) ) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
    	}

	memset(newname, 0, sizeof(newname));

	if ( ( (int)*length <= (int)vp->namelen) || 
             ( rtest == -1) ) {
		memmove(newname, vp->name, (int)vp->namelen * sizeof(oid));
		newname[vp->namelen] = 0;
		*length = vp->namelen + 1;
	} else {
		if ((int)*length > ((int)vp->namelen + 1)) { /* exact case checked earlier */
			*length = vp->namelen + 1;
			memmove(newname, name, (*length) * sizeof(oid));
			if (name[*length - 1] < ULONG_MAX) {
				newname[*length - 1] = name[*length - 1] + 1;
			} else {
				newname[*length - 1] = name[*length - 1];
        		}
		} else {
			*length = vp->namelen + 1;
			memmove(newname, name, (*length) * sizeof(oid));
			if (!exact) {
				if (name[*length - 1] < ULONG_MAX) {
					newname[*length - 1] = name[*length - 1] + 1;
				} else {
					newname[*length - 1] = name[*length - 1];
				}
			} else {
				newname[*length - 1] = name[*length - 1];
			}
		}
    	}
	if ( ( (max >= 0) && (newname[*length - 1] > max) ) ) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memmove(name, newname, (*length) * sizeof(oid));
	if (write_method)
		*write_method = 0;
	if (var_len)
		*var_len = sizeof(long);        /* default */
	return (MATCH_SUCCEEDED);
}
/*
 * var_cySerialPortTable():
 *   Handle this table separately from the scalar value case.
 *   The workings of this are basically the same as for var_ above.
 */
unsigned char *var_cySerialPortTable(struct variable *vp,
				     oid * name,
				     size_t * length,
				     int exact,
				     size_t * var_len,
				     WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	static unsigned char string[255];
	static int size ;
	char *errorMessage;

	// verify index 0 == ALL 

	if (header_sp_table
	    (vp, name, length, exact, var_len, write_method, cy_numports)
	    == MATCH_FAILED)
		return NULL;

	cy_index = name[CYSSPEC_OID_LEN];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	memset(string,0,255);
	aux.s_addr = 0;
	aux_server.ss_family = 0;
	long_ret = 0;
	*var_len = sizeof(long);
	*write_method = 0;

	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
	      case CYSPORTNUMBER:
		return (u_char *) & cy_index;
		break;

	      case CYSPORTTTY:
		if (cy_index) {
			*write_method = write_cySPort_str;
			size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
			if (!size) strcpy(string,"inactive");
		} else {
			strcpy(string,"ALL");
		} 
		*var_len = strlen(string);
		return (u_char *) &string;
		break;

	      case CYSPORTNAME:
		if (cy_index) {
			size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
			*write_method = write_cySPort_str;
		} 
		*var_len = strlen(string);
		return (u_char *) &string;
		break;

		/* INTEGER */
	      case CYSPORTSPEED:
	      case CYSPORTDATASIZE:
	      case CYSPORTSTOPBITS:
	      case CYSPORTDTRDELAY:
	      case CYSPORTDCDCTRL:
	      case CYSPORTLOGUTMP:
	      case CYSPORTLOGWTMP:
	      case CYSPORTAUTHTMO:
	      case CYSPORTAUTHRETR:
	      case CYSPORTAUTHRADP:
	      case CYSPORTPPPMTU:
	      case CYSPORTPPPMRU:
	      case CYSPORTSOCKTX:
	      case CYSPORTSOCKPOLL:
	      case CYSPORTSOCKIDLE:
	      case CYSPORTDBSIZE:
	      case CYSPORTDBTIME:
	      case CYSPORTDBSYSLOG:
	      case CYSPORTDBMENU:
	      case CYSPORTDBALARM:
	      case CYSPORTTELNETMODE:
	      case CYSPORTSYSBUFSESS:
	      case CYSPORTLFSUP:
	      case CYSPORTPMNOUTLETS:
	      case CYSPORTBREAKINTERVAL:
	      case CYSPORTPREFIX6:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (size) {
			 size = atoi(string);
		} else {
			switch (vp->magic) {
				case CYSPORTSPEED : size = 9600; break;
				case CYSPORTDATASIZE : size = 8; break;
	      			case CYSPORTSTOPBITS: size = 1; break;
	      			case CYSPORTAUTHTMO: size = 3; break;
	      			case CYSPORTSOCKTX: size = 100; break;
	      			case CYSPORTSOCKPOLL: size = 1000; break;
				default : size = 0; break;
			}
		}
		*write_method = write_cySPort_int;
		return (u_char *) & size;
		break;

		/* STRING */
	      case CYSPORTAUTHSECRET:
	      case CYSPORTAUTHACC:
	      case CYSPORTBANNER:
	      case CYSPORTPROMPT:
	      case CYSPORTTERMTYPE:
	      case CYSPORTAUTOMUSR:
	      case CYSPORTPPPOPTIONS:
	      case CYSPORTPPPFOPTION:
	      case CYSPORTMODEMCHAT:
	      case CYSPORTSTTYCMD:
	      case CYSPORTAUTOIN:
	      case CYSPORTAUTOOUT:
	      case CYSPORTSSHBREAK:
	      case CYSPORTSNIFFADM:
	      case CYSPORTSNIFFESC:
	      case CYSPORTSNIFFMSESS:
	      case CYSPORTPMUSERS:
	      case CYSPORTPMOUTLET:
	      case CYSPORTPMKEY:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		*write_method = write_cySPort_str;
		*var_len = strlen(string);
		return (u_char *) &string;
		break;

		/* IPv4 ADDRESS */
	      case CYSPORTNETMASK:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (size) inet_aton(string,&aux);
		*write_method = write_cySPort_ipaddr;
		return (u_char *) & aux.s_addr;
		break;
		
		
		/* IPv4 or IPv6 ADDRESS */
	      case CYSPORTAUTHSRV1:
	      case CYSPORTACCSRV1:
	      case CYSPORTAUTHSRV2:
	      case CYSPORTACCSRV2:
	      case CYSPORTREMHOST:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		
		if (size > 0 && size < sizeof(ipaddr) ) {
			strcpy(ipaddr,string);
		} else {
			strcpy(ipaddr,"");
		}
		*write_method = write_cySPort_ipaddr_4_6;
		*var_len = strlen(ipaddr);
		return (u_char *) ipaddr;
		break;

	      case CYSPORTPARITY:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		*write_method = write_cySPortParity;
		if (!size) strcpy(string,"none");
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTFLOWCTRL:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		*write_method = write_cySPortFlowCtrl;
		if (!size) strcpy(string,"none");
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTLOGFORM:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (!size) { // default value
			strcpy(string,"%t:%p.%3.%4");
		}
		*write_method = write_cySPort_str;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTAUTHTYPE:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (!size) { // default value
			strcpy(string,"none");
		}
		*write_method = write_cySPortAuthtype;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTPROTOCOL:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (!size) { // default value
			strcpy(string,"not");
		}
		*write_method = write_cySPortProtocol;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTREMOTEIP:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (size && cy_index) {
			unsigned char *p;
			if ((p = strchr(string,'+')) != NULL) {
				*p = 0x00;
				inet_aton(string,&aux);
				aux.s_addr = (unsigned int)htonl(ntohl(aux.s_addr) + (cy_index & 0xff) -1);
				p = inet_ntoa(aux);		
				strcpy(string,p);
			}
		}
		*write_method = write_cySPortRemoteIP;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;
		
	      case CYSPORTREMOTEIP6:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (size && cy_index) {
			unsigned char *p;
			struct sockaddr_in6 *addr;
			if ((p = strchr(string,'+')) != NULL) {
				*p = 0x00;
				errorMessage = xinet_aton((char *)string, &aux_server);
				if (errorMessage == NULL) {
					addr = (struct sockaddr_in6 *)&aux_server;
					addr->sin6_addr.s6_addr32[3] += (cy_index & 0xff) - 1;
					xinet_ntoa(&aux_server,string,sizeof(string));
				}
			}
		}
		*write_method = write_cySPortRemoteIP6;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTSOCKETPORT:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (size && cy_index) {
			unsigned char *p;
			if ((p = strchr(string,'+')) != NULL) {
				*p = 0x00;
				size = atoi(string);
				size += cy_index - 1;
				sprintf(string,"%d",size);
			}
		}
		*write_method = write_cySPortSocketPort;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTDBMODE:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (!size) { // default value
			strcpy(string,"circ");
		}
		*write_method = write_cySPortDBmode;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;

	      case CYSPORTSNIFFSESS:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		*write_method = write_cySPortSniffSess;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;
	      case CYSPORTPMTYPE:
		size = cy_rwsp_conf(cy_prefix,var_cySPT_name[vp->magic-2],string,255,0);
		if (!size) { // default value
			strcpy(string,PROD_PMTYPE);
		}
		*write_method = write_cySPortPmType;
		*var_len = strlen(string);
		return (u_char *) & string;
		break;
		      
	      default:
		break;
	}
	return NULL;
}

int
write_cySPort_str(int action,
		  u_char * var_val,
		  u_char var_val_type,
		  size_t var_val_len,
		  u_char * statP, oid * name, size_t name_len)
{
	cy_index = name[CYSSPEC_OID_LEN];
	cy_column = name[CYSSPEC_OID_LEN - 1];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		break;

	case RESERVE2:
		break;

	case FREE:
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,var_cySPT_name[cy_column-2],var_val, var_val_len,1);
		break;

	case UNDO:
		break;

	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPort_int(int action,
		   u_char * var_val,
		   u_char var_val_type,
		   size_t var_val_len,
		   u_char * statP, oid * name, size_t name_len)
{
#define CY_NSPEED	22
	unsigned speedlist[] = {
		    50,     75,    110,   134,    150,   200,   300,   600, 
		  1200,   1800,   2400,  4800,    9600, 14400, 19200, 28800, 
		 38400,  57600, 76800, 115200, 230400, 460800};

	cy_index = name[CYSSPEC_OID_LEN];
	cy_column = name[CYSSPEC_OID_LEN - 1];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_INTEGER) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len > sizeof(long)) {
			return SNMP_ERR_WRONGLENGTH;
		}
		switch (cy_column) {
			case CYSPORTSPEED : 
				for (cy_index=0; cy_index < CY_NSPEED; cy_index++) {
					if (*(long*)var_val == speedlist[cy_index]) break;
				}
				if (cy_index == CY_NSPEED) 
					return SNMP_ERR_WRONGVALUE;
				break;

			case CYSPORTDATASIZE : 
				switch(*(long*)var_val) {
					case 4: case 5 : case 6 : case 7: case 8: break;
					default : return SNMP_ERR_WRONGVALUE; break;
				}
				break;

	      		case CYSPORTSTOPBITS: 
				switch (*(long*)var_val) {
					case 1 : case 2: break;
					default : return SNMP_ERR_WRONGVALUE; break;
				}
				break;
		}

		break;

	case RESERVE2:
		break;

	case FREE:
		break;

	case ACTION:
		sprintf(ipaddr,"%d", *(long*)var_val);
		cy_rwsp_conf(cy_prefix,var_cySPT_name[cy_column-2],ipaddr,strlen(ipaddr),1);
		break;

	case UNDO:
		break;

	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortParity(int action,
		    u_char * var_val,
		    u_char var_val_type,
		    size_t var_val_len,
		    u_char * statP, oid * name, size_t name_len)
{
	unsigned char *paritylist[] = {"none", "odd", "even"};

	cy_index = name[CYSSPEC_OID_LEN];
	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		for (cy_index = 0; cy_index < 3; cy_index++) {
			if ((var_val_len == strlen(paritylist[cy_index])) &&
			     !strncmp(var_val,paritylist[cy_index],var_val_len)) {
				break;
			}
		}
		if (cy_index == 3) return SNMP_ERR_WRONGVALUE;
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"parity",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;

	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortFlowCtrl(int action,
		      u_char * var_val,
		      u_char var_val_type,
		      size_t var_val_len,
		      u_char * statP, oid * name, size_t name_len)
{
	unsigned char *flowlist[] = {"none", "hard", "soft"};

	cy_index = name[CYSSPEC_OID_LEN];
	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len != 4) {
			return SNMP_ERR_WRONGLENGTH;
		}
		for (cy_index = 0 ; cy_index < 3; cy_index++) {
			if (!strncmp(var_val, flowlist[cy_index], 4)) {
				break;
			}
		}
		if (cy_index == 3) return SNMP_ERR_WRONGVALUE;
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"flow",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortAuthtype(int action,
		      u_char * var_val,
		      u_char var_val_type,
		      size_t var_val_len,
		      u_char * statP, oid * name, size_t name_len)
{
	unsigned char *authtypelist[] = {
		"none",                  "radius",   "TacacsPlus",            "local", 
		"radius/local", "TacacsPlus/local", "local/radius", "local/TacacsPlus", 
		"RadiusDownLocal", "TacacsPlusDownLocal", "kerberos", "ldap",
		"ldapDownLocal", "kerberosDownLocal"};

	cy_index = name[CYSSPEC_OID_LEN];
	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len > 25) {
			return SNMP_ERR_WRONGLENGTH;
		}
		for (cy_index = 0; cy_index < 14; cy_index++) {
			if ((var_val_len == strlen(authtypelist[cy_index])) &&
			     !strncmp(var_val, authtypelist[cy_index], var_val_len)) {
					break;
			}
		}
		if (cy_index == 14) return SNMP_ERR_WRONGVALUE;
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"authtype",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPort_ipaddr(int action,
		      u_char * var_val,
		      u_char var_val_type,
		      size_t var_val_len,
		      u_char * statP, oid * name, size_t name_len)
{
	cy_index = name[CYSSPEC_OID_LEN];
	cy_column = name[CYSSPEC_OID_LEN - 1];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_IPADDRESS) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len > sizeof(u_long)) {
			return SNMP_ERR_WRONGLENGTH;
		}
		aux.s_addr = *(unsigned long *)var_val;
		cy_paux = inet_ntoa(aux);
		if (!inet_aton(cy_paux, &aux)) {
			return SNMP_ERR_WRONGVALUE;
			break;
		}
		strcpy(ipaddr,cy_paux);
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,var_cySPT_name[cy_column-2],ipaddr, strlen(ipaddr) ,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPort_ipaddr_4_6(int action,
		      u_char * var_val,
		      u_char var_val_type,
		      size_t var_val_len,
		      u_char * statP, oid * name, size_t name_len)
{
	char *errorMessage;
	cy_index = name[CYSSPEC_OID_LEN];
	cy_column = name[CYSSPEC_OID_LEN - 1];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		// the address can have a "+" at the end
		if (var_val_len > sizeof(ipaddr)+1) {
			return SNMP_ERR_WRONGLENGTH;
		}
		
		errorMessage = xinet_aton((char *)var_val,&aux_server);
		if (errorMessage != NULL) {
			return SNMP_ERR_WRONGVALUE;
		}
		
		xinet_ntoa(&aux_server,ipaddr,sizeof(ipaddr));
		
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,var_cySPT_name[cy_column-2],ipaddr, strlen(ipaddr) ,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortProtocol(int action,
		      u_char * var_val,
		      u_char var_val_type,
		      size_t var_val_len,
		      u_char * statP, oid * name, size_t name_len)
{
	unsigned char *protocollist[] = {
		"login",    "rlogin",         "telnet",     "ssh", 
		"slip",      "cslip",           "ppp", "tcpclear", 
		"tcplogin",    "console", "socket_client", 
		"socket_server", "socket_ssh",     "raw_data",
		"ssh2",  "ppp_only", "ipdu" }; //[RK]Jan/08/03 PM8

	cy_index = name[CYSSPEC_OID_LEN];
	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len > 40) {
			return SNMP_ERR_WRONGLENGTH;
		}
		for (cy_index=0; cy_index < 17; cy_index++) { //[RK]Jan/08/03 PM8
			if ((var_val_len == strlen(protocollist[cy_index])) &&
			    !strncmp(var_val, protocollist[cy_index], var_val_len)) {
				break;
			}
		}
		if (cy_index == 17) return SNMP_ERR_WRONGVALUE; //[RK]Jan/08/03 PM8
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"protocol",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortRemoteIP(int action,
		      u_char * var_val,
		      u_char var_val_type,
		      size_t var_val_len,
		      u_char * statP, oid * name, size_t name_len)
{
	int size;
	cy_index = name[CYSSPEC_OID_LEN];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len > 20) {
			return SNMP_ERR_WRONGLENGTH;
		}
		strncpy(ipaddr, var_val, var_val_len);
		ipaddr[var_val_len] = 0x00;
		if (!cy_index) {
			if ((cy_paux = strchr(ipaddr,'+')) != NULL) {
				*cy_paux = 0x00;
			}
		}
		if (!inet_aton(ipaddr, &aux)) {
			return SNMP_ERR_WRONGVALUE;
			break;
		}
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"ipno",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortRemoteIP6(int action,
		      u_char * var_val,
		      u_char var_val_type,
		      size_t var_val_len,
		      u_char * statP, oid * name, size_t name_len)
{
	int size;
	char *errorMessage;
	cy_index = name[CYSSPEC_OID_LEN];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len > INET6_ADDRSTRLEN) {
			return SNMP_ERR_WRONGLENGTH;
		}
		strncpy(ipaddr, var_val, var_val_len);
		ipaddr[var_val_len] = 0x00;
		if (!cy_index) {
			if ((cy_paux = strchr(ipaddr,'+')) != NULL) {
				*cy_paux = 0x00;
			}
		}
		
		errorMessage = xinet_aton((char *)ipaddr, &aux_server);
		if (errorMessage != NULL || aux_server.ss_family != AF_INET6) {
			return SNMP_ERR_WRONGVALUE;
		}
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"ipno6",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortSocketPort(int action,
			u_char * var_val,
			u_char var_val_type,
			size_t var_val_len,
			u_char * statP, oid * name, size_t name_len)
{
	cy_index = name[CYSSPEC_OID_LEN];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"socket_port",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortSniffSess(int action,
		       u_char * var_val,
		       u_char var_val_type,
		       size_t var_val_len,
		       u_char * statP, oid * name, size_t name_len)
{
	unsigned char *snifmodelist[] = {"in", "out", "i/o", "no" };

	cy_index = name[CYSSPEC_OID_LEN];
	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		for (cy_index=0; cy_index < 4; cy_index++) {
			if ((var_val_len == strlen(snifmodelist[cy_index])) &&
			    !strncmp(var_val, snifmodelist[cy_index],var_val_len)) {
				break;
			}
		}
		if (cy_index == 4) {
			return SNMP_ERR_WRONGVALUE;
		}
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"sniff_mode",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortDBmode(int action,
		    u_char * var_val,
		    u_char var_val_type,
		    size_t var_val_len,
		    u_char * statP, oid * name, size_t name_len)
{
	cy_index = name[CYSSPEC_OID_LEN];

	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}

	switch (action) {
	case RESERVE1:
		if (var_val_type != ASN_OCTET_STR) {
			return SNMP_ERR_WRONGTYPE;
		}
		if (var_val_len > 5) {
			return SNMP_ERR_WRONGLENGTH;
		}
		break;

	case ACTION:
		cy_rwsp_conf(cy_prefix,"DB_mode",var_val, var_val_len,1);
		break;

	case RESERVE2:
	case FREE:
	case UNDO:
	case COMMIT:
		break;
	}
	return SNMP_ERR_NOERROR;
}

int
write_cySPortPmType(int action,
		u_char * var_val,
		u_char var_val_type,
		size_t var_val_len,
		u_char * statP, oid * name, size_t name_len)
{
	cy_index = name[CYSSPEC_OID_LEN];
	if (cy_index) {
		sprintf(cy_prefix,"s%d",cy_index);
	} else {
		strcpy(cy_prefix,"all");
	}
	switch (action) {
		case RESERVE1:
			if (var_val_type != ASN_OCTET_STR) {
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > 40) {
				return SNMP_ERR_WRONGLENGTH;
			}
			{
				struct stat st;
				char buffer[50], *s;
				strcpy(buffer, "/etc/pm.");
				strncat(buffer, var_val, var_val_len);
				for (s=buffer; !isspace(*s) && *s; s++);
				*s = 0;
				if (stat(buffer, &st) == -1) {
					return SNMP_ERR_WRONGVALUE;
				}
			}
			break;
			
		case ACTION:
			cy_rwsp_conf(cy_prefix,"pmtype",var_val, var_val_len,1);
			break;
			
		case RESERVE2:
		case FREE:
		case UNDO:
		case COMMIT:
			break;
	}
	return SNMP_ERR_NOERROR;
}
										
