<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<!--meta http-equiv="refresh" content="5;url=/normal/applications/accessConnect.asp"-->
    <%
        // Set page-wide variables:
        var portName = getQueryVal("name", 0);
        var connect = getQueryVal("connect", 0);
        var RC = getQueryVal("rc", 0);
    %>
	<title><% getLabel("title");%></title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>

	<script type="text/javascript">
	function submit() {
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = null;
	}

	function KVMconnect(type, code) {
		switch (code) {
			case 0:
			alert("RDP connection failed.\nInvalid Server parameters.");
			break;
			case 1:
			alert("RDP connection failed.\nUndefined Server address.");
			break;
			case 2:
			alert("RDP connection failed. Please verify Port and IP.");
			break;
			case 3:
			alert("RDP connection already exists.\n");
			break;
			case 4:
			alert("RDP connections limit exceeded.\n");
			break;
		}
		if (type == 2) {
			Url = "/normal/applications/appl/accessConnectPorts.asp" + 
    	   			"?name=<%write(portName);%>&rc=<%write(RC);%>";
			submitForm(self, Url);
		} else {
			submit();
		}
	}

	function RDPconnect(pn, ip, port, ident) {
		Url = "/normal/applications/appl/RDPviewer.asp" +
			"?SSID=<%get("_sid_");%>" +
			"&portname=" + pn + "&start=yes" + 
			"&ident=" + ident + 
			"&rdpport=" + port + "&rdpaddr=" + ip;
		newWindow(Url, ident, 1024, 768, 'yes');
		submit();
	}	

	</script>
</head>

<body <% showRDPHtml(); %> class="bodyMain" >

<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/applications/<%write(RC);%>">
	<input type="hidden" name="urlError" value="/normal/applications/<%write(RC);%>">
	<input type="hidden" name="request" value="0">
	<input type="hidden" name="system.tablePage.bogus" value="">
</form>
<script type="text/javascript">
	top.mainReady = true;
</script>
</body>
<% set("_sid_","0");%>
</html>

