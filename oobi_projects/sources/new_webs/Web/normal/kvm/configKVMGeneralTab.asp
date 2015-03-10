<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>

<%
    var tab = getQueryVal("tab", 0);
	var width = "25%";
    var user1 = "User 1";
    var family_model = getVal('_familymodel_');
	if (getVal("system.device.kvmanalog") != 0)
		width = "34%";
    if (family_model == "ONS") {
		width = "34%";
		user1 = "Local User";
    }
%>

<script type="text/javascript">
	var user = "<%write(tab);%>";
</script>

</head>

<body class="bodyMainTabs" marginheight="0" marginwidth="0">
<table border="0" cellpadding="0" cellspacing="0" width="100%">
   <tr>
<% if (tab == "gen") bg="bgColor1"; else bg="bgColor4"; %>
    <td class=<%write(bg);%> align="center" width=<%write(width);%> nowrap>
		<a href="javascript:loadNextPage('/normal/kvm/configKVMGeneral.asp')" class="tabsTextAlt">
		<font class="tabsText">General</font></a></td>
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>
<% if (tab == "u1") bg="bgColor1"; else bg="bgColor4"; %>
	<td class=<%write(bg);%> align="center" width=<%write(width);%> nowrap>
		<a href="javascript:loadNextPage('/normal/kvm/configKVMGeneralUser1.asp')" class="tabsTextAlt">
		<font class="tabsText"><%write(user1);%></font></a></td>  
<% if (family_model != "ONS") {
    write('<td class="bgColor4" align="left" height="20" width="1"><img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>');
    if (tab == "u2") bg="bgColor1"; else bg="bgColor4";
	write('<td class="');
    write(bg);
    write('" align="center" width="');
    write(width);
    write('" nowrap><a href="javascript:loadNextPage(');
    write("'/normal/kvm/configKVMGeneralUser2.asp'");
    write(')" class="tabsTextAlt"><font class="tabsText">User 2</font></a></td>');
} %>

<%
if (getVal("system.device.kvmanalog") == 0){
	write('<td class="bgColor4" align="left" height="20" width="1">\n');
	write('  <img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>\n');
   if (tab == "ip") bg="bgColor1"; else bg="bgColor4";
	write('<td class="');
    write(bg);
    write('" align="center" width="');
    write(width);
    write('" nowrap>\n');
	write('<a href="javascript:loadNextPage(\'/normal/kvm/configKVMGeneralWebUser.asp\')" class="tabsTextAlt">\n');
	write('		<font class="tabsText">IP Users</font></a></td>\n');
}
%>
   </tr>
   <tr>
      <td class="<% if (tab == "gen") write("bgColor1"); else write("bgColor3");%>" width=<%write(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="<% if (tab == "u1") write("bgColor1"); else write("bgColor3"); %>" width=<%write(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
   <% if (family_model != "ONS") {
      if (tab == "u2") bg="bgColor1"; else bg="bgColor3";
      write('<td class="'); write(bg); write('" width='); write(width); write('><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');
      write('<td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');
      }
   %>
      <td class="<% if (tab == "ip") write("bgColor1"); else write("bgColor3"); %>" width=<%write(width);%>><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
   </tr>
</table>
</body>
</html>

