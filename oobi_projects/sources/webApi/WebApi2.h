#ifndef WEBAPI2_H
#define WEBAPI2_H

#include <webApi.h>

extern int qDebug;

#define CONFIG_FILE	"/etc/config_files" //[RK]May/22/06 - update automatically

#define SECURITY_SHELL	     "/etc/security.opts" /* [LMT] Security Enhancements */
#define INETD_FILE           "/etc/inetd.conf"
#ifdef NISenable
#define NIS_SHELL            "/etc/daemon.d/ypbind.conf"
#endif
#define IPSEC_SHELL          "/etc/daemon.d/ipsec.sh"
#define SSHD_SHELL           "/etc/daemon.d/sshd.sh"
#define CYBUFFERING_SHELL    "/etc/daemon.d/cy_buffering.sh"
#define SNMPD_SHELL          "/etc/daemon.d/snmpd.sh"
#define NTPCLIENT_SHELL      "/etc/daemon.d/ntpclient.conf"
#define HOSTNAME_FILE        "/etc/hostname"
#define HOSTS_FILE           "/etc/hosts"
#define ISSUE_FILE           "/etc/issue"
#define PSLAVE_FILE          "/etc/portslave/pslave.conf"
#ifdef BONDING_FEATURE
#define BONDING_FILE         "/etc/bonding.opts"
#endif
#define KVMD_FILE	     "/etc/kvmd.conf"
#define RESOLV_FILE          "/etc/resolv.conf"
#define HOST_CONF_FILE       "/etc/host.conf"
#define ST_ROUTES_FILE       "/etc/network/st_routes"
#define PASSWD_FILE          "/etc/passwd"
#define GROUP_FILE           "/etc/group"
#define SYSLOG_NG_FILE       "/etc/syslog-ng/syslog-ng.conf"
#define ETC_DIR		 "/etc/"
#define IPSEC_SECRET_FILE    "/etc/ipsec.secrets"
#define PROC_VERSION_FILE    "/proc/version"
#define PROC_CPU_FILE        "/proc/cpuinfo"
#define PROC_MEM_FILE        "/proc/meminfo"
#define DEV_LED_FILE         "/dev/led"
#define WEB_TEMP_FILE        "/tmp/webtemp"
#define SNMPD_FILE           "/etc/snmp/snmpd.conf"
#define SNMPD_LOCAL_FILE     "/etc/snmp/snmpd.local.conf"
#define DOMAINNAME_CONF_FILE "/etc/domainname.conf"
#define YP_CONF_FILE         "/etc/yp.conf"
#define NSSWITCH_CONF_FILE   "/etc/nsswitch.conf"
#define NETWK_INTERFACE_FILE "/etc/network/interfaces"
#ifdef PCMCIA
#define WIRELESS_OPTS_FILE   "/etc/pcmcia/wireless.opts"
#define ISDN_OPTS_FILE       "/etc/pcmcia/isdn.opts"
#define IDE_OPTS_FILE        "/etc/pcmcia/ide.opts"
#define SERIAL_OPTS_FILE     "/etc/pcmcia/serial.opts"
#endif
#define MGETTY_LOGIN_FILE    "/etc/mgetty/login.config"
#define MGETTY_CONFIG_FILE   "/etc/mgetty/mgetty.config"
#define PROC_MOUNTS_FILE     "/proc/mounts"
#ifdef ONS
#define MENUSH_CFG_FILE      "/etc/menush.cfg"
#else
#define MENUSH_CFG_FILE      "/bin/menush.cfg"
#endif
#define LDAP_CONF_FILE       "/etc/ldap.conf"
#ifdef KRBenable
#define KRB5_CONF_FILE       "/etc/krb5.conf"
#endif
#define RADIUS_CONF_FILE		"/etc/raddb/server"
#define TACPLUS_CONF_FILE	"/etc/tacplus.conf"
#define ERROR_LOG_FILE       "/tmp/webErr.log"
#define CHAP_SECRETS_FILE    "/etc/ppp/chap-secrets"
#define PAP_SECRETS_FILE     "/etc/ppp/pap-secrets"
#define ERROR_LOG_FILE       "/tmp/webErr.log"
#define WEBUI_SHELL          "/etc/daemon.d/webui.conf"
#define PORTMAP_SHELL	     "/etc/daemon.d/portmap.conf"
#define SMB_CONF_FILE	     "/etc/pam_smb.conf"
#ifdef KVM
#define KVMCONFFILE          "/etc/kvmd.conf"
#define KVMSLAVELISTFILE     "/etc/kvmd.slaves"
#define KVMGROUP_FILE         "/etc/kvmd.groups"
#endif

#define FLAG_READ         1 // The parameter was read from the file 

#define TYPE_NUM			'N'
#define TYPE_STRING			'S'
#define TYPE_LIST			'L'
#define TYPE_IP				'I'
#define TYPE_IPMASK			'M'
#define TYPE_LISTVAR		'l'
#define TYPE_STRVAR			's'
#define TYPE_STRVAR2		't'
#define TYPE_STRLINE		'x'

#define BUFLEN  127
#define MAXOUTSET 16

#define FILE_WRITE_ERROR    "Error opening %s to write"
#define FILE_READ_ERROR     "Error opening %s to read"
#define FILE_STAT_ERROR     "Error getting stats for %s"
#define MALLOC_FAIL         "Malloc of %u bytes failed"
#define DIR_OPEN_ERROR      "Error opening directory %s"

#define EXE_SIGNAL_RAS	0x01
#define EXE_DAEMON_SH	0x02
#define EXE_SIGHUP_KVM	0x04
#define EXE_FIREWALL	0x08

#define dolog(X, P... )  if (qDebug) {         \
          printf(X, ##P);      \
          printf("\r\n");     \
          fflush(stdout);     \
      }


typedef Result (readWriteParamRoutine) (void *, CycParam *, int);
typedef void *(parseRoutine) (unsigned char *, CycParam *, void *);
typedef Result (writeroutine) (unsigned char *, CycParam *);

typedef char Outletset[MAXOUTSET];

extern int executeFlg;

/** Category-Key structure: 
 ** Contains the category, key, and corresponding info **/
typedef struct 
{
	CycCategory category;
	CycKey key;
	unsigned char *fileName;
	readWriteParamRoutine *getParamRoutine;
	parseRoutine *parser;
	writeroutine *write;
	unsigned char *defaultPrefix;
	unsigned char *name;
	unsigned char type;
	unsigned short size;
	void *listOptions;
	int nOptions;
	unsigned char flag;
} CycCatKeyTable; 


typedef struct {
  char name[4];
  unsigned char flag;
} ttyList;

typedef struct 
{
	unsigned char *paramName;
	void *listOptions;
	int nOptions;	
	int type;
	int off;
	int size;
} ParamEntry;

extern void LogError(char *fmt, ...);
extern int Min(int x, int y);
extern int ParseLineLoginConfig(unsigned char *buf, unsigned char **username, 
		unsigned char **userid, unsigned char **utmpEntry, 
		unsigned char **loginPrgm, unsigned char **arguments);
extern int ParseLineNis(unsigned char *buf, unsigned char **word, unsigned char **argument);
extern void PrintLinePslave (FILE *fp, unsigned char *w, unsigned char *c, unsigned char *a);
extern int ParseLinePslave (unsigned char *buf, unsigned char **word,
                            unsigned char **command, unsigned char **argument);
extern int ParseLineSnmpd(unsigned char *buf, unsigned char **word, unsigned char **argument);
#ifdef PMD1
extern int cycPmIpduConf(UInt32,char *,UInt32 *,UInt32);
extern int cycPmIpduInfo(UInt32 ,UInt32 ,PmIpduInfo *);
extern int cycPmIpduAlarm(UInt32, UInt32, PmAlarmStr *, UInt32);
extern int cycPmIpduStatus(UInt32 , UInt32 , char * , PmIpduUnitValues *);
#endif
extern CycYesNoOption ServiceActive(char *, char *, CycYesNoOption , int);
extern int ReturnNisType(int rdWrtFlg);

#endif /* WEBAPI2_H */
