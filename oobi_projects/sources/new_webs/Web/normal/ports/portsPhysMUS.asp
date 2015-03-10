<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<title>Access</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">
	
      function init()
         {
            if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady)
            {
               setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
            }
            else
                setTimeout('init()', 200);
         }
</script>
</head>
<!-- frames -->
<% var sid=getQueryVal("SSID","0"); %>
<frameset  rows="22,*,26" border="0" frameborder="0">
    <frame name="tabs" src="portsPhysMUSTab.asp?SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
    <frame name="page" src="portsPhysMUSMod.asp?SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="auto" frameborder="0" noresize>
	<frame name="done" src="portsPhysDone.asp?SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
</frameset>
</html>
