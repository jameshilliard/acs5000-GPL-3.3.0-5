<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Configure RDP servers</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
<%
	var deviceSel = getQueryVal("deviceSel", -1);
	if (deviceSel == "-1") {
		action = "Add";
        	set("system.rdpconf.device.deviceSel",0);
	} else {
		action = "Edit";
        	set("system.rdpconf.device.deviceSel",deviceSel);
	}
%>
	var action = "<% write(action);%>";
	var ReqNum = opener.document.configForm.elements[4].value;

	function form_verification()
	{	   	  
		continue_verification = true;
	  	if (checkPortDevice (document.deviceEntryForm.devName)) {
			alert ('Invalid value for the server name.');
			continue_verification = false;
		}
		if (continue_verification == true) {
		  	if (checkElement (document.deviceEntryForm.devSrvport, false, false, false, true, 1024, 65525, false)) {
				continue_verification = false;
			}
		}
// continue verification ipaddr, kvmport ...
	}	  

	function init()
	{
	}

	function modifyOK()
    {                                    
	try {
		if (opener.window.name != document.configForm.target) {
			alert("The page which called this form is not available anymore. The changes will not be effective.");
			self.close();
			return;
		}
		form_verification();
		if (continue_verification == true) {	
			document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
			document.configForm.elements[5].value = opener.document.configForm.elements[5].value;
			document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
			document.configForm.elements[11].value = opener.document.configForm.elements[8].value;

			document.configForm.elements[7].value = document.deviceEntryForm.devName.value;
			document.configForm.elements[8].value = document.deviceEntryForm.devIpaddr.value;
			document.configForm.elements[9].value = document.deviceEntryForm.devSrvport.value;
			document.configForm.elements[10].value = document.deviceEntryForm.devKVMport.value;
			setQuerySSID(document.configForm);
			document.configForm.submit(); 
			self.close();
		}
	} catch (error) {
	  self.close();
	}
    }

	</script>
</head>
<body class="bodyForLayers">
<form name="deviceEntryForm" onSubmit="return false;">
   	<table border="0" cellpadding="5" cellspacing="0" align="center">
	   	<tr> 
          	<td colspan="2" align="center"  valign="top">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
	<td><a href="javascript:modifyOK();"><img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a></td>
	<td><a href="javascript:window.close();"><img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a></td>
</tr>
</table>			
          	</td>
        </tr>
	</table>
	<table border="0" cellpadding="5" cellspacing="0" align="center">
	   	<tr> 
          	<td width="120"><font class="label">Server Name</font></td>
		<td>
		<script>
		if (opener.document.configForm.elements[6].value == '3') {
          		document.write('<input name="devName" type="hidden" value="<%get("system.rdpconf.device.deviceConf.name");%>">');
			document.write('<font class="label"><%get("system.rdpconf.device.deviceConf.name");%></font>');
		} else {
          		document.write('<input name="devName" type="text" size="20" maxlength="19" class="formText" value="<%get("system.rdpconf.device.deviceConf.name");%>">');
		}
		</script>
		</td>
		</tr>
		<tr>
		<td><font class="label">IP Address</font></td>
          	<td><input name="devIpaddr" type="text" size="20" maxlength="40" class="formText" value="<%get("system.rdpconf.device.deviceConf.ipaddr");%>"></td>
        </tr>
		<tr>
		<td><font class="label">Server Port</font></td>
          	<td><input name="devSrvport" type="text" size="20" maxlength="5" class="formText" value="<%get("system.rdpconf.device.deviceConf.srvport");%>"></td>
        </tr>
		<tr>
		<td><font class="label">KVM Port</font></td>
          	<td><select name="devKVMport" class="formText">
			<% get("system.rdpconf.device.RDPconnectKVMPorts"); %>
			</select></td>
        </tr>
	</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
  <input type="hidden" name="system.req.action" value="">
  <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
  <input type="hidden" name="urlOk" value="/normal/rdp/configRDPGeneral.asp">
  <input type="hidden" name="urlError" value="/normal/rdp/configRDPGeneral.asp">
  <input type="hidden" name="request" value="<%write(getRequestNumber());%>"> 
   <input type="hidden" name="system.rdpconf.device.deviceCs" value="">
   <input type="hidden" name="system.rdpconf.device.action" value="0">
   <input type="hidden" name="system.rdpconf.device.deviceConf.name" value="">
   <input type="hidden" name="system.rdpconf.device.deviceConf.ipaddr" value="">
   <input type="hidden" name="system.rdpconf.device.deviceConf.srvport" value="">
   <input type="hidden" name="system.rdpconf.device.deviceConf.kvmport" value="">
   <input type="hidden" name="system.rdpconf.device.rdpport" value="">
</form>
</body>
</html>
