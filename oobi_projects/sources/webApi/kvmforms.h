#define	IDX_CYCLE		-1	/* Code to cycle through the ports */
#define	IDX_EXIT		-2	/* Code to exit the user session */
#define IDX_NEXT		-3
#define	IDX_PREV		-4
#define	IDX_SAVE		-5
#define	IDX_CANCEL		-6
#define	IDX_USERCONFIG		-7
#define	IDX_GENCONFIG		-8
#define	IDX_STATIONCONFIG	-9
#define	IDX_ACCLIST		-10
#define	IDX_SAVECONFIG		-11
#define IDX_MPORTS		-12
#define IDX_SLVPORTS		-13
#define IDX_PORTCONFIG		-14
#define IDX_LOGOUT		-15
#define IDX_QUIT		-16
#define IDX_CONNRO		-17
#define IDX_CONNRW		-18
#define IDX_CONNRTRO		-19
#define IDX_AUXPORT		-20
#define IDX_USERDBASE		-21
#define IDX_RESETPORTS		-22
#define IDX_AUTHTYPECONFIG	-23
#ifdef KRBenable
#define IDX_KRB			-24
#endif
#define IDX_LDAP		-25
#define IDX_RADIUS		-26
#define IDX_TACPLUS		-27
#ifdef NISenable
#define IDX_NIS			-28
#endif
#ifdef SMBenable
#define IDX_SMB			-29
#endif
#define IDX_SERIALPORTCONFIG    -30
#define IDX_ALLPORTS            -31
#define IDX_EDIT		-32
#define IDX_DELETE		-33
#define IDX_DEFAULT_ACCESS	-34

#define DEFAULT_ACCESS		0
#define SINGLE_PORT		1
#define MULTI_PORT		2

#define EMPTY 0
#define COMMA 1
#define MINUS 2
#define NUMBR 3

#define CENTER(s)	((24 - strlen(s))/2)

enum {FORM_NONE = 0,
	FORM_LOGIN, FORM_MAINMENU, FORM_CONNMENU, FORM_CONFMENU, FORM_POWERMENU,
	FORM_SNIFF, FORM_MESSAGE, FORM_SEQCFG, FORM_GENCFG, FORM_STATCFG,
	FORM_TERMCFG, FORM_USERCFG, FORM_USERADD, FORM_USERLIST, FORM_USERADMIN,
	FORM_PORTLIST, FORM_PERM, FORM_MPORTS, FORM_VIDCONF, FORM_SLVCFG, 
	FORM_PORTCONFMENU, FORM_WARN, FORM_SLVADD, FORM_SLVTYPECFG,
	FORM_SLVPORTLIST, FORM_SLVMODELCFG, FORM_SLAVELIST, FORM_RESYNC, 
	FORM_USERDBASEOPT, FORM_USERDBASEADD, FORM_USERDBASELIST, 
	FORM_USERDBASEPWD1, FORM_USERDBASEPWD2, FORM_AUXPORTOPT, 
	FORM_AUXPORTSOCK, FORM_AUXPORTSPEED, FORM_SAVELOAD, FORM_USERGROUP, 
	FORM_INFO, FORM_AUTH, 
#ifdef KRBenable
	FORM_KRB, 
#endif
	FORM_LDAP, FORM_RADIUS,
	FORM_TACPLUS, 
#ifdef NISenable
	FORM_NIS, 
#endif
#ifdef SMBenable
	FORM_SMB, 
#endif
	FORM_FTP, FORM_NETWORK, FORM_HOSTS,
	FORM_ROUTES, FORM_OSDMESSAGE, FORM_NTP, FORM_PCMCIA, FORM_SYSLOG,
	FORM_PRNTSCR, FORM_RGBMAIN, FORM_RGBSET};

void kvm_name_port(unsigned char, unsigned char);
void osd_login_create_screen(unsigned char);
void free_portmenu(struct _kvm_setting *pkvm);
void assemble_connport_menu(unsigned char);
void osd_confmenu_create_screen(unsigned char);
void osd_connmenu_create_screen(unsigned char);
void osd_mainmenu_create_screen(unsigned char);
void osd_power_create_screen(unsigned char);
void osd_sniff_connro(unsigned char);
void osd_sniff_connrw(unsigned char);
void osd_sniff_create_screen(unsigned char, unsigned char);
void osd_message_create_screen(unsigned char, char *, void (*)(unsigned char));
void osd_info_create_screen(unsigned char, unsigned char *, unsigned char *, 
	int, void (*)(unsigned char));
void osd_back_connect(unsigned char);
int pmGetStatus (unsigned char, unsigned short); 
int pmGetAllStatus (unsigned char, unsigned char, char, unsigned char); 
void pmSendCommand (unsigned char, int, unsigned char, unsigned char, 
	unsigned char *);
void insert_power_management_status (unsigned char, unsigned char,
	unsigned char);
void insert_power_info (unsigned char, int);
//void insert_error_info (unsigned char, int, unsigned char *);
void get_next_device(unsigned char, int);
void check_power_management (unsigned char);
void check_video_config (unsigned char);
void check_resync (unsigned char);
void resync_server(int);
void kvm_slave_check (unsigned char);
void osd_check_ro(unsigned char);
void osd_conn_ro(unsigned char, unsigned char);
void osd_pm_create_screen(unsigned char);
void assemble_pm_menu (unsigned char);
void osd_rgbadj_create_screen(unsigned char);
