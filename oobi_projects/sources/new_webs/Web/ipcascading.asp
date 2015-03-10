<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<!--meta http-equiv="refresh" content="5;url=/normal/access/accessConnect.asp"-->
    <%
        // Set page-wide variables:
        var portName = getQueryVal("portname", 0);
        var master = getQueryVal("master", "");
        var sid = getQueryVal("sid", "");
    %>
	<title><% write(portName); %></title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

	function submit() //it is also automatically called when clicking the left menu (it must exist!!!)
	{
		location.replace("<%write(master);%>/goform/Dmf?system.req.action=0&system.req.sid=<%write(sid);%>&urlOk=normal/access/accessConnect.asp&urlError=normal/access/accessConnect.asp&request=0&system.tablePage.bogus=");
	}

	</script>
</head>

<body onload="setTimeout('submit()', 5000);" class="bodyMain">
<p><% showIPCascadeHtml(); %></p>
</body>
</html>
