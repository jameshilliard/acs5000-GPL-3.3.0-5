<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: ConfigValues.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title>DMF Tutorial: <%get("_chapter_");%></title>
</head>

<h1>Edit Parameter Configuration</h1>
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
<table border>
<tr>
   <td /> <!-- we'll add the item # here... -->
   <td>Label</td>
   <td>Lower Alarm</td>
   <td>Upper Alarm</td>
</tr>
<%
   // create the rows of the table, containing all of the values.

   for (i = 0; __success__; i++)
   {
      label = getVal("system.values.label", i);
      if (__success__)
      {
         write('<tr>\n<td>' + (i + 1) + '</td>\n');
         write(' <td><input name="system.values.label_' + i + '" ');
         write('value="' + label + '" size="32"></td>\n');

         write(' <td><input name="system.values.lowerAlarm_' + i + '" ');
         write('value="' + getVal("system.values.lowerAlarm", i) + '" ');
         write('size="3"></td>\n');

         write(' <td><input name="system.values.upperAlarm_' + i + '" ');
         write('value="' + getVal("system.values.upperAlarm", i) + '" ');
         write('size="3"></td>\n</tr>\n');
      }
   }
%>
</table>

<input type="hidden" name="urlOk"
 value="/user/Values.asp">

<input type="hidden" name="urlError"
 value="/tech/ConfigValues.asp">
 
<input type="hidden" name="request" value="<%write(getRequestNumber());%>">

<input type="submit" name="Submit" value="Ok">
</form>
</html>
