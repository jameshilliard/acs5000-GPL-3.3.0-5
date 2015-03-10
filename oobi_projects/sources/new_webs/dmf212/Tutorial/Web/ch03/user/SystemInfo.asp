<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: SystemInfo.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->
<html>
<head>
<%language=javascript%>
<title>DMF Tutorial - System Information</title>
</head>

<h1>System Information</h1>
System Name:   <%get("system.info.name");%><br />
Description:   <%get("system.info.description");%><br />
Uptime:        <%get("system.info.uptime");%> seconds<br />


</html>
