<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

<% ReqNum = getRequestNumber(); %>	
	
   window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
   parent.currentPage = self;
   parent.helpPage = 'helpPages/portsStatusHelp.asp';

   if ((parent.family_model == "ACS") || (parent.family_model == "AVCS"))
     var topMenu = '/normal/ports/portsMenu.asp';
   else
     var topMenu = '/normal/information/infoMenu.asp';

   function init()
   {
      if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
      {
         if (checkLeftMenu(topMenu) == 0) {
            parent.menuReady = 0;
            setTimeout('init()', 200);
            return;
         }
         setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
         parent.menu.selectItem('ST');
         parent.topMenu.selectItem(parent.menu.topItem);
      }
      else
         setTimeout('init()', 200);
   }

   function submit()
   {
      document.configForm.submit();
	  parent.mainReady = null;
   }
    
    </script>
</head>
<body onload="init();" class="bodyMain">
<table border="0" cellpadding="0" cellspacing="0" align="center">
<tr>
   <td height="40" colspan="1" align="center" valign="bottom">
   <a href="javascript:loadNextPage('/normal/ports/portsStatus.asp');"><img src="/Images/refresh_button.gif" alt="" border="0"></a></td>
</tr>
</table>    
<script type="text/javascript">
document.write('<table border="0" cellpadding="2" cellspacing="0" ALIGN="LEFT">');
document.write('<tr>&nbsp;</tr>');
document.write('<tr class="colorTHeader">');
document.write('<td width="42"><font class="tabsTextBig">&nbsp;Port</font></td>');
document.write('<td width="160"><font class="tabsTextBig">Alias</font></td>');
document.write('<td width="200"><font class="tabsTextBig">RS232 Signal Status</font></td>');
document.write('<td><font class="tabsTextBig">Current User(s)</font></td></tr>');

var portStatus = new Array(<%get("system.ports.portstatus");%>);
if (parent.family_model == "ONS")
var numPorts = "<%get("system.device.number2");%>";
else
var numPorts = "<%get("system.device.number");%>";
var line;

for (var j=0; j < numPorts; j++) {
	if (j%2 != 0) 
		line = "colorTRowMiddle";
	else
		line = "colorTRowAltMiddle";

	document.write('<tr class="'+line+'">');
	document.write('<td><font class="label">'+ (j+1) + '</font></td>');
	document.write('<td><font class="label">'+ portStatus[j][0] + '</font></td>');
	document.write('<td><font class="label">'+ portStatus[j][1] + '</font></td>');
	document.write('<td><font class="label">'+ portStatus[j][2] + '</font></td></tr>');
}
document.write('</table>');
parent.mainReady = true;
</script>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/ports/portsStatus.asp">
   <input type="hidden" name="urlError" value="/normal/ports/portsStatus.asp">
   <input type="hidden" name="request" value="0">
   <input type="hidden" name="system.tablePage.bogus" value="">
</form>   
<%set("_sid_","0");%>
</body>
</html>
