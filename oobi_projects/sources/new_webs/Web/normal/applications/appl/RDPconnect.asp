<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
    <%
        // Set page-wide variables:
        var portName = getQueryVal("portname", 0);
        var kvmip = getQueryVal("rdpaddr", 0);
        var rdpport = getQueryVal("rdpport", 0);
        var restart = getQueryVal("start", 0);
        var ident = getQueryVal("ident", 0);
        var sid = getQueryVal("SSID", "0");
    %>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

function init() {
	location.replace(
	"/normal/applications/appl/RDPconnect.asp" +
	"?portname=<%write(portName);%>&start=no&ident=<%write(ident);%>" + 
	"&rdpport=<%write(rdpport);%>&rdpaddr=<%write(kvmip);%>"+
	"&SSID=<%write(sid);%>");
}

function launchRDP() {
	setTimeout('init()', 5000);
	if ("<%write(restart);%>" != "yes") {
		return;
	}
	parent.RDPcontrol.launchMSRDP();
}

function RDPerror(msg) {
	alert("<%getLabel("prodname");%>: " + msg);
	if ("<%write(restart);%>" != "yes") {
	        parent.RDPcontrol.closeMSRDP();
	}
	parent.window.close();
}
</script>
                                                                        
</head>

<body <% checkRDPconn(); %>>
</body>
</html>

