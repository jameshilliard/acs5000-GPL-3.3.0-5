<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

<% ReqNum = getRequestNumber(); %>

	parent.currentPage = self;
	parent.helpPage = 'helpPages/auxportHelp.asp';
  	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	function init()
  	{
  		if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
  		{
    		if (checkLeftMenu('/normal/auxport/configAuxMenu.asp') == 0) {
               parent.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
    		parent.menu.leftHeadMenu("AUX");
    		setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
    		parent.topMenu.selectItem(parent.menu.topItem);
			parent.menu.selectItem("AUX");
  		} else {
    		setTimeout('init()', 200);
		}
  	}
	
	function copyData()
	{
		if (document.settings.elements[0].checked)
		        document.configForm.elements[13].value = 1;
		else
		        document.configForm.elements[13].value = 0;
		document.configForm.elements[5].value = 
			document.settings.elements[1].options[document.settings.elements[1].selectedIndex].value;
		document.configForm.elements[6].value = 
			document.settings.elements[2].options[document.settings.elements[2].selectedIndex].value;
		document.configForm.elements[7].value = 
			document.settings.elements[3].options[document.settings.elements[3].selectedIndex].value;
		document.configForm.elements[8].value = 
			document.settings.elements[4].options[document.settings.elements[4].selectedIndex].value;
		document.configForm.elements[9].value = 
			document.settings.elements[5].options[document.settings.elements[5].selectedIndex].value;
		document.configForm.elements[10].value = 
			document.settings.elements[6].options[document.settings.elements[6].selectedIndex].value;
		document.configForm.elements[11].value = document.settings.elements[7].value;
		document.configForm.elements[12].value = concatPPP();
	}
	  
	function setLayer()
	{
		if (document.settings.protocol.value == 0)
			hide('document','LayerPPP');
		else 
			show('document','LayerPPP');
	}

	function form_verification()
	{
		continue_verification = true;
		if (document.settings.protocol.value == 1)
		{
			checkMaxLength(document.settings.initChat, 299);
			if (continue_verification == true)
				checkElement (document.settings.localIP, false, false, true, false,0,0,false,false,true);
			if (continue_verification == true)
				checkElement (document.settings.remoteIP, false, false, true, false,0,0,false,false,true);
			if (continue_verification == true)	
				checkElement (document.settings.mtu, false, false, false, true, 1, 1500);
			if (continue_verification == true)
				checkMaxLength(document.settings.autoPPP, 299);
		}
	}
	
	function concatPPP ()
	{	
		// 0.0.0.0:0.0.0.0 auth mtu 1500 mru 1500 proxyarp modem asyncmap 000A0000 noipx noccp
		var pppStringOut;
		//concat Local IP
		if (document.settings.localIP.value == '')
			pppStringOut = '0.0.0.0';
		else pppStringOut = document.settings.localIP.value
		//concat Remote IP
		if (document.settings.remoteIP.value == '')
			pppStringOut += ':0.0.0.0';
		else pppStringOut = pppStringOut.concat(':',document.settings.remoteIP.value);
		//concat auth or noauth
		if (document.settings.authentication.checked == true)
			pppStringOut = pppStringOut.concat(' auth mtu ');
		else pppStringOut = pppStringOut.concat(' noauth mtu ')
		//concat MTU and MRU
		if (document.settings.mtu.value != '')
			pppStringOut = pppStringOut.concat(document.settings.mtu.value,' mru '+document.settings.mtu.value,' ');// 1500 default
		else pppStringOut = pppStringOut.concat('1500 mru 1500 ');
		//concat autoPPP
		pppStringOut = pppStringOut.concat(document.settings.autoPPP.value);
		return pppStringOut;
	}
	
	function slicePPP ()
	{
	var pppStringIn = document.settings.pppMain.value;
	//separate Local IP
	var local_ip = pppStringIn.slice(0,pppStringIn.indexOf(':'))
	if (local_ip != '0.0.0.0')
		{document.settings.localIP.value = local_ip;}
	else document.settings.localIP.value = '';	
	//separate Remote IP
	var remote_ip = pppStringIn.slice(pppStringIn.indexOf(':')+1,pppStringIn.indexOf(' '))
	if (remote_ip != '0.0.0.0')
		{document.settings.remoteIP.value = remote_ip;}
	else document.settings.remoteIP.value = '';	
	//separate authentication
	if (pppStringIn.indexOf('noauth') == -1)
		{document.settings.authentication.checked = true;}
	else 
		{document.settings.authentication.checked = false;}
	//separate MTU
	document.settings.mtu.value = pppStringIn.slice(pppStringIn.indexOf('mtu ')+4,pppStringIn.indexOf(' mru'));
	//separate authPPP
	var separator = ' mru '+document.settings.mtu.value;
	document.settings.autoPPP.value = pppStringIn.slice(pppStringIn.indexOf(separator)+separator.length+1,pppStringIn.length)
	}	


	function submit()
    {
		form_verification();
		if (continue_verification == true)
		{			
			copyData();
	         	document.configForm.submit();
		 	parent.mainReady = null;
		}	
	}
	</script>
</head>

<body class="bodyMiniWizard" onload="init(); setLayer(); slicePPP();">
<form name="settings">	
<table width="460" border="0" align="center" cellpadding="2" cellspacing="0">
  <tr align="left"> 
    <td align="center" height="60" valign="bottom">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
<% var enabled = getVal("system.kvm.conf.auxPort.enable"); %>
    <td align="right"><input type="checkbox" name="enablePort" <% if (enabled == 1) write("checked");%> value="" class="formText"></td>
    <td><font class="label">Enable Port</font></td>
</tr>
<tr>
	<td><font class="label">Profile</font></td>
	<td>
	<select name="protocol" class="formText" onchange="setLayer();">
		<%var protosel = getVal("system.kvm.conf.auxPort.protocol");%>
		<option value="0" <% if (protosel == "0") write("selected"); %>>Power Management</option>
		<option value="1" <% if (protosel == "1") write("selected"); %>>PPP</option>
    </select>	
	</td>
</tr>
</table>	
	</td>
  </tr>
</table>
<div id="LayerPPP" name="LayerPPP" style="position:absolute; top:75px; width:100%; z-index:1; visibility: hidden;" align="center">	
<table width="400" border="0" align="center" cellpadding="2" cellspacing="0">
<tr align="center" valign="top">
    <td height="45" align="center" colspan="4">
<table align="center" cellspacing="10" cellpadding="0" border="0">
<tr>
	<td><font class="label">Baud Rate (Kbps)</font></td>
	<td>
	<select name="baudRate" class="formText">
	    <% var selected = getVal("system.kvm.conf.auxPort.baudRate"); %>
              <option value="2400" <% if (2400 == selected) write("selected");%>> 2400 </option>
              <option value="4800" <% if (4800 == selected) write("selected");%>> 4800 </option>
              <option value="9600" <% if (9600 == selected) write("selected");%>> 9600 </option>
              <option value="14400" <% if (14400 == selected) write("selected");%>> 14400 </option>
              <option value="19200" <% if (19200 == selected) write("selected");%>> 19200 </option>
              <option value="28800" <% if (28800 == selected) write("selected");%>> 28800 </option>
              <option value="38400" <% if (38400 == selected) write("selected");%>> 38400 </option>
              <option value="57600" <% if (57600 == selected) write("selected");%>> 57600 </option>
              <option value="76800" <% if (76800 == selected) write("selected");%>> 76800 </option>
              <option value="115200" <% if (115200 == selected) write("selected");%>> 115200 </option>
              <option value="230400" <% if (230400 == selected) write("selected");%>> 230400 </option>
              <option value="460800" <% if (460800 == selected) write("selected");%>> 460800 </option>
              <option value="921600" <% if (921600 == selected) write("selected");%>> 921600 </option>
    </select>	
	</td>
</tr>
</table>	
	</td>
  </tr>
  <tr align="left" valign="top"> 
	<td height="45" align="left" width="100">
	<font class="label">Flow Control</font></td>
    <td height="45">
            <select name="flowControl" class="formText">
	    <% var selected = getVal("system.kvm.conf.auxPort.flowControl"); %>
              <option value="0" <% if (0 == selected) write("selected");%>>None</option>
              <option value="1" <% if (1 == selected) write("selected");%>>Hardware</option>
              <option value="2" <% if (2 == selected) write("selected");%>>Software</option>
            </select>	
	</td>
    <td height="45" align="left" width="100">
	<font class="label">Data Size</font></td>
    <td height="45">            
	<select name="dataSize" class="formText">
	    <% var selected = getVal("system.kvm.conf.auxPort.dataSize"); %>
              <option value="5" <% if (5 == selected) write("selected");%>> 5 </option>
              <option value="6" <% if (6 == selected) write("selected");%>> 6 </option>
              <option value="7" <% if (7 == selected) write("selected");%>> 7 </option>
              <option value="8" <% if (8 == selected) write("selected");%>> 8 </option>
    </select></td></tr>
  <tr align="left" valign="top"> 
    <td height="45" align="left">
	<font class="label">Parity</font></td>
    <td height="45">
            <select name="parity" class="formText">
	    <% var selected = getVal("system.kvm.conf.auxPort.parity");%>
              <option value="1" <% if (1 == selected) write("selected");%>>None </option>
              <option value="2" <% if (2 == selected) write("selected");%>>Odd </option>
              <option value="3" <% if (3 == selected) write("selected");%>>Even </option>
            </select>	
	</td>
	<td height="45" align="left">
	<font class="label">Stop Bits</font>
	</td>
    <td height="45">
            <select name="stopBits" class="formText">
	    <% var selected = getVal("system.kvm.conf.auxPort.stopBits");%>
              <option value="1" <% if (1 == selected) write("selected");%>>1</option>
              <option value="2" <% if (2 == selected) write("selected");%>>2</option>
            </select>	
	</td>
  </tr>
</table>
<table border="0" align="center" cellpadding="0" cellspacing="10">
  <tr>  
  <td align="left" width="110"><font class="label">Modem Initialization</font></td>
  <td align="left" colspan="3"><font class="formText">
	<textarea name="initChat" cols="30" rows="3" class="formText"><% get("system.kvm.conf.auxPort.initChat");%></textarea>
	</font></td>
  </tr>
  <tr>
  <td colspan="4">
<table align="center" cellspacing="8" cellpadding="0" border="0">
<tr>
	<td><font class="label">Local IP address</font></td>
	<td><input value="" type="text" name="localIP" id="localIP" size="15" maxlength="15" class="formText">
	<input type="hidden" name="pppMain" value="<% get("system.kvm.conf.auxPort.autoPPP");%>"></td>
</tr>
<tr>
	<td><font class="label">Remote IP address</font></td>
	<td><input value="" type="text" name="remoteIP" id="remoteIP" size="15" maxlength="15" class="formText"></td>
</tr>
<tr>
	<td align="right"><input type="checkbox" name="authentication" value="" class="formText"></td>
	<td><font class="label">Authentication Required</font></td>
</tr>
<tr>
	<td align="right"><font class="label">MTU/MRU</font></td>
	<td><input value="" type="text" name="mtu" id="mtu" size="4" maxlength="4" class="formText"></td>
</tr>
</table>  
  </td>
  </tr>  
  <tr>  
  <td align="left" width="100" height="40" valign="middle"><font class="label">PPP Options</font></td>
  <td align="left" colspan="3"><font class="formText">
	<textarea name="autoPPP" cols="30" rows="3" class="formText"></textarea>
	</font></td>
  </tr>
</table>
</div>
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
        <!--these hidden inputs are used to store values of form elements inside layers-->
	    <input type="hidden" name="system.req.action" value="0">
        <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
 		<input type="hidden" name="urlOk" value="/normal/auxport/configAuxPorts.asp">
		<input type="hidden" name="urlError" value="/normal/auxport/configAuxPorts.asp">
        <input type="hidden" name="request" value=<%write(ReqNum);%>>
	<!--***************************************************************************-->
 		<input type="hidden" name="system.kvm.conf.auxPort.protocol" value="">
 		<input type="hidden" name="system.kvm.conf.auxPort.baudRate" value="">
 		<input type="hidden" name="system.kvm.conf.auxPort.flowControl" value="">
 		<input type="hidden" name="system.kvm.conf.auxPort.dataSize" value="">
 		<input type="hidden" name="system.kvm.conf.auxPort.parity" value="">
 		<input type="hidden" name="system.kvm.conf.auxPort.stopBits" value="">
 		<input type="hidden" name="system.kvm.conf.auxPort.initChat" value="">
 		<input type="hidden" name="system.kvm.conf.auxPort.autoPPP" value="">
	        <input type="hidden" name="system.kvm.conf.auxPort.enable" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
