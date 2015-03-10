<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title><%getLabel("title");%></title>
   <script type="text/javascript">
   <%
         var IPv6 = getVal('_IPv6_');
         var enable4 = 0;
         var enable6 = 0;
         if (IPv6) {
            var enable4 = getVal("system.netSettings.enableIPv4");
            var enable6 = getVal("system.netSettings.enableIPv6");
         }
   %>
      var topMenuReady = 1; // there is no top-menu in the wizard
      var menuReady = 0;
      var controlsReady = 0;
	  var helpPage = 0;
	  var mainReady;
      var family_model = "<%var family_model=getVal('_familymodel_'); write(family_model);%>";
      var sid = "<%write(getNewSid(getVal('_sid_')));%>";
      var headMenuPages = new Array;
      var IPmode = (<%write(IPv6);%>)? (<%write(enable4);%> + 2*<%write(enable6);%>) : 1;
      var oldIPmode = IPmode;
   </script>
</head>
<!-- frames -->
<frameset  rows="*,590,*" framespacing="0" frameborder="0" border="0">
    <frame name="topBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
    <frameset  cols="*,900,*" framespacing="0" frameborder="0" border="0">
        <frame name="leftBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
        <frameset  rows="98,*,102" cols="202,*" framespacing="0" frameborder="0" border="0">
            <frame name="logo" src="/logo.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
            <frame name="top" src="/top.asp?SSID=<%write(getVal('_sid_'));%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
            <frame name="menu" src="menu.asp?SSID=<%write(getVal('_sid_'));%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
            <frameset  cols="*,55" framespacing="0" frameborder="0" border="0">
                <frameset  rows="6,*,6" framespacing="0" frameborder="0" border="0">
                    <frame name="frameTop" src="/frameTop.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                    <frameset  cols="6,*,6" framespacing="0" frameborder="0" border="0">
                        <frame name="frameLeft" src="/frameLeft.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                        <frame name="main" src="<%if (family_model != "AVCS") write('secProfile.asp'); else write('netSettings.asp');%>?SSID=<%write(getVal('_sid_'));%>" marginwidth="0" marginheight="0" scrolling="auto" frameborder="0" noresize>
                        <frame name="frameRight" src="/frameRight.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                    </frameset>
                    <frame name="frameBottom" src="/frameBottom.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                </frameset>
                <frame name="rightMargin" src="/rightMargin.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
            </frameset>
            <frame name="menuBottom" src="menuBottom.asp?SSID=<%write(getVal('_sid_'));%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
            <frame name="controls" src="controls.asp?SSID=<%write(getVal('_sid_'));%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
        </frameset>
        <frame name="rightBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
    </frameset>
    <frame name="bottomBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
</frameset>
</html>
