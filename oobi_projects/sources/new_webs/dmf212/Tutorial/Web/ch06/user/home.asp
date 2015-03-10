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
   /* CH06: If the currently logged in user has admin access, show
    * them a link to get to the user management page...
    */
   if (userCanAccess("admin"))
   {
      write('<br /><a href="/admin/UserList.asp">User Management</a>');
   }
%>
</html>
