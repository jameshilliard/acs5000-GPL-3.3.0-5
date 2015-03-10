<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>
	var cols;
	if (getIPmode() & 2) {
		cols = new Array(
			new _col("IP Address", 30, 32),
			new _col("Name",       13, 15),
			new _col("Alias",      13, 13)
		);
	} else {
		cols = new Array(
			new _col("IP Address", 16, 18),
			new _col("Name",       20, 22),
			new _col("Alias",      20, 20)
		);
	}
	var table = new selTable("table", cols, 10);
</script>

<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script type="text/javascript">	

	<%
		ReqNum = getRequestNumber();
		var IPv6 = getVal('_IPv6_');
	%>
	
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
	parent.helpPage = 'helpPages/netHostTableHelp.asp';
	var target='target=' + window.name;
	
	var hostTable = <%get("system.network.hostTable.hostTableHtml");%>;
	var x = (<%write(IPv6);%>)? 450 : 350;
	var y = 300;

	function fill() {

		fillSelTable(document.HostTableForm, table, hostTable);
		document.HostTableForm.table.selectedIndex = hostTable.length;

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
			parent.menu.selectItem("HOST");
		} else {
			setTimeout('init()', 200);
		}
	}

	function copyData() {
		document.configForm.elements[5].value = document.HostTableForm.table.length - 1;
		document.configForm.elements[6].value = getValues(document.HostTableForm.table, 0);
	}

	function delHostTableEntry() {
		deleteSelOpt(document.HostTableForm.table);
	}

	function addHostTableEntry() {
		copyData();
		getSelectedAdd('netHostTableEntry.asp',
			'host', x, y, target+"&SSID=<%get("_sid_");%>", 'yes');
	}

	function editHostTableEntry(index) {
		if (document.HostTableForm.table.options[index].value == '-1') {
			alert('Please select an item to edit.');
		} else {
			var selHost = "";
			var selHostSplit = "";
			var selHostIP = "";
			var selHostName = "";
			var selHostAlias = "";

			copyData();

			selHost = document.HostTableForm.table.options[index].value;
			selHostSplit = selHost.split(' ');
			if(selHostSplit.length >= 1) { selHostIP = selHostSplit[0];}
			if(selHostSplit.length >= 2) { selHostName = selHostSplit[1];}
			if(selHostSplit.length >= 3) { selHostAlias = selHostSplit[2];}
	
			newRef = 'netHostTableEntry.asp' + "?hostIndex=" + index + "&" +
				target +
				"&hostIP=" + hostTable[index][0] +
				"&hostName=" + hostTable[index][1] +
				"&hostAlias=" + hostTable[index][2] +
				"&SSID=<%get("_sid_");%>";
			newWindow(newRef, 'name', x, y, 'no');
		}
	}

	function submit(){
		copyData();
		document.configForm.submit();
		parent.mainReady = null;
	}

     </script>
</head>

<body onload="init();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netHostTable.asp">
	<input type="hidden" name="urlError" value="/normal/network/netHostTable.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<input type="hidden" name="system.network.hostTable.numHosts" value="">
	<input type="hidden" name="system.network.hostTable.hostTableSet" value="">
</form>

<form name="HostTableForm" onSubmit="return false;">
	<script>
		table.drawSelTable();
	</script>
</form>

<table border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="center" height="40" valign="middle">
			<a href="javascript:editHostTableEntry(document.HostTableForm.table.selectedIndex);">
			<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:delHostTableEntry();">
			<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:addHostTableEntry();">
			<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:optionUp(document.HostTableForm.table,document.HostTableForm.table.selectedIndex)">
			<img src="/Images/upButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:optionDown(document.HostTableForm.table,document.HostTableForm.table.selectedIndex)">
			<img src="/Images/downButton.gif" width="47" height="21" border="0" alt=""></a>
		</td>
	</tr>
</table>

<script type="text/javascript">
	fill();
	parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>

