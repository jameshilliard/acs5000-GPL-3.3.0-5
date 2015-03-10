<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>New/Modify Host</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">
<%
	var ehostIndex = getQueryVal("hostIndex",-1);
	var ehostIP = getQueryVal("hostIP",-1);
	var ehostName = getQueryVal("hostName",-1);
	var ehostAlias = getQueryVal("hostAlias","");
	if (ehostIndex == "-1") {
		ehostIP = "";
		ehostName = "";
		ehostAlias = "";
	}
	var IPv6 = getVal('_IPv6_');
	var len1  = 15;
	var mlen1 = 15;
	var len2  = 20;
	
	if (IPv6) {
		len1  = 40;
		mlen1 = 50;
		len2  = 30;
	}
	
	ReqNum = getRequestNumber();
%>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	var hostIndex = "<%write(ehostIndex);%>";

	function copy_data() {
		document.configForm.elements[1].value
			= opener.document.configForm.elements[1].value;
		document.configForm.elements[4].value
			= opener.document.configForm.elements[4].value;
		document.configForm.elements[5].value
			= opener.document.configForm.elements[5].value;
		document.configForm.elements[6].value
			= getValues(opener.document.HostTableForm.table, 0);
	}

	function addHost() {
		copy_data();
		document.configForm.elements[6].value +=
			document.HostTableEntryForm.ip_address.value + 
			' ' + document.HostTableEntryForm.name.value +
			' ' + document.HostTableEntryForm.alias.value +
			",";
		document.configForm.elements[5].value = opener.document.HostTableForm.table.length;
	}

	function form_verification() {

		var form_location = document.HostTableEntryForm

		continue_verification = verify([
			[form_location.ip_address, "Fill,IP"      ],
			[form_location.name,       "Fill,HostName"],
			[form_location.alias,      "Fill?,Alias"  ],
		]);
		
		return continue_verification;
		
		/*checkElement (form_location.elements[0], true, false, true, false);
		if (continue_verification == false) return;
		checkElement (form_location.elements[1], true, true, false, false);
		if(form_location.elements[1].value !=''){
                	continue_verification = checkUsername(form_location.elements[1]);
                	if (!continue_verification) {
                        	alert("Invalid host name.");
                        	return;
                	}
		}
		if(form_location.elements[2].value !=''){
                	continue_verification = checkUsername(form_location.elements[2]);
                	if (!continue_verification) {
                        	alert("Invalid alias name.");
                        	return;
                	}
		}*/

	}	
	
	function addEditHost() {
		try {
			if (opener.window.name != document.configForm.target) {
				alert("The page which called this form is not available anymore." +
					" The changes will not be effective.");
				self.close();
				return;
			}
			form_verification();
			if (continue_verification == true) {
				if (hostIndex == "-1") {
					addHost();
				} else {
					opener.document.HostTableForm.table.options[hostIndex].value=
						document.HostTableEntryForm.ip_address.value + ' ' +
						document.HostTableEntryForm.name.value + ' ' +
						document.HostTableEntryForm.alias.value;
					copy_data();
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
<form name="HostTableEntryForm" onSubmit="return false;">
	<table align="center" valign="middle" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
		<tr>
			<td align="center" valign="middle" height="50">
				<a href="javascript:addEditHost();">
				<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
				&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="javascript:window.close();">
				<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
			</td>
		</tr>
		<tr>
			<td align="center" valign="middle">
				<table border="0" align="center" cellpadding="0" cellspacing="0">
					<tr height="30" valign="bottom">
						<td>
							<font class="label">IP Address</font>
						</td>
					</tr>
					<tr valign="top">
						<td>
							<input name="ip_address" type="text" maxlength="<%write(mlen1);%>" size="<%write(len1);%>" class="formText" value="<%write(ehostIP);%>">
						</td>
					</tr>
					<tr height="30" valign="bottom">
						<td>
							<font class="label">Name</font>
						</td>
					</tr>
					<tr valign="top">
						<td>
							<input name="name" type="text" maxlength="30" size="<%write(len2);%>" value="<%write(ehostName);%>" class="formText">
						</td>
					</tr>
					<tr height="30" valign="bottom">
						<td>
							<font class="label">Alias</font>
						</td>
					</tr>
					<tr valign="top">
						<td>
							<input name="alias" type="text" maxlength="30" size="<%write(len2);%>" value="<%write(ehostAlias);%>" class="formText">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netHostTable.asp">
	<input type="hidden" name="urlError" value="/normal/network/netHostTable.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<input type="hidden" name="system.network.hostTable.numHosts" value="">
	<input type="hidden" name="system.network.hostTable.hostTableSet" value="">
</form>

</body>
</html>