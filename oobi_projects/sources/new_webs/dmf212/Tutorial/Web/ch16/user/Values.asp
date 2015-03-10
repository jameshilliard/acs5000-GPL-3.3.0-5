<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: Values.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

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
	<a href="/user/SystemInfo.asp"><img src="/images/<%write(getLanguage());%>/tab_system_info_off.gif" width=85 height=19 border=0 ></a><img src="/images/pix.gif" width=1 height=1 border=0 ><a href="/user/Values.asp"><img
	src="/images/<%write(getLanguage());%>/tab_data_values_on.gif" width=85 height=19 border=0 ></a><img src="/images/pix.gif" width=1 height=1 border=0 ><a href="/user/Graphs.asp"><img
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
				<b><%getLabel("data.values");%></b>
				<br /><br />
				<%getLabel("data.instructions");%>
				</span>
				<br /><br />
				
			</td>
		</tr>
		
		<tr>
			<td>
			
				<table cellpadding="0" cellspacing="0" border="0" bordercolor="red" align="center">
					
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="11" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>			
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="11" bgcolor="#cccccc" valign="middle">
						<img src="/images/pix.gif" width=3 height=20 border=0 align="middle">
							<% 
							if (userCanAccess("tech"))
							{
								write('<a href="/tech/ConfigValues.asp"><img src="/images/');
								getLanguage();
								write('/btn_configure_values.gif" height=19 border=0 align="middle"></a>\n');
							}
							%>
							</td>
							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>					
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="11" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>						
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>					
					
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=25 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="150" height="1" border=0 ><br /><span class="text"><b><%getLabel("label");%></b></span></td>

						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="80" height="1" border=0 ><br /><span class="text"><b><%getLabel("value");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="100" height="1" border=0 ><br /><span class="text"><b><%getLabel("lower.alarm");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width="100" height="1" border=0 ><br /><span class="text"><b><%getLabel("upper.alarm");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
					</tr>
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
				
				
									<!-- DATA ROW -->
					
					
					<%
   // see where we're supposed to start listing values.
   startPos = getQueryVal("start", 0);
   // see how many we're supposed to show at one time.
   count = getVal("_rowCount");
   if (!__success__)
   {
      /* The server hasn't been configured with a number of rows to 
       * show on a single page. We'll set a default.
       */
      count = 8;
      set("_rowCount", count);
   }
   endPos = startPos + count;
   upperAlarmColor = getVal("_upperAlarmColor");
   if (!__success__)
   {
      /* The upper Alarm color hasn;t been set. We'll set it to a 
       * default value of red here.
       */
       upperAlarmColor = "red";
       set("_upperAlarmColor", upperAlarmColor);
    }
   lowerAlarmColor = getVal("_lowerAlarmColor");
   if (__error__ == "ParameterName")
   {
      /* The lower Alarm color hasn;t been set. We'll set it to a 
       * default value of red here.
       */
       lowerAlarmColor = "blue";
       set("_lowerAlarmColor", lowerAlarmColor);
    }

   for (i = startPos; __success__ && (i < endPos); i++)
   {
      label = getVal("system.values.label", i);
      if (__success__)
      {
         value = getVal("system.values.value", i);
         upperAlarm = getVal("system.values.upperAlarm", i);
         lowerAlarm = getVal("system.values.lowerAlarm", i);
         write("<tr>\n");
         write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=25 border=0 ></td>');
         write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');         
         write('<td colspan="1" bgcolor="#ffffff"><span class="text">' + label + '</span></td>\n');
         isAlarm = 0
         write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
         write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');   
         write('<td colspan="1" bgcolor="#ffffff"><font face="verdana" size="2">');
         
                 if (value >= upperAlarm)
         {
            write("<font color=\"" + upperAlarmColor + "\">");
            isAlarm = 1
         }
         else if (value <= lowerAlarm)
         {
            write("<font color=\"" + lowerAlarmColor + "\">");
            isAlarm = 1
         }

         write(value); 
         if (isAlarm)
         {
            write("</font>");
         }
         write("</font></td>\n");
         
         
         write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
         write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');   
         write('<td colspan="1" bgcolor="#ffffff"><font face="verdana" size="2">'+ lowerAlarm + '</font></td>\n');
         
         write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
         write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');   
         write('<td colspan="1" bgcolor="#ffffff"><font face="verdana" size="2">'+ upperAlarm + '</font></td>');
         write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>\n');
         
       
       write('<td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
         
      }
   }

   
%>
				

					
					<!-- // DATA ROW -->


					<tr>
					<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="11" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="5" bgcolor="#cccccc">
							
							<!-- SUBMIT -->
							<img src="/images/pix.gif" width=6 height=1 border=0 >
							
								<%
								/*
								* see if we need to display a 'previous' link.
								*/
								if (startPos >= count)
								{
								write('<a href="/user');
								write('/Values.asp?start=' + (startPos - count) + '"><img src="/images/');
								getLanguage();
								write('/btn_previous.gif" height=10 border=0 ></a>\n');
								}
								%>
							
							</td>
							<td colspan="6" bgcolor="#cccccc" align="right">
							
								<%
								/* test to see if there are any items after what we've just displayed.*/
								value = getVal("system.values.value", endPos);
								if (__success__)
								{
								// we successfully got a value past what we've already displayed -- 
								// display a link to show the next page of values.
								write('<a href="/user');
								write('/Values.asp?start=' + endPos + '"><img src="/images/');
								getLanguage();
								write('/btn_more_values.gif" height=10 border=0 ></a>\n');
								}
								%>
								<img src="/images/pix.gif" width=6 height=1 border=0 ></td>
								
							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="11" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>			
					
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
				</table>
			
			</td>
		</tr>
		
	</table>
	
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


</body>
</html>

