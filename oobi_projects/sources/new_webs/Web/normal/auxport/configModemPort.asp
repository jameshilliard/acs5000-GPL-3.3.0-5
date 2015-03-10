<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>General</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
</head>
<!-- frames -->
<% var sid=getQueryVal("SSID","0"); %>
<frameset  rows="22,*" border="0" frameborder="0">
    <frame name="tabs" src="configModemPortTab.asp?SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
    <frame name="page" src="configAuxPortMod.asp?port=2&SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="auto" frameborder="0" noresize> 
</frameset>
</html>
