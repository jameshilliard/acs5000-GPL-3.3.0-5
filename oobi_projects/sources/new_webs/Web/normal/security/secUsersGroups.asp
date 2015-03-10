<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
    <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

    <% ReqNum = getRequestNumber(); %>

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
    parent.helpPage = 'helpPages/secUsersGroupsHelp.asp';

    var target = 'target=' + window.name;

    function init()
    {
       if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
       {
          if (checkLeftMenu('/normal/security/securityMenu.asp') == 0) {
             parent.menuReady = 0;
             setTimeout('init()', 200);
             return;
          }
          setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
          parent.topMenu.selectItem(parent.menu.topItem);
          parent.menu.selectItem("USR");
       }
       else
          setTimeout('init()', 200);
    }

    function checkNewEntry(table,entry)
    {
	var type = document.configForm.elements[6].value;
       for (var i=0;i<table.options.length;i++)
       {
          if (table.options[i].text == entry) {
		if (type == "3" && (i == table.selectedIndex)) {
			return true;
		} else {
            		return false;
		}
	  }
       }
       return true;
    }

	function copyData()
	{
		document.configForm.elements[7].value = ','+getValues(document.userForm.userTable, 0);
		document.configForm.elements[8].value = ','+getValues(document.userForm.groupTable, 0);
	}
 
	function deleteUser(formTable)
	{
		if (formTable.options[formTable.selectedIndex].value == '-1') {
			alert('Please select an item to delete.');
		} else {
			if (formTable.options[formTable.selectedIndex].text == "root") {
				alert('root user cannot be deleted.');
			} else {
				if (formTable.options[formTable.selectedIndex].text == "admin") {
					alert('admin user cannot be deleted.');
				} else {
             				deleteSelOpt(formTable);
				}
			}
		}
	}

	function editUser()
	{
		if (document.userForm.userTable.options[document.userForm.userTable.selectedIndex].value == '-1') {
			alert('Please select an item to edit.');
			return;
		} 
		copyData();
		document.configForm.elements[6].value = '1';
		getValueSelectedEdit('changePassword.asp','user','350','250',target+"&SSID=<%get("_sid_");%>",document.userForm.userTable, 'yes',1);
	}
      
	function addUser()
	{
		copyData();
		document.configForm.elements[6].value = '2';
		getSelectedAdd('secUsersEntry.asp','user','450','300',target+"&SSID=<%get("_sid_");%>",'yes');
	}
      
	function editGroup()
	{
		if (document.userForm.groupTable.options[document.userForm.groupTable.selectedIndex].value == '-1') {
			alert('Please select an item to edit.');
			return;
		} 
		copyData();
		document.configForm.elements[6].value = '3';
		getSelectedEdit('secGroupsEntry.asp','group','350','250',target+"&SSID=<%get("_sid_");%>", document.userForm.groupTable, 'yes',1);
	}
      
	function addGroup()
	{
		copyData();
		document.configForm.elements[6].value = '4';
		getSelectedAdd('secGroupsEntry.asp','group','350','250',target+"&SSID=<%get("_sid_");%>",'yes');
	}
      
	function deleteGroup()
	{
		deleteSelOpt(document.userForm.groupTable);
	}

	function submit()
	{
		copyData();
		document.configForm.submit();
		parent.mainReady = null;
	}
      
    </script>
</head>

<body onload="init();" class="bodyMain">
<form name="userForm" method="POST" action="" onSubmit="return false;">
<br>
<table border="0" align="center" cellpadding="0" cellspacing="0">
  <tr align="center" valign="top"> 
    <td width="300">
       <table border="0" align="center" cellpadding="0" cellspacing="0">
         <tr align="center" valign="top"> 
           <td class="tableColor">
             <font class="tabsTextBig">&nbsp;User List</font>
           </td>
         </tr>
         <tr align="center"> 
           <td>
             <select name="userTable" size="12" class="formText" width="100%">
                <%get("system.access.usersHtml");%>
                <option value="-1" selected>
                  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
                  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
                </option>
             </select>
           </td>
         </tr>
        </tr>
      </table
     </td>
    <td width="250">
       <table border="0" align="center" cellpadding="0" cellspacing="0">
         <tr align="center" valign="top"> 
           <td class="tableColor">
             <font class="tabsTextBig">&nbsp;Group List</font>
           </td>
         <tr align="center"> 
           <td>
             <select name="groupTable" size="12" class="formText" width="100%">
                <%get("system.access.groupsHtml");%>
                <option value="-1" selected>
                  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
                  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
                </option>
             </select>
           </td>
         </tr>      
       </table
      </td>
    </tr>
    <tr align="center" valign="middle"  height="40"> 
       <td>
             <a href="javascript:addUser()">
             <img src="/Images/addButton.gif" alt="" width="47" height="21" border="0"></a>&nbsp;&nbsp;
             <a href="javascript:deleteUser(document.userForm.userTable)">
             <img src="/Images/deleteButton.gif" alt="" width="47" height="21" border="0"></a>&nbsp;&nbsp;
             <a href="javascript:editUser()">
             <img src="/Images/changePassButton.gif" alt="" width="108" height="21" border="0"></a> 
      </td>
      <td>
              <a href="javascript:addGroup()">
              <img src="/Images/addButton.gif" alt="" width="47" height="21" border="0"></a>&nbsp;&nbsp;
              <a href="javascript:deleteGroup();">
              <img src="/Images/deleteButton.gif" alt="" width="47" height="21" border="0"></a>&nbsp;&nbsp;
              <a href="javascript:editGroup()">
              <img src="/Images/editButton.gif" alt="" width="47" height="21" border="0"></a>
      </td>
      </tr>
   </table>
</form>
   <form name="configForm" method="POST" action="/goform/Dmf">
      <input type="hidden" name="system.req.action" value="">
      <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
      <input type="hidden" name="urlOk" value="/normal/security/secUsersGroups.asp">
      <input type="hidden" name="urlError" value="/normal/security/secUsersGroups.asp">
      <input type="hidden" name="request" value="<%write(ReqNum);%>">
      <input type="hidden" name="system.access.mode" value="1">
      <input type="hidden" name="system.access.action" value="0">
      <input type="hidden" name="system.access.usersCs" value="">
      <input type="hidden" name="system.access.groupsCs" value="">
   </form>
<script type="text/javascript">
parent.mainReady = true;
</script>   
<%set("_sid_","0");%>
</body>
</html>
