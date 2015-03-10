<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: UserList.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title>DMF Tutorial: <%get("_chapter_");%></title>


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

<h1>User Management</h1>

<h2>User List</h2>
<p>Click on a user name to edit that user's settings.</p>
<table border>
<tr>
   <td>User Name</td>
   <td>Password</td>
   <td>Group</td>
   <td>Enabled</td>
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
      write("Delete</a></td>\n");
      
      write("</tr>\n");   
   }
%>

</table>


<a href="/admin/EditUser.asp">Add new user</a>


<!-- a hidden form to delete a user - works with clientside JavaScript -->
<form name="deleteForm" method="POST" action="/goform/Dmf">
<input type="hidden" name="name" value="">
<input type="hidden" name="command" value="DELETEUSER">
<input type="hidden" name="urlOk"
 value="/admin/UserList.asp">
<input type="hidden" name="urlError"
 value="/admin/UserList.asp">
</form>


</html>
