<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	top.currentPage = self;
	var sysMsg = parent.sysMsg;

	<% ReqNum = getRequestNumber(); %> 
window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function init()	{
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			document.configForm.elements[7].value = parent.memSign;
			if (sysMsg != 'OK') {
				if (sysMsg && (sysMsg[0] != '\0'))
					alert(sysMsg);
				else
					alert("Error in Retrieving.");
			}
		}
		else
			setTimeout('init()', 200);
	}

	function RefreshLatestSw()
    {
		document.configForm.elements[5].value = 10; //Refresh_LatestSw
		document.configForm.elements[6].value = '';
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
    }

	function UpgradeSw(index)
    {
		if (top.mainReady != false) {
			if (confirm("After you press OK, please be patient!\nThe Upgrade action will take a while...")) {
				document.configForm.elements[5].value = 11; //Upgrade_UnitFw
				document.configForm.elements[6].value = swupInfo[index][5];
				setQuerySSID(document.configForm);
				document.configForm.submit();
				top.mainReady = false;
			}
		}
    }

	function submit()
	{
		document.configForm.elements[5].value = 0; //no_Action
		setQuerySSID(document.configForm);
        document.configForm.submit();
		top.mainReady = false;
	}

</script>
</head>

<body class="bodyForLayers" onload="init();">
<form name="PMswUpgradeForm" method="POST" action=" " onSubmit="return false;">
<script type="text/javascript">
var family_model = parent.family_model;

if ((family_model != 'ACS')&&(family_model != 'AVCS')&&(family_model != 'ONS'))
	alert('This page is not prepared to run with '+family_model+'.');

var validIpdus = parent.validIpdus;

var swupInfo = parent.swupInfo;

var latestSW = parent.latestSW;

if (validIpdus) {

document.write('\
<table align="center" width="375" height="40" border="0" cellspacing="0" cellpadding="0">\
<tr>\
	<td valign="middle" align="left" width="295">\
		<font class="label">&nbsp;Latest software version available:&nbsp;'+latestSW+'</font></td>\
	<td align="center" width="80"><a href="javascript:RefreshLatestSw();">\
		<img src="/Images/refresh_button2.gif" alt="" width="70" height="21" border="0"></a></td>\
</tr>\
</table>');

	for (var j=0; j<validIpdus; j++)
	{
document.write('\
<table class="colorTHeaderAlt" align="center" width="375" height="20" border="0" cellspacing="0" cellpadding="0">\
<tr>\
	<td align="left">&nbsp;ID: '+swupInfo[j][0]+' ('+swupInfo[j][1]+')</td></td>\
</tr>\
<tr>\
	<td align="left">&nbsp;Model: '+swupInfo[j][2]+' '+swupInfo[j][3]+'&nbsp;</td></td>\
</tr>\
</table>\
<table align="center" width="375" border="0" cellspacing="0" cellpadding="0">\
<tr>\
	<td align="left" valign="middle" width="295" height="30">\
		<font class="label">&nbsp;Software Version:&nbsp;'+swupInfo[j][4]+'</font></td>\
	<td align="center" width="80">');
if (swupInfo[j][4] < latestSW)
document.write('\
<a href="javascript:UpgradeSw('+j+');"\
		class="tableButtons" style="padding: 0px 1px 0px 1px" onfocus="blur()">Upgrade</a>');
document.write('\
	</td>\
</tr>\
</table>');
	}
} else {
document.write('\
	<table align="center" height="88%" width="100%" cellspacing="0" cellpadding="0" border="0">\
	<tr valign="middle">\
		<td>\
		<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">\
		<tr>\
			<td>\
			<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">\
			<tr>\
				<td align="center" class="label_for_tables">\
					There is no Cyclades IPDU under management at this moment.\
				</td>\
			</tr>\
			</table>\
			</td>\
		</tr>\
		</table>\
		</td>\
	</tr>\
	</table>');
}
</script>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMsoftUpgrade.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsPMsoftUpgrade.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.IpduSwUpgrade.action" value="">
	<input type="hidden" name="system.applications.IpduSwUpgrade.sysMsg" value="">
	<input type="hidden" name="system.applications.IpduSwUpgrade.memSignature" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
</body>
<%set("_sid_","0");%>
</html>
