<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	top.currentPage = self;
	var sysMsg = "<%get("system.applications.pmdGen.sysMsg");%>";

	<% ReqNum = getRequestNumber(); %>
window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function init()	{
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			if (checkLeftMenu('/normal/applications/appsMenu.asp') == 0) {
				top.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			top.topMenu.selectItem(top.menu.topItem);
			top.menu.selectItem("PMDC");
			setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			if (sysMsg != 'OK') {
				if (sysMsg && (sysMsg[0] != '\0'))
					alert(sysMsg);
				else
					alert("Error in Retrieving.");
			}
		} else {
			setTimeout('init()', 200);
		}
	}

	function form_verification() {
		continue_verification = true;
		for (var p=0; p<numVendors; p++) {
		var	elmname = 'username_'+ p;
			if (continue_verification == true)
			checkElement (document.PMDconfGenForm.elements[elmname], false, true, false, false);
		elmname = 'passwd_'+ p;
			if (continue_verification == true)
			checkElement (document.PMDconfGenForm.elements[elmname], false, true, false, false);
		}
	}

	function prepareData() {
		for (var p=0; p<numVendors; p++) {
		var elmname = 'username_'+ p;
			if (document.PMDconfGenForm.elements[elmname].value == '')
				document.configForm.elements[5].value += 'not-defined' + ' ';
			else
				document.configForm.elements[5].value +=
					document.PMDconfGenForm.elements[elmname].value + ' ';
		elmname = 'passwd_'+ p;
			if (document.PMDconfGenForm.elements[elmname].value == '')
				document.configForm.elements[5].value += 'not-defined' + ' ';
			else
				document.configForm.elements[5].value +=
					document.PMDconfGenForm.elements[elmname].value + ' ';
		}
	}

	function submit() {
		form_verification();
		if (continue_verification == true) {
			prepareData();
			setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = false;
		}
	}

	</script>
</head>

<body class="bodyForLayers" onload="init();">
<form name="PMDconfGenForm" method="POST" action=" " onSubmit="return false;">
<script type="text/javascript">

//this number shouldn't ever be 0 (middleware controls it)
var numVendors = 3;

//The idea of using this flexible "array method" even for something that
//right now is fixed (3 vendors) is to facilitate future implementations.
//So all control is under middleware scope.
var confInfo = <%get("system.applications.pmdGen.confInfoHtml");%>
//var confInfo = [['Cyclades','cycla','cypass'],
//				['SPC','spc','spcpass'],
//				['ServerTech','servtech','stpass']];
		// confInfo is [[0. vendor
		//				 1. username(str),
		//				 2. passwd(str)],...]

for (var j=0; j < numVendors; j++) {
document.write('\
	<table width="460" border="0" cellpadding="0" cellspacing="0" align="center">\
		<tr>\
			<td height="30" colspan="2" valign="bottom"><font class="tabsTextBig">'+confInfo[j][0]+'</font></td>\
		</tr>\
		<tr><td height="5"></td></tr>\
		<tr>\
			<td valign="bottom" width="230"><font class="label">Username</font></td>\
			<td valign="bottom" width="230"><font class="label">Password</font></td>\
		</tr>\
		<tr>\
			<td valign="top" width="230"><input class="formText" type="text" maxlength="30" name="username_'+j+'" size="15"\
				value="'+confInfo[j][1]+'"></td>\
			<td valign="top" width="230"><input class="formText" type="text" maxlength="30" name="passwd_'+j+'" size="15"\
				value="'+confInfo[j][2]+'"></td>\
		</tr>\
	</table>');
}

</script>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMDconfGen.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsPMDconfGen.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.pmdGen.sysMsg" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
</body>
<%set("_sid_","0");%>
</html>
