<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script language="JavaScript" src="/menu.js" type="text/javascript"></script>
	<script language="JavaScript">
	
		<%
		// Set page-wide variables:
		var ip = getVal("system.device.ip");
		var hostname = getVal("system.device.hostname");
		var model = getVal("system.device.model");
		var familyModel = getVal('_familymodel_');
		var accessPage;
		var hostPage;
		if (getVal('_IPv6_')) {
			hostPage = "netHost";
		} else {
			hostPage = "netHostSettings";
		}
	
		if (familyModel == "ONS")
			accessPage = "/normal/applications/appsConnect.asp";
		else {
			accessPage = "/normal/applications/accessConnect.asp";
			if (getVal("system.device.kvmanalog") != 0)
				accessPage = "/normal/applications/appsPMOutletsMan.asp";
		}
		%>
	
		var previousTextHigh;
		var previousText;
	
		if ((parent.family_model != "ACS")&&(parent.family_model != "AVCS")) {
			var menuPages = new Array (
			new menuObj("ACC", "Access", "<%write(accessPage);%>", 20),
			new menuObj("CON", "Configuration", '/normal/kvm/configKVMGeneral.asp', 95),
			new menuObj("INF", "Information", '/normal/information/infoGeneral.asp', 220),
			new menuObj("MGM", "Management", '/normal/administration/adminBackupConfig.asp', 325)
			);
		} else {
			var menuPages = new Array (
			new menuObj("APP", "Applications", "/normal/applications/appsConnect.asp", 0),
			new menuObj("NET", "Network", "/normal/network/<%write(hostPage);%>.asp", 108),
			new menuObj("PWD", "Security", "/normal/security/secUsersGroups.asp", 185),
			new menuObj("PORT", "Ports", "/normal/ports/portsStatus.asp", 262),
			new menuObj("ADM", "Administration", "/normal/information/infoGeneral.asp", 320)
			);
		}
	
		function init() {
			parent.topMenuReady = 1;
			for (var i=0; i < menuPages.length; i++) {
				show ('document', 'text' + i);
			}
		}
	
		function selectItem(name) {
	
			page = getPageIndex(name, menuPages);
	
			if (page == -1) return;
	
			selectedTextHigh="textHigh" + page;
			selectedText="text" + page;
	
			if (selectedTextHigh != previousTextHigh) {
	
				showItem(selectedTextHigh);
				showItem(selectedText);
	
				if (previousTextHigh != null && previousText != null) {
					hideSelected(previousTextHigh);
					showItem(previousText);
				}
	
				previousTextHigh = selectedTextHigh;
				previousText = selectedText;
			}      
	
		}
	
		function hideSelected(object) {
			if (type == 'NN') {
				if (this.document.layers[object] != null)
					this.document.layers[object].visibility = 'hidden';
			} else if (type == 'MO')
				document.getElementById(object).style.visibility = "hidden";   
			else if (type == 'IE')
				this.document.all[object].style.visibility = 'hidden';
		}
	 
	   
		function showItem(object) {
			if (object != previousTextHigh && object != previousText) {
				if (type == 'NN') {
					if (this.document.layers[object] != null)
						this.document.layers[object].visibility = 'visible';
				} else if (type == 'MO')          
					document.getElementById(object).style.visibility = "visible";   
				else if (type == 'IE')
					this.document.all[object].style.visibility = 'visible';
			}
		}
	
		function hideItem(object) {
			if (object != previousTextHigh && object != previousText) {
				if (type == 'NN') {
					if (this.document.layers[object] != null)
						this.document.layers[object].visibility = 'hidden';
				} else if (type == 'MO')          
					document.getElementById(object).style.visibility = "hidden";   
				else if (type == 'IE')
					this.document.all[object].style.visibility = 'hidden';
			}
		} 
	
	</script>
</head>

<body onload="init();" class="bgColor8" bottommargin="0" topmargin="0" leftmargin="0" rightmargin="0" marginheight="0" marginwidth="0">   
	<table border="0" cellpadding="0" cellspacing="0" width="100%">					
		<tr>
			<td background="/Images/topBackground.gif" height="22" align="left" width="100%"><img src="/Images/topBackground.gif" alt="" width="5" height="22" border="0"></td>
			<td background="/Images/topBackground.gif" height="22" align="right"><a href="javascript:document.logout.submit();"><img src="/Images/logout1.gif" alt="" width="142" height="22" border="0"></a></td>
		</tr>
	</table>
	<table border="0" cellpadding="0" cellspacing="0" width="100%">
		<tr>
			<td align="center" valign="middle" width="600"><img src="/Images/1pixelTrans.gif" alt="" width="426" height="1" border="0"></td>
			<td align="left" width="153" valign="middle" nowrap>
				<font class="IPGatewayText">Host Name: <% write(hostname); %></font><br>
<%if (getVal('_IPv6_') == 0) {
write('\
				<font class="IPGatewayText">IP Address: ' + ip + '</font><br>
');}%>
				<font class="IPGatewayText">Model: <% write(model); %></font><br>
				<form name="logout" method="POST" action="/goform/Dmf" target="_top">
					<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
					<input type="hidden" name="urlOk" value="/redirect.asp?to=login">
					<input type="hidden" name="urlError" value="/redirect.asp?to=login">
					<input type="hidden" name="system.req.logout" value="">
					<input type="hidden" name="request" value=0>
				</form>					
			</td>
			<td width="6" height="100%" align="right">
				<table border="0" cellpadding="0" cellspacing="0" height="76">
					<tr>
						<td class="bgColor8" width="100%">
							<img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0">
						</td>
						<td background="/Images/rightEdge.gif" width="2" height="100%"><img src="/Images/rightEdge.gif" alt="" width="6" height="6" border="0"></td>
					</tr>
				</table>
			</td>
		</tr>
	</table>  

	<script language="JavaScript">
	<!-- Begin Hide
		print_top_table_menu(50, menuPages);
	// End Hide -->
	</script>
</body>
</html>
