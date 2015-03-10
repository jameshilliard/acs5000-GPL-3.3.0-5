<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: home.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
</head>
<body bgcolor=<%get("_bgcolor");%>>

<a href="/user/SystemInfo.asp"><%getLabel("system.info");%></a>
<br />
<a href="/user/Values.asp"><%getLabel("data.values");%></a>

<%
   if (userCanAccess("admin"))
   {
      write('<br /><br /><a href="/admin');
      write('/UserList.asp">' + getLabelVal("user.management") + '</a>\n');
      write('<br /><br /><a href="/admin');
      write('/Config.asp">' + getLabelVal("configure.device") + '</a>\n');

   }
%>
</body>
</html>
