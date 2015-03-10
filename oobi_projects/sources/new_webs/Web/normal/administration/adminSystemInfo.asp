
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
  
  <%
  	var self = "system.administration.unitInfo.";
   	var self2 = "system.administration.unitInfo.cpuInfo.";
   	var family_model = getVal('_familymodel_');
	ReqNum = getRequestNumber();
  %>  

  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = 'helpPages/adminSystemInfoHelp.asp';

  function init()
  {
    if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    {
      if (checkLeftMenu('/normal/administration/adminMenu.asp') == 0) {
         parent.menuReady = 0;
         setTimeout('init()', 200);
         return;
      }
      setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
      parent.topMenu.selectItem(parent.menu.topItem);
      parent.menu.selectItem("SYS");
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
      <tr>
        <td colspan="2" class="colorTHeader"><font class="tabsTextBig">System Information</font></td>
      </tr>
<!--- Unit Version Info --->
       <tr class="colorTRow">
        <td width="155" class="colorTRow">Kernel Version</td>
        <td class="colorTRow">
          <%get(self + "versionHtml");%>
        </td>
       </tr>
<!--- Date Info --->
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">Date</td>
        <td class="colorTRowAlt"><%get(self + "date");%></td>
       </tr>
<!--- UpTime Info --->
      <tr class="colorTRow">
        <td class="colorTRow">Up Time</td>
        <td class="colorTRow"><%get(self + "upTime");%></td>
       </tr>
<!--- Power Supply Info --->
<script type="text/javascript">
var PwrSupplyHtml = "<%get(self + "pwrSupplyHtml");%>";
if (PwrSupplyHtml != "SINGLE") document.write('\
      <tr class="colorTRowAlt">\
        <td class="colorTRowAlt">Power Supply State</td>\
        <td class="colorTRowAlt">'+PwrSupplyHtml+'</td>\
       </tr>');
</script>
<!--- CPU Info --->
      <tr>
        <td colspan="2" class="colorTHeader"><font class="tabsTextBig">CPU Information</font></td>
      </tr>
      <tr class="colorTRow">
        <td class="colorTRow">Cpu</td>
        <td class="colorTRow"><%get(self2 + "cpu");%></td>
      </tr>
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">Clock</td>
        <td class="colorTRowAlt"><%get(self2 + "clock");%></td>
      </tr>
      <tr class="colorTRow">
        <td class="colorTRow">Revision</td>
        <td class="colorTRow"><%get(self2 + "revision");%></td>
      </tr>
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">Bogomips</td>
        <td class="colorTRowAlt"><%get(self2 + "bogomips");%></td>
      </tr>
<!--- Memory Info --->
      <tr>
        <td colspan="2" class="colorTHeader"><font class="tabsTextBig">Memory Information</font></td>
      </tr><%get(self + "memHtml");%>
<!--- PCMCIA Info --->
    <%
        if (family_model != "AVCS") {
          write('<tr><td colspan="2" class="colorTHeader"><font class="tabsTextBig">PCMCIA Information</font></td></tr>');
          write(getVal(self + "pcmciaHtml"));
        }
    %>
    </table>
<!-- Ram Disk Usage -->
    <table border="0" cellpadding="2" cellspacing="0" width="450">
      <tr>
        <td colspan="6" class="colorTHeader"><font class="tabsTextBig">Ram Disk Usage</font></td>
      </tr>
        <tr class="colorTRowAlt2">
          <td class="colorTRowAlt2"><font class="colorTRowAlt2">Filesystem</font></td>
          <td class="colorTRowAlt2"><font class="colorTRowAlt2">1k-blocks</font></td>
          <td class="colorTRowAlt2"><font class="colorTRowAlt2">Used</font></td>
          <td class="colorTRowAlt2"><font class="colorTRowAlt2">Available</font></td>
          <td class="colorTRowAlt2"><font class="colorTRowAlt2">Use%</font></td>
          <td class="colorTRowAlt2"><font class="colorTRowAlt2">Mounted</font></td>
        </tr>
        <%get(self + "ramHtml");%>
    </table>
  </td>
</tr>
</table>  
<!-- End  Ram Disk Usage -->
<form name="configForm" method="POST" action="/goform/Dmf">
  <input type="hidden" name="system.req.action" value="">
  <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
  <input type="hidden" name="urlOk" value="/normal/administration/adminSystemInfo.asp">
  <input type="hidden" name="urlError" value="/normal/administration/adminSystemInfo.asp">
  <input type="hidden" name="request" value="0">
  <input type="hidden" name="system.tablePage.bogus" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
