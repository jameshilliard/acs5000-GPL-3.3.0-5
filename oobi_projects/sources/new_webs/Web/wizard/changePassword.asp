<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Change User Password</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
     <script type="text/javascript">
      <%
        var user = getQueryVal("user", -1);
        set("system.access.selected", user);
      %>

      function form_verification()
	  {	   	  
	  var form_location = document.configForm
	  checkElement (form_location.elements[8], true, true, false, false);
	  // checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)
	  if (continue_verification == true)
	  checkElement (form_location.elements[9], true, true, false, false, 'no', 'no', true, form_location.elements[8].value);
	  }	 
	  
	  function changePasswdSubmit()
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

<body onload="setFocus();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
  <input type="hidden" name="system.req.action" value="">
  <input type="hidden" name="system.req.sid" value="">
  <input type="hidden" name="urlOk" value="/wizard/access.asp">
  <input type="hidden" name="urlError" value="/wizard/access.asp">
  <input type="hidden" name="request" value=<%write(getRequestNumber());%>> 
<!--*************************************************************************-->
  <input type="hidden" name="system.access.mode" value="0">
  <input type="hidden" name="system.access.action" value="1">
  <input type="hidden" name="system.access.usersCs" value="">

<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
  <td align="center" valign="middle">
    <table border="0" cellpadding="5" cellspacing="0">
	     <tr> 
          <td colspan="2" align="center"  height="40"  valign="top"> 
		         <a href="javascript:changePasswdSubmit();">
		         <img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a> 
		         &nbsp;&nbsp;&nbsp;&nbsp;
             <a href="javascript:window.close();">
		         <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
          </td>
       </tr>
       <tr  height="40"  valign="middle"> 
          <td align="left"><font class="label">New Password</font></td>
          <td> 
             <input class="formText" type="password" maxlength="30" name="system.access.addModUser.password" size="15" value="">
          </td>
       </tr>
       <tr  height="40" valign="middle"> 
          <td align="left"><font class="label">Repeat <br> New Password</font></td>
          <td> 
             <input class="formText" type="password" maxlength="30" name="system.access.addModUser.password2" size="15" value="">
          </td>
       </tr>
   </table>
  </td>
</tr>
</table>
</form>
</body>
</html>

