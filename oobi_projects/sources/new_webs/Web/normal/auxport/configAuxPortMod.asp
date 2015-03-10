<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">


<% var port = getQueryVal("port", 0);
   var idx = port+1;
 %>

<html>
<head>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

	var port = <%write(port);%>
<% ReqNum = getRequestNumber(); %>

    if (top.family_model == "ONS")
      var pageMenu = '/normal/ports/portsMenu.asp';
    else
      var pageMenu = '/normal/auxport/configAuxMenu.asp';


    top.currentPage = self;
    top.helpPage = 'helpPages/auxportHelp.asp';
    parent.window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

    function init()
    {
        if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady)
            {
            if (checkLeftMenu(pageMenu) == 0) {
               top.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
            //top.menu.leftHeadMenu("AUX");
            setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
            top.topMenu.selectItem(top.menu.topItem);
            top.menu.selectItem("AUX");
        } else {
            setTimeout('init()', 200);
        }
        setLayer();
        slicePPP();
    }

    function copyData()
    {
		document.configForm.elements[5].value = port+1;
		if ((parent.frames[0].porttype == 'aux1') ||
			(parent.frames[0].porttype == 'aux2') ) {
		        document.configForm.elements[6].value =
		            document.auxportcfg.elements[1].options[document.auxportcfg.elements[1].selectedIndex].value;
			if (document.auxportcfg.elements[0].checked)
			        document.configForm.elements[14].value = 1;
			else
			        document.configForm.elements[14].value = 0;
		} else {
        		document.configForm.elements[6].value =
		            document.auxportcfg.elements[3].options[document.auxportcfg.elements[3].selectedIndex].value;
			if (document.auxportcfg.elements[2].checked)
				document.configForm.elements[14].value = 1;
			else
				document.configForm.elements[14].value = 0;
		}
        document.configForm.elements[7].value =
            document.auxportcfg.elements[4].options[document.auxportcfg.elements[4].selectedIndex].value;
        document.configForm.elements[8].value =
            document.auxportcfg.elements[5].options[document.auxportcfg.elements[5].selectedIndex].value;
        document.configForm.elements[9].value =
            document.auxportcfg.elements[6].options[document.auxportcfg.elements[6].selectedIndex].value;
        document.configForm.elements[10].value =
            document.auxportcfg.elements[7].options[document.auxportcfg.elements[7].selectedIndex].value;
        document.configForm.elements[11].value =
            document.auxportcfg.elements[8].options[document.auxportcfg.elements[8].selectedIndex].value;
        document.configForm.elements[12].value = document.auxportcfg.elements[9].value;
        document.configForm.elements[13].value = concatPPP();
    }

    function setLayer()
    {
	if (parent.frames[0].porttype == 'modem') {
		hide('document', 'LayerProfile');
		show('document', 'LayerProfileModem');
		show('document', 'LayerPPP1');
		if (document.auxportcfg.protocolModem.value == 1) 
			show('document','LayerPPP2');
		else
			hide('document','LayerPPP2');
	} 
	else {
		show('document', 'LayerProfile');
		hide('document', 'LayerProfileModem');
       		if (document.auxportcfg.protocol.value == 0) {
       			hide('document','LayerPPP1');
       			hide('document','LayerPPP2');
			if (parent.frames[0].porttype == 'aux2') 
    				if (top.family_model == "ONS") 
					hide('document','LayerMsg');
				else  
					show('document','LayerMsg');
			else
				hide('document','LayerMsg');
		}
       		else {
            		show('document','LayerPPP1');
			hide('document','LayerMsg');
			if (document.auxportcfg.protocol.value == 1) 
	            		show('document','LayerPPP2');
       			else
        	    		hide('document','LayerPPP2');
		}
	}
    }
    function form_verification()
    {
        continue_verification = true;
        if ((parent.frames[0].porttype == 'modem') ||
		(document.auxportcfg.protocol.value == 1))
        {
            checkMaxLength(document.auxportcfg.initChat, 299);
        if (continue_verification == true)
            checkElement (document.auxportcfg.localIP, false, false, true, false,0,0,false,false,true);
        if (continue_verification == true)
            checkElement (document.auxportcfg.remoteIP, false, false, true, false,0,0,false,false,true);
        if (continue_verification == true)
            checkElement (document.auxportcfg.mtu, false, false, false, true, 1, 1500);
        if (continue_verification == true)
            checkMaxLength(document.auxportcfg.autoPPP, 299);
        }
    }
    function concatPPP ()
    {
    // 0.0.0.0:0.0.0.0 auth mtu 1500 mru 1500 proxyarp modem asyncmap 000A0000 noipx noccp
    var pppStringOut;
    //concat Local IP
    if (document.auxportcfg.localIP.value == '')
        pppStringOut = '';
    else pppStringOut = document.auxportcfg.localIP.value
    //concat Remote IP
    if (document.auxportcfg.remoteIP.value == '')
        pppStringOut += ':';
    else pppStringOut = pppStringOut.concat(':',document.auxportcfg.remoteIP.value);
    //concat auth or noauth
    if (document.auxportcfg.authentication.checked == true)
        pppStringOut = pppStringOut.concat(' auth mtu ');
    else pppStringOut = pppStringOut.concat(' noauth mtu ')
    //concat MTU and MRU
    if (document.auxportcfg.mtu.value != '')
        pppStringOut = pppStringOut.concat(document.auxportcfg.mtu.value,' mru '+document.auxportcfg.mtu.value,' ');// 1500 default
    else pppStringOut = pppStringOut.concat('1500 mru 1500 ');
    //concat autoPPP
    pppStringOut = pppStringOut.concat(document.auxportcfg.autoPPP.value);
    return pppStringOut;
    }

    function slicePPP ()
    {
    var pppStringIn = document.auxportcfg.pppMain.value;
    //separate Local IP
    var local_ip = pppStringIn.slice(0,pppStringIn.indexOf(':'))
	document.auxportcfg.localIP.value = local_ip;
    //separate Remote IP
    var remote_ip = pppStringIn.slice(pppStringIn.indexOf(':')+1,pppStringIn.indexOf(' '))
     document.auxportcfg.remoteIP.value = remote_ip;
    //separate authentication
    if (pppStringIn.indexOf('noauth') == -1)
        {document.auxportcfg.authentication.checked = true;}
    else
        {document.auxportcfg.authentication.checked = false;}
    //separate MTU
    document.auxportcfg.mtu.value = pppStringIn.slice(pppStringIn.indexOf('mtu ')+4,pppStringIn.indexOf(' mru'));
    //separate authPPP
    var separator = ' mru '+document.auxportcfg.mtu.value;
    document.auxportcfg.autoPPP.value = pppStringIn.slice(pppStringIn.indexOf(separator)+separator.length+1,pppStringIn.length)
    }

    function submit()
    {
        form_verification();
        if (continue_verification == true)
        {
            copyData();
            document.configForm.submit();
            top.mainReady = null;
        }
    }
    </script>
</head>
<body class="bodyMiniWizard" onload="init();">
<form name="auxportcfg" onSubmit="return false;">
<div id="LayerProfile" name="LayerProfile" style="position:absolute; top:20px; width:100%; z-index:1; visibility: hidden;" align="center">
<table width="460" border="0" align="center" cellpadding="2" cellspacing="0">
  <tr align="left">
    <td align="center" height="60" valign="bottom">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
<% var enabled = getVal("system.kvm.conf.auxPort.enable", port); %>
    <td align="right"><input type="checkbox" name="enablePort" <% if (enabled == 1) write("checked");%> value="" class="formText"></td>
    <td><font class="label">Enable Port</font></td>
</tr>
<tr>
    <td><font class="label">Profile:</font></td>
    <td>
    <select name="protocol" class="formText" onchange="setLayer();">
        <%var protosel = getVal("system.kvm.conf.auxPort.protocol", port);%>
        <option value="0" <% if (protosel == "0") write("selected"); %>>Power Management</option>
        <option value="1" <% if (protosel == "1") write("selected"); %>>PPP</option>
        <option value="2" <% if (protosel == "2") write("selected"); %>>Login</option>
    </select>
    </td>
</tr>
</table>
    </td>
  </tr>
</table>
</div>
<div id="LayerProfileModem" name="LayerProfileModem" style="position:absolute; top:20px; width:100%; z-index:1; visibility: hidden;" align="center">
<table width="460" border="0" align="center" cellpadding="2" cellspacing="0">
  <tr align="left">
    <td align="center" height="60" valign="bottom">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
<% var enabled = getVal("system.kvm.conf.auxPort.enable", port); %>
    <td align="right"><input type="checkbox" name="enablePortModem" <% if (enabled == 1) write("checked");%> value="" class="formText"></td>
    <td><font class="label">Enable Port</font></td>
</tr>
<tr>
    <td><font class="label">Profile:</font></td>
    <td>
    <select name="protocolModem" class="formText" onchange="setLayer();">
        <%var protosel = getVal("system.kvm.conf.auxPort.protocol", port);%>
        <option value="1" <% if (protosel == "1") write("selected"); %>>PPP</option>
        <option value="2" <% if (protosel == "2") write("selected"); %>>Login</option>
    </select>
    </td>
</tr>
</table>
    </td>
  </tr>
</table>
</div>
<div id="LayerPPP1" name="LayerPPP1" style="position:absolute; top:95px; width:100%; z-index:1; visibility: hidden;" align="center">
<table width="400" border="0" align="center" cellpadding="2" cellspacing="0">
<tr align="center" valign="top">
    <td height="45" align="center" colspan="4">
<table align="center" cellspacing="10" cellpadding="0" border="0">
<tr>
    <td><font class="label">Baud Rate (Kbps):</font></td>
    <td>
    <select name="baudRate" class="formText">
        <% var selected = getVal("system.kvm.conf.auxPort.baudRate", port); %>
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
    <font class="label">Flow Control:</font></td>
    <td height="45">
            <select name="flowControl" class="formText">
        <% var selected = getVal("system.kvm.conf.auxPort.flowControl", port); %>
              <option value="0" <% if (0 == selected) write("selected");%>>None</option>
              <option value="1" <% if (1 == selected) write("selected");%>>Hardware</option>
              <option value="2" <% if (2 == selected) write("selected");%>>Software</option>
            </select>
    </td>
    <td height="45" align="left" width="100">
    <font class="label">Data Size:</font></td>
    <td height="45">
    <select name="dataSize" class="formText">
        <% var selected = getVal("system.kvm.conf.auxPort.dataSize", port); %>
              <option value="5" <% if (5 == selected) write("selected");%>> 5 </option>
              <option value="6" <% if (6 == selected) write("selected");%>> 6 </option>
              <option value="7" <% if (7 == selected) write("selected");%>> 7 </option>
              <option value="8" <% if (8 == selected) write("selected");%>> 8 </option>
    </select></td></tr>
  <tr align="left" valign="top">
    <td height="45" align="left">
    <font class="label">Parity:</font></td>
    <td height="45">
            <select name="parity" class="formText">
        <% var selected = getVal("system.kvm.conf.auxPort.parity", port);%>
              <option value="1" <% if (1 == selected) write("selected");%>>None </option>
              <option value="2" <% if (2 == selected) write("selected");%>>Odd </option>
              <option value="3" <% if (3 == selected) write("selected");%>>Even </option>
            </select>
    </td>
    <td height="45" align="left">
    <font class="label">Stop Bits:</font>
    </td>
    <td height="45">
            <select name="stopBits" class="formText">
        <% var selected = getVal("system.kvm.conf.auxPort.stopBits", port);%>
              <option value="1" <% if (1 == selected) write("selected");%>>1</option>
              <option value="2" <% if (2 == selected) write("selected");%>>2</option>
            </select>
    </td>
  </tr>
</table>
<table border="0" align="center" cellpadding="0" cellspacing="10">
  <tr>
  <td align="left" width="110"><font class="label">Modem Initialization:</font></td>
  <td align="left" colspan="3"><font class="formText">
    <textarea name="initChat" cols="30" rows="3" class="formText"><% get("system.kvm.conf.auxPort.initChat", port);%></textarea>
    </font></td>
  </tr>
</table>
</div>
<div id="LayerPPP2" name="LayerPPP2" style="position:absolute; top:300px; width:100%; z-index:1; visibility: hidden;" align="center">
<table border="0" align="center" cellpadding="0" cellspacing="10">
  <tr>
  <td colspan="4">
<table align="center" cellspacing="8" cellpadding="0" border="0">
<tr>
    <td><font class="label">Local IP address:</font></td>
    <td><input value="" type="text" name="localIP" id="localIP" size="15" maxlength="15" class="formText">
    <input type="hidden" name="pppMain" value="<% get("system.kvm.conf.auxPort.autoPPP", port);%>"></td>
</tr>
<tr>
    <td><font class="label">Remote IP address:</font></td>
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
  <td align="left" width="100" height="40" valign="middle"><font class="label">PPP Options:</font></td>
  <td align="left" colspan="3"><font class="formText">
    <textarea name="autoPPP" cols="30" rows="3" class="formText"></textarea>
    </font></td>
  </tr>
</table>
</div>
<div id="LayerMsg" name="LayerMsg" style="position:absolute; top:100px; width:100%; z-index:1; visibility: hidden;" align="center">
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="400" align="center">
    <tr>
        <td>
            <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
                <tr>
                    <td align="center" class="label_for_tables" >
             Any connection to KVM port can not access<br>
				or control the power management unit <br>
				connected to aux port 2
                    </td>
                </tr>
               </table>
        </td>
    </tr>
</table>  
</div>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
        <!--these hidden inputs are used to store values of form elements inside layers-->
        <input type="hidden" name="system.req.action" value="0">
        <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
        <input type="hidden" name="urlOk" value=<%if (idx != 3) write("/normal/auxport/configAuxPort"+idx+".asp"); else write("/normal/auxport/configModemPort.asp");%>>
        <input type="hidden" name="urlError" value=<%if (idx != 3) write("/normal/auxport/configAuxPort"+idx+".asp"); else write("/normal/auxport/configModemPort.asp");%>>
        <input type="hidden" name="request" value=<%write(ReqNum);%>>
    <!--***************************************************************************-->
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.pidx_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.protocol_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.baudRate_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.flowControl_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.dataSize_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.parity_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.stopBits_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.initChat_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.autoPPP_"+port);%> value="">
        <input type="hidden" name=<% write("system.kvm.conf.auxPort.enable_"+port);%> value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
