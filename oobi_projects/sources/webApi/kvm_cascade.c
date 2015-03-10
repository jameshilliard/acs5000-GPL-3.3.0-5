/**
 *  kvm_cascade.c  -  Parses and writes KVM/net cascading information
 *  Copyright (C), 2004 Cyclades Corporation
 *
 *  Written by:
 *          Rodrigo Parra Novo <rodrigo.novo@cyclades.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 *  This file is part of webApi
 */

static char *RCSid __attribute__ ((unused)) = "$Id: kvm_cascade.c,v 1.10 2007/08/28 15:46:49 sergio Exp $ ";

/**
 * global includes
 */
#include <libgen.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <asm/prxk.h>

#include <cyc.h>
#include <event_lib.h>

extern struct cyc_info *cyc;

/**
 * local includes
 */
#include "kvm_cascade.h"

/**
 * local structures (unused outside this file)
 */
struct _port_config_line_t {
	int port;
	int gport;
	char *alias;
	char *lockoutmacro;
	char *active;
	char *device;
	char *pmoutletstr;
	char *pmdevice1;
	char *pmdevice2;
	int pmoutlet1;
	int pmoutlet2;
#ifdef KVMTCPport
	int tcpport;
#endif
	int lclagc;
	int lclbright;
	int lclcontr;
	int remagc;
	int rembright;
	int remcontr;
	int ipbright[4];
	int ipcontr[4];
};

/**
 * locally defined support functions
 */
static char *line_cleanup (char *line);
static struct kvm_cascade_dev_t *kvm_cascade_parse_line (kvm_cascade kc, char *line);
static int get_port (struct _port_config_line_t *ports, int port, int numports);
static int kvm_cascade_configure_cascades (kvm_cascade kc);
static int read_proc_bdinfo (void);

/**
 * this is a kludge to support optional parameters to kvm_cascade_strerror ()
 */
static char *_kvm_cascade_strerror_message = NULL;
static void kvm_cascade_strerror_set (char *format, ...);

/**
 * kvm_cascade kvm_cascade_new (void)
 *
 * Parameters
 *   None
 *
 * Return Values
 *   kvm_cascade : a newly allocated (and empty) kvm_cascade object
 *   NULL : in case there is no memory to allocate the object
 *
 * External Resources
 *   None
 *
 * Bugs / Environment changes
 *   None
 *
 * Comments
 *   The newly allocated object must be freed later, issuing a call to
 *   kvm_cascade_free ()
 */
kvm_cascade kvm_cascade_new (void)
{
	kvm_cascade kc = NULL;

	kc = (kvm_cascade) calloc ((size_t) 1, sizeof (struct kvm_cascade_t));
	if (kc == NULL) {
		return (NULL);
	}

	return (kc);
}

/**
 * int kvm_ports_config_write (kvm_cascade kc, char *config_file)
 *
 * Parameters
 *   kvm_cascade kc : the kvm_cascade object, previously filled with
 *           information, on a call to kvm_ports_config_read ()
 *   char *config_file : the configuration file to be written
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_cascade object not initialized
 *   KC_EFILE : could not open/write configuration file
 *   KC_EMEM : memory exhausted
 *   KC_EPARSE : 
 *
 * External Resources
 *   None
 *
 * Bugs / Environment changes
 *   The newly created configuration file
 *
 * Comments
 *   None
 */
int kvm_ports_config_write (kvm_cascade kc, char *config_file)
{
	int i;
	int j;
	int k;
	int rc;
	FILE *_f, *_f2;
	struct kvm_cascade_dev_t *kcd;
	char tempfile[32];
	char buf[256], *p;

	if (kc == NULL) {
		return (KC_NULL);
	}

	snprintf(tempfile, 31, "%s.tmp", config_file);
	_f = fopen (tempfile, "w");
	if (_f == NULL) {
		kvm_cascade_strerror_set ("Could not open %s for writing.", config_file);
		return (KC_EFILE);
	}
	_f2 = fopen (config_file, "r");
#if 0
	// this can be deleted - all these will be written using kc->extra_config
	// fixes: #2186 (oliver)
	if (_f2) {
		while(fgets(buf, 255, _f2)){
			p = buf;
			while(*p && isspace(*p)) p++; // skip spaces
			if (*p != 'p') {
				fprintf(_f, buf);
			}
 		}
                fclose(_f2);
	}
#endif
	if (kc->extra_config) 
		fprintf (_f, "%s", kc->extra_config);

	/**
	 * now starts our own config writing
	 */
	j = 1;
	kcd = kc->head;
	/**
	 * first we write the configuration for all local ports
	 */
	for (i = 0; i < kcd->numports; i++) {
		/**
		 * first we find out if this port is cascaded, and in this case, write only
		 * commented code for it
		 */
		if ((kcd->ports[i].active != NULL) && (strcmp (kcd->ports[i].active, "cascade")) == 0) {
			/* port "i + 1" is cascaded */
			continue;
		}
		fprintf (_f, "p%d.port %d\n", j, i + 1);

		if (kcd->ports[i].alias == NULL) {
			if (!strcmp(kcd->name,"master")) {
				const char *port_str = libcyc_port_name_get_default(cyc, i+1, PORT_TYPE_KVM);
				strncpy(buf, port_str, 20);
				buf[20] = '\0';
			} else {
				sprintf(buf,"%s.%d",kcd->name,i+1);
			}
			kcd->ports[i].alias = strdup(buf);
		}

		if (kcd->ports[i].active != NULL) {
			fprintf (_f, "p%d.active %s\n", j, kcd->ports[i].active);
		}
		if (kcd->ports[i].alias != NULL) {
			fprintf (_f, "p%d.alias %s\n", j, kcd->ports[i].alias);
		}

		if (kcd->ports[i].lockoutmacro != NULL) {
			fprintf (_f, "p%d.lockoutmacro %s\n", j, kcd->ports[i].lockoutmacro);
		}

/*
		if ((kcd->ports[i].pmoutlet1 != 0) && (kcd->ports[i].pmdevice1 != NULL)) {
			if (!strcmp(kcd->ports[i].pmdevice1,"master")) {
				fprintf (_f, "p%d.pmoutlet %d", j, kcd->ports[i].pmoutlet1);
			} else {
				fprintf (_f, "p%d.pmoutlet %s.%d", j, kcd->ports[i].pmdevice1,kcd->ports[i].pmoutlet1);
			}
			if ((kcd->ports[i].pmoutlet2 != 0) && (kcd->ports[i].pmdevice2 != NULL)) {
				if (!strcmp(kcd->ports[i].pmdevice2,"master")) {
					fprintf (_f, ",%d", kcd->ports[i].pmoutlet2);
				} else {
					fprintf (_f, ",%s.%d", kcd->ports[i].pmdevice2,kcd->ports[i].pmoutlet2);
				}
			}
			fprintf (_f, "\n");
		}
*/
		if ((kcd->ports[i].pmoutletstr != NULL) && (kcd->ports[i].pmoutletstr[0] != 0)) {
			fprintf (_f, "p%d.pmoutlet %s", j, kcd->ports[i].pmoutletstr);
			fprintf (_f, "\n");
		}

#ifdef KVMTCPport
		if (kcd->ports[i].tcpport > 0 && kcd->ports[i].tcpport < 0x10000) {
			fprintf (_f, "p%d.tcpport %d\n", j, kcd->ports[i].tcpport);
		}
#endif
		if (kcd->ports[i].lclagc) {
			fprintf (_f, "p%d.lclagc enable\n", j);
		} else {
			fprintf (_f, "p%d.lclagc disable\n", j);
		}
		fprintf(_f, "p%d.lclbright %d\n", j, kcd->ports[i].lclbright);
		fprintf(_f, "p%d.lclcontr %d\n", j, kcd->ports[i].lclcontr);
		if (kcd->ports[i].remagc) {
			fprintf (_f, "p%d.remagc enable\n", j);
		} else {
			fprintf (_f, "p%d.remagc disable\n", j);
		}
		fprintf(_f, "p%d.rembright %d\n", j, kcd->ports[i].rembright);
		fprintf(_f, "p%d.remcontr %d\n", j, kcd->ports[i].remcontr);
		for (k = 0; k < 4; k ++) {
			fprintf(_f, "p%d.ip%dbright %d\n", j, k+1, kcd->ports[i].ipbright[k]);
			fprintf(_f, "p%d.ip%dcontr %d\n", j, k+1, kcd->ports[i].ipcontr[k]);
		}
		j++;
	}

	kcd = kcd->next;
	/**
	 * now we write the configuration for all physically connected ports
	 */
	for (; kcd != NULL; kcd = kcd->next) {
		if (kcd->type == KVM_CASCADE_PHYSICAL) {
			for (i = 0; i < kcd->numports; i++) {
				/**
				 * first we find out if this port is cascaded, and in this case, write only
				 * commented code for it
				 */
				if ((kcd->ports[i].active != NULL) && (strcmp (kcd->ports[i].active, "cascade")) == 0) {
					/* port "i + 1" of switch "kcd->name" is cascaded */
					continue;
				}
				fprintf (_f, "p%d.port %s.%d\n", j, kcd->name, i + 1);
				if (kcd->ports[i].active != NULL) {
					fprintf (_f, "p%d.active %s\n", j, kcd->ports[i].active);
				}
				if (kcd->ports[i].alias != NULL) {
					fprintf (_f, "p%d.alias %s\n", j, kcd->ports[i].alias);
				}
				if (kcd->ports[i].lockoutmacro != NULL) {
					fprintf (_f, "p%d.lockoutmacro %s\n", j, kcd->ports[i].lockoutmacro);
				}

/*
				if ((kcd->ports[i].pmoutlet1 != 0) && (kcd->ports[i].pmdevice1 != NULL)) {
					if (!strcmp(kcd->ports[i].pmdevice1,"master")) {
						fprintf (_f, "p%d.pmoutlet %d", j, kcd->ports[i].pmoutlet1);
					} else {
						fprintf (_f, "p%d.pmoutlet %s.%d", j, kcd->ports[i].pmdevice1,kcd->ports[i].pmoutlet1);
					}
					if ((kcd->ports[i].pmoutlet2 != 0) && (kcd->ports[i].pmdevice2 != NULL)) {
						if (!strcmp(kcd->ports[i].pmdevice2,"master")) {
							fprintf (_f, ",%d", kcd->ports[i].pmoutlet2);
						} else {
							fprintf (_f, ",%s.%d", kcd->ports[i].pmdevice2,kcd->ports[i].pmoutlet2);
						}
					}
					fprintf (_f, "\n");
				}
*/
				if ((kcd->ports[i].pmoutletstr != NULL) && kcd->ports[i].pmoutletstr[0] != 0) {
					fprintf (_f, "p%d.pmoutlet %s", j, kcd->ports[i].pmoutletstr);
					fprintf (_f, "\n");
				}
#ifdef KVMTCPport
				if (kcd->ports[i].tcpport > 0 && kcd->ports[i].tcpport < 0x10000) {
					fprintf (_f, "p%d.tcpport %d\n", j, kcd->ports[i].tcpport);
				}
#endif
				if (kcd->ports[i].lclagc) {
					fprintf (_f, "p%d.lclagc enable\n", j);
				} else {
					fprintf (_f, "p%d.lclagc disable\n", j);
				}
				fprintf(_f, "p%d.lclbright %d\n", j, kcd->ports[i].lclbright);
				fprintf(_f, "p%d.lclcontr %d\n", j, kcd->ports[i].lclcontr);
				if (kcd->ports[i].remagc) {
					fprintf (_f, "p%d.remagc enable\n", j);
				} else {
					fprintf (_f, "p%d.remagc disable\n", j);
				}
				fprintf(_f, "p%d.rembright %d\n", j, kcd->ports[i].rembright);
				fprintf(_f, "p%d.remcontr %d\n", j, kcd->ports[i].remcontr);
				for (k = 0; k < 4; k ++) {
					fprintf(_f, "p%d.ip%dbright %d\n", j, k+1, kcd->ports[i].ipbright[k]);
					fprintf(_f, "p%d.ip%dcontr %d\n", j, k+1, kcd->ports[i].ipcontr[k]);
				}
				j++;
			}
		}
	}

	kcd = kc->head;
	kcd = kcd->next;
	/**
	 * now we write the configuration for all network connected ports
	 */
	for (; kcd != NULL; kcd = kcd->next) {
		if (kcd->type == KVM_CASCADE_NETWORK) {
			for (i = 0; i < kcd->numports; i++) {
				/**
				 * first we find out if this port is cascaded, and in this case, write only
				 * commented code for it
				 */
				if ((kcd->ports[i].active != NULL) && (strcmp (kcd->ports[i].active, "cascade")) == 0) {
					/* port "i + 1" of switch "kcd->name" is cascaded */
					continue;
				}
				fprintf (_f, "p%d.port %s.%d\n", j, kcd->name, i + 1);
				if (kcd->ports[i].active != NULL) {
					fprintf (_f, "p%d.active %s\n", j, kcd->ports[i].active);
				}
				if (kcd->ports[i].alias != NULL) {
					fprintf (_f, "p%d.alias %s\n", j, kcd->ports[i].alias);
				}
				if (kcd->ports[i].lockoutmacro != NULL) {
					fprintf (_f, "p%d.lockoutmacro %s\n", j, kcd->ports[i].lockoutmacro);
				}

/*
				if ((kcd->ports[i].pmoutlet1 != 0) && (kcd->ports[i].pmdevice1 != NULL)) {
					if (!strcmp(kcd->ports[i].pmdevice1,"master")) {
						fprintf (_f, "p%d.pmoutlet %d", j, kcd->ports[i].pmoutlet1);
					} else {
						fprintf (_f, "p%d.pmoutlet %s.%d", j, kcd->ports[i].pmdevice1,kcd->ports[i].pmoutlet1);
					}
					if ((kcd->ports[i].pmoutlet2 != 0) && (kcd->ports[i].pmdevice2 != NULL)) {
						if (!strcmp(kcd->ports[i].pmdevice2,"master")) {
							fprintf (_f, ",%d", kcd->ports[i].pmoutlet2);
						} else {
							fprintf (_f, ",%s.%d", kcd->ports[i].pmdevice2,kcd->ports[i].pmoutlet2);
						}
					}
					fprintf (_f, "\n");
				}
*/
				if ((kcd->ports[i].pmoutletstr != NULL) && kcd->ports[i].pmoutletstr[0] != 0) {
					fprintf (_f, "p%d.pmoutlet %s", j, kcd->ports[i].pmoutletstr);
					fprintf (_f, "\n");
				}
#ifdef KVMTCPport
				if (kcd->ports[i].tcpport > 0 && kcd->ports[i].tcpport < 0x10000) {
					fprintf (_f, "p%d.tcpport %d\n", j, kcd->ports[i].tcpport);
				}
#endif
				if (kcd->ports[i].lclagc) {
					fprintf (_f, "p%d.lclagc enable\n", j);
				} else {
					fprintf (_f, "p%d.lclagc disable\n", j);
				}
				fprintf(_f, "p%d.lclbright %d\n", j, kcd->ports[i].lclbright);
				fprintf(_f, "p%d.lclcontr %d\n", j, kcd->ports[i].lclcontr);
				if (kcd->ports[i].remagc) {
					fprintf (_f, "p%d.remagc enable\n", j);
				} else {
					fprintf (_f, "p%d.remagc disable\n", j);
				}
				fprintf(_f, "p%d.rembright %d\n", j, kcd->ports[i].rembright);
				fprintf(_f, "p%d.remcontr %d\n", j, kcd->ports[i].remcontr);
				for (k = 0; k < 4; k ++) {
					fprintf(_f, "p%d.ip%dbright %d\n", j, k+1, kcd->ports[i].ipbright[k]);
					fprintf(_f, "p%d.ip%dcontr %d\n", j, k+1, kcd->ports[i].ipcontr[k]);
				}
				j++;
			}
		}
	}

	rc = fclose (_f);
	if (rc != 0) {
		kvm_cascade_strerror_set ("Failure closing configuration file %s.", config_file);
		return (KC_EFILE);
	}
	unlink(config_file);
	rename(tempfile, config_file);
	return (KC_OK);
}

/**
 * int kvm_ports_config_read (kvm_cascade kc, char *config_file)
 *
 * Parameters
 *   kvm_cascade kc : the kvm_cascade object, previously filled with
 *           information, on a call to kvm_cascade_config_read ()
 *   char *config_file : the configuration file to be written
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_cascade object not initialized
 *   KC_EFILE : could not open/write configuration file
 *   KC_EMEM : memory exhausted
 *   KC_EPARSE : failed to parse ports configuration file
 *
 * External Resources
 *   None
 *
 * Bugs / Environment changes
 *   The newly created configuration file
 *
 * Comments
 *   Unknown configuration options (anything which does not start with
 *   the 'p' character) is just copied into a "temporary buffer", and
 *   blindly copied into the new configuration file
 */
void parse_pmoutlet(struct _port_config_line_t *ports,char *pmoutlet)
{
//[LC - 06/30/2005] teh outlets cannot be limited to one single device, so 
// for now the syntax to configure it will follow the same used bu OSD:
// dev.outlet,dev.outlet,...,dev.outlet. dev must be ommited for master.
		ports->pmoutletstr = strdup(pmoutlet);
}

int kvm_ports_config_read (kvm_cascade kc, char *config_file)
{
	int i = 0;
	int j = 0;
	int idx2 = 0;
	int rc = 0;
	int cfg_index = 0;
	int _current_error = KC_OK;
	int chars = 1024; /* maximum line size */
	int total_lines = 0;
	int gport = 0;
	char *line = NULL;
	char full_line[1025];
	char *port = NULL;
	char *parameter = NULL;
	char *command = NULL;
	char *should_be_null = NULL;
	char *tmp_buf = NULL;
	char *phys_port = NULL;
	char *device = NULL;
	char *tmp_var = NULL;
	FILE *_f = NULL;
	struct _port_config_line_t *ports;
	struct kvm_cascade_dev_t *kcd;
	struct stat st;

	if (kc == NULL) {
		return (KC_NULL);
	}

	_f = fopen (config_file, "r");
	if (_f == NULL) {
		kvm_cascade_strerror_set ("Configuration file %s was not found.", config_file);
		return (KC_EFILE);
	}

	line = (char *) calloc (chars + 1, sizeof (char)); /* make line size 1 byte bigger, to accomodate the \0 */
	if (line == NULL) {
		_current_error = KC_EMEM;
		goto _err_close_f;
	}

	/* Workaround - set some defaults */
	kcd = kc->head;
	for (i = 0; i < kcd->numports; i++) {
		kcd->ports[i].lclagc = 1;
		kcd->ports[i].lclbright = 128;
		kcd->ports[i].lclcontr = 96;
		kcd->ports[i].remagc = 1;
		kcd->ports[i].rembright = 128;
		kcd->ports[i].remcontr = 96;
		for (j = 0; j < 4; j ++) {
			kcd->ports[i].ipbright[j] = 128;
			kcd->ports[i].ipcontr[j] = 96;
		}
	}

	/**
	 * find out the number of lines on the configuration file, to allocate
	 * our internal structure
	 */
	while (fgets (line, chars, _f) != NULL) {
		if (line[0] != 'p') {
			continue;
		}
		total_lines++;
	}
	rewind (_f);

	if (total_lines == 0) {
		// set default to master ports
		kcd = kc->head;
		/* Generate a list of default port names */
		for (i = 0; i < kcd->numports; i++) {
			const char *port_name = libcyc_port_name_get_default_list(cyc, &(cyc->def_port_names), i+1, PORT_TYPE_KVM);

			kcd->ports[i].active = strdup("yes");

			if (port_name) {
				strncpy(full_line, port_name, KVM_ALIAS_LEN);
				full_line[KVM_ALIAS_LEN-1] = '\0';
			}
			kcd->ports[i].alias = strdup(full_line);
			kcd->ports[i].lockoutmacro = strdup("");
			kcd->ports[i].pmoutletstr = strdup("");
			kcd->ports[i].pmdevice1 = strdup("master");
			kcd->ports[i].pmoutlet1 = 0;
			kcd->ports[i].pmdevice2 = strdup("master");
			kcd->ports[i].pmoutlet2 = 0;
			kcd->ports[i].lclagc = 1;
			kcd->ports[i].lclbright = 128;
			kcd->ports[i].lclcontr = 96;
			kcd->ports[i].remagc = 1;
			kcd->ports[i].rembright = 128;
			kcd->ports[i].remcontr = 96;
			for (j = 0; j < 4; j ++) {
				kcd->ports[i].ipbright[j] = 128;
				kcd->ports[i].ipcontr[j] = 96;
			}
		}
		fclose(_f);
		return (KC_OK);
	}
	
	stat(config_file,&st);
	kc->extra_config = calloc (st.st_size+1,1);
	kc->extra_config_size = st.st_size+1;
	
	ports = (struct _port_config_line_t *) calloc (total_lines, sizeof (struct _port_config_line_t));

	i = 1;
	while (fgets (line, chars, _f) != NULL) {
		memset (full_line, 0, 1025);
		strncpy (full_line, line, 1024);
		line = line_cleanup (line);

		/**
		 * this line is not a port configuration line - just save it's information,
		 * to write it later to new the new configuration file
		 */
		if ((strlen (line) == 0) || (line[0] != 'p')) {
			strcat (kc->extra_config, full_line);
			i++;
			continue;
		}

		// small change to accept parameters with spaces
		//port = strtok (line, " ");
		//parameter = strtok (NULL, " ");
		//should_be_null = strtok (NULL, " ");
		{
			unsigned char *s = line;
			while (*s && (*s!=' ')) s++;
			if (*s) *s++=0;
			port = line;
			parameter = (*s) ? s : NULL;
			should_be_null = NULL;
		}

		if ((port != NULL) && (parameter != NULL) && should_be_null == NULL) {
			command = index (port, '.');
			if (command == NULL) {
				kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
				_current_error = KC_EPARSE;
				free (line);
				goto _err_close_f;
			}
			tmp_var = ++command;
			command = tmp_var;
			tmp_var = ++port;
			port = tmp_var;
			for (tmp_buf = port; isdigit(*tmp_buf); tmp_buf++);
			*tmp_buf = '\0';

			gport = atoi (port);
			if (gport <= 0) {
				kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
				_current_error = KC_EPARSE;
				free (line);
				goto _err_close_f;
			}
			cfg_index = get_port (ports, gport, total_lines);
			if (cfg_index == -1) {
				kvm_cascade_strerror_set ("Memory overflow while parsing ports configuration file. Please contact the support team.");
				_current_error = KC_EPARSE;
				free (line);
				goto _err_close_f;
			}
			/* Set default values */
			ports[cfg_index].lclagc = 1;
			ports[cfg_index].lclbright = 128;
			ports[cfg_index].lclcontr = 96;
			ports[cfg_index].remagc = 1;
			ports[cfg_index].rembright = 128;
			ports[cfg_index].remcontr = 96;
			for (j = 0; j < 4; j ++) {
				ports[cfg_index].ipbright[j] = 128;
				ports[cfg_index].ipcontr[j] = 96;
			}
			
			if (strcmp (command, "port") == 0) { /* p5.port slave1.4 */
				phys_port = rindex (parameter, '.');
				if (phys_port == NULL) {
					phys_port = parameter;
					device = NULL;
				} else {
					*phys_port = '\0';
					phys_port++;
					device = parameter;
				}
				ports[cfg_index].port = atoi (phys_port);
				if (ports[cfg_index].port <= 0) {
					kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
					_current_error = KC_EPARSE;
					free (line);
					goto _err_close_f;
				}
				if (device != NULL) {
					ports[cfg_index].device = strdup (device);
				}
				ports[cfg_index].gport = gport;
			} else {
				/**
				 * fills structure for the rest of the item types
				 */
				ports[cfg_index].gport = gport;
				if (strcmp (command, "active") == 0) { /* p5.active yes */
					ports[cfg_index].active = strdup (parameter);
				} else if (strcmp (command, "alias") == 0) { /* p5.alias myserver */
					ports[cfg_index].alias = strdup (parameter);
				} else if (strcmp (command, "lockoutmacro") == 0) { 
					ports[cfg_index].lockoutmacro = strdup (parameter);

				} else if (strcmp (command, "pmoutlet") == 0) { /* p5.pmoutlet slave.13,12 */
					parse_pmoutlet(&ports[cfg_index],parameter);
					//printf("********Parameter: [%s], device[%s], outletstr[%s]\n", parameter, ports[cfg_index].pmdevice1, ports[cfg_index].pmoutletstr);
				} else 
#ifdef KVMTCPport
					if (strcmp (command, "tcpport") == 0) { 
					ports[cfg_index].tcpport = atoi (parameter);
					if (ports[cfg_index].tcpport <= 0 || ports[cfg_index].tcpport >= 0x10000) {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else
#endif
				       	if (strcmp (command, "lclagc") == 0) {
					if (!strcmp (parameter, "enable")) {
						ports[cfg_index].lclagc = 1;
					} else if (!strcmp (parameter, "disable")) {
						ports[cfg_index].lclagc = 0;
					} else {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else if (strcmp (command, "lclbright") == 0) { 
					ports[cfg_index].lclbright = atoi (parameter);
					if (ports[cfg_index].lclbright < 0 || ports[cfg_index].lclbright >= 0x100) {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else if (strcmp (command, "lclcontr") == 0) { 
					ports[cfg_index].lclcontr = atoi (parameter);
					if (ports[cfg_index].lclcontr < 0 || ports[cfg_index].lclcontr >= 0x100) {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else if (strcmp (command, "remagc") == 0) {
					if (!strcmp (parameter, "enable")) {
						ports[cfg_index].remagc = 1;
					} else if (!strcmp (parameter, "disable")) {
						ports[cfg_index].remagc = 0;
					} else {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else if (strcmp (command, "rembright") == 0) { 
					ports[cfg_index].rembright = atoi (parameter);
					if (ports[cfg_index].rembright < 0 || ports[cfg_index].rembright >= 0x100) {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else if (strcmp (command, "remcontr") == 0) { 
					ports[cfg_index].remcontr = atoi (parameter);
					if (ports[cfg_index].remcontr < 0 || ports[cfg_index].remcontr >= 0x100) {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else if (command[0] == 'i' && 
					command[1] == 'p' && 
					command[2] >= '1' && 
					command[2] <= '4' && 
					strcmp(command+3, "bright") == 0) { 
					idx2 = command[2] - '1';
					ports[cfg_index].ipbright[idx2] = 
						atoi (parameter);
					if (ports[cfg_index].ipbright[idx2] < 0 || ports[cfg_index].ipbright[idx2] >= 0x100) {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else if (command[0] == 'i' && 
					command[1] == 'p' && 
					command[2] >= '1' && 
					command[2] <= '4' && 
					strcmp(command+3, "contr") == 0) { 
					idx2 = command[2] - '1';
					ports[cfg_index].ipcontr[idx2] = atoi (parameter);
					if (ports[cfg_index].ipcontr[idx2] < 0 || ports[cfg_index].ipcontr[idx2] >= 0x100) {
						kvm_cascade_strerror_set ("Parse error reading %s, line %d", config_file, i);
						_current_error = KC_EPARSE;
						free (line);
						goto _err_close_f;
					}
				} else { /* unknown configuration option - not one of the above */

					/* could not parse line - leaving it untouched */
				//	syslog (LOG_WARNING, "Parse error reading %s, line %d - copying line untouched", config_file, i);
					strcat (kc->extra_config, full_line);
					i++;
					continue;
				}

			}
		} else {
			/* could not parse line - leaving it untouched */
			//syslog (LOG_WARNING, "Parse error reading %s, line %d - copying line untouched", config_file, i);
			strcat (kc->extra_config, full_line);
			i++;
			continue;
		}

		i++;
	}

	free (line);

	rc = fclose (_f);
	if (rc != 0) {
		kvm_cascade_strerror_set ("Failure closing configuration file %s.", config_file);
		return (KC_EFILE);
	}

	/**
	 * now that we have everything on place, start filling the port structures
	 */
	for (i = 0; i < total_lines; i++) {
		if (ports[i].gport == 0) { /* in this case, we haven't used all ports we could */
			break;
		}

		if (ports[i].port == 0) {
			kvm_cascade_strerror_set ("Parse error: global port %d has no physical port identification.", ports[i].gport);
			_current_error = KC_EPARSE;
			free (line);
			goto _err_close_f;
		}

		kcd = kc->head;
		cfg_index = ports[i].port - 1;

		/**
		 * if the current port is inside one of the cascaded
		 * devices, we need to find out which one it is.
		 */
		if (ports[i].device != NULL) {
			while (strcmp (ports[i].device, kcd->name) != 0) {
				if (kcd->next != NULL) {
					kcd = kcd->next;
				} else {
					kvm_cascade_strerror_set ("Parse error: port [%d] has invalid device [%s] configured.", ports[i].gport, ports[i].device);
					_current_error = KC_EPARSE;
					free (line);
					goto _err_close_f;
				}
			}
		}

		if (ports[i].active != NULL) {
			kcd->ports[cfg_index].active = ports[i].active;
		}
		if (ports[i].alias != NULL) {
			kcd->ports[cfg_index].alias = ports[i].alias;
		} else {
			if (!strcmp(kcd->name,"master")) {
				const char *port_str = libcyc_port_name_get_default(cyc, cfg_index+1, PORT_TYPE_KVM);
				strncpy(full_line, port_str, 20);
				full_line[20] = '\0';
			} else {
				sprintf(full_line,"%s.%d",kcd->name,cfg_index+1);
			}
			kcd->ports[cfg_index].alias = strdup(full_line);
		}
		if (ports[i].lockoutmacro != NULL) {
			kcd->ports[cfg_index].lockoutmacro = ports[i].lockoutmacro;
		} else {
			kcd->ports[cfg_index].lockoutmacro = strdup("");
		}

		if (ports[i].pmdevice1 != NULL) {
			kcd->ports[cfg_index].pmdevice1 = ports[i].pmdevice1;
		} else {
			kcd->ports[cfg_index].pmdevice1 = strdup("master");
		}
		
		if (ports[i].pmdevice2 != NULL) {
			kcd->ports[cfg_index].pmdevice2 = ports[i].pmdevice2;
		} else {
			kcd->ports[cfg_index].pmdevice2 = strdup("master");
		}
		if (ports[i].pmoutlet1 > 0) {
			kcd->ports[cfg_index].pmoutlet1 = ports[i].pmoutlet1;
		} else {
			kcd->ports[cfg_index].pmoutlet1 = 0;
		}
		if (ports[i].pmoutlet2 > 0) {
			kcd->ports[cfg_index].pmoutlet2 = ports[i].pmoutlet2;
		} else {
			kcd->ports[cfg_index].pmoutlet2 = 0;
		}
		
		if (ports[i].pmoutletstr != NULL) {
			kcd->ports[cfg_index].pmoutletstr = ports[i].pmoutletstr;
		} else {
			kcd->ports[cfg_index].pmoutletstr = strdup("");
		}
#ifdef KVMTCPport
		if (ports[i].tcpport > 0 && ports[i].tcpport < 0x10000) {
			kcd->ports[cfg_index].tcpport = ports[i].tcpport;
		}
#endif
		kcd->ports[cfg_index].lclagc = ports[i].lclagc;
		kcd->ports[cfg_index].lclbright = ports[i].lclbright;
		kcd->ports[cfg_index].lclcontr = ports[i].lclcontr;
		kcd->ports[cfg_index].remagc = ports[i].remagc;
		kcd->ports[cfg_index].rembright = ports[i].rembright;
		kcd->ports[cfg_index].remcontr = ports[i].remcontr;
		for (j = 0; j < 4; j ++) {
			kcd->ports[cfg_index].ipbright[j] = ports[i].ipbright[j];
			kcd->ports[cfg_index].ipcontr[j] = ports[i].ipcontr[j];
		}
	}

	return (KC_OK);

_err_close_f:
	fclose (_f);
	return (_current_error);
}

/**
 * int kvm_cascade_config_write (kvm_cascade kc, char *config_file)
 *
 * Parameters
 *   kvm_cascade kc : the kvm_cascade object, previously filled with
 *           information, on a call to kvm_cascade_config_read ()
 *   char *config_file : the configuration file to be written
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_cascade object not initialized
 *   KC_EFILE : could not open/write configuration file
 *   KC_EMEM : memory exhausted
 *   KC_EPARSE : 
 *
 * External Resources
 *   None
 *
 * Bugs / Environment changes
 *   The newly created configuration file
 *
 * Comments
 *   None
 */
int kvm_cascade_config_write (kvm_cascade kc, char *config_file)
{
	int rc;
	int _current_error = KC_OK;
	FILE *_f;
	struct kvm_cascade_dev_t *kcd;
	char cfg_str[1024];
	char parent[1024];
	char port[1024];
	char user1[16];
	char user2[16];

	if (kc == NULL) {
		return (KC_NULL);
	}

	_f = fopen (config_file, "w");
	if (_f == NULL) {
		kvm_cascade_strerror_set ("Could not open %s for writing.", config_file);
		return (KC_EFILE);
	}
	kcd = kc->head;

	while (kcd->next != NULL) {
		memset (cfg_str, 0, 1024);
		memset (parent, 0, 1024);
		memset (port, 0, 1024);
		memset (user1, 0, 16);
		memset (user2, 0, 16);
		kcd = kcd->next;
		switch (kcd->type) {
			case KVM_CASCADE_PHYSICAL:
				if (kcd->parent != NULL) {
					strcat (parent, kcd->parent);
					strcat (parent, ".");
				}
				snprintf (user2, 16, "%d", kcd->user2);
				strcat (parent, user2);
				if (kcd->user1 > 0) {
					snprintf (user1, 16, "%d", kcd->user1);
					strcat (parent, ",");
					strcat (parent, user1);
				}
				snprintf (cfg_str, 1024, "%s %s %d\n", kcd->name, parent, kcd->numports);
				break;
			case KVM_CASCADE_NETWORK:
				snprintf (cfg_str, 1024, "%s net(%s) %d\n", kcd->name, kcd->hostname, kcd->numports);
				break;
			default:
				kvm_cascade_strerror_set ("Internal error writing configuration file %s. Please contact the support team.", config_file);
				_current_error = KC_EPARSE;
				goto _cfg_write_error;
		}
		/* write the newly created line to the configuration file */
		fprintf (_f, "%s", cfg_str);
	}

	rc = fclose (_f);
	if (rc != 0) {
		kvm_cascade_strerror_set ("Failure closing configuration file %s.", config_file);
		return (KC_EFILE);
	}

	return (KC_OK);

_cfg_write_error:
	fclose (_f);
	return (_current_error);
}

/**
 * int kvm_cascade_config_read (kvm_cascade kc, char *config_file)
 *
 * Parameters
 *   kvm_cascade kc : the kvm_cascade object, which will be filled with
 *           all information parsed. Must be previously allocated with
 *           a call to kvm_cascade_new ()
 *   char *config_file : the configuration file to be parsed
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_cascade object not initialized
 *   KC_EFILE : configuration file not found, or not readable
 *   KC_EMEM : memory exhausted
 *   KC_EPARSE : failed to parse configuration file
 *
 * External Resources
 *   The configuration file to be parsed
 *
 * Bugs / Environment changes
 *   None
 *
 * Comments
 *   None
 */
int kvm_cascade_config_read (kvm_cascade kc, char *config_file)
{
	int i;
	int j;
	int rc;
	int chars = 1024; /* maximum line size */
	int _current_error = KC_OK;
	char *line;
	FILE *_f;
	struct kvm_cascade_dev_t *kcd;

	if (kc == NULL) {
		return (KC_NULL);
	}

	_f = fopen (config_file, "r");
	if (_f == NULL) {
		kvm_cascade_strerror_set ("Configuration file %s was not found.", config_file);
		return (KC_EFILE);
	}

	line = (char *) calloc (chars + 1, sizeof (char)); /* make line size 1 byte bigger, to accomodate the \0 */
	if (line == NULL) {
		_current_error = KC_EMEM;
		goto _err_close_f;
	}

	/**
	 * this is the primary KVM connected (the "master"), and
	 * needs to be added manually by us, as it is not
	 * explicitly shown on the configuration file kvmd.slaves
	 */
	kc->head = calloc (1, sizeof (struct kvm_cascade_dev_t));
	kc->head->name = strdup ("master");
	kc->head->type = KVM_CASCADE_PHYSICAL;
	{
		int _numports = read_proc_bdinfo ();
		if (_numports == -1) {
			kvm_cascade_strerror_set ("Failure reading ports information from /proc/bdinfo.");
			_current_error = KC_INTERNAL;
			goto _err_close_f;
		}
		kc->head->numports = _numports;
	}
	kc->head->ports = (struct kvm_port_t *) calloc (kc->head->numports, sizeof (struct kvm_port_t));

	/**
	 * reserved space for unknown config options
	 */

	i = j = 1;
	while (fgets (line, chars, _f) != NULL) {
		line = line_cleanup (line);

		/* no reason to parse empty lines :) */
		if (strlen (line) == 0) {
			j++;
			continue;
		}

		/**
		 * all the grunt work is done on a separate function,
		 * to keep all functions small enough for one to
		 * understand
		 */
		kcd = kvm_cascade_parse_line (kc, line);
		if (kcd == NULL) {
			kvm_cascade_strerror_set ("Could not parse line %d of configuration file %s.", j, config_file);
			_current_error = KC_EPARSE;
			free (line);
			goto _err_close_f;
		}

		/* now add the configuration to our list of devices */
		{
			struct kvm_cascade_dev_t *current_kcd;

			for (current_kcd = kc->head; current_kcd->next != NULL; current_kcd = current_kcd->next) {
			}
			current_kcd->next = kcd;
		}
		i++;
		j++;
	}

	kc->numdevices = i;

	free (line);

	rc = fclose (_f);
	if (rc != 0) {
		kvm_cascade_strerror_set ("Failure closing configuration file %s.", config_file);
		return (KC_EFILE);
	}

	rc = kvm_cascade_configure_cascades (kc);
	if (rc != KC_OK) {
		return (rc);
	}

	/**
	 * set the name of the config file which was read
	 * */
	kc->filename = strdup (config_file);

	return (KC_OK);

	/**
	 * this part of the code is only reached if some other error
	 * occurred, so we just ignore errors here, and return the
	 * previous error code
	 */
_err_close_f:
	fclose (_f);
	return (_current_error);
}

/**
 * int kvm_cascade_free (kvm_cascade kc)
 *
 * Parameters
 *   kvm_cascade kc : the kvm_cascade object to be freed. Must be
 *           previously allocated with a call to kvm_cascade_new ()
 *
 * Return Values
 *   KC_OK : kvm_cascade object was successfully freed
 *   KC_NULL : function was called on a non-initialized kvm_cascade object
 *
 * External Resources
 *   The configuration file to be parsed
 *
 * Bugs / Environment changes
 *   None
 *
 * Comments
 *   It is a good practice to set kc to NULL, after you call this function
 */
int kvm_cascade_free (kvm_cascade kc)
{
	struct kvm_cascade_dev_t *kcd,*aux;
	struct kvm_port_t *port;
	int i;
	
	if (kc == NULL) {
		return (KC_NULL);
	}
	
	if (kc->filename) free(kc->filename);
	if (kc->extra_config) free(kc->extra_config);
	
	for (kcd=kc->head; kcd != NULL; ) {
		if (kcd->name) free(kcd->name);
		if (kcd->parent) free(kcd->parent);
		if (kcd->hostname) free(kcd->hostname);
		if (kcd->ports) {
			for (i=0; i < kcd->numports;i++) {
				port = &kcd->ports[i];
				if (port->active) free(port->active);
				if (port->alias) free(port->alias);
				if (port->lockoutmacro) free(port->lockoutmacro);
				if (port->pmoutletstr) free(port->pmoutletstr);
				if (port->pmdevice1) free(port->pmdevice1);
				if (port->pmdevice2) free(port->pmdevice2);
			}
			free(kcd->ports);
		}
		aux = kcd;
		kcd = aux->next;
		free(aux);
	}
	free (kc);
	return (KC_OK);
}

/**
 * const char *kvm_cascade_strerror (int error_number)
 *
 * Parameters
 *   int error_number : the numeric value of the error (returned by any of
 *           the kvm_cascade_* functions
 *
 * Return Values
 *   const char * : the error message
 *
 * External Resources
 *   None
 *
 * Bugs / Environment changes
 *   None
 *
 * Comments
 *   No kvm_cascade function prints any value to stdout and/or stderr,
 *   as those functions are meant to be used as library calls
 */
const char *kvm_cascade_strerror (int error_number)
{
	const char *msg = NULL;

	switch (error_number) {
		case KC_OK:
			msg = "Success.";
			break;
		case KC_NULL:
			msg = "kvm_cascade object not initialized. Please call kvm_cascade_new () before anything else.";
			break;
		case KC_EFILE:
			if (_kvm_cascade_strerror_message != NULL) {
				msg = _kvm_cascade_strerror_message;
			} else {
				msg = "Configuration file not found.";
			}
			break;
		case KC_EMEM:
			msg = "Memory exhausted.";
			break;
		case KC_EPARSE:
			if (_kvm_cascade_strerror_message != NULL) {
				msg = _kvm_cascade_strerror_message;
			} else {
				msg = "Could not parse configuration file.";
			}
			break;
		case KC_INTERNAL:
			if (_kvm_cascade_strerror_message != NULL) {
				msg = _kvm_cascade_strerror_message;
			} else {
				msg = "Internal error. Please contact the development team.";
			}
			break;
		default:
			msg = "Unknown error. Please contact the development team.";
	}

	return (msg);
}

static void kvm_cascade_strerror_set (char *format, ...)
{
	va_list ap;

	if (_kvm_cascade_strerror_message == NULL) {
		_kvm_cascade_strerror_message = (char *) calloc (1025, sizeof (char));
	}

	va_start (ap, format);
	vsnprintf (_kvm_cascade_strerror_message, 1024, format, ap);
	va_end (ap);
}

/**
 * - removes trailing '\n' and '\r'
 * - removes spaces & tabs from the start and end of line
 * - removes comments (comments start with the '#' char, and spawn to the
 *   rest of line
 * - make line all-lowercase
 */
static char *line_cleanup (char *line)
{
	int len;
	char *f;

	/* remove comments, onwards */
	if (*line) {
		f = index (line, '#');
		if (f != NULL) {
			*f = '\0';
		}
	}

	/* cleanup end of line */
	if (*line) {
		len = strlen (line);
		f = line + len - 1;
		while (--len && isspace ((int) *f)) {
			--f;
		}

		*(f + 1) = '\0';
	}

	/* clean up start of line */
	if (*line) {
		f = line;
		while (isspace ((int) *f)) {
			++f;
		}
		if (f != line) {
			len = strlen (line) - strlen (f);
			strcpy (line, line + len);
		}
	}

	/* make line all-lowercase */
	//for (f = line; *f != '\0'; f++) {
	//	*f = tolower (*f);
	//}

	return (line);
}

static struct kvm_cascade_dev_t *kvm_cascade_parse_line (kvm_cascade kc, char *line)
{
	char f[1025];
	char *g = NULL;
	char *name = NULL;
	int type = -1;
	char *port = NULL;
	char *model = NULL;
	char *parent = NULL;
	char *should_be_null = NULL;
	char *user1 = NULL;
	char *user2 = NULL;
	struct kvm_cascade_dev_t *kcd = NULL;

	memset (f, 0, 1025);
	strncpy (f, line, 1024);
	name = strtok (f, " ");
	port = strtok (NULL, " ");
	model = strtok (NULL, " ");
	should_be_null = strtok (NULL, " ");

	/* the OSD writes 'sw' or 'pem' for some reason. Ignore that for now.
		see bug #2574 */
//	if ((name != NULL) && (port != NULL) && (model != NULL) && (should_be_null == NULL)) {
	if ((name != NULL) && (port != NULL) && (model != NULL)) {

		g = index (port, '(');

		if (g == NULL) {
			/* physical connection */
			type = KVM_CASCADE_PHYSICAL;
			g = index (port, '.');
			if (g != NULL) {
				parent = port;
				parent[strlen (port) - strlen (g)] = '\0';

				port = g + 1;
			}
			if (*port == '\0') {
				goto _parse_error;
			}
			g = index (port, ',');
			if (g == NULL) {
				user2 = port;
			} else {
				user1 = g + 1;
				user2 = port;
				user2[strlen (port) - strlen (g)] = '\0';
			}
			if (*user2 == '\0') {
				goto _parse_error;
			}
			if (user1 != NULL) {
				if (*user1 == '\0') {
					goto _parse_error;
				}
			}
		} else {
			/* network connection */
			type = KVM_CASCADE_NETWORK;
			parent = g + 1;
			if ((*parent == '\0') || (parent[strlen (parent) - 1] != ')')) {
				goto _parse_error;
			}
			parent[strlen (parent) - 1] = '\0';
		}

		/**
		 * now that we have all the needed information, allocate a
		 * new kvm_cascade_dev structure, and fill it.
		 */
		kcd = (struct kvm_cascade_dev_t *) calloc (1, sizeof (struct kvm_cascade_dev_t));
		if (kcd == NULL) {
			goto _parse_error;
		}
		kcd->name = strdup (name);
		kcd->type = type;
		kcd->numports = atoi (model);
		if (kcd->type == KVM_CASCADE_PHYSICAL) {
			if (parent != NULL) {
				kcd->parent = strdup (parent);
			}
		} else {
			/* KVM_CASCADE_NETWORK */
			kcd->hostname = strdup (parent);
		}
		if (user2 != NULL) {
			kcd->user2 = atoi (user2);
		} else {
			kcd->user2 = 0;
		}
		if (user1 != NULL) {
			kcd->user1 = atoi (user1);
		} else {
			kcd->user1 = 0;
		}
		kcd->ports = (struct kvm_port_t *) calloc (kcd->numports, sizeof (struct kvm_port_t));
	} else {
		goto _parse_error;
	}

	return (kcd);

_parse_error:
	if (kcd != NULL) {
		free (kcd);
	}
	return (NULL);
}

static int get_port (struct _port_config_line_t *ports, int port, int numports)
{
	int i;

	for (i = 0; i < numports; i++) {
		if (ports[i].gport == port) {
			return (i);
		}
	}
	for (i = 0; i < numports; i++) {
		if (ports[i].gport == 0) {
			return (i);
		}
	}
	return (-1);
}

static int kvm_cascade_configure_cascades (kvm_cascade kc)
{
	int user1 = 0;
	int user2 = 0;
	char *parent = NULL;
	struct kvm_cascade_dev_t *kcd;
	struct kvm_cascade_dev_t *current_kcd;

	kcd = kc->head;

	for (kcd = kcd->next; kcd != NULL; kcd = kcd->next) {
		if (kcd->type == KVM_CASCADE_PHYSICAL) {
			parent = kcd->parent;
			if (parent != NULL) { /* find out which switch is our parent switch */
				for (current_kcd = kc->head; current_kcd != NULL; current_kcd = current_kcd->next) {
					if (strcmp (current_kcd->name, parent) == 0) {
						break;
					}
				}
				if (current_kcd == NULL) {
					/**
					 * if we reached this section, the specified parent does not exist
					 */
					kvm_cascade_strerror_set ("Invalid parent switch [%s] specified on configuration file.", parent);
					return (KC_EPARSE);
				}
			} else {
				current_kcd = kc->head;
			}

			/**
			 * this is the parent switch. now we need to find
			 * out which ports are used by us.
			 */
			user2 = kcd->user2 - 1;
			user1 = kcd->user1 - 1;
			/* first configure user2 */
			if ((current_kcd->ports[user2].active != NULL) && strcmp (current_kcd->ports[user2].active, "cascade") == 0) {
				/* port is already cascaded - shouldn't happen */
				kvm_cascade_strerror_set ("Failure cascading device [%s], port [%s]. This port is already cascaded to another device.", current_kcd->name, kcd->user2);
				return (KC_EPARSE);
			} else {
				if (current_kcd->ports[user2].active != NULL) {
					free (current_kcd->ports[user2].active);
				}
				current_kcd->ports[user2].active = strdup ("cascade");
			}

			/* then configure user1, if it exists */
			if (user1 >= 0) {
				if ((current_kcd->ports[user1].active != NULL) && strcmp (current_kcd->ports[user1].active, "cascade") == 0) {
					/* port is already cascaded - shouldn't happen */
					kvm_cascade_strerror_set ("Failure cascading device [%s], port [%s]. This port is already cascaded to another device.", current_kcd->name, kcd->user1);
					return (KC_EPARSE);
				} else {
					if (current_kcd->ports[user1].active != NULL) {
						free (current_kcd->ports[user1].active);
					}
					current_kcd->ports[user1].active = strdup ("cascade");
				}
			}
		} /* only physically connected switches use ports off other devices */
	}

	return (KC_OK);
}

/**
 * reads the model number (first field, delimited by a '!') from /proc/bdinfo.
 * then find out how many ports this model has looking for the board model
 * number and translating to the number of ports this model has.
 * returns -1 if something went wrong
 */
static int read_proc_bdinfo (void)
{
	int numports = -1, boardtype = 0, i;
	char *token = NULL, *lineptr = NULL;
	char line[1025];
	FILE *_f = NULL;

#ifdef KVM_TEST_INTEL_HARDWARE
	/**
	 * this is useful to test kvm_cascade on Intel hardware, which has no
	 * /proc/bdinfo file
	 */
	return (32);
#endif /* KVM_TEST_INTEL_HARDWARE */

        /* open bdinfo file */
	_f = fopen ("/proc/bdinfo", "r");
	if (_f == NULL) {
		return (-1);
	}
	
        /* reads bdinfo line */
	memset (line, 0, 1025);
	if (fgets (line, 1024, _f) == NULL) {
		fclose (_f);
		return (-1);
	}
	lineptr = line;

	/* search for the board model */
	for (i = 0; i < 4; i++) {
		if ((token = strtok(lineptr, "!")) == NULL) {
			fclose (_f);
			return (-1);
		}
		lineptr = NULL;
	}

	/* convert board model */
	boardtype = atoi(token);

	/* read number of ports */
	switch (boardtype) {
		
		case BOARD_KVM16:
		case BOARD_KVMNET16:
		case BOARD_KVMP16:
			numports = 16;
			break;
			
		case BOARD_KVM32:
		case BOARD_KVMNET32:
		case BOARD_KVMP32:
			numports = 32;
			break;

		case BOARD_ONS441:
                case BOARD_ONS841:
                case BOARD_ONS442:
                case BOARD_ONS842:
			numports = 4;
			break;
		
		case BOARD_ONS481:
		case BOARD_ONS881:
		case BOARD_ONS482:
		case BOARD_ONS882:
			numports = 8;
			break;
                
		default:
			numports = -1;
			break;
	}

	/* close file and return */
	fclose (_f);
	return (numports);
}
