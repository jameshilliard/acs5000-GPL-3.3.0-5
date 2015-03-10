/* -*- linux-c -*- */
/*****************************************************************
 * File: cyc.h
 *
 * Copyright (C) 2004-2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc header
 *      
 ****************************************************************/

#ifndef LIBCYC_H
#define LIBCYC_H 1

#ifndef LICENSE
//#ifdef TARGET_CPS
#ifdef OEM3
#define LICENSE "APC"
#else
#define LICENSE "Cyclades"
#endif
#endif

#define LIBCYC_VERSION "3.0"

/* Some definitions */
#ifndef TRUE
#define TRUE   1
#endif
#ifndef FALSE
#define FALSE  0
#endif
#ifndef ERROR
#define ERROR -1
#endif

#define LOCK_EXT "pid"
#define SOCK_EXT "sock"

#ifndef MAX_STR_LEN
#define MAX_STR_LEN 255
#endif
#define IP_ADDR_STR_LEN 16
#define SESS_STR_LEN 32
#define KVM_ALIAS_LEN 21
#define MD5_DIGEST_LEN 16
#define MD5SUM_HEXSTR_LEN MD5_DIGEST_LEN*2 + 1
#define OUTLET_STR_LEN 4
#define PHYSPORT_STR_LEN 6
#define MAX_MSG_LEN 1024     /* Max length for IPC messages */
#define BDINFO_STR_LEN 32
#define KVM_USERNAME_LEN 32
#define PROC_BDINFO_FILE "/proc/bdinfo"
#define PROC_UPTIME_FILE "/proc/uptime"
#define PROC_BDINFO_DELIMITER '!'
#define NETCONF_RUNFILE "/tmp/internal_net.running"
#define VCARD_MAC_FILE_PREFIX "/tmp/vcard_mac"
#define NETCONF_RUNFILE_DELIMITER ':'
#define VCARD_TYPE_FILE "/etc/vcard_type"
#define VCARD_TYPE_DELIMITER ':'
#define KVM_CASC_DELIMITER '.'

/* AlterPath Product Defines */
#define KVM_BDINFO_STR "KVM"
#define KVMNET_BDINFO_STR "KVMN"
#define ONS_BDINFO_STR "ONS"
#define KVMNETPLUS_BDINFO_STR "KVMP"
/* For all products with fans */
#define AP_MIN_FAN_SPEED 2000
/* For KVM (analog) */
#define KVM_MAX_FANS 1
#define KVM_MAX_IP_MODULES 0
#define KVM_MAX_STATIONS 2
/* For KVM/net */
#define KVMNET_MAX_FANS 2
#define KVMNET_MAX_IP_MODULES 2
#define KVMNET_MAX_STATIONS 2
/* For KVM/netPlus */
#define KVMNET_PLUS_MAX_FANS 3
#define KVMNET_PLUS_MAX_IP_MODULES 4
#define KVMNET_PLUS_MAX_STATIONS 6
#define KVMNET_PLUS_LOCAL_USER_STATIONS 2
/* For OnSite */
#define ONSITE_MAX_FANS 2
#define ONSITE_MAX_IP_MODULES 2
#define ONSITE_V1_MAX_STATIONS 2
#define ONSITE_V2_MAX_STATIONS 3
#define ONSITE_LOCAL_USER_STATIONS 1
/* For ACS (power supply bitmap) */
#define PS_1 0x01
#define PS_2 0x02
#define PS_MASK 0x03
#define ACS_MAX_PS 2
	
/* File locations */
#define KVMIP_LED_DEVFILE "/dev/cy_kvmip_led"
#define APCTL_DEVFILE "/dev/cy_apctl"
#define LED_DEVFILE "/dev/led"
#define WDT_DEVFILE "/dev/wdt"
#define KVMD_RUNFILE "/var/run/kvmd.pid"
#define KVMD_CONFFILE "/etc/kvmd.conf"
#define KVMD_SLAVES_CONFFILE "/etc/kvmd.slaves"

/* Program locations */
#define KVM_COMMAND "/bin/kvm"
#define RDP_NAT_COMMAND "/usr/bin/rdp_nat.sh"
#define KVMIPCTL_COMMAND "/usr/bin/kvmipctl"
#define MONITOR_COMMAND "/usr/bin/monitor"
#define RDP_CONNTRACK_COMMAND "/usr/bin/rdp_conntrack"
#define PROXY_COMMAND "/usr/bin/proxy"
#define W_COMMAND "/usr/bin/w"
#define FORWARD_SH_COMMAND "/usr/bin/forward.sh"
#define PING_COMMAND "/bin/ping"
#define PM_COMMAND "/bin/pmCommand"
#define TRANSLATE_COMMAND "/usr/bin/translate.sh"

#ifndef ON
#define ON 1
#endif
#ifndef OFF
#define OFF 0
#endif

#ifdef __cplusplus
#include <cstdlib>
extern "C" {
#else
#include <stdlib.h>
#endif

#include <pthread.h>

typedef struct _libcyc_list {
	int active;
	void *data;
	struct _libcyc_list *next;
	struct _libcyc_list *prev;
} libcyc_list;

enum libcyc_flags {
	CYC_CLEAR = 0,      /* No flags set */
	CYC_INIT = 1<<0,    /* Is the library initialized? */
	CYC_DEBUG = 1<<1,   /* Debug mode */
	CYC_CLIENT = 1<<2,  /* Initialize as a client */
	CYC_SERVER = 1<<3,  /* Initialize as a server */
	CYC_DAEMON = 1<<4,  /* Initialize as a daemon */
	CYC_CGI = 1<<5,     /* Initialize as a CGI */
	CYC_USE_SYSLOG = 1<<6, /* Use syslog for output */
	CYC_USE_STDERR = 1<<7, /* Use stderr for output */
	CYC_DEV_PERSIST = 1<<8, /* Devs stay open until libcyc_destroy() */
	CYC_NOLOCK = 1<<9,  /* Don't use lockfile */
	CYC_CONF_NOUPDATE = 1<<10, /* Use current config */
};

/* This order should be kept in sync with the KVM_MODEL
   array in libcyc_kvm.c */
enum kvm_model {
	MODEL_INVALID = -1,
        MODEL_KVM = 0,
        MODEL_KVMNET,
	MODEL_ONSITE,
        MODEL_KVMNET_PLUS,
	MODEL_KVMNET_4000,
        MODEL_KVMNET_MAX,
	MODEL_PC_SIMULATION,
};

enum kvm_ports {
	PORT_4=4,
	PORT_8=8,
        PORT_16=16,
        PORT_32=32,
};

enum kvmip_id {
	NO_CARDS=0,
	CARD_1=1<<0,
	CARD_2=1<<1,
	CARD_3=1<<2,
	CARD_4=1<<3
};

enum cyc_fanid {
	NO_FAN=0,
	FAN_1=1<<0,
	FAN_2=1<<1,
	FAN_3=1<<2
};

/* This should match the kernel header prxk.h */
enum board_type {
	MODEL_PR3K = 0,
	MODEL_PR2K = 1,
	MODEL_PR4K = 2,
	MODEL_PR1K = 3,
	MODEL_TS1K = 4,
	MODEL_TS2K = 5,
	MODEL_TS4H = 6,
	MODEL_TS8H = 7,
	MODEL_TS3K = 8,
	MODEL_TX1K = 9,
	MODEL_TX2K = 10,
	MODEL_PL1K = 11,
	MODEL_TS1H = 12,
	MODEL_NL1K = 13,
	MODEL_TS1100 = 14,
	MODEL_ACS16 = 14,
	MODEL_TS2100 = 15,
	MODEL_ACS32 = 15,
	MODEL_SM100 = 16,
	MODEL_PR3500 = 17,
	MODEL_TS110 = 18,
	MODEL_ACS48 = 19,
	MODEL_ACS4 = 20,
	MODEL_ACS8 = 21,
	MODEL_ACS1 = 22,
	/* Model #23: Reserved */
	MODEL_KVM16 = 24,
	MODEL_KVM32 = 25,
	MODEL_KVMNET16 = 26,
	MODEL_KVMNET32 = 27,
	MODEL_ONSITE441 = 28,
	MODEL_ONSITE481 = 29,
	MODEL_ONSITE841 = 30,
	MODEL_ONSITE881 = 31,
	MODEL_ONSITE442 = 32,
	MODEL_ONSITE482 = 33,
	MODEL_ONSITE842 = 34,
	MODEL_ONSITE882 = 35,
	MODEL_KVMNETPLUS16 = 36,
	MODEL_KVMNETPLUS32 = 37,
	MODEL_AVCS1 = 38,
	MODEL_AVCS4 = 39,
	MODEL_AVCS8 = 40,
	MODEL_AVCS16 = 41,
	MODEL_AVCS32 = 42,
	MODEL_AVCS48 = 43,
	MODEL_ACS5001 = 44,
	MODEL_ACS5004 = 45,
	MODEL_ACS5008 = 46,
	MODEL_ACS5016 = 47,
	MODEL_ACS5032 = 48,
	MODEL_ACS5048 = 49,
};

/* This structure is deprecated along with the get_machine_type()
   function; use the libcyc_prod_* and libcyc_kvm_* API for new code */
struct MachineType {
	int nportsS;
	int nportsA;
	int nportsK;
	int nportsM;
	int board_type;
	char name[32];
	char board_name[32];
	char vendor_name[32];
	char prod_line[32];
	char prod_name[32];
	char version[32];
	int cpuclock;
	int ramsize;
	int flashsize;
	char serial_number[32];
	unsigned int fpga_version;
	unsigned int board_version;
	char ipboard[2];
	char mac_address[32];
	char app_serial_number[11]; 
};

enum kvmipm_type {
	KVMIP_TYPE_1 = 1, /* Soronti */
	KVMIP_TYPE_2, /* Midas */
};

enum port_type {
	PORT_TYPE_UNKNOWN,
	PORT_TYPE_SERIAL,
	PORT_TYPE_KVM,
	PORT_TYPE_POWER,
	PORT_TYPE_SOL,
};

struct prod_info {
	char board_name[BDINFO_STR_LEN];
	int nportsS;
	int nportsA;
	int nportsK;
	int nportsM;
	char name_code[BDINFO_STR_LEN];
	enum board_type type;
	char vendor_name[BDINFO_STR_LEN];
	char prod_line[BDINFO_STR_LEN];
	char prod_name[BDINFO_STR_LEN];
	char kernel_version[BDINFO_STR_LEN];
	int cpuclock;
	int ramsize;
	int flashsize;
	char serial_number[BDINFO_STR_LEN];
	unsigned int fpga_version;
	unsigned int board_version;
	char ipboard[2];
	char mac_address[BDINFO_STR_LEN];
	char app_serial_number[11]; 
};

#define KVM_PERMS_READ   1<<0
#define KVM_PERMS_WRITE  1<<1
#define KVM_PERMS_POWER  1<<2

#define POWER_STATE_OFF      1<<0
#define POWER_STATE_ON       1<<1
#define POWER_STATE_LOCKED   1<<2
#define POWER_STATE_UNLOCKED 1<<3

struct power_info {
	int outlet; /* Outlet to which this info applies */
	int state; /* State of the outlet */
};

enum slave_type {
	KVM_TYPE_SLAVE_INVALID,
	KVM_TYPE_SLAVE_SWITCH, /* A KVM Switch */
	KVM_TYPE_SLAVE_EXP, /* KVM Expander */
};

struct kvm_slave_info {
	char name[KVM_ALIAS_LEN]; /* Name of the slave device */
	int port_a; /* Port of the primary cascaded connection */
	int port_b; /* Port of the secondary cascaded connection */
	int num_ports; /* Number of ports on the slave device */
	enum slave_type type; /* Type of the slave device */
};

struct session_info {
	char physport_str[PHYSPORT_STR_LEN];
	char user[SESS_STR_LEN]; /* Username associated with the session */
	char to_alias[SESS_STR_LEN]; /* Port alias to which
					user is connected */
	char *to_port; /* Physical port number to which user is connected */
	char from[SESS_STR_LEN]; /* Station ID string */
	int from_station; /* Station ID number of the session */
	char login_start[SESS_STR_LEN]; /* Login start time */
        int login_duration; /* Duration in seconds */
	char pid_cmd[SESS_STR_LEN]; /* PID of prog handling session */
	int perms; /* Permissions of the session */
	libcyc_list *pinfo; /* Power status (Data of type struct power_info
			       should be contained in this linked list) */
};

enum op_status {
	NOT_RESPONDING,
	RESPONDING,
};

enum sess_status {
	INACTIVE,
	ACTIVE
};

struct cyc_port_status {
	int vport;
	char alias[KVM_ALIAS_LEN];
	char physport[PHYSPORT_STR_LEN];
	enum op_status op_stat;
	enum sess_status sess_stat;
	int power_state;
	enum port_type protocol;
	libcyc_list *pinfo; /* Power status (Data of type struct power_info
			       should be contained in this linked list) */
};

struct shmem_info {
	int id;
	void *mem;
};

struct sem_info {
	int id;
	int nsems;
};

struct dev_info {
	char dev_filepath[MAX_STR_LEN];
	char name[MAX_STR_LEN];
	int fd;
	int semid;
};

struct cyc_port_info {
	int port; /* Physical port number (NN) */
	enum port_type type; /* Type of the port */
	char alias[KVM_ALIAS_LEN]; /* Port name */
	char physport[PHYSPORT_STR_LEN];
	int active;
};

struct power_outlet {
	int outlet;
};

struct kvmd_config {
	int vport;
	int port;
	enum port_type type;
	char alias[KVM_ALIAS_LEN];
	int active;
	libcyc_list *outlets;
};

struct kvm_info {
        enum kvmip_id ip_module_map;
        enum kvm_model model;
        enum kvm_ports ports;
	int num_ip_modules;
	int max_ip_modules;
	int max_fans;
	int num_stations;
	libcyc_list *slave_data;
	char *kvmd_slave_conf_file_md5sum;
};

struct kvm_hwinfo {
	int sysfpga_ver; /* Sys FPGA Ver */
	int muxfpga_ver; /* MUX FPGA Ver (only for KVM, KVM/net,
			    and OnSite v1) */
};

struct acs_info {
	int has_dual_power;
	int ps_status;
};

struct cyc_info {
	enum libcyc_flags flags;
	int e_pipe[2];
	char appname[MAX_STR_LEN];
	char appver[MAX_STR_LEN];
	char lockdir[MAX_STR_LEN];
	char lock_filepath[2*MAX_STR_LEN];
	char lock_daemon_filepath[2*MAX_STR_LEN];
	char lock_cgi_filepath[2*MAX_STR_LEN];
	char tmpdir[MAX_STR_LEN];
	char sock_filepath[2*MAX_STR_LEN];
	char mac_last3[9];
	libcyc_list *sem_ids;
	libcyc_list *shmem_ids;
	libcyc_list *mutexes;
	libcyc_list *mem_ptrs;
	libcyc_list *devs;
	libcyc_list *def_port_names;
	libcyc_list *conf_data;
	char *conf_file_md5sum;
	pthread_mutex_t *m;
	struct prod_info prod;
	struct kvm_info kvm;
	struct acs_info acs;
};

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun
{
        int val;           
        struct semid_ds *buf;      
        unsigned short int *array;
        struct seminfo *__buf;
};
#endif
	
/* Main libcyc functions (init/destroy) */
struct cyc_info *libcyc_init(enum libcyc_flags,const char *,
			     const char *,const char *, const char *);
void libcyc_destroy(struct cyc_info *);
void libcyc_debug(struct cyc_info *, const char *);
int get_machine_type(struct MachineType *pmt); /* Deprecated */
enum libcyc_flags libcyc_add_flag(struct cyc_info *, enum libcyc_flags);
enum libcyc_flags libcyc_del_flag(struct cyc_info *, enum libcyc_flags);
int libcyc_port_names_get_default(struct cyc_info *, libcyc_list **);
int libcyc_kvm_port_names_get_default(struct cyc_info *, libcyc_list **);
void libcyc_port_names_destroy_default(struct cyc_info *, libcyc_list **);
const char *libcyc_port_name_get_default_list(struct cyc_info *,
	                                      libcyc_list **, int,
					      enum port_type);
const char *libcyc_port_name_get_default(struct cyc_info *, int,
					 enum port_type);
int libcyc_prod_is_cs(struct cyc_info *);
int libcyc_prod_is_avcs(struct cyc_info *);
int libcyc_prod_is_acs5k(struct cyc_info *);
int libcyc_prod_is_csORavcs(struct cyc_info *);
int libcyc_prod_is_kvm(struct cyc_info *);
int libcyc_prod_is_onsite(struct cyc_info *);
const char *libcyc_prod_get_model_str(struct cyc_info *);
const char *libcyc_prod_get_prodcode_str(struct cyc_info *);
/* These are simple accessor functions to cyc->prod; the macros are
   used by default in order to provide better performance */
#ifdef CYC_REAL_DATA_FNS
enum board_type libcyc_prod_get_bd_type(struct cyc_info *);
const char *libcyc_get_ver(void);
enum libcyc_flags libcyc_get_flags(struct cyc_info *)
int libcyc_prod_get_bd_ver(struct cyc_info *);
int libcyc_prod_get_fpga_ver(struct cyc_info *);
const char *libcyc_prod_get_serno(struct cyc_info *);
const char *libcyc_prod_get_appserno(struct cyc_info *);
int libcyc_prod_get_nportsS(struct cyc_info *);
#else
#define libcyc_prod_get_bd_type(A) A->prod.type
#define libcyc_get_ver() LIBCYC_VERSION
#define libcyc_get_flags(A) A->flags
#define libcyc_prod_get_bd_ver(A) A->prod.board_version
#define libcyc_prod_get_fpga_ver(A) A->prod.fpga_version
#define libcyc_prod_get_serno(A) A->prod.serial_number
#define libcyc_prod_get_appserno(A) A->prod.app_serial_number
#define libcyc_prod_get_nportsS(A) A->prod.nportsS
#endif

/* Libcyc malloc replacement functions */
void *libcyc_malloc(struct cyc_info *, size_t);
void libcyc_free(struct cyc_info *, void *);
void libcyc_free_all(struct cyc_info *);
char *libcyc_strdup(struct cyc_info *, const char *);

/* Stack functions */
int libcyc_push(libcyc_list **, void *);
void *libcyc_pop(libcyc_list **);

/* Linked list functions */
libcyc_list *libcyc_ll_append(libcyc_list *,void *);
libcyc_list *libcyc_ll_remove(libcyc_list *,void *);
void libcyc_ll_free(libcyc_list *);
void *libcyc_ll_iterate(libcyc_list *);
int libcyc_ll_get_size(libcyc_list *);

/* Utility functions */
int libcyc_system_pipe(struct cyc_info *, char *, char **,
		       void (*data_fn)
		       (struct cyc_info *, char *, int, void *), void *);
int libcyc_system_str(struct cyc_info *, char *, char **, char *, int *);
int libcyc_system(struct cyc_info *, char *, char **);
int libcyc_system_quiet(struct cyc_info *, char *, char **);
int libcyc_system_nowait(struct cyc_info *, char *, char **);
char *libcyc_create_filepath(struct cyc_info *, const char *,
                             const char *, const char *);
pid_t libcyc_get_daemon_pid(struct cyc_info *);
pid_t libcyc_get_cgi_pid(struct cyc_info *);
pid_t libcyc_get_pid(struct cyc_info *);

char *libcyc_get_sock_filepath(struct cyc_info *);
int libcyc_file_exist(const char *);
pid_t libcyc_update_pid(struct cyc_info *);
const char *libcyc_iface_get_ip(struct cyc_info *, const char *);
char *libcyc_iface_get_mac(struct cyc_info *, const char *);
char *libcyc_iface_get_mac_delim(struct cyc_info *, const char *, char);
int libcyc_iface_get_mac_raw(struct cyc_info *, const char *,
			     char **, int *);
char *libcyc_get_lockfile(struct cyc_info *,
			  enum libcyc_flags);
double libcyc_get_uptime(void);
long libcyc_get_uptime_ms(void);
pid_t libcyc_get_pid_from_runfile(const char *);
int libcyc_lock(struct cyc_info *, const char *);
int libcyc_unlock(struct cyc_info *, const char *);
int libcyc_lock_status(struct cyc_info *, const char *);
int libcyc_open_dev(struct cyc_info *, const char *, int);
void libcyc_close_dev(struct cyc_info *, int);
int libcyc_ping(struct cyc_info *, const char *);
char *libcyc_md5sum_hexstr_get(struct cyc_info *, const char *);
int libcyc_update_list(struct cyc_info *, const char *, char **,
		       void (*)(struct cyc_info *, libcyc_list **),
		       libcyc_list **);

/* IPC functions */
int libcyc_sock_init(struct cyc_info *);
void libcyc_sock_destroy(struct cyc_info *,int);
void *libcyc_sock_recv(struct cyc_info *,int);
int libcyc_sock_send(struct cyc_info *,int,void *);
int libcyc_get_sock_from_sockfile(struct cyc_info *);
int libcyc_ipc_key_get(struct cyc_info *, const char *, int);
int libcyc_ipc_key_get_private(void);
int libcyc_ipc_shmem_create(struct cyc_info *, key_t, size_t);
int libcyc_ipc_shmem_get(struct cyc_info *, key_t, size_t, int);
void *libcyc_ipc_shmem_attach(struct cyc_info *, int, void *, int);
void libcyc_ipc_shmem_detach(struct cyc_info *, int);
int libcyc_ipc_shmem_destroy(struct cyc_info *, int);
int libcyc_ipc_sem_create(struct cyc_info *, key_t, int);
int libcyc_ipc_sem_get(struct cyc_info *, key_t, int, int);
int libcyc_ipc_sem_op(struct cyc_info *, int, int, int);
int libcyc_ipc_sem_op_flag(struct cyc_info *, int, int, int, int);
int libcyc_ipc_sem_timed_op(struct cyc_info *, int, int, int, long int);
int libcyc_ipc_sem_timed_op_flag(struct cyc_info *, int, int,
				 int, long int, int);
int libcyc_ipc_sem_destroy(struct cyc_info *, int);
int libcyc_ipc_sem_set_all(struct cyc_info *, key_t, int);
int libcyc_ipc_sem_set(struct cyc_info *, int, int, int);
int libcyc_ipc_sem_getval(struct cyc_info *, int, int);
int libcyc_ipc_sem_nsems_get(struct cyc_info *, int);

/* KVM Functions */
int libcyc_kvm_check_id(struct cyc_info *, int);
int libcyc_kvm_set_vidqual_disable(struct cyc_info *, int);
int libcyc_kvm_get_vidqual_disable(struct cyc_info *);
int libcyc_kvm_kvmipm_reset(struct cyc_info *, int);
enum kvmipm_type libcyc_kvm_kvmipm_get_type(struct cyc_info *, int);
const char *libcyc_kvm_get_model_str(struct cyc_info *);
const char *libcyc_kvm_get_model_short_str(struct cyc_info *);
char *libcyc_kvm_get_kvmip_ip_addr(struct cyc_info *, int);
int libcyc_kvm_rdpserver_probe(struct cyc_info *, const char *,
			       unsigned short);
int libcyc_kvm_get_station_from_kvmipm_id(struct cyc_info *, int);
int libcyc_kvm_get_kvmipm_id_from_station(struct cyc_info *, int);
char *libcyc_kvm_get_username_from_station(struct cyc_info *, int);
int libcyc_kvm_osd_warning(struct cyc_info *, const char *);
int libcyc_kvm_session_info_get(struct cyc_info *, libcyc_list **, int *);
void libcyc_kvm_session_info_destroy(struct cyc_info *, libcyc_list **);
int libcyc_kvm_viewer_title_create(struct cyc_info *, char *, int,
				   const char *, int);
int libcyc_kvm_wait_for_kvmipm_life(struct cyc_info *, int);
char *libcyc_kvm_get_physport_from_alias(struct cyc_info *, const char *);
char *libcyc_kvm_get_alias_from_physport(struct cyc_info *, const char *);
void libcyc_kvm_physport_alias_data_destroy(struct cyc_info *, libcyc_list **);
int libcyc_kvm_slave_get_port_a_from_name(struct cyc_info *, const char *);
int libcyc_kvm_slave_get_port_b_from_name(struct cyc_info *, const char *);
char *libcyc_kvm_get_slave_name_from_alias(struct cyc_info *, const char *);
int libcyc_kvm_get_slave_port_from_alias(struct cyc_info *, const char *);
int libcyc_kvm_session_active_check(struct cyc_info *, libcyc_list **,
				    const char *);
int libcyc_kvm_port_status_info_get(struct cyc_info *, libcyc_list **, int *);
void libcyc_kvm_port_status_info_destroy(struct cyc_info *, libcyc_list **);
const char *libcyc_kvm_slave_get_name_from_port_a(struct cyc_info *, int);
int libcyc_kvm_slave_get_num_ports_from_name(struct cyc_info *, const char *);
int libcyc_kvm_get_slave_port_from_physport_str(struct cyc_info *,
						const char *);
int libcyc_kvm_get_slave_port_a_from_physport_str(struct cyc_info *,
						  const char *);
int libcyc_kvm_get_port_active_from_alias(struct cyc_info *, const char *);
int libcyc_kvm_get_port_active_from_alias_noupdate(struct cyc_info *,
						   const char *);
int libcyc_kvm_port_event_generate(struct cyc_info *);
void libcyc_kvm_parse_kvmd_conf(struct cyc_info *, libcyc_list **);
void libcyc_kvm_parse_kvmd_slaves_conf(struct cyc_info *, libcyc_list **);
/* These are simple accessor functions to cyc->kvm; the macros are used
   by default to provide better performance  */
#ifdef CYC_KVM_REAL_FNS
enum kvm_model libcyc_kvm_get_model(struct cyc_info *);
enum kvmip_id libcyc_kvm_get_ip_modules_map(struct cyc_info *);
enum kvm_ports libcyc_kvm_get_ports(struct cyc_info *);
int libcyc_kvm_get_num_ip_modules(struct cyc_info *);
int libcyc_kvm_get_max_ip_modules(struct cyc_info *);
int libcyc_kvm_get_num_stations(struct cyc_info *);
int libcyc_kvm_get_max_fans(struct cyc_info *);
char *libcyc_kvm_get_serno(struct cyc_info *);
int libcyc_kvm_get_bd_ver(struct cyc_info *);
int libcyc_kvm_get_fpga_ver(struct cyc_info *);
#else
#define libcyc_kvm_get_model(A) A->kvm.model
#define libcyc_kvm_get_ip_modules_map(A) A->kvm.ip_module_map
#define libcyc_kvm_get_ports(A) A->kvm.ports
#define libcyc_kvm_get_num_ip_modules(A) A->kvm.num_ip_modules
#define libcyc_kvm_get_max_ip_modules(A) A->kvm.max_ip_modules
#define libcyc_kvm_get_num_stations(A) A->kvm.num_stations
#define libcyc_kvm_get_max_fans(A) A->kvm.max_fans
#define libcyc_kvm_get_bd_ver(A) A->prod.board_version
#define libcyc_kvm_get_fpga_ver(A) A->prod.fpga_version
#define libcyc_kvm_get_serno(A) A->prod.serial_number
#endif

/* KVM/netPlus functions */
enum kvmip_id libcyc_kvm_led_get_status(struct cyc_info *);
enum kvmip_id libcyc_kvm_led_set_on(struct cyc_info *, enum kvmip_id);
enum kvmip_id libcyc_kvm_led_set_off(struct cyc_info *, enum kvmip_id);
enum kvmip_id libcyc_kvm_led_set(struct cyc_info *, enum kvmip_id);

/* apctl functions */
int libcyc_apctl_get_fan_speed(struct cyc_info *, enum cyc_fanid);
int libcyc_apctl_set_alarm(struct cyc_info *, int);
int libcyc_apctl_get_alarm(struct cyc_info *);
int libcyc_apctl_set_gpled(struct cyc_info *, int);
int libcyc_apctl_get_gpled(struct cyc_info *);
int libcyc_apctl_set_hdleden(struct cyc_info *, int);
int libcyc_apctl_get_hdleden(struct cyc_info *);
int libcyc_apctl_set_cpuledorngen(struct cyc_info *, int);
int libcyc_apctl_get_cpuledorngen(struct cyc_info *);
int libcyc_apctl_get_kvm_hwinfo(struct cyc_info *, struct kvm_hwinfo *);
int libcyc_apctl_get_dual_power_cap(struct cyc_info *);
int libcyc_apctl_get_dual_power_stat(struct cyc_info *);

/* Threading functions */
pthread_t libcyc_thread_create(struct cyc_info *,
			       void *(*)(void *),void *);
int libcyc_thread_exit(struct cyc_info *);
int libcyc_thread_cancel(struct cyc_info *, pthread_t);
void *libcyc_thread_join(struct cyc_info *, pthread_t);
int libcyc_thread_detach(struct cyc_info *, pthread_t);
int libcyc_thread_kill(struct cyc_info *, pthread_t, int);
void libcyc_thread_testcancel();
pthread_mutex_t *libcyc_mutex_create(struct cyc_info *);
int libcyc_mutex_lock(struct cyc_info *, pthread_mutex_t *);
int libcyc_mutex_unlock(struct cyc_info *, pthread_mutex_t *);
int libcyc_mutex_trylock(struct cyc_info *, pthread_mutex_t *);
void libcyc_mutex_destroy(struct cyc_info *, pthread_mutex_t *);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* LIBCYC_H */
