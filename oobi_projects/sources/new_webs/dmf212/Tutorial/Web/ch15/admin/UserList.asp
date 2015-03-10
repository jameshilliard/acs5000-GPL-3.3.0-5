<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: UserList.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<!--	CH08: Literal text removed from ASP source, replaced with calls to
		getLabel() and getLabelVal() as appropriate.
-->

<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
</head>


<script langauge="JavaScript">
<!-- hide me
function deleteUser(name)
{
   if (confirm("Really delete " + name + "'s account?"))
   {
      window.document.deleteForm.name.value = name;
      window.document.deleteForm.submit();
   }
}

// -->
</script>

</head>
<body bgcolor=<%get("_bgcolor");%>>
<h1><%getLabel("user.management");%></h1>

<h2><%getLabel("user.list");%></h2>
<p><%getLabel("user.instructions");%></p>
<table border>
<tr>
   <td><%getLabel("user.name");%></td>
   <td><%getLabel("password");%></td>
   <td><%getLabel("group");%></td>
   <td><%getLabel("enabled");%></td>
   <td />
</tr>
<%
   for (name = getNextUser(); __success__; name = getNextUser(name))
   {
      write('<tr><td><a href="/admin');
      write('/EditUser.asp?name=' + name + '">');
      write( name + '</a></td>\n');
      password = getUserDetail(name, "password");
      write('<td>' + password + '</td>\n');
      group = getUserDetail(name, "group");
      write('<td>' + group + '</td>\n');
      enabled = getUserDetail(name, "enabled");
      write("<td>");
      if (enabled)
      {
         write("*");
      }
      write("</td>\n");
      write("<td>");
      write('<a href="#" onClick="deleteUser(\'' + name + '\'); return false;">');
      write(getLabelVal("delete") + "</a></td>\n");
      
      write("</tr>\n");   
   }
%>

</table>


<a href="/admin/EditUser.asp"><%getLabel("add.user");%></a>


<!-- a hidden form to delete a user - works with clientside JavaScript -->
<form name="deleteForm" method="POST" action="/goform/Dmf">
<input type="hidden" name="name" value="">
<input type="hidden" name="command" value="DELETEUSER">
<input type="hidden" name="urlOk"
 value="/admin/UserList.asp">
<input type="hidden" name="urlError"
 value="/admin/UserList.asp">
</form>

</body>
</html>
