<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<title>Services</title>
	<link rel="STYLESHEET" type="text/css" href="../../stylesLayout.css">
	<script language="JavaScript" src="../../scripts.js" type="text/javascript"></script>
	<script language="JavaScript">
<%
	ReqNum = getRequestNumber();

	var ssh2sport = getVal("system.security.profile.ssh2sport");
	var telnet2sport = getVal("system.security.profile.telnet2sport");
	var raw2sport = getVal("system.security.profile.raw2sport");
	var auth2sport = getVal("system.security.profile.auth2sport");
	var http_enable = getVal("system.security.profile.http");
	var https_enable = getVal("system.security.profile.https");
	var http2https = getVal("system.security.profile.http2https");
%>

	window.name='<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    top.currentPage = self;

    function init()
    {
        if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady)
        {
            if (checkLeftMenu("/normal/security/securityMenu.asp") == 0) {
               top.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
            setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
            top.topMenu.selectItem(top.menu.topItem);
            top.menu.selectItem("SRV");
        } else
            setTimeout('init()', 200);
    }
  
	function verify(){
		if (document.servicesForm.HTTPS_checkbox.checked == false
			 ) {
			 document.servicesForm.HTTP_redirect.disabled = true;
			 document.servicesForm.HTTP_redirect.checked = false;
		 }else{
			document.servicesForm.HTTP_redirect.disabled = false;
		 }
	}

	function submit()
	{
		if (document.servicesForm.Telnet_checkbox.checked == true) {
			document.configForm.elements[5].value = '1';
		} else {
			document.configForm.elements[5].value = '0';
		}
		if (document.servicesForm.SSHv1_checkbox.checked == true) {
			document.configForm.elements[6].value = '1';
		} else {
			document.configForm.elements[6].value = '0';
		}
		if (document.servicesForm.SSHv2_checkbox.checked == true) {
			document.configForm.elements[7].value = '1';
		} else {
			document.configForm.elements[7].value = '0';
		}
		if (document.servicesForm.SSH_root.checked == true) {
			document.configForm.elements[8].value = '1';
		} else {
			document.configForm.elements[8].value = '0';
		}
		if (document.servicesForm.HTTP_checkbox.checked == true) {
			document.configForm.elements[9].value = '1';
		} else {
			document.configForm.elements[9].value = '0';
		}
		if (document.servicesForm.HTTPS_checkbox.checked == true) {
			document.configForm.elements[10].value = '1';
		} else {
			document.configForm.elements[10].value = '0';
		}
		if (document.servicesForm.HTTP_redirect.checked == true) {
			document.configForm.elements[11].value = '1';
		} else {
			document.configForm.elements[11].value = '0';
		}
		if (document.servicesForm.SNMP_checkbox.checked == true) {
			document.configForm.elements[12].value = '1';
		} else {
			document.configForm.elements[12].value = '0';
		}
		if (document.servicesForm.RPC_checkbox.checked == true) {
			document.configForm.elements[13].value = '1';
		} else {
			document.configForm.elements[13].value = '0';
		}
		if (document.servicesForm.ICMP_checkbox.checked == true) {
			document.configForm.elements[14].value = '1';
		} else {
			document.configForm.elements[14].value = '0';
		}
		if (document.servicesForm.ssh2sport.checked == true) {
			document.configForm.elements[15].value = '1';
		} else {
			document.configForm.elements[15].value = '0';
		}
		if (document.servicesForm.telnet2sport.checked == true) {
			document.configForm.elements[16].value = '1';
		} else {
			document.configForm.elements[16].value = '0';
		}
		if (document.servicesForm.raw2sport.checked == true) {
			document.configForm.elements[17].value = '1';
		} else {
			document.configForm.elements[17].value = '0';
		}
		if (document.servicesForm.auth2sport.checked == true) {
			document.configForm.elements[18].value = '1';
		} else {
			document.configForm.elements[18].value = '0';
		}

		setQuerySSID(document.configForm);

		if((<%write(ssh2sport);%> && document.servicesForm.ssh2sport.checked != true) ||
		   (<%write(telnet2sport);%> && document.servicesForm.telnet2sport.checked != true) ||
		   (<%write(raw2sport);%> && document.servicesForm.raw2sport.checked != true) ||
		   (!<%write(auth2sport);%> && document.servicesForm.auth2sport.checked == true)) {
				alert('Changing port access connection option does not affect Serial Ports configuration!');
		}

		if (<%write(http_enable);%> != document.configForm.elements[9].value ||
			<%write(https_enable);%> != document.configForm.elements[10].value ||
			<%write(http2https);%> != document.configForm.elements[11].value){
				alert('The Web Server will be restarted once you Try or Apply changes. You will have to log in again after that...');
		}

		document.configForm.submit();
		top.mainReady = null;
	}

</script>
</head>

<body class="bodyMain" onload="init();verify();">
<form name="servicesForm" onSubmit="return false;">
<table width="100%" align="top" cellpadding="0" cellspacing="8" border="0">
<tr>
  <td>
	<input type="checkbox" <%if (getVal("system.security.profile.telnet") != 0) write("checked");%> name="Telnet_checkbox">
	<font class="label">Telnet</font>
  </td>
  <td>
	<input type="checkbox" <%if (getVal("system.security.profile.snmp") != 0) write("checked");%> name="SNMP_checkbox">
	<font class="label">SNMP</font>
  </td>
  <td>
	<input type="checkbox" <%if (getVal("system.security.profile.rpc") != 0) write("checked");%> name="RPC_checkbox">
	<font class="label">RPC</font>
  </td>
  <td>
	<input type="checkbox" <%if (getVal("system.security.profile.icmp") != 0) write("checked");%> name="ICMP_checkbox">
	<font class="label">ICMP</font>
  </td>
</tr>
</table>
<table width="100%" align="top" cellpadding="0" cellspacing="8" border="0">
<tr height="30">
  <td valign="bottom"><font class="tabsTextBig">SSH</font></td>
  <td valign="bottom"><font class="tabsTextBig">SSH Options</font></td>
</tr>
<tr>
  <td>
	<input type="checkbox" <%if (getVal("system.security.profile.sshv1") != 0) write("checked");%> name="SSHv1_checkbox">
	<font class="label">SSH v1</font>
  </td>
  <td>
	<input type="checkbox" <%if (getVal("system.security.profile.sshRoot") != 0) write("checked");%> name="SSH_root">
	<font class="label">allow root access</font>
  </td>
</tr>
<tr>
  <td>
	<input type="checkbox" <%if (getVal("system.security.profile.sshv2") != 0) write("checked");%> name="SSHv2_checkbox">
	<font class="label">SSH v2</font>
  </td>
</tr>
<tr height="30">
  <td valign="bottom"><font class="tabsTextBig">Web Access</font></td>
  <td colspan="2" valign="bottom"><font class="tabsTextBig">HTTP & HTTPS Options</font></td>
</tr>
<tr>
  <td>
	<input type="checkbox" <%if (http_enable != 0) write("checked");%> name="HTTP_checkbox">
	<font class="label">HTTP</font>
  </td>
  <td>
  <input type="checkbox" <%if (http2https != 0) write("checked");%> name="HTTP_redirect">
	<font class="label">HTTP redirects to HTTPS</font>
  </td>
</tr>            
<tr>
  <td>
	<input type="checkbox" <%if (https_enable != 0) write("checked");%> name="HTTPS_checkbox" onclick="verify();" >
	<font class="label">HTTPS</font>
  </td>
</tr>
<tr><td colspan="8">
<table align="left" border="0">
<tr height="30">
  <td colspan="3" valign="bottom">
	<font class="tabsTextBig">Access to Serial Ports</font>
  </td>
</tr>
<tr>
  <td colspan="3">
	<input type="checkbox" <%if (ssh2sport != 0) write("checked");%> name="ssh2sport">
	<font class="label">allow SSH to Serial Ports</font>
  </td>
</tr>
<tr>
  <td colspan="3">
	<input type="checkbox" <%if (telnet2sport != 0) write("checked");%> name="telnet2sport">
	<font class="label">allow Telnet to Serial Ports</font>
  </td>
</tr>
<tr>
	<td colspan="3">
	<input type="checkbox" <%if (raw2sport != 0) write("checked");%> name="raw2sport">
	<font class="label">allow Raw connection to Serial Ports</font>
  </td>
</tr>
<tr>
	<td colspan="3">
	<input type="checkbox" <%if (auth2sport != 0) write("checked");%> name="auth2sport">
	<font class="label">require Authentication to access Serial Ports</font>
  </td>
</tr>
</table>
</td></tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/security/secServices.asp">
	<input type="hidden" name="urlError" value="/normal/security/secServices.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<!--***************************** 05 a 09 *****************************-->
	<input type="hidden" name="system.security.profile.telnet" value="">
	<input type="hidden" name="system.security.profile.sshv1" value="">
	<input type="hidden" name="system.security.profile.sshv2" value="">
	<input type="hidden" name="system.security.profile.sshRoot" value="">
	<input type="hidden" name="system.security.profile.http" value="">
	<!--***************************** 10 a 14 *****************************-->
	<input type="hidden" name="system.security.profile.https" value="">
	<input type="hidden" name="system.security.profile.http2https" value="">
	<input type="hidden" name="system.security.profile.snmp" value="">
	<input type="hidden" name="system.security.profile.rpc" value="">
	<input type="hidden" name="system.security.profile.icmp" value="">
	<!--***************************** 15 a 19 *****************************-->
	<input type="hidden" name="system.security.profile.ssh2sport" value="">
	<input type="hidden" name="system.security.profile.telnet2sport" value="">
	<input type="hidden" name="system.security.profile.raw2sport" value="">
	<input type="hidden" name="system.security.profile.auth2sport" value="">
	<input type="hidden" name="system.security.profile.mode" value="1">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
