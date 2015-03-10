<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script language="JavaScript">

   // Decide to whom we're forwarding, then do it.
   var to = '<%write(getQueryVal("to", ""));%>';
   var sid = '?SSID=<%write(getQueryVal("SSID", 0));%>';
   var page;

   if (to == "wizard")
      page = "/wizard/main.asp" + sid;
   else if (to == "expert")
      page = "/normal/main.asp" + sid + "&x=e";
   else
      page = "/login.asp" + sid;

   //top.window.location.replace(page);
   top.location.href = page;

   </script>
</head>

<body class="bodyMain">
</body>
</html>
