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
			if (top.access != "restricted") // for regular user
				setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			document.configForm.elements[7].value = parent.memSign;
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

	function clearCurrent(ipduindex)
	{				
		document.configForm.elements[5].value = 8; //Clear_Current
		document.configForm.elements[6].value = ipduInfo[ipduindex][14];
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function clearEnvSensor(ipduindex)
	{
		document.configForm.elements[5].value = 9; //Clear_ENVMON
		document.configForm.elements[6].value = ipduInfo[ipduindex][14];
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function clearPower(ipduindex)
	{
		document.configForm.elements[5].value = 19; //Clear_Power
		document.configForm.elements[6].value = ipduInfo[ipduindex][14];
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function clearVoltage(ipduindex)
	{
		document.configForm.elements[5].value = 20; //Clear_Voltage
		document.configForm.elements[6].value = ipduInfo[ipduindex][14];
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function clearPowerFactor(ipduindex)
	{
		document.configForm.elements[5].value = 21; //Clear_PowerFactor
		document.configForm.elements[6].value = ipduInfo[ipduindex][14];
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function clearHWOvercurrent(ipduindex)
	{
		document.configForm.elements[5].value = 22; //Clear_HWOvercurrent
		document.configForm.elements[6].value = ipduInfo[ipduindex][14];
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function submit()
	{
		document.configForm.elements[5].value = 0; //no_Action
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

</script>
</head>

<body class="bodyForLayers" onload="init();">
<form name="PM_IPDUinfoForm" method="POST" action=" " onSubmit="return false;">
<script type="text/javascript">
var family_model = parent.family_model;

var nIpdus = parent.nIpdus;
//var nIpdus = 2;

var ipduInfo = parent.ipduInfo;
//var ipduInfo = [['myIpdu','serial port 3','Cyclades','PM20', 20, '1.9.11', 'ON', 'OFF', 'ON',
//				 1, 5, 10, 1, [['Total Load', '15 A', '17 A', '20 A', '2 A']], 1, [['temp1','30 C','n/a']], 15, 0, 12, 11],
//				['myIpdu','serial port 3','Cyclades','PM45', 45, '1.9.11', 'OFF', 'ON', 'OFF',
//				 3, 5, 10, 3, [['Inlet 1', '10 A', 'n/a', '15 A', '1 A'],
//							   ['Inlet 2', '10 A', '13 A', 'n/a', '1 A'],
//							   ['Inlet 3', '10 A', '13 A', '15 A', 'n/a']], 1, [['temp1','33 C','47 C']], 15, 0, 12, 11]];
		// ipduInfo is [[ 0. name(str),
		//		  1. complement(str),
		//		  2. vendor(str),
		//		  3. model(str),
		//		  4. num_outlets(int),
		//		  5. sw_ver(str),
		//		  6. num_phases(int),
		//		  7. num_banks(int),
		//		  8. nominal_voltage(int),
		//		  9. curr_sensor_info[][]
		//			0. name(str),
		//			1. value(str),
		//			2. maxval(str),
		//		 10. banks_sensor_info[][][]
		//			0. bank name(str),
		//				0. name(str),
		//				1. value(str),
		//				2. maxval(str),
		//		 11. phases sensor_info[][]
		//			0. phase name(str),
		//				0. name(str),
		//				1. value(str),
		//				2. maxval(str),
		//		 12. envmon_sensors(int),
		//		 13. envmon_sensor_info[][]
		//			0. type (str),
		//			1. name(str),
		//			2. value(str),
		//			3. maxval(str),
		//		 14. gindex

var confcap = parent.confcap;
//var confcap = [[0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,1,0,1],
//			   [0,0,0,0,0,0,1,1,1,0,1,1,0,0,0,0,1,0,1]];
		// confcap is [[ 0. reset max current,
		//	  	 1. reset max power,
		//		 2. reset max envmon,
		//		 3. reset HWOCP,
		//		 4. reset max voltage,
		//		 5. reset max pf,
		//		 6. ,
		//		 7. ,
		//		 8. nominal_voltage(bool),
		//		 9. [ av, max_val,](bool),
		//		10. [ av, max val](bool),
		//		11. [ av, max val] (bool),
		//		12. environ_sensor(bool),
		//	        ...]

for (var i=0; i<nIpdus; i++)
{
document.write('\
<table class="colorTHeader2" align="center" border="0" width="600" cellspacing="0" cellpadding="0">\
<tr>\
	<td><font class="tabsTextBig">&nbsp;ID: '+ipduInfo[i][0]+' ('+ipduInfo[i][1]+')</font></td>\
	');
if (top.access != "restricted") {// not regular user
if (confcap[i][0])
document.write('\
		<td align="center"><a href="javascript:clearCurrent('+i+');"\
		class="tableButtons" style="padding: 0px 3px 0px 3px" onfocus="blur()">Clear Max Current</a></td>');
}
document.write('\
</tr>\
<tr>\
	<td><font class="tabsTextBig">&nbsp;Model: '+ipduInfo[i][2]+' '+ipduInfo[i][3]+'</font></td>\
	');
if (top.access != "restricted") {// not regular user
if (confcap[i][1])
document.write('\
		<td align="center"><a href="javascript:clearPower('+i+');"\
		class="tableButtons" style="padding: 0px 3px 0px 3px" onfocus="blur()">Clear Max Power</a></td>');
document.write('</tr><tr align="rigth"><td>');
if (confcap[i][2]){ //reset max envsensor
document.write('\
		<a href="javascript:clearEnvSensor('+i+');"\
		class="tableButtons" style="padding: 0px 3px 0px 3px" onfocus="blur()">Reset Max Env Sensors</a>');
}
if (confcap[i][3]){ // reset hw overcurrent protection
document.write('\
		<a href="javascript:clearHWOvercurrent('+i+');"\
		class="tableButtons" style="padding: 0px 3px 0px 3px" onfocus="blur()">Reset HW OCP</a>');
}
if (confcap[i][4]){ // reset max voltage
document.write('\
		<a href="javascript:clearVoltage('+i+');"\
		class="tableButtons" style="padding: 0px 3px 0px 3px" onfocus="blur()">Reset Max Voltage</a>');
}
if (confcap[i][5]){ // reset power factor
document.write('\
		<a href="javascript:clearPowerFactor('+i+');"\
		class="tableButtons" style="padding: 0px 3px 0px 3px" onfocus="blur()">Reset Max Power Factor</a>');
}
} // not regular user 

document.write('</td>\
</tr>\
</table>\
<table class="colorTRowAltMiddle" align="center" border="0" width="600" cellspacing="0" cellpadding="0">\
<tr>\
	<td align="left" width="250"><font class="label">&nbsp;Number of Outlets:&nbsp;'+ipduInfo[i][4]+'</font></td></tr>');
document.write('<tr><td align="left" width="250"><font class="label">&nbsp;Number of banks:&nbsp;'+ipduInfo[i][7]+'</font></td>');
if (ipduInfo[i][6]) {
document.write('\
	<td align="left" width="250"><font class="label">&nbsp;3-Phases&nbsp;</font></td>\
	</tr>');
} else {
document.write('\
	<td align="left" width="250"><font class="label">&nbsp;Single-Phase&nbsp;</font></td>\
	</tr>');
}
document.write('\
	<tr>\
	<td align="left" width="600"><font class="label">&nbsp;Software Version:&nbsp;'+ipduInfo[i][5]+'</font></td>\
	</tr>');

for (var j=0; j< 4; j++) { // PDU Elecmon
	var line_color;
	if (j % 2 != 0) 
		line_color = "colorTRowAltMiddleOdd";
	else 
		line_color = "colorTRowAltMiddleEven";
if (confcap[i][9][j][0]) {
document.write('\
<tr class="'+line_color+'">\
	<td align="left">&nbsp;PDU&nbsp;'+ipduInfo[i][9][j][0]+':&nbsp;&nbsp;'+ipduInfo[i][9][j][1]+'&nbsp;</font></td>');
if (confcap[i][9][j][1]) {
document.write('<td align="rigth">Max Detected:&nbsp;'+ipduInfo[i][9][j][2]+'</font></td>');
}

document.write('</tr>');
}
} // PDU elecmon

for (var j=0; j<ipduInfo[i][6]; j++) { // Phases Elecmon
document.write('\
<tr>\
	<td align="left"><class="colorTRowAltMiddle">&nbsp;'+ipduInfo[i][11][j][0]+':&nbsp;</font></td>\
</tr>');

for (var k=1; k < 5; k++) { 
	var line_color;
	if (k % 2 != 0) 
		line_color = "colorTRowAltMiddleOdd";
	else 
		line_color = "colorTRowAltMiddleEven";

if (confcap[i][11][j][k-1][0]) {
document.write('\
<tr class="'+line_color+'">\
	<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'+ipduInfo[i][11][j][k][0]+':&nbsp;&nbsp;'+ipduInfo[i][11][j][k][1]+'&nbsp;</font></td>');
if (confcap[i][11][j][k-1][1]) {
document.write('<td align="rigth">Max Detected:&nbsp;'+ipduInfo[i][11][j][k][2]+'</font></td>');
}
document.write('</tr>');
} 
} // inter loop
} // phases elecmon

for (var j=0; j<ipduInfo[i][7]; j++) { // Banks Elecmon
document.write('\
<tr>\
	<td align="left"><class="colorTRowAltMiddle">&nbsp;'+ipduInfo[i][10][j][0]+':&nbsp;</font></td>\
</tr>');

for (var k=1; k < 5; k++) { 
	var line_color;
	if (k % 2 != 0) 
		line_color = "colorTRowAltMiddleOdd";
	else 
		line_color = "colorTRowAltMiddleEven";

if (confcap[i][10][j][k-1][0]) {
document.write('\
<tr class="'+line_color+'">\
	<td align="left">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'+ipduInfo[i][10][j][k][0]+':&nbsp;&nbsp;'+ipduInfo[i][10][j][k][1]+'&nbsp;</font></td>');
if (confcap[i][10][j][k-1][1]) {
document.write('<td align="rigth">Max Detected:&nbsp;'+ipduInfo[i][10][j][k][2]+'</font></td>');
}
document.write('</tr>');
} 
} // inter loop
} // banks elecmon

for (var j=0; j<ipduInfo[i][12]; j++) { // envmon sensors
	var line_color;
	if (j % 2 != 0) 
		line_color = "colorTRowAltMiddleOdd";
	else 
		line_color = "colorTRowAltMiddleEven";

document.write('\
<tr class="'+line_color+'">\
	<td align="left">&nbsp;'+ipduInfo[i][13][j][0]+'('+ipduInfo[i][13][j][1]+'):&nbsp;&nbsp;'+ipduInfo[i][13][j][2]+'&nbsp;</font></td>');
if (ipduInfo[i][13][j][3]) {
document.write('<td align="rigth">Max Detected:&nbsp;'+ipduInfo[i][13][j][3]+'</font></td>');
}
document.write('</tr>');
} // envmon sensors

document.write('\
</table>'); 
}//for nIpdus

</script>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/appsPM_IPDUinfo.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsPM_IPDUinfo.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.ViewIpduInfo.action" value="">
	<input type="hidden" name="system.applications.ViewIpduInfo.sysMsg" value="">
	<input type="hidden" name="system.applications.ViewIpduInfo.memSignature" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
</body>
<%set("_sid_","0");%>
</html>
