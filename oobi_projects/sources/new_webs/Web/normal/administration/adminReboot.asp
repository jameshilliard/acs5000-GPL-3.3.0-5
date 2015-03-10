<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

  <%
    var self = "system.administration.unitInfo."; 
	ReqNum = getRequestNumber();
  %>  

  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = 'helpPages/adminRebootHelp.asp';

  if ((parent.family_model == "ACS")||(parent.family_model == "AVCS"))
    var menuPage = "/normal/administration/adminMenu.asp";
  else 
    var menuPage = "/normal/management/manageMenu.asp";

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
       parent.menu.selectItem("REB");
    }
    else
        setTimeout('init()', 200);
  }

  function reboot()
  {
    if (confirm("Reboot your system now?")) {
      document.configForm.elements[5].value = 1;
      document.configForm.submit();
    }
  }

  function submit()
  {
    document.configForm.submit();
	parent.mainReady = null;
  }

  </script>
</head>

<body onload="init();" class="bodyMain">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
  <td align="center"  height="50%"  valign="top">
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" align="center">
    <tr>
        <td>
            <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
                <tr>
                    <td align="center" class="helpBox">
                       This administration tool will reboot the system.
                    </td>
                </tr>
            </table>
        </td>
    </tr>    
</table>  
  </td>
</tr>
<tr>
  <td align="center" valign="top">
<table border="0" cellpadding="0" cellspacing="0" align="center">
    <tr>
        <td><a href="javascript:reboot();"><img src="/Images/rebootButton.gif" border="0" alt="" width="47" height="21"></a></td>
    </tr>    
</table>  
  </td>
</tr>
</table>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/administration/adminReboot.asp">
   <input type="hidden" name="urlError" value="/normal/administration/adminReboot.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
<!----------------------------------------------------------------------------->
   <input type="hidden" name="<%write(self);%>reboot" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>  
</body>
</html>
