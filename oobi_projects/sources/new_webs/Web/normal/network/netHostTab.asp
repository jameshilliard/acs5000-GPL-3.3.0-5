<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<title>Access</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">
		<% 
			var sid =     getQueryVal("SSID","0");
			var iptab =   getQueryVal("iptab","0");
		%>
		function init() {
			if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
				setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			} else {
				setTimeout('init()', 200);
			}
		}
		
	</script>
</head>

<!-- frames -->
<body class="bodyMainTabs" marginheight="0" marginwidth="0">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">
		var hostTab = new Array(
			new _tab("gen",  1,                        "General", "normal/network/netHost.asp?iptab=0", "34%"),
			new _tab("tab4", (getIPmode() & 1)? 1 : 2, "IPv4",    "normal/network/netHost.asp?iptab=1", "33%"),
			new _tab("tab6", (getIPmode() & 2)? 1 : 2, "IPv6",    "normal/network/netHost.asp?iptab=2", "33%")
		);
		hostTab[<%write(iptab);%>].state = 0;
		
		var hostTabs = new _tabs("netHostTab", hostTab);
		
		hostTabs.drawTabs();
	</script>
</body>
</html>
