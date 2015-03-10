
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Access</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">     

	top.currentPage = self;
	top.helpPage = 'helpPages/portsPhysAccessHelp.asp';
   	  function showLayer(sel) 
	{
		var index = sel.selectedIndex;
		switch(sel.options[index].value) {
			case "0":
            case "32":
			case "16":			
			    hide('document', 'Layer1');
				hide('document', 'Layer2');
				hide('document', 'Layer3');
				hide('document', 'Layer4');
				break;
			case "89":
			case "90":
			case "91":
			case "92":
				show('document', 'Layer1');
				hide('document', 'Layer2');
				hide('document', 'Layer3');
				hide('document', 'Layer4');
				break;
			case "8":
			case "88":
				hide('document', 'Layer1');
				show('document', 'Layer2');
				hide('document', 'Layer3');
				hide('document', 'Layer4');
				break;
			case "4":
			case "84":
				hide('document', 'Layer1');
				hide('document', 'Layer2');
				show('document', 'Layer3');
				hide('document', 'Layer4');
				break;
			case "1":
			case "2":
			case "65":
			case "49":
			case "66":
			case "50":
			case "81":
			case "82":
				hide('document', 'Layer1');
				hide('document', 'Layer2');
				hide('document', 'Layer3');
				show('document', 'Layer4');
				break;
			default:
				alert("VALOR:" + sel.options[index].value);
		}
	}
        
	function copyData()
	{
		for (var i=0;i<16;i++) {
			document.configForm.elements[i+6].value = document.secAuthenticationForm.elements[i].value;
		}
		if (document.secAuthenticationForm.elements[6].checked == true) {
			document.configForm.elements[12].value = 1;
		} else {
			document.configForm.elements[12].value = 0;
		}
	}

     function form_verification()
	 {
		continue_verification = true;
		var form_location = document.secAuthenticationForm;
			
		checkElement (form_location.elements[0], false, true, false, false);
			var index = form_location.elements[1].selectedIndex;
			switch(form_location.elements[1].options[index].value) {			
				case "1":
				case "2":
				case "65":
				case "49":
				case "66":
				case "50":
				case "81":
				case "82":
					if (continue_verification == true)
						{checkElement (form_location.elements[9], true, false, true, false);}
					if (continue_verification == true)
						{checkElement (form_location.elements[10], false, false, true, false);} 
					if (continue_verification == true)
						{checkElement (form_location.elements[11], true, false, true, false);}
					if (continue_verification == true)
						{checkElement (form_location.elements[12], false, false, true, false);}
					if (continue_verification == true)
						{checkElement (form_location.elements[13], true, true, false, false);}
					if (continue_verification == true)
						{checkElement (form_location.elements[14], true, false, false, true);}
					if (continue_verification == true)
						{checkElement (form_location.elements[15], false, false, false, true);}
					break;
				case "8":
				case "88":
					if (continue_verification == true)
						{checkElement (form_location.elements[4], true, false, true, false);}
					if (continue_verification == true)
						{checkElement (form_location.elements[5], true, true, false, false);}	
					break;							
				case "89":
				case "90":
				case "91":
				case "92":
					if (continue_verification == true)
						{checkElement (form_location.elements[2], true, true, false, false);}
					if (continue_verification == true)
						{checkElement (form_location.elements[3], true, false, true, false);}	
					break;
				case "4":
				case "84":
					if (continue_verification == true)
						{checkElement (form_location.elements[7], true, false, true, false);}
					if (continue_verification == true)
						{checkElement (form_location.elements[8], true, true, false, false);}	
					break; 
			}	
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
        	document.configForm.submit();
			top.mainReady =0;
			}
      }
      
</script>
</head>
<body class="bodyMiniWizard">
<form name="secAuthenticationForm" method="POST" action=" ">
<table align="center" cellspacing="10" cellpadding="0" border="0">
<tr>	
	<td align="left">&nbsp;</td>
	<td align="left">&nbsp;</td>
</tr>
<tr>	
	<td align="left"><font class="label">Authorized Users/Groups:</font></td>
	<td align="left"><input type="text" class="formText" name="aut_users_groups" maxlength="255" size="23" value="<% get("system.ports.physP.access.users");%>"></td>
</tr>
<tr>
	<td align="right"><font class="label">Type</font></td>
	<td align="left">
    <select name="authType" size="1" class="formText" onChange="showLayer(this);">
	<% var selauth=getVal("system.ports.physP.access.authType");%>
    <option value="0" <% if (selauth=="0") write("selected");%>> None&nbsp;&nbsp; </option>
	<option value="32" <% if (selauth=="32") write("selected");%>> Local&nbsp;&nbsp; </option>
	<option value="16" <% if (selauth=="16") write("selected");%>> Remote&nbsp;&nbsp; </option>
	<option value="1" <% if (selauth=="1") write("selected");%>> Radius&nbsp;&nbsp; </option>
	<option value="2" <% if (selauth=="2") write("selected");%>> TacacsPlus&nbsp;&nbsp; </option>
	<option value="8" <% if (selauth=="8") write("selected");%>> Ldap&nbsp;&nbsp; </option>
	<option value="89" <% if (selauth=="89") write("selected");%>> NIS&nbsp;&nbsp; </option>
	<option value="65" <% if (selauth=="65") write("selected");%>> Local/Radius&nbsp;&nbsp; </option>
	<option value="49" <% if (selauth=="49") write("selected");%>> Radius/Local&nbsp;&nbsp; </option>
	<option value="66" <% if (selauth=="66") write("selected");%>> Local/TacacsPlus &nbsp;&nbsp; </option>
	<option value="50" <% if (selauth=="50") write("selected");%>> TacacsPlus/Local&nbsp;&nbsp; </option>
	<option value="81" <% if (selauth=="81") write("selected");%>> RadiusDownLocal&nbsp;&nbsp; </option>
	<option value="82" <% if (selauth=="82") write("selected");%>> TacacsPlusDownLocal&nbsp;&nbsp; </option>
	<option value="88" <% if (selauth=="88") write("selected");%>> LdapDownLocal&nbsp;&nbsp; </option>
	<option value="90" <% if (selauth=="90") write("selected");%>> LocalNIS&nbsp;&nbsp; </option>
	<option value="91" <% if (selauth=="91") write("selected");%>> NISLocal&nbsp;&nbsp; </option>
	<option value="92" <% if (selauth=="92") write("selected");%>> NISDownLocal&nbsp;&nbsp; </option>
	<option value="4" <% if (selauth=="4") write("selected");%>> Kerberos&nbsp;&nbsp; </option>
	<option value="84" <% if (selauth=="84") write("selected");%>> KerberosDownLocal&nbsp;&nbsp; </option>
	</select>	
	</td>
</tr>
</table>
<div id="Layer1" name="NISLayer" style="position:absolute; left:0px; top:100px; width:100%; z-index:1; visibility: hidden;" align="center">
 <table width="360" border="0" align="center" cellpadding="0" cellspacing="10">
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">NIS Domain Name</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="nis_domain_name" type="text" maxlength="39" size="15" class="formText" value="<% get("system.ports.authP.nisDomainName");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">NIS Server IP</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="nis_server_ip" type="text" maxlength="15" size="15" class="formText" value="<% get("system.ports.authP.nisServerIp");%>" ></font>
	</td>
  </tr>
</table>
</div>
<div id="Layer2" name="ldapLayer" style="position:absolute; left:0px; top:100px; width:100%; z-index:1; visibility: hidden;" align="center">
 <table width="360" border="0" align="center" cellpadding="0" cellspacing="10">
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">Ldap Server</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="ldap_server" type="text" maxlength="15" size="15" class="formText" value="<% get("system.ports.authP.ldapServer");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">Ldap Base Domain Name</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="ldap_base_dom_name" type="text" maxlength="39" size="15" class="formText" valeu="<% get("system.ports.authP.ldapBase");%>"></font>
	</td>
  </tr>
  <tr align="center" valign="top"> 
    <td align="center" width="300"  colspan="2">
	<% var checked=getVal("system.ports.authP.ldapsecure");%>
	<input type="checkbox" name="SecureLdap" <% if (0!=checked) write("checked");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	<font class="label">Secure Ldap</font></td>
  </tr>
</table>
</div>

<div id="Layer3" name="kerberosLayer" style="position:absolute; left:0px; top:100px; width:100%; z-index:1; visibility: hidden;" align="center">
 <table width="360" border="0" align="center" cellpadding="0" cellspacing="10">
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">Kerberos Server (Realm)</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="kerberos_server" type="text" maxlength="15" size="15" class="formText" value="<% get("system.ports.authP.krbServer");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">Kerberos Realm Domain Name</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="kerberos_dom_name" type="text" maxlength="39" size="15" class="formText" value="<% get("system.ports.authP.krbDomainName");%>"></font>
	</td>
  </tr>
</table>
</div>
<div id="Layer4" name="radius_or_tacacsLayer" style="position:absolute; left:0px; top:100px; width:100%; z-index:1; visibility: hidden;" align="center">
 <table width="360" border="0" align="center" cellpadding="0" cellspacing="10">
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">First Authentication Server</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="first_aut_serv_hostname" type="text" maxlength="15" size="15" class="formText" value="<% get("system.ports.authP.radiusAuthServer");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
	<td align="left" width="180"><font class="label">Second Authentication Server</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="second_aut_serv_hostname" type="text" maxlength="15" size="15" class="formText" value="<% get("system.ports.authP.radiusAuth2Server");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">First Accounting Server</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="first_account_serv_hostname" type="text" maxlength="15" size="15" class="formText" value="<% get("system.ports.authP.radiusAccServer");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
	<td align="left" width="180"><font class="label">Second Accounting Server</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="second_account_serv_hostname" type="text" maxlength="15" size="15" class="formText" value="<% get("system.ports.authP.radiusAcc2Server");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">Secret</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="secret" type="password" maxlength="30" size="15" class="formText" value="<% get("system.ports.authP.radiusSecret");%>"></font>
	</td>
  </tr>
  <tr align="left" valign="top"> 
	<td align="left" width="180"><font class="label">Timeout</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="timeout" type="text" size="15" class="formText" value="<% get("system.ports.authP.radiusTimeout");%>"></font>
	</td>
  </tr>	
  <tr align="left" valign="top"> 
    <td align="left" width="180"><font class="label">Retries</font></td>
    <td align="left" width="180">
	<font class="formText"><input name="retries" type="text" size="15" class="formText" value="<% get("system.ports.authP.radiusRetries");%>"></font>
	</td>
  </tr>
</table>
</div>
</form>
<script type="text/javascript">     
   	  showLayer(document.secAuthenticationForm.authType);
</script>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<% get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/ports/physPorts.asp">
   <input type="hidden" name="urlError" value="/normal/ports/physPorts.asp">
   <input type="hidden" name="request" value=<% write(getRequestNumber());%>>
   <input type="hidden" name="system.ports.physP.action" value="2">
   <input type="hidden" name="system.ports.physP.access.users" value="">
   <input type="hidden" name="system.ports.physP.access.authtype" value="">
   <input type="hidden" name="system.ports.authP.nisDomainName" value="">
   <input type="hidden" name="system.ports.authP.nisServerIp" value="">
   <input type="hidden" name="system.ports.authP.ldapServer" value="">
   <input type="hidden" name="system.ports.authP.ldapBase" value="">
   <input type="hidden" name="system.ports.authP.ldapsecure" value="">
   <input type="hidden" name="system.ports.authP.krbServer" value="">
   <input type="hidden" name="system.ports.authP.krbDomainName" value="">
   <input type="hidden" name="system.ports.authP.radiusAuthServer" value="">
   <input type="hidden" name="system.ports.authP.radiusAuth2Server" value="">
   <input type="hidden" name="system.ports.authP.radiusAccServer" value="">
   <input type="hidden" name="system.ports.authP.radiusAcc2Server" value="">
   <input type="hidden" name="system.ports.authP.radiusSecret" value="">
   <input type="hidden" name="system.ports.authP.radiusTimeout" value="">
   <input type="hidden" name="system.ports.authP.radiusRetries" value="">
</form>
</body>
</html>
