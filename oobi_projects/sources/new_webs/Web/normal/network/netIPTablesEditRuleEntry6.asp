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
		var interfaces="<%write(interfaces);%>";
		var optTarget=<%get("system.network.ipTables.ruleSpec.targetHtml");%>;
		var optInterfaces=new Array();
  	
		window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
 	
		// Layer 1 - num : protocol number
		// Layer 2 - log : LOG options  - TARGET
		// Layer 3 - reject : Reject options - TARGET
		// Layer 4 - tcp : TCP options
		// Layer 5 - udp : UDP options
		// Layer 6 - icmp : ICMP options
		function showProtocolLayer() {
			switch(document.EditRuleEntryForm.protocol.value) {         
				case "255": //number
					show('document', 'Layer1');
					hide('document', 'Layer4');
					hide('document', 'Layer5');
					hide('document', 'Layer6');
				break;
				case "58": //icmpv6
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

		function showTarget() {
			switch(document.EditRuleEntryForm.target.value) {         
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

		function validateMask(what) {
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

		function copyData() {

			// target
			document.configForm.elements["system.network.ipTables.ruleSpec.target"].value =
				document.EditRuleEntryForm.target.value;

			// source IP/mask - invert
			document.configForm.elements["system.network.ipTables.ruleSpec.sIp"].value =
				document.EditRuleEntryForm.sIp.value;
			document.configForm.elements["system.network.ipTables.ruleSpec.sIpMask"].value =
				"<%get("system.network.ipTables.ruleSpec.sIpMask");%>";
			document.configForm.elements["system.network.ipTables.ruleSpec.sIpInvert"].value =
				document.EditRuleEntryForm.sIpInvert.checked? 1 : 0;

			// destination IP/mask - invert
			document.configForm.elements["system.network.ipTables.ruleSpec.dIp"].value =
				document.EditRuleEntryForm.dIp.value;
			document.configForm.elements["system.network.ipTables.ruleSpec.dIpMask"].value =
				"<%get("system.network.ipTables.ruleSpec.dIpMask");%>";
			document.configForm.elements["system.network.ipTables.ruleSpec.dIpInvert"].value =
				document.EditRuleEntryForm.dIpInvert.checked? 1 : 0;

			// protocol / invert / number
			document.configForm.elements["system.network.ipTables.ruleSpec.protocol"].value =
				document.EditRuleEntryForm.protocol.value;
			document.configForm.elements["system.network.ipTables.ruleSpec.protocolInvert"].value =
				document.EditRuleEntryForm.protocolInvert.checked? 1 : 0;
			if (document.EditRuleEntryForm.protocol.value == "255") {
				document.configForm.elements["system.network.ipTables.ruleSpec.protocolNumber"].value =
					document.EditRuleEntryForm.protocolNumber.value;
			}

			// in Interface / invert
			document.configForm.elements["system.network.ipTables.ruleSpec.inInterface"].value =
				document.EditRuleEntryForm.inInterface.value;
			document.configForm.elements["system.network.ipTables.ruleSpec.inInterfaceInvert"].value =
				document.EditRuleEntryForm.inInterfaceInvert.checked? 1 : 0;

			// out Interface / invert
			document.configForm.elements["system.network.ipTables.ruleSpec.outInterface"].value =
				document.EditRuleEntryForm.outInterface.value;
			document.configForm.elements["system.network.ipTables.ruleSpec.outInterfaceInvert"].value =
				document.EditRuleEntryForm.outInterfaceInvert.checked? 1 : 0;

			// Fragment
			document.configForm.elements["system.network.ipTables.ruleSpec.fragment"].value =
				<%get("system.network.ipTables.ruleSpec.fragment");%>;

			// Reject With
			document.configForm.elements["system.network.ipTables.ruleSpec.RejectWith"].value =
				document.EditRuleEntryForm.target.value=="REJECT"?
				document.EditRuleEntryForm.RejectWith.value :
				"";

			// if protocol == TCP
			if (document.configForm.elements["system.network.ipTables.ruleSpec.protocol"].value == "6") {
				// source port - last - invert
				document.configForm.elements["system.network.ipTables.ruleSpec.sPort"].value =
					document.EditRuleEntryForm.sPortTcp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.LastSPort"].value =
					document.EditRuleEntryForm.LastSPortTcp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.sPortInvert"].value =
					document.EditRuleEntryForm.sPortInvertTcp.checked? 1 : 0;
				// destination port - last - invert
				document.configForm.elements["system.network.ipTables.ruleSpec.dPort"].value =
					document.EditRuleEntryForm.dPortTcp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.LastDPort"].value =
					document.EditRuleEntryForm.LastDPortTcp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.dPortInvert"].value =
					document.EditRuleEntryForm.dPortInvertTcp.checked? 1 : 0;
				// TCP flags
				var TcpFlags = 0;
				var TcpFlagsSet = 0;
				switch (document.EditRuleEntryForm.tcpFin.value) {
					case "1" : TcpFlagsSet |= 1;  break;
					case "2" : TcpFlags    |= 1;  break;
				}
				switch (document.EditRuleEntryForm.tcpSyn.value) {
					case "1" : TcpFlagsSet |= 2;  break;
					case "2" : TcpFlags    |= 2;  break;
				}
				switch (document.EditRuleEntryForm.tcpRst.value) {
					case "1" : TcpFlagsSet |= 4;  break;
					case "2" : TcpFlags    |= 4;  break;
				}
				switch (document.EditRuleEntryForm.tcpPsh.value) {
					case "1" : TcpFlagsSet |= 8;  break;
					case "2" : TcpFlags    |= 8;  break;
				}
				switch (document.EditRuleEntryForm.tcpAck.value) {
					case "1" : TcpFlagsSet |= 16; break;
					case "2" : TcpFlags    |= 16; break;
				}
				switch (document.EditRuleEntryForm.tcpUrg.value) {
					case "1" : TcpFlagsSet |= 32; break;
					case "2" : TcpFlags    |= 32; break;
				}
				document.configForm.elements["system.network.ipTables.ruleSpec.TcpFlags"].value = TcpFlags;
				document.configForm.elements["system.network.ipTables.ruleSpec.TcpFlagsSet"].value = TcpFlagsSet;
				document.configForm.elements["system.network.ipTables.ruleSpec.TcpFlagsInvert"].value =
					document.EditRuleEntryForm.TcpFlagsInvert.checked? 1 : 0;
			}

			// if protocol == UDP
			if (document.configForm.elements["system.network.ipTables.ruleSpec.protocol"].value == "17") {
				// source port - last - invert
				document.configForm.elements["system.network.ipTables.ruleSpec.sPort"].value =
					document.EditRuleEntryForm.sPortUdp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.LastSPort"].value =
					document.EditRuleEntryForm.LastSPortUdp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.sPortInvert"].value =
					document.EditRuleEntryForm.sPortInvertUdp.checked? 1 : 0;
				// destination port - last - invert
				document.configForm.elements["system.network.ipTables.ruleSpec.dPort"].value =
					document.EditRuleEntryForm.dPortUdp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.LastDPort"].value =
					document.EditRuleEntryForm.LastDPortUdp.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.dPortInvert"].value =
					document.EditRuleEntryForm.dPortInvertUdp.checked? 1 : 0;
			}

			// ICMP - invert
			// if protocol == ICMP
			if (document.configForm.elements["system.network.ipTables.ruleSpec.protocol"].value == "58") {
				document.configForm.elements["system.network.ipTables.ruleSpec.ICMPType"].value =
					document.EditRuleEntryForm.ICMPType.value;
				document.configForm.elements["system.network.ipTables.ruleSpec.ICMPTypeInvert"].value =
					document.EditRuleEntryForm.ICMPTypeInvert.checked? 1 : 0;
			}

			// LOG level / Prefix / TCP options
			document.configForm.elements["system.network.ipTables.ruleSpec.LogLevel"].value = 
				document.EditRuleEntryForm.LogLevel.value;
			document.configForm.elements["system.network.ipTables.ruleSpec.LogPrefix"].value = 
				document.EditRuleEntryForm.LogPrefix.value;
			var LogTcpOptions = 0;
			LogTcpOptions |= document.EditRuleEntryForm.tcpSequence.checked? 1 : 0;
			LogTcpOptions |= document.EditRuleEntryForm.tcpOptions.checked?  2 : 0;
			LogTcpOptions |= document.EditRuleEntryForm.ipOptions.checked?   4 : 0;
			document.configForm.elements["system.network.ipTables.ruleSpec.LogTcpOptions"].value =
				LogTcpOptions;
		}

		function form_verification() {
			var form_location = document.EditRuleEntryForm;
			continue_verification = true;
			continue_verification = verify([
				[form_location.sIp, "Fill?,IPv6m"],
				[form_location.dIp, "Fill?,IPv6m"],
			]);

			// *********  LOG ***********	
			if (continue_verification == true && form_location.target.value == "LOG") {
				continue_verification = validate([form_location.LogPrefix, "Fill?,Space"]);
			}
		
			if (continue_verification == true) {
				switch (parseInt(form_location.protocol.value)) {
					case 255:
						continue_verification = validate([form_location.protocolNumber, "Fill,Num"]);
					break;
					case 6:
						continue_verification = verify([
							[form_location.sPortTcp,     "Fill?,Num"],
							[form_location.LastSPortTcp, "Fill?,Num"],
							[form_location.dPortTcp,     "Fill?,Num"],
							[form_location.LastSPortTcp, "Fill?,Num"]
						]);
					break;
					case 17:
						continue_verification = verify([
							[form_location.sPortUdp,     "Fill?,Num"],
							[form_location.LastSPortUdp, "Fill?,Num"],
							[form_location.dPortUdp,     "Fill?,Num"],
							[form_location.LastSPortUdp, "Fill?,Num"]
						]);
					break;
					case 0:
					case 58:
					break;
					default:
						continue_verification = false;
					break;
				}
			}
			/*var vinterfaces = interfaces.split(",");
			if (continue_verification == true && (chainName == "INPUT" || chainName == "FORWARD")) { 
				continue_verification = false;
				for(v in vinterfaces)
					if ( form_location.inInterface.value == vinterfaces[v] )
						continue_verification = true;
				if(continue_verification == false)
					alert('Invalid Input Interface');
			}
			if (continue_verification == true && (chainName == "OUTPUT" || chainName == "FORWARD")) { 
				continue_verification = false;
				for(v in vinterfaces)
					if ( form_location.outInterface.value == vinterfaces[v] )
						continue_verification = true;
				if(continue_verification == false)
					alert('Invalid Output Interface');
			}*/
		}
		
		function fill() {
			var i, ifs = ("," + interfaces).split(",");

			for (i=0; i<ifs.length; i++) {
				optInterfaces[i] = [ifs[i], ifs[i]];
			}

			document.EditRuleEntryForm.sIp.value = "<%get("system.network.ipTables.ruleSpec.sIp");%>";
			document.EditRuleEntryForm.sIpInvert.checked =
				<%get("system.network.ipTables.ruleSpec.sIpInvert");%>? true : false;

			document.EditRuleEntryForm.dIp.value = "<%get("system.network.ipTables.ruleSpec.dIp");%>";
			document.EditRuleEntryForm.dIpInvert.checked =
				<%get("system.network.ipTables.ruleSpec.dIpInvert");%>? true : false;

			document.EditRuleEntryForm.protocol.value = "<%get("system.network.ipTables.ruleSpec.protocol");%>";
			document.EditRuleEntryForm.protocolInvert.checked =
				<%get("system.network.ipTables.ruleSpec.protocolInvert");%>? true : false;
			document.EditRuleEntryForm.protocolNumber.value = "<%get("system.network.ipTables.ruleSpec.protocolNumber");%>";

			fillSelect(document.EditRuleEntryForm.target, optTarget);
			document.EditRuleEntryForm.target.value = "<%get("system.network.ipTables.ruleSpec.target");%>";

			fillSelect(document.EditRuleEntryForm.inInterface, optInterfaces);
			document.EditRuleEntryForm.inInterface.value = "<%get("system.network.ipTables.ruleSpec.inInterface");%>";
			document.EditRuleEntryForm.inInterfaceInvert.checked =
				<%get("system.network.ipTables.ruleSpec.inInterfaceInvert");%>? true : false;
			
			fillSelect(document.EditRuleEntryForm.outInterface, optInterfaces);
			document.EditRuleEntryForm.outInterface.value = "<%get("system.network.ipTables.ruleSpec.outInterface");%>";
			document.EditRuleEntryForm.outInterfaceInvert.checked =
				<%get("system.network.ipTables.ruleSpec.outInterfaceInvert");%>? true : false;

			document.EditRuleEntryForm.LogLevel.value = "<%get("system.network.ipTables.ruleSpec.LogLevel");%>";
			document.EditRuleEntryForm.LogPrefix.value = "<%get("system.network.ipTables.ruleSpec.LogPrefix");%>";
			
			var LogTcpOptions = <%get("system.network.ipTables.ruleSpec.LogTcpOptions");%>;
			document.EditRuleEntryForm.tcpSequence.checked =
				(LogTcpOptions & 1)? true : false;
			document.EditRuleEntryForm.tcpOptions.checked =
				(LogTcpOptions & 2)? true : false;
			document.EditRuleEntryForm.ipOptions.checked =
				(LogTcpOptions & 4)? true : false;
			
			document.EditRuleEntryForm.sPortTcp.value = "<%get("system.network.ipTables.ruleSpec.sPort");%>"
			document.EditRuleEntryForm.LastSPortTcp.value = "<%get("system.network.ipTables.ruleSpec.LastSPort");%>"
			document.EditRuleEntryForm.sPortInvertTcp.checked = 
				<%get("system.network.ipTables.ruleSpec.sPortInvert");%>? true : false;
			document.EditRuleEntryForm.dPortTcp.value = "<%get("system.network.ipTables.ruleSpec.dPort");%>"
			document.EditRuleEntryForm.LastDPortTcp.value = "<%get("system.network.ipTables.ruleSpec.LastDPort");%>"
			document.EditRuleEntryForm.dPortInvertTcp.checked = 
				<%get("system.network.ipTables.ruleSpec.dPortInvert");%>? true : false;
			
			var TcpFlags = <%get("system.network.ipTables.ruleSpec.TcpFlags");%>;
			var TcpFlagsSet = <%get("system.network.ipTables.ruleSpec.TcpFlagsSet");%>;
			var tcpFlags = ["tcpFin", "tcpSyn", "tcpRst", "tcpPsh", "tcpAck", "tcpUrg"];
			for (i=0; i<tcpFlags.length; i++) {
				document.EditRuleEntryForm.elements[tcpFlags[i]].value = (TcpFlagsSet & 1) + 2*(TcpFlags & 1);
				TcpFlags >>= 1; TcpFlagsSet >>= 1;
			}
			document.EditRuleEntryForm.TcpFlagsInvert.checked = 
				<%get("system.network.ipTables.ruleSpec.TcpFlagsInvert");%>? true : false;
			
			document.EditRuleEntryForm.sPortUdp.value = "<%get("system.network.ipTables.ruleSpec.sPort");%>"
			document.EditRuleEntryForm.LastSPortUdp.value = "<%get("system.network.ipTables.ruleSpec.LastSPort");%>"
			document.EditRuleEntryForm.sPortInvertUdp.checked = 
				<%get("system.network.ipTables.ruleSpec.sPortInvert");%>? true : false;
			document.EditRuleEntryForm.dPortUdp.value = "<%get("system.network.ipTables.ruleSpec.dPort");%>"
			document.EditRuleEntryForm.LastDPortUdp.value = "<%get("system.network.ipTables.ruleSpec.LastDPort");%>"
			document.EditRuleEntryForm.dPortInvertUdp.checked = 
				<%get("system.network.ipTables.ruleSpec.dPortInvert");%>? true : false;
			
			document.EditRuleEntryForm.RejectWith.value = "<%get("system.network.ipTables.ruleSpec.RejectWith");%>";
			document.EditRuleEntryForm.ICMPType.value = "<%get("system.network.ipTables.ruleSpec.ICMPType");%>";
			document.EditRuleEntryForm.ICMPTypeInvert.checked = 
				<%get("system.network.ipTables.ruleSpec.ICMPTypeInvert");%>? true : false
		}		

		function init() {
			//fill();
			if ((chainName != "OUTPUT") && (chainName != "FORWARD") && (chainName == "INPUT")) { 
				document.EditRuleEntryForm.outInterface.disabled = true;
				document.EditRuleEntryForm.outInterfaceInvert.disabled = true;
			}
			if ((chainName != "INPUT") && (chainName != "FORWARD") && (chainName == "OUTPUT")) { 
				document.EditRuleEntryForm.inInterface.disabled = true;
				document.EditRuleEntryForm.inInterfaceInvert.disabled = true;
			}

		}
	
		function submitRule() {
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
		<td height="30" colspan="5">
			<font class="label">Target</font>
		</td>
	</tr>
	<tr align="center" valign="middle"> 
		<td height="30" colspan="5">
			<select name="target" size="1" class="formText" onChange="showTarget();">
			</select>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td width="20%" height="35">
			<font class="label">Source IP</font>
		</td>
		<td width="72%" height="35" colspan="3">
			<input name="sIp" type="text" size="35" maxlength="50" class="formText">
		</td>
		<td width="18%" height="35">
			&nbsp;&nbsp;<input type="checkbox" name="sIpInvert">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td width="20%" height="30">
			<font class="label">Destination IP</font>
		</td>
		<td width="72%" height="30" colspan="3">
			<input name="dIp" type="text" size="35" maxlength="50" class="formText">
		</td>
		<td width="18%" height="30">
			&nbsp;&nbsp;<input type="checkbox" name="dIpInvert">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td width="20%" height="30">
			<font class="label">Protocol</font>
		</td>
		<td width="25%" height="30">
			<select name="protocol" size="1" class="formText" onChange="showProtocolLayer();">
				<option value="255">Numeric</option>
				<option value="0">All</option>
				<option value="6">TCP</option>
				<option value="17">UDP</option>
				<option value="58">ICMPv6</option>
			</select>
		</td>
		<td width="12%" height="30">&nbsp;</td>
		<td width="25%" height="30" align="center">
			&nbsp;&nbsp;<input type="checkbox" name="protocolInvert">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
		<td width="18%" height="30">&nbsp;</td>
	</tr>
	<tr align="left" valign="middle">
		<!-- This and the next field are shown only if user chose the following predefined Chains INPUT, FORWARD. 
		If user chose to add a rule to just a user defined chain, then this field is not asked.-->
		<td width="20%" height="30">
			<font class="label">Input Interface</font>
		</td>
		<td width="25%" height="30">
			<select name="inInterface" class="formText">
			</select>
		</td>
		<td height="30" colspan="3">
			&nbsp;&nbsp;<input type="checkbox" name="inInterfaceInvert">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
	<!-- This and the next field are shown only if user chose the following predefined chains: FORWARD, OUTPUT--> 
		<td width="20%" height="30">
			<font class="label">Output Interface</font>
		</td>
		<td width="25%" height="30">
			<select name="outInterface" class="formText">
			</select>
		</td>
		<td height="30" colspan="3">
			&nbsp;&nbsp;<input type="checkbox" name="outInterfaceInvert">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
</table>

<div id="Layer1" name="num" style="position:absolute; left:232px; top:184px; width:100px; height:25 z-index:1; visibility: visible;" align="center">
<table>
	<tr align="center" valign="bottom">
		<td width="180" height="25">
			<input name="protocolNumber" type="text" size="8" class="formText">
		</td>
	</tr>
</table>
</div>

<div id="Layer2" name="log" style="position:absolute; left:21px; top:310px; width:559px; height:200px; z-index:5; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" cellspacing="0" cellpadding="0">
	<tr align="center" valign="bottom"> 
		<td height="40" colspan="5">
			<font class="tabsTextBig">LOG Options Section</font>
		</td>
	</tr>
	<tr align="left" valign="bottom">
		<td width="18%" height="30">
			<font class="label">Log Level</font>
		</td>
		<td width="18%" height="30">
			<select name="LogLevel" size="1" class="formText">
				<option value="0">emergency</option>
				<option value="1">alert</option>
				<option value="2">critical</option>
				<option value="3">error</option>
				<option value="4">warning</option>
				<option value="5">notice</option>
				<option value="6">info</option>
				<option value="7">debug</option>
			</select>
		</td>
		<td width="18%" height="30" align="center">
			<font class="label">Log Prefix</font>
		</td>
		<td height="30" colspan="2">
			<input name="LogPrefix" type="text" size="15" maxlength="29" class="formText">
		</td>
	</tr>
	<tr align="center" valign="bottom"> 
		<td height="30" colspan="5">
			<table width="560" border="0" align="center" cellpadding="0" cellspacing="0">
				<tr align="center" valign="middle">
					<td height="30">
						<input type="checkbox" name="tcpSequence">
						&nbsp;&nbsp;&nbsp;<font class="label">TCP sequence</font>
					</td>
					<td height="30">
						<input type="checkbox" name="tcpOptions">
						&nbsp;&nbsp;&nbsp;<font class="label">TCP options</font>
					</td>
					<td height="30">
						<input type="checkbox" name="ipOptions">
						&nbsp;&nbsp;&nbsp;<font class="label">IP options</font>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</div>

<div id="Layer3" name="reject" style="position:absolute; left:21px; top:310px; width:559px; height:200px; z-index:5; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" cellspacing="0" cellpadding="0" align="left">
	<tr align="center" valign="bottom"> 
		<td height="40" colspan="4">
			<font class="tabsTextBig">REJECT Options Section</font>
		</td>
	</tr>
	<tr align="center" valign="bottom">
		<td height="30" width="100" align="center"></td> 
		<td height="30" width="140" align="center">
			<font class="label">Reject with</font>
		</td>
		<td height="30" width="200" align="center">
			<select name="RejectWith" size="1" class="formText">
				<option value="icmp6-no-route">icmp6-no-route</option>
				<option value="icmp6-adm-prohibited">icmp6-adm-prohibited</option>
				<option value="icmp6-addr-unreachable">icmp6-addr-unreachable</option>
				<option value="icmp6-port-unreachable">icmp6-port-unreachable</option>
			</select>
		</td>
		<td height="30" width="100" align="center"></td>
	</tr>
</table>
</div>

<div id="Layer4" name="tcp" style="position:absolute; left:21px; top:430px; width:559px; z-index:6; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" cellspacing="0" cellpadding="0">
	<tr align="center" valign="middle"> 
		<td height="40" colspan="6">
			<font class="tabsTextBig">TCP Options Section</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td width="20%" height="30">
			<font class="label">Source Port</font>
		</td>
		<td width="25%" height="30">
			<input name="sPortTcp" type="text" size="15" class="formText">
		</td>
		<td width="12%" height="30" align="center">
			<font class="label">to</font>
		</td>
		<td width="25%" height="30">
			<input name="LastSPortTcp" type="text" size="15" class="formText">
		</td>
		<td width="18%" height="30" colspan="2">
			&nbsp;&nbsp;<input type="checkbox" name="sPortInvertTcp">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td width="20%" height="30">
			<font class="label">Destination Port</font>
		</td>
		<td width="25%" height="30">
			<input name="dPortTcp" type="text" size="15" class="formText">
		</td>
		<td width="12%" height="30" align="center">
			<font class="label">to</font>
		</td>
		<td width="25%" height="30">
			<input name="LastDPortTcp" type="text" size="15" class="formText">
		</td>
		<td width="18%" height="30" colspan="2">
			&nbsp;&nbsp;<input type="checkbox" name="dPortInvertTcp">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
	<tr align="center" valign="middle">
		<td height="30" colspan="6">
			<font class="label">TCP Flags</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td height="30" colspan="6">
			<table width="100%" border="0" cellspacing="0" cellpadding="0">
				<tr valign="middle">
					<td height="30" align="right">
						<font class="label">SYN</font>&nbsp;&nbsp;&nbsp;&nbsp;
					</td>
					<td height="30" align="left">
						<select name="tcpSyn" size="1" class="formText">
							<option value="0">Any</option>
							<option value="1">Set</option>
							<option value="2">Unset</option>
						</select>
					</td>
					<td height="30" align="right">
						<font class="label">ACK</font>&nbsp;&nbsp;&nbsp;&nbsp;
					</td>
					<td height="30" align="left">
						<select name="tcpAck" size="1" class="formText">
							<option value="0">Any</option>
							<option value="1">Set</option>
							<option value="2">Unset</option>
						</select>
					</td>
					<td height="30" align="right">
						<font class="label">FIN</font>&nbsp;&nbsp;&nbsp;&nbsp;
					</td>
					<td height="30" align="left">
						<select name="tcpFin" size="1" class="formText">
							<option value="0">Any</option>
							<option value="1">Set</option>
							<option value="2">Unset</option>
						</select>
					</td>
				</tr>
				<tr valign="middle">
					<td height="30" align="right">
						<font class="label">RST</font>&nbsp;&nbsp;&nbsp;&nbsp;
					</td>
					<td height="30" align="left">
						<select name="tcpRst" size="1" class="formText">
							<option value="0">Any</option>
							<option value="1">Set</option>
							<option value="2">Unset</option>
						</select>
					</td>
					<td height="30" align="right">
						<font class="label">URG</font>&nbsp;&nbsp;&nbsp;&nbsp;
					</td>
					<td height="30" align="left">
						<select name="tcpUrg" size="1" class="formText">
							<option value="0">Any</option>
							<option value="1">Set</option>
							<option value="2">Unset</option>
						</select>
					</td>
					<td height="30" align="right">
						<font class="label">PSH</font>&nbsp;&nbsp;&nbsp;&nbsp;
					</td>
					<td height="30" align="left">
						<select name="tcpPsh" size="1" class="formText">
							<option value="0">Any</option>
							<option value="1">Set</option>
							<option value="2">Unset</option>
						</select>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr>
		<td height="30" colspan="6" align="center" valign="middle">
			&nbsp;&nbsp;<input type="checkbox" name="TcpFlagsInvert">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
</table>
</div>

<div id="Layer5" name="udp" style="position:absolute; left:21px; top:430px; width:559px; z-index:6; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" cellspacing="0" cellpadding="0">
	<tr align="center" valign="middle"> 
		<td height="40" colspan="6">
			<font class="tabsTextBig">UDP Options Section</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td width="20%" height="30">
			<font class="label">Source Port</font>
		</td>
		<td width="25%" height="30">
			<input name="sPortUdp" type="text" size="15" class="formText">
		</td>
		<td width="12%" height="30" align="center">
			<font class="label">to</font>
		</td>
		<td width="25%" height="30">
			<input name="LastSPortUdp" type="text" size="15" class="formText">
		</td>
		<td width="18%" height="30" colspan="2">
			&nbsp;&nbsp;<input type="checkbox" name="sPortInvertUdp">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td width="20%" height="30">
			<font class="label">Destination Port</font>
		</td>
		<td width="25%" height="30">
			<input name="dPortUdp" type="text" size="15" class="formText">
		</td>
		<td width="12%" height="30" align="center">
			<font class="label">to</font>
		</td>
		<td width="25%" height="30">
			<input name="LastDPortUdp" type="text" size="15" class="formText">
		</td>
		<td width="18%" height="30" colspan="2">
			&nbsp;&nbsp;<input type="checkbox" name="dPortInvertUdp">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
</table>
</div>

<div id="Layer6" name="icmp" style="position:absolute; left:21px; top:430px; width:540px; z-index:6; visibility: hidden;" class="bgColor12" align="left">
<table width="540" border="0" align="left" cellpadding="0" cellspacing="3">
	<tr align="center" valign="bottom">
		<td height="40">
			<font  class="tabsTextBig">ICMPv6 Options Section</font>
		</td>
	</tr>
	<tr align="center" valign="bottom">
		<td height="25">
			<font class="label">ICMPv6 Type</font>
		</td>
	</tr>
	<tr align="center" valign="bottom">
		<td height="25">
			<select name="ICMPType" size="1" class="formText">
				<option value="destination-unreachable">destination-unreachable</option>
				<option value="no-route">no-route</option>
				<option value="communication-prohibited">communication-prohibited</option>
				<option value="address-unreachable">address-unreachable</option>
				<option value="port-unreachable">port-unreachable</option>
				<option value="packet-too-big">packet-too-big</option>
				<option value="ttl-exceeded">ttl-exceeded</option>
				<option value="ttl-zero-during-transit">ttl-zero-during-transit</option>
				<option value="ttl-zero-during-reassembly">ttl-zero-during-reassembly</option>
				<option value="parameter-problem">parameter-problem</option>
				<option value="bad-header">bad-header</option>
				<option value="unknown-header-type">unknown-header-type</option>
				<option value="unknown-option">unknown-option</option>
				<option value="echo-request">echo-request</option>
				<option value="echo-reply">echo-reply</option>
				<option value="router-solicitation">router-solicitation</option>
				<option value="router-advertisement">router-advertisement</option>
				<option value="neighbour-solicitation">neighbour-solicitation</option>
				<option value="neighbour-advertisement">neighbour-advertisement</option>
				<option value="redirect">redirect</option>
			</select>
		</td>
	</tr>
	<tr align="center" valign="bottom">
		<td height="25">
			&nbsp;&nbsp;<input type="checkbox" name="ICMPTypeInvert">
			&nbsp;&nbsp;<font class="label">Inverted</font>
		</td>
	</tr>
</table>
</div>
</form>

<script>
	fill();
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
