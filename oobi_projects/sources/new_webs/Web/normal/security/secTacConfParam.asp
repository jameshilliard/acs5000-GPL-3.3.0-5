<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		ReqNum = getRequestNumber();
		var IPv6  = getVal('_IPv6_');
		var input_size = 15;
		var input_maxlength = 30;
		if (IPv6) {
			input_size = 25;
			input_maxlength = 50;
		}
	%>

	top.currentPage = self;
	top.helpPage = 'helpPages/secTacacsHelp.asp';
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
		if (document.secAuthenticationForm.raccess.checked == true) {
			document.configForm.elements[10].value = 1;
		} else {
			document.configForm.elements[10].value = 0;
		}
		document.configForm.elements[5].value = document.secAuthenticationForm.elements[0].value; 
		document.configForm.elements[6].value = document.secAuthenticationForm.elements[1].value; 
		document.configForm.elements[7].value = document.secAuthenticationForm.elements[2].value; 
		document.configForm.elements[8].value = document.secAuthenticationForm.elements[3].value; 
		document.configForm.elements[9].value = document.secAuthenticationForm.elements[4].value; 
		document.configForm.elements[11].value = document.secAuthenticationForm.timeout.value; 
		document.configForm.elements[12].value = document.secAuthenticationForm.retries.value; 
	}

	function form_verification() {
		continue_verification = true;
		var form_location = document.secAuthenticationForm;
		var v_first_aut_serv_hostname =
			check(form_location.first_aut_serv_hostname.value,"IP")?
				"Fill,IP" :
				"Fill,HostName";
		var v_second_aut_serv_hostname =
			check(form_location.second_aut_serv_hostname.value,"IP")?
				"Fill?,IP" :
				"Fill?,HostName";
		var v_first_account_serv_hostname =
			check(form_location.first_account_serv_hostname.value,"IP")?
				"Fill?,IP" :
				"Fill?,HostName";
		var v_second_account_serv_hostname =
			check(form_location.second_account_serv_hostname.value,"IP")?
				"Fill?,IP" :
				"Fill?,HostName";
		continue_verification = verify([
			[form_location.first_aut_serv_hostname,      v_first_aut_serv_hostname     ],
			[form_location.second_aut_serv_hostname,     v_second_aut_serv_hostname    ],
			[form_location.first_account_serv_hostname,  v_first_account_serv_hostname ],
			[form_location.second_account_serv_hostname, v_second_account_serv_hostname],
			[form_location.secret,                       "Fill,Space"                  ],
			[form_location.timeout,                      "Fill,Num"                    ],
			[form_location.retries,                      "Fill?,Num"                   ],
		]);
		return continue_verification;
	}

	function submit() {
		if(document.secAuthenticationForm.elements[0].value != '') {
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
			<font class="label">First Authentication Server</font>
		</td>
		<td align="left" width="180">
			<input name="first_aut_serv_hostname" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<% get("system.security.tacplusP.tacAuthServer"); %>">
		</td>
	</tr>
	<tr align="left" valign="top">
		<td align="left" width="180">
			<font class="label">Second Authentication Server</font>
		</td>
		<td align="left" width="180">
			<input name="second_aut_serv_hostname" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<% get("system.security.tacplusP.tacAuth2Server"); %>">
		</td>
	</tr>
	<tr align="left" valign="top"> 
		<td align="left" width="180">
			<font class="label">First Accounting Server</font>
		</td>
		<td align="left" width="180">
			<input name="first_account_serv_hostname" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<% get("system.security.tacplusP.tacAccServer"); %>">
		</td>
	</tr>
	<tr align="left" valign="top">
		<td align="left" width="180">
			<font class="label">Second Accounting Server</font>
		</td>
		<td align="left" width="180">
			<input name="second_account_serv_hostname" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<% get("system.security.tacplusP.tacAcc2Server"); %>">
		</td>
	</tr>
	<tr align="left" valign="top">
		<td align="left" width="180">
			<font class="label">Secret</font>
		</td>
		<td align="left" width="180">
			<input name="secret" type="password" maxlength="30" size="<%write(input_size);%>" class="formText" value="<% get("system.security.tacplusP.tacSecret"); %>">
		</td>
	</tr>
<%
var checked = " ";
if (getVal("system.security.tacplusP.tacRaccess")) {
	checked = " checked";
}
write('\
	<tr align="left" valign="top">
		<td align="left" width="180">
			<font class="label">Enable Raccess Authorization</font>
		</td>
		<td align="left" width="180">
			<input type="checkbox" name="raccess"' + checked + '>
		</td>
	</tr>');
%>
	<tr align="left" valign="top"> 
		<td align="left" width="180">
			<font class="label">Timeout</font>
		</td>
		<td align="left" width="180">
			<input name="timeout" type="text" maxlength="5" size="<%write(input_size);%>" class="formText" value="<% get("system.security.tacplusP.tacTimeout"); %>">
		</td>
	</tr>
	<tr align="left" valign="top"> 
		<td align="left" width="180"><font class="label">Retries</font></td>
		<td align="left" width="180">
			<input name="retries" type="text" maxlength="5" size="<%write(input_size);%>" class="formText" value="<% get("system.security.tacplusP.tacRetries"); %>">
		</td>
	</tr>
</table>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="0">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/security/secTacConf.asp">
	<input type="hidden" name="urlError" value="/normal/security/secTacConf.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name="system.security.tacplusP.tacAuthServer" value="">
	<input type="hidden" name="system.security.tacplusP.tacAuth2Server" value="">
	<input type="hidden" name="system.security.tacplusP.tacAccServer" value="">
	<input type="hidden" name="system.security.tacplusP.tacAcc2Server" value="">
	<input type="hidden" name="system.security.tacplusP.tacSecret" value="">
	<input type="hidden" name="system.security.tacplusP.tacRaccess" value="">
	<input type="hidden" name="system.security.tacplusP.tacTimeout" value="">
	<input type="hidden" name="system.security.tacplusP.tacRetries" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
