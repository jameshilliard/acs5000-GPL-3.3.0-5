<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title>Untitled</title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
</head>
<body class="bodyMain">
<table border="0" cellpadding="0" cellspacing="0" align="center" height="100%" width="100%">
   <tr valign="middle">
      <td>
    </td>
   </tr>
   <tr valign="middle">
      <td>
      </td>
   </tr>
<%
var family_ext = getVal('_familyext_');
if (family_ext == "KVMNETP") {
   write('<tr valign="bottom" align="right"><td>\n');
   write(' <a href="javascript:newWindow(\'/normal/applications/accessConnectPopUp.asp?tabx=0&SSID=' + getVal("_sid_") + '\', \'kvmcontrol\', 600, 400, \'yes\');">Show Connections</a>\n');
   write('</td></tr>\n');
}
%>
</table>
<%set("_sid_","0");%>
</body>
</html>
