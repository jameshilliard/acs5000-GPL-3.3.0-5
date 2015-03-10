<html>
<head>
<title>Add/Edit Outlet Groups</title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/JavaScript">
<%
	var SaddCall = 0;

	// I'm using sysMsg as a communication channel to the middleware
	set("system.applications.pmdOutGrp.singleGroup.sysMsg", getQueryVal("index",-1));

	// These get()'s make the appropriate query function to be called in the middleware
	var SgroupName = getVal("system.applications.pmdOutGrp.singleGroup.groupName");
	var Soutlets = getVal("system.applications.pmdOutGrp.singleGroup.outlets");

	if (SgroupName == 'addaddadd')
		SaddCall = 1;

%>

	<% ReqNum = getRequestNumber(); %>
window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	var family_model = "<%get('_familymodel_');%>";
	var Stty, _group, group = "<% write(SgroupName); %>";

	switch (family_model) {
		case "ONS":
			_group = /^tty[SA]/;
		break;
		case "KVM":
			_group = /^ttyA/;
		break;
		default:
			_group = /^ttyS/;
		break;
	}
	
	Stty = (group.match(_group)==null)? false : true;
	
	//if (group.charAt(0) == "t" && group.charAt(1) == "t" &&
    //        group.charAt(2) == "y" && group.charAt(3) == "S") {
	//	Stty = 1;
	//}

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
		var form_location = document.PMDoutGrpEntryForm;
		continue_verification = true;
		if (!Stty) {
			// chek the group name
			checkElement(form_location.group, true, true, false, false);
			if (continue_verification == true) {
				var ret;
				ret = validateNameOutIpdu(form_location.group.value,0);
				if (ret == false) {
					alert("Invalid group name");
					outlist.focus();
					outlist.select();
					continue_verification = false;
				} else {
					var name = form_location.group.value;
					if (name.match(_group) != null) {
						alert("Reserved group name");
						outlist.focus();
						outlist.select();
						continue_verification = false;
					}
				}
			}
		}
		if (continue_verification == true) {
			// chek the outlet list
			checkElement(form_location.group_outlets, true, true, false, false);
			if (continue_verification == true)
				checkOutletList(form_location.group_outlets);
		}
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
			if (!Stty) {
				document.configForm.elements[6].value =	document.PMDoutGrpEntryForm.group.value;
			} else {
				document.configForm.elements[6].value = "<% write(SgroupName); %>";
			}
			document.configForm.elements[7].value =	document.PMDoutGrpEntryForm.group_outlets.value;

            setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = false;
			self.close();
		}
	}
	  
</script>
</head>

<body class="bodyMain">
<form name="PMDoutGrpEntryForm" method="POST" action=" " onSubmit="return false;">
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
<script type="text/javascript">
	if (Stty) {
		document.write('<td height="50" width="50" align="left"><font class="label">Group :&nbsp;<%write(SgroupName);%></font></td>');
	 } else { // write edit box when user choose ADD
		document.write('<td height="50" width="50" align="left"><font class="label">Group</font></td><td align="left">');
		document.write('<input class="formText" name="group" type="text" maxlength="30" size="20"');
		if (!<% write(SaddCall); %>)
			document.write('value="<% write(SgroupName); %>"></td>');
		else
			document.write('></td>');
	}
</script>
</tr>
<tr>
	<td colspan="2" align="left"><font class="label">Outlets</font></td>
</tr>
<tr>
	<td colspan="2" align="left"><input class="formText" name="group_outlets" type="text" maxlength="120" size="50"
	<%if (!SaddCall)
		write(" value=\"" + Soutlets + "\"");%>>
	</td>
</tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
    <input type="hidden" name="system.req.action" value="">
    <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
    <input type="hidden" name="urlOk" value="/normal/applications/appsPMDconfOutGrp.asp">
    <input type="hidden" name="urlError" value="/normal/applications/appsPMDconfOutGrp.asp">
    <input type="hidden" name="request" value="0">
    <!--*******************************************************************-->
	<input type="hidden" name="system.applications.pmdOutGrp.entries" value="">
	<input type="hidden" name="system.applications.pmdOutGrp.singleGroup.groupName" value="">
	<input type="hidden" name="system.applications.pmdOutGrp.singleGroup.outlets" value="">
</form>
</body>
</html>
