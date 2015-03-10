<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: EditUser.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<!--	CH08: Literal text removed from ASP source, replaced with calls to
		getLabel() and getLabelVal() as appropriate.
-->

<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
</head>
<body bgcolor=<%get("_bgcolor");%>>
<!--  NOTE: We use this page for the case where we are adding a new user, as
      well as when we're editing an existing user. When this page is passed a
      query string like
      EditUser.asp?name=someUserName
      we grab the data and allow the user to edit it. 
      If there's no query string, we fill in with blanks and defaults as
      appropriate.
-->      
<%
   count = getErrorCount();
   if (count > 0)
   {
      write("<h2>");
      write('<font color="red">' + getLabelVal("errors.head") + '</font>');
      write("</h2>\n");
      write("<p>");
      getLabel("errors.instruction");
      write("</p>\n");

      for (i = 0; i < count; i++)
      {
         if (count > 1)
         {
            write((i + 1) + ": ");
         }
         write(getErrorMessage(i) + "<br />\n");
      }
   }   
   name = getQueryVal("name", "");
   editing = (name != "");

   write("<h2>");
   if (editing)
   {
      getLabel("edit.user");
      userCommand = "MODIFYUSER";
   }
   else
   {
      getLabel("add.user");
      userCommand = "ADDUSER";
   }
   write("</h2>");




   write("<form method=\"POST\" action=\"/goform/Dmf\">\n");
   if (editing)
   {
      write("<h2>" + name + "</h2>\n");
      password = getUserDetail(name, "password");
      group = getUserDetail(name, "group");
      enabled = getUserDetail(name, "enabled");

   }
   else
   {
      write(getLabelVal("user.name") + ": ");
      password = "";
      group = "users";
      enabled = "1";
   }
   write('<input type="');
   if (editing)
   {
      write('hidden" value="' + name);
   }
   else
   {
      write("text");
   }
   write('" name="name"><br />\n');
   write(getLabelVal("password") + ': <input type="password" name="password" value="');
   write(password + '"><br />\n');
   write(getLabelVal("group") + ': <select name="group">\n');
   /* get the group names, and set the selected one... */
   for (groupName = getNextGroup(); __success__;\
    groupName = getNextGroup(groupName))
   {
      write("<option");
      if (group == groupName)
      {
         write(" selected");
      }
      write(">" + groupName + "</option>\n");
   }
   write("</select><br />\n");
   /* set whether the user is enabled or not...*/
   write(getLabelVal("enabled") + ': <input name="enabled" type="checkbox"');
   if (enabled)
   {
      write(" checked ");
   }
   write("><br />\n");
   
   write('<input type="hidden" name="command" value="');
   write(userCommand + '">\n');
   write('<input type="hidden" name="urlOk" value="');
   write('/admin/UserList.asp">\n');
   write('<input type="hidden" name="urlError" value="');
   write('/admin/EditUser.asp">\n');
   write('<input type="submit" name="submit" value="Ok">\n');
   write("</form>");
%>

</body>
</html>
