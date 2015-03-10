<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>

<head>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		var IPv6 = getVal('_IPv6_');
		var selChain = getQueryVal("chainIndex",-1);
		var chainName = getQueryVal("name",-1);
		var chainPolicy = getQueryVal("policy",-1);
		var chainTable = getQueryVal("table",-1);
		var title = "Edit Chain";
		if (IPv6) {
			chainName = chainName + " (" + chainTable + ")";
		}
		if (selChain == "-1") {
			chainName = "";
			title = "Add Chain";
		}
		ReqNum = getRequestNumber();
	%>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	var selChain = <%write(selChain);%>; 
	var target='target=' + window.name;

	function init() {
		if (selChain != -1) {
			hide2("document", "name");
<%if (IPv6) {
write('\
			hide2("document", "table");');
}%>
			show2("document", "staticName");
			show2("document", "policy");
		} else {
			show2("document", "name");
<%if (IPv6) {
write('\
			show2("document", "table");');
}%>
			hide2("document", "staticName");
			hide2("document", "policy");
		}
		document.IPTablesEntryForm.chainPolicy.value = "<%write(chainPolicy);%>";
	}

	function copyData() {
		document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
		document.configForm.elements[4].value = opener.document.configForm.elements[4].value;
		document.configForm.elements[5].value = opener.document.configForm.elements[5].value;
		document.configForm.elements[6].value = getValues(opener.document.netIPTableForm.table,0);
	}

	function isValid(parm) {
		var val = '0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ';
		var val1 = '_-';
		for (i=0; i<parm.length; i++) {
			if (i == 0 && val.indexOf(parm.charAt(i),0) == -1) {
				return false;
			} else if (val.indexOf(parm.charAt(i),0) == -1 && val1.indexOf(parm.charAt(i),0) == -1) {
				return false;
			}
		}
		return true;
	}

	function addChain() {
		copyData();
		document.configForm.elements[5].value = opener.document.netIPTableForm.table.length;
		document.configForm.elements[6].value += "0 " +
			document.IPTablesEntryForm.chainName.value +
<%if (IPv6) {
write('\
			" - " +
			document.IPTablesEntryForm.chainTable.value +
			" ,";');
} else {
write('\
			" - ,";');
}%>
	}

	function form_verification() {
		var form_location = document.IPTablesEntryForm;
		continue_verification = verify([
			[form_location.chainName, "Fill,ChainName"]
		]);
		return continue_verification;
	}

	function submitEntry() {
		try {
			if (opener.window.name != document.configForm.target) {
				alert("The page which called this form is not available anymore. The changes will not be effective.");
				self.close();
				return;
			}
			if (selChain == -1) {
				if (form_verification()) {
					addChain();
				} else {
					return;
				}
			} else {
				valueChain = opener.document.netIPTableForm.table.options[selChain].value;
				array = valueChain.split(" ");
				opener.document.netIPTableForm.table.options[selChain].value = array[0] + 
					" " + array[1] +
					" " + document.IPTablesEntryForm.chainPolicy.value<%if (IPv6) {
write(' +
					" " + array[3];');
} else {
write(';
');
}%>
				copyData();
			}
			setQuerySSID(document.configForm);
			document.configForm.submit();
			self.close();
		} catch (error) {
			self.close();
		}
	}
    </script>
	<title><%write(title);%></title>
</head>

<body class="bodyMain" onLoad="init();">
<form name="IPTablesEntryForm" onSubmit="return false;">
<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
	<tr>
		<td align="center" valign="middle" height="50">
			<a href="javascript:submitEntry();">
			<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
			&nbsp;&nbsp;&nbsp;&nbsp;
			<a href="javascript:window.close();">
			<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
		</td>
	</tr>
	<tr>
		<td height="50" align="center" valign="middle">
		<div id="name" style="display:none;">
			<font class="label">Name </font>
			&nbsp;&nbsp;&nbsp;&nbsp;
			<input type="text" size="19" maxlength="19" name="chainName" class="formText" value="">
		</div>
		<div id="staticName" style="display:none;">
			<font class="label">Name&nbsp;&nbsp;&nbsp;&nbsp;<%write(chainName);%></font>
		</div>
		</td>
	</tr>
	<tr align="center" valign="middle">
		<td height="50" valign="middle">
		<div id="policy" style="display:none;">
			<font class="label">Policy&nbsp;&nbsp;&nbsp;&nbsp;</font>
			<select name="chainPolicy" size="1" class="formText">
				<option value="ACCEPT">ACCEPT&nbsp;&nbsp;</option>
				<option value="DROP">DROP&nbsp;&nbsp;</option>
			</select>
		</div>
<%if (IPv6) {
write('\
		<div id="table" style="display:none;">
			<font class="label">Table&nbsp;&nbsp;&nbsp;&nbsp;</font>
			<select name="chainTable" size="1" class="formText">
				<option value="IPv4">IPv4&nbsp;&nbsp;</option>
				<option value="IPv6">IPv6&nbsp;&nbsp;</option>
			</select>
		</div>');
}%>
		</td>
	</tr>
</table>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="">
	<input type="hidden" name="urlOk" value="/normal/network/netIPTables.asp">
	<input type="hidden" name="urlError" value="/normal/network/netIPTables.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<!---------------- elements 5-7 below ---------------------------------------------->
	<input type="hidden" name="system.network.ipTables.chain.numChains" value="">
	<input type="hidden" name="system.network.ipTables.chain.chainsCs" value="">
	<input type="hidden" name="system.network.ipTables.chain.chainSelected" value="">
</form>
</table>

</body>
</html>
