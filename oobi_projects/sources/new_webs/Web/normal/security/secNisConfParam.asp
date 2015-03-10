<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<% ReqNum = getRequestNumber(); %>

	top.currentPage = self;
	top.helpPage = 'helpPages/secNisHelp.asp';
	parent.window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	var menuPage;

	if ((top.family_model == "ACS")||(top.family_model == "AVCS")) {
		menuPage = "/normal/security/securityMenu.asp";
	} else {
		menuPage = "/normal/security/secAuthMenu.asp";
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
			if (parseInt(getIPmode()) == 2) {
				show2('document', 'nisNOK');
				hide2('document', 'nisOK');
			} else {
				show2('document', 'nisOK');
				hide2('document', 'nisNOK');
			}
		} else {
			setTimeout('init()', 200);
		}
	}

    function copyData() {
		for (var i=0;i<2;i++) {
			document.configForm.elements[i+5].value = document.secAuthenticationForm.elements[i].value;
		}
	}
	
	function form_verification() {
		continue_verification = true;
		var form_location = document.secAuthenticationForm;
		if (continue_verification == true) {
			checkElement (form_location.elements[0], true, true, false, false);
		}
		if (continue_verification == true) {
			checkElement (form_location.elements[1], true, false, true, false);
		}
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
<div id="nisOK" style="display:none;">
<table align="center" cellspacing="10" cellpadding="0" border="0">
	<tr>
		<td align="left">&nbsp;</td>
		<td align="left">&nbsp;</td>
	</tr>
</table>
<table width="360" border="0" align="center" cellpadding="0" cellspacing="10">
	<tr align="left" valign="top">
		<td align="left" width="180">
			<font class="label">NIS Domain Name</font>
		</td>
		<td align="left" width="180">
			<input name="nis_domain_name" type="text" maxlength="39" size="15" class="formText" value="<% get("system.security.nisP.nisDomainName");%>">
		</td>
	</tr>
	<tr align="left" valign="top">
		<td align="left" width="180">
			<font class="label">NIS Server IP</font>
		</td>
		<td align="left" width="180">
			<input name="nis_server_ip" type="text" maxlength="15" size="15" class="formText" value="<% get("system.security.nisP.nisServerIp");%>" >
		</td>
	</tr>
</table>
</div>
<div id="nisNOK" style="display:none;">
<table align="center" height="100%" cellspacing="0" cellpadding="0" border="0">
	<tr>
		<td height="100%" align="center">
			<table height="100%" width="100%" cellpadding="0" cellspacing="0" border="0">
				<tr height="100%" align="center">
					<td align="center" valign="middle">
						<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">
							<tr>
								<td align="center" valign="middle">
									<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
										<tr>
											<td align="center" valign="middle" class="label_for_tables">
												You have selected a service that is currently not supported in IPv6.<br>
												If you like to use this service please enable IPv4 for NIS to work
											</td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</div>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="0">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/security/secNisConf.asp">
	<input type="hidden" name="urlError" value="/normal/security/secNisConf.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name="system.security.nisP.nisDomainName" value="">
	<input type="hidden" name="system.security.nisP.nisServerIp" value="">
</form>

<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
