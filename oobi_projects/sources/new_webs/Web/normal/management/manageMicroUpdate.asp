<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

<% ReqNum = getRequestNumber(); %>;
	
    <% if (getVal('_familymodel_') == "ONS")
        var Sports = getVal("system.device.number2");
        else
        var Sports = getVal("system.device.number");
    %>

	var expander = <% getLabel("system.kvm.expander");%>;
	var ports = <% write(Sports);%>;
	
	parent.currentPage = self;
	parent.helpPage = 'helpPages/microUpdateHelp.asp';
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function init()
	{   
    	if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    	{
	    var message = "<%get("system.kvm.uCupdate.sysMsg");%>";

            if (checkLeftMenu('/normal/management/manageMenu.asp') == 0) {
               parent.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
            parent.topMenu.selectItem(parent.menu.topItem);
            parent.menu.selectItem("MUP");
      		setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
		if (message != "") alert(message);
    	}
    	else
      		setTimeout('init()', 200);
	}

	function showHidePort()
	{
		if (portVal.length == 0) {
			document.microUpdateForm.targets[0].checked = false;
			document.microUpdateForm.targets[0].disabled = true;
		} else {
			document.microUpdateForm.targets[0].disabled = false;
		}

		if (expander) {
			if (cascVal.length == 0) {
				document.microUpdateForm.targets[4].checked = false;
				document.microUpdateForm.targets[4].disabled = true;
			} else {
				document.microUpdateForm.targets[4].disabled = false;
			}
		}

		if (document.microUpdateForm.targets[0].checked == true) {
			document.microUpdateForm.ports.length = portVal.length;
			for (var i = 0; i < portVal.length; i++) {
				document.microUpdateForm.ports.options[i].value = portVal[i];
				document.microUpdateForm.ports.options[i].text = portName[i];	
			}
			document.microUpdateForm.ports.disabled = false;
			document.microUpdateForm.ports.style.visibility = 'visible';
			return;
		}

		if (expander) {
			if (document.microUpdateForm.targets[4].checked == true) {
				document.microUpdateForm.ports.length = cascVal.length;
				for (var i = 0; i < cascVal.length; i++) {
					document.microUpdateForm.ports.options[i].value = cascVal[i];
					document.microUpdateForm.ports.options[i].text = cascName[i];
				}
				document.microUpdateForm.ports.disabled = false;
				document.microUpdateForm.ports.style.visibility = 'visible';
				return;
			}
		}
		document.microUpdateForm.ports.length = 0;
		document.microUpdateForm.ports.disabled = true;
		document.microUpdateForm.ports.style.visibility = 'hidden';
	}

	function form_verification()
	{
		var form_location = document.microUpdateForm;

	  	continue_verification = true;
		checkElement(form_location.ftpServer, true, true, false, false);
		if (continue_verification == true)
			checkElement (form_location.user, true, true, false, false);
		if (continue_verification == true)
			checkElement (form_location.password, true, true, false, false);
		if (continue_verification == true)
			checkElement (form_location.directory, true, true, false, false);
		if (continue_verification == true)
			checkElement (form_location.fileName, true, true, false, false);												 
	}

    function copy_data()
    {
		for (var i = 0; i < document.microUpdateForm.targets.length; i++) {
			if (document.microUpdateForm.targets[i].checked == true) {
				document.configForm.elements[5].value =
					document.microUpdateForm.targets[i].value;
				break;
			}
		}

		if (i >= document.microUpdateForm.targets.length)
			document.configForm.elements[5].value = 99;

		document.configForm.elements[6].value = '';
		for (var i = 0; i < document.microUpdateForm.ports.length; i++) {
			if (document.microUpdateForm.ports.options[i].selected)
				document.configForm.elements[6].value += document.microUpdateForm.ports.options[i].value+' ';
       	}

		document.configForm.elements[8].value = document.microUpdateForm.ftpServer.value;
		document.configForm.elements[9].value = document.microUpdateForm.user.value;
		document.configForm.elements[10].value = document.microUpdateForm.password.value;
		document.configForm.elements[11].value = document.microUpdateForm.directory.value;
		document.configForm.elements[12].value = document.microUpdateForm.fileName.value;
	}

	function upGradeNow()
	{
		form_verification();
		if (continue_verification == true)
		{
			copy_data();
			document.configForm.elements[7].value = 1;
			setQuerySSID(document.configForm);
    	    document.configForm.submit();
			alert("Please wait. This process can take some minutes.\nA message will be prompted at process completion.");
			parent.mainReady =0;
		}
	}

    function submit()
    { 
		copy_data();
   	    document.configForm.submit();
		parent.mainReady = 0;
    }

	</script>
</head>
<body onload="init(); showHidePort();" class="bodyMain">
<table align="center" height="100%" cellspacing="0" cellpadding="0" border="0">
<form name="microUpdateForm">
<tr>
	<td>
<table align="center" cellspacing="0" cellpadding="3" border="0">
<tr>
	<td valign="middle" align="left" width="60"><font class="label">Target:</font></td>
<td align="center">
<table align="center" cellspacing="0" cellpadding="3" border="0">
<tr>
	<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="0" onclick="showHidePort();"></td>
	<td align="left" height="20" valign="middle"><font class="label">KVM Terminator</font></td>
</tr>
<tr>
	<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="1" onclick="showHidePort();"></td>
	<td align="left" height="20" valign="middle"><font class="label">KVM Switch (internal)</font></td>
</tr>
<script type="text/javascript">
if (top.family_model != "ONS") {
	document.write('<tr>\
	<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="2" onclick="showHidePort();"></td>\
	<td align="left" height="20" valign="middle"><font class="label">KVM RP Main</font></td>\
</tr>\
<tr>\
	<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="3" onclick="showHidePort();"></td>\
	<td align="left" height="20" valign="middle"><font class="label">KVM RP Local</font></td>\
</tr>');
	if (expander) {
		document.write('<tr>\
	<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="4" onclick="showHidePort();"></td>\
	<td align="left" height="20" valign="middle"><font class="label">KVM Port Expander Module</font></td>\
</tr>');
	}
}
</script>
<%
if (getVal("system.device.kvmanalog") == 0) {
	write('<tr>\n');
	write('<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="5" onclick="showHidePort();"></td>\n');
	write('<td align="left" height="20" valign="middle"><font class="label">KVM Video Compression Modules</font></td>\n');
	write('</tr>\n');
}
%>
</table>
</td>
	<td valign="top" width="150" align="center">
	<select name="ports" size="6" class="formText" multiple>
	</select>
	</td>
</tr>
<table align="center" cellspacing="0" cellpadding="3" border="0">
<tr>
	<td align="center" height="30" ><font class="label">FTP Server</font></td>
	<td align="left"><input type="text" name="ftpServer" size="15" maxlength="39" class="formText"
	value="<%get("system.kvm.uCupdate.ftpserver");%>"></td>
</tr>
<tr>
	<td align="left" height="30"><font class="label">User</font></td>
	<td align="left"><input type="text" name="user" size="15" maxlength="30" class="formText"
	value="<%get("system.kvm.uCupdate.username");%>"></td>
</tr>
<tr>	
	<td align="left"><font class="label">Password</font></td>
	<td align="left"><input type="password" name="password" size="15" maxlength="30" class="formText"
	value="<%get("system.kvm.uCupdate.password");%>"></td>
</tr>
<tr>
	<td align="left" height="30"><font class="label">Directory</font></td>
	<td align="left"><input type="text" name="directory" size="15" maxlength="99" class="formText"
	value="<%get("system.kvm.uCupdate.directory");%>"></td>
</tr>
<tr>	
	<td align="left"><font class="label">File Name</font></td>
	<td align="left"><input type="text" name="fileName" size="15" maxlength="99" class="formText"
	value="<%get("system.kvm.uCupdate.filename");%>"></td>
</tr>
<tr>
	<td colspan="2" align="center" height="40" valign="bottom">
	<a href="javascript:upGradeNow();">
	<img src="/Images/upgrade_nowButton.gif" alt="" width="80" height="21" border="0"></a></td>
</tr>
</table>
	</td>
</tr>	
</form>

<script type="text/javascript">
	var portName = new Array(<%get("system.kvm.uCupdate.portsNameHtml");%>);
	var portVal = new Array(<%get("system.kvm.uCupdate.portsValHtml");%>);
	var cascName = new Array(<%get("system.kvm.uCupdate.cascNameHtml");%>);
	var cascVal = new Array(<%get("system.kvm.uCupdate.cascValHtml");%>);
	showHidePort();
</script>	
<form name="configForm" method="POST" action="/goform/Dmf">
<!--these hidden inputs are used to store values of form elements inside layers--> 
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/management/manageMicroUpdate.asp">
   <input type="hidden" name="urlError" value="/normal/management/manageMicroUpdate.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
<!--***************************************************************************-->
   <input type="hidden" name="system.kvm.uCupdate.targettype" value="">
   <input type="hidden" name="system.kvm.uCupdate.targetport" value="">
   <input type="hidden" name="system.kvm.uCupdate.action" value="99">
   <input type="hidden" name="system.kvm.uCupdate.ftpserver" value="">
   <input type="hidden" name="system.kvm.uCupdate.username" value="">
   <input type="hidden" name="system.kvm.uCupdate.password" value="">
   <input type="hidden" name="system.kvm.uCupdate.directory" value="">
   <input type="hidden" name="system.kvm.uCupdate.filename" value="">
</form>
</table>
<script type="text/javascript">
	parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
