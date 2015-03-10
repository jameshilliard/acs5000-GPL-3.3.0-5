<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	top.currentPage = self;
	var target='&target=' + parent.window.name;
	var sysMsg = parent.sysMsg;

	<% ReqNum = getRequestNumber(); %> 
window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function showDebug() {
	  window.top.debugWindow =
	      window.open("",
			  "Debug",
			  "left=0,top=0,width=300,height=700,scrollbars=yes,"
			  +"status=yes,resizable=yes");
	  window.top.debugWindow.opener = self;
	  // open the document for writing
	  window.top.debugWindow.document.open();
	  window.top.debugWindow.document.write(
	      "<HTML><HEAD><TITLE>Debug Window</TITLE></HEAD><BODY><PRE>\n");
	}

	function debug(text) {
	  if (window.top.debugWindow && ! window.top.debugWindow.closed) {
	    window.top.debugWindow.document.write(text+"\n");
	  }
	}

	function init()	{
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			if (checkLeftMenu('/normal/applications/appsMenu.asp') == 0) {
				top.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			document.configForm.elements[7].value = parent.memSign;
			if (sysMsg == 'reload_outlets') {
				alert('Command submitted. Please reload the page to update outlet state');
				document.configForm.elements[5].value = 18; //Automatic_reload
				setQuerySSID(document.configForm);
				document.configForm.submit();
				top.mainReady = false;
				return;
			}
			top.topMenu.selectItem(top.menu.topItem);
			top.menu.selectItem("PWIPDU");
			if (top.access != "restricted") // for regular user
				setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			if (sysMsg != 'OK') {
				if (sysMsg && (sysMsg[0] != '\0'))
					alert(sysMsg);
				else
					alert("Error in Retrieving.");
			}
		} else {
			setTimeout('init()', 200);
		}
	    //showDebug();
	    //debug("Debug Started");
	}

	function changeCycPmOnOff(port, outlet, device) {
		var where = (device * 100000) +  (port * 1000) + outlet;
		var name1 = "CycPmOnOff_" + where;
	 	var name2 = "CycPmLockUnlock_" + where;
		var OnOff = document.PMOutletsManForm.elements[name1];
		if (document.PMOutletsManForm.elements[name2].value != 'locked') {
			if (OnOff.value=="on") {
				document.configForm.elements[5].value = 2; //Turn_OFF
			} else {
				document.configForm.elements[5].value = 1; //Turn_ON
			}
			document.configForm.elements[6].value = where;
			setQuerySSID(document.configForm);
			document.configForm.submit();
		} else {
			alert("This outlet is locked.");
		}
	}

	function changeCycPmLockUnlock(port, outlet, device ) {
		var where = (device * 100000) +  (port * 1000) + outlet;
	 	var name1 = "CycPmLockUnlock_" + where;
		var locked = document.PMOutletsManForm.elements[name1]; 
		if(locked.value=="locked") {
			document.configForm.elements[5].value = 4; //Unlock
		} else {
			document.configForm.elements[5].value = 3; //Lock
		}
		document.configForm.elements[6].value = where;
		setQuerySSID(document.configForm);
		document.configForm.submit();
	}

	function changeCycPmCycle(port, outlet, device) {
		var where = (device * 100000) +  (port * 1000) + outlet;
		var name1 = "CycPmOnOff_" + where;
	 	var name2 = "CycPmLockUnlock_" + where;
		if (document.PMOutletsManForm.elements[name2].value == 'locked') {
			alert("This outlet is locked.");
		} else {
			if (document.PMOutletsManForm.elements[name1].value != 'off') {
				document.configForm.elements[5].value = 5; //Cycle
				document.configForm.elements[6].value = where;
				setQuerySSID(document.configForm);
				document.configForm.submit();
			} else {
				alert("You can only cycle a turned on outlet.");
			}
		}
	}

	function saveCycPortIPDUs(port) {
		document.configForm.elements[5].value = 6; //Save
		document.configForm.elements[6].value = port * 1000;
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function oldEditOutlet(port, outlet, name, pm, puint, name_length) {
		var where = port * 1000 + outlet;
		var newRef = "appsPMOutletsManEntry.asp?where="+where+"&name="+name+"&pm="+pm+"&puint="+puint+"&name_length="+name_length+target+"&SSID=<%get("_sid_");%>";
		newRef = newRef.replace(/#/g, "-nuMber-");
		document.configForm.elements[6].value = where;
		newWindow(newRef,'name', 350, 250,'yes');
	}

	function saveOutlets() {
		document.configForm.elements[5].value = 6; //Save
		document.configForm.elements[6].value = parent.showingIpdu;
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function newChangeCycPmOnOff(outletindex) {
		var name1 = "CycPmOnOff_" + outletindex;
	 	var name2 = "CycPmLockUnlock_" + outletindex;
		if (document.PMOutletsManForm.elements[name2].value == 0) {
			var outN = outletInfo[outletindex][0];
			var portN = parent.showingIpdu - 1;
			var destO = (portN * 1000) + outN;
			if (document.PMOutletsManForm.elements[name1].value == 1) {
				document.configForm.elements[5].value = 2; //Turn_OFF
			} else {
				document.configForm.elements[5].value = 1; //Turn_ON
			}
			document.configForm.elements[6].value = destO;
			setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = false;
		} else {
			alert("This outlet is locked.");
		}
	}

	function newChangeCycPmLockUnlock(outletindex) {
	 	var name1 = "CycPmLockUnlock_" + outletindex;
		var outN = outletInfo[outletindex][0];
		var portN = parent.showingIpdu - 1;
		var destO = (portN * 1000) + outN;
		if (document.PMOutletsManForm.elements[name1].value == 1) {
			document.configForm.elements[5].value = 4; //Unlock
		} else {
			document.configForm.elements[5].value = 3; //Lock
		}
		document.configForm.elements[6].value = destO;
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	function newChangeCycPmCycle(outletindex) {
		var name1 = "CycPmOnOff_" + outletindex;
	 	var name2 = "CycPmLockUnlock_" + outletindex;
		if (document.PMOutletsManForm.elements[name2].value == 1) {
			alert("This outlet is locked.");
		} else {
			if (document.PMOutletsManForm.elements[name1].value == 1) {
				var outN = outletInfo[outletindex][0];
				var portN = parent.showingIpdu - 1 ;
				var destO = (portN * 1000) + outN;
				document.configForm.elements[5].value = 5; //Cycle
				document.configForm.elements[6].value = destO;
				setQuerySSID(document.configForm);
				document.configForm.submit();
				top.mainReady = false;
			} else {
				alert("You can only cycle a turned on outlet.");
			}
		}
	}

	function newEditOutlet(outletindex) {
		var outN = outletInfo[outletindex][0];
		var name = outletInfo[outletindex][1];
	 	var name2 = "CycPmLockUnlock_" + outletindex;
		var lock = document.PMOutletsManForm.elements[name2].value;
		var minOn = outletInfo[outletindex][4];
		var minOff = outletInfo[outletindex][5];
		var poDelay = outletInfo[outletindex][6];
		var wuState = outletInfo[outletindex][7];
		var realName = outletInfo[outletindex][8];
                var poffDelay = outletInfo[outletindex][12];
                var thre_hc = outletInfo[outletindex][13][0];
                var thre_hw = outletInfo[outletindex][13][1];
                var thre_lw = outletInfo[outletindex][13][2];
                var thre_lc = outletInfo[outletindex][13][3];

		var sminOn = outcap[outletindex][4];
		var sminOff = outcap[outletindex][5];
		var spoDelay = outcap[outletindex][6];
		var swuState = outcap[outletindex][7];
		var spoffDelay = outcap[outletindex][12];
		var sthre = outcap[outletindex][13];

		var isST = parent.isServerTech;

		var portN = parent.showingIpdu - 1;
		var destO = (portN * 1000) + outN;
		var newRef = "appsPMOutletsManEntry.asp?name="+name+"&lock="+lock+"&minOn="+minOn+"&minOff="+minOff+"&poDelay="+poDelay+"&wuState="+wuState+"&realName="+realName+
                        "&spoffDelay="+spoffDelay+"&poffDelay="+poffDelay+
			"&sthre="+sthre+"&thre_hc="+thre_hc+"&thre_hw="+thre_hw+"&thre_lw="+thre_lw+"&thre_lc="+thre_lc+
			"&sminOn="+sminOn+"&sminOff="+sminOff+"&spoDelay="+spoDelay+"&swuState="+swuState+"&isST="+isST+target+"&SSID=<%get("_sid_");%>";
		newRef = newRef.replace(/#/g, "-nuMber-");
		document.configForm.elements[6].value = destO;
		newWindow(newRef,'name', 450, 290,'yes');
	}

	function actionDone() {
		document.configForm.elements[5].value = 16; //Done
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
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
<form name="PMOutletsManForm" method="POST" action=" " onSubmit="return false;">
<script type="text/javascript">
var family_model = parent.family_model;
var outletInfo = parent.outlinfo;
//var outletInfo = [[1, 'outletOne', 0, 0, 0, 1, '0.1', 1, 0],
//				  [2, 'outletTwo', 1, 0, 2, 3, '0.2', 0, 1],
//				  [3, 'outletThree', 1, 0, 4, 11, '0.3', 1, 1],
//				  [4, 'outletFour', 0, 1, 12, 13, '0.4', 0, 1],
//				  [5, 'myFakeServer', 1, 1, 14, 15, '0.5', 2, 1]];
		// outletInfo is [[0. number(int),
		//				   1. name(str),
		//				   2. OnOff(bool),
		//				   3. LockUnlock(bool),
		//				   4. minOnTime(0..15),
		//				   5. minOffTime(0..15),
		//				   6. postOnDelay(str),
		//				   7. wakeupState(0..2),
		//				   8. real_name(bool)
		//				   9. current (str)
		//				  10. power (str)
		//				  11. alarm (str)
		//				  12. postoffdelay (str)
		//				  13. threshold (str = [hc,hw,lw,lc] ) ],...]

var outcap = parent.outcap;
//var outcap = [[0,0,0,0,1,1,1,1,0],
//			  [0,0,0,0,0,1,1,1,0],
//			  [0,0,0,0,1,0,1,1,0],
//			  [0,0,0,0,1,1,0,1,0],
//			  [0,0,0,0,1,1,1,0,0]];
		// outcap is [[0. number(-),
		//			   1. name(-),
		//			   2. OnOff(bool),
		//			   3. LockUnlock(bool),
		//			   4. minOnTime(bool),
		//			   5. minOffTime(bool),
		//			   6. postOnDelay(bool),
		//			   7. wakeupState(bool),
		//			   8. real_name(-),
		//			   9. current (bool),
		//			  10. power (bool),
		//			  11. alarm (bool),
		//			  12. postoffdelay (bool),
		//			  13. threshold (bool) ],...]

	if (outletInfo != 'no-Outlets') {
		//ACS
document.write('<table class="colorTHeader2" align="center" border="0" width="500" cellspacing="0" cellpadding="0">\
		<tr>\
		<td><font class="tabsTextBig">&nbsp;ID: '+parent.ipduName_str+'</font></td>');
		if (top.access != "restricted") // not regular user
document.write('<td align="center"><a href="javascript:saveOutlets();"\
				class="tableButtons" style="padding: 0px 3px 0px 3px" onfocus="blur()">Save Outlets State</a></td>');
document.write('\
			</tr><tr>\
			<td align="left"><font class="tabsTextBig">&nbsp;Model: '+parent.ipduModel_str+'</font></td>\
			<td align="center"><a href="javascript:actionDone();"\
				class="tableButtons" style="padding: 0px 5px 0px 5px" onfocus="blur()">Done</a></td>\
			</tr>\
			</table>\
			<table class="colorTHeaderAlt" align="center" border="0" width="500" cellspacing="0" cellpadding="0">\
			<tr>\
			<td align "left">&nbsp;Outlet</td>\
			<td align="left">Outlet Name</td>\
			<td align="left">&nbsp;Outlet State</td>');
if (outcap[0][9]) { // has current
document.write('\
			<td align="left">&nbsp;Current(A)</td>\
			<td align="left">&nbsp;Power(W)</td>\
			<td align="left">&nbsp;Alarm</td>\
                        <td align="left" width="50">&nbsp;</td>');
}

document.write('<td align="left">&nbsp;&nbsp;</td>');

document.write('\
			</tr>\
			</table>\
			<table align="center" width="500" border="0" cellspacing="0" cellpadding="0">');

		for (var i=0; i < parent.ipduNumberOfOutlets; i++) {
			var line_color;
			if (i%2 != 0)
				line_color = "colorTRowMiddle";
			else
				line_color = "colorTRowAltMiddle";
if (outcap[0][9]) { // has current
document.write('<tr class="'+line_color+'">\
			<td align="left" width="50"><font class="label">&nbsp;'+outletInfo[i][0]+'</font></td>\
			<td align="left" width="85"><font class="label">'+outletInfo[i][1]+'</font></td>');
} else {
document.write('<tr class="'+line_color+'">\
			<td align="left" width="100"><font class="label">&nbsp;'+outletInfo[i][0]+'</font></td>\
			<td align="left" width="200"><font class="label">'+outletInfo[i][1]+'</font></td>');
}

//-------------------------------------------------
document.write('<td align="left" width="120">');
if (outcap[i][2]) { // has on/off
document.write('		<input type="hidden" name="CycPmOnOff_'+i+'" value="'+outletInfo[i][2]+'">\
						<a href="javascript:newChangeCycPmOnOff('+i+');">');
} else {
document.write('		<input type="hidden" name="CycPmOnOff_'+i+'" value="'+outletInfo[i][2]+'">');
}
if (outletInfo[i][2])
document.write('		<img name="bulb_'+i+'" src="../../Images/bulbOn.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
else
document.write('		<img name="bulb_'+i+'" src="../../Images/bulbOff.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
//-------------------------------------------------
if (outcap[i][3]) { // has lock/unlock
document.write('		<input type="hidden" name="CycPmLockUnlock_'+i+'" value="'+outletInfo[i][3]+'">\
						<a href="javascript:newChangeCycPmLockUnlock('+i+');">');
if (outletInfo[i][3])
document.write('		<img name="lock_unlock_'+i+'" src="../../Images/locked.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
				else
document.write('		<img name="lock_unlock_'+i+'" src="../../Images/unlocked.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
} else {
document.write('		<input type="hidden" name="CycPmLockUnlock_'+i+'" value="0">');
}

//-------------------------------------------------
if (outcap[i][2]) { // has on/off
document.write('		<input type="hidden" name="CycPmCycle_'+i+'" value="'+outletInfo[i][2]+'">\
						<a href="javascript:newChangeCycPmCycle('+i+');">');
				if (outletInfo[i][2])
document.write('		<img name="cycleBut_'+i+'" src="../../Images/cycleButOn.gif" width="35" height="16" border="0" alt=""></a>');
				else
document.write('		<img name="cycleBut_'+i+'" src="../../Images/cycleButOff.gif" width="35" height="16" border="0" alt=""></a>');
}
document.write('</td>');
//-------------------------------------------------
if (outcap[i][9]) { // has current
document.write('	<td align="left" width="80"><font class="label">'+outletInfo[i][9]+'</font></td>');
} else {
document.write('	<td align="left"><font class="label">&nbsp;</font></td>');
}
//-------------------------------------------------
if (outcap[i][10]) { // has power
document.write('	<td align="left" width="60"><font class="label">'+outletInfo[i][10]+'</font></td>');
} else {
document.write('	<td align="left"><font class="label">&nbsp;</font></td>');
}
//-------------------------------------------------
if (outcap[i][11]) { // has alarm
document.write('	<td align="left" width="60"><font class="label">'+outletInfo[i][11]+'</font></td>');
} else {
document.write('	<td align="left" width="60"><font class="label">&nbsp;</font></td>');
}
//-------------------------------------------------
document.write('	</td>\
					<td align="right"><a href="javascript:newEditOutlet('+i+');"\
						class="tableButtons" style="padding: 0px 1px 0px 1px" onfocus="blur()">Edit</a>&nbsp;</td>');
		}//for
document.write('</table>');
	} else {
		document.write('\
			<table align="center" height="90%" width="100%" cellspacing="0" cellpadding="0" border="0">\
			<tr valign="middle">\
				<td>\
				<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">\
				<tr>\
					<td>\
					<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">\
					<tr>\
						<td align="center" class="label_for_tables" >\
							There is no outlet assigned to the current user or<br>\
							the PDU does not allow power control of outlets.</td>\
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
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMOutletsMan.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsPMOutletsMan.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.IpduOutMan.action" value="">
	<input type="hidden" name="system.applications.IpduOutMan.sysMsg" value="">
	<input type="hidden" name="system.applications.IpduOutMan.memSignature" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
</body>
<%set("_sid_","0");%>
</html>
