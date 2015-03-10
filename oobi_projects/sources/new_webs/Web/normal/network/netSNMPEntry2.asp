<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<title>New/Mod SNMPv3 Configuration</title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

  <%
		var SaddCall = 0;

		// I'm using webMsg as a communication channel to the middleware
		set("system.network.SNMPdaemon.webMsg", getQueryVal("snmpv3entry",-1));

		// This get() makes the appropriate query function to be called in the middleware
		var Spermission = getVal("system.network.SNMPdaemon.SNMPv3.permission");

		if (Spermission == 99)
			SaddCall = 1;
	    ReqNum = getRequestNumber();
	%>

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	function copy_data()
	{
		document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
		document.configForm.elements[4].value = opener.document.configForm.elements[4].value;

		document.configForm.elements[5].value = opener.document.configForm.elements[5].value;
		document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
		document.configForm.elements[7].value = opener.document.configForm.elements[8].value;

		document.configForm.elements[9].value = document.forms.SNMPEntry2Form.user_name.value;
		document.configForm.elements[10].value = document.forms.SNMPEntry2Form.password.value;
		var index = document.SNMPEntry2Form.Permission.selectedIndex;
		document.configForm.elements[11].value =
			document.SNMPEntry2Form.Permission.options[index].value;
		document.configForm.elements[12].value = document.forms.SNMPEntry2Form.oid.value;
	}
	
	function form_verification()
	{
		form_location = document.SNMPEntry2Form;
		continue_verification = true;
			 checkElement (form_location.user_name, true, true, false, false);
		if (continue_verification == true) {
			checkElement (form_location.password, true, true, false, false);
		}
		if (continue_verification == true && form_location.oid.value != '') {
			var is_dot_ok=form_location.oid.value.indexOf('.')
			if (is_dot_ok != 0) {
            			alert("OID must start with a . (dot)");
				continue_verification = false;
			} else {
				checkPermitedChars (form_location.oid, '0123456789.', 'Please enter only numbers and points (without spaces).');
			}
		}
	}	
	
	function addEditSNMPv3Entry()
	{
	try {
	     if (opener.window.name != document.configForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
		form_verification();
	    if (continue_verification == true)
		{
			if (<%write(SaddCall);%>) {
				copy_data();
				document.configForm.elements[8].value = 'v3entryADDED';
			} else {
				copy_data();
				document.configForm.elements[8].value = 'v3entryEDITED';
        	}
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
<body  class="bodyMain">
<table border="0" align="center" cellpadding="0" cellspacing="10">
<tr valign="top"  height="40"> 
    <td colspan="2" align="center">
         <a href="javascript:addEditSNMPv3Entry();">
         <img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
         &nbsp;&nbsp;&nbsp;&nbsp;
		 <a href="javascript:window.close();">
         <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
	</td>
  </tr>
<form name="SNMPEntry2Form" onSubmit="return false;">
  <tr valign="bottom"> 
    <td align="left"><font class="label">User Name</font></td>
    <td align="left"><input name="user_name" type="text" maxlength="30" size="20" class="formText"
	<%if (!SaddCall)
		write(" value=\"" + getVal("system.network.SNMPdaemon.SNMPv3.userName") + "\"");%>>
	</td>
  </tr>
  <tr valign="bottom">
    <td align="left"><font class="label">Password</font></td>
    <td align="left"><input name="password" type="password" maxlength="30" size="20" class="formText"
	<%if (!SaddCall)
		write(" value=\"" + getVal("system.network.SNMPdaemon.SNMPv3.password") + "\"");%>>
	</td>
  </tr>
  <tr valign="bottom"> 
    <td align="left"><font class="label">OID</font></td>
    <td align="left"><input name="oid" type="text" size="20" maxlength="39" class="formText"
	<%if (!SaddCall)
		write(" value=\"" + getVal("system.network.SNMPdaemon.SNMPv3.oid") + "\"");%>>
	</td>
  </tr>
  <tr valign="bottom">
    <td align="left"><font class="label">Permission</font></td>
    <td align="left">
	  <select name="Permission" class="formText">
        <option value="0" <%if (Spermission == 0) write("selected");%>>Read Only </option>
        <option value="1" <%if (Spermission == 1) write("selected");%>>Read/Write</option>
      </select>
	</td>
  </tr>
</form>
</table>   
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
    <input type="hidden" name="system.req.action" value="">
    <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
    <input type="hidden" name="urlOk" value="/normal/network/netSNMP.asp">
    <input type="hidden" name="urlError" value="/normal/network/netSNMP.asp">
    <input type="hidden" name="request" value="<%write(ReqNum);%>">
    <!--*******************************************************************-->
    <input type="hidden" name="system.network.SNMPdaemon.sysContact" value="">
    <input type="hidden" name="system.network.SNMPdaemon.sysLocation" value="">
    <input type="hidden" name="system.network.SNMPdaemon.SNMPv3Entries" value="">
    <input type="hidden" name="system.network.SNMPdaemon.webMsg" value="">
    <input type="hidden" name="system.network.SNMPdaemon.SNMPv3.userName" value="">
    <input type="hidden" name="system.network.SNMPdaemon.SNMPv3.password" value="">
    <input type="hidden" name="system.network.SNMPdaemon.SNMPv3.permission" value="">
    <input type="hidden" name="system.network.SNMPdaemon.SNMPv3.oid" value="">
</form>
</body>
</html>
