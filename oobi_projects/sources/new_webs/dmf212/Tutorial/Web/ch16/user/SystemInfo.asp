<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: SystemInfo.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title><%getLabel("page.title");%></title>
<link rel="STYLESHEET" type="text/css" href="/style/style.css">
</head>


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
	<a href="/user/SystemInfo.asp"><img src="/images/<%write(getLanguage());%>/tab_system_info_on.gif" width=85 height=19 border=0 ></a><img src="/images/pix.gif" width=1 height=1 border=0 ><a href="/user/Values.asp"><img
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

	
	
		<table width="450" cellpadding="0" cellspacing="0" border="0" align="center">
		<tr>
			<td>
				<span class="text">
				<b><%getLabel("system.info");%></b>
				<br /><br />
				<%getLabel("system.instructions");%>
				</span>
				<br /><br />
			</td>
		</tr>
		
		<tr>
			<td>
			

<%
   editing = 0;
   if (userCanAccess("tech"))
   {
      editing = 1;
   }
   if (editing)
   {
      write('<form method="POST" action="/goform/Dmf">\n');
   }
%>
				
				
			
				<table cellpadding="0" cellspacing="0" border="0" align="center">
				
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=150 height=2 border=0 ></td>							
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=300 height=2 border=0 ></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>											
					</tr>
					
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
				


<%
   editing = 0;
   if (userCanAccess("tech"))
   {
      editing = 1;
   }
   if (editing)
   {
      write('<form method="POST" action="/goform/Dmf">\n');
   }
   
   
write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><span class="text">');
getLabel("system.name");
write(':</span></td>'); 
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><span class="text">');  
   
   if (editing)
   {
      write('<input size="30" name = "system.info.name" value="');
   }
   get("system.info.name");
   if (editing)
   {
      write('" id="Text3">');	
   }
   
write('</span></td><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
write('<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>');
   
  
   
write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><span class="text">');
getLabel("description");
write(':</span></td>');
write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
write('<td colspan="1" bgcolor="#ffffff"><span class="text">');

      
   if (editing)
   {
      write('<input size="30" name="system.info.description" value="');
   }
   get("system.info.description");
   if (editing)
   {
      write('" id="Text3">');
   }

write('</span></td><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
write('<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>');

%>

			

					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><span class="text"><%getLabel("uptime");%>:</span></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><span class="text"><%get("system.info.uptime");%></span></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>


					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="5" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>			

					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="5" bgcolor="#cccccc">
						<img src="/images/pix.gif" width=6 height=1 border=0 >	
												
						<%
						editing = 0;
						if (userCanAccess("tech"))
						{
							editing = 1;
						}
						if (editing)
						{
							write('<input type="hidden" name="urlOk" value="/user/SystemInfo.asp" id="Hidden1">\n');
							write('<input type="hidden" name="urlError" value="/user/SystemInfo.asp" id="Hidden2">\n');
							write('<input type="hidden" name="request" value="');
							write(getRequestNumber() + '" id="Hidden3">\n');
							write('<input type="image" name="_Submit" value="Ok" src="/images/');
							getLanguage();
							write('/btn_submit_form.gif" height="19" border="0" id="Image1">\n');
						}
						%>
			
						</td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>	
					
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
				</table>

				<%
				editing = 0;
				if (editing)
					{
							write('</form>');
					}
				%>			

			
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

