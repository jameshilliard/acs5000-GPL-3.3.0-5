<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: EditUser.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title><%getLabel("page.title");%></title>
<link rel="STYLESHEET" type="text/css" href="/style/style.css">
</head>


<body bgcolor=<%get("_bgcolor");%> leftmargin="0" topmargin="0" marginheight="0" marginwidth="0">

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

	
	
		<table width="450" cellpadding="0" cellspacing="0" border="0" align="center" id="Table2">
		<tr>
			<td>
				<span class="text">
				

					<b><%getLabel("user.management");%> > <%
 					name = getQueryVal("name", "");
					editing = (name != "");

					if (editing)
					{
						getLabel("edit.user");
						userCommand = "MODIFYUSER";
						write('</b><br /><br />' + getLabelVal("edit.instructions"));
					}
					else
					{
						getLabel("add.user");
						userCommand = "ADDUSER";
						write('</b><br /><br />' + getLabelVal("add.instructions"));
					}
					
					%>
				
				</span>
				<br /><br />
				
			</td>
		</tr>
		
		<tr>
			<td>
			
			
				<form method="POST" action="/goform/Dmf" id="Form1">
			
				<table cellpadding="0" cellspacing="0" border="0" bordercolor="red" align="center">
				
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=150 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=300 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
					</tr>
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>


					<%


write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff">');
write('<span class="text">User Name:</span></td>');
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><span class="text">');

   if (editing)
   {
      write("<b>" + name + "</b>\n");
      password = getUserDetail(name, "password");
      group = getUserDetail(name, "group");
      enabled = getUserDetail(name, "enabled");

   }
   else
   {
      //write(getLabelVal("user.name"));
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
	write('" name="name"></span><br />\n');
   
write('</td><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
write('<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>');


write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff">');
write('<span class="text">');
write(getLabelVal("password") + ':</span></td>');
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff">');
   
   write('<input type="password" name="password" value="');
   write(password + '"></td>\n');
   
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
write('<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>');
   
   
 
write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff">');
write('<span class="text">');
write(getLabelVal("group") + ':</span></td>');
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><select name="group">');
   
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
   write("</select></td>\n");
   
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
write('<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>');
   
   
   
   
write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff">');
write('<span class="text">');
write(getLabelVal("enabled") + ':</span></td>');
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><input name="enabled" type="checkbox"');
   
   if (enabled)
   {
      write(" checked ");
   }
   
write('></td><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
write('<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>');   
   
%>

					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="5" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>			

					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="5" bgcolor="#cccccc">
							
							<!-- SUBMIT FORM -->
							
							<img src="/images/pix.gif" width=6 height=1 border=0 >
							<input type="hidden" name="command" value="<%write(userCommand);%>" id="Hidden1">
							<input type="hidden" name="urlOk" value="/admin/UserList.asp" id="Hidden2">
							<input type="hidden" name="urlError" value="/admin/EditUser.asp" id="Hidden3">
							<input type="image" name="_Submit" src="/images/<%write(getLanguage());%>/btn_submit_form.gif" height=19 border=0 id="Image1">

							<!-- SUBMIT FORM -->
							
						</td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>	
					
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="5" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>						
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
				</table>
				</form>			

<br />
<br />
<br />

			
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

