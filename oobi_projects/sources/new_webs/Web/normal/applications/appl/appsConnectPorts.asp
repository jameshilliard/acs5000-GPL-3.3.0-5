<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <%
        // Set page-wide variables:
        var portName = getQueryVal("name", 0);
        var id = getQueryVal("id", 0);
    %>

   </script> 
</head>

<title><% write(portName); %></title>
<body>
<table border="0" align="center">
 <tr>
  <td>
   <div align="right"><a href="javascript:window.location.reload()">
     <img src="/Images/refresh.gif" alt="Reconnect" title="Reconnnect" border="0"
     width="13" height="16"></a>
   </div>
  </td>
  <td width="3%">
   <div align="left"><a href="javascript:window.close()">
     <img src="/Images/close.gif" alt="Close" title="Close" border="0" width="13" height="16"></a>
   </div>
  </td>
 </tr>
 <tr>
  <td colspan="2">
    <% showAppletHtml(id);
	// check user timeout error
	if (__success__ != 1) {
		write("Your web session has timed out. Close this window and log in again.");
	}
     %>
  </td>
 </tr>
</table>
</body>
</html>
