/*
 * Module: pmc.h
 *
*/

// Strings to be sent to the user when there's any kind
// of error in startPmdConnection
#ifdef PMDCLIENT
char menuHeaderLine[] = 
"-------------------------------------------------------\r\n";
char menuHeader[] =
"Power Management Utility\r\n";
char endpmsession[] = "Exit from PM session\r\n";
char noaccesslistother[] = "\r\n"
"You cannot manage the outlet(s) entered.\r\n"
"\r\n";
char internalerror[] = "Internal Problem. Contact your Console Sever Administrator\r\n ";
char chooseAnOption[] = "Please choose an option: ";
char invalidOption[] = "Invalid Option\r\n";
char outletquestion[] = "Please enter the outlet(s) ([?] for help): ";
char outletHelp[] =
"\r\n"
"Enter the outlets you want to manage. You should have\r\n"
"    permission to access them.\r\n"
"The format of the outlet can be <IPDU ID>[<outlet number>],\r\n"
"    <outlet name>, or $<outlet group name>.\r\n"
"If <blank> is entered, you will manage the outlet(s) assigned\r\n"
"    to this server.\r\n"
"\r\n";

char pmcerrorbegin[] = "\r\n"
	"It was not possible to start a Power Management Session.\r\n";
char pmcerrorend[] = "\r\n"
	"Please contact your Console Server Administrator.\r\n";

char nooutletlist[] = "The outlet list has a wrong syntax.";
char anothersession[] = "There is another session connected in this port.";
char noserverline[] = "Could not access the server line configuration.";
char noserversess[] = "Could not access the server session information.";
char noaccess[] = "You can't access any Power Management functionality.";
char servernotready[] =	"The server is not ready. It's probably on its start process.";
char ipdunotvalid[] = "Outlet %d.%d - the serial port #%d is not configured as IPDU.";
char outletnotvalid[] = "Outlet %d.%d - the outlet is out of range.";
char noaccesslist[] = "You don't have permission to manage the outlet(s) used by this server.";

#define OPT_EXIT   1
#define OPT_HELP   2
#define OPT_ON     3
#define OPT_OFF    4
#define OPT_CYCLE  5
#define OPT_LOCK   6
#define OPT_UNLOCK 7
#define OPT_STATUS 8
#define OPT_DELAY  9
#define OPT_OTHER 10

struct _pmc_menu {
	char *menuName;
	char *commandHelp;
};

struct _pmc_menu pmcas_menu[] = {
{"Exit",  "Exits the Power Management Session"},
{"Help",  "Shows this message"},
{"On",    "Turn outlet(s) ON"},
{"Off",   "Turn outlet(s) OFF"},
#ifdef OEM3
{"Reboot", "Turn outlet(s) OFF and back ON"},
#else
{"Cycle", "Turn outlet(s) OFF and back ON"},
#endif
{"Lock",  "Lock the current status of outlet(s)"},
{"Unlock","Unlock the current status of outlet(s)"},
{"Status", "Show the current status of outlet(s)"},
{"Interval","Configures the post power on delay"},
{"Other",   "Allows the user to manage other outlets"}
};

#define NUMBER_OF_MENU_ITEMS      (sizeof(pmcas_menu)/sizeof(struct _pmc_menu))

static void pmcas_mount_menu(void);
static void pmcas_treat_option(int opt);
#else
char servernotready[] =	"\r\n"
			"It was impossible to start a Power Management Session\r\n"
			"The server is not ready. It's probably on its start process.\r\n"
			"Please contact your Console Server Administrator if this message persists.\r\n";
#endif
void pmcas_logout(CY_SESS *sess);
void pmcas_login(CY_SESS * sess, char *device, 
		void write_line(char * line,int cnt));
void pmcas_treat_input(CY_SESS * sess, void write_data(char *data,int cnt));
