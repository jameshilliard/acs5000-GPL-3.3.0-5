<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Other Port Settings</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">     

	<%
		var protocol = getVal("system.ports.physP.general.protocol");
		var pmsess   = getVal("system.ports.physP.general.pmsessions");
		var ONS      = (getVal('_familymodel_') == "ONS");
		var showHost = !(protocol == 81 || protocol == 86 || protocol == 87 || protocol == 89);
		ReqNum = getRequestNumber();
		var IPv6     = getVal('_IPv6_');
		var input_size      = 15;
		var input_maxlength = 15;
		if (IPv6) {
			input_size      = 35;
			input_maxlength = 50;
		}
	%>

	var protocol = <%write(protocol);%>;
	var pmsessions = <%write(pmsess);%>;

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	top.currentPage = self;
	top.helpPage = 'helpPages/portsPhysOtherSettHelp.asp';

	function showEnabled() {
		hide('document','Layer1');
		hide('document','Layer2');
		show('document','Layer3');
		hide('document','Layer4');
		hide('document','Layer5');
		switch (protocol) {
			case 81:
			case 89:
				show('document','Layer2');
			case 87:
			case 86: 
				if (top.family_model != "AVCS") {
					show('document','Layer1');
					show('document','Layer4');
				}
			break;
			case 79:
			case 80:
			case 83:
			case 67:
				show('document','Layer4');
				show('document','Layer5');
			break;
			case 50:
			case 69:
			case 72:
			case 76:
				hide('document','Layer0');
				show('document','Layer4');
			break;
			case 105:
				hide('document','Layer0');
				hide('document','Layer3');
				//if ((pmsessions == '1') || (pmsessions == '3')) {
				//	show('document','Layer2');
				//}
			break;
			default :  
				show('document','Layer4');
			break;
		}
	}

	function init() {
		var i;
		var terminalType = "<%get("system.ports.physP.other.terminalType");%>";
		var optTerminalType = [
			["aixterm",    "aixterm"   ],
			["ansi",       "ansi"      ],
			["at386",      "at386"     ],
			["att6386",    "att6386"   ],
			["ibm3151",    "ibm3151"   ],
			["linux",      "linux"     ],
			["linux-lat",  "linux-lat" ],
			["scoansi",    "scoansi"   ],
			["sun",        "sun"       ],
			["tandem6530", "tandem6530"],
			["vt52",       "vt52"      ],
			["vt100",      "vt100"     ],
			["vt102",      "vt102"     ],
			["vt220",      "vt220"     ],
			["vt320",      "vt320"     ],
			["xterm",      "xterm"     ],
			[terminalType, "**"        ]
		];
		for (i=0; i<optTerminalType.length; i++) {
			if (terminalType == optTerminalType[i][0]) {
				break;
			}
		}
		if (i != optTerminalType.length) {
			optTerminalType.pop();
		}
		if (top.family_model == "ACS") {
			optTerminalType.splice(9, 1);
		}
		document.OthersForm.socketPort.value    = <%get("system.ports.physP.other.socketPort");%>;
		document.OthersForm.portIpAlias.value   = "<%get("system.ports.physP.other.portIpAlias");%>";
<%if (IPv6) {
write('\
		document.OthersForm.portIp6Alias.value   = "' + getVal("system.ports.physP.other.portIp6Alias") + '";');
}%>
		document.OthersForm.winEms.value        = <%get("system.ports.physP.other.winEms");%>;
		if (top.family_model != "AVCS") {
			document.OthersForm.winEms.checked = parseInt(document.OthersForm.winEms.value)? true : false
		}
		document.OthersForm.SSHexit.value       = "<%get("system.ports.physP.other.SSHexit");%>";
		document.OthersForm.authBio.value       = "<%get("system.ports.physP.other.authBio");%>";
		document.OthersForm.tcpKeepAlive.value  = <%get("system.ports.physP.other.tcpKeepAlive");%>;
		document.OthersForm.tcpIdleTmo.value    = <%get("system.ports.physP.other.tcpIdleTmo");%>;
		document.OthersForm.stty.value          = "<%get("system.ports.physP.other.stty");%>";
		document.OthersForm.breakInterval.value = <%get("system.ports.physP.other.breakInterval");%>;
		/*document.OthersForm.breakSequence.value = "<%get("system.ports.physP.other.breakSequence");%>";*/
		/*document.OthersForm.loginBanner.value   = "<%get("system.ports.physP.other.loginBanner");%>");*/
		document.OthersForm.host.value          = "<%get("system.ports.physP.other.host");%>";
		fillSelect(document.OthersForm.terminalType, optTerminalType);
		document.OthersForm.terminalType.value  = terminalType;
		/*document.OthersForm.initChat.value      = "<%get("system.ports.physP.other.initChat");%>";*/
		document.OthersForm.pppOpt.value        = "<%get("system.ports.physP.other.pppOpt");%>";
		showEnabled();
	}

	function checkSConfHotKey (element_for_verification)
	{
		var chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; // all roman alphabet letters
		if (element_for_verification.value.charAt(0) != '^' || 
		    element_for_verification.value.charAt(1) == '' || 
		    chars.indexOf(element_for_verification.value.charAt(1)) == -1) {
			continue_verification = false;
			alert ('Invalid hotkey. Format should be a caret (^) and a letter.');
			element_for_verification.focus();
			element_for_verification.select();
		} else 
			continue_verification = true;  
	}	

	function form_verification () {
		var form_location = document.OthersForm;
		continue_verification = true;
		continue_verification = verify([
			[form_location.socketPort,    "Fill?,Num"     ],
			[form_location.tcpKeepAlive,  "Fill?,Num"     ],
			[form_location.tcpIdleTmo,    "Fill?,Num"     ],
			[form_location.breakInterval, "Fill?,Num"     ],
			[form_location.loginBanner,   "Length",  [299]],
			[form_location.portIpAlias,   "Fill?,IPv4"    ],
<%if (IPv6) {
write('\
			[form_location.portIp6Alias,  "Fill?,IPv6"    ],');
}%>
		]);
		if (continue_verification == true && (protocol == 69 || protocol == 72 || protocol == 50 || protocol == 76 || protocol == 73)) {
			continue_verification = verify([
				[form_location.host,     "Fill,IP"    ],
				[form_location.pppOpt,   "Fill?,Space"],
			]);
		}
		if (continue_verification == true && top.family_model == "ONS" && (protocol == 81 || protocol == 89)) {
			continue_verification = verify([
				[form_location.SSHexit, "Fill?,CtrlKey"],
			]);
		}
		if (continue_verification == true && (protocol == 79 || protocol == 80 || protocol == 83 || protocol == 67)) {
			continue_verification = verify([
				[form_location.initChat, "Length", [299]],
			]);
		}
		if (continue_verification == true && form_location.sconf.value != '') {
			checkSConfHotKey(form_location.sconf);
		}
		return continue_verification;
	}
	  
	function submit() {
		if (form_verification()) {
			moveData(document.configForm, document.OthersForm);
			if (document.configForm.elements[0].value != 0) {
				document.configForm.urlOk.value="/normal/ports/physPorts.asp";
			}
			setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = null;
		}
	}

	</script>
</head>

<body class="bodyMiniWizard">
<form name="OthersForm" method="post" action=" " onSubmit="return false;">
<table width="100%" border="0" cellspacing="0" cellpadding="0" align="center">
	<tr>
		<td>
			<table width="500" border="0" align="center" cellpadding="0" cellspacing="8">
				<tr>
					<td>&nbsp;</td>
				</tr>
				<tr>
					<td align="left" width="110">
						<font class="label">TCP Port</font>
					</td>
					<td align="left">
						<input name="socketPort" class="formText" type="text" size="15">
					</td>
				</tr>
			</table>
			<div id="Layer0" style="visibility:visible;" align="center">
			<table width="500" border="0" align="center" cellpadding="0" cellspacing="8">
				<tr>
					<td align="left" width="110">
						<font class="label">Port IP Alias</font>
					</td>
					<td align="left"><font>
						<input name="portIpAlias" class="formText" type="text"  maxlength="15" size="15">
					</td>
				</tr>
<%if (IPv6) {
write('\
				<tr>
					<td align="left" width="110">
						<font class="label">Port IPv6 Alias</font>
					</td>
					<td align="left"><font>
						<input name="portIp6Alias" class="formText" type="text"  maxlength="50" size="35">
					</td>
				</tr>');
}%>
			</table>
			</div>
		</td>
	</tr>
</table>
<div id="Layer1" style="visibility:visible;" align="center">
<table width="100%" border="0" cellspacing="0" cellpadding="0" align="center">
	<tr>
		<td>
			<table width="500" border="0" align="center" cellpadding="0" cellspacing="8">
				<tr>
					<td align="left" colspan="2">
						<input type="checkbox" name="winEms" onClick="this.value = this.checked? 1 : 0">&nbsp;&nbsp;
						<font class="label">Windows EMS</font>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
						<font class="label">Serial Configuration Mode Key</font>&nbsp;
						<input type="text" name="sconf" class="formText" size="2" maxlength="2" value="<%get("system.ports.physP.other.sconf");%>">
					</td>
					<td align="left">&nbsp;</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</div>
<!-- For socket_ssh only  -->
<div id="Layer2" style="visibility:visible;" align="center">
<table width="100%" border="0" cellspacing="0" cellpadding="0" align="center">
	<tr>
		<td>
<% if (ONS) {
write('\
			<table width="500" border="0" align="center" cellpadding="8" cellspacing="0">
				<tr>
					<td align="left" valign="middle" height="35" width="110">
						<font class="label">SSH Exit Key</font>
					</td>
					<td align="left" valign="middle" height="35">
						<input type="text" name="SSHexit" class="formText" size="2" maxlength="2">
						<input type="hidden" name="authBio">
					</td>
				</tr>
			</table>');
} else {
write('\
			<input type="hidden" name="SSHexit">
			<input type="hidden" name="authBio">');
}%>
		</td>
	</tr>
</table>
</div>
<!-- For all  -->
<div id="Layer3" style="visibility:visible;" align="center">
<table width="100%" border="0" cellspacing="0" cellpadding="0" align="center">
	<tr>
		<td>
			<table width="500" border="0" align="center" cellpadding="0" cellspacing="8">
				<tr>
					<td align="left" width="110" height="35" valign="middle">
						<font class="label">TCP Keep-alive<br> Interval(ms)</font>
					</td>
					<td align="left" height="35" valign="middle">
						<input name="tcpKeepAlive" type="text" size="12" class="formText">
					</td>
					<td align="right" height="35" valign="middle">
						<font class="label">Idle Timeout(min)</font>
					</td>
					<td align="left" height="35" valign="middle">
						<input name="tcpIdleTmo" type="text" size="12" class="formText">
					</td>
				</tr>
				<tr>
					<td align="left" width="110" height="35" valign="middle">
						<font class="label">STTY Options</font>
					</td>
					<td align="left" colspan="3" height="35" valign="middle">
						<input name="stty" type="text" size="35" maxlength="64" class="formText">
					</td>
				</tr>
				<tr>
					<td align="left" width="110" height="35" valign="middle">
						<font class="label">Break Interval</font>
					</td>
					<td align="left" height="35" valign="middle">
						<input name="breakInterval" type="text" size="12" class="formText">
					</td>
					<td align="right" height="35" valign="middle">
						<font class="label">Break Sequence</font>
					</td>
					<td align="left" height="35" valign="middle">
						<input name="breakSequence" type="text" size="12" class="formText" value="<%get('system.ports.physP.other.breakSequence');%>">
					</td>
				</tr>
				<tr>
					<td align="left" width="110" height="35" valign="middle">
						<font class="label">Login Banner</font>
					</td>
					<td align="left" colspan="3" height="35" valign="middle">
<!--[LMT] CyBTS Bug 3360 - Please, do not alter the get(...) function position in relation to TEXTAREA below. -->
						<textarea name="loginBanner" cols="35" rows="3" class="formText"><%get('system.ports.physP.other.loginBanner');%></textarea>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</div>
<!-- Show this Layer only for Terminal Server Profile-->
<div id="Layer4" style="visibility:visible;" align="center">
<table width="100%" border="0" cellspacing="0" cellpadding="0" align="center">
	<tr>
		<td colspan="4">&nbsp;</td>
	</tr>
	<tr>
		<td>
			<table width="500" border="0" align="center" cellpadding="0" cellspacing="8">
				<tr>
<% if (showHost) {
write('\
					<td align="left" width="110">
						<font class="label">Host to Connect</font>
					</td>
					<td align="left">
						<input name="host" class="formText" type="text" maxlength="' + input_maxlength + '" size="' + input_size + '">
					</td>');
} else {
write('\
					<td colspan="2">
						<input type="hidden" name="host" value="">
					</td>');
}
%>
				</tr>
				<tr>
					<td align="left" width="110">
						<font class="label">Terminal Type</font>
					</td>
					<td align="left">
						<select name="terminalType" class="formText">
						</select>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</div>
<!-- Show this Layer only for Dial in Profile -->
<div id="Layer5" style="visibility:visible;" align="center">
<table width="100%" border="0" cellspacing="0" cellpadding="0" align="center">
	<tr>
		<td>
			<table width="500" border="0" align="center" cellpadding="0" cellspacing="8">
				<tr>
					<td align="left" width="110">
						<font class="label">Modem Initialization</font>
					</td>
					<td align="left" colspan="3">
						<textarea name="initChat" cols="35" rows="2" class="formText"><%get("system.ports.physP.other.initChat");%></textarea>
					</td>
				</tr>
				<tr>
					<td align="left" width="100" height="40" valign="middle">
						<font class="label">PPP Options</font>
					</td>
					<td align="left" colspan="3">
						<input name="pppOpt" type="text" maxlength="299" size="30">
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</form>
</div>

<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/ports/physPorts.asp">
	<input type="hidden" name="urlError" value="/normal/ports/physPorts.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name="system.ports.physP.action" value="2">
	<input type="hidden" name="system.ports.physP.other.socketPort" value=""> 
	<input type="hidden" name="system.ports.physP.other.portIpAlias" value="">
<%if (IPv6) {
write('\
	<input type="hidden" name="system.ports.physP.other.portIp6Alias" value="">');
}%>
	<input type="hidden" name="system.ports.physP.other.winEms" value=""> 
	<input type="hidden" name="system.ports.physP.other.SSHexit" value="">
	<input type="hidden" name="system.ports.physP.other.sconf" value="">
	<input type="hidden" name="system.ports.physP.other.authBio" value="">
	<input type="hidden" name="system.ports.physP.other.tcpKeepAlive" value=""> 
	<input type="hidden" name="system.ports.physP.other.tcpIdleTmo" value=""> 
	<input type="hidden" name="system.ports.physP.other.stty" value=""> 
	<input type="hidden" name="system.ports.physP.other.breakInterval" value=""> 
	<input type="hidden" name="system.ports.physP.other.breakSequence" value=""> 
	<input type="hidden" name="system.ports.physP.other.loginBanner" value="">
	<input type="hidden" name="system.ports.physP.other.host" value=""> 
	<input type="hidden" name="system.ports.physP.other.initChat" value=""> 
	<input type="hidden" name="system.ports.physP.other.pppOpt" value="">
	<input type="hidden" name="system.ports.physP.other.terminalType" value=""> 
</form>
<script type="text/javascript">
init();
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html> 
