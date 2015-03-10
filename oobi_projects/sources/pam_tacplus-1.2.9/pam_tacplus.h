/* pam_tacplus command line options */
#define PAM_TAC_DEBUG		01
#define PAM_TAC_ENCRYPT		02
#define PAM_TAC_FIRSTHIT	04
#define PAM_TAC_ACCT		010 /* account on all specified servers */
#define PAM_USE_FIRST_PASS	020

/* how many TACPLUS+ servers can be defined */
#define TAC_MAX_SERVERS		4
#define MAX_TAC_VERS		2

/* pam_tacplus major, minor and patchlevel version numbers */
#define PAM_TAC_VMAJ		1
#define PAM_TAC_VMIN		2
#define PAM_TAC_VPAT		9

/* default configuration file */
#define DEF_CONF_FILE	"/etc/tacplus.conf"

#define BUFFER_SIZE		1024
