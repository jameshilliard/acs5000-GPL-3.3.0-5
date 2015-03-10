<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: home.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<!--	CH08: Literal text removed from ASP source, replaced with calls to
		getLabel() and getLabelVal() as appropriate.
-->

<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
</head>

<a href="/user/SystemInfo.asp"><%getLabel("system.info");%></a>
<br />
<a href="/user/Values.asp"><%getLabel("data.values");%></a>

<%
   if (userCanAccess("admin"))
   {
      write('<br /><br /><a href="/admin');
      write('/UserList.asp">' + getLabelVal("user.management") + '</a>\n');
   }
%>
</html>
