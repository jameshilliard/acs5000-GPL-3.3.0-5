<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
  
<% ReqNum = getRequestNumber(); %>

	var msgE="<% get("system.kvm.conf.device.deviceMsgError");%>";

  	parent.currentPage = self;
  	parent.helpPage = 'helpPages/KVMDeviceHelp.asp';
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    var target='target=' + window.name;
   
  	function init()
  	{
  	if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
  	{
        if (checkLeftMenu('/normal/kvm/configKVMMenu.asp') == 0) {
           parent.menuReady = 0;
           setTimeout('init()', 200);
           return;
        }
    	setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
        parent.topMenu.selectItem(parent.menu.topItem);
        parent.menu.selectItem("DEV");
		if (msgE != "OK" && msgE != "") {
			alert(msgE);
		}
  	}
  	else
    	setTimeout('init()', 200);
  	}
  
	function copyData()
	{
		document.configForm.elements[5].value = ',' + getValues(document.KVMDeviceForm.deviceTable,0);

	}

  	function deleteDevice ()
  	{
      		var table = document.KVMDeviceForm.deviceTable;
		
		if (table.selectedIndex == -1 || table.options[table.selectedIndex].value == '-1') {
   			alert('Please select a device to delete.');
			return;		
	 	} else {
			if (table.options[table.selectedIndex].value == '1') {
				alert('You can not delete the Master device');
				return;
			}
		}
		optname = table.options[table.selectedIndex].text;
		if (optname.substring(20,26) == 'master') {
			p = optname.substring(27,37).split(',');
			for (i = 0; i < p.length; i ++) {
				j = parseInt(p[i]);
				if (j > 0 && j <= ports) {
					portlist[j - 1] = 0;
				}
			}
		}
		deleteSelOpt(document.KVMDeviceForm.deviceTable);
	}

  	function editDevice ()
  	{
      		var table = document.KVMDeviceForm.deviceTable;
		
		if (table.selectedIndex == -1 || table.options[table.selectedIndex].value == '-1') {
   			alert('Please select a device to edit.');
			return;		
	 	} else {
			if (table.options[table.selectedIndex].value == '1') {
				alert('You can not edit the Master device.');
				return;
			}
		}
		copyData();
		document.configForm.elements[6].value = "3";
  		getSelectedEdit('configKVMDeviceEntry.asp','deviceSel','450','350',target+"&SSID=<%get("_sid_");%>",document.KVMDeviceForm.deviceTable,'yes',1);
  	}
  
  	function addDevice ()
  	{
		for (i = 0; i < ports; i ++) {
			if (portlist[i] == 0) {
				break;
			}
		}
		if (i == ports) {
			alert('There is no physical port in the master device to do cascading.');
			return;
		}
		copyData();
		document.configForm.elements[6].value = "2";
  		getSelectedAdd('configKVMDeviceEntry.asp','deviceSel','450','350',target+"&SSID=<%get("_sid_");%>",'yes');
  	}
  
	function listPorts()
  	{                                                               
      		var table = document.KVMDeviceForm.deviceTable;
		
		if (table.selectedIndex == -1 || table.options[table.selectedIndex].value == '-1')
   			{alert('Please select an item.');}		
	 	else
	  		{
	  		document.configForm.urlOk.value = '/normal/kvm/configKVMPorts.asp';
			copyData();
			document.configForm.elements[6].value = "4";
			document.configForm.elements[7].value = table.options[table.selectedIndex].value;
			setQuerySSID(document.configForm);
      			document.configForm.submit();
			} 
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
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<form name="KVMDeviceForm" method="POST" action="/goform/Dmf">
<tr>
	<td align="center">	
<table border="0" align="center" cellpadding="0" cellspacing="0" class="tableColor">
 <tr align="left" valign="bottom"> 
    <td>
	<table border="0" cellpadding="0" cellspacing="0">
       <tr align="left" class="tableColor">         		
          <td width="187"><font class="tabsTextBig">&nbsp;Device Name</font></td>
		  <td width="178"><font class="tabsTextBig">Physical ID</font></td>
		  <td><font class="tabsTextBig">Number of Ports</font></td>
        </tr>
    </table>
	</td>
 </tr>
 <tr align="left">
    <td align="left" valign="top"><font class="formText">
 	<!-- option = 
				Device Name (1-15 characters)+3 white spaces (&nbsp;)+ 
				Physical ID (1-15 characters)+3 white spaces (&nbsp;)+
				Number of Ports (1-11 characters);	
	--> 	
	<select name="deviceTable" size="10" class="formText">            
		<%get("system.kvm.conf.device.deviceTableHtml");%>
		<option value="-1">
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		</option>
	</select>
	</td>
 </tr>
</table>
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
   <script type="text/javascript">
   if (top.family_model != "ONS") {
      var portlist = new Array(<% get("system.kvm.conf.device.portsUsedHtml");%>);
      var ports = <%get("system.device.number");%>;
      document.write('<td><a href="javascript:editDevice();"><img src="/Images/editDevice.gif" width="80" height="21" border="0" alt=""></a></td>');
      document.write('<td><a href="javascript:deleteDevice()"><img src="/Images/deleteDevice.gif" width="80" height="21" border="0" alt=""></a></td>');
      document.write('<td><a href="javascript:addDevice();"><img src="/Images/addDevice.gif" width="80" height="21" border="0" alt=""></a></td>');
    }
    </script>
	<td><a href="javascript:listPorts();"><img src="/Images/ports.gif" width="47" height="21" border="0" alt=""></a></td>
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
   <input type="hidden" name="urlOk" value="/normal/kvm/configKVMDevice.asp">
   <input type="hidden" name="urlError" value="/normal/kvm/configKVMDevice.asp">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">
   <input type="hidden" name="system.kvm.conf.device.deviceCs" value="">
   <input type="hidden" name="system.kvm.conf.device.action" value="0">
   <input type="hidden" name="system.kvm.conf.device.deviceSel" value="">
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
