#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pmclib.h>

#define exec_func(function, args...) \
	if (function(args)) printf("  SUCCESS\n"); else printf("  ERROR ON COMMAND\n");

/* static void process_response(result_t *res) { */
/* 	result_t *aux_res = res, *param_res; */
/* 	if (res) { */
/* 		printf("  Response: <%s>.%d ", res->text, res->type); */
/* 		for (param_res = res->next_param; param_res; param_res = param_res->next_param) { */
/* 			printf("[%s].%d ", param_res->text, param_res->type); */
/* 		} */
/* 		printf("\n"); */
/* 		while ((res = res->next_label)) { */
/* 			printf("            <%s>.%d ", res->text, res->type); */
/* 			for (param_res = res->next_param; param_res; param_res = param_res->next_param) { */
/* 				printf("[%s].%d ", param_res->text, param_res->type); */
/* 			} */
/* 			printf("\n"); */
/* 		} */
/* 		power_mk_result_free(&aux_res); */
/* 	} */
/* } */

void myprintf(char *text) { printf("%s\n", text); }

int main () {
	pmInfo *pminfo;
	ipduInfo *ipdu;
//	result_t *res = NULL;
	int option = 0, pdus;
	char menu[] = "What do you want to do?\n"
		"  0. Show PDUs\n"
		"  1. Show 1st outlet name\n"
		"  2. Change 1st outlet name\n"
		"  3. Turn 1st outlet On\n"
		"  4. Turn 1st outlet Off\n"
		"  5. Cycle 1st outlet\n"
		"  6. Test #6\n"
		"  7. Test #7\n"
		"  8. Test #8\n"
		"  9. Test #9\n"
		"  Q. Quit\n"
		"Option: ";
	char sinput[100];

	if (pmc_shm_init()) {
		printf("Aborting: problems attaching SHM.\n");
		return -1;
	}
//	process_response(res);

	if (!shmp) printf("waiting for a good Shared Memory pointer...\n");
	while (!shmp) sleep(1);

#ifdef DEBUG
	debugLevel = PMCLIB_SEND;
#endif

	printf("Detected PDUs:\n");
show_pdus:
	for (pdus=0, pminfo=shmp->pmInfo; pminfo; pminfo=pminfo->next) {
		for (ipdu=pminfo->ipdu; ipdu; ipdu=ipdu->next) {
			if (ipdu->ready) {
				printf("  %s\n", ipdu->id);
				pdus++;
			}
		}
	}
	if (!pdus) printf("  not a single PDU !!!\n");

	while ((option != 'q') && (option != 'Q')) {
		printf(menu);
		if ((option = getchar()) == '\n') continue;
		if (getchar() != '\n') {
			printf("error reading option\n");
			return -1;
		}
		switch (option) {
		case '0':
			goto show_pdus;
		case '1': //Show 1st outlet name
			printf("  <%s>\n", shmp->pmInfo->ipdu->outlet->name);
			break;
		case '2': //Change 1st outlet name
			printf("  new outlet name: ");
			if (!fgets(sinput, sizeof(sinput), stdin)) {
				printf("error reading input\n");
				return -1;
			}
			sinput[strlen(sinput) - 1] = '\0';
			exec_func(pmc_sendcmd_outcfg_name, shmp->pmInfo->ipdu->outlet, sinput, NULL, myprintf);
			break;
		case '3': //Turn 1st outlet On
/* 			exec_func(API_admin_power_port_outlet_on, 1, 1, 1, NULL, &res); */
/* 			process_response(res); */
			break;
		case '4': //Turn 1st outlet Off
/* 			exec_func(API_admin_power_port_outlet_off, 1, 1, 1, NULL, &res); */
/* 			process_response(res); */
			break;
		case '5': //Cycle 1st outlet
/* 			exec_func(API_admin_power_port_outlet_cycle, 1, 1, 1, NULL, &res); */
/* 			process_response(res); */
			break;
		case '6': //Test #6
		{
			exec_func(pmc_sendcmd_ipdu_reset_old, &shmp->pmInfo->ipdu, 1, PROTO_ELEMENT_ELECMON_CURRENT, NULL, myprintf);
		}
		break;
		case '7': //Test #7
		{
			exec_func(pmc_sendcmd_ipdu_reset_old, &shmp->pmInfo->ipdu, 1, ENVMON_SENSOR_TEMPERATURE, NULL, myprintf);
		}
		break;
		case '8': //Test #8
		{
			ipduInfo *ilist[3] = { shmp->pmInfo->ipdu,
								   shmp->pmInfo->ipdu->next, NULL };
			exec_func(pmc_sendcmd_ipdu_reset_old, ilist, 2, PROTO_ELEMENT_ELECMON_VOLTAGE, NULL, myprintf);
			exec_func(pmc_sendcmd_ipdu_reset_old, ilist, 2, ENVMON_SENSOR_HUMIDITY, NULL, myprintf);
		}
		break;
		case '9': //Test #9
		{
			pmc_show_ipdu_voltage(shmp->pmInfo->ipdu);
			printf("\n");
		}
		break;
		default:
			if ((option != 'q') && (option != 'Q'))
				printf("option not available\n");
		}
	}

#ifdef DEBUG
	debugLevel = 0;
#endif

	return 0;
}

#if 0 //Tests performed...
----------------
			{
				ipduInfo *ilist[3] = { shmp->pmInfo->ipdu,
									   shmp->pmInfo->ipdu->next, NULL };
				exec_func(pmc_sendcmd_ipdu_reset_old, ilist, 2, PROTO_ELEMENT_ELECMON_CURRENT, NULL, myprintf);
				exec_func(pmc_sendcmd_ipdu_reset_old, ilist, 2, ENVMON_SENSOR_TEMPERATURE, NULL, myprintf);
			}
			exec_func(pmc_sendcmd_ipdu_reset_old, ilist, 2, PROTO_ELEMENT_ELECMON_VOLTAGE, NULL, myprintf);
			exec_func(pmc_sendcmd_ipdu_reset_old, ilist, 2, ENVMON_SENSOR_HUMIDITY, NULL, myprintf);
			pmc_show_ipdu_current(shmp->pmInfo->ipdu);
			printf("\n");
			pmc_show_ipdu_voltage(shmp->pmInfo->ipdu);
			printf("\n");
			pmc_show_ipdu_temperature(shmp->pmInfo->ipdu);
			printf("\n");
			pmc_show_ipdu_humidity(shmp->pmInfo->ipdu);
----------------
			exec_func(pmc_sendcmd_outcfg_interval, 1, SINGLE_OUTLET, shmp->pmInfo->ipdu->outlet, OUTLET_CAP_POSTPOWERON, NULL, myprintf);
			exec_func(pmc_sendcmd_outcfg_interval, 1, OUTLET_LIST, olist, OUTLET_CAP_POSTPOWERON, NULL, myprintf);
			exec_func(pmc_sendcmd_outcfg_interval, 1, OUTLIST_CHAIN, shmp->pmInfo, OUTLET_CAP_POSTPOWERON, NULL, myprintf);
			pmc_show_outlet_delay(0, shmp->pmInfo->ipdu->outlet, OUTLET_CAP_POSTPOWERON, NULL);
			pmc_show_outlet_delay(0, shmp->pmInfo->ipdu->outlet+1, OUTLET_CAP_POSTPOWERON, NULL);
			pmc_show_outlet_delay(0, shmp->pmInfo->ipdu->outlet+2, OUTLET_CAP_POSTPOWERON, NULL);
----------------
			exec_func(pmc_sendcmd_outcfg_wakeup, OUTLIST_IPDU, shmp->pmInfo->ipdu, WAKEUP_STATE_ON, NULL, myprintf);
			{
				oData *olist[4] = { shmp->pmInfo->ipdu->outlet + 1,
									shmp->pmInfo->ipdu->outlet + 2, NULL };
				exec_func(pmc_sendcmd_outcfg_wakeup, OUTLET_LIST, olist, WAKEUP_STATE_ON, NULL, myprintf);
			}
			exec_func(pmc_sendcmd_outcfg_wakeup, OUTLIST_CHAIN, shmp->pmInfo, WAKEUP_STATE_ON, NULL, myprintf);
----------------
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_ON, SINGLE_OUTLET, shmp->pmInfo->ipdu->outlet, NULL, myprintf);
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_OFF, SINGLE_OUTLET, shmp->pmInfo->ipdu->outlet, NULL, myprintf);
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_OFF, OUTLET_LIST, olist, NULL, myprintf);
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_CYCLE, SINGLE_OUTLET, shmp->pmInfo->ipdu->outlet, NULL, myprintf);
-----
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_ON, OUTLIST_IPDU, shmp->pmInfo->ipdu->next, NULL, myprintf);
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_OFF, OUTLIST_IPDU, shmp->pmInfo->ipdu->next, NULL, myprintf);
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_ON, OUTLIST_CHAIN, shmp->pmInfo, NULL, myprintf);
			exec_func(pmc_sendcmd_outlet, OUTLET_CONTROL_OFF, OUTLIST_ALL, NULL, NULL, myprintf);
----------------
			exec_func(API_admin_power_port_outlet_cycle, 1, 1, 1, NULL, &res);
			process_response(res);
----------------
			exec_func(printf,"***");
			//process_response(res);
#endif
