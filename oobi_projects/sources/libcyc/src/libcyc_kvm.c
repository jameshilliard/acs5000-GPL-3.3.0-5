/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_kvm.c
 *
 * Copyright (C) 2004-2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_kvm
 *
 *****************************************************************
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ctype.h>

#include <cyc.h>
#include <cy_kvmip_led.h>
#include <cy_apctl.h>
#include <names.h>

static char *parse_netconf_runfile(struct cyc_info *, int);
static void parse_username(struct cyc_info *, char *, int, void *);
static void parse_w(struct cyc_info *, char *, int, void *);
static void parse_kvmstat(struct cyc_info *, char *, int, void *);
static int get_port_from_alias_list(libcyc_list **, const char *);
static const char *get_alias_from_alias_list(libcyc_list **, int);
static void parse_outlet_str(struct cyc_info *, libcyc_list **, const char *);
static void parse_power_status_str(struct cyc_info *, libcyc_list **,
				   const char *);
static void parse_pmcommand_outlet(struct cyc_info *, char *, int, void *);
static int get_pmcommand_outlet_info(struct cyc_info *, libcyc_list **, int);
static int get_active_from_alias_list(libcyc_list **, const char *);

/* The order here must be kept in sync with enum kvm_model */
static const char *KVM_MODEL[] = {
	NAME_KVMANALOG,
	NAME_KVMNET,
	NAME_ONS,
	NAME_KVMPLUS,
	NAME_KVMNET_4000,
	NAME_KVMMAX,
	"KVM-over-IP PC Simulation",
};

/* The order here must be kept in sync with enum kvm_model */
static const char *KVM_MODEL_SHORT[] = {
	NAME_SHORT_KVMANALOG,
	NAME_SHORT_KVMNET,
	NAME_SHORT_ONS,
	NAME_SHORT_KVMPLUS,
	NAME_SHORT_KVMNET_4000,
	NAME_SHORT_KVMMAX,
	"KVM-over-IP PC Simulation",
};

static const int kvmip_id_lookup[] = { 0, CARD_1, CARD_2, CARD_3, CARD_4, 0 };

enum sess_parse_state {
	SESS_STATE_PARSE_ERROR,
	SESS_STATE_PARSE_KVM,
	SESS_STATE_PARSE_READY,
};

struct sess_parse_info {
	libcyc_list **slist;
	enum sess_parse_state state;
};

enum pmcommand_parse_state {
	PMC_STATE_PARSE_ERROR,
	PMC_STATE_PARSE_READY,
};

struct pmcommand_parse_info {
	libcyc_list **plist;
	enum pmcommand_parse_state state;
	int outlet;
};

static char *parse_netconf_runfile(struct cyc_info *cyc, int id)
{
	FILE *fp = NULL;
	char conf_str[MAX_STR_LEN];
	char *tmp_ip=NULL,*next=NULL;
	char *ip_str = NULL;
	int i=0;

	if ((cyc == NULL) || (id == ERROR)) {
		return(NULL);
	}

	if ((fp=fopen(NETCONF_RUNFILE, "r")) == NULL) {
		perror("read_netconf_runfile");
		return(NULL);
	}
	fread(conf_str, MAX_STR_LEN, 1, fp);
	fclose(fp);

	conf_str[MAX_STR_LEN-1] = '\0';
	
	for (tmp_ip = conf_str ;
	     (next = strchr(tmp_ip, NETCONF_RUNFILE_DELIMITER)) ;
	     tmp_ip = next + 1) {
		*next='\0';
		if (id == i++) {
			ip_str = libcyc_strdup(cyc, tmp_ip);
			break;
		}
	}

	return(ip_str);
}

const char *libcyc_kvm_get_model_str(struct cyc_info *cyc)
{
	enum kvm_model model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	model = libcyc_kvm_get_model(cyc);

        if (model != ERROR) return(KVM_MODEL[model]);
        
        return(NULL);
}

const char *libcyc_kvm_get_model_short_str(struct cyc_info *cyc)
{
	enum kvm_model model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	model = libcyc_kvm_get_model(cyc);

        if (model != ERROR) return(KVM_MODEL_SHORT[model]);
        
        return(NULL);
}

enum kvmip_id libcyc_kvm_led_get_status(struct cyc_info *cyc)
{
	int fd;
	enum kvmip_id led_map;
	
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_kvm_get_model(cyc) != MODEL_KVMNET_PLUS) return(ERROR);

	if ((fd = libcyc_open_dev(cyc, KVMIP_LED_DEVFILE, O_RDONLY)) == -1) {
		return(ERROR);
	}

	ioctl(fd, CYKVMIPLEDGETSTAT, &led_map);

	libcyc_close_dev(cyc, fd);

	return(led_map);
}

enum kvmip_id libcyc_kvm_led_set_on(struct cyc_info *cyc, enum kvmip_id led_map)
{
	int fd;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (libcyc_kvm_get_model(cyc) != MODEL_KVMNET_PLUS) return(ERROR);

	if ((fd = libcyc_open_dev(cyc, KVMIP_LED_DEVFILE, O_WRONLY)) == -1) {
		return(ERROR);
	}
	
	ioctl(fd, CYKVMIPLEDSETON, &led_map);

	libcyc_close_dev(cyc, fd);

	return(led_map);
}

enum kvmip_id libcyc_kvm_led_set_off(struct cyc_info *cyc, enum kvmip_id led_map)
{
	int fd;
	
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (libcyc_kvm_get_model(cyc) != MODEL_KVMNET_PLUS) return(ERROR);

	if ((fd = libcyc_open_dev(cyc, KVMIP_LED_DEVFILE, O_WRONLY)) == -1) {
		return(ERROR);
	}
	
	ioctl(fd, CYKVMIPLEDSETOFF, &led_map);

	libcyc_close_dev(cyc, fd);

	return(led_map);
}

enum kvmip_id libcyc_kvm_led_set(struct cyc_info *cyc, enum kvmip_id led_map)
{
	int fd;
	
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (libcyc_kvm_get_model(cyc) != MODEL_KVMNET_PLUS) return(ERROR);

	if ((fd = libcyc_open_dev(cyc, KVMIP_LED_DEVFILE, O_WRONLY)) == -1) {
		return(ERROR);
	}
	
	ioctl(fd, CYKVMIPLEDSETLEDS, &led_map);

	libcyc_close_dev(cyc, fd);

	return(led_map);
}

int libcyc_kvm_check_id(struct cyc_info *cyc, int id)
{
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (id == ERROR) return(ERROR);
	
	if (libcyc_kvm_get_ip_modules_map(cyc) & kvmip_id_lookup[id])
		return(TRUE);
	
	return(FALSE);
}

int libcyc_kvm_set_vidqual_disable(struct cyc_info *cyc, int state)
{
	int fd, ret=0;
        
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

        if ((libcyc_kvm_get_model(cyc) == MODEL_KVMNET) ||
            (libcyc_kvm_get_model(cyc) == MODEL_KVM) ||
	    (libcyc_kvm_get_model(cyc) == MODEL_ONSITE) ||
	    (libcyc_kvm_get_model(cyc) == MODEL_KVMNET_PLUS)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_WRONLY)) == -1) {
                        return(ERROR);
                }

                ret=ioctl(fd, CYAPCTLSETVIDQUAL, &state);
                libcyc_close_dev(cyc, fd);
	} else {
		return(ERROR);
	}

	return(ret);
}

int libcyc_kvm_get_vidqual_disable(struct cyc_info *cyc)
{
        int fd;
	int state=0, ret=0;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

        if ((libcyc_kvm_get_model(cyc) == MODEL_KVMNET) ||
            (libcyc_kvm_get_model(cyc) == MODEL_KVM) ||
	    (libcyc_kvm_get_model(cyc) == MODEL_ONSITE) ||
	    (libcyc_kvm_get_model(cyc) == MODEL_KVMNET_PLUS)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_RDONLY)) == -1) {
                        return(ERROR);
                }

                ret=ioctl(fd, CYAPCTLGETVIDQUAL, &state);
                libcyc_close_dev(cyc, fd);
	}

	if (ret == -1) return(ERROR);

        return(state ? ON : OFF);
}

int libcyc_kvm_kvmipm_reset(struct cyc_info *cyc, int id)
{
	int fd, ret=0;
	char debug_str[MAX_STR_LEN];
        
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (libcyc_kvm_get_model(cyc) == MODEL_KVM) return(ERROR);
	if (libcyc_kvm_check_id(cyc, id) == FALSE) return(ERROR);

	snprintf(debug_str, MAX_STR_LEN, "%s id: %d", __func__, id);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_WRONLY)) == -1) {
		return(ERROR);
	}
	
	ret=ioctl(fd, CYAPCTLKVMIPRESET, &id);
	libcyc_close_dev(cyc, fd);
	
	return(ret);
}

enum kvmipm_type libcyc_kvm_kvmipm_get_type(struct cyc_info *cyc, int id)
{
	int index;
	char type_str[MAX_STR_LEN];
	char *line=NULL,*next=NULL;
	FILE *fp = NULL;
	char debug_str[MAX_STR_LEN];
	int type_len = 0;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (libcyc_kvm_get_model(cyc) == MODEL_KVM) return(ERROR);
	if (libcyc_kvm_check_id(cyc, id) == FALSE) return(ERROR);

	/* Read in the vcard type file */
        if ((fp = fopen(VCARD_TYPE_FILE, "r")) == NULL) {
                perror("libcyc_kvm_kvmipm_get_type");
                return(ERROR);
        }
        type_len = fread(type_str, 1, MAX_STR_LEN, fp);
        fclose(fp);

	type_str[type_len] = '\0';	

	snprintf(debug_str, MAX_STR_LEN, "%s type_str: %s",
		 __func__, type_str);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	for (index = 1, line=type_str; line; index++, line=next) {
		next = strchr(line, VCARD_TYPE_DELIMITER);
		if (next) *next++ = '\0';
		if (index == id) return(atoi(line));
	}
	
	return(ERROR);
}

char *libcyc_kvm_get_kvmip_ip_addr(struct cyc_info *cyc, int id)
{
	int max_modules = ERROR;

	if (cyc == NULL) return(NULL);

	max_modules = libcyc_kvm_get_max_ip_modules(cyc);

	if ((id > max_modules) || (id < 1)) return(NULL);
	if (libcyc_kvm_check_id(cyc, id) == FALSE) return(NULL);

	return(parse_netconf_runfile(cyc, id));
}

int libcyc_kvm_rdpserver_probe(struct cyc_info *cyc, const char *srvip,
			       unsigned short srvport)
{
	int tcpsock = socket(PF_INET, SOCK_STREAM, 0);
	int ret, syncnt = 1;
	struct sockaddr_in serv;
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return(ERROR);

	snprintf(debug_str, MAX_STR_LEN, "%s srvip: %s srvport: %d",
		 __func__, (char *)srvip, (int)srvport);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	if (tcpsock == -1) {
		return(FALSE); 
	}

	if (setsockopt(tcpsock, SOL_TCP, TCP_SYNCNT, &syncnt, sizeof (ulong)) == -1) {
		close (tcpsock);
		return(FALSE);
    	}

	memset((char *)&serv, 0, sizeof(serv));
	if ((serv.sin_addr.s_addr = inet_addr(srvip)) != INADDR_NONE) {
		serv.sin_family = AF_INET;
	} else {
		close (tcpsock);
		return(FALSE);
	}
	serv.sin_port = htons(srvport);

	ret = (connect(tcpsock, (struct sockaddr *)&serv, sizeof (serv))) ?
		FALSE : TRUE; 

	shutdown(tcpsock, SHUT_RDWR);
	close (tcpsock);

	return(ret);
}

int libcyc_kvm_get_kvmipm_id_from_station(struct cyc_info *cyc, int station)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	switch (cyc->kvm.model) {
	case MODEL_KVMNET_PLUS:
		return(station - 2);
		break;
	case MODEL_ONSITE:
		if (cyc->prod.board_version == 2) return(station - 1);
		break;
	default:
		break;
	}
	return(station);
}

int libcyc_kvm_get_station_from_kvmipm_id(struct cyc_info *cyc, int ipBoard)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	switch (cyc->kvm.model) {
	case MODEL_KVMNET_PLUS:
		return(ipBoard + 2);
		break;
	case MODEL_ONSITE:
		if (cyc->prod.board_version == 2) return(ipBoard + 1);
		break;
	default:
		break;
	}
	return(ipBoard);
}

static void parse_username(struct cyc_info *cyc, char *buf,
			   int buf_sz, void *handle)
{
	const char *needle = "Login: Logged with user ";
	char *haystack = NULL;
	char *tmp = NULL;
	char *username = (char *)handle;

	if (buf_sz <= strlen(needle)) return;
	if (buf == NULL) return;
	if (username == NULL) return;
       
	haystack = buf;
	if ((haystack = strstr(haystack, needle))) {
		haystack += strlen(needle);
		if ((tmp = strstr(haystack, " for"))) {
			*tmp = '\0';
		} else {
			return;
		}
		strncpy(username, haystack, KVM_USERNAME_LEN);
		username[KVM_USERNAME_LEN-1] = '\0';
	}
}

/* Return the username of the user connected to the station specified
   as an argument; returns NULL if no users are connected to the station
   or if an error occurs. */
char *libcyc_kvm_get_username_from_station(struct cyc_info *cyc, int station)
{
	char *s_argv[] = { "kvm", "status","-s", NULL, NULL };
	char station_str[2];
	char debug_str[MAX_STR_LEN];
	char *ret_str = NULL;
	char username[KVM_USERNAME_LEN];
 
	if (cyc == NULL) return(NULL);
	if ((station < 0) || (station > libcyc_kvm_get_num_stations(cyc))) {
		return(NULL);
	}

	snprintf(station_str, 2, "%d", station);
	station_str[1] = '\0';
	s_argv[3] = station_str;

	username[0] = '\0';
	libcyc_system_pipe(cyc, KVM_COMMAND, s_argv,
			   parse_username, username);

	if (username[0]) {
		username[KVM_USERNAME_LEN-1] = '\0';
		ret_str = libcyc_strdup(cyc, username);
	}

	snprintf(debug_str, MAX_STR_LEN, "%s station: %d username: %s",
		 __func__, station, ret_str);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	return(ret_str);
}

/* Use the KVM command to show a warning message on the OSD */
int libcyc_kvm_osd_warning(struct cyc_info *cyc, const char *str)
{
	char debug_str[MAX_STR_LEN];
	char warn_str[MAX_STR_LEN];
	char *w_argv[] = { "kvm", "warning", "-m", warn_str, NULL };
	
	if (cyc == NULL) return(ERROR);
	if (str == NULL) return(ERROR);
	
	snprintf(debug_str, MAX_STR_LEN, "%s str: %s", __func__, str);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	strncpy(warn_str, str, MAX_STR_LEN);
	warn_str[MAX_STR_LEN-1] = '\0';

	return(libcyc_system(cyc, KVM_COMMAND, w_argv));
}

static void parse_w(struct cyc_info *cyc, char *buf, int buf_sz,
		    void *handle)
{
	struct sess_parse_info *spinfo = (struct sess_parse_info *)handle;
	libcyc_list **sess_list = NULL;
	struct session_info *sinfo = NULL;
	enum sess_parse_state *state = NULL;
	const char *needle1 = "KVM sessions";
	const char *needle2 = "USER";
	char debug_str[MAX_STR_LEN];

	if ((cyc == NULL) || (buf == NULL)) return;
	if (spinfo == NULL) return;

	sess_list = spinfo->slist;
	state = &(spinfo->state);

	snprintf(debug_str, MAX_STR_LEN, "%s state: %d", __func__, *state);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	if (*state == SESS_STATE_PARSE_READY) {
		char *p = NULL, *q = NULL;
		
		p = q = buf;

		sinfo = libcyc_malloc(cyc, sizeof(struct session_info));

		/* Parse the output to find the active sessions.
		   The algorithm is to jump over characters starting
		   with the user (assuming there's no whitespace before
		   the username) and mark the end with \0.  Then, copy
		   the string into the session_info structure.  The next
		   field is parsed by jumping over the whitespace to get
		   the new start: the process repeats until we've
		   parsed all the fields. */

		/* User */
		while(*p && (! isspace(*p))) p++;
		*p = '\0';
		strncpy(sinfo->user, q, SESS_STR_LEN);
		sinfo->user[SESS_STR_LEN-1] = '\0';

		/* Port alias and port number the user is connected to*/
		q = ++p;
		while (*q && isspace(*q)) q++;
		p = q;
		while(*p && (! isspace(*p))) p++;
		*p = '\0';
		strncpy(sinfo->to_alias, q, SESS_STR_LEN);
		sinfo->to_alias[SESS_STR_LEN-1] = '\0';
		sinfo->to_port = libcyc_kvm_get_physport_from_alias(cyc, sinfo->to_alias);

		/* From where is the user connecting */
		q = ++p;
		while (*q && isspace(*q)) q++;
		p = q;
		while(*p && (! isspace(*p))) p++;
		*p = '\0';
		strncpy(sinfo->from, q, SESS_STR_LEN);
		sinfo->from[SESS_STR_LEN-1] = '\0';

		if (sscanf(sinfo->from, "user%d",
			   &(sinfo->from_station)) != 1) {
			sinfo->from_station = ERROR;
		}

		/* Session start time */
		q = ++p;
		while (*q && isspace(*q)) q++;
		p = q;
		while(*p && (! isspace(*p))) p++;
		*p = '\0';
		strncpy(sinfo->login_start, q, SESS_STR_LEN);
		sinfo->login_start[SESS_STR_LEN-1] = '\0';
		
		/* Add this newly-collected data to our linked list */
		*sess_list = libcyc_ll_append(*sess_list, sinfo);
	} else if (strstr(buf, needle1)) {
		*state = SESS_STATE_PARSE_KVM;
	} else if ((*state == SESS_STATE_PARSE_KVM) &&
		   (strstr(buf, needle2))) {
		*state = SESS_STATE_PARSE_READY;
	}
}

static void parse_power_status_str(struct cyc_info *cyc,
				   libcyc_list **powerstat_list,
				   const char *powerstat_str)
{
	const char outlet_delim = ' ';
	const char *pstate_on_str = "on";
	const char *pstate_off_str = "off";
	const char *lstate_lock_str = "locked";
	const char *lstate_unlock_str = "unlocked";
	char *p_stat_str = NULL;
	char *begin, *end;
	struct power_info *p = NULL;
	char pstat[4];
	char lstat[9];

	if (cyc == NULL) return;
	if (powerstat_list == NULL) return;
	if (powerstat_str == NULL) return;

        p_stat_str = libcyc_strdup(cyc, powerstat_str);

        for (begin = p_stat_str, end = strchr(p_stat_str, outlet_delim);
             begin && end;
             begin = ++end, end = strchr(begin, outlet_delim)) {
                *end = '\0';
		p = libcyc_malloc(cyc, sizeof(struct power_outlet));
		p->outlet = ERROR;
		p->state = 0;

		if (sscanf(begin, "%d:%3s:%8s",
			   &(p->outlet), pstat, lstat) == 3) {
			pstat[sizeof(pstat)-1] = '\0';
			lstat[sizeof(lstat)-1] = '\0';
			
			if (strncmp(pstat, pstate_on_str, 2) == 0) {
				p->state |= POWER_STATE_ON;
			} else if (strncmp(pstat, pstate_off_str, 3) == 0) {
				p->state |= POWER_STATE_OFF;
			}
			
			if (strncmp(lstat, lstate_lock_str, 6) == 0) {
				p->state |= POWER_STATE_LOCKED;
			} else if (strncmp(lstat, lstate_unlock_str, 8) == 0) {
				p->state |= POWER_STATE_UNLOCKED;
			}
		} else {
			libcyc_free(cyc, p);
			p = NULL;
			continue;
		}

		libcyc_push(powerstat_list, p);
        }

	p = libcyc_malloc(cyc, sizeof(struct power_outlet));
	p->outlet = ERROR;
	p->state = 0;
	
	if (sscanf(begin, "%d, %3s, %8s",
		   &(p->outlet), pstat, lstat) == 3) {
		pstat[sizeof(pstat)-1] = '\0';
		lstat[sizeof(lstat)-1] = '\0';

		if (strncmp(pstat, pstate_on_str, 2) == 0) {
			p->state |= POWER_STATE_ON;
		} else if (strncmp(pstat, pstate_off_str, 3) == 0) {
			p->state |= POWER_STATE_OFF;
		}
		
		if (strncmp(lstat, lstate_lock_str, 6) == 0) {
			p->state |= POWER_STATE_LOCKED;
		} else if (strncmp(lstat, lstate_unlock_str, 8) == 0) {
			p->state |= POWER_STATE_UNLOCKED;
		}
	} else {
		libcyc_free(cyc, p);
		p = NULL;
		goto cleanup;
	}
	
	libcyc_push(powerstat_list, p);
	
 cleanup:
        libcyc_free(cyc, p_stat_str);
        p_stat_str = NULL;
}


static void parse_kvmstat(struct cyc_info *cyc, char *buf, int buf_sz,
			  void *handle)
{
	const char *needle = "Login: Logged with user ";
	const char *p_needle = "Current permission";
	const char *cm_needle = "Connection Mode";
	const char *ps_needle = "Power status";
	const char *physical_str = "Physical";
	const char *network_str = "Network from ";
	const char *read_perm_str = "read";
	const char *write_perm_str = "write";
	const char *power_perm_str = "power";
	const char *local_port_str = "Local Port";
	struct session_info *sinfo = (struct session_info *)handle;
	char d_needle[MAX_STR_LEN];
	char *haystack = NULL;
	char *tmp = NULL;
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return;
	if (sinfo == NULL) return;
	if (buf_sz <= strlen(needle)) return;
	if (buf == NULL) return;

	snprintf(debug_str, MAX_STR_LEN, "%s station: %d", __func__,
		 sinfo->from_station);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	snprintf(d_needle, MAX_STR_LEN, "%s%s for ", needle, sinfo->user);
	d_needle[MAX_STR_LEN-1] = '\0';
	
	haystack = buf;
	if (strstr(haystack, d_needle)) {
		/* Connection duration */
		haystack = strstr(haystack, d_needle);
		haystack += strlen(d_needle);
		if ((tmp = strstr(haystack, " seconds"))) {
			*tmp = '\0';
		} else {
			return;
		}
		snprintf(debug_str, MAX_STR_LEN, "%s login duration: %s",
			 __func__, haystack);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		sinfo->login_duration = atoi(haystack);
	} else if (strstr(haystack, p_needle)) {
		/* Permissions */
		if (strstr(haystack, read_perm_str)) {
			sinfo->perms |= KVM_PERMS_READ;
		}
		if (strstr(haystack, write_perm_str)) {
			sinfo->perms |= KVM_PERMS_WRITE;
		}
		if (strstr(haystack, power_perm_str)) {
			sinfo->perms |= KVM_PERMS_POWER;
		}
	} else if (strstr(haystack, cm_needle)) {
		/* Connection Mode */
		if (strstr(haystack, physical_str)) {
			strncpy(sinfo->from, local_port_str, SESS_STR_LEN);
			sinfo->from[SESS_STR_LEN-1] = '\0';
		} else if (strstr(haystack, network_str)) {
			haystack = strstr(haystack, network_str);
			haystack += strlen(network_str);
			if ((tmp = strstr(haystack, " SSID"))) {
				*tmp = '\0';
			} else {
				return;
			}
			strncpy(sinfo->from, haystack, SESS_STR_LEN);
			sinfo->from[SESS_STR_LEN-1] = '\0';
		}
	} else if (strstr(haystack, ps_needle)) {
		/* Power status */
		parse_power_status_str(cyc, &(sinfo->pinfo), haystack);
	}
}

/* This function gets the session info for the KVM and populates a linked
   list containing the data about each active session.  See the kvminfo
   utility for use of this function call.
*/
int libcyc_kvm_session_info_get(struct cyc_info *cyc,
				libcyc_list **sess_list,
				int *num_sessions)
{
	char *w_argv[] = { "kvm", "w", NULL };
	struct sess_parse_info spinfo;
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return(ERROR);
	if ((sess_list == NULL) || (num_sessions == NULL)) return(ERROR);
	
	snprintf(debug_str, MAX_STR_LEN, "%s entering", __func__);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);
	
	spinfo.slist = sess_list;
	spinfo.state = SESS_STATE_PARSE_ERROR;

	/* Populate the sess_list structure */
	libcyc_system_pipe(cyc, KVM_COMMAND, w_argv, parse_w, &spinfo);

	if (*sess_list) {
		struct session_info *sinfo;

		/* Populate the duration field */
		while ((sinfo = (struct session_info *)libcyc_ll_iterate(*sess_list))) {
			char station_str[2];
			char *k_argv[] = {
				"kvm", "status", "-s", station_str, NULL
			};

			snprintf(station_str, 2, "%d", sinfo->from_station);
			station_str[1] = '\0';

			libcyc_system_pipe(cyc, KVM_COMMAND, k_argv,
					   parse_kvmstat, sinfo);
		}

		*num_sessions = libcyc_ll_get_size(*sess_list);
	}

	return(0);
}

/* When we're done with the session information, free it */
void libcyc_kvm_session_info_destroy(struct cyc_info *cyc,
				     libcyc_list **slist)
{
	struct session_info *sinfo = NULL;

	if ((slist == NULL) || (*slist == NULL)) return;

	while ((sinfo = (struct session_info *)
		libcyc_ll_iterate(*slist))) {
		struct power_info *pinfo = NULL;

		while ((pinfo = (struct power_info *)
			libcyc_ll_iterate(sinfo->pinfo))) {
			libcyc_free(cyc, pinfo);
			pinfo = NULL;
		}

		libcyc_ll_free(sinfo->pinfo);
		sinfo->pinfo = NULL;

		libcyc_free(cyc, sinfo);
		sinfo = NULL;
	}

	libcyc_ll_free(*slist);
	*slist = NULL;
} 

int libcyc_kvm_viewer_title_create(struct cyc_info *cyc,
				   char *viewer_title, int len,
				   const char *portname, int kvmip_id)
{
	if (len <= 0) return(ERROR);
	if (libcyc_kvm_check_id(cyc, kvmip_id) == FALSE) return(ERROR);

	snprintf(viewer_title, len, "%s - %s #%d",
		 PROXY_VIEWER_NAME, portname, kvmip_id);
	viewer_title[len-1] = '\0';

	return(0);
}

int libcyc_kvm_wait_for_kvmipm_life(struct cyc_info *cyc, int kvmip_id)
{
	const int time_towait = 30;
	char *kvmip_ip = NULL;
	int ret = ERROR;
        char debug_str[MAX_STR_LEN];
	int i;

	if (cyc == NULL) return(ERROR);
	if (libcyc_kvm_check_id(cyc, kvmip_id) == FALSE) return(ERROR);

	snprintf(debug_str, MAX_STR_LEN, "%s kvmip_id: %d",
		 __func__, kvmip_id);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	kvmip_ip = libcyc_kvm_get_kvmip_ip_addr(cyc, kvmip_id);
	if (kvmip_ip == NULL) return(ERROR);

	for (i = time_towait; i > 0; i--) {
		if (libcyc_ping(cyc, kvmip_ip) == TRUE) {
			ret = 0;
			break;
		}
		sleep(1);
	}

	if (ret) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s host %s did not wake up in %d seconds",
			 __func__, kvmip_ip, time_towait);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	libcyc_free(cyc, kvmip_ip);
	kvmip_ip = NULL;

	return(ret);
}

static void parse_outlet_str(struct cyc_info *cyc, libcyc_list **outlet_list,
			     const char *outlet_str)
{
	const char outlet_delim = ',';
	char *t_out_str = NULL;
	char *begin, *end;
	struct power_outlet *p = NULL;

	if (cyc == NULL) return;
	if (outlet_list == NULL) return;
	if (outlet_str == NULL) return;

        t_out_str = libcyc_strdup(cyc, outlet_str);

        for (begin = t_out_str, end = strchr(t_out_str, outlet_delim);
             begin && end;
             begin = ++end, end = strchr(begin, outlet_delim)) {
                *end = '\0';
		p = libcyc_malloc(cyc, sizeof(struct power_outlet));
		p->outlet = atoi(begin);
		libcyc_push(outlet_list, p);
        }
	p = libcyc_malloc(cyc, sizeof(struct power_outlet));
	p->outlet = atoi(begin);
	libcyc_push(outlet_list, p);

        libcyc_free(cyc, t_out_str);
        t_out_str = NULL;
}

void libcyc_kvm_parse_kvmd_conf(struct cyc_info *cyc, libcyc_list **alist)
{
	FILE *fp;
	char *buf = NULL;
	int buf_sz = 0;
	int len = 0;
	struct kvmd_config *kcfg = NULL;
	const char *yes_str = "yes";

	if (cyc == NULL) return;
	if (alist == NULL) return;

	if ((fp = fopen(KVMD_CONFFILE, "r")) == NULL) return;

	while ((buf_sz = getline(&buf, &len, fp)) != -1) {
		int port = ERROR;
		int vport = ERROR;
		char alias[KVM_ALIAS_LEN];
		char state[4];
		char outlet_str[255];

		if (kcfg == NULL) {
			kcfg = libcyc_malloc(cyc, sizeof(struct kvmd_config));
			kcfg->vport = ERROR;
			kcfg->port = ERROR;
			kcfg->type = PORT_TYPE_UNKNOWN;
			kcfg->active = FALSE;
			kcfg->outlets = NULL;
			memset(kcfg->alias, '\0', sizeof(KVM_ALIAS_LEN));
		}

		/* This if/else block populates data based on the lines
		   that match.  The walk-the-list/add-to-list construct
		   gives us flexibility -- data is read in properly
		   regardless of the order of the parameters in kvmd.conf */
		if (sscanf(buf, "p%d.port %d", &vport, &port) == 2) {
			struct kvmd_config *tmp_kcfg = NULL;
			int changed = FALSE;

			while ((tmp_kcfg = (struct kvmd_config *)
				libcyc_ll_iterate(*alist))) {
				if (vport == tmp_kcfg->vport) {
					tmp_kcfg->vport = vport;
					tmp_kcfg->port = port;
					if ((tmp_kcfg->port > 0) &&
					    (tmp_kcfg->port <= libcyc_kvm_get_ports(cyc)) &&
					    (! *(tmp_kcfg->alias))) {
						/* Set default name; alias
						   parameter will overwrite
						   if it exists */
						strncpy(tmp_kcfg->alias, libcyc_port_name_get_default(cyc, tmp_kcfg->port, tmp_kcfg->type), KVM_ALIAS_LEN);
						tmp_kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
					}
					changed = TRUE;
				}
			}
			if (changed == FALSE) {
				kcfg->vport = vport;
				kcfg->port = port;
				kcfg->type = PORT_TYPE_KVM;
				if ((kcfg->port > 0) && (kcfg->port <= libcyc_kvm_get_ports(cyc)) && (! *(kcfg->alias))) {
					/* Set default name; alias parameter
					   will overwrite if it exists */
					strncpy(kcfg->alias, libcyc_port_name_get_default(cyc, kcfg->port, kcfg->type), KVM_ALIAS_LEN);
					kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
				}
				*alist = libcyc_ll_append(*alist, kcfg);
				kcfg = NULL;
			}
		} else if (sscanf(buf, "p%2d.alias %20s", &vport, alias) == 2) {
			struct kvmd_config *tmp_kcfg = NULL;
			int changed = FALSE;

			while ((tmp_kcfg = (struct kvmd_config *)
				libcyc_ll_iterate(*alist))) {
				if (vport == tmp_kcfg->vport) {
					strncpy(tmp_kcfg->alias, alias,
						KVM_ALIAS_LEN);
					tmp_kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
					changed = TRUE;
				}
			}
			if (changed == FALSE) {
				kcfg->vport = vport;
				strncpy(kcfg->alias, alias, KVM_ALIAS_LEN);
				kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
				kcfg->type = PORT_TYPE_KVM;
				*alist = libcyc_ll_append(*alist, kcfg);
				kcfg = NULL;
			}
		} else if (sscanf(buf, "p%2d.active %3s", &vport, state) == 2) {
			struct kvmd_config *tmp_kcfg = NULL;
			int changed = FALSE;
			
			while ((tmp_kcfg = (struct kvmd_config *)
				libcyc_ll_iterate(*alist))) {
				if (vport == tmp_kcfg->vport) {
					if (strncmp(state, yes_str, 3) == 0) {
						tmp_kcfg->active = TRUE;
					} else {
						tmp_kcfg->active = FALSE;
					}

					if ((tmp_kcfg->port > 0) && (tmp_kcfg->port <= libcyc_kvm_get_ports(cyc)) && (! *(tmp_kcfg->alias))) {
						/* Set default name; alias parameter
						   will overwrite if it exists */
						strncpy(tmp_kcfg->alias,
							libcyc_port_name_get_default(cyc, tmp_kcfg->port,
										     tmp_kcfg->type),
							KVM_ALIAS_LEN);
						tmp_kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
					}
					changed = TRUE;
				}
			}
			if (changed == FALSE) {
				kcfg->vport = vport;
				if (strncmp(state, yes_str, 3) == 0) {
					kcfg->active = TRUE;
				} else {
					kcfg->active = FALSE;
				}
				kcfg->type = PORT_TYPE_KVM;
				if ((kcfg->port > 0) && (kcfg->port <= libcyc_kvm_get_ports(cyc)) && (! *(kcfg->alias))) {
					/* Set default name; alias parameter
					   will overwrite if it exists */
					strncpy(kcfg->alias, libcyc_port_name_get_default(cyc, kcfg->port, kcfg->type), KVM_ALIAS_LEN);
					kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
				}
				*alist = libcyc_ll_append(*alist, kcfg);
				kcfg = NULL;
			}
		} else if (sscanf(buf, "p%2d.pmoutlet %255s", &vport, outlet_str) == 2) {
			struct kvmd_config *tmp_kcfg = NULL;
			int changed = FALSE;
			
			while ((tmp_kcfg = (struct kvmd_config *)
				libcyc_ll_iterate(*alist))) {
				if (vport == tmp_kcfg->vport) {
					parse_outlet_str(cyc,
							 &(tmp_kcfg->outlets),
							 outlet_str);
					if ((tmp_kcfg->port > 0) && (tmp_kcfg->port <= libcyc_kvm_get_ports(cyc)) && (! *(tmp_kcfg->alias))) {
						/* Set default name; alias
						   parameter will overwrite
						   if it exists */
						strncpy(tmp_kcfg->alias, libcyc_port_name_get_default(cyc, tmp_kcfg->port, tmp_kcfg->type), KVM_ALIAS_LEN);
						tmp_kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
					}
					changed = TRUE;
				}
			}
			if (changed == FALSE) {
				kcfg->vport = vport;
				parse_outlet_str(cyc, &(kcfg->outlets),
						 outlet_str);
				kcfg->type = PORT_TYPE_KVM;
				if ((kcfg->port > 0) && (kcfg->port <= libcyc_kvm_get_ports(cyc)) && (! *(kcfg->alias))) {
					/* Set default name; alias parameter
					   will overwrite if it exists */
					strncpy(kcfg->alias, libcyc_port_name_get_default(cyc, kcfg->port, kcfg->type), KVM_ALIAS_LEN);
					kcfg->alias[KVM_ALIAS_LEN-1] = '\0';
				}
				*alist = libcyc_ll_append(*alist, kcfg);
				kcfg = NULL;
			}
		}
	}
	
	fclose(fp);
}

static int get_port_from_alias_list(libcyc_list **alist, const char *alias)
{
	struct kvmd_config *kcfg = NULL;
	int port = ERROR;

	if ((alist == NULL) || (*alist == NULL)) return(ERROR);
	if (alias == NULL) return(ERROR);

	while ((kcfg = (struct kvmd_config *)
		libcyc_ll_iterate(*alist))) {
		if (strncmp(alias, kcfg->alias,
			    (strlen(alias) < KVM_ALIAS_LEN) ?
			    strlen(alias)+1 : KVM_ALIAS_LEN) == 0) {
			port = kcfg->port;
		}
	}
	
	return(port);
}

static const char *get_alias_from_alias_list(libcyc_list **alist, int port)
{
	struct kvmd_config *kcfg = NULL;
	char *alias = NULL;

	if ((alist == NULL) || (*alist == NULL)) return(NULL);

	while ((kcfg = (struct kvmd_config *)
		libcyc_ll_iterate(*alist))) {
		if (port == kcfg->port) {
			alias = kcfg->alias;
		}
	}
	
	return(alias);
}

static int get_active_from_alias_list(libcyc_list **alist, const char *alias)
{
	struct kvmd_config *kcfg = NULL;
	int active = ERROR;

	if ((alist == NULL) || (*alist == NULL)) return(ERROR);
	if (alias == NULL) return(ERROR);

	while ((kcfg = (struct kvmd_config *)
		libcyc_ll_iterate(*alist))) {
		if (strncmp(alias, kcfg->alias,
			    (strlen(alias) < KVM_ALIAS_LEN) ?
			    strlen(alias)+1 : KVM_ALIAS_LEN) == 0) {
			active = kcfg->active;
		}
	}
	
	return(active);
}

char *libcyc_kvm_get_physport_from_alias(struct cyc_info *cyc,
					 const char *alias)
{
	int slave_port = ERROR;
	char *casc_devname;
	char physport_str[6];
	char *ret_str = NULL;

	if (cyc == NULL) return(NULL);
	if (alias == NULL) return(NULL);
	
	if (libcyc_update_list(cyc, KVMD_CONFFILE, &(cyc->conf_file_md5sum),
			libcyc_kvm_parse_kvmd_conf,
			&(cyc->conf_data))) return(NULL);
	
	casc_devname = libcyc_kvm_get_slave_name_from_alias(cyc, alias);
	if (casc_devname) {
		slave_port = libcyc_kvm_slave_get_port_a_from_name(cyc, casc_devname);
	}

	if (slave_port != ERROR) {
		int slave_devport = libcyc_kvm_get_slave_port_from_alias(cyc, alias);

		if ((slave_devport != ERROR) && (slave_devport > 0) &&
		    (slave_devport <= libcyc_kvm_slave_get_num_ports_from_name(cyc, casc_devname))) {
			snprintf(physport_str, sizeof(physport_str),
				 "%d.%d", slave_port, slave_devport);
			physport_str[sizeof(physport_str)-1] = '\0';

			ret_str = libcyc_strdup(cyc, physport_str);
		}
	} else {
		int port = get_port_from_alias_list(&(cyc->conf_data), alias);
		if (port == ERROR) return(NULL);
		
		snprintf(physport_str, sizeof(physport_str), "%d", port);
		physport_str[sizeof(physport_str)-1] = '\0';
		
		ret_str = libcyc_strdup(cyc, physport_str);
	}

	if (casc_devname) {
		libcyc_free(cyc, casc_devname);
		casc_devname = NULL;
	}
	
	return(ret_str);
}

char *libcyc_kvm_get_alias_from_physport(struct cyc_info *cyc,
					 const char *physport_str)
{
	int slave_port = ERROR;
	int slave_devport = ERROR;
	char *alias = NULL;

	if (cyc == NULL) return(NULL);
	if (physport_str == NULL) return(NULL);

	if (libcyc_update_list(cyc, KVMD_CONFFILE, &(cyc->conf_file_md5sum),
			      libcyc_kvm_parse_kvmd_conf,
			      &(cyc->conf_data))) return(NULL);
	
	slave_port = libcyc_kvm_get_slave_port_a_from_physport_str(cyc, physport_str);
	
	if (slave_port != ERROR) {
		const char *casc_name = libcyc_kvm_slave_get_name_from_port_a(cyc, slave_port);

		slave_devport = libcyc_kvm_get_slave_port_from_physport_str(cyc, physport_str);
		if (casc_name && (slave_devport > 0) && (slave_devport <= libcyc_kvm_slave_get_num_ports_from_name(cyc, casc_name))) {
			char tmp_alias[KVM_ALIAS_LEN];

			snprintf(tmp_alias, KVM_ALIAS_LEN, "%s.%d",
				 casc_name, slave_devport);
			tmp_alias[KVM_ALIAS_LEN-1] = '\0';

			alias = libcyc_strdup(cyc, tmp_alias);
		}
	} else {
		int physport = atoi(physport_str);

		if (physport > 0) {
			const char *t_alias = get_alias_from_alias_list(&(cyc->conf_data), physport);
			if (t_alias) alias = libcyc_strdup(cyc, t_alias);
		}
	}

	return(alias);
}

int libcyc_kvm_get_port_active_from_alias_noupdate(struct cyc_info *cyc,
						   const char *alias)
{
	if (cyc == NULL) return(ERROR);
	if (alias == NULL) return(ERROR);

	return(get_active_from_alias_list(&(cyc->conf_data), alias));
}

int libcyc_kvm_get_port_active_from_alias(struct cyc_info *cyc,
					  const char *alias)
{
	if (cyc == NULL) return(ERROR);
	if (alias == NULL) return(ERROR);
	
	if (libcyc_update_list(cyc, KVMD_CONFFILE, &(cyc->conf_file_md5sum),
			libcyc_kvm_parse_kvmd_conf,
			&(cyc->conf_data))) return(ERROR);
	
	return(libcyc_kvm_get_port_active_from_alias_noupdate(cyc, alias));
}

/* When we're done with the list of ports/aliases, free them */
void libcyc_kvm_physport_alias_data_destroy(struct cyc_info *cyc,
					    libcyc_list **alist)
{
	struct kvmd_config *kcfg = NULL;

	if ((alist == NULL) || (*alist == NULL)) return;

	while ((kcfg = (struct kvmd_config *)
		libcyc_ll_iterate(*alist))) {
		struct power_outlet *poutlet = NULL;
		
		while ((poutlet = (struct power_outlet *)
			libcyc_ll_iterate(kcfg->outlets))) {
			libcyc_free(cyc, poutlet);
			poutlet = NULL;
		}

		libcyc_ll_free(kcfg->outlets);
		kcfg->outlets = NULL;

		libcyc_free(cyc, kcfg);
		kcfg = NULL;
	}

	libcyc_ll_free(*alist);
	*alist = NULL;
}

void libcyc_kvm_parse_kvmd_slaves_conf(struct cyc_info *cyc, libcyc_list **slist)
{
	const char *slave_type_sw = "sw";
	const char *slave_type_exp = "pem";
	FILE *fp;
	char *buf = NULL;
	int buf_sz = 0;
	int len = 0;
	struct kvm_slave_info *sli = NULL;
	char port_str[6];
	char type_str[4];
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return;
	if (slist == NULL) return;

	if ((fp = fopen(KVMD_SLAVES_CONFFILE, "r")) == NULL) return;

	while ((buf_sz = getline(&buf, &len, fp)) != -1) {
		int found = 0;

		if (sli == NULL) {
			sli = libcyc_malloc(cyc, sizeof(struct kvm_slave_info));
		}

		found = sscanf(buf, "%20s %6s %d %4s", sli->name, port_str,
			       &(sli->num_ports), type_str);
		if (found == 3) {
			strncpy(type_str, "sw", 2);
			type_str[2] = '\0';
		} else if ((found < 3) || (found > 4)) {
			continue;
		}

		if (sscanf(port_str, "%d,%d", &(sli->port_a),
			   &(sli->port_b)) == 1) {
			sli->port_a = atoi(port_str);
			sli->port_b = ERROR;
		}
		    
		if (strncmp(type_str, slave_type_sw,
			    strlen(slave_type_sw)) == 0) {
			sli->type = KVM_TYPE_SLAVE_SWITCH;
		} else if (strncmp(type_str, slave_type_exp,
				   strlen(slave_type_exp)) == 0) {
			sli->type = KVM_TYPE_SLAVE_EXP;
		} else {
			sli->type = KVM_TYPE_SLAVE_INVALID;
		}

		snprintf(debug_str, MAX_STR_LEN,
			 "%s name: %s port_a: %d port_b: %d num_ports: %d type: %d",
			 __func__, sli->name, sli->port_a, sli->port_b,
			 sli->num_ports, (int)sli->type);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		
		*slist = libcyc_ll_append(*slist, sli);
		sli = NULL;
	}
	
	fclose(fp);
}

/* This function returns the primary (or A) connection port for a slave device
   name specified as an argument */
int libcyc_kvm_slave_get_port_a_from_name(struct cyc_info *cyc,
					  const char *name)
{
	int port = ERROR;
	struct kvm_slave_info *sli = NULL;

	if (cyc == NULL) return(ERROR);
	if (name == NULL) return(ERROR);

	if (libcyc_update_list(cyc, KVMD_SLAVES_CONFFILE,
			&(cyc->kvm.kvmd_slave_conf_file_md5sum),
			libcyc_kvm_parse_kvmd_slaves_conf,
			&(cyc->kvm.slave_data))) return(ERROR);

	
	while ((sli = (struct kvm_slave_info *)
		libcyc_ll_iterate(cyc->kvm.slave_data))) {
		if (strncmp(name, sli->name,
			    (strlen(sli->name) < KVM_ALIAS_LEN) ?
			    strlen(sli->name)+1 : KVM_ALIAS_LEN) == 0) {
			port = sli->port_a;
		}
	}
	
	return(port);
}

const char *libcyc_kvm_slave_get_name_from_port_a(struct cyc_info *cyc,
						  int port_a)
{
	char *slave_name = NULL;
	struct kvm_slave_info *sli = NULL;

	if (cyc == NULL) return(NULL);
	if (port_a == ERROR) return(NULL);

	if (libcyc_update_list(cyc, KVMD_SLAVES_CONFFILE,
			&(cyc->kvm.kvmd_slave_conf_file_md5sum),
			libcyc_kvm_parse_kvmd_slaves_conf,
			&(cyc->kvm.slave_data))) return(NULL);

	
	while ((sli = (struct kvm_slave_info *)
		libcyc_ll_iterate(cyc->kvm.slave_data))) {
		if (port_a == sli->port_a) {
			slave_name = sli->name;
		}
	}
	
	return(slave_name);
}

/* This function returns the secondary (or B) connection port for a slave
   device name specified as an argument */
int libcyc_kvm_slave_get_port_b_from_name(struct cyc_info *cyc,
					  const char *name)
{
	char debug_str[MAX_STR_LEN];
	int port = ERROR;
	struct kvm_slave_info *sli = NULL;

	if (cyc == NULL) return(ERROR);
	if (name == NULL) return(ERROR);

	snprintf(debug_str, MAX_STR_LEN, "%s name: %s", __func__, name);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	if (libcyc_update_list(cyc, KVMD_SLAVES_CONFFILE,
			&(cyc->kvm.kvmd_slave_conf_file_md5sum),
			libcyc_kvm_parse_kvmd_slaves_conf,
			&(cyc->kvm.slave_data))) return(ERROR);

	
	while ((sli = (struct kvm_slave_info *)
		libcyc_ll_iterate(cyc->kvm.slave_data))) {
		if (strncmp(name, sli->name,
			    (strlen(sli->name) < KVM_ALIAS_LEN) ?
			    strlen(sli->name)+1 : KVM_ALIAS_LEN) == 0) {
			port = sli->port_b;
		}
	}
	
	return(port);
}

int libcyc_kvm_slave_get_num_ports_from_name(struct cyc_info *cyc,
					     const char *name)
{
	int num_ports = ERROR;
	struct kvm_slave_info *sli = NULL;

	if (cyc == NULL) return(ERROR);
	if (name == NULL) return(ERROR);

	if (libcyc_update_list(cyc, KVMD_SLAVES_CONFFILE,
			&(cyc->kvm.kvmd_slave_conf_file_md5sum),
			libcyc_kvm_parse_kvmd_slaves_conf,
			&(cyc->kvm.slave_data))) return(ERROR);

	
	while ((sli = (struct kvm_slave_info *)
		libcyc_ll_iterate(cyc->kvm.slave_data))) {
		if (strncmp(name, sli->name,
			    (strlen(sli->name) < KVM_ALIAS_LEN) ?
			    strlen(sli->name)+1 : KVM_ALIAS_LEN) == 0) {
			num_ports = sli->num_ports;
		}
	}
	
	return(num_ports);
}

char *libcyc_kvm_get_slave_name_from_alias(struct cyc_info *cyc,
					   const char *alias)
{
	const char casc_delim = KVM_CASC_DELIMITER;
	char *slave_name = NULL;
	char *tmp;

	if (cyc == NULL) return(NULL);
	if (alias == NULL) return(NULL);

	slave_name = libcyc_strdup(cyc, alias);

	if ((tmp = strrchr(slave_name, casc_delim)) == NULL) {
		libcyc_free(cyc, slave_name);
		slave_name = NULL;
		return(NULL);
	}

	*tmp = '\0';

	return(slave_name);
}

int libcyc_kvm_get_slave_port_from_alias(struct cyc_info *cyc,
					 const char *alias)
{
	const char casc_delim = KVM_CASC_DELIMITER;
	char *tmp;

	if (cyc == NULL) return(ERROR);
	if (alias == NULL) return(ERROR);

	if ((tmp = strrchr(alias, casc_delim)) == NULL) {
		return(ERROR);
	}

	tmp++;

	return(atoi(tmp));
}

int libcyc_kvm_get_slave_port_a_from_physport_str(struct cyc_info *cyc,
						  const char *physport_str)
{
	const char casc_delim = KVM_CASC_DELIMITER;
	char *t_pport_str = NULL, *tmp;
	int slave_porta = ERROR;

	if (cyc == NULL) return(ERROR);
	if (physport_str == NULL) return(ERROR);

	t_pport_str = libcyc_strdup(cyc, physport_str);

	if ((tmp = strrchr(t_pport_str, casc_delim)) == NULL) {
		libcyc_free(cyc, t_pport_str);
		t_pport_str = NULL;
		return(ERROR);
	}

	*tmp = '\0';

	slave_porta = atoi(t_pport_str);

	libcyc_free(cyc, t_pport_str);
	t_pport_str = NULL;

	return(slave_porta);
}

int libcyc_kvm_get_slave_port_from_physport_str(struct cyc_info *cyc,
						const char *physport_str)
{
	return(libcyc_kvm_get_slave_port_from_alias(cyc, physport_str));
}

int libcyc_kvm_session_active_check(struct cyc_info *cyc, libcyc_list **slist,
				    const char *alias)
{
	struct session_info *sinfo;
	int ret = FALSE;
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return(ERROR);
	if (alias == NULL) return(ERROR);

	/* If there's nothing in our session linked list, there are
	   no sessions */
	if ((slist == NULL) || (*slist == NULL)) return(FALSE);

	snprintf(debug_str, MAX_STR_LEN, "%s entering", __func__);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);
	
	while ((sinfo = (struct session_info *)libcyc_ll_iterate(*slist))) {
		if (strncmp(sinfo->to_alias, alias,
			    (strlen(alias) < KVM_ALIAS_LEN) ?
			    strlen(alias)+1 : KVM_ALIAS_LEN) == 0) {
			ret = TRUE;
		}
	}
	
	return(ret);
}

static void parse_pmcommand_outlet(struct cyc_info *cyc, char *buf, int buf_sz,
				   void *handle)
{
	struct pmcommand_parse_info *pminfo =
		(struct pmcommand_parse_info *)handle;
	libcyc_list **pmstat_list = NULL;
	struct power_info *pinfo = NULL;
	enum pmcommand_parse_state *state = NULL;
	int outlet = ERROR;
	const char *needle = "Outlet";
	const char *locked_str = "Locked";
	const char *unlocked_str = "Unlocked";
	const char *on_str = "ON";
	const char *off_str = "OFF";
	char debug_str[MAX_STR_LEN];

	if ((cyc == NULL) || (buf == NULL)) return;
	if (pminfo == NULL) return;

	pmstat_list = pminfo->plist;
	state = &(pminfo->state);
	outlet = pminfo->outlet;

	snprintf(debug_str, MAX_STR_LEN, "%s state: %d outlet: %d",
		 __func__, *state, outlet);
        debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	if (*state == PMC_STATE_PARSE_READY) {
		char outlet_str[OUTLET_STR_LEN];
		
		snprintf(outlet_str, sizeof(outlet_str), "%d", outlet);
		outlet_str[sizeof(outlet_str)-1] = '\0';

		/* Make sure that pmCommand gave us the outlet that
		   we asked for */
		if (strstr(buf, outlet_str) == NULL) return;

		pinfo = libcyc_malloc(cyc, sizeof(struct power_info));
		pinfo->state = 0;
		pinfo->outlet = outlet;

		if (strstr(buf, locked_str)) {
			pinfo->state |= POWER_STATE_LOCKED;
		} else if (strstr(buf, unlocked_str)) {
			pinfo->state |= POWER_STATE_UNLOCKED;
		}

		if (strstr(buf, on_str)) {
			pinfo->state |= POWER_STATE_ON;
		} else if (strstr(buf, off_str)) {
			pinfo->state |= POWER_STATE_OFF;
		}
		
		*pmstat_list = libcyc_ll_append(*pmstat_list, pinfo);
	} else if (strstr(buf, needle)) {
		*state = PMC_STATE_PARSE_READY;
	}
}

static int get_pmcommand_outlet_info(struct cyc_info *cyc,
				     libcyc_list **pinfo_list,
				     int outlet)
{
	char outlet_str[OUTLET_STR_LEN];
	char *p_argv[] = { "pmCommand", "1", "status", outlet_str };
	struct pmcommand_parse_info pminfo;
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return(ERROR);
	if (pinfo_list == NULL) return(ERROR);
	if (outlet <= 0) return(ERROR);

	snprintf(outlet_str, OUTLET_STR_LEN, "%d", outlet);
	outlet_str[OUTLET_STR_LEN-1] = '\0';
	
	snprintf(debug_str, MAX_STR_LEN, "%s outlet: %d", __func__, outlet);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	pminfo.plist = pinfo_list;
	pminfo.state = PMC_STATE_PARSE_ERROR;
	pminfo.outlet = outlet;

	/* Populate the pinfo_list structure */
	libcyc_system_pipe(cyc, PM_COMMAND, p_argv, parse_pmcommand_outlet,
			   &(pminfo));

	return(0);
}

/* This function gets the port status information for the KVM and populates a
   linked list containing the data about each port.  See the kvminfo
   utility for the use of this function call.
*/
int libcyc_kvm_port_status_info_get(struct cyc_info *cyc,
				    libcyc_list **pstat_list,
				    int *num_ports)
{
	char debug_str[MAX_STR_LEN];
	struct cyc_port_status *pstat = NULL;
	struct kvmd_config *kcfg = NULL;
	struct cyc_port_info *pai = NULL;
	libcyc_list *slist = NULL;
	libcyc_list *tmp_plist = NULL;
	int num_sessions = 0;

	if (cyc == NULL) return(ERROR);
	if ((pstat_list == NULL) || (num_ports == NULL)) return(ERROR);

	snprintf(debug_str, MAX_STR_LEN, "%s entering", __func__);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	/* Get an updated list of aliases/ports */
	if (libcyc_update_list(cyc, KVMD_CONFFILE, &(cyc->conf_file_md5sum),
			libcyc_kvm_parse_kvmd_conf, &(cyc->conf_data)))
		return(ERROR);

	/* Get the updated cascading information */
	if (libcyc_update_list(cyc, KVMD_SLAVES_CONFFILE,
			&(cyc->kvm.kvmd_slave_conf_file_md5sum),
			libcyc_kvm_parse_kvmd_slaves_conf,
			&(cyc->kvm.slave_data))) return(ERROR);

	/* Retrieve session information */
	libcyc_kvm_session_info_get(cyc, &slist, &num_sessions);
	
	/* Build the port status linked list */
	while ((kcfg = (struct kvmd_config *)
		libcyc_ll_iterate(cyc->conf_data))) {
		struct power_outlet *poutlet = NULL;
		struct cyc_port_info *pinfo = NULL;

		if (! *(kcfg->alias)) continue;

		pstat = libcyc_malloc(cyc, sizeof(struct cyc_port_status));
		pstat->pinfo = NULL;

		/* Virtual port */
		pstat->vport = kcfg->vport;

		/* First pass - physical port */
		pinfo = libcyc_malloc(cyc, sizeof(struct cyc_port_info));
		strncpy(pinfo->alias, kcfg->alias, KVM_ALIAS_LEN);
		pinfo->alias[KVM_ALIAS_LEN-1] = '\0';
		strncpy(pstat->alias, kcfg->alias, KVM_ALIAS_LEN);
		pstat->alias[KVM_ALIAS_LEN-1] = '\0';
		tmp_plist = libcyc_ll_append(tmp_plist, pinfo);

		/* Operation status (mapped to KVM port active/inactive) */
		pstat->op_stat = kcfg->active ? RESPONDING : NOT_RESPONDING;

		/* Session status */
		pstat->sess_stat =
			(libcyc_kvm_session_active_check(cyc, &slist,
							 kcfg->alias)) ?
			ACTIVE : INACTIVE;
		
		/* Power */
		while ((poutlet = (struct power_outlet *)
			libcyc_ll_iterate(kcfg->outlets))) {
			get_pmcommand_outlet_info(cyc, &(pstat->pinfo),
						  poutlet->outlet);
		}

		/* Protocol */
		pstat->protocol = kcfg->active ? kcfg->type : PORT_TYPE_UNKNOWN;

		*pstat_list = libcyc_ll_append(*pstat_list, pstat);
	}

	libcyc_kvm_session_info_destroy(cyc, &slist);
	
	*num_ports = libcyc_ll_get_size(*pstat_list);

	libcyc_add_flag(cyc, CYC_CONF_NOUPDATE);
	/* Second pass - populate the physical port */
	while ((pai = (struct cyc_port_info *)
		libcyc_ll_iterate(tmp_plist))) {
		char *physport_str = libcyc_kvm_get_physport_from_alias(cyc, pai->alias);

		if (physport_str) {
			strncpy(pai->physport, physport_str, PHYSPORT_STR_LEN);
			pai->physport[PHYSPORT_STR_LEN-1] = '\0';
			
			libcyc_free(cyc, physport_str);
			physport_str = NULL;
		}
	}
	libcyc_del_flag(cyc, CYC_CONF_NOUPDATE);
	
	/* Third pass - update the port status list with the physical port */
	while ((pstat = (struct cyc_port_status *)
		libcyc_ll_iterate(*pstat_list))) {
		while ((pai = (struct cyc_port_info *)
			libcyc_ll_iterate(tmp_plist))) {
			if (strncmp(pai->alias, pstat->alias,
				    (strlen(pai->alias) < KVM_ALIAS_LEN) ?
				    strlen(pai->alias)+1 : KVM_ALIAS_LEN) == 0) {
				/* We have the right record */
				strncpy(pstat->physport, pai->physport,
					PHYSPORT_STR_LEN);
				pstat->physport[PHYSPORT_STR_LEN-1] = '\0';
			}
		}
	}

	/* Free tmp list */
	while ((pai = (struct cyc_port_info *)
		libcyc_ll_iterate(tmp_plist))) {
		libcyc_free(cyc, pai);
		pai = NULL;
	}
	libcyc_ll_free(tmp_plist);
	tmp_plist = NULL;
	       
	return(0);
}

/* When we're done with the session information, free it */
void libcyc_kvm_port_status_info_destroy(struct cyc_info *cyc,
					 libcyc_list **pstat_list)
{
	struct cyc_port_status *pstat = NULL;

	if ((pstat_list == NULL) || (*pstat_list == NULL)) return;

	while ((pstat = (struct cyc_port_status *)
		libcyc_ll_iterate(*pstat_list))) {
		struct power_info *pinfo = NULL;

		while ((pinfo = (struct power_info *)
			libcyc_ll_iterate(pstat->pinfo))) {
			libcyc_free(cyc, pinfo);
			pinfo = NULL;
		}

		libcyc_ll_free(pstat->pinfo);
		pstat->pinfo = NULL;

		libcyc_free(cyc, pstat);
		pstat = NULL;
	}

	libcyc_ll_free(*pstat_list);
	*pstat_list = NULL;
}

/* These are implemented as macros (see cyc.h) unless CYC_KVM_REAL_FNS
   is defined */
#ifdef CYC_REAL_DATA_FNS
enum kvm_model libcyc_kvm_get_model(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (cyc->kvm == NULL)) return(ERROR);

	if (cyc->kvm.model != ERROR) return(cyc->kvm.model);

	return(ERROR);
}

enum kvmip_id libcyc_kvm_get_ip_modules_map(struct cyc_info *cyc)
{
	char debug_str[MAX_STR_LEN];

	if ((cyc == NULL) || (cyc->kvm == NULL)) return(ERROR);

	if (cyc->kvm.ip_module_map != ERROR) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s ip_module_map: 0x%x",
			 __func__,cyc->kvm.ip_module_map);
		libcyc_debug(cyc, debug_str);
		return(cyc->kvm.ip_module_map);
		
	}

	return(ERROR);
}

enum kvm_ports libcyc_kvm_get_ports(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (cyc->kvm == NULL)) {
		return(ERROR);
	}

	if (cyc->kvm.ports != ERROR) return(cyc->kvm.ports);

	return(ERROR);
}

int libcyc_kvm_get_num_ip_modules(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (cyc->kvm == NULL)) {
		return(ERROR);
	}

	return(cyc->kvm.num_ip_modules);
}

int libcyc_kvm_get_max_ip_modules(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (cyc->kvm == NULL)) {
		return(ERROR);
	}

	return(cyc->kvm.max_ip_modules);
}

int libcyc_kvm_get_num_stations(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (cyc->kvm == NULL)) {
		return(ERROR);
	}

	if (cyc->flags & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];
		snprintf(debug_str, MAX_STR_LEN, "%s stations: %d",
		         __func__, cyc->kvm.num_stations);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}

	return(cyc->kvm.num_stations);
}

int libcyc_kvm_get_max_fans(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (cyc->kvm == NULL)) {
		return(ERROR);
	}

	return(cyc->kvm.max_fans);
}

char *libcyc_kvm_get_serno(struct cyc_info *cyc)
{
	return(libcyc_prod_get_serno(cyc));
}

char *libcyc_kvm_get_appserno(struct cyc_info *cyc)
{
	return(libcyc_prod_get_appserno(cyc));
}

int libcyc_kvm_get_bd_ver(struct cyc_info *cyc)
{
	return(libcyc_prod_get_bd_ver(cyc));
}

int libcyc_kvm_get_fpga_ver(struct cyc_info *cyc)
{
	return(libcyc_prod_get_fpga_ver(cyc));
}
#endif
