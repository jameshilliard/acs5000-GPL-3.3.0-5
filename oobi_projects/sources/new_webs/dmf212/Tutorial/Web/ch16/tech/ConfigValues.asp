<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: ConfigValues.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->
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
      write('/tab_users_off.gif" width=85 height=19 border=0 ></a><img ');
      write('src="/images/pix.gif" width=1 height=1 border=0 ><a href="/admin/Config.asp"><img src="/images/');
      getLanguage();
      write('/tab_customize_on.gif" width=85 height=19 border=0 ></a><img src="\n');
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
				<b><%getLabel("data.values");%></b>
				<br /><br />
				<%getLabel("data.configinstructions");%>
				</span>
				<br /><br />
				
				
				

			</td>
		</tr>
		
		<tr>
			<td>
			
			
				<form method="POST" action="/goform/Dmf" name="form1" onsubmit="return validateAlarms(this)" id="Form1">
				<table cellpadding="0" cellspacing="0" border="0" bordercolor="red" align="center" id="Table3">
					
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
							<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=20 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
							<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=210 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
							<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=100 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
							<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=100 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
					</tr>




					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=25 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><span class="text"><b>#</b></span></td>

						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><span class="text"><b><%getLabel("label");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc" align="center"><span class="text"><b><%getLabel("lower.alarm");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#cccccc" align="center"><span class="text"><b><%getLabel("upper.alarm");%></b></span></td>
						
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
					</tr>
					<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
				
				
					<!-- DATA ROW -->
					
					<script language=javascript>
					<!--

					function validateAlarms(form1)
					{
						

						var bSendForm = false
						
						for (i = 0; i < 16; i++)
						{
							var lowerVal = document.form1["system.values.lowerAlarm_" + i].value
							var upperVal = document.form1["system.values.upperAlarm_" + i].value
							
							
							
							//alert(i + ' lowerVal ' + lowerVal);
							//alert(i + ' upperVal ' + upperVal);
							//alert(isNaN(parseInt(lowerVal)));
							if(isNaN(parseInt(lowerVal)) != true && isNaN(parseInt(upperVal)) != true )
							{
								if (parseInt(lowerVal) >= parseInt(upperVal))
								{
									alert("Lower Alarm value cannot exceed or be equal to Upper Alarm value.");
									document.form1["system.values.lowerAlarm_" + i].focus();
									return false;
								}
								else if(parseInt(lowerVal) < parseInt(upperVal))
								{
									bSendForm = true;
								}
							}
							else
							{
								alert("You must enter numeric data.");
								return false;
							}
						}
						if(bSendForm)
						{
							return true;
						}
						else
						{
							return false;
						}
					}


					//-->
					</script>

	
					
					<%
					// create the rows of the table, containing all of the values.


					for (i = 0; __success__; i++)
					{
						label = getVal("system.values.label", i);
						if (__success__)
						{
					        
							write('<tr><td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=25 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff"><span class="text">' + (i + 1) + '</td>\n');
							
							
							write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff"><span class="text">');
							
							write('<input name="system.values.label_' + i + '" ');
							write('value="' + label + '" size="20"></td>\n');

							write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff" align="center"><span class="text">');


							write('<input name="system.values.lowerAlarm_' + i + '" ');
							write('value="' + getVal("system.values.lowerAlarm", i) + '" ');
							write('size="3"></td>\n');


							write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>');
							write('<td colspan="1" bgcolor="#ffffff" align="center"><span class="text">');


							write('<input name="system.values.upperAlarm_' + i + '" ');
							write('value="' + getVal("system.values.upperAlarm", i) + '" size="3"></td>');
							write('<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td></tr>');
							write('<tr><td colspan="13" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>');
						}
					}
					%>
					
					
					<!-- // DATA ROW -->
					
					
					

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
							<input type="hidden" name="request" value="<%getRequestNumber();%>" id="request">
							<input type="hidden" name="urlOk" value="/user/Values.asp" id="urlOk">
							<input type="hidden" name="urlError" value="/tech/ConfigValues.asp" id="urlError">
							<input type="image" name="_Submit" value="Ok" src="/images/<%write(getLanguage());%>/btn_submit_form.gif" height="19" border="0" id="submit" >
							<!--img name="_Submit" onclick="validateAlarms(form1)" value="Ok" src="/images/btn_submit_form.gif" width="100" height="19" border="0" id="submit" -->
							
							
														
							</td>
							<td colspan="6" bgcolor="#cccccc" align="center">&nbsp;</td>
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
				</form>
			
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
