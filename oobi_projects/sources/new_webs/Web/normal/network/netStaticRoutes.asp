<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>
	var cols;
	if (getIPmode() & 2) {
		cols = new Array(
			new _col("Destination IP",   16, 18),
			new _col("Destination Mask", 16, 18),
			new _col("Gateway",          16, 18),
			new _col("Interface",        8,  10),
			new _col("Metric",           5,  5)
		);
	} else {
		cols = new Array(
			new _col("Destination IP",   16, 18),
			new _col("Destination Mask", 16, 18),
			new _col("Gateway",          16, 18),
			new _col("Interface",        8,  10),
			new _col("Metric",           6,  6)
		);
	}
	var table = new selTable("table", cols, 10);
</script>

<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		ReqNum = getRequestNumber();
		var IPv6 = getVal('_IPv6_');
	%>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
	parent.helpPage = 'helpPages/netStaticRoutesHelp.asp';
	var target='target=' + window.name;

	var stRoutes = <%get("system.network.staticRoutes.stRoutesHtml");%>;
	var x = (<%write(IPv6);%>)? 450 : 350;
	var y = 300;

	function fill() {
		var i;
		fillSelTable(document.netStaticRoutesForm, table, stRoutes);
		document.netStaticRoutesForm.table.selectedIndex = stRoutes.length;
		for (i=0; i<(document.netStaticRoutesForm.table.options.length-1); i++) {
			document.netStaticRoutesForm.table.options[i].value = i + " ";
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
			parent.menu.selectItem("STRT");
		}
		else
			setTimeout('init()', 200);
    }

	function updateEntries() {
		if (document.configForm.elements[5].value == '') {
			for (var i = 0; i < document.netStaticRoutesForm.table.length; i++) {
				if (document.netStaticRoutesForm.table.options[i].value != '-1')
					document.configForm.elements[5].value +=
						document.netStaticRoutesForm.table.options[i].value;
			}
		}
	}

	function addStaticRoutesEntry() {
		updateEntries();
		getSelectedAdd('netStaticRoutesEntry.asp','route','500','350',target+"&SSID=<%get("_sid_");%>",'yes');
	}

	function editStaticRoutesEntry() {
		updateEntries();
		getSelectedEdit('netStaticRoutesEntry.asp','route','500','350',target+"&SSID=<%get("_sid_");%>",document.netStaticRoutesForm.table,'yes');
	}

	function deleteStaticRoutesEntry() {
		if (deleteSelOpt(document.netStaticRoutesForm.table) == true) {
			document.configForm.elements[5].value = '';
		}
	}

	function submit() {
		updateEntries();
		document.configForm.submit();
		parent.mainReady = null;
	}

	</script>
</head>

<body onload="init();" class="bodyMain">
<form name="netStaticRoutesForm" onSubmit="return false;">
	<script>
		table.drawSelTable();
	</script>
</form>

<table border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="center" height="40" valign="middle">
			<a href="javascript:editStaticRoutesEntry();">
			<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a> 
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:deleteStaticRoutesEntry();">
			<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a> 
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:addStaticRoutesEntry();">
			<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a> 
<!--
      	&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		<a href="javascript:document.configForm.elements[5].value = '';
							optionUp(document.netStaticRoutesForm.table,
									 document.netStaticRoutesForm.table.selectedIndex);"> 
      	<img src="/Images/upButton.gif" width="47" height="21" border="0" alt=""></a> 
      	&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
		<a href="javascript:document.configForm.elements[5].value = '';
							optionDown(document.netStaticRoutesForm.table,
									   document.netStaticRoutesForm.table.selectedIndex);"> 
      	<img src="/Images/downButton.gif" width="47" height="21" border="0" alt=""></a> 
-->
<!--		<a href="javascript:alert(':::');"> 
      	<img src="/Images/downButton.gif" width="47" height="21" border="0" alt=""></a> 
-->
		</td>
	</tr>
</table>

<form name="configForm" method="POST" action="/goform/Dmf">
    <input type="hidden" name="system.req.action" value="">
    <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
    <input type="hidden" name="urlOk" value="/normal/network/netStaticRoutes.asp">
    <input type="hidden" name="urlError" value="/normal/network/netStaticRoutes.asp">
    <input type="hidden" name="request" value="<%write(ReqNum);%>">
    <!--*******************************************************************-->
    <input type="hidden" name="system.network.staticRoutes.entriesList"
		value="<%get("system.network.staticRoutes.entriesList");%>">
</form>

<script type="text/javascript">
fill();
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
