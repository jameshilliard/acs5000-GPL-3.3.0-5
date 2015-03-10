<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>New/Modify Connection</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		ReqNum = getRequestNumber();
		set ("_sel1_" , getQueryVal("vpnIndex",0));
	%>
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	
	var form_remote;
	var form_remote_rsa;
	var form_local;
	var form_local_rsa;
	var form_boot;

	function showSelectedLayer(object) {
		if (object.options[object.selectedIndex].value == 0) {
			show('document','rsaRight');
			show('document','rsaLeft');
			hide('document','secret');
		} else if (object.options[object.selectedIndex].value == 1) {
			hide('document','rsaRight');
			hide('document','rsaLeft');
			show('document','secret');
		}
	}

	function copyData() {
		moveData(document.configForm, form_remote);
		moveData(document.configForm, form_remote_rsa);
		moveData(document.configForm, form_local);
		moveData(document.configForm, form_local_rsa);
		moveData(document.configForm, form_boot);
	}
	
	function fill() {
		form_remote.authProtocol.value = <%get("system.network.VPNEntry.authProtocol");%>;
		form_remote.authMethod.value = <%get("system.network.VPNEntry.authMethod");%>;
		form_boot.bootAction.value = "<%get("system.network.VPNEntry.bootAction");%>";
	}
    
	function init( ) {
		if (type == 'NN') {
			form_remote     = document.Remote.document.RemoteForm;
			form_remote_rsa = document.rsaRight.document.keyRht;
			form_local      = document.local.document.localForm;
			form_local_rsa  = document.rsaLeft.document.keyLft;
			form_boot       = document.boot.document.bootForm;
		} else {
			form_remote     = document.RemoteForm;
			form_remote_rsa = document.keyRht;
			form_local      = document.localForm;
			form_local_rsa  = document.keyLft;
			form_boot       = document.bootForm;
		}
		fill();
		showSelectedLayer(form_remote.authMethod);
	}

	function form_verification() {
		continue_verification = true;
		if (continue_verification) {
			continue_verification = verify([
				[form_remote.connectionName, "Fill,Space"],
				[form_remote.idRht,          "Fill,VpnId"],
				[form_remote.ipAddressRht,   "Fill,IP"   ],
				[form_remote.nextHopRht,     "Fill?,IP"  ],
				[form_remote.subnetMaskRht,  "Fill,IPm"  ],
			]);
		}
		if (continue_verification && form_remote.authMethod.value == 0) {
			continue_verification = verify([
				[form_remote_rsa.rsaKeyRht, "Fill,Space,Length", [255]],
			]);
		}
		if (continue_verification) {
			continue_verification = verify([
				[form_local.idLft,          "Fill,VpnId"],
				[form_local.ipAddressLft,   "Fill,IP"   ],
				[form_local.nextHopLft,     "Fill?,IP"  ],
				[form_local.subnetMaskLft,  "Fill,IPm"  ],
			]);
		}
		if (continue_verification && form_remote.authMethod.value == 0) {
			continue_verification = verify([
				[form_local_rsa.rsaKeyLft, "Fill,Space,Length", [255]],
			]);
		}
		if (continue_verification) {
			continue_verification = verify([
				[form_boot.sharedSecret, "Fill?,Space"],
			]);
		}
		return continue_verification;
	}
	
	function checkElementID (elem) {
		if (elem.value.charAt(0) == '@') {
			for (var i=0; i<elem.value.length; i++) {
				if (elem.value.charAt(i)== ' ') {
					alert("Sorry, spaces are not allowed.");
					continue_verification = false;
					elem.focus();
					elem.select();
					break;
				}
			}
		} else if (!validateIP(elem.value, false, false)) {
			alert('The ID you entered is not valid.');
			continue_verification = false;
			elem.focus();
			elem.select();
		} else {
			continue_verification = true;
		}
	}
	
	function submit( ) {
		try {
			if (opener.window.name != document.configForm.target) {
				alert("The page which called this form is not available anymore. The changes will not be effective.");
				self.close();
				return;
			}
			if (form_verification()) {
				copyData();
				document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
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

<body onload="init();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netVPNConnect.asp">
	<input type="hidden" name="urlError" value="/normal/network/netVPNConnect.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<!--hidden inputs used to store values of form elements inside layers-->
	<input type="hidden" name="system.network.VPNEntry.connectionName" value="">
	<input type="hidden" name="system.network.VPNEntry.authProtocol" value="">
	<input type="hidden" name="system.network.VPNEntry.authMethod" value="">
	<input type="hidden" name="system.network.VPNEntry.idRht" value="">
	<input type="hidden" name="system.network.VPNEntry.ipAddressRht" value="">
	<input type="hidden" name="system.network.VPNEntry.nextHopRht" value="">
	<input type="hidden" name="system.network.VPNEntry.subnetMaskRht" value="">
	<input type="hidden" name="system.network.VPNEntry.rsaKeyRht" value="">
	<input type="hidden" name="system.network.VPNEntry.idLft" value="">
	<input type="hidden" name="system.network.VPNEntry.ipAddressLft" value="">
	<input type="hidden" name="system.network.VPNEntry.nextHopLft" value="">
	<input type="hidden" name="system.network.VPNEntry.subnetMaskLft" value="">
	<input type="hidden" name="system.network.VPNEntry.rsaKeyLft" value="">
	<input type="hidden" name="system.network.VPNEntry.bootAction" value="">
	<input type="hidden" name="system.network.VPNEntry.sharedSecret" value="">
	<input type="hidden" name="system.network.VPNEntry.VPNTableDel" value="">
	<!--*************************************************************************-->
<table border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>
		<td align="center" height="40" valign="middle">
			<a href="javascript:submit();">
				<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:window.close();">
				<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
		</td>
	</tr>
</table>
</form>
<div id="Remote" class="visible">
<form name="RemoteForm"  onSubmit="return false;">
<table border="0" cellpadding="5" cellspacing="0" width="600" align="center">
	<tr>
		<td colspan="4" align="center" height="50" valign="middle">
			<font class="label">Connection Name</font>
			&nbsp;&nbsp;&nbsp;&nbsp;
			<input type="text" maxlength="19" class="formText" name="connectionName" size="19" value="<%get("system.network.VPNEntry.connectionName");%>">
		</td>
	</tr>
	<tr>
		<td>
			<font class="label">Authentication <br>Protocol</font>
		</td>
		<td>
			<select name="authProtocol" class="formText">
				<option value="0" selected> ESP </option>
				<option value="1"> AH </option>
			</select>        
		</td>
		<td>
			<font class="label">Authentication <br>Method</font>
		</td>
		<td>
			<select name="authMethod" class="formText" onChange="showSelectedLayer(this)">
				<option value="0" selected> RSA Public Keys </option>
				<option value="1"> Shared Secret </option>
			</select>
		</td>
	</tr>
</table>
<table border="0" cellpadding="5" cellspacing="0" width="600" align="center">
	<tr>
		<td colspan="4" height="40" valign="bottom">         
			<font class="tabsTextBig">Remote ("Right")</font>
		</td>
	</tr>
	<tr>
		<td width="10%">
			<font class="label">ID</font>
		</td>
		<td width="40%">
			<input class="formText" maxlength="50" type="text" name="idRht" size="25" value="<%get("system.network.VPNEntry.idRht");%>">
		</td>
		<td width="10%">
			<font class="label">IP</font>
		</td>
		<td width="40%">
			<input class="formText" maxlength="50" type="text" name="ipAddressRht" size="25" value="<%get("system.network.VPNEntry.ipAddressRht");%>">
		</td>
	</tr>
	<tr>
		<td width="10%">
			<font class="label">NextHop</font>
		</td>
		<td width="40%">
			<input class="formText" maxlength="50" type="text" name="nextHopRht" size="25" value="<%get("system.network.VPNEntry.nextHopRht");%>">
		</td>
		<td width="10%">
			<font class="label">Subnet</font>
		</td>
		<td width="40%">
			<input class="formText"  maxlength="50" type="text" name="subnetMaskRht" size="25" value="<%get("system.network.VPNEntry.subnetMaskRht");%>">
		</td>
	</tr>
</table>
</form>
<div id="rsaRight" class="visible">
	<form name="keyRht"  onSubmit="return false;">
		<table border="0" cellpadding="5" cellspacing="0" width="600" align="center">
			<tr>
				<td valign="top">
					<font class="label">RSA Key</font>
				</td>
				<td>
					<textarea wrap=soft cols="55" rows="2" name="rsaKeyRht" class="formText"><%get("system.network.VPNEntry.rsaKeyRht");%></textarea>
				</td>
			</tr>
		</table>
	</form>
</div>
<div id="local">
<form name="localForm"  onSubmit="return false;">
<table border="0" cellpadding="5" cellspacing="0" width="600" align="center">
	<tr>
		<td colspan="4" height="40" valign="bottom">
			<font class="tabsTextBig">Local ("Left")</font>
		</td>
	</tr>
	<tr>
		<td width="10%">
			<font class="label">ID</font>
		</td>
		<td width="40%">
			<input class="formText" maxlength="50" type="text" name="idLft" size="25" value="<%get("system.network.VPNEntry.idLft");%>">
		</td>
		<td width="10%">
			<font class="label">IP</font>
		</td>
		<td width="40%">
			<input class="formText"  maxlength="50" type="text" name="ipAddressLft" size="25" value="<%get("system.network.VPNEntry.ipAddressLft");%>">
		</td>
	</tr>
	<tr>
		<td width="10%">
			<font class="label">NextHop</font>
		</td>
		<td width="40%">
			<input class="formText" maxlength="50" type="text" name="nextHopLft" size="25" value="<%get("system.network.VPNEntry.nextHopLft");%>">
		</td>
		<td width="10%">
			<font class="label">Subnet</font>
		</td>
		<td width="40%">
			<input class="formText" maxlength="50" type="text" name="subnetMaskLft" size="25" value="<%get("system.network.VPNEntry.subnetMaskLft");%>">
		</td>
	</tr>
</table>
</form>
<div id="rsaLeft" class="visible">
	<form name="keyLft"  onSubmit="return false;">
		<table border="0" cellpadding="5" cellspacing="0" width="600" align="center">
			<tr>
				<td valign="top">
					<font class="label">RSA Key</font>
				</td>
				<td>
					<textarea wrap="soft" cols="55" rows="2" name="rsaKeyLft" class="formText"><%get("system.network.VPNEntry.rsaKeyLft");%></textarea>
				</td>
			</tr>
	</table>
</form>
</div>
<div id="boot">
<form name="bootForm" action="" onSubmit="return false;">
<table border="0" cellpadding="2" cellspacing="0" width="600" align="center">
	<tr>
		<td align="center" height="40" valign="top">
			<font class="label">Boot Action</font>
			&nbsp;&nbsp;&nbsp;
			<select name="bootAction" class="formText">
				<option value="1" selected>Ignore</option>
				<option value="2">Add</option>
				<option value="3">Start</option>
			</select>
		</td>
	</tr>
</table>
<div id="secret">
<table border="0" cellpadding="2" cellspacing="0" width="600" align="center">
	<tr>
		<td align="center" height="40" valign="top">
			<font class="label">Pre Shared Secret</font>
			&nbsp;&nbsp;&nbsp;
			<input class="formText" maxlength="30" type="password" name="sharedSecret" size="30" value="<%get("system.network.VPNEntry.sharedSecret");%>">
		</td>
	</tr>
</table>
</div>
</form>
</div>
</body>
</html>
