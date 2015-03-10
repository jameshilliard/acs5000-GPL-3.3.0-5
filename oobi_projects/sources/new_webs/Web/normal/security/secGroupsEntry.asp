<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
      <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
      <script type="text/javascript">

  <%
    // Set page-wide variables:
    var group = getQueryVal("group", -1);
    set("system.access.selected", group);
	if (group == "-1") {
		set("system.access.action","4");
	} else {
		set("system.access.action","3");
	}
	ReqNum = getRequestNumber();
  %>   

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
      
	function form_verification()
	{
		var form_location = document.configForm;
		checkElement (form_location.elements[9], true, true, false, false);
		if (continue_verification == true) { 
			checkElement (form_location.elements[10], true, true, false, false);
		}

        if (continue_verification == true) {
           continue_verification = opener.checkNewEntry(opener.document.forms["userForm"].elements["groupTable"], form_location.elements[9].value);
           if (continue_verification != true) {
              alert("Group name already exists.");
           }
        }
    }
	
	function submitGroup()
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
			document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
			document.configForm.elements[7].value = opener.document.configForm.elements[7].value;
			document.configForm.elements[8].value = opener.document.configForm.elements[8].value;
            setQuerySSID(document.configForm);
			document.configForm.submit();
			self.close();                                  
		}
	} catch (error) {
	  self.close();
	}
	}   

      function setFocus() 
      {
         document.configForm.elements[9].focus();
      }

      </script>
   <title><% if (-1 == group) write('Add Group'); else write('Edit Group'); %></title>
</head>
<body onload="setFocus();" class="bodyMain">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td align="center" valign="middle">
<table border="0" cellpadding="0" cellspacing="0">
	<tr> 
    <td align="center"  height="40"  valign="top"> 
	   <a href="javascript:submitGroup();">
	   <img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a> 
	   &nbsp;&nbsp;&nbsp;&nbsp;
           <a href="javascript:self.close();">
	   <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
     </td>
  </tr>
</table>
</td>
</tr>
<tr>
	<td align="center" valign="middle">	  	  
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>" onSubmit="return false;">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/security/secUsersGroups.asp">
   <input type="hidden" name="urlError" value="/normal/security/secUsersGroups.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
      <input type="hidden" name="system.access.mode" value="1">
      <input type="hidden" name="system.access.action" value="0">
      <input type="hidden" name="system.access.usersCs" value="">
      <input type="hidden" name="system.access.groupsCs" value="">

    <table border="0" cellpadding="10" cellspacing="0">
      <tr> 
        <td ><font class="label">Group Name</font></td>
        <td> 
          <input class="formText" type="text" maxlength="20" name="system.access.addModGroup.grpName" size="20" value="<%get("system.access.addModGroup.grpName");%>">
        </td>
      </tr>
      <tr> 
        <td><font class="label">Users</font></td>
        <td> 
          <input class="formText" type="text" name="system.access.addModGroup.grpMembers" size="30" maxlength="250" value="<%get("system.access.addModGroup.grpMembers");%>">
        </td>
      </tr>
    </table>
</form>
</td>
</tr>
</table>
</body>
</html>
