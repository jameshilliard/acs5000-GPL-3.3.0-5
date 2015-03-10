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
			if (sysMsg == 'reload_outlets') {
				alert('Command submitted. Please reload the page to update outlet state');

				document.configForm.elements[5].value = 18; //Automatic_reload
				setQuerySSID(document.configForm);
				document.configForm.submit();
				top.mainReady = false;
				return;
			}

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

	function actionOutletGroupOnOff(groupindex, oneOutletOff) {
	 	var name = "OutletGroupLockUnlock_" + groupindex;

		if (document.PMOutGroupsForm.elements[name].value == 1) {
			alert("This group is locked.");
		} else {

			if (oneOutletOff)
				document.configForm.elements[5].value = 1; //Turn_ON
			else
				document.configForm.elements[5].value = 2; //Turn_OFF

			document.configForm.elements[6].value = groupInfo[groupindex][0];
			setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = false;
		}
	}

	function actionOutletGroupLockUnlock(groupindex, oneOutletLocked) {

		if (oneOutletLocked)
			document.configForm.elements[5].value = 4; //Unlock
		else
			document.configForm.elements[5].value = 3; //Lock

		document.configForm.elements[6].value = groupInfo[groupindex][0];
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function actionOutletGroupCycle(groupindex, oneOutletOff) {
	 	var name = "OutletGroupLockUnlock_" + groupindex;

		if (document.PMOutGroupsForm.elements[name].value == 1) {
			alert("This group is locked.");
		} else {
			if (oneOutletOff) {
				alert("All outlets must be turned 'on'.");
			} else {
				document.configForm.elements[5].value = 5; //Cycle
				document.configForm.elements[6].value = groupInfo[groupindex][0];
				setQuerySSID(document.configForm);
				document.configForm.submit();
				top.mainReady = false;
			}
		}
	}

	function submit() {
		document.configForm.elements[5].value = 0; //no_Action
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

</script>
</head>

<body class="bodyForLayers" onload="init();">
<form name="PMOutGroupsForm" method="POST" action=" " onSubmit="return false;">
<script type="text/javascript">
var family_model = parent.family_model;

if ((family_model != 'ACS')&&(family_model != 'ONS'))
	alert('This page is not prepared to run with '+family_model+'.');

var nGroups = parent.nGroups;

if (nGroups > 0) { 

var groupInfo = parent.groupInfo;
//var groupInfo = [['Firewall',2],
//				 ['Ventilador',1],
//				 ['MySuperServer',3]];
		// groupInfo is [[0. name(str),
		//				  1. num_outlets(int),
		//				  2. gindex(int)],...]

var outletInfo = parent.outletInfo;
//var outletInfo = [[['tomadaX','serial port 1',1,1],['tomadaY','serial port 1',1,1]],
//				  [['IpduZ:9','serial port 4',1,0]],
//				  [['out1','serial port 3',1,0],['out2','serial port 5',1,1],['out3','serial port 5',0,0]]];
		// outletInfo is [[[0. name(str),
		//					1. complement(str),
		//					2. OnOff(bool),
		//					3. LockUnlock(bool)],...],...]


document.write('<table class="colorTHeader2" width="460" border="0" cellpadding="0" cellspacing="0" align="center">\
				<tr>\
					<td align="left" width="180"><font class="tabsTextBig">&nbsp;Group Name:</font></td>\
					<td align="left" width="205"><font class="tabsTextBig">&nbsp;&nbsp;Group Ctrl</font></td>\
					<td align="center" width="75"><font class="tabsTextBig">Individual</font></td>\
				</tr>\
				<tr>\
					<td align="left" width="180"><font class="tabsTextBig">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Outlets</font></td>\
					<td align="left" width="205"><font class="tabsTextBig">&nbsp;(Ipdu and Port)</font></td>\
					<td align="center" width="75"><font class="tabsTextBig">Status</font></td>\
				</tr>\
				</table>');

for (var j=0; j < nGroups; j++) {//groups

var oneisoff = new Array(nGroups);
var oneislocked = new Array(nGroups);
oneisoff[j] = 0; //used to indicate if at least one outlet is off
oneislocked[j] = 0; //used to indicate if at least one outlet is locked
var group_status = 0;

	for (var k=0; k < groupInfo[j][1]; k++) {
		if (outletInfo[j][k][2] == 1) {
			group_status = 1;
		} else {
			oneisoff[j] = 1;
		}
	}

var group_lock = 0;
	for (k=0; k < groupInfo[j][1]; k++) {
		if (outletInfo[j][k][3] == 0) {
		} else if (outletInfo[j][k][3] == 1) {
			group_lock = 1;
		} else { // outlet not suppor lock/unlock
			group_lock = 10;
			break;
		}
	}

document.write('\
	<table width="460" border="0" cellpadding="0" cellspacing="0" align="center">\
	<tr class="colorTRowAltMiddle">\
		<td align="left" width="180"><font class="colorTHeaderAlt">&nbsp;'+groupInfo[j][0]+':&nbsp;</font></td>\
		<td align="left" colspan="2">\
				<input type="hidden" name="OutletGroupOnOff_'+j+'" value="'+group_status+'">\
					<a href="javascript:actionOutletGroupOnOff('+j+','+oneisoff[j]+');">');
	if (group_status)
document.write('	<img name="bulb_'+j+'" src="../../Images/bulbOn.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
	else
document.write('	<img name="bulb_'+j+'" src="../../Images/bulbOff.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');

	document.write('<input type="hidden" name="OutletGroupLockUnlock_'+j+'" value="'+group_lock+'">');
	if (group_lock == 0) {
		document.write('<a href="javascript:actionOutletGroupLockUnlock('+j+', 0);">');
		document.write('<img name="lock_unlock_'+j+'" src="../../Images/unlocked.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
	} else if (group_lock == 1) {
		document.write('<a href="javascript:actionOutletGroupLockUnlock('+j+', 1);">');
		document.write('<img name="lock_unlock_'+j+'" src="../../Images/locked.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
	} else {
		document.write('	<img name="lock_unlock_'+j+'" src="../../Images/empty.gif" width="15" height="20" border="0" alt="">&nbsp;');
	}

document.write('<input type="hidden" name="OutletGroupCycle_'+j+'" value="0">\
					<a href="javascript:actionOutletGroupCycle('+j+','+oneisoff[j]+');">');
	if (oneisoff[j])
document.write('<img name="cycleBut_'+j+'" src="../../Images/cycleButOff.gif" width="35" height="16" border="0" alt=""></a></td></tr>');
	else
document.write('<img name="cycleBut_'+j+'" src="../../Images/cycleButOn.gif" width="35" height="16" border="0" alt=""></a></td></tr>');

// ------------------------------------------------------------------------------------------

for (var i=0; i < groupInfo[j][1]; i++) {//outlets

var linecolor;

	if (i%2)
		linecolor = "colorTRowAltMiddle";
	else
		linecolor = "colorTRowMiddle";

document.write('\
	<tr class="'+linecolor+'">\
		<td class="'+linecolor+'" align="left" width="180"><font class="label">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'+outletInfo[j][i][0]+'</font></td>\
		<td class="'+linecolor+'" align="left" width="205"><font class="label">('+outletInfo[j][i][1]+')</font></td>\
		<td class="'+linecolor+'" align="center" width="75">');

	if (outletInfo[j][i][2])
document.write('<img name="bulb_'+j+'_'+i+'" src="../../Images/bulbOn.gif" width="15" height="20" border="0" alt="">&nbsp;');
	else
document.write('<img name="bulb_'+j+'_'+i+'" src="../../Images/bulbOff.gif" width="15" height="20" border="0" alt="">&nbsp;');

	if (outletInfo[j][i][3]==1) {
		document.write('<img name="lock_unlock_'+j+'_'+i+'" src="../../Images/locked.gif" width="15" height="20" border="0" alt="">&nbsp;</td></tr>');
	} else if (outletInfo[j][i][3]==0) {
		document.write('<img name="lock_unlock_'+j+'_'+i+'" src="../../Images/unlocked.gif" width="15" height="20" border="0" alt="">&nbsp;</td></tr>');
	} else {
		document.write('<img name="lock_unlock_'+j+'_'+i+'" src="../../Images/empty.gif" width="15" height="20" border="0" alt="">&nbsp;</td></tr>');
	}
}//outlets

}//groups

	document.write('</table>');

} else {
	document.write('\
		<table align="center" height="100%" width="100%" cellspacing="0" cellpadding="0" border="0">\
		<tr valign="middle">\
		<td>\
			<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">\
		    <tr>\
			<td>\
				<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">\
				<tr>\
				<td align="center" class="label_for_tables" >\
					There is no outlet group defined for the current user or<br>\
					the <%getLabel("nickname");%> has lost IPDU communication.\
				</td>\
				</tr>\
			</table>\
			</td>\
		    </tr>\
		</table>\
		</td>\
		</tr>\
		</table>');
}

</script>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMOutGroups.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsPMOutGroups.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.IpduOutGrpCtrl.action" value="">
	<input type="hidden" name="system.applications.IpduOutGrpCtrl.sysMsg" value="">
	<input type="hidden" name="system.applications.IpduOutGrpCtrl.memSignature" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
</body>
<%set("_sid_","0");%>
</html>
