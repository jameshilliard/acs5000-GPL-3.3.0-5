<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
  
<% ReqNum = getRequestNumber(); %>

	<% if (getVal('_familymodel_') == "ONS") 
		var Sports = getVal("system.device.number2");
		else
		var Sports = getVal("system.device.number");
	%>
	var ports = <% write(Sports);%>;
	var expander = <% getLabel("system.kvm.expander");%>;

	parent.currentPage = self;
	parent.helpPage = 'helpPages/microResetHelp.asp';
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function init()
	{   
    	if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    	{
			var message = '<%get("system.kvm.uCreset.sysMsg");%>';

            if (checkLeftMenu('/normal/management/manageMenu.asp') == 0) {
               parent.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
			setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
            parent.topMenu.selectItem(parent.menu.topItem);
            parent.menu.selectItem("MR");
			if (message != '') alert(message);
		}
		else
			setTimeout('init()', 200);
	}

	function showHidePort()
	{
		if (portVal.length == 0) {
			document.microResetForm.targets[0].checked = false;
			document.microResetForm.targets[0].disabled = true;
		} else {
			document.microResetForm.targets[0].disabled = false;
		}
		if (expander) {
			if (cascVal.length == 0) {
				document.microResetForm.targets[4].checked = false;
				document.microResetForm.targets[4].disabled = true;
			} else {
				document.microResetForm.targets[4].disabled = false;
			}
		}

		if (document.microResetForm.targets[0].checked == true) {
			document.microResetForm.ports.length = portVal.length;
			for (var i = 0; i < portVal.length; i++) {
				document.microResetForm.ports.options[i].value = portVal[i];
				document.microResetForm.ports.options[i].text = portName[i];	
			}
			document.microResetForm.ports.disabled = false;
			document.microResetForm.ports.style.visibility = 'visible';
			return;
		}

		if (expander) {
			if (document.microResetForm.targets[4].checked == true) {
				document.microResetForm.ports.length = cascVal.length;
				for (var i = 0; i < cascVal.length; i++) {
					document.microResetForm.ports.options[i].value = cascVal[i];
					document.microResetForm.ports.options[i].text = cascName[i];
				}
				document.microResetForm.ports.disabled = false;
				document.microResetForm.ports.style.visibility = 'visible';
				return;
			}
		}

		document.microResetForm.ports.length = 0;
		document.microResetForm.ports.disabled = true;
		document.microResetForm.ports.style.visibility = 'hidden';
	}

    function copy_data()
    {
		for(var i=0; i < document.microResetForm.targets.length; i++) {
			if (document.microResetForm.targets[i].checked == true)	{
				document.configForm.elements[5].value =
					document.microResetForm.targets[i].value;
				break;
			}
		}

  		if (i >= document.microResetForm.targets.length)	
			document.configForm.elements[5].value = 99;

		document.configForm.elements[6].value = '';
		for (var i = 0; i < document.microResetForm.ports.length; i++) {
			if (document.microResetForm.ports.options[i].selected)
				document.configForm.elements[6].value += document.microResetForm.ports.options[i].value+' ';
       		}
	}

	function resetNow()
	{
		copy_data();
		document.configForm.elements[7].value = 0;
		setQuerySSID(document.configForm);
		document.configForm.submit();
		parent.mainReady =0;
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
<form name="microResetForm">
<tr>
	<td>	
	<table align="center" cellspacing="0" cellpadding="3" border="0">
<tr>
	<td valign="middle" align="center"><font class="label">Target:</font></td>
	<td align="center" valign="middle">
	<table align="center" cellspacing="0" cellpadding="3" border="0">
	<tr>
	<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="0" onclick="showHidePort();"></td>
	<td align="left" height="20" valign="middle"><font class="label">KVM Terminator</font></td>
</tr><tr>
	<td align="left" height="20" valign="middle"><input type="radio" name="targets" value="1" onclick="showHidePort();"></td>
	<td align="left" height="20" valign="middle"><font class="label">KVM Switch (internal)</font></td>
</tr>
<script type="text/javascript">
if (top.family_model != "ONS")
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
	<td valign="top" width="150" align="center" rowspan="2">
	<select name="ports" size="6" class="formText" multiple>
	</select>
	</td>
<tr>
    <td></td>
	<td align="center" height="40" valign="bottom">
	<a href="javascript:resetNow();"><img src="/Images/reset_nowButton.gif" alt="" width="80" height="21" border="0"></a></td>
    <td></td>
</tr>
</table>
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
   <input type="hidden" name="urlOk" value="/normal/management/manageMicroReset.asp">
   <input type="hidden" name="urlError" value="/normal/management/manageMicroReset.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
<!--***************************************************************************-->
   <input type="hidden" name="system.kvm.uCreset.targettype" value="">
   <input type="hidden" name="system.kvm.uCreset.targetport" value="">
   <input type="hidden" name="system.kvm.uCreset.action" value=99>
</form>
</table>
<script type="text/javascript">
	parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
