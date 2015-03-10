<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title><%getLabel("page.login");%></title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script language="JavaScript">
 <%
      // Set page-wide variables:
      var self = "system.login.";
      // Handle errors:
      var errors = "";
      count = getErrorCount();
      for (i = 0; i < count; i++)
      {
         name = getErrorName(i);
         error = getLabelVal("error." + name);
         if (__error__ == "ParameterName") // ("error." + name) isn't actually set in language.*
         {
            error = getLabelVal("error.unhandledDmfError") + " " + name + ". " + getLabelVal("error.pleaseNotify");
            if (1 == getVal("_kDebug_"))
               error = error + " (" + getErrorMessage(i) + ")";
         }
         else
               error = error + " " + getErrorMessage(i);
         errors = errors + error + "<br />";
      }
      name = getQueryVal("e", 0); // "errors" that result in login requirement may be passed via e=
      if (0 != name)
         errors = errors + getLabelVal("error." + name);

	resetError();
      da = 0;
      var family_model = getVal('_familymodel_');
      if (family_model == "KVM" || family_model == "ONS") {
         da=getVal("system.login.directacc");
      }
   %>

function screenresolution()
{
if (screen.width < 799)
alert ('These pages can only be properly viewed with screen resolution of at least 800 x 600.');
}

var unit_ip = '<%get("system.device.ip");%>';

if (opener != null) {
    var page = "/login.asp";
	try {
      var splitName = opener.window.name.split("_");
      try {
        if (splitName.length == 3 && splitName[0] == unit_ip) {
          opener.top.location.href = page;
          self.close();
        }
	  } catch (error) {
        self.close();
	  }
    }
    catch (error) {
    }

}

   </script>
</head>

<body class="bodyMain" onload="screenresolution();">

<form name="mainForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="urlOk" value="/redirect.asp"> <!-- an arg will be appended to this by Login() in UserFunc.c -->
    <input type="hidden" name="urlError" value="/login.asp">
    <input type="hidden" name="request" value=<%write(getRequestNumber()+1000);%>>

<table border="0" align="center">
	<tr height="60">
		<td width="200"><table border="0" align="center" class="securityBox">
			<tr>
				<td>     
            		<% write(errors); %>
                </td>
			</tr>
		</table></td>
    </tr>
    <tr>
		<td height="50" align="center">
			<font class="newloginText"><%getLabel(self + "login");%></font>
        </td>
    </tr>
    <tr>
        <td>
			<font class="yellowText"><%getLabel(self + "username");%></font>
        </td>
    </tr>
    <tr>
        <td align="right">
			<input type="text" maxlength="30" name="<%write(self);%>username" size="15" class="formText">
        </td>
    </tr>
    <tr>
        <td>
            <font class="yellowText"><%getLabel(self + "password");%></font> 
        </td>
    </tr>
    <tr>
        <td align="right">
            <input type="password" maxlength="30" name="<%write(self);%>password" size="15" class="formText"
				 onKeyDown="if(event.keyCode==13)document.mainForm.submit();">
        </td>
    </tr>

<%
if (da != 0) {
    write("<tr><td><font class=\"yellowText\">port name</font></td></tr>");
    write("<tr><td align=\"right\"><input type=\"kvmport\" maxlength=\"19\" name=\"");
    write(self);
    write("kvmport\" size=\"15\" class=\"formText\" onKeyDown=\"if(event.keyCode==13)document.mainForm.submit();\">");
    write("</td></tr>");
}
%>

    <tr>
        <td>
        	<p><br>
           	<a href="javascript:document.mainForm.submit();"><img src="Images/go.gif" alt="" width="32" height="21" border="0" align="right"></a>
        </td>
    </tr>
</table>
</form>
<script language="JavaScript">
<%
var portname=getQueryVal("portname","");
var username=getQueryVal("username","");
if (username != "") {
    write("document.mainForm.elements[3].value = \"" + username + "\";\n");
    write("document.mainForm.elements[4].focus();");
} else {
    write("document.mainForm.elements[3].focus();");
}
if (da != 0 && portname != "") {
    if (username == "" && da > 1) {
       write("document.mainForm.elements[3].value = \"default\";\n");
    }
    write("document.mainForm.elements[4].value = \"\";\n");
    write("document.mainForm.elements[5].value = \"" + portname + "\";\n");
	if (da > 1) {
	    write("document.mainForm.submit();");
	}
}
%>
</script>
</body>
</html>
