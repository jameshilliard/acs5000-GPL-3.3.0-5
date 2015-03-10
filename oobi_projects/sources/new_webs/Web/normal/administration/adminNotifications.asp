<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

  <%
	var self = "system.administration.notifications.";
	ReqNum = getRequestNumber();
  %>	
  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    
    var target='target=' + window.name;

	function copyData()
	{
		if (document.notifForm.elements[0].checked == false)
			document.configForm.elements[5].value = 0;
		else
			document.configForm.elements[5].value = 1;
	
		document.configForm.elements[6].value = "," + getValues(document.notifForm.notifTable,0);
	}

	function deleteNotif()
	{
		deleteSelOpt(document.notifForm.notifTable);
	}

	function choosePage(selectedOption)
	{
		copyData();
		document.configForm.elements[7].value = "-1";
		document.configForm.elements[8].value = selectedOption;
		getSelectedEdit('adminNotifEntry.asp','notifType','500','400',target+"&SSID=<%get("_sid_");%>",document.addForm.addType,'yes');
	}
    
	function chooseNotif(selectedOption)
	{
		copyData();
		getValueSelectedEdit('adminNotifEntry.asp','triggerId','500','400',target+"&SSID=<%get("_sid_");%>",document.notifForm.notifTable,'yes');
	}

	parent.currentPage = self;
	parent.helpPage = 'helpPages/adminNotificationsHelp.asp';
      
    if ((parent.family_model == "ACS")||(parent.family_model == "AVCS"))
      var topMenu = '/normal/administration/adminMenu.asp';
    else
      var topMenu = '/normal/ports/portsMenu.asp';

      function init()
      {
         if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
         {
            if (checkLeftMenu(topMenu) == 0) {
               parent.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
            setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
            parent.topMenu.selectItem(parent.menu.topItem);
            parent.menu.selectItem("NTF");
         }
         else
            setTimeout('init()', 200);
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


<form name="configForm" method="POST" action="/goform/Dmf">         
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/administration/adminNotifications.asp">
	<input type="hidden" name="urlError" value="/normal/administration/adminNotifications.asp">
	<input type="hidden" name="request" value=<%write(ReqNum);%>>
	<input type="hidden" name="<%write(self);%>alarm" value="">
	<input type="hidden" name="<%write(self);%>notifTable" value="">
	<input type="hidden" name="<%write(self);%>triggerId" value="-1">
	<input type="hidden" name="<%write(self);%>notifType" value="-1">
</form>

<form name="notifForm" action="" onSubmit="return false;">
	<div align="center">
	<font class="label">Notification Alarm For Data Buffering </font>
		<% var checked = getVal(self+"alarm");%>
		<input name="<%write(self);%>alarm" type="checkbox" <%if (checked!=0) write("checked");%>>
	</div>
<table border="0" cellpadding="0" cellspacing="0" align="center" class="tableColor">	
<br>
	<tr>
      <td>      
         <table border="0" cellpadding="0" cellspacing="0">
         	<tr align="left">
         		<td width="104"><font class="tabsTextBig">&nbsp;Type</font></td>
         		<td><font class="tabsTextBig">Trigger</font></td>
         	</tr>
         </table>
		</td>
	</tr>
	<tr>
      <td>
	 <select name="notifTable" size="8" class="formText">
		<% get(self + "notifHtml"); %>
			<option value="-1" selected>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
	 </select>
	     
</td>
</tr>
</table>
</form>
<form name="addForm" method="POST" onSubmit="return false;">
<table border="0" cellpadding="2" cellspacing="0" align="center">	
   <tr>
      <td align="center" height="40" valign="middle"> 
         <a href="javascript:chooseNotif(document.notifForm.notifTable.selectedIndex);">
         <img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
         &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
         <a href="javascript:deleteNotif();">
         <img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>   
         &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;         
         <a href="javascript:choosePage(document.addForm.addType.selectedIndex);">
         <img src="/Images/addButton.gif" width="47" height="21" border="0" alt="""></a>                      
      </td>
   </tr>
   <tr>
      <td align="center" height="30" valign="middle">
         <select name="addType" class="formText">
          	<option value="0" selected> Email </option>       
               <option value="1" > Pager </option> 
               <option value="2" > SNMP Trap </option>      	
            </select>
       </td>
   </tr>
</table>
</form> 
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
