<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: SystemInfo.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<!--	CH08: Literal text removed from ASP source, replaced with calls to
		getLabel() and getLabelVal() as appropriate.
-->

<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
</head>


<h1><%getLabel("system.info");%></h1>

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
%>
      


<form method="POST" action="/goform/Dmf">
<%getLabel("system.name");%>:   <input size="40" name = "system.info.name" 
 value="<%get("system.info.name");%>"><br />

<%getLabel("description");%>:   <input size="40" name="system.info.description" 
 value="<%get("system.info.description");%>"><br />

<%getLabel("uptime");%>:        <%get("system.info.uptime");%> seconds<br />

<input type="hidden" name="urlOk" value="/user/SystemInfo.asp">
<input type="hidden" name="urlError" value="/user/SystemInfo.asp">
<input type="hidden" name="request" value=<%write(getRequestNumber());%>>
<input type="submit" name="Submit" value="Ok">
</form>

</html>
