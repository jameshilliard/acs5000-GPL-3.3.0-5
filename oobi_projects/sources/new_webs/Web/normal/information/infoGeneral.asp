
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
  
  <%
    // Set page-wide variables:
    var self = "system.administration.unitInfo.";
    var self2 = "system.administration.unitInfo.cpuInfo.";
	ReqNum = getRequestNumber();
  var IPv6 = getVal('_IPv6_');
  var staticIp6;
  if (IPv6) {
    staticIp6 = getVal("system.device.ip6");
    if (staticIp6 != "") {
      var prefix6 = getVal("system.device.prefix6");
      if (prefix6 > 0) {
        staticIp6 = staticIp6 + "/" + prefix6;
      }
    }
  }
  %>

  parent.currentPage = self;
  parent.helpPage = '/normal/helpPages/adminSystemInfoHelp.asp';
  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

  var menuPage = "/normal/administration/adminMenu.asp";

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
      parent.menu.selectItem("SYS");
    }
    else
      setTimeout('init()', 200);
  }

  function splitMHz(mhz)
  {
	var speeds = mhz.split('MHz');
	var speedsstr = "";
	speedsstr = speeds[0]+"MHz / " + speeds[1] + "MHz";
	return speedsstr;
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
<!--- appliance serial number --->
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">Serial Number:</td>
        <td class="colorTRowAlt"><%get(self + "serialNumberHtml");%></td>
       </tr>
<!--- Unit Version Info --->
       <tr class="colorTRow">
        <td width="155" class="colorTRow">Kernel Version:</td>
        <td class="colorTRow">
          <%get(self + "versionHtml");%>
        </td>
       </tr>
<!--- Date Info --->
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">Date:</td>
        <td class="colorTRowAlt"><%get(self + "date");%></td>
       </tr>
<!--- UpTime Info --->
      <tr class="colorTRow">
        <td class="colorTRow">Up Time:</td>
        <td class="colorTRow"><%get(self + "upTime");%></td>
       </tr>
<!--- Power Supply Info --->
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">Power Supply State:</td>
        <td class="colorTRowAlt"><%get(self + "pwrSupplyHtml");%></td>
       </tr>
<!--- Mac Address --->
      <tr class="colorTRow">
        <td class="colorTRow">System Mac Address:</td>
        <td class="colorTRow"><%get(self + "macAddress");%></td>
       </tr>
<!--- IPv4 and Static IPv6 Addresses --->
<%if (IPv6) {
write('\
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">IPv4 Address:</td>
        <td class="colorTRowAlt">' + getVal("system.device.ip") + '</td>
       </tr>
      <tr class="colorTRow">
        <td class="colorTRow">Static IPv6 Address:</td>
        <td class="colorTRow">' + staticIp6 + '</td>
       </tr>
');}%>
<!--- CPU Info --->
      <tr>
        <td colspan="2" class="colorTHeader"><font class="tabsTextBig">CPU Information</font></td>
      </tr>
      <tr class="colorTRow">
        <td class="colorTRow">Cpu:</td>
        <td class="colorTRow"><%get(self2 + "cpu");%></td>
      </tr>
	<tr class="colorTRowAlt">
    <script type="text/javascript">
	document.write(' <td class="colorTRowAlt">Clock:</td>' );
	document.write('<td class="colorTRowAlt">' + "<%get(self2 + "clock");%>" + '</td>');
	</script>
	</tr>
      <tr class="colorTRow">
        <td class="colorTRow">Revision:</td>
        <td class="colorTRow"><%get(self2 + "revision");%></td>
      </tr>
      <tr class="colorTRowAlt">
        <td class="colorTRowAlt">Bogomips:</td>
        <td class="colorTRowAlt"><%get(self2 + "bogomips");%></td>
      </tr>
<!--- Memory Info --->
      <tr>
        <td colspan="2" class="colorTHeader"><font class="tabsTextBig">Memory Information</font></td>
      </tr><%get(self + "memHtml");%>
<!--- PCMCIA Info --->
    <%
        if (getVal('_acs5k_') != "1") {
          write('<tr> <td colspan="2" class="colorTHeader"><font class="tabsTextBig">PCMCIA Information</font></td></tr>');
          write(getVal(self + "pcmciaHtml"));
        }
    %>
</table>
</td></tr>
  <td align="center" valign="middle">
<table border="0" cellpadding="2" cellspacing="0" width="450">
<!-- Ram Disk Usage -->
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
  </td>
</tr>
</table>  
<!-- End  Ram Disk Usage -->
<form name="configForm" method="POST" action="/goform/Dmf">
  <input type="hidden" name="system.req.action" value="">
  <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
  <input type="hidden" name="urlOk" value="/normal/information/infoGeneral.asp">
  <input type="hidden" name="urlError" value="/normal/information/infoGeneral.asp">
  <input type="hidden" name="request" value="0">
  <input type="hidden" name="system.tablePage.bogus" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
