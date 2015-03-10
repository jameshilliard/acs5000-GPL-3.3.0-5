<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Notifications Entry</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		var self = "system.administration.notifications.";
		var selfE = "system.administration.notifications.eventsNotif.email.";
		var selfP = "system.administration.notifications.eventsNotif.pager.";
		var selfS = "system.administration.notifications.eventsNotif.snmptrap.";
		var triggerId = getQueryVal("triggerId", -1);
		var notifType = 2;
		set(self+"triggerId",triggerId);
		var page="/normal/helpPages/adminNotifSNMPHelp.asp";
		ReqNum = getRequestNumber();

	%>
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	function form_verification()
	{	  
		var form_location = document.eventForm; 
		checkElement (form_location.elements[0], true, false, false, false);
		// checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)
		if (continue_verification == true)
		checkElement (form_location.elements[2], true, false, false, false);
		if (continue_verification == true)
		checkPermitedChars (form_location.elements[2], '0123456789.', 'Please enter only numbers and points (without spaces).')
		// function that verify OID Type Value - only numbers and points (element_for_verification, chars)
		if (continue_verification == true)
		checkElement (form_location.elements[4], true, true, false, false);
		if (continue_verification == true)
		checkElement (form_location.elements[5], true, false, false, false);
		if (continue_verification == true)
		checkMaxLength (form_location.elements[6], 249);
	}	
	
	function addNotifEntry()
	{
	try {
	 	if (opener.window.name != document.configForm.target) {
			alert("The page which called this form is not available anymore. The changes will not be effective.");
			window.close();
			return;
         	}
		form_verification();
		if (continue_verification == true)
		{	
			var dosub=1;

			document.configForm.elements[5].value=opener.document.configForm.elements[5].value;
			document.configForm.elements[6].value=opener.document.configForm.elements[6].value;
			document.configForm.elements[9].value=document.eventForm.elements[0].value;

			<%
				write("for (var i=2; i < 7  ; i++) {");
				write("document.configForm.elements[i+8].value = document.eventForm.elements[i].value;");
				write("}");
			%>

			document.configForm.elements[1].value=opener.document.configForm.elements[1].value;
			setQuerySSID(document.configForm);
			document.configForm.submit();
			window.close();
		}		
	} catch (error) {
	  self.close();
	}
	}
   </script>
</head>

<body class="bodyMain">

<p>
    <form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
		<input type="hidden" name="system.req.action" value="">
		<input type="hidden" name="system.req.sid" value='<%get("_sid_");%>'>
		<input type="hidden" name="urlOk" value="/normal/network/netNotifications.asp">
		<input type="hidden" name="urlError" value="/normal/network/netNotifications.asp">
		<input type="hidden" name="request" value=<%write(ReqNum);%>>
		<input type="hidden" name="<%write(self);%>alarm" value="">
		<input type="hidden" name="<%write(self);%>notifTable" value="">
		<input type="hidden" name="<%write(self);%>triggerId" value="<%write(triggerId);%>">
		<input type="hidden" name="<%write(self);%>notifType" value="<%write(notifType);%>">
		<input type="hidden" name="<%write(self);%>eventsNotif.trigger" value="">
	<%
		write('<input type="hidden" name="' + selfS + 'oid" value="">\n');
		write('<input type="hidden" name="' + selfS + 'trapNumber" value="">\n');
		write('<input type="hidden" name="' + selfS + 'community" value="">\n');
		write('<input type="hidden" name="' + selfS + 'serverIp" value="">\n');
		write('<input type="hidden" name="' + selfS + 'body" value="">\n');
	%>

	</form>
	 
	   <table border="0" cellpadding="2" cellspacing="0" align="center">
		<tr>
		<td align="center" valign="middle" height="40">
			<a href="javascript:addNotifEntry();">
			<img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a>
		&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:self.close();">
			<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
		</td>
		</tr>
	   </table>

	<form name="eventForm" onSubmit="return false;">
   <table border="0" cellpadding="0" cellspacing="0" align="center">	
   	<tr>
         <td align="left" height="30" valign="middle"><font class="label">Alarm Trigger</font></td>
	</tr>
	 <td align="left" height="40" valign="middle">
	<input class="formText" type="text" maxlength="99" name="newTrigger" size="40" value='<%get(self+"eventsNotif.trigger");%>'></td>
	    <tr>
            <td align="center" height="30" valign="middle">
            <select name="oldTrigger" class="formText" onChange="document.eventForm.newTrigger.value=document.eventForm.oldTrigger.options[document.eventForm.oldTrigger.selectedIndex].text">
            	<%get(self + "eventsNotif.triggerHtml");%>
		<option selected>&nbsp;</option>
            </select></td></tr>
  <%
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Binding OID </font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="39" size="30" name="oid" value="'+getVal(selfS+"oid")+'"\n');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Trap Number</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<select name="trapNumber" class="formText">\n');
	var selected =getVal(selfS+"trapNumber");
	write('<option value="0"');if (selected == 0) write("selected");write('>Cold Start</option>');
	write('<option value="1"');if (selected == 1) write("selected");write('>Warm Start</option>');
	write('<option value="7"');if (selected == 7) write("selected");write('>User Login</option>');
	write('<option value="8"');if (selected == 8) write("selected");write('>User Logoff</option>');
	write('<option value="4"');if (selected == 4) write("selected");write('>Authentication Failure</option>');
	write('<option value="9"');if (selected == 9) write("selected");write('>Authentication Successful</option>');
	write('</select></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Community</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="30" size="30" name="community" value="'+getVal(selfS+"community")+'"');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Server</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="99" size="15" name="serverIP" value="'+getVal(selfS+"serverIp")+'"');
	write(' >');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Body</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<textarea class="formText" cols="40" rows="5" name="body" >'+getVal(selfS+"body")+'\n');
	write('</textarea></font></td></tr>\n');
%>      
   </table>
</form>
</body>
</html>
