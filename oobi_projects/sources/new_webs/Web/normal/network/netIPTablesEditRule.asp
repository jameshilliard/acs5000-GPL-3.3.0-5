<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>
<%
var chainType = "IPv4";
var chainName = getVal("system.network.ipTables.rules.chainName");
var chainLabel = chainName;
var IPv6 = getVal('_IPv6_');
if (IPv6) {
	chainType = getVal("system.network.ipTables.rules.chainType");
	chainLabel = chainName + " (" + chainType + ")";
	if (chainType=="IPv4") {
	write('\
		var cols = new Array(
			new _col("Packets",             8,  12),
			new _col("Bytes",               8,  10),
			new _col("Target",              6,  8 ),
			new _col("Source",              16, 17),
			new _col("Destination",         16, 17),
			new _col("Protocol",            9,  10),
			new _col("Fragment",            9,  11),
			new _col("Input<br>Interface",  8,  9 ),
			new _col("Output<br>Interface", 8,  8 )
		);');
	} else {
	write('\
		var cols = new Array(
			new _col("Packets",             8,  10),
			new _col("Bytes",               8,  10),
			new _col("Target",              6,  8 ),
			new _col("Source",              21, 22),
			new _col("Destination",         21, 22),
			new _col("Protocol",            9,  10),
			new _col("Input<br>Interface",  8,  9 ),
			new _col("Output<br>Interface", 8,  8 )
		);');
	}
} else {
write('\
	var cols = new Array(
		new _col("Packets",             8,  12),
		new _col("Bytes",               8,  10),
		new _col("Target",              6,  8 ),
		new _col("Source",              16, 17),
		new _col("Destination",         16, 17),
		new _col("Protocol",            9,  10),
		new _col("Fragment",            9,  11),
		new _col("Input<br>Interface",  8,  9 ),
		new _col("Output<br>Interface", 8,  8 )
	);');
}
%>
	var table = new selTable("table", cols, 10);
</script>

<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">
	
		<%
			ReqNum = getRequestNumber();
		%>
	
		var rulesTable = <%get("system.network.ipTables.rules.rulesTableHtml");%>;
		window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
		parent.currentPage = self;
		parent.helpPage = 'helpPages/netIPTablesEditRuleHelp.asp';
		var target='&target=' + window.name;
	
		var chainName="<%write(chainName);%>";
		var chainType="<%write(chainType);%>";
		var editPage=(chainType=="IPv6")?
			"netIPTablesEditRuleEntry6.asp" :
			"netIPTablesEditRuleEntry.asp";
	
		function fill() {
			var i;
			fillSelTable(document.netIPTableRuleForm, table, rulesTable);
			document.netIPTableRuleForm.table.selectedIndex = rulesTable.length;
			for (i=0; i<document.netIPTableRuleForm.table.options.length-1; i++) {
				document.netIPTableRuleForm.table.options[i].value = i+1;
			}
		}

		function init() {
			if (1 == parent.menuReady && 1 == parent.topMenuReady && 
				1 == parent.controlsReady) {
				setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
			} else
				setTimeout('init()', 200);
		}
	
		function copyData() {
			document.configForm.elements[5].value = document.netIPTableRuleForm.table.length-1;
			document.configForm.elements[6].value = ','+getValues(document.netIPTableRuleForm.table, 0)+',';
		}
	
		function editRule(index) {
			var ruleval = document.netIPTableRuleForm.table.options[index].value;
			if (ruleval == '-1') {
				alert('Please select a rule to edit.');
			} else {
				copyData();
				document.configForm.elements[7].value = 3;
				document.configForm.elements[8].value = ruleval;
				newref = editPage + "?ruleIndex=" + ruleval + 
					"&chain=" + chainName + target + "&SSID=<%get("_sid_");%>";
				newWindow(newref,'editrule','600','590','yes');
			}
		}
	
		function addRule() {
			copyData();
			document.configForm.elements[7].value = 2;
			newref = editPage + "?chain=" + chainName + target + "&SSID=<%get("_sid_");%>";
			newWindow(newref,'addrule','600','590','yes');
		}
	
		function deleteRule(formTable) {
			if (formTable.options[formTable.selectedIndex].value == '-1') {
				alert('Please select a rule to delete.');
			} else {
				deleteSelOpt(formTable);
			}
		}
	
		function submit() {
			copyData();
			document.configForm.elements[7].value = 0;
			document.configForm.submit();
			parent.mainReady = null;
		}
	
	</script>
</head>

<body  onload="init();" class="bodyMain">
<form name="netIPTableRuleForm" onSubmit="return false;">
<table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr valign="top" align="left">
		<td height="25">
			<font class="tabsTextBig">Edit Rules for Chain [<%write(chainLabel);%>]</font>
		</td>
	</tr>
</table>

<script>
	table.drawSelTable();
</script>

<table height="80" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr valign="bottom">
		<td align="center">
			<a href="javascript:editRule(document.netIPTableRuleForm.table.selectedIndex)">
			<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:deleteRule(document.netIPTableRuleForm.table)">
			<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript: addRule()">
			<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:optionUp(document.netIPTableRuleForm.table,document.netIPTableRuleForm.table.selectedIndex)">
			<img src="/Images/upButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:optionDown(document.netIPTableRuleForm.table,document.netIPTableRuleForm.table.selectedIndex)"> 
			<img src="/Images/downButton.gif" width="47" height="21" border="0" alt=""></a> 
		</td>
	</tr>
	<tr valign="bottom">
		<td align="center">
			<a href="javascript:submitForm(parent.currentPage,'/normal/network/netIPTables.asp');"> 
			<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a> 
		</td>
	</tr>
</table>
</form>

<!--**************************************************************************-->

<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netIPTables.asp">
	<input type="hidden" name="urlError" value="/normal/network/netIPTablesEditRule.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
<!---------------- elements 5-7 below ---------------------------------------------->
	<input type="hidden" name="system.network.ipTables.rules.numRules" value="">
	<input type="hidden" name="system.network.ipTables.rules.rulesCs" value="">
	<input type="hidden" name="system.network.ipTables.rules.actionRule" value="">
	<input type="hidden" name="system.network.ipTables.rules.ruleSelected" value="">
</form>

<script type="text/javascript">
fill();
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>