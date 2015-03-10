<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: home.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title>DMF Tutorial: <%get("_chapter_");%></title>
<head>

<a href="/user/SystemInfo.asp">System Information</a>
<br />
<a href="/user/Values.asp">Data Values</a>

<%
   if (userCanAccess("admin"))
   {
      write('<br /><br /><a href="/admin');
      write('/UserList.asp">User Management</a>\n');
   }
%>
</html>
