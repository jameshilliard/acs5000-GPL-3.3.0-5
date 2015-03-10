<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">   

  <%
	var chain = getQueryVal("chain",-1);
	var ruleIndex = getQueryVal("ruleIndex",-1);
	var action;
	var interfaces = getVal("system.administration.unitInfo.ifInfo.allif");
	if (ruleIndex == "-1") {
		action="Add";
		set("system.network.ipTables.rules.ruleSelected",0);
		set("system.network.ipTables.rules.actionRule","2");
	} else {
		action = "Edit";
		set("system.network.ipTables.rules.ruleSelected",ruleIndex);
		set("system.network.ipTables.rules.actionRule","3");
	} 
	ReqNum = getRequestNumber();
   %>
	// Global javascript variables
	var action="<%write(action);%>";
	var chainName="<%write(chain);%>";
  	var interfaces = '<%write(interfaces);%>';
  	
    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
 	
// Layer 1 - num : protocol number
// Layer 2 - log : LOG options  - TARGET
// Layer 3 - reject : Reject options - TARGET
// Layer 4 - tcp : TCP options
// Layer 5 - udp : UDP options
// Layer 6 - icmp : ICMP options

	function showProtocolLayer() 
	{
		switch(document.EditRuleEntryForm.elements[7].value) {         
			case "255": //number
				show('document', 'Layer1');
				hide('document', 'Layer4');
				hide('document', 'Layer5');
				hide('document', 'Layer6');
				break;
			case "1": //icmp
				hide('document', 'Layer1');
				hide('document', 'Layer4');
				hide('document', 'Layer5');
				show('document', 'Layer6');
				break;
			case "6": //tcp
				hide('document', 'Layer1');
				show('document', 'Layer4');
				hide('document', 'Layer5');
				hide('document', 'Layer6');
				break;
			case "17": //udp
				hide('document', 'Layer1');
				hide('document', 'Layer4');
				show('document', 'Layer5');
				hide('document', 'Layer6');
				break;
			case "0": //all
			default :
				hide('document', 'Layer1');
				hide('document', 'Layer4');
				hide('document', 'Layer5');
				hide('document', 'Layer6');
				break;
		}
	}
   
	function showTarget() 
	{
		switch(document.EditRuleEntryForm.elements[0].value) {         
			case "ACCEPT": 
			case "DROP":
			case "RETURN":
				hide('document', 'Layer2');
				hide('document', 'Layer3');
				break;
			case "LOG":
				show('document', 'Layer2');
				hide('document', 'Layer3');
				break;
			case "REJECT":
				hide('document', 'Layer2');
				show('document', 'Layer3');
				break;
			default:
				hide('document', 'Layer2');
				hide('document', 'Layer3');
				break;
		}
	}       
     
	function validateMask(what) 
	{
		var string = what.value;
		var numValid = true;   
		var Chars = ".:0123456789";

		for (var i = 0; i < string.length; i++) {
			if (Chars.indexOf(string.charAt(i)) == -1) {
				what.focus;
				return false;
			}
		}
		if (string > 32) {
			what.focus;
			return false;
		}
		return true;
	} 

	function copyData()
	{
		// target
		document.configForm.elements[8].value = document.EditRuleEntryForm.elements[0].value;

		// source IP/mask - invert
		document.configForm.elements[9].value = document.EditRuleEntryForm.elements[1].value;
		document.configForm.elements[10].value = document.EditRuleEntryForm.elements[2].value;
		if (document.EditRuleEntryForm.elements[3].checked == true) {
			document.configForm.elements[11].value = 1;
		} else {
			document.configForm.elements[11].value = 0;
		}

		// destination IP/mask - invert
		document.configForm.elements[12].value = document.EditRuleEntryForm.elements[4].value;
		document.configForm.elements[13].value = document.EditRuleEntryForm.elements[5].value;
		if (document.EditRuleEntryForm.elements[6].checked == true) {
			document.configForm.elements[14].value = 1;
		} else {
			document.configForm.elements[14].value = 0;
		}

		// protocol / invert / number
		document.configForm.elements[15].value = document.EditRuleEntryForm.elements[7].value;
		if (document.configForm.elements[15].value == "255") {
			document.configForm.elements[16].value = document.EditRuleEntryForm.elements[14].value;
		}
		if (document.EditRuleEntryForm.elements[8].checked == true) {
			document.configForm.elements[17].value = 1;
		} else {
			document.configForm.elements[17].value = 0;
		}

		// in Interface / invert
		document.configForm.elements[18].value = document.EditRuleEntryForm.elements[9].value;
		if (document.EditRuleEntryForm.elements[10].checked == true) {
			document.configForm.elements[19].value = 1;
		} else {
			document.configForm.elements[19].value = 0;
		}

		// out Interface / invert
		document.configForm.elements[20].value = document.EditRuleEntryForm.elements[11].value;
		if (document.EditRuleEntryForm.elements[12].checked == true) {
			document.configForm.elements[21].value = 1;
		} else {
			document.configForm.elements[21].value = 0;
		}

		// Fragment
		document.configForm.elements[22].value = document.EditRuleEntryForm.elements[13].value;

		// if protocol == TCP
		if (document.configForm.elements[15].value == "6") {
			// source port - last - invert
			document.configForm.elements[23].value=document.EditRuleEntryForm.elements[21].value;
			document.configForm.elements[24].value=document.EditRuleEntryForm.elements[22].value;
			if (document.EditRuleEntryForm.elements[23].checked == true) {
				document.configForm.elements[25].value = 1;
			} else {
				document.configForm.elements[25].value = 0;
			}
			// destination port - last - invert
			document.configForm.elements[26].value=document.EditRuleEntryForm.elements[24].value;
			document.configForm.elements[27].value=document.EditRuleEntryForm.elements[25].value;
			if (document.EditRuleEntryForm.elements[26].checked == true) {
				document.configForm.elements[28].value = 1;
			} else {
				document.configForm.elements[28].value = 0;
			}
			
			// TCP flags
			document.configForm.elements[29].value = 0;
			document.configForm.elements[30].value = 0;
			switch (document.EditRuleEntryForm.elements[29].value) { // FIN unset
				case "2" : document.configForm.elements[29].value |= 1; break;
				case "1" : document.configForm.elements[30].value |= 1; break;
			}
			switch (document.EditRuleEntryForm.elements[27].value) { // SYN 
				case "2" : document.configForm.elements[29].value |= 2; break;
				case "1" : document.configForm.elements[30].value |= 2; break;
			}
			switch (document.EditRuleEntryForm.elements[30].value) { // RST unset
				case "2" : document.configForm.elements[29].value |= 4; break;
				case "1" : document.configForm.elements[30].value |= 4; break;
			}
			switch (document.EditRuleEntryForm.elements[32].value) { // PSH unset
				case "2" : document.configForm.elements[29].value |= 8; break;
				case "1" : document.configForm.elements[30].value |= 8; break;
			}
			switch (document.EditRuleEntryForm.elements[28].value) { // ACK unset
				case "2" : document.configForm.elements[29].value |= 16;break;
				case "1" : document.configForm.elements[30].value |= 16; break;
			}
			switch (document.EditRuleEntryForm.elements[31].value) { // URG unset
				case "2" : document.configForm.elements[29].value |= 32; break;
				case "1" : document.configForm.elements[30].value |= 32; break;
			}
			
			// TCP flags invert
			if (document.EditRuleEntryForm.elements[33].checked == true) {
				document.configForm.elements[31].value = 1;
			} else {
				document.configForm.elements[31].value = 0;
			}
		} else {
			// if protocol == UDP
			if (document.configForm.elements[15].value == "17") {
				// source port - last - invert
				document.configForm.elements[23].value=document.EditRuleEntryForm.elements[34].value;
				document.configForm.elements[24].value=document.EditRuleEntryForm.elements[35].value;
				if (document.EditRuleEntryForm.elements[36].checked == true) {
					document.configForm.elements[25].value = 1;
				} else {
					document.configForm.elements[25].value = 0;
				}
				// destination port - last - invert
				document.configForm.elements[26].value=document.EditRuleEntryForm.elements[37].value;
				document.configForm.elements[27].value=document.EditRuleEntryForm.elements[38].value;
				if (document.EditRuleEntryForm.elements[39].checked == true) {
					document.configForm.elements[28].value = 1;
				} else {
					document.configForm.elements[28].value = 0;
				}
			}
		}

		// ICMP - invert
		// if protocol == ICMP
		if (document.configForm.elements[15].value == "1") {
			document.configForm.elements[32].value = document.EditRuleEntryForm.elements[40].value;
			if (document.EditRuleEntryForm.elements[41].checked == true) {
				document.configForm.elements[33].value = 1;
			} else {
				document.configForm.elements[33].value = 0;
			}	
		}
		
		// LOG level / Prefix / TCP options
		document.configForm.elements[34].value = document.EditRuleEntryForm.elements[15].value;
		document.configForm.elements[35].value = document.EditRuleEntryForm.elements[16].value;
		document.configForm.elements[36].value = 0;
		if (document.EditRuleEntryForm.elements[17].checked == true) { // TCP sequence
			document.configForm.elements[36].value = 1;
		}
		if (document.EditRuleEntryForm.elements[18].checked == true) { // TCP options
			document.configForm.elements[36].value |= 2;
		}
		if (document.EditRuleEntryForm.elements[19].checked == true) { // IP options
			document.configForm.elements[36].value |= 4;
		}
	}

  function form_verification()
  {
	var form_location = document.EditRuleEntryForm;
	
	continue_verification = true; 
	checkElement (form_location.source_ip, false, false, true, false);				
	// Source IP				 	
	if (continue_verification == true && form_location.mask_source.value != '')
	  	{checkNetmask (form_location.mask_source);}						
	// Destination IP
	if (continue_verification == true)
		{checkElement (form_location.destin_ip, false, false, true, false);}		
	// Mask	
	if (continue_verification == true && form_location.mask_destin.value != '')
		{checkNetmask (form_location.mask_destin);} 					
	// Protocol
	if (continue_verification == true && form_location.target_protocol.value == 255)
		{checkElement (form_location.protocolNum, true, false, false, true);}		
	var vinterfaces = interfaces.split(",");
	// Input Interface
	if (continue_verification == true && (chainName == "INPUT" || chainName == "FORWARD")) { 
		continue_verification = false;
		for(v in vinterfaces)
			if ( form_location.input_interface.value == vinterfaces[v] )
				continue_verification = true;
		if(continue_verification == false)
			alert('Invalid Input Interface');
	}
	// Output Interface	
	if (continue_verification == true && (chainName == "OUTPUT" || chainName == "FORWARD")) { 
		continue_verification = false;
		for(v in vinterfaces)
			if ( form_location.output_interface.value == vinterfaces[v] )
				continue_verification = true;
		if(continue_verification == false)
			alert('Invalid Output Interface');
	}
		
	// *********  LOG ***********	
	if (continue_verification == true && form_location.target.value == "LOG")
		{checkElement (form_location.log_prefix, false, true, false, false);}		
	// Log Prefix																				  			
	
	// ********  TCP  ***********
	if (continue_verification == true && form_location.target_protocol.value == 6)	
		{checkElement (form_location.source_port, false, false, false, true);}		
	// Source Port
	if (continue_verification == true && form_location.target_protocol.value == 6)	
		{checkElement (form_location.to_source_port, false, false, false, true);}	
	// To
	if (continue_verification == true && form_location.target_protocol.value == 6)	
		{checkElement (form_location.destin_port, false, false, false, true);}		
	// Destination Port					
	if (continue_verification == true && form_location.target_protocol.value == 6)	
		{checkElement (form_location.to_destin_port, false, false, false, true);}	
	// To
	
	// *********  UDP  **********
	if (continue_verification == true && form_location.target_protocol.value == 17)	
		{checkElement (form_location.source_port_udp, false, false, false, true);}	
	// Source Port
	if (continue_verification == true && form_location.target_protocol.value == 17)	
		{checkElement (form_location.to_source_port_udp, false, false, false, true);}
	// To  
	if (continue_verification == true && form_location.target_protocol.value == 17)	
		{checkElement (form_location.destin_port_udp, false, false, false, true);}	
	// Destination Port	
	if (continue_verification == true && form_location.target_protocol.value == 17)	
		{checkElement (form_location.to_destin_port_udp, false, false, false, true);}	
	// To							
  }		

	function init()
	{
		if ((chainName != "OUTPUT") && (chainName != "FORWARD") && (chainName == "INPUT")) { 
			document.EditRuleEntryForm.output_interface.disabled = true;
			document.EditRuleEntryForm.inverted_output_interface.disabled = true;
		}
		if ((chainName != "INPUT") && (chainName != "FORWARD") && (chainName == "OUTPUT")) { 
			document.EditRuleEntryForm.input_interface.disabled = true;
			document.EditRuleEntryForm.inverted_input_interface.disabled = true;
		}
	}
	
	function submitRule()
	{
	try {
	     if (opener.window.name != document.configForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
		form_verification();
		if (continue_verification == true) {
			copyData();
			document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
			document.configForm.elements[2].value = "/normal/network/netIPTablesEditRule.asp";
			document.configForm.elements[4].value = opener.document.configForm.elements[4].value;
				// number of rules
				if (action == "Add") {
					document.configForm.elements[5].value = opener.document.netIPTableRuleForm.table.length;
				} else {
					document.configForm.elements[5].value = opener.document.configForm.elements[5].value;
				}
			// rule list
			document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
			// number of rule selected
			document.configForm.elements[7].value = opener.document.configForm.elements[7].value;
            setQuerySSID(document.configForm);
			document.configForm.submit();
			self.close();
		}
	} catch (error) {
	  self.close();
	}
	}
	
	function interfaceSelection(iface) {
	  var sinterfaces = ','+interfaces;
	  sinterfaces = sinterfaces.split(",");
      for(i in sinterfaces) {
        document.write("<option value=\""+sinterfaces[i]+"\"");
        if(sinterfaces[i]==iface) {
          document.write(" selected");
        }
        document.write(">"+sinterfaces[i]+"</option>");
      }  
	}
    </script>

<title><%write(chain + ' chain - ' + action+' Rule');%></title>
</head>
<body onLoad="init();" class="bodyMain">   
<form name="EditRuleEntryForm" onSubmit="return false;">
<table width="540" border="0" cellspacing="0" cellpadding="0">
  <tr align="center" valign="middle"> 
    <td height="40" colspan="5">
        <a href="javascript:submitRule();">
         <img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
         &nbsp;&nbsp;&nbsp;&nbsp;
       <a href="javascript:window.close();">
         <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
   </td>
  </tr>
  <tr align="center" valign="middle"> 
    <td height="30" colspan="5"><font class="label">Target</font></td>
  </tr>
  <tr align="center" valign="middle"> 
    <td height="30" colspan="5">
   <select name="target" size="1" class="formText" onChange="showTarget();">
	<%get("system.network.ipTables.ruleSpec.targetHtml");%>
   </select>
   </td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="35"><font class="label">Source IP</font></td>
    <td width="25%" height="35"><font class="formText"><input name="source_ip" value="<%get("system.network.ipTables.ruleSpec.sIp");%>" type="text" size="15" maxlength="15" class="formText"></font></td>
    <td width="12%" height="35" align="center"><font class="label">Mask</font></td>
    <td width="25%" height="35"><font class="formText"><input name="mask_source" type="text" size="15" maxlength="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.sIpMask");%>"></font></td>
	<% var check = getVal("system.network.ipTables.ruleSpec.sIpInvert");%>
    <td width="18%" height="35">&nbsp;&nbsp;<input type="checkbox" name="inverted_source" <% if (check==1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="30"><font class="label">Destination IP</font></td>
    <td width="25%" height="30"><font class="formText"><input name="destin_ip" type="text" size="15" maxlength="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.dIp");%>"></font></td>
    <td width="12%" height="30" align="center"><font class="label">Mask</font></td>
    <td width="25%" height="30"><font class="formText"><input name="mask_destin" type="text" size="15" maxlength="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.dIpMask");%>"></font></td>
	<% var check = getVal("system.network.ipTables.ruleSpec.dIpInvert");%>
    <td width="18%" height="30">&nbsp;&nbsp;<input type="checkbox" name="inverted_destin" <% if (check==1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="30"><font class="label">Protocol</font></td>
    <td width="25%" height="30">
	<% var sel=getVal("system.network.ipTables.ruleSpec.protocol");%>
   <select name="target_protocol" size="1" class="formText" onChange="showProtocolLayer();">
      <option value="255" <% if (sel==255) write("selected"); %>>Numeric</option>
      <option value="0" <% if (sel==0) write("selected");%> >All</option>
      <option value="6" <% if (sel==6) write("selected");%>>TCP</option>
      <option value="17" <% if (sel==17) write("selected");%>>UDP</option>
      <option value="1" <% if (sel==1) write("selected");%>>ICMP</option>
   </select>
   </td>
    <td width="12%" height="30">&nbsp;</td>
	<% var check=getVal("system.network.ipTables.ruleSpec.protocolInvert");%>
    <td width="25%" height="30" align="center">&nbsp;&nbsp;<input type="checkbox" name="inverted_protocol" <% if (check==1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
    <td width="18%" height="30">&nbsp;</td>
  </tr>
  <tr align="left" valign="middle">
<!-- This and the next field are shown only if user chose the following predefined Chains INPUT, FORWARD. 
If user chose to add a rule to just a user defined chain, then this field is not asked.-->
    <td width="20%" height="30"><font class="label">Input Interface</font></td>
    <td width="25%" height="30"><select name="input_interface" class="formText">
      <script type="text/javascript">
         interfaceSelection('<%get("system.network.ipTables.ruleSpec.inInterface");%>');
      </script>
    </select></td>
	<% var check=getVal("system.network.ipTables.ruleSpec.inInterfaceInvert");%>
    <td height="30" colspan="3">&nbsp;&nbsp;<input type="checkbox" name="inverted_input_interface" <%if (check==1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
  <tr align="left" valign="middle">
<!-- This and the next field are shown only if user chose the following predefined chains: FORWARD, OUTPUT--> 
    <td width="20%" height="30"><font class="label">Output Interface</font></td>
    <td width="25%" height="30"><select name="output_interface" class="formText">
      <script type="text/javascript">
         interfaceSelection('<%get("system.network.ipTables.ruleSpec.outInterface");%>');
      </script>
    </select></td>
	<% var check=getVal("system.network.ipTables.ruleSpec.outInterfaceInvert");%>
    <td height="30" colspan="3">&nbsp;&nbsp;<input type="checkbox" name="inverted_output_interface" <%if (check==1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="30"><font class="label">Fragments</font></td>
    <td height="30" colspan="4">
	<% var sel=getVal("system.network.ipTables.ruleSpec.fragment");%>
   <select name="Fragments" size="1" class="formText">
      <option value="0" <% if (sel==0) write("selected");%>>All packets</option> 
      <option value="1" <% if (sel==1) write("selected");%>>2nd, 3rd&#8230; fragmented packets</option>
      <option value="2" <% if (sel==2) write("selected");%>>Non-fragmented and 1st fragmented packets</option>
   </select>
   </td>
  </tr>
</table>

<div id="Layer1" name="num" style="position:absolute; left:232px; top:184px; width:100px; height:25 z-index:1; visibility: visible;" align="center">
<table><tr align="center" valign="bottom">
   <td width="180" height="25">
   <input name="protocolNum" type="text" size="8" class="formText" value="<%get("system.network.ipTables.ruleSpec.protocolNumber");%>"></td>
</tr></table>
</div>

<div id="Layer2" name="log" style="position:absolute; left:21px; top:310px; width:559px; height:200px; z-index:5; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" cellspacing="0" cellpadding="0">
  <tr align="center" valign="bottom"> 
    <td height="40" colspan="5"><font  class="tabsTextBig">LOG Options Section</font></td>
  </tr>
  <tr align="left" valign="bottom"> 
    <td width="18%" height="30"><font  class="label">Log Level</font></td>
    <td width="18%" height="30">
	<% var sel=getVal("system.network.ipTables.ruleSpec.LogLevel");%>
         <select name="log_level" size="1" class="formText">
                <option value="0" <% if (sel==0) write("selected");%>>emerg</option>
                <option value="1" <% if (sel==1) write("selected");%>>alert</option>
                <option value="2" <% if (sel==2) write("selected");%>>crit</option>
                <option value="3" <% if (sel==3) write("selected");%>>err</option>
                <option value="4" <% if (sel==4) write("selected");%>>warning</option>
                <option value="5" <% if (sel==5) write("selected");%>>notice</option>
                <option value="6" <% if (sel==6) write("selected");%>>info</option>
                <option value="7" <% if (sel==7) write("selected");%>>debug</option>
          </select>
   </td>
    <td width="18%" height="30" align="center"><font  class="label">Log Prefix</font></td>
    <td height="30" colspan="2"><font  class="formText"><input name="log_prefix" type="text" size="15" maxlength="29" class="formText" value="<%get("system.network.ipTables.ruleSpec.LogPrefix");%>"></font></td>
  </tr>
  <tr align="center" valign="bottom"> 
    <td height="30" colspan="5"><table width="560" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr align="center" valign="middle"> 
	<% var check=getVal("system.network.ipTables.ruleSpec.LogTcpOptions");
	   var check1 = (check % 2);
           var check2 = (check >> 1) % 2;
	   var check4 = (check >> 2) % 2;%>
          <td height="30"><input type="checkbox" name="tcp_sequence" <% if (check==1) write("checked");%>>
         &nbsp;&nbsp;&nbsp;<font  class="label">TCP sequence</font></td>
          <td height="30"><input type="checkbox" name="tcp_options" <% if (check2==1) write("checked");%>>
         &nbsp;&nbsp;&nbsp;<font  class="label">TCP options</font></td>
          <td height="30"><input type="checkbox" name="ip_options" <% if (check4==1) write("checked");%>>
         &nbsp;&nbsp;&nbsp;<font  class="label">IP options</font></td>
        </tr>
      </table></td>
  </tr>
</table>
</div>
<div id="Layer3" name="reject" style="position:absolute; left:21px; top:310px; width:559px; height:200px; z-index:5; visibility: hidden;" class="bgColor12" align="left">
  <table width="540" border="0" cellspacing="0" cellpadding="0" align="left">
          <tr align="center" valign="bottom"> 
            <td height="40" colspan="4"><font  class="tabsTextBig">REJECT Options Section</font></td>
          </tr>
          <tr align="center" valign="bottom">
          <td height="30" width="100" align="center"></td> 
            <td height="30" width="140" align="center"><font  class="label">Reject with</font></td>
         <td height="30" width="200" align="center">
		<% var sel=getVal("system.network.ipTables.ruleSpec.RejectWith");%>
         <select name="reject_with" size="1" class="formText">
                <option value="0" <% if (sel==0) write("selected"); %>>icmp-net-unreachable</option>
                <option value="1" <% if (sel==1) write("selected"); %>>icmp-host-unreachable</option>
                <option value="2" <% if (sel==2) write("selected"); %>>icmp-port-unreachable</option>
                <option value="3" <% if (sel==3) write("selected"); %>>icmp-proto-unreachable</option>
                <option value="4" <% if (sel==4) write("selected"); %>>icmp-net-prohibited</option>
                <option value="5" <% if (sel==5) write("selected"); %>>icmp-host-prohibited</option>
                <option value="6" <% if (sel==6) write("selected"); %>>echo-reply</option>
                <option value="7" <% if (sel==7) write("selected"); %>>tcp-reset</option>
            </select>
         </td>
         <td height="30" width="100" align="center"></td>
          </tr>
</table>
</div>    
<div id="Layer4" name="tcp" style="position:absolute; left:21px; top:430px; width:559px; z-index:6; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" cellspacing="0" cellpadding="0">
  <tr align="center" valign="middle"> 
    <td height="40" colspan="6"><font  class="tabsTextBig">TCP Options Section</font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="30"><font class="label">Source Port</font></td>
    <td width="25%" height="30"><font class="formText"><input name="source_port" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.sPort");%>"></font></td>
    <td width="12%" height="30" align="center"><font class="label">to</font></td>
    <td width="25%" height="30"><font class="formText"><input name="to_source_port" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.LastSPort");%>"></font></td>
	<% var check=getVal("system.network.ipTables.ruleSpec.sPortInvert");%>
    <td width="18%" height="30" colspan="2">&nbsp;&nbsp;<input type="checkbox" name="inverted_source_port" <%if (check==1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="30"><font  class="label">Destination Port</font></td>
    <td width="25%" height="30"><font class="formText"><input name="destin_port" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.dPort");%>"></font></td>
    <td width="12%" height="30" align="center"><font  class="label">to</font></td>
    <td width="25%" height="30"><font class="formText"><input name="to_destin_port" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.LastDPort");%>"></font></td>
	<% var check=getVal("system.network.ipTables.ruleSpec.dPortInvert");%>
    <td width="18%" height="30" colspan="2">&nbsp;&nbsp;<input type="checkbox" name="inverted_destin_port" <%if (check==1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
  <tr align="center" valign="middle">
    <td height="30" colspan="6"><font  class="label">TCP Flags</font></td>
  </tr>
  <tr align="left" valign="middle">
    <td height="30" colspan="6">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr valign="middle"> 
	<% var flagset = getVal("system.network.ipTables.ruleSpec.TcpFlagsSet");
	   var flagunset = getVal("system.network.ipTables.ruleSpec.TcpFlags");
	   var synset = (flagset >>1) % 2;
	   var ackset = (flagset >>4) % 2;
       var finset = (flagset >>0) % 2;
	   var rstset = (flagset >>2) % 2;
	   var urgset = (flagset >>5) % 2;
	   var pshset = (flagset >>3) % 2;
	   var synunset = (flagunset >> 1) % 2;
	   var ackunset = (flagunset >> 4) % 2;
       var finunset = (flagunset >> 0) % 2;
	   var rstunset = (flagunset >> 2) % 2;
	   var urgunset = (flagunset >> 5) % 2;
	   var pshunset = (flagunset >> 3) % 2;
	%>
    <td height="30" align="right"><font  class="label">SYN</font>&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td height="30" align="left">
   <select name="syn" size="1" class="formText">
        <option value="0" <% if ((synset==0) && (synunset==0)) write("selected");%>>Any</option>
        <option value="1" <% if ((synset==1) && (synunset==0)) write("selected");%>>Set</option>
        <option value="2" <% if ((synset==0) && (synunset==1)) write("selected");%>>Unset</option>
    </select>
   </td>
    <td height="30" align="right"><font  class="label">ACK</font>&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td height="30" align="left">
   <select name="ack" size="1" class="formText">
        <option value="0" <% if ((ackset==0) && (ackunset==0)) write("selected");%>>Any</option>
        <option value="1" <% if ((ackset==1) && (ackunset==0)) write("selected");%>>Set</option>
        <option value="2" <% if ((ackset==0) && (ackunset==1)) write("selected");%>>Unset</option>
    </select>
   </td>
    <td height="30" align="right"><font  class="label">FIN</font>&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td height="30" align="left">
   <select name="fin" size="1" class="formText">
        <option value="0" <% if ((finset==0) && (finunset==0)) write("selected");%>>Any</option>
        <option value="1" <% if ((finset==1) && (finunset==0)) write("selected");%>>Set</option>
        <option value="2" <% if ((finset==0) && (finunset==1)) write("selected");%>>Unset</option>
    </select>
   </td>
  </tr>
    <tr valign="middle"> 
    <td height="30" align="right"><font  class="label">RST</font>&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td height="30" align="left">
   <select name="rst" size="1" class="formText">
        <option value="0" <% if ((rstset==0) && (rstunset==0)) write("selected");%>>Any</option>
        <option value="1" <% if ((rstset==1) && (rstunset==0)) write("selected");%>>Set</option>
        <option value="2" <% if ((rstset==0) && (rstunset==1)) write("selected");%>>Unset</option>
    </select>
   </td>
    <td height="30" align="right"><font  class="label">URG</font>&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td height="30" align="left">
   <select name="urg" size="1" class="formText">
        <option value="0" <% if ((urgset==0) && (urgunset==0)) write("selected");%>>Any</option>
        <option value="1" <% if ((urgset==1) && (urgunset==0)) write("selected");%>>Set</option>
        <option value="2" <% if ((urgset==0) && (urgunset==1)) write("selected");%>>Unset</option>
    </select>
   </td>
    <td height="30" align="right"><font  class="label">PSH</font>&nbsp;&nbsp;&nbsp;&nbsp;</td>
    <td height="30" align="left">
   <select name="psh" size="1" class="formText">
        <option value="0" <% if ((pshset==0) && (pshunset==0)) write("selected");%>>Any</option>
        <option value="1" <% if ((pshset==1) && (pshunset==0)) write("selected");%>>Set</option>
        <option value="2" <% if ((pshset==0) && (pshunset==1)) write("selected");%>>Unset</option>
    </select>
   </td>
  </tr>
   </table>
   </td>
  </tr>
  <tr> 
	<% var check=getVal("system.network.ipTables.ruleSpec.TcpFlagsInvert");%>
    <td height="30" colspan="6" align="center" valign="middle">&nbsp;&nbsp;<input type="checkbox" name="inverted_tcp_flags" <% if (check == 1) write("checked");%>>
   &nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
</table>
</div>
<div id="Layer5" name="udp" style="position:absolute; left:21px; top:430px; width:559px; z-index:6; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" cellspacing="0" cellpadding="0">
  <tr align="center" valign="middle"> 
    <td height="40" colspan="6"><font  class="tabsTextBig">UDP Options Section</font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="30"><font class="label">Source Port</font></td>
    <td width="25%" height="30"><font class="formText"><input name="source_port_udp" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.sPort");%>"></font></td>
    <td width="12%" height="30" align="center"><font class="label">to</font></td>
    <td width="25%" height="30"><font class="formText"><input name="to_source_port_udp" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.LastsPort");%>"></font></td>
	<% var check=getVal("system.network.ipTables.ruleSpec.sPortInvert");%>
    <td width="18%" height="30" colspan="2">&nbsp;&nbsp;<input type="checkbox" name="inverted_source_port_udp" <%if (check == 1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="20%" height="30"><font  class="label">Destination Port</font></td>
    <td width="25%" height="30"><font class="formText"><input name="destin_port_udp" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.dPort");%>"></font></td>
    <td width="12%" height="30" align="center"><font  class="label">to</font></td>
    <td width="25%" height="30"><font class="formText"><input name="to_destin_port_udp" type="text" size="15" class="formText" value="<%get("system.network.ipTables.ruleSpec.LastDPort");%>"></font></td>
	<% var check=getVal("system.network.ipTables.ruleSpec.dPortInvert");%>
    <td width="18%" height="30" colspan="2">&nbsp;&nbsp;<input type="checkbox" name="inverted_destin_port_udp" <% if (check == 1) write("checked");%>>&nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
</table>
</div>   
<div id="Layer6" name="icmp" style="position:absolute; left:21px; top:430px; width:540px; z-index:6; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" align="left" cellpadding="0" cellspacing="3">
  <tr align="center" valign="bottom"> 
    <td height="40"><font  class="tabsTextBig">ICMP Options Section</font></td>
  </tr>
  <tr align="center" valign="bottom"> 
    <td height="25"><font class="label">ICMP Type</font></td>
  </tr>
  <tr align="center" valign="bottom"> 
    <td height="25">
	<% var sel=getVal("system.network.ipTables.ruleSpec.ICMPType");%>
     <select name="icmptype" size="1" class="formText">
      <option value="all" <%if (sel=="all") write("selected");%>>all</option>
      <option value="echo-reply" <% if (sel=="echo-reply") write("selected");%>>echo-reply</option>
      <option value="destination-unreachable" <% if (sel=="destination-unreachable") write("selected");%>>destination-unreachable</option>
      <option value="network-unreachable" <% if (sel=="network-unreachable") write("selected");%>>network-unreachable</option>
      <option value="host-unreachable" <% if (sel=="host-unreachable") write("selected");%>>host-unreachable</option>
      <option value="protocol-unreachable" <% if (sel=="protocol-unreachable") write("selected");%>>protocol-unreachable</option>
      <option value="port-unreachable" <% if (sel=="port-unreachable") write("selected");%>>port-unreachable</option>
      <option value="fragmentation-needed" <% if (sel=="fragmentation-needed") write("selected");%>>fragmentation-needed</option>
      <option value="source-route-failed" <% if (sel=="source-route-failed") write("selected");%>>source-route-failed</option>
      <option value="network-unknown" <% if (sel=="network-unknown") write("selected");%>>network-unknown</option>
      <option value="host-unknown" <% if (sel=="host-unknown") write("selected");%>>host-unknown</option>
      <option value="network-prohibited" <% if (sel=="network-prohibited") write("selected");%>>network-prohibited</option>
      <option value="host-prohibited" <% if (sel=="host-prohibited") write("selected");%>>host-prohibited</option>
      <option value="TOS-network-unreachable" <% if (sel=="TOS-network-unreachable") write("selected");%>>TOS-network-unreachable</option>
      <option value="TOS-host-unreachable" <% if (sel=="TOS-host-unreachable") write("selected");%>>TOS-host-unreachable</option>
      <option value="communication-prohibited" <% if (sel=="communication-prohibited") write("selected");%>>communication-prohibited</option>
      <option value="host-precedence-violation" <% if (sel=="host-precedence-violation") write("selected");%>>host-precedence-violation</option>
      <option value="precedence-cutoff" <% if (sel=="precedence-cutoff") write("selected");%>>precedence-cutoff</option>
      <option value="source-quench" <% if (sel=="source-quench") write("selected");%>>source-quench</option>
      <option value="redirect" <% if (sel=="redirect") write("selected");%>>redirect</option>
      <option value="network-redirect" <% if (sel=="network-redirect") write("selected");%>>network-redirect</option>
      <option value="host-redirect" <% if (sel=="host-redirect") write("selected");%>>host-redirect</option>
      <option value="TOS-network-redirect" <% if (sel=="TOS-network-redirect") write("selected");%>>TOS-network-redirect</option>
      <option value="TOS-host-redirect" <% if (sel=="TOS-host-redirect") write("selected");%>>TOS-host-redirect</option>
      <option value="echo-request" <% if (sel=="echo-request") write("selected");%>>echo-request</option>
      <option value="router-advertisement" <% if (sel=="router-advertisement") write("selected");%>>router-advertisement</option>
      <option value="router-solicitation" <% if (sel=="router-solicitation") write("selected");%>>router-solicitation</option>
      <option value="time-exceeded" <% if (sel=="time-exceeded") write("selected");%>>time-exceeded</option>
      <option value="ttl-zero-during-transit" <% if (sel=="ttl-zero-during-transit") write("selected");%>>ttl-zero-during-transit</option>
      <option value="ttl-zero-during-reassembly" <% if (sel=="ttl-zero-during-reassembly") write("selected");%>>ttl-zero-during-reassembly</option>
      <option value="parameter-problem" <% if (sel=="parameter-problem") write("selected");%>>parameter-problem</option>
      <option value="ip-header-bad" <% if (sel=="ip-header-bad") write("selected");%>>ip-header-bad</option>
      <option value="required-option-missing" <% if (sel=="required-option-missing") write("selected");%>>required-option-missing</option>
      <option value="timestamp-request" <% if (sel=="timestamp-request") write("selected");%>>timestamp-request</option>
      <option value="timestamp-reply" <% if (sel=="timestamp-reply") write("selected");%>>timestamp-reply</option>
      <option value="address-mask-request" <% if (sel=="address-mask-request") write("selected");%>>address-mask-request</option>
      <option value="address-mask-reply" <% if (sel=="address-mask-reply") write("selected");%>>address-mask-reply</option>
      </select>
   </td>    
  </tr>
  <tr align="center" valign="bottom">
	<% var check=getVal("system.network.ipTables.ruleSpec.ICMPTypeInvert");%>
  <td height="25">&nbsp;&nbsp;<input type="checkbox" name="inverted_icmp_type" <%if (check==1) write("checked");%>>
   &nbsp;&nbsp;<font class="label">Inverted</font></td>
  </tr>
</table>
</div>
</form>

<script>
	showProtocolLayer();
	showTarget();
</script>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/network/netIPTablesEditRule.asp">
   <input type="hidden" name="urlError" value="/normal/network/netIPTablesEditRuleEntry.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
   <input type="hidden" name="system.network.ipTables.rules.numRules" value="">
   <input type="hidden" name="system.network.ipTables.rules.rulesCs" value="">
   <input type="hidden" name="system.network.ipTables.rules.actionRule" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.target" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.sIp" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.sIpMask" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.sIpInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.dIp" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.dIpMask" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.dIpInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.protocol" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.protocolNumber" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.protocolInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.inInterface" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.inInterfaceInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.outInterface" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.outInterfaceInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.fragment" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.sPort" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.LastSPort" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.sPortInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.dPort" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.LastDPort" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.dPortInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.TcpFlags" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.TcpFlagsSet" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.TcpFlagsInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.ICMPType" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.ICMPTypeInvert" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.LogLevel" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.LogPrefix" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.LogTcpOptions" value="">
   <input type="hidden" name="system.network.ipTables.ruleSpec.RejectWith" value="">
</form>
</body>
</html>
