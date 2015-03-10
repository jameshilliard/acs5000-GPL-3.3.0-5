<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<%
	var portName = getQueryVal("portname", 0);
	var kvmip = getQueryVal("rdpaddr", 0);
	var rdpport = getQueryVal("rdpport", 0);
	var start = getQueryVal("start", 0);
	var ident = getQueryVal("ident", 0);
	var sid = getQueryVal("SSID", "0");
%>
	<title><% getLabel("title");%> - RDP Server: <% write(portName); %></title>
</head>

<!-- frames -->
<frameset  rows="100%,0%" framespacing="0" frameborder="0" border="0">
    <frame name="RDPcontrol" src="RDPcontrol.asp?portname=<%write(portName);%>&rdpport=<%write(rdpport);%>&rdpaddr=<%write(rdpaddr);%>&SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="yes" frameborder="0">
    <frame name="RDPconnect" src="RDPconnect.asp?portname=<%write(portName);%>&start=<%write(start);%>&ident=<%write(ident);%>&rdpport=<%write(rdpport);%>&rdpaddr=<%write(rdpaddr);%>&SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" >
</frameset>

</html>
