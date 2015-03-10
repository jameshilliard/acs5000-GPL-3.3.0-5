#ifndef __uboot_def_h
#define __uboot_def_h

#define DEFAULT_WDT_OPT 1

typedef struct _NAMEVAL_LIST {
        char *name;
        char *value;
} NAMEVAL_LIST;

typedef struct _HASH_LIST {
        char *name;
        int value;
} HASH_LIST;

char yes_no           [] = {'Y', 'Y', 'N', 'N', 0xff, 0xff};
char save_op          [] = {'N', 'N', 'Y', 'Y', 'Q', 'Q', 0xff, 0xff};
char img_op           [] = {'1', '1', '2', '2', 0xff, 0xff};
char wdt_op           [] = {'A', 1, 'I', 0, 0xff, 0xff};
char boot_op          [] = {'F', 0, 'N', 1, 0xff, 0xff};
char mii_op           [] = {'A', 0, '1', 1, 'T', 2, 'F', 3, 'H', 4, 0xff, 0xff};

NAMEVAL_LIST NameValListDefault [] = {
{ "bootcmd", "hw_boot"},
{ "baudrate", "9600"},
{ "wdt", "on"},
{ "fecmode", "auto"},
{ "ethaddr", "00:60:2e:00:00:01"},
{ "bootfile", DEFAULT_TFTP_PATH},
{ "ipaddr", "192.168.160.10"},
{ "serverip", "192.168.160.1"},
{ "MaxIntEvents", "0"},
{ "currentimage", "1"},
{ NULL }
};

#define         BOOTCMD         0
#define         BAUDRATE        1
#define         WDT             2
#define         FECMODE         3
#define         ETHADDR         4
#define         BOOTFILE        5
#define         IPADDR          6
#define         SERVERIP        7
#define         MAXINT          8
#define         CURR_IMG        9
#define         NUM_VARIABLES   10

HASH_LIST CmdValList [] = {
{"bootcmd", BOOTCMD},
{"baudrate", BAUDRATE},
{"wdt", WDT},
{"fecmode", FECMODE},
{"ethaddr", ETHADDR},
{"bootfile", BOOTFILE},
{"ipaddr", IPADDR},
{"serverip", SERVERIP},
{"MaxIntEvents", MAXINT},
{"currentimage", CURR_IMG},
{NULL}
};

HASH_LIST WdtList [] = {
{"off", 'I'},
{"on", 'A'},
{NULL}
};

HASH_LIST BootcmdList [] = {
{"net_boot", 'N'},
{"hw_boot", 'F'},
{NULL}
};

HASH_LIST FecmodeList [] = {
{"auto", 'A'},
{"100H", '1'},
{"100F", 'F'},
{"10F", 'T'},
{"10H", 'H'},
{NULL}
};

HASH_LIST BaudrateList [] = {
{"4800", 0},
{"9600", 1},
{"19200", 2},
{"38400", 3},
{"57600", 4},
{"115200", 5},
{NULL}
};

#endif	/* __uboot_def_h */
