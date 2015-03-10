<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
	
  <% ReqNum = getRequestNumber(); %>
	<%
		var IPv6 = getVal('_IPv6_');
		var input_size = 15;
		var input_maxlength = 30;
		if (IPv6) {
			input_size = 30;
			input_maxlenght = 50;
		}
	%>

    var pageMenu = '/normal/administration/adminMenu.asp';

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
	parent.helpPage = 'helpPages/adminBackupConfigHelp.asp';
	
      function init()
      {
         if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
         {
            if (checkLeftMenu(pageMenu) == 0) {
               parent.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
            setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
            parent.topMenu.selectItem(parent.menu.topItem);
            parent.menu.selectItem("BKP");
			callAlert(document.configForm.elements[6].value);
         }
         else
            setTimeout('init()', 200);
      }

	  function form_verification_ftp()
	  {
	  var form_location = document.configForm;

	  checkElement (form_location.elements[10], true, true, false, false);
	  if (continue_verification == true)
	  checkElement (form_location.elements[11], true, true, false, false);
	  if (continue_verification == true)
	  checkElement (form_location.elements[12], true, true, false, false);
	  if (continue_verification == true)
	  checkElement (form_location.elements[13], true, true, false, false);	  	  
	  } 

	function copyData() {
		if (document.configForm.elements[8].checked == true) 
			document.configForm.elements[8].value="1";
		else
			document.configForm.elements[8].value="0";
		if (document.configForm.elements[9].checked == true) 
			document.configForm.elements[9].value="1";
		else
			document.configForm.elements[9].value="0";
	}

	  function execFTP(command)
	  {
		continue_verification = true;
		if (command == "restoreconf") {
			if (!confirm("The backup configuration will be activated after the load."))
				return;
		}
	  	if (document.configForm.elements[7].value == "1")
	  		form_verification_ftp();
		else
			copyData();
		if (continue_verification == true)// check form before submit
	    {
			document.configForm.elements[5].value = command;
			document.configForm.elements[6].value = "";
            setQuerySSID(document.configForm);
      		document.configForm.submit();  
			parent.mainReady = null;  
		}	  
	  }
	  
      function submit()
      {	        
		document.configForm.elements[5].value = "";
		document.configForm.elements[6].value = "";
        setQuerySSID(document.configForm);
        document.configForm.submit();
		parent.mainReady = null;  
      }
      
      function callAlert(string)
      {
      if (string != "")
        alert(string);
      }

      function showLayer() {
		if (document.configForm.elements[7].value == "1") {
			if ("<%get('_acs5k_');%>" != "1") {
				hide('document','Layer1');
			}
			show('document','Layer2');
		} else {
			hide('document','Layer2');
			show('document','Layer1');
		}
	  }

     </script>
</head>

<body onload="init();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/administration/adminBackupConfig.asp">
   <input type="hidden" name="urlError" value="/normal/administration/adminBackupConfig.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
   <input type="hidden" name="system.administration.backupConfig.command" value="">
   <input type="hidden" name="system.req.bogus" value="<%get("system.administration.backupConfig.Message");%>">

<table width="100%" align="center" cellspacing="0" cellpadding="0" border="0">
<tr><td align="center" valign="middle">
<script type="text/javascript">
if ("<%get('_acs5k_');%>" != "1")
document.write('\
	<font class="label">Type</font>&nbsp;&nbsp;\
	<select name="system.administration.backupConfig.bkptype" size="1" class="formText" onChange="showLayer();">\
		<option value="1" selected>FTP</option>\
		<option value="2" >Storage Device</option>\
	</select>');
else
document.write('\
	<input name="system.administration.backupConfig.bkptype" type="hidden" value="1">');

document.write('</td></tr><tr><td align="center" valign="middle">');

if ("<%get('_acs5k_');%>" != "1") {
	document.write('<table id="Layer1" align="center" cellspacing="0" cellpadding="0" border="0">');
	document.write('<tr><td><input name="system.administration.backupConfig.SDDefaultoption" type="checkbox" value="1">');
	document.write('<font class="label"> Default Configuration </font></td>');
	document.write('<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>');
	document.write('<td><input name="system.administration.backupConfig.SDReplaceoption" type="checkbox" value="1">');
	document.write('<font class="label"> Replace Configuration </font></td></tr></table>');
} else {
	document.write('<input name="system.administration.backupConfig.SDDefaultoption" type="hidden" value="1">');
	document.write('<input name="system.administration.backupConfig.SDReplaceoption" type="hidden" value="1">');
}
</script>

</td></tr><tr><td align="center" valign="middle">

	<table id="Layer2" align="center" cellspacing="0" cellpadding="0" border="0">
		<tr>
<script type="text/javascript">
if ("<%get('_acs5k_');%>" != "1")
document.write('\
		<td><font class="label">Server IP &nbsp;</font></td>');
else
document.write('\
		<td><font class="label">FTP Server IP &nbsp;</font></td>');
</script>
		<td><input name="system.administration.backupConfig.ServerName" type="text" class="formText" maxlength="30" size="<%write(input_size);%>" value="<%get("system.administration.backupConfig.ServerName");%>"></td>
		</tr>
		<tr>
		<td><font class="label">Path and Filename &nbsp;</font></td>
		<td><input name="system.administration.backupConfig.PathName" type="text" class="formText" size="<%write(input_size);%>" maxlength="100" value="<%get("system.administration.backupConfig.PathName");%>"></td>
		</tr>
		<tr>
		<td><font class="label">Username &nbsp;</font></td>
		<td><input name="system.administration.backupConfig.UserName" type="text" class="formText" size="<%write(input_size);%>" maxlength="30" value="<%get("system.administration.backupConfig.UserName");%>"></td>
		</tr>
		<tr>
		<td><font class="label">Password &nbsp;</font></td>
		<td><input name="system.administration.backupConfig.Password" type="password" class="formText" size="<%write(input_size);%>" maxlength="30" value=""></td>
		</tr>
	</table>
</td></tr>
</form>

<tr>
	<td colspan="2" align="center" height="40" valign="bottom">
	<a href="javascript:execFTP('saveconf');"><img src="/Images/saveButton.gif" border="0" alt=""></a>
	&nbsp;&nbsp;
	<a href="javascript:execFTP('restoreconf');"><img src="/Images/loadButton.gif" border="0" alt=""></a>
	</td>
</tr>

</td></tr></table>
</form>

<script type="text/javascript">
parent.mainReady = true;
showLayer();
</script>
<%set("_sid_","0");%>
</body>
</html>
