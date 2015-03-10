/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_main.c
 *
 * Copyright (C) 2003-2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_main
 *
 ****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#include <cyc.h>
#include <products.h>

static void parse_bdinfo(struct cyc_info *cyc);
static int lock(struct cyc_info *);
static int unlock(struct cyc_info *);

static int lock(struct cyc_info *cyc)
{
        int fd;
	char pid_str[MAX_STR_LEN];
        pid_t pid=ERROR;
	char *filepath = NULL;

	if (cyc == NULL) return(ERROR);

	if (cyc->flags & CYC_DAEMON) {
		filepath = cyc->lock_daemon_filepath;
	} else if (cyc->flags & CYC_CGI) {
		filepath = cyc->lock_cgi_filepath;
	} else {
		filepath = cyc->lock_filepath;
	}

	if (libcyc_file_exist(filepath) == TRUE) {
		int msgid = 0;
		pid = libcyc_get_pid_from_runfile(filepath);
		if (kill(pid,0) == -1) {
			unlink(filepath);
			/* The process was not die cleanly; remove 
			 * the message queue.  State: runfile exists,
			 * process dead */
			if ((msgid = msgget(pid, 0)) != -1) {
				msgctl(msgid, IPC_RMID, NULL);
			}
		}

	}
	if ((fd=open(filepath, O_CREAT|O_EXCL|O_WRONLY, 0644)) == -1) {
		perror("libcyc_lock: open");
		return(ERROR);
	}
	
	snprintf(pid_str, MAX_STR_LEN, "%d\n", getpid());
	write(fd, pid_str, strlen(pid_str));
        close(fd);

	return(0);
}

static int unlock(struct cyc_info *cyc)
{
	char *filepath = NULL;

	if (cyc == NULL) return(ERROR);

	if (cyc->flags & CYC_DAEMON) {
		filepath = cyc->lock_daemon_filepath;
	} else if (cyc->flags & CYC_CGI) {
		filepath = cyc->lock_cgi_filepath;
	} else {
		filepath = cyc->lock_filepath;
	}

	if (unlink(filepath) == -1) {
		perror("libcyc_unlock: unlink");
		return(ERROR);
	}
	
	return(0);
}

void libcyc_debug(struct cyc_info *cyc, const char *str)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_DEBUG))) return;

	if (cyc->flags & CYC_USE_STDERR) {
		fprintf(stderr, "%s\n", str);
	}
	if (cyc->flags & CYC_USE_SYSLOG) {
		syslog(LOG_DEBUG, "%s", str);
	}
	if (! (cyc->flags | (CYC_USE_STDERR|CYC_USE_SYSLOG))) {
		printf("%s\n", str);
	}
}

/* static void parse_bdinfo(void);
 * The purpose of this function is to parse /proc/bdinfo and to
 * populate the cyc->prod structure.  The /proc/bdinfo entry looks
 * like this for KVM/netPlus:
 * KVMP16!2:16:2:0!KVMP!36!Cyclades!AlterPath! KVM/net Plus!1!
 *    V_2.1.1a (Feb/28/06)#1!130!0!0!f61b9a0b0000!0!3!0!00602e019265!1!0!
 */
static void parse_bdinfo(struct cyc_info *cyc)
{
	char bdinfo_str[MAX_STR_LEN];
	char *tmp_str=NULL;
	char *line=NULL,*next=NULL;
	int index;
	FILE *fp = NULL;

	if (cyc == NULL) return;

	/* Read in /proc/bdinfo */
        if ((fp = fopen(PROC_BDINFO_FILE, "r")) == NULL) {
                perror("kvm_parse_bdinfo");
		return;
        }
        fread(bdinfo_str, MAX_STR_LEN, 1, fp);
        fclose(fp);

	bdinfo_str[MAX_STR_LEN-1] = '\0';
	tmp_str = strdup(bdinfo_str);
	tmp_str[strlen(tmp_str)-1] = '\0'; /* Remove newline */

	/* Some initial values */
	cyc->prod.nportsS = 0;
	cyc->prod.nportsA = 0;
	cyc->prod.nportsK = 0;
	cyc->prod.nportsM = 0;
	strncpy(cyc->prod.board_name, "UNKNOWN", BDINFO_STR_LEN);
	cyc->prod.board_name[BDINFO_STR_LEN-1] = '\0';
	cyc->kvm.model = ERROR;
	strcpy(cyc->prod.app_serial_number,"0000000000");

	/* Walk through bdinfo and populate the data structures with
	   the data that we find */
	for (index=0, line = tmp_str; line; index++, line = next) {
		next = strchr(line, PROC_BDINFO_DELIMITER);
		if (next) {
			*next++ = '\0';
		}
		switch (index) {
		case 0:
			strncpy(cyc->prod.board_name, line, BDINFO_STR_LEN);
			cyc->prod.board_name[BDINFO_STR_LEN-1] = '\0';
			break;
		case 1:
			sscanf(line, "%d:%d:%d:%d",
			       &cyc->prod.nportsA,
			       &cyc->prod.nportsK,
			       &cyc->prod.nportsM,
			       &cyc->prod.nportsS);
			break;
		case 2:
			strncpy(cyc->prod.name_code, line, BDINFO_STR_LEN);
			cyc->prod.name_code[BDINFO_STR_LEN-1] = '\0';
			break;
		case 3:
			cyc->prod.type = atoi(line);
			break;
		case 4:
			strncpy(cyc->prod.vendor_name, line, BDINFO_STR_LEN);
			cyc->prod.vendor_name[BDINFO_STR_LEN-1] = '\0';
			break;
		case 5:
			strncpy(cyc->prod.prod_line, line, BDINFO_STR_LEN);
			cyc->prod.prod_line[BDINFO_STR_LEN-1] = '\0';
			break;
		case 6:
			strncpy(cyc->prod.prod_name, line, BDINFO_STR_LEN);
			cyc->prod.prod_name[BDINFO_STR_LEN-1] = '\0';
			break;
		case 7:
			/* Not used */
			break;
		case 8:
			strncpy(cyc->prod.kernel_version, line,
				BDINFO_STR_LEN);
			cyc->prod.kernel_version[BDINFO_STR_LEN-1] = '\0';
			break;
		case 9:
			cyc->prod.cpuclock = atoi(line);
			break;
		case 10:
			cyc->prod.ramsize = atoi(line);
			break;
		case 11:
			cyc->prod.flashsize = atoi(line);
			break;
		case 12:
			strncpy(cyc->prod.serial_number, line, BDINFO_STR_LEN);
			cyc->prod.serial_number[BDINFO_STR_LEN-1] = '\0';
			break;
		case 13:
			cyc->prod.board_version = atoi(line+strlen(line)-1);
			break;
		case 14:
			cyc->prod.ipboard[0] = atoi(line);
			break;
		case 15:
			cyc->prod.ipboard[1] = atoi(line);
		case 16:
			strncpy(cyc->prod.mac_address, line, BDINFO_STR_LEN);
			cyc->prod.mac_address[BDINFO_STR_LEN-1] = '\0';
			break;
		case 17:
			/* Not used */
			break;
		case 18:
			/* Not used */
			break;
		case 19:
			/* appliance serial number */
			strncpy(cyc->prod.app_serial_number, line, BDINFO_STR_LEN);
			cyc->prod.app_serial_number[BDINFO_STR_LEN-1] = '\0';
			break;
		}
	}
	free(tmp_str);
	tmp_str = NULL;

	/* Take some actions based on the model type ID;
	   enum board_type describes the IDs for each model */
	switch (cyc->prod.type) {
	case MODEL_PR3K:
	case MODEL_PR2K:
	case MODEL_PR4K:
	case MODEL_PR1K:
	case MODEL_TS1K:
	case MODEL_TS2K:
	case MODEL_TS4H:
	case MODEL_TS8H:
	case MODEL_TS3K:
	case MODEL_TX1K:
	case MODEL_TX2K:
	case MODEL_PL1K:
	case MODEL_TS1H:
	case MODEL_NL1K:
	case MODEL_ACS16:
	case MODEL_ACS32:
	case MODEL_SM100:
	case MODEL_PR3500:
	case MODEL_TS110:
	case MODEL_ACS48:
	case MODEL_ACS4:
	case MODEL_ACS8:
	case MODEL_ACS1:
	case MODEL_AVCS1:
	case MODEL_AVCS4:
	case MODEL_AVCS8:
	case MODEL_AVCS16:
	case MODEL_AVCS32:
	case MODEL_AVCS48:
	case MODEL_ACS5001:
	case MODEL_ACS5004:
	case MODEL_ACS5008:
	case MODEL_ACS5016:
	case MODEL_ACS5032:
	case MODEL_ACS5048:
		break;
	case MODEL_KVM16:
		cyc->kvm.model=MODEL_KVM;
		cyc->kvm.ports=16;
		break;
	case MODEL_KVM32:
		cyc->kvm.model=MODEL_KVM;
		cyc->kvm.ports=32;
		break;
	case MODEL_KVMNET16:
		cyc->kvm.model=MODEL_KVMNET;
		cyc->kvm.ports=16;
		break;
	case MODEL_KVMNET32:
		cyc->kvm.model=MODEL_KVMNET;
		cyc->kvm.ports=32;
		break;
	case MODEL_ONSITE441:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=441;
		break;
	case MODEL_ONSITE481:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=481;
		break;
	case MODEL_ONSITE841:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=841;
		break;
	case MODEL_ONSITE881:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=881;
		break;
	case MODEL_ONSITE442:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=442;
		break;
	case MODEL_ONSITE482:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=482;
		break;
	case MODEL_ONSITE842:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=842;
		break;
	case MODEL_ONSITE882:
		cyc->kvm.model=MODEL_ONSITE;
		cyc->kvm.ports=882;
		break;
	case MODEL_KVMNETPLUS16:
		cyc->kvm.model=MODEL_KVMNET_PLUS;
		cyc->kvm.ports=16;
		break;
	case MODEL_KVMNETPLUS32:
		cyc->kvm.model=MODEL_KVMNET_PLUS;
		cyc->kvm.ports=32;
		break;
	}


	/* Fill in KVM data structures based on our
	   product type and values we collected */
	switch (cyc->kvm.model) {
	case MODEL_KVM:
		cyc->kvm.max_fans = KVM_MAX_FANS;
		cyc->kvm.max_ip_modules = KVM_MAX_IP_MODULES;
		cyc->kvm.num_stations = KVM_MAX_STATIONS;
		break;

	case MODEL_KVMNET:
		if (cyc->prod.ipboard[1]) {
			cyc->kvm.ip_module_map |= CARD_2;
			cyc->kvm.num_ip_modules++;
		}
		if (cyc->prod.ipboard[0]) {
			cyc->kvm.ip_module_map |= CARD_1;
			cyc->kvm.num_ip_modules++;
		}
		cyc->kvm.max_fans = KVMNET_MAX_FANS;
		cyc->kvm.max_ip_modules = KVMNET_MAX_IP_MODULES;
		cyc->kvm.num_stations = KVMNET_MAX_STATIONS;
		break;

	case MODEL_ONSITE:
		/* A bd_ver of 2 refers to the new OnSite HW
		   with non-blocking station access */
		if (cyc->prod.ipboard[1]) {
			cyc->kvm.ip_module_map |= CARD_2;
			cyc->kvm.num_ip_modules++;
		}
		if ((cyc->prod.board_version == 2 ||(cyc->kvm.ports % 10) > 1) &&
		    (cyc->prod.ipboard[0])) {
			cyc->kvm.ip_module_map |= CARD_1;
			cyc->kvm.num_ip_modules++;
		}
		
		cyc->kvm.ports /= 100;
		cyc->kvm.max_fans=ONSITE_MAX_FANS;
		cyc->kvm.max_ip_modules=ONSITE_MAX_IP_MODULES;
		if (cyc->prod.board_version == 2) {
			cyc->kvm.num_stations = cyc->kvm.num_ip_modules +
				ONSITE_LOCAL_USER_STATIONS;
		} else {
			cyc->kvm.num_stations = ONSITE_V1_MAX_STATIONS;
		}
		break;
	case MODEL_KVMNET_PLUS:
		if (cyc->prod.ipboard[0] & 8) {
			cyc->kvm.ip_module_map |= CARD_4;
			cyc->kvm.num_ip_modules++;
		}
		if (cyc->prod.ipboard[0] & 4) {
			cyc->kvm.ip_module_map |= CARD_3;
			cyc->kvm.num_ip_modules++;
		}
		if (cyc->prod.ipboard[0] & 2) {
			cyc->kvm.ip_module_map |= CARD_2;
			cyc->kvm.num_ip_modules++;
		}
		if (cyc->prod.ipboard[0] & 1) {
			cyc->kvm.ip_module_map |= CARD_1;
			cyc->kvm.num_ip_modules++;
		}
		cyc->kvm.max_fans=KVMNET_PLUS_MAX_FANS;
		cyc->kvm.max_ip_modules=KVMNET_PLUS_MAX_IP_MODULES;
		cyc->kvm.num_stations = cyc->kvm.num_ip_modules +
			KVMNET_PLUS_LOCAL_USER_STATIONS;
		break;
	case MODEL_KVMNET_MAX:
	case MODEL_PC_SIMULATION:
	case MODEL_KVMNET_4000:
	case MODEL_INVALID:
		break;
	}
}

struct cyc_info *libcyc_init(enum libcyc_flags flags,
			     const char *appname,
			     const char *appver,
			     const char *lockdir,
			     const char *tmpdir)
{
	struct cyc_info *cyc=
		(struct cyc_info *)calloc(1, sizeof(struct cyc_info));
	char *str;
	char lock_daemon_appname[MAX_STR_LEN];
	char lock_cgi_appname[MAX_STR_LEN];
	char debug_str[MAX_STR_LEN];
	int error=0;

	cyc->flags = flags;

	if (appname) strncpy(cyc->appname, appname, MAX_STR_LEN);
	cyc->appname[MAX_STR_LEN - 1] = '\0';

	if (appver) strncpy(cyc->appver, appver, MAX_STR_LEN-1);
	cyc->appver[MAX_STR_LEN - 1] = '\0';

	if (lockdir) strncpy(cyc->lockdir, lockdir, MAX_STR_LEN-1);
	cyc->lockdir[MAX_STR_LEN - 1] = '\0';

	if (tmpdir) strncpy(cyc->tmpdir, tmpdir, MAX_STR_LEN-1);
	cyc->tmpdir[MAX_STR_LEN - 1] = '\0';

	snprintf(lock_daemon_appname, MAX_STR_LEN, "%s-daemon", cyc->appname);
	lock_daemon_appname[MAX_STR_LEN-1] = '\0';
	snprintf(lock_cgi_appname, MAX_STR_LEN, "%s-cgi", cyc->appname);
	lock_cgi_appname[MAX_STR_LEN-1] = '\0';

	if ((str=libcyc_create_filepath(cyc, cyc->lockdir,
					lock_daemon_appname, LOCK_EXT))) {
		strncpy(cyc->lock_daemon_filepath, str, MAX_STR_LEN - 1);
		cyc->lock_daemon_filepath[MAX_STR_LEN-1] = '\0';
		libcyc_free(cyc,str);
		str=NULL;
	} else {
		error++;
		goto errors;
	}

	if ((str=libcyc_create_filepath(cyc, cyc->lockdir,
					lock_cgi_appname, LOCK_EXT))) {
		strncpy(cyc->lock_cgi_filepath, str, MAX_STR_LEN - 1);
		cyc->lock_cgi_filepath[MAX_STR_LEN-1] = '\0';
		libcyc_free(cyc,str);
		str=NULL;
	} else {
		error++;
		goto errors;
	}

	if ((str=libcyc_create_filepath(cyc, cyc->lockdir,
					cyc->appname, LOCK_EXT))) {
		strncpy(cyc->lock_filepath, str, MAX_STR_LEN - 1);
		cyc->lock_filepath[MAX_STR_LEN-1] = '\0';
		libcyc_free(cyc,str);
		str=NULL;
	} else {
		error++;
		goto errors;
	}

	if ((str=libcyc_create_filepath(cyc, cyc->tmpdir,
					cyc->appname, SOCK_EXT))) {
		strncpy(cyc->sock_filepath, str, MAX_STR_LEN - 1);
		cyc->sock_filepath[MAX_STR_LEN-1] = '\0';
		libcyc_free(cyc,str);
		str=NULL;
	} else {
		error++;
	}

	if (! (cyc->flags & CYC_NOLOCK)) {
		if (lock(cyc)) {
			error++;
			goto errors;
		}
	}

	/* Parse /proc/bdinfo and populate the product-specific
	   data structures */
	parse_bdinfo(cyc);
	libcyc_add_flag(cyc, CYC_INIT);

	if (libcyc_port_names_get_default(cyc, &(cyc->def_port_names)) == ERROR) {
		error++;
		goto errors;
	}


	if (libcyc_prod_is_kvm(cyc)) {
		libcyc_update_list(cyc, KVMD_CONFFILE,
				   &(cyc->conf_file_md5sum),
				   libcyc_kvm_parse_kvmd_conf,
				   &(cyc->conf_data));
		
		libcyc_update_list(cyc, KVMD_SLAVES_CONFFILE,
				   &(cyc->kvm.kvmd_slave_conf_file_md5sum),
				   libcyc_kvm_parse_kvmd_slaves_conf,
				   &(cyc->kvm.slave_data));
	}

 errors:
	if (error) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s: Problem initializing libcyc: %s", __func__, appname);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		free(cyc);
		cyc=NULL;
		return(NULL);
	}

	if (cyc->flags & CYC_USE_SYSLOG) {
		openlog(cyc->appname, LOG_PID, LOG_USER);
	}

	snprintf(debug_str, MAX_STR_LEN,
		 "%s libcyc initialized", __func__);
	libcyc_debug(cyc, debug_str);

	return(cyc);
}

void libcyc_destroy(struct cyc_info *cyc)
{
	char debug_str[MAX_STR_LEN];

	if (cyc && (cyc->flags & CYC_INIT)) {
		if (! (cyc->flags & CYC_NOLOCK)) {
			unlock(cyc);
		}
		snprintf(debug_str, MAX_STR_LEN,
			 "%s libcyc uninitialized: %s", __func__, cyc->appname);
		libcyc_debug(cyc, debug_str);

		if (cyc->flags & CYC_USE_SYSLOG) {
			closelog();
		}

		if (cyc->devs) {
			struct dev_info *dev = NULL;
			
			while ((dev = libcyc_ll_iterate(cyc->devs))) {
				close(dev->fd);
				if (libcyc_ipc_sem_getval(cyc, dev->semid, 0) < 1) {
					/* Release the sem */
					libcyc_ipc_sem_op(cyc, dev->semid, 0, 1);
				}
				libcyc_free(cyc, dev);
				dev = NULL;
			}
			libcyc_ll_free(cyc->devs);
			cyc->devs = NULL;
		}
		if (cyc->def_port_names) {
			libcyc_port_names_destroy_default(cyc, &(cyc->def_port_names));
		}
		if (cyc->conf_data) {
			libcyc_kvm_physport_alias_data_destroy(cyc, &(cyc->conf_data));
		}
		if (cyc->conf_file_md5sum) {
			libcyc_free(cyc, cyc->conf_file_md5sum);
			cyc->conf_file_md5sum = NULL;
		}
		if (cyc->kvm.kvmd_slave_conf_file_md5sum) {
			libcyc_free(cyc, cyc->kvm.kvmd_slave_conf_file_md5sum);
			cyc->kvm.kvmd_slave_conf_file_md5sum = NULL;
		}
		if (cyc->kvm.slave_data) {
			struct kvm_slave_info *sli = NULL;

			while ((sli = (struct kvm_slave_info *)
				libcyc_ll_iterate(cyc->kvm.slave_data))) {
				libcyc_free(cyc, sli);
				sli = NULL;
			}

			libcyc_ll_free(cyc->kvm.slave_data);
			cyc->kvm.slave_data = NULL;
		}

		libcyc_del_flag(cyc, CYC_INIT);
		libcyc_free_all(cyc);

		free(cyc);
		cyc=NULL;
	}
}

/* This function is deprecated; do not use this function for new code.
   The replacement for this function is parse_bdinfo and the
   associated set of accessor functions: libcyc_kvm_get_*,
   libcyc_apctl_get_*, and libcyc_prod_get_*.  Many of the accessor functions
   are implemented as macros by default so there isn't any more overhead
   than accessing the members of MachineType directly.  The benefit of
   this new standard API is that the API doesn't need to change if the
   data does. */
int get_machine_type(struct MachineType *pmt)
{
	char buffer[256], *p;
	int fd;

	pmt->nportsS = 0;
	pmt->nportsA = 0;
	pmt->nportsK = 0;
	pmt->nportsM = 0;
	strcpy(pmt->name, "UNKNOWN");

	if ((fd = open(PROC_BDINFO_FILE, O_RDONLY)) < 0) {
		return(0);
	}

	if (read(fd, buffer, sizeof(buffer)) <= 0) {
		close(fd);
		return(0);
	}

	close(fd);

	if ((p = strtok(buffer, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->board_name, p);

	//Port field is of the format Aports:Kports:Mports:Sports
	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	sscanf(p, "%d:%d:%d:%d",
		&pmt->nportsA,
		&pmt->nportsK,
		&pmt->nportsM,
		&pmt->nportsS);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->name, p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	pmt->board_type = atoi(p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->vendor_name, p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->prod_line, p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->prod_name, p);

	if ((p = strtok(NULL, "!")) == NULL) { // pcmcia
		return(0);
	}

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->version, p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	pmt->cpuclock = atoi(p);
	
	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	pmt->ramsize = atoi(p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	pmt->flashsize = atoi(p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->serial_number, p);
	
	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	//FPGA field is of the format FPGA_ver:Board_ver
	sscanf(p, "%x:%x",
		&pmt->fpga_version,
		&pmt->board_version);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	pmt->ipboard[0] = atoi(p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	pmt->ipboard[1] = atoi(p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->mac_address, p);

	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	/* appliance serial number */
	if ((p = strtok(NULL, "!")) == NULL) {
		return(0);
	}
	strcpy(pmt->app_serial_number, p);
	return(1);
}

/* This function adds a flag to the set and returns the current set */
enum libcyc_flags libcyc_add_flag(struct cyc_info *cyc, enum libcyc_flags flag)
{
	if (cyc) {
		cyc->flags |= flag;
		return(cyc->flags);
	}

	return(ERROR);
}

/* This function deletes a flag to the set and returns the current set */
enum libcyc_flags libcyc_del_flag(struct cyc_info *cyc, enum libcyc_flags flag)
{
	if (cyc) {
		cyc->flags &= ~flag;
		return(cyc->flags);
	}

	return(ERROR);
}

/* This function generates a list of default port names based on the
   MAC address and the physical port number.  A C++ implementation
   temporarily inserted into apctl as a "unit test" will give some clarity
   on how to use this function call:

	libcyc_list *slist = NULL;
	struct cyc_port_info *def_port = NULL;

	libcyc_port_names_get_default(a->get_cyc(), &slist);

        while ((def_port = static_cast<struct cyc_port_info *>
		(libcyc_ll_iterate(slist)))) {
		cout << "Physical port: " << def_port->port << endl;
		cout << "Default Alias: " << def_port->alias << endl;

		libcyc_free(a->get_cyc(), def_port);
		def_port = NULL;
        }
	
	libcyc_ll_free(slist);
	slist = NULL;
*/
int libcyc_port_names_get_default(struct cyc_info *cyc,
				  libcyc_list **pnames_list)
{
	char *mac_addr_str = NULL;
	int i, num_kports = 0, num_sports = 0;
	struct cyc_port_info *p = NULL;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (pnames_list == NULL) return(ERROR);

	if (libcyc_prod_is_kvm(cyc)) {
		/* KVM Family */
		num_kports = libcyc_kvm_get_ports(cyc);
	} else if (libcyc_prod_is_onsite(cyc)) {
		/* OnSite */
		num_sports = libcyc_prod_get_nportsS(cyc);
		num_kports = libcyc_kvm_get_ports(cyc);
	} else if (libcyc_prod_is_csORavcs(cyc)) {
		/* ACS */
		num_sports = libcyc_prod_get_nportsS(cyc);
	} else {
		return(ERROR);
	}

	if (! cyc->mac_last3[0]) {
		mac_addr_str = libcyc_iface_get_mac_delim(cyc, "eth0", '-');
		
		/* Copy last three octets (assume 6 octets total
		   plus dashes) */
		strncpy(cyc->mac_last3, mac_addr_str + 9,
			sizeof(cyc->mac_last3));
		cyc->mac_last3[8] = '\0';
		libcyc_free(cyc, mac_addr_str);
		mac_addr_str = NULL;
	}

	for (i = 1; i <= num_kports; i++) {
		p = libcyc_malloc(cyc, sizeof(struct cyc_port_info));
		p->port = i;
		p->type = PORT_TYPE_KVM;
		snprintf(p->alias, sizeof(p->alias),
			 "%sK%02d", cyc->mac_last3, i);
		p->alias[sizeof(p->alias)-1] = '\0';
		*pnames_list = libcyc_ll_append(*pnames_list, p);
	}
	for (i = 1; i <= num_sports; i++) {
		p = libcyc_malloc(cyc, sizeof(struct cyc_port_info));
		p->port = i;
		p->type = PORT_TYPE_SERIAL;
		snprintf(p->alias, sizeof(p->alias),
			 "%sP%02d", cyc->mac_last3, i);
		p->alias[sizeof(p->alias)-1] = '\0';
		*pnames_list = libcyc_ll_append(*pnames_list, p);
	}
	
	return(0);
}

int libcyc_kvm_port_names_get_default(struct cyc_info *cyc,
				  libcyc_list **pnames_list)
{
	char *mac_addr_str = NULL;
	int i, num_kports = 0;
	struct cyc_port_info *p = NULL;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (pnames_list == NULL) return(ERROR);

	if (libcyc_prod_is_kvm(cyc) || libcyc_prod_is_onsite(cyc)) {
		/* KVM and OnSite Family */
		num_kports = libcyc_kvm_get_ports(cyc);
	} else {
		return(ERROR);
	}

	if (! cyc->mac_last3[0]) {
		mac_addr_str = libcyc_iface_get_mac_delim(cyc, "eth0", '-');
		
		/* Copy last three octets (assume 6 octets total
		   plus dashes) */
		strncpy(cyc->mac_last3, mac_addr_str + 9,
			sizeof(cyc->mac_last3));
		cyc->mac_last3[8] = '\0';
		libcyc_free(cyc, mac_addr_str);
		mac_addr_str = NULL;
	}

	for (i = 1; i <= num_kports; i++) {
		p = libcyc_malloc(cyc, sizeof(struct cyc_port_info));
		p->port = i;
		p->type = PORT_TYPE_KVM;
		snprintf(p->alias, sizeof(p->alias),
			 "%sK%02d", cyc->mac_last3, i);
		p->alias[sizeof(p->alias)-1] = '\0';
		*pnames_list = libcyc_ll_append(*pnames_list, p);
	}
	
	return(0);
}

/* When we're done with the list of default names, free them */
void libcyc_port_names_destroy_default(struct cyc_info *cyc,
				      libcyc_list **pnames_list)
{
	libcyc_list *slist = NULL;
	struct cyc_port_info *def_port = NULL;
	
	if (cyc == NULL) return;
	if (pnames_list == NULL) return;
	
	slist = *pnames_list;
	
        while ((def_port = (struct cyc_port_info *)libcyc_ll_iterate(slist))) {
		libcyc_free(cyc, def_port);
		def_port = NULL;
        }
	
	libcyc_ll_free(slist); 
	slist = NULL; 
} 

/* Return a single default port name that corresponds to the physical port */
const char *libcyc_port_name_get_default_list(struct cyc_info *cyc,
	                                      libcyc_list **pnames_list,
					      int port, enum port_type type)
{
	libcyc_list *slist = NULL;
	struct cyc_port_info *def_port = NULL;
	char *pname_str = NULL;
	
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);
	if (pnames_list == NULL) return(NULL);
	
	slist = *pnames_list;
	
        while ((def_port = (struct cyc_port_info *)libcyc_ll_iterate(slist))) {
		if ((def_port->port == port) && (def_port->type == type)) {
			pname_str = def_port->alias;
		}
        }

	return(pname_str);
}

const char *libcyc_port_name_get_default(struct cyc_info *cyc,
					 int port, enum port_type type)
{
	const char *pname;

	if ((cyc == NULL) || (cyc->def_port_names == NULL)) return(NULL);
	
	pname = libcyc_port_name_get_default_list(cyc, &(cyc->def_port_names),
						  port, type);
	
	return(pname);
}

/* Returns true if the model is a type of ACS */
int libcyc_prod_is_cs(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_prod_get_bd_type(cyc);

	if ((model == MODEL_ACS1) ||
	    (model == MODEL_ACS4) ||
	    (model == MODEL_ACS8) ||
	    (model == MODEL_ACS16) ||
	    (model == MODEL_ACS32) ||
	    (model == MODEL_ACS48)) {
		return(TRUE);
	}

	return(FALSE);
}

/* Returns true if the model is a type of AutoView CS */
int libcyc_prod_is_avcs(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_prod_get_bd_type(cyc);

	if ((model == MODEL_AVCS1) ||
	    (model == MODEL_AVCS4) ||
	    (model == MODEL_AVCS8) ||
	    (model == MODEL_AVCS16) ||
	    (model == MODEL_AVCS32) ||
	    (model == MODEL_AVCS48)) {
		return(TRUE);
	}

	return(FALSE);
}

/* Returns true if the model is a type of ACS5xxx */
int libcyc_prod_is_acs5k(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_prod_get_bd_type(cyc);

	if ((model == MODEL_ACS5001) ||
	    (model == MODEL_ACS5004) ||
	    (model == MODEL_ACS5008) ||
	    (model == MODEL_ACS5016) ||
	    (model == MODEL_ACS5032) ||
	    (model == MODEL_ACS5048)) {
		return(TRUE);
	}

	return(FALSE);
}

/* Returns true if the model is a type of ACS or AutoView CS */
int libcyc_prod_is_csORavcs(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_prod_get_bd_type(cyc);

	if ((model == MODEL_ACS1) ||
	    (model == MODEL_ACS4) ||
	    (model == MODEL_ACS8) ||
	    (model == MODEL_ACS16) ||
	    (model == MODEL_ACS32) ||
	    (model == MODEL_ACS48) ||
		(model == MODEL_AVCS1) ||
	    (model == MODEL_AVCS4) ||
	    (model == MODEL_AVCS8) ||
	    (model == MODEL_AVCS16) ||
	    (model == MODEL_AVCS32) ||
	    (model == MODEL_AVCS48) ||
	    (model == MODEL_ACS5001) ||
	    (model == MODEL_ACS5004) ||
	    (model == MODEL_ACS5008) ||
	    (model == MODEL_ACS5016) ||
	    (model == MODEL_ACS5032) ||
	    (model == MODEL_ACS5048)) {
		return(TRUE);
	}

	return(FALSE);
}

/* Returns true if the model is a type of OnSite */
int libcyc_prod_is_onsite(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_prod_get_bd_type(cyc);

	if ((model == MODEL_ONSITE441) ||
	    (model == MODEL_ONSITE481) ||
	    (model == MODEL_ONSITE841) ||
	    (model == MODEL_ONSITE881) ||
	    (model == MODEL_ONSITE442) ||
	    (model == MODEL_ONSITE482) ||
	    (model == MODEL_ONSITE842) ||
	    (model == MODEL_ONSITE882)) {
		return(TRUE);
	}

	return(FALSE);
}

/* Returns true if the model is a type of KVM */
int libcyc_prod_is_kvm(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_prod_get_bd_type(cyc);

	if ((model == MODEL_KVMNET16) ||
	    (model == MODEL_KVMNET32) ||
	    (model == MODEL_KVMNETPLUS16) ||
	    (model == MODEL_KVMNETPLUS32) ||
	    (model == MODEL_KVM16) ||
	    (model == MODEL_KVM32)) {
		return(TRUE);
	}

	return(FALSE);
}

const char *libcyc_prod_get_model_str(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	model = libcyc_prod_get_bd_type(cyc);

        if (model != ERROR) return(AP_MODEL[model]);
        
        return(NULL);
}

const char *libcyc_prod_get_prodcode_str(struct cyc_info *cyc)
{
	enum board_type model;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	model = libcyc_prod_get_bd_type(cyc);

        if (model != ERROR) return(AP_PRODCODE[model]);
        
        return(NULL);
}

/* Check the cyc.h header; these are implemented as macros unless
   CYC_KVM_REAL_FNS is defined */
#ifdef CYC_REAL_DATA_FNS
enum board_type libcyc_prod_get_bd_type(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(cyc->prod.type);
}

const char *libcyc_get_ver(void)
{
	return(LIBCYC_VERSION);
}

enum libcyc_flags libcyc_get_flags(struct cyc_info *cyc)
{
	return(cyc ? cyc->flags : ERROR);
}

const char *libcyc_prod_get_serno(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	if (cyc->prod.serial_number != ERROR) return(cyc->prod.serial_number);

	return(NULL);
}

const char *libcyc_prod_get_appserno(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	if (cyc->prod.app_serial_number != ERROR) return(cyc->prod.app_serial_number);

	return(NULL);
}

int libcyc_prod_get_bd_ver(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(cyc->prod.board_version);
}

int libcyc_prod_get_fpga_ver(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(cyc->prod.fpga_version);
}

int libcyc_prod_get_nportsS(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(cyc->prod.nportsS);
}
#endif
