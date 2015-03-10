<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">	

    var ReqNum = "<%write(getRequestNumber());%>";
    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_' + ReqNum;
	var sysMsg = '<%get("system.applications.pmipmi.sysMsg");%>';

	top.currentPage = self;
	top.helpPage = '/normal/helpPages/appsPMIPMIHelp.asp';
    var target='&target=' + window.name;

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
			 if (top.access != "restricted") // for regular user
				 setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			 top.topMenu.selectItem(top.menu.topItem);
			 top.menu.selectItem("PWIPMI");
			 if (sysMsg != 'OK')
				alert(sysMsg);
		} else {
			 setTimeout('init()', 200);
		}
	    //showDebug();
	    //debug("Debug Started");
	}

	function changeCycPmIpmiOnOff(device) {
		var name1 = "CycPmIpmiOnOff_" + device;
		var OnOff = document.PMIPMIForm.elements[name1];

			if (OnOff.value=="on") {
				document.configForm.elements[5].value = 2; //Turn_OFF
				document.configForm.elements[6].value = device;
			} else {
				document.configForm.elements[5].value = 1; //Turn_ON
				document.configForm.elements[6].value = device;
			}
			setQuerySSID(document.configForm);
			document.configForm.submit();
	}

	function changeCycPmIpmiCycle(device) {
		var name1 = "CycPmIpmiOnOff_" + device;
		if (document.PMIPMIForm.elements[name1].value == 'on') {
			document.configForm.elements[5].value = 5; //Cycle
			document.configForm.elements[6].value = device;
	        setQuerySSID(document.configForm);
			document.configForm.submit();
		} else {
			alert("You can only cycle a turned on device.");
		}
	}

    function editDevice(i) {
        var newRef = "appsPMIPMIEntry.asp?alias="+IPMIinfo[i][0]+"&ip="+IPMIinfo[i][1]+"&at="+IPMIinfo[i][4]+"&al="+IPMIinfo[i][5]+"&un="+IPMIinfo[i][6]+"&pw="+IPMIinfo[i][7]+"&add=0"+target+"&SSID=<%get("_sid_");%>";
        document.configForm.elements[6].value = IPMIinfo[i][3];
        newWindow(newRef,'name', 350, 300,'yes');
    }

	function addIPMIdevice()
	{
		var newRef = "appsPMIPMIEntry.asp?add=1"+target+"&SSID=<%get("_sid_");%>";
        	document.configForm.elements[6].value = 0;
		newWindow(newRef,'name','350','300','no');
	}

	function deleteDevice(device) {
		document.configForm.elements[5].value = 14; //Delete
		document.configForm.elements[6].value = device;
        setQuerySSID(document.configForm);
		document.configForm.submit();
	}

	function submit() {
		document.configForm.request.value = ReqNum;
		document.configForm.elements[5].value = 0; //no_Action
		document.configForm.submit();
		parent.mainReady = null;
	}

	</script>
</head>
<body class="bodyForLayers" onload="init();">
<form name="PMIPMIForm" method="POST" action=" " onSubmit="return false;">

<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td align="center" valign="middle" height="40">
	  <a href="javascript:addIPMIdevice()">
      <img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
	  <br>
	</td>
</tr>
</table>

<script type="text/javascript">
	var IPMIinfo = <%get("system.applications.pmIpmi.ipmiInfoHtml");%>;
	var numIPMIDev = <%get("system.applications.pmIpmi.numActiveIpmiDev");%>;

//for test
//[
//["alias01", "111.111.111.111", "on", 11, "None", "Operator", "", ""],
//["alias02", "222.222.222.222", "off", 22, "MD5", "Operator", "Joao", "passJ"],
//["alias03", "333.333.333.333", "on", 33, "MD2", "Administrator", "Maria", "passM"],
//]; 

	if (IPMIinfo != 'no-IPMIdevice') { 
			document.write('\
			<table width="460" border="0" align="center" cellpadding="0" cellspacing="0">\
			  <tr>\
			    <td class="colorTHeader2"><font class="tabsTextBig">&nbsp;IPMI Devices: \
							<%get("system.applications.pmIpmi.numActiveIpmiDev");%>\
				</font></td>\
			  </tr>\
			</table>\
			<table width="460" border="0" cellpadding="0" cellspacing="0" align="center">\
			  <tr>\
			    <td class="colorTHeaderAlt" width="71"><font class="colorTHeaderAlt">&nbsp;Device Alias</font></td>\
			    <td class="colorTHeaderAlt" align="left" width="42"><font class="colorTHeaderAlt">Server IP</font></td>\
			    <td class="colorTHeaderAlt" align="left" width="80"><font class="colorTHeaderAlt">Device State</font></td>\
			    <td class="colorTHeaderAlt" align="right" width="60"></td>\
			    <td class="colorTHeaderAlt" align="right" width="60"></td>\
			  </tr>');

			var line_color;

		for (var i=0; i < numIPMIDev; i++) {

			if (i%2 != 0)
				line_color = "colorTRowMiddle";
			else
				line_color = "colorTRowAltMiddle";

			document.write('\
				<tr class="'+line_color+'">\
				<td class="'+line_color+'" width="85"><font class="label">&nbsp;'+IPMIinfo[i][0]+'</font></td>\
				<td class="'+line_color+'" align="left" width="73"><font class="label">'+IPMIinfo[i][1]+'</font></td>\
				<td class="'+line_color+'" align="left" width="79">\
				<input type="hidden" name="CycPmIpmiOnOff_'+IPMIinfo[i][3]+'" value="'+IPMIinfo[i][2]+'">\
				<a href="javascript:changeCycPmIpmiOnOff('+IPMIinfo[i][3]+');">');

			if (IPMIinfo[i][2] == 'off')
				document.write('<img name="bulb_'+IPMIinfo[i][3]+'" src="../../Images/bulbOff.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
			else if (IPMIinfo[i][2] == 'on')
				document.write('<img name="bulb_'+IPMIinfo[i][3]+'" src="../../Images/bulbOn.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');
			     else if (IPMIinfo[i][2] == 'uk')
				document.write('<img name="bulb_'+IPMIinfo[i][3]+'" src="../../Images/bulbUK.gif" width="15" height="20" border="0" alt=""></a>&nbsp;');

			document.write('\
				<input type="hidden" name="CycPmIpmiCycle_'+IPMIinfo[i][3]+'" value="'+IPMIinfo[i][2]+'">\
				<a href="javascript:changeCycPmIpmiCycle('+IPMIinfo[i][3]+');">');
			if (IPMIinfo[i][2] == 'off')
				document.write('<img name="cycleBut_'+IPMIinfo[i][3]+'" src="../../Images/cycleButOff.gif" width="35" height="16" border="0" alt=""></a>');
			else
				document.write('<img name="cycleBut_'+IPMIinfo[i][0]+'" src="../../Images/cycleButOn.gif" width="35" height="16" border="0" alt=""></a>');

			document.write('\
				</td>\
				<td class="'+line_color+'"  align="right" width="65" valign="middle">\
				<a href="javascript:editDevice('+i+')")>\
				<img src="../../Images/edit_smallButton.gif" alt="" width="35" height="16" border="0"></a>&nbsp;</td>\
				<td class="'+line_color+'"  align="right" width="45" valign="middle">\
				<a href="javascript:deleteDevice('+IPMIinfo[i][3]+')")>\
				<img src="../../Images/delete_smallButton.gif" alt="" width="35" height="16" border="0"></a>&nbsp;</td>\
				</tr>');

		}
		document.write('</table>');
	} else {
			document.write('\
			<br>\
			<table align="center" height="90%" width="100%" cellspacing="0" cellpadding="0" border="0">\
			<tr valign="middle">\
				<td>\
			<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">\
			    <tr>\
				<td>\
				    <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">\
					<tr>\
					    <td align="center" class="label_for_tables" >\
									There is no IPMI device currently defined.<br>\
									Press "Add" button in case you wish to create one.\
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
<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMIPMI.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsConnect.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.pmIpmi.action" value="">
	<input type="hidden" name="system.applications.pmIpmi.sysMsg" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
</body>
<% set("_sid_","0");%>
</html>
