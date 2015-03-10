<%
      // Set page-wide variables:
      var device = getQueryVal("device", "-1");
      set("_sel1_", device);
%>
<html>
<head>
	<title>Set KVM Permissions for <%
if (getVal("system.kvm.conf.acclist.currusergroup")) {
	write("group");
} else {
	write("user");
}%> <%get("system.kvm.conf.acclist.currname");%>, device <%
get("system.kvm.conf.acclist.device.currdevice");%></title>

<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	var ReqNum = opener.document.configForm.elements[4].value;
	
	function form_verification()
	{	   	  
		continue_verification = true;
		checkPorts (document.kvmSetPermissions.noAccess);
		if (continue_verification == true)
			checkPorts (document.kvmSetPermissions.readOnlyAccess);
		if (continue_verification == true)
			checkPorts (document.kvmSetPermissions.readWriteAccess);
		if (continue_verification == true)
			checkPorts (document.kvmSetPermissions.fullAccess);
		if (continue_verification == true)
			comparePorts();					
	}	

	var ports;
	
	function comparePorts ()
	{
		ports = convert(document.kvmSetPermissions.noAccess);
		var ports2 = convert(document.kvmSetPermissions.readOnlyAccess);
		var ports3 = convert(document.kvmSetPermissions.readWriteAccess);
		var ports4 = convert(document.kvmSetPermissions.fullAccess);

		if (ports != "" && ports2 != "") {
			if (compare(ports, ports2,
				document.kvmSetPermissions.readOnlyAccess)=='error') {
				continue_verification = false
			}
			ports = ports.concat(ports2); 
		} else if (ports2 != "") ports = ports2;
	  
		if (ports != "" && ports3 != "") {
			if (continue_verification == true &&
					compare(ports, ports3,
					document.kvmSetPermissions.readWriteAccess)=='error') {
				continue_verification = false
			}	
			ports = ports.concat(ports3);
		} else if (ports3 != "") ports = ports3;
		
		if (ports != "" && ports4 != "") {
			if (continue_verification == true &&
					compare(ports, ports4,
					document.kvmSetPermissions.fullAccess)=='error') {
				continue_verification = false
			}		
			ports = ports.concat(ports4);
		} else if (ports4 != "") ports = ports4;
	}
	
	function compare(allPorts, newArrayPorts, element_for_verification)
	{
		for (i = 0; i < newArrayPorts.length; i++) {
			if (allPorts.join().indexOf(newArrayPorts[i]) != -1) {
				for (a = 0; a < allPorts.length; a++) {
					if (newArrayPorts[i].indexOf(allPorts[a])!= -1) {
						alert ('You have already used port number '+
							newArrayPorts[i]); 
						element_for_verification.focus(); 
						element_for_verification.select();
						return 'error'; break;
					}
				}
			}
		}
	}
	
	function checkPorts (element_for_verification)
	{
		if (element_for_verification.value != "") {
			var out = element_for_verification.value;
			var chars = '1234567890,-';
		
			for (i = 0; i < out.length; i++) {
      				if (chars.indexOf(out.charAt(i)) == -1) {
					continue_verification = false;
					alert ('The port numbers  you entered are not valid.'); 
					element_for_verification.focus();
					element_for_verification.select();
					return false;
					break;
				}
				
				if (out.charAt(i) == ',' &&
						(i == 0 || i == out.length-1 ||
						out.charAt(i-1) == '-' || out.charAt(i+1) == '-')) {
					continue_verification = false;
					alert ('The port numbers  you entered are not valid.'); 
					element_for_verification.focus();
					element_for_verification.select();
					return false;
					break;
				}
				
				if (out.charAt(i) == '-' &&
						(i == 0 || i == out.length-1 ||
						out.charAt(i-1) == ',' || out.charAt(i+1) == ',')) {
					continue_verification = false;
					alert ('The port numbers  you entered are not valid.'); 
					element_for_verification.focus();
					element_for_verification.select();
					return false;
					break;
				}					
   			}
					
			var listNumbers = convert(element_for_verification);
			maxPortNumber = <%get("system.kvm.conf.acclist.device.numports");%>;
			for (i = 0; i < listNumbers.length; i++) {
				if (listNumbers[i] > maxPortNumber || listNumbers[i] < 1) {
					continue_verification = false;
					maxPortNumber++;
					alert ('Value must be more than 0 and less than '+
						maxPortNumber); 
					element_for_verification.focus();
					element_for_verification.select();
					return false;
					break;
				}					
			}
		}				
	}
	
	function convert(element)
	{
		var listNumbers = element.value.split(',');
		for (i = 0; i < listNumbers.length; i++) {
			if (listNumbers[i].indexOf('-') != -1)
			listNumbers[i] = convertToList(listNumbers[i]);
		}	
		return listNumbers.join().split(',');	
	}
	
	function convertToList(numbers)
	{
		var number1 = numbers.split('-')[0];
		var number2 = numbers.split('-')[1];

		if (number1 < number2) {
			var numbersString = number1;

			for (i=number1; i <= number2; i++) {
				if (i != number1) {
					numbersString += ',';
					numbersString += i;
				}
			}
		} else {
			numbersString = number2;
			for (i = number2; i <= number1; i++) {
				if (i != number2) {
					numbersString += ',';
					numbersString += i;
				}
			}
		}	
		return numbersString;
	}
	
	function okPermissionEntry()
	{
		form_verification();
		if (continue_verification == true) {
		    if (ReqNum != opener.document.configForm.elements[4].value) {
			alert("The page which called this form is not available anymore. The changes will not be effective.");
		    } else {
			document.configForm.elements[5].value =
				document.kvmSetPermissions.noAccess.value;
			document.configForm.elements[6].value =
				document.kvmSetPermissions.readOnlyAccess.value;
			document.configForm.elements[7].value =
				document.kvmSetPermissions.readWriteAccess.value;
			document.configForm.elements[8].value =
				document.kvmSetPermissions.fullAccess.value;
			setQuerySSID(document.configForm);
			document.configForm.submit();
		    }
		    self.close();
		}
 	}

	</script>
</head>

<body onload="passSessionID();" class="bodyMain">

<form name="kvmSetPermissions" method="POST" action=" ">
  <table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
	    <tr>    
			<td align="center" colspan="2" height="30" valign="top">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
	<td><a href="javascript:okPermissionEntry();"><img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a></td>
	<td><a href="javascript:window.close();"><img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a></td>
</tr>
</table>			       
			</td>
		</tr>
  </table>
  <table border="0" align="center" cellpadding="2" cellspacing="0">      
  <tr align="left" valign="middle"> 
    <td nowrap align="center" colspan="2">
	<font class="tabsTextBig">Device <%get("system.kvm.conf.acclist.device.currdevice");%>, <%get("system.kvm.conf.acclist.device.numports");%> ports.</font></td>
  </tr>  
  <tr align="left" valign="middle"> 
    <td nowrap align="right">
	<font class="label">Ports with No Permission</font></td>
    <td nowrap align="center" height="40" valign="middle">
	<input name="noAccess" type="text" value="<%get("system.kvm.conf.acclist.device.portsnone"); %>" size="15" class="formText"></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td nowrap align="right">
	<font class="label">Ports with Read Only Permission</font></td>
    <td nowrap align="center" height="40" valign="middle">
	<input name="readOnlyAccess" type="text" value="<%get("system.kvm.conf.acclist.device.portsro"); %>" size="15" class="formText"></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td nowrap align="right">
	<font class="label">Ports with Read/Write Permission</font></td>
    <td nowrap align="center" height="40" valign="middle">
	<input name="readWriteAccess" type="text" value="<%get("system.kvm.conf.acclist.device.portsrw"); %>" size="15" class="formText"></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td nowrap align="right">
	<font class="label">Ports with Full Permission</font></td>
    <td nowrap align="center" height="40" valign="middle">
	<input name="fullAccess" type="text" value="<%get("system.kvm.conf.acclist.device.portsfull"); %>" size="15" class="formText"></td>    
  </tr>       
</table>
</form>
	<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
   		<input type="hidden" name="system.req.action" value="">
   		<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   		<input type="hidden" name="urlOk" value="/normal/kvm/configKVMSetPermissions.asp">
   		<input type="hidden" name="urlError" value="/normal/kvm/configKVMSetPermissions.asp">
   		<input type="hidden" name="request" value=<%write(getRequestNumber());%>>
   		<input type="hidden" name="system.kvm.conf.acclist.device.portsnone" value="">
   		<input type="hidden" name="system.kvm.conf.acclist.device.portsro" value="">
   		<input type="hidden" name="system.kvm.conf.acclist.device.portsrw" value="">
   		<input type="hidden" name="system.kvm.conf.acclist.device.portsfull" value="">
	</form>
</body>
</html>
