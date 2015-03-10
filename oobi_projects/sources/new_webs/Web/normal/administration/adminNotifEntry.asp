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
		var notifType = getQueryVal("notifType",-1);
		var triggerId = getQueryVal("triggerId", -1);
		set(self+"triggerId",triggerId);
		set(self+"notifType",notifType);
		notifType = getVal("system.administration.notifications.eventsNotif.notifType");
		var page;
		if (notifType == 0) 
			page="/normal/helpPages/adminNotifEmailHelp.asp";
		else
			if (notifType == 1) 
				page="/normal/helpPages/adminNotifPagerlHelp.asp";
			else
				page="/normal/helpPages/adminNotifSNMPHelp.asp";
	ReqNum = getRequestNumber();
	%>
	
    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	  function form_verification()
	  {	  var form_location = document.eventForm; 
	  if (document.configForm.elements[8].value == 0) // e-mail
	  {		 
	  		checkElement (form_location.elements[0], true, false, false, false);
	  		// checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[2], true, true, false, false);
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[3], true, true, false, false);
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[4], true, false, false, false);
			if (continue_verification == true) {
				checkMaxLength (form_location.elements[5], 499);
				checkNotPermitedChars (form_location.elements[5], '"`\'', '<">, <`> and <\'> are not allowed characters.');
			}
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[6], true, false, false, false);
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[7], false, false, false, true);
	  }	
	  	  if (document.configForm.elements[8].value == 1) // pager
	  {		 
	  		checkElement (form_location.elements[0], true, false, false, false);
	  		// checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[2], true, false, false, true);
			if (continue_verification == true) {
				checkMaxLength (form_location.elements[3], 159);
				checkNotPermitedChars (form_location.elements[3], '"`\'', '<">, <`> and <\'> are not allowed characters.');
			}
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[4], true, true, false, false);
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[5], true, false, false, false);
	  		if (continue_verification == true)
	  		checkElement (form_location.elements[6], false, false, false, true);
	  }
	  	  if (document.configForm.elements[8].value == 2) // SNMP
	  {		 
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
			if (continue_verification == true) {
				checkMaxLength (form_location.elements[6], 249);
				checkNotPermitedChars (form_location.elements[6], '"`\'', '<">, <`> and <\'> are not allowed characters.');
			}
	  }
	  }	
	
	function addNotifEntry()
	{
	try {
		if (opener.window.name != document.configForm.target) {
			alert("The page which called this form is not available anymore. The changes will not be effective.");
			self.close();
			return;
		}
		form_verification();
		if (continue_verification == true) {
			var dosub=1;

			document.configForm.elements[5].value=opener.document.configForm.elements[5].value;
			document.configForm.elements[6].value=opener.document.configForm.elements[6].value;
			document.configForm.elements[9].value=document.eventForm.elements[0].value;
<%
if (notifType == 0) // Notification Type is EMAIL
{ 	
	write("for (var i=2; i < 8  ; i++) {");
} else {
	write("for (var i=2; i < 7  ; i++) {");
}
%>
			document.configForm.elements[i+8].value = document.eventForm.elements[i].value;
		}
		document.configForm.elements[1].value=opener.document.configForm.elements[1].value;
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

<p>
    <form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
		<input type="hidden" name="system.req.action" value="">
		<input type="hidden" name="system.req.sid" value='<%get("_sid_");%>'>
		<input type="hidden" name="urlOk" value="/normal/administration/adminNotifications.asp">
		<input type="hidden" name="urlError" value="/normal/administration/adminNotifications.asp">
		<input type="hidden" name="request" value=<%write(ReqNum);%>>
		<input type="hidden" name="<%write(self);%>alarm" value="">
		<input type="hidden" name="<%write(self);%>notifTable" value="">
		<input type="hidden" name="<%write(self);%>triggerId" value="<%write(triggerId);%>">
		<input type="hidden" name="<%write(self);%>notifType" value="<%write(notifType);%>">
		<input type="hidden" name="<%write(self);%>eventsNotif.trigger" value="">
	<%
		if (notifType == 0) // Notification Type is EMAIL
		{ 	
			write('<input type="hidden" name="' + selfE + 'toAddress" value="">\n');
			write('<input type="hidden" name="' + selfE + 'fromAddress" value="">\n');
			write('<input type="hidden" name="' + selfE + 'subject" value="">\n');
			write('<input type="hidden" name="' + selfE + 'body" value="">\n');
			write('<input type="hidden" name="' + selfE + 'serverIp" value="">\n');
			write('<input type="hidden" name="' + selfE + 'serverPort" value="">\n');
		} else {
			if (notifType == 1) { // type = Pager
			write('<input type="hidden" name="' + selfP + 'phoneNum" value="">\n');
			write('<input type="hidden" name="' + selfP + 'text" value="">\n');
			write('<input type="hidden" name="' + selfP + 'smsUserName" value="">\n');
			write('<input type="hidden" name="' + selfP + 'smsServerIp" value="">\n');
			write('<input type="hidden" name="' + selfP + 'smsPort" value="">\n');

			} else { // type = snmptrap

			write('<input type="hidden" name="' + selfS + 'oid" value="">\n');
			write('<input type="hidden" name="' + selfS + 'trapNumber" value="">\n');
			write('<input type="hidden" name="' + selfS + 'community" value="">\n');
			write('<input type="hidden" name="' + selfS + 'serverIp" value="">\n');
			write('<input type="hidden" name="' + selfS + 'body" value="">\n');
			}
		}
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
	if (notifType == 0) // Notification Type is EMAIL
	{ 	
        write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">To</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" size="30" maxlength="299" name="emailTo" value="' + getVal(selfE + "toAddress") + '"');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">From</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" size="30" maxlength="299" name="emailFrom" value="' + getVal(selfE + "fromAddress")+ '"');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Subject</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" size="35" maxlength="199" name="subject" value="'+ getVal(selfE+"subject")+'">\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Body</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<textarea cols="40" rows="10" name="body" class="formText">'+ getVal(selfE+"body") + '\n');
	write('</textarea></font></td></tr>\n');      
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">SMTP Server</font></td></tr><tr valign="top"><td>\n');
	write('<input class="formText" type="text" size="30" maxlength="99" name="serverIp" value="'+getVal(selfE+"serverIp")+'"');
	write(' >');
	write('</td></tr><tr valign="top"><td align="left" height="30" valign="middle"><font class="label">SMTP Port</font></td></tr><tr valign="top"><td>\n');
	write('<input class="formText" type="text" name="serverPort" size="10" value="' + getVal(selfE+"serverPort") + '"');
	write(' >');
	write('</td></tr>\n');
	} else {
		if (notifType == 1) { // type = Pager
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Pager Number</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="19" size="19" name="pagerNumber"  value="'+ getVal(selfP+"phoneNum") + '"');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Text</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<textarea class="formText" cols="40" rows="4" name="text">'+getVal(selfP+"text")+'\n');
	write('</textarea></font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">SMS User Name</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="30" size="30" name="smsUserName" value="'+getVal(selfP+"smsUserName")+'"');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">SMS Server</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="99" size="30" name="smsServerIP" value="'+getVal(selfP+"smsServerIp")+'"');
	write(' >');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">SMS Port</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" size="10" name="smsPort" value="'+getVal(selfP+"smsPort")+'"');
	write(' >\n');
	write('</font></td></tr>\n');

	} else { // type = SNMPTRAP
	
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">OID Type Value </font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="39" size="30" name="oid" value="'+getVal(selfS+"oid")+'"\n');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Trap Number</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<select name="trapNumber" class="formText">\n');
	var selected =getVal(selfS+"trapNumber");
	write('<option value="0"');if (selected == 0) write("selected");write('>Cold Start</option>');
	write('<option value="1"');if (selected == 1) write("selected");write('>Warm Start</option>');
	write('<option value="2"');if (selected == 2) write("selected");write('>Link Down</option>');
	write('<option value="3"');if (selected == 3) write("selected");write('>Link UP</option>');
	write('<option value="4"');if (selected == 4) write("selected");write('>Authentication Failure</option>');
	write('<option value="5"');if (selected == 5) write("selected");write('>EGP neighbor loss</option>');
	write('<option value="6"');if (selected == 6) write("selected");write('>Enterprise specific</option>');
	write('</select></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Community</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="30" size="30" name="community" value="'+getVal(selfS+"community")+'"');
	write(' >\n');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Server</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<input class="formText" type="text" maxlength="99" size="30" name="serverIP" value="'+getVal(selfS+"serverIp")+'"');
	write(' >');
	write('</font></td></tr>\n');
	write('<tr valign="top"><td align="left" height="30" valign="middle"><font class="label">Body</font></td></tr><tr valign="top"><td><font class="formText">\n');
	write('<textarea class="formText" cols="40" rows="5" name="body" >'+getVal(selfS+"body")+'\n');
	write('</textarea></font></td></tr>\n');
		}
	}
%>      
   </table>
</form>
</body>
</html>
