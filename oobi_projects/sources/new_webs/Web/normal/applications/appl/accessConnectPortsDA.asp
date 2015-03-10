<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<meta http-equiv="refresh" content="5;url=javascript:timeout();">
    <%
        // Set page-wide variables:
        var portName = getQueryVal("name", 0);
        var sniffopt = getQueryVal("sniffopt", 0);
        var otherstation = getQueryVal("otherstation", 0);
	var ReqNum = getRequestNumber();
    %>
<script type="text/javascript">
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	var sniff = 0;

	function timeout()
	{
		if (sniff == 0) {
<%
var family_ext = getVal('_familyext_');
if (family_ext == "KVMNETP") {
	write('document.logout.urlOk.value = \'/normal/empty.asp?SSID=' + getVal("_sid_") + '\';\n');
	write('document.logout.elements[3].value = \'1\';\n');
}
%>
			document.logout.submit();
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
		document.logout.urlOk.value = 'normal/applications/appl/accessConnectPortsDA.asp?name=<%write(portName);%>&SSID=<%get("_sid_");%>&otherstation=' + document.connectForm.station.value + '&sniffopt=' + sniffopt;
		document.logout.elements[3].value = '1';
   		document.logout.submit();
	}
</script>
</head>

<title><% write(portName); %></title>
<body class="bodyMain">
    <% showViewerHtml(); %>
   <form name="logout" method="POST" action="/goform/Dmf" target="_top">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/redirect.asp?to=login">
   <input type="hidden" name="urlError" value="/redirect.asp?to=login">
   <input type="hidden" name="system.req.logout" value="">
   <input type="hidden" name="request" value=<%write(getRequestNumber());%>>
   </form>
</body>
<% set("_sid_","0");%>
</html>
