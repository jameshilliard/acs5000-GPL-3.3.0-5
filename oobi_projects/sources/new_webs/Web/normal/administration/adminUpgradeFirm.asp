<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

  <%
    var self = "system.administration.upGradeInfo.";
	ReqNum = getRequestNumber();
	var IPv6 = getVal('_IPv6_');
	var input_size = 15;
	var input_maxlength = 39;
	var ftpSiteSep = "&nbsp;&nbsp;&nbsp;";
	if (IPv6) {
		input_size = 40;
		input_maxlength = 50;
		ftpSiteSep = "<br>";
	}
  %>
  
  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = 'helpPages/adminUpgradeFirmHelp.asp';
  
  if ((parent.family_model == "ACS")||(parent.family_model == "AVCS"))
    var menuPage = "/normal/administration/adminMenu.asp";
  else 
    var menuPage = "/normal/management/manageMenu.asp";

  var sysMsg = "<%get("system.administration.upGradeInfo.sysMsg");%>";
  <%set("system.administration.upGradeInfo.sysMsg", "");%>

  function init()
  {   
    if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    {
      if (checkLeftMenu(menuPage) == 0) {
         parent.menuReady = 0;
         setTimeout('init()', 200);
         return;
      }
      setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
      parent.topMenu.selectItem(parent.menu.topItem);
      parent.menu.selectItem("FW");
      if (sysMsg != "") {
          alert(sysMsg);
          sysMsg = "";
      }
    }
    else
      setTimeout('init()', 200);
  }
  
  function showFTP_site(sel) 
  {
    var index = sel.selectedIndex;
    if (sel.options[index].value != 0) 
    {      
      hide('document', 'Layer1');
    }
    else   
    {
      show('document', 'Layer1');
    }
  }

  function copyData()
  {
    var visLayer1 = getVisible('Layer1');
    var visLayer2 = getVisible('Layer2');

    for (var i=0, x=0; i < 4; i++, x++)
    {
      if (type == 'NN')
      {
        if (!i && visLayer1 == true) { // only first element
          document.configForm.elements[i+5].value = document.Layer1.document.UpgradeFirmForm1.elements[i].value;
          x++;
        }
        if (visLayer2 == true) {
          document.configForm.elements[x+5].value = document.Layer2.document.UpgradeFirmForm2.elements[i].value;
        }
      }
      if (type == 'IE' || type == 'MO')
      {
        if (!i && visLayer1 == true) {
          document.configForm.elements[i+5].value = document.UpgradeFirmForm1.elements[i].value;
          x++;
        }
        if (visLayer2 == true) {
          document.configForm.elements[x+5].value = document.UpgradeFirmForm2.elements[i].value;
        }
      }
      else
      {
        document.configForm.elements[i+5].value = '';
      }
    }
  }

  function upGradeNow()
  {
	form_verification();
	if (continue_verification == true)
	{  
    // set action variable to distinguish from user actually pressing
    // the Upgrade button or navigating in menu
    	if (disallowBlank(document.UpgradeFirmForm1.elements[0], 'Please fill in this field.', true) == true) return;
    	if (disallowBlank(document.UpgradeFirmForm2.elements[0], 'Please fill in this field.', true) == true) return;
    	if (disallowBlank(document.UpgradeFirmForm2.elements[1], 'Please fill in this field.', true) == true) return;
    	if (disallowBlank(document.UpgradeFirmForm2.elements[2], 'Please fill in this field.', true) == true) return;
		if (confirm("Do you want to upgrade now?")) {
			document.configForm.elements[0].value = 1;  
			setQuerySSID(document.configForm);
			document.getElementById("container").style.display="block";
			document.getElementById("maincontent").style.display="none";
			submit();
		}
	}	
  }  

  function form_verification()
  {    
	        var form_location1
			var form_location2
			
	if (type == 'NN') 
	  		{
			form_location1 = document.Layer1.document.UpgradeFirmForm1;
			form_location2 = document.Layer2.document.UpgradeFirmForm2;
			}
	else 
	  		{
			form_location1 = document.UpgradeFirmForm1;
			form_location2 = document.UpgradeFirmForm2;
	  		}
	  
	if (document.UpgradeFirmForm.elements[0].value == 0)
	{checkElement (form_location1.elements[0], true, true, false, false);}
		
  	if (continue_verification == true)													 
	{checkElement (form_location2.elements[0], true, true, false, false);}
	if (continue_verification == true)													 
	{checkElement (form_location2.elements[1], true, true, false, false);}
	if (continue_verification == true)													 
	{checkElement (form_location2.elements[2], true, true, false, false);}  
  } 
  
  function submit()
{ 
    copyData();
    document.configForm.submit();
	parent.mainReady = null;
}
      
  </script>
</head>
<body onload="init();" class="bodyForLayers">

<div id="container">
<div id="loading"><img src="/Images/loading.gif" /></div>
</div>
<div id="maincontent">
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="400" align="center">
    <tr>
        <td>
            <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
                <tr>
                    <td align="center" class="label_for_tables" >
             The upgrade will only be performed if <br>"Upgrade Now" button is pressed.
                    </td>
                </tr>
               </table>
        </td>
    </tr>
</table>  
<table width="300" border="0" align="center" cellpadding="0" cellspacing="0">
<form name="UpgradeFirmForm"  onSubmit="return false;">
  <tr valign="middle"> 
<!--
    <td align="center" height="35" valign="bottom"><font class="label">Type</font>&nbsp;&nbsp;&nbsp; 
      <select name="type" size="1" class="formText" onChange="showFTP_site(this);">
        <option value="0" selected>FTP</option>
      </select>
    </td>
-->
	<td><input type="hidden" name="type" value="0"></td>
  </tr>
</form>
</table>
<div id="Layer1" name="ftp_site" style="visibility:visible;" align="center">
<form name="UpgradeFirmForm1"  onSubmit="return false;">
<table width="300" border="0" align="center" cellpadding="0" cellspacing="5">
  <tr>
    <td>&nbsp;</td>
  </tr>
  <tr>
    <td align="center" valign="middle"><font class="label">FTP Site</font><%write(ftpSiteSep);%><input name="ftpSite" type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" value="<%get("system.administration.upGradeInfo.ftpSite");%>"></td>
  </tr>
</table>
</form>
</div>
<div id="Layer2" name="normal" style="visibility:visible;" align="center">
<form name="UpgradeFirmForm2"  onSubmit="return false;">
<table border="0" align="center" cellpadding="0" cellspacing="5">
<tr align="left" valign="middle"> 
  <td height="30" width="80" align="right"><font class="label">Username</font>&nbsp;&nbsp;</td>
  <td width="80"><input name="username" type="text" maxlength="30" class="formText" size="15" value="<%get("system.administration.upGradeInfo.userName");%>"></td>
  <td width="80" align="right"><font class="label">Password</font>&nbsp;&nbsp;</td>
  <td width="80"><input name="password" type="password" maxlength="30" class="formText" size="15" value=""></td>
</tr>
<tr align="center" valign="middle"> 
  <td height="30" valign="middle" colspan="4"><font class="label">Path and Filename</font>&nbsp;&nbsp;&nbsp;&nbsp;
  <input name="file_version" type="text" class="formText" maxlength="100" size="20" value="<%get("system.administration.upGradeInfo.filePathName");%>"></td>
</tr>
<script type="text/javascript">
if (parent.family_model == "ONS" || parent.family_ext == "KVMNETP") {
	document.write('<input type="hidden" name="run_checksum" value="0">');
} else {
<%
var checkSel = "";
if (getVal("system.administration.upGradeInfo.checkSum") == "1")
        checkSel = "selected";
%>
	document.write('<tr align="center" valign="middle"> \
		  <td height="30" valign="middle" colspan="4"><font class="label">Run Checksum</font>&nbsp;&nbsp;&nbsp;&nbsp;\
		  <select name="run_checksum" size="1" class="formText">\
		        <option value="0">No</option>\
		        <option value="1" <%write(checkSel);%> >Yes</option>\
		  </select></td>\
		</tr>');
}
</script>
<tr align="center" valign="middle"> 
    <td colspan="4" height="60" align="center">
  <a href="javascript:upGradeNow();">
  <img src="/Images/upgrade_nowButton.gif" alt="" width="80" height="21" border="0"></a>
  </td>
  </tr>
</table>
</form>
</div>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="/normal/administration/adminExecuteUpgrade.asp">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/administration/adminUpgradeFirm.asp">
   <input type="hidden" name="urlError" value="/normal/administration/adminUpgradeFirm.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
<!--*************************************************************************-->
<!--these hidden inputs are used to store values of form elements inside layers-->
   <input type="hidden" name="<%write(self);%>ftpSite" value="">
   <input type="hidden" name="<%write(self);%>userName" value="">
   <input type="hidden" name="<%write(self);%>passWd" value="">
   <input type="hidden" name="<%write(self);%>filePathName" value="">
   <input type="hidden" name="<%write(self);%>checkSum" value="">  
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</div> 
</body>
</html>
