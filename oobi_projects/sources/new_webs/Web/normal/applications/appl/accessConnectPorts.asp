<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<!--meta http-equiv="refresh" content="5;url=/normal/applications/accessConnect.asp"-->
    <%
        // Set page-wide variables:
        var portName = getQueryVal("name", 0);
        var sniffopt = getQueryVal("sniffopt", 0);
        var otherstation = getQueryVal("otherstation", 0);
        var RC = getQueryVal("rc", 0);
    %>
	<title><% write(portName); %></title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">
	var sniff = 0;
	function submit() //it is also automatically called when clicking the left menu (it must exist!!!)
	{
		setQuerySSID(document.configForm);
		document.configForm.submit();
		top.mainReady = null;
	}

	function timeout()
	{
		if (sniff == 0) {
			submit();
		}
	}

	function connect()
	{
		var sniffopt = 0;
		for (i = 0; i < 4; i ++) {
			if (document.connectForm.option[i].checked == true) {
				sniffopt = document.connectForm.option[i].value;
				break;
			}
		}
		document.configForm.urlOk.value = "normal/applications/appl/accessConnectPorts.asp?name=<%write(portName);%>&rc=<%write(RC);%>&otherstation=" + document.connectForm.station.value + "&sniffopt=" + sniffopt;
		setQuerySSID(document.configForm);
   		document.configForm.submit();
	}
	</script>
</head>

<body onload="setTimeout('timeout()', 5000);" class="bodyMain">

    <% showViewerHtml(); %>

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

