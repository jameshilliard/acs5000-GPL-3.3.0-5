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
			if (sysMsg == 'restart') {
				alert('Due to SPC/ServerTech Ipdu restart, this page will be\n' +
						'automatically reloaded after you press OK.\n' +
						'Please press OK and wait...');
				setTimeout('init()', 10000);
				sysMsg = 'reload';
				return;
			}
			if (sysMsg == 'reload') {
				document.configForm.elements[5].value = 17; //Page_reload
				setQuerySSID(document.configForm);
				document.configForm.submit();
				top.mainReady = false;
				return;
			}
			top.topMenu.selectItem(top.menu.topItem);
			top.menu.selectItem("PWIPDU");
			if (top.access != "restricted") // for regular user
				setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			top.topMenu.selectItem(top.menu.topItem);
			top.menu.selectItem("PWIPDU");
			viewdiv.style.visibility='visible';
			if (sysMsg != 'OK')
				alert(sysMsg);
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
			viewdiv.style.visibility='hidden';
			if (OnOff.value=="on") {
				document.configForm.elements[5].value = 2; //Turn_OFF
				document.configForm.elements[6].value = where;
			} else {
				document.configForm.elements[5].value = 1; //Turn_ON
				document.configForm.elements[6].value = where;
			}
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
			document.configForm.elements[6].value = where;
		} else {
			document.configForm.elements[5].value = 3; //Lock
			document.configForm.elements[6].value = where;
		}
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

	function showIpdu(portindex, ipduindex) {
		document.configForm.elements[5].value = 15; //Show
		document.configForm.elements[6].value = (portindex * 1000) + ipduindex + 1;
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
<DIV ID="viewdiv" style="visibility:visible"> 
<script type="text/javascript">
var family_model = parent.family_model;
if (family_model == 'ONS')
	var serialPorts = parent.serialPorts;

if ((family_model != 'ACS') && (family_model != 'ONS'))
	alert('This page is not prepared to run with '+family_model+'.');

	var nPorts = parent.nPorts;

	var portInfo = parent.portInfo;
//var portInfo = [[2,1],[4,0],[6,2]];
		// portInfo is [[0. number(int),
		//				 1. num_ipdus(int)],...]

	var ipduInfo = parent.ipduInfo;
//var ipduInfo = [[['IpduOne','PM8','Cyclades']],
//			    [],
//			    [['IpduTwo','spcTypeXY','SPC'],['IpduThree','Sentry32','Servertech']]];
		// ipduInfo is [[[0. name(str),
		//				  1. model(str),
		//				  2. vendor(str)],...],...]

	for (var j=0; j < nPorts; j++) { //port level
		if (portInfo[j][1]) {
				if((family_model == 'ONS')&&(portInfo[j][0] > serialPorts)) {
					var portType = "Auxiliary";
					var portNumber = portInfo[j][0] - serialPorts;
				}
				else {
					var portType = "Serial";
					var portNumber = portInfo[j][0];
				}
document.write('<table class="colorTHeader2" width="460" border="0" cellpadding="0" cellspacing="0" align="center">\
				<tr>\
					<td align="left" width="280"><font class="tabsTextBig">&nbsp;'+portType+' Port: '+portNumber+'</font></td>\
					<td rowspan="2" align="center" valign="center" width="180"></td>\
				</tr><tr>\
					<td><font class="tabsTextBig">&nbsp;Number of Units: '+portInfo[j][1]+'</font></td>\
				</tr>\
				</table>\
				<table width="460" border="0" cellpadding="0" cellspacing="0" align="center">\
				<tr>\
					<td class="colorTHeaderAlt" align="left" width="150">&nbsp;Ipdu ID</td>\
					<td class="colorTHeaderAlt" align="left" width="135">Model</td>\
					<td class="colorTHeaderAlt" align="left" width="175">Vendor</td>\
				</tr>\
				</table>\
				<table width="460" border="0" cellpadding="0" cellspacing="0" align="center">');

			for (var i=0; i < portInfo[j][1]; i++) { //ipdu level
				var line_color;
				if (i%2 != 0)
					line_color = "colorTRowMiddle";
				else
					line_color = "colorTRowAltMiddle";
document.write('<tr class="'+line_color+'">\
					<td align="left" width="150"><font class="label">&nbsp;'+ipduInfo[j][i][0]+'</font></td>\
					<td align="left" width="135"><font class="label">'+ipduInfo[j][i][1]+'</font></td>\
					<td align="left" width="125"><font class="label">'+ipduInfo[j][i][2]+'</font></td>\
					<td align="right" width="50"><a href="javascript:showIpdu('+j+','+i+');"\
						class="tableButtons" style="padding: 0px 1px 0px 1px" onfocus="blur()">Show</a>&nbsp;</td>\
				</tr>');
			}//ipdus
document.write('</table>');
		}//if "chain" not empty
	}//ports

</script>
</div>
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
