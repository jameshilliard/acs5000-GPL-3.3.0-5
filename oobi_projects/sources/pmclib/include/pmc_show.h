/*****************************************************************
* File: pmc_show.h
*
* Copyright (C) 2006 Avocent Corporation
*
* www.avocent.com
*
* Description:
*     This file has the text for show functions.
*
*****************************************************************/
#ifndef PMC_SHOW_H
#define PMC_SHOW_H

/*
 * The following enum must follow the same order as 'ipduerr_table' in pmc_show.c
 */
enum ipduerr_table_index {
    IPDUERR_IPDUCAP_NOTSUPPORTED,
    IPDUERR_IPDUCAP_NOSENSOR,
    IPDUERR_IPDUCAP_NOOVERCURRENT,
    IPDUERR_INVRANGE,
    IPDUERR_IPDUCOMMAND_NOTSUPPORTED,
    IPDUERR_IPDU_FIXEDON
};

/*
 * The following enum must follow the same order as 'outlerr_table' in pmc_show.c
 */
enum outlerr_table_index {
	OUTLERR_NOERROR,
	OUTLERR_NOPERM,
	OUTLERR_CAP_NOTSUPPORTED,
	OUTLERR_UNKNOWN_MEASUREMENT,
    OUTLERR_MEASUREMENT_NOTSUPPORTED,
    OUTLERR_MAXCAP_NOTSUPPORTED,
	OUTLERR_NO_THRESHOLD_ALARMS,
	OUTLERR_NO_SPEC_THRESHOLD_ALARM,
	OUTLERR_INVALID_THRESHOLD_ALARM,
	OUTLERR_BUSY,
	OUTLERR_LOCK
};

/*
 * The following enum must follow the same order as 'elemerr_table' in pmc_show.c
 */
enum elemerr_table_index {
	ELEMERR_UNKNOWN_ELEMENT,
	ELEMERR_UNKNOWN_MEASUREMENT,
    ELEMERR_MEASUREMENT_NOTSUPPORTED,
    ELEMERR_MAXCAP_NOTSUPPORTED,
	ELEMERR_NO_THRESHOLD_ALARMS,
	ELEMERR_NO_SPEC_THRESHOLD_ALARM,
	ELEMERR_INVALID_THRESHOLD_ALARM
};

/*
 * The following enum must follow the same order as 'envmerr_table' in pmc_show.c
 */
enum envmerr_table_index {
	ENVMERR_CAP_NOT_SUPPORTED,
	ENVMERR_UNKNOWN_SENSOR_TYPE,
	ENVMERR_UNKNOWN_UNIT,
	ENVMERR_UNKNOWN_ARG_UNIT,
    ENVMERR_SENSOR_NOTFOUND,
    ENVMERR_SENSORS_NOTFOUND,
    ENVMERR_UNIT_SENSORS_NOTFOUND,
	ENVMERR_MAXCAP_NOTSUPPORTED,
	ENVMERR_NO_THRESHOLD_ALARMS,
	ENVMERR_NO_SENSOR_THRESHOLD_ALARMS,
	ENVMERR_CAPUNIT_NOTSUPPORTED,
	ENVMERR_UNIT_NOTSUPPORTED,
	ENVMERR_NO_THRESHOLD_ALARM,
	ENVMERR_THRESHOLD_VALUE
};

extern char pmca_elemerr_unknown_element[];

extern char pmca_envmerr_cap_not_supported[];
extern char pmca_envmerr_unknown_sensor_type[];
extern char pmca_envmerr_no_threshold_alarms[];
extern char pmca_envmerr_maxcap_notsupported[];
extern char pmca_envmerr_unknown_unit[];
extern char pmca_envmerr_capunit_notsupported[];
extern char pmca_envmerr_unit_notsupported[];
extern char pmca_envmerr_sensors_notfound[];

extern char pmc_str_for[];
extern char pmc_str_on[];
extern char pmc_str_est[];

extern char pmc_str_between[];

extern char pmcOldId[];
extern char *pmc_errormsg[];

extern char pmc_answer_outoldpre[]; // outlet number in the chain
extern char pmc_answer_outpre[]; // ipduID, outlet number
extern char pmc_answer_outlocpre[]; // outlet number in the chain
extern char pmc_answer_ipduoldpre[]; // ipdu number in the chain
extern char pmc_answer_ipdupre[]; // ipdu ID 
extern char pmc_answer_ipdulocpre[]; // device name - position

extern char pmc_answer_name[];

extern char pmc_min_and_agv[];
extern char pmc_answer_currentReset[];
extern char pmc_answer_voltageReset[];
extern char pmc_answer_powerReset[];
extern char pmc_answer_energyReset[];
extern char pmc_answer_powerfactorReset[];
extern char pmc_answer_currentReset_out[];
extern char pmc_answer_voltageReset_out[];
extern char pmc_answer_powerReset_out[];
extern char pmc_answer_powerfactorReset_out[];
extern char pmc_answer_energyReset_out[];

extern char pmc_answer_temperatureReset[];
extern char pmc_answer_humidityReset[];
extern char pmc_answer_envmonReset[];

extern char pmc_answer_id[]; // id

extern char pmc_answer_display1[];
extern char pmc_answer_display2[];
extern char pmc_answer_display3[];
extern char pmc_answer_display4[];
extern char pmc_answer_display5[];
extern char pmc_display_normal[];
extern char pmc_display_inverted[];
extern char pmc_display_current[];
extern char pmc_display_voltage[];

extern char pmc_answer_wakeup[];
extern char *pmc_wakeupopt[];

extern char pmc_answer_minon[];
extern char pmc_answer_minoff[];
extern char pmc_answer_poweron[];
extern char pmc_answer_poweroff[];

extern char pmc_answer_control_status[];
extern char pmc_answer_outlet_control_off[];
extern char pmc_answer_outlet_control_on[];
extern char pmc_answer_outlet_control_cycle[];
extern char pmc_answer_outlet_control_lock[];
extern char pmc_answer_outlet_control_unlock[];

extern char pmc_answer_syslog[];
extern char pmc_answer_buzzer[];
extern char pmc_answer_currentprotection[];
extern char pmc_answer_sens_currentprotection[];
extern char *pmc_onoff[];

extern char pmc_answer_rebootdelay[];
extern char pmc_answer_seqinterval[];
extern char pmc_answer_ipducoldstart[];
extern char pmc_answer_ipducoldstart_never[];

extern char pmc_answer_alarm[];
extern char pmc_answer_alarm_gsp[];
extern char pmc_answer_alarm_gsp_out[];
extern char pmc_answer_nearovld[];
extern char pmc_answer_nearlowload[];
extern char pmc_answer_lowload[];
extern char pmc_answer_envmon_alarm[];
extern char pmc_answer_envmon_unit[];

extern char phase_str[];
extern char bank_str[];
//extern char outlet_str[];
extern char capital_ipdu_str[];
extern char capital_phase_str[];
extern char capital_bank_str[];

extern char current_str[];
extern char voltage_str[];
extern char power_str[];
extern char powerFactor_str[];
extern char energy_str[];
extern char critHigh_str[];
extern char warnHigh_str[];
extern char warnLow_str[];
extern char critLow_str[];

extern char temperature_str[];
extern char humidity_str[];
extern char airFlow_str[];
extern char smoke_str[];
extern char dryContact_str[];
extern char waterLevel_str[];
extern char motion_str[];
extern char capital_temperature_str[];
extern char capital_humidity_str[];
extern char capital_airFlow_str[];
extern char capital_smoke_str[];
extern char capital_dryContact_str[];
extern char capital_waterLevel_str[];
extern char capital_motion_str[];

int list_ipdu_related_elecmons(ipduInfo *ipdu,  elecMonList_t *elecMonList);
int count_elecmon_caps(elecMonList_t *elecMonList, unsigned caps);

#endif //PMC_SHOW_H
