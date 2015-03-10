<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Custom Profile</title>
	<link rel="STYLESHEET" type="text/css" href="../../stylesLayout.css">
	<script language="JavaScript" src="../../scripts.js" type="text/javascript"></script>
	<script language="JavaScript" type="text/JavaScript">

<%
	var ehostIndex = getQueryVal("hostIndex",-1);
	if (ehostIndex == "-1") {
		ehostIP = "";
		ehostName = "";
		ehostAlias = "";
	}
	ReqNum = getRequestNumber();

	// Handle POST errors:
	var errors = "";
	count = getErrorCount();
	for (i = 0; i < count; i++)
	{
		name = getErrorName(i);
		error = getLabelVal("error." + name);
		if (__error__ == "ParameterName") // ("error." + name) isn't actually set in language.*
		{
			error = getLabelVal("error.unhandledDmfError") + " " + name + ". " + getLabelVal("error.pleaseNotify");
			if (1 == getVal("_kDebug_"))
				error = error + " (" + getErrorMessage(i) + ")";
		}
		else
		{
			error = error + " " + getErrorMessage(i);
		}
		errors = errors + error;
	}

	var family_model = getVal('_familymodel_');
	if (family_model == "ACS" || family_model == "ONS" || family_model == "AVCS") {
		var ssh2sport = getVal("system.security.profile.ssh2sport");
		var telnet2sport = getVal("system.security.profile.telnet2sport");
		var raw2sport = getVal("system.security.profile.raw2sport");
		var bidirect = getVal("system.security.profile.bidirect");
		var auth2sport = getVal("system.security.profile.auth2sport");
	} else {
		var ssh2sport = 0;
		var telnet2sport = 0;
		var raw2sport = 0;
		var bidirect = 0;
		var auth2sport = 0;
	}

	var directaccess = 0;
	var kvmAnalog = 1;
	if (family_model == "KVM" || family_model == "ONS") {
		kvmAnalog = getVal("system.device.kvmanalog");
		if (kvmAnalog == 0) {
			var directaccess = getVal("system.security.profile.directaccess");
		}
	}
%>

        window.name='<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
        var hostIndex="<%write(ehostIndex);%>";
        var http_enable=<%get("system.security.profile.http");%>;
	var https_enable=<%get("system.security.profile.https");%>;
	var http_port=<%get("system.security.profile.httpPort");%>; 
	var https_port=<%get("system.security.profile.httpsPort");%>; 
	var http2https=<%get("system.security.profile.http2https");%>;
	//[GY]2006/Mar,20  BUG#6759
        var load_ssh2sport = <%write(ssh2sport);%>;
        var load_telnet2sport = <%write(telnet2sport);%>;
        var load_raw2sport = <%write(raw2sport);%>;
        var load_bidirect = <%write(bidirect);%>;
        var load_auth2sport = <%write(auth2sport);%>;

	function decideHelpWindow()
	{
		if (!opener.parent.wizard)
			var helpPage = '/normal/helpPages/secProfileCustomEntryHelp.asp';
		else
			var helpPage = '/wizard/helpPages/secProfileCustomEntryHelp.asp';
	}

	function form_verification()
	{
		continue_verification = true;

		checkElement(document.customProfileForm.SSH_port, false, false, false, true)
		if (continue_verification == true)
			checkElement(document.customProfileForm.HTTP_port, false, false, false, true)
		if (continue_verification == true)
			checkElement(document.customProfileForm.HTTPS_port, false, false, false, true)
	}
	function verify(){
                if (document.customProfileForm.HTTPS_checkbox.checked == false) {
                         document.customProfileForm.HTTP_redirect.disabled = true;
                         document.customProfileForm.HTTP_redirect.checked = false;
                }else{
                        document.customProfileForm.HTTP_redirect.disabled = false;
                }
        }
	
	function setCustomProfile()
	{
	try {
		if (opener.window.name != document.configForm.target) {
			alert("The page which called this form is not available anymore." +
					" The changes will not be effective.");
			self.close();
			return;
		}

		form_verification();
		if (continue_verification == true)
		{
			var tcas = 0; 
			var tkvm = 0; 
			var tons = 0; 
			if ("<%write(family_model);%>" == "ACS" || 
				"<%write(family_model);%>" == "AVCS" || 
				"<%write(family_model);%>" == "ONS") {
				tcas = 1;
			}
			if (("<%write(family_model);%>" == "KVM" || 
				"<%write(family_model);%>" == "ONS") &&
				("<%write(kvmAnalog);%>" == "0")) {
				tkvm = 1;
			}
			if ("<%write(family_model);%>" == "ONS") { 
				tons = 1;
			}
			document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
			document.configForm.elements[4].value = opener.document.configForm.elements[4].value;

			if (document.customProfileForm.Telnet_checkbox.checked == true) {
				document.configForm.elements[5].value = '1';
			} else {
				document.configForm.elements[5].value = '0';
			}
			if (document.customProfileForm.SSHv1_checkbox.checked == true) {
				document.configForm.elements[6].value = '1';
			} else {
				document.configForm.elements[6].value = '0';
			}
			if (document.customProfileForm.SSHv2_checkbox.checked == true) {
				document.configForm.elements[7].value = '1';
			} else {
				document.configForm.elements[7].value = '0';
			}
			if (document.customProfileForm.SSH_root.checked == true) {
				document.configForm.elements[9].value = '1';
			} else {
				document.configForm.elements[9].value = '0';
			}
			if (document.customProfileForm.HTTP_checkbox.checked == true) {
				document.configForm.elements[10].value = '1';
			} else {
				document.configForm.elements[10].value = '0';
			}
			if (document.customProfileForm.HTTPS_checkbox.checked == true) {
				document.configForm.elements[11].value = '1';
			} else {
				document.configForm.elements[11].value = '0';
			}
			if (document.customProfileForm.HTTP_redirect.checked == true) {
				document.configForm.elements[14].value = '1';
			} else {
				document.configForm.elements[14].value = '0';
			}
			if (document.customProfileForm.SNMP_checkbox.checked == true) {
				document.configForm.elements[15].value = '1';
			} else {
				document.configForm.elements[15].value = '0';
			}
			if (document.customProfileForm.RPC_checkbox.checked == true) {
				document.configForm.elements[16].value = '1';
			} else {
				document.configForm.elements[16].value = '0';
			}
			if (document.customProfileForm.FTP_checkbox.checked == true) {
				document.configForm.elements[17].value = '1';
			} else {
				document.configForm.elements[17].value = '0';
			}
			if (document.customProfileForm.IPSec_checkbox.checked == true) {
				document.configForm.elements[18].value = '1';
			} else {
				document.configForm.elements[18].value = '0';
			}
			if (document.customProfileForm.ICMP_checkbox.checked == true) {
				document.configForm.elements[19].value = '1';
			} else {
				document.configForm.elements[19].value = '0';
			}
			if (tcas && document.customProfileForm.ssh2sport.checked == true) {
				document.configForm.elements[21].value = '1';
			} else {
				document.configForm.elements[21].value = '0';
			}
			if (tcas && document.customProfileForm.telnet2sport.checked == true) {
				document.configForm.elements[22].value = '1';
			} else {
				document.configForm.elements[22].value = '0';
			}
			if (tcas && document.customProfileForm.raw2sport.checked == true) {
				document.configForm.elements[23].value = '1';
			} else {
				document.configForm.elements[23].value = '0';
			}
			if (tcas && document.customProfileForm.auth2sport.checked == true) {
				document.configForm.elements[24].value = '1';
			} else {
				document.configForm.elements[24].value = '0';
			}
			if (tcas && document.customProfileForm.bidirect.checked == true) {
				document.configForm.elements[25].value = '1';
			} else {
				document.configForm.elements[25].value = '0';
			}

			if (tkvm && document.customProfileForm.directaccess.checked == true) {
				document.configForm.elements[26].value = 1;
			} else {
				document.configForm.elements[26].value = 0;
			}

			if (tons) {
				if (document.customProfileForm.TFTP_checkbox.checked == true) {
					document.configForm.elements[28].value = 1;
				} else {
					document.configForm.elements[28].value = 0;
				}
			}
			document.configForm.elements[8].value = document.customProfileForm.SSH_port.value;
			document.configForm.elements[12].value = document.customProfileForm.HTTP_port.value;
			document.configForm.elements[13].value = document.customProfileForm.HTTPS_port.value;
			setQuerySSID(document.configForm);

            //[GY]2006/Mar,20  BUG#6759
           if(tcas && 
			  ((load_ssh2sport && document.customProfileForm.ssh2sport.checked != true) ||
               (load_telnet2sport && document.customProfileForm.telnet2sport.checked != true) ||
               (load_raw2sport && document.customProfileForm.raw2sport.checked != true) ||
               (load_bidirect && document.customProfileForm.bidirect.checked != true) ||
               (!load_auth2sport && document.customProfileForm.auth2sport.checked == true))) {
              alert('Changing port access connection option does not affect Serial Ports configuration! Please make sure the current protocols and access method for all Serial Ports match the Security Level chosen.');
            }

			if (http_enable != document.configForm.elements[10].value || 
                            https_enable != document.configForm.elements[11].value ||
 			    http_port != document.customProfileForm.HTTP_port.value ||
			    https_port != document.customProfileForm.HTTPS_port.value ||
			    http2https != document.configForm.elements[14].value){
				alert('The Web Server will be restarted once you Try or Apply changes. You will have to log in again after that...');
			}

			document.configForm.submit();
			self.close();
		}
	} catch (error) {
	  self.close();
	}
	}

</script>
</head>

<body class="bodyMain"onload="verify();" >
<form name="customProfileForm" onSubmit="return false;">
<table width="100%" align="top" cellpadding="0" cellspacing="8" border="0">
<tr valign="top"> 
	<td height="40" colspan="3" align="center">
	<a href="javascript:setCustomProfile();">
	<img src="../../Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
	&nbsp;&nbsp;&nbsp;&nbsp;
	<a href="javascript:window.close();">
	<img src="../../Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
	</td>
</tr>
<tr>
	<td width="150">
<!-- <input type="checkbox" <%if (getVal("system.security.profile.telnet") != 0) write("checked");%> name="Telnet_checkbox"> -->
       <input type="checkbox" <%if (getVal("system.security.profile.telnet") != 0) write("checked");%> name="Telnet_checkbox">
	<font class="label">Telnet</font>
	</td>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.snmp") != 0) write("checked");%>
		 name="SNMP_checkbox">
	<font class="label">SNMP</font>
	</td>
	<td width="120">
	<input type="checkbox" <%if (getVal("system.security.profile.ipsec") != 0) write("checked");%>
		 name="IPSec_checkbox">
	<font class="label">IPSec</font>
	</td>
</tr>            
<tr>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.ftp") != 0) write("checked");%> name="FTP_checkbox">
	<font class="label">FTP</font>
	</td>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.rpc") != 0) write("checked");%>
		 name="RPC_checkbox">
	<font class="label">RPC</font>
	</td>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.icmp") != 0) write("checked");%>
		 name="ICMP_checkbox">
	<font class="label">ICMP</font>
	</td>
</tr>

<script type="text/javascript">
if ("<%write(family_model);%>" == "ONS")
{
document.write('\
<tr>\
	<td>\
	<input type="checkbox" <%if (getVal("system.security.profile.tftp") != 0) write("checked");%> name="TFTP_checkbox">\
	<font class="label">TFTP</font>\
	</td>\
</tr>');
}
</script>

<tr height="30">
	<td valign="bottom"><font class="tabsTextBig">SSH</font></td>
	<td valign="bottom"><font class="tabsTextBig">SSH Options</font></td>
</tr>
<tr>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.sshv1") != 0) write("checked");%>
		 name="SSHv1_checkbox">
	<font class="label">SSH v1</font>
	</td>
	<td colspan="2">
	<font class="label">SSH port&nbsp;</font>
	<input class="formText" type="text" maxlength="10" name="SSH_port" size="7"
		value="<%get("system.security.profile.sshPort");%>"></td>
	</td>
</tr>
<tr>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.sshv2") != 0) write("checked");%>
		 name="SSHv2_checkbox">
	<font class="label">SSH v2</font>
	</td>
	<td colspan="2">
	<input type="checkbox" <%if (getVal("system.security.profile.sshRoot") != 0) write("checked");%>
		 name="SSH_root">
	<font class="label">allow root access</font>
	</td>
</tr>            
<tr height="30">
	<td valign="bottom"><font class="tabsTextBig">Web Access</font></td>
	<td colspan="2" valign="bottom"><font class="tabsTextBig">HTTP & HTTPS Options</font></td>
</tr>
<tr>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.http") != 0) write("checked");%>
		 name="HTTP_checkbox">
	<font class="label">HTTP</font>
	</td>
	<td colspan="2">
	<font class="label">HTTP port&nbsp;&nbsp;&nbsp;</font>
	<input class="formText" type="text" maxlength="10" name="HTTP_port" size="7"
		value="<%get("system.security.profile.httpPort");%>"></td>
	</td>
</tr>            
<tr>
	<td></td>
	<td colspan="2">
	<input type="checkbox" <%if (getVal("system.security.profile.http2https") != 0) write("checked");%>
		 name="HTTP_redirect">
	<font class="label">HTTP redirects to HTTPS</font>
	</td>
</tr>            
<tr>
	<td>
	<input type="checkbox" <%if (getVal("system.security.profile.https") != 0) write("checked");%>
		 name="HTTPS_checkbox" onclick="verify();">
	<font class="label">HTTPS</font>
	</td>
	<td colspan="2">
	<font class="label">HTTPS port&nbsp;</font>
	<input class="formText" type="text" maxlength="10" name="HTTPS_port" size="7"
		value="<%get("system.security.profile.httpsPort");%>"></td>
	</td>
</tr>
<script type="text/javascript">
if ("<%write(family_model);%>" == "ACS" || "<%write(family_model);%>" == "ONS" || "<%write(family_model);%>" == "AVCS")
{
document.write('\
<tr><td colspan="8">\
<table align="left" border="0" >\
<tr height="30">\
	<td colspan="3" valign="bottom">\
	<font class="tabsTextBig">Access to Serial Ports</font>\
	</td>\
</tr>\
<tr>\
	<td colspan="3">\
	<input type="checkbox" <%if (ssh2sport != 0) write("checked");%> name="ssh2sport">\
	<font class="label">allow SSH to Serial Ports</font>\
	</td>\
</tr>\
<tr>\
	<td colspan="3">\
	<input type="checkbox" <%if (telnet2sport != 0) write("checked");%> name="telnet2sport">\
	<font class="label">allow Telnet to Serial Ports</font>\
	</td>\
</tr>\
<tr>\
	<td colspan="3">\
	<input type="checkbox" <%if (raw2sport != 0) write("checked");%> name="raw2sport">\
	<font class="label">allow Raw connection to Serial Ports</font>\
	</td>\
</tr>\
<tr>\
        <td colspan="3">\
        <input type="checkbox" <%if (bidirect != 0) write("checked");%> name="bidirect">\
        <font class="label">allow Bidirect connection to Serial Ports</font>\
        </td>\
</tr>\
<tr>\
	<td colspan="3">\
	<input type="checkbox" <%if (auth2sport != 0) write("checked");%> name="auth2sport">\
	<font class="label">require Authentication to access Serial Ports</font>\
	</td>\
</tr>\
</table>\
</td></tr>');
}

if ("<%write(family_model);%>" == "KVM" || "<%write(family_model);%>" == "ONS") {
	if ("<%write(kvmAnalog);%>" == "0") {
	document.write('\
<tr><td colspan="8">\
	<table align="left" border="0" >\
	<tr height="30">\
		<td colspan="5" valign="bottom">\
		<font class="tabsTextBig">Access to KVM Ports</font>\
		</td>\
	</tr>\
	<tr><td colspan="5">\
		<input type="checkbox" name="directaccess" <% if (directaccess != 0) write("checked");%> >\
		<font class="label">allow Direct Access to KVM Ports</font>\
	</td></tr>\
	</table>\
</td></tr>');
	} else {
	document.write('\
<tr><td colspan="8">\
		<input type="hidden" name="directaccess" value="0">\
</td></tr>');
	}
}
</script>
</table>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/security/secProfile.asp">
	<input type="hidden" name="urlError" value="/normal/security/secProfile.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<!--***************************** 05 a 09 *****************************-->
	<input type="hidden" name="system.security.profile.telnet" value="">
	<input type="hidden" name="system.security.profile.sshv1" value="">
	<input type="hidden" name="system.security.profile.sshv2" value="">
	<input type="hidden" name="system.security.profile.sshPort" value="">
	<input type="hidden" name="system.security.profile.sshRoot" value="">
	<!--***************************** 10 a 14 *****************************-->
	<input type="hidden" name="system.security.profile.http" value="">
	<input type="hidden" name="system.security.profile.https" value="">
	<input type="hidden" name="system.security.profile.httpPort" value="">
	<input type="hidden" name="system.security.profile.httpsPort" value="">
	<input type="hidden" name="system.security.profile.http2https" value="">
	<!--***************************** 15 a 19 *****************************-->
	<input type="hidden" name="system.security.profile.snmp" value="">
	<input type="hidden" name="system.security.profile.rpc" value="">
	<input type="hidden" name="system.security.profile.ftp" value="">
	<input type="hidden" name="system.security.profile.ipsec" value="">
	<input type="hidden" name="system.security.profile.icmp" value="">
	<!--***************************** 20 a 25 *****************************-->
	<input type="hidden" name="system.security.profile.type" value="1"><!--kSecCustom-->
	<input type="hidden" name="system.security.profile.ssh2sport" value="">
	<input type="hidden" name="system.security.profile.telnet2sport" value="">
	<input type="hidden" name="system.security.profile.raw2sport" value="">
	<input type="hidden" name="system.security.profile.auth2sport" value="">
	<input type="hidden" name="system.security.profile.bidirect" value="">
	<!--****************************** 26, 27 *****************************-->
	<input type="hidden" name="system.security.profile.directaccess" value="">
	<input type="hidden" name="system.security.profile.mode" value="1">
<script type="text/javascript">
if ("<%write(family_model);%>" == "ONS")
{
	document.write('<input type="hidden" name="system.security.profile.tftp" value="">');
}
</script>
</form>
</body>
</html>
