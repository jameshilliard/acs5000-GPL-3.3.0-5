<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<title>New/Mod SNMP v1 v2 Configuration</title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	<%
		var SaddCall = 0;

		// I'm using webMsg as a communication channel to the middleware
		set("system.network.SNMPdaemon.webMsg", getQueryVal("snmpv12entry",-1));

		// This get() makes the appropriate query function to be called in the middleware
		var permission = getVal("system.network.SNMPdaemon.SNMPv12.permission");
		
		var IPv6 = getVal('_IPv6_');
		var input_size = 18;
		var input_maxlength = 18;
		if (IPv6) {
			input_size = 30;
			input_maxlength = 50;
		}
		
		var communityName = getVal("system.network.SNMPdaemon.SNMPv12.communityName");
		var source = getVal("system.network.SNMPdaemon.SNMPv12.source");
		var oid = getVal("system.network.SNMPdaemon.SNMPv12.oid");

		if (permission == 99) {
			SaddCall = 1;
			communityName = "";
			source = "";
			oid = ""
			permission = 0;
		}
		ReqNum = getRequestNumber();
	%>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function fill() {
		document.SNMPEntry1Form.permission.value = <%write(permission);%>;
	}

	function copy_data() {
		document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
		document.configForm.elements[4].value = opener.document.configForm.elements[4].value;
		document.configForm.elements[5].value = opener.document.configForm.elements[5].value;
		document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
		document.configForm.elements[7].value = opener.document.configForm.elements[7].value;

		moveData(document.configForm, document.forms.SNMPEntry1Form);
	}

	function form_verification() {
		form_location = document.SNMPEntry1Form;
		continue_verification = true;
		if (form_location.source.value.length == 0 || form_location.source.value == "default") {
			form_location.source.value = "default";
			continue_verification = verify([
				[form_location.communityName, "Fill,Space"],
				[form_location.oid,           "Fill?,OID" ],
			]);
		} else {
			continue_verification = verify([
				[form_location.communityName, "Fill,Space"],
				[form_location.source,        "Fill,IPm"  ],
				[form_location.oid,           "Fill?,OID" ],
			]);
		}
		return continue_verification;
	}
	
	function addEditSNMPv12Entry() {
		try {
			if (opener.window.name != document.configForm.target) {
				alert("The page which called this form is not available anymore. The changes will not be effective.");
				self.close();
				return;
			}
			if (form_verification()) {
				if (<%write(SaddCall);%>) {
					copy_data();
					document.configForm.elements[8].value = 'v12entryADDED';
				} else {
					copy_data();
					document.configForm.elements[8].value = 'v12entryEDITED';
				}
				setQuerySSID(document.configForm);
				document.configForm.submit();
				self.close();
			}
		} catch (error) {
			self.close();
		}
	}

</script>
</head>

<body class="bodyMain">   
<form name="SNMPEntry1Form" onSubmit="return false;">
<table border="0" align="center" cellpadding="0" cellspacing="10">
	<tr valign="top">
		<td height="40" colspan="2" align="center">
			<a href="javascript:addEditSNMPv12Entry();">
				<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:window.close();">
				<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
		</td>
	</tr>
	<tr valign="bottom">
		<td align="left">
			<font class="label">Community</font>
		</td>
		<td align="left">
			<input name="communityName" type="text" maxlength="30" size="30" class="formText" value="<%write(communityName);%>">
		</td>
	</tr>
	<tr valign="bottom">
		<td align="left">
			<font class="label">Source</font>
		</td>
		<td align="left">
			<input name="source" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<%write(source);%>">
		</td>
	</tr>
	<tr valign="bottom">
		<td align="left">
			<font class="label">OID</font>
		</td>
		<td align="left">
			<input name="oid" type="text" maxlength="39" size="<%write(input_size);%>" class="formText" value="<%write(oid);%>">
		</td>
	</tr>
	<tr valign="bottom">
		<td align="left">
			<font class="label">Permission</font>
		</td>
		<td align="left">
			<select name="permission" class="formText">
				<option value=0>Read Only</option>
				<option value=1>Read/Write</option>
			</select>
		</td>
	</tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netSNMP.asp">
	<input type="hidden" name="urlError" value="/normal/network/netSNMP.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<!--*******************************************************************-->
	<input type="hidden" name="system.network.SNMPdaemon.sysContact" value="">
	<input type="hidden" name="system.network.SNMPdaemon.sysLocation" value="">
	<input type="hidden" name="system.network.SNMPdaemon.SNMPv12Entries" value="">
	<input type="hidden" name="system.network.SNMPdaemon.webMsg" value="">
	<input type="hidden" name="system.network.SNMPdaemon.SNMPv12.communityName" value="">
	<input type="hidden" name="system.network.SNMPdaemon.SNMPv12.source" value="">
	<input type="hidden" name="system.network.SNMPdaemon.SNMPv12.oid" value="">
	<input type="hidden" name="system.network.SNMPdaemon.SNMPv12.permission" value="">
</form>
<script>
fill();
</script>
</body>
</html>
