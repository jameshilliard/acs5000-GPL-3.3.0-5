<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">
	var ReqNum = <%write(getRequestNumber());%>;
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_' + ReqNum;
	var family_model = '<%get("_familymodel_");%>';
<%
	var nIpdus = getVal("system.applications.IpduOutMan.numIpdus");
	var level2 = getVal("system.applications.IpduOutMan.showIpdu");
	var name;
	var model;
	var numOut;
	var outInfo;
	var outCap;
	var isST;
	if (level2 != 0) {
		name = getVal("system.applications.IpduOutMan.ipduName_str");
		model = getVal("system.applications.IpduOutMan.ipduModel_str");
		numOut = getVal("system.applications.IpduOutMan.ipduNofOut");
		outInfo = getVal("system.applications.IpduOutMan.outletInfoHtml"); 
		outCap = getVal("system.applications.IpduOutMan.outcapInfoHtml");
		isST = getVal("system.applications.IpduOutMan.isServerTech");
	} else { 
		name = "not-used";
		model = "not-used";
		numOut = 0;
		outInfo = "\'not-used\'";
		outCap = "\'not-used\'";
		isST = 0;
	}
%>

</script>
</head>

<script type="text/javascript">

//we fetch all "system.applications.IpduOutMan..." in this page to avoid a second call
//to GetIpduOutMan() while loading appsPMOutletsManMod2nd.asp or appsPMOutletsManMod.asp pages

var num_ipdus = <%write(nIpdus);%>;

if (num_ipdus > 0) {

var sysMsg = "<%get("system.applications.IpduOutMan.sysMsg");%>";
var memSign = "<%get("system.applications.IpduOutMan.memSignature");%>";
var showingIpdu = <%write(level2);%>;
var nPorts = <%get("system.applications.IpduOutMan.numPorts");%>;
var portInfo = <%get("system.applications.IpduOutMan.portInfoHtml");%>;
var ipduInfo = <%get("system.applications.IpduOutMan.ipduInfoHtml");%>;
var ipduName_str = "<%write(name);%>";
var ipduModel_str = "<%write(model);%>";
var ipduNumberOfOutlets = <% write(numOut);%>;
var outlinfo = <% write(outInfo);%>;
var outcap = <% write(outCap);%>;
var isServerTech = "<% write(isST);%>";
if (family_model == 'ONS' ) {
	var serialPorts = <%get("system.device.number2");%>;
}

	document.write (
		'<frameset  rows="22,*" border="0" frameborder="0">',
		'<frame name="tabs" src="appsPMOutletsManTab.asp?SSID=<%get('_sid_');%>" marginwidth="0"',
		' marginheight="0" scrolling="no" frameborder="0" noresize>');
	if (!showingIpdu)
		document.write (
			'<frame name="page" src="appsPMOutletsManMod.asp?SSID=<%get('_sid_');%>" marginwidth="0"',
			' marginheight="0" scrolling="auto" frameborder="0" noresize>',
			'</frameset>');
	else
		document.write (
			'<frame name="page" src="appsPMOutletsManMod2nd.asp?SSID=<%get('_sid_');%>" marginwidth="0"',
			' marginheight="0" scrolling="auto" frameborder="0" noresize>',
			'</frameset>');
} else {

	function init()	{
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			if (checkLeftMenu('/normal/applications/appsMenu.asp') == 0) {
				top.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			top.topMenu.selectItem(top.menu.topItem);
			top.menu.selectItem("PWIPDU");
			if (top.access != "restricted") // for regular user
				setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
		}
		else
			setTimeout('init()', 200);
	}

	top.currentPage = self;
	top.helpPage = '/normal/helpPages/appsPMOutletsManHelp.asp';

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
			<font class="tableTextInactive">View <%getLabel("system.application.IPDU");%>s Info</font></td>');
	if (top.access == "restricted")
	{
document.write('\
		<td class="bgColor9" align="center" width="40%" nowrap>&nbsp;</td>\
	</tr><tr>\
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		<td class="bgColor2" colspan="5" width="60%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');
	} else {
document.write('\
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
		<td class="bgColor2" width="20%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td></tr>');
	}
document.write('\
	</table>\
	<table align="center" height="88%" width="100%" cellspacing="0" cellpadding="0" border="0">\
	<tr valign="middle">\
		<td>\
		<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">\
		<tr>\
			<td>\
			<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">\
			<tr>\
				<td align="center" class="label_for_tables">');
document.write('There is no IPDU available to be managed, either because<br>\
				the serial ports are not configured for Power Management protocol,<br>\
				or the <%getLabel("nickname");%> did not detect any IPDU');
	if (top.access == "restricted")
	{
document.write('<br> \
		or the current user does not have permission to manage any outlet');
	}
document.write('.\
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
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMOutletsMan.asp">\
	<input type="hidden" name="urlError" value="/normal/applications/appsPMOutletsMan.asp">\
	<input type="hidden" name="request" value="0">\
	<input type="hidden" name="system.tablePage.bogus" value="">\
</form>\
</body>');
top.mainReady = true;
}
</script>
</html>
