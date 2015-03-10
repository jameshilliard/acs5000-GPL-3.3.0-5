
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
   <%
      // Set page-wide variables:
      var self = "system.netSettings.";
      var device = "system.device.";
      var model = getVal(device + "model");

      var ReqNum = getRequestNumber();
      var SID = getVal("_sid_");
      trial = getVal(self + "netMask1"); // representative of the rest (could test all if anal)
      if (!__success__) // if any error accessing, forward to login page!
         write('top.window.location.replace("/login.asp?e=' + __error__ + '");');
      var IPv6 = getVal('_IPv6_');
   %>

      var page = 0;
	  parent.helpPage = 'helpPages/netSettingsHelp.asp';
      window.name = '<%get("system.device.ip");%>_<%write(SID);%>_<%write(ReqNum);%>';
      parent.currentPage = self;
      var form_location = document.settings;

      function copyData() {    
         moveData(document.configForm, document.settings);
      }

      function fill() {
         document.settings.dhcp.checked = <%get("system.netSettings.dhcp");%>? true : false;

<%if (IPv6) {
write('document.settings.DHCPv6Opts.checked = ');
write(getVal("system.netSettings.DHCPv6Opts"));
write(' ? true : false;\
         document.settings.ethIpPrefix6.value = document.settings.ethIp6.value;
         if (document.settings.ethIpPrefix6.value.length && document.settings.ethPrefix6.value > 0) {
            document.settings.ethIpPrefix6.value += "/" + document.settings.ethPrefix6.value;
         }
         document.settings.IPmode.value = parseInt(document.settings.enableIPv4.value) + 2 * parseInt(document.settings.enableIPv6.value);
         updateIPmode();
');
} else {
write('showDhcp();');
}%>
      }

      function init() {
         if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
            page = parent.menu.getPageIndex('NET', parent.menu.menuPages);
            parent.menu.selectItem('NET');
            setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
            if (top.family_model == "AVCS")
               parent.controls.document.getElementById('back').style.visibility = "hidden";
            else
               parent.controls.document.getElementById('back').style.visibility = "visible";
            parent.controls.document.getElementById('next').style.visibility = "visible";
         }else
            setTimeout('init()', 200);
      }

<%if (IPv6) {
write('\
      function showDhcp() {
         var IpMode = parseInt(getIPmode());
         document.settings.dhcp.value = (document.settings.dhcp.checked &&
                                        !document.settings.dhcp.disabled)? 2 : 0;
         document.settings.DHCPv6Opts.value = (document.settings.DHCPv6Opts.checked &&
                                              !document.settings.DHCPv6Opts.disabled)? 3 : 0;
         document.settings.dhcp.checked = parseInt(document.settings.dhcp.value)? true : false;
         document.settings.DHCPv6Opts.checked = parseInt(document.settings.DHCPv6Opts.value)? true : false;
         document.settings.IPv6method.value = document.settings.DHCPv6Opts.checked? 2 :
         									(document.settings.ethIpPrefix6.value.length? 1 : 0);
         if (document.settings.dhcp.value == 0 && document.settings.DHCPv6Opts.value == 0) {
            show2("document", "netSettings");
            switch (IpMode) {
               case 1:
                  show2("document", "gateWay");
                  hide2("document", "gateWay6");
               break;
               case 2:
                  show2("document", "gateWay6");
                  hide2("document", "gateWay");
               break;
               case 3:
                  show2("document", "gateWay,gateWay6");
               break;
            }
         } else {
            hide2("document", "netSettings,gateWay,gateWay6");
         }
         if (document.settings.dhcp.value == 0 && (IpMode & 1)) {
            show2("document", "ipSettings");
         } else {
            hide2("document", "ipSettings");
         }
         if (document.settings.DHCPv6Opts.value == 0 && (IpMode & 2)) {
            show2("document", "ip6Settings");
         } else {
            hide2("document", "ip6Settings");
         }
      }

      function updateIPmode() {
         document.settings.enableIPv4.value = document.settings.IPmode.value % 2;
         document.settings.enableIPv6.value = document.settings.IPmode.value / 2;
         setIPmode(document.settings.IPmode.value);
         alertIPmodeChange();
         document.settings.dhcp.disabled = (parseInt(getIPmode()) & 1)? false : true;
         document.settings.DHCPv6Opts.disabled = (parseInt(getIPmode()) & 2)? false : true;
         showDhcp();
      }

      function updateEth() {
         var eth6 = document.settings.ethIpPrefix6.value.split("/");
         document.settings.ethIp6.value = eth6[0];
         document.settings.ethPrefix6.value = (eth6.length==2 && eth6[1] > 0)? eth6[1] : 0;
         document.settings.IPv6method.value = document.settings.ethIpPrefix6.value.length? 1 : 0;
      }');
} else {
write('\
      function showDhcp() {
         if (document.settings.dhcp.checked) {
            document.settings.dhcp.value = 2;
            hide2("document", "ipSettings,netSettings");
         } else {
            document.settings.dhcp.value = 0;
            show2("document", "ipSettings,netSettings");
         }
      }');
}%>

<%if (IPv6) {
write('\
      function form_verification() {
         var form_location = document.settings;
         continue_verification = verify([
            [form_location.hostName, "Fill?,Space"]
         ]);
         if (continue_verification && !form_location.dhcp.checked && !form_location.dhcp.disabled) {
            continue_verification = verify([
               [form_location.ipAddress1, "Fill,IPv4"   ],
               [form_location.netMask1,   "Fill,NetMask"],
            ]);
         }
         if (continue_verification /*&& !form_location.DHCPv6Opts.checked && !form_location.DHCPv6Opts.disabled*/) {
            if (form_location.IPv6method.value == "1") {
               continue_verification = verify([
                  [form_location.ethIpPrefix6, "Fill,IPv6P"],
               ]);
            } else {
               form_location.ethIp6.value = "";
               form_location.ethPrefix6.value = "";
            }
         }
         if (continue_verification && form_location.dhcp.value == 0 && form_location.DHCPv6Opts.value == 0) {
            continue_verification = verify([
               [form_location.domain,   "Fill?,Space"],
               [form_location.dns1,     "Fill?,IP"   ],
            ]);
            if (continue_verification && (parseInt(getIPmode()) & 1)) {
               continue_verification = verify([
                  [form_location.gateway, "Fill?,IPv4"],
               ]);
            }
            if (continue_verification && (parseInt(getIPmode()) & 2)) {
               continue_verification = verify([
                  [form_location.gateway6, "Fill?,IPv6"],
               ]);
            }
         }
         return continue_verification;
      }');
} else {
write('\
      function form_verification() {
         var form_location = document.settings;
         if (form_location.dhcp.checked == false) {
            continue_verification = verify([
               [form_location.hostName,   "Fill?,Space" ],
               [form_location.ipAddress1, "Fill,IPv4"   ],
               [form_location.netMask1,   "Fill,NetMask"],
               [form_location.domain,     "Fill?,Space" ],
               [form_location.dns1,       "Fill?,IP"    ],
               [form_location.gateway,    "Fill?,IP"    ],
            ]);
         } else {
            continue_verification = true;
         }
         return continue_verification;
      }');}%>

      function submit() {	 
         if (form_verification()) { 
            copyData();
<%if (IPv6) {
write('\
            document.configForm.elements["system.netSettings.mode"].value = 2;');
} else {
write('\
            document.configForm.elements["system.netSettings.mode"].value = document.settings.dhcp.value==0? 2 : 0;');
}%>
            document.configForm.submit();
            parent.mainReady = null;
         }
      }

</script>
</head>

<body onload="init();" class="bodyMain">
<p>
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="450" align="center">
   <tr>
      <td>
         <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
            <tr>
               <td align="center" class="label_for_tables" >
<%if (IPv6) {
write('\
                  Set up the network parameters for IPv4 and IPv6.<br>
                  Uncheck the DHCP box to perform manual IPv4 configuration.<br>
                  Uncheck the DHCPv6 box to perform manual IPv6 configuration.<br>');
} else {
write('\
                  Set up the network parameters.<br>
                  Select the DHCP checkbox for automatic configuration.<br>
                  Uncheck the DHCP box to perform manual configuration.');
}%>
               </td>
            </tr>
         </table>
      </td>
   </tr>     
</table>

<form name="configForm" onsubmit="copyData();" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%write(SID);%>">
   <input type="hidden" name="urlOk" value="">
   <input type="hidden" name="urlError" value="/wizard/netSettings.asp">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">
<!--these hidden inputs are used to store values of form elements inside layers-->
   <input type="hidden" name="system.netSettings.hostName" value="">
   <input type="hidden" name="system.netSettings.ipAddress1" value="">
   <input type="hidden" name="system.netSettings.netMask1" value="">
   <input type="hidden" name="system.netSettings.domain" value="">
   <input type="hidden" name="system.netSettings.dns1" value="">
   <input type="hidden" name="system.netSettings.gateway" value="">
   <input type="hidden" name="system.netSettings.dhcp" value="">
   <input type="hidden" name="system.netSettings.mode" value="0">
<%if (IPv6) {
write('\
   <input type="hidden" name="system.netSettings.DHCPv6Opts" value="">
   <input type="hidden" name="system.netSettings.enableIPv4" value="">
   <input type="hidden" name="system.netSettings.enableIPv6" value="">
   <input type="hidden" name="system.netSettings.IPv6method" value="">
   <input type="hidden" name="system.netSettings.ethIp6"     value="">
   <input type="hidden" name="system.netSettings.ethPrefix6" value="">
   <input type="hidden" name="system.netSettings.gateway6"   value="">');}%>
<!--*************************************************************************-->
</form>

<form name="settings" onSubmit="return false;">
<table border="0" cellpadding="2" cellspacing="0" align="center">
<%if (IPv6) {
write('\
   <tr>
      <td align=center colspan="2">
         <font class="label">Mode</font>
         <select name="IPmode" size="1" onChange="updateIPmode();">
            <option value="1">IPv4</option>
            <option value="2">IPv6</option>
            <option value="3">Dual-Stack</option>
         </select>
         <input type="hidden" name="enableIPv4" value="' + getVal("system.netSettings.enableIPv4") + '">
         <input type="hidden" name="enableIPv6" value="' + getVal("system.netSettings.enableIPv6") + '">
      </td>
   </tr>
   <tr>
      <td align="center">
         <font class="label">DHCP</font>
         <input type="checkbox" name="dhcp" onclick="showDhcp()" value="' + getVal("system.netSettings.dhcp") + '">
      </td>
      <td align="center">
         <font class="label">DHCPv6</font>
         <input type="checkbox" name="DHCPv6Opts" onclick="showDhcp()" value="' + getVal("system.netSettings.DHCPv6Opts") + '">
      </td>
   </tr>');
} else {
write('\
   <tr>
      <td align="center" colspan="2">
         <font class="label">DHCP</font>
         <input type="checkbox" name="dhcp" onclick="showDhcp()" value="' + getVal("system.netSettings.dhcp") + '">
      </td>
   </tr>');}%>
   <tr>
      <td colspan="2" align="left">
         <font class="label"><%getLabel(self + "hostName");%></font>
      </td>
   </tr>
   <tr>
      <td colspan="2" align="left">
         <input class="formText" maxlength="30" type="text" name="hostName" size="30" value="<%get(self + "hostName");%>">
      </td>
   </tr>
   <tr>
      <td colspan="2" align="left">
         <table id="ipSettings" style="display:none;">
            <tr>
               <td width="210">
                  <font class="label"><%getLabel(self + "ipAddress");%></font>
               </td>
               <td>
                  <font class="label"><%getLabel(self + "netMask1");%></font>
               </td>
            </tr>
            <tr>
               <td>
                  <input class="formText" maxlength="15" type="text" name="ipAddress1" size="15" value="<%get(self + "ipAddress1");%>">
               </td>
               <td>
                  <input class="formText" maxlength="15" type="text" name="netMask1" size="15" value="<%get(self + "netMask1");%>">
               </td>
            </tr>
         </table>
      </td>
   </tr>
<%if (IPv6) {
write('\
   <tr>
      <td colspan="2" align="left">
         <table id="ip6Settings" style="display:none;">
            <tr>
               <td>
                  <font class="label">Static IPv6 Address</font>
               </td>
            </tr>
            <tr>
               <td>
                  <input class="formText" maxlength="50" type="text" name="ethIpPrefix6" size="39" value="" onChange="updateEth();">
                  <input type="hidden" name="ethIp6" value="' + getVal("system.netSettings.ethIp6") + '">
                  <input type="hidden" name="ethPrefix6" value="' + getVal("system.netSettings.ethPrefix6") + '">
                  <input type="hidden" name="IPv6method" value="' + getVal("system.netSettings.IPv6method") + '">
               </td>
            </tr>
         </table>
      </td>
   </tr>');}%>
   <tr>
      <td colspan="2" align="left">
         <table id="netSettings" border="0" cellpadding="2" cellspacing="0" style="display:none;">
            <tr>
               <td colspan="2">
                  <font class="label"><%getLabel(self + "domain");%></font>
               </td>
            </tr>
            <tr>
               <td colspan="2">
                  <input class="formText" maxlength="39" type="text" name="domain" size="39" value="<%get(self + "domain");%>">
               </td>
            </tr>
<%if (IPv6) {
write('\
            <tr>
               <td colspan="2">
                  <font class="label">DNS Server</font>
               </td>
            </tr>
            <tr>
               <td colspan="2">
                  <input class="formText" maxlength="50" type="text" name="dns1" size="39" value="' + getVal(self + "dns1") + '">
               </td>
            </tr>');
} else {
write('\
            <tr>
               <td>
                  <font class="label">DNS Server</font>
               </td>
               <td>
                  <font class="label">Gateway IP</font>
               </td>
            </tr>
            <tr>
               <td>
                  <input class="formText" maxlength="15" type="text" name="dns1" size="15" value="' + getVal(self + "dns1") + '">
               </td>
               <td>
                  <input class="formText" maxlength="15" type="text" name="gateway" size="15" value="' + getVal(self + "gateway") + '">
               </td>
            </tr>');}%>
         </table>
      </td>
   </tr>
<%if (IPv6) {
write('\
   <tr>
      <td colspan="2" align="left">
         <table id="gateWay" border="0" cellpadding="2" cellspacing="0" style="display:none;">
            <tr>
               <td>
                  <font class="label">IPv4 Gateway</font>
               </td>
            </tr>
            <tr>
               <td>
                  <input class="formText" maxlength="15" type="text" name="gateway" size="15" value="' + getVal(self + "gateway") + '">
               </td>
            </tr>
         </table>
      </td>
   </tr>
   <tr>
      <td colspan="2" align="left">
         <table id="gateWay6" border="0" cellpadding="2" cellspacing="0" style="display:none;">
            <tr>
               <td colspan="2">
                  <font class="label">IPv6 Gateway</font>
               </td>
            </tr>
            <tr>
               <td>
                  <input class="formText" maxlength="50" type="text" name="gateway6" size="39" value="' + getVal(self + "gateway6") + '">
               </td>
            </tr>
         </table>
      </td>
   </tr>');}%>
</table>
</form>

<script type="text/javascript">
fill();
parent.mainReady = true;
</script>
<% set("_sid_", "0"); %>
</body>
</html>
