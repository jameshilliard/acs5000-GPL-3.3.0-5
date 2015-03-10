
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title><%write('Add User');%></title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

  <%
      var user = getQueryVal("user", -1);
      set("system.access.selected", user);
   %>

  function check_string(formstr) 
  {
	continue_verification = true;
	if (formstr.value != '') {
		var string = formstr.value;
		for (var i = 0; i < string.length; i++) {
			if (string.charAt(i) == '"') {
				continue_verification = false;
				alert ('quote (") character is not permitted');
				break;
			}
		}
	}
   }

   function form_verification()
   {   
      var form_location = document.configForm
      checkElement (form_location.elements[9], true, true, false, false);
      if (continue_verification == true)
         checkElement (form_location.elements[10], true, true, false, false);
      if (continue_verification == true)
         checkElement (form_location.elements[11], true, true, false, false, 'no', 'no', true, form_location.elements[10].value);

      if (continue_verification == true)
		check_string(form_location.elements[13]);

      if (continue_verification == true)
		check_string(form_location.elements[14]);

      if (continue_verification == true) {
         continue_verification = opener.checkNewEntry(opener.document.forms["userForm"].elements["userTable"], form_location.elements[9].value);
         if (continue_verification != true) {
            alert("User already exists.");
            return;
         }
      }
    } 

    //[LMT] Check if the username has the same name of Linux system groups
    function chk_regular_user()
    {
        continue_verification = true;
        var group_name = new Array("bin","daemon","sys","adm","tty","disk","lp","mem","kmem","nobody","wheel","admin","biouser","sshd","dialout","pam");

        for (var i = 0; i < group_name.length; i++)
        {
            if(document.configForm.elements[9].value == group_name[i])
            {
                alert("You cannot create this user. Please, choose other username and try again!");
                continue_verification = false;
                document.configForm.close();
                break;
            }
        }

        return;
    }


	function addUserSubmit()
	{
	try {
	    if (opener.window.name != document.configForm.target) {
           alert("The page which called this form is not available anymore. The changes will not be effective.");
           self.close();
           return;
        }

	    chk_regular_user();

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
</head>

<body onload="setFocus();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>" onSubmit="return false;">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/security/secUsersGroups.asp">
   <input type="hidden" name="urlError" value="/normal/security/secUsersGroups.asp">
   <input type="hidden" name="request" value=<%write(getRequestNumber());%>>
      <input type="hidden" name="system.access.mode" value="1">
      <input type="hidden" name="system.access.action" value="0">
      <input type="hidden" name="system.access.usersCs" value="">
      <input type="hidden" name="system.access.groupsCs" value="">

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

