/*
 * Module: sconf.h (support for serial port configuration through hot key)
 *
 */

#ifndef SCONFMOD_H
#define SCONFMOD_H

#ifdef SCONFMOD //We have full declarations here! (keeping the style...)

char sconfmenuHeaderLine[] = 
"----------------------------------------\r\n";
char sconfmenuHeader[] =
" Physical port configuration for ";
char sconfSpeedMenuHeader[] =
" Speed configuration options";
char sconfDatasizeMenuHeader[] =
" Data size configuration options";
char sconfParityMenuHeader[] =
" Parity configuration options";
char sconfStopbitsMenuHeader[] =
" Stop bits configuration options";
char sconfFlowctrlMenuHeader[] =
" Flow control configuration options";

#define PARITY_NONE  1
#define PARITY_ODD   2
#define PARITY_EVEN  3

#define FLOW_NONE  0
#define FLOW_HARD  1
#define FLOW_SOFT  2

char notPrimaryUser[] = "\r\n"
"It was not possible to enter port configuration mode\r\n"
"because this is not the primary session to the port.\r\n";
char endsconfmode[] = "\r\nExited port configuration mode.\r\n";
char chooseOption[] = "\r\nChoose an option (ESC to cancel): ";
char chooseOptionOrRet[] = "\r\nChoose an option (ESC to return): ";
char wrongOption[] = "\r\nInvalid Option...\r\n\r\n";
char cancelOption[] = "press ESC to forget changes and exit port configuration mode\r\n";
char speedquestion[] = "\r\n  Enter the speed ([?] for help): ";
char listofspeeds[] = "\r\n"
"Valid speeds are: 300, 1200, 2400, 4800, 9600, 14400,\r\n"
"  19200, 28800, 38400, 57600, 76800, 115200, 230400\r\n";

char applyingConfig[] = "\r\nApplying current port configuration...\r\n";
char configApplied[] = "...port configuration has been applied.\r\n";
char configFileError[] = "\r\nProblems saving changes to configuration file !!!\r\n";

#define OPT_SPEED       1
#define OPT_DATASIZE    2
#define OPT_PARITY      3
#define OPT_STOPBITS    4
#define OPT_FLOWCTRL    5
#define OPT_APPLY       6

typedef enum {
	SCONFSTATE_SPEED,      // keep these values
	SCONFSTATE_DATASIZE,   // at the top because
	SCONFSTATE_PARITY,     // they are used as
	SCONFSTATE_STOPBITS,   // indexes with
	SCONFSTATE_FLOWCTRL,   // sconf_menu_values
	SCONFSTATE_NORMAL,
	SCONFSTATE_SPEED2
} _sconfstate;

struct _sconf_menu {
	char *menuName;
	char *commandHelp;
	char *pslavepar;
};
struct _sconf_menu sconf_menu[] = {
	{"speed", "changes serial port speed", "speed"},
	{"data size", "changes number of data bits", "datasize"},
	{"parity", "changes parity type", "parity"},
	{"stop bits", "changes number of stop bits", "stopbits"},
	{"flow control", "changes flow control setting", "flow"},
	{"Apply this configuration", "makes changes effective", ""}
};

struct _sconf_menu_values {
	char strval[10];
	int value;
	char pslaveitem[20];
};
struct _sconf_menu_values sconf_menu_values[5];

struct _sconf_option_menu {
	char stropt[20];
	int optval;
};

struct _sconf_option_menu sconf_speed_menu[] = {
	{"1200",   1200},
	{"2400",   2400},
	{"4800",   4800},
	{"9600",   9600},
	{"19200",  19200},
	{"38400",  38400},
	{"57600",  57600},
	{"115200", 115200},
	{"Other..."}
};

#define SCONF_OTHER_SPEED  9

struct _sconf_option_menu sconf_datasize_menu[] = {
	{"5 bits", 5},
	{"6 bits", 6},
	{"7 bits", 7},
	{"8 bits", 8}
};

struct _sconf_option_menu sconf_parity_menu[] = {
	{"none", 1},
	{"odd",  2},
	{"even", 3}
};

struct _sconf_option_menu sconf_stopbits_menu[] = {
	{"1 stop bit",  1},
	{"2 stop bits", 2}
};

struct _sconf_option_menu sconf_flowctrl_menu[] = {
	{"none",     0},
	{"harware",  1},
	{"software", 2}
};

#define SCONF_MENU_ITEMS            (sizeof(sconf_menu)/sizeof(struct _sconf_menu))

#define SCONF_SPEED_MENU_ITEMS      (sizeof(sconf_speed_menu)/sizeof(struct _sconf_option_menu))
#define SCONF_DATASIZE_MENU_ITEMS (sizeof(sconf_datasize_menu)/sizeof(struct _sconf_option_menu))
#define SCONF_PARITY_MENU_ITEMS     (sizeof(sconf_parity_menu)/sizeof(struct _sconf_option_menu))
#define SCONF_STOPBITS_MENU_ITEMS   (sizeof(sconf_stopbits_menu)/sizeof(struct _sconf_option_menu))
#define SCONF_FLOWCTRL_MENU_ITEMS   (sizeof(sconf_flowctrl_menu)/sizeof(struct _sconf_option_menu))

static void sconf_mount_menu(struct line_cfg *pconf);
static void sconf_mount_speed_menu();
static void sconf_mount_secondary_menu(char *menuheader, int num_of_items, struct _sconf_option_menu *menulist, int startno);

static void sconf_treat_option(int opt, CY_SESS *sess, struct line_cfg *pconf);

#else //here only the external API

void sconf_login(CY_SESS* sess, struct line_cfg *pconf, void write_data(char * line, int cnt));
void sconf_treat_input(CY_SESS * sess, struct line_cfg *pconf, void write_data(char *data, int cnt));
void sconf_logout(CY_SESS *sess);

int apply_newconfig(int pfd, struct line_cfg *pconf);

#endif //SCONFMOD

#endif //SCONFMOD_H
