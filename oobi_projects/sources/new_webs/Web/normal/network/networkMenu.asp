<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script language="JavaScript" src="/menu.js" type="text/javascript"></script>
	<script language="JavaScript">
		<%var hostPage; 
		if (getVal('_IPv6_')) {
			hostPage = "netHost";
		} else {
			hostPage = "netHostSettings"
		}%>
	
		var pageName = "/normal/network/networkMenu.asp";
	
		var topItem = "NET";
	
		if (parent.family_model == "AVCS")
			var menuPages = new Array(
			new menuObj("ETH", "Host Settings", "/normal/network/netHostSettings.asp"),
			new menuObj("LOG", "Syslog", "/normal/network/netSyslog.asp"),
			new menuObj("SNMP", "SNMP", "/normal/network/netSNMP.asp"),
			new menuObj("IPFW", "Firewall Configuration", "/normal/network/netIPTables.asp"),
			new menuObj("HOST", "Host Tables", "/normal/network/netHostTable.asp"),
			new menuObj("STRT", "Static Routes", "/normal/network/netStaticRoutes.asp")
			);
		else if ("<%get('_acs5k_');%>" == "1" )
			var menuPages = new Array(
			new menuObj("ETH", "Host Settings", "/normal/network/<%write(hostPage);%>.asp"),
			new menuObj("LOG", "Syslog", "/normal/network/netSyslog.asp"),
			new menuObj("VPN", "VPN Connections", "/normal/network/netVPNConnect.asp"),
			new menuObj("SNMP", "SNMP", "/normal/network/netSNMP.asp"),
			new menuObj("IPFW", "Firewall Configuration", "/normal/network/netIPTables.asp"),
			new menuObj("HOST", "Host Tables", "/normal/network/netHostTable.asp"),
			new menuObj("STRT", "Static Routes", "/normal/network/netStaticRoutes.asp")
			);
		else
			var menuPages = new Array(
			new menuObj("ETH", "Host Settings", "/normal/network/netHostSettings.asp"),
			new menuObj("LOG", "Syslog", "/normal/network/netSyslog.asp"),
			new menuObj("PCM", "PCMCIA Management", "/normal/network/netAccessMeth.asp"),
			new menuObj("VPN", "VPN", "/normal/network/netVPNConnect.asp"),
			new menuObj("SNMP", "SNMP", "/normal/network/netSNMP.asp"),
			new menuObj("IPFW", "IP Filtering", "/normal/network/netIPTables.asp"),
			new menuObj("HOST", "Host Tables", "/normal/network/netHostTable.asp"),
			new menuObj("STRT", "Static Routes", "/normal/network/netStaticRoutes.asp")
			);
	</script> 

	<script language="JavaScript" src="/menuNav.js" type="text/javascript"></script>

	<script language="JavaScript">

	function init() {
		parent.menuReady = 1;

		if ((parent.family_model != "ACS")&&(parent.family_model != "AVCS")) {
			leftHeadMenu("NET");
		}
	}

	</script> 

</head>

<body onload="init();" class="bgColor11" bottommargin="0" topmargin="0" leftmargin="0" rightmargin="0" marginheight="0" marginwidth="0">
<table border="0" cellpadding="0" cellspacing="0" width="204" height="100%">    
	<tr>
		<td class="bgColor8">   
			<table border="0" cellpadding="0" cellspacing="0" width="16" height="100%">
				<tr>
					<td background="/Images/leftEdge.gif" height="100%"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
				</tr>
			</table>
		</td>
		<td class="bgColor8" valign="top">
			<table border="0" cellpadding="0" cellspacing="0" width="163" height="100%">
				<tr>
					<td class="bgColor8" align="center" valign="bottom"><img src="/Images/menuTop.gif" alt="" width="163" height="4" border="0"></td>
				</tr>
				<tr>
					<td background="/Images/menuBackground.gif" height="100%" width="100%" valign="top">
						&nbsp;
					</td>                    
				</tr>
			</table>
		</td>
		<td class="bgColor8" width="25">
			&nbsp;
		</td>
	</tr>
</table>     

<script language="JavaScript">
<!-- Begin Hide
  print_head_menu(20, 2, menuPages, "NET");
// End Hide -->
</script>

</body>
</html>
