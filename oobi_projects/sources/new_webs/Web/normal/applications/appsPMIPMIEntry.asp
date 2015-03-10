<html>
<head>
<title>Add/Edit IPMI Device</title>

<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script language="JavaScript" type="text/JavaScript">

  <%
	var Saddwin = getQueryVal("add",-1);
	var Salias = getQueryVal("alias",-1);
	var Sip = getQueryVal("ip",-1);
	var Sat = getQueryVal("at",-1);
	var Sal = getQueryVal("al",-1);
	var Sun = getQueryVal("un",-1);
	var Spw = getQueryVal("pw",-1);
	ReqNum = getRequestNumber();
  %>

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function form_verification()
	{
		var form_location = document.PMIPMIEntryForm;
		var edId = opener.document.configForm.elements[6].value - 1;
		continue_verification = true;
		
		checkElement (form_location.alias, true, true, false, false);
		for (var i=0; i < opener.numIPMIDev; i++) {
			if (opener.IPMIinfo[i][0] == form_location.alias.value) {
				if (<%write(Saddwin);%> ||(i != edId)) {
					alert("This alias is already in use by another IPMI device");
					continue_verification = false;
					break;
				}
			}
		}
		if (continue_verification == true)
			checkElement (form_location.ipaddress, true, true, true, false);
		if (continue_verification == true)
			checkElement (form_location.username, false, true, false, false);
		if (continue_verification == true)
			checkElement (form_location.password, false, true, false, false);
	}
	
	function SendIPMIInfo()
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
			document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
			document.configForm.elements[4].value = opener.document.configForm.elements[4].value;
			if (<%write(Saddwin);%>)
				document.configForm.elements[5].value = 13; //Add
			else
				document.configForm.elements[5].value = 7; //Edit

			if (<%write(Saddwin);%>) {
				document.configForm.elements[6].value = document.PMIPMIEntryForm.alias.value;
			} else {
				document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
				document.configForm.elements[6].value += " "+document.PMIPMIEntryForm.alias.value;
			}	
			document.configForm.elements[6].value += " "+document.PMIPMIEntryForm.ipaddress.value;
			document.configForm.elements[6].value += " "+document.PMIPMIEntryForm.authtype.value;
			document.configForm.elements[6].value += " "+document.PMIPMIEntryForm.accesslevel.value;
			if (document.PMIPMIEntryForm.username.value)
				document.configForm.elements[6].value += " "+document.PMIPMIEntryForm.username.value;
			else
				document.configForm.elements[6].value += " blankblank"; //signal name was erased
			if (document.PMIPMIEntryForm.password.value)
				document.configForm.elements[6].value += " "+document.PMIPMIEntryForm.password.value;
			else
				document.configForm.elements[6].value += " blankblank"; //signal name was erased

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

<body class="bodyMain">
<table border="0" align="center" cellpadding="0" cellspacing="10">
  <tr valign="top"> 
    <td height="40" colspan="2" align="center">
	     <a href="javascript:SendIPMIInfo();">
         <img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
         &nbsp;&nbsp;&nbsp;&nbsp;
		 <a href="javascript:window.close();">
         <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
	</td>
  </tr>
<form name="PMIPMIEntryForm" method="POST" action=" " onSubmit="return false;">
  <tr valign="bottom"> 
    <td align="right"><font class="label">Alias</font></td>
	<td align="left"><input class="formText" name="alias" type="text" maxlength="15" size="10"
		value="<%if (!Saddwin) write(Salias);%>"></td>
  </tr>
  <tr valign="bottom"> 
    <td align="right"><font class="label">IP Address</font></td>
	<td align="left"><input class="formText" name="ipaddress" type="text" maxlength="15" size="15"\
        value="<%if (!Saddwin) write(Sip);%>"></td>
  </tr>
  <tr valign="bottom">
    <td align="right"><font class="label">Authentication Type</font></td>
    <td align="left"><select name="authtype" class="formText">
		<option value="none" <%if (!Saddwin && (Sat == "none")) write("selected");%>>None</option>
		<option value="password" <%if (!Saddwin && ((Sat == "straight") || (Sat == "password"))) write("selected");%>>Straight Password</option>
		<option value="md5" <%if (!Saddwin && (Sat == "md5")) write("selected");%>>MD5</option>
		<option value="md2" <%if (!Saddwin && (Sat == "md2")) write("selected");%>>MD2</option>
    </select></td>
  </tr>
  <tr valign="bottom">
    <td align="right"><font class="label">Access Level</font></td>
    <td align="left"><select name="accesslevel" class="formText">
		<option value="user" <%if (!Saddwin && (Sal == "user")) write("selected");%>>User</option>
		<option value="operator" <%if (!Saddwin && (Sal == "operator")) write("selected");%>>Operator</option>
		<option value="admin" <%if (!Saddwin && (Sal == "admin")) write("selected");%>>Administrator</option>
    </select></td>
  </tr>
  <tr valign="bottom"> 
    <td align="right"><font class="label">Username</font></td>
	<td align="left"><input class="formText" name="username" type="text" maxlength="15" size="10"
        value="<%if (!Saddwin) write(Sun);%>"></td>
  </tr>
  <tr valign="bottom"> 
    <td align="right"><font class="label">Password</font></td>
	<td align="left"><input class="formText" type="password" name="password" type="text"
        maxlength="15" size="10" value="<%if (!Saddwin) write(Spw);%>"></td>
  </tr>
</form>
</table>
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
    <input type="hidden" name="system.req.action" value="">
    <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
    <input type="hidden" name="urlOk" value="/normal/applications/appsPMIPMI.asp">
    <input type="hidden" name="urlError" value="/normal/applications/appsPMIPMI.asp">
    <input type="hidden" name="request" value="<%write(getRequestNumber());%>">
    <!--*******************************************************************-->
	<input type="hidden" name="system.applications.pmIpmi.action" value="">
	<input type="hidden" name="system.applications.pmIpmi.sysMsg" value="">
</form>
</body>
</html>
