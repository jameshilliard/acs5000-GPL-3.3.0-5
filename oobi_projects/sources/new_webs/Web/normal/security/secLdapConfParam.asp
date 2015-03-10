<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript"> 
	<% ReqNum = getRequestNumber(); %>
	<%
		var IPv6 = getVal('_IPv6_');
		var input_size = 15;
		var input_maxlength = 39;
		if (IPv6) {
			input_size = 25;
			input_maxlength = 50;
		}
	%>

	top.currentPage = self;
	top.helpPage = 'helpPages/secLdapHelp.asp';
	parent.window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	if ((top.family_model == "ACS")||(top.family_model == "AVCS")) {
		var menuPage = "/normal/security/securityMenu.asp";
	} else {
		var menuPage = "/normal/security/secAuthMenu.asp";
	}
	

	function init() {
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			if (checkLeftMenu(menuPage) == 0) {
				top.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			top.menu.leftHeadMenu("AUTH");
			setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			top.topMenu.selectItem(top.menu.topItem);
			top.menu.selectItem("AUTH");
		} else {
			setTimeout('init()', 200);
		}
	}

	function copyData() {
		if (document.secAuthenticationForm.secureldap.checked == true) {
			document.configForm.elements[7].value = 1;
		} else {
			document.configForm.elements[7].value = 0;
		}
		document.configForm.elements[5].value  = document.secAuthenticationForm.ldap_server.value;
		document.configForm.elements[6].value  = document.secAuthenticationForm.ldap_domain.value;
		document.configForm.elements[8].value  = document.secAuthenticationForm.ldap_username.value;
		document.configForm.elements[9].value  = document.secAuthenticationForm.ldap_password.value;
		document.configForm.elements[10].value = document.secAuthenticationForm.ldap_loginattr.value;
	}

	function form_verification() {
		continue_verification = true;
		var form_location = document.secAuthenticationForm;
		var v_ldap_server =
			check(form_location.ldap_server.value,"IP")? 
				"Fill,IP" : 
				"Fill,HostName";
		continue_verification = verify([
			[form_location.ldap_server,    v_ldap_server],
			[form_location.ldap_domain,    "Fill"       ],
			[form_location.ldap_username,  "Fill?,Space"],
			[form_location.ldap_password,  "Fill?,Space"],
			[form_location.ldap_loginattr, "Fill?,Space"],
		]);
		return continue_verification;
	}

	function submit() {
		if(document.secAuthenticationForm.ldap_server.value != '') {
			if (form_verification()) {
				copyData();
				document.configForm.submit();
				top.mainReady = null;
			}
		} else {
			copyData();
			document.configForm.submit();
			top.mainReady = null;
		}
	}

</script>
</head>

<body class="bodyMiniWizard" onload="init();">
<form name="secAuthenticationForm" method="POST" action=" " onSubmit="return false;">
<table align="center" cellspacing="10" cellpadding="0" border="0">
	<tr>
		<td align="left">&nbsp;</td>
		<td align="left">&nbsp;</td>
	</tr>
	</table>
 	<table width="360" border="0" align="center" cellpadding="0" cellspacing="10">
		<tr align="left" valign="top">
			<td align="left" width="180">
				<font class="label">Ldap Server</font>
			</td>
			<td align="left" width="180">
				<input name="ldap_server" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<%get("system.security.ldapP.ldapServer");%>">
			</td>
		</tr>
		<tr align="left" valign="top">
			<td align="left" width="180">
				<font class="label">Ldap Base</font>
			</td>
			<td align="left" width="180">
				<input name="ldap_domain" type="text" maxlength="69" size="<%write(input_size);%>" class="formText" value="<%get("system.security.ldapP.ldapBase");%>">
			</td>
		</tr>
		<tr align="left" valign="top">
			<td align="center" width="300"  colspan="2">
<% var checked=getVal("system.security.ldapP.ldapsecure");%>
				<input type="checkbox" name="secureldap" <% if (0!=checked) write("checked");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
				<font class="label">Secure Ldap</font>
			</td>
		</tr>
		<tr align="left" valign="top">
			<td align="left" width="180">
				<font class="label">Ldap User Name</font>
			</td>
			<td align="left" width="180">
				<input name="ldap_username" type="text" maxlength="69" size="<%write(input_size);%>" class="formText" value="<% get("system.security.ldapP.ldapUser");%>">
			</td>
		</tr>
		<tr align="left" valign="top">
			<td   align="left" width="180">
				<font class="label">Ldap Password</font>
			</td>
			<td align="left" width="180">
				<input name="ldap_password" type="password" maxlength="30" size="<%write(input_size);%>" class="formText" value="<% get("system.security.ldapP.ldapPassword");%>">
			</td>
		</tr>
		<tr align="left" valign="top">
			<td align="left" width="180"><font class="label">Ldap Login Attribute</font></td>
			<td align="left" width="180"><input name="ldap_loginattr" type="text" maxlength="39" size="<%write(input_size);%>" class="formText" value="<% get("system.security.ldapP.ldapLoginAttribute");%>">
		</td>
	</tr>
</table>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="0">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/security/secLdapConf.asp">
	<input type="hidden" name="urlError" value="/normal/security/secLdapConf.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name="system.security.ldapP.ldapServer" value="">
	<input type="hidden" name="system.security.ldapP.ldapBase" value="">
	<input type="hidden" name="system.security.ldapP.ldapsecure" value="">
	<input type="hidden" name="system.security.ldapP.ldapUser" value="">
	<input type="hidden" name="system.security.ldapP.ldapPassword" value="">
	<input type="hidden" name="system.security.ldapP.ldapLoginAttribute" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
