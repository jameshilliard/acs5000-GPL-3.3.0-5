#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "../util_funcs.h"
#include "cyPM.h"
#include "cystrings.h"

#include <pslave_cfg.h>
#include <rot_buf.h>
#include <rwconf.h>
#include <cy_shm.h>

#define CYPM 1
#include <pmclib.h>

extern int cy_numports;

#ifdef  CYPM_ONS
#define MAX_DEV AUX_OFFSET+cy_shm->max_devA
#else
#define MAX_DEV cy_shm->max_devS
#endif

#define ELECPDU		1
#define ELECPHASE	2
#define ELECBANK	3
#define ELECOUTLET 	4

static int ipdu_in_pm_range(int nsp, int ipdu);
static int ipdu_in_pm_range_envMon(int nsp, int ipdu);
static int ipdu_in_pm_range_elecMon(int nsp, int ipdu, int element);
static int outlet_in_pm_range(int nsp, int outlet);
static int envmon_in_pm_range(int nsp, int ipdu, int envmon);
static int elecmon_in_pm_range(int nsp, int ipdu, int elecmon, int element);
static long first_outlet_number(pmInfo *pmc, int pm);
void find_idpu_name(int cy_nsp,int cy_pm,char *nsp_name);
static int header_PMUnitTable(struct variable *vp, oid * name, size_t * length,
                int exact, size_t * var_len, WriteMethod ** write_method);

/* Find first outlet number in the ipdu connected to chain pmc */
static long first_outlet_number(pmInfo *pmc, int pm)
{
	int noutlet;
	unsigned char nipdu;
	ipduInfo *ipdu;

	noutlet = 0;
	nipdu = 0; 
	ipdu = pmc->ipdu;

	while (nipdu < pmc->num_ipdus) {
		if ((int) ipdu->number != pm) {
			noutlet = noutlet + ipdu->num_outlets;
			ipdu = ipdu->next;
			nipdu++;
		} else {
			noutlet += 1;
			break;
		}
	}

	return (long) noutlet;
}

/*
 *  cy_nsp - number of serial port
 *  cy_pm  - number of IPDU on the chain
 *  outlet - number of outlet (if 0, searches using only cy_pm, otherwise
 *	only the outlet #)
 */
static void find_ipdu_name(int cy_nsp, int cy_pm, int * outlet, char *nsp_name)
{
	pmInfo *pmc;
	ipduInfo *ipdu;
	int total_outlets;

	nsp_name[0] = '\0';

	pmc = pmc_validate_port(cy_nsp,"ttyS");
#ifdef CYPM_ONS
	if (pmc == NULL)
		pmc = pmc_validate_port(cy_nsp,"ttyA");
#endif
	if (pmc == NULL)
		return;

	ipdu = pmc->ipdu;

	if (outlet == NULL) {
		/* Find name using ipdu number */
		while (ipdu != NULL) {
			if (ipdu->number == cy_pm) {
				sprintf(nsp_name, "%.*s", IPDUIDSIZE, ipdu->id);
				return;
			}
			ipdu = ipdu->next;
		}
	}
	else {
		/* Find name using outlet number */
		total_outlets = 0;
		while (ipdu != NULL) {
			total_outlets += ipdu->num_outlets;
			if (total_outlets >= *outlet) {
				sprintf(nsp_name, "%.*s", IPDUIDSIZE, ipdu->id);
				*outlet = *outlet - (total_outlets - ipdu->num_outlets);
				return;
			}
			ipdu = ipdu->next;
		}
	}
}

static int outlet_status_to_power(unsigned char ostatus)
{
	switch (ostatus)
	{
		case OUTLET_STATUS_OFF:
		case OUTLET_STATUS_LOCK_OFF:
		case OUTLET_STATUS_PEND_ON:
		case OUTLET_STATUS_PEND_CYCLE:
		case OUTLET_STATUS_IDLE_OFF:
		case OUTLET_STATUS_SHUTDOWN:
		case OUTLET_STATUS_WAKE_OFF:
			return 0;

		case OUTLET_STATUS_ON:
		case OUTLET_STATUS_LOCK_ON:
		case OUTLET_STATUS_PEND_OFF:
		case OUTLET_STATUS_IDLE_ON:
		case OUTLET_STATUS_FIXED_ON:
		case OUTLET_STATUS_WAKE_ON:
			return 1;

		case OUTLET_STATUS_CYCLE:
			return 2;

		default:
			return 3;
	}
}

static int outlet_status_to_lock(unsigned char ostatus)
{
	switch (ostatus)
	{
		case OUTLET_STATUS_OFF:
		case OUTLET_STATUS_PEND_ON:
		case OUTLET_STATUS_PEND_CYCLE:
		case OUTLET_STATUS_IDLE_OFF:
		case OUTLET_STATUS_SHUTDOWN:
		case OUTLET_STATUS_WAKE_OFF:
		case OUTLET_STATUS_ON:
		case OUTLET_STATUS_PEND_OFF:
		case OUTLET_STATUS_IDLE_ON:
		case OUTLET_STATUS_FIXED_ON:
		case OUTLET_STATUS_WAKE_ON:
		case OUTLET_STATUS_CYCLE:
			return 0;
		case OUTLET_STATUS_LOCK_OFF:
		case OUTLET_STATUS_LOCK_ON:
			return 1;
		default:
			return 2;
	}
}

oid cyNumberOfPM_oid[] = { PROD_OID, 5 , 1};
oid cyPMTable_oid[] = { PROD_OID, 5 , 2};
oid cyPMUnitTable_oid[] = { PROD_OID, 5 , 3};
oid cyOutletTable_oid[] = { PROD_OID, 5, 4 };
oid cyPMElecPduTable_oid[] = { PROD_OID, 5, 5 };
oid cyPMElecPhaseTable_oid[] = { PROD_OID, 5, 6 };
oid cyPMElecBankTable_oid[] = { PROD_OID, 5, 7 };
oid cyPMElecOutletTable_oid[] = { PROD_OID, 5, 8 };
oid cyPMEnvMonTable_oid[] = { PROD_OID, 5, 9};

#define CYPM_INDEX	PROD_OID_SIZE + 4
#define CYPMUnit_INDEX	PROD_OID_SIZE + 5
#define CYOUTLET_INDEX	PROD_OID_SIZE + 5
#define CYPMElecPdu_PN_INDEX	PROD_OID_SIZE + 4
#define CYPMElecPdu_PDUN_INDEX	PROD_OID_SIZE + 5
#define CYPMElecPhase_PN_INDEX		PROD_OID_SIZE + 4
#define CYPMElecPhase_PDUN_INDEX	PROD_OID_SIZE + 5
#define CYPMElecPhase_TABLE_INDEX	PROD_OID_SIZE + 6
#define CYPMElecBank_PN_INDEX		PROD_OID_SIZE + 4
#define CYPMElecBank_PDUN_INDEX		PROD_OID_SIZE + 5
#define CYPMElecBank_TABLE_INDEX	PROD_OID_SIZE + 6
#define CYPMElecOutlet_PN_INDEX		PROD_OID_SIZE + 4
#define CYPMElecOutlet_PDUN_INDEX	PROD_OID_SIZE + 5
#define CYPMElecOutlet_TABLE_INDEX	PROD_OID_SIZE + 6
#define CYPMEnvMon_PN_INDEX	PROD_OID_SIZE + 4
#define CYPMEnvMon_PDUN_INDEX	PROD_OID_SIZE + 5
#define CYPMEnvMon_TABLE_INDEX	PROD_OID_SIZE + 6


struct variable4 cyPMTable_variables[] = {
#define CYPMSerialPortN	1
	{CYPMSerialPortN, ASN_INTEGER, RONLY, var_cyPMTable, 2, {1, 1}},
#define CYPMNumOutlets	2
	{CYPMNumOutlets, ASN_INTEGER, RONLY, var_cyPMTable, 2, {1, 2}},
#define CYPMNumUnits	3
	{CYPMNumUnits, ASN_INTEGER, RONLY, var_cyPMTable, 2, {1, 3}},
#define CYPMCurrent	4
	{CYPMCurrent, ASN_OCTET_STR, RONLY, var_cyPMTable, 2, {1, 4}},
#define CYPMVersion	5
	{CYPMVersion, ASN_OCTET_STR, RONLY, var_cyPMTable, 2, {1, 5}},
#ifdef OBSOLETED
#define CYPMTemperature	6
	{CYPMTemperature, ASN_OCTET_STR, RONLY, var_cyPMTable, 2, {1, 6}},
#endif
#define CYPMCommand	7
	{CYPMCommand, ASN_OCTET_STR, RWRITE, var_cyPMTable, 2, {1, 7}}
};

struct variable4 cyPMUnitTable_variables[] = {
#define CYPMUnitVersion	2
	{CYPMUnitVersion, ASN_OCTET_STR, RONLY, var_cyPMUnitTable, 2, {1, 2}},
#define CYPMUnitOutlets	3
	{CYPMUnitOutlets, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 3}},
#define CYPMUnitFirstOutlet	4
	{CYPMUnitFirstOutlet, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 4}},
#define CYPMUnitCurrent	5
	{CYPMUnitCurrent, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 5}},
#define CYPMUnitMaxCurrent	6
	{CYPMUnitMaxCurrent, ASN_INTEGER, RWRITE, var_cyPMUnitTable, 2, {1, 6}},
#ifdef OBSOLETED
#define CYPMUnitTemperature	7
	{CYPMUnitTemperature, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 7}},
#define CYPMUnitMaxTemperature	8
	{CYPMUnitMaxTemperature, ASN_INTEGER, RWRITE, var_cyPMUnitTable, 2, {1, 8}},
#endif
#define CYPMUnitSequenceInterval 9
	{CYPMUnitSequenceInterval, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 9}},
#define CYPMUnitCycleInterval 10
	{CYPMUnitCycleInterval, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 10}},
#define CYPMUnitID	11
	{CYPMUnitID, ASN_OCTET_STR, RONLY, var_cyPMUnitTable, 2, {1, 11}},
#define CYPMUnitPhases	12
	{CYPMUnitPhases, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 12}},
#define CYPMUnitBanks	13
	{CYPMUnitBanks, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 13}},
#define CYPMUnitNominalVoltage	14
	{CYPMUnitNominalVoltage, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 14}},
#define CYPMUnitPower	15
	{CYPMUnitPower, ASN_INTEGER, RONLY, var_cyPMUnitTable, 2, {1, 15}}
};

struct variable4 cyOutletTable_variables[] = {
#define CYOUTLETName	2
	{CYOUTLETName, ASN_OCTET_STR, RWRITE, var_cyOutletTable, 2, {1, 2}},
#define CYOUTLETServer	3
	{CYOUTLETServer, ASN_OCTET_STR, RONLY, var_cyOutletTable, 2, {1, 3}},
#define CYOUTLETPower	4
	{CYOUTLETPower, ASN_INTEGER, RWRITE, var_cyOutletTable, 2, {1, 4}},
#define CYOUTLETLock	5
	{CYOUTLETLock, ASN_INTEGER, RWRITE, var_cyOutletTable, 2, {1, 5}},
#define CYOUTLETInterval	6
	{CYOUTLETInterval, ASN_INTEGER, RWRITE, var_cyOutletTable, 2, {1, 6}},
#define CYOUTLETMinimumOnInterval	7
	{CYOUTLETMinimumOnInterval, ASN_INTEGER, RONLY, var_cyOutletTable, 2, {1, 7}},
#define CYOUTLETMinimumOffInterval	8
	{CYOUTLETMinimumOffInterval, ASN_INTEGER, RONLY, var_cyOutletTable, 2, {1, 8}},
#define CYOUTLETWakeupState	9
	{CYOUTLETWakeupState, ASN_INTEGER, RONLY, var_cyOutletTable, 2, {1, 9}},
#define CYOUTLETPduID	10
	{CYOUTLETPduID, ASN_OCTET_STR, RONLY, var_cyOutletTable, 2, {1, 10}},
#define CYOUTLETPduNumber	11
	{CYOUTLETPduNumber, ASN_INTEGER, RONLY, var_cyOutletTable, 2, {1, 11}}
};

struct variable4 cyPMElecPduTable_variables[] = {
#define CYPMElecPduTablePortNumber    1
        {CYPMElecPduTablePortNumber, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 1}},
#define CYPMElecPduTablePduNumber     2
        {CYPMElecPduTablePduNumber, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 2}},
#define CYPMElecPduTableCurrentValue  3
        {CYPMElecPduTableCurrentValue, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 3}},
#define CYPMElecPduTableCurrentMax    4
        {CYPMElecPduTableCurrentMax, ASN_INTEGER, RWRITE, var_cyPMElecPduTable, 2, {1, 4}},
#define CYPMElecPduTablePowerValue    5
        {CYPMElecPduTablePowerValue, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 5}},
#define CYPMElecPduTablePowerMax      6
        {CYPMElecPduTablePowerMax, ASN_INTEGER, RWRITE, var_cyPMElecPduTable, 2, {1, 6}},
#define CYPMElecPduTableVoltageValue  7
        {CYPMElecPduTableVoltageValue, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 7}},
#define CYPMElecPduTableVoltageMax    8
        {CYPMElecPduTableVoltageMax, ASN_INTEGER, RWRITE, var_cyPMElecPduTable, 2, {1, 8}},
#define CYPMElecPduTablePowerFactorValue      9
        {CYPMElecPduTablePowerFactorValue, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 9}},
#define CYPMElecPduTablePowerFactorMax        10
        {CYPMElecPduTablePowerFactorMax, ASN_INTEGER, RWRITE, var_cyPMElecPduTable, 2, {1, 10}},
#define CYPMElecPduTablePowerType	11
        {CYPMElecPduTablePowerType, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 11}},
#define CYPMElecPduTableVoltageType   12
        {CYPMElecPduTableVoltageType, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 12}},
#define CYPMElecPduTablePowerFactorType       13
        {CYPMElecPduTablePowerFactorType, ASN_INTEGER, RONLY, var_cyPMElecPduTable, 2, {1, 13}}
};

struct variable4 cyPMElecPhaseTable_variables[] = {
#define CYPMElecPhaseTablePortNumber	1
	{CYPMElecPhaseTablePortNumber, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 1}},
#define CYPMElecPhaseTablePduNumber	2
        {CYPMElecPhaseTablePduNumber, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 2}},
#define CYPMElecPhaseTableIndex		3
        {CYPMElecPhaseTableIndex, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 3}},
#define CYPMElecPhaseTableName		4
        {CYPMElecPhaseTableName, ASN_OCTET_STR, RONLY, var_cyPMElecPhaseTable, 2, {1, 4}},
#define CYPMElecPhaseTableCurrentValue	5
	{CYPMElecPhaseTableCurrentValue, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 5}},
#define CYPMElecPhaseTableCurrentMax	6
        {CYPMElecPhaseTableCurrentMax, ASN_INTEGER, RWRITE, var_cyPMElecPhaseTable, 2, {1, 6}},
#define CYPMElecPhaseTablePowerValue	7
        {CYPMElecPhaseTablePowerValue, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 7}},
#define CYPMElecPhaseTablePowerMax	8
        {CYPMElecPhaseTablePowerMax, ASN_INTEGER, RWRITE, var_cyPMElecPhaseTable, 2, {1, 8}},
#define CYPMElecPhaseTableVoltageValue	9
        {CYPMElecPhaseTableVoltageValue, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 9}},
#define CYPMElecPhaseTableVoltageMax	10
        {CYPMElecPhaseTableVoltageMax, ASN_INTEGER, RWRITE, var_cyPMElecPhaseTable, 2, {1, 10}},
#define CYPMElecPhaseTablePowerFactorValue	11
        {CYPMElecPhaseTablePowerFactorValue, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 11}},
#define CYPMElecPhaseTablePowerFactorMax	12
        {CYPMElecPhaseTablePowerFactorMax, ASN_INTEGER, RWRITE, var_cyPMElecPhaseTable, 2, {1, 12}},
#define CYPMElecPhaseTablePowerType       13
        {CYPMElecPhaseTablePowerType, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 13}},
#define CYPMElecPhaseTableVoltageType   14
        {CYPMElecPhaseTableVoltageType, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 14}},
#define CYPMElecPhaseTablePowerFactorType       15
        {CYPMElecPhaseTablePowerFactorType, ASN_INTEGER, RONLY, var_cyPMElecPhaseTable, 2, {1, 15}}
};

struct variable4 cyPMElecBankTable_variables[] = {
#define CYPMElecBankTablePortNumber    1
        {CYPMElecBankTablePortNumber, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 1}},
#define CYPMElecBankTablePduNumber     2
        {CYPMElecBankTablePduNumber, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 2}},
#define CYPMElecBankTableIndex         3
        {CYPMElecBankTableIndex, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 3}},
#define CYPMElecBankTableName          4
        {CYPMElecBankTableName, ASN_OCTET_STR, RONLY, var_cyPMElecBankTable, 2, {1, 4}},
#define CYPMElecBankTableCurrentValue  5
        {CYPMElecBankTableCurrentValue, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 5}},
#define CYPMElecBankTableCurrentMax    6
        {CYPMElecBankTableCurrentMax, ASN_INTEGER, RWRITE, var_cyPMElecBankTable, 2, {1, 6}},
#define CYPMElecBankTablePowerValue    7
        {CYPMElecBankTablePowerValue, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 7}},
#define CYPMElecBankTablePowerMax      8
        {CYPMElecBankTablePowerMax, ASN_INTEGER, RWRITE, var_cyPMElecBankTable, 2, {1, 8}},
#define CYPMElecBankTableVoltageValue  9
        {CYPMElecBankTableVoltageValue, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 9}},
#define CYPMElecBankTableVoltageMax    10
        {CYPMElecBankTableVoltageMax, ASN_INTEGER, RWRITE, var_cyPMElecBankTable, 2, {1, 10}},
#define CYPMElecBankTablePowerFactorValue      11
        {CYPMElecBankTablePowerFactorValue, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 11}},
#define CYPMElecBankTablePowerFactorMax        12
        {CYPMElecBankTablePowerFactorMax, ASN_INTEGER, RWRITE, var_cyPMElecBankTable, 2, {1, 12}},
#define CYPMElecBankTablePowerType       13
        {CYPMElecBankTablePowerType, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 13}},
#define CYPMElecBankTableVoltageType   14
        {CYPMElecBankTableVoltageType, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 14}},
#define CYPMElecBankTablePowerFactorType       15
        {CYPMElecBankTablePowerFactorType, ASN_INTEGER, RONLY, var_cyPMElecBankTable, 2, {1, 15}}
};

struct variable4 cyPMElecOutletTable_variables[] = {
#define CYPMElecOutletTablePortNumber    1
        {CYPMElecOutletTablePortNumber, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 1}},
#define CYPMElecOutletTablePduNumber     2
        {CYPMElecOutletTablePduNumber, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 2}},
#define CYPMElecOutletTableNumber         3
        {CYPMElecOutletTableNumber, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 3}},
#define CYPMElecOutletTableName          4
        {CYPMElecOutletTableName, ASN_OCTET_STR, RONLY, var_cyPMElecOutletTable, 2, {1, 4}},
#define CYPMElecOutletTableCurrentValue  5
        {CYPMElecOutletTableCurrentValue, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 5}},
#define CYPMElecOutletTableCurrentMax    6
        {CYPMElecOutletTableCurrentMax, ASN_INTEGER, RWRITE, var_cyPMElecOutletTable, 2, {1, 6}},
#define CYPMElecOutletTablePowerValue    7
        {CYPMElecOutletTablePowerValue, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 7}},
#define CYPMElecOutletTablePowerMax      8
        {CYPMElecOutletTablePowerMax, ASN_INTEGER, RWRITE, var_cyPMElecOutletTable, 2, {1, 8}},
#define CYPMElecOutletTableVoltageValue  9
        {CYPMElecOutletTableVoltageValue, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 9}},
#define CYPMElecOutletTableVoltageMax    10
        {CYPMElecOutletTableVoltageMax, ASN_INTEGER, RWRITE, var_cyPMElecOutletTable, 2, {1, 10}},
#define CYPMElecOutletTablePowerFactorValue      11
        {CYPMElecOutletTablePowerFactorValue, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 11}},
#define CYPMElecOutletTablePowerFactorMax        12
        {CYPMElecOutletTablePowerFactorMax, ASN_INTEGER, RWRITE, var_cyPMElecOutletTable, 2, {1, 12}},
#define CYPMElecOutletTablePowerType       13
        {CYPMElecOutletTablePowerType, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 13}},
#define CYPMElecOutletTableVoltageType   14
        {CYPMElecOutletTableVoltageType, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 14}},
#define CYPMElecOutletTablePowerFactorType       15
        {CYPMElecOutletTablePowerFactorType, ASN_INTEGER, RONLY, var_cyPMElecOutletTable, 2, {1, 15}}
};

struct variable4 cyPMEnvMonTable_variables[] = {
#define CYPMEnvMonTablePortNumber	1
	{CYPMEnvMonTablePortNumber, ASN_INTEGER, RONLY, var_cyPMEnvMonTable, 2, {1, 1}},
#define CYPMEnvMonTablePduNumber	2
	{CYPMEnvMonTablePduNumber, ASN_INTEGER, RONLY, var_cyPMEnvMonTable, 2, {1, 2}},
#define CYPMEnvMonTableIndex	3
	{CYPMEnvMonTableIndex, ASN_INTEGER, RONLY, var_cyPMEnvMonTable, 2, {1, 3}},
#define CYPMEnvMonTableName	4
	{CYPMEnvMonTableName, ASN_OCTET_STR, RONLY, var_cyPMEnvMonTable, 2, {1, 4}},
#define CYPMEnvMonTableType	5
	{CYPMEnvMonTableType, ASN_INTEGER, RONLY, var_cyPMEnvMonTable, 2, {1, 5}},
#define CYPMEnvMonTableState	6
	{CYPMEnvMonTableState, ASN_INTEGER, RONLY, var_cyPMEnvMonTable, 2, {1, 6}},
#define CYPMEnvMonTableValue	7
	{CYPMEnvMonTableValue, ASN_INTEGER, RONLY, var_cyPMEnvMonTable, 2, {1, 7}},
#define CYPMEnvMonTableMaxValue 	8
	{CYPMEnvMonTableMaxValue, ASN_INTEGER, RWRITE, var_cyPMEnvMonTable, 2, {1, 8}}
};

CY_SHM *cy_shm;

static void cypm_ng_system(unsigned char *cmd, char *nps,
	int outlet, unsigned char *extra, int extra_len)
{
	unsigned char cypm[256];

//	snmp_log(LOG_INFO,
//		 "cypm_ng_system: cmd=%p, nps=%p, o=%d, extra=%p, ms=%d\n",
//		cmd, nps, outlet, extra, extra_len);

	switch (outlet) {
		case -1 : 
			sprintf(cypm, "/bin/pmCommand %s %s &>/dev/null", cmd, nps);
			break;
		case 0 :
			sprintf(cypm, "/bin/pmCommand %s %s all &>/dev/null", cmd, nps);
			break;
		default :
			if (extra_len && extra) {
				sprintf(cypm, "/bin/pmCommand %s '%s[%d]' %.*s &>/dev/null",
					cmd, nps, outlet, extra_len, extra);
			} else {
				sprintf(cypm, "/bin/pmCommand %s '%s[%d]' &>/dev/null",
					cmd, nps, outlet);
			}
			break;
	}

//	snmp_log(LOG_INFO, "cypm_ng_system: Running '%s'\n", cypm);
	system(cypm);
}

void init_cyPM(void) 
{
	int shm_id;
	
	if ((shm_id = shmget(CY_ID_SHM,0,SHM_PERM)) < 0) {
		return;
	}
	
	cy_shm = (CY_SHM *)shmat(shm_id,(void *) CY_ADDR_SHM,0);

	pmc_shm_init();

	netsnmp_register_instance(netsnmp_create_handler_registration
	                         ("cyNumberOfPM",
	                           do_cyNumberOfPM,
	                           cyNumberOfPM_oid,
	                           OID_LENGTH(cyNumberOfPM_oid),
  				   HANDLER_CAN_RONLY));

	/** Initializes the cyPMTable module */
	REGISTER_MIB("cyPMTable", cyPMTable_variables, variable4, cyPMTable_oid);
	
	/** Initializes the cyPMUnitTable module */
	REGISTER_MIB("cyPMUnitTable", cyPMUnitTable_variables, variable4,
		       	cyPMUnitTable_oid);
	
	/** Initializes the cyOutletTable module */
	REGISTER_MIB("cyOutletTable", cyOutletTable_variables, variable4, 
			cyOutletTable_oid);

	/** Initializes the cyPMElecPduTable module */
        REGISTER_MIB("cyPMElecPduTable", cyPMElecPduTable_variables, variable4,
                        cyPMElecPduTable_oid);

	/** Initializes the cyPMElecPhaseTable module */
        REGISTER_MIB("cyPMElecPhaseTable", cyPMElecPhaseTable_variables, variable4,
                        cyPMElecPhaseTable_oid);
	
	/** Initializes the cyPMElecBankTable module */
        REGISTER_MIB("cyPMElecBankTable", cyPMElecBankTable_variables, variable4,
                        cyPMElecBankTable_oid);

	/** Initializes the cyPMElecOutletTable module */
        REGISTER_MIB("cyPMElecOutletTable", cyPMElecOutletTable_variables, variable4,
                        cyPMElecOutletTable_oid);

	/** Initializes the cyPMEnvMonTable module */
        REGISTER_MIB("cyPMEnvMonTable", cyPMEnvMonTable_variables, variable4,
                        cyPMEnvMonTable_oid);
}

/* Get number of PMs in the unit */
int do_cyNumberOfPM(netsnmp_mib_handler *handler,
	netsnmp_handler_registration *reginfo,
	netsnmp_agent_request_info *reqinfo,
	netsnmp_request_info *requests)
{
	int res;
	
	if (reqinfo->mode == MODE_GET) {
		if ( pmc_pmd_status() ) { /* PMD core not running */
			res = 0;
		}
		else {
			res = pmc_numPhysicalPorts("ttyS");
#ifdef CYPM_ONS
			res += pmc_numPhysicalPorts("ttyA");
#endif
		}
		snmp_set_var_typed_value(requests->requestvb, 
					 ASN_INTEGER,(u_char *)&res, sizeof(int));
		return SNMP_ERR_NOERROR;
	} else {
		return SNMP_ERR_GENERR;
	}
}

/* Find out the next PMD serial port number */
static int cy_find_nsp(int first) 
{
	pmInfo *pmc;
	int port;

	pmc = pmc_first_pmInfo();
	port = 0;

	/* No IPDUs connected */
	if ( pmc == NULL ){
		return 0;
	}

	/* Return the first PMD serial port number */
	if ( first == 0 ) {
		return((int)pmc->number);
	} else {
		while ( pmc != NULL ) {
			if ( (int)pmc->number == first ) {
				if ( pmc->next == NULL ) {
					return 0;
				} else {
					return ((int)pmc->next->number);
				}
			}
			pmc = pmc->next;
		}
	}
	return 0;
}

/* Find out the next PMD serial port number with first ipdu ready adn licensed*/
static int cy_find_nsp_default(int first) 
{
	pmInfo *pmc;
	int port;
	ipduInfo *ipdu = NULL;

	pmc = pmc_first_pmInfo();
	port = 0;

	/* No IPDUs connected */
	if ( pmc == NULL ){
		return 0;
	}

	/* Return the first PMD serial port number */
	if ( first == 0 ) {
		while ( pmc != NULL ) {
			ipdu = pmc->ipdu;
			if ((ipdu != NULL) && ipdu->ready && ipdu->license) {
				return((int)pmc->number);
			}
			pmc = pmc->next;
		}
	} else {
		while ( pmc != NULL ) {
			if ( (int)pmc->number == first ) {
				if ( pmc->next == NULL ) {
					return 0;
				} else {
					pmc = pmc->next;
					while ( pmc != NULL ){
						ipdu = pmc->ipdu;
						if ((ipdu != NULL) && ipdu->ready && ipdu->license) {
							return ((int)pmc->number);
						}
					pmc = pmc->next;
					}
					break;
				}
			}
			pmc = pmc->next;
		}
	}
	return 0;
}

/* Find out the next PMD with Envmon serial port number */
static int cy_find_nsp_envMon(int first) 
{
	pmInfo *pmc;
	ipduInfo *ipdu = NULL;

	pmc = pmc_first_pmInfo();

	/* No IPDUs connected */
	if ( pmc == NULL ){
		return 0;
	}
	/* Return the first PMD serial port number */
	if ( first == 0 ) {
		while ( pmc != NULL ){
			ipdu = pmc->ipdu;
			while( ipdu != NULL ){
				if (ipdu->ready && ipdu->license) {
					if (((ipdu->num_envMons == 1) && 
								!(ipdu->envMon->cap & ENVMON_CAP_NOT_PRESENT)) ||
							(int)ipdu->num_envMons > 1 ) {
						return((int)pmc->number);
					}
				}
				ipdu = ipdu->next;
			}
			pmc = pmc->next;
		}
	} else {
		while ( pmc != NULL ) {
			if ( (int)pmc->number == first ) {
				if ( pmc->next == NULL ) {
					return 0;
				} else {
					pmc = pmc->next;
					while ( pmc != NULL ){
						ipdu = pmc->ipdu;
						while( ipdu != NULL ){
							if (ipdu->ready && ipdu->license) {
			                        		if (((ipdu->num_envMons == 1) && 
							     	     !(ipdu->envMon->cap & ENVMON_CAP_NOT_PRESENT)) ||
							    	    (int)ipdu->num_envMons > 1 ) {
			                                		return((int)pmc->number);
								}
							}
							ipdu = ipdu->next;
						}
						pmc = pmc->next;
					}
					break;
				}
			}
			pmc = pmc->next;
		}
	}
	return 0;
}

/* Find out the next PMD number with EnvMon*/
static int cy_find_ipdu_envMon(int nsp, int first) 
{
	pmInfo *pm;
	ipduInfo *ipdu = NULL;

	pm = pmc_validate_port (nsp,"ttyS");

	if ( pm == NULL ) {
		return 0;
	}
	/* Return the first PMD with envmon*/
	if ( first == 0 ) {
		ipdu = pm->ipdu;
		while( ipdu != NULL ){
			if (ipdu->ready && ipdu->license) {
                       		if (((ipdu->num_envMons == 1) && 
			     	     !(ipdu->envMon->cap & ENVMON_CAP_NOT_PRESENT)) ||
			    	    (int)ipdu->num_envMons > 1 ) {
                               		return((int)ipdu->number);
				}
			}
			ipdu = ipdu->next;
		}
	} else {
		ipdu = pm->ipdu;
		while ( ipdu != NULL ) {
			if ( (int)ipdu->number == first ) {
				if ( ipdu->next == NULL ) {
					return 0;
				} else {
					ipdu = ipdu->next;
					while( ipdu != NULL ){
						if (ipdu->ready && ipdu->license) {
			                       		if (((ipdu->num_envMons == 1) && 
						     	     !(ipdu->envMon->cap & ENVMON_CAP_NOT_PRESENT)) ||
						    	    (int)ipdu->num_envMons > 1 ) {
			                               		return((int)ipdu->number);
							}
						}
						ipdu = ipdu->next;
					}
					break;
				}
			}
			ipdu = ipdu->next;
		}
	}
	return 0;
}

/* Find out the next PMD with ElecMon serial port number */
static int cy_find_nsp_elecMon(int first, int element) 
{
	pmInfo *pmc;
	ipduInfo *ipdu = NULL;

	pmc = pmc_first_pmInfo();

	/* No IPDUs connected */
	if ( pmc == NULL ){
		return 0;
	}
	/* Return the first PMD serial port number */
	if ( first == 0 ) {
		while ( pmc != NULL ){
			ipdu = pmc->ipdu;
			while( ipdu != NULL ){
				if (ipdu->ready && ipdu->license) {
					switch(element){
						case ELECPDU :
							if ( (ipdu->sflag & IPDU_CAP_ELECMON) )
								return((int)pmc->number);
							break;
						case ELECPHASE :
							if ( ((int)ipdu->num_phases == 3) 
									&& (ipdu->phase[0].sflag & PHASE_CAP_ELECMON) )
								return((int)pmc->number);
							break;
						case ELECBANK :
							if ( ((int)ipdu->num_circuits != 0)
									&& (ipdu->circuit[0].sflag & CIRCUIT_CAP_ELECMON) )
								return((int)pmc->number);
							break;
						case ELECOUTLET :
							if ( ((int)ipdu->num_outlets != 0)
									&& (ipdu->outlet[0].sflag & OUTLET_CAP_ELECMON) )
								return((int)pmc->number);
							break;
						default :
							break;
					}
				}
				ipdu = ipdu->next;
			}
			pmc = pmc->next;
		}
	} else {
		while ( pmc != NULL ) {
			if ( (int)pmc->number == first ) {
				if ( pmc->next == NULL ) {
					return 0;
				} else {
					pmc = pmc->next;
					while ( pmc != NULL ){
						ipdu = pmc->ipdu;
						while( ipdu != NULL ){
							if (ipdu->ready && ipdu->license) {
								switch(element){
									case ELECPDU :
										if ( (ipdu->sflag & IPDU_CAP_ELECMON) )
											return((int)pmc->number);
										break;
									case ELECPHASE :
										if ( ((int)ipdu->num_phases == 3)
												&& (ipdu->phase[0].sflag & PHASE_CAP_ELECMON) )
											return((int)pmc->number);
										break;
									case ELECBANK :
										if ( ((int)ipdu->num_circuits != 0)
												&& (ipdu->circuit[0].sflag & CIRCUIT_CAP_ELECMON) )
											return((int)pmc->number);
										break;
									case ELECOUTLET :
										if ( ((int)ipdu->num_outlets != 0)
												&& (ipdu->outlet[0].sflag & OUTLET_CAP_ELECMON) )
											return((int)pmc->number);
										break;
									default :
										break;
								}
							}
							ipdu = ipdu->next;
						}
						pmc = pmc->next;
					}
					break;
				}
			}
			pmc = pmc->next;
		}
	}
	return 0;
}

/* Find out the first IPDU in <nsp> serial port with ElecMon*/
static int cy_find_ipdu_elecMon(int nsp, int first, int element) 
{
	pmInfo *pm;
	ipduInfo *ipdu = NULL;

	pm = pmc_validate_port (nsp,"ttyS");

	if ( pm == NULL ) {
		return 0;
	}
	/* Return the first IPDU in <nsp> serial port with element*/
	if ( first == 0 ) {
		ipdu = pm->ipdu;
		while( ipdu != NULL ){
			if (ipdu->ready && ipdu->license) {
				switch(element){
				case ELECPDU :
					if ( (ipdu->sflag & IPDU_CAP_ELECMON) )
						return((int)ipdu->number);
					break;
				case ELECPHASE :
					if ( ((int)ipdu->num_phases == 3)
							&& (ipdu->phase[0].sflag & PHASE_CAP_ELECMON) )
						return((int)ipdu->number);
					break;
				case ELECBANK :
					if ( ((int)ipdu->num_circuits != 0)
							&& (ipdu->circuit[0].sflag & CIRCUIT_CAP_ELECMON) )
						return((int)ipdu->number);
					break;
				case ELECOUTLET :
					if ( ((int)ipdu->num_outlets != 0)
							&& (ipdu->outlet[0].sflag & OUTLET_CAP_ELECMON) )
						return((int)ipdu->number);
					break;
				default :
					break;
				}
			}
			ipdu = ipdu->next;
		}
	} else {
		ipdu = pm->ipdu;
		while ( ipdu != NULL ) {
			if ( (int)ipdu->number == first ) {
				if ( ipdu->next == NULL ) {
					return 0;
				} else {
					ipdu = ipdu->next;
					while( ipdu != NULL ){
						if (ipdu->ready && ipdu->license) {
							switch(element){
							case ELECPDU :
								if ( (ipdu->sflag & IPDU_CAP_ELECMON) )
									return((int)ipdu->number);
								break;
							case ELECPHASE :
								if ( ((int)ipdu->num_phases == 3)
										&& (ipdu->phase[0].sflag & PHASE_CAP_ELECMON) )
									return((int)ipdu->number);
								break;
							case ELECBANK :
								if ( ((int)ipdu->num_circuits != 0)
										&& (ipdu->circuit[0].sflag & CIRCUIT_CAP_ELECMON) )
									return((int)ipdu->number);
								break;
							case ELECOUTLET :
								if ( ((int)ipdu->num_outlets != 0)
										&& (ipdu->outlet[0].sflag & OUTLET_CAP_ELECMON) )
									return((int)ipdu->number);
								break;
							default :
								break;

							}
						}
						ipdu = ipdu->next;
					}
					break;
				}
			}
			ipdu = ipdu->next;
		}
	}
	return 0;
}


/* Check if ipdu is in the range of IPDUs connected to nsp serial port */
static int ipdu_in_pm_range(int nsp, int ipdu)
{
	pmInfo *pm;
	ipduInfo *ipduinfo;

	if((ipdu == 0)||(nsp == 0)){
		return 1;
	}

	pm = pmc_validate_port (nsp,"ttyS");

	if ( pm == NULL ) {
#ifdef CYPM_ONS
		pm = pmc_validate_port(nsp,"ttyA");

		if ( pm == NULL ) {
			/* Serial port nsp does not have IPDUs connected */
			return -1;
		}
#else
		return -1;
#endif
	}

        ipduinfo = pmc_validate_ipdu_old(nsp,ipdu,"ttyS");
	if ( ipduinfo == NULL ) {
		/* IPDU not connected to Serial Port */
		return 1;
	}
	else {
		/* IPDU connected to Serial Port */
		return 0;
	}
}

/* Check if ipdu is in the range of IPDUs connected to nsp serial port with envMon*/
static int ipdu_in_pm_range_envMon(int nsp, int ipdu)
{
	pmInfo *pm;
	ipduInfo *ipduinfo;

	if(ipdu == 0){
		return 1;
	}
	
	pm = pmc_validate_port (nsp,"ttyS");
	
	ipduinfo = pmc_validate_ipdu_old(nsp,ipdu,"ttyS");

	if ( ipduinfo == NULL ) {
                return 1;
        }
	if ((ipduinfo->num_envMons == 0) || 
	    ((ipduinfo->num_envMons == 1) && (ipduinfo->envMon->cap & ENVMON_CAP_NOT_PRESENT))) {
		/* IPDU not connected to Serial Port */
		return 1;
	}
	else {
		/* IPDU connected to Serial Port */
		return 0;
	}
}

/* Check if ipdu is in the range of IPDUs connected to nsp serial port with elecMon element*/
static int ipdu_in_pm_range_elecMon(int nsp, int ipdu, int element)
{
	pmInfo *pm;
	ipduInfo *ipduinfo;
	int element_exists = 0;

	if(ipdu == 0){
		return 1;
	}

	pm = pmc_validate_port (nsp,"ttyS");

	ipduinfo = pmc_validate_ipdu_old(nsp,ipdu,"ttyS");

	if ( ipduinfo == NULL ) {
		return 1;
	}

	switch(element){
		case ELECPDU :
			if ( (ipduinfo->sflag & IPDU_CAP_ELECMON) )
				element_exists = 1;
			break;
		case ELECPHASE :
			if ( ((int)ipduinfo->num_phases == 3)
					&& (ipduinfo->phase[0].sflag & PHASE_CAP_ELECMON) )
				element_exists = 1;
			break;
		case ELECBANK :
			if ( ((int)ipduinfo->num_circuits != 0)
					&& (ipduinfo->circuit[0].sflag & CIRCUIT_CAP_ELECMON) )
				element_exists = 1;
			break;
		case ELECOUTLET :
			if ( ((int)ipduinfo->num_outlets != 0)
					&& (ipduinfo->outlet[0].sflag & OUTLET_CAP_ELECMON) )
				element_exists = 1;
			break;
		default :
			break;
	}

	if (element_exists == 0) {
		/* IPDU not connected to Serial Port */
		return 1;
	}
	else {
		/* IPDU connected to Serial Port */
		return 0;
	}
}

/* Check if outlet is in the range of OUTLETS connected to nsp serial port */
static int outlet_in_pm_range(int nsp, int outlet)
{
	int num_total_outlets;

	if (nsp == 0){
		return 1;
	}
		
	if ((num_total_outlets = pmc_numOutlets_port(nsp,"ttyS")) == 0) {
#ifdef CYPM_ONS
		num_total_outlets = pmc_numOutlets_port(nsp,"ttyA");
		if (num_total_outlets == 0) 
#endif
			return 1;
	}

	if ( outlet > num_total_outlets ) { 
		/* Outlet not in the Serial Port */
		return 1;
	} else {
		/* Outlet in the Serial Port */
		return 0;
	}
}

/* Check if envMon is in the range of ENVMONS connected to nsp serial port */
static int envmon_in_pm_range(int nsp, int ipdu, int envmon)
{
        int num_total_envmons;
	ipduInfo *ipduinfo;
        ipduinfo = pmc_validate_ipdu_old(nsp,ipdu,"ttyS");
        if ( ipduinfo == NULL ) {
                return 1;
        }	
	
	if ((ipduinfo->num_envMons == 0) || 
	    ((ipduinfo->num_envMons == 1) && (ipduinfo->envMon->cap & ENVMON_CAP_NOT_PRESENT))) {
                        return 1;
        }

        num_total_envmons = ipduinfo->num_envMons;

        if ( envmon > num_total_envmons ) { 
                /* Envmon not in the Serial Port */
                return 1;
        } else {
                /* Envmon in the Serial Port */
                return 0;
        }
}

/* Check if elecMon element is in the range of element ELECMONS connected to nsp serial port */
static int elecmon_in_pm_range(int nsp, int ipdu, int elecmon, int element)
{
	int num_total_elecmons = 0;
	ipduInfo *ipduinfo;
	ipduinfo = pmc_validate_ipdu_old(nsp,ipdu,"ttyS");
	if ( ipduinfo == NULL ) {
		return 1;
	}
	
	switch(element){
		case ELECPHASE :
			if ( ((int)ipduinfo->num_phases == 3)
					&& (ipduinfo->phase[0].sflag & PHASE_CAP_ELECMON) )
				num_total_elecmons = 3;
			break;
		case ELECBANK :
			if ( ((int)ipduinfo->num_circuits != 0)
					&& (ipduinfo->circuit[0].sflag & CIRCUIT_CAP_ELECMON) )
				num_total_elecmons = (int)ipduinfo->num_circuits;
			break;
		case ELECOUTLET :
			if ( ((int)ipduinfo->num_outlets != 0)
					&& (ipduinfo->outlet[0].sflag & OUTLET_CAP_ELECMON) )
				num_total_elecmons = (int)ipduinfo->num_outlets;
			break;
		default :
			break;
	}

	if (num_total_elecmons == 0) {
		return 1;
	}

	if ( elecmon > num_total_elecmons ) {
		/* element Elecmon not in the Serial Port */
		return 1;
	} else {
		/* element Elecmon in the Serial Port */
		return 0;
	}
}

/* Compare 'name' to vp->name for the best match or an exact match
 * Also check that 'name' is valid PM port serial number.
 * Store a successful match in 'name', and increment the OID instance if the
 * match was not exact.
 */
static int header_PMTable(struct variable *vp, oid * name, size_t * length,
			int exact, size_t * var_len, WriteMethod ** write_method)
{
	int i, rtest,nsp;
	oid newname[MAX_OID_LEN];

	for (i=0, rtest=0; (i < (int)vp->namelen) && (i < (int)(*length)) &&
		 !rtest; i++) {
		if (name[i] != vp->name[i]) {
			if (name[i] < vp->name[i])
				rtest = -1;
			else
				rtest = 1;
		}
	}

	// find the first serial port with PM connected
	nsp = cy_find_nsp(0);

	if ((rtest > 0) || 
		((exact == 1) && (rtest || ((int)*length != (int)(vp->namelen+1))))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memset(newname, 0, sizeof(newname));

	if (((int)*length <= (int)vp->namelen) || (rtest == -1)) {
		memmove(newname, vp->name,(int)vp->namelen * sizeof(oid));
		newname[vp->namelen] = nsp; 
		*length = vp->namelen + 1;
	} else {
		if ((int)*length > ((int)vp->namelen+1)) {
			// exact case checked earlier
			*length = vp->namelen + 1;
			memmove(newname, name, (int)*length * sizeof(oid));
			if (name[*length - 1] < ULONG_MAX) {
				nsp = cy_find_nsp(name[*length - 1]);
				newname[*length - 1] = nsp;
			} else {
				nsp = newname[*length - 1] = name[*length - 1];
			}
		} else {
			*length = vp->namelen + 1;
			memmove(newname,name, (*length) * sizeof(oid));
			if (!exact) {
				if (name[*length - 1] < ULONG_MAX) {
					nsp = cy_find_nsp(name[*length - 1]);
					newname[*length - 1] = nsp;
				} else {
					nsp = newname[*length - 1] = name[*length - 1];
				}
			}
		}
	}

	if (nsp == 0) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memmove(name, newname, (*length) * sizeof(oid));
	// default values
	if (write_method) *write_method = 0;
	if (var_len) *var_len = sizeof(long);

	return MATCH_SUCCEEDED;
}		

/*
 * var_cyPMTable():
 */
unsigned char *var_cyPMTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp;
	pmInfo *pmc;
	ipduInfo *ipdu;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_PMTable(vp, name, length, exact, var_len, write_method) == MATCH_FAILED) 
		return NULL;

	memset(string, 0, 1024);

	cy_nsp = name[CYPM_INDEX];

	pmc = pmc_validate_port(cy_nsp,"ttyS");
#ifdef CYPM_ONS
	if ( pmc == NULL ) {
		pmc = pmc_validate_port(cy_nsp,"ttyA");
		if ( pmc == NULL ) {
			return NULL; /* It is not an IPDU */
		}
	}
#else
	if ( pmc == NULL ) {
		return NULL;	/* It is not an IPDU */
	}
#endif

	ipdu = pmc->ipdu;

	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYPMSerialPortN :
			long_ret = pmc->number;
			return (unsigned char *) &long_ret;
			break;

		case CYPMNumOutlets:
			long_ret = pmc_numOutlets_pmInfo(pmc);
			return (unsigned char *) &long_ret;
			break;

		case CYPMNumUnits:
			long_ret = pmc->num_ipdus; 
			return (unsigned char *) &long_ret;
			break;

		case CYPMCurrent:
			pmc_show_chain_current(pmc,string);
			*var_len = strlen(string);
			return (unsigned char *) &string;
			break;
			
		case CYPMVersion:
			pmc_show_chain_version (pmc,string);
			*var_len = strlen(string);
			return (unsigned char *) &string;
			break;
			
#ifdef OBSOLETED
		case CYPMTemperature:
			pmc_show_chain_temperature (pmc,string);
			*var_len = strlen(string);
			return (unsigned char *) &string;
			break;
#endif			
		case CYPMCommand:
			*var_len = 0;
			*write_method = write_cyPM_command;
			return (unsigned char *) &string;
			break;
			
		default:
			break;
	}
	return NULL;
}

int write_cyPM_command(int action, u_char * var_val, u_char var_val_type,
		   size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp;
	int cy_pm;
	char nsp_name[IPDUIDSIZE+1];
	char cmd[100];

	cy_nsp = name[CYPM_INDEX];
	cy_pm = name[CYPMUnit_INDEX];

	find_ipdu_name(cy_nsp,cy_pm,NULL,nsp_name);

	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_OCTET_STR) {
				fprintf(stderr,"write to cyPMCommand not ASN_OCTET_STR\n");
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len >= sizeof(cmd)) {
				return SNMP_ERR_WRONGLENGTH;
			}
			break;

		case MODE_SET_ACTION:
			snprintf(cmd, sizeof(cmd), "%.*s", var_val_len, var_val);
			cypm_ng_system(cmd, nsp_name, -1, NULL, 0);
			break;

		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:
		case MODE_SET_FREE:
		case MODE_SET_UNDO:
			break;

	}
	return SNMP_ERR_NOERROR;
}

/*
 * var_cyPMUnitTable():
 */
unsigned char *var_cyPMUnitTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp,cy_pm;
	pmInfo *pmc;
	ipduInfo *ipdu;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_PMUnitTable(vp, name, length, exact, var_len, write_method) == MATCH_FAILED) {
		return NULL;
	}

	memset(string, 0, 1024);

	cy_nsp = name[CYPM_INDEX];
	cy_pm = name[CYPMUnit_INDEX];

	/* Check PMD serial port and IPDU number */
	pmc = pmc_validate_port(cy_nsp,"ttyS");
#ifdef CYPM_ONS
	if ( pmc == NULL ) {
		pmc = pmc_validate_port(cy_nsp,"ttyA");
		if ( pmc == NULL ) {
			return NULL;	/* It is not an IPDU */
		}
	}
#else
	if ( pmc == NULL ) {
		return NULL;	/* It is not an IPDU */
	}
#endif
	else {
		/* Check IPDU number */
		ipdu = pmc_validate_ipdu_old (cy_nsp,cy_pm,"ttyS"); 
#ifdef CYPM_ONS
		if ( ipdu == NULL ) {
			ipdu = pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyA");
			if ( ipdu == NULL ) {
				return NULL;
			}
		}
#else
		if ( ipdu == NULL ) {
			return NULL; /* IPDU number not found */
		}
#endif
	}

	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYPMUnitVersion:
			strncpy(string,ipdu->version,strlen(ipdu->version));
			*var_len = strlen(string);
			return (unsigned char *) &string;
			break;
			
		case CYPMUnitOutlets:
			long_ret = ipdu->num_outlets;
			return (unsigned char *) &long_ret;
			break;
			
		case CYPMUnitFirstOutlet:
			long_ret = first_outlet_number(pmc,cy_pm);
			return (unsigned char *) &long_ret;
			
		case CYPMUnitCurrent :
			if ( ipdu->sflag & IPDU_CAP_ELECMON) {
				long_ret = (long) ipdu->elecMon.current; 
			}
			else {
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;

		case CYPMUnitMaxCurrent : 
			if ( (ipdu->sflag & IPDU_CAP_ELECMON)  && 
			     (ipdu->elecMon.sflag & ELECMON_CAP_MAX_CURRENT)) {
				long_ret = (long int) ipdu->elecMon.max_current;
			} else {
				long_ret = 0;
			}
			*write_method = write_cyPMUnit_current;
			return (unsigned char *) &long_ret;

#ifdef OBSOLETED
		case CYPMUnitTemperature :
			if ( ipdu->num_envmons ) {
				long_ret = (long)(ipdu->temperature[0].value - 2730);
			}
			else {
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;

		case CYPMUnitMaxTemperature :
			if ( ipdu->num_temperature_sensors &&
			     (ipdu->temperature[0].cap & SENSOR_CAP_MAXVALUE)) {
				long_ret = (long)(ipdu->temperature[0].max_value - 2730);
			}
			else {
				long_ret = 0;
			}
			*write_method = write_cyPMUnit_temp; 
			return (unsigned char *) &long_ret;
#endif
		case CYPMUnitSequenceInterval :
			if ( ipdu->sflag & IPDU_CAP_SEQINTERVAL ) {
				long_ret = (long) ipdu->seqinterval;
			} else {
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;

		case CYPMUnitCycleInterval :
			if ( ipdu->sflag & IPDU_CAP_REBOOTDELAY ) {
				long_ret = (long) ipdu->cycledelay;
			} else {
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;

		case CYPMUnitID :
			strncpy(string,ipdu->id,strlen(ipdu->id));
			*var_len = strlen(string);
			return (unsigned char *) &string;

		case CYPMUnitPhases :
			if ( ipdu->num_phases == 3 ){
				long_ret = 3;
			}
			else{
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;

		case CYPMUnitBanks :
			long_ret = ipdu->num_circuits;
			return (unsigned char *) &long_ret;

		case CYPMUnitNominalVoltage :
			if ( ipdu->elecMon.voltage_type == ESTIMATED ){
				long_ret = ipdu->elecMon.voltage;
			}
			else{
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;

		case CYPMUnitPower :
			long_ret = ipdu->elecMon.power;
			return (unsigned char *) &long_ret;
		
		default:
			break;
	}
	return NULL;
}

/* Compare 'name' to vp->name for the best match or an exact match
 * Also check that 'name' is valid PM port serial number.
 * Store a successful match in 'name', and increment the OID instance if the
 * match was not exact.
 */
static int header_PMUnitTable(struct variable *vp, oid * name, size_t * length,
		int exact, size_t * var_len, WriteMethod ** write_method)
{
	int i, rtest,nsp,ipdu;
	oid newname[MAX_OID_LEN];

	for (i=0, rtest=0; (i < (int)vp->namelen) && (i < (int)(*length)) &&
		 !rtest; i++) {
		if (name[i] != vp->name[i]) {
			if (name[i] < vp->name[i])
				rtest = -1;
			else
				rtest = 1;
		}
	}

	// find the first serial port with PM connected
	nsp = cy_find_nsp_default(0);
	ipdu = 1;

	if ((rtest > 0) || 
		((exact == 1) && (rtest || ((int)*length != (int)(vp->namelen+2))))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memset(newname, 0, sizeof(newname));

	if (((int)*length <= (int)vp->namelen) || (rtest == -1)) {
		memmove(newname, vp->name,(int)vp->namelen * sizeof(oid));
		newname[vp->namelen] = nsp; 
		newname[vp->namelen + 1] = ipdu; 
		*length = vp->namelen + 2;
	} else {
		if ((int)*length > ((int)vp->namelen+2)) {
			// exact case checked earlier
			*length = vp->namelen + 2;
			memmove(newname, name, (int)*length * sizeof(oid));
			if (name[*length - 1] < ULONG_MAX) {
				nsp = name[*length - 2];
				ipdu = newname[*length - 1] = name[*length-1] + 1;
				if ( ipdu_in_pm_range(nsp,ipdu) ) {
					nsp = cy_find_nsp_default(nsp);
					newname[*length - 2] = nsp;
					ipdu = newname[*length - 1] = 1;
				} 
			} else {
				nsp = newname[*length - 2] = name[*length - 2];
				ipdu = newname[*length - 1] = name[*length - 1];
			}
		} else {
			*length = vp->namelen + 2;
			memmove(newname,name, (*length) * sizeof(oid));
			if (!exact) {
				if (name[*length - 1] < ULONG_MAX) {
					nsp = name[*length - 2];
					ipdu = newname[*length - 1] = name[*length-1] + 1;
					if ( ipdu_in_pm_range(nsp,ipdu) ) {
						nsp = cy_find_nsp_default(nsp);
						newname[*length - 2] = nsp;
						ipdu = newname[*length - 1] = 1;
					} 
				} else {
					nsp = newname[*length - 2] = name[*length - 2];
					ipdu = newname[*length - 1] = name[*length - 1];
				}
			}
		}
		
	}

	//if ((nsp == 0) || (ipdu > cy_shm->PMD_pms[nsp-1])) {
	if ((nsp == 0) || (ipdu_in_pm_range(nsp,ipdu))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memmove(name, newname, (*length) * sizeof(oid));
	// default values
	if (write_method) *write_method = 0;
	if (var_len) *var_len = sizeof(long);
	
	return MATCH_SUCCEEDED;
}		

int write_cyPMUnit_current(int action, u_char * var_val, u_char var_val_type,
	   size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp, cy_pm;
	ipduInfo *ipduinfo;

	cy_nsp = name[CYPM_INDEX];
	cy_pm = name[CYPMUnit_INDEX];

	ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");

        if ( !(ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_CURRENT) ){
                return SNMP_ERR_GENERR;
        }

	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_INTEGER) {
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > sizeof(long)) {
				return SNMP_ERR_WRONGLENGTH;
			}
			if (*(long*)var_val) {
					return SNMP_ERR_WRONGVALUE;
			}
			break;

		case MODE_SET_ACTION:
			pmc_sendcmd_elecmon_reset(ipduinfo, ELEMENT_IPDU, ELECMON_CAP_CURRENT, NULL, NULL);
			break;

		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:
		case MODE_SET_FREE:
		case MODE_SET_UNDO:
			break;

	}
	return SNMP_ERR_NOERROR;
}

#ifdef OBSOLETED
int write_cyPMUnit_temp(int action, u_char * var_val, u_char var_val_type,
		   size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp, cy_pm;
	char nsp_name[IPDUIDSIZE+1];

	cy_nsp = name[CYPM_INDEX];
	cy_pm = name[CYPMUnit_INDEX];

	find_ipdu_name(cy_nsp,cy_pm,NULL,nsp_name);

	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_INTEGER) {
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > sizeof(long)) {
				return SNMP_ERR_WRONGLENGTH;
			}
			if (*(long*)var_val) {
					return SNMP_ERR_WRONGVALUE;
			}
			break;

		case MODE_SET_ACTION:
			cypm_ng_system("temperature reset",nsp_name,-1,NULL,0);
			break;

		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:
		case MODE_SET_FREE:
		case MODE_SET_UNDO:
			break;

	}
	return SNMP_ERR_NOERROR;
}
#endif

/* Compare 'name' to vp->name for the best match or an exact match
 * Also check that 'name' is valid PM port serial number.
 * Store a successful match in 'name', and increment the OID instance if the
 * match was not exact.
 */
static int header_OutletTable(struct variable *vp, oid * name, size_t * length,
			int exact, size_t * var_len, WriteMethod ** write_method)
{
	int i, rtest,nsp,outlet;
	oid newname[MAX_OID_LEN];

	for (i=0, rtest=0; (i < (int)vp->namelen) && (i < (int)(*length)) &&
		 !rtest; i++) {
		if (name[i] != vp->name[i]) {
			if (name[i] < vp->name[i])
				rtest = -1;
			else
				rtest = 1;
		}
	}

	// find the first serial port with PM connected
	nsp = cy_find_nsp_default(0);
	outlet = 1;

	if ((rtest > 0) || 
		((exact == 1) && (rtest || ((int)*length != (int)(vp->namelen+2))))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memset(newname, 0, sizeof(newname));

	if (((int)*length <= (int)vp->namelen) || (rtest == -1)) {
		memmove(newname, vp->name,(int)vp->namelen * sizeof(oid));
		newname[vp->namelen] = nsp; 
		newname[vp->namelen + 1] = outlet; 
		*length = vp->namelen + 2;
	} else {
		if ((int)*length > ((int)vp->namelen+2)) {
			// exact case checked earlier
			*length = vp->namelen + 2;
			memmove(newname, name, (int)*length * sizeof(oid));
			if (name[*length - 1] < ULONG_MAX) {
				nsp = name[*length - 2];
				outlet = newname[*length - 1] = name[*length-1] + 1;
				if ( outlet_in_pm_range(nsp,outlet) ) {
					nsp = cy_find_nsp_default(nsp);
					newname[*length - 2] = nsp;
					outlet = newname[*length - 1] = 1;
				} 
			} else {
				nsp = newname[*length - 2] = name[*length - 2];
				outlet = newname[*length - 1] = name[*length - 1];
			}
		} else {
			*length = vp->namelen + 2;
			memmove(newname,name, (*length) * sizeof(oid));
			if (!exact) {
				if (name[*length - 1] < ULONG_MAX) {
					nsp = name[*length - 2];
					outlet = newname[*length - 1] = name[*length-1] + 1;
					if ( outlet_in_pm_range(nsp,outlet) ) {
						nsp = cy_find_nsp_default(nsp);
						newname[*length - 2] = nsp;
						outlet = newname[*length - 1] = 1;
					} 
				} else {
					nsp = newname[*length - 2] = name[*length - 2];
					outlet = newname[*length - 1] = name[*length - 1];
				}
			}
		}
		
	}

	//if ((nsp == 0) || (outlet > cy_shm->PMD_outlets[nsp-1])) {
	if ((nsp == 0) || outlet_in_pm_range(nsp,outlet)) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memmove(name, newname, (*length) * sizeof(oid));
	// default values
	if (write_method) *write_method = 0;
	if (var_len) *var_len = sizeof(long);

	return MATCH_SUCCEEDED;
}		

/*
 * var_cyOutletTable():
 */
unsigned char *var_cyOutletTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp,cy_outlet,server;
	oData *outlet;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_OutletTable(vp, name, length, exact, var_len, write_method) == MATCH_FAILED)
		return NULL;

	memset(string, 0, 1024);

	cy_nsp = name[CYPM_INDEX];
	cy_outlet = name[CYOUTLET_INDEX];

	/* Check PMD serial port */
	outlet = pmc_validate_outlet_number(cy_nsp,cy_outlet,NULL,"ttyS");
#ifdef CYPM_ONS
	if ( outlet == NULL ) {
		outlet = pmc_validate_outlet_number(cy_nsp,cy_outlet,NULL,"ttyA");
		if ( outlet == NULL ) {
			return NULL;	/* Outlet number not valid */
		}
	}
#else
	if ( outlet == NULL ) {
		return NULL;	/* Outlet number not valid */
	}
#endif

	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYOUTLETName:
			strncpy(string,outlet->name,strlen(outlet->name));
			*write_method = write_cyOutlet_name;
			*var_len = strlen(string);
			return (unsigned char *) &string;
			break;
			
		case CYOUTLETServer:
			server = pmc_servername_out(outlet);
			if (server) {
				strcpy(string,cy_shm->line_conf[server-1].alias);
			} else {
				strcpy(string,"none");
			}
			*var_len = strlen(string);
			return (unsigned char *) &string;
			break;
			
		case CYOUTLETPower:
			if (outlet->sflag & OUTLET_CAP_RELSTATUS) {
				if (outlet->rstatus == OUTLET_RELSTATUS_OFF) {
					long_ret = 0;
				} else {
					long_ret = 1;
				}
			} else {
				long_ret = outlet_status_to_power(outlet->status);
			}
			*write_method = write_cyOutlet_power;
			return (unsigned char *) &long_ret;
			break;

		case CYOUTLETLock:
			long_ret = outlet_status_to_lock(outlet->status);
			*write_method = write_cyOutlet_lock;
			return (unsigned char *) &long_ret;
			
		case CYOUTLETInterval:
			if (outlet->sflag & OUTLET_CAP_POSTPOWERON) {
				long_ret = outlet->post_power_on;
			} else {
				long_ret = 0;
			}
			*write_method = write_cyOutlet_interval;
			return (unsigned char *) &long_ret;
			
		case CYOUTLETMinimumOnInterval:
			if (outlet->cap & OUTLET_CAP_MINIMUMON) {
				long_ret = outlet->minimum_on;
			} else {
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;
			
		case CYOUTLETMinimumOffInterval:
			if (outlet->cap & OUTLET_CAP_MINIMUMOFF) {
				long_ret = outlet->minimum_off;
			} else {
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;
			
		case CYOUTLETWakeupState:
			if (outlet->cap & OUTLET_CAP_WAKEUP) {
				long_ret = (long)outlet->wakeup;
			} else {
				long_ret = 0;
			}
			return (unsigned char *) &long_ret;
		
		case CYOUTLETPduID :
			strncpy(string,outlet->ipdu->id,strlen(outlet->ipdu->id));
                        *var_len = strlen(string);
                        return (unsigned char *) &string;
		
		case CYOUTLETPduNumber :
			long_ret = outlet->ipdu->number;
			return (unsigned char *) &long_ret;

		default:
			break;
	}
	return NULL;
}

int write_cyOutlet_name(int action, u_char * var_val, u_char var_val_type,
	size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp, cy_outlet;
	oData *outlet;

	cy_nsp = name[CYPM_INDEX];
	cy_outlet = name[CYOUTLET_INDEX];

	/* Check PMD serial port */
        outlet = pmc_validate_outlet_number(cy_nsp,cy_outlet,NULL,"ttyS");

	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_OCTET_STR) {
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > 8) {
				return SNMP_ERR_WRONGLENGTH;
			}
			break;
		
		case MODE_SET_ACTION:
			pmc_sendcmd_outcfg_name(outlet,var_val,NULL,NULL);
			break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

	}
        return SNMP_ERR_NOERROR;
}

int write_cyOutlet_power(int action, u_char * var_val, u_char var_val_type,
		   size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp, cy_outlet;
	void *outlet;
	char multOutFlag;

	cy_nsp = name[CYPM_INDEX];
	cy_outlet = name[CYOUTLET_INDEX];

	/* Check PMD serial port */
	
	if(cy_outlet == 0) { // handle all outlets on a port
		multOutFlag = OUTLIST_CHAIN;
		outlet = pmc_validate_port(cy_nsp, "ttyS");
	}
	else { // handle a specific outlet
		multOutFlag = SINGLE_OUTLET;
    	outlet = pmc_validate_outlet_number(cy_nsp,cy_outlet,NULL,"ttyS");
    }

	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_INTEGER) {
				fprintf(stderr,"write to cyOutletPower not ASN_INTEGER\n");
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > sizeof(long)) {
				return SNMP_ERR_WRONGLENGTH;
			}
			if (*(long*)var_val > 2) {
				return SNMP_ERR_WRONGVALUE;
			}
			break;

		case MODE_SET_ACTION:
			switch (*(long *) var_val) {
				case 0 : // Power OFF
					pmc_sendcmd_outlet(OUTLET_CONTROL_OFF,multOutFlag,outlet,NULL,NULL);
					break;
				case 1 : // Power ON
					pmc_sendcmd_outlet(OUTLET_CONTROL_ON,multOutFlag,outlet,NULL,NULL);
					break;
				case 2 : // Power CYCLE
					pmc_sendcmd_outlet(OUTLET_CONTROL_CYCLE,multOutFlag,outlet,NULL,NULL);
					break;
				default :
					return SNMP_ERR_WRONGVALUE;
					break;
			}
			break;

		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:
		case MODE_SET_FREE:
		case MODE_SET_UNDO:
			break;

	}
	return SNMP_ERR_NOERROR;
}

int write_cyOutlet_lock(int action, u_char * var_val, u_char var_val_type,
		   size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp, cy_outlet;
	void *outlet;
	char multOutFlag;

	cy_nsp = name[CYPM_INDEX];
	cy_outlet = name[CYOUTLET_INDEX];

	/* Check PMD serial port */
	
	if(cy_outlet == 0) { // handle all outlets on a port
		multOutFlag = OUTLIST_CHAIN;
		outlet = pmc_validate_port(cy_nsp, "ttyS");
	}
	else { // handle a specific outlet
		multOutFlag = SINGLE_OUTLET;
    	outlet = pmc_validate_outlet_number(cy_nsp,cy_outlet,NULL,"ttyS");
    }

	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_INTEGER) {
				fprintf(stderr,"write to cyOutletLock not ASN_INTEGER\n");
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > sizeof(long)) {
				return SNMP_ERR_WRONGLENGTH;
			}
			if (*(long*)var_val > 1) {
				return SNMP_ERR_WRONGVALUE;
			}
			break;

		case MODE_SET_ACTION:
			switch (*(long *) var_val) {
				case 0 : // UNLOCK
					//cypm_ng_system("unlock",nsp_name,cy_outlet,NULL,0);
					pmc_sendcmd_outlet(OUTLET_CONTROL_UNLOCK,multOutFlag,outlet,NULL,NULL);
					break;
				case 1 : // LOCK
					//cypm_ng_system("lock",nsp_name,cy_outlet,NULL,0);
					pmc_sendcmd_outlet(OUTLET_CONTROL_LOCK,multOutFlag,outlet,NULL,NULL);
					break;
				default :
					return SNMP_ERR_WRONGVALUE;
					break;
			}
			break;

		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:
		case MODE_SET_FREE:
		case MODE_SET_UNDO:
			break;

	}
	return SNMP_ERR_NOERROR;
}

int write_cyOutlet_interval(int action, u_char * var_val, u_char var_val_type,
		   size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp, cy_outlet;
	oData *outlet;

	cy_nsp = name[CYPM_INDEX];
	cy_outlet = name[CYOUTLET_INDEX];

	/* Check PMD serial port */
        outlet = pmc_validate_outlet_number(cy_nsp,cy_outlet,NULL,"ttyS");

	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_INTEGER) {
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > sizeof(long)) {
				return SNMP_ERR_WRONGLENGTH;
			}
			if (*(long*)var_val > 655360) {
				return SNMP_ERR_WRONGVALUE;
			}
			if (*(long*)var_val < 5) {
				return SNMP_ERR_WRONGVALUE;
			}
			break;

		case MODE_SET_ACTION:
			pmc_sendcmd_outcfg_interval((double) *((long *) var_val) / 10.0,SINGLE_OUTLET,outlet,OUTLET_CAP_POSTPOWERON,NULL,NULL);
			break;

		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:
		case MODE_SET_FREE:
		case MODE_SET_UNDO:
			break;

	}
	return SNMP_ERR_NOERROR;
}

/* Compare 'name' to vp->name for the best match or an exact match
 * Also check that 'name' is valid PM port serial number.
 * Store a successful match in 'name', and increment the OID instance if the
 * match was not exact.
 */
static int header_PMElecPduTable(struct variable *vp, oid * name, size_t * length,
		int exact, size_t * var_len, WriteMethod ** write_method)
{
	int i, rtest,nsp,ipdu;
	oid newname[MAX_OID_LEN];

	for (i=0, rtest=0; (i < (int)vp->namelen) && (i < (int)(*length)) &&
			!rtest; i++) {
		if (name[i] != vp->name[i]) {
			if (name[i] < vp->name[i])
				rtest = -1;
			else
				rtest = 1;
		}
	}

	// find the first serial port with PM which has a element with elecmon connected
	nsp = cy_find_nsp_elecMon(0,ELECPDU);
	// find the first PM on given serial port which has a element with elecmon connected
	ipdu = cy_find_ipdu_elecMon(nsp,0,ELECPDU);

	if ((rtest > 0) || 
			((exact == 1) && (rtest || ((int)*length != (int)(vp->namelen+2))))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memset(newname, 0, sizeof(newname));

	if (((int)*length <= (int)vp->namelen) || (rtest == -1)) {
		memmove(newname, vp->name,(int)vp->namelen * sizeof(oid));
		newname[vp->namelen] = nsp; 
		newname[vp->namelen + 1] = ipdu; 
		*length = vp->namelen + 2;
	} else {
		if ((int)*length > ((int)vp->namelen+2)) {
			// exact case checked earlier
			*length = vp->namelen + 2;
			memmove(newname, name, (int)*length * sizeof(oid));
			if (name[*length - 1] < ULONG_MAX) {
				nsp = name[*length - 2];
				ipdu = newname[*length - 1] = name[*length - 1] + 1;
				if ( ipdu_in_pm_range_elecMon(nsp,ipdu,ELECPDU) ) {
					nsp = cy_find_nsp_elecMon(nsp,ELECPDU);
					ipdu = cy_find_ipdu_elecMon(nsp,0,ELECPDU);
					newname[*length - 2] = nsp;
					newname[*length - 1] = ipdu;
				} 
			} else {
				nsp = newname[*length - 2] = name[*length - 2];
				ipdu = newname[*length - 1] = name[*length - 1];
			}
		} else {
			*length = vp->namelen + 2;
			memmove(newname,name, (*length) * sizeof(oid));
			if (!exact) {
				if (name[*length - 1] < ULONG_MAX) {
					nsp = name[*length - 2];
					ipdu = newname[*length - 1] = name[*length - 1] + 1;
					if ( ipdu_in_pm_range_elecMon(nsp,ipdu,ELECPDU) ) {
						nsp = cy_find_nsp_elecMon(nsp,ELECPDU);
						ipdu = cy_find_ipdu_elecMon(nsp,0,ELECPDU);
						newname[*length - 2] = nsp;
						newname[*length - 1] = ipdu;
					} 
				} else {
					nsp = newname[*length - 2] = name[*length - 2];
					ipdu = newname[*length - 1] = name[*length - 1];
				}
			}
		}

	}

	//if ((nsp == 0) || (ipdu > cy_shm->PMD_pms[nsp-1])) {
	if ((nsp == 0) || (ipdu_in_pm_range_elecMon(nsp,ipdu,ELECPDU))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memmove(name, newname, (*length) * sizeof(oid));
	// default values
	if (write_method) *write_method = 0;
	if (var_len) *var_len = sizeof(long);

	return MATCH_SUCCEEDED;
}		


/*
 * var_cyPMElecPduTable():
 */
unsigned char *var_cyPMElecPduTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp,cy_pm;
	ipduInfo *ipduinfo;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_PMElecPduTable(vp, name, length, exact, var_len, write_method) == MATCH_FAILED) {
		return NULL;
	}

	memset(string, 0, 1024);

	cy_nsp = name[CYPMElecPdu_PN_INDEX];
	cy_pm = name[CYPMElecPdu_PDUN_INDEX];

	/* Check PMD serial port and IPDU number */
	ipduinfo = pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");

	if ( ipduinfo == NULL ) {
                return NULL;
        }
	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYPMElecPduTablePortNumber :
			long_ret = ipduinfo->pmInfo->number;
			return (unsigned char *) &long_ret;	
			
		case CYPMElecPduTablePduNumber :
			long_ret = ipduinfo->number;
			return (unsigned char *) &long_ret;
			
		case CYPMElecPduTableCurrentValue : 
			long_ret = ipduinfo->elecMon.current;
			return (unsigned char *) &long_ret;

		case CYPMElecPduTableCurrentMax :
			if ( ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_CURRENT ){
                        	long_ret = ipduinfo->elecMon.max_current;
			}
			else {
				long_ret = 0;
			}
			*write_method = write_cyPMElecPduTable_CurrValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecPduTablePowerValue : 
			long_ret = ipduinfo->elecMon.power;
			return (unsigned char *) &long_ret;

		case CYPMElecPduTablePowerMax :
			if ( ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_POWER ){
                                long_ret = ipduinfo->elecMon.max_power;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecPduTable_PowValRst;
			return (unsigned char *) &long_ret;
		
		case CYPMElecPduTableVoltageValue : 
			long_ret = ipduinfo->elecMon.voltage;
			return (unsigned char *) &long_ret;

		case CYPMElecPduTableVoltageMax :
			if ( ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE ){
	                        long_ret = ipduinfo->elecMon.max_voltage;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecPduTable_VoltValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecPduTablePowerFactorValue : 
			long_ret = ipduinfo->elecMon.power_fac;
			return (unsigned char *) &long_ret;

		case CYPMElecPduTablePowerFactorMax :
			if ( ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR ){
                        	long_ret = ipduinfo->elecMon.max_power_fac;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecPduTable_PowfacValRst;
			return (unsigned char *) &long_ret;

		case CYPMElecPduTablePowerType :
                        if ( ipduinfo->elecMon.power_type == MEASURED ){
                                long_ret = 2;
                        }
                        else{
                                long_ret = 1;
                        }
                        return (unsigned char *) &long_ret;

		case CYPMElecPduTableVoltageType :
			if ( ipduinfo->elecMon.voltage_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;

		case CYPMElecPduTablePowerFactorType :
			if ( ipduinfo->elecMon.power_fac_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;	

		default:
			break;
	}
	return NULL;
}

int write_cyPMElecPduTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPdu_PN_INDEX];
        cy_pm = name[CYPMElecPdu_PDUN_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
/*
	if ( !(ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_CURRENT) ){
		return SNMP_ERR_GENERR;
	}
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(ipduinfo, ELEMENT_IPDU, ELECMON_CAP_CURRENT, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecPduTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPdu_PN_INDEX];
        cy_pm = name[CYPMElecPdu_PDUN_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
/*
        if ( !(ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_POWER) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
			pmc_sendcmd_elecmon_reset(ipduinfo, ELEMENT_IPDU, ELECMON_CAP_POWER, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecPduTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPdu_PN_INDEX];
        cy_pm = name[CYPMElecPdu_PDUN_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
/*
        if ( !(ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
			pmc_sendcmd_elecmon_reset(ipduinfo, ELEMENT_IPDU, ELECMON_CAP_VOLTAGE, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecPduTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPdu_PN_INDEX];
        cy_pm = name[CYPMElecPdu_PDUN_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
/*
        if ( !(ipduinfo->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                         pmc_sendcmd_elecmon_reset(ipduinfo, ELEMENT_IPDU, ELECMON_CAP_POWER_FACTOR, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

/* Compare 'name' to vp->name for the best match or an exact match
 * Also check that 'name' is valid PM port serial number.
 * Store a successful match in 'name', and increment the OID instance if the
 * match was not exact.
 */
static int header_PMElecElementTable(struct variable *vp, oid * name, size_t * length,
		int exact, size_t * var_len, WriteMethod ** write_method, int element)
{
	int i, rtest,nsp,ipdu,elemn;
	oid newname[MAX_OID_LEN];

	for (i=0, rtest=0; (i < (int)vp->namelen) && (i < (int)(*length)) &&
		 !rtest; i++) {
		if (name[i] != vp->name[i]) {
			if (name[i] < vp->name[i])
				rtest = -1;
			else
				rtest = 1;
		}
	}
	// find the first serial port with PM which has a element with elecmon connected
	nsp = cy_find_nsp_elecMon(0,element);
	// find the first IPDU on given serial port which has a element with elecmon connected
	ipdu = cy_find_ipdu_elecMon(nsp,0,element);

	elemn = 1;

	if ((rtest > 0) || 
		((exact == 1) && (rtest || ((int)*length != (int)(vp->namelen+3))))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memset(newname, 0, sizeof(newname));

	if (((int)*length <= (int)vp->namelen) || (rtest == -1)) {
		memmove(newname, vp->name,(int)vp->namelen * sizeof(oid));
		newname[vp->namelen] = nsp;
		newname[vp->namelen + 1] = ipdu;
		newname[vp->namelen + 2] = 1;
		*length = vp->namelen + 3;
	} else {
		// check element number
		if ((int)*length >= ((int)vp->namelen+3)) {
			*length = vp->namelen + 3;
			memmove(newname, name, (int)*length * sizeof(oid));
			if (name[*length - 2] < ULONG_MAX) {
				nsp = name[*length - 3];
				ipdu = newname[*length - 2] = name[*length-2];
				if (exact) {
					elemn = newname[*length - 1] = name[*length-1];
				} else {
					elemn = newname[*length - 1] = name[*length-1] + 1;
					// check number of elements in this port/ipdu
					if (elecmon_in_pm_range(nsp,ipdu,elemn,element)) {
						//ipdu = find next ipdu on this nsp with element with elecmon; 
						ipdu = cy_find_ipdu_elecMon(nsp,ipdu,element);
						newname[*length - 2] = ipdu;
						elemn = newname[*length - 1] = 1;
						// get number of pdus in this port 
						if ( ipdu_in_pm_range_elecMon(nsp,ipdu,element) ) {
							//nsp = find next port and ipdu with element with elecmon;
							nsp = cy_find_nsp_elecMon(nsp,element);
							ipdu = cy_find_ipdu_elecMon(nsp,0,element);    
							newname[*length - 3] = nsp;
							newname[*length - 2] = ipdu;
						}
					}
				}
			} else {
				nsp = newname[*length - 3] = name[*length - 3];
				ipdu = newname[*length - 2] = name[*length - 2];
				elemn = newname[*length - 1] = name[*length - 1];
			}
		} else {
			// pdu index
			if ((int)*length > ((int)vp->namelen+2)) {
				*length = vp->namelen + 3;
				memmove(newname, name, (int)*length * sizeof(oid));
				if (name[*length - 2] < ULONG_MAX) {
					nsp = name[*length - 3];
					ipdu = cy_find_ipdu_elecMon(nsp,ipdu,element);
					newname[*length - 2] = ipdu;//****CHK
					elemn = newname[*length - 1] = name[*length-1] = 1;//****CHK
					// get number of pdus in this port
					if ( ipdu_in_pm_range_elecMon(nsp,ipdu,element) ) {
						//find next port and ipdu with env;
						nsp = cy_find_nsp_elecMon(nsp,element);
						ipdu = cy_find_ipdu_elecMon(nsp,0,element);
						newname[*length - 3] = nsp;
						newname[*length - 2] = ipdu;
					}
				} else {
					nsp = newname[*length - 3] = name[*length - 3];
					ipdu = newname[*length - 2] = name[*length - 2];
					elemn = newname[*length - 1] = name[*length-1] = 1;
				}
			} else {
				*length = vp->namelen + 3;
				memmove(newname,name, (*length) * sizeof(oid));
				if (!exact) {
					if (name[*length - 1] < ULONG_MAX) {
						nsp = name[*length - 3];
						ipdu = cy_find_ipdu_elecMon(nsp,ipdu,element);
						newname[*length - 2] = ipdu;
						elemn = newname[*length - 1] = name[*length-1] = 1;
						// get number of pdus in this port
						if (ipdu_in_pm_range_elecMon(nsp,ipdu,element) ) {
							//nsp = find next port and ipdu with ipdu with env;
							nsp = cy_find_nsp_elecMon(nsp,element);
							ipdu = cy_find_ipdu_elecMon(nsp,0,element);
							newname[*length - 3] = nsp;
							newname[*length - 2] = ipdu;
							elemn = newname[*length - 1] = name[*length-1] = 1;
						}
					} else {
						nsp = newname[*length - 3] = name[*length - 3];
						ipdu = newname[*length - 2] = name[*length - 2];
						elemn = newname[*length - 1] = name[*length-1] = 1;
					}
				}
			}
		}
	}
	if ((nsp == 0) || (ipdu_in_pm_range_elecMon(nsp,ipdu,element)) || elecmon_in_pm_range(nsp,ipdu,elemn,element)) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memmove(name, newname, (*length) * sizeof(oid));
	// default values
	if (write_method) *write_method = 0;
	if (var_len) *var_len = sizeof(long);
	
	return MATCH_SUCCEEDED;
}		

/*
 * var_cyPMElecPhaseTable():
 */
unsigned char *var_cyPMElecPhaseTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp,cy_pm,cy_elem;
	phaseInfo *phase;
	ipduInfo *ipduinfo;
	//int sensortype = 0;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_PMElecElementTable(vp, name, length, exact, var_len, write_method, ELECPHASE) == MATCH_FAILED) {
		return NULL;
	}

	memset(string, 0, 1024);

	cy_nsp = name[CYPMElecPhase_PN_INDEX];
	cy_pm = name[CYPMElecPhase_PDUN_INDEX];
	cy_elem = name[CYPMElecPhase_TABLE_INDEX];

	/* Check PMD serial port and IPDU number */
	ipduinfo = pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");

	if ( ipduinfo == NULL ) {
                return NULL;
        }
        phase = &ipduinfo->phase[cy_elem - 1];

	if ( phase == NULL ) {
		return NULL;
	}
	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYPMElecPhaseTablePortNumber :
			long_ret = phase->ipdu->pmInfo->number;
			return (unsigned char *) &long_ret;	
			
		case CYPMElecPhaseTablePduNumber :
			long_ret = phase->ipdu->number;
			return (unsigned char *) &long_ret;
			
		case CYPMElecPhaseTableIndex :
			long_ret = phase->number;
			return (unsigned char *) &long_ret;

		case CYPMElecPhaseTableName :
			strncpy(string,phase->sysname,strlen(phase->sysname));
                        *var_len = strlen(string);
                        return (unsigned char *) &string;

		case CYPMElecPhaseTableCurrentValue : 
			long_ret = phase->elecMon.current;
			return (unsigned char *) &long_ret;

		case CYPMElecPhaseTableCurrentMax :
			if ( phase->elecMon.sflag & ELECMON_CAP_MAX_CURRENT ){
                        	long_ret = phase->elecMon.max_current;
			}
			else {
				long_ret = 0;
			}
			*write_method = write_cyPMElecPhaseTable_CurrValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecPhaseTablePowerValue : 
			long_ret = phase->elecMon.power;
			return (unsigned char *) &long_ret;

		case CYPMElecPhaseTablePowerMax :
			if ( phase->elecMon.sflag & ELECMON_CAP_MAX_POWER ){
                                long_ret = phase->elecMon.max_power;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecPhaseTable_PowValRst;
			return (unsigned char *) &long_ret;
		
		case CYPMElecPhaseTableVoltageValue : 
			long_ret = phase->elecMon.voltage;
			return (unsigned char *) &long_ret;

		case CYPMElecPhaseTableVoltageMax :
			if ( phase->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE ){
	                        long_ret = phase->elecMon.max_voltage;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecPhaseTable_VoltValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecPhaseTablePowerFactorValue : 
			long_ret = phase->elecMon.power_fac;
			return (unsigned char *) &long_ret;

		case CYPMElecPhaseTablePowerFactorMax :
			if ( phase->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR ){
                        	long_ret = phase->elecMon.max_power_fac;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecPhaseTable_PowfacValRst;
			return (unsigned char *) &long_ret;
	
		case CYPMElecPhaseTablePowerType :
                        if ( phase->elecMon.power_type == MEASURED ){
                                long_ret = 2;
                        }
                        else{
                                long_ret = 1;
                        }
                        return (unsigned char *) &long_ret;

		case CYPMElecPhaseTableVoltageType :
			if ( phase->elecMon.voltage_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;

		case CYPMElecPhaseTablePowerFactorType :
			if ( phase->elecMon.power_fac_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;	

		default:
			break;
	}
	return NULL;
}

int write_cyPMElecPhaseTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        phaseInfo *phase;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPhase_PN_INDEX];
        cy_pm = name[CYPMElecPhase_PDUN_INDEX];
        cy_elem = name[CYPMElecPhase_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        phase = &ipduinfo->phase[cy_elem - 1];
/*
	if ( !(phase->elecMon.sflag & ELECMON_CAP_MAX_CURRENT) ){
		return SNMP_ERR_GENERR;
	}
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(phase, ELEMENT_PHASE, ELECMON_CAP_CURRENT, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecPhaseTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        phaseInfo *phase;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPhase_PN_INDEX];
        cy_pm = name[CYPMElecPhase_PDUN_INDEX];
        cy_elem = name[CYPMElecPhase_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        phase = &ipduinfo->phase[cy_elem - 1];
/*
        if ( !(phase->elecMon.sflag & ELECMON_CAP_MAX_POWER) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(phase, ELEMENT_PHASE, ELECMON_CAP_POWER, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecPhaseTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        phaseInfo *phase;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPhase_PN_INDEX];
        cy_pm = name[CYPMElecPhase_PDUN_INDEX];
        cy_elem = name[CYPMElecPhase_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        phase = &ipduinfo->phase[cy_elem - 1];
/*
        if ( !(phase->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(phase, ELEMENT_PHASE, ELECMON_CAP_VOLTAGE, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecPhaseTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        phaseInfo *phase;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecPhase_PN_INDEX];
        cy_pm = name[CYPMElecPhase_PDUN_INDEX];
        cy_elem = name[CYPMElecPhase_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        phase = &ipduinfo->phase[cy_elem - 1];
/*
        if ( !(phase->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(phase, ELEMENT_PHASE, ELECMON_CAP_POWER_FACTOR, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

/*
 * var_cyPMElecBankTable():
 */
unsigned char *var_cyPMElecBankTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp,cy_pm,cy_elem;
	circuitInfo *circuit;
	ipduInfo *ipduinfo;
	//int sensortype = 0;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_PMElecElementTable(vp, name, length, exact, var_len, write_method, ELECBANK) == MATCH_FAILED) {
		return NULL;
	}

	memset(string, 0, 1024);

	cy_nsp = name[CYPMElecBank_PN_INDEX];
	cy_pm = name[CYPMElecBank_PDUN_INDEX];
	cy_elem = name[CYPMElecBank_TABLE_INDEX];

	/* Check PMD serial port and IPDU number */
	ipduinfo = pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");

	if ( ipduinfo == NULL ) {
                return NULL;
        }
        circuit = &ipduinfo->circuit[cy_elem - 1];

	if ( circuit == NULL ) {
		return NULL;
	}
	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYPMElecBankTablePortNumber :
			long_ret = circuit->ipdu->pmInfo->number;
			return (unsigned char *) &long_ret;	
			
		case CYPMElecBankTablePduNumber :
			long_ret = circuit->ipdu->number;
			return (unsigned char *) &long_ret;
			
		case CYPMElecBankTableIndex :
			long_ret = circuit->number;
			return (unsigned char *) &long_ret;

		case CYPMElecBankTableName :
			strncpy(string,circuit->sysname,strlen(circuit->sysname));
                        *var_len = strlen(string);
                        return (unsigned char *) &string;

		case CYPMElecBankTableCurrentValue : 
			long_ret = circuit->elecMon.current;
			return (unsigned char *) &long_ret;

		case CYPMElecBankTableCurrentMax :
			if ( circuit->elecMon.sflag & ELECMON_CAP_MAX_CURRENT ){
                        	long_ret = circuit->elecMon.max_current;
			}
			else {
				long_ret = 0;
			}
			*write_method = write_cyPMElecBankTable_CurrValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecBankTablePowerValue : 
			long_ret = circuit->elecMon.power;
			return (unsigned char *) &long_ret;

		case CYPMElecBankTablePowerMax :
			if ( circuit->elecMon.sflag & ELECMON_CAP_MAX_POWER ){
                                long_ret = circuit->elecMon.max_power;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecBankTable_PowValRst;
			return (unsigned char *) &long_ret;
		
		case CYPMElecBankTableVoltageValue : 
			long_ret = circuit->elecMon.voltage;
			return (unsigned char *) &long_ret;

		case CYPMElecBankTableVoltageMax :
			if ( circuit->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE ){
	                        long_ret = circuit->elecMon.max_voltage;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecBankTable_VoltValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecBankTablePowerFactorValue : 
			long_ret = circuit->elecMon.power_fac;
			return (unsigned char *) &long_ret;

		case CYPMElecBankTablePowerFactorMax :
			if ( circuit->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR ){
                        	long_ret = circuit->elecMon.max_power_fac;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecBankTable_PowfacValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecBankTablePowerType :
                        if ( circuit->elecMon.power_type == MEASURED ){
                                long_ret = 2;
                        }
                        else{
                                long_ret = 1;
                        }
                        return (unsigned char *) &long_ret;

		case CYPMElecBankTableVoltageType :
			if ( circuit->elecMon.voltage_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;

		case CYPMElecBankTablePowerFactorType :
			if ( circuit->elecMon.power_fac_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;

		default:
			break;
	}
	return NULL;
}

int write_cyPMElecBankTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        circuitInfo *circuit;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecBank_PN_INDEX];
        cy_pm = name[CYPMElecBank_PDUN_INDEX];
        cy_elem = name[CYPMElecBank_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        circuit = &ipduinfo->circuit[cy_elem - 1];
/*
	if ( !(circuit->elecMon.sflag & ELECMON_CAP_MAX_CURRENT) ){
		return SNMP_ERR_GENERR;
	}
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(circuit, ELEMENT_CIRCUIT, ELECMON_CAP_CURRENT, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecBankTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        circuitInfo *circuit;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecBank_PN_INDEX];
        cy_pm = name[CYPMElecBank_PDUN_INDEX];
        cy_elem = name[CYPMElecBank_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        circuit = &ipduinfo->circuit[cy_elem - 1];
/*
        if ( !(circuit->elecMon.sflag & ELECMON_CAP_MAX_POWER) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(circuit, ELEMENT_CIRCUIT, ELECMON_CAP_POWER, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecBankTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        circuitInfo *circuit;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecBank_PN_INDEX];
        cy_pm = name[CYPMElecBank_PDUN_INDEX];
        cy_elem = name[CYPMElecBank_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        circuit = &ipduinfo->circuit[cy_elem - 1];
/*
        if ( !(circuit->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(circuit, ELEMENT_CIRCUIT, ELECMON_CAP_VOLTAGE, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecBankTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        circuitInfo *circuit;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecBank_PN_INDEX];
        cy_pm = name[CYPMElecBank_PDUN_INDEX];
        cy_elem = name[CYPMElecBank_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        circuit = &ipduinfo->circuit[cy_elem - 1];
/*
        if ( !(circuit->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_elecmon_reset(circuit, ELEMENT_CIRCUIT, ELECMON_CAP_POWER_FACTOR, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

/*
 * var_cyPMElecOutletTable():
 */
unsigned char *var_cyPMElecOutletTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp,cy_pm,cy_elem;
	oData *outlet;
	ipduInfo *ipduinfo;
	//int sensortype = 0;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_PMElecElementTable(vp, name, length, exact, var_len, write_method, ELECOUTLET) == MATCH_FAILED) {
		return NULL;
	}

	memset(string, 0, 1024);

	cy_nsp = name[CYPMElecOutlet_PN_INDEX];
	cy_pm = name[CYPMElecOutlet_PDUN_INDEX];
	cy_elem = name[CYPMElecOutlet_TABLE_INDEX];

	/* Check PMD serial port and IPDU number */
	ipduinfo = pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");

	if ( ipduinfo == NULL ) {
                return NULL;
        }
        outlet = &ipduinfo->outlet[cy_elem - 1];

	if ( outlet == NULL ) {
		return NULL;
	}
	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYPMElecOutletTablePortNumber :
			long_ret = outlet->ipdu->pmInfo->number;
			return (unsigned char *) &long_ret;	
			
		case CYPMElecOutletTablePduNumber :
			long_ret = outlet->ipdu->number;
			return (unsigned char *) &long_ret;
			
		case CYPMElecOutletTableNumber :
			long_ret = outlet->onumber;
			return (unsigned char *) &long_ret;

		case CYPMElecOutletTableName :
			strncpy(string,outlet->name,strlen(outlet->name));
                        *var_len = strlen(string);
                        return (unsigned char *) &string;

		case CYPMElecOutletTableCurrentValue : 
			long_ret = outlet->elecMon.current;
			return (unsigned char *) &long_ret;

		case CYPMElecOutletTableCurrentMax :
			if ( outlet->elecMon.sflag & ELECMON_CAP_MAX_CURRENT ){
                        	long_ret = outlet->elecMon.max_current;
			}
			else {
				long_ret = 0;
			}
			*write_method = write_cyPMElecOutletTable_CurrValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecOutletTablePowerValue : 
			long_ret = outlet->elecMon.power;
			return (unsigned char *) &long_ret;

		case CYPMElecOutletTablePowerMax :
			if ( outlet->elecMon.sflag & ELECMON_CAP_MAX_POWER ){
                                long_ret = outlet->elecMon.max_power;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecOutletTable_PowValRst;
			return (unsigned char *) &long_ret;
		
		case CYPMElecOutletTableVoltageValue : 
			long_ret = outlet->elecMon.voltage;
			return (unsigned char *) &long_ret;

		case CYPMElecOutletTableVoltageMax :
			if ( outlet->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE ){
	                        long_ret = outlet->elecMon.max_voltage;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecOutletTable_VoltValRst;
			return (unsigned char *) &long_ret;
			
		case CYPMElecOutletTablePowerFactorValue : 
			long_ret = outlet->elecMon.power_fac;
			return (unsigned char *) &long_ret;

		case CYPMElecOutletTablePowerFactorMax :
			if ( outlet->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR ){
                        	long_ret = outlet->elecMon.max_power_fac;
			}
			else {
                                long_ret = 0;
                        }
			*write_method = write_cyPMElecOutletTable_PowfacValRst;
			return (unsigned char *) &long_ret;
	
		case CYPMElecOutletTablePowerType :
                        if ( outlet->elecMon.power_type == MEASURED ){
                                long_ret = 2;
                        }
                        else{
                                long_ret = 1;
                        }
                        return (unsigned char *) &long_ret;

		case CYPMElecOutletTableVoltageType :
			if ( outlet->elecMon.voltage_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;

		case CYPMElecOutletTablePowerFactorType :
			if ( outlet->elecMon.power_fac_type == MEASURED ){
				long_ret = 2;
			}
			else{
				long_ret = 1;
			}
			return (unsigned char *) &long_ret;

		default:
			break;
	}
	return NULL;
}

int write_cyPMElecOutletTable_CurrValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        oData *outlet;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecOutlet_PN_INDEX];
        cy_pm = name[CYPMElecOutlet_PDUN_INDEX];
        cy_elem = name[CYPMElecOutlet_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        outlet = &ipduinfo->outlet[cy_elem - 1];
/*
	if ( !(outlet->elecMon.sflag & ELECMON_CAP_MAX_CURRENT) ){
		return SNMP_ERR_GENERR;
	}
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_outlet_reset(SINGLE_OUTLET, outlet, ELECMON_CAP_CURRENT, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecOutletTable_PowValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        oData *outlet;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecOutlet_PN_INDEX];
        cy_pm = name[CYPMElecOutlet_PDUN_INDEX];
        cy_elem = name[CYPMElecOutlet_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        outlet = &ipduinfo->outlet[cy_elem - 1];
/*
        if ( !(outlet->elecMon.sflag & ELECMON_CAP_MAX_POWER) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_outlet_reset(SINGLE_OUTLET, outlet, ELECMON_CAP_POWER, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecOutletTable_VoltValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        oData *outlet;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecOutlet_PN_INDEX];
        cy_pm = name[CYPMElecOutlet_PDUN_INDEX];
        cy_elem = name[CYPMElecOutlet_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        outlet = &ipduinfo->outlet[cy_elem - 1];
/*
        if ( !(outlet->elecMon.sflag & ELECMON_CAP_MAX_VOLTAGE) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_outlet_reset(SINGLE_OUTLET, outlet, ELECMON_CAP_VOLTAGE, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

int write_cyPMElecOutletTable_PowfacValRst(int action, u_char * var_val, u_char var_val_type,
           size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
        int cy_nsp, cy_pm, cy_elem;
        oData *outlet;
        ipduInfo *ipduinfo;

        cy_nsp = name[CYPMElecOutlet_PN_INDEX];
        cy_pm = name[CYPMElecOutlet_PDUN_INDEX];
        cy_elem = name[CYPMElecOutlet_TABLE_INDEX];
        ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");
        outlet = &ipduinfo->outlet[cy_elem - 1];
/*
        if ( !(outlet->elecMon.sflag & ELECMON_CAP_MAX_POWER_FACTOR) ){
                return SNMP_ERR_GENERR;
        }
*/
        switch (action) {
                case MODE_SET_RESERVE1:
                        if (var_val_type != ASN_INTEGER) {
                                return SNMP_ERR_WRONGTYPE;
                        }
                        if (var_val_len > sizeof(long)) {
                                return SNMP_ERR_WRONGLENGTH;
                        }
                        if (*(long*)var_val) {
                                return SNMP_ERR_WRONGVALUE;
                        }
                        break;

                case MODE_SET_ACTION:
                        pmc_sendcmd_outlet_reset(SINGLE_OUTLET, outlet, ELECMON_CAP_POWER_FACTOR, NULL, NULL);
                        break;

                case MODE_SET_RESERVE2:
                case MODE_SET_COMMIT:
                case MODE_SET_FREE:
                case MODE_SET_UNDO:
                        break;

        }
        return SNMP_ERR_NOERROR;
}

/*
 *Grouping of sensortype according to snmp
 */
int EnvMonSensorType(unsigned char sensortype)
{
	switch (sensortype) {
		case ENVMON_SENSOR_AVOCENT_TEMP_INTERNAL :
			return 1;
		case ENVMON_SENSOR_AVOCENT_TEMP_EXTERNAL :
		case ENVMON_SENSOR_TEMPERATURE :
			return 2;
		case ENVMON_SENSOR_AVOCENT_HUM_EXTERNAL :
		case ENVMON_SENSOR_HUMIDITY :
			return 3;
		case ENVMON_SENSOR_AVOCENT_AIR_FLOW :
			return 4;
		case ENVMON_SENSOR_AVOCENT_SMOKE :
			return 5;
		case ENVMON_SENSOR_AVOCENT_DRY_CONTACT :
		case ENVMON_SENSOR_AVOCENT_DRY_CONTACT_OPEN :
		case ENVMON_SENSOR_AVOCENT_DRY_CONTACT_CLOSED :
		case ENVMON_SENSOR_DRY_CONTACT :
			return 6;
		case ENVMON_SENSOR_AVOCENT_WATER_LEVEL :
		case ENVMON_SENSOR_WATER_LEVEL :
			return 7;
		case ENVMON_SENSOR_AVOCENT_MOTION :
			return 8;
		case ENVMON_SENSOR_UNDEFINED :
			return 9;
		default :
			return 10;
	}
}

/* Compare 'name' to vp->name for the best match or an exact match
 * Also check that 'name' is valid PM port serial number.
 * Store a successful match in 'name', and increment the OID instance if the
 * match was not exact.
 */
static int header_PMEnvMonTable(struct variable *vp, oid * name, size_t * length,
		int exact, size_t * var_len, WriteMethod ** write_method)
{
	int i, rtest,nsp,ipdu,envn;
	oid newname[MAX_OID_LEN];

	for (i=0, rtest=0; (i < (int)vp->namelen) && (i < (int)(*length)) &&
		 !rtest; i++) {
		if (name[i] != vp->name[i]) {
			if (name[i] < vp->name[i])
				rtest = -1;
			else
				rtest = 1;
		}
	}
	// find the first serial port with PM which has a envMon connected
	nsp = cy_find_nsp_envMon(0);
	// find the first PM on given serial port which has a envMon connected
	ipdu = cy_find_ipdu_envMon(nsp,0);

	envn = 1;

	if ((rtest > 0) || 
		((exact == 1) && (rtest || ((int)*length != (int)(vp->namelen+3))))) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memset(newname, 0, sizeof(newname));

	if (((int)*length <= (int)vp->namelen) || (rtest == -1)) {
		memmove(newname, vp->name,(int)vp->namelen * sizeof(oid));
		newname[vp->namelen] = nsp;
		newname[vp->namelen + 1] = ipdu;
		newname[vp->namelen + 2] = 1;
		*length = vp->namelen + 3;
	} else {
		// check outlet number
		if ((int)*length >= ((int)vp->namelen+3)) {
			*length = vp->namelen + 3;
			memmove(newname, name, (int)*length * sizeof(oid));
			if (name[*length - 2] < ULONG_MAX) {
				nsp = name[*length - 3];
				ipdu = newname[*length - 2] = name[*length-2];
				if (exact) {
					envn = newname[*length - 1] = name[*length-1];
				} else {
					envn = newname[*length - 1] = name[*length-1] + 1;
					// get number of outs in this port
					if (envmon_in_pm_range(nsp,ipdu,envn)) {
						//ipdu = find next ipdu on this nsp with env; 
						ipdu = cy_find_ipdu_envMon(nsp,ipdu);
						newname[*length - 2] = ipdu;
						envn = newname[*length - 1] = 1;
						// get number of pdus in this port 
						if ( ipdu_in_pm_range_envMon(nsp,ipdu) ) {
							//nsp = find next port and ipdu with ipdu with env;
							nsp = cy_find_nsp_envMon(nsp);
							ipdu = cy_find_ipdu_envMon(nsp,0);    
							newname[*length - 3] = nsp;
							newname[*length - 2] = ipdu;
						}
					}
				}
			} else {
				nsp = newname[*length - 3] = name[*length - 3];
				ipdu = newname[*length - 2] = name[*length - 2];
				envn = newname[*length - 1] = name[*length - 1];
			}
		} else {
			// pdu index
			if ((int)*length > ((int)vp->namelen+2)) {
				*length = vp->namelen + 3;
				memmove(newname, name, (int)*length * sizeof(oid));
				if (name[*length - 2] < ULONG_MAX) {
					nsp = name[*length - 3];
					ipdu = cy_find_ipdu_envMon(nsp,ipdu);
					newname[*length - 2] = ipdu;//****CHK
					envn = newname[*length - 1] = name[*length-1] = 1;//****CHK
					// get number of pdus in this port
					if ( ipdu_in_pm_range_envMon(nsp,ipdu) ) {
						//find next port and ipdu with env;
						nsp = cy_find_nsp_envMon(nsp);
						ipdu = cy_find_ipdu_envMon(nsp,0);
						newname[*length - 3] = nsp;
						newname[*length - 2] = ipdu;
					}
				} else {
					nsp = newname[*length - 3] = name[*length - 3];
					ipdu = newname[*length - 2] = name[*length - 2];
					envn = newname[*length - 1] = name[*length-1] = 1;
				}
			} else {
				*length = vp->namelen + 3;
				memmove(newname,name, (*length) * sizeof(oid));
				if (!exact) {
					if (name[*length - 1] < ULONG_MAX) {
						nsp = name[*length - 3];
						ipdu = cy_find_ipdu_envMon(nsp,ipdu);
						newname[*length - 2] = ipdu;
						envn = newname[*length - 1] = name[*length-1] = 1;
						// get number of pdus in this port
						if (ipdu_in_pm_range_envMon(nsp,ipdu) ) {
							//nsp = find next port and ipdu with ipdu with env;
							nsp = cy_find_nsp_envMon(nsp);
							ipdu = cy_find_ipdu_envMon(nsp,0);
							newname[*length - 3] = nsp;
							newname[*length - 2] = ipdu;
							envn = newname[*length - 1] = name[*length-1] = 1;
						}
					} else {
						nsp = newname[*length - 3] = name[*length - 3];
						ipdu = newname[*length - 2] = name[*length - 2];
						envn = newname[*length - 1] = name[*length-1] = 1;
					}
				}
			}
		}
	}
	if ((nsp == 0) || (ipdu_in_pm_range_envMon(nsp,ipdu)) || envmon_in_pm_range(nsp,ipdu,envn)) {
		if (var_len)
			*var_len = 0;
		return MATCH_FAILED;
	}

	memmove(name, newname, (*length) * sizeof(oid));
	// default values
	if (write_method) *write_method = 0;
	if (var_len) *var_len = sizeof(long);
	
	return MATCH_SUCCEEDED;
}		

/*
 * var_cyPMEnvMonTable():
 */
unsigned char *var_cyPMEnvMonTable(struct variable *vp,
				 oid * name,
				 size_t * length,
				 int exact,
				 size_t * var_len,
				 WriteMethod ** write_method)
{
	/*
	 * variables we may use later 
	 */
	static long long_ret;
	//static int size;
	static unsigned char string[1024];
	int cy_nsp,cy_pm,cy_envn;
	envMonInfo *envMon;
	ipduInfo *ipduinfo;
	int sensortype = 0;

	if ( pmc_pmd_status() == -1 ) { // PMD is not OK
		return NULL;
	}

	// verify the PM serial port index
	if (header_PMEnvMonTable(vp, name, length, exact, var_len, write_method) == MATCH_FAILED) {
		return NULL;
	}

	memset(string, 0, 1024);

	cy_nsp = name[CYPMEnvMon_PN_INDEX];
	cy_pm = name[CYPMEnvMon_PDUN_INDEX];
	cy_envn = name[CYPMEnvMon_TABLE_INDEX];

	/* Check PMD serial port and IPDU number */
	ipduinfo = pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");

	if ( ipduinfo == NULL ) {
                return NULL;
        }
        envMon = &ipduinfo->envMon[cy_envn - 1];

	if ( envMon == NULL ) {
		return NULL;	/* It is not an EnvMon sensor */
	}
	/*
	 * * this is where we do the value assignments for the mib results.
	 */
	switch (vp->magic) {
		case CYPMEnvMonTablePortNumber :
			long_ret = envMon->ipdu->pmInfo->number;
			return (unsigned char *) &long_ret;	
			
		case CYPMEnvMonTablePduNumber :
			long_ret = envMon->ipdu->number;
			return (unsigned char *) &long_ret;
			
		case CYPMEnvMonTableIndex :
			long_ret = envMon->number;
			return (unsigned char *) &long_ret;

		case CYPMEnvMonTableName :
			strncpy(string,envMon->sysname,strlen(envMon->sysname));
                        *var_len = strlen(string);
                        return (unsigned char *) &string;

		case CYPMEnvMonTableType : 
			long_ret = EnvMonSensorType(envMon->sensor_type);
			return (unsigned char *) &long_ret;

		case CYPMEnvMonTableState :
			//logic to check the sensor type then return value.
			sensortype = EnvMonSensorType(envMon->sensor_type);
			if((sensortype >= 5)&&(sensortype <= 8)){
				long_ret = envMon->value;
			}	
			else{	
				long_ret = 3;
			}
			return (unsigned char *) &long_ret;

		case CYPMEnvMonTableValue :
			sensortype = EnvMonSensorType(envMon->sensor_type);
			if((sensortype == 1)||(sensortype == 2)){
				long_ret = envMon->value - 2731.5 ; //Kelvin to C*10
			}
			else{
				long_ret = envMon->value;	
			}
			return (unsigned char *) &long_ret;

		case CYPMEnvMonTableMaxValue :
			sensortype = EnvMonSensorType(envMon->sensor_type);
			if((sensortype == 1)||(sensortype == 2)){
                        	long_ret = envMon->max_value - 2731.5 ; //Kelvin to C*10
                        }
			else{
				long_ret = envMon->max_value;
			}
			*write_method = write_cyPMEnvMonTable_ValRst;
			return (unsigned char *) &long_ret;

		default:
			break;
	}
	return NULL;
}

int write_cyPMEnvMonTable_ValRst(int action, u_char * var_val, u_char var_val_type,
	   size_t var_val_len, u_char * statP, oid * name, size_t name_len)
{
	int cy_nsp, cy_pm, cy_envn;
	envMonInfo *envmon;
	ipduInfo *ipduinfo;

	cy_nsp = name[CYPMEnvMon_PN_INDEX];
        cy_pm = name[CYPMEnvMon_PDUN_INDEX];
        cy_envn = name[CYPMEnvMon_TABLE_INDEX];
	ipduinfo= pmc_validate_ipdu_old(cy_nsp,cy_pm,"ttyS");	
	envmon = &ipduinfo->envMon[cy_envn - 1];
	switch (action) {
		case MODE_SET_RESERVE1:
			if (var_val_type != ASN_INTEGER) {
				return SNMP_ERR_WRONGTYPE;
			}
			if (var_val_len > sizeof(long)) {
				return SNMP_ERR_WRONGLENGTH;
			}
			if (*(long*)var_val) {
				return SNMP_ERR_WRONGVALUE;
			}
			break;

		case MODE_SET_ACTION:
			pmc_sendcmd_envmon_reset(envmon, NULL, NULL);
			break;

		case MODE_SET_RESERVE2:
		case MODE_SET_COMMIT:
		case MODE_SET_FREE:
		case MODE_SET_UNDO:
			break;

	}
	return SNMP_ERR_NOERROR;
}


