/*****************************************************************
* File: pmclib.h
*
* Copyright (C) 2006 Avocent Corporation
*
* www.avocent.com
*
* Description:
*     This file has the prototypes of the PM Client Library 
*     routines and the definition of the global variables
*     used by PM Client Library.
*
*****************************************************************/
#ifndef PMCLIB_H
#define PMCLIB_H

#ifdef PMCLIB_SRC
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <signal.h>
#endif

#include <power-mgmt-defs.h>
#include <shm-structs.h>
#include <power-mgmt-codes.h>
#include <shm-defs.h>
#include <debug.h>

#ifdef PMCLIB_SRC
//---------------
#include <sys/shm.h>

#define USER_ROOT  "root"
#ifdef KVM
#define DEF_DEVICE "ttyA"
#else
#define DEF_DEVICE "ttyS"
#endif
#define MAXMSGSIZE 1024
#define LBSIZE 255

extern char *gsp_power_api_username;
extern char localBuffer[];

#else //PMCLIB_SRC

extern char pmcOldId[];

#endif //PMCLIB_SRC
//-----------------

extern int pmcOldFormat;

typedef struct {
	char *sysname;
	char *desc;
	elecMonInfo *elecMon;
} elecMonList_t;

typedef enum {
	OFF = 0,
	ON
} PMC_Boolean;

typedef enum {
	SINGLE_OUTLET,
	OUTLET_GROUP,
	OUTLET_LIST,
	OUTLIST_IPDU,
	OUTLIST_CHAIN,
	OUTLIST_ALL,
	OUTLIST_INVALID
} Outlet_List;

typedef enum {
	ELEMENT_IPDU,
	ELEMENT_PHASE,
	ELEMENT_CIRCUIT,
	ELEMENT_OUTLET,
	ELEMENT_INVALID
} Element_Type;

typedef enum {
	PMC_SENSORUNIT_TEMP_C,
	PMC_SENSORUNIT_TEMP_F,
	PMC_SENSORUNIT_TEMP_K,
	PMC_SENSORUNIT_AF_FEED,
	PMC_SENSORUNIT_AF_METERS
} EnvMon_Unit;


// Communication with PMD core
// ---------------------------
int pmc_shm_init(void); // open the PM shared memory
int pmc_shm_init_safe(void); // open the PM shared memory
int pmc_pmd_status(void); // return the status of PMD core


// Validation routines
// -------------------
// validation of serial port
pmInfo *pmc_validate_port(int number, char *device); 
// validation of ipdu
ipduInfo *pmc_validate_ipdu(char *id);
ipduInfo *pmc_validate_ipdu_old(int port_number, int ipdu_number, char *device);
// validation of outlet
oData *pmc_validate_outlet_number(int port_number, int outlet_number, char *username, char *device);
oData *pmc_validate_outlet_name(char *name, char *username);
oData *pmc_validate_outlet_ipdu(char *ipdu_id, int outlet_number, char *username);
oData *pmc_validate_outlet_old(pmInfo *pminfo, int outlet_number, char *username);
oData *pmc_validate_outlet_sysname(ipduInfo *ipdu, char *sysname, char *username);
// validation of multiple outlets/outlet group
oData **pmc_validate_multout_name(char *group_name, char *username);
#ifdef PMCLIB_SRC
char *pmc_validate_multout_out(oData *outlet);
#endif
// special validation
int pmc_validate_access_one(char *username);
// special validation - return the list of outlet that the user has permission to manage
oData **pmc_validate_access_ipdu(char *username, ipduInfo *ipdu);
// NEW Validation routines
// -----------------------
phaseInfo *pmc_validate_phase(ipduInfo *ipdu, char *phase_name);
circuitInfo *pmc_validate_circuit(ipduInfo *ipdu, char *circuit_name);
envMonInfo *pmc_validate_envmon(ipduInfo *ipdu, char *envmon_name);


// Send Command routines
// ---------------------
// send outlet commands
int pmc_sendcmd_outlet(int command, char multout_flag, void *outlet, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_outcfg_interval(double interval,char multout_flag, void *outlet, unsigned int cap, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_outcfg_name(oData *outlet, char *name, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_outcfg_wakeup(char multout_flag, void *outlet, int state, char *username, void (*treatAnswer)(char *));
// send IPDU commands
int pmc_sendcmd_ipdu_reset_old(ipduInfo **ipdu, int len, int sensor, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipducfg_onoff(ipduInfo **ipdu, int len, unsigned int cap, PMC_Boolean  onoff, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipducfg_id(ipduInfo *ipdu, char *id, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipdu(ipduInfo **ipdu, int len, int command, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipdu_fwupgrade(ipduInfo *pdu, char *listpdus, char *file, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipdu_hwocp(ipduInfo *ipdu, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipducfg_alarm_ex(ipduInfo *ipdu, int phase, unsigned int alarm, int level, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipducfg_display(ipduInfo *ipdu, int display_mode, int cycle, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_ipducfg_interval(ipduInfo *ipdu, unsigned int cap, int delay, char *username, void (*treatAnswer)(char *));
// NEW Send Command routines
// -------------------------
int pmc_sendcmd_outlet_reset(char multout_flag, void *outlet, unsigned int cap, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_elecmon_reset(void *element, int element_type, unsigned int cap, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_outcfg_alarm(char multout_flag, void *outlet, unsigned int alarm, int level, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_eleccfg_alarm(void *element, int element_type, unsigned int alarm, int level, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_envmon_reset(envMonInfo *envmon, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_envmon_reset_all(ipduInfo *ipdu, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_envcfg_alarm(envMonInfo *envmon, unsigned int alarm, int level, char *username, void (*treatAnswer)(char *));
int pmc_sendcmd_envcfg_unit(ipduInfo *ipdu, int sensor_type, char *unit, char *username, void (*treatAnswer)(char *));


// Show routines
// -------------
// show PMD core answer
int pmc_show_answer(char *answer, int size, void (*treatAnswer)(char *), char param);
// show device detection status
void pmc_show_detection_all(void (*treatAnswer)(char *));
void pmc_show_detection_port(int port, void (*treatAnswer)(char *));
// show outlet data
void pmc_show_outlet_data(char multout_flag, void *outlet, char *textBuffer);
void pmc_show_outlet_delay(char multout_flag, void *outlet, int type, char *textBuffer);
void pmc_show_outlet_wakeup(char multout_flag, void *outlet, char *textBuffer);
void pmc_show_all_outlets(char *username);
// show outlet groups
void pmc_show_outlet_groups(char *username);
// show IPDU data
void pmc_show_ipdu_version(ipduInfo *ipdu);
void pmc_show_ipdu_current(ipduInfo *ipdu);
void pmc_show_ipdu_temperature(ipduInfo *ipdu);
void pmc_show_ipdu_alarmThreshold(ipduInfo *ipdu);
void pmc_show_ipdu_buzzer(ipduInfo *ipdu);
void pmc_show_ipdu_syslog(ipduInfo *ipdu);
void pmc_show_ipdu_overcurrent(ipduInfo *ipdu);
void pmc_show_ipdu_humidity(ipduInfo *ipdu);
void pmc_show_ipdu_voltage(ipduInfo *ipdu);
void pmc_show_ipdu_nearoverload(ipduInfo *ipdu);
void pmc_show_ipdu_lowload(ipduInfo *ipdu);
void pmc_show_ipdu_display(ipduInfo *ipdu);
void pmc_show_ipdu_hwocp(ipduInfo *ipdu);
void pmc_show_ipdu_rebootdelay(ipduInfo *ipdu);
void pmc_show_ipdu_seqinterval(ipduInfo *ipdu);
void pmc_show_ipdu_coldstartdelay(ipduInfo *ipdu);
void pmc_show_ipdu_power(ipduInfo *ipdu);
void pmc_show_ipdu_outlets(ipduInfo *ipduinfo, char *username);
void pmc_show_ipdu_outlet_delay(ipduInfo *ipduinfo, int type, char *username);
void pmc_show_ipdu_outlet_wakeup(ipduInfo *ipduinfo, char *username);
//int pmc_show_ipdu_sensor_overcurrent(ipduInfo *ipdu); /*not available*/
//int pmc_show_ipdu_ID (ipduInfo * ipdu);
// show list of ipdus 
void pmc_show_ipdu_list(void);
// show chain data
void pmc_show_chain_outlets(pmInfo *pminfo, char *username);
void pmc_show_chain_outlet_delay(pmInfo *pminfo, int type, char *username);
void pmc_show_chain_outlet_wakeup(pmInfo *pminfo, char *username);
void pmc_show_chain_version(pmInfo *pminfo, char * text);
void pmc_show_chain_temperature(pmInfo *pminfo, char * text);
void pmc_show_chain_current(pmInfo * pminfo, char * text);
// NEW Show routines
// -----------------
void pmc_show_elecmon_current(void *element, int element_type);
void pmc_show_elecmon_threshold(void *element, int element_type);
void pmc_show_elecmon_voltage(void *element, int element_type);
void pmc_show_elecmon_power(void *element, int element_type);
void pmc_show_elecmon_powerfactor(void *element, int element_type);
void pmc_show_elecmon_energy(void *element, int element_type);
void pmc_show_outlet_current(char multout_flag, void *outlet);
void pmc_show_outlet_threshold(char multout_flag, void *outlet);
void pmc_show_outlet_voltage(char multout_flag, void *outlet);
void pmc_show_outlet_power(char multout_flag, void *outlet);
void pmc_show_outlet_powerfactor(char multout_flag, void *outlet);
void pmc_show_outlet_energy(char multout_flag, void *outlet);
void pmc_show_envmon_value(ipduInfo *ipdu, int sensor_type);
void pmc_show_envmon_threshold(envMonInfo *envmon);
void pmc_show_envmon_threshold_all(ipduInfo *ipdu);
void pmc_show_envmon_unit(ipduInfo *ipdu, int sensor_type);


// Request Information routines
// ----------------------------
int pmc_numPhysicalPorts(char *device);
int pmc_numOutlets_pmInfo(pmInfo *pminfo);
int pmc_numOutlets_port(int port_number, char *device);
pmInfo *pmc_get_pmInfo_by_port(int port);
pmInfo *pmc_get_pmInfo_by_device(char *device);
pmInfo *pmc_get_pmInfo_by_id(const char *ipdu_id);
ipduInfo *pmc_get_ipduInfo_by_number(pmInfo *pm, int number);
int pmc_check_outlet_SPC_ServerTech(void *out);
pmInfo *pmc_first_pmInfo(void);
//int pmc_isServerTechAllowed (char * device);
int pmc_servername_out (oData * outlet);
#ifdef PMCLIB_SRC
pmInfo *pmc_get_pmInfo_by_logical_port(int port);
#endif


// Special routines
// ----------------
// wait device detection
void pmc_wait_detection_all(int timeout);
void pmc_wait_detection_port(int timeout, int port);
void pmc_answer_ipdu_error(ipduInfo *ipdu, int err, void (*treatAnswer)(char *));
#ifdef PMCLIB_SRC
int pmc_get_old_ipdu(char * ipduID);
int pmc_get_old_outlet(char * ipduID, int outletnumber);
int pmc_check_userperm(oData *outlet, char * username);
void pmc_answer_outlet_error(oData *pout, int err, char *measurement, void (*treatAnswer)(char *));
void pmc_answer_element_error(ipduInfo *ipdu, int element_type, char *element_name, char *measurement, int err, void (*treatAnswer)(char *));
void pmc_answer_envmon_error(ipduInfo *ipdu, char *stype, int err, void (*treatAnswer)(char *));
#endif


// event notification
void pmc_writeevt(int evtn, char *format,...);

int list_ipdu_related_elecmons(ipduInfo *ipdu,  elecMonList_t *elecMonList);

#endif //PMCLIB_H
