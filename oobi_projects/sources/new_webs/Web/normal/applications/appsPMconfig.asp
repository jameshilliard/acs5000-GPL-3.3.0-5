<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">
	var family_model = '<%get("_familymodel_");%>';
</script>
</head>

<script type="text/javascript">

	<%
		var nIpdus = getVal("system.applications.IpduConfig.numIpdus");
		var ipduConf;
		var confcap;

		if (nIpdus > 0) {
			ipduConf = getVal("system.applications.IpduConfig.ipduInfoHtml");
			confcap = getVal("system.applications.IpduConfig.confcapHtml");
		} else {
			ipduConf = "\'none\'";
			confcap = "\'none\'";
		}
	%>
//we fetch all "system.applications.IpduConfig..." in this page to avoid a second call
//to GetIpduConfig() while loading appsPMconfigMod.asp

var nIpdus = <% write(nIpdus); %>;

if (nIpdus > 0) {

var sysMsg = "<%get("system.applications.IpduConfig.sysMsg");%>";
var memSign = "<%get("system.applications.IpduConfig.memSignature");%>";
var ipduConf = <%write(ipduConf);%>;
var confcap = <%write(confcap);%>;

<!-- frames -->
	document.write(
'<frameset  rows="22,*" border="0" frameborder="0">',
	'<frame name="tabs" src="appsPMconfigTab.asp?SSID=<%get("_sid_");%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>',
	'<frame name="page" src="appsPMconfigMod.asp?SSID=<%get("_sid_");%>" marginwidth="0" marginheight="0" scrolling="auto" frameborder="0" noresize>',
'</frameset>');

} else {

	function init()	{
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
		}
		else
			setTimeout('init()', 200);
	}

	top.currentPage = self;
	top.helpPage = '/normal/helpPages/appsPMconfigHelp.asp';

	function submit()
	{
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

document.write('\
<body class="bodyMainTabs" onload="init();">\
	<table border="0" cellpadding="0" cellspacing="0" width="102%">\
	<tr>\
		<td class="bgColor9" align="center" width="20%" nowrap>\
			<font class="tableTextInactive">Outlets Manager</font></td>\
		<td class="bgColor9" align="left" height="20" width="1">\
			<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>\
		<td class="bgColor9" align="center" width="20%" nowrap>\
			<font class="tableTextInactive">Outlet Groups Ctrl</font></td>\
		<td class="bgColor9" align="left" height="20" width="1">\
			<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>\
		<td class="bgColor9" align="center" width="20%" nowrap>\
			<font class="tableTextInactive">View <%getLabel("system.application.IPDU");%>s Info</font></td>\
		<td class="bgColor9" align="left" height="20" width="1">\
			<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>\
		<td class="bgColor9" align="center" width="20%" nowrap>\
			<font class="tableTextInactive">Configuration</font></td>\
		<td class="bgColor9" align="left" height="20" width="1">\
			<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>\
		<td class="bgColor9" align="center" width="20%" nowrap>\
			<font class="tableTextInactive">Software Upgrade</font></td>\
	</tr><tr>\
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
	</tr>\
	</table>\
	<table align="center" height="88%" width="100%" cellspacing="0" cellpadding="0" border="0">\
	<tr valign="middle">\
		<td>\
		<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">\
		<tr>\
			<td>\
			<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">\
			<tr>\
				<td align="center" class="label_for_tables">\
					There is no IPDU available to be managed.<br>\
					The <%getLabel("nickname");%> has lost IPDU communication.\
				</td>\
			</tr>\
			</table>\
			</td>\
		</tr>\
		</table>\
		</td>\
	</tr>\
	</table>\
<form name="configForm" method="POST" action="/goform/Dmf">\
	<input type="hidden" name="system.req.action" value="">\
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">\
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMconfig.asp">\
	<input type="hidden" name="urlError" value="/normal/applications/appsPMconfig.asp">\
	<input type="hidden" name="request" value="0">\
	<input type="hidden" name="system.tablePage.bogus" value="">\
</form>\
</body>');
top.mainReady = true;
}
</script>
</html>
