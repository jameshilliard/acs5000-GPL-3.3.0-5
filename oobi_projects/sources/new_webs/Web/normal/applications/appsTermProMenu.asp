<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

  <% ReqNum = getRequestNumber(); %>
  
  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = 'helpPages/appsTermProMenuHelp.asp';
  var target='target=' + window.name;

  function init()
  {
    if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    {
      if (checkLeftMenu('/normal/applications/appsMenu.asp') == 0) {
         parent.menuReady = 0;
         setTimeout('init()', 200);
         return;
      }
      if (parent.access == "easy")
         setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
      parent.topMenu.selectItem(parent.menu.topItem);
      parent.menu.selectItem("TPM");
    }
    else
      setTimeout('init()', 200);
  }
  
	function deleteMenuOption(formTable) {
		if (formTable.options[formTable.selectedIndex].value != "-1") {
			deleteSelOpt(formTable);
			document.configForm.elements[7].value = 1;
			setQuerySSID(document.configForm);
			submit(0);
		} else {
			alert('Please select an item to delete.');
		}
	}
  
   function editOption(index)
   {
      if (document.appsTermProMenuForm.actionTable.options[index].value == '-1') {
         alert('Please select an item to edit.');
      } else {
         var option;
         var array;

         option = document.appsTermProMenuForm.actionTable.options[index].value;
         array = option.split("!");
         newRef = 'appsTermProMenuEntry.asp' + "?menuoption=" + index + "&" + target + "&title=" + array[0] + "&command=" + array[1];
         newWindow(newRef,'name','350','250','no');
      }
   }
     
   function form_verification()
   {    
      var form_location = document.appsTermProMenuForm
      
      if (form_location.elements[1].length > 1)
      {
         checkElement (form_location.elements[0], true, false, false, false);
      }
      else {continue_verification = true;}
   } 
  
  function submit(verify)
  {  
      if (verify == 0) // will not make form verification
      {
          document.configForm.elements[5].value = getValues(document.appsTermProMenuForm.actionTable, 0);
          document.configForm.elements[6].value = document.appsTermProMenuForm.actionTable.length-1;
          document.configForm.elements[8].value = document.appsTermProMenuForm.elements[0].value;
          document.configForm.submit();
      }
      else if (verify == undefined)
      {
         form_verification();
         if (continue_verification == true)
         {  
            document.configForm.elements[5].value = getValues(document.appsTermProMenuForm.actionTable, 0);
            document.configForm.elements[6].value = document.appsTermProMenuForm.actionTable.length-1;
            document.configForm.elements[8].value = document.appsTermProMenuForm.elements[0].value;
            document.configForm.submit();
            parent.mainReady = null;
         }
      }                 
   }
   </script>
</head>
<body onload="init();" class="bodyMain">
<form name="appsTermProMenuForm" onSubmit="return false;">
<table width="300"  border="0" align="center" cellpadding="0" cellspacing="0">
  <tr align="left" valign="top"> 
    <td width="100" height="40"><font class="label">Menu title</font></td>
    <td width="200" height="40"><input class="formText" type="text" maxlength="49" name="menuTitle" size="15" value="<%get("system.applications.menuShell.menuTitle");%>"></td>
  </tr>
  <tr align="center" valign="top"> 
    <td height="30" colspan="2"><font class="tabsTextBig">Menu Options</font></td>
  </tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" align="center"  class="tableColor">          
  <tr align="left" valign="top">
      <td>
     <table border="0" cellpadding="0" cellspacing="0">
          <tr align="left" class="tableColor">             
          <td width="184"><font class="tabsTextBig">&nbsp;Action Name</font></td>
          <td><font class="tabsTextBig">Action/Command</font></td>
          </tr>
         </table>
    </td>
  </tr>
  <tr align="left" valign="top">
      <td>
  <!-- options width = 
  1 (0 ... 18); 2 whitespace; (205px) 
  2 (0 ... 20)
  option -1 == 44-->
<select  name="actionTable" size="10" class="formText">
<% get("system.applications.menuShell.menuOptionHtml"); %>
</select>
    </td>
  </tr>
</table>
<table border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>      
    <td align="center" height="40" valign="middle">
        <a href="javascript:editOption(document.appsTermProMenuForm.actionTable.selectedIndex)">
      <img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;          
          <a href="javascript:deleteMenuOption(document.appsTermProMenuForm.actionTable)">
      <img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
        <a href="javascript:getSelectedAdd('appsTermProMenuEntry.asp','menuoption','350','250',target,'yes')">
      <img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
          <a href="javascript:optionUp(document.appsTermProMenuForm.actionTable,document.appsTermProMenuForm.actionTable.selectedIndex)">
      <img src="/Images/upButton.gif" width="47" height="21" border="0" alt=""></a>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
          <a href="javascript:optionDown(document.appsTermProMenuForm.actionTable,document.appsTermProMenuForm.actionTable.selectedIndex)">
      <img src="/Images/downButton.gif" width="47" height="21" border="0" alt=""></a>
    </td>
  </tr>
</table>
</form>  
<form name="configForm" method="POST" action="/goform/Dmf">
      <input type="hidden" name="system.req.action" value="">
      <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
      <input type="hidden" name="urlOk" value="/normal/applications/appsTermProMenu.asp">
      <input type="hidden" name="urlError" value="/normal/applications/appsTermProMenu.asp">
      <input type="hidden" name="request" value="<%write(ReqNum);%>">
<!----------------------------------------------------------------------------------->
      <input type="hidden" name="system.applications.menuShell.optionsCs" value="">
      <input type="hidden" name="system.applications.menuShell.numOptions" value="">
      <input type="hidden" name="system.applications.menuShell.action" value="">
      <input type="hidden" name="system.applications.menuShell.menuTitle" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
