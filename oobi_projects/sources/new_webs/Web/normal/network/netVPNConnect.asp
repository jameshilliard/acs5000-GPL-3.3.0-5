<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>
	var cols;
	if (getIPmode() & 2) {
		cols = new Array(
			new _col("Connection Name",        12, 14),
			new _col("Right Subnet (IP/mask)", 23, 25),
			new _col("Left Subnet (IP/mask)",  23, 23)
		);
	} else {
		cols = new Array(
			new _col("Connection Name",        22, 24),
			new _col("Right Subnet (IP/mask)", 18, 20),
			new _col("Left Subnet (IP/mask)",  18, 18)
		);
	}
	var table = new selTable("table", cols, 15);
</script>

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		ReqNum = getRequestNumber();
		var IPv6 = getVal('_IPv6_');
		var VPNConnEntryPage = "netVPNConnEntry.asp";
		var width = 600;
		if (IPv6) {
			VPNConnEntryPage = "netVPNConnEntry6.asp";
			width = 700;
		}
	%>
      
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>'
	parent.currentPage = self;
	parent.helpPage = 'helpPages/netVPNConnectHelp.asp';
	var target='target=' + window.name;

	var VPNConnEntryPage = "<%write(VPNConnEntryPage);%>";
	var width = "<%write(width);%>";
	var VPNTable = <%get("system.network.VPNTable.VPNTableHtml");%>;
	

	function fill() {
		fillSelTable(document.netVPNForm, table, VPNTable);
		document.netVPNForm.table.selectedIndex = VPNTable.length;
		for (i=0; i<(document.netVPNForm.table.options.length-1); i++) {
			document.netVPNForm.table.options[i].value = i;
		}
	}

	function init() {
		if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
			if (checkLeftMenu('/normal/network/networkMenu.asp') == 0) {
				parent.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
			parent.topMenu.selectItem(parent.menu.topItem);
			parent.menu.selectItem("VPN");
		} else {
			setTimeout('init()', 200);
		}
	}

	function submit() {
		document.configForm.submit();
		parent.mainReady = null;
	}
      
	function deleteFromTable() {
		var table = document.netVPNForm.table;
		var Index = table.options[table.selectedIndex].value;
		if (table.selectedIndex == -1 || table.options[table.selectedIndex].value == '-1') {
			alert("Please select an item to delete");
		} else {
			deleteSelOpt(table);
			if (document.configForm.elements[5].value == "")
				document.configForm.elements[5].value = "Deleted=" + Index;
			else
				document.configForm.elements[5].value += ";" + Index;
		}
	}

	</script>
</head>

<body onload="init();" class="bodyMain">
<form name="netVPNForm" method="POST" action="/goform/Dmf">
	<script>
		table.drawSelTable();
	</script>
</form>

<table border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>
		<td align="center" height="40" valign="middle">
			<a href="javascript:getSelectedEdit(VPNConnEntryPage,'vpnIndex',width,'590',target+'&SSID=<%get("_sid_");%>',document.netVPNForm.table,'yes','val')">
				<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:deleteFromTable()">
				<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:getSelectedAdd(VPNConnEntryPage,'vpnIndex',width,'590',target+'&SSID=<%get("_sid_");%>','yes')">
				<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
		</td>
	</tr>
</table>

<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netVPNConnect.asp">
	<input type="hidden" name="urlError" value="/normal/network/netVPNConnect.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<input type="hidden" name="system.network.VPNTable.VPNTableDel" value="">
</form>

<script type="text/javascript">
fill();
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
