<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	top.currentPage = self;
	var sysMsg = parent.sysMsg;

	<% ReqNum = getRequestNumber(); %>
window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function init()	{
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			document.configForm.elements[6].value = parent.memSign;
			if (sysMsg != 'OK') {
				if (sysMsg && (sysMsg[0] != '\0'))
					alert(sysMsg);
				else
					alert("Error in Retrieving.");
			}
		}
		else
			setTimeout('init()', 200);
	}

	function prepareData()
	{
		document.configForm.elements[5].value = '';
		for (var p=0; p<nIpdus; p++) {
			elmname = 'changed_'+ p;
			if (document.PM_configForm.elements[elmname].value == 1) {
				document.configForm.elements[5].value += ipduConf[p][10] + ':';
				elmname = 'name_'+ p;
				document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
				if (confcap[p][4]==1) {
					elmname = 'ocp_'+ p;
					if (document.PM_configForm.elements[elmname].checked == 1)
						document.configForm.elements[5].value += '1 ';
					else
						document.configForm.elements[5].value += '0 ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][4] + ' ';
				}
				if (confcap[p][5]==1) {
					elmname = 'syslog_'+ p;
					if (document.PM_configForm.elements[elmname].checked == 1)
						document.configForm.elements[5].value += '1 ';
					else
						document.configForm.elements[5].value += '0 ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][5] + ' ';
				}
				if (confcap[p][6]==1) {
					elmname = 'buzzer_'+ p;
					if (document.PM_configForm.elements[elmname].checked == 1)
						document.configForm.elements[5].value += '1 ';
					else
						document.configForm.elements[5].value += '0 ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][6] + ' ';
				}
				if (confcap[p][9]==1) {
					elmname = 'pollrate_'+ p;
					if (document.PM_configForm.elements[elmname].value != '') {
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += ipduConf[p][9] + ' ';
					}
				} else {
					document.configForm.elements[5].value += ipduConf[p][9] + ' ';
				}
				if (confcap[p][7]==1) {
					elmname = 'cycleint_'+ p;
					document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][7] + ' ';
				}
				if (confcap[p][8]==1) {
					elmname = 'seqint_'+ p;
					document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][8] + ' ';
				}
				if (confcap[p][22]==1) {
					elmname = 'coldstart_'+ p;
					document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][22] + ' ';
				}
				if (confcap[p][23]==1) {
					elmname = 'display_'+ p;
					document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][23] + ' ';
				}
				if (confcap[p][24]==1) {
					elmname = 'display_cycle_'+ p;
					document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][24] + ' ';
				}
				if (confcap[p][12]) {
					elmname = 'defvoltage_'+ p;
					document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][12] + ' ';
				}
				if (confcap[p][13]) {
					elmname = 'pwrfactor_'+ p;
					document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][13] + ' ';
				}
				if (confcap[p][14]) {
					elmname = 'numphases_'+ p;
					document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += ipduConf[p][14] + ' ';
				}
				if (confcap[p][15][0]) {
					elmname = 'thres_PduHC_'+p;
					document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += '99999.9 ';
				}
				if (confcap[p][15][1]) {
					elmname = 'thres_PduLC_'+p;
					document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += '99999.9 ';
				}
				if (confcap[p][15][2]) {
					elmname = 'thres_PduHW_'+p;
					document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
					elmname = 'thres_PduLW_'+p;
					document.configForm.elements[5].value +=
						document.PM_configForm.elements[elmname].value + ' ';
				} else {
					document.configForm.elements[5].value += '99999.9 99999.9 ';
				}
				for (var u=0; u < ipduConf[p][16]; u++) {
					if (confcap[p][17][u][1]==1) {
						elmname = 'thres_BankHC_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][17][u][2]==1) {
						elmname = 'thres_BankLC_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][17][u][3]==1) {
						elmname = 'thres_BankHW_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][17][u][4]==1) {
						elmname = 'thres_BankLW_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
				}
				for (var u=0; u < ipduConf[p][18]; u++) {
					if (confcap[p][19][u][1]==1) {
						elmname = 'thres_PhaseHC_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][19][u][2]==1) {
						elmname = 'thres_PhaseLC_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][19][u][3]==1) {
						elmname = 'thres_PhaseHW_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][19][u][4]==1) {
						elmname = 'thres_PhaseLW_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
				}
				for (var u=0; u < ipduConf[p][20]; u++) {
					if (confcap[p][21][u][1]==1) {
						elmname = 'thres_envHC_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][21][u][2]==1) {
						elmname = 'thres_envLC_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][21][u][3]==1) {
						elmname = 'thres_envHW_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
					if (confcap[p][21][u][3]==1) {
						elmname = 'thres_envLW_'+p+'_'+u;
						document.configForm.elements[5].value +=
							document.PM_configForm.elements[elmname].value + ' ';
					} else {
						document.configForm.elements[5].value += '99999.9 ';
					}
				}
			}
		}
		document.configForm.elements[5].value += 'end';
	}

	function form_verification()
	{
		continue_verification = true;
		for (var p=0; p<nIpdus; p++) {
			elmname = 'changed_'+ p;
			if (document.PM_configForm.elements[elmname].value == 1) {
				elmname = 'name_'+ p;
				var name = document.PM_configForm.elements[elmname];
				var nameval = name.value;
				checkElement(name, false, true, false, false);
				if (continue_verification != true) break;
				if (validateNameOutIpdu(nameval,0) == false) {
					alert ('Invalid IPDU ID.');
					continue_verification = false;
					name.focus();
					name.select();
					break;
				}
				// Check for duplication 
				for (var q = 0; q < nIpdus; q ++) {
					if (q != p && nameval == document.PM_configForm.elements['name_'+q].value) {
						break;
					}
				}
				if (q < nIpdus) {
					alert ('Duplicated IPDU ID!');
					continue_verification = false;
					name.focus();
					name.select();
					break;
				}
				if (confcap[p][9]==1) {
					elmname = 'pollrate_'+ p;
					if ((ipduConf[p][9] != 0) || 
					    (document.PM_configForm.elements[elmname].value != '')) {
						checkElement(document.PM_configForm.elements[elmname], true, true, false, true, 500, 300000);
						if (continue_verification != true) break;
					}
				}
				if (confcap[p][7]==1) {
					elmname = 'cycleint_'+ p;
					if (ipduConf[p][2] == "ServerTech") {
						minval = 5;
					} else {
						minval = 1
					}
					checkElement(document.PM_configForm.elements[elmname], true, true, false, true, minval, 60);
					if (continue_verification != true) break;
				}
				if (confcap[p][8]==1) {
					elmname = 'seqint_'+ p;
					checkElement(document.PM_configForm.elements[elmname], true, true, false, true, 2, 15);
					if (continue_verification != true) break;
				}
				if (confcap[p][12]==1) {
					continue_verification = verify([
						[document.PM_configForm.elements['defvoltage_'+p], "Fill,Bound", [0, undefined]]
					]);
					if (continue_verification != true) break;
				}
				if (confcap[p][13]==1) {
					continue_verification = verify([
						[document.PM_configForm.elements['pwrfactor_'+p],  "Fill,Bound", [0.0, 1.0]    ]
					]);
					if (continue_verification != true) break;
				}
				if (confcap[p][14]==1) {
					elmname = 'numphases_'+ p;
					checkElement(document.PM_configForm.elements[elmname], true, true, false, true, 1, 3);
					if (continue_verification != true) break;
				}
				if (confcap[p][22]==1) {
					elmname = 'coldstart_'+ p;
					checkElement(document.PM_configForm.elements[elmname], true, true, false, true, 0, 60);
					if (continue_verification != true) break;
				}
				if (confcap[p][24]==1) {
					elmname = 'display_cycle_'+ p;
					checkElement(document.PM_configForm.elements[elmname], true, true, false, true, 0, 60);
					if (continue_verification != true) break;
				}
				if (ipduConf[p][11] == 0) {
					maxVal = 30;
				} else {
					maxVal = ipduConf[p][11];
				}
				maxC = maxVal;
				minC = 0;
				if (confcap[p][15][1]) {
					elmname = 'thres_PduLC_'+p;
					continue_verification = verify([
						[document.PM_configForm.elements[elmname],  "Fill,Bound", [0.0, maxC]    ]
					]);
					if (continue_verification != true) break;
					minC = document.PM_configForm.elements[elmname].value;
				}
				if (confcap[p][15][2]) {
					elmname = 'thres_PduLW_'+p;
					LW = document.PM_configForm.elements[elmname].value;
					continue_verification = verify([
						[document.PM_configForm.elements[elmname],  "Fill,Bound", [minC, maxC]    ]
					]);
					if (continue_verification != true) break;
					minC = document.PM_configForm.elements[elmname].value;
					elmname = 'thres_PduHW_'+p;
					LW = document.PM_configForm.elements[elmname].value;
					continue_verification = verify([
						[document.PM_configForm.elements[elmname],  "Fill,Bound", [minC, maxC]    ]
					]);
					if (continue_verification != true) break;
					minC = document.PM_configForm.elements[elmname].value;
				}
				if (confcap[p][15][0]) {
					if (minC == 0) minC = 2;
					elmname = 'thres_PduHC_'+p;
					continue_verification = verify([
						[document.PM_configForm.elements[elmname],  "Fill,Bound", [minC, maxVal]    ]
					]);
					if (continue_verification != true) break;
				}
				for (var u=0; u < ipduConf[p][16]; u++) {
					maxC = maxVal; minC = 0;
					if (confcap[p][17][u][2]==1) {
						elmname = 'thres_BankLC_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][17][u][4]==1) {
						elmname = 'thres_BankLW_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][17][u][3]==1) {
						elmname = 'thres_BankHW_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][17][u][1]==1) {
						if (minC == 0) minC = 2;
						elmname = 'thres_BankHC_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxVal]    ]
						]);
						if (continue_verification != true) break;
					}
				}
				if (continue_verification != true) break;
				for (var u=0; u < ipduConf[p][18]; u++) {
					maxC = maxVal;  minC = 0;
					if (confcap[p][19][u][2]==1) {
						elmname = 'thres_PhaseLC_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[0.0, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][19][u][4]==1) {
						elmname = 'thres_PhaseLW_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][19][u][3]==1) {
						elmname = 'thres_PhaseHW_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][19][u][1]==1) {
						if (minC == 0) minC = 2;
						elmname = 'thres_PhaseHC_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxVal]    ]
						]);
						if (continue_verification != true) break;
					}
				}
				if (continue_verification != true) break;
				for (var u=0; u < ipduConf[p][20]; u++) {
					maxC = 9999; minC = 0;
					if (confcap[p][21][u][2]==1) {
						elmname = 'thres_envLC_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[0.0, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][21][u][3]==1) {
						elmname = 'thres_envLW_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][21][u][3]==1) {
						elmname = 'thres_envHW_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC, maxC]    ]
						]);
						if (continue_verification != true) break;
						minC = document.PM_configForm.elements[elmname].value;
					}
					if (confcap[p][21][u][1]==1) {
						elmname = 'thres_envHC_'+p+'_'+u;
						continue_verification = verify([
							[document.PM_configForm.elements[elmname],  "Fill,Bound", 
							[minC,9999]    ]
						]);
						if (continue_verification != true) break;
					}
				}
				if (continue_verification != true) break;
			}
		}
	}

	function submit()
	{
		form_verification();
		if (continue_verification == true)
		{
			prepareData();
			setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = false;
		}
	}

	function syncConfig(name, idx, p)
	{
		// All IPDUs in the same port should be set up the same configuration
		for (q = 0; q < nIpdus; q ++) {
			if (p != q && ipduConf[p][1] == ipduConf[q][1] && confcap[q][idx] == 1) {
				document.PM_configForm.elements[name+q].checked = 
					document.PM_configForm.elements[name+p].checked;
			}
		}
	}
</script>
</head>

<body class="bodyForLayers" onload="init();">
<form name="PM_configForm" method="POST" action=" " onSubmit="return false;">
<script type="text/javascript">
var family_model = parent.family_model;

if ((family_model != 'ACS')&&(family_model != 'AVCS')&&(family_model != 'ONS'))
	alert('This page is not prepared to run with '+family_model+'.');

var nIpdus = parent.nIpdus;

var ipduConf = parent.ipduConf;
//var ipduConf = [['myIpdu1','serial port 1','Cyclades','PM45',1,1,0,3,
//					[['Phase A',1,10],
//					 ['Phase B',1,10],
//					 ['Phase C',1,10]],9,10,11,1001,13,14],
//				['myIpdu2','serial port 2','Cyclades','PM20',1,0,1,1,
//					[['Total Load',3,30]],9,10,11,2001,13,14]];
		// ipduConf is [[0. name(str),
		//		 1. complement(str),
		//		 2. vendor(str),
		//		 3. model(str),
		//		 4. sw_over_curr_prot(bool),
		//		 5. syslog(bool),
		//		 6. buzzer(bool),
		//		 7. cycle_int(int),
		//		 8. seq_int(int),
		//		 9. poll_rate(int),
		//		10. gindex(int),
		//		11. max_current(int),
		//		12. nominal_voltage(int),
		//		13. power_factor(str),
		//		14. phase (int),
		//		15. PDU_sensor_info[][]
		//			0. threshold_hc(str),
		//			1. threshold_lc(str)
		//			2. threshold_hw(str),
		//			3. threshold_lw(str),
		//		16. Num banks
		//		17. bank current[][]
		//			0. name(str),
		//			1. threshold_hc(str),
		//			2. threshold_lc(str)
		//			3. threshold_hw(str),
		//			4. threshold_lw(str),
		//		18. Num phases
		//		19. phases current[][]
		//			0. name(str),
		//			1. threshold_hc(str),
		//			2. threshold_lc(str)
		//			3. threshold_hw(str),
		//			4. threshold_lw(str),
		//		20. num envmon
		//		21. envmon_info[][]
		//			0. type (str),
		//			1. threshold_hc(str),
		//			2. threshold_lc(str)
		//			3. threshold_hw(str),
		//			4. threshold_lw(str),
		//		22. cold_start


var confcap = parent.confcap;
//var confcap = [[0,0,0,0,1,1,1,0,0,1,1,1,0,1],
//			   [0,0,0,0,1,1,1,0,0,1,1,1,0,1]];
		// confcap is [[ 0. name(-),
		//		 1. complement(-),
		//		 2. vendor(-),
		//		 3. model(-),
		//		 4. sw_over_curr_prot(bool),
		//		 5. syslog(bool),
		//		 6. buzzer(bool),
		//		 7. cycle_int(bool),
		//		 8. seq_int(bool),
		//		 9. poll_rate(bool),
		//		10. gindex(-),
		//		11. max_current(-),
		//		12. nominal_voltage(bool),
		//		13. power_factor(bool),
		//		14. phase (bool),
		//		15. pdu []
		//		16. banks (bool)
		//		17. banks threshold[]
		//		18. phases (bool)
		//		19. phases threshold[]
		//		20. envmon (bool)
		//		21. envmon threshold[]
		//		22. cold_start (bool)

for (var j=0; j<nIpdus; j++)
{
document.write('\
<table class="colorTHeader2" width="600" border="0" align="center" cellpadding="0" cellspacing="0" class="colorTHeaderAlt">\
<tr>\
	<td align="left"><font class="tabsTextBig">&nbsp;ID: '+ipduConf[j][0]+' ('+ipduConf[j][1]+')</font></td>\
	<td align="left"><font class="tabsTextBig">Model: '+ipduConf[j][2]+' '+ipduConf[j][3]+'&nbsp;</font></td>\
</tr>\
</table>\
<table class="colorTRowAltMiddle" width="600" border="0" align="center" cellpadding="0" cellspacing="0">\
<tr">\
	<td align="left" ><font class="label">ID&nbsp;&nbsp;</font>\
		<input type="hidden" name="changed_'+j+'" value="0">\
		<input class="formText" name="name_'+j+'" type="text" size="17" maxlength="16"\
			value="'+ipduConf[j][0]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
if (confcap[j][9]) { // Polling Rate
	if (ipduConf[j][9] == 0) {
document.write('\
	<td align="left"><font class="label">Polling Rate&nbsp;&nbsp;</font>\
		<input class="formText" name="pollrate_'+j+'" type="text" size="6" maxlength="6"\
			onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;><font class="label">&nbsp;ms</font></td>');
	} else {
document.write('\
	<td align="left" width="50%"><font class="label">Polling Rate&nbsp;&nbsp;</font> \
		<input class="formText" name="pollrate_'+j+'" type="text" size="6" maxlength="6"\
			value="'+ipduConf[j][9]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;><font class="label">&nbsp;ms</font></td>');
	}
} else {
document.write('<td>&nbsp;</td>');
} // polling rate

document.write('\
</tr>');

// sw over current protection
document.write('<tr>');
if (confcap[j][4]) {
document.write('\
	<td align="left"><input type="checkbox" name="ocp_'+j+'" value="" \
		onChange="javascript:document.PM_configForm.changed_'+j+'.value=1;" \
		onClick="syncConfig(\'ocp_\', 4, '+j+')"');
	if (ipduConf[j][4])
		document.write(' checked>&nbsp;');
	else
		document.write('>&nbsp;');
document.write('\
		<font class="label">Enable SW Over Current Protection</font></td>');
}
document.write('<td>&nbsp;</td>');
document.write('</tr>');
if (confcap[j][5] || confcap[j][7] ) { // syslog  and cycle interval
document.write('<tr>');
if (confcap[j][5]) {
document.write('\
	<td align="left"><input type="checkbox" name="syslog_'+j+'" value=""\
		onChange="javascript:document.PM_configForm.changed_'+j+'.value=1;" \
		onClick="syncConfig(\'syslog_\', 5, '+j+')"');
	if (ipduConf[j][5])
		document.write(' checked>&nbsp;');
	else
		document.write('>&nbsp;');
document.write('\
		<font class="label">Enable Syslog</font></td>');
}
if (confcap[j][7]){
document.write('\
	<td align="left"><font class="label">Cycle Interval&nbsp;&nbsp;</font> \
		<input class="formText" name="cycleint_'+j+'" type="text" size="3" maxlength="2"\
			value="'+ipduConf[j][7]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;><font class="label">&nbsp;s</font></td>');
}
document.write('</tr>');
} // syslog && cycle interval
if (confcap[j][6] || confcap[j][8]) { // buzzer && seq interval
document.write('<tr>');
if (confcap[j][6]) {
document.write('\
	<td align="left"><input type="checkbox" name="buzzer_'+j+'" value="" \
		onChange="javascript:document.PM_configForm.changed_'+j+'.value=1;" \
		onClick="syncConfig(\'buzzer_\', 6, '+j+')"');
	if (ipduConf[j][6])
		document.write(' checked>&nbsp;');
	else
		document.write('>&nbsp;');
document.write('\
		<font class="label">Enable Buzzer</font></td>');
}
if (confcap[j][8]) {
document.write('\
	<td align="left"><font class="label">Seq. Interval&nbsp;&nbsp;</font> \
		<input class="formText" name="seqint_'+j+'" type="text" size="3" maxlength="2"\
			value="'+ipduConf[j][8]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;><font class="label">&nbsp;s</font></td>');
}
document.write('</tr>');
}  // buzzer && seq interval
document.write('\
</table>\
<table class="colorTRowAltMiddle" width="600" border="0" align="center" cellpadding="0" cellspacing="0">');
if (confcap[j][22] || confcap[j][23] || confcap[j][24]){ // cold start, display, display cycle
document.write('<tr>');
if (confcap[j][23]) {
document.write('\
	<td align="left"><font class="label">Display&nbsp;&nbsp;</font> \
		<select name="display_'+j+'" class="formText" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;>\
                ');
if (ipduConf[j][23]==0) {
		document.write('<option value=0 selected >Normal/Current</option>\
             			<option value=1>Inverted/Current</option>\
				');
} else {
	if (ipduConf[j][23]==1) {
		document.write('<option value=0>Normal/Current</option>\
             			<option value=1 selected>Inverted/Current</option>\
				');
	} else {
		document.write('<option value=0 selected >Normal/Current</option>\
             			<option value=1>Inverted/Current</option>\
				');
	}
}
document.write('</select></td>');
}
if (confcap[j][24]) {
document.write('\
	<td align="left"><font class="label">Display Cycle&nbsp;&nbsp;</font>\
		<input class="formText" name="display_cycle_'+j+'" type="text" size="3" maxlength="2"\
			value="'+ipduConf[j][24]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;><font class="label">&nbsp;s</font></td>');
}
if (confcap[j][22]) {
document.write('\
	<td align="left"><font class="label">Cold Start Delay&nbsp;&nbsp;</font>\
		<input class="formText" name="coldstart_'+j+'" type="text" size="3" maxlength="2"\
			value="'+ipduConf[j][22]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;><font class="label">&nbsp;s</font></td>');
}
document.write('\
</tr>');
} //coldstart, display
if (confcap[j][12] || confcap[j][13] || confcap[j][14]){ // nominal Voltage, phases
document.write('<tr>');
if (confcap[j][12]) { // nominal voltage
document.write('\
		<td align="left">\
			<font class="label">Nominal Voltage&nbsp;&nbsp;</font>\
			<input class="formText" name="defvoltage_'+j+'" type="text" size="5" maxlength="5" value="'+ipduConf[j][12]+'"\
				onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;><font class="label">&nbsp;V</font>\
		</td>');
}
if (confcap[j][13]) { // power factor
document.write('\
		<td align="left">\
			<font class="label">Power Factor&nbsp;&nbsp;</font>\
			<input class="formText" name="pwrfactor_'+j+'" type="text" size="8" maxlength="8" value="'+ipduConf[j][13]+'"\
				onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;>\
		</td>');
}
if (confcap[j][14]){ // num phases
document.write('\
		<td align="left">\
		<font class="label">Phase&nbsp;&nbsp;</font>\
			<select name="numphases_'+j+'" class="formText" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;>\
                               	<option value=1');
if (ipduConf[j][14]==3)
		document.write(' >1-Phase</option>\
					<option value=3 selected>3-Phases</option>');
else
		document.write(' select >1-Phase</option>\
             			<option value=3>3-Phases</option>');
document.write('</select></td>');
} else{
	document.write('<input name="numphases_'+j+'" type="hidden" value="'+ipduConf[j][14]+'">');
	document.write('<td>&nbsp;</td>');
}
document.write('</tr>');
}
document.write('</table>');

document.write('\
<table class="colorTRowAltMiddle" width="600" border="0" align="center" cellpadding="0" cellspacing="0">');
// PDU Current -- 
if (confcap[j][15][0]) {
document.write('\
<tr>\
	<td align="left" width="74"><font class="colorTHeaderAlt">PDU&nbsp;&nbsp;</font></td>');
		if (ipduConf[j][15][0] != 99999.9) {
document.write('\
	<td align="left"><font class="label">HCrit&nbsp;</font>\
		<input class="formText" name="thres_PduHC_'+j+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][15][0]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
		} else {
document.write('\
	<td align="left"><font class="label">HCrit&nbsp;</font>\
		<input class="formText" name="thres_PduHC_'+j+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
		}
if (confcap[j][15][2]) {
		if (ipduConf[j][15][2] != 99999.9) {
document.write('\
	<td align="left"><font class="label">HWarn&nbsp;</font>\
		<input class="formText" name="thres_PduHW_'+j+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][15][2]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left"><font class="label">LWarn&nbsp;</font>\
		<input class="formText" name="thres_PduLW_'+j+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][15][3]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
		} else {
document.write('\
	<td align="left"><font class="label">HWarn&nbsp;</font>\
		<input class="formText" name="thres_PduHW_'+j+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left"><font class="label">LWarn&nbsp;</font>\
		<input class="formText" name="thres_PduLW_'+j+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
		}
} else {
document.write('<td>&nbsp;</td><td>&nbsp;</td>');
	}
	if (confcap[j][15][1]) {
		if (ipduConf[j][15][1] != 99999.9) {
document.write('\
	<td align="left"><font class="label">LCrit&nbsp;</font>\
		<input class="formText" name="thres_PduLC_'+j+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][15][1]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
		} else {
document.write('\
	<td align="left"><font class="label">LCrit&nbsp;</font>\
		<input class="formText" name="thres_PduLC_'+j+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
		}
} else {
document.write('<td>&nbsp;</td>');
	}
	
document.write('</tr>');
} // PDU thresholds

// Phases -- 
if (confcap[j][18]) { // power sensor
	for (var i=0; i<ipduConf[j][18]; i++) { 
		if (confcap[j][19][i][0]) {
document.write('\
<tr>\
	<td align="left"><font class="colorTHeaderAlt">&nbsp;'+ipduConf[j][19][i][0]+'&nbsp;</font></td>');
		if (confcap[j][19][i][1]) {
			if (ipduConf[j][19][i][1] != 99999.9) {
document.write('\
	<td align="left"><font class="label">HCrit&nbsp;</font>\
		<input class="formText" name="thres_PhaseHC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][19][i][1]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			} else {
			}
		}
		if (confcap[j][19][i][3]) {
			if (ipduConf[j][19][i][1] != 99999.9) {
document.write('\
	<td align="left"><font class="label">HWarn&nbsp;</font>\
	<input class="formText" name="thres_PhaseHW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][19][i][3]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left" width="135"><font class="label">LWarn&nbsp;</font>\
	<input class="formText" name="thres_PhaseLW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][19][i][4]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			} else {
document.write('\
	<td align="left"><font class="label">HWarn&nbsp;</font>\
	<input class="formText" name="thres_PhaseHW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left" width="135"><font class="label">LWarn&nbsp;</font>\
	<input class="formText" name="thres_PhaseLW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			}
		}
		if (confcap[j][19][i][2]) {
			if (ipduConf[j][19][i][2] != 99999.9) {
document.write('\
	<td align="left"><font class="label">LCrit&nbsp;</font>\
	<input class="formText" name="thres_PhaseLC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][19][i][2]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			} else {
document.write('\
	<td align="left"><font class="label">LCrit&nbsp;</font>\
	<input class="formText" name="thres_PhaseLC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			}
		}

document.write('\
</tr>');
		} // if confcap
	} // loop of phases
} // phases


// BANKS -- 
if (confcap[j][16]) { // power sensor
	for (var i=0; i<ipduConf[j][16]; i++) { 
		if (confcap[j][17][i][0]) {
document.write('\
<tr>\
	<td align="left"><font class="colorTHeaderAlt">&nbsp;'+ipduConf[j][17][i][0]+'&nbsp;</font></td>');
		if (confcap[j][17][i][1]) {
			if (ipduConf[j][17][i][1] != 99999.9) {
document.write('\
	<td align="left"><font class="label">HCrit&nbsp;</font>\
		<input class="formText" name="thres_BankHC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][17][i][1]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			} else {
document.write('\
	<td align="left"><font class="label">HCrit&nbsp;</font>\
		<input class="formText" name="thres_BankHC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			}
		}
		if (confcap[j][17][i][3]) {
			if (ipduConf[j][17][i][3] != 99999.9) {
document.write('\
	<td align="left"><font class="label">HWarn&nbsp;</font>\
		<input class="formText" name="thres_BankHW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][17][i][3]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left" ><font class="label">LWarn&nbsp;</font>\
		<input class="formText" name="thres_BankLW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][17][i][4]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			} else {
document.write('\
	<td align="left"><font class="label">HWarn&nbsp;</font>\
		<input class="formText" name="thres_BankHW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left" ><font class="label">LWarn&nbsp;</font>\
		<input class="formText" name="thres_BankLW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			}
		}
		if (confcap[j][17][i][2]) {
			if (ipduConf[j][17][i][2] != 99999.9) {
document.write('\
	<td align="left"><font class="label">LCrit&nbsp;</font>\
		<input class="formText" name="thres_BankLC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][17][i][2]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			} else {
document.write('\
	<td align="left"><font class="label">LCrit&nbsp;</font>\
		<input class="formText" name="thres_BankLC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>');
			}
		}

document.write('\
</tr>');
		} // if confcap
	} // loop of banks
} // banks 

// EnvMon -- 
if (confcap[j][20]) { // envmon sensor
	for (var i=0; i<ipduConf[j][20]; i++) {
		if (confcap[j][21][i][0]) {
document.write('\
<tr>\
	<td align="left"><font class="colorTHeaderAlt" nowrap="nowrap" style="white-space: nowrap;">&nbsp;'+ipduConf[j][21][i][0]+'&nbsp;</font></td>\
	<td align="left"><font class="label">HCrit&nbsp;</font>\
		<input class="formText" name="thres_envHC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][21][i][1]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left"><font class="label">HWarn&nbsp;</font>\
		<input class="formText" name="thres_envHW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][21][i][3]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left"><font class="label">LWarn&nbsp;</font>\
		<input class="formText" name="thres_envLW_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][21][i][4]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	<td align="left"><font class="label">LCrit&nbsp;</font>\
		<input class="formText" name="thres_envLC_'+j+'_'+i+'" type="text" size="5" maxlength="4"\
		value="'+ipduConf[j][21][i][2]+'" onChange=javascript:document.PM_configForm.changed_'+j+'.value=1;></td>\
	</tr>');
		}
	}
}

document.write('\
</table>');
}
</script>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMconfig.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsPMconfig.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.IpduConfig.sysMsg" value="">
	<input type="hidden" name="system.applications.IpduConfig.memSignature" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
</body>
<%set("_sid_","0");%>
</html>
