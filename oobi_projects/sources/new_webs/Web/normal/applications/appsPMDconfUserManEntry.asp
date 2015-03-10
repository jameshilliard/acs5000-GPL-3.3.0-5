<html>
<head>
<title>Add/Edit PM Users</title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/JavaScript">
<%
	var SaddCall = 0;

	// I'm using sysMsg as a communication channel to the middleware
	set("system.applications.pmdUserMan.singleUser.sysMsg", getQueryVal("index",-1));

	// These get()'s make the appropriate query function to be called in the middleware
	var SuserName = getVal("system.applications.pmdUserMan.singleUser.userName");
	var Soutlets = getVal("system.applications.pmdUserMan.singleUser.outlets");

	if (SuserName == 'addaddadd')
		SaddCall = 1;
%>

	<% ReqNum = getRequestNumber(); %>
window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	var family_model = "<%get('_familymodel_');%>";

	function checkOutletList(outlist)
	{
		var list = chgSeparator(outlist.value);

		if (continue_verification == true)
			validateListElements(list);

		if (continue_verification != true) {
			outlist.focus();
			outlist.select();
		}
	}

	function form_verification()
	{
		var form_location = document.PMDuserManEntryForm;
		continue_verification = true;
//		if (<%write(SaddCall);%>) {
			continue_verification = checkUsername(form_location.user);
			if (continue_verification != true) {
				form_location.user.focus(); 
				form_location.user.select();
				alert("Invalid user name.");
				return;
			}
//		}
		if (continue_verification == true)
			checkElement(form_location.user_outlets, true, true, false, false);
		if (continue_verification == true)
			checkOutletList(form_location.user_outlets);
	}

	function AddEditOutletInfo()
	{
		if (opener.parent.window.name != document.configForm.target) {
			alert("The page which called this form is not available anymore. The changes will not be effective.");
			self.close();
			return;
		}
		form_verification();
		if (continue_verification == true)
		{		
			document.configForm.elements[5].value = opener.document.configForm.elements[5].value;
//			if (<%write(SaddCall);%>)
				document.configForm.elements[6].value =	document.PMDuserManEntryForm.user.value;
			document.configForm.elements[7].value =	document.PMDuserManEntryForm.user_outlets.value;

            setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = false;
			self.close();
		}
	}
	  
</script>
</head>

<body class="bodyMain">
<form name="PMDuserManEntryForm" method="POST" action=" " onSubmit="return false;">
<table border="0" align="center" cellpadding="0" cellspacing="5">
<tr valign="top"> 
	<td height="40" colspan="2" align="center">
		<a href="javascript:AddEditOutletInfo();">
			<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
	  &nbsp;&nbsp;&nbsp;&nbsp;
		<a href="javascript:window.close();">
			<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
	</td>
</tr>
<tr> 
	<td height="50" width="50" align="left"><font class="label">User</font></td>
	<td align="left">
	<input class="formText" name="user" type="text" maxlength="30" size="20"
	<%if (!SaddCall)
		write(" value=\"" + SuserName + "\"");%>>
	</td>
</tr>
<tr>
	<td colspan="2" align="left"><font class="label">Outlets</font></td>
</tr>
<tr>
	<td colspan="2" align="left"><input class="formText" name="user_outlets" type="text" maxlength="120" size="50"
	<%if (!SaddCall)
		write(" value=\"" + Soutlets + "\"");%>>
	</td>
</tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
    <input type="hidden" name="system.req.action" value="">
    <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
    <input type="hidden" name="urlOk" value="/normal/applications/appsPMDconfUserMan.asp">
    <input type="hidden" name="urlError" value="/normal/applications/appsPMDconfUserMan.asp">
    <input type="hidden" name="request" value="0">
    <!--*******************************************************************-->
	<input type="hidden" name="system.applications.pmdUserMan.entries" value="">
	<input type="hidden" name="system.applications.pmdUserMan.singleUser.userName" value="">
	<input type="hidden" name="system.applications.pmdUserMan.singleUser.outlets" value="">
</form>
</body>
</html>
