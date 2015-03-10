/*
 *  kvm_auxport.c  -  Parses and writes KVM/net AUX Port information
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

static char *RCSid __attribute__ ((unused)) = "$Id: kvm_auxport.c,v 1.21 2007/09/27 19:26:52 acyr Exp $ ";

/**
 * global includes
 */
#include <libgen.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <webApi.h>

/**
 * local includes
 */
#include "kvm_auxport.h"
#include "kvmdebug.h"

/**
 * locally defined support functions
 */
static char *line_cleanup (char *line);

/**
 * this is a kludge to support optional parameters to kvm_auxport_strerror ()
 */
static char *_kvm_auxport_strerror_message = NULL;
static void kvm_auxport_strerror_set (char *format, ...);

/**
 * kvm_auxport kvm_auxport_new (void)
 *
 * Parameters
 *   None
 *
 * Return Values
 *   kvm_auxport : a newly allocated (and empty) kvm_auxport object
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
 *   kvm_auxport_free ()
 */
kvm_auxport kvm_auxport_new (void)
{
	kvm_auxport kap = NULL;
	char ttystr[10];
	int i;

	kap = (kvm_auxport) calloc ((size_t) MAX_AUX, sizeof (struct kvm_auxport_t));
	if (kap == NULL) {
		return (NULL);
	}

	/**
	 * fill defaults for AUX Port Configuration
	 */
	for (i = 0; i < MAX_AUX; i++) {
		kap[i].auxidx = i+1;
		kap[i].filename = strdup ("<none>");
		kap[i].protocol = KVM_AUXPORT_PROTOCOL_PPP_ONLY;
		kap[i].parity = KVM_AUXPORT_PARITY_NONE;
		kap[i].flow = KVM_AUXPORT_FLOW_NONE;
		kap[i].speed = 9600;
		kap[i].datasize = 8;
		kap[i].stopbits = 1;
		/* retrieved during development, from /etc/portslave/pslave.conf */
		kap[i].initchat = strdup (
			"TIMEOUT 10 \n"
			"\"\" \\d\\l\\dATZ \n"
			"OK\\r\\n-ATZ-OK\\r\\n \"\" \n"
			"TIMEOUT 10 \n"
			"\"\" ATM0 \n"
			"OK\\r\\n \"\" \n"
			"TIMEOUT 3600 \n"
			"RING \"\" \n"
			"STATUS Incoming %p:I.HANDSHAKE \n"
			"\"\" ATA \n"
			"TIMEOUT 60 \n"
			"CONNECT@ \"\" \n"
			"STATUS Connected %p:I.HANDSHAKE");
		kap[i].autoppp = strdup (
			"0.0.0.0:0.0.0.0 auth mtu 1500 mru 1500 \n"
			"proxyarp modem asyncmap 000A0000 \n"
			"noipx noccp login novj require-pap refuse-chap \n"
			"ms-dns 0.0.0.0 \n"
			"plugin /usr/lib/libpsr.so");
		sprintf(ttystr, "ttyA%d", (i+1));
		kap[i].tty = strdup (ttystr);
#ifdef OEM3
		kap[i].pmtype = KVM_AUXPORT_PM_APC;
#else
		//kap[i].pmtype = KVM_AUXPORT_PM_CYCLADES;
		kap[i].pmtype = KVM_AUXPORT_PM_AUTO;
#endif
		kap[i].pmNumOfOutlets = 8;
	}

	return (kap);
}

#ifdef PMDNG
/**
 * kvm_auxport_result kvm_auxport_pmdps_config_read (kvm_auxport kap, char *pm_conf_file, char *ps_conf_file)
 *
 * Parameters
 *   kvm_auxport kap : the kvm_auxport object, which will be filled with
 *           all information parsed. Must be previously allocated with
 *           a call to kvm_auxport_new ()
 *   char *pm_conf_file : The PMD configuration file to be parsed
 *   char *ps_conf_file : The Port Slave configuration file to be parsed
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_auxport object not initialized
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
kvm_auxport_result kvm_auxport_pmdps_config_read (kvm_auxport kap, char *pm_conf_file, char *ps_conf_file)
{
	FILE *_f, *_F;
	char *line;
	int chars = 1024;
	char full_line[chars + 1];
	struct stat pm_st;
	struct stat ps_st;
	int j, i, rc;
	int enable_port = 1;
	int idx = 0;
	char *termchar = NULL;
	kvm_auxport_result _current_error = KAP_OK;
	char *position = NULL;
	char *parameter = NULL;
	char *saved_option = NULL;
	char saved_parameter[1024] = {0};
	char *option = NULL;
	int linesize;

	if (kap == NULL) {
		return (KAP_NULL);
	}

	_f = fopen (pm_conf_file, "r");
	if (_f == NULL) {
		kvm_auxport_strerror_set ("Configuration file %s was not found.",pm_conf_file);
		return (KAP_EFILE);
	}

	/**
	 * the current line being read
	 */
	line = (char *) calloc (chars + 1, sizeof (char));
	if (line == NULL) {
		return (KAP_EMEM);
	}

	stat(pm_conf_file,&pm_st);
	kap[0].extra_config_pm = calloc (pm_st.st_size + 50, sizeof (char));
	kap[0].extra_config_pm_size = pm_st.st_size + 50;
	j = 1;

	/* Start parsing /etc/pmd.conf */
	while (fgets (line, chars, _f) != NULL) {
		memset (full_line, 0, chars + 1);
		strcpy (full_line, line);
		line = line_cleanup (line);
		linesize = strlen(line);
		enable_port = kYes;

		if ( linesize > 4 ) {
			if ( line[0] == '#' ) {
				line++;
				enable_port = kNo;
			}
			if ( strncmp("ttyA",line,4) == 0 ) {
				/* Parse line and fill the right structure */
				char *pline;
				int i , field;
	
				pline = line;
				i = atoi(pline+4);
				idx = i - 1;
				memset(&kap[idx],'\0',sizeof(kvm_auxport));
				kap[idx].auxidx = idx + 1;
				//kap[idx].protocol = KVM_AUXPORT_PROTOCOL_IPDU;
				if ( enable_port ) { 
					kap[idx].enable = kYes; 
				} else {
					kap[idx].enable = kNo; 
				}
				kap[idx].filename = strdup(pm_conf_file);
				for ( field = 0; field < 7; field++) {
					char fieldValue[20];
					int v = 0;
	
					memset(fieldValue,'\0',20);
					while ( (*pline != ':') && (*pline != '\n') ) {
						fieldValue[v++] = *(pline++);
						//v++;
						//pline++;
					}
					switch (field) {
						case 0:
							if ( v > 0 ) {
								kap[idx].tty = strdup(fieldValue);
							} else {
								kap[idx].tty = NULL;
							}
							break;
						case 1:
							if ( v > 0 ) {
								kap[idx].speed = atoi(fieldValue);
							} else {
								kap[idx].speed = 9600;
							}
							break;
						case 2:
							if ( v > 0 ) {
								kap[idx].datasize = atoi(fieldValue);
							} else {
								kap[idx].datasize = 8;
							}
							break;
						case 3:
							if ( v > 0 ) {
								if ( strcmp(fieldValue,"none") == 0 ) {
									kap[idx].parity = KVM_AUXPORT_PARITY_NONE;
								} else if ( strcmp(fieldValue,"odd") == 0 ) {
									kap[idx].parity = KVM_AUXPORT_PARITY_ODD;	
								} else if ( strcmp(fieldValue,"even") == 0 ) {
									kap[idx].parity = KVM_AUXPORT_PARITY_EVEN;	
								}
							} else {
								kap[idx].parity = KVM_AUXPORT_PARITY_NONE;
							}
							break;
						case 4:
							if ( v > 0 ) {
								kap[idx].stopbits = atoi (fieldValue);
							} else {
								kap[idx].stopbits = 1;
							}
							break;
						case 5:
							if ( v > 0 ) {
								if ( strcmp(fieldValue,"none") == 0 ) {
									kap[idx].flow = KVM_AUXPORT_FLOW_NONE;
								} else if ( strcmp(fieldValue,"soft") == 0 ) {
									kap[idx].flow = KVM_AUXPORT_FLOW_SOFT;
								} else if ( strcmp(fieldValue,"hard") == 0 ) {
									kap[idx].flow = KVM_AUXPORT_FLOW_HARD;
								}
							} else {
								kap[idx].flow = KVM_AUXPORT_FLOW_NONE;
							}
							break;
						case 6:
							if ( v > 0 ) {
#ifdef OEM3
								if ( strcmp(fieldValue,"apc") == 0 ) {
									kap[idx].pmtype = KVM_AUXPORT_PM_APC;
#else
								if ( strcmp(fieldValue,"cyclades") == 0 ) {
									kap[idx].pmtype = KVM_AUXPORT_PM_CYCLADES;
//#endif                                                                                        //KVM_APC [GY]07/Aug/15
								} else if ( strcmp(fieldValue,"servertech") == 0 ) {
									kap[idx].pmtype = KVM_AUXPORT_PM_SERVERTECH;
								} else if ( strcmp(fieldValue,"spc") == 0 ) {
									kap[idx].pmtype = KVM_AUXPORT_PM_SPC;
#endif
								} else if ( strcmp(fieldValue,"auto") == 0 ) {
									kap[idx].pmtype = KVM_AUXPORT_PM_AUTO;
								}
							} else {
								kap[idx].pmtype =
#ifdef OEM3
									KVM_AUXPORT_PM_APC;
#else 
									KVM_AUXPORT_PM_AUTO;
#endif 
							}
							break;
					}
				}
			}
			else {
				strcat (kap[0].extra_config_pm, full_line);
			}
		}
	}
	
	/* Stop parsing /etc/pmd.conf */
	fclose(_f);

	/* Starting parsing pslave.conf */
	_F = fopen (ps_conf_file,"r");
	if (_F == NULL) {
		kvm_auxport_strerror_set ("Configuration file %s was not found.", ps_conf_file);
		return (KAP_EFILE);
	}

	stat(ps_conf_file,&ps_st);
	kap[0].extra_config_ps = calloc (ps_st.st_size + 50, sizeof (char));
	kap[0].extra_config_ps_size = ps_st.st_size + 50;
	j = 1;

	while (fgets (line, chars, _F) != NULL) {
		int linesize;

		memset (full_line, 0, chars + 1);
		strcpy (full_line, line);
		line = line_cleanup (line);
		linesize = 0;

		enable_port = kYes;
		linesize = strlen(line);

		if ( linesize == 0 ) {
			strcat (kap[0].extra_config_ps, full_line);
			j++;
			continue;
		}
		else {
			if ( linesize > 3 ) {
				if ( strncmp(line,"a1.",3) != 0 && strncmp(line,"a2.",3) != 0 
					&& strncmp(line,"a3.",3) != 0 ) {
					/* Check port disable */
					if ( strncmp(line,"#a1.tty",7) != 0 && strncmp(line,"#a2.tty",7) != 0
				     	     && strncmp(line,"#a3.tty",7) != 0 ) {
						strcat (kap[0].extra_config_ps, full_line);
						j++;
						continue;
					}
					else {
						enable_port = kNo;
						line++;
					}
				}
			}
			else {
				strcat (kap[0].extra_config_ps, full_line);
				j++;
				continue;
			}
		}

		/**
		 * now parse the line
		 */
		if ((termchar = strchr(line, '.')) == NULL) {
			kvm_auxport_strerror_set("Parse error (no dot on the line) on %s, line %d.",ps_conf_file,j);
			_current_error = KAP_EPARSE;
			goto _error_cleanup;
		}
			
		*termchar='\0';
		sscanf(line, "a%d", &idx);
		if (idx <= 0 || idx > MAX_AUX) {
			if (enable_port) {
				kvm_auxport_strerror_set ("enabled auxport#%d greater than MAX(%d). line %d.",
					idx, MAX_AUX, j); 
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		}
		idx--;		
		kap[idx].auxidx = idx+1;
		*termchar='.';
		position = line + 3; /* 'a1.' */

		/* there must be something after the '.' */
		if (*position == '\0') {
			kvm_auxport_strerror_set("Parse error (no data after the dot) on %s, line %d.",ps_conf_file,j);
			_current_error = KAP_EPARSE;
			goto _error_cleanup;
		}

		for (i = 0;;i++) {
			option = position + i;
			if (isspace (*option) != 0) {
				/* we finally found the next parameter */
				break;
			}
		}
		option = position;
		option[i] = '\0';
		position = option + i + 1;

		if ((*option == '\0') || (*position == '\0')) {
			if (strcmp (option, "initchat") == 0) { /* s1.initchat "<initchat>" */
				/// initchat can have an empty string.
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d.",ps_conf_file,j);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		}

		/* find next non-blank char */
		for (i = 0;;i++) {
			parameter = position + i;
			if (isspace (*parameter) == 0) {
				break;
			}
		}

		/**
		 * now we check if this line continues on the next line
		 */
		i = strlen (parameter);
		if (parameter[i - 1] == '\\') {
			j++;
			saved_option = strdup (option);
			strcpy(saved_parameter,parameter);
			saved_parameter[i - 1] = '\n';
			if (saved_option == NULL) {
				_current_error = KAP_EMEM;
				goto _error_cleanup;
			}

			/* now we get the "clean" line, and add it into our buffer */
			while (fgets (line, chars, _F) != NULL) {
				line = line_cleanup (line);
				i = strlen (line);
				if ((i > 0) && ((strlen(saved_parameter)+i) < 1024)) {
					strcat (saved_parameter, line);
				}
				j++;
				i = strlen (saved_parameter);
				if (saved_parameter[i - 1] != '\\') {
					/* ok, this sequence was finished */
					break;
				}
				saved_parameter[i - 1] = '\n';
			}

			option = saved_option;
			parameter = strdup(saved_parameter);
		}

		/**
		 * test for strings inside ""
		 */
		if (parameter[0] == '"') {
			i = strlen (parameter);
			if (parameter[i - 1] != '"') {
				/* if the string starts with a '"', it must end with another '"' */
				if (saved_option != NULL) {
					free (saved_option);
					saved_option = NULL;
				}
				kvm_auxport_strerror_set ("Parse error on %s, at line %d.",ps_conf_file,j);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}

			/**
			 * remove the double brackets (") from the line
			 */
			position = parameter + 1;
			parameter[i - 1] = '\0';
			strcpy (parameter, position);
		}

		/**
		 * now we know which option to fill, and have the parameter for it.
		 */
		if (strcmp (option, "protocol") == 0) { /* s1.protocol <protocol_name> */
			if (strcmp (parameter, "pm_ng") == 0) {
				kap[idx].protocol = KVM_AUXPORT_PROTOCOL_IPDU;
			} else if (strcmp (parameter, "ppp_only") == 0) {
				kap[idx].protocol = KVM_AUXPORT_PROTOCOL_PPP_ONLY;
			} else if (strcmp (parameter, "login") == 0) {
				kap[idx].protocol = KVM_AUXPORT_PROTOCOL_LOGIN;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid protocol type %s).",ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "speed") == 0) { /* s1.speed <speed> */
			kap[idx].speed = atoi (parameter);
			if (kap[idx].speed <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid port speed %s).", ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "datasize") == 0) { /* s1.datasize <datasize> */
			kap[idx].datasize = atoi (parameter);
			if (kap[idx].datasize <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid datasize %s).", ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "stopbits") == 0) { /* s1.stopbits <stopbits> */
			kap[idx].stopbits = atoi (parameter);
			if (kap[idx].stopbits <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid stopbits %s).", ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "parity") == 0) { /* s1.parity <parity> */
			if (strcmp (parameter, "none") == 0) {
				kap[idx].parity = KVM_AUXPORT_PARITY_NONE;
			} else if (strcmp (parameter, "odd") == 0) {
				kap[idx].parity = KVM_AUXPORT_PARITY_ODD;
			} else if (strcmp (parameter, "even") == 0) {
				kap[idx].parity = KVM_AUXPORT_PARITY_EVEN;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid parity type %s).", ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "flow") == 0) { /* s1.flow <flow> */
			if (strcmp (parameter, "none") == 0) {
				kap[idx].flow = KVM_AUXPORT_FLOW_NONE;
			} else if (strcmp (parameter, "soft") == 0) {
				kap[idx].flow = KVM_AUXPORT_FLOW_SOFT;
			} else if (strcmp (parameter, "hard") == 0) {
				kap[idx].flow = KVM_AUXPORT_FLOW_HARD;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid parity type %s).", ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "initchat") == 0) { /* s1.initchat "<initchat>" */
			if (kap[idx].initchat) {
				free(kap[idx].initchat);
				kap[idx].initchat = NULL;
			}
			if ( (parameter != NULL) ) {
				kap[idx].initchat = strdup (parameter);
				if (kap[idx].initchat == NULL) {
					_current_error = KAP_EMEM;
					goto _error_cleanup;
				}
			}
			
		} else if (strcmp (option, "pppopt") == 0) { /* s1.autoppp "<autoppp>" */
			if ((parameter != NULL) && (parameter[0] != '\0')) {
				if (kap[idx].autoppp != NULL) {
					free (kap[idx].autoppp);
				}
				kap[idx].autoppp = strdup (parameter);
				if (kap[idx].autoppp == NULL) {
					_current_error = KAP_EMEM;
					goto _error_cleanup;
				}
			}
		} else if (strcmp (option, "tty") == 0) { /* s1.tty ttyA1 */
			if ((parameter != NULL) && (parameter[0] != '\0')) {
				if (kap[idx].tty != NULL) {
					free (kap[idx].tty);
				}
				kap[idx].tty = strdup (parameter);
//#ifdef ONS
				if ( enable_port ) {
					kap[idx].enable = kYes;
				} else {
					kap[idx].enable = kNo;
				}
//#endif
				if (kap[idx].tty == NULL) {
					_current_error = KAP_EMEM;
					goto _error_cleanup;
				}
			}
		/*} else if (strcmp (option, "pmtype") == 0) { // s1.pmtype cyclades
#ifdef OEM3
	char pmtype[] = "APCPDU";
	int  pmid = KVM_AUXPORT_PM_APC;
#else
	char pmtype[] = "auto";
	int  pmid = KVM_AUXPORT_PM_AUTO;
#endif
			if (strcmp (parameter, pmtype) == 0) {
				kap[idx].pmtype = pmid;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid pm type %s).", ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "pmNumOfOutlets") == 0) { // s1.pmNumOfOutlets <numOfOutlets>
			kap[idx].pmNumOfOutlets = atoi (parameter);
			if (kap[idx].pmNumOfOutlets <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid pmNumOfOutlets %s).", ps_conf_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}*/
		} else if (strcmp(option, "authtype") && strcmp(option, "ipno")) {
			strcat (kap[0].extra_config_ps, full_line);
		}

		/**
		 * before you add *anything* here, make sure you understand the last "else"
		 * statement above
		 */

		if (saved_option != NULL) {
			free (saved_option);
			saved_option = NULL;
		}
		j++;
	}
	free (line);

	rc = fclose(_F);
	if (rc != 0) {
		kvm_auxport_strerror_set ("Failure closing configuration file %s.", ps_conf_file);
		return (KAP_EFILE);
	} 

	/**
	 * set the name of the config file which was read
	 * */
	for (i = 0; i < MAX_AUX; i++) {
		if (kap[i].filename != NULL ) {
			if ( strncmp(kap[i].filename,pm_conf_file,strlen(pm_conf_file)) != 0 ) {
				free (kap[i].filename);
				kap[i].filename = strdup (ps_conf_file);
			}
		}
	}

	return (KAP_OK);

	/**
	 * this part of the code is only reached if some other error
	 * occurred, so we just ignore errors here, and return the
	 * previous error code
	 */
_error_cleanup:
	free (line);
	fclose (_F);
	return (_current_error);
}

/**
 * kvm_auxport_result kvm_auxport_pmdps_config_write (kvm_auxport kap, char *pm_conf_file, char *ps_conf_file)
 *
 * Parameters
 *   kvm_auxport kap : the kvm_auxport object, previously filled with
 *           information, on a call to kvm_auxport_config_read ()
 *   char *pm_conf_file : the pmd.conf file to be written
 *   char *ps_conf_file : the pslave.conf file to be written
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_auxport object not initialized
 *   KC_EFILE : could not open/write configuration file
 *   KC_EMEM : memory exhausted
 *   KC_EPARSE : failed to parse the kvm_auxport object - internal error
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
kvm_auxport_result kvm_auxport_pmdps_config_write (kvm_auxport kap, char *pm_conf_file, char *ps_conf_file)
{
	int rc;
	kvm_auxport_result _current_error = KAP_OK;
	FILE *_f,*_F;
	char pref[10];
	int i, k;
	char *pip;
	char ConfSection[2048];
	char PortSection[2048];
	char IpduSection[2048];
	char hasConfSection;
	char hasPortSection;
	char hasIpduSection;
	char parity[5];
	char flow[5];
	char speed[7];
	char datasize[3];
	char stopbits[2];
	char initchat[1024];
	char autoppp[1024];
	char ipno[21];
	char ps_serial[100];
	
#ifdef OEM3
	char pmtype[] = "apc";
#else
	char pmtype[] = "auto";
#endif

	LOGDEBUG(LD_SERIAL | LD_CONFIG, "Writing %s", pm_conf_file);
	LOGDEBUG(LD_SERIAL | LD_CONFIG, "Writing %s", ps_conf_file);

	if (kap == NULL) {
		return (KAP_NULL);
	}

	_f = fopen(pm_conf_file,"w");
	_F = fopen(ps_conf_file,"w");

	if (_f == NULL) {
		kvm_auxport_strerror_set ("Could not open %s for writing.",pm_conf_file);
		return (KAP_EFILE);
	}
	else {
		char *pch;
		char Line[500];

		/* Get all sections */
		pch = kap[0].extra_config_pm;
		memset(ConfSection,'\0',2048);
		memset(PortSection,'\0',2048);
		memset(IpduSection,'\0',2048);
		hasConfSection = 0;
		hasPortSection = 0;
		hasIpduSection = 0;

		while (1) {
			int i;
			/* Get a line */
			memset(Line,'\0',500);
			i = 0;
			while ( *pch != '\0' && *pch != '\n' ) {
				Line[i] = *pch;
				i++;
				pch++;
			}
			if ( *pch != '\0' ) {
				pch++;
			}
			if ( strncmp("[CONFIG]",Line,8) == 0 ) {
				int cs;
				strncpy(ConfSection,"[CONFIG]\n",9);
				cs = 9;
				hasConfSection = 1;
				/* Keep everything until '\0' or another section */
				while ( *pch != '\0' ) {
					if ( *pch == '[' ) {
						/* Another section started */
						break;
					} else {
						ConfSection[cs] = *pch;
						cs++;
						pch++;
					}
				}
			} else if ( strncmp("[PORTS]",Line,7) == 0 ) {
				int ps;
				strncpy(PortSection,"[PORTS]\n",8);
				ps = 8;
				hasPortSection = 1;
				/* Keep everything until '\0' or another section */
				while ( *pch != '\0' ) {
					if ( *pch == '[' ) {
						/* Another section started */
						break;
					} else {
						PortSection[ps] = *pch;
						ps++;
						pch++;
					}
				}
			} else if ( strncmp("[IPDU]",Line,6) == 0 ) {
				int is;
				strncpy(IpduSection,"[IPDU]\n",7);
				is = 7;
				hasIpduSection = 1;
				/* Keep everything until '\0' or another section */
				while ( *pch != '\0' ) {
					if ( *pch == '[' ) {
						/* Another section started */
						break;
					} else {
						IpduSection[is] = *pch;
						is++;
						pch++;
					}
				}
			} else {
				fprintf(_f,"%s\n",Line);
			}
			if ( *pch == '\0' ) {
				break;
			}
		}

		if ( hasConfSection ) {
			fprintf(_f,"%s",ConfSection);
		}
		LOGDEBUG(LD_SERIAL|LD_CONFIG, "Extra config");

		if ( hasPortSection ) {
			fprintf(_f,"%s",PortSection);
		}
		LOGDEBUG(LD_SERIAL|LD_CONFIG, "Extra config");
	}

	if (_F == NULL) {
		kvm_auxport_strerror_set ("Could not open %s for writing.",ps_conf_file);
		return (KAP_EFILE);
	}
	else {
		/* Write all pslave.conf not related to aux ports */
		if (kap[0].extra_config_ps) 
			fprintf (_F,"%s",kap[0].extra_config_ps);
		LOGDEBUG(LD_SERIAL|LD_CONFIG, "Extra config");
	}

	for (i = 0; i < MAX_AUX; i++) {
		LOGDEBUG(LD_SERIAL|LD_CONFIG, "Writing config for AUX port %d", i);
		
		/* Start data setup */
		sprintf(pref, "a%d.", kap[i].auxidx);
		
		sprintf(ps_serial,
			"%sspeed %%s\n"
			"%sdatasize %%s\n"
			"%sstopbits %%s\n"
			"%sparity %%s\n"
			"%sflow %%s\n"
			"%sinitchat %%s\n",
			pref, pref, pref, pref, pref, pref);
		
		switch (kap[i].parity) {
			case KVM_AUXPORT_PARITY_NONE:
				strcpy(parity, "none");
				break;
			case KVM_AUXPORT_PARITY_ODD:
				strcpy(parity, "odd");
				break;
			case KVM_AUXPORT_PARITY_EVEN:
				strcpy(parity, "even");
				break;
			default:
				strcpy(parity, "");
				break;
		}

		switch (kap[i].flow) {
			case KVM_AUXPORT_FLOW_NONE:
				strcpy(flow, "none");
				break;
			case KVM_AUXPORT_FLOW_SOFT:
				strcpy(flow, "soft");
				break;
			case KVM_AUXPORT_FLOW_HARD:
				strcpy(flow, "hard");
				break;
			default:
				strcpy(flow, "");
				break;
		}
		
		if (kap[i].speed > 0)
			sprintf(speed, "%d", kap[i].speed);
		else
			strcpy(speed, "");
		
		if (kap[i].datasize > 0)
			sprintf(datasize, "%d", kap[i].datasize);
		else
			strcpy(datasize, "");
		
		if (kap[i].stopbits >= 0)
			sprintf(stopbits, "%d", kap[i].stopbits);
		else
			strcpy(stopbits, "");
		/* Finish data setup */

		/* Start writing to /etc/pmd.conf */
		fprintf(_f, "%sttyA%d:%s:%s:%s:%s:%s:",
			kap[i].enable && kap[i].protocol == KVM_AUXPORT_PROTOCOL_IPDU ? "":"#",
			kap[i].auxidx,
			speed,
			datasize,
			parity,
			stopbits,
			flow);
		switch (kap[i].pmtype) {
#ifdef OEM3 //KVM_APC [GY]07/Aug/15
			case KVM_AUXPORT_PM_APC:
				fprintf(_f, "apc");
				break;
#else
			case KVM_AUXPORT_PM_CYCLADES:
				fprintf(_f, "cyclades");
				break;
			case KVM_AUXPORT_PM_SERVERTECH:
				fprintf(_f, "servertech");
				break;
			case KVM_AUXPORT_PM_SPC:
				fprintf(_f, "spc");
				break;
#endif
			case KVM_AUXPORT_PM_AUTO:
				fprintf(_f, "auto");
				break;
			default:
				fprintf(_f, pmtype);
				break;
		} 
		fprintf(_f, "\n");
		/* Finish writing to /etc/pmd.conf */
		
		if ((kap[i].protocol == KVM_AUXPORT_PROTOCOL_PPP_ONLY || kap[i].protocol == KVM_AUXPORT_PROTOCOL_LOGIN)) {
	
			/* Start /etc/portslave/pslave.conf bad data handling */
			if (!strcmp(speed, "")) {
				kvm_auxport_strerror_set ("Internal error: invalid speed %d selected.", kap[i].speed);
				_current_error = KAP_EPARSE;
			}
			if (!strcmp(datasize, "")) {
				kvm_auxport_strerror_set ("Internal error: invalid datasize %d selected.", kap[i].datasize);
				_current_error = KAP_EPARSE;
			}
			if (!strcmp(stopbits, "")) {
				kvm_auxport_strerror_set ("Internal error: invalid stopbits %d selected.", kap[i].stopbits);
				_current_error = KAP_EPARSE;
			}
			if (!strcmp(parity, "")) {
				kvm_auxport_strerror_set ("Internal error: invalid parity %d selected.", kap[i].parity);
				_current_error = KAP_EPARSE;
			}
			if (!strcmp(flow, "")) {
				kvm_auxport_strerror_set ("Internal error: invalid flow %d selected.", kap[i].flow);
				_current_error = KAP_EPARSE;
			}
			if (_current_error == KAP_EPARSE) {
				fclose (_f);
				fclose (_F);
				return (_current_error);
			}
			/* Finish /etc/portslave/pslave.conf bad data handling */			
			
			/* Start multi-line strings formatting */
			if (kap[i].initchat != NULL) 
				kvm_auxport_format_mstring(initchat, kap[i].initchat, 1024);
			else
				strcpy(initchat, "");

			if (kap[i].autoppp != NULL) 
				kvm_auxport_format_mstring(autoppp, kap[i].autoppp, 1024);
			else
				strcpy(autoppp, "");
			/* Finish multi-line strings formatting */
		}
				
		/* Start writing to /etc/portslave/pslave.conf */
		fprintf(_F, "\n%s%stty ttyA%d\n", kap[i].enable? "" : "#", pref, kap[i].auxidx);
		fprintf(_F, "%sprotocol ", pref);
		switch (kap[i].protocol) {
			case KVM_AUXPORT_PROTOCOL_IPDU:
				LOGDEBUG(LD_SERIAL|LD_CONFIG, "Protocol PMDNG");
				fprintf(_F, "pm_ng\n");
				break;
			case KVM_AUXPORT_PROTOCOL_PPP_ONLY:
				LOGDEBUG(LD_SERIAL|LD_CONFIG, "Protocol PPP Only");
				fprintf(_F, "ppp_only\n");
				fprintf(_F, ps_serial, speed, datasize, stopbits, parity, flow, initchat);
				if (strcmp(autoppp, "")) {
					if ((pip = strstr(kap[i].autoppp, ":")) != NULL) {
						pip++;
						for (k=0;k < 20; k++) 
							if (*(pip+k) == ' ')
								break;
						if (k) {
							strncpy(ipno, pip, k+1)[k] = '\0';
							fprintf(_F, "%sipno %s\n", pref, ipno);
						}
					} 
					fprintf(_F, "%sauthtype %s\n", pref, strstr(kap[i].autoppp, "noauth") == NULL? "local" : "none");
					LOGDEBUG(LD_SERIAL|LD_CONFIG, "AutoPPP \n------\n[%s]\n-------", autoppp);
					fprintf (_F, "%spppopt %s\n", pref, autoppp);
				}
				fprintf (_F,"\n");
				break;
			case KVM_AUXPORT_PROTOCOL_LOGIN:
				LOGDEBUG(LD_SERIAL|LD_CONFIG, "Protocol Login");
				fprintf(_F, "login\n");
				fprintf(_F, "%sdcd 1\n", pref);
				fprintf(_F, ps_serial, speed, datasize, stopbits, parity, flow, initchat);
		} /* Finish writing to /etc/portslave/pslave.conf */
	}

	/* Writing [IPDU] section */
	if ( hasIpduSection ) {
		fprintf(_f, "%s", IpduSection);
	}
	LOGDEBUG(LD_SERIAL|LD_CONFIG, "Extra config");

	/* We're done - closing all files */	
	if (fclose(_f) != 0) {
		kvm_auxport_strerror_set ("Failure closing configuration file %s.",pm_conf_file);
		fclose(_F);
		return (KAP_EFILE);
	}
	LOGDEBUG(LD_SERIAL|LD_CONFIG, "File %s closed, Returning",pm_conf_file);

	if (fclose (_F) != 0) {
		kvm_auxport_strerror_set ("Failure closing configuration file %s.",ps_conf_file);
		return (KAP_EFILE);
	}
	LOGDEBUG(LD_SERIAL|LD_CONFIG, "File %s closed, Returning",ps_conf_file);

	return (KAP_OK);

	/**
	 * this part of the code is only reached if some other error
	 * occurred, so we just ignore errors here, and return the
	 * previous error code
	 */
_error_cleanup:
	fclose (_f);
	return (_current_error);
}

char* kvm_auxport_format_mstring (char *dst, char *src, int len) {
	int k = 0;
	int l = 0;
	memset (dst, 0, len);
	l = strlen (src);
	for (k = 0; k < l; k++) {
		switch (src[k]) {
			case ('\r'):
				/* just ignore the '\r' */
				break;
			case ('\n'):
				/* replace the '\n' with a '\' + new line + tab */
				strcat (dst, "\\\n\t");
				break;
			default:
				strncat (dst, &src[k], 1);
		}
	}
	return dst;
}
#else

/**
 * kvm_auxport_result kvm_auxport_config_read (kvm_auxport kap, char *config_file)
 *
 * Parameters
 *   kvm_auxport kap : the kvm_auxport object, which will be filled with
 *           all information parsed. Must be previously allocated with
 *           a call to kvm_auxport_new ()
 *   char *config_file : the configuration file to be parsed
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_auxport object not initialized
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
kvm_auxport_result kvm_auxport_config_read (kvm_auxport kap, char *config_file)
{
	int i = 0;
	int j = 0;
	int rc = 0;
	kvm_auxport_result _current_error = KAP_OK;
	int chars = 1024;
	char *line = NULL;
	char full_line[chars + 1];
	char *position = NULL;
	char *parameter = NULL;
	char *saved_option = NULL;
	char saved_parameter[1024]={0};
	char *option = NULL;
	FILE *_f = NULL;
	struct stat st;
	int idx = 0;
	char *termchar = NULL;
	int enable_port = 1;

	if (kap == NULL) {
		return (KAP_NULL);
	}

	_f = fopen (config_file, "r");
	if (_f == NULL) {
		kvm_auxport_strerror_set ("Configuration file %s was not found.", config_file);
		return (KAP_EFILE);
	}

	/**
	 * the current line being read
	 */
	line = (char *) calloc (chars + 1, sizeof (char));
	if (line == NULL) {
		return (KAP_EMEM);
	}

	stat(config_file,&st);
	kap[0].extra_config = calloc (st.st_size + 50, sizeof (char));
	kap[0].extra_config_size = st.st_size + 50;
	j = 1;
	while (fgets (line, chars, _f) != NULL) {
		memset (full_line, 0, chars + 1);
		strcpy (full_line, line);
		line = line_cleanup (line);

		/**
		 * check if the line:
		 *
		 * - is empty
		 * - does not start with 
		 * 's1.' (KVM/KVMNet)(everything else is non-important)
		 * 's1.' || 's2.' (KVMNet PLUS)
		 * 'a1.' || 'a2.' || 'a3.' (ONS)
		 */
		enable_port = 1;
		if (*line == '#') {
			line++;
			enable_port = 0;
		}

#ifdef ONS
		if ((strlen (line) == 0) || ((strncmp (line, "a1.", 3) != 0) &&
			(strncmp (line, "a2.", 3) != 0) &&
			(strncmp (line, "a3.", 3) != 0))) 
#else
		if ((strlen (line) == 0) || ((strncmp (line, "s1.", 3) != 0) &&
			    (strncmp(line, "s2.", 3) != 0))) 
#endif
		{
			strcat (kap[0].extra_config, full_line);
			j++;
			continue;
		}

		/**
		 * now parse the line
		 */
		if ((termchar = strchr(line, '.')) == NULL) {
			kvm_auxport_strerror_set ("Parse error (no dot on the line) on %s, line %d.",
				config_file, j); 
			_current_error = KAP_EPARSE;
			goto _error_cleanup;
		}
			
		*termchar='\0';
#ifdef ONS
		sscanf(line, "a%d", &idx);
#else
		sscanf(line, "s%d", &idx);
#endif
		if (idx <= 0 || idx > MAX_AUX) {
			if (enable_port) {
				kvm_auxport_strerror_set ("enabled auxport#%d greater than MAX(%d). line %d.",
					idx, MAX_AUX, j); 
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		}
		idx--;		
		kap[idx].auxidx = idx+1;
		*termchar='.';
		position = line + 3; /* 's1.' */

		/* there must be something after the '.' */
		if (*position == '\0') {
			kvm_auxport_strerror_set ("Parse error (no data after the dot) on %s, line %d.", config_file, j);
			_current_error = KAP_EPARSE;
			goto _error_cleanup;
		}

		for (i = 0;;i++) {
			option = position + i;
			if (isspace (*option) != 0) {
				/* we finally found the next parameter */
				break;
			}
		}
		option = position;
		option[i] = '\0';
		position = option + i + 1;

		if ((*option == '\0') || (*position == '\0')) {
			if (strcmp (option, "initchat") == 0) { /* s1.initchat "<initchat>" */
				/// initchat can have an empty string.
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d.", config_file, j);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		}

		/* find next non-blank char */
		for (i = 0;;i++) {
			parameter = position + i;
			if (isspace (*parameter) == 0) {
				break;
			}
		}

		/**
		 * now we check if this line continues on the next line
		 */
		i = strlen (parameter);
		if (parameter[i - 1] == '\\') {
			j++;
			saved_option = strdup (option);
			strcpy(saved_parameter,parameter);
			saved_parameter[i - 1] = '\n';
			if (saved_option == NULL) {
				_current_error = KAP_EMEM;
				goto _error_cleanup;
			}

			/* now we get the "clean" line, and add it into our buffer */
			while (fgets (line, chars, _f) != NULL) {
				line = line_cleanup (line);
				i = strlen (line);
				if ((i > 0) && ((strlen(saved_parameter)+i) < 1024)) {
					strcat (saved_parameter, line);
				}
				j++;
				i = strlen (saved_parameter);
				if (saved_parameter[i - 1] != '\\') {
					/* ok, this sequence was finished */
					break;
				}
				saved_parameter[i - 1] = '\n';
			}

			option = saved_option;
			parameter = strdup(saved_parameter);
		}

		/**
		 * test for strings inside ""
		 */
		if (parameter[0] == '"') {
			i = strlen (parameter);
			if (parameter[i - 1] != '"') {
				/* if the string starts with a '"', it must end with another '"' */
				if (saved_option != NULL) {
					free (saved_option);
					saved_option = NULL;
				}
				kvm_auxport_strerror_set ("Parse error on %s, at line %d.", config_file, j);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}

			/**
			 * remove the double brackets (") from the line
			 */
			position = parameter + 1;
			parameter[i - 1] = '\0';
			strcpy (parameter, position);
		}

		/**
		 * now we know which option to fill, and have the parameter for it.
		 */
		if (strcmp (option, "protocol") == 0) { /* s1.protocol <protocol_name> */
			if (strcmp (parameter, "ipdu") == 0) {
				kap[idx].protocol = KVM_AUXPORT_PROTOCOL_IPDU;
			} else if (strcmp (parameter, "ppp_only") == 0) {
				kap[idx].protocol = KVM_AUXPORT_PROTOCOL_PPP_ONLY;
			} else if (strcmp (parameter, "login") == 0) {
				kap[idx].protocol = KVM_AUXPORT_PROTOCOL_LOGIN;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid protocol type %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "speed") == 0) { /* s1.speed <speed> */
			kap[idx].speed = atoi (parameter);
			if (kap[idx].speed <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid port speed %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "datasize") == 0) { /* s1.datasize <datasize> */
			kap[idx].datasize = atoi (parameter);
			if (kap[idx].datasize <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid datasize %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "stopbits") == 0) { /* s1.stopbits <stopbits> */
			kap[idx].stopbits = atoi (parameter);
			if (kap[idx].stopbits <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid stopbits %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "parity") == 0) { /* s1.parity <parity> */
			if (strcmp (parameter, "none") == 0) {
				kap[idx].parity = KVM_AUXPORT_PARITY_NONE;
			} else if (strcmp (parameter, "odd") == 0) {
				kap[idx].parity = KVM_AUXPORT_PARITY_ODD;
			} else if (strcmp (parameter, "even") == 0) {
				kap[idx].parity = KVM_AUXPORT_PARITY_EVEN;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid parity type %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "flow") == 0) { /* s1.flow <flow> */
			if (strcmp (parameter, "none") == 0) {
				kap[idx].flow = KVM_AUXPORT_FLOW_NONE;
			} else if (strcmp (parameter, "soft") == 0) {
				kap[idx].flow = KVM_AUXPORT_FLOW_SOFT;
			} else if (strcmp (parameter, "hard") == 0) {
				kap[idx].flow = KVM_AUXPORT_FLOW_HARD;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid parity type %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "initchat") == 0) { /* s1.initchat "<initchat>" */
			if (kap[idx].initchat) {
				free(kap[idx].initchat);
				kap[idx].initchat = NULL;
			}
			if ( (parameter != NULL) ) {
				kap[idx].initchat = strdup (parameter);
				if (kap[idx].initchat == NULL) {
					_current_error = KAP_EMEM;
					goto _error_cleanup;
				}
			}
			
		} else if (strcmp (option, "pppopt") == 0) { /* s1.autoppp "<autoppp>" */
			if ((parameter != NULL) && (parameter[0] != '\0')) {
				if (kap[idx].autoppp != NULL) {
					free (kap[idx].autoppp);
				}
				kap[idx].autoppp = strdup (parameter);
				if (kap[idx].autoppp == NULL) {
					_current_error = KAP_EMEM;
					goto _error_cleanup;
				}
			}
		} else if (strcmp (option, "tty") == 0) { /* s1.tty ttyA1 */
			if ((parameter != NULL) && (parameter[0] != '\0')) {
				if (kap[idx].tty != NULL) {
					free (kap[idx].tty);
				}
				kap[idx].tty = strdup (parameter);
//#ifdef ONS
				if (enable_port == 0)
					kap[idx].enable = 0;
				else
					kap[idx].enable = 1;
//#endif
				if (kap[idx].tty == NULL) {
					_current_error = KAP_EMEM;
					goto _error_cleanup;
				}
			}
		} else if (strcmp (option, "pmtype") == 0) { /* s1.pmtype cyclades */
#ifdef OEM3
	char pmtype[] = "APCPDU";
	int  pmid = KVM_AUXPORT_PM_APC;
#else
	char pmtype[] = "cyclades";
	int  pmid = KVM_AUXPORT_PM_CYCLADES;
#endif
			if (strcmp (parameter, pmtype) == 0) {
				kap[idx].pmtype = pmid;
			} else {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid pm type %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "pmNumOfOutlets") == 0) { /* s1.pmNumOfOutlets <numOfOutlets> */
			kap[idx].pmNumOfOutlets = atoi (parameter);
			if (kap[idx].pmNumOfOutlets <= 0) {
				kvm_auxport_strerror_set ("Parse error on %s, line %d (invalid pmNumOfOutlets %s).", config_file, j, parameter);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
		} else if (strcmp (option, "authtype") &&
			   strcmp (option, "ipno")) { /* !s1.authtype */
			strcat (kap[0].extra_config, full_line);
		}

		/**
		 * before you add *anything* here, make sure you understand the last "else"
		 * statement above
		 */

		if (saved_option != NULL) {
			free (saved_option);
			saved_option = NULL;
		}
		j++;
	}

	free (line);

	rc = fclose (_f);
	if (rc != 0) {
		kvm_auxport_strerror_set ("Failure closing configuration file %s.", config_file);
		return (KAP_EFILE);
	}

	/**
	 * set the name of the config file which was read
	 * */
	for (i = 0; i < MAX_AUX; i++) {
		if (kap[i].filename != NULL) {
			free (kap[i].filename);
		}
		kap[i].filename = strdup (config_file);
	}

	return (KAP_OK);

	/**
	 * this part of the code is only reached if some other error
	 * occurred, so we just ignore errors here, and return the
	 * previous error code
	 */
_error_cleanup:
	free (line);
	fclose (_f);
	return (_current_error);
}

/**
 * kvm_auxport_result kvm_auxport_config_write (kvm_auxport kap, char *config_file)
 *
 * Parameters
 *   kvm_auxport kap : the kvm_auxport object, previously filled with
 *           information, on a call to kvm_auxport_config_read ()
 *   char *config_file : the configuration file to be written
 *
 * Return Values
 *   KC_OK : configuration file parsed successfully
 *   KC_NULL : kvm_auxport object not initialized
 *   KC_EFILE : could not open/write configuration file
 *   KC_EMEM : memory exhausted
 *   KC_EPARSE : failed to parse the kvm_auxport object - internal error
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
kvm_auxport_result kvm_auxport_config_write (kvm_auxport kap, char *config_file)
{
	int rc;
	kvm_auxport_result _current_error = KAP_OK;
	FILE *_f;
	char pref[10];
	int i;
#ifdef OEM3
	char pmtype[] = "APCPDU";
#else
	char pmtype[] = "cyclades";
#endif

	LOGDEBUG(LD_SERIAL | LD_CONFIG, "Writing %s", config_file);
	if (kap == NULL) {
		return (KAP_NULL);
	}

	_f = fopen (config_file, "w");
	if (_f == NULL) {
		kvm_auxport_strerror_set ("Could not open %s for writing.", config_file);
		return (KAP_EFILE);
	}

	LOGDEBUG(LD_SERIAL|LD_CONFIG, "File Opened");
	/**
 	* first, we write the fields which are not relevant to us
 	*/
	if (kap[0].extra_config) 
		fprintf (_f, "%s", kap[0].extra_config);
	//LOGDEBUG(LD_SERIAL|LD_CONFIG, "Writing extra config: \n-------\n[%s]\n-------", kap[0].extra_config);
	LOGDEBUG(LD_SERIAL|LD_CONFIG, "Extra config");

	for (i = 0; i < MAX_AUX; i++) {
		LOGDEBUG(LD_SERIAL|LD_CONFIG, "Writing config for AUX port %d", i);
#ifdef ONS
		sprintf(pref, "a%d.", kap[i].auxidx);
#else
		sprintf(pref, "s%d.", kap[i].auxidx);
#endif
		/**
	 	* then we write our own configuration
	 	*/
		switch (kap[i].protocol) { /* protocol */
		case (KVM_AUXPORT_PROTOCOL_IPDU):
			LOGDEBUG(LD_SERIAL|LD_CONFIG, "Protocol IPDU");
			/**
			 * write all IPDU-Only Information here
			 */
			fprintf (_f, "%sprotocol ipdu\n", pref);

			switch (kap[i].pmtype) { /* pmtype */
#ifdef OEM3
				case (KVM_AUXPORT_PM_APC):
#else
				case (KVM_AUXPORT_PM_CYCLADES):
#endif
					fprintf (_f, "%spmtype %s\n", pref, pmtype);
					break;

				default: /* no reason to fight ;) */
					fprintf (_f, "%spmtype %s\n", pref, pmtype);
					goto _error_cleanup;
			}

			if (kap[i].pmNumOfOutlets > 0) { /* pmNumOfOutlets */
				fprintf (_f, "%spmNumOfOutlets %d\n",
					 pref, kap[i].pmNumOfOutlets);
			} else {
				fprintf (_f, "%spmNumOfOutlets 8\n", pref);
			}

			break;
		case (KVM_AUXPORT_PROTOCOL_PPP_ONLY):
			LOGDEBUG(LD_SERIAL|LD_CONFIG, "Protocol PPP Only");
			/**
			 * write all PPP-Only Information here
			 */
			fprintf (_f, "%sprotocol ppp_only\n", pref);

			if (kap[i].speed <= 0) { /* speed */
				kvm_auxport_strerror_set ("Internal error: invalid speed %d selected.", kap[i].speed);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
			LOGDEBUG(LD_SERIAL|LD_CONFIG, "Speed %d", kap[i].speed);
			fprintf (_f, "%sspeed %d\n", pref, kap[i].speed);

			if (kap[i].datasize <= 0) { /* datasize */
				kvm_auxport_strerror_set ("Internal error: invalid datasize %d selected.", kap[i].datasize);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
			fprintf (_f, "%sdatasize %d\n", pref, kap[i].datasize);

			if (kap[i].stopbits <= 0) { /* stopbits */
				kvm_auxport_strerror_set ("Internal error: invalid stopbits %d selected.", kap[i].stopbits);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
			fprintf (_f, "%sstopbits %d\n", pref, kap[i].stopbits);

			switch (kap[i].parity) { /* parity */
				case (KVM_AUXPORT_PARITY_NONE):
					fprintf (_f, "%sparity none\n", pref);
					break;
				case (KVM_AUXPORT_PARITY_ODD):
					fprintf (_f, "%sparity odd\n", pref);
					break;
				case (KVM_AUXPORT_PARITY_EVEN):
					fprintf (_f, "%sparity even\n", pref);
					break;
				default:
					kvm_auxport_strerror_set ("Internal error: invalid parity %d selected.", kap[i].parity);
					_current_error = KAP_EPARSE;
					goto _error_cleanup;
			}

			switch (kap[i].flow) { /* flow */
				case (KVM_AUXPORT_FLOW_NONE):
					fprintf (_f, "%sflow none\n", pref);
					break;
				case (KVM_AUXPORT_FLOW_SOFT):
					fprintf (_f, "%sflow soft\n", pref);
					break;
				case (KVM_AUXPORT_FLOW_HARD):
					fprintf (_f, "%sflow hard\n", pref);
					break;
				default:
					kvm_auxport_strerror_set ("Internal error: invalid flow %d selected.", kap[i].flow);
					_current_error = KAP_EPARSE;
					goto _error_cleanup;
			}

			if (kap[i].initchat == NULL) { /* initchat */
				fprintf (_f, "%sinitchat   \n", pref);
			} else {
				/**
				 * initchat is a bit longer than the other configurations, so it
				 * gets a whole new indentation level
				 */
				int k = 0;
				int l = 0;
				char new_string[1024];

				memset (new_string, 0, 1024);
				l = strlen (kap[i].initchat);
				for (k = 0; k < l; k++) {
					switch (kap[i].initchat[k]) {
						case ('\r'):
							/* just ignore the '\r' */
							break;
						case ('\n'):
							/* replace the '\n' with a '\' + new line + tab */
							strcat (new_string, "\\\n\t");
							break;
						default:
							strncat (new_string, kap[i].initchat + k, 1);
					}
				}
				fprintf (_f, "%sinitchat   %s\n", pref, new_string);
			}

			if (kap[i].autoppp != NULL) { /* autoppp */
				/**
				 * autoppp is a bit longer than the other configurations, so it
				 * gets a whole new indentation level
				 */
				int k = 0;
				int l = 0;
				char new_string[1024],*pip;

				memset (new_string, 0, 1024);
				if ((pip = strstr(kap[i].autoppp,":")) != NULL) {
					pip++;
					for (k=0;k < 20; k++) 
						if (*(pip+k) == ' ')
							break;
					if (k) {
						strncpy(new_string,pip,k+1);
						new_string[k] = 0x00;
						fprintf(_f,"%sipno %s\n",pref, new_string);
						memset(new_string,0, k+1);
					}
				} 
				
				if (strstr(kap[i].autoppp, "noauth") == NULL) {
					// authentication is required
					fprintf(_f, "%sauthtype local\n", pref);
				} else {
					// no authentication
					fprintf(_f, "%sauthtype none\n", pref);
				}
	
				l = strlen (kap[i].autoppp);
				for (k = 0; k < l; k++) {
					switch (kap[i].autoppp[k]) {
						case ('\r'):
							/* just ignore the '\r' */
							break;
						case ('\n'):
							/* replace the '\n' with a '\' + new line + tab */
							strcat (new_string, "\\\n\t");
							break;
						default:
							strncat (new_string, kap[i].autoppp + k, 1);
					}
				}
				LOGDEBUG(LD_SERIAL|LD_CONFIG, "AutoPPP \n------\n[%s]\n-------", new_string);
				fprintf (_f, "%spppopt %s\n", pref, new_string);
			}

			break;

		case (KVM_AUXPORT_PROTOCOL_LOGIN):
			LOGDEBUG(LD_SERIAL|LD_CONFIG, "Protocol Login");
			/**
			 * write all Login Information here
			 */
			fprintf (_f, "%sprotocol login\n", pref);
			fprintf (_f, "%sdcd 1\n", pref);

			if (kap[i].speed <= 0) { /* speed */
				kvm_auxport_strerror_set ("Internal error: invalid speed %d selected.", kap[i].speed);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
			LOGDEBUG(LD_SERIAL|LD_CONFIG, "Speed %d", kap[i].speed);
			fprintf (_f, "%sspeed %d\n", pref, kap[i].speed);

			if (kap[i].datasize <= 0) { /* datasize */
				kvm_auxport_strerror_set ("Internal error: invalid datasize %d selected.", kap[i].datasize);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
			fprintf (_f, "%sdatasize %d\n", pref, kap[i].datasize);

			if (kap[i].stopbits <= 0) { /* stopbits */
				kvm_auxport_strerror_set ("Internal error: invalid stopbits %d selected.", kap[i].stopbits);
				_current_error = KAP_EPARSE;
				goto _error_cleanup;
			}
			fprintf (_f, "%sstopbits %d\n", pref, kap[i].stopbits);

			switch (kap[i].parity) { /* parity */
				case (KVM_AUXPORT_PARITY_NONE):
					fprintf (_f, "%sparity none\n", pref);
					break;
				case (KVM_AUXPORT_PARITY_ODD):
					fprintf (_f, "%sparity odd\n", pref);
					break;
				case (KVM_AUXPORT_PARITY_EVEN):
					fprintf (_f, "%sparity even\n", pref);
					break;
				default:
					kvm_auxport_strerror_set ("Internal error: invalid parity %d selected.", kap[i].parity);
					_current_error = KAP_EPARSE;
					goto _error_cleanup;
			}

			switch (kap[i].flow) { /* flow */
				case (KVM_AUXPORT_FLOW_NONE):
					fprintf (_f, "%sflow none\n", pref);
					break;
				case (KVM_AUXPORT_FLOW_SOFT):
					fprintf (_f, "%sflow soft\n", pref);
					break;
				case (KVM_AUXPORT_FLOW_HARD):
					fprintf (_f, "%sflow hard\n", pref);
					break;
				default:
					kvm_auxport_strerror_set ("Internal error: invalid flow %d selected.", kap[i].flow);
					_current_error = KAP_EPARSE;
					goto _error_cleanup;
			}

			if (kap[i].initchat == NULL) { /* initchat */
				fprintf (_f, "%sinitchat   \n", pref);
			} else {
				/**
				 * initchat is a bit longer than the other configurations, so it
				 * gets a whole new indentation level
				 */
				int k = 0;
				int l = 0;
				char new_string[1024];

				memset (new_string, 0, 1024);
				l = strlen (kap[i].initchat);
				for (k = 0; k < l; k++) {
					switch (kap[i].initchat[k]) {
						case ('\r'):
							/* just ignore the '\r' */
							break;
						case ('\n'):
							/* replace the '\n' with a '\' + new line + tab */
							strcat (new_string, "\\\n\t");
							break;
						default:
							strncat (new_string, kap[i].initchat + k, 1);
					}
				}
				fprintf (_f, "%sinitchat   %s\n", pref, new_string);
			}
			break;

		default:
			kvm_auxport_strerror_set ("Internal error: invalid protocol %d selected.", kap[i].protocol);
			_current_error = KAP_EPARSE;
			goto _error_cleanup;
		}

	/**
	 * KVM AUX Port is *always* on ttyA1
	 *
	 * (but we have the previous value of tty saved on kap->tty,
	 * just in case we need it someday)
	 */
//#ifdef ONS
        if (kap[i].enable == 0)
                fprintf (_f, "#%stty ttyA%d\n", pref, kap[i].auxidx);
        else
                fprintf (_f, "%stty ttyA%d\n", pref, kap[i].auxidx);
//#else
//        fprintf (_f, "%stty ttyA%d\n", pref, kap[i].auxidx);
//#endif
	}
	rc = fclose (_f);
	if (rc != 0) {
		kvm_auxport_strerror_set ("Failure closing configuration file %s.", config_file);
		return (KAP_EFILE);
	}
	LOGDEBUG(LD_SERIAL|LD_CONFIG, "File Closed, Returning");

	return (KAP_OK);

	/**
	 * this part of the code is only reached if some other error
	 * occurred, so we just ignore errors here, and return the
	 * previous error code
	 */
_error_cleanup:
	fclose (_f);
	return (_current_error);
}
#endif

/**
 * kvm_auxport_result kvm_auxport_free (kvm_auxport kc)
 *
 * Parameters
 *   kvm_auxport kap : the kvm_auxport object to be freed. Must be
 *           previously allocated with a call to kvm_auxport_new ()
 *
 * Return Values
 *   KAP_OK : kvm_auxport object was successfully freed
 *   KAP_NULL : function was called on a non-initialized kvm_auxport object
 *
 * External Resources
 *   The configuration file to be parsed
 *
 * Bugs / Environment changes
 *   None
 *
 * Comments
 *   It is a good practice to set kap to NULL, after you call this function
 */
kvm_auxport_result kvm_auxport_free (kvm_auxport kap)
{
	int i;
	if (kap == NULL) {
		return (KAP_NULL);
	}

	for (i = 0; i < MAX_AUX; i++) {
		if (kap[i].filename) {
			free(kap[i].filename);
			kap[i].filename = NULL;
		}
#ifdef PMDNG
		if (kap[i].extra_config_ps) {
			free(kap[i].extra_config_ps);
			kap[i].extra_config_ps = NULL;
		}
		if (kap[i].extra_config_pm) {
			free(kap[i].extra_config_pm);
			kap[i].extra_config_pm = NULL;
		}
#else
		if (kap[i].extra_config) {
			free(kap[i].extra_config);
			kap[i].extra_config = NULL;
		}
#endif
		if (kap[i].initchat) {
			free(kap[i].initchat);
			kap[i].initchat = NULL;
		}
		if (kap[i].autoppp) {
			free(kap[i].autoppp);
			kap[i].autoppp = NULL;
		}
		if (kap[i].tty) {
			free(kap[i].tty);
			kap[i].tty = NULL;
		}
	}
	free (kap);
	return (KAP_OK);
}

/**
 * const char *kvm_auxport_strerror (kvm_auxport_result error_number)
 *
 * Parameters
 *   kvm_auxport_result error_number : the numeric value of the error (returned by any of
 *           the kvm_auxport_* functions
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
 *   No kvm_auxport function prints any value to stdout and/or stderr,
 *   as those functions are meant to be used as library calls
 */
const char *kvm_auxport_strerror (kvm_auxport_result error_number)
{
	const char *msg = NULL;

	switch (error_number) {
		case (KAP_OK):
			msg = "Success.";
			break;
		case (KAP_NULL):
			msg = "kvm_auxport object not initialized. Please call kvm_auxport_new () before anything else.";
			break;
		case (KAP_EFILE):
			if (_kvm_auxport_strerror_message != NULL) {
				msg = _kvm_auxport_strerror_message;
			} else {
				msg = "Configuration file not found.";
			}
			break;
		case (KAP_EMEM):
			msg = "Memory exhausted.";
			break;
		case (KAP_EPARSE):
			if (_kvm_auxport_strerror_message != NULL) {
				msg = _kvm_auxport_strerror_message;
			} else {
				msg = "Could not parse configuration file.";
			}
			break;
		default:
			msg = "Unknown error. Please contact the development team.";
	}

	return (msg);
}

static void kvm_auxport_strerror_set (char *format, ...)
{
	va_list ap;

	if (_kvm_auxport_strerror_message == NULL) {
		_kvm_auxport_strerror_message = (char *) malloc (sizeof (char) * 1025);
	}
	memset (_kvm_auxport_strerror_message, 0, 1025);

	va_start (ap, format);
	vsnprintf (_kvm_auxport_strerror_message, 1024, format, ap);
	va_end (ap);
}

/**
 * - removes trailing '\n' and '\r'
 * - removes spaces & tabs from the start and end of line
 * - removes comments (comments start with the '#' char, and spawn to the rest of line
 * - make line all-lowercase
 */
static char *line_cleanup (char *line)
{
	int len;
	char *f;

	/* remove comments, onwards */
	/*if (*line) {
		f = index (line, '#');
		if (f != NULL) {
			*f = '\0';
		}
	}*/

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
//	for (f = line; *f != '\0'; f++) {
//		*f = tolower (*f);
//	}

	return (line);
}
