<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
    </script>
</head>

<%
var width = "17%";
var family_model = getVal('_familymodel_');
if (family_model == "KVM" || family_model == "ONS") {
    width = "14%";
   }
%>

<body class="bodyMainTabs" marginheight="0" marginwidth="0">
<table border="0" cellpadding="0" cellspacing="0" width="100%">
   <tr>
    <td class="bgColor4" align="center" width=<%get(width);%> nowrap>
		<a href="javascript:loadNextPagePhysP('/normal/security/secAccessConf.asp')" class="tabsTextAlt">
		<font class="tabsText">AuthType</font></a></td>
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>
	<td class="bgColor4" align="center" width=<%get(width);%> nowrap>
		<a href="javascript:loadNextPagePhysP('/normal/security/secRadConf.asp')" class="tabsTextAlt">
		<font class="tabsText">Radius</font></a></td>  
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>
	<td class="bgColor4" align="center" width=<%get(width);%> nowrap>
		<a href="javascript:loadNextPagePhysP('/normal/security/secTacConf.asp')" class="tabsTextAlt">
		<font class="tabsText">Tacacs+</font></a></td>  
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>
	<td class="bgColor4" align="center" width=<%get(width);%> nowrap>
		<a href="javascript:loadNextPagePhysP('/normal/security/secLdapConf.asp')" class="tabsTextAlt">
		<font class="tabsText">Ldap</font></a></td>
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>
	<td class="bgColor1" align="center" width=<%get(width);%> nowrap>
		<font class="tabsText">Kerberos</font></td>
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>

<%
if (family_model == "KVM" || family_model == "ONS") {

	write("<td class=\"bgColor4\" align=center width='");
	write(width);
	write("' nowrap>");
	write("<a href=\"javascript:loadNextPagePhysP('/normal/security/secSmbConf.asp')\" class=\"tabsTextAlt\">");
	write("<font class=\"tabsText\">Smb(NTLM)</font></a></td>");
    write("<td class=\"bgColor4\" align=\"left\" height=\"20\" width=\"1\">");

	write("	<img src=\"/Images/brownLine.gif\" alt=\"\" width=\"1\" height=\"20\" border=\"0\"></td>");

}
%>
	<td class="bgColor4" align="center" width=<%get(width);%> nowrap>
		<a href="javascript:loadNextPagePhysP('/normal/security/secNisConf.asp')" class="tabsTextAlt">
		<font class="tabsText">NIS</font></a></td>

   </tr>
   <tr>
      <td class="bgColor3" width=<%get(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" width=<%get(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" width=<%get(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" width=<%get(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor1" width=<%get(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" width=<%get(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" width=<%get(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
   </tr>
</table>
</body>
</html>

