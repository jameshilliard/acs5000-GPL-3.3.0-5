<!-- (c) 2002 Art & Logic, Inc. All Rights Reserved. -->
<!-- $Id: Config.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<title><%getLabel("page.title");%></title>
<link rel="STYLESHEET" type="text/css" href="/style/style.css">


<script language="JavaScript">
<!--

function checkData(field) {
  var rowNumber = parseInt(field.value);   
  if ((rowNumber) && (rowNumber >= 0) && (rowNumber <=100)) {
      return true;
  } else {
    alert("You must indicate a number between 1 and 100.");
    field.focus();
    field.blur();
    field.select();
    return false;
  }
}

function validateForm(form) {
  var retVal = true;

  if (!checkData(form._rowCount)) {
    // failed Data check
    retVal = false;
  }
  return retVal;
}
// -->
</script>

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

	
	
		<table width="450" cellpadding="0" cellspacing="0" border="0" align="center" id="Table1">
		<tr>
			<td>
				<span class="text">
				<b><%getLabel("customize");%></b>
				<br /><br />
				<%getLabel("customize.instructions");%>
				</span>
				<br /><br />
			</td>
		</tr>
		
		<tr>
			<td>
			
			
				<form method="POST" action="/goform/Dmf" name="form" onSubmit="return validateForm(this)">
			
				<table cellpadding="0" cellspacing="0" border="0" align="center" id="Table2">
				
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=210 height=2 border=0 ></td>							
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=6 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#999999"><img src="/images/pix.gif" width=230 height=2 border=0 ></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>											
					</tr>


					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>
					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff">
							<span class="text"><%getLabel("configure.language"); write(":");%></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff">
								<img src="/images/pix.gif" width=150 height=1 border=0 ><br />
								<select name="_language" id="Select3">
                           <%
                              lang = getVal("_language");
                              write('<option ');
                              if (lang == "en")
                              {
                                 write('selected ');
                              } 
                              write('value="en">' + getLabelVal("english"));
                              write('<option ');
                              if (lang == "es")
                              {
                                 write('selected ');
                              }
                              write('value="es">' + getLabelVal("spanish"));
                           %>
								</select></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>

					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff">
							<span class="text"><%getLabel("configure.bgcolor");write(":");%></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff">
								<select name="_bgcolor" id="Select4">
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
								</select></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>


					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff">
							<span class="text"><%getLabel("configure.logo"); write(":");%></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><input type=text name="_logo" value="<%get("_logo");%>" id="Text3"></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>


					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff">
							<span class="text"><%getLabel("configure.rowcount1");%></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff"><img src="/images/pix.gif" width=6 height=1 border=0 ></td>
						<td colspan="1" bgcolor="#ffffff">
									<input type=text name="_rowCount" value="<%get("_rowCount");%>" size="3">
									<span class="text"><%getLabel("configure.rowcount2");%></span></td>							
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=1 border=0 ></td>											
					</tr>
					<tr><td colspan="7" bgcolor="#666666"><img src="/images/pix.gif" width=450 height=1 border=0 ></td></tr>



					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="5" bgcolor="#999999"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=2 border=0 ></td>											
					</tr>			

					<tr>
						<td colspan="1" bgcolor="#666666"><img src="/images/pix.gif" width=1 height=35 border=0 ></td>
						<td colspan="5" bgcolor="#cccccc">
							
							<!-- SUBMIT FORM -->
							
							<img src="/images/pix.gif" width=6 height=1 border=0 >
							<input type="hidden" name="request" value="<%getRequestNumber();%>" id="Hidden4"> 
							<input type="image" name="_Submit" src="/images/<%write(getLanguage());%>/btn_submit_form.gif" height=19 border=0 onSubmit="return validateForm(this)">
							<input type="hidden" name="urlOk" value="/user/home.asp" id="Hidden5">
							<input type="hidden" name="urlError" value="/admin/Config.asp" id="Hidden6">
														
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

