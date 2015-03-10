<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>
	var cols;
	if (getIPmode() & 2) {
		cols = new Array(
			new _col("Community",  15, 17),
			new _col("Source",     23, 25),
			new _col("OID",        15, 17),
			new _col("Permission", 10, 10)
		);
	} else {
		cols = new Array(
			new _col("Community",  20, 22),
			new _col("Source",     18, 20),
			new _col("OID",        15, 17),
			new _col("Permission", 10, 10)
		);
	}
	var table1 = new selTable("table1", cols, 10);
</script>

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		ReqNum = getRequestNumber();
	%>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
	parent.helpPage = 'helpPages/netSNMPHelp.asp';
	var target='target=' + window.name;

	var SNMPv12 = <%get("system.network.SNMPdaemon.SNMPv12Html");%>;

	function fill() {
		fillSelTable(document.netSNMPForm, table1, SNMPv12);
		document.netSNMPForm.table1.selectedIndex = SNMPv12.length;
		for (i=0; i<(document.netSNMPForm.table1.options.length-1); i++) {
			document.netSNMPForm.table1.options[i].value = i;
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
			parent.menu.selectItem("SNMP");
		} else {
			setTimeout('init()', 200);
		}
	}

	function updateContactLocation() {
		document.configForm.elements[5].value = document.netSNMPForm.sysContact.value;
		document.configForm.elements[6].value = document.netSNMPForm.sysLocation.value;
	}

	function updateSNMPv12Entries() {
		if (document.configForm.elements[7].value == '') {
			for (var i = 0; i < document.netSNMPForm.table1.length; i++) {
				if (document.netSNMPForm.table1.options[i].value != '-1') {
					document.configForm.elements[7].value +=
						document.netSNMPForm.table1.options[i].value;
				}
			}
		}
	}

	function updateSNMPv3Entries() {
		if (document.configForm.elements[8].value == '') {
			for (var i = 0; i < document.netSNMPForm.table2.length; i++) {
				if (document.netSNMPForm.table2.options[i].value != '-1') {
					document.configForm.elements[8].value +=
						document.netSNMPForm.table2.options[i].value;
				}
			}
		}
	}

	function addSNMPv12Entry() {
		updateContactLocation();
		updateSNMPv12Entries();
		getSelectedAdd('netSNMPEntry1.asp','snmpv12entry','450','250',target+"&SSID=<%get("_sid_");%>",'yes');
	}

	function editSNMPv12Entry() {
		updateContactLocation();
		updateSNMPv12Entries();
		getSelectedEdit('netSNMPEntry1.asp','snmpv12entry','450','250',target+"&SSID=<%get("_sid_");%>",document.netSNMPForm.table1,'yes');
	}

	function deleteSNMPv12Entry() {
		if (deleteSelOpt(document.netSNMPForm.table1) == true) {
			document.configForm.elements[7].value = '';
		}
	}

	function addSNMPv3Entry() {
		updateContactLocation();
		updateSNMPv3Entries();
		getSelectedAdd('netSNMPEntry2.asp','snmpv3entry','350','250',target+"&SSID=<%get("_sid_");%>",'yes');
	}

	function editSNMPv3Entry() {
		updateContactLocation();
		updateSNMPv3Entries();
		getSelectedEdit('netSNMPEntry2.asp','snmpv3entry','350','250',target,document.netSNMPForm.table2, 'yes');
	}

	function deleteSNMPv3Entry() {
		if (deleteSelOpt(document.netSNMPForm.table2) == true) {
			document.configForm.elements[8].value = '';
		}
	}

	function submit() {
		updateContactLocation();
		updateSNMPv12Entries();
		updateSNMPv3Entries();
		document.configForm.submit();
		parent.mainReady = null;
	}

	</script>		   
</head>

<body onload="init();" class="bodyMain">
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="450" align="center">
	<tr>
		<td>
			<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
				<tr>
					<td align="center" class="tabsTextBig">
						<script type="text/javascript">
							if (top.family_model == "AVCS") {
								secprofile = 'Security > Services';
							} else {
								secprofile = 'Security Profile';
							}
							document.write('To activate the snmpd services, you should go to the <br> '+secprofile+' section.');
						</script>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
<form name="netSNMPForm" onSubmit="return false;">
<table width="470" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr align="center" valign="middle">
		<td height="40" colspan="4">
			<font class="tabsTextBig">System Information Settings</font>
		</td>
	</tr>
	<tr align="left" valign="middle">
		<td height="30" align="right">
			<font class="label">SysContact&nbsp;&nbsp;</font>
		</td>
		<td height="30" colspan="3">
			<input name="sysContact" type="text" maxlength="299" class="formText" size="40" value="<%get("system.network.SNMPdaemon.sysContact");%>">
		</td>
	</tr>
	<tr align="left" valign="middle"> 
		<td height="30" align="right">
			<font class="label">&nbsp;SysLocation&nbsp;&nbsp;</font>
		</td>
		<td height="30" colspan="3">
			<input name="sysLocation" type="text" maxlength="299" class="formText" size="40" value="<%get("system.network.SNMPdaemon.sysLocation");%>">
		</td>
	</tr>
	<tr align="center" valign="middle"> 
		<td height="30" colspan="4">
			<font class="tabsTextBig">Access Control</font>
		</td>
	</tr>
	<tr align="center" valign="middle"> 
		<td height="40" colspan="4">
			<font class="tabsTextBig">SNMPv1/SNMPv2 Configuration</font>
		</td>
	</tr>
</table>
<script>
	table1.drawSelTable();
</script>
<!-- table border="0" cellpadding="0" cellspacing="0" align="center" class="tableColor">
	<tr align="left" valign="top">
		<td>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr align="left" class="tableColor">
					<td width="203">
						<font class="tabsTextBig">&nbsp;Community</font>
					</td>
					<td width="179">
						<font class="tabsTextBig">Source</font>
					</td>
					<td width="153">
						<font class="tabsTextBig">OID</font>
					</td>
					<td>
						<font class="tabsTextBig">Permission</font>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr align="left" valign="top">
		<td align="left">
	options width = 
	1 (0 ... 20); 2 whitespace;
	2 (0 ... 18); 2 whitespace;
	3 (0 ... 15); 2 whitespace;
	4 (0 ... 10);
	
			<select name="table1" size="10" class="formText">
				<%get("system.network.SNMPdaemon.SNMPv12Html");%>
					<option value="-1"> 
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
			</select>
		</td>
	</tr>
</table-->
<table border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="center" height="40" valign="middle">
			<a href="javascript:addSNMPv12Entry()"> 
				<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:deleteSNMPv12Entry()">
				<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:editSNMPv12Entry()">
				<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
	</tr>
</table>
<table width="470" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr align="center" valign="middle">
		<td height="40" colspan="4">
			<font class="tabsTextBig">SNMPv3 Configuration</font>
		</td>
	</tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" align="center"  class="tableColor">
	<tr align="left" valign="top">
		<td>
			<table border="0" cellpadding="0" cellspacing="0">
				<tr align="left" class="tableColor">
					<td width="202">
						<font class="tabsTextBig">&nbsp;User Name</font>
					</td>
					<td width="109">
						<font class="tabsTextBig">Permission</font>
					</td>
					<td>
						<font class="tabsTextBig">OID</font>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr align="left" valign="top">
		<td align="left"><font class="formText">
	<!-- options width = 
	1 (0 ... 20); 2 whitespace;
	2 (0 ... 10); 2 whitespace;
	3 (0 ... 15);
	-->
			<select name="table2" size="10" class="formText">
				<%get("system.network.SNMPdaemon.SNMPv3Html");%>
				<option value="-1"> 
<!-- 49 spaces (newlines counts as one space !!!) -->
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
			</select>
		</td>
	</tr>
</table>
<table border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td align="center" height="40" valign="middle">
			<a href="javascript:addSNMPv3Entry()">
				<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:deleteSNMPv3Entry()">
				<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:editSNMPv3Entry()"> 
				<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
	</tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netSNMP.asp">
	<input type="hidden" name="urlError" value="/normal/network/netSNMP.asp">
	<input type="hidden" name="request" value=<%write(ReqNum);%>>
	<!--*******************************************************************-->
	<input type="hidden" name="system.network.SNMPdaemon.sysContact" value="">
	<input type="hidden" name="system.network.SNMPdaemon.sysLocation" value="">
	<input type="hidden" name="system.network.SNMPdaemon.SNMPv12Entries"
		value="<%get("system.network.SNMPdaemon.SNMPv12Entries");%>">
	<input type="hidden" name="system.network.SNMPdaemon.SNMPv3Entries"
		value="<%get("system.network.SNMPdaemon.SNMPv3Entries");%>">
</form>

<script type="text/javascript">
fill();
parent.mainReady = true;
</script>	
<%set("_sid_","0");%>
</body>
</html>
