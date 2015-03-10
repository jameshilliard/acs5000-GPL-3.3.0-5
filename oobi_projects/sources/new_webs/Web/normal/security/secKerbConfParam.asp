<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript"> 
	<%
		ReqNum = getRequestNumber();
		var IPv6 = getVal('_IPv6_');
		var input_size = 15;
		var input_maxlength = 39;
		if (IPv6) {
			input_size = 25;
			input_maxlength = 50;
		}
	%>
	top.currentPage = self;
	top.helpPage = 'helpPages/secKerbHelp.asp';
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
		document.configForm.elements[5].value = document.secAuthenticationForm.elements[0].value;
		document.configForm.elements[6].value = document.secAuthenticationForm.elements[1].value;
	}

	function form_verification() {
		continue_verification = true;
		var form_location = document.secAuthenticationForm;
		var v_kerberos_server =
			check(form_location.kerberos_server.value,"IP")? 
				"Fill,IP" : 
				"Fill,HostName";
		continue_verification = verify([
			[form_location.kerberos_server,    v_kerberos_server],
			[form_location.kerberos_dom_name,  "Fill,Space" ],
		]);
		return continue_verification;
	}

	function submit() {
		if(document.secAuthenticationForm.elements[0].value != '') {
			form_verification();
			if (continue_verification == true) {
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
			<font class="label">Kerberos Server (Realm)</font>
		</td>
		<td align="left" width="180">
			<input name="kerberos_server" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<%get("system.security.kerberosP.krbServer");%>">
		</td>
	</tr>
	<tr align="left" valign="top">
		<td align="left" width="180">
			<font class="label">Kerberos Realm Domain Name</font>
		</td>
		<td align="left" width="180">
			<input name="kerberos_dom_name" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<%get("system.security.kerberosP.krbDomainName");%>">
		</td>
	</tr>
</table>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="0">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/security/secKerbConf.asp">
	<input type="hidden" name="urlError" value="/normal/security/secKerbConf.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name="system.security.kerberosP.krbServer" value="">
	<input type="hidden" name="system.security.kerberosP.krbDomainName" value="">
</form>

<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
