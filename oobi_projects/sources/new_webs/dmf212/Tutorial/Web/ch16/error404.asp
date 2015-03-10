<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: error404.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title><%getLabel("page.title"); get("_chapter_");%></title>
<link rel="STYLESHEET" type="text/css" href="/style/style.css">
</head>


<body bgcolor="<%
   color = getVal("_bgcolor");
   if (!__success__)
   {
      color = "White";
      set("_bgcolor", color);
   }
   write(color);
%>" leftmargin="0" topmargin="0" marginheight="0" marginwidth="0">

<br />

<table border=0 cellpadding=0 cellspacing=0 width=604 align="center" id="Table1">
<tr>
<td colspan=4 width=604 height=21>
	<img src="/images/top_grey.gif" width=604 height=21 border=0 ></td>
</tr>
<tr>
<td rowspan=2 colspan=2 width=171 height=60>
	<a href="/home.asp"><img src="/images/dmf_logo.png" width=171 height=60 border=0 ></a></td>
<td rowspan=1 colspan=2 width=433 height=36>
	<img src="/images/dmf-2x2.png" width=433 height=36 border=0 ></td>
</tr>
<tr>
<td colspan=2 width=433 height=24 background="/images/tabs_bg.gif" align="right" valign="bottom">
	<img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>

<tr>
<td colspan="4" width=5 bgcolor="#666666">
	<img src="/images/pix.gif" width=604 height=5 border=0 ></td>
</tr>
<tr>
<td width=5 bgcolor="#666666">
	<img src="/images/pix.gif" width=5 height=325 border=0 ></td>
<td colspan=2 width=594 height=325 bgcolor="#ffffff" valign="top">
	
	<br />
	<br />

	<!-- MAIN CONTENT AREA -->
	
	<% 
   count = getErrorCount();
   if (count > 0)
   {
      write('<table width="450" align="center" bgcolor="#ffffcc" border="2" bordercolor="red" cellspacing="0" cellpadding="10"><tr><td><h2>');
      write('<img src="/images/error.gif" width="40" height="28"><font color="red">' + getLabelVal("errors.head") + '</font>');
      write('</h2><span class="text"><b>\n');
      getLabel("errors.instruction");
      write("</b><br /><br />\n");

      for (i = 0; i < count; i++)
      {
         if (count > 1)
         {
            write((i + 1) + ": ");
         }
         write(getErrorMessage(i) + "<br />\n");
      }
      write("</span></td></tr></table><br />\n");      
   }
%>

	
	
		<table width="450" cellpadding="0" cellspacing="5" border="0" align="center" id="Table2">
		<tr>
			<td>
			<img src="/images/page_error.gif" height="229" width="208" border="0" alt="???">
			</td>
			
			<td valign="top">
			<font face="arial" size="5" color="#cc3333"><b>404 Error.</b></font>
			<br />
			<br />
			
				<span class="text">
			The page you are looking for may have been removed, had its name changed, or is temporarily unavailable.
			<br />
			<br />
			<%	now = getTime();
				timeString = formatTime(now, "%c");
				write("Current time = " + timeString + "<br />");
				write("Error Code = " + getVal("_errorCode_") + "<br />");
				write("Error Text = " + getVal("_errorText_") + "<br />");
				write("Error Message = " + getVal("_errorMessage_") + "<br />");
				write("Error URL = " + getVal("_errorUrl_") + "<br />");
			%>
			<br />
			HTTP 404 - <b>File not found</b>

		
			</td>
		</tr>
		
	</table>
	
	<!-- / MAIN CONTENT AREA -->
		
	</td>
<td width=5 bgcolor="#666666">
	<img src="/images/pix.gif" width=5 height=325 border=0 ></td>
</tr>


<tr>
<td width=5 bgcolor="#666666"><img src="/images/pix.gif" width=5 height=20 border=0 ></td>
<td width=594 colspan="2" bgcolor="#ffffff" align="right"><a href="http://www.artlogic.com/dmf"><img src="/images/copyright.gif" width=271 height=15 border=0 ></a></td>
<td width=5 bgcolor="#666666"><img src="/images/pix.gif" width=5 height=20 border=0 ></td>
</tr>



<tr>
<td colspan=4 width=604 height=23>
	<img src="/images/btm_photo.gif" width=604 height=23 border=0 ></td>
</tr>
<tr>
<td width=5 height=1><img src="/images/pix.gif" width=5 height=1 border=0></td>
<td width=166 height=1><img src="/images/pix.gif" width=166 height=1 border=0></td>
<td width=428 height=1><img src="/images/pix.gif" width=428 height=1 border=0></td>
<td width=5 height=1><img src="/images/pix.gif" width=5 height=1 border=0></td>
</tr>
</table>


</body>
</html>
