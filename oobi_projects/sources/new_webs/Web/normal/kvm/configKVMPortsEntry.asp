<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Modify Port</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
	  
<%
	var term = getQueryVal("term", -1); 
	termCs = ','+term+',';
	set("system.kvm.conf.device.term.selected",termCs);
%>
	var pmoutlet_list = <%get("system.kvm.conf.device.term.pmoutletHtml");%>;
	var deviceSel = "<%get("system.kvm.conf.device.deviceConf.name");%>";
	var aliasSel = "<%get("system.kvm.conf.device.term.termSel.alias");%>";
	
	function form_verification ()
	{
		continue_verification = true;
			
		if (checkPortDevice (document.portsEntryForm.alias)) {
			alert ('Invalid value for the alias.');
			continue_verification = false;
		}
		if (document.portsEntryForm.lockoutmacro.value !="" && checkSequence (document.portsEntryForm.lockoutmacro,3)){
			alert ('Invalid value for the lockoutmacro.');
			continue_verification = false;

		}
		if (continue_verification == true )
			checkPorts(document.portsEntryForm.outlet1);
	}

	function checkPorts (element_for_verification)
	{
		if (element_for_verification.value == "") {
			return true;
		}
		
		var cfg = element_for_verification.value;
		var devs = cfg.split(',');
		var outlet_list = "";
		
		for (ix = 0; ix < devs.length; ix++) {
			var direct;
			var range;
				
			if (devs[ix].indexOf(".") == -1) {
				direct = 1;
				range = devs[ix];
			}
			else {
				direct = 0;
				if (deviceSel == "master") {
					continue_verification = false;
					alert('Power outlets for this port can only be from IPDUs connected to "master" device');
					element_for_verification.focus();
					element_for_verification.select();
					return false;
				}

				var i = devs[ix].lastIndexOf(".");
				var device_name = devs[ix].substring(0, i);
					
				if (device_name != deviceSel) {
					continue_verification = false;
                              		alert('Power outlets for this port should be from IPDUs connected to "master" or "' + deviceSel + '" devices.');
                               		element_for_verification.focus();
                               		element_for_verification.select();
                               		return false;
				}
				range = devs[ix].substring(i+1, devs[ix].length);			
			}

			var outlet = range.split('-');

			for (k = 0; k < outlet.length; k++) {
				if (outlet[k] == "" || isNaN(outlet[k])) {
					continue_verification = false;
					alert ('Invalid outlet number: ' + outlet[k]); 
					element_for_verification.focus();
					element_for_verification.select();
					return false;
				}
				if (outlet[k] < 1 || outlet[k] > 128) {
					continue_verification = false;
					alert ('Out of range outlet number: ' + outlet[k]); 
					element_for_verification.focus();
					element_for_verification.select();
					return false;
				}

//				var maxOutlet = <%get("system.kvm.conf.device.maxOutlet");%>;
//				if (direct && outlet[k] > maxOutlet) {
//					continue_verification = false;
//					alert ('The outlet does not exist: ' + outlet[k]); 
//					element_for_verification.focus();
//					element_for_verification.select();
//					return false;
//				}		
			}
		
			if (outlet.length == 2) {
				if (Number(outlet[1]) < Number(outlet[0])) {
					temp = outlet[1];
					outlet[1] = outlet[0];
					outlet[0] = temp; 
				}
			}
			else {
				outlet[1] = outlet[0];
			}

			for (num = Number(outlet[0]); num <= Number(outlet[1]); num++) {
				var power_outlet;

				if (direct == 1)
					power_outlet = num;
				else
					power_outlet = device_name + "." + num;
				
				checkDuplicate(power_outlet, outlet_list);
				if (continue_verification == false) 
					return false;

				outlet_list += power_outlet;
				if (num < outlet[1])
					outlet_list += ",";
			}
			if (ix < devs.length - 1)
				outlet_list += ",";
		}
		document.portsEntryForm.outlet1.value = outlet_list;
	}
	
	function checkDuplicate(outlet_to_check, input_list)
	{
		var list = input_list.split(',');
		for (j = 0; j < list.length; j++) {
			if (outlet_to_check == list[j]) {
				continue_verification = false;
				alert('Duplicate outlet in input: ' + outlet_to_check);
				return false;
			}
		}
				

		for (var i = 0; pmoutlet_list[i][0] != -1; i++) {
			if ( (pmoutlet_list[i][1] != deviceSel) || (pmoutlet_list[i][2] != aliasSel) ) {
				list = pmoutlet_list[i][3].split(',');
				for (var j = 0; j < list.length; j++) {
					if (outlet_to_check == list[j]) {
						continue_verification = false;
						alert('Duplication in outlet number: ' + outlet_to_check);
						return false;
					}
				}
			}
		}
	}
	function cancelOK()
	{
		setQuerySSID(document.configForm);
		self.close();
	}
	
	function modifyOK()
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
			document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
			document.configForm.elements[5].value = "3";
			document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
			document.configForm.elements[7].value = document.portsEntryForm.elements[0].value;
			document.configForm.elements[8].value = document.portsEntryForm.elements[1].value;
			document.configForm.elements[9].value = document.portsEntryForm.elements[2].value;
//			alert('outlet list: ' + document.configForm.elements[9].value);

            		setQuerySSID(document.configForm);
			document.configForm.submit();
 			self.close();
		}	
	} catch (error) {
	  self.close();
	}
	}          

</script>

</head>
<body class="bodyMain">
<form name="portsEntryForm"  onSubmit="return false;">
	<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
		<tr>
		<td align="center" valign="middle">
		<table border="0" cellpadding="5" cellspacing="0">
			<tr> 
			<td colspan="2" align="center"  height="40"  valign="top">
			<table align="center" cellspacing="15" cellpadding="0" border="0">
				<tr>
				<td><a href="javascript:modifyOK();"><img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a></td>
				<td><a href="javascript:cancelOK();"><img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a></td>
				</tr>
			</table>			     
			</td>
			</tr>
		</table>
		<table border="0" cellpadding="5" cellspacing="0">
			<tr> 
				<td colspan="2" align="center"><font class="label">Alias</font></td>

				<td colspan="2" align="center">
				<input name="alias" type="text" size="15" maxlength="19" value="<%get("system.kvm.conf.device.term.termSel.alias");%>"
				class="formText" value="<%%>">
				</td>

			</tr>
			<tr> 
				<td colspan="2" align="center"><font class="label">Lockout Macro</font></td>

				<td colspan="2" align="center">
				<input name="lockoutmacro" type="text" size="15" maxlength="27" value="<%get("system.kvm.conf.device.term.termSel.lockoutmacro");%>"
				class="formText" value="<%%>">
				</td>

			</tr>

			<tr> 
				<td colspan="4" align="center" height="40" valign="middle"><font class="tabsTextBig">Power Outlets</font></td>
			</tr>
			<tr>
			<script type="text/javascript">
			var family_model = '<%get("_familymodel_");%>';
			var pmstring="<%get("system.kvm.conf.device.term.termSel.pmoutletstr");%>";
			if (family_model == "ONS") {
				document.write('<input name="outlet1" type="hidden" size="24" maxlength="128" value="'+pmstring+'" class="formText">');
				document.write('<td colspan="4" align="center"><font class="label">Power Management is not available through KVM sessions.</font></td>');
			} else {
				document.write('<td colspan="2" align="center"><font class="label">Device.Outlet</font></td>');
				document.write('<td>');
				document.write('<input name="outlet1" type="text" size="24" maxlength="128" value="'+pmstring+'" class="formText">');
				document.write('</td>');
			}
			</script>
			</tr>
		</table>

		</td>
		</tr>
	</table>		
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/kvm/configKVMPorts.asp">
	<input type="hidden" name="urlError" value="/normal/kvm/configKVMDevice.asp">
	<input type="hidden" name="request" value="<%write(getRequestNumber());%>">
	<input type="hidden" name="system.kvm.conf.device.term.action" value="">
	<input type="hidden" name="system.kvm.conf.device.term.selected" value="">
	<INput type="hidden" name="system.kvm.conf.device.term.termSel.alias" value="">
	<input type="hidden" name="system.kvm.conf.device.term.termSel.lockoutmacro" value="">
	<input type="hidden" name="system.kvm.conf.device.term.termSel.pmoutletstr" value="">
	</form>
</body>
</html>
