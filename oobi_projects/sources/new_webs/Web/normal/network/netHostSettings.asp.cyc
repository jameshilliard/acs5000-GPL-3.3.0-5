<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   	<script type="text/javascript">
	<%
		ReqNum = getRequestNumber();
		var iptab=getQueryVal("iptab","0");
		var sid=getQueryVal("SSID","0");
		var family_model = getVal('_familymodel_');
		var acs5k = getVal('_acs5k_');
		var IPv6 = getVal('_IPv6_');
		var BONDING_ENABLED = "no";
		var input_size = "15";
		var input_maxlength = "15";
		var Url = "netHostSettings";
		if (IPv6) {
			Url = "netHost";
			input_size = "37";
			input_maxlength = "50";
		}
		if ((family_model == "ACS") && (acs5k != "1"))
			BONDING_ENABLED = "yes";
	%>
<%if (IPv6) {
write('\
	var optDHCPv6 = [[0,"none"],[1,"DNS"],[2,"Domain"],[3,"DNS-Domain"]];
	var optIPmode = [[1,"IPv4"],[2,"IPv6"],[3,"Dual-Stack"]];
	var optIPv6method = [[0,"Stateless only"],[1,"Static"],[2,"DHCP"]];
	var iptab = ' + iptab + ';');
}%>	
	var ref = (<%write(IPv6);%>)? top : parent;
	
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	
	ref.currentPage = self;
	ref.helpPage = 'helpPages/netHostSettingsHelp.asp';
	
	function fill() {

		document.mainForm.dhcp.checked = (<%get("system.netSettings.dhcp");%>)? true : false;
		document.mainForm.dhcp.value = document.mainForm.dhcp.checked? 2 : 0;

		document.mainForm.hostName.value = '<%get("system.netSettings.hostName");%>';
		document.mainForm.consBanner.value = '<%get("system.netSettings.consBanner");%>';

		document.mainForm.ipAddress1.value = '<%get("system.netSettings.ipAddress1");%>';
		document.mainForm.netMask1.value = '<%get("system.netSettings.netMask1");%>';
		
		document.mainForm.ipAddress2.value = '<%get("system.netSettings.ipAddress2");%>';
		document.mainForm.netMask2.value = '<%get("system.netSettings.netMask2");%>';

		var mtu = <%get("system.netSettings.mtu");%>;
		document.mainForm.mtu.value = mtu? mtu : "";
		
		document.mainForm.dns1.value = "<%get("system.netSettings.dns1");%>";
		document.mainForm.dns2.value = "<%get("system.netSettings.dns2");%>";
		
		document.mainForm.domain.value = '<%get("system.netSettings.domain");%>';
		document.mainForm.gateway.value = '<%get("system.netSettings.gateway");%>';

<%if (BONDING_ENABLED == "yes") {
write('
		document.mainForm.bondenabled.checked = '); 
		if (getVal("system.netSettings.bondenabled") == 0)
			write('false;');
		else
			write('true;');
write('
		document.mainForm.bondenabled.value = document.mainForm.bondenabled.checked? 1 : 0;
');
write('
		document.mainForm.bondmiimon.value = ');get("system.netSettings.bondmiimon");write(';');
write('
		document.mainForm.bondupdelay.value = ');get("system.netSettings.bondupdelay");write(';');
}%>
<%if (IPv6) {
	var enable4 = getVal("system.netSettings.enableIPv4");
	var enable6 = getVal("system.netSettings.enableIPv6");
write('\
		document.mainForm.enableIPv4.value = ' + enable4 + ';
		document.mainForm.enableIPv6.value = ' + enable6 + ';
		fillSelect(document.mainForm.IPmode, optIPmode);
		document.mainForm.IPmode.selectedIndex = ' + enable4 + ' + 2*' + enable6 + ' -1;
		fillSelect(document.mainForm.DHCPv6Opts, optDHCPv6);
		document.mainForm.DHCPv6Opts.selectedIndex = ');get("system.netSettings.DHCPv6Opts");write(';
		fillSelect(document.mainForm.IPv6method, optIPv6method);
		document.mainForm.IPv6method.selectedIndex = ');get("system.netSettings.IPv6method");write(';
		var eth_ip6 = "');get("system.netSettings.ethIp6");write('";
		document.mainForm.ethIp6.value = eth_ip6;
		var eth_prefix6 = ');get("system.netSettings.ethPrefix6");write(';
		document.mainForm.ethPrefix6.value = eth_prefix6;
		if (eth_ip6.length > 0 && eth_prefix6 > 0) {
			eth_ip6 += "/" + eth_prefix6;
		}
		document.mainForm.ethIpPrefix6.value = eth_ip6;');
}%>
	}
	
	function init() {
		if (1 == ref.menuReady && 1 == ref.topMenuReady && 1 == ref.controlsReady) {
			if (checkLeftMenu('/normal/network/networkMenu.asp') == 0) {
				ref.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			setUnsavedImage(ref.controls.document, <%get("_changesLed_");%>);
			ref.topMenu.selectItem(ref.menu.topItem);
			ref.menu.selectItem("ETH");
			showDhcp();
			show2('document', 'main');
		} else
			setTimeout('init()', 200);
	}

<%if (BONDING_ENABLED == "yes")
	write('\
	function showBonding() {
		document.mainForm.bondenabled.value = document.mainForm.bondenabled.checked? 1 : 0;
		if (document.mainForm.bondenabled.checked == true)
			show2("document", "Bonding2");
		else
			hide2("document", "Bonding2");
	}');%>

	function showDhcp() {
<%
var divs = "Ethernet";
if (IPv6) {
write('\
		if (iptab == 0) {
			show2("document", "General,Domain,DNS");
			hide2("document", "dhcp4,dhcp6,Ethernet,Ethernet6");
			document.mainForm.domain.disabled = false;
			document.mainForm.dns1.disabled   = false;
			document.mainForm.dns2.disabled   = false;
			if (parseInt(getIPmode()) & 1) {
				if (document.mainForm.dhcp.checked) {
					document.mainForm.domain.disabled = true;
					document.mainForm.dns1.disabled   = true;
					document.mainForm.dns2.disabled   = true;
				}
			}
			if (parseInt(getIPmode()) & 2) {
				if (parseInt(document.mainForm.DHCPv6Opts.value) & 1) {
					document.mainForm.dns1.disabled   = true;
					document.mainForm.dns2.disabled   = true;
				}
				if (parseInt(document.mainForm.DHCPv6Opts.value) & 2) {
					document.mainForm.domain.disabled = true;
				}
			}');
	if (BONDING_ENABLED == "yes")
	write('
			show2("document", "Bonding");
			showBonding();');
	write('	} else if (iptab == 1) {');
} else {
	divs = divs + ",DNS,Domain,Gateway";
	if (BONDING_ENABLED == "yes")
		divs = divs + ",Bonding";
}%>
<%
var divs2 = "dhcp4";
if (!IPv6) {
	divs2 = divs2 + ",General";
}
write('\
			show2("document", "' + divs2 + '");
');
%>			
			document.mainForm.dhcp.value = document.mainForm.dhcp.checked? 2 : 0;
			if (document.mainForm.dhcp.checked == true) {
				hide2("document", "<%write(divs);%>");
			} else {
				show2("document", "<%write(divs);%>");
<%if (!IPv6 && BONDING_ENABLED == "yes")
write('\
				showBonding();');
%>
			}
<%if (IPv6) {
write('\
			hide2("document", "dhcp6,Ethernet6");
		} else {
			show2("document", "dhcp6,Ethernet6");
			hide2("document", "dhcp4,Ethernet");
			document.mainForm.ethIpPrefix6.disabled = (document.mainForm.IPv6method.value != "1");
		}
	}
	
	function updateIP () {
		var IPmode = document.mainForm.IPmode.value;
		document.mainForm.enableIPv4.value = IPmode % 2;
		document.mainForm.enableIPv6.value = IPmode / 2;
		setIPmode(IPmode);
		alertIPmodeChange();
		parent.document.getElementById("tabs").src = "netHostTab.asp?SSID=' + sid + '&iptab=" + iptab;
	}
	
	function updateEth () {
		var eth6 = document.mainForm.ethIpPrefix6.value.split(/\\//);
		document.mainForm.ethIp6.value = eth6[0];
		document.mainForm.ethPrefix6.value = (eth6.length>1)?eth6[1]:0;
	}');
} else {
write('\
	}');
}%>
	
	function form_verification() {

		form_location = document.mainForm;
		continue_verification = true;
		
<%
if (IPv6) {
write('\
		if (iptab == 0) {');
}%>
			continue_verification = verify([
				[form_location.hostName, "Fill?,Space"],
			]);
<%if (IPv6) {
write('\
			if (continue_verification && !form_location.dhcp.checked && !(parseInt(form_location.DHCPv6Opts.value)==1)) {
				continue_verification = verify([
					[form_location.dns1,     "Fill?,IP"   ],
					[form_location.dns2,     "Fill?,IP"   ],
				]);
			}
			if (continue_verification && !form_location.dhcp.checked && !(parseInt(form_location.DHCPv6Opts.value)==2)) {
				continue_verification = verify([
					[form_location.domain,   "Fill?,Space"],
				]);
			}
		} else if (iptab == 1) {');
}%>
			if (document.mainForm.dhcp.checked != true) {
				if (continue_verification == true) {
					continue_verification = verify([
						[form_location.ipAddress1, "Fill,IPv4"                ],
						[form_location.netMask1,   "Fill,NetMask"             ],
						[form_location.ipAddress2, "Fill?,IPv4"               ],
						[form_location.netMask2,   "Fill?,NetMask"            ],
						[form_location.mtu,        "Fill?,Num,Bound", [1,1500]],
<%if (!IPv6) {
write('\
						[form_location.dns1,       "Fill?,IPv4"               ],
						[form_location.dns2,       "Fill?,IPv4"               ],
						[form_location.gateway,    "Fill?,IP"                 ],
						[form_location.domain,     "Fill?,Space"              ],
					]);
				}
			}');
} else {
write('\
					]);
				}
			}
		} else {
			if (form_location.IPv6method.options[form_location.IPv6method.selectedIndex].text == "Static") {
				continue_verification = verify([
					[form_location.ethIpPrefix6, "Fill,IPv6P"],
				]);
			} else {
				form_location.ethIp6.value = "";
				form_location.ethPrefix6.value = "";
			}
		}');
}%>
		return continue_verification;
	}
	
	function submit() { 

		if (form_verification()) {
			moveData(document.configForm, document.mainForm);			
			document.configForm.submit();
			ref.mainReady = null;
		}
	}

	</script>

</head>

<body onload="init();" class="bodyForLayers">
<form id="main" name="mainForm" action="" onSubmit="return false;" style="display:none;">

	<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%">
		<tr valign="top">
			<td>
				<div id="dhcp4" style="display:none;" align="center">  	  		 
				<table border="0" cellpadding="0" cellspacing="10" align="center" width="400">
					<tr valign="middle" align="center">
						<td>
							<input type="checkbox" name="dhcp" value="2" onclick="showDhcp()">
							<font class="label">DHCP</font>
						</td>
					</tr>            
				</table>
				</div>
<%if (IPv6) {
write('\
				<div id="dhcp6" style="display:none;" align="center">  	  		 
				<table border="0" cellpadding="0" cellspacing="10" align="center" width="400">
					<tr valign="middle" align="center">
						<td>
							<font class="label">DHCPv6</font>
							<select name="DHCPv6Opts" onchange="showDhcp()"></select>
						</td>
					</tr>            
				</table>
				</div>');
}%>
			</td>
		</tr>
	</table>

	<div id="General" style="display:none" align="center">
	<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%">
		<tr valign="top">
			<td>
<%if (IPv6) {
write('\
				<table border="0" cellpadding="0" cellspacing="5" align="center"  width="400">
					<tr>
						<td valign="middle" align="center" colspan="2">
							<font class="label">Mode</font>
							<select name="IPmode" onchange="updateIP()"></select>
							<input type="hidden" name="enableIPv4">
							<input type="hidden" name="enableIPv6">
						</td>
					</tr>
				</table>');
}%>
				<table border="0" cellpadding="0" cellspacing="5" align="center"  width="400">
					<tr>
						<td valign="bottom" width="200"><font class="label">Host Name</font></td>
						<td valign="bottom" width="200"><font class="label">Console Banner</font></td>
					</tr>
					<tr>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="30" name="hostName" size="15">
						</td>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="49" name="consBanner" size="15">
						</td>
					</tr>
				</table>
			</td>
		</tr>
	</table>
	</div>

	<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%">
		<tr valign="middle">
			<td>
				<div id="Ethernet" style="display:none" align="center">
				<table border="0" cellpadding="0" cellspacing="5" align="center" width="400">
					<tr>
						<td valign="middle" colspan="2" height="30">
							<font class="tabsTextBig">Ethernet Port</font>
						</td>		
					</tr>
					<tr>
						<td valign="bottom" width="200"><font class="label">Primary Address</font></td>
						<td valign="bottom" width="200"><font class="label"><%getLabel("system.netSettings.netMask1");%></font></td>
					</tr>
					<tr>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="15" name="ipAddress1" size="15">
						</td>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="15" name="netMask1" size="15">&nbsp;&nbsp;&nbsp;
						</td>
					</tr>
					<tr>
						<td valign="bottom" width="200"><font class="label">Secondary Address</font></td>
						<td valign="bottom" width="200"><font class="label"><%getLabel("system.netSettings.netMask2");%></font></td>
					</tr>
					<tr>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="15" name="ipAddress2" size="15">
						</td>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="15" name="netMask2" size="15">&nbsp;&nbsp;&nbsp;
						</td>
					</tr>
					<tr>
						<td valign="bottom" width="200"><font class="label">MTU</font></td>
						<td valign="bottom" width="200">&nbsp;</td>
					</tr>
					<tr>
						<%var mtu=getVal("system.netSettings.mtu"); %>
						<td valign="top" width="200">
							<input class="formText" type="text" name="mtu" size="15">
						</td>
						<td valign="top" width="200">&nbsp;</td>
					</tr>
				</table>
				</div>
<%if (IPv6) {
write('\
				<div id="Ethernet6" style="display:none" align="center">
				<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%">
					<tr valign="middle">
						<td>		 
							<table border="0" cellpadding="0" cellspacing="5" align="center" width="400">
								<tr>
									<td valign="middle" colspan="2" height="30">
										<font class="tabsTextBig">Ethernet Port</font>
									</td>		
								</tr>
								<tr>
									<td colspan="2" valign="middle" align="left">
										<font class="label">Method</font>
										<select name="IPv6method" onchange="showDhcp();"></select>
									</td>
								</tr>            
								<tr>
									<td colspan="2" valign="bottom"><font class="label">Static Address</font></td>
								</tr>
								<tr>
									<td colspan="2" valign="top">
										<input class="formText" type="text" name="ethIpPrefix6" size="40" maxlength="50" onchange="updateEth()">
										<input type="hidden" name="ethIp6">
										<input type="hidden" name="ethPrefix6">
									</td>
								</tr>
							</table>
						</td>
					</tr>
				</table>
				</div>');
}%>
				<div id="DNS" style="display:none" align="center">
				<table border="0" cellpadding="0" cellspacing="5" align="center" width="400">
					<tr>
						<td height="30" valign="middle" colspan="2"><font class="tabsTextBig">DNS Service</font></td>		
					</tr>
<%if (IPv6) {
write('\
					<tr>
						<td valign="bottom" colspan="2"><font class="label">Primary DNS Server</font></td>
					</tr>
					<tr>
						<td valign="top" colspan="2">
							<input class="formText" type="text" maxlength="50" name="dns1" size="37">
						</td>
					</tr>
					<tr>
						<td valign="bottom" colspan="2"><font class="label">Secondary DNS Server</font></td>
					</tr>
					<tr>
						<td valign="top" colspan="2">
							<input class="formText" type="text" maxlength="50" name="dns2" size="37">&nbsp;&nbsp;&nbsp;
						</td>
					</tr>');
} else {
write('\
					<tr>
						<td valign="bottom" width="200"><font class="label">Primary DNS Server</font></td>
						<td valign="bottom" width="200"><font class="label">Secondary DNS Server</font></td>
					</tr>
					<tr>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="15" name="dns1" size="15">
						</td>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="15" name="dns2" size="15">&nbsp;&nbsp;&nbsp;
						</td>
					</tr>');
}%>
				</table>
				</div>
				<div id="Domain" style="display:none" align="center">
				<table border="0" cellpadding="0" cellspacing="5" align="center" width="400">
					<tr>
						<td height="30" valign="middle" colspan="2"><font class="tabsTextBig">Domain</font></td>		
					</tr>
					<tr>
						<td colspan="2" valign="bottom" valign="left"><font class="label">Name</font></td>
					</tr>
					<tr>
						<td  colspan="2" valign="top" valign="left">
							<input class="formText" type="text" maxlength="39" name="domain" size="37">
						</td>
					</tr>
				</table>
				</div>
				<div id="Gateway" style="display:none" align="center">
				<table border="0" cellpadding="0" cellspacing="5" align="center" width="400">
					<tr>
						<td height="30" valign="middle" colspan="2"><font class="tabsTextBig">Gateway</font></td>
					</tr>
					<tr>
						<td colspan="2" valign="bottom" align="left"><font class="label">Address</font></td>
					</tr>
					<tr>
						<td colspan="2" valign="top" align="left">
							<input class="formText" type="text" maxlength="<%write(input_maxlength);%>" name="gateway" size="<%write(input_size);%>">
						</td>
					</tr>
				</table>
				</div>
			</td>
		</tr>
	</table>
	
<%if (BONDING_ENABLED == "yes") {
write('\
	<div id="Bonding" style="display:none" align="center">
	<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%">
		<tr valign="top">
			<td>
				<table border="0" cellpadding="0" cellspacing="5" align="center"  width="400">
					<tr>
						<td valign="bottom" width="200"><font class="tabsTextBig">Bonding</font></td>
						<td valign="bottom" width="200"></td>
					</tr>
				</table>
				<table border="0" cellpadding="0" cellspacing="10" align="center"  width="400">
					<tr valign="middle" align="center">
						<td><input type="checkbox" name="bondenabled" value="1" onclick="showBonding()">
							<font class="label">Enabled</font>&nbsp;&nbsp;&nbsp;&nbsp;
						</td>
					</tr>
				</table>
				<div id="Bonding2" style="display:none;" align="center">
				<table border="0" cellpadding="0" cellspacing="5" align="center"  width="400">
					<tr>
						<td valign="bottom" width="200"><font class="label">Miimon</font></td>
						<td valign="bottom" width="200"><font class="label">Updelay</font></td>
					</tr>
					<tr>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="30" name="bondmiimon" size="15">
						</td>
						<td valign="top" width="200">
							<input class="formText" type="text" maxlength="49" name="bondupdelay" size="15">
						</td>
					</tr>
				</table>
				</div>
			</td>
		</tr>
	</table>
	</div>
');
}%>

</form>

<form name="configForm" method="POST" action="/goform/Dmf" <%if (IPv6) write('target="_parent"');%>>
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/<%write(Url);%>.asp">
	<input type="hidden" name="urlError" value="/normal/network/<%write(Url);%>.asp">
	<input type="hidden" name="request" value=<%write(ReqNum);%>>
	<!--***********************************************************-->
	<input type="hidden" name="system.netSettings.mode" value="1">
	<!--***********************************************************-->
	<input type="hidden" name="system.netSettings.dhcp" value="">
	<input type="hidden" name="system.netSettings.hostName" value="">
	<input type="hidden" name="system.netSettings.ipAddress1" value="">
	<input type="hidden" name="system.netSettings.netMask1" value="">
	<input type="hidden" name="system.netSettings.dns1" value="">
	<input type="hidden" name="system.netSettings.domain" value="">
	<input type="hidden" name="system.netSettings.gateway" value="">
	<input type="hidden" name="system.netSettings.consBanner" value="">
	<input type="hidden" name="system.netSettings.ipAddress2" value="">
	<input type="hidden" name="system.netSettings.netMask2" value="">
	<input type="hidden" name="system.netSettings.mtu" value="">
	<input type="hidden" name="system.netSettings.dns2" value="">
<%if (BONDING_ENABLED == "yes")
write('\
	<input type="hidden" name="system.netSettings.bondenabled" value="">
	<input type="hidden" name="system.netSettings.bondmiimon" value="">
	<input type="hidden" name="system.netSettings.bondupdelay" value="">');
%>
<%if (IPv6) {
write('\
	<input type="hidden" name="system.netSettings.enableIPv4" value="">
	<input type="hidden" name="system.netSettings.enableIPv6" value="">
	<input type="hidden" name="system.netSettings.IPv6method" value="">
	<input type="hidden" name="system.netSettings.ethIp6"     value="">
	<input type="hidden" name="system.netSettings.ethPrefix6" value="">
	<input type="hidden" name="system.netSettings.DHCPv6Opts" value="">');
}%>
</form>
<%set("_sid_","0");%>
<script type="text/javascript">
	fill();
	ref.mainReady = true;
</script>

</body>
</html>
