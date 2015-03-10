<html>
<head>

  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
  <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
  <script language="JavaScript" type="text/JavaScript">

  <%
      // Set page-wide variables:
      var self = "system.applications.menuShell.";
      var self2 = "system.applications.menuShell.addModOption.";
      var menuoption = getQueryVal("menuoption", -1);
      var title = getQueryVal("title", "");
      var command = getQueryVal("command", "");
      set("_sel1_", menuoption);
	  ReqNum = getRequestNumber();
   %>

  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  var option = "<%write(menuoption);%>";
  
  function setFocus()
  {
    document.configForm.elements[8].focus();
  }

  function form_verification()
  {           
      var form_location = document.configForm
      checkElement (form_location.elements[8], true, true, false, false);
      // checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)
      if (continue_verification == true)
      checkElement (form_location.elements[9], true, false, false, false);
   }    

   function addOpt()
   {
      document.configForm.elements[5].value = getValues(opener.document.appsTermProMenuForm.actionTable, 0);
      document.configForm.elements[5].value += document.configForm.elements[8].value + "!" + document.configForm.elements[9].value + ",";
      document.configForm.elements[6].value = opener.document.appsTermProMenuForm.actionTable.options.length;
   }

   function editOpt()
   {
      opener.document.appsTermProMenuForm.actionTable.options[option].value = document.configForm.elements[8].value + "!" + document.configForm.elements[9].value;
      document.configForm.elements[5].value = getValues(opener.document.appsTermProMenuForm.actionTable, 0);
      document.configForm.elements[6].value = opener.document.appsTermProMenuForm.actionTable.options.length-1;
   }
  
  function submitOption()
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
         if (option == "-1") {
            addOpt();
         } else {
            editOpt();
         }
	document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
         document.configForm.elements[7].value = opener.document.appsTermProMenuForm.elements[0].value;
         setQuerySSID(document.configForm);
         document.configForm.submit();
         self.close();
       }   
	} catch (error) {
	  self.close();
	}
  }

  </script>
 <title><% if (-1 == menuoption) write('Add Option'); else write('Edit Option'); %></title>
</head>

<body onload="passSessionID(); setFocus();" class="bodyMain">
<table border="0" align="center" cellpadding="0" cellspacing="10">
  <tr valign="top"> 
    <td height="40" colspan="2" align="center">
      <a href="javascript:submitOption();">
      <img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
        &nbsp;&nbsp;&nbsp;&nbsp;
      <a href="javascript:window.close();">
      <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
    </td>
  </tr>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/applications/appsTermProMenu.asp">
   <input type="hidden" name="urlError" value="/normal/applications/appsTermProMenu.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
<!--*********************************************************************************-->
   <input type="hidden" name="<%write(self);%>optionsCs" value="">
   <input type="hidden" name="<%write(self);%>numOptions" value="">
   <input type="hidden" name="<%write(self);%>menuTitle" value="">

  <tr valign="bottom"> 
    <td align="right"><font class="label">Title</font></td>
    <td align="left"><input class="formText" name="<%write(self2);%>title" type="text" size="15" value="<%write(title);%>" maxlength="49"></td>
  </tr>
  <tr valign="bottom">
    <td align="right"><font class="label">Action/Command</font></td>
    <td align="left"><input class="formText" name="<%write(self2);%>command" type="text" size="15" value="<%write(command);%>" maxlength="49"></td>
  </tr>
</table>
</form>
</body>
</html>
