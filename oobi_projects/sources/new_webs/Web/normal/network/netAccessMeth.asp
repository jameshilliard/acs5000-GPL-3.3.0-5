<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<%
		self = "system.network.accessMethod.";
		ReqNum = getRequestNumber();
		var IPv6 = getVal('_IPv6_');
	%>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
	parent.helpPage = 'helpPages/netAccessMethHelp.asp';

	var target = '&target=' + window.name;
	var cardType0 = "<% get(self + "ident", 0); %>"; //lower
	var cardType1 = "<% get(self + "ident", 1); %>"; //upper
	var width = (<%write(IPv6);%>)? 500 : 400;

	function init()	{
		if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
			if (checkLeftMenu('/normal/network/networkMenu.asp') == 0) {
				parent.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
			parent.topMenu.selectItem(parent.menu.topItem);
			parent.menu.selectItem("PCM");
		} else {
			setTimeout('init()', 200);
		}
	}

	function submit() {
		document.configForm.submit();
		parent.mainReady = null;
	}

	function insert(slot) {
		// insert = kAdd
		if (confirm("Press OK when the PCMCIA card is inserted.")) { 
			document.configForm.elements[5].value = 2;
			document.configForm.elements[6].value = slot;
			setQuerySSID(document.configForm);
			submit();
		}
	}

	function eject(slot) {
		if ((slot == 0 && cardType0 == "no card") || (slot == 1 && cardType1 == "no card")) {
			alert("No card detected. Eject not allowed.");
			return;
		}
		// eject = kDelete
		document.configForm.elements[5].value = 1;
		document.configForm.elements[6].value = slot;
		setQuerySSID(document.configForm);
		submit();
		alert("Please remove your card and then press OK.");
	}

	function config(slot) {
		var newRef;
		if (top.family_model == "KVM") {
			if ((slot == 0 && cardType0 == "no card") || 
				(slot == 1 && cardType1 == "no card")) {
				alert("No card detected. Configuration not allowed.");
				return;
			}
		}
		newRef="netAccessMethConfig.asp?slot="+ slot + target +"&SSID=<%get("_sid_");%>";
		newWindow(newRef,'name',width,'350','yes');
	}

    </script>
</head>

<body onload="init();" class="bodyMain">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
	<tr>
		<td align="center">
			<form name="configForm" method="POST" action="/goform/Dmf">
				<input type="hidden" name="system.req.action" value="">
				<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
				<input type="hidden" name="urlOk" value="/normal/network/netAccessMeth.asp">
				<input type="hidden" name="urlError" value="/normal/network/netAccessMeth.asp">
				<input type="hidden" name="request" value=<%write(ReqNum);%>>
				<!---------------------- elements 5-6 below---------------------------------->
				<input type="hidden" name="<%write(self);%>action" value="0">
				<input type="hidden" name="<%write(self);%>selectedOption" value="">
			</form>
		</td>
	</tr>
	<tr>
		<td align="center">
			<form name="netAccessMethForm" action="" onSubmit="return false;">
			<table border="0" cellpadding="2" cellspacing="0" align="center">
				<tr>
					<td colspan="3" align="center" height="25" valign="top">
						<font class="redBold">PCMCIA</font>
					</td>
				</tr>
				<tr class="colorTHeader">
					<td width="100" class="colorTHeader">
						<font class="tabsTextBig">Slot #</font>
					</td>
					<td width="90" class="colorTHeader">
						<font class="tabsTextBig">Card Type</font>
					</td>
					<td width="200" align="center" class="colorTHeader">
						<font class="tabsTextBig">Action</font>
					</td>
				</tr>
				<tr class="colorTRowAlt">
					<td class="colorTRowAlt">
						&nbsp;<font class="tableText">1</font>
					</td>
					<td class="colorTRowAlt">
						<font class="tableText"><% get(self + "ident", 0); %></font>
					</td>
					<td class="colorTRowAlt">
						<a href="javascript:insert(0);">
							<img src="/Images/insertButton.gif" alt="" width="47" height="21" border="0"></a>
						&nbsp;&nbsp;
						<a href="javascript:eject(0);">
							<img src="/Images/ejectButton.gif" alt="" width="47" height="21" border="0"></a>
						&nbsp;&nbsp;
						<a href="javascript:config(0);">
							<img src="/Images/configButton.gif" border="0" width="65" height="21" alt=""></a>
					</td>
				</tr>
				<tr class="colorTRow">
					<td class="colorTRow">
						&nbsp;<font class="tableText">2</font>
					</td>
					<td class="colorTRow">
						<font class="tableText"><% get(self + "ident", 1); %></font>
					</td>
					<td class="colorTRow">
						<a href="javascript:insert(1);">
							<img src="/Images/insertButton.gif" alt="" width="47" height="21" border="0"></a>
						&nbsp;&nbsp;
						<a href="javascript:eject(1);">
							<img src="/Images/ejectButton.gif" alt="" width="47" height="21" border="0"></a>
						&nbsp;&nbsp;
						<a href="javascript:config(1);">
							<img src="/Images/configButton.gif" border="0" width="65" height="21" alt=""></a>
					</td>
				</tr>
			</table>
			</form>
		</td>
	</tr>
</table>

<script type="text/javascript">
	parent.mainReady = true;
</script>

<%set("_sid_","0");%>
</body>
</html>
