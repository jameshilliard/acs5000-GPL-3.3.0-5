/*****************************************************************
* File: pmc_show.c
*
* Copyright (C) 2006 Avocent Corporation
*
* www.avocent.com
*
* Description:
*     This file has the show routines of the PM Client Library.
*
*****************************************************************/
#define PMCLIB_SRC
#include <pmclib.h>
#include <pmc_show.h>
#include <unistd.h>
#include <linux/unistd.h>

// error messages
char * pmc_errormsg[] = {
// 1 - ERROR_IDLE_TIMEOUT
"idle timeout.", 
// 2 - ERROR_NOT_LOGGED
"[Error] - IPDU could not be logged in.",  
// 3 - ERROR_ACCESS_DENIED
"[Error] - Not have permission to manage the outlet(s).",
// 4 - ERROR_INVALID_IPDU_NUMBER
"[Error] - Invalid IPDU number.",
// 5 - ERROR_INVALID_IPDU_ID
"Invalid ID.",
// 6 - ERROR_DUPLICATED_IPDU_ID
"ID is already being used in the chain.",
// 7 - ERROR_INVALID_OUTLET_NUMBER
"This is not a valid outlet.",
// 8 - ERROR_INVALID_OUTLET_NAME
"Invalid outlet name.",
// 9 - ERROR_DUPLICATED_OUTLETNAME
"Name is already being used for outlet.",
//10 - ERROR_OUTLET_LOCKED
"Not possible, outlet locked.",
//11 - ERROR_OVERCURRENT
"Outlet disabled (over-current protection).",
//12 - ERROR_OUTLET_NOT_ON
"Outlet could not be turned on.",
//13 - ERROR_OUTLET_NOT_OFF
"Outlet could not be turned off.",
//14 - ERROR_INVALID_THRESHOLD
"Invalid current threshold value.",
//15 - ERROR_QUEUE_FULL
"[Error] - Command queue is full.",
//16 - ERROR_QUEUE_EMPTY
"[Error] - Command queue is empty.",
//17 - ERROR_RESTORE_FAILED
"Restore configuration from flash failed.",
//18 - ERROR_SAVE_DONE
"Saving configuration to flash on IPDU ... Done.",
//19 - ERROR_SAVE_FAILED
"Saving configuration to flash on IPDU ... Failed.",
//20 - ERROR_DEFAULT_DONE
"Setting configuration to defaults.",
//21 - ERROR_IPDU_NOT_READY
"[Error] - IPDU is not ready",
//22 - ERROR_IPDU_BUSY
"[Error] - IPDU is busy", 
//23 - ERROR_CONNECTION_LOST
"[Error] - Connection lost",
//24 - ERROR_IPDU_RESTARTED
"IPDU restarted",
//25 - ERROR_TIMEOUT
"[Error] - Timeout waiting",
//26 - ERROR_UNSUPPORTED_COMMAND
"[Error] - This command cannot be issued",
//27 - ERROR_RESTORE_DONE
"Restore configuration from flash done.",
//28 - ERROR_INVALID_INTERVAL
"[Error] - Invalid interval value.",
//29 - ERROR_HWOCP_RESET
"HW overcurrent protection is reset.",
//30 - ERROR_COMMAND_NOT_EXECUTED
"[Error] - Command not executed.",
//31 - ERROR_REBOOT_DONE
"Unit is rebooting.",
//32 - ERROR_UPGRADE_TIMEOUT
"[Error] - Upgrade communication timeout.",
//33 - ERROR_UPGRADE_ERROR
"[Error] - Upgrade communication error.",
//34 - ERROR_UPGRADE_DONE
"Upgrade successfully completed.",
// 35 ERROR_CURRENT_OVER
"Overcurrent detected.",
// 36 ERROR_CURRENT_BACK
"Current back to normal.",
// 37 ERROR_DEFAULT_DONE
//"Resetting IPDU and outlet configurations to defaults.",
// 37 ERROR_PASSWD_ERROR
"Password error",
// 38 ERROR_PASSWD_DONE
"Password done",
// 39 ERROR_INVALID_WAKEUP
"Invalid wakeup",
// 40 ERROR_INVALID_POWER_FACTOR
"Invalid power factor",
// 41 ERROR_OUTLET_FIXED_ON
"Outlet fixed ON",
};

char *pmc_PU_type[] = {
    "Cyclades",               /* Cyclades */
    "SPC",                    /* SPC */
    "ServerTech",             /* ServerTech */
    "Unknown"                 /* APC */
};

char *pmc_onoff[] = { "OFF", "ON" };
char *pmc_wakeupopt[] = { "OFF", "ON", "LAST" };

// answer for outlet commands 
char pmc_answer_outoldpre[] = "\r\n%d: "; // outlet number in the chain
char pmc_answer_outpre[] = "\r\n%s[%d]: "; // ipduID, outlet number
char pmc_answer_outlocpre[] = "\r\n%s-%c[%d]: "; // outlet number in the chain

char pmc_answer_control_status[] = "Outlet %s.";

char pmc_answer_outlet_control_off[] = "turned off";
char pmc_answer_outlet_control_on[] = "turned on";
char pmc_answer_outlet_control_cycle[] = "rebooting";
char pmc_answer_outlet_control_lock[] = "locked";
char pmc_answer_outlet_control_unlock[] = "unlocked";
char pmc_answer_outlet_control_undefined[] = "undefined";

char pmc_answer_status[] = "Outlet turned %s.";
char pmc_answer_lock[] = "Outlet locked.";
char pmc_answer_unlock[] = "Outlet unlocked.";
char pmc_answer_name[] = "Outlet now named %s.";
char pmc_answer_name0[] = "Outlet no longer has a name.";
char pmc_answer_minon[] = "Outlet minimum ON time set to %02d:%02d:%02d.";
char pmc_answer_minoff[] = "Outlet minimum OFF time set to %02d:%02d:%02d.";
char pmc_answer_poweron[] = "Outlet power on interval set to %.1f seconds.";
char pmc_answer_poweroff[] = "Outlet power off interval set to %.1f seconds.";
char pmc_answer_wakeup[] = "Wake up state set to %s.";
char pmc_answer_outlet_rebootdelay[] = "Cycle delay set to %.1f seconds.";
char pmc_answer_outlet_powerondelay[] = "Power on delay set to %.1f seconds.";
char pmc_answer_outlet_powerondelay_never[] = "Setting power on delay to never.";
char pmc_answer_outlet_poweroffdelay[] = "Power off delay set to %.1f seconds.";
char pmc_answer_outlet_poweroffdelay_never[] = "Setting power off delay to never.";

// answer for IPDU commands
char pmc_answer_ipduoldpre[] = "\r\nIPDU #%d: "; // ipdu number in the chain
char pmc_answer_ipdupre[] = "\r\n%s: "; // ipdu ID 
char pmc_answer_ipdulocpre[] = "\r\n%s-%c: "; // device name - position
char pmc_min_and_agv[] = ", minimum and average";
char pmc_answer_currentReset[] = "Clearing maximum%s recorded current%s%s%s to zero.";
char pmc_answer_voltageReset[] = "Clearing maximum%s recorded voltage%s%s%s to zero.";
char pmc_answer_powerReset[] = "Clearing maximum%s recorded power%s%s%s to zero.";
char pmc_answer_powerfactorReset[] = "Clearing maximum%s recorded power factor%s%s%s to zero.";
char pmc_answer_energyReset[] = "Clearing recorded cumulative power%s%s%s to zero.";
char pmc_answer_currentReset_out[] = "Clearing maximum%s recorded current for Outlet %d to zero.";
char pmc_answer_voltageReset_out[] = "Clearing maximum%s recorded voltage for Outlet %d to zero.";
char pmc_answer_powerReset_out[] = "Clearing maximum%s recorded power for Outlet %d to zero.";
char pmc_answer_powerfactorReset_out[] = "Clearing maximum%s recorded power factor for Outlet %d to zero.";
char pmc_answer_energyReset_out[] = "Clearing recorded cumulative power for Outlet %d to zero.";
char pmc_answer_temperatureReset[] = "Clearing maximum recorded temperature%s%s to zero.";
char pmc_answer_humidityReset[] = "Clearing maximum recorded humidity%s%s to zero.";
char pmc_answer_envmonReset[] = "Clearing maximum%s recorded %s for sensor %s to zero.";
char pmc_answer_currentprotection[] = "Overcurrent protection turned %s."; // ON/OFF
char pmc_answer_sens_currentprotection[] = "Overcurrent protection%s%s turned %s."; // ON/OFF
char pmc_answer_alarm[] = "Setting alarm threshold on phase %s to %.1fA.";
char pmc_answer_nearovld[] = "Setting near overload warning threshold on phase %s to %.1fA."; // A/B/C
char pmc_answer_nearlowload[] = "Setting near low load warning threshold on phase %s to %.1fA."; // A/B/C
char pmc_answer_lowload[] = "Setting low load warning threshold on phase %s to %.1fA."; // A/B/C
char pmc_answer_alarm_gsp[] = "Setting %s threshold%s%s%s to %.1fA.";
char pmc_answer_alarm_gsp_out[] = "Setting %s threshold on outlet %d to %.1fA.";
char pmc_answer_envmon_alarm_hc[] = "Setting thresholds on sensor %s: high critical to %.1f%s";
char pmc_answer_envmon_alarm[] = ", %s to %.1f%s";
char pmc_answer_envmon_unit[] = "Setting %s scale to %s.";
char pmc_answer_buzzer[] = "Buzzer turned %s."; //ON/OFF
char pmc_answer_display1[] = "Display mode set to %s";  
char pmc_answer_display2[] = "Display mode set to %s, show %s, and cycle manually.";
char pmc_answer_display3[] = "Display mode set to %s, show %s, and cycle each %d seconds.";
char pmc_answer_display4[] = "Display mode set to %s and cycle manually.";
char pmc_answer_display5[] = "Display mode set to %s and cycle each %d seconds.";
char pmc_answer_syslog[] = "Syslog turned %s."; // on/off
char pmc_answer_id[] = "ID is set to %s."; // id
char pmc_answer_rebootdelay[] = "Cycle Delay is set to %d seconds."; 
char pmc_answer_seqinterval[] = "Sequence Interval is set to %d seconds."; 
char pmc_answer_ipducoldstart[] = "IPDU cold start delay set to %d seconds.";
char pmc_answer_ipducoldstart_never[] = "IPDU cold start delay set to never.";

// outlet status
char pmc_status_header[]=
"\r\nOutlet                Name                Status          Post-on Delay(s)";

char pmc_status_outpre[] = "\r\n%s[%d]"; // ipduID, outlet number
char pmc_status_outoldpre[] = "\r\n%d"; // outlet number in the chain
char pmc_status_line[]=
// outname, Locked /Unlocked , OFF+/OFF?/ON?/ON!/OFF!/ON/OFF, interval
"%-19s %-15s %s"; 
char pmc_no_outlet[] = "\r\nNo outlet available.";

char pmc_show_poweronoff[] = "Outlet power %s interval is %.1f seconds.";
char pmc_show_minon[] = "Outlet mininum ON time is %02d:%02d:%02d.";
char pmc_show_minoff[] = "Outlet minimum OFF time is %02d:%02d:%02d.";
char pmc_show_wakeup[] = "The wake up status is %s.";
char pmc_show_power[] = "Power consumption%s%s%s: %.1fW%s.";
char pmc_show_maxpower[] =" Maximum power%s%s%s: %.1fW";  
char pmc_show_outlet_rebootdelay[] = "Cycle Delay is %.1f seconds.";
char pmc_show_outlet_powerondelay[] = "Power on delay is %.1f seconds.";
char pmc_show_outlet_powerondelay_never[] = "Power on delay is never.";
char pmc_show_outlet_poweroffdelay[] = "Power off delay is %.1f seconds.";
char pmc_show_outlet_poweroffdelay_never[] = "Power off delay is never.";
char pmc_show_ipducoldstart[] = "IPDU cold start delay is %d seconds.";
//char pmc_show_ipducoldstart_never[] = "IPDU cold start delay is never.";

char pmc_show_power_gsp[] = "Power%s%s%s: %.1fW.";
char pmc_show_power_gsp_out[] = "Power for Outlet %d: %.1fW.";
char pmc_show_power_factor[] = "Power factor%s%s%s: %.2f.";
char pmc_show_power_factor_out[] = "Power factor for Outlet %d: %.2f.";
char pmc_show_energy[] = "Cumulative power%s%s%s from %s: %.1fkWh.";
char pmc_show_energy_out[] = "Cumulative power of Outlet %d from %s: %.1fkWh.";

char pmc_show_outlet_status_off[] = "OFF";
char pmc_show_outlet_status_on[] = "ON";
char pmc_show_outlet_status_lock_off[] = "OFF(locked)";
char pmc_show_outlet_status_lock_on[] = "ON(locked)";
char pmc_show_outlet_status_cycle[] = "OFF(cycle)";
char pmc_show_outlet_status_pend_off[] = "ON(pend OFF)";
char pmc_show_outlet_status_pend_on[] = "OFF(pend ON)";
char pmc_show_outlet_status_pend_cycle[] = "OFF(pend cycle)";
char pmc_show_outlet_status_not_set[] = "NOT SET";
char pmc_show_outlet_status_fixed_on[] = "ON(fixed)";
char pmc_show_outlet_status_shutdown[] = "OFF(shutdown)";

char pmc_show_currentprotection[] =
"Overcurrent protection is %s."; // ON/OFF

char pmc_show_current[] = "RMS current%s%s%s: %.1fA.";  
char pmc_show_current_out[] = "RMS current for Outlet %d: %.1fA.";  
char pmc_show_maxcurrent[] = " Maximum current%s%s%s: %.1fA.";  
char pmc_show_maxcurrent_out[] = " Maximum current for Outlet %d: %.1fA.";  
char pmc_show_mincurrent[] = " Minimum current: %.1fA.";  
char pmc_show_avgcurrent[] = " Average current: %.1fA.";  

char pmc_show_alarm[] = "Alarm threshold%s%s%s is %.1fA.";
char pmc_show_alarmOld[] = "\r\nAlarm threshold on IPDU #%d%s%s%s is %.1fA.";
char pmc_show_alarms[] = "Thresholds%s%s%s: High Critical:%.1fA.";
char pmc_show_alarms_out[] = "Thresholds for outlet %d: High Critical:%.1fA.";
char pmc_show_alarm_warn_high_unit[] = " High Warning:%.1f%s.";
char pmc_show_alarm_warn_low_unit[] = " Low Warning:%.1f%s.";
char pmc_show_alarm_crit_low_unit[] = " Low Critical:%.1f%s.";

char pmc_show_sens_currentprotection[] =
"Overcurrent protection%s%s is %s.";

char pmc_show_sens_currentprotectionOld[] =
"\r\nOvercurrent protection on IPDU #%d is %s.";

char pmc_show_voltage[] = "RMS voltage%s%s: %.1fV%s.";
char pmc_show_maxvoltage[] = " Maximum voltage%s%s: %.1fV";
char pmc_show_voltage_phases[] = "RMS voltage between %c and %c: %.1fV%s.";
char pmc_show_maxvoltage_phases[] = " Maximum voltage between %c and %c: %.1fV";  
char pmc_show_voltage_gsp[] = "RMS voltage%s%s%s: %.1fV.";
char pmc_show_voltage_gsp_out[] = "RMS voltage on Outlet %d: %.1fV.";
char pmc_show_voltage_between_phases[] = "RMS voltage between phases %c and %c: %.1fV.";

char pmc_show_temperature[] = "Temperature%s%s: %.1f C (%.1f F).";
char pmc_show_maxtemperature[] = " Maximum: %.1f C (%.1f F).";
char pmc_show_mintemperature[] = " Minimum: %.1f C (%.1f F).";
char pmc_show_avgtemperature[] = " Average: %.1f C (%.1f F).";

char pmc_show_humidity[] =
"Humidity%s%s: %.1f%%.";

char pmc_show_maxhumidity[] =
" Maximum: %.1f%%.";

char pmc_show_buzzer[] =
"Buzzer is %s on ipdu.";

char pmc_show_buzzerOld[] =
"\r\nBuzzer is %s on ipdu #%d.";

char pmc_show_display1[] = "Display mode set to %s";  
char pmc_show_display2[] = "Display mode set to %s, show %s, and cycle manually.";
char pmc_show_display3[] = "Display mode set to %s, show %s, and cycle each %d seconds.";
char pmc_show_display4[] = "Display mode set to %s and cycle manually.";
char pmc_show_display5[] = "Display mode set to %s and cycle each %d seconds.";
char pmc_display_normal[] = "normal";
char pmc_display_inverted[] = "180 degrees rotated";
char pmc_display_current[] = "current";
char pmc_display_voltage[] = "voltage";

char pmc_show_ver_begin[] =
"Hw with %d outlets"; // numOutlets

char pmc_show_ver_maxcur[] =
" %d AMPs max"; // maxCurrent

char pmc_show_ver_fw[] =
" SW %s"; // firmware

char pmc_show_ver_os[] =
" OS %s"; // firmware

char pmc_show_ver_id[] =
" id %s"; // id

char pmc_show_ver_model[] =
" model %s"; // model

char pmc_show_ver_inptype[] =
" input %s"; // input type

char pmc_show_ver_end[] =
".";

char pmc_show_syslog[] =
"Syslog is %s."; // ON/OFF

char pmc_show_syslogOld[] =
"\r\nSyslog is %s on IPDU #%d."; // ON/OFF

char pmc_show_hwocp[] =
"HW overcurrent protection is %stripped.";

char pmc_show_rebootdelay[] =
"Cycle Delay is %d seconds.";

char pmc_show_seqinterval[] =
"Sequence Interval is %d seconds.";

char pmc_show_id[] =
"ID = %s."; // ipdu ID / <none>

char pmc_show_sensor_error[] =
"IPDU does not support %s measurement."; 

char pmc_show_max_unit[] = " Max:%.1f%s.";
char pmc_show_min_unit[] = " Min:%.1f%s.";
char pmc_show_avg_unit[] = " Average:%.1f%s.";

char pmc_show_pf_max_unit[] = " Max:%.2f.";
char pmc_show_pf_min_unit[] = " Min:%.2f.";
char pmc_show_pf_avg_unit[] = " Average:%.2f.";

char pmc_show_envmon[] = "%s%s: %.1f%s.";
char pmc_show_envmon_bool[] = "%ssensor %s is %s.";
char pmc_show_envmon_configured_unit[] = "%s scale on sensor %s: %s.";
char pmc_show_envmon_alarms[] = "%s thresholds on sensor %s: High Critical:%.1f%s.";
char pmc_show_envmon_unplugged[] = "Unplugged %s: n/a.";

/**
 * When changing this vector see the enumeration #ipduerr_table_index in pmc_show.h
 */
char pmca_ipducap_notsupported[] = "This ipdu does not support this capability.";
char pmca_ipducap_nosensor[] = "No sensor detected.";
char pmc_senscap_noovercurrent[] = "No overcurrent protection for the selected phase.";
char pmc_show_invrange[] = "Parameter out of range.";
char pmca_ipducommand_notsupported[] = "IPDU does not support this command.";
char pmca_ipdu_fixedon[] = "IPDU is not switchable. All outlets are Fixed ON.";
char *ipduerr_table[] = {
	pmca_ipducap_notsupported,
	pmca_ipducap_nosensor,
    pmc_senscap_noovercurrent,
    pmc_show_invrange,
	pmca_ipducommand_notsupported,
	pmca_ipdu_fixedon
};

/**
 * When changing this vector see the enumeration #outlerr_table_index in pmc_show.h
 */
char pmc_show_outlet_noperm[] = "User does not have permission to manage this outlet.";
char pmc_show_outcap_not_supported[] = "Outlet does not support this capability.";
char pmc_show_outcap_unknown_measurement[] = "Outlet does not support this measurement.";
char pmc_show_outcap_measurement_notsupported[] = "Outlet does not support %s measurement.";
char pmc_show_outcap_maxcap_notsupported[] = "Outlet does not support max %s capability.";
char pmc_show_outcap_no_threshold_alarms[] = "Outlet does not support threshold alarms.";
char pmc_show_outcap_no_spec_threshold_alarm[] = "Outlet does not support %s threshold alarm.";
char pmc_show_outcap_invalid_threshold_alarm[] = "Outlet does not support this threshold alarm.";
char pmc_show_outcap_pending_cmd[] = "Outlet has another pending command.";
char pmc_show_outcap_locked_outlet[] = "Outlet is locked.";
char *outlerr_table[] = {
	NULL, //since index 0 means NO ERROR
	pmc_show_outlet_noperm,
	pmc_show_outcap_not_supported,
	pmc_show_outcap_unknown_measurement,
	pmc_show_outcap_measurement_notsupported,
	pmc_show_outcap_maxcap_notsupported,
	pmc_show_outcap_no_threshold_alarms,
	pmc_show_outcap_no_spec_threshold_alarm,
	pmc_show_outcap_invalid_threshold_alarm,
	pmc_show_outcap_pending_cmd,
	pmc_show_outcap_locked_outlet
};

/**
 * When changing this vector see the enumeration #elemerr_table_index in pmc_show.h
 */
char pmca_elemerr_unknown_element[] = "Unknown element.";
char pmca_elemerr_unknown_measurement[] = "%s%s does not support this measurement.";
char pmca_elemerr_measurement_notsupported[] = "%s%s does not support %s measurement.";
char pmca_elemerr_maxcap_notsupported[] = "%s%s does not support max %s capability.";
char pmca_elemerr_no_threshold_alarms[] = "%s%s does not support threshold alarms.";
char pmca_elemerr_no_spec_threshold_alarm[] = "%s%s does not support %s threshold alarm.";
char pmca_elemerr_invalid_threshold_alarm[] = "%s%s does not support this threshold alarm.";
char *elemerr_table[] = {
	pmca_elemerr_unknown_element,
	pmca_elemerr_unknown_measurement,
	pmca_elemerr_measurement_notsupported,
	pmca_elemerr_maxcap_notsupported,
	pmca_elemerr_no_threshold_alarms,
	pmca_elemerr_no_spec_threshold_alarm,
	pmca_elemerr_invalid_threshold_alarm
};

/**
 * When changing this vector see the enumeration #envmerr_table_index in pmc_show.h
 */
char pmca_envmerr_cap_not_supported[] = "Sensor does not support this capability.";
char pmca_envmerr_unknown_sensor_type[] = "Unknown environmental sensor type.";
char pmca_envmerr_unknown_unit[] = "Unknown environmental sensor scale.";
char pmca_envmerr_unknown_arg_unit[] = "Unknown %s scale.";
char pmca_envmerr_sensor_notfound[] = "IPDU does not have %s sensor.";
char pmca_envmerr_sensors_notfound[] = "IPDU does not have any sensor.";
char pmca_envmerr_unit_sensors_notfound[] = "IPDU does not have any sensor supporting scale configuration.";
char pmca_envmerr_maxcap_notsupported[] = "%s sensor does not support reset.";
char pmca_envmerr_no_threshold_alarms[] = "%s sensor does not support threshold alarms.";
char pmca_envmerr_no_sensor_threshold_alarms[] = "IPDU does not have any sensor supporting threshold alarms.";
char pmca_envmerr_capunit_notsupported[] = "%s sensor does not support unit configuration.";
char pmca_envmerr_unit_notsupported[] = "%s sensor does not support this scale.";
char pmca_envmerr_no_threshold_alarm[] = "Sensor does not support this threshold alarm.";
char pmca_envmerr_threshold_value[] = "Invalid sensor threshold value.";
char *envmerr_table[] = {
	pmca_envmerr_cap_not_supported,
	pmca_envmerr_unknown_sensor_type,
	pmca_envmerr_unknown_unit,
	pmca_envmerr_unknown_arg_unit,
	pmca_envmerr_sensor_notfound,
	pmca_envmerr_sensors_notfound,
	pmca_envmerr_unit_sensors_notfound,
	pmca_envmerr_maxcap_notsupported,
	pmca_envmerr_no_threshold_alarms,
	pmca_envmerr_no_sensor_threshold_alarms,
	pmca_envmerr_capunit_notsupported,
	pmca_envmerr_unit_notsupported,
	pmca_envmerr_no_threshold_alarm,
	pmca_envmerr_threshold_value
};

char pmc_group_header[] = "\r\nGroup           Outlets\r\n";
char pmc_nogroup[] = "\r\n[Error] - There is no outlet group configured.\r\n";
char pmc_ipdulist_header[] = "\r\nIPDU ID           Location   Type        Outlets\r\n";
char pmc_noipdu[] = "\r\n[Error] - No IPDU was detected.\r\n";

// set by pmCommand to use the old format of messages
int pmcOldFormat;
char pmcOldId[IPDUIDSIZE + 1];

char localBuffer[LBSIZE];

char pmc_str_on[]  = " on ";
char pmc_str_for[] = " for ";
char pmc_str_of[] = " of ";
char pmc_str_between_phases[]  = " between phases ";
char pmc_str_est[]  = " (est.)";

#define ENVMON_TEMPERATURE_SENSOR   1
#define ENVMON_HUMIDITY_SENSOR      2

char phase_str[] = "phase ";
char bank_str[] = "bank ";
char outlet_str[] = "outlet ";
char capital_ipdu_str[] = "IPDU";
char capital_phase_str[] = "Phase ";
char capital_bank_str[] = "Bank ";
char capital_outlet_str[] = "Outlet ";

char current_str[] = "current";
char voltage_str[] = "voltage";
char power_str[] = "power";
char powerFactor_str[] = "power factor";
char energy_str[] = "cumulative power";
char critHigh_str[] = "high critical";
char warnHigh_str[] = "high warning";
char warnLow_str[] = "low warning";
char critLow_str[] = "low critical";

char temperature_str[] = "temperature";
char humidity_str[] = "humidity";
char airFlow_str[] = "air flow";
char smoke_str[] = "smoke";
char digital_str[] = "digital";
char dryContact_str[] = "dry contact";
char waterLevel_str[] = "water level";
char motion_str[] = "motion";
char capital_temperature_str[] = "Temperature ";
char capital_humidity_str[] = "Humidity ";
char capital_airFlow_str[] = "Air flow ";
char capital_smoke_str[] = "Smoke ";
char capital_dryContact_str[] = "Dry contact ";
char capital_waterLevel_str[] = "Water level ";
char capital_motion_str[] = "Motion ";

int list_ipdu_related_elecmons(ipduInfo *ipdu,  elecMonList_t *elecMonList)
{
    int i, cnt=0;

    if (!ipdu) return 0;

	elecMonList[cnt].desc = "";
	elecMonList[cnt].sysname = "";
	elecMonList[cnt++].elecMon = &(ipdu->elecMon);

	for (i=0; i < ipdu->num_phases; i++) {
		elecMonList[cnt].desc = phase_str;
		elecMonList[cnt].sysname = ipdu->phase[i].sysname;
		elecMonList[cnt++].elecMon = &(ipdu->phase[i].elecMon);
	}
	for (i=0; i < ipdu->num_circuits; i++) {
		elecMonList[cnt].desc = bank_str;
		elecMonList[cnt].sysname = ipdu->circuit[i].sysname;
		elecMonList[cnt++].elecMon = &(ipdu->circuit[i].elecMon);
	}

	elecMonList[cnt].elecMon = NULL;

    return cnt;
}

static int count_envmon_sensors(ipduInfo *ipdu, int type, envMonInfo **envMonList)
{
    int i, cnt=0;

    if (!ipdu) return 0;

	switch (type) {
	case ENVMON_TEMPERATURE_SENSOR:
		for (i=0; i < ipdu->num_envMons; i++) {
			if (ipdu->envMon[i].sensor_type == ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL ||
				ipdu->envMon[i].sensor_type == ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL ||
				ipdu->envMon[i].sensor_type == ENVMON_SENSOR_TEMPERATURE) {
				envMonList[cnt++] = &(ipdu->envMon[i]);
			}
		}
		break;
	case ENVMON_HUMIDITY_SENSOR:
		for (i=0; i < ipdu->num_envMons; i++) {
			if (ipdu->envMon[i].sensor_type == ENVMON_SENSOR_AVOCENT_HUM_INTERNAL ||
				ipdu->envMon[i].sensor_type == ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL ||
				ipdu->envMon[i].sensor_type == ENVMON_SENSOR_HUMIDITY) {
				envMonList[cnt++] = &(ipdu->envMon[i]);
			}
		}
		break;
	}
	envMonList[cnt] = NULL;

    return cnt;
}

int count_elecmon_caps(elecMonList_t *elecMonList, unsigned int caps)
{
    int i, cnt=0;

    for (i=0; elecMonList[i].elecMon; i++) {
        if (elecMonList[i].elecMon->sflag & caps) {
            cnt++;
        }
    }

    return cnt;
}

static char *get_control_command(unsigned char code) 
{ 
 	switch (code) { 
 		case OUTLET_CONTROL_OFF: 
 			return pmc_answer_outlet_control_off; 
 		case OUTLET_CONTROL_ON: 
 			return pmc_answer_outlet_control_on; 
 		case OUTLET_CONTROL_CYCLE: 
 			return pmc_answer_outlet_control_cycle; 
 		case OUTLET_CONTROL_LOCK: 
 			return pmc_answer_outlet_control_lock; 
 		case OUTLET_CONTROL_UNLOCK: 
 			return pmc_answer_outlet_control_unlock; 
 	} 
	return pmc_answer_outlet_control_undefined; 
} 

/* pmc_show_outlet_answer
 * treats the answer for outlet commands
 * <PROTO_ELEMENT_OUTLET_CONTROL_ID>
 * <PROTO_ELEMENT_OUTLET_POST_ON_INTERVAL>
 * <PROTO_ELEMENT_OUTLET_NAME>
 * <PROTO_ELEMENT_MESSAGE_ID> ==> error
 * ret -1 = error
 *     size of the text
 */
static int pmc_show_outlet_answer(char **ans, char *buf, int bsize, oData *outl, char param)
{
	int ret = 0;
	unsigned char *pans = *ans;
	int time = 0;
	double auxdbl=0;

	switch (*pans) {

	case PROTO_ELEMENT_OUTLET_CONTROL_ID:
		ret = snprintf(buf, bsize, pmc_answer_control_status, get_control_command(*(pans + 2))); 
		break; 

	case PROTO_ELEMENT_OUTLET_MINIMUM_ON_TIME:
		time = *(pans+2) * 0x100 + *(pans+3); 
		ret = snprintf(buf, bsize, pmc_answer_minon, time/60/60, (time/60)%60, time%60); 
		break; 

	case PROTO_ELEMENT_OUTLET_MINIMUM_OFF_TIME:
		time = *(pans+2) * 0x100 + *(pans+3); 
		ret = snprintf(buf, bsize, pmc_answer_minoff, time/60/60, (time/60)%60, time%60); 
		break; 

	case PROTO_ELEMENT_OUTLET_POST_ON_INTERVAL:
		auxdbl = (double)((*(pans+2) * 0x100) + (*(pans+3))) / 10; 
		ret = snprintf(buf, bsize, pmc_answer_poweron,auxdbl); 
		break; 

	case PROTO_ELEMENT_OUTLET_POST_OFF_INTERVAL:
		auxdbl = (double)((*(pans+2) * 0x100) + (*(pans+3))) / 10; 
		ret = snprintf(buf, bsize, pmc_answer_poweroff,auxdbl); 
		break; 

	case PROTO_ELEMENT_OUTLET_NAME:
		if (*(pans+1) > 2) {
			char name[OUTLETNAMESIZE + 1]; 
			strncpy(name, pans + 2, OUTLETNAMESIZE); 
			if (pans[1] - 2 < OUTLETNAMESIZE) {
				name[pans[1] - 2] = 0; 
			} else {
				name[OUTLETNAMESIZE] = 0;
			}
			ret = snprintf(buf, bsize, pmc_answer_name, name); 
		} else {
			ret = snprintf(buf, bsize, pmc_answer_name0);
		}
		break;

	case PROTO_ELEMENT_OUTLET_WAKEUP:
		ret = snprintf(buf, bsize, pmc_answer_wakeup,pmc_wakeupopt[(int)*(pans+2)]); 
		break; 

	case PROTO_ELEMENT_ELECMON_RESET_CURRENT:
		ret = snprintf(buf, bsize, pmc_answer_currentReset,
					   (outl->elecMon.sflag & ELECMON_CAP_MIN_CURRENT) ? pmc_min_and_agv : "", "", "", "");
		break;

	case PROTO_ELEMENT_ELECMON_RESET_VOLTAGE:
		ret = snprintf(buf, bsize, pmc_answer_voltageReset,
					   (outl->elecMon.sflag & ELECMON_CAP_MIN_VOLTAGE) ? pmc_min_and_agv : "", "", "", "");
		break;

	case PROTO_ELEMENT_ELECMON_RESET_POWER:
		ret = snprintf(buf, bsize, pmc_answer_powerReset,
					   (outl->elecMon.sflag & ELECMON_CAP_MIN_POWER) ? pmc_min_and_agv : "", "", "", "");
		break;

	case PROTO_ELEMENT_ELECMON_RESET_POWER_FACTOR:
		ret = snprintf(buf, bsize, pmc_answer_powerfactorReset,
					   (outl->elecMon.sflag & ELECMON_CAP_MIN_POWER_FACTOR) ? pmc_min_and_agv : "", "", "", "");
		break;

	case PROTO_ELEMENT_ELECMON_RESET_ENERGY:
		ret = snprintf(buf, bsize, pmc_answer_energyReset, "", "", "");
		break;

	case PROTO_ELEMENT_ELECMON_CUR_CRIT_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critHigh_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;

	case PROTO_ELEMENT_ELECMON_CUR_WARN_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnHigh_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;

	case PROTO_ELEMENT_ELECMON_CUR_LOW_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnLow_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;

	case PROTO_ELEMENT_ELECMON_CUR_LOWCRIT_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critLow_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;

	case PROTO_ELEMENT_MESSAGE_ID:
		ret = snprintf(buf, bsize, pmc_errormsg[pans[2]-1]);
		break;

	}

	pans += *(pans+1);
	*ans = pans;

	return(ret);
}

/* pmc_show_ipdu_answer
 * treats the answer for ipdu commands
 * <PROTO_ELEMENT_MESSAGE_ID> ==> error
 */
static int pmc_show_ipdu_answer(char **ans, char *buf, int bsize, ipduInfo *ipdu, char param)
{
	int ret = 0, aux;
	unsigned char *pans = *ans;
	char *porient, *ptype;

	switch (*pans) {

	case PROTO_ELEMENT_IPDU_SYSLOG:
		ret = snprintf(buf, bsize, pmc_answer_syslog, pmc_onoff[*(pans+2)]); 
		break; 
	case PROTO_ELEMENT_IPDU_BUZZER:
		ret = snprintf(buf, bsize, pmc_answer_buzzer, pmc_onoff[*(pans+2)]); 
		break; 
	case PROTO_ELEMENT_IPDU_OVERCURRENT:
		ret = snprintf(buf, bsize, pmc_answer_currentprotection, pmc_onoff[*(pans+2)]); 
		break; 

	case PROTO_ELEMENT_IPDU_DISPLAY:
		aux = (*(pans+1) == 3) ? 0 : *(pans+3); 
		if (pans[2] == IPDU_DISPLAY_NORMAL_CURRENT || pans[2] == IPDU_DISPLAY_NORMAL_VOLTAGE) { 
			porient = pmc_display_normal; 
		} else { 
			porient = pmc_display_inverted; 
		} 
		if (pans[2] == IPDU_DISPLAY_NORMAL_CURRENT || pans[2] == IPDU_DISPLAY_INVERTED_CURRENT) { 
			ptype = pmc_display_current; 
		} else { 
			ptype = pmc_display_voltage; 
		} 
		if (strstr(ipdu->model, "PM45") || strstr(ipdu->model, "PM42")) { 
			if (aux) { // cycle 
				ret = snprintf(buf, bsize, pmc_answer_display3, porient, ptype, aux); 
			} else { 
				ret = snprintf(buf, bsize, pmc_answer_display2, porient, ptype); 
			} 
		} else if (strstr(ipdu->model, "PM20")) { 
			if (aux) { // cycle 
				ret = snprintf(buf, bsize, pmc_answer_display5, porient, aux); 
			} else { 
				ret = snprintf(buf, bsize, pmc_answer_display4, porient); 
			} 
		} else { 
			ret = snprintf(buf, bsize, pmc_answer_display1, porient); 
		}
		break; 

	case PROTO_ELEMENT_IPDU_REBOOT_DELAY:
		ret = snprintf(buf, bsize, pmc_answer_rebootdelay, pans[2]*0x100 + pans[3]); 
		break; 
	case PROTO_ELEMENT_IPDU_SEQ_INTERVAL:
		ret = snprintf(buf, bsize, pmc_answer_seqinterval, pans[2]*0x100 + pans[3]); 
		break; 
	case PROTO_ELEMENT_IPDU_COLDSTART_DELAY:
		ret = snprintf(buf, bsize, pmc_answer_ipducoldstart, pans[2]*0x100 + pans[3]);
		break;

	case PROTO_ELEMENT_ELECMON_RESET_CURRENT:
		ret = snprintf(buf, bsize, pmc_answer_currentReset,
					   (ipdu->elecMon.sflag & ELECMON_CAP_MIN_CURRENT) ? pmc_min_and_agv : "", "", "", "");
		break;
	case PROTO_ELEMENT_ELECMON_RESET_VOLTAGE:
		ret = snprintf(buf, bsize, pmc_answer_voltageReset,
					   (ipdu->elecMon.sflag & ELECMON_CAP_MIN_VOLTAGE) ? pmc_min_and_agv : "", "", "", "");
		break;
	case PROTO_ELEMENT_ELECMON_RESET_POWER:
		ret = snprintf(buf, bsize, pmc_answer_powerReset,
					   (ipdu->elecMon.sflag & ELECMON_CAP_MIN_POWER) ? pmc_min_and_agv : "", "", "", "");
		break;
	case PROTO_ELEMENT_ELECMON_RESET_POWER_FACTOR:
		ret = snprintf(buf, bsize, pmc_answer_powerfactorReset,
					   (ipdu->elecMon.sflag & ELECMON_CAP_MIN_POWER_FACTOR) ? pmc_min_and_agv : "", "", "", "");
		break;
	case PROTO_ELEMENT_ELECMON_RESET_ENERGY:
		ret = snprintf(buf, bsize, pmc_answer_energyReset, "", "", "");
		break;

	case PROTO_ELEMENT_ELECMON_CUR_CRIT_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critHigh_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_WARN_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnHigh_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_LOW_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnLow_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_LOWCRIT_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critLow_str, "", "", "",
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;

	case PROTO_ELEMENT_MESSAGE_ID:
		ret = snprintf(buf, bsize, pmc_errormsg[pans[2]-1]);
		break;

	}

	pans += *(pans+1);
	*ans = pans;

	return(ret);
}

static int pmc_show_phase_answer(char **ans, char *buf, int bsize, phaseInfo *phase, char param)
{
	int ret = 0;
	unsigned char *pans = *ans;

	switch (*pans) {

	case PROTO_ELEMENT_ELECMON_CUR_CRIT_THRES:
//old printing for the same answer (don't know which function produced the request; sticking to the new stuff)
//		ret = snprintf(buf, bsize, pmc_answer_alarm, phase->sysname,
//					   (double)(pans[2]*0x100 + pans[3])/10);
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critHigh_str, pmc_str_for, phase_str, phase->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_WARN_THRES:
//old printing for the same answer (don't know which function produced the request; sticking to the new stuff)
//		ret = snprintf(buf, bsize, pmc_answer_nearovld, phase->sysname,
//					   (double)(pans[2]*0x100 + pans[3])/10);
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnHigh_str, pmc_str_for, phase_str, phase->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_LOW_THRES:
//old printing for the same answer (don't know which function produced the request; sticking to the new stuff)
//		ret = snprintf(buf, bsize, pmc_answer_nearlowload, phase->sysname,
//					   (double)(pans[2]*0x100 + pans[3])/10);
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnLow_str, pmc_str_for, phase_str, phase->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_LOWCRIT_THRES:
//old printing for the same answer (don't know which function produced the request; sticking to the new stuff)
//		ret = snprintf(buf, bsize, pmc_answer_lowload, phase->sysname,
//					   (double)(pans[2]*0x100 + pans[3])/10);
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critLow_str, pmc_str_for, phase_str, phase->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;

	case PROTO_ELEMENT_ELECMON_RESET_CURRENT:
		ret = snprintf(buf, bsize, pmc_answer_currentReset,
					   (phase->elecMon.sflag & ELECMON_CAP_MIN_CURRENT) ? pmc_min_and_agv : "",
					   pmc_str_for, phase_str, phase->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_VOLTAGE:
		ret = snprintf(buf, bsize, pmc_answer_voltageReset,
					   (phase->elecMon.sflag & ELECMON_CAP_MIN_VOLTAGE) ? pmc_min_and_agv : "",
					   pmc_str_for, phase_str, phase->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_POWER:
		ret = snprintf(buf, bsize, pmc_answer_powerReset,
					   (phase->elecMon.sflag & ELECMON_CAP_MIN_POWER) ? pmc_min_and_agv : "",
					   pmc_str_for, phase_str, phase->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_POWER_FACTOR:
		ret = snprintf(buf, bsize, pmc_answer_powerfactorReset,
					   (phase->elecMon.sflag & ELECMON_CAP_MIN_POWER_FACTOR) ? pmc_min_and_agv : "",
					   pmc_str_for, phase_str, phase->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_ENERGY:
		ret = snprintf(buf, bsize, pmc_answer_energyReset, pmc_str_for, phase_str, phase->sysname);
		break;

	case PROTO_ELEMENT_MESSAGE_ID:
		ret = snprintf(buf, bsize, pmc_errormsg[pans[2]-1]);
		break;

	}

	pans += *(pans+1);
	*ans = pans;

	return(ret);
}

static int pmc_show_circuit_answer(char **ans, char *buf, int bsize, circuitInfo *circuit, char param)
{
	int ret = 0;
	unsigned char *pans = *ans;

	switch (*pans) {

	case PROTO_ELEMENT_ELECMON_RESET_CURRENT:
		ret = snprintf(buf, bsize, pmc_answer_currentReset,
					   (circuit->elecMon.sflag & ELECMON_CAP_MIN_CURRENT) ? pmc_min_and_agv : "",
					   pmc_str_for, bank_str, circuit->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_VOLTAGE:
		ret = snprintf(buf, bsize, pmc_answer_voltageReset,
					   (circuit->elecMon.sflag & ELECMON_CAP_MIN_VOLTAGE) ? pmc_min_and_agv : "",
					   pmc_str_for, bank_str, circuit->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_POWER:
		ret = snprintf(buf, bsize, pmc_answer_powerReset,
					   (circuit->elecMon.sflag & ELECMON_CAP_MIN_POWER) ? pmc_min_and_agv : "",
					   pmc_str_for, bank_str, circuit->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_POWER_FACTOR:
		ret = snprintf(buf, bsize, pmc_answer_powerfactorReset,
					   (circuit->elecMon.sflag & ELECMON_CAP_MIN_POWER_FACTOR) ? pmc_min_and_agv : "",
					   pmc_str_for, bank_str, circuit->sysname);
		break;
	case PROTO_ELEMENT_ELECMON_RESET_ENERGY:
		ret = snprintf(buf, bsize, pmc_answer_energyReset, pmc_str_for, bank_str, circuit->sysname);
		break;

	case PROTO_ELEMENT_ELECMON_CUR_CRIT_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critHigh_str, pmc_str_for, bank_str, circuit->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_WARN_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnHigh_str, pmc_str_for, bank_str, circuit->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_LOW_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, warnLow_str, pmc_str_for, bank_str, circuit->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;
	case PROTO_ELEMENT_ELECMON_CUR_LOWCRIT_THRES:
		ret = snprintf(buf, bsize, pmc_answer_alarm_gsp, critLow_str, pmc_str_for, bank_str, circuit->sysname,
					   (double)(pans[2]*0x100 + pans[3])/10);
		break;

	case PROTO_ELEMENT_MESSAGE_ID:
		ret = snprintf(buf, bsize, pmc_errormsg[pans[2]-1]);
		break;
	}

	pans += *(pans+1);
	*ans = pans;

	return(ret);
}

// changed the logic because the answer for each command to set threshold contains:
// "the unit, the HC, the HW, the LW and the LC"
// 
static int pmc_show_envmon_answer(char **ans, char *buf, int bsize, envMonInfo *envmon, char param)
{
	int ret = 0, j, flag_thres=0;
	unsigned char *pans = *ans;
	char *sensortype = NULL, *unit = NULL;
	double value;

	// check if it was a threhsold command
	if ((param == PROTO_ELEMENT_ENVMON_CRIT_THRESHOLD) ||
		(param == PROTO_ELEMENT_ENVMON_CRIT_WARN_THRESHOLD) ||
		(param == PROTO_ELEMENT_ENVMON_LOW_THRESHOLD) ||
		(param == PROTO_ELEMENT_ENVMON_LOW_WARN_THRESHOLD)) {
		pans += *(pans+1);
		flag_thres = 1;
	}

	if (!flag_thres) {
		// it is not threshold answer
		switch (envmon->sensor_type) {
		case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
		case ENVMON_SENSOR_TEMPERATURE:
			sensortype = temperature_str;
			break;
		case ENVMON_SENSOR_AVOCENT_HUM_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
		case ENVMON_SENSOR_HUMIDITY:
			sensortype = humidity_str;
			break;
		case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
			sensortype = airFlow_str;
			break;
		}
		switch (*pans) {
		case PROTO_ELEMENT_ENVMON_RESET_VALUE:
			//not using "pmc_answer_temperatureReset" nor "pmc_answer_humidityReset" strings anymore,
			//since we don't know which function produced the request (sticking to the new stuff)
			ret = snprintf(buf, bsize, pmc_answer_envmonReset,
					   (envmon->sflag & ENVMON_CAP_MIN_VALUE) ? pmc_min_and_agv : "", sensortype, envmon->sysname);
			break;

		case PROTO_ELEMENT_ENVMON_UNIT:
			ret = snprintf(buf, bsize, pmc_answer_envmon_unit, sensortype, pans[2]==TempScaleC ? "oC" : "oF");
			flag_thres = 1;
			break;

		case PROTO_ELEMENT_MESSAGE_ID:
			ret = snprintf(buf, bsize, pmc_errormsg[pans[2]-1]);
			break;
		}
		pans += *(pans+1);
		if (flag_thres) {
			//if PROTO_ELEMENT_ENVMON_UNIT, skip next THRESHOLDs...
			while ((*pans == PROTO_ELEMENT_ENVMON_CRIT_THRESHOLD) ||
				   (*pans == PROTO_ELEMENT_ENVMON_CRIT_WARN_THRESHOLD) ||
				   (*pans == PROTO_ELEMENT_ENVMON_LOW_THRESHOLD) ||
				   (*pans == PROTO_ELEMENT_ENVMON_LOW_WARN_THRESHOLD)) {
				pans += *(pans+1);
			}
		}
		*ans = pans;

		return(ret);
	}

	// it is threshold answer == the protocol is sending the four values
	for (j=0; j < 4; j++) {
		value = pans[2]*0x100 + pans[3];
		switch (envmon->sensor_type) {
		case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
		case ENVMON_SENSOR_TEMPERATURE:
			sensortype = temperature_str;
			switch (envmon->unit) {
			case TempScaleC:
				unit = "oC";
				value = value - 2731.5;
				break;
			case TempScaleF:
				unit = "oF";
				value = value * 9 / 5 - 4596.7;
				break;
			default:
				unit = "oK";
			}
			break;
		case ENVMON_SENSOR_AVOCENT_HUM_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
		case ENVMON_SENSOR_HUMIDITY:
			sensortype = humidity_str;
			unit = "%";
			break;
		case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
			sensortype = airFlow_str;
			unit = "afu";
			break;
		}

		switch (*pans) {
		case PROTO_ELEMENT_ENVMON_CRIT_THRESHOLD:
			ret += snprintf(buf+ret, bsize-ret, pmc_answer_envmon_alarm_hc,
					   envmon->sysname, value/10, unit);
			break;
		case PROTO_ELEMENT_ENVMON_CRIT_WARN_THRESHOLD:
			ret += snprintf(buf+ret, bsize-ret, pmc_answer_envmon_alarm, warnHigh_str,
					   value/10, unit);
			break;
		case PROTO_ELEMENT_ENVMON_LOW_WARN_THRESHOLD:
			ret += snprintf(buf+ret, bsize-ret, pmc_answer_envmon_alarm, warnLow_str,
					   value/10, unit);
			break;
		case PROTO_ELEMENT_ENVMON_LOW_THRESHOLD:
			ret += snprintf(buf+ret, bsize-ret, pmc_answer_envmon_alarm, critLow_str,
					   value/10, unit);
			break;
		case PROTO_ELEMENT_MESSAGE_ID:
			ret += snprintf(buf+ret, bsize-ret, pmc_errormsg[pans[2]-1]);
			return(ret);
			break;
		default:
			return(ret);
		}
		pans += *(pans+1);
	}
	*ans = pans;
	return(ret);
}

/* pmc_show_answer
 * Translate the answer received from PMD, that is in PM protocol
 * format, to text format.
 * return: 0 on operation success; -1 otherwise
 * Parameters:
 *   answer - the answer received from the PMD core.
 *   size - the number of valid characters of the answer.
 *   treatAnswer - function pointer used to process text message
 *   param - first command argument
 * Answer Format:
 * <IPDU ID>
 * <outlet number> : when outlet answer
 * <command> or <PROTO_ELEMENT_MESSAGE_ID> == error
 * <arguments of command>
 */
int pmc_show_answer (char * answer, int size, void (* treatAnswer)(char *), char param)
{
	char *pans = answer;
	char *buffer;
	int lsize;
	int port, ipdu;
	pmInfo *pminfo;
	ipduInfo *ipduinfo;

	buffer = localBuffer;
	memset(buffer,0,LBSIZE);

/*  for TEST... */
//	int i;
//	printf("<<----- ");
//	for (i=0; i < size; i++)
//		printf("%02x ", pans[i]);
//	printf("\n");

	while (pans < answer + size) {
		lsize = 0;
		if (*pans != PROTO_ELEMENT_CHAIN_NUMBER) {
			// error in the answer
			syslog(4,"%s : invalid answer [%d]",__func__,*pans);
			return -1;
		}
		port = pans[2];
		pans += pans[1];
		pminfo = pmc_get_pmInfo_by_logical_port (port);
		if (pminfo == NULL) {
			syslog(4,"%s : port number does not exist [%d]",__func__, port);
			return -1;
		}
		if (*pans == PROTO_ELEMENT_MESSAGE_ID) {
			if (!pmcOldFormat) {
				lsize = sprintf(buffer,pmc_answer_ipdupre, pminfo->device);
			} else {
				lsize = sprintf(buffer,"\r\n");
			}
			sprintf(buffer+lsize, pmc_errormsg[pans[2]-1]);
			pans += pans[1];
		} else if (*pans == PROTO_ELEMENT_IPDU_NUMBER) {
			ipdu = pans[2];
			pans += pans[1];
			ipduinfo = pmc_get_ipduInfo_by_number (pminfo, ipdu);

			if (*pans == PROTO_ELEMENT_MESSAGE_ID &&
			   ((pans[2] == ERROR_UPGRADE_DONE) ||
				(pans[2] == ERROR_UPGRADE_TIMEOUT) ||
				(pans[2] == ERROR_UPGRADE_ERROR))) {
				sprintf(buffer, pmc_errormsg[pans[2]-1]);
				pans += pans[1];
				if (treatAnswer) {
					(* treatAnswer)(buffer);
				}
				memset(buffer,0,255);
				continue;
			}
			// workaround
			if (*pans == PROTO_ELEMENT_UPGRADE_IPDUS) {
				sprintf(buffer, "[Error] Unknown status of the upgrade - check the version of the PDUs");
				pans += pans[1];
				if (treatAnswer) {
					(* treatAnswer)(buffer);
				}
				memset(buffer,0,255);
				continue;
			}
			if (ipduinfo == NULL) {
				syslog(4,"%s : ipdu number does not exist [%d]",__func__, ipdu);
				return -1;
			}
			if (*pans == PROTO_ELEMENT_OUTLET_NUMBER) {
				oData *outlinfo = ipduinfo->outlet + (pans[2]-1);
				if (pmcOldFormat) {
					lsize = sprintf(buffer,pmc_answer_outoldpre, pmc_get_old_outlet(ipduinfo->id, pans[2]));
				} else if (ipduinfo->id[0] == 0) {
					lsize = sprintf(buffer,pmc_answer_outlocpre, pminfo->device, ipdu + 'A' - 1, pans[2]);
				} else {
					lsize = sprintf(buffer,pmc_answer_outpre, ipduinfo->id, pans[2]);
				}
				pans += pans[1];
				if (pmc_show_outlet_answer(&pans, buffer+lsize, LBSIZE-lsize, outlinfo, param) == -1) {
					return -1;
				}
			} else if (*pans == PROTO_ELEMENT_IPDU_ID) {
 				char id[IPDUIDSIZE + 1];
 				lsize = sprintf(buffer, pmc_answer_ipdupre, pmcOldId);
 				strncpy(id, pans + 2, IPDUIDSIZE);
				if (pans[1] - 2 < IPDUIDSIZE) {
					id[pans[1] - 2] = 0;
				} else {
					id[IPDUIDSIZE] = 0;
				}
 				sprintf(buffer+lsize, pmc_answer_id, id);
 				pans += pans[1];
			} else {
				if (pmcOldFormat) {
					lsize = sprintf(buffer,pmc_answer_ipduoldpre, ipdu);
				} else if (ipduinfo && ipduinfo->id[0]) {
					lsize = sprintf(buffer,pmc_answer_ipdupre, ipduinfo->id);
				} else {
					lsize = sprintf(buffer,pmc_answer_ipdulocpre, pminfo->device, ipdu + 'A' - 1);
				}
				if (*pans == PROTO_ELEMENT_PHASE_NUMBER) {
					phaseInfo *phaseinfo = ipduinfo->phase + (pans[2]-1);
					pans += pans[1];
					if (pmc_show_phase_answer(&pans, buffer+lsize, LBSIZE-lsize, phaseinfo, param) == -1) {
						return -1;
					}
				} else if (*pans == PROTO_ELEMENT_CIRCUIT_NUMBER) {
					circuitInfo *circuitinfo = ipduinfo->circuit + (pans[2]-1);
					pans += pans[1];
					if (pmc_show_circuit_answer(&pans, buffer+lsize, LBSIZE-lsize, circuitinfo, param) == -1) {
						return -1;
					}
				} else if (*pans == PROTO_ELEMENT_ENVMON_NUMBER) {
					envMonInfo *envmoninfo = ipduinfo->envMon + (pans[2]-1);
					pans += pans[1];
					if (pmc_show_envmon_answer(&pans, buffer+lsize, LBSIZE-lsize, envmoninfo, param) == -1) {
						return -1;
					}
				} else if (pmc_show_ipdu_answer(&pans, buffer+lsize, LBSIZE-lsize, ipduinfo, param) == -1) {
					return -1;
				}
			}
		} else {
			// error in the answer
			syslog(4,"%s : invalid answer [%d]",__func__,*pans);
			return -1;
		}
		if (treatAnswer) {
			(* treatAnswer)(buffer);
		}
		memset(buffer,0,255);
	}
	return 0;
}

static char *get_control_status_str (int code)
{
	switch (code) {
		case OUTLET_STATUS_OFF:
		case OUTLET_STATUS_WAKE_OFF:
		case OUTLET_STATUS_IDLE_OFF:
             return pmc_show_outlet_status_off;
		case OUTLET_STATUS_ON:
		case OUTLET_STATUS_WAKE_ON:
		case OUTLET_STATUS_IDLE_ON:
             return pmc_show_outlet_status_on;
		case OUTLET_STATUS_LOCK_OFF:
             return pmc_show_outlet_status_lock_off;
		case OUTLET_STATUS_LOCK_ON:
             return pmc_show_outlet_status_lock_on;
		case OUTLET_STATUS_CYCLE:
             return pmc_show_outlet_status_cycle;
		case OUTLET_STATUS_PEND_OFF:
             return pmc_show_outlet_status_pend_off;
		case OUTLET_STATUS_PEND_ON:
             return pmc_show_outlet_status_pend_on;
		case OUTLET_STATUS_PEND_CYCLE:
             return pmc_show_outlet_status_pend_cycle;
		case OUTLET_STATUS_NOT_SET:
             return pmc_show_outlet_status_not_set;
		case OUTLET_STATUS_FIXED_ON:
             return pmc_show_outlet_status_fixed_on;
		case OUTLET_STATUS_SHUTDOWN:
             return pmc_show_outlet_status_shutdown;
	}

	return "UNDEF";
}

/* pmc_show_list_outlet_status
 * return: the size of the line when buf is not NULL
 * Parameters: pointer to outlet data
 */
static int pmc_show_list_outlet_status(oData *out,char *buffer)
{
	int old_out;
	int size = 0;
	char aux[8];

//	debug(PMCLIB_SHOW, "pmcOldFormat[%d] out[%x] name[%s] onumber[%d]",pmcOldFormat,(int)out,out->name,out->onumber);

	// Outlet
	if (pmcOldFormat) {
		old_out = pmc_get_old_outlet(out->ipdu->id, out->onumber);
		if (buffer) {
			size = sprintf(buffer, pmc_status_outoldpre, old_out);
			while (size < 24) {
				buffer[size++] = ' ';	
			}
		} else {
			size = printf(pmc_status_outoldpre, old_out);
			if (size < 24) {
				printf("%*c",(24-size),' ');
			}
		}
	} else {
		if (buffer) {
			size = sprintf(buffer, pmc_status_outpre, out->ipdu->id, out->onumber);
			while (size < 24) {
				buffer[size++] = ' ';	
			}
		} else {
			size = printf(pmc_status_outpre, out->ipdu->id, out->onumber);
			if (size < 24) {
				printf("%*c",(24-size),' ');
			}
		}
	}

	if (out->sflag & OUTLET_CAP_POSTPOWERON) {
		snprintf(aux, 7, "%3.1f", (double)out->post_power_on/10);
	} else {
		strcpy(aux, "N/A");
	}

	if (buffer) {
		size += sprintf((buffer+size), pmc_status_line, out->name,
			get_control_status_str(out->status), aux);
	} else {
		size += printf(pmc_status_line, out->name,
			get_control_status_str(out->status), aux);
	}

	return size;
}

/* pmc_show_outlet_data
 *     Show outlet information.
 * Parameters:
 *   multout_flag - define the type of the outlet parameter
 *                  1 - oultet is the address of a vector
 *                      of oData pointers (list of outlets)
 *                  0 - outlet is oData pointer (one outlet)
 */
void pmc_show_outlet_data(char multout_flag, void *outlet, char *textBuf)
{
	oData **outlist = NULL;
	oData *out;
	char *buffer = NULL;
	int size = 0, tsize=0;
	int count = 0;
	int header = 0;

	if (multout_flag) {
		outlist = (oData **)outlet;
		out = outlist[count++];
	} else {
		out = (oData *)outlet;
	}
	
	debug(PMCLIB_SHOW, "pmcOldFormat[%d] out[%x] multout_flag[%d]", pmcOldFormat, (int)out, multout_flag);

	while (out) {

		if (textBuf) {
			tsize += size;
			buffer = textBuf+tsize;
			size = 0;
		}

		if (!header) {
			if (textBuf) {
				size = sprintf(buffer, pmc_status_header);
			} else {
				printf(pmc_status_header);
			}
			header = 1;
			tsize += size;
			buffer += size;
		}

		size = pmc_show_list_outlet_status(out, buffer);

		if (multout_flag) {
			out = outlist[count++];
		} else {
			break;
		}	
	}
}

/* pmc_show_line_outlet_delay
 * return: the size of the line when buffer is not NULL
 * type - type of the delay : OUTLET_CAP_MINIMUMON
 *                            OUTLET_CAP_MINIMUMOFF
 *                            OUTLET_CAP_POSTPOWERON
 *                            OUTLET_CAP_POSTPOWEROFF
 */
static int pmc_show_line_outlet_delay(oData *out, int type, char *buffer)
{
	int size = 0, old_out;
	double delay = 0;
	int time;
	char *msg = NULL;

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] out[%x] type[%d]", pmcOldFormat, (int)out, type);

	if (pmcOldFormat) {
		old_out = pmc_get_old_outlet(out->ipdu->id, out->onumber);
		if (buffer) {
			size = sprintf(buffer,pmc_answer_outoldpre, old_out);
		} else {
			printf(pmc_answer_outoldpre, old_out);
		}
	} else {
		if (buffer) {
			size = sprintf(buffer, pmc_answer_outpre, out->ipdu->id, out->onumber);
		} else {
			printf(pmc_answer_outpre, out->ipdu->id, out->onumber);
		}
	}

	switch (out->sflag & type) {
	case OUTLET_CAP_MINIMUMON:
		time = out->minimum_on;
		if (buffer) {
			size += sprintf((buffer+size), pmc_show_minon, time/60/60, 
							(time/60)%60, time%60);
		} else {
			printf(pmc_show_minon, time/60/60, (time/60)%60, time%60);
		}
		break;

	case OUTLET_CAP_MINIMUMOFF:
		time = out->minimum_off;
		if (buffer) {
			size += sprintf((buffer+size), pmc_show_minoff, time/60/60, 
							(time/60)%60, time%60);
		} else {
			printf(pmc_show_minoff, time/60/60, (time/60)%60, time%60);
		}
		break;

	case OUTLET_CAP_POSTPOWERON:
		delay = (double)out->post_power_on / 10;
		msg = pmc_show_poweronoff;	
		if (buffer) {
			size += sprintf((buffer+size), msg, "on", delay);
		} else {
			printf(msg, "on", delay);
		}
		break;

	case OUTLET_CAP_POSTPOWEROFF:
		delay = (double)out->post_power_off / 10;
		msg = pmc_show_poweronoff;	
		if (buffer) {
			size += sprintf((buffer+size), msg, "off", delay);
		} else {
			printf(msg, "off", delay);
		}
		break;

/* 	case OUTLET_CAP_REBOOTDELAY : */
/* 		delay = (double) out->cycle_delay / 10; */
/* 		msg = pmc_show_outlet_rebootdelay; */
/* 		if (buffer) { */
/* 			size += sprintf((buffer+size), msg, delay); */
/* 		} else { */
/* 			printf(msg, delay); */
/* 		} */
/* 		break; */

/* 	case OUTLET_CAP_POWERONDELAY : */
/* 		if (out->power_on_delay == PM_INFINITE_DELAY_16) { */
/* 			msg = pmc_show_outlet_powerondelay_never; */
/* 			if (buffer) { */
/* 				size += sprintf((buffer+size), msg); */
/* 			} else { */
/* 				printf(msg); */
/* 			} */
/* 		} else { */
/* 			delay = (double) out->power_on_delay / 10; */
/* 			msg = pmc_show_outlet_powerondelay; */
/* 			if (buffer) { */
/* 				size += sprintf((buffer+size), msg, delay); */
/* 			} else { */
/* 				printf(msg, delay); */
/* 			} */
/* 		} */
/* 		break; */

/* 	case OUTLET_CAP_POWEROFFDELAY : */
/* 		if (out->power_off_delay == PM_INFINITE_DELAY_16) { */
/* 			msg = pmc_show_outlet_poweroffdelay_never; */
/* 			if (buffer) { */
/* 				size += sprintf((buffer+size), msg); */
/* 			} else { */
/* 				printf(msg); */
/* 			} */
/* 		} else { */
/* 			delay = (double) out->power_off_delay / 10; */
/* 			msg = pmc_show_outlet_poweroffdelay; */
/* 			if (buffer) { */
/* 				size += sprintf((buffer+size), msg, delay); */
/* 			} else { */
/* 				printf(msg, delay); */
/* 			} */
/* 		} */
/* 		break; */

	default:
		if (buffer) {
			size += sprintf((buffer+size), pmc_show_outcap_not_supported);
		} else {
			printf(pmc_show_outcap_not_supported);
		}
	}

	return size;
}

/* pmc_show_outlet_delay
 *     Show delay times for an outlet.
 * Parameters:
 *   multout_flag - define the type of the outlet parameter
 *                  1 - oultet is the address of a vector
 *                      of oData pointers (list of outlets)
 *                  0 - outlet is oData pointer (one outlet)
 *           type - OUTLET_CAP_MINIMUMON
 *                  OUTLET_CAP_MINIMUMOFF
 *                  OUTLET_CAP_POSTPOWERON
 *                  OUTLET_CAP_POSTPOWEROFF
 */
void pmc_show_outlet_delay(char multout_flag, void *outlet, int type, char *textBuf)
{
	oData **outlist = NULL;
	oData *out;
	char *buffer = NULL;
	int size = 0;
	int count = 0;

	if (multout_flag) {
		outlist = (oData **)outlet;
		out = outlist[count++];
	} else {
		out = (oData *)outlet;
	}
	
	debug(PMCLIB_SHOW, "pmcOldFormat[%d] out[%x] multout_flag[%d]", pmcOldFormat, (int)out, multout_flag);

	while (out) {
		if (textBuf) {
			buffer = textBuf+size;
		}
		size += pmc_show_line_outlet_delay(out, type, buffer);
		if (multout_flag) {
			out = outlist[count++];
		} else {
			break;
		}	
	}
}

/* pmc_show_ipdu_outlet_delay
 *     Show delay times for outlets in the IPDU.
 * Parameters:
 *       ipdu - pointer to IPDU data
 *       type - OUTLET_CAP_MINIMUMON
 *              OUTLET_CAP_MINIMUMOFF
 *              OUTLET_CAP_POSTPOWERON
 *              OUTLET_CAP_POSTPOWEROFF
 *   username - used to check authorization
 */
void pmc_show_ipdu_outlet_delay(ipduInfo *ipdu, int type, char *username)
{
	oData *out;
	int outNI = 0;
	
	debug(PMCLIB_SHOW, "username[%s] pmcOldFormat[%d] ipdu[%x] ipduID[%s]", username, pmcOldFormat, (int)ipdu, ipdu->id);

	out = ipdu->outlet;	
	for (outNI = 0; outNI < ipdu->num_outlets; out++, outNI++) {
		if (username && !pmc_check_userperm(out, username)) {
			// the user does not have permission to manage this outlet
			continue;
		}
		pmc_show_line_outlet_delay(out, type, NULL);
	}
}

/* pmc_show_chain_outlet_delay
 *     Show delay times for outlets in the chain.
 * Parameters:
 *     pminfo - pointer to chain data
 *       type - OUTLET_CAP_MINIMUMON
 *              OUTLET_CAP_MINIMUMOFF
 *              OUTLET_CAP_POSTPOWERON
 *              OUTLET_CAP_POSTPOWEROFF
 *   username - used to check authorization
 */
void pmc_show_chain_outlet_delay(pmInfo *pminfo, int type, char *username)
{
	ipduInfo *ipdu;
	oData *out;
	int outNI = 0;
	
	debug(PMCLIB_SHOW, "username[%s] pmcOldFormat[%d] pminfo[%x] device[%s]", username, pmcOldFormat, (int)pminfo, pminfo->device);

	for (ipdu = pminfo->ipdu; ipdu; ipdu = ipdu->next) {
		if (!ipdu->ready || !ipdu->license) continue;
		out = ipdu->outlet;	
		for (outNI = 0; outNI < ipdu->num_outlets; out++, outNI++) {
			if (username && !pmc_check_userperm(out, username)) {
				// the user does not have permission to manage this outlet
				continue;
			}
			pmc_show_line_outlet_delay(out, type, NULL);
		}
	}
}

/* pmc_show_ipdu_version
 *     Show IPDU version.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_version(ipduInfo *ipdu)
{
	debug(PMCLIB_SHOW, "pmcOldFormat[%d] ipdu[%x] ipduId[%s]", pmcOldFormat, (int)ipdu, ipdu->id);

	if (!ipdu->ready || !ipdu->license) return;

	if (pmcOldFormat) {
		printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		printf(pmc_answer_ipdupre, ipdu->id);
	}

	printf(pmc_show_ver_begin, ipdu->num_outlets);
	if (ipdu->sflag & IPDU_CAP_MAXCURRENT) {
		printf(pmc_show_ver_maxcur, (int)ipdu->max_current/10);
	}
	if (ipdu->sflag & IPDU_CAP_VERSION) {
		printf(pmc_show_ver_fw, ipdu->version);
	}
	if (ipdu->sflag & IPDU_CAP_ID) {
		printf(pmc_show_ver_id, ipdu->id);
	}
	if (ipdu->sflag & IPDU_CAP_MODEL) {
		printf(pmc_show_ver_model, ipdu->model);
	}

	printf(pmc_show_ver_end);
}

/* pmc_show_ipdu_power
 *     Show IPDU power consumption.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_power(ipduInfo *ipdu)
{
	int old_ipdu, i;
	elecMonList_t elecMonList[30];

	if (!ipdu->ready || !ipdu->license) return;

	old_ipdu = pmc_get_old_ipdu(ipdu->id);

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] ipdu[%x] ipduN[%d] ipduID[%s]", pmcOldFormat, (int)ipdu, old_ipdu, ipdu->id);

	list_ipdu_related_elecmons(ipdu, elecMonList);

	if (!count_elecmon_caps(elecMonList, ELECMON_CAP_POWER)) {
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmc_show_sensor_error, power_str);
		return;
	}

	for (i=0; elecMonList[i].elecMon; i++) {
		if (!(elecMonList[i].elecMon->sflag & ELECMON_CAP_POWER)) {
			continue;
		}
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmc_show_power, elecMonList[i].desc[0] ? pmc_str_for : "", elecMonList[i].desc, elecMonList[i].sysname,
			   (double) elecMonList[i].elecMon->power / 10, (elecMonList[i].elecMon->power_type == ESTIMATED) ? pmc_str_est : "");
/* 		if (elecMonList[i].elecMon->sflag & ELECMON_CAP_MAX_POWER) { */
/* 			printf(pmc_show_maxpower, elecMonList[i].desc[0] ? pmc_str_for : "", elecMonList[i].desc, elecMonList[i].sysname, */
/* 				   (double)elecMonList[i].elecMon->max_power / 10); */
/* 		} */
	}
}

/* pmc_show_ipdu_voltage
 *     Show IPDU voltages.
 * Parameters:
 *   ipdu - the register with the IPDU data.
 */
void pmc_show_ipdu_voltage(ipduInfo *ipdu)
{
	int old_ipdu, i,flg_show=1;
	elecMonList_t elecMonList[30];

	if (!ipdu->ready || !ipdu->license) return;

	old_ipdu = pmc_get_old_ipdu(ipdu->id);

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] ipdu[%x] ipduN[%d] ipduID[%s]", pmcOldFormat, (int)ipdu, old_ipdu, ipdu->id);

	list_ipdu_related_elecmons(ipdu, elecMonList);

	if (!count_elecmon_caps(elecMonList, ELECMON_CAP_VOLTAGE)) {
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmc_show_sensor_error, voltage_str);
		return;
	}

	for (i=0; elecMonList[i].elecMon; i++) {
		if (!(elecMonList[i].elecMon->sflag & ELECMON_CAP_VOLTAGE)) {
			continue;
		}
		if (!strcmp(elecMonList[i].sysname, "XY") ||
			!strcmp(elecMonList[i].sysname, "YZ") ||
			!strcmp(elecMonList[i].sysname, "XZ")) { //this elecmon refers to the banks on PM42
			if (pmcOldFormat) {
				printf(pmc_answer_ipduoldpre, old_ipdu);
			} else {
				printf(pmc_answer_ipdupre, ipdu->id);
			}
			printf(pmc_show_voltage_phases, elecMonList[i].sysname[0], elecMonList[i].sysname[1], (double)elecMonList[i].elecMon->voltage / 10,
				   (elecMonList[i].elecMon->voltage_type == ESTIMATED) ? pmc_str_est : "");
			if (elecMonList[i].elecMon->sflag & ELECMON_CAP_MAX_VOLTAGE) {
				printf(pmc_show_maxvoltage_phases, elecMonList[i].sysname[0], elecMonList[i].sysname[1], (double)elecMonList[i].elecMon->max_voltage / 10);
			} 
		} else {
			if (elecMonList[i].desc[0]) { 
				//filtering out segments/phases (BugFix 56412)
				//except for ServerTech with 3 inlets
				if (flg_show && (ipdu->num_inlets == 3)) {
					// show PDU info and return
					if (pmcOldFormat) {
						printf(pmc_answer_ipduoldpre, old_ipdu);
					} else {
						printf(pmc_answer_ipdupre, ipdu->id);
					}
					printf(pmc_show_voltage, "", "", (double)elecMonList[i].elecMon->voltage / 10,
						   (elecMonList[i].elecMon->voltage_type == ESTIMATED) ? pmc_str_est : "");
					break; // stop

				}
				continue;
			}
			
			if (pmcOldFormat) {
				printf(pmc_answer_ipduoldpre, old_ipdu);
			} else {
				printf(pmc_answer_ipdupre, ipdu->id);
			}
			printf(pmc_show_voltage, "", elecMonList[i].sysname, (double)elecMonList[i].elecMon->voltage / 10,
				   (elecMonList[i].elecMon->voltage_type == ESTIMATED) ? pmc_str_est : "");
			if (elecMonList[i].elecMon->sflag & ELECMON_CAP_MAX_VOLTAGE) {
				printf(pmc_show_maxvoltage, "", elecMonList[i].sysname,(double)elecMonList[i].elecMon->max_voltage / 10);
			}
			flg_show = 0;
		}
	}
}

/* pmc_show_ipdu_temperature
 *     Show IPDU temperature sensors.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_temperature(ipduInfo *ipdu)
{
	int old_ipdu, i;
	double temp;
	envMonInfo *envMonList[30];

	if (!ipdu->ready || !ipdu->license) return;

	old_ipdu = pmc_get_old_ipdu(ipdu->id);

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] ipdu[%x] ipduN[%d] ipduID[%s]", pmcOldFormat, (int)ipdu, old_ipdu, ipdu->id);

	if (!count_envmon_sensors(ipdu, ENVMON_TEMPERATURE_SENSOR, envMonList)) {
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmca_envmerr_sensor_notfound, "temperature");
		return;
	}

	for (i=0; envMonList[i]; i++) {
		char *pon;
		if (*(envMonList[i]->name)) {
			pon = pmc_str_on;
		} else {
			pon = "";
		}
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		temp = (double)(envMonList[i]->value - 2730);
		printf(pmc_show_temperature, pon, envMonList[i]->sysname, (double)temp/10, (double)temp*9/50+32); 

		if (envMonList[i]->sflag & ENVMON_CAP_MAX_VALUE) {
			temp = (double)(envMonList[i]->max_value - 2730);
			printf(pmc_show_maxtemperature, (double)temp/10, (double)temp*9/50+32);
		}
		if (envMonList[i]->sflag & ENVMON_CAP_MIN_VALUE) {
			temp = (double)(envMonList[i]->min_value - 2730);
			printf(pmc_show_mintemperature, (double)temp/10, (double)temp*9/50+32);
		}
		if (envMonList[i]->sflag & ENVMON_CAP_AVG_VALUE) {
			temp = (double)(envMonList[i]->avg_value - 2730);
			printf(pmc_show_avgtemperature, (double)temp/10, (double)temp*9/50+32);
		}
	}
}

/* pmc_show_ipdu_humidity
 *     Show IPDU humidity sensors.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_humidity(ipduInfo *ipdu)
{
	int old_ipdu, i;
	envMonInfo *envMonList[30];

	if (!ipdu->ready || !ipdu->license) return;

	old_ipdu = pmc_get_old_ipdu(ipdu->id);

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] ipdu[%x] ipduN[%d] ipduID[%s]", pmcOldFormat, (int)ipdu, old_ipdu, ipdu->id);

	if (!count_envmon_sensors(ipdu, ENVMON_HUMIDITY_SENSOR, envMonList)) {
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmca_envmerr_sensor_notfound, "humidity");
		return;
	}

	for (i=0; envMonList[i]; i++) {
		char *pon;
		if (*(envMonList[i]->name)) {
			pon = pmc_str_on;
		} else {
			pon = "";
		}
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmc_show_humidity, pon, envMonList[i]->sysname, (double)envMonList[i]->value/10);
		if (envMonList[i]->sflag & ENVMON_CAP_MAX_VALUE) {
			printf(pmc_show_maxhumidity, (double)envMonList[i]->max_value/10); 
		}
	}
}

/* pmc_show_ipdu_alarmThreshold
 *     Show alarm threshold for IPDU.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_alarmThreshold(ipduInfo *ipdu)
{
	int old_ipdu, i;
	elecMonList_t elecMonList[30];
	
	if (!ipdu->ready || !ipdu->license) return;

	old_ipdu = pmc_get_old_ipdu(ipdu->id);

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] ipdu[%x] ipduN[%d] ipduID[%s]", pmcOldFormat, (int)ipdu, old_ipdu, ipdu->id);

	if(ipdu->pmInfo->ipdu_type!=IPDU_TYPE_CYCLADES){
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmca_ipducommand_notsupported);

		return;	
	}

	list_ipdu_related_elecmons(ipdu, elecMonList);

	if (!count_elecmon_caps(elecMonList, ELECMON_CAP_CUR_CRIT_THRES)) { 
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, old_ipdu);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmca_ipducap_notsupported);

		return;
	}

	for (i=0; elecMonList[i].elecMon; i++) {
		if (!(elecMonList[i].elecMon->sflag & ELECMON_CAP_CUR_CRIT_THRES)) {
			continue;
		}
		if (pmcOldFormat) {
			printf(pmc_show_alarmOld, old_ipdu, elecMonList[i].desc[0] ? " " : "", elecMonList[i].desc,
				   elecMonList[i].sysname, (double)elecMonList[i].elecMon->crit_threshold / 10);
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
			printf(pmc_show_alarm, elecMonList[i].desc[0] ? pmc_str_on : "", elecMonList[i].desc,
				   elecMonList[i].sysname, (double)elecMonList[i].elecMon->crit_threshold / 10);
		}
	}
}

/* pmc_show_ipdu_buzzer
 *     Show buzzer configuration for IPDU.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_buzzer(ipduInfo *ipdu)
{
	if (!ipdu->ready || !ipdu->license) return;

	if (pmcOldFormat) {
		if (ipdu->sflag & IPDU_CAP_BUZZER) {
			printf(pmc_show_buzzerOld, pmc_onoff[ipdu->buzzer], pmc_get_old_ipdu(ipdu->id));
		} else {
			printf(pmca_ipducap_notsupported);
		}
	} else {
		printf(pmc_answer_ipdupre, ipdu->id);
		if (ipdu->sflag & IPDU_CAP_BUZZER) {
			printf(pmc_show_buzzer, pmc_onoff[ipdu->buzzer]);
		} else {
			printf(pmca_ipducap_notsupported);
		}
	}
}

/* pmc_show_ipdu_syslog
 *     Show syslog configuration for IPDU.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_syslog(ipduInfo *ipdu)
{
	if (!ipdu->ready || !ipdu->license) return;

	if (pmcOldFormat) {
		if (ipdu->sflag & IPDU_CAP_SYSLOG) {
			printf(pmc_show_syslogOld, pmc_onoff[ipdu->syslog], pmc_get_old_ipdu(ipdu->id));
		} else {
			printf(pmca_ipducap_notsupported);
		}
	} else {
		printf(pmc_answer_ipdupre, ipdu->id);
		if (ipdu->sflag & IPDU_CAP_SYSLOG) {
			printf(pmc_show_syslog, pmc_onoff[ipdu->syslog]);
		} else {
			printf(pmca_ipducap_notsupported);
		}
	}
}

/* pmc_show_ipdu_overcurrent
 *     Show overcurrent configuration for IPDU.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_overcurrent(ipduInfo *ipdu)
{
	if (!ipdu->ready || !ipdu->license) return;

	if (pmcOldFormat) {
		printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		printf(pmc_answer_ipdupre, ipdu->id);
	}
	if (ipdu->sflag & IPDU_CAP_OVERCURRENT) {
		printf(pmc_show_currentprotection, pmc_onoff[ipdu->overcurrent]);
	} else {
		printf(pmca_ipducap_notsupported);
	}
}

/* pmc_show_ipdu_display
 *     Show LED display mode.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_display(ipduInfo *ipdu)
{
	if (!ipdu->ready || !ipdu->license) return;

	if (pmcOldFormat) {
		printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		printf(pmc_answer_ipdupre,ipdu->id);
	}

	if (ipdu->sflag & IPDU_CAP_DISPLAY) {
		char *porient, *ptype;
		if (ipdu->display == IPDU_DISPLAY_NORMAL_CURRENT || 
			ipdu->display == IPDU_DISPLAY_NORMAL_VOLTAGE) {
			porient = pmc_display_normal;
		} else {
			porient = pmc_display_inverted;
		}
		if (ipdu->display == IPDU_DISPLAY_NORMAL_CURRENT || 
			ipdu->display == IPDU_DISPLAY_INVERTED_CURRENT) {
			ptype = pmc_display_current;
		} else {
			ptype = pmc_display_voltage;
		}
		if (strstr(ipdu->model, "PM45") || strstr(ipdu->model, "PM42")) {
			if (ipdu->display_cycle) { // cycle
				printf(pmc_show_display3, porient, ptype, ipdu->display_cycle);
			} else {
				printf(pmc_show_display2, porient, ptype);
			}
		} else if (strstr(ipdu->model, "PM20")) {
			if (ipdu->display_cycle) { // cycle
				printf(pmc_show_display5, porient, ipdu->display_cycle);
			} else {
				printf(pmc_show_display4, porient);
			}
		} else {
			printf(pmc_show_display1, porient);
		}
	} else {
		printf(pmca_ipducap_notsupported);
	}
}

/* pmc_show_ipdu_hwocp
 *     Show HW Over Current protection.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_hwocp(ipduInfo *ipdu)
{
	if (!ipdu->ready || !ipdu->license) return;

	if (pmcOldFormat) {
		printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		printf(pmc_answer_ipdupre,ipdu->id);
	}

	if (ipdu->elecMon.sflag & ELECMON_CAP_HWOCP) {
		printf(pmc_show_hwocp, (ipdu->elecMon.hwocp)?"":"not ");
	} else {
		printf(pmca_ipducap_notsupported);
	}
}

/* pmc_show_chain_version
 *     Show the version of the chain.
 * Parameters:
 *       pminfo - the register with the chain data
 *   textBuffer - pointer where the text should be stored
 */
void pmc_show_chain_version(pmInfo *pminfo, char *textBuffer)
{
	int old_ipdu;
	ipduInfo *ipdu;
	char *buffer = textBuffer;
	int size = 0;

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] pminfo[%x] device[%s]", pmcOldFormat, (int)pminfo, pminfo->device);

	for (old_ipdu = 1, ipdu = pminfo->ipdu; ipdu; old_ipdu++, ipdu = ipdu->next) {
		if (!ipdu->ready || !ipdu->license) continue;

		if (textBuffer) {
			buffer += size;
		} else {
			if (size) printf(localBuffer);
			buffer = localBuffer;
			size = 0;
			memset(buffer, 0, 255);
		}
		if (pmcOldFormat) {
			size = sprintf(buffer,pmc_answer_ipduoldpre, old_ipdu);
		} else {
			size = sprintf(buffer,pmc_answer_ipdupre,ipdu->id);
		}
		size += sprintf((buffer+size), pmc_show_ver_begin, ipdu->num_outlets);
		if (ipdu->sflag & IPDU_CAP_MAXCURRENT) {
			size += sprintf((buffer+size), pmc_show_ver_maxcur, (int)ipdu->max_current/10);
		}
		if (ipdu->sflag & IPDU_CAP_VERSION) {
			size += sprintf((buffer+size), pmc_show_ver_fw, ipdu->version);
		}
		if (ipdu->sflag & IPDU_CAP_ID) {
			size += sprintf((buffer+size), pmc_show_ver_id, ipdu->id);
		}
		if (ipdu->sflag & IPDU_CAP_MODEL) {
			size += sprintf((buffer+size), pmc_show_ver_model, ipdu->model);
		}
		size += sprintf((buffer+size), pmc_show_ver_end);
	}
}

/* pmc_show_chain_temperature
 *     Show the temperature of the chain.
 * Parameters:
 *       pminfo - the register with the chain data
 *   textBuffer - pointer where the text should be stored
 */
void pmc_show_chain_temperature(pmInfo *pminfo, char *textBuffer)
{
	int old_ipdu;
	ipduInfo *ipdu;
	char *pon, *buffer = textBuffer;
	int i, size=0;
	double temp;
	envMonInfo *envMonList[30];

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] pminfo[%x] device[%s]", pmcOldFormat, (int)pminfo, pminfo->device);

	for (old_ipdu=1, ipdu=pminfo->ipdu; ipdu; old_ipdu++, ipdu=ipdu->next) {
		if (!ipdu->ready || !ipdu->license) continue;
		if (textBuffer) {
			buffer += size;
		} else {
			if (size) printf(localBuffer);
			buffer = localBuffer;
			size = 0;
			memset(buffer,0,255);
		}
		if (count_envmon_sensors(ipdu, ENVMON_TEMPERATURE_SENSOR, envMonList)) {
			for (i=0; envMonList[i]; i++) {
				if (*(envMonList[i]->name)) {
					pon = pmc_str_on;
				} else {
					pon = "";
				}
				if (pmcOldFormat) {
					size = sprintf(buffer,pmc_answer_ipduoldpre, old_ipdu);
				} else {
					size = sprintf(buffer,pmc_answer_ipdupre,ipdu->id);
				}
				temp = (double)envMonList[i]->value - 2730;
				size += sprintf((buffer+size), pmc_show_temperature, pon, envMonList[i]->name, 
								(double)temp/10, (double)temp*9/50+32);

				if (envMonList[i]->sflag & ENVMON_CAP_MAX_VALUE) {
					temp = (double)envMonList[i]->max_value - 2730;
					size += sprintf((buffer+size),pmc_show_maxtemperature,
									(double)temp/10, (double)temp*9/50+32); 
				}
				if (envMonList[i]->sflag & ENVMON_CAP_MIN_VALUE) {
					temp = (double)envMonList[i]->min_value - 2730;
					size += sprintf((buffer+size),pmc_show_mintemperature,
									(double)temp/10, (double)temp*9/50+32); 
				}
				if (envMonList[i]->sflag & ENVMON_CAP_AVG_VALUE) {
					temp = (double)envMonList[i]->avg_value - 2730;
					size += sprintf((buffer+size),pmc_show_avgtemperature,
									(double)temp/10, (double)temp*9/50+32); 
				}
			}
		} else {
			if (pmcOldFormat) {
				size = sprintf(buffer,pmc_answer_ipduoldpre, old_ipdu);
			} else {
				size = sprintf(buffer,pmc_answer_ipdupre,ipdu->id);
			}
			size += sprintf((buffer+size),pmca_envmerr_sensor_notfound, "temperature");
		}
	}
}

/* pmc_show_chain_current
 *     Show the current of the chain
 * Parameters:
 *       pminfo - the register with the chain data
 *   textBuffer - pointer where the text should be stored
 */
void pmc_show_chain_current(pmInfo *pminfo, char *textBuffer)
{
	int i, size=0;
	ipduInfo *ipdu;
	elecMonList_t elecMonList[30];
	
	debug(PMCLIB_SHOW, "pmcOldFormat[%d] pminfo[%x] device[%s]", pmcOldFormat, (int)pminfo, pminfo->device);

	for (ipdu = pminfo->ipdu; ipdu; ipdu = ipdu->next) {
		if (!ipdu->ready || !ipdu->license) continue;

		list_ipdu_related_elecmons(ipdu, elecMonList);

		for (i=0; elecMonList[i].elecMon; i++) {
			if (!(elecMonList[i].elecMon->sflag & ELECMON_CAP_CURRENT)) {
                continue;
            }
			if (pmcOldFormat) {
				if (textBuffer) {
					size += sprintf((textBuffer+size), pmc_answer_ipduoldpre, ipdu->number);	
				} else {
					printf(pmc_answer_ipduoldpre,ipdu->number);	
				}
			} else {
				if (textBuffer) {
					size += sprintf((textBuffer+size), pmc_answer_ipdupre, ipdu->id);
				} else {
					printf(pmc_answer_ipdupre, ipdu->id);
				}
			}
			if (textBuffer) {
				size += sprintf((textBuffer+size), pmc_show_current, elecMonList[i].desc[0] ? pmc_str_for : "",
								elecMonList[i].desc, elecMonList[i].sysname, (double)elecMonList[i].elecMon->current / 10);
				if (elecMonList[i].elecMon->sflag & ELECMON_CAP_MAX_CURRENT) {
					size += sprintf((textBuffer+size), pmc_show_maxcurrent, elecMonList[i].desc[0] ? pmc_str_for : "",
									elecMonList[i].desc, elecMonList[i].sysname, (double)elecMonList[i].elecMon->max_current / 10);
				}
				if (elecMonList[i].elecMon->sflag & ELECMON_CAP_MIN_CURRENT) {
					size += sprintf((textBuffer+size), pmc_show_mincurrent, (double)elecMonList[i].elecMon->min_current / 10);
				}
				if (elecMonList[i].elecMon->sflag & ELECMON_CAP_AVG_CURRENT) {
					size += sprintf((textBuffer+size), pmc_show_avgcurrent, (double)elecMonList[i].elecMon->avg_current / 10);
				}
			} else {
				printf(pmc_show_current, elecMonList[i].desc[0] ? pmc_str_for : "",
					   elecMonList[i].desc, elecMonList[i].sysname, (double)elecMonList[i].elecMon->current / 10); 
				if (elecMonList[i].elecMon->sflag & ELECMON_CAP_MAX_CURRENT) {
					printf(pmc_show_maxcurrent, elecMonList[i].desc[0] ? pmc_str_for : "",
						   elecMonList[i].desc, elecMonList[i].sysname, (double)elecMonList[i].elecMon->max_current / 10);
				}
				if (elecMonList[i].elecMon->sflag & ELECMON_CAP_MIN_CURRENT) {
					printf(pmc_show_mincurrent, (double)elecMonList[i].elecMon->min_current / 10);
				}
				if (elecMonList[i].elecMon->sflag & ELECMON_CAP_AVG_CURRENT) {
					printf(pmc_show_avgcurrent, (double)elecMonList[i].elecMon->avg_current / 10);
				}
			}
		}
	}
}

/* pmc_show_ipdu_outlets
 *     Show data for the outlets in the IPDU.
 * Parameters:
 *       ipdu - pointer to IPDU data
 *   username - used to check authorization
 */
void pmc_show_ipdu_outlets(ipduInfo *ipdu, char *username)
{
	oData *out;
	int outNI, header=0;

	debug(PMCLIB_SHOW, "username[%s] pmcOldFormat[%d] ipdu[%x] ipduID[%s]", username, pmcOldFormat, (int)ipdu, ipdu->id);

	if (!ipdu->ready || !ipdu->license) {
		return;
	}

	if (ipdu->num_outlets == 0) {
		if (pmcOldFormat) {
			printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
		} else {
			printf(pmc_answer_ipdupre, ipdu->id);
		}
		printf(pmca_ipdu_fixedon);
		return;
	}
	out = ipdu->outlet;	
	for (outNI = 0; outNI < ipdu->num_outlets; out++, outNI++) {
		if (username && !pmc_check_userperm(out,username)) {
			// the user does not have permission to manage this outlet
			continue;
		}
		if (!header) {
			printf(pmc_status_header);
			header = 1;
		}
		pmc_show_list_outlet_status(out,NULL);
	}

	if (!header) printf(pmc_no_outlet);
}

/* pmc_show_chain_outlets
 *     Show data for the outlets in the chain.
 * Parameters:
 *     pminfo - pointer to chain data
 *   username - used to check authorization
 */
void pmc_show_chain_outlets(pmInfo *pminfo, char *username)
{
	oData *out;
	int outNI, header=0;
	ipduInfo *ipdu;
	
	debug(PMCLIB_SHOW, "username[%s] pmcOldFormat[%d] pminfo[%x] device[%s]", username, pmcOldFormat, (int)pminfo, pminfo->device);

	for (ipdu = pminfo->ipdu; ipdu; ipdu = ipdu->next) {
		if (!ipdu->ready || !ipdu->license) continue;
		if (ipdu->num_outlets == 0) {
			if (pmcOldFormat) {
				printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
			} else {
				printf(pmc_answer_ipdupre, ipdu->id);
			}
			printf(pmca_ipdu_fixedon);
			continue;
		}
		out = ipdu->outlet;	
		debug(PMCLIB_SHOW,"ipdu[%x] iflag[%x] out[%x] flag[%x]",(int)ipdu, ipdu->flag, (int)out,out->flag);
		for (outNI = 0; outNI < ipdu->num_outlets; out++, outNI++) {
			if (username && !pmc_check_userperm(out,username)) {
				// the user does not have permission to manage this outlet
				continue;
			}
			if (!header) {
				printf(pmc_status_header);
				header = 1;
			}
			pmc_show_list_outlet_status(out,NULL);
		}
	}

	if (!header) printf(pmc_no_outlet);
}

/* pmc_show_all_outlets
 *     Show data for all outlets.
 * Parameters:
 *   username - used to check authorization
 */
void pmc_show_all_outlets(char *username)
{
	oData *out;
	int outNI, header=0;
	pmInfo *pm;
	ipduInfo *ipdu;
	
	for (pm = shmp->pmInfo; pm; pm = pm->next) {
		for (ipdu = pm->ipdu; ipdu; ipdu = ipdu->next) {
			if (!ipdu->ready || !ipdu->license) continue;
			if (ipdu->num_outlets == 0) {
				if (pmcOldFormat) {
					printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
				} else {
					printf(pmc_answer_ipdupre, ipdu->id);
				}
				printf(pmca_ipdu_fixedon);
				continue;
			}
			out = ipdu->outlet;	
			for (outNI = 0; outNI < ipdu->num_outlets; out++, outNI++) {
				if (username && !pmc_check_userperm(out,username)) {
					// the user does not have permission to manage this outlet
					continue;
				}
				if (!header) {
					printf(pmc_status_header);
					header = 1;
				}
				pmc_show_list_outlet_status(out, NULL);
			}
		}
	}

	if (!header) printf(pmc_no_outlet);
}

/* pmc_show_ipdu_list
 *     Show the list of IPDUs connected to the appliance.
 */
void pmc_show_ipdu_list(void)
{
	pmInfo *phys;
	ipduInfo *ipdu;
	int count, num=0;

	for (phys = shmp->pmInfo; phys; phys = phys->next) {
		count = 0;
		for (ipdu = phys->ipdu; ipdu; ipdu = ipdu->next, count++) {
			if (ipdu->ready && ipdu->license) {
				char location[11];
				if (!num) {
					printf(pmc_ipdulist_header);
					num = 1;
				}
				snprintf(location, 10, "%s-%c", phys->device, (char)('A'+count));
				printf("%-16s  %-10s %-10s  %d\r\n", ipdu->id, location,
					   pmc_PU_type[phys->ipdu_type - 1], 
					   ipdu->num_outlets);
			}
		}
	}

	if (!num) printf(pmc_noipdu);
}

/* pmc_show_outlet_groups
 *     Show outlet groups configured in the appliance.
 * Parameters:
 *   username - used to check authorization
 */
void pmc_show_outlet_groups(char *username)
{
	outletgroup *group = shmp->oGroups;
	groupmember *member;
	int count, ngrp=0;

	while (group) {
		/* validate the group first */
		member = group->groupmembers;
		for (count = 0; count < group->numOfMembers; count++, member++) {
			if (member->ipduname[0] == 0) {
				if (pmc_validate_outlet_name(member->outlname, username) == NULL) {
					break;
				}
			} else if ((member->ipduname[0] == '!') && (strchr(member->ipduname, '-') == NULL)) {
				if (pmc_validate_outlet_number(0, member->outletnum, username, member->ipduname + 1) == NULL) {
					break;
				}
			} else {
				if (pmc_validate_outlet_ipdu(member->ipduname, member->outletnum, username) == NULL) {
					break;
				}
			}
		}
		if (count == group->numOfMembers) {
			if (ngrp == 0) {
				printf(pmc_group_header);
			}
			ngrp ++;
			member = group->groupmembers;
			printf("%-16s", group->groupname);
			for (count = 0; count < group->numOfMembers; count++, member++) {
				if (count) printf(", ");
				if (member->ipduname[0]) {
					printf("%s[%d]", member->ipduname, member->outletnum);
				} else {
					printf("%s", member->outlname);
				}
			}
			printf("\r\n");
		}
		group = group->next;
	}

	if (ngrp == 0) printf(pmc_nogroup);
}

/* pmc_show_line_outlet_wakeup
 * return the size of the line if buffer is not NULL
 */
static int pmc_show_line_outlet_wakeup(oData *out, char *buffer)
{
	int size=0, old_out=0;

	debug(PMCLIB_SHOW, "pmcOldFormat[%d] out[%x]", pmcOldFormat, (int)out);

	if (pmcOldFormat) {
		old_out = pmc_get_old_outlet(out->ipdu->id, out->onumber);
		if (buffer) {
			size = sprintf(buffer, pmc_answer_outoldpre, old_out);
		} else {
			printf(pmc_answer_outoldpre, old_out);
		}
	} else {
		if (buffer) {
			size = sprintf(buffer, pmc_answer_outpre, out->ipdu->id, out->onumber);
		} else {
			printf(pmc_answer_outpre, out->ipdu->id, out->onumber);
		}
	}

	if (out->sflag & OUTLET_CAP_WAKEUP) {
		if (buffer) {
			size += sprintf((buffer+size), pmc_show_wakeup, pmc_wakeupopt[(int)out->wakeup]);
		} else {
			printf(pmc_show_wakeup, pmc_wakeupopt[(int)out->wakeup]);
		}
	} else {
		if (buffer) {
			size += sprintf((buffer+size), pmc_show_outcap_not_supported);
		} else {
			printf(pmc_show_outcap_not_supported);
		}
	}

	return size;
}

/* pmc_show_outlet_wakeup
 *     Show wake up state of the outlet (or group of outlets).
 * Parameters:
 *   multout_flag - define the type of the outlet parameter
 *                  1 - oultet is the address of a vector
 *                      of oData pointers (list of outlets)
 *                  0 - outlet is oData pointer (one outlet)
 */
void pmc_show_outlet_wakeup(char multout_flag, void *outlet, char *textBuf)
{
	oData **outlist = NULL;
	oData *out;
	char *buffer = NULL;
	int size = 0;
	int count = 0;

	if (multout_flag) {
		outlist = (oData **)outlet;
		out = outlist[count++];
	} else {
		out = (oData *)outlet;
	}
	
	debug(PMCLIB_SHOW, "pmcOldFormat[%d] out[%x] multout_flag[%d]", pmcOldFormat, (int)out, multout_flag);

	while (out) {
		if (textBuf) {
			buffer = textBuf + size;
		}
		size = pmc_show_line_outlet_wakeup(out, buffer);
		if (multout_flag) {
			out = outlist[count++];
		} else {
			break;
		}	
	}
}

/* pmc_show_chain_outlet_wakeup
 *     Show wake up state for the outlets in the chain.
 * Parameters:
 *   pminfo - pointer to chain data
 *   username - used to check authorization
 */
void pmc_show_chain_outlet_wakeup(pmInfo *pminfo, char *username)
{
	ipduInfo *ipdu;
	oData *out;
	int outNI;
	
	debug(PMCLIB_SHOW, "username[%s] pmcOldFormat[%d] pminfo[%x] device[%s]", username, pmcOldFormat, (int)pminfo, pminfo->device);

	for (ipdu=pminfo->ipdu; ipdu; ipdu=ipdu->next) {
		if (!ipdu->ready || !ipdu->license) continue;
		out = ipdu->outlet;	
		for (outNI=0; outNI < ipdu->num_outlets; out++, outNI++) {
			if (username && !pmc_check_userperm(out,username)) {
				// the user does not have permission to manage this outlet
				continue;
			}
			pmc_show_line_outlet_wakeup(out, NULL);
		}
	}
}

/* pmc_show_ipdu_outlet_wakeup
 *     Show wake up state for the outlets in the IPDU.
 * Parameters:
 *   ipdu - pointer to IPDU data
 *   username - used to check authorization
 */
void pmc_show_ipdu_outlet_wakeup(ipduInfo *ipdu, char *username)
{
	oData *out;
	int outNI = 0;
	
	debug(PMCLIB_SHOW, "username[%s] pmcOldFormat[%d] ipdu[%x] ipduID[%s]",username, pmcOldFormat, (int)ipdu, ipdu->id);

	if (!ipdu->ready || !ipdu->license) {
		return;
	}

	out = ipdu->outlet;	
	for (outNI = 0; outNI < ipdu->num_outlets; out++, outNI++) {
		if (username && !pmc_check_userperm(out,username)) {
			// the user does not have permission to manage this outlet
			continue;
		}
		pmc_show_line_outlet_wakeup(out, NULL);
	}
}

/* pmc_show_ipdu_rebootdelay
 *     Show reboot/cycle delay configuration.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_rebootdelay(ipduInfo *ipdu)
{
	if (pmcOldFormat) {
		printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		printf(pmc_answer_ipdupre, ipdu->id);
	}
	if (ipdu->sflag & IPDU_CAP_REBOOTDELAY) {
		printf(pmc_show_rebootdelay, ipdu->cycledelay);
	} else {
		printf(pmca_ipducap_notsupported);
	}
}

/* pmc_show_ipdu_seqinterval
 *     Show sequence interval configuration.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_seqinterval(ipduInfo *ipdu)
{
	if (pmcOldFormat) {
		printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		printf(pmc_answer_ipdupre, ipdu->id);
	}
	if (ipdu->sflag & IPDU_CAP_SEQINTERVAL) {
		printf(pmc_show_seqinterval, ipdu->seqinterval);
	} else {
		printf(pmca_ipducap_notsupported);
	}
}

/* pmc_show_ipdu_coldstartdelay
 *     Show cold start delay configuration.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_ipdu_coldstartdelay(ipduInfo *ipdu)
{
	if (pmcOldFormat) {
		printf(pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		printf(pmc_answer_ipdupre, ipdu->id);
	}
	if (ipdu->sflag & IPDU_CAP_COLDSTARTDELAY) {
		printf(pmc_show_ipducoldstart, ipdu->coldstart_delay);
	} else {
		printf(pmca_ipducap_notsupported);
	}
}

void pmc_answer_ipdu_error(ipduInfo *ipdu, int err, void (*treatAnswer)(char *))
{
	char buf[64];
	int size;

	if (!treatAnswer) return;

	if (pmcOldFormat) {
		size = sprintf(buf, pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		size = sprintf(buf, pmc_answer_ipdupre, ipdu->id);
	}
	sprintf(buf + size, ipduerr_table[err]);

	treatAnswer(buf);
}

void pmc_answer_outlet_error(oData *pout, int err, char *errarg, void (*treatAnswer)(char *))
{
	char buf[64];
	int size;

	if (!treatAnswer) return;

	if (pmcOldFormat) {
		int nout = pmc_get_old_outlet(pout->ipdu->id, pout->onumber);
		size = sprintf(buf, pmc_answer_outoldpre, nout);
	} else {
		size = sprintf(buf, pmc_answer_outpre, pout->ipdu->id, pout->onumber);
	}
	sprintf(buf + size, outlerr_table[err], errarg);

	treatAnswer(buf);
}

void pmc_answer_element_error(ipduInfo *ipdu, int element_type, char *element_name,
							  char *errarg, int err, void (*treatAnswer)(char *))
{
	char buf[64], *eletype;
	int size;

	if (!treatAnswer) return;

	if (pmcOldFormat) {
		size = sprintf(buf, pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		size = sprintf(buf, pmc_answer_ipdupre, ipdu->id);
	}

	if (element_type == ELEMENT_PHASE)
		eletype = capital_phase_str;
	else if (element_type == ELEMENT_CIRCUIT)
		eletype = capital_bank_str;
	else
		eletype = capital_ipdu_str;
	
	sprintf(buf + size, elemerr_table[err], eletype, element_name, errarg);

	treatAnswer(buf);
}

void pmc_answer_envmon_error(ipduInfo *ipdu, char *stype, int err, void (*treatAnswer)(char *))
{
	char buf[64];
	int size;

	if (!treatAnswer) return;

	if (pmcOldFormat) {
		size = sprintf(buf, pmc_answer_ipduoldpre, pmc_get_old_ipdu(ipdu->id));
	} else {
		size = sprintf(buf, pmc_answer_ipdupre, ipdu->id);
	}

	sprintf(buf + size, envmerr_table[err], stype);

	treatAnswer(buf);
}

// used by Plug-in
// unavailable ==> means detected but unlicensed
// available ==> means detected and licensed
// detecting ==> means "not ready"
void pmc_show_detection_all(void (*treatAnswer)(char *))
{
	pmInfo *pm;
	ipduInfo *ipdu;
	char line[128];
	int navail;	/* # of ipdus available for use. */
	int ndetect;	// # of detected pdus

	/* For each chain: */
	for (pm = shmp->pmInfo; pm; pm = pm->next) {
		if (!pm->got_num_ipdus) {
			sprintf(line, "%s unknown\r\n", pm->device);
			if (treatAnswer)
				treatAnswer(line);
			continue;
		}
		ndetect = navail = 0;
		/* For each ipdu in the chain: */
		for (ipdu = pm->ipdu; ipdu; ipdu = ipdu->next) {
			if (ipdu->ready) {
				ndetect++;
				if (ipdu->license) {
					navail++;
				}
			}
		}
		if (!(int)pm->num_ipdus || (ndetect < (int) pm->num_ipdus)) {
			sprintf(line, "%s detecting %s\r\n",
				pm->device, pmc_PU_type[pm->ipdu_type - 1]);
		} else {
			sprintf(line, "%s detected %s (%savailable)\r\n",
				pm->device, pmc_PU_type[pm->ipdu_type - 1],
				(navail < (int) pm->num_ipdus) ? "un" : "");
		}
		if (treatAnswer)
			treatAnswer(line);
	}
}


// used by Plug-in
// unavailable ==> means detected but unlicensed
// available ==> means detected and licensed
// detecting ==> means "not ready"
void pmc_show_detection_port(int port, void (*treatAnswer)(char *))
{
	pmInfo *pm;
	ipduInfo *ipdu;
	char line[128];
	int navail;	/* # of ipdus available for use. */
	int ndetect;	// # of detected pdus

	/* Find the port in the chain: */
	for (pm = shmp->pmInfo; pm; pm = pm->next) {
        
        if (pm->number == port) {
            if (!pm->got_num_ipdus) {
                sprintf(line, "%s unknown\r\n", pm->device);
                if (treatAnswer)
                    treatAnswer(line);
                break;
            }
            ndetect = navail = 0;
            /* For each ipdu in the chain: */
            for (ipdu = pm->ipdu; ipdu; ipdu = ipdu->next) {
                if (ipdu->ready) {
                    ndetect++;
                    if (ipdu->license) {
                        navail++;
                    }
                }
            }
            
            if (!(int)pm->num_ipdus || (ndetect < (int) pm->num_ipdus)) {
                sprintf(line, "%s detecting %s\r\n",
				pm->device, pmc_PU_type[pm->ipdu_type - 1]);
            } else {
                sprintf(line, "%s detected %s (%savailable)\r\n",
				pm->device, pmc_PU_type[pm->ipdu_type - 1],
				(navail < (int) pm->num_ipdus) ? "un" : "");
            }
            
            if (treatAnswer)
                treatAnswer(line);
            
            break;
        }
    }
}


// PMCLIB Extension below this point

#define PMC_MEASUREMENT_CURRENT       1
#define PMC_MEASUREMENT_VOLTAGE       2
#define PMC_MEASUREMENT_POWER         3
#define PMC_MEASUREMENT_POWER_FACTOR  4
#define PMC_MEASUREMENT_ENERGY        5

static void pmc_show_elecmon_measurement(void *element, int element_type, int measurement_type)
{
	char *eletype, *sysname, *eletypeerr, *time_str;
	elecMonInfo *elecMon;
	ipduInfo *ipdu;
	char outn[10];

	if (!element) return;

	switch (element_type) {
	case ELEMENT_IPDU:
		eletype = "";
		eletypeerr = capital_ipdu_str;
		sysname = "";
		elecMon = &(((ipduInfo *)element)->elecMon);
		ipdu = (ipduInfo *)element;
		break;
	case ELEMENT_PHASE:
		eletype = phase_str;
		eletypeerr = capital_phase_str;
		sysname = ((phaseInfo *)element)->sysname;
		elecMon = &(((phaseInfo *)element)->elecMon);
		ipdu = ((phaseInfo *)element)->ipdu;
		break;
	case ELEMENT_CIRCUIT:
		eletype = bank_str;
		eletypeerr = capital_bank_str;
		sysname = ((circuitInfo *)element)->sysname;
		elecMon = &(((circuitInfo *)element)->elecMon);
		ipdu = ((circuitInfo *)element)->ipdu;
		break;
	case ELEMENT_OUTLET:
		eletype = outlet_str;
		eletypeerr = capital_outlet_str;
		sprintf(outn,"%d",((oData *)element)->onumber);
		sysname = outn;
		elecMon = &(((oData *)element)->elecMon);
		ipdu = ((oData *)element)->ipdu;
		break;
	default:
		printf(pmca_elemerr_unknown_element);
		return;
	}

	debug(PMCLIB_SHOW, "ipduID[%s] elementType[%d] sysname[%s] measurement[%d]",
		  ipdu->id, element_type, sysname, measurement_type);

	if (!ipdu->ready || !ipdu->license) {
                return;
        }

	// new commands do not accept old format <IPDU #>
	// display new format <IPDU-ID>
	printf(pmc_answer_ipdupre, ipdu->id);

	switch (measurement_type) {
	case PMC_MEASUREMENT_CURRENT:
		if (!(elecMon->sflag & ELECMON_CAP_CURRENT)) {
			printf(pmca_elemerr_measurement_notsupported, eletypeerr, sysname, current_str);
			return;
		}
		printf(pmc_show_current, eletype[0] ? pmc_str_for : "", eletype, sysname, (double)elecMon->current / 10);

		if (elecMon->sflag & ELECMON_CAP_MAX_CURRENT) {
			printf(pmc_show_maxcurrent, eletype[0] ? pmc_str_for : "", eletype, sysname, (double)elecMon->max_current / 10);
		}
		if (elecMon->sflag & ELECMON_CAP_MIN_CURRENT) {
			printf(pmc_show_mincurrent, (double)elecMon->min_current / 10);
		}
		if (elecMon->sflag & ELECMON_CAP_AVG_CURRENT) {
			printf(pmc_show_avgcurrent, (double)elecMon->avg_current / 10);
		}
		break;
	case PMC_MEASUREMENT_VOLTAGE:
		if (!(elecMon->sflag & ELECMON_CAP_VOLTAGE)) {
			printf(pmca_elemerr_measurement_notsupported, eletypeerr, sysname, voltage_str);
			return;
		}
		if ((element_type != ELEMENT_PHASE) || (strlen(sysname) == 1)) {
			printf(pmc_show_voltage_gsp, eletype[0] ? pmc_str_on : "", eletype, sysname, (double)elecMon->voltage / 10);
		} else {
			printf(pmc_show_voltage_between_phases, sysname[0], sysname[1], (double)elecMon->voltage / 10);			
		}

		if (elecMon->sflag & ELECMON_CAP_MAX_VOLTAGE) {
			printf(pmc_show_max_unit, (double)elecMon->max_voltage / 10, "V");
		}
		if (elecMon->sflag & ELECMON_CAP_MIN_VOLTAGE) {
			printf(pmc_show_min_unit, (double)elecMon->min_voltage / 10, "V");
		}
		if (elecMon->sflag & ELECMON_CAP_AVG_VOLTAGE) {
			printf(pmc_show_avg_unit, (double)elecMon->avg_voltage / 10, "V");
		}

		if (elecMon->voltage_type == ESTIMATED) printf(pmc_str_est);
		break;
	case PMC_MEASUREMENT_POWER:
		if (!(elecMon->sflag & ELECMON_CAP_POWER)) {
			printf(pmca_elemerr_measurement_notsupported, eletypeerr, sysname, power_str);
			return;
		}
		printf(pmc_show_power_gsp, eletype[0] ? pmc_str_for : "", eletype, sysname, (double)elecMon->power / 10);

		if (elecMon->sflag & ELECMON_CAP_MAX_POWER) {
			printf(pmc_show_max_unit, (double)elecMon->max_power / 10, "W");
		}
		if (elecMon->sflag & ELECMON_CAP_MIN_POWER) {
			printf(pmc_show_min_unit, (double)elecMon->min_power / 10, "W");
		}
		if (elecMon->sflag & ELECMON_CAP_AVG_POWER) {
			printf(pmc_show_avg_unit, (double)elecMon->avg_power / 10, "W");
		}

		if (elecMon->power_type == ESTIMATED) printf(pmc_str_est);
		break;
	case PMC_MEASUREMENT_POWER_FACTOR:
		if (!(elecMon->sflag & ELECMON_CAP_POWER_FACTOR)) {
			printf(pmca_elemerr_measurement_notsupported, eletypeerr, sysname, powerFactor_str);
			return;
		}
		printf(pmc_show_power_factor, eletype[0] ? pmc_str_for : "", eletype, sysname, (double)elecMon->power_fac / 100);

		if (elecMon->sflag & ELECMON_CAP_MAX_POWER_FACTOR) {
			printf(pmc_show_pf_max_unit, (double)elecMon->max_power_fac / 100);
		}
		if (elecMon->sflag & ELECMON_CAP_MIN_POWER_FACTOR) {
			printf(pmc_show_pf_min_unit, (double)elecMon->min_power_fac / 100);
		}
		if (elecMon->sflag & ELECMON_CAP_AVG_POWER_FACTOR) {
			printf(pmc_show_pf_avg_unit, (double)elecMon->avg_power_fac / 100);
		}

		if (elecMon->power_fac_type == ESTIMATED) printf(pmc_str_est);
		break;
	case PMC_MEASUREMENT_ENERGY:
		if (!(elecMon->sflag & ELECMON_CAP_ENERGY)) {
			printf(pmca_elemerr_measurement_notsupported, eletypeerr, sysname, energy_str);
			return;
		}
        time_str = ctime((time_t*) &elecMon->energy_start);
        time_str[strlen(time_str)-1] = 0;
		printf(pmc_show_energy, eletype[0] ? pmc_str_of : "", eletype, sysname,
			   time_str, (double)elecMon->energy / 1000);
		break;
	}
}

/* pmc_show_elecmon_current
 *     Show current values for a given element.
 * Parameters:
 *        element - pointer the "element" data
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 */
void pmc_show_elecmon_current(void *element, int element_type)
{
	pmc_show_elecmon_measurement(element, element_type, PMC_MEASUREMENT_CURRENT);
}

/* pmc_show_elecmon_voltage
 *     Show voltage values for a given element.
 * Parameters:
 *        element - pointer the "element" data
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 */
void pmc_show_elecmon_voltage(void *element, int element_type)
{
	pmc_show_elecmon_measurement(element, element_type, PMC_MEASUREMENT_VOLTAGE);
}

/* pmc_show_elecmon_power
 *     Show power values for a given element.
 * Parameters:
 *        element - pointer the "element" data
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 */
void pmc_show_elecmon_power(void *element, int element_type)
{
	pmc_show_elecmon_measurement(element, element_type, PMC_MEASUREMENT_POWER);
}

/* pmc_show_elecmon_powerfactor
 *     Show power factor values for a given element.
 * Parameters:
 *        element - pointer the "element" data
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 */
void pmc_show_elecmon_powerfactor(void *element, int element_type)
{
	pmc_show_elecmon_measurement(element, element_type, PMC_MEASUREMENT_POWER_FACTOR);
}

/* pmc_show_elecmon_energy
 *     Show energy consumption data for a given element.
 * Parameters:
 *        element - pointer the "element" data
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 */
void pmc_show_elecmon_energy(void *element, int element_type)
{
	pmc_show_elecmon_measurement(element, element_type, PMC_MEASUREMENT_ENERGY);
}

/* pmc_show_elecmon_threshold
 *     Show current threshold levels for a given element.
 * Parameters:
 *        element - pointer the "element" data
 *   element_type - which element the caller refers to 
 *                ELEMENT_IPDU
 *                ELEMENT_PHASE
 *                ELEMENT_CIRCUIT
 */
void pmc_show_elecmon_threshold(void *element, int element_type)
{
	char *eletype, *sysname, *eletypeerr;
	elecMonInfo *elecMon;
	ipduInfo *ipdu;
	char out[10];

	if (!element) return;

	switch (element_type) {
	case ELEMENT_IPDU:
		eletype = "";
		eletypeerr = capital_ipdu_str;
		sysname = "";
		elecMon = &(((ipduInfo *)element)->elecMon);
		ipdu = (ipduInfo *)element;
		break;
	case ELEMENT_PHASE:
		eletype = phase_str;
		eletypeerr = capital_phase_str;
		sysname = ((phaseInfo *)element)->sysname;
		elecMon = &(((phaseInfo *)element)->elecMon);
		ipdu = ((phaseInfo *)element)->ipdu;
		break;
	case ELEMENT_CIRCUIT:
		eletype = bank_str;
		eletypeerr = capital_bank_str;
		sysname = ((circuitInfo *)element)->sysname;
		elecMon = &(((circuitInfo *)element)->elecMon);
		ipdu = ((circuitInfo *)element)->ipdu;
		break;
	case ELEMENT_OUTLET:
		eletype = outlet_str;
		eletypeerr = capital_outlet_str;
		sprintf(out,"%d",((oData *)element)->onumber);
		sysname = out;
		elecMon = &(((oData *)element)->elecMon);
		ipdu = ((oData *)element)->ipdu;
		break;
	default:
		printf(pmca_elemerr_unknown_element);
		return;
	}

	debug(PMCLIB_SHOW, "ipduID[%s] elementType[%d] sysname[%s]", ipdu->id, element_type, sysname);

	if (!ipdu->ready || !ipdu->license)
		return;

	printf(pmc_answer_ipdupre, ipdu->id);

	if (ipdu->pmInfo->ipdu_type!=IPDU_TYPE_CYCLADES) {
		printf(pmca_ipducommand_notsupported);
		return;
	}

	if (!(elecMon->sflag & ELECMON_CAP_CUR_CRIT_THRES)) {
		//We don't need to print anything here because PMHDs always have this capability somewhere...
		return;
	}

	printf(pmc_show_alarms, eletype[0] ? pmc_str_for : "", eletype, sysname, (double)elecMon->crit_threshold / 10);

	if (elecMon->sflag & ELECMON_CAP_CUR_WARN_THRES) {
		printf(pmc_show_alarm_warn_high_unit, (double)elecMon->warn_threshold / 10, "A");
	}
	if (elecMon->sflag & ELECMON_CAP_CUR_LOW_THRES) {
		printf(pmc_show_alarm_warn_low_unit, (double)elecMon->low_threshold / 10, "A");
	}
	if (elecMon->sflag & ELECMON_CAP_CUR_LOWCRIT_THRES) {
		printf(pmc_show_alarm_crit_low_unit, (double)elecMon->lowcrit_threshold / 10, "A");
	}
}

static void show_envmon_row_temp(ipduInfo *ipdu, envMonInfo *env, char* strtype)
{
	printf(pmc_answer_ipdupre, ipdu->id);

        if ((env->sflag & ENVMON_CAP_UNIT) && (env->unit == TempScaleF)) {
		printf(pmc_show_envmon, strtype, env->sysname,
			   ((double)env->value * 9 / 5 - 4596.7) / 10,
			   "oF");
                   
		if (env->sflag & ENVMON_CAP_MAX_VALUE) {
			printf(pmc_show_max_unit, ((double)env->max_value * 9 / 5 - 4596.7) / 10, "oF");
		}
		if (env->sflag & ENVMON_CAP_MIN_VALUE) {
			printf(pmc_show_min_unit, ((double)env->min_value * 9 / 5 - 4596.7) / 10, "oF");
		}
		if (env->sflag & ENVMON_CAP_AVG_VALUE) {
			printf(pmc_show_avg_unit, ((double)env->avg_value * 9 / 5 - 4596.7) / 10, "oF");
		}
	} else {
		printf(pmc_show_envmon, strtype, env->sysname,
			   ((double)env->value - 2731.5) / 10,
			   "oC");
                   
		if (env->sflag & ENVMON_CAP_MAX_VALUE) {
			printf(pmc_show_max_unit, ((double)env->max_value - 2731.5) / 10, "oC");
		}
		if (env->sflag & ENVMON_CAP_MIN_VALUE) {
			printf(pmc_show_min_unit, ((double)env->min_value - 2731.5) / 10, "oC");
		}
		if (env->sflag & ENVMON_CAP_AVG_VALUE) {
			printf(pmc_show_avg_unit, ((double)env->avg_value - 2731.5) / 10, "oC");
		}
	}
}

static void show_envmon_row_value(ipduInfo *ipdu, envMonInfo *env, char* strtype,char *unit)
{
	printf(pmc_answer_ipdupre, ipdu->id);

	printf(pmc_show_envmon, strtype, env->sysname,
		   (double)env->value / 10,
		   unit);
                   
	if (env->sflag & ENVMON_CAP_MAX_VALUE) {
		printf(pmc_show_max_unit, (double)env->max_value / 10, unit);
	}
	if (env->sflag & ENVMON_CAP_MIN_VALUE) {
		printf(pmc_show_min_unit, (double)env->min_value / 10, unit);
	}
	if (env->sflag & ENVMON_CAP_AVG_VALUE) {
		printf(pmc_show_avg_unit, (double)env->avg_value / 10, unit);
	}
}

static void show_envmon_row(ipduInfo *ipdu, envMonInfo *env,char *strtype)
{
	printf(pmc_answer_ipdupre, ipdu->id);

	printf(pmc_show_envmon_bool, strtype, env->sysname,
			   env->value ? "triggered" : "normal");
}

/* pmc_show_envmon_value
 *     Show measured values for environmental monitors.
 * Parameters:
 *          ipdu - pointer to IPDU data
 *   sensor_type - which type of environmental monitor
 *               ENVMON_SENSOR_TEMPERATURE
 *               ENVMON_SENSOR_HUMIDITY
 *               ENVMON_SENSOR_AVOCENT_AIR_FLOW
 *               ENVMON_SENSOR_AVOCENT_SMOKE
 *               ENVMON_SENSOR_DRY_CONTACT
 *               ENVMON_SENSOR_WATER_LEVEL
 *               ENVMON_SENSOR_AVOCENT_MOTION
 *               ENVMON_SENSOR_UNKNOWN (shows all)
 *               ENVMON_SENSOR_DIGITAL (shows DRY_CONTACT/WATER_LEVEL/MOTION/SMOKE)
 */
void pmc_show_envmon_value(ipduInfo *ipdu, int sensor_type)
{
	int i, all, digital;
	char *stype=NULL;
	int sensors = 0;

	if (!ipdu->ready || !ipdu->license) return;

	all = (sensor_type == ENVMON_SENSOR_UNKNOWN) ? 1 : 0;
	digital = (sensor_type == ENVMON_SENSOR_DIGITAL) ? 1 : 0;

	switch (sensor_type) {
	case ENVMON_SENSOR_DIGITAL:
		stype = digital_str;
		break;
	case ENVMON_SENSOR_TEMPERATURE:
		stype = temperature_str;
		break;
	case ENVMON_SENSOR_HUMIDITY:
		stype = humidity_str;
		break;
	case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
		stype = airFlow_str;
		break;
	case ENVMON_SENSOR_UNKNOWN:
		break;
	}

	for (i=0; i < ipdu->num_envMons; i++) {

		debug(PMCLIB_SHOW, "ipduID[%s] sensor_type[%d]", ipdu->id, sensor_type);
		
		if (ipdu->envMon[i].cap & ENVMON_CAP_NOT_PRESENT) continue;

		if (!(ipdu->envMon[i].sflag & ENVMON_CAP_VALUE)) continue;


		switch (ipdu->envMon[i].sensor_type) {
		case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
		case ENVMON_SENSOR_TEMPERATURE:
			if (all || (sensor_type == ENVMON_SENSOR_TEMPERATURE)) {
				show_envmon_row_temp(ipdu,&ipdu->envMon[i], capital_temperature_str);
				sensors++;
			}
			break;
		case ENVMON_SENSOR_AVOCENT_HUM_INTERNAL:
		case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
		case ENVMON_SENSOR_HUMIDITY:
			if (all || (sensor_type == ENVMON_SENSOR_HUMIDITY)) {
				show_envmon_row_value(ipdu,&ipdu->envMon[i], capital_humidity_str, "%");
				sensors++;
			}
			break;
		case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
			if (all || (sensor_type == ENVMON_SENSOR_AVOCENT_AIR_FLOW)) {
				show_envmon_row_value(ipdu,&ipdu->envMon[i], capital_airFlow_str, "afu");
				sensors++;
			}
			break;
		case ENVMON_SENSOR_AVOCENT_SMOKE:
			if (all || digital || (sensor_type == ENVMON_SENSOR_AVOCENT_SMOKE)) {
				show_envmon_row(ipdu,&ipdu->envMon[i], capital_smoke_str);
				sensors++;
			}
			break;
		case ENVMON_SENSOR_AVOCENT_DRY_CONTACT:
		case ENVMON_SENSOR_AVOCENT_DRY_CONTACT_OPEN:
		case ENVMON_SENSOR_AVOCENT_DRY_CONTACT_CLOSED:
		case ENVMON_SENSOR_DRY_CONTACT:
			if (all || digital || (sensor_type == ENVMON_SENSOR_DRY_CONTACT)) {
				show_envmon_row(ipdu,&ipdu->envMon[i], capital_dryContact_str);
				sensors++;
			}
			break;
		case ENVMON_SENSOR_AVOCENT_WATER_LEVEL:
		case ENVMON_SENSOR_WATER_LEVEL:
			if (all || digital || (sensor_type == ENVMON_SENSOR_WATER_LEVEL)) {
				show_envmon_row(ipdu,&ipdu->envMon[i], capital_waterLevel_str);
				sensors++;
			}
			break;
		case ENVMON_SENSOR_AVOCENT_MOTION:
			if (all || digital || (sensor_type == ENVMON_SENSOR_AVOCENT_MOTION)) {
				show_envmon_row(ipdu,&ipdu->envMon[i], capital_motion_str);
				sensors++;
			}
			break;
		case ENVMON_SENSOR_UNDEFINED:
			if (all) {
				printf(pmc_answer_ipdupre, ipdu->id);
				printf(pmc_show_envmon_unplugged, ipdu->envMon[i].sysname);
			}
			break;
		default:
			//printf(pmca_envmerr_unknown_sensor_type);
			return;
		}
	}

	if (!sensors) {
		if (sensor_type == ENVMON_SENSOR_UNKNOWN)
			pmc_answer_envmon_error(ipdu, NULL, ENVMERR_SENSORS_NOTFOUND, (void (*)(char *))printf);
		else
			pmc_answer_envmon_error(ipdu, stype, ENVMERR_SENSOR_NOTFOUND, (void (*)(char *))printf);
	}
}

/* pmc_show_envmon_threshold
 *     Show threshold alarm levels for environmental monitors.
 * Parameters:
 *   envmon - pointer to environmental monitor data
 */
void pmc_show_envmon_threshold(envMonInfo *envmon)
{
	int ftemp = 0;
	char unit[10];

	debug(PMCLIB_SHOW, "ipduID[%s] sensor[%s]", envmon->ipdu->id, envmon->sysname);

	printf(pmc_answer_ipdupre, envmon->ipdu->id);

	if (!(envmon->sflag & ENVMON_CAP_CRIT_THRES)) {
		printf(pmca_envmerr_no_threshold_alarms, envmon->sysname);
		return;
	}

	switch (envmon->sensor_type) {
	case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
	case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
	case ENVMON_SENSOR_TEMPERATURE:
		ftemp = 1;
		break;
	case ENVMON_SENSOR_AVOCENT_HUM_INTERNAL:
	case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL:
	case ENVMON_SENSOR_HUMIDITY:
		printf(pmc_show_envmon_alarms, capital_humidity_str, envmon->sysname,
			   (double)envmon->max_threshold / 10, "%");
		strcpy(unit,"%");
		break;
	case ENVMON_SENSOR_AVOCENT_AIR_FLOW:
		printf(pmc_show_envmon_alarms, airFlow_str, envmon->sysname,
			   (double)envmon->max_threshold / 10, "afu");
		strcpy(unit,"afu");
		break;
	default:
		//printf(pmca_envmerr_unknown_sensor_type);
		return;
	}

	if (!ftemp) {
		if (envmon->sflag & ENVMON_CAP_CRIT_WARN_THRES) {
			printf(pmc_show_alarm_warn_high_unit, (double)envmon->max_warn_threshold / 10, unit);
		}
		if (envmon->sflag & ENVMON_CAP_LOW_WARN_THRES) {
			printf(pmc_show_alarm_warn_low_unit, (double)envmon->min_warn_threshold / 10, unit);
		}
		if (envmon->sflag & ENVMON_CAP_LOW_THRES) {
			printf(pmc_show_alarm_crit_low_unit, (double)envmon->min_threshold / 10, unit);
		}
	} else {
		if ((envmon->sflag & ENVMON_CAP_UNIT) && (envmon->unit == TempScaleF)) {
			printf(pmc_show_envmon_alarms, capital_temperature_str, envmon->sysname,
				   ((double)envmon->max_threshold* 9 / 5 - 4596.7) / 10, "oF");
			if (envmon->sflag & ENVMON_CAP_CRIT_WARN_THRES) {
				printf(pmc_show_alarm_warn_high_unit, ((double)envmon->max_warn_threshold* 9 / 5 - 4596.7) / 10, "oF");
			}
			if (envmon->sflag & ENVMON_CAP_LOW_WARN_THRES) {
				printf(pmc_show_alarm_warn_low_unit, ((double)envmon->min_warn_threshold* 9 / 5 - 4596.7) / 10, "oF");
			}
			if (envmon->sflag & ENVMON_CAP_LOW_THRES) {
				printf(pmc_show_alarm_crit_low_unit, ((double)envmon->min_threshold* 9 / 5 - 4596.7) / 10, "oF");
			}
		} else {
			printf(pmc_show_envmon_alarms, capital_temperature_str, envmon->sysname,
				   ((double)envmon->max_threshold- 2731.5) / 10, "oC");
			if (envmon->sflag & ENVMON_CAP_CRIT_WARN_THRES) {
				printf(pmc_show_alarm_warn_high_unit, ((double)envmon->max_warn_threshold- 2731.5) / 10, "oC");
			}
			if (envmon->sflag & ENVMON_CAP_LOW_WARN_THRES) {
				printf(pmc_show_alarm_warn_low_unit, ((double)envmon->min_warn_threshold- 2731.5) / 10, "oC");
			}
			if (envmon->sflag & ENVMON_CAP_LOW_THRES) {
				printf(pmc_show_alarm_crit_low_unit, ((double)envmon->min_threshold- 2731.5) / 10, "oC");
			}
		}
	}
}

/* pmc_show_envmon_threshold_all
 *     Show threshold alarm levels for all environmental monitors.
 * Parameters:
 *   ipdu - pointer to IPDU data
 */
void pmc_show_envmon_threshold_all(ipduInfo *ipdu)
{
	int i, sensors=0;

	if (!ipdu->ready || !ipdu->license) {
                return;
        }

	for (i=0; i < ipdu->num_envMons; i++) {
		if (ipdu->envMon[i].sflag & ENVMON_CAP_CRIT_THRES) {
			pmc_show_envmon_threshold(&ipdu->envMon[i]);
			sensors++;
		}
	}

	if (!sensors) {
		pmc_answer_envmon_error(ipdu, NULL, ENVMERR_NO_SENSOR_THRESHOLD_ALARMS, (void (*)(char *))printf);
	}
}

/* pmc_show_envmon_unit
 *     Show IPDU unit for a certaim environmental monitor class.
 * Parameters:
 *          ipdu - pointer to IPDU data
 *   sensor_type - which type of environmental monitor
 *               ENVMON_SENSOR_TEMPERATURE
 *               ENVMON_SENSOR_UNKNOWN (shows all)
 */
void pmc_show_envmon_unit(ipduInfo *ipdu, int sensor_type)
{
	int i, sensors = 0;
	char *stype=NULL, *capital_stype, *unit;

	if (!ipdu->ready || !ipdu->license) {
                return;
        }

	for (i=0; i < ipdu->num_envMons; i++) {

		debug(PMCLIB_SHOW, "ipduID[%s] sensor_type[%d]", ipdu->id, sensor_type);

		if (!(ipdu->envMon[i].sflag & ENVMON_CAP_UNIT))
			continue;

		switch (ipdu->envMon[i].sensor_type) {
			case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL:
			case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL:
			case ENVMON_SENSOR_TEMPERATURE:
				stype = temperature_str;
				capital_stype = capital_temperature_str;
				switch (ipdu->envMon[i].unit) {
				case TempScaleC:
					unit = "oC";
					break;
				case TempScaleF:
					unit = "oF";
					break;
				case TempScaleK:
					unit = "oK";
					break;
				default:
					printf(pmca_envmerr_unknown_arg_unit, stype);
					continue;
				}
				break;
			default:
				//printf(pmca_envmerr_unknown_sensor_type);
				continue;
		}

		sensors++;

		printf(pmc_answer_ipdupre, ipdu->id);

		printf(pmc_show_envmon_configured_unit, capital_stype,ipdu->envMon[i].sysname, unit);
	}

	if (!sensors) {
		pmc_answer_envmon_error(ipdu, NULL, ENVMERR_UNIT_SENSORS_NOTFOUND, (void (*)(char *))printf);
	}
}
