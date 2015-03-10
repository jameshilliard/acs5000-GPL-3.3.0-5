<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
	var user = 'user2'; 
    </script>
</head>

<%
	var width = "25%";
	if (getVal("system.device.kvmanalog") != 0)
		width = "34%";
%>

<body class="bodyMainTabs" marginheight="0" marginwidth="0">
<table border="0" cellpadding="0" cellspacing="0" width="100%">
   <tr>
    <td class="bgColor4" align="center" width=<%get(width);%> nowrap>
		<a href="javascript:loadNextPage('/normal/kvm/configKVMGeneral.asp')" class="tabsTextAlt">
		<font class="tabsText">General</font></a></td>
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>
	<td class="bgColor4" align="center" width=<%get(width);%> nowrap>
		<a href="javascript:loadNextPage('/normal/kvm/configKVMGeneralUser1.asp')" class="tabsTextAlt">
		<font class="tabsText">User 1</font></a></td>  
    <td class="bgColor4" align="left" height="20" width="1">
		<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>
	<td class="bgColor1" align="center" width=<%get(width);%> nowrap>
		<font class="tabsText">User 2</font></td>  
<%
if (getVal("system.device.kvmanalog") == 0){
	write('<td class="bgColor4" align="left" height="20" width="1">\n');
	write('  <img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>\n');
	write('<td class="bgColor4" align="center" width="25%" nowrap>\n');
	write('<a href="javascript:loadNextPage(\'/normal/kvm/configKVMGeneralWebUser.asp\')" class="tabsTextAlt">\n');
	write('         <font class="tabsText">IP Users</font></a></td>\n');
}
%>
   </tr>
   <tr>
      <td class="bgColor3" width="25%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
      <td class="bgColor3" width="25%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor1" width="25%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
	  <td class="bgColor3" width="25%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
   </tr>
</table>
</body>
</html>

