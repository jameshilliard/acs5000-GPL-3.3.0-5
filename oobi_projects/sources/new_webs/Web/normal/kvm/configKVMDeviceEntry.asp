<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Modify Device</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
<%
	var deviceSel = getQueryVal("deviceSel", -1);
	if (deviceSel == "-1") {
		action = "Add";
        	set("system.kvm.conf.device.deviceSel",0);
	} else {
		action = "Edit";
        	set("system.kvm.conf.device.deviceSel",deviceSel);
	}
%>
	var devices = "";
	var action = "<% write(action);%>";
	var ReqNum = opener.document.configForm.elements[4].value;

	function form_verification()
	{	   	  
		continue_verification = true;
	  	if (checkPortDevice (document.deviceEntryForm.deviceName)) {
			alert ('Invalid value for the device name.');
			continue_verification = false;
		}
		if (continue_verification == true &&
			document.deviceEntryForm.numPorts.value == 0) {
			alert('A number of ports must be selected.');
			continue_verification = false;
		}
	//if (continue_verification == true && document.deviceEntryForm.connectType[1].checked == true)
		//{checkElement (document.deviceEntryForm.hostName, true, false, false, false);}
	//if (continue_verification == true && document.deviceEntryForm.connectType[1].checked == true)
		//{checkHostOrIP (document.deviceEntryForm.hostName);}	  	
	  }	  
	function checkPort(user)
	{
	if (user == 1)
		{
		if (document.deviceEntryForm.port_user1.selectedIndex-1 == document.deviceEntryForm.port_user2.selectedIndex)
			{
			alert ('You can not use the same port for both users.');
			document.deviceEntryForm.port_user1.selectedIndex = -1
			}
		}
	else if (user == 2)
		{
		if (document.deviceEntryForm.port_user2.selectedIndex == document.deviceEntryForm.port_user1.selectedIndex-1)
			{
			alert ('You can not use the same port for both users.');
			document.deviceEntryForm.port_user2.selectedIndex = -1
			}
		}
	}	  	 
	function init()
	{
		var selectedDevice = "<% getVal("system.kvm.conf.device.deviceConf.device"); %>"; 
		
		devices="";
		devices +='<option value="master" selected >master</option>';

//		for (var i=0; i < opener.document.KVMDeviceForm.deviceTable.options.length -1; i++)
//		{
//			var dev = opener.document.KVMDeviceForm.deviceTable.options[i].text;
//			devName = dev.split(/\s/);
//			devices +='<option value="'+devName[0]+'">'+devName[0]+'</option>';
//			if (devName[0] == selectedDevice) {
//				document.deviceEntryForm.devices.options[i].selected = true;
//			}
//		}
	}

	/*function showLayer()
	{
	if (document.deviceEntryForm.connectType[1].checked == true)	
		{
		show('document', 'network');
		hide('document', 'physical');
		}
	else 
		{
		hide('document', 'network');
		show('document', 'physical');	
		}
	}*/	
	
	function modifyOK()
    {                                    
	try {
	     if (opener.window.name != document.configForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
		form_verification();
		if (continue_verification == true) {	
			document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
			document.configForm.elements[5].value = opener.document.configForm.elements[5].value;
			document.configForm.elements[6].value = opener.document.configForm.elements[6].value;

			document.configForm.elements[7].value = document.deviceEntryForm.deviceName.value;
			document.configForm.elements[8].value = document.deviceEntryForm.numPorts.value;
			document.configForm.elements[9].value = 0
			//if (document.deviceEntryForm.connectType[0].checked == true)
			//	{document.configForm.elements[9].value = 0;}//physical
			//else 
			//	{document.configForm.elements[9].value = 1;}//network
			//document.configForm.elements[10].value = document.deviceEntryForm.hostName.value;
			document.configForm.elements[11].value = document.deviceEntryForm.devices.value;
			document.configForm.elements[12].value = document.deviceEntryForm.port_user2.value;
			document.configForm.elements[13].value = document.deviceEntryForm.port_user1.value;
            setQuerySSID(document.configForm);
			document.configForm.submit(); 
 		    self.close();
		}	
	} catch (error) {
	  self.close();
	}
    }          
	
	function mount_portlist2 (port1)
	{
		port2 = document.deviceEntryForm.port_user2.options[document.deviceEntryForm.port_user2.selectedIndex].value;
		document.deviceEntryForm.port_user2.selectedIndex = 0;
		k = 0;
// if no port selected (none) user2 list can have all avail ports.
		if (port1 == "" )
			document.deviceEntryForm.port_user2.length++;
		for (i = 1; i <= opener.ports; i ++) {
			if (i == port2) {
			} else if (i == port1) {
				continue;
			} else if (opener.portlist[i - 1] != 0) {
				continue;
			}
			document.deviceEntryForm.port_user2.options[k].value = i;
			document.deviceEntryForm.port_user2.options[k].text = i;
			if (i == port2) {
				document.deviceEntryForm.port_user2.selectedIndex = k;
			}
			k ++;
		}
		document.deviceEntryForm.port_user2.length = k;
	}

	function mount_portlist1 (port2)
	{
		port1 = document.deviceEntryForm.port_user1.options[document.deviceEntryForm.port_user1.selectedIndex].value;
		document.deviceEntryForm.port_user1.options[0].value = '';
		document.deviceEntryForm.port_user1.options[0].text = 'none';
		document.deviceEntryForm.port_user1.selectedIndex = 0;
		k = 1;
		for (i = 1; i <= opener.ports; i ++) {
			if (i == port1) {
			} else if (i == port2) {
				continue;
			} else if (opener.portlist[i - 1] != 0) {
				continue;
			}
			document.deviceEntryForm.port_user1.options[k].value = i;
			document.deviceEntryForm.port_user1.options[k].text = i;
			if (i == port1) {
				document.deviceEntryForm.port_user1.selectedIndex = k;
			}
			k ++;
		}
		document.deviceEntryForm.port_user1.length = k;
	}
     </script>
</head>
<body class="bodyForLayers">
<form name="deviceEntryForm" onSubmit="return false;">
   	<table border="0" cellpadding="5" cellspacing="0" align="center">
	   	<tr> 
          	<td colspan="2" align="center"  valign="top">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
	<td><a href="javascript:modifyOK();"><img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a></td>
	<td><a href="javascript:window.close();"><img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a></td>
</tr>
</table>			
          	</td>
        </tr>
	</table>
	<table border="0" cellpadding="5" cellspacing="0" align="center">
	   	<tr> 
          	<td width="120"><font class="label">Device Name</font></td>
		<td>
		<script type="text/javascript">
		var devconf_name = "<%get("system.kvm.conf.device.deviceConf.name");%>";
		if (opener.document.configForm.elements[6].value == '3') {
          		document.write('<input name="deviceName" type="hidden" value="' + devconf_name + '">');
			document.write('<font class="label">' + devconf_name + '</font>');
		} else {
          		document.write('<input name="deviceName" type="text" size="16" maxlength="16" class="formText" value="' + devconf_name + '">');
		}
		</script>
		</td>
		</tr>
		<tr>	
		<td><font class="label">Number of Ports</font></td>
		<td><select name="numPorts" class="formText">
			<script>
				var nports = (action == "Add")? 0 : <%get("system.kvm.conf.device.deviceConf.numTerms");%>;
				var ports = [[0, "  "], [8, "8"], [16, "16"], [32, "32"]];
				var port;
				for (port in ports) {
					document.write('<option value="'+ports[port][0]+'"');
					if (ports[port][0] == nports) {
						document.write(' selected');
					}
					document.write('>'+ports[port][1]+'</option>');
				}
			</script>
		</select>
		</td>
        </tr>
		<!--<tr>	
		<td colspan="2" align="center"><font class="label">Connection Type:</font></td>
		</tr>
		<tr>
          	<td colspan="2" align="center">
<table align="center" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td><input type="radio" name="connectType" value="0" onclick="showLayer();"></td>
	<td><font class="label">Physical</font></td>
	<td><input type="radio" name="connectType" value="1" onclick="showLayer();"></td>
	<td><font class="label">Network</font></td>
</tr>
</table>			
		</td>			
        </tr>-->	
	</table>
<!--<div id="network" style="position:absolute; left:0px; top:210px; width:100%; z-index:1; visibility: hidden;" align="center">		
	<table border="0" cellpadding="5" cellspacing="0" align="center">		
	  	<tr> 
          	<td><font class="label">Host Name</font></td>
          	<td><input type="text" name="hostName" size="15" maxlength="255" class="formText" value=""></td>			
        </tr>	
	</table>
</div>-->
<!--<div id="physical" style="position:absolute; left:0px; top:210px; width:100%; z-index:1; visibility: hidden;" align="center">-->
	<table border="0" cellpadding="5" cellspacing="0" align="center">		
	  	<tr> 
          	<td height="35" valign="middle"><input type="hidden" name="devices" value="master">	
			<font class="label">KVM Port Connected to <%getLabel("system.kvm.user2");%></font></td>
          	<td height="35" valign="middle">
		<select name="port_user2" class="formText" onchange="mount_portlist1(document.deviceEntryForm.port_user2.options[document.deviceEntryForm.port_user2.selectedIndex].value);">
<script type="text/javascript">
	for (var i = 1; i <= opener.ports; i ++) {
		document.write('<option value="'+i+'">'+i+'</option>');
	}
</script>									
</select>
	</td>			
        </tr>
	  	<tr> 
          	<td height="35" valign="middle"><font class="label">KVM Port Connected to <%getLabel("system.kvm.user1");%></font></td>
          	<td height="35" valign="middle">
			<select name="port_user1" class="formText" onchange="mount_portlist2(document.deviceEntryForm.port_user1.options[document.deviceEntryForm.port_user1.selectedIndex].value);">
<script type="text/javascript">
	var selectedPort1 = <% get("system.kvm.conf.device.deviceConf.connectTerms",1);%> - 0;
	var selectedPort2 = <% get("system.kvm.conf.device.deviceConf.connectTerms",0);%> - 0;
	for (var i=0; i <= opener.ports; i++)
	{
		if (i==0) 
			{document.write('<option value="">none</option>');}
		else
			{document.write('<option value="'+i+'">'+i+'</option>');}
	}
	document.deviceEntryForm.port_user1.selectedIndex = selectedPort1;	
</script>									
	</select>			
	</td>
        </tr>		
	</table>
<!--</div>-->
</form>

<script type="text/javascript">		

	k = 0;
	document.deviceEntryForm.port_user2.selectedIndex = 0;
	for (i = 1; i <= opener.ports; i ++) {
		if (i == selectedPort2) {
		} else if (i == selectedPort1) {
			continue;
		} else if (opener.portlist[i - 1] != 0) {
			continue;
		}
		document.deviceEntryForm.port_user2.options[k].value = i;
		document.deviceEntryForm.port_user2.options[k].text = i;
		if (i == selectedPort2) {
			document.deviceEntryForm.port_user2.selectedIndex = k;
		}
		k ++;
	}
	document.deviceEntryForm.port_user2.length = k;
	selectedPort2 = document.deviceEntryForm.port_user2.options[document.deviceEntryForm.port_user2.selectedIndex].value; 

	document.deviceEntryForm.port_user1.options[0].value = '';
	document.deviceEntryForm.port_user1.options[0].text = 'none';
	document.deviceEntryForm.port_user1.selectedIndex = 0;
	k = 1;
	for (i = 1; i <= opener.ports; i ++) {
		if (i == selectedPort1) {
		} else if (i == selectedPort2) {
			continue;
		} else if (opener.portlist[i - 1] != 0) {
			continue;
		}
		document.deviceEntryForm.port_user1.options[k].value = i;
		document.deviceEntryForm.port_user1.options[k].text = i;
		if (i == selectedPort1) {
			document.deviceEntryForm.port_user1.selectedIndex = k;
		}
		k ++;
	}
	document.deviceEntryForm.port_user1.length = k;
</script>									

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
  <input type="hidden" name="system.req.action" value="">
  <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
  <input type="hidden" name="urlOk" value="/normal/kvm/configKVMDevice.asp">
  <input type="hidden" name="urlError" value="/normal/kvm/configKVMDevice.asp">
  <input type="hidden" name="request" value="<%write(getRequestNumber());%>"> 
   <input type="hidden" name="system.kvm.conf.device.deviceCs" value="">
   <input type="hidden" name="system.kvm.conf.device.action" value="0">
   <input type="hidden" name="system.kvm.conf.device.deviceConf.name" value="">
   <input type="hidden" name="system.kvm.conf.device.deviceConf.numTerms" value="">
   <input type="hidden" name="system.kvm.conf.device.deviceConf.connectionType" value="">
   <input type="hidden" name="system.kvm.conf.device.deviceConf.hostName" value="">
   <input type="hidden" name="system.kvm.conf.device.deviceConf.device" value="">
   <input type="hidden" name="system.kvm.conf.device.deviceConf.connectTerms_0" value="">
   <input type="hidden" name="system.kvm.conf.device.deviceConf.connectTerms_1" value="">
</form>
</body>
</html>
