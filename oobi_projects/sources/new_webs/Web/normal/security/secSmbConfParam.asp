<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript"> 
<% ReqNum = getRequestNumber(); %>

    top.currentPage = self;
    top.helpPage = 'helpPages/secSmbHelp.asp';
    parent.window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    if ((top.family_model == "ACS")||(top.family_model == "AVCS")) {
        var menuPage = "/normal/security/securityMenu.asp";
	} else {
		var menuPage = "/normal/security/secAuthMenu.asp";
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
			top.menu.leftHeadMenu("AUTH");
            setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
            top.topMenu.selectItem(top.menu.topItem);
            top.menu.selectItem("AUTH");
        } else
            setTimeout('init()', 200);
    }

    function copyData()
    {
		for (var i=0;i<3;i++) {
            document.configForm.elements[i+5].value = document.secAuthenticationForm.elements[i].value;
		}
    }
	function form_verification()
	{
		continue_verification = true;
		 var form_location = document.secAuthenticationForm;
        if (continue_verification == true)//Domain
           {checkElement (form_location.smbAuthdomain, true, true, false, false);}
        if (continue_verification == true)//Primary Domain Controller
           {checkElement (form_location.smbAuthserver1, true, false, false, false);}
        if (continue_verification == true)//Primary Domain Controller
           {checkHostOrIP (form_location.smbAuthserver1);}
        if (continue_verification == true)//Secondary Domain Controller
           {checkHostOrIP (form_location.smbAuthserver2);}
	}

        function submit()
        {
	    if (document.secAuthenticationForm.elements[0].value != '') {
	            form_verification();
        	    if (continue_verification == true) {
                	copyData();
	                document.configForm.submit();
        	        top.mainReady = null;
        	    }
	    } else {
	        copyData();
                document.configForm.submit();
                top.mainReady = null;
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
</table>
 <table width="360" border="0" align="center" cellpadding="0" cellspacing="10">
  <tr align="left" valign="top">
   <td align="left"><font class="label">Domain</font></td>
    <td align="left">
    <input name="smbAuthdomain" type="text" size="15" class="formText" value=<%get("system.security.smbP.authdomain");%>>
    </td>
  </tr>
  <tr align="left" valign="top">
    <td align="left"><font class="label">Primary Domain Controller</font></td>
    <td align="left">
    <input name="smbAuthserver1" type="text" size="15"  maxlength="255" class="formText" value=<%get("system.security.smbP.authserver1");%>>
    </td>
  </tr>
  <tr align="left" valign="top">
    <td align="left"><font class="label">Secondary Domain Controller</font></td>
    <td align="left"><input name="smbAuthserver2" type="text" size="15" maxlength="255" class="formText" value=<%get("system.security.smbP.authserver2");%>>
    </td>
  </tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
   <input type="hidden" name="system.req.action" value="0">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/security/secSmbConf.asp">
   <input type="hidden" name="urlError" value="/normal/security/secSmbConf.asp">
   <input type="hidden" name="request" value=<% write(ReqNum);%>>
   <input type="hidden" name="system.security.smbP.authdomain" value="">
   <input type="hidden" name="system.security.smbP.authserver1" value="">
   <input type="hidden" name="system.security.smbP.authserver2" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
