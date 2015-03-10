<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: Config.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->


<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
</head>
<body bgcolor=<%get("_bgcolor");%>>
<img src="/images/<%get("_logo");%>" alt="logo image">
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
<%getLabel("configure.language"); write(":");%>
<select name="_language">
<!-- add additional language options here as they are translated...-->
<option selected value="en">English
<option value="es" >Spanish
</select><br>

<%getLabel("configure.bgcolor");write(":");%>

<select name="_bgcolor">
<!-- note - this should really start out with the current color selected, 
   not always (hardcoded) white.
-->
<!-- we use the color names as defined by HTML to specify a color, 
   (the 'value' attribute of each option)
   but use translated names in the interface
-->
<option selected value="White"><%getLabel("color.white");%>
<option value="#CCCCCC"><%getLabel("color.ltgrey");%>
<option value="#999999"><%getLabel("color.dkgrey");%>
<option value="#CCCC99"><%getLabel("color.beige");%>
<option value="#999966"><%getLabel("color.olive");%>
<option value="#99CCCC"><%getLabel("color.teal");%>
<option value="#99CCFF"><%getLabel("color.skyblue");%>
<option value="#FFFFCC"><%getLabel("color.yellow");%>


</select><br>

<%getLabel("configure.logo"); write(":");%> 
<input type=text name="_logo" value="<%get("_logo");%>"><br>

<%getLabel("configure.rowcount1");%>
<input type=text name="_rowCount" value="<%get("_rowCount");%>">
<%getLabel("configure.rowcount2");%><br>



<input type="hidden" name="request" value="<%getRequestNumber();%>"> 
<input type="submit" name="submit" value="Ok">
<input type="hidden" name="urlOk"
 value="/user/home.asp">
<input type="hidden" name="urlError"
 value="/admin/Config.asp">

</form>
</body>
</html>
