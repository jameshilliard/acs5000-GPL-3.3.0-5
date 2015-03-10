<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<meta http-equiv="refresh" content="3;url=javascript:timeout();">
<script type="text/javascript">
	var sniff = 0;

	function timeout()
	{
		if (sniff == 0) {
			document.logout.submit();
		}
	}
</script>
</head>

<title><%getLabel("title");%>: Saving Changes...</title>
<body class="bodyMain">
<p>Saving Changes &amp; Restarting Web Server.  Please Wait...</p>
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
