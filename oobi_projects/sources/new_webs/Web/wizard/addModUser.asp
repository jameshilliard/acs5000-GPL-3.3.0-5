
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title><%write('Add User');%></title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

	<%
		var user = getQueryVal("user", "-1");
		set("system.access.selected", user);
	%>
	var user = <%write(user);%>

   function form_verification()
   {           
      var form_location = document.configForm
      checkElement (form_location.elements[8], true, true, false, false);
     // checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)

      if ((opener.top.family_model == "KVM" || opener.top.family_model == "ONS")
			&& (form_location.elements[8].value == "root" || form_location.elements[8].value == "default")) {
            alert("Cannot create " + form_location.elements[8].value + " user.");
            continue_verification = false;
      }

      if (continue_verification == true) {
		continue_verification = opener.checkNewEntry(opener.document.forms["userForm"].elements["userTable"], form_location.elements[8].value, user);
		if (!continue_verification) {
			alert("User already exists.");
			return;
		}
      }

      if (continue_verification == true)
         checkElement (form_location.elements[9], true, true, false, false);
      if (continue_verification == true)
         checkElement (form_location.elements[10], true, true, false, false, 'no', 'no', true, form_location.elements[9].value);
   }          
     
   function addUserSubmit()
   {    
	try {
	  if (opener.window.name != document.configForm.target) {
         alert("The page which called this form is not available anymore. The changes will not be effective.");
         self.close(); 
         return;
      }
      form_verification();
      if (continue_verification == true) {
         document.configForm.elements[7].value = ',' + getValues(opener.document.userForm.userTable, 0);
         document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
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
      document.configForm.elements[8].focus();
   }
 
   </script>
</head>

<body onload="passSessionID(); setFocus();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="">
   <input type="hidden" name="urlOk" value="/wizard/access.asp">
   <input type="hidden" name="urlError" value="/wizard/access.asp">
   <input type="hidden" name="request" value=<%write(getRequestNumber());%>>
<!--*********************************************************************************-->
   <input type="hidden" name="system.access.mode" value="0">
   <input type="hidden" name="system.access.action" value="2">
   <input type="hidden" name="system.access.usersCs" value="">

<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
   <td align="center" valign="middle">
<table border="0" cellpadding="5" cellspacing="0">
   <tr>
        <td colspan="4" align="center">        
          <a href="javascript:addUserSubmit();">
        <img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a>          
          &nbsp;&nbsp;&nbsp;&nbsp;
          <a href="javascript:window.close();">
        <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
        </td>
    </tr>

      <tr> 
        <td align="left"><font class="label">User Name</font></td>
        <td> 
          <input class="formText" type="text" maxlength="30" name="system.access.addModUser.userName" size="25">
        </td>
      </tr>
      <tr> 
        <td align="left"><font class="label">Password</font></td>
        <td> 
          <input class="formText" type="password" maxlength="30" name="system.access.addModUser.passWord" size="25">
        </td>
      </tr>
      <tr> 
        <td align="left"><font class="label">Repeat<br> Password</font></td>
        <td> 
          <input class="formText" type="password" maxlength="30" name="system.access.addModUser.passWord2" size="25">
        </td>
      </tr>
      <tr> 
        <td align="left" height="11"><font class="label">Group</font></td>
        <td height="11"> 
          <select class="formText" name="system.access.addModUser.groupAdminReg">
            <option value="1">Admin</option>
            <option value="2" SELECTED>Regular User</option>
          </select>
<script type="text/javascript">
    document.write('<input type="hidden" name="system.req.bogus" value="">\r\n');
</script>
        </td>
      </tr>
      <tr> 
        <td valign="bottom" align="left"><font class="label">Shell</font></td>
        <td valign="top"> 
          <input class="formText" type="text" maxlength="49" name="system.access.addModUser.shell" size="25">
        </td>
      </tr>
      <tr> 
        <td valign="bottom" align="left"><font class="label">Comments</font></td>
        <td valign="top">
        <input class="formText" type="text" maxlength="49" name="system.access.addModUser.comments" size="25">
        </td>
      </tr>
</table>   
   </td>
</tr>
</table>
</form>
</body>
</html>

