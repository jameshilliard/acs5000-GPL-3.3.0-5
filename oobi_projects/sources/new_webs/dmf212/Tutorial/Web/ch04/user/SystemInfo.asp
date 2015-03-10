<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: SystemInfo.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->
<html>
<head>
<%language=javascript%>
<title>DMF Tutorial - System Information</title>
</head>

<h1>System Information</h1>

<% 
   count = getErrorCount();
   if (count > 0)
   {
      write("<h2>");
      write('<font color="red">Errors</font>');
      write("</h2>\n");
      write("<p>");
      write("Please correct the following error(s) and resubmit the form.");
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
System Name:   <input size="40" name = "system.info.name" 
 value="<%get("system.info.name");%>"><br />

Description:   <input size="40" name="system.info.description" 
 value="<%get("system.info.description");%>"><br />

Uptime:        <%get("system.info.uptime");%> seconds<br />

<input type="hidden" name="urlOk" value="/user/SystemInfo.asp">
<input type="hidden" name="urlError" value="/user/SystemInfo.asp">
<input type="hidden" name="request" value=<%write(getRequestNumber());%>>
<input type="submit" name="Submit" value="Ok">
</form>

</html>
