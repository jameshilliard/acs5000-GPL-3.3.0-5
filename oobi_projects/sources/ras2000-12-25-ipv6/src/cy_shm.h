#ifndef CY_SHM_H
#define CY_SHM_H 1

#define lineconf cy_shm->line_conf
#define mainconf cy_shm->main_conf
#define allconf  cy_shm->all_conf

#define SHM_PERM 0666

#define CY_ID_MSG 0xFACDAC00
#define CY_ID_SHM 0xFACBADEC
#define CY_ID_SEM 0xFACBADEF
#define CY_ADDR_SHM	0x3FF00000	// All proccess must attach at this address !!

/* The first 64 semaphores are used by the tty ports */
#define CY_SEM_CONFIG		64
#define CY_SEM_SHM			65
#define CY_SEM_NFS			66
#define CY_SEM_CNF_SESSION	67

#define CY_MAX_SEM 68

#define CY_SESS_FLAG_NONE           0x0000     //None
#define CY_SESS_FLAG_W              0x0100     //Write session
#define CY_SESS_FLAG_R              0x0200     //Read  session
#define CY_SESS_FLAG_RW             0x0300     //Read/Write session
#define CY_SESS_FLAG_MENU           0x0400     //Show menu for this session
#define CY_SESS_FLAG_DBMENU         0x0800     //Showing data buffring
#define CY_SESS_FLAG_PMD            0x1000     // used by the pmd server
#define CY_SESS_FLAG_MUNTF_NO       0x0000     // no multiuser notification due
#define CY_SESS_FLAG_MUNTF_BIT      0x2000     // multiuser notification bit
#define CY_SESS_FLAG_MUNTF_IN       0x2000     // warn about new user connection
#define CY_SESS_FLAG_MUNTF_OUT      0x6000     // warn about user disconnection
#define CY_SESS_FLAG_MUNTF_OUT_BIT  0x4000     // user disconnection bit
#define CY_SESS_FLAG_MUNTF_M        0x6000     // Multiuser Notification mask
#define CY_SESS_FLAG_DB_USRLOG      0x8000     // Data Buffer User Logs
#define CY_SESS_FLAG_IPMI           0x10000
#define CY_SESS_FLAG_SCONF          0x40000
#define CY_SESS_FLAG_SCONF_APPLY    0x80000
#define CY_SESS_FLAG_MASK           0x0003ff00 //Mask
#define CY_SESS_FLAG_SSHD           0x20000    // session opened by ssh connection

#define DB_STATUS_RESET				0x00
#define DB_STATUS_RD_ACTIVE			0x01
#define DB_STATUS_NFS_ACTIVE		0x02
#define DB_STATUS_NFS_CHANGED		0x04
#define DB_STATUS_FAC_CHANGED		0x08
#define DB_STATUS_NFS_ERR			0x10

// This structure hold the Power Management Client Information.

#define MAXPMDACCESS	100
#define MAXOutletsPS	129
#define	MAX_PMC_MENU	512
#define	PMC_CMDSIZE	64
#define	PMC_OUTLETS	128	
// pmc_status
#define PMC_IDLE	0
#define PMC_REQUEST	1
#define PMC_WAITANS	2
#define PMC_ANSOK	3
#define PMC_UPGRADE	4
#define PMC_UPGRADE_END 5
#define PMC_PASSWD	6
#define PMC_PASSWDOK	7
#define PMD_STATUS	10
#define PMD_NOAUTH	0x80000000

// upgrade : erro code
#define PMC_UPG_OK	0
#define PMC_UPG_NAK	1
#define PMC_UPG_CAN	2
#define PMC_UPG_ERROR	3
#define PMC_UPG_CRC	4

struct Tonqueue{
	int number; //number of commands in the queue
	char queue[50][100]; //queue for multiple on commands
	char username[20];
	int orig_status; // command received from the application
	int index;          //next command
}; //type for "on" queue (queue that will keep on commands)


struct _pmcInfo {
 	int idx_pmclist;	// index in pmc_list
	int pmc_ppid;
	int pmc_pid;		// the requester
 	int pmc_ipdu;		// port number of the IPDU
	int pmc_aux;			// 1 if the port is dedicated aux port
	char pmc_username[31];	// user that access this
	char pmc_outlet[PMC_OUTLETS]; // outlets 
 	int pmc_status;		 
	char pmc_cmd[PMC_CMDSIZE];
	char pmc_menu[MAX_PMC_MENU];
	int pmc_sizeAnswerBuf;	// max size of pmc_answerBuf
	char *pmc_answerBuf;	//pmd_to_client
	struct _pmcInfo	*next;		// used to CAS with two outlets
	unsigned char iscycle; //signs that a cycle was changed into off and ons
};

typedef struct _pmcInfo pmcInfo;

struct CY_SESSION_LIST {
	char   layout;	// 0-sess, 1-ipdu
	char   id;
	short  port;
	short  flow_flag;
	pid_t  vet_pid;
	char   *tty_user;
	unsigned long time;	//[RK]Nov/26/02 - W-CAS
	int   sendbreak;	//[RK]May/05/03 - break_interval
	int  sess_flag;		/* Increased to int for more space [EL] */
	pmcInfo *pmc;
	BufferType InBuffer;	//tcp_to_tty
	BufferType OutBuffer;	//tty_to_tcp
	char   *user_away;              //mp[Nov2003] DB user logs
	unsigned long time_user_left;	//mp[Nov2003] DB user logs
	struct CY_SESSION_LIST *next;
	struct CY_SESSION_LIST *prev;
};

typedef struct CY_SESSION_LIST CY_SESS;

typedef enum {
	CNF_APPL_NONE,
	CNF_APPL_CLI,
	CNF_APPL_WEB,
	CNF_APPL_OSD,
} CY_CNF_APPL;


typedef struct _CY_CONFIG_SESSION {
	CY_CNF_APPL		appl;
	int				sid;
	int				pid;
	int				new_pid;
	char			username[32];
} CY_CONFIG_SESSION;

struct pam_pwd_ctrl {
	pid_t pid;
	char  passwd[64];
	char  *group_name;
	struct pam_pwd_ctrl *next;
};

struct CY_SHARED_MEMORY {
	int id_shm;
	int sem_id;
	char max_devS;
	char max_devA;
	char max_devK;
	char max_devM;
	int	st_cy_ras;
	int	DB_pid;
	int web_date_set;
	int web_start_time;
	int	DB_pid_poll;
	int	DB_status;
	char   DB_path[32];
	int 	PMD_pid;        // This is the PMD server pid
	int PMD_outlets[MAXLINES];
	int PMD_pms[MAXLINES];
	CY_CONFIG_SESSION	cnf_session;
	struct CY_SESSION_LIST *sess_list[MAXVLINES];
	pmcInfo	*pmc_list[MAXPMDACCESS];
	pid_t vet_pid[MAXVLINES];	
	pid_t vet_pool[MAXVLINES];
	char vet_chg[MAXVLINES];
	char vet_ppp[MAXLINES];
	char flag_dcd[MAXLINES];
	char dcd_status[MAXLINES];
	char st_data_buffering[MAXLINES];
	char *tty_user[MAXLINES];
	unsigned long st_time[MAXLINES];
	struct main_cfg main_conf;
	struct line_cfg line_conf[MAXVLINES];
	struct line_cfg all_conf;
	char   *alloc_str;
	char   *free_str;
	struct pam_pwd_ctrl *pam_passwd; //[MP]Aug/2004
	struct bonding_cfg bconfig;  //[RK]Feb/23/06
	char   strings[1]; //[RK]Nov/19/2003 - Dynamic allocation
};

typedef struct CY_SHARED_MEMORY CY_SHM;

#ifdef MAIN_SHM
CY_SHM *cy_shm = (CY_SHM *)-1;
#else
extern CY_SHM *cy_shm;
#endif

#ifdef ONS
#define AUX_OFFSET	49
#else
#define AUX_OFFSET	0
#endif	

#endif
