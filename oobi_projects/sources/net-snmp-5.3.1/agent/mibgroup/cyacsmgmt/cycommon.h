#ifndef CYCOMMON_H
#define CYCOMMON_H
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CY_MAX_STR	100
#define CY_PCMCIA_MAX	2

struct gtable {
	int index;
	unsigned char gname[20];
	unsigned char gusers[80];
} ;

typedef struct cy_pcmcia_ident_st {
	unsigned char prod[CY_MAX_STR];
	unsigned char man[CY_MAX_STR];
	unsigned char func[CY_MAX_STR];
	unsigned char pci[CY_MAX_STR];
  }  CY_PCMCIA_IDENT;

typedef struct cy_pcmcia_conf_st {
	unsigned char power[CY_MAX_STR];
	unsigned char type[CY_MAX_STR];
	unsigned char inter[CY_MAX_STR];
	unsigned char func[CY_MAX_STR];
	unsigned char cardv[CY_MAX_STR];
	unsigned char port1[CY_MAX_STR];
	unsigned char port2[CY_MAX_STR];
  } CY_PCMCIA_CONF;

typedef struct cy_pcmcia_status_st{
	unsigned char card[CY_MAX_STR];
	unsigned char func[CY_MAX_STR];
  } CY_PCMCIA_STATUS;

/*
 * function declarations 
 */

int cy_rw_parse (char *filename, char *parse, char *buf, 
		int size, int flag_rw);

int cy_rwsp_conf (char *prefix, char *command, unsigned char *buf, 
		int size, int flag_rw);

int cy_rwsp_group(int flag_rw);

void cy_pcmcia_read(void);

#endif
