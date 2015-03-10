<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
    <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
   <%
      // Set page-wide variables:
      device = "system.device.";
      var ip = getVal("system.device.ip");
      var hostname = getVal("system.device.hostname");
	  var model = getVal("system.device.model");
   %>

   function init()
   {
      parent.topMenuReady = 1;
   }

   function selectItem(name) {}

   </script>
</head>
<body onload="init();" class="bgColor5" bottommargin="0" topmargin="0" leftmargin="0" rightmargin="0" marginheight="0" marginwidth="0">
<table border="0" cellpadding="0" cellspacing="0" width="100%">
        <tr>
            <td background="/Images/topBackground.gif" height="22" align="left" width="100%">
                <img src="/Images/topBackground.gif" alt="" width="5" height="22" border="0"></td>
            <td background="/Images/topBackground.gif" height="22" align="right">
						<a href="javascript:document.logout.submit();"><img src="/Images/logout1.gif" alt="" width="142" height="22" border="0"></a></td>
        </tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" width="100%">
		<tr>
            <td align="center" valign="middle" width="100">&nbsp;</td>
			<td align="center" valign="middle" width="500"><font class="topMenuText"><% getLabel("title"); %> - Regular User</font></td>
			<td align="left" width="160" valign="middle" nowrap>
                 <font class="IPGatewayText">Host Name <% write(hostname); %></font><br>
                 <font class="IPGatewayText">IP Address <% write(ip); %></font><br>
				 <font class="IPGatewayText">Model: <% write(model); %></font><br> 
		<form name="logout" method="POST" action="/goform/Dmf" target="_top">
   			<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   			<input type="hidden" name="urlOk" value="/login.asp">
   			<input type="hidden" name="urlError" value="/login.asp">
   			<input type="hidden" name="system.req.logout" value="">
  			<input type="hidden" name="request" value=0>				   
		</form>			
            </td>
            <td width="6" height="100%" align="right">
            <table border="0" cellpadding="0" cellspacing="0" height="76">
                    <tr>
                        <td class="bgColor5" width="100%">
                        <img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0">
                        </td>
                        <td background="/Images/rightEdge.gif" width="2" height="100%"><img src="/Images/rightEdge.gif" alt="" width="6" height="6" border="0"></td>
                    </tr>
            </table>
            </td>
        </tr>
    </table>
</body>
</html>
