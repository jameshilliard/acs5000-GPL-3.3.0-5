
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

   <%
      var ReqNum = getRequestNumber();
      var SID = getVal("_sid_");
      var admin = getLabelVal("system.access.admUser");
   %>

  window.name = '<%get("system.device.ip");%>_<%write(SID);%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  var target='target=' + window.name;
  var ssid="&SSID=<%get("_sid_");%>";

   var page = 0;
   parent.helpPage = 'helpPages/accessHelp.asp'; 

       function checkNewEntry(table,entry, val)
       {
          for (var i=0;i<table.options.length;i++)
          {
              if ((table.options[i].text == entry) && (table.options[i].value != val))
                return false;
          }
          return true;
       }

   function init()
   {
      if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
      {
		 page = parent.menu.getPageIndex('ACC', parent.menu.menuPages);
         parent.menu.selectItem('ACC');
         setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
      }
      else
         setTimeout('init()', 200);
   }

   function deleteUser(formTable)
   {
       if (formTable.options[formTable.selectedIndex].value == '-1')
       {
            alert('Please select an item to delete.');
       }
       else
       {
			if (formTable.options[formTable.selectedIndex].text == "root") {
				alert('root user cannot be deleted.');
			} else if (formTable.options[formTable.selectedIndex].text == "<%write(admin);%>") {
                alert('<%write(admin);%> user cannot be deleted.');
            } else if (formTable.options[formTable.selectedIndex].text == "Generic User") {
				alert('Generic User cannot be deleted.');
			} else {
                deleteSelOpt(formTable);
            }
       }
   }

   function submit() 
   {                                                                                         
       document.configForm.elements[7].value = ',' + getValues(document.userForm.userTable, 0);         
       document.configForm.submit();
	   parent.mainReady = null;
   }
    </script>
</head>

<body onload="init();" class="bodyMain">
<form name="userForm" method="POST" action="" onSubmit="return false;">
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="450" align="center">
   <tr>
      <td>
         <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
            <tr>
               <td align="center" class="label_for_tables">
<script type="text/javascript">
if (top.family_model == "ONS") {
    document.write("By default, no users except the \"<%write(admin);%>\" can access <br>",
				"any KVM or serial ports through the Web Manager.<br>",
				"Configuring users' access to ports is done in Expert mode.<br>",
				"See the <I>Quick Start Guide or the User's Guide</I> for details.<br>");
} else if (top.family_model == "KVM") {
    document.write("By default, no users except the \"<%write(admin);%>\" can access <br>",
				"any KVM port through the Web Manager.<br>",
				"Configuring users' access to ports is done in Expert mode.<br>",
				"See the <I>Quick Start Guide</I> or <I>the User's Guide</I> for details.<br>");
} else {
    document.write("Add users who will be able to access all ports.<br>",
                  "By default all users can access all ports.");
}
</script>
                </td>
             </tr>
         </table>
      </td>
    </tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" align="center">
   <tr>
      <td>&nbsp;</td>
   </tr>
   <tr>
      <td align="left" class="tableColor">         
         <font class="tabsTextBig">&nbsp;Users</font>         
      </td>
   </tr>
   <tr>
      <td align="left">            
         <select name="userTable" size="8" class="formText">
            <%get("system.access.usersHtml");%>
            <option value="-1" selected>
            &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
            &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
            </option>
         </select>
      </td>
   </tr>
</table> 
<table border="0" cellpadding="0" cellspacing="0" align="center">
   <tr>
      <td align="center" height="40" valign="middle">
         <a href="javascript:getSelectedAdd('addModUser.asp','user','500','400',target+ssid, 'yes')">
         <img src="/Images/addButton.gif" alt="" width="47" height="21" border="0"></a>
            &nbsp;&nbsp;&nbsp;&nbsp;
         <a href="javascript:getValueSelectedEdit('changePassword.asp','user','350','250',target+ssid, document.userForm.userTable,'yes',1)">
         <img src="/Images/changePassButton.gif" alt="" width="108" height="21" border="0"></a>
            &nbsp;&nbsp;&nbsp;&nbsp;
         <a href="javascript:deleteUser(document.userForm.userTable)">
         <img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
      </td>
   </tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%write(SID);%>">
   <input type="hidden" name="urlOk" value="/wizard/access.asp">
   <input type="hidden" name="urlError" value="/wizard/access.asp">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">
<!--*************************************************************************-->
   <input type="hidden" name="system.access.mode" value="0">
   <input type="hidden" name="system.access.action" value="0">
   <input type="hidden" name="system.access.usersCs" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
parent.controls.document.getElementById('back').style.visibility = "visible";
parent.controls.document.getElementById('next').style.visibility = "visible";

</script>
<%set("_sid_", "0");%>
</body>
</html>
