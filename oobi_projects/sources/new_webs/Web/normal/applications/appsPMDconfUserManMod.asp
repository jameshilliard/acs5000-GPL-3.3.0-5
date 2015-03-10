<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	top.currentPage = self;
	var target='&target=' + parent.window.name;
	var sysMsg = "<%get("system.applications.pmdUserMan.sysMsg");%>";

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

	function updateEntries()
	{
		var object = document.PMDconfUserManForm.users_man_table;

		if (document.configForm.elements[5].value == '') {
			for (var j=0; j<object.length; j++) {
				if (object.options[j].value != '-1')
					document.configForm.elements[5].value += object.options[j].value + ' ';
      		}
			document.configForm.elements[5].value += 'end';
        }
	}

	function addUserEntry()
	{
		var newRef = "appsPMDconfUserManEntry.asp?index=-1"+target+"&SSID=<%get("_sid_");%>";

		if (top.mainReady == null) return;
		if (document.configForm.elements[0].value != "") {
			return;
		}
		updateEntries();
		newWindow(newRef,'name','550','250','yes');
	}

	function editUserEntry()
	{
		var object = document.PMDconfUserManForm.users_man_table;

		if (top.mainReady == null) return;
		if (document.configForm.elements[0].value != "") {
			return;
		}
		if (object.selectedIndex == -1 || object.options[object.selectedIndex].value == '-1')
		{
			alert('Please select an item to edit.');
		}
		else
		{
		var newRef = "appsPMDconfUserManEntry.asp?index="+object.options[object.selectedIndex].value+target+"&SSID=<%get("_sid_");%>";
			updateEntries();
			newWindow(newRef,'name','550','250','yes');		
		}
	}

	function delUserEntry()
	{
		var object = document.PMDconfUserManForm.users_man_table;

		if (document.configForm.elements[0].value != "") {
			return;
		}
		deleteSelOpt(object);
	}

	function submit() {
		updateEntries();
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = false;
	}

	</script>
</head>

<body class="bodyForLayers" onload="init();">
<form name="PMDconfUserManForm" method="POST" action=" " onSubmit="return false;">

<table class="tableColor" align="center" border="0" cellspacing="0" cellpadding="0">
	<tr align="left" height="18" valign="center">
		<td width="156"><font class="tabsTextBig">&nbsp;User</font></td>
		<td><font class="tabsTextBig">Outlets</font></td>
	</td></tr>
	<tr><td colspan="2">
		<!-- options width is "1"(15); 2whitespaces; "2"(28);-->
		<select name="users_man_table" size="12" class="formText">
<script type="text/javascript">
document.write('<%get("system.applications.pmdUserMan.usersListHtml");%>');
</script>
			<option value="-1">
<!-- 45 spaces (HTML newlines counts as one space !!!) -->
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
		</select>
	</td></tr>
</table>

<table align="center" width="100%" border="0" cellspacing="0" cellpadding="0">
<tr>
	<td align="center" valign="middle" height="40">
		<a href="javascript:addUserEntry()">
		<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
	  &nbsp;&nbsp;&nbsp;&nbsp;
		<a href="javascript:editUserEntry()">
		<img src="/Images/editButton.gif" alt="" width="47" height="21" border="0"></a>
	  &nbsp;&nbsp;&nbsp;&nbsp;
		<a href="javascript:document.configForm.elements[5].value = ''; delUserEntry()">
		<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
	</td>
</tr>
</table>

</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/appsPMDconfUserMan.asp">
	<input type="hidden" name="urlError" value="/normal/applications/appsPMDconfUserMan.asp">
	<input type="hidden" name="request" value="0">
	<!--*******************************************************************-->
	<input type="hidden" name="system.applications.pmdUserMan.entries"
		value="<%get("system.applications.pmdUserMan.entries");%>">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
</body>
<%set("_sid_","0");%>
</html>
