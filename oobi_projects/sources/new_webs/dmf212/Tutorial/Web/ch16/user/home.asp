<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: home.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title><%getLabel("page.title");%></title>
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
	<a href="/home.asp"><img src="/images/<%get("_logo");%>" width=171 height=60 border=0 ></a></td>
<td rowspan=1 colspan=2 width=433 height=36>
	<img src="/images/dmf-2x2.png" width=433 height=36 border=0 ></td>
</tr>
<tr>
<td colspan=2 width=433 height=24 background="/images/tabs_bg.gif" align="right" valign="bottom">
	<a href="/user/SystemInfo.asp"><img src="/images/<%write(getLanguage());%>/tab_system_info_off.gif" width=85 height=19 border=0 ></a><img src="/images/pix.gif" width=1 height=1 border=0 ><a href="/user/Values.asp"><img
	src="/images/<%write(getLanguage());%>/tab_data_values_off.gif" width=85 height=19 border=0 ></a><img src="/images/pix.gif" width=1 height=1 border=0 ><a href="/user/Graphs.asp"><img
	src="/images/<%write(getLanguage());%>/tab_graphs_off.gif" width=85 height=19 border=0 ></a><img src="
<%
   if (userCanAccess("admin"))
   {
      write('/images/pix.gif" width=1 height=1 border=0 ><a href="/admin/UserList.asp"><img src="/images/');
      getLanguage();
      write('/tab_users_off.gif" width=85 height=19 border=0 ></a><img ');
      write('src="/images/pix.gif" width=1 height=1 border=0 ><a href="/admin/Config.asp"><img src="/images/');
      getLanguage();
      write('/tab_customize_off.gif" width=85 height=19 border=0 ></a><img src="\n');
   }
write('/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
%>

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


<table width="594" cellpadding="0" cellspacing="0" border="0">
<tr>
<td background="/images/home_bg.jpg">
<img src="/images/pix.gif" width=13 height=113 border=0 >
<img src="/images/bracket_lt.gif" width=23 height=73 border=0 >	
<img src="/images/pix.gif" width=5 height=113 border=0 >
<a href="/user/SystemInfo.asp"><img src="/images/<%write(getLanguage());%>/home_info.gif" alt="<%getLabel("system.info");%>" width=51 height=62 border=0 ></a>
<img src="/images/pix.gif" width=5 height=113 border=0 >
<a href="/user/Values.asp"><img src="/images/<%write(getLanguage());%>/home_values.gif" alt="<%getLabel("data.values");%>" width=51 height=62 border=0 ></a>
<img src="/images/pix.gif" width=5 height=113 border=0 >
<a href="/user/Graphs.asp"><img src="/images/<%write(getLanguage());%>/home_graphs.gif" alt="<%getLabel("graph.name");%>" width=51 height=62 border=0 ></a>
<img src="/images/pix.gif" width=5 height=113 border=0 >

<%
   if (userCanAccess("admin"))
   {
      write('<a href="/admin');
      write('/UserList.asp"><img src="/images/');
      getLanguage();
      write('/home_users.gif" alt="' + getLabelVal("user.management") + '" width=51 height=62 border=0 ></a><img src="/images/pix.gif" width=8 height=113 border=0 >\n');
      write('<a href="/admin');
      write('/Config.asp"><img src="/images/');
      getLanguage();
      write('/home_customize.gif" alt="' + getLabelVal("configure.device") + '"width=51 height=62 border=0 ></a><img src="/images/pix.gif" width=5 height=113 border=0 >\n');

   }
%>

<img src="/images/bracket_rt.gif" width=23 height=73 border=0 >	
</td>
</tr>
</table><img src="/images/pix.gif" width=592 height=5 border=0 ><table
width="594" border="0" bgcolor="#999999" cellpadding="0" cellspacing="0"><tr><td><img src="/images/pix.gif" width=592 height=5 border=0 ></td></tr></table>


<span class="text">

<ul>

<li><a href="/user/SystemInfo.asp"><%getLabel("system.info");%></a> - <%getLabel("system.description");%>
<br /><br />
<li><a href="/user/Values.asp"><%getLabel("data.values");%></a> - <%getLabel("data.description");%>

<br /><br />
<li><a href="/user/Graphs.asp"><%getLabel("graph.name");%></a> - <%getLabel("graph.description");%>

<%
   if (userCanAccess("admin"))
   {
      write('<br /><br /><li><a href="/admin');
      write('/UserList.asp">' + getLabelVal("user.management") + '</a> - ' + getLabelVal("user.description") + '\n');
      write('<br /><br /><li><a href="/admin');
      write('/Config.asp">' + getLabelVal("configure.device") + '</a> - ' + getLabelVal("configure.description") + '\n');

   }
%>

</ul>

</span>

<table width="594" border="0" bgcolor="#999999" cellpadding="0" cellspacing="0" id="Table2"><tr><td><img src="/images/pix.gif" width=592 height=5 border=0 ></td></tr></table>

	
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
