<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
  
<% ReqNum = getRequestNumber(); %>


  parent.currentPage = self;
  parent.helpPage = 'helpPages/KVMPortsHelp.asp';
  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  var target='target=' + window.name;
   
	function init()
	{
		  var msgE="<% get("system.kvm.conf.device.term.deviceMsgError");%>";
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
		
	function copySelectedPorts ()
	{		
		var sel=0;
		var table = document.portsForm.portsTable;
		
		document.configForm.elements[6].value = "";

		for (var i=0; i < table.options.length; i++) {
			if (table.options[i].selected) {
				sel += 1;
				document.configForm.elements[6].value += "," + 
					table.options[i].value;
			}
		}
		document.configForm.elements[6].value += "," ;
		return sel;		
	}
	
	function modifyPort ()
	{
		var ret ;

		ret = copySelectedPorts();
		if (ret == "0") {
   			alert('Please select a port to modify.');
			document.configForm.elements[6].value = "";
			return;
		}
		
		if (ret != "1") {
   			alert('Please select only one port to modify.');
			document.configForm.elements[6].value = "";
			return;
		}
		
		document.configForm.elements[5].value = "3";
		getSelectedEdit('configKVMPortsEntry.asp','term','400','300',target+"&SSID=<%get("_sid_");%>",document.portsForm.portsTable,'yes',1);	
	}
	
	function enablePort ()
	{
		if (copySelectedPorts() == "0") {
   			alert('Please select a port to enable.');
			document.configForm.elements[6].value = "";
			return;
		}
		
		document.configForm.elements[2].value = "/normal/kvm/configKVMPorts.asp";
		document.configForm.elements[5].value = "2";
		setQuerySSID(document.configForm);
		document.configForm.submit();
		parent.mainReady = null;	
	}
	
	function disablePort()
	{
		if (copySelectedPorts() == "0") {
   			alert('Please select a port to disable.');
			document.configForm.elements[6].value = "";
			return;
		}
		
		document.configForm.elements[2].value = "/normal/kvm/configKVMPorts.asp";
		document.configForm.elements[5].value = "1";
		setQuerySSID(document.configForm);
		document.configForm.submit();
		parent.mainReady = null;	
	}  
        
	function donePort()
	{
		document.configForm.elements[2].value = "/normal/kvm/configKVMDevice.asp";
		document.configForm.elements[5].value = "0";
		document.configForm.elements[6].value = "";
		setQuerySSID(document.configForm);
		document.configForm.submit();
		parent.mainReady = null;
	}

	function submit()
	{
		document.configForm.elements[5].value = "0";
		document.configForm.elements[6].value = "";
		document.configForm.submit();
		parent.mainReady = null;
	}
    
    </script>
</head>
<body onload="init();" class="bodyMain">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td align="center">
	<table class="bgColor7" align="center" cellpadding="2" cellspacing="0" border="0">
	<tr><td>
    	<table class="bgColor1" cellpadding="15" cellspacing="0" class="securityBox" border="0">
		<tr><td>
Disable mouse acceleration in servers attached to KVM switch ports in order to keep the mouse cursor synchronized with the mouse pointer on your user workstation. For details, please refer to the User's Guide section "Disabling Mouse Acceleration".
		</td></tr>
		</table>
	</td></tr>
	</table>
</td></tr>
<tr><td align="center">
<table border="0" align="center" cellpadding="0" cellspacing="0" class="tableColor">
<form name="portsForm"  onSubmit="return false;">
 <tr align="left" valign="bottom"> 
    <td>
	<table border="0" cellpadding="0" cellspacing="0">
       <tr align="left" class="tableColor">         		
          <td width="215"><font class="tabsTextBig">&nbsp;Port Name</font></td>
		  <td width="204"><font class="tabsTextBig">Physical ID</font></td>
		  <td><font class="tabsTextBig">Disable</font></td>
        </tr>
    </table>
	</td>
 </tr>
 <tr align="left">
 	<!-- option = port name (1-15 characters)+3 white spaces (&nbsp;)+ Physical ID (1-15 characters)+3 white spaces (&nbsp;)+disable (1-3);	--> 
    <td align="left" valign="top">
	<select name="portsTable" size="10" class="formText" multiple>            
	<%get("system.kvm.conf.device.term.termTableHtml");%>
		<option value="-1">
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		</option>
	</select>
	</td>
 </tr></form>
</table>
<table border="0" align="center" cellpadding="0" cellspacing="0">
    <tr>
      <td align="center" valign="middle">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
	<td><a href="javascript:donePort();"><img src="/Images/doneButton.gif" alt="" width="47" height="21" border="0"></a></td>
	<td><a href="javascript:modifyPort();"><img src="/Images/modify.gif" alt="" width="60" height="21" border="0"></a></td>
	<td><a href="javascript:enablePort();"><img src="/Images/enable.gif" alt="" width="60" height="21" border="0"></a></td>
	<td><a href="javascript:disablePort();"><img src="/Images/disable.gif" alt="" width="60" height="21" border="0"></a></td>
</tr>
</table>	  
      </td>
   </tr>
</table>

</tr>
</table>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/kvm/configKVMDevice.asp">
   <input type="hidden" name="urlError" value="/normal/kvm/configKVMPorts.asp">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">
   <!-- *********************************** -->
   <input type="hidden" name="system.kvm.conf.device.term.action" value="">
   <input type="hidden" name="system.kvm.conf.device.term.selected" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>    
</body>
</html>
