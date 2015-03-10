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
	parent.helpPage = 'helpPages/netServicesHelp.asp';

	function init()
    {
       if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
          if (checkLeftMenu('/normal/network/networkMenu.asp') == 0) {
             parent.menuReady = 0;
             setTimeout('init()', 200);
             return;
          }
          setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
          parent.topMenu.selectItem(parent.menu.topItem);
          parent.menu.selectItem("SRV");
         }
         else
            setTimeout('init()', 200);
      }

      function submit()
      {
        if (document.netServicesForm.telnet.checked == true) {
        	document.configForm.elements[5].value = '1';
		} else {
         	document.configForm.elements[5].value = '0';
		}
        if (document.netServicesForm.ssh.checked == true) {
        	document.configForm.elements[6].value = '1';
		} else {
         	document.configForm.elements[6].value = '0';
		}
        if (document.netServicesForm.snmp.checked == true) {
        	document.configForm.elements[7].value = '1';
		} else {
         	document.configForm.elements[7].value = '0';
		}
        if (document.netServicesForm.ipsec.checked == true) {
        	document.configForm.elements[8].value = '1';
		} else {
         	document.configForm.elements[8].value = '0';
		}

         document.configForm.submit();
		 parent.mainReady = null;
      }


     </script>
</head>

<body onload="init();" class="bodyMain">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td align="center">
<form name="netServicesForm" onSubmit="return false;">
<table align="center" width="400" cellspacing="5" cellpadding="0" border="0">
<tr>
	<td width="80"><font class="label"><input type="checkbox" <%if (getVal("system.network.services.telnet") != 0) write("checked");%> name="telnet" id="telnet" value="">&nbsp;Telnet</font></td>
	<td width="80"><font class="label"><input type="checkbox" <%if (getVal("system.network.services.ssh") != 0) write("checked");%> name="ssh" id="ssh" value="">&nbsp;SSH</font></td>
	<td width="80"><font class="label"><input type="checkbox" <%if (getVal("system.network.services.snmp") != 0) write("checked");%> name="snmp" id="snmp" value="">&nbsp;SNMP</font></td>
	<td width="80"><font class="label"><input type="checkbox" <%if (getVal("system.network.services.ipsec") != 0) write("checked");%> name="ipsec" id="ipsec" value="">&nbsp;IPSec</font></td>
</tr>
</table>
	</td>
</tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/network/netServices.asp">
   <input type="hidden" name="urlError" value="/normal/network/netServices.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
   <!--***********************************************************-->
   <input type="hidden" name="system.network.services.telnet" value="">
   <input type="hidden" name="system.network.services.ssh" value="">
   <input type="hidden" name="system.network.services.snmp" value="">
   <input type="hidden" name="system.network.services.ipsec" value="">
</form>	
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
