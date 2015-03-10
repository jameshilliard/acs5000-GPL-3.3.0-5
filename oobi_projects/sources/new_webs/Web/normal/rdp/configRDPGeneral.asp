<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
  
<% ReqNum = getRequestNumber(); %>

	var msgE="<% get("system.rdpconf.device.deviceMsgError");%>";

  	parent.currentPage = self;
  	parent.helpPage = 'helpPages/RDPDeviceHelp.asp';
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    var target='target=' + window.name;
   
  	function init()
  	{
  	if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
  	{
        if (checkLeftMenu('/normal/rdp/configRDPMenu.asp') == 0) {
           parent.menuReady = 0;
           setTimeout('init()', 200);
           return;
        }
        parent.menu.leftHeadMenu("RDP");
        setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
        parent.topMenu.selectItem(parent.menu.topItem);
        parent.menu.selectItem("RDP");
		if (msgE != "OK" && msgE != "") {
			alert(msgE);
		}
  	}
  	else
    	setTimeout('init()', 200);
  	}
  
	function copyData()
	{
		document.configForm.elements[5].value = ',' + getValues(document.RDPDeviceForm.deviceTable,0);
		document.configForm.elements[8].value = document.RDPDeviceForm.rdpport.value;
	}

  	function deleteDevice ()
  	{
      		var table = document.RDPDeviceForm.deviceTable;
		
		if (table.selectedIndex == -1 || table.options[table.selectedIndex].value == '-1') {
   			alert('Please select an RDP server to delete.');
			return;		
		}
		deleteSelOpt(document.RDPDeviceForm.deviceTable);
	}

  	function editDevice ()
  	{
      		var table = document.RDPDeviceForm.deviceTable;
		
		if (table.selectedIndex == -1 || table.options[table.selectedIndex].value == '-1') {
   			alert('Please select an RDP server to edit.');
			return;		
		}
		copyData();
		document.configForm.elements[6].value = "3";
  		getSelectedEdit('configRDPDeviceEntry.asp','deviceSel','450','350',target+"&SSID=<%get("_sid_");%>",document.RDPDeviceForm.deviceTable,'yes',1);
  	}
  
  	function addDevice ()
  	{
		copyData();
		document.configForm.elements[6].value = "2";
  		getSelectedAdd('configRDPDeviceEntry.asp','deviceSel','450','350',target+"&SSID=<%get("_sid_");%>",'yes');
  	}

    function form_verification ()
    {
        continue_verification = true;
        var form_location = document.RDPDeviceForm;
        checkRanges (form_location.rdpport, 1, 8, 1024, 65535, true, true);
	}

  	function submit()
  	{
        form_verification();
        if (continue_verification == true) {
   		   copyData();
		   document.configForm.submit();
		   parent.mainReady = null;
        }
  	}
    
    </script>
</head>
<body onload="init();" class="bodyMain">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<form name="RDPDeviceForm" method="POST" action="/goform/Dmf">
<tr>
   <td align="center" colspan="4" valign="middle" height="40">
   <font class="label">&nbsp;RDP Viewer Ports</font>
   <input name="rdpport" type="text" size="39" maxlength="39" class="formText" value="<% get("system.rdpconf.device.rdpport"); %>">
   </td>
</tr>
<tr>
	<td align="center">	
<table border="0" align="center" cellpadding="0" cellspacing="0" class="tableColor">
 <tr align="left" valign="bottom"> 
    <td>
	<table border="0" cellpadding="0" cellspacing="0">

      <tr align="left" class="tableColor">         		
      <td width="187"><font class="tabsTextBig">&nbsp;Server Name</font></td>
      <td width="143"><font class="tabsTextBig">IP Address</font></td>
      <td width="55"><font class="tabsTextBig">Port</font></td>
      <td width="80"><font class="tabsTextBig">KVM Port</font></td>
      </tr>
    </table>
	</td>
 </tr>
 <tr align="left">
    <td align="left" valign="top"><font class="formText">
	<select name="deviceTable" size="10" class="formText">            
		<%get("system.rdpconf.device.deviceTableHtml");%>
		<option value="-1">
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		</option>
	</select>
	</td>
 </tr>
</table>
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
	<td><a href="javascript:addDevice();"><img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a></td>
	<td><a href="javascript:editDevice();"><img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a></td>
	<td><a href="javascript:deleteDevice()"><img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a></td>
</tr>
</table>	
	</td>
</tr>
</form>
<tr>
	<td align="center">
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/rdp/configRDPGeneral.asp">
   <input type="hidden" name="urlError" value="/normal/rdp/configRDPGeneral.asp">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">
   <input type="hidden" name="system.rdpconf.device.deviceCs" value="">
   <input type="hidden" name="system.rdpconf.device.action" value="0">
   <input type="hidden" name="system.rdpconf.device.deviceSel" value="">
   <input type="hidden" name="system.rdpconf.device.rdpport" value="">
</form>	
	</td>
</tr>
</table>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>  
</body>
</html>
