<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript"> 
<% ReqNum = getRequestNumber();
   acs5k_flag = getVal('_acs5k_');
   showDSVIEWoptions = getVal('_showDSVIEWoptions_');
%>

    parent.window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    top.currentPage = self;
    top.helpPage = 'helpPages/secAccessConfHelp.asp';

	var menuPage = "/normal/security/securityMenu.asp";

    var unitauth="<%write(getVal("system.security.authtypeP.unitauthtype"));%>";
    <%
    if (acs5k_flag != "1") {
	write('var pppauth="');
	write(getVal("system.security.authtypeP.pppauthtype"));
	write('";');
    }
    %>

    function set_select(obj, val)
    {
       var opt=obj.options;

       for(var i=0;i<opt.length;i++) {
         if (opt[i].value == val) {
           opt[i].selected = true;
           break;
          }
       }
    }

    function init()
    {
        if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady)
        {
            if (checkLeftMenu(menuPage) == 0) {
               top.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
            setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
            top.topMenu.selectItem(top.menu.topItem);
            top.menu.selectItem("AUTH");
            set_select(document.secAuthenticationForm.unitauthType,unitauth);
            <%
    		if (acs5k_flag != "1") {
			write("set_select(document.secAuthenticationForm.PPPauthType,pppauth);");
		}
            %>
        } else
            setTimeout('init()', 200);
    }

    function copyData()
    {
		document.configForm.elements[5].value = document.secAuthenticationForm.elements[0].value;
<% 
    		if (acs5k_flag != "1") {
			write('document.configForm.elements[6].value = document.secAuthenticationForm.elements[1].value;');
		}
%>
    }

     function submit()
	{
		copyData();
       	document.configForm.submit();
        top.mainReady = null;
      }

    function check_nis()
    {
// Check if auth method changed to/from any NIS
        var nis_pre = nis_pos = 0;
        var sec_pre = unitauth;
        var sec_pos = document.secAuthenticationForm.elements[0].value;
        if (sec_pre == "32773" || sec_pre == "20480" ||
            sec_pre == "20481" || sec_pre == "20482") {
            nis_pre = 1;
        }
        if (sec_pos == "32773" || sec_pos == "20480" ||
            sec_pos == "20481" || sec_pos == "20482") {
            nis_pos = 1;
        }
        if (nis_pre != nis_pos) {
            alert("Changing authentication type to/from NIS will "+
                    "require some processes to restart.\n"+
                    "Please refer to the User's Guide for details.");
        }
    }
      
</script>
</head>
<body class="bodyMiniWizard" onload="init();">
<form name="secAuthenticationForm" method="POST" action=" " onSubmit="return false;">
<table align="center" cellspacing="10" cellpadding="0" border="0">
<tr>	
	<td align="left">&nbsp;</td>
	<td align="left">&nbsp;</td>
</tr>
<tr>
	<td align="right" width="50%"><font class="label">Unit Authentication</font></td>
	<td align="left">
    <select name="unitauthType" size="1" class="formText" onchange="check_nis();">
 <option value="12288" >Kerberos </option>
 <option value="12289" >Kerberos/Local </option>
 <option value="12290" >KerberosDownLocal</option>
 <option value="16384" >Ldap </option>
 <option value="16385" >Ldap/Local </option>
 <option value="16386" >LdapDownLocal</option>
 <option value="16387" >LdapDownLocal-Radius</option>
 <option value="128" >Local </option>
 <option value="32769" >Local/Radius </option>
 <option value="32770" >Local/TacacsPlus</option>
 <option value="32773" >Local/Nis</option>
 <option value="20480" >Nis </option>
 <option value="20481" >Nis/Local </option>
 <option value="20482" >NisDownLocal </option>
 <option value="4096" >Radius</option>
 <option value="4097" >Radius/Local </option>
 <option value="4098" >RadiusDownLocal </option>
 <option value="8192" >TacacsPlus</option>
 <option value="8193" >TacacsPlus/Local</option>
 <option value="8194" >TacacsPlusDownLocal </option>
<%
 if (showDSVIEWoptions == "1") {
	write('<option value="28672" >DSView</option>');
	write('<option value="28673" >DSView/Local</option>');
	write('<option value="28674" >DSViewDownLocal</option>');
 }
%>

	</select>	
	</td>
</tr>
</table>
<div id="PPPauthDiv" style="display: <%
if (acs5k_flag != "1") {
	write("block");
} else {
	write("none");
}
%>">
<table align="center" cellspacing="10" cellpadding="0" border="0">
<tr><td colspan="2" align="center">
<br><br>
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="550" align="center">
<tr><td>
<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
<tr><td align="center" class="tabsTextBig">
By default PPP authentication is set to Unit Authentication. To change the<br>default, please select PPP authentication from the drop down menu below.
</td></tr>
</table>
</td></tr>
</table>
<br><br>
</td>
</tr>
<tr>
<td align="right" width="50%"><font class="label">PCMCIA-PPP Authentication</font></td><td align="left">
<select name="PPPauthType" size="1" class="formText" onchange="check_nis();">
<option value="256" >unit-auth </option>
<option value="12288" >Kerberos </option>
<option value="12289" >Kerberos/Local </option>
<option value="12290" >KerberosDownLocal</option>
<option value="16384" >Ldap </option>
<option value="16385" >Ldap/Local </option>
<option value="16386" >LdapDownLocal</option>
<option value="128" >Local </option>
<option value="32769" >Local/Radius </option>
<option value="32770" >Local/TacacsPlus</option>
<option value="32773" >Local/Nis</option>
<option value="4096" >Radius</option>
<option value="4097" >Radius/Local </option>
<option value="4098" >RadiusDownLocal </option>
<option value="8192" >TacacsPlus</option>
<option value="8193" >TacacsPlus/Local</option>
<option value="8194" >TacacsPlusDownLocal </option>
<%
 if (showDSVIEWoptions == "1") {
	write('<option value="28672" >DSView</option>');
	write('<option value="28673" >DSView/Local</option>');
	write('<option value="28674" >DSViewDownLocal</option>');
 }
%>
</select>
</td>
</tr>
</table>
</div>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/security/secAccessConf.asp">
   <input type="hidden" name="urlError" value="/normal/security/secAccessConf.asp">
   <input type="hidden" name="request" value=<% write(ReqNum);%>>
   <input type="hidden" name="system.security.authtypeP.unitauthtype" value="">
   <%
   if (acs5k_flag != "1") {
	write('<input type="hidden" name="system.security.authtypeP.pppauthtype" value="">');
   }
   %>
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
