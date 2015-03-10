#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include <asm/cyc_config.h>
#include <asm/flash.h>
#include <asm/prxk.h>
#include "apcdisc.h"
#include "cystrings.h"

oid apcSizeTable_oid[] = { PROD_DISC_OID };
oid apcInfoTable_oid[] = { PROD_DISC_OID, 2 , 1};
oid apcFwTable_oid[] = { PROD_DISC_OID, 4 , 1};
oid apcProtocolTable_oid[] = { PROD_DISC_OID, 6, 1 };

#define APCINDEX	1

struct variable4 apcSizeTable_variables[] = {
#define   APCINFOTABLE          1
  { APCINFOTABLE        , ASN_INTEGER , RONLY , var_apcSizeTable, 1, { 1 } },
#define   APCFWTABLE            2
  { APCFWTABLE          , ASN_INTEGER , RONLY , var_apcSizeTable, 1, { 3 } },
#define   APCPROTOTABLE         3
  { APCPROTOTABLE       , ASN_INTEGER , RONLY , var_apcSizeTable, 1, { 5 } },
};

struct variable4 apcInfoTable_variables[] = {
  { APCINDEX		, ASN_INTEGER , RONLY , var_apcInfo, 1, { 1 } },
#define   APCMODEL            2
  { APCMODEL		, ASN_OCTET_STR , RONLY , var_apcInfo, 1, { 2 } },
#define   APCSERIALNUMBER     3
  { APCSERIALNUMBER	, ASN_OCTET_STR , RONLY , var_apcInfo, 1, { 3 } },
#define   APCSTATUS	     4
  { APCSTATUS		, ASN_INTEGER , RONLY , var_apcInfo, 1, { 4 } },
#define   APCLABEL	5
 { APCLABEL		, ASN_OCTET_STR , RONLY , var_apcInfo, 1, { 5 } },
};

#define APCFW_INDEX_OID	PROD_DISC_OID_SIZE+3
struct variable4 apcFwTable_variables[] = {
  { APCINDEX		, ASN_INTEGER , RONLY , var_apcFw, 1, { 1 } },
#define   APCFWSERIAL            2
  { APCFWSERIAL		, ASN_OCTET_STR , RONLY , var_apcFw, 1, { 2 } },
#define   APCFWNAME		 3
  { APCFWNAME		, ASN_OCTET_STR , RONLY , var_apcFw, 1, { 3 } },
#define   APCFWREV	4
  { APCFWREV		, ASN_OCTET_STR , RONLY , var_apcFw, 1, { 4 } },
};

#define APCPROTOCOL_INDEX_OID	PROD_DISC_OID_SIZE+3

struct variable4 apcProtocolTable_variables[] = {
	{ APCINDEX	, ASN_INTEGER , RONLY , var_apcProtoTable, 1, { 1 } },
#define APCPROTONUMBER	2
	{APCPROTONUMBER	, ASN_INTEGER, RONLY, var_apcProtoTable, 1, { 2}},
#define APCPROTOVERSION 3
	{APCPROTOVERSION , ASN_OCTET_STR, RONLY, var_apcProtoTable, 1, { 3}},
#define APCPROTOPORT	4
	{APCPROTOPORT	, ASN_OCTET_STR, RONLY, var_apcProtoTable, 1, { 4}},
#define APCPROTOSTATE 	5
	{ APCPROTOSTATE, ASN_INTEGER, RONLY, var_apcProtoTable, 1, { 5}},
};

char apc_model[21];
char apc_label[21];
char apc_serialnumber[41];
char apc_fwname[41];
char apc_fwversion[41];
char apc_boardtype;

#define NUMPROTO 9
struct {
	int number;
	char version[5];
	char port[10];
	int state;
	} protoTable[NUMPROTO] = 
{ { 1,  "1" , "MTYx", 2 }, // SNMP v 1
  { 1,  "2" , "MTYx", 1 }, // SNMP v 2
  { 1,  "3" , "MTYx", 1 }, // SNMP v 3
  { 2,  "0" , "MjM=", 2 }, // telnet
  { 3,  "0" , "ODA=", 2 }, // HTTP
  { 6,  "0" , "NDQz", 2 }, // HTTPS
  { 7,  "1" , "MjI=", 2 }, // SSH v1
  { 7,  "2" , "MjI=", 2 }, // SSH v2
  { 8,  "0" , "MjI=", 2 }, // SCP
};

void apc_read_info(void)
{
        char buffer[256],*s1;
        int fd, num;

	memset(apc_model,0,sizeof(apc_model));
	memset(apc_label,0,sizeof(apc_label));
	memset(apc_serialnumber,0,sizeof(apc_serialnumber));
	memset(apc_fwname,0,sizeof(apc_fwname));
	memset(apc_fwversion,0,sizeof(apc_fwversion));

        if ((fd = open("/proc/bdinfo", O_RDONLY)) > 0) {

                read(fd, buffer, sizeof(buffer));
                close(fd);

                s1 = strtok(buffer, "!"); // board name
                strncpy(apc_model, s1, sizeof(apc_model)-1);
		s1 = strtok(NULL, "!"); // number of ports
		s1 = strtok(NULL,"!"); // card name
		s1 = strtok(NULL,"!"); // board type
		apc_boardtype = atoi(s1);
		s1 = strtok(NULL,"!"); // vendor name
		strncpy(apc_label, s1, sizeof(apc_label)-1);
		s1 = strtok(NULL,"!"); // prod line
		s1 = strtok(NULL,"!"); // firmware name 
		strncat(apc_label, s1, sizeof(apc_label) - strlen(apc_label)-1);
		strncpy(apc_fwname,s1, sizeof(apc_fwname)-1);
		s1 = strtok(NULL,"!"); // pcmcia
		s1 = strtok(NULL,"!"); // firmware version
		strncpy(apc_fwversion,s1, sizeof(apc_fwversion)-1);
		s1 = strtok(NULL,"!"); // clock freq 
		s1 = strtok(NULL,"!"); // memory size
		s1 = strtok(NULL,"!"); // flash size
		s1 = strtok(NULL,"!"); // device ID 
		s1 = strtok(NULL,"!"); // fpga version
		s1 = strtok(NULL,"!"); // ipboard 1 
		s1 = strtok(NULL,"!"); // ipboard 2 
		s1 = strtok(NULL,"!"); // mac address 
		strncpy(apc_serialnumber,s1,sizeof(apc_serialnumber)-1);
	}

	if (apc_boardtype >= BOARD_KVM16 && apc_boardtype <= BOARD_KVMNET32) {
		return;
	}

        if ((fd = open("/etc/hostname", O_RDONLY)) > 0) {
                read(fd, buffer, sizeof(apc_label));
                close(fd);
		s1 = buffer;
		while (*s1 && isalpha(*s1)) s1++;
		if (s1) *s1 = 0x00;
		strncpy(apc_label, buffer, sizeof(apc_label)-1);	
		close(fd);
	}
}

#define TEMPPROTO "/tmp/temproto"

void apc_state_proto(int index) 
{
	unsigned char cmd[100],*s1;
	static struct stat st;
        int fd, i,j;

	switch (protoTable[index].number) {
		case 1: // SNMP
			if (!strcmp(protoTable[index].version ,"3")) {
				strcpy(cmd,"grep user /etc/snmp/snmpd.conf | grep -v ^#");
			} else { // v3
				strcpy(cmd,"grep community /etc/snmp/snmpd.conf | grep -v ^#");	
			}
			break;

		case 2: // telnet
			strcpy(cmd,"grep ^telnet /etc/inetd.conf");
			break;

		case 3: // HTTP
			strcpy(cmd,"grep ^HTTP=YES /etc/webui.conf");
			break;

		case 6: // HTTPS
			strcpy(cmd,"grep ^HTTPS=YES /etc/webui.conf");
			break;

		case 7: // SSH
			strcpy(cmd,"grep ^ENABLE=YES /etc/sshd.sh");
			break;

		case 8: // SCP
			strcpy(cmd,"grep ^ENABLE=YES /etc/sshd.sh");
			break;
	
		default:
			return;
	}
	strcat (cmd, ">"TEMPPROTO);

	system(cmd);

	if (stat(TEMPPROTO, &st)) {
		protoTable[index].state = 1; // disable
	} else {
		if (st.st_size == 0) {
			protoTable[index].state = 1; // disabled
		} else {
			if (protoTable[index].number == 7) { //SSH
				// version v1, v2
				strcpy(cmd,"grep ^Protocol /etc/ssh/sshd_config");
				strcat (cmd, ">"TEMPPROTO);
				system(cmd);
				if (stat(TEMPPROTO, &st) || (st.st_size == 0)) {
					protoTable[index].state = 2; // enabled
				} else {
        				if ((fd = open(TEMPPROTO, O_RDONLY)) > 0) {
                				read(fd, cmd, sizeof(cmd));
                				close(fd);
						s1 = cmd;
						while (*s1 && !isdigit(*s1)) s1++;
						i = *s1++ - 0x30;
						if (*s1 == ',') {
							s1++;
							j = *s1 - 0x30;
						}
						if (strcmp(protoTable[index].version,"1")) {
							// SSH V2
							if ((i == 2) || (j == 2)) {
								protoTable[index].state = 2;
							} else {
								protoTable[index].state = 1; 
							}
						} else {
							// SSH V1
							if ((i == 1) || (j == 1)) {
								protoTable[index].state = 2;
							} else {
								protoTable[index].state = 1; 
							}
						}
					}
				}
			} else {
				protoTable[index].state = 2; // enabled
			}
		}
	}	
	strcpy (cmd, "rm "TEMPPROTO);
	system(cmd);
}

static const char Base64[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char Pad64 = '=';

int b64_ntop(u_char const *src, size_t srclength, char *target, size_t targsize)
{
	size_t datalength = 0;
	u_char input[3];
	u_char output[4];
	int i;

	while (2 < srclength) {
		input[0] = *src++;
		input[1] = *src++;
		input[2] = *src++;
		srclength -= 3;

		output[0] = input[0] >> 2;
		output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
		output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
		output[3] = input[2] & 0x3f;

		if (datalength + 4 > targsize)
			return (-1);
		target[datalength++] = Base64[output[0]];
		target[datalength++] = Base64[output[1]];
		target[datalength++] = Base64[output[2]];
		target[datalength++] = Base64[output[3]];
	}
    
	/* Now we worry about padding. */
	if (0 != srclength) {
		/* Get what's left. */
		input[0] = input[1] = input[2] = '\0';
		for (i = 0; i < srclength; i++)
			input[i] = *src++;
	
		output[0] = input[0] >> 2;
		output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
		output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);

		if (datalength + 4 > targsize)
			return (-1);
		target[datalength++] = Base64[output[0]];
		target[datalength++] = Base64[output[1]];
		if (srclength == 1)
			target[datalength++] = Pad64;
		else
			target[datalength++] = Base64[output[2]];
		target[datalength++] = Pad64;
	}
	if (datalength >= targsize)
		return (-1);
	target[datalength] = '\0';	/* Returned value doesn't count \0. */
	return (datalength);
}

void apc_port_proto(int index) 
{
	unsigned char cmd[100],*s1,*s2;
	static struct stat st;
        int fd, i,j;

	switch (protoTable[index].number) {
		case 2: // telnet
			strcpy(cmd,"grep ^telnet /etc/services");
			break;
	
		case 3: // HTTP
			strcpy(cmd,"grep ^HTTP_PORT /etc/webui.conf");
			break;

		case 6: // HTTPS
			strcpy(cmd,"grep ^HTTPS_PORT /etc/webui.conf");
			break;

		default :
			return;
	}
	strcat (cmd, ">"TEMPPROTO);

	system(cmd);

	if (stat(TEMPPROTO, &st) || (st.st_size == 0)) {
		return;
	}
       	if ((fd = open(TEMPPROTO, O_RDONLY)) > 0) {
       		read(fd, cmd, sizeof(cmd));
       		close(fd);
		s1 = cmd;
		memset(protoTable[index].port,0,10);
		while (*s1 && !isdigit(*s1)) s1++;
		if (!*s1) { // does not have digit
			switch (protoTable[index].number) {
				case 2: // telnet = def 23
  					strcpy(protoTable[index].port,"MjM=");
					break;
				case 3: // HTTP = def 80
  					strcpy(protoTable[index].port,"ODA=");
					break;
				case 6: // HTTPS = def 443
  					strcpy(protoTable[index].port,"NDQz");
					break;
			}	
			return;
		}
		s2 = s1;
		while (*s1 && isdigit(*s1)) s1++;
		if (*s1) *s1=0x00;
		b64_ntop(s2,strlen(s2),protoTable[index].port,10);	
	}
	strcpy (cmd, "rm "TEMPPROTO);
	system(cmd);
}

void init_apcdisc(void) 
{
	DEBUGMSGTL(("apc","Initializing\n"));

	apc_read_info();

	REGISTER_MIB("apcSizeTable", apcSizeTable_variables, variable4, apcSizeTable_oid);

	REGISTER_MIB("apcInfoTable", apcInfoTable_variables, variable4, apcInfoTable_oid);
	
	REGISTER_MIB("apcFWTable", apcFwTable_variables, variable4, apcFwTable_oid);
	
	REGISTER_MIB("apcProtocolTable", apcProtocolTable_variables, variable4, apcProtocolTable_oid);
}

unsigned char *var_apcSizeTable(struct variable *vp,
			    oid * name,
			    size_t * length,
			    int exact, size_t * var_len,
			    WriteMethod ** write_method)
{
	static int int_val;

	if (header_generic(vp, name, length, exact, var_len, write_method)
	    == MATCH_FAILED)
		return NULL;

	*write_method = NULL;
	*var_len = sizeof(int);

	switch (vp->magic) {
		case APCINFOTABLE :
			int_val = 1;
                	return (unsigned char *) &int_val;
		case APCFWTABLE   :      
			int_val = DEVICE_TABLE_SIZE;
                	return (unsigned char *) &int_val;
		case APCPROTOTABLE :
			int_val = NUMPROTO;
                	return (unsigned char *) &int_val;
		default :
	}
	return NULL;
}

unsigned char *var_apcInfo(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	static int int_val;
	static unsigned char string[200];

	// find the index
	if (header_simple_table
		(vp, name, length, exact, var_len, write_method, 1)
		== MATCH_FAILED)
		return NULL;

	*write_method = NULL;
	*var_len = sizeof(int);

	switch (vp->magic) {
		case APCINDEX :
			int_val = 1;
                	return (unsigned char *) &int_val;

		case APCMODEL :
			*var_len = strlen(apc_model);
			return (u_char *) & apc_model;
		case APCSERIALNUMBER :
			*var_len = strlen(apc_serialnumber);
			return (u_char *) & apc_serialnumber;
		case APCSTATUS :	
			int_val = 2;
			return (u_char *) &int_val;
		case APCLABEL :
			*var_len = strlen(apc_label);
			return (u_char *) & apc_label;
		default :
	}
	return NULL;

}

unsigned char *var_apcFw(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	static int int_val;
	static unsigned char string[200];
	int index;

	// find the index
	if (header_simple_table
		(vp, name, length, exact, var_len, write_method, DEVICE_TABLE_SIZE)
		== MATCH_FAILED)
		return NULL;

	*write_method = NULL;
	*var_len = sizeof(int);

	index = name[APCFW_INDEX_OID] - 1;

	switch (vp->magic) {
		case APCINDEX :
			int_val = index + 1;
                	return (unsigned char *) &int_val;

		case APCFWSERIAL :
			*var_len = strlen(apc_serialnumber);
			return (u_char *) & apc_serialnumber;
		case APCFWNAME :	
			if (index) {
				*var_len = strlen(apc_fwname);
				return (u_char *) & apc_fwname;
			} else {
				*var_len = strlen(apc_fwname);
				return (u_char *) & apc_fwname;
			}
		case APCFWREV :
			if (index) {
				*var_len = strlen(apc_fwversion);
				return (u_char *) & apc_fwversion;
			} else {
				*var_len = strlen(apc_fwversion);
				return (u_char *) & apc_fwversion;
			}
		default :
	}
	return NULL;
}

unsigned char *var_apcProtoTable(struct variable *vp,
	oid * name,
	size_t * length,
	int exact,
	size_t * var_len,
	WriteMethod ** write_method)
{
	static long int_val;
	static unsigned char strval[10];
	int index;

	// find the index
	if (header_simple_table
		(vp, name, length, exact, var_len, write_method, NUMPROTO)
		== MATCH_FAILED)
		return NULL;

	index = name[APCPROTOCOL_INDEX_OID] - 1;

	switch (vp->magic) {
		case APCINDEX :
			int_val = index+1;
                	return (unsigned char *) &int_val;

		case APCPROTONUMBER :
			int_val = protoTable[index].number;
                	return (unsigned char *) &int_val;

		case APCPROTOVERSION :
			strcpy(strval,protoTable[index].version);
			*var_len = strlen(strval);
			return (u_char *) & strval;

		case APCPROTOPORT :
			apc_port_proto(index);
			strcpy(strval,protoTable[index].port);
			*var_len = strlen(strval);
			return (u_char *) & strval;

		case APCPROTOSTATE :
			apc_state_proto(index);
			int_val = protoTable[index].state;
			return (u_char *) &int_val;

		default :
			break;
	}
	return NULL;
}
