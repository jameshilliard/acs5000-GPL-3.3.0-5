<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">
	
<% ReqNum = getRequestNumber();%>
	var profile = <%get("system.security.profile.type");%>;
	var direct = <%get("system.security.profile.directaccess");%>;

	top.currentPage = self;
	top.helpPage = 'helpPages/generalConfigHelp.asp';
  	parent.window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  
	function init()
	{
		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady) {
			if (checkLeftMenu('/normal/kvm/configKVMMenu.asp') == 0) {
				top.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
			top.topMenu.selectItem(top.menu.topItem);
			top.menu.selectItem("GC");
		} else
			setTimeout('init()', 200);
	}
	 
	function copyData()
	{
		for (var i=0;i<4;i++) {
			document.configForm.elements[i+5].value =
				document.generalForm.elements[i].value;
		}
		if (document.generalForm.elements[1].checked == true) {
			document.configForm.elements[5].value = 1;
		} else {
			document.configForm.elements[5].value = 0;
		}
	}

     function form_verification()
	 {
		continue_verification = true;
		var form_location = document.generalForm;
		checkSequence (form_location.escapeseq, 2);//Common Escape Sequence
		if (continue_verification == true) {
			checkSequence (form_location.sunmodifier, 0);
		}
	  }
	 
	function showalert()
	{
		if (document.generalForm.authtype.value > 128)
			alert('Authentication type changed.\nPlease reconfigure related parameters in the Authentication page.');
	}

	function submit()
	{		 
		form_verification();

		if (continue_verification != true) return;
		if (continue_verification == true) 
		{
			if (top.kvmAnalog == 0 && (profile == 2 || (profile == 1 && direct != 0))) {
				if (document.generalForm.directacc.checked) {
					document.configForm.elements[5].value = 1;
				} else {
					document.configForm.elements[5].value = 0;
				}
			} else {
				document.configForm.elements[5].value = <%get("system.kvm.conf.general.directacc");%>;
			}
		}
	
		document.configForm.elements[6].value = document.generalForm.escapeseq.value;
		document.configForm.elements[7].value = document.generalForm.sunmodifier.value;
		if (top.kvmAnalog == 0) {
			document.configForm.elements[8].value = document.generalForm.authtype.value;
		}
		else {
			document.configForm.elements[8].value = <%get("system.kvm.conf.general.authtype");%>;
		}
		document.configForm.submit();
		top.mainReady = null;
	}	

	function switchPortAuth()
	{
		if (top.kvmAnalog == 0 && 
			(profile == 2 || (profile == 1 && direct != 0))) {
			document.generalForm.directacc.disabled = false;
			if (document.generalForm.directacc.checked == true) {
				document.generalForm.authtype.disabled = false;
			} else {
				document.generalForm.authtype.disabled = true;
			}
		} else {
			document.generalForm.directacc.disabled = true;
			document.generalForm.authtype.disabled = true;
		}
	}
	</script>	 
</head>

<body class="bodyForLayers" onload="init();">
<form name="generalForm" method="POST" action=" ">
<table align="center" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td colspan="2" align="center" height="30" valign="middle">
<table align="center" cellspacing="0" cellpadding="0" border="0">
<tr><td>
<script type="text/javascript">
	if (top.kvmAnalog == 0) {
		if (profile == 2 || (profile == 1 && direct != 0)) {
			chda = '<%if (getVal("system.kvm.conf.general.directacc") != 0) write(" checked");%>';
		} else {
			chda = '';
		}
		document.write('<input type="checkbox" name="directacc" value="" onclick="switchPortAuth()"'+chda+'>');
		document.write('<td><font class="label">&nbsp;&nbsp;Direct Access</font></td>');
	} else {
		document.write('<input type="hidden" name="directacc" value="0">');
	}
</script>
</td></tr></table>	
</td></tr>
<tr>
	<td colspan="2" height="30" valign="middle" align="center">
<table align="center" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td><font class="label">Common Escape Sequence&nbsp;&nbsp;</font></td>
	<td><input name="escapeseq" type="text" size="15" maxlength="15" class="formText" value="<%get("system.kvm.conf.general.escapeseq");%>"></td>
</tr>
<tr>
	<td><font class="label">Sun Keyboard Modifier Keys&nbsp;&nbsp;</font></td>
	<td><input name="sunmodifier" type="text" size="15" maxlength="15" class="formText" value="<%get("system.kvm.conf.general.sunmodifier");%>"></td>
</tr>
</table>
</td></tr>
<tr><td>
<script type="text/javascript">
if (top.kvmAnalog == 0) {
<%
	var selNone = "";
	var selLocal = "";
	var selRadius = "";
	var selTacplus = "";
	var selKerberos = "";
	var selLdap = "";
	var selRadDL = "";
	var selTacDL = "";
	var selKrbDL = "";
	var selLdapDL = "";
	var selNTLM = "";
	var selNTLMDL = "";
	var selDSView = "";
	var selDSViewLocal = "";
	var selDSViewDownLocal = "";
	var selauth=getVal("system.kvm.conf.general.authtype");
	if (selauth == "0") selNone = "selected";
	else if (selauth == "128") selLocal = "selected";
	else if (selauth == "4096") selRadius = "selected";
	else if (selauth == "8192") selTacplus = "selected";
	else if (selauth == "12288") selKerberos = "selected";
	else if (selauth == "16384") selLdap = "selected";
	else if (selauth == "4098") selRadDL = "selected";
	else if (selauth == "8194") selTacDL = "selected";
	else if (selauth == "12290") selKrbDL = "selected";
	else if (selauth == "16386") selLdapDL = "selected";
	else if (selauth == "24576") selNTLM = "selected";
	else if (selauth == "24577") selNTLMDL = "selected";
	else if (selauth == "28672") selDSView = "selected";
	else if (selauth == "28673") selDSViewLocal = "selected";
	else if (selauth == "28674") selDSViewDownLocal = "selected";
%>
document.write('\
  <tr><td colspan="2" align="center" height="40" valign="middle">\
  <table align="center" cellspacing="0" cellpadding="0" border="0">\
    <tr><td><font class="label">Port Authentication&nbsp;&nbsp;</font></td>\
    <td><select name="authtype" size="1" class="formText" onchange="showalert();">\
      <option value="0" <%write(selNone);%>> None&nbsp;&nbsp; </option>\
      <option value="128" <%write(selLocal);%>> Local&nbsp;&nbsp; </option>\
      <option value="4096" <%write(selRadius);%>> Radius&nbsp;&nbsp; </option>\
      <option value="8192" <%write(selTacplus);%>> TacacsPlus&nbsp;&nbsp; </option>\
      <option value="12288" <%write(selKerberos);%>> Kerberos&nbsp;&nbsp; </option>\
      <option value="16384" <%write(selLdap);%>> Ldap&nbsp;&nbsp; </option>\
      <option value="4098" <%write(selRadDL);%>> RadiusDownLocal&nbsp;&nbsp; </option>\
      <option value="8194" <%write(selTacDL);%>> TacacsPlusDownLocal&nbsp;&nbsp; </option>\
      <option value="12290" <%write(selKrbDL);%>> KerberosDownLocal&nbsp;&nbsp; </option>\
      <option value="16386" <%write(selLdapDL);%>> LdapDownLocal&nbsp;&nbsp; </option>\
      <option value="24576" <%write(selNTLM);%>> NTLM (Windows NT/2000/2003 Domain)</option>\
      <option value="24577" <%write(selNTLMDL);%>> NTLM DownLocal </option>');
      if (<%get('_showDSVIEWoptions_');%> == 1) {
          document.write('\
          <option value="28672" <%write(selDSView);%>> DSView&nbsp;&nbsp; </option>\
          <option value="28673" <%write(selDSViewLocal);%>> DSViewLocal &nbsp;&nbsp; </option>\
          <option value="28674" <%write(selDSViewDownLocal);%>> DSViewDownLocal &nbsp;&nbsp; </option>');
      }
document.write('\
   </select></td>\
   </tr></table>\
   </td></tr>');
} else {
document.write('\
  <tr><td colspan="2" align="center" height="40" valign="middle">\
  <table align="center" cellspacing="0" cellpadding="0" border="0">\
    <tr><td><input type="hidden" name="authtype" value="0"></td></tr>\
  </table></td></tr>');
}
</script>
</td></tr>
</table>
</form>
<!--
<form name="configForm" onsubmit="copyData();" method="POST" action="/goform/Dmf" target="_parent">
-->
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="0">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/kvm/configKVMGeneral.asp">
	<input type="hidden" name="urlError" value="/normal/kvm/configKVMGeneral.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name="system.kvm.conf.general.directacc" value="">
	<input type="hidden" name="system.kvm.conf.general.escapeseq" value="">
	<input type="hidden" name="system.kvm.conf.general.sunmodifier" value="">
	<input type="hidden" name="system.kvm.conf.general.authtype" value="">
</form>
<script type="text/javascript">
switchPortAuth();
top.mainReady = true;
</script>
<% set("_sid_","0");%>
</body>
</html>
