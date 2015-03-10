<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: Config.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<!--	CH08: Literal text removed from ASP source, replaced with calls to
		getLabel() and getLabelVal() as appropriate.
-->

<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
</head>

<h1><%getLabel("configure.device");%></h1>
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
<%getLabel("select.language"); write(":");%>
<select name="_language">
<option selected value="en">English
<option value="es" >Spanish
</select><br>


<input type="hidden" name="request" value="<%getRequestNumber();%>"> 
<input type="submit" name="submit" value="Ok">
<input type="hidden" name="urlOk" value="/user/home.asp">
<input type="hidden" name="urlError" value="/admin/Config.asp">
</form>
