<html>
<head>
<title>Add Outlet</title>

<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script language="JavaScript" type="text/JavaScript">
    
	function outlet_number_verify()
	{
		var n1 = document.OutletsForm.elements[1].value;
		var nouts = n1.split(',');
		for (var i = 0; i < nouts.length; i++) {
			if (nouts[i] < 1 || nouts[i] > 128) {
					continue_verification = false;
					alert ('Outlet number must be between 1 and 128');
					document.OutletsForm.elements[1].focus();
					document.OutletsForm.elements[1].select();
					return(false);
			}
		}
		return(true);
	}

	function outlet_verify(index)
	{
		var ol = opener.document.PowerMgmForm.outlets[index].text;
		var ol1 = ol.split(':');
		for (var i=1; i < ol1.length ; i++) {
			if (ol1[i] != "") {
				break;
			}
		}
		var list = ol1[i].split(',');
		var n1 = document.OutletsForm.elements[1].value;
		var nouts = n1.split(',');
		for (var i = 0; i < nouts.length; i++) {
			for (var j=0; j < list.length; j++) {
				if (list[j] == nouts[i]) {
					continue_verification = false;
					alert ('Outlet is already configured for this port');
					return;
				}
			}
		}
	}

	function form_verification()
	{
		checkElement (document.OutletsForm.elements[1], true, false, false, false);
		if (continue_verification != true) return;

		checkPermitedChars (document.OutletsForm.elements[1], '0123456789,', 'The Outlet Number you entered is not valid.');

		if (continue_verification != true) return;

		if (outlet_number_verify() == false) return;

		for (var i = 0; i < opener.document.PowerMgmForm.outlets.length; i++) {
			if (opener.document.PowerMgmForm.outlets[i].value ==
   				document.OutletsForm.elements[0].value) {
				outlet_verify(i);
				if (continue_verification != true) return;
			}
		}		
	}
	
	function copyData()
	{
	try {
	     if (opener.window.name != document.OutletsForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
		form_verification();
		if (continue_verification == true)
		{
		opener.document.configForm.elements[2].value ="/normal/ports/portsPhysPowerMan.asp";
		opener.document.configForm.elements[3].value ="/normal/ports/portsPhysPowerMan.asp";
		opener.document.configForm.elements[5].value ="0";
		opener.document.configForm.elements[11].value = document.OutletsForm.ipdu.value;
		opener.document.configForm.elements[12].value = document.OutletsForm.outlet.value;
		opener.popup_submit = 1;
		opener.submit();
		window.close();
		}
	} catch (error) {
	  self.close();
	}
	}
</script>
</head>
<body class="bodyMain">
<table border="0" align="center" cellpadding="0" cellspacing="10">
<tr valign="top"> 
    <td height="40" colspan="2" align="center">
	     <a href="javascript:copyData();">
         <img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
         &nbsp;&nbsp;&nbsp;&nbsp;
		 <a href="javascript:window.close();">
         <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
	</td>
  </tr>
<form name="OutletsForm" method="POST" action=" " onSubmit="return false;" target="<%write(getQueryVal("target", "main"));%>">
  <tr valign="bottom"> 
    <td align="left"><font class="label">Power Management Alias</font></td>
    <td align="left">
	<select name="ipdu" size="1" class="formText">
	<%get("system.ports.physP.powerMgm.outEntry.pmIpduNameHtml");%>
	</select>
	</td>
  </tr>
  <tr valign="bottom">
    <td align="left"><font class="label">Outlet Number</font></td>
    <td align="left"><input class="formText" name="outlet" maxlength="15" type="text" size="15"></td>
  </tr>
  </table>
</form>
</body>
</html>
