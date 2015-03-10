<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<title>Access</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript"> 

<% ReqNum = getRequestNumber(); 
%>    

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	top.currentPage = self;
	top.helpPage = 'helpPages/portsPhysAccessHelp.asp';
        
	function init()
	{
		if (top.bidirectActive == true) {
			show('document', 'bidirectL');
		} else {
			hide('document', 'bidirectL');
		}
	}
	function showwarn()
	{
		if ((document.secAuthenticationForm.elements[1].value == 20480) ||
			(document.secAuthenticationForm.elements[1].value == 20481) ||
			(document.secAuthenticationForm.elements[1].value == 20482) ||
			(document.secAuthenticationForm.elements[1].value == 32773))
			alert("The unit authentication is also changed by this selection!");
	}
	function copyData()
	{
		var maxvars;
		if (top.family_model == "AVCS")	maxvars=2; else maxvars=4;
		for (i=0;i<maxvars;i++) {
			document.configForm.elements[i+6].value = document.secAuthenticationForm.elements[i].value;
		}
	}

     function form_verification()
	 {
		continue_verification = true;
		var form_location = document.secAuthenticationForm;
			
		checkElement (form_location.elements[0], false, false, false, false);
	  }
	 
	 function submit()
      {		 
	   form_verification();
	   if (continue_verification == true)
			{
			copyData();
			if (document.configForm.elements[0].value != 0) 
			{
				document.configForm.urlOk.value="/normal/ports/physPorts.asp";
			}
			setQuerySSID(document.configForm);
        	document.configForm.submit();
			top.mainReady = null;
			}
      }
      
      function alertWrongAuthType()
      {
		if (top.family_model != "AVCS")
                alert('The current access authtype is incompatible with the Security Profile chosen.');
		else
                alert('The current access authtype is incompatible with Security/Services selections.');
      }

</script>
</head>
<body onload="init();" class="bodyMiniWizard">
<form name="secAuthenticationForm" method="POST" action=" " onSubmit="return false;">
<table align="center" cellspacing="10" cellpadding="0" border="0">
<tr>	
	<td align="left">&nbsp;</td>
	<td align="left">&nbsp;</td>
</tr>
<tr>	
	<td align="left"><font class="label">Authorized Users/Groups</font></td>
	<td align="left"><input type="text" class="formText" name="aut_users_groups" maxlength="255" size="23" value="<% get("system.ports.physP.access.users");%>"></td>
</tr>
<tr>
	<td align="right"><font class="label">Authentication type</font></td>
	<td align="left">
    <select name="authType" size="1" class="formText" onChange="showwarn();">
<% var selauth=getVal("system.ports.physP.access.authType");%>

<script type="text/javascript">
var profile = 0;
if (top.family_model != "AVCS")
	profile = <%get("system.security.profile.type");%>
var needsAuth = <%get("system.security.profile.auth2sport");%>
var localselauth = <%write(selauth);%>
var protocol = <%get("system.ports.physP.general.protocol");%>

if (protocol != "105") { //not Power Management
	if ((!localselauth) || ((profile!=4) && ((profile!=1 && profile!=0) || !needsAuth))) {
		if ((profile==4) || ((profile==1 || !localselauth) & needsAuth))
			alertWrongAuthType();
		document.write('<option value="0" <% if (selauth=="0") write("selected");%>> None&nbsp;&nbsp; </option>');
	}
}

if (top.family_model != "AVCS")
document.write('\
 <option value="12288" <% if (selauth=="12288") write("selected");%>>Kerberos </option>\
 <option value="12289" <% if (selauth=="12289") write("selected");%>>Kerberos/Local </option>\
 <option value="12290" <% if (selauth=="12290") write("selected");%>>KerberosDownLocal</option>');
document.write('\
 <option value="16384" <% if (selauth=="16384") write("selected");%>>Ldap </option>\
 <option value="16385" <% if (selauth=="16385") write("selected");%>>Ldap/Local </option>\
 <option value="16386" <% if (selauth=="16386")  write("selected");%>>LdapDownLocal</option>');
if (top.family_model != "AVCS")
document.write('\
 <option value="16387" <% if (selauth=="16387")  write("selected");%>>LdapDownLocal-Radius</option>');
document.write('\
 <option value="128" <% if (selauth=="128") write("selected");%>>Local </option>\
 <option value="32769" <% if (selauth=="32769") write("selected");%>>Local/Radius </option>\
 <option value="32770" <% if (selauth=="32770") write("selected");%>>Local/TacacsPlus</option>');
if (top.family_model != "AVCS")
document.write('\
 <option value="32773" <% if (selauth=="32773") write("selected");%>>Local/Nis</option>\
 <option value="20480" <% if (selauth=="20480")  write("selected");%>>Nis </option>\
 <option value="20481" <% if (selauth=="20481") write("selected");%>>Nis/Local </option>\
 <option value="20482" <% if (selauth=="20482") write("selected");%>>NisDownLocal </option>\
 <option value="6" <% if (selauth=="6") write("selected");%>>OTP </option>\
 <option value="7" <% if (selauth=="7") write("selected");%>>OTP/Local </option>');
document.write('\
 <option value="4096" <% if (selauth=="4096") write("selected");%>>Radius</option>\
 <option value="4097" <% if (selauth=="4097") write("selected");%>>Radius/Local </option>\
 <option value="4098" <% if (selauth=="4098") write("selected");%>>RadiusDownLocal </option>\
 <option value="8192" <% if (selauth=="8192") write("selected");%>>TacacsPlus</option>\
 <option value="8193" <% if (selauth=="8193") write("selected");%>>TacacsPlus/Local</option>\
 <option value="8194" <% if (selauth=="8194") write("selected");%>>TacacsPlusDownLocal </option>');
if (("<%get('_showDSVIEWoptions_');%>" == "1"))
document.write('\
 <option value="28672" <% if (selauth=="28672") write("selected");%>>DSView</option>\
 <option value="28673" <% if (selauth=="28673") write("selected");%>>DSView/Local</option>\
 <option value="28674" <% if (selauth=="28674") write("selected");%>>DSViewDownLocal</option>');
</script>
	</select>	
	</td>
</tr>
</table>
<div id="bidirectL" style="position:absolute; top:100px; width:100%; z-index:1; visibility: visible;"align="center">
<table align="center" cellspacing="10" cellpadding="0" border="0">
<tr>
   <td align="left"><font class="label">BidirectionLogin Timeout</font></td>
    <td align="left"><font class="formText">
    <input name="LoginTmo" type="text" size="12" class="formText" value="<% get("system.ports.physP.access.lgtimeout");%>">
    </font></td>
</tr>
<tr>
   <td align="left"><font class="label">BidirectionShell Command</font></td>
    <td align="left"><font class="formText">
    <input name="TermSh" type="text" size="30" class="formText" value="<% get("system.ports.physP.access.termsh");%>">
    </font></td>
  </tr>
</table>
</div>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/ports/physPorts.asp">
   <input type="hidden" name="urlError" value="/normal/ports/physPorts.asp">
   <input type="hidden" name="request" value=<% write(ReqNum);%>>
   <input type="hidden" name="system.ports.physP.action" value="2">
   <input type="hidden" name="system.ports.physP.access.users" value="">
   <input type="hidden" name="system.ports.physP.access.authtype" value="">
<script type="text/javascript">
if (top.family_model != "AVCS") document.write('\
   <input type="hidden" name="system.ports.physP.access.lgtimeout" value="">\
   <input type="hidden" name="system.ports.physP.access.termsh" value="">');
document.write('\
</form>');
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
