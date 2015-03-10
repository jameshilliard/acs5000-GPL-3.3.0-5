<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: Config.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<!--	CH08: Literal text removed from ASP source, replaced with calls to
		getLabel() and getLabelVal() as appropriate.
-->

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
<%
   lang = getVal("_language");
   write('<option ');
   if (lang == "en")
   {
      write('selected ');
   } 
   write('value="en">English');
   write('<option ');
   if (lang == "es")
   {
      write('selected ');
   }
   write('value="es">Spanish');
%>
</select><br>

<%getLabel("configure.bgcolor");write(":");%>
<select name="_bgcolor">
<!-- note - this should really start out with the current color selected, 
   not always (hardcoded) white.
-->
<%
   color = getVal("_bgColor");
   write('<option ');
   if (color == "White")
   {
      write('selected ');
   }
   write('value="White">' + getLabelVal("color.white"));

   write('<option ');
   if (color == "#CCCCCC")
   {
      write('selected ');
   }
   write('value="#CCCCCC">' + getLabelVal("color.ltgrey"));

   write('<option ');
   if (color == "#999999")
   {
      write('selected ');
   }
   write('value="#999999">' + getLabelVal("color.dkgrey"));

   write('<option ');
   if (color == "#CCCC99")
   {
      write('selected ');
   }
   write('value="#CCCC99">' + getLabelVal("color.beige"));

   write('<option ');
   if (color == "#999966")
   {
      write('selected ');
   }
   write('value="#999966">' + getLabelVal("color.olive"));

   write('<option ');
   if (color == "#99CCCC")
   {
      write('selected ');
   }
   write('value="#99CCCC">' + getLabelVal("color.teal"));

   write('<option ');
   if (color == "#99CCFF")
   {
      write('selected ');
   }
   write('value="#99CCFF">' + getLabelVal("color.skyblue"));

   write('<option ');
   if (color == "#FFFFCC")
   {
      write('selected ');
   }
   write('value="#FFFFCC">' + getLabelVal("color.yellow"));
%>
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
