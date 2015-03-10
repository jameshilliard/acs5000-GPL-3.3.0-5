
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

  <% ReqNum = getRequestNumber(); %>	
  
  parent.currentPage = self;
  parent.helpPage = 'helpPages/infoPortStatusHelp.asp';
  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

  function init()
  {
    if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    {
      if (checkLeftMenu('/normal/information/infoMenu.asp') == 0) {
         parent.menuReady = 0;
         setTimeout('init()', 200);
         return;
      }
      parent.topMenu.selectItem(parent.menu.topItem);
      parent.menu.selectItem("KPS");
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
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
  <td align="center" valign="middle">
    <table border="0" cellpadding="2" cellspacing="0" width="450">
<%
var status=getVal("system.administration.unitInfo.kvmPortStatus",0);
var family_model=getVal('_familymodel_');
if ((status != "") && (status != 0)) {
      write("<tr><td colspan=\"2\" class=colorTHeader>");
	if (family_model == "ONS") 
	  write("<font class=tabsTextBig>KVM User 1 Status</font>");
	else
      write("<font class=tabsTextBig>KVM Station 1 Status</font>");
      write("</td></tr>");
      write(status);
}
status=getVal("system.administration.unitInfo.kvmPortStatus",1);
if (status != "" && (status != 1)) {
      write("<tr><td colspan=\"2\" class=colorTHeader>");
	if (family_model == "ONS")
      write("<font class=tabsTextBig>KVM User 2 Status</font>");
	else
      write("<font class=tabsTextBig>KVM Station 2 Status</font>");
      write("</td></tr>");
      write(status);
}
status=getVal("system.administration.unitInfo.kvmPortStatus",2);
if (status != "" && (status != 2)) {
      write("<tr><td colspan=\"2\" class=colorTHeader>");
	if (family_model == "ONS")
      write("<font class=tabsTextBig>KVM User 3 Status</font>");
	else
      write("<font class=tabsTextBig>KVM Station 3 Status</font>");
      write("</td></tr>");
      write(status);
}
status=getVal("system.administration.unitInfo.kvmPortStatus",3);
if (status != "" && (status != 3)) {
      write("<tr><td colspan=\"2\" class=colorTHeader>");
	if (family_model == "ONS")
      write("<font class=tabsTextBig>KVM User 4 Status</font>");
	else
      write("<font class=tabsTextBig>KVM Station 4 Status</font>");
      write("</td></tr>");
      write(status);
}
status=getVal("system.administration.unitInfo.kvmPortStatus",4);
if (status != "" && (status != 4)) {
      write("<tr><td colspan=\"2\" class=colorTHeader>");
	if (family_model == "ONS")
      write("<font class=tabsTextBig>KVM User 5 Status</font>");
	else
      write("<font class=tabsTextBig>KVM Station 5 Status</font>");
      write("</td></tr>");
      write(status);
}
status=getVal("system.administration.unitInfo.kvmPortStatus",5);
if (status != "" && (status != 5)) {
      write("<tr><td colspan=\"2\" class=colorTHeader>");
	if (family_model == "ONS")
      write("<font class=tabsTextBig>KVM User 6 Status</font>");
	else
      write("<font class=tabsTextBig>KVM Station 6 Status</font>");
      write("</td></tr>");
      write(status);
}
%>
    </table>
  </td>
</tr>
</table>  
<form name="configForm" method="POST" action="/goform/Dmf">
  <input type="hidden" name="system.req.action" value="">
  <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
  <input type="hidden" name="urlOk" value="/normal/information/infoPortStatus.asp">
  <input type="hidden" name="urlError" value="/normal/information/infoPortStatus.asp">
  <input type="hidden" name="request" value="0">
  <input type="hidden" name="system.tablePage.bogus" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
