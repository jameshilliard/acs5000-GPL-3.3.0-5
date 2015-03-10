<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>
<%var IPv6 = getVal('_IPv6_');
if (IPv6) {
write('\
	var cols = new Array(
		new _col("Name",    18, 20),
		new _col("Policy",  10, 12),
		new _col("Packets", 10, 12),
		new _col("Bytes",   10, 12),
		new _col("Table",   4,  4)
	);');
} else {
write('\
	var cols = new Array(
		new _col("Name",    22, 24),
		new _col("Policy",  10, 12),
		new _col("Packets", 10, 12),
		new _col("Bytes",   10, 10)
	);');
}%>
	var table = new selTable("table", cols, 10);
</script>

<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

		<% ReqNum = getRequestNumber(); %>
	
		var ipTable = <%get("system.network.ipTables.ipTableHtml");%>;
		window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
		parent.currentPage = self;
		parent.helpPage = 'helpPages/netIPTablesFilterHelp.asp';      
		var target='target=' + window.name;

		function fill() {
			var i, val;
			fillSelTable(document.netIPTableForm, table, ipTable);
			document.netIPTableForm.table.selectedIndex = ipTable.length;
			for (i=0; i<document.netIPTableForm.table.options.length-1; i++) {
<%if (IPv6) {
write('\
				val = (i+1) + " " + ipTable[i][0] + " " + ipTable[i][1] + " " + ipTable[i][4];');
} else {
write('\
				val = (i+1) + " " + ipTable[i][0] + " " + ipTable[i][1];');
}%>
				document.netIPTableForm.table.options[i].value = val;
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
				parent.menu.selectItem("IPFW");
			} else {
				setTimeout('init()', 200);
			}
		}
	 	
		function copyData() {
			document.configForm.elements[5].value = document.netIPTableForm.table.length-1;
			document.configForm.elements[6].value = getValues(document.netIPTableForm.table,0);
		}
	
		function editIpTableEntry(index) {
			var selChain = document.netIPTableForm.table.options[index].value;
			var selChainName = "";
			if (selChain == '-1' || index == -1) { 
				alert('Please select a chain to edit.');
				return;
			}     
			copyData();
			array = selChain.split(" ");
			selChainName = array[1];
			selChainPolicy = array[2];
<%if (IPv6) {
write('\
			selChainTable = array[3];');
}%>
			if (ipTable[index][0] != "INPUT" && ipTable[index][0] != "FORWARD" && ipTable[index][0] != "OUTPUT") {
				alert("User-defined chain cannot be edited.");
				return;
			}
	
			newRef = 'netIPTablesEntry.asp' + "?chainIndex=" + index + "&" + target +
<%if (IPv6) {
write('\
				"&name=" + selChainName + "&policy=" + selChainPolicy + "&table=" + selChainTable +');
} else {
write('\
				"&name=" + selChainName + "&policy=" + selChainPolicy +');
}%>
				"&SSID=<%get("_sid_");%>";
			newWindow(newRef,'network','350','250','no');
		}
	
		function addIpTableEntry() {
			copyData();
			getSelectedAdd('netIPTablesEntry.asp','network','350','205',target+"&SSID=<%get("_sid_");%>");
		}
	
		function deleteIpTableEntry(index) {
			selChain = document.netIPTableForm.table.options[index].value;
			array = selChain.split(" ");
			selChainName = array[1];
	
			if (selChain == '-1' || index == -1) { 
				alert('Please select a chain to delete.');
				return;
			}     
			if (selChainName == "INPUT" || selChainName == "FORWARD" || 
				selChainName == "OUTPUT") {
				alert("Non user-defined chain cannot be deleted.");
				return;
			}
			deleteSelOpt(document.netIPTableForm.table);
		}
	
		function listRules(index) {
			selChain = document.netIPTableForm.table.options[index].value;
			if (selChain == '-1' || index == -1) { 
				alert('Please select a chain to edit rules.');
				return;
			}     
			array = selChain.split(" ");
			copyData();
			document.configForm.elements[2].value = "/normal/network/netIPTablesEditRule.asp";
			document.configForm.elements[7].value = array[0];
			setQuerySSID(document.configForm);
			document.configForm.submit();
		}	
		
		function submit() {
			copyData();
			document.configForm.submit();
			parent.mainReady = null;
		}

	</script>
</head>

<body onload="init();" class="bodyMain">
<form name="netIPTableForm" onSubmit="return false;">
	<script>
		table.drawSelTable();
	</script>
</form>

<table border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="center" height="40" valign="middle">
			<a href="javascript:editIpTableEntry(document.netIPTableForm.table.selectedIndex)">
			<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:deleteIpTableEntry(document.netIPTableForm.table.selectedIndex)">
			<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:addIpTableEntry()">
			<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:listRules(document.netIPTableForm.table.selectedIndex)">
			<img src="/Images/editRulesButton.gif" width="80" height="21" border="0" alt=""></a>
		</td>
	</tr>
</table>

<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netIPTables.asp">
	<input type="hidden" name="urlError" value="/normal/network/netIPTables.asp">
	<input type="hidden" name="request" value=<%write(ReqNum);%>>
	<!-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% -->
	<input type="hidden" name="system.network.ipTables.chain.numChains" value="">
	<input type="hidden" name="system.network.ipTables.chain.chainsCs" value="">
	<input type="hidden" name="system.network.ipTables.chain.chainSelected" value="">
</form>

<script type="text/javascript">
fill();
parent.mainReady = true;
</script>   
<% set("_sid_","0");%>
</body>
</html>
