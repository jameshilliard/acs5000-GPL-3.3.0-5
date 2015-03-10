<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: UserList.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title><%getLabel("page.title");%></title>
<link rel="STYLESHEET" type="text/css" href="/style/style.css">
</head>

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

<body bgcolor=<%get("_bgcolor");%> leftmargin="0" topmargin="0" marginheight="0" marginwidth="0">

<br />

<table border=0 cellpadding=0 cellspacing=0 width=604 align="center">
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
      write('/tab_users_on.gif" width=85 height=19 border=0 ></a><img ');
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

	
	
		<table width="450" cellpadding="0" cellspacing="0" border="0" align="center">
		<tr>
			<td>
				<span class="text">
				<b><%getLabel("user.management");%> > <%getLabel("user.list");%></b>
				<br /><br />
				<%getLabel("user.instructions");%>
				</span>
				<br /><br />
			</td>
		</tr>
		
		<tr>
			<td>
			
				<table cellpadding="0" cellspacing="0" border="0" align="center">
					
					<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="14" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>			
					<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=25 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="120" height="1" border=0 ><br /><span class="text"><b><%getLabel("user.name");%></b></span></td>

						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="150" height="1" border=0 ><br /><span class="text"><b><%getLabel("password");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="70" height="1" border=0 ><br /><span class="text"><b><%getLabel("group");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="60" height="1" border=0 ><br /><span class="text"><b><%getLabel("enabled");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="50" height="1" border=0 ><br /><span class="text"><b>Delete</b></span></td>
					
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
					</tr>
					<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>


<!-- DATA ROW -->

<%
   for (name = getNextUser(); __success__; name = getNextUser(name))
   {
      write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=25 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff"><span class="text"><a href="/admin/EditUser.asp?name=' + name + '">');
      write( name + '</a></span></td>\n');
      
      password = getUserDetail(name, "password");
      write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff"><span class="text">' + password + '</span></td>\n');
      
      group = getUserDetail(name, "group");
      write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff"><span class="text">' + group + '</td>\n');
      
      enabled = getUserDetail(name, "enabled");
      write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>');
      write('<td colspan="1" bgcolor="#ffffff" align="center">');
      if (enabled)
      {
         write('<img src="/images/enabled.gif" width=8 height=8 border=0 >');
      }
      write("</td>\n");
      
      write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>');
      write('<td colspan="2" bgcolor="#ffffff" align="center"><a href="#" onClick="deleteUser(\'' + name + '\'); return false;">');
      write('<img src="/images/x.gif" width=9 height=9 border=0 ></a></td>\n');
      
      write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>\n');   
      write('<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>\n');

   }
%>


<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
<!-- // DATA ROW -->



					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="14" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="14" bgcolor="#cccccc">
							
							<!-- SUBMIT -->
							<img src="/images/pix.gif" width=6 height=1 border=0 >
							<a href="/admin/EditUser.asp"><img src="/images/<%write(getLanguage());%>/btn_add_new_user.gif" height=10 border=0 ></a></td>
							<!-- / SUBMIT -->
							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="14" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>			
					
					<tr><td colspan="16" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
				</table>
			
			</td>
		</tr>
		
	</table>
	
	
	<br />
	<br />
	<br />
	<br />
	
	
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


<!-- a hidden form to delete a user - works with clientside JavaScript -->
<form name="deleteForm" method="POST" action="/goform/Dmf" id="Form1">
<input type="hidden" name="name" value="" id="Hidden1">
<input type="hidden" name="command" value="DELETEUSER" id="Hidden2">
<input type="hidden" name="urlOk"
 value="/admin/UserList.asp" id="Hidden3">
<input type="hidden" name="urlError"
 value="/admin/UserList.asp" id="Hidden4">
</form>


</body>
</html>

