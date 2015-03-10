<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="../../stylesLayout.css">
	<script language="JavaScript" src="../../scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

        <%ReqNum = getRequestNumber();%>

        window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
        parent.currentPage = self;
        parent.helpPage = 'helpPages/secProfileHelp.asp';

        var target='target=' + window.name;
        var selectedBut = 'But3';
        var selectedText = 'textBut3';
        var page = 0;
        var oldSecurityProfile = <%get("system.security.profile.type");%>;
        var newSecurityProfile = oldSecurityProfile;
        var showWarning = <%get("system.security.profile.showWarning");%>;
        var oldHTTP = <%get("system.security.profile.http");%>;
        var oldHTTPS = <%get("system.security.profile.https");%>;
        var oldHTTPredirect = <%get("system.security.profile.http2https");%>;
        var pageWarning = 0; //kNoSecWarning defined in SystemData.h

        if (top.family_model == "ACS") {
           var menuPage = "/normal/security/securityMenu.asp";
        } else {
           var menuPage = "/normal/security/secAuthMenu.asp";
        }

        function addsecProfileCustomEntry() {
                getSelectedAdd('secProfileCustomEntry.asp','name','500','450',target+"&SSID=<%get("_sid_");%>",'yes');
        }

	// [LMT] Security Enhancements 
        function init()
        {
                if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
		{
                        if (checkLeftMenu(menuPage) == 0) {
                                parent.menuReady = 0;
                                setTimeout('init()', 200);
                                return;
                        }
                        setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
                        parent.topMenu.selectItem(parent.menu.topItem);
                        parent.menu.selectItem("SRV");

			showSecurityInfo();
			if (showWarning == 1) //kSecAdvisory
				newWindow('../../sec-warning.asp', 'Warning', 500, 500, 'yes');
                } else
                        setTimeout('init()', 200);
        }

	function showSecurityInfo()
	{
		switch(oldSecurityProfile) {
			case 4: //kSecSecured
				showSecuredInfo();
			break;
			case 3: //kSecModerate
				showModerateInfo();
			break;
			case 2: //kSecOpen
				showOpenInfo();
			break;
			case 1: //kSecCustom
				showCustomInfo();
			break;
			default: //No Profile set
				showNoProfileInfo();
			break;
		}
	}

	function showSecuredInfo()
	{
		hide('document', 'no_profile');
		hide('document', 'moderate');
		hide('document', 'open');
		hide('document', 'custom');
		show('document', 'secured');
	}

	function showModerateInfo()
	{
		hide('document', 'no_profile');
		hide('document', 'secured');
		hide('document', 'open');
		hide('document', 'custom');
		show('document', 'moderate');
	}

	function showOpenInfo()
	{
		hide('document', 'no_profile');
		hide('document', 'secured');
		hide('document', 'moderate');
		hide('document', 'custom');
		show('document', 'open');
	}

	function showCustomInfo()
	{
		hide('document', 'no_profile');
		hide('document', 'secured');
		hide('document', 'moderate');
		hide('document', 'open');
		show('document', 'custom');
	}

	function showNoProfileInfo()
	{
		hide('document', 'moderate');
		hide('document', 'open');
		hide('document', 'custom');
		hide('document', 'secured');
		show('document', 'no_profile');
	}

	function submit()
	{
		if (document.configForm.elements[5].value != 0) { //not kSecNone
			if (top.family_model != "KVM" &&
				newSecurityProfile == 4) { //kSecSecured
				alert('Changing Security Level does not affect Serial Ports configuration!  Please make sure the current protocols and access method for all Serial Ports match the Security Level chosen.');
			}
			document.configForm.submit();
			parent.mainReady = null;
			continue_verification = true;
		} else {
			alert('You must select a Security Profile before moving to another page...');
			continue_verification = false;
		}
	}

	function setSecurityProfile(chosen_profile)
	{
		switch(chosen_profile) {
			case 'Secured':
				document.configForm.elements[5].value = 4; //kSecSecured
				showSecuredInfo();
				newSecurityProfile = 4;
				if (oldSecurityProfile < 2) { //kSecCustom or kSecNone
					if (oldHTTP ==  1 ||
						oldHTTPS ==  0 ||
						oldHTTPredirect == 1)
						pageWarning = 2; //pagekSecWebsRestart
				}
			break;
			case 'Moderate':
				document.configForm.elements[5].value = 3; //kSecModerate
				showModerateInfo();
				newSecurityProfile = 3;
				if (oldSecurityProfile < 2) { //kSecCustom or kSecNone
					if (oldHTTP ==  0 ||
						oldHTTPS ==  0 ||
						oldHTTPredirect == 0)
						pageWarning = 2; //pagekSecWebsRestart
				}
			break;
			case 'Open':
				document.configForm.elements[5].value = 2; //kSecOpen
				showOpenInfo();
				newSecurityProfile = 2;
				if (oldSecurityProfile < 2) { //kSecCustom or kSecNone
					if (oldHTTP ==  0 ||
						oldHTTPS ==  0 ||
						oldHTTPredirect == 1)
						pageWarning = 2; //pagekSecWebsRestart
				}
			break;
		}

                if ((newSecurityProfile != oldSecurityProfile &&
					 oldSecurityProfile > 1) || //not kSecCustom nor kSecNone
					 (pageWarning == 2)) {
					pageWarning = 0;
					alert('The Web Server will be restarted once you Try or Apply changes. You will have to log in again after that...');
					document.configForm.elements[6].value = 2; //kSecWebsRestart
                } else {
					document.configForm.elements[6].value = 0; //kNoSecWarning
				}
	}

</script>
</head>

<body onload="init();" class="bodyMain">
<form name="secProfileForm" onSubmit="return false;">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">

<table width="100%" align="top" cellpadding="15" cellspacing="0" border="0">
<tr>
	<td>
	<table width="100%" cellpadding="5" cellspacing="0" border="0">
	<tr>
		<td width="90"></td>
		<td align="center"><a href="javascript:setSecurityProfile('Secured');"
		class="mainButtons" style="padding: 3px 18px 3px 18px" onfocus="blur()">Secured</a></td>
		<td align="center"><a href="javascript:setSecurityProfile('Moderate');"
		class="mainButtons" style="padding: 3px 15px 3px 15px" onfocus="blur()">Moderate</a></td>
		<td align="center"><a href="javascript:setSecurityProfile('Open');"
		class="mainButtons" style="padding: 3px 25px 3px 28px" onfocus="blur()">Open</a></td>
		<td width="90"></td>
	</tr>
	</table>
	</td>
</tr>
<tr height="182px">
	<td>
	<div style="position:absolute; left:0px; top:0px;">
	<div id="no_profile" style="position:absolute; visibility:hidden; left:105px; top:80px;">
	<table class="bgColor7" align="center" width="460px" cellpadding="2" cellspacing="0" border="0" >
    <tr>
        <td>
		<table class="bgColor1" width="460px" cellpadding="15" cellspacing="0" class="securityBox" border="0">
		<tr>
			<td>
			Currently there is <span style="font-weight: bold">NO SECURITY PROFILE</span> set.<br><br>
			- You must define a Security Level before proceeding with<br>
			&nbsp;&nbsp;&nbsp;further configuration.<br>
			- Please choose from the three preset security levels:<br>
			&nbsp;&nbsp;&nbsp;Secured, Moderate or Open.<br>
			- You can also configure a customized security profile.
			</td>
		</tr>
		</table>
		</td>
	</tr>
	</table>
	</div>
	<div id="secured" style="position:absolute; left:105px; top:80px; visibility:hidden; z-index:1;">
	<table class="bgColor7" align="center" width="460px" cellpadding="2" cellspacing="0" border="0" >
    <tr>
        <td>
		<table class="bgColor1" width="460px" cellpadding="15" cellspacing="0" class="securityBox" border="0">
		<tr>
			<td>
			Profile is set to:<span style="font-weight: bold"> SECURED</span><br><br>
<script type="text/javascript">
		if (top.family_model == "ACS") {
			document.write('\
			- The use of "Secured" profile disables all protocols except<br>\
			&nbsp;&nbsp;&nbsp;SSH v2, HTTPS and SSH to Serial Ports.<br>\
			- Authentication to access Serial Ports is required.<br>');
                        document.write('\
			- Only very secure services are enabled.<br>\
			- Less secure services are disabled.<br>\
			- Ssh root access is not allowed.');
		} else if (top.family_model == "KVM") {
			document.write('\
			- The use of "Secured" profile disables all protocols except<br>\
			&nbsp;&nbsp;&nbsp;SSH v2 and HTTPS.<br>');
			if (top.kvmAnalog == 0) {
			document.write('\
			- Direct Access to KVM connections are not available.<br>');
			}
			document.write('\
			- Only very secure services are enabled.<br>\
			- Ssh root access is not allowed.');
		} else if (top.family_model == "ONS") {
			document.write('\
			- The use of "Secured" profile disables all protocols except<br>\
			&nbsp;&nbsp;&nbsp;SSH v2, HTTPS and SSH to Serial Ports.<br>\
			- Authentication to access Serial Ports is required.<br>\
			- Direct Access to KVM connections are not available.<br>\
			- Ssh root access is not allowed.');
		} 
</script>
			</td>
		</tr>
		</table>
		</td>
	</tr>
	</table>
	</div>
	<div id="moderate" style="position:absolute; left:105px; top:80px; visibility:hidden; z-index:1;">
	<table class="bgColor7" align="center" width="460px" cellpadding="2" cellspacing="0" border="0" >
    <tr>
        <td>
		<table class="bgColor1" width="460px" cellpadding="15" cellspacing="0" class="securityBox" border="0">
		<tr>
			<td>
			Profile is set to:<span style="font-weight: bold"> MODERATE</span><br><br>
			- "Moderate" is the recommended Security Level.<br>
<script type="text/javascript">
		if (top.family_model == "ACS") {
			document.write('\
			- This profile enables: SSH v1, SSH v2, HTTP, HTTPS,<br>\
			&nbsp;&nbsp;&nbsp;Telnet, SSH and Raw connections to Serial Ports,<br>\
			&nbsp;&nbsp;&nbsp;ICMP and HTTP redirection to HTTPS.<br>\
			- Authentication to access Serial Ports is not required.');
		} else if (top.family_model == "KVM") {
			document.write('\
			- This profile enables: SSH v1, SSH v2, HTTP, HTTPS,<br>\
			&nbsp;&nbsp;&nbsp;ICMP and HTTP redirection to HTTPS.<br>');
			if (top.kvmAnalog == 0) {
			document.write('\
			- Direct Access to KVM connections are not available.<br>');
			}
		} else if (top.family_model == "ONS") {
			document.write('\
			- This profile enables: SSH v1, SSH v2, HTTP, HTTPS,<br>\
			&nbsp;&nbsp;&nbsp;Telnet, SSH and Raw connections to Serial Ports,<br>\
			&nbsp;&nbsp;&nbsp;ICMP and HTTP redirection to HTTPS.<br>\
			- Authentication to access Serial Ports is not required.<br>\
			- Direct Access to KVM connections are not available.<br>');
		}
</script>		
			</td>
		</tr>
		</table>
		</td>
	</tr>
	</table>
	</div>
	<div id="open" style="position:absolute; left:105px; top:80px; visibility:hidden; z-index:1;">
	<table class="bgColor7" align="center" width="460px" cellpadding="2" cellspacing="0" border="0" >
    <tr>
        <td>
		<table class="bgColor1" width="460px" cellpadding="15" cellspacing="0" class="securityBox" border="0">
		<tr>
			<td>
			Profile is set to:<span style="font-weight: bold"> OPEN</span><br><br>
			- It is HIGHLY recommended to review all Security Services.<br>
<script type="text/javascript">
		if (top.family_model == "ACS") {
			document.write('\
			- The use of "Open" profile enables all services: Telnet,<br>\
			&nbsp;&nbsp;&nbsp;SSH v1, SSH v2, HTTP, HTTPS, SNMP, RPC, ICMP and<br>\
			&nbsp;&nbsp;&nbsp;Telnet, SSH and Raw connections to Serial Ports.<br>\
			- Authentication to access Serial Ports is not required.');
		} else if (top.family_model == "KVM") {
			document.write('\
			- The use of "Open" profile enables all services: Telnet,<br>\
			&nbsp;&nbsp;&nbsp;SSH v1, SSH v2, HTTP, HTTPS, SNMP, RPC and ICMP<br>');
			if (top.kvmAnalog == 0) {
			document.write('\
			- Direct Access to KVM connections are available.<br>');
			}
		} else if (top.family_model == "ONS") {
			document.write('\
			- The use of "Open" profile enables all services: Telnet,<br>\
			&nbsp;&nbsp;&nbsp;SSH v1, SSH v2, HTTP, HTTPS, SNMP, RPC, ICMP and<br>\
			&nbsp;&nbsp;&nbsp;Telnet, SSH and Raw connections to Serial Ports.<br>\
			- Authentication to access Serial Ports is not required.<br>\
			- Direct Access to KVM connections are available.<br>');
		}
</script>
			</td>
		</tr>
		</table>
		</td>
	</tr>
	</table>
	</div>
	<div id="custom" style="position:absolute; left:105px; top:80px; visibility:hidden; z-index:1;">
	<table class="bgColor7" align="center" width="460px" cellpadding="2" cellspacing="0" border="0" >
    <tr>
        <td>
		<table class="bgColor1" width="460px" cellpadding="15" cellspacing="0" class="securityBox" border="0">
		<tr>
			<td>
			<%getLabel("nickname");%> currently has a <span style="font-weight: bold">CUSTOMIZED</span> security profile.<br><br>
<script type="text/javascript">
		if (top.family_model == "ACS") {
			document.write('\
			- Console Management provides critical access to management<br>\
			&nbsp;&nbsp;&nbsp;features of attached equipment.<br>');
		} else if (top.family_model == "KVM") {
			document.write('\
			- KVM connection provides critical access to management<br>\
			&nbsp;&nbsp;&nbsp;features of attached equipment.<br>');
		} else if (top.family_model == "ONS") {
			document.write('\
			- Console Management and KVM provides critical access to<br>\
			&nbsp;&nbsp;&nbsp;management features of attached equipment.<br>');
		}
</script>
			- Please take the required precautions to understand the<br>
			&nbsp;&nbsp;&nbsp;potential impacts of each individual service<br>
			&nbsp;&nbsp;&nbsp;configured under the "Custom" profile.
			</td>
		</tr>
		</table>
		</td>
	</tr>
	</table>
	</div>
	</div>
	</td>
</tr>
<tr>
	<td>
	<table width="100%" cellpadding="5" cellspacing="0" border="0">
	<tr>
		<td width="120"></td>
<!--		<td align="center"><a href="javascript:newWindow('secProfileCustomEntry.asp','name','500','450','yes');" -->
		<td align="center"><a href="javascript:addsecProfileCustomEntry();"
		class="mainButtons" style="padding: 3px 15px 3px 15px" onfocus="blur()">Custom</a></td>
		<td align="center"><a href="javascript:setSecurityProfile('Moderate');"
		class="mainButtons" style="padding: 3px 15px 3px 15px" onfocus="blur()">Default</a></td>
		<td width="120"></td>
	</tr>
	</table>
	</td>
</tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/security/secProfile.asp">
	<input type="hidden" name="urlError" value="/normal/security/secProfile.asp">
   	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<!--*******************************************************************-->
	<input type="hidden" name="system.security.profile.type"
		value="<%get("system.security.profile.type");%>">
	<input type="hidden" name="system.security.profile.showWarning" value="">
	<input type="hidden" name="system.security.profile.mode" value="1">
	<!--*******************************************************************-->
</form>

<script type="text/javascript">
	parent.mainReady = true;
</script>
<% set("_sid_", "0"); %>
</body>
</html>
