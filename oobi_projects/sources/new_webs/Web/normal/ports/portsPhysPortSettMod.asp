<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

	<%
		var numPorts = getVal("system.ports.physP.numPortsel");
		'[LMT] Cybts BUG 4476'
		 var numPMs = getVal("system.ports.physAction.numPowerMgm") + 
                      getVal("system.applications.pmIpmi.numActiveIpmiDev"); 
	    ReqNum = getRequestNumber();
    %>

	var numPorts = "<% write(numPorts);%>";
	var numPMs;
	if (top.family_model != "AVCS")
		numPMs = "<% write(numPMs);%>";
	else
		numPMs = 0;
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	top.currentPage = self;
	top.helpPage = 'helpPages/portsPhysPortSettHelp.asp';

	function copyData()
	{
		document.configForm.elements[6].value = document.settings.elements[0].value;
		if (numPorts == "1") {
			document.configForm.elements[7].value = document.settings.elements[1].value;
			if (top.family_model != "AVCS") {
				if (document.settings.elements[0].value == "105") {
					document.configForm.elements[14].value = document.settings.elements[2].value;
					document.configForm.elements[15].value = document.settings.elements[3].value;
				}
			}
			for (var i=0; i < 6 ; i++) {
				document.configForm.elements[i+8].value = 
					document.settings.elements[i+4].value;
			}
		} else {
			if (top.family_model != "AVCS") {
				if (document.settings.elements[0].value == "105") {
					document.configForm.elements[14].value = document.settings.elements[1].value;
					document.configForm.elements[15].value = document.settings.elements[2].value;
				}
			}
			for (var i=0; i < 6 ; i++) {
				document.configForm.elements[i+8].value = 
					document.settings.elements[i+3].value;
			}
		}
	}
    
	function form_verification ()
	{ 
		continue_verification = true;
		if (numPorts == "1")
	  		{checkAlias (document.settings.port_name);}
	}
	  
	function checkAlias (element_for_verification)
	{
		if (element_for_verification.value != '')
  		{
  	 	var string = element_for_verification.value;
		var tabooChars = "/*?~'\" "
   		for (var i = 0; i < string.length; i++) 
   			{
      		if (tabooChars.indexOf(string.charAt(i)) != -1)
      			{
	  			continue_verification = false;
	  			alert ('The Alias you entered is not valid.');
   	  			element_for_verification.focus();
   	  			element_for_verification.select();
	 			break;
	  			}
    		}
  		}
	}
	  
	  function submit()
      {
		form_verification();
		if (continue_verification == true)
		{
		copyData();
		if (document.configForm.elements[0].value != 0) 
		{
			document.configForm.urlOk.value="/normal/ports/physPorts.asp";
		}
		setQuerySSID(document.configForm);
         document.configForm.submit();
		 top.mainReady = null;
		 }
      }
	  
	  /*this page define variables (active or not) see normal/main.asp
	  var accessActive
	  var dataBuffActive
	  var multiUserActive
	  var powManActive
	  var otherActive  and reload frame tabs*/
	  
	  function disablePMAccess(sel)
	  { 
		var index = sel.selectedIndex;
		if (sel.options[index].value == 0) {
				top.accessActive = false;
	  			top.otherActive	= false;
		} else {
				top.accessActive = true;
	  			top.otherActive = true;
		}
	  	parent.tabs.location.reload(); 
	  }

	  function disableLinks(sel)
	  { 
	  var index = sel.selectedIndex;
		hide('document', 'Layer2');
	  switch(sel.options[index].value) {
	        case "105":
			if (top.family_model != "AVCS") {
				top.accessActive = false;
	    		top.dataBuffActive = false;
	  			top.multiUserActive = false;
	  			top.powManActive = false;
	  			top.otherActive	= false;
				top.bidirectActive = false;
	  			parent.tabs.location.reload(); 
	  			show('document', 'Layer1');
				show('document', 'Layer2');
			}
			break;
		case "87":
		case "89":
		case "81":
		case "86":
			top.accessActive = true;
	        	top.dataBuffActive = true;
	  		top.multiUserActive = true;
		        //[LMT] Cybts BUG 4476	
			if (numPMs != 0) {
	        		top.powManActive = true;
			} else {
	  			top.powManActive = false;
			}
	  		top.otherActive	= true; 
			top.bidirectActive = false;
	  		parent.tabs.location.reload();
	  		show('document', 'Layer1');
			break;
		case "70":
			top.bidirectActive = true;
		    top.accessActive = true;
	  		top.dataBuffActive = false;
	  		top.multiUserActive = false;
	  		top.powManActive = false;
	  		top.otherActive	= true; 
	  		parent.tabs.location.reload();
	  		show('document', 'Layer1');
			break;
		default:
			top.bidirectActive = false;
		    top.accessActive = true;
	  		top.dataBuffActive = false;
	  		top.multiUserActive = false;
	  		top.powManActive = false;
	  		top.otherActive	= true; 
	  		parent.tabs.location.reload();
	  		show('document', 'Layer1');}
	  }

          function alertWrongProtocol()
          {
			if (top.family_model != "AVCS")
                alert('The current protocol is incompatible with the Security Profile chosen.');
			else
                alert('The current protocol is incompatible with Security/Services selections.');
          }

     </script>
</head>

<body class="bodyMiniWizard">
<form name="settings"  onSubmit="return false;">	
<table width="400" border="0" align="center" cellpadding="1" cellspacing="0">
  <tr align="left"> 
    <td align="center" height="50" valign="bottom">
	<font class="label">Connection Protocol</font>
	&nbsp;&nbsp;&nbsp;&nbsp;
    <select name="protocol" class="formText" onchange="disableLinks(this);">
	<% var sel = getVal("system.ports.physP.general.protocol");%>
<script type="text/javascript">
var profile = 0;
var bidirect = 0;
if (top.family_model != "AVCS") {
  profile = <%get("system.security.profile.type");%>
  bidirect = <%get("system.security.profile.bidirect");%>
}
  var telnet = <%get("system.security.profile.telnet2sport");%>
  var ssh = <%get("system.security.profile.ssh2sport");%>
  var raw = <%get("system.security.profile.raw2sport");%>
  var localsel = <%write(sel);%>

	//[LMT] Security Enhancement
        if ((localsel == 87) || ((profile!=4) && ((profile!=1 && profile!=0) || telnet))) {
          if (profile==4 || (((profile==1 || localsel == 87) && !telnet)))
                alertWrongProtocol();
          document.write('\                 <option value="87" <% if (sel == "87") write("selected"); %>>Console (Telnet)</option>');
        }

        if ((localsel == 89) || (profile!=1 && profile!=0) || ssh) {
          if ((profile==1 || localsel == 89) && !ssh)
                alertWrongProtocol();
          document.write('\                 <option value="89" <% if (sel == "89") write("selected"); %>>Console (SSH)</option>');
        }
        if ((localsel == 86) || ((profile!=4) && ((profile!=1 && profile!=0) || raw))) {
          if (profile==4 || (((profile==1 || localsel == 86) && !raw)))
                alertWrongProtocol();
          document.write('\                 <option value="86" <% if (sel == "86") write("selected"); %>>Console (Raw)</option>');
        }
        if ((localsel == 81) || ((profile!=4) && ((profile!=1 && profile!=0) || (telnet && ssh)))) {
          if (profile==4 || ((profile==1 || localsel == 81) && (!telnet || !ssh)))
                alertWrongProtocol();
          document.write('\                 <option value="81" <% if (sel == "81") write("selected"); %>>Console (TelnetSSH)</option>');
        }
if (top.family_model != "AVCS")
        if ((localsel == 70) || ((profile!=4) && ((profile!=1) || bidirect))) {
          if (profile==4 || (profile==1 && !bidirect))
                alertWrongProtocol();
          document.write('\                 <option value="70" <% if (sel == "70") write("selected"); %>>Bidirectional Telnet</option>');
        }
</script>

		<option value="69" <% if (sel == "69") write("selected"); %>>Telnet</option>
		<option value="72" <% if (sel == "72") write("selected"); %>>SSHv1</option>
		<option value="50" <% if (sel == "50") write("selected"); %>>SSHv2</option>
		<option value="76" <% if (sel == "76") write("selected"); %>>Local Terminal</option>
		<option value="73" <% if (sel == "73") write("selected"); %>>Raw Socket</option>
		<option value="79" <% if (sel == "79") write("selected"); %>>PPP-No Auth</option>
		<option value="80" <% if (sel == "80") write("selected"); %>>PPP</option>
		<option value="83" <% if (sel == "83") write("selected"); %>>SLIP</option>
		<option value="67" <% if (sel == "67") write("selected"); %>>CSLIP</option>
<script type="text/javascript">
if (top.family_model != "AVCS") document.write('\
		<option value="105" <% if (sel == "105") write("selected"); %>>Power Management</option>');
</script>
    </select>
	</td>
  </tr>

 <script type="text/javascript">
  if (numPorts == "1") {
	document.write('<tr align="center"> <td align="center" height="40" valign="middle">');
	document.write('<font class="label">Alias</font>&nbsp;&nbsp;&nbsp;&nbsp;<input name="port_name" class="formText" maxlength="30" type="text" size="15" value="<% get("system.ports.physP.general.alias");%>">');
	document.write('</td></tr>');
  }
  </script>
</table>
<div id="Layer2" name="Layer2" style="position:absolute; top:90px; width:100%; z-index:1; visibility: hidden;" align="left">	
<table width=400 border="0" align="center" cellpadding="2" cellspacing="0">
<tr align="middle">
    <td align="middle" width=50% valign="middle">
      <font class="label">Power management type</font>
      <select name="pmvendor" class="formText">
	<% var sel = getVal("system.ports.physP.general.pmVendor");
	   var stok = getVal("system.ports.physP.general.stallowed");%>
	<option value="1" <% if (sel == "1") write("selected"); %>>auto detect</option>
	<option value="2" <% if (sel == "2") write("selected"); %>>Cyclades</option>
	<option value="3" <% if (sel == "3") write("selected"); %>>SPC</option>
<script type="text/javascript">
if (<%write(stok);%>) {
	document.write('<option value="4" ');
	if (<%write(sel);%> == "4") document.write("selected");
	document.write('>ServerTech</option>');
}
</script>
	  </select>
	</td>
</tr>
<tr align="middle">
    <td align="middle" width=50% valign="middle">
	<font class="label">Allow Access by </font>
	<select name="pmsessions" class="formText" onchange="disablePMAccess(this);">
	<option value="0" <%  if (sel == "0") write("selected"); %>>NONE</option>
    <script type="text/javascript">
	<% var sel = getVal("system.ports.physP.general.pmsessions");%>
  	var localsel;
	if (top.family_model != "AVCS")
		localsel = <%write(sel);%>
	else
		localsel = 0;

        if ((localsel == 1) || (profile!=1) || ssh) {
          if ((profile==1) & !ssh)
                alertWrongProtocol();
          document.write('\                 <option value="1" <% if (sel == "1") write("selected"); %>>SSH</option>');
        }

        if ((localsel == 2) || ((profile!=4) && ((profile!=1) || telnet))) {
          if ((profile==4) || ((profile==1) & !telnet))
                alertWrongProtocol();
          document.write('\                <option value="2" <% if (sel == "2") write("selected"); %>>Telnet</option>');
        }

        if ((localsel == 3) || telnet) {
          if ((profile==4) || ((profile==1) & !telnet))
                alertWrongProtocol();
          document.write('\                 <option value="3" <% if (sel == "3") write("selected"); %>>SSH and Telnet</option>');
        }
    </script>
</td></tr>
</table>
</div>
<div id="Layer1" name="Layer1" style="position:absolute; top:150px; width:100%; z-index:1; visibility: visible;" align="center">	
<table width="550" border="0" align="center" cellpadding="2" cellspacing="0">
<tr align="center" valign="top">
    <td height="45" align="center" colspan="4">
	<font class="label">Baud Rate (Kbps)</font>&nbsp;&nbsp;&nbsp;&nbsp;           
	<select name="baudRate" class="formText">
	    <% var selected = getVal("system.ports.physP.serial.baudRate"); 
               var nospeed=1; %>
              <option value="300" <% if (300 == selected) { write("selected");  nospeed=0; }; %>>  300 </option>
              <option value="1200" <% if (1200 == selected) { write("selected"); nospeed=0; }; %>> 1200 </option>
              <option value="2400" <% if (2400 == selected) { write("selected"); nospeed=0; }; %>> 2400 </option>
              <option value="4800" <% if (4800 == selected) { write("selected"); nospeed=0; }; %>> 4800 </option>
              <option value="9600" <% if (9600 == selected) { write("selected"); nospeed=0; }; %>> 9600 </option>
              <option value="14400" <% if (14400 == selected) { write("selected"); nospeed=0; }; %>> 14400 </option>
              <option value="19200" <% if (19200 == selected) { write("selected"); nospeed=0; }; %>> 19200 </option>
              <option value="28800" <% if (28800 == selected) { write("selected"); nospeed=0; }; %>> 28800 </option>
              <option value="38400" <% if (38400 == selected) { write("selected"); nospeed=0; }; %>> 38400 </option>
              <option value="57600" <% if (57600 == selected) { write("selected"); nospeed=0; }; %>> 57600 </option>
              <option value="76800" <% if (76800 == selected) { write("selected"); nospeed=0; }; %>> 76800 </option>
              <option value="115200" <% if (115200 == selected) { write("selected"); nospeed=0; }; %>> 115200 </option>
              <option value="230400" <% if (230400 == selected) { write("selected"); nospeed=0; }; %>> 230400 </option>
	      <% if (nospeed == 1) { write("<option value='");write(selected);write("' selected> ");write(selected);write(" ** </option>"); }; %>
    </select></td>
  </tr>
  <tr align="left" valign="top"> 
	<td height="45" align="left" width="100">
	<font class="label">Flow Control</font></td>
    <td height="45">
            <select name="flowControl" class="formText">
	    <% var selected = getVal("system.ports.physP.serial.flowControl"); %>
              <option value="0" <% if (0 == selected) write("selected");%>>None</option>
              <option value="1" <% if (1 == selected) write("selected");%>>Hardware</option>
              <option value="2" <% if (2 == selected) write("selected");%>>Software</option>
            </select>	
	</td>
    <td height="45" align="left" width="100">
	<font class="label">Data</font></td>
    <td height="45">            
	<select name="dataSize" class="formText">
	    <% var selected = getVal("system.ports.physP.serial.dataSize"); %>
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
	    <% var selected = getVal("system.ports.physP.serial.parity");%>
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
	    <% var selected = getVal("system.ports.physP.serial.stopBits");%>
              <option value="1" <% if (1 == selected) write("selected");%>>1</option>
              <option value="2" <% if (2 == selected) write("selected");%>>2</option>
            </select>	
	</td>
  </tr>
  <tr align="left" valign="top"> 
    <td height="45" align="left">
	<font class="label">DCD State</font></td>
    <td height="45">
            <select name="dcdState" class="formText">
	    <% var selected = getVal("system.ports.physP.serial.dcdState");%>
              <option value="0" <% if (0 == selected) write("selected");%>>Disregard</option>
              <option value="1" <% if (1 == selected) write("selected");%>>Regard</option>
            </select>	
	</td>
  </tr>
</table>
</div>
</form>
<script type="text/javascript">
	disableLinks(document.settings.protocol);
if (top.family_model != "AVCS")
	if (document.settings.protocol.options[document.settings.protocol.selectedIndex].value == "105")
		disablePMAccess(document.settings.pmsessions);
</script>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
    <input type="hidden" name="system.req.action" value="0">
        <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
 	<input type="hidden" name="urlOk" value="/normal/ports/physPorts.asp">
	<input type="hidden" name="urlError" value="/normal/ports/physPorts.asp">
        <input type="hidden" name="request" value=<% write(ReqNum);%>>
   	<input type="hidden" name="system.ports.physP.action" value="2">
   
        <!--these hidden inputs are used to store values of form elements inside layers-->
        <input type="hidden" name="system.ports.physP.general.protocol" value="">
        <input type="hidden" name="system.ports.physP.general.alias" value="">
        <input type="hidden" name="system.ports.physP.serial.baudRate" value="">
        <input type="hidden" name="system.ports.physP.serial.flowControl" value="">
        <input type="hidden" name="system.ports.physP.serial.dataSize" value="">
        <input type="hidden" name="system.ports.physP.serial.parity" value="">
        <input type="hidden" name="system.ports.physP.serial.stopBits" value="">
        <input type="hidden" name="system.ports.physP.serial.dcdState" value="">
<script type="text/javascript">
if (top.family_model != "AVCS") document.write('\
        <input type="hidden" name="system.ports.physP.general.pmVendor" value="">\
        <input type="hidden" name="system.ports.physP.general.pmsessions" value="">');
</script>
        <!--*************************************************************************-->
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
