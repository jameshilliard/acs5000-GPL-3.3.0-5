<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   	<script type="text/javascript">
	
<% ReqNum = getRequestNumber();
var family_model = getVal('_familymodel_');
var BONDING_ENABLED = "no";
if (family_model == "ACS" || family_model == "ONS")
  BONDING_ENABLED = "yes"; %>
    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
	parent.helpPage = 'helpPages/netHostSettingsHelp.asp';

    function init()
    {
       if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
       {
          if (checkLeftMenu('/normal/network/networkMenu.asp') == 0) {
             parent.menuReady = 0;
             setTimeout('init()', 200);
             return;
          }
          setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
          parent.topMenu.selectItem(parent.menu.topItem);
          parent.menu.selectItem("ETH");
       }
       else
          setTimeout('init()', 200);
    }

    function showDhcpOrNot()
    {
      if (document.mainForm.DHCPcheckbox.checked == true) {
        hide('document', 'Layer1');
<%if (BONDING_ENABLED == "yes")
write("\
        hide('document', 'Layer2');");
%>
      } else {
        show('document', 'Layer1');
<%if (BONDING_ENABLED == "yes")
write("\
        if (document.mainForm.bondingEnabled.checked == true)
          show('document', 'Layer2');
        else
          hide('document', 'Layer2');");
%>
      }
    }   

<%if (BONDING_ENABLED == "yes")
write("\
    function showBondingOrNot()
    {
      if (document.mainForm.bondingEnabled.checked == true)
        show('document', 'Layer2');
      else
        hide('document', 'Layer2');
    }");
%>

    function form_verification()
    {
		form_location = document.mainForm;
	    continue_verification = true;
	    if (form_location.hostName.value != "")	  
		   	{ checkElement (form_location.hostName, false, true, false, false); }
        if (document.mainForm.DHCPcheckbox.checked != true)
	  	{   
		    // function checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)
			if (continue_verification == true)	  
	    		{ checkElement (form_location.PrimIP, true, false, true, false); }
			if (continue_verification == true)	  
	    		{ checkElement (form_location.netMask, true, false, false, false); }				
			if (continue_verification == true)	  
	    		{ checkNetmask (form_location.netMask); }					
			if (continue_verification == true && form_location.secIP.value != "")	  
	    		{ checkElement (form_location.secIP, false, false, true, false); }			
			if (continue_verification == true && form_location.secNetMask.value != "")
			    { checkNetmask (form_location.secNetMask); } 				
			if (continue_verification == true && form_location.mtu.value != "")	  
	    		{ checkElement (form_location.mtu, false, false, false, true, 1, 1500); }
			if (continue_verification == true && form_location.DNSServer.value != "")	  
	    		{ checkElement (form_location.DNSServer, false, false, true, false); }
			if (continue_verification == true && form_location.secDNSServer.value != "")	  
	    		{ checkElement (form_location.secDNSServer, false, false, true, false); }
			if (continue_verification == true && form_location.domain_name.value != "")	  
	    		{ checkElement (form_location.domain_name, false, true, false, false); }
			if (continue_verification == true && form_location.gatewayIP.value != "")	  
	    		{ checkElement (form_location.gatewayIP, false, false, true, false); }		
		}
	}
	  
    function submit()
    { 
		form_verification();
	    if (continue_verification == true)
		{
	        if (document.mainForm.DHCPcheckbox.checked == true) {
	        	document.configForm.elements[6].value = '2';
			} else {
   		      	document.configForm.elements[6].value = '0';
				document.configForm.elements[8].value = document.mainForm.PrimIP.value;
				document.configForm.elements[9].value = document.mainForm.netMask.value;
				document.configForm.elements[10].value = document.mainForm.DNSServer.value;
				document.configForm.elements[11].value = document.mainForm.domain_name.value;
				document.configForm.elements[12].value = document.mainForm.gatewayIP.value;
				document.configForm.elements[14].value = document.mainForm.secIP.value;
				document.configForm.elements[15].value = document.mainForm.secNetMask.value;
				document.configForm.elements[16].value = document.mainForm.mtu.value;
				document.configForm.elements[17].value = document.mainForm.secDNSServer.value;
<%if (BONDING_ENABLED == "yes")
write("\
				if (document.mainForm.bondingEnabled.checked == false) {
				document.configForm.elements[18].value = '0';
				}
				else{
				document.configForm.elements[18].value = '1';
				}
				document.configForm.elements[19].value = document.mainForm.miimonValue.value;
				document.configForm.elements[20].value = document.mainForm.updelayValue.value;");
%>
			}

			document.configForm.elements[07].value = document.mainForm.hostName.value;
			document.configForm.elements[13].value = document.mainForm.consoleBanner.value;

    	    document.configForm.submit();
			parent.mainReady = null;
		}
    }
      
    </script>
</head>
<body onload="init(); showDhcpOrNot();" class="bodyForLayers">
<form name="mainForm" action="" onSubmit="return false;">
<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%">
   <tr valign="top">
      <td>
         <table border="0" cellpadding="0" cellspacing="10" align="center"  width="400">
                     <tr valign="middle" align="center">
                        <td>
						<input type="checkbox" <%if (getVal("system.netSettings.dhcp") != 0) write("checked");%> name="DHCPcheckbox" value="1" onclick="showDhcpOrNot()">
                        <font class="label">DHCP</font>&nbsp;&nbsp;&nbsp;&nbsp;						
                     </tr>            
         </table>
         <table border="0" cellpadding="0" cellspacing="5" align="center"  width="400">
         	<tr>
         		<td valign="bottom" width="200"><font class="label">Host Name</font></td>
         		<td valign="bottom" width="200"><font class="label">Console Banner</font></td>
			</tr>
			<tr>
         		<td valign="top" width="200"><input class="formText" type="text" maxlength="30" name="hostName" size="15" value="<%get("system.netSettings.hostName");%>"></td>

         		<td valign="top" width="200"><input class="formText" type="text" maxlength="49" name="consoleBanner" size="15" value="<%get("system.netSettings.consBanner");%>"></td>

         	</tr>
         </table>
      </td>
   </tr>
</table> 
<div id="Layer1" name="noDHCP" style="position:absolute; top:110px; z-index:1; visibility: hidden;" align="center">  	  		 
<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%">
   <tr valign="middle">
      <td>		 
         <table border="0" cellpadding="0" cellspacing="5" align="center" width="400">
            <tr>
         	   <td valign="middle" colspan="2" height="30">
               <font class="tabsTextBig">Ethernet Port</font>
               </td>		
         	</tr>
            <tr>		
         		<td valign="bottom" width="200"><font class="label">Primary IP</font></td>
         		<td valign="bottom" width="200"><font class="label"><%getLabel("system.netSettings.netMask1");%></font></td>
            </tr>
            <tr>
               <td width="200"><input class="formText" type="text" maxlength="15" name="PrimIP" size="15" value="<%get("system.netSettings.ipAddress1");%>"></td>
         		<td width="200">
               <input class="formText" type="text" maxlength="15" name="netMask" size="15" value="<%get("system.netSettings.netMask1");%>">&nbsp;&nbsp;&nbsp;
               </td>
         	</tr>
            <tr>		
         		<td width="200"><font class="label">Secondary IP</font></td>
         		<td width="200"><font class="label"><%getLabel("system.netSettings.netMask2");%></font></td>
            </tr>
            <tr>
               <td width="200"><input class="formText" type="text" maxlength="15" name="secIP" size="15" value="<%get("system.netSettings.ipAddress2");%>"></td>
         		<td width="200">
               <input class="formText" type="text" maxlength="15" name="secNetMask" size="15" value="<%get("system.netSettings.netMask2");%>">&nbsp;&nbsp;&nbsp;
               </td>
         	</tr>
            <tr>		
         		<td  width="200"><font class="label">MTU</font></td>
         		<td width="200">&nbsp;</td>
            </tr>
            <tr>
		<%var mtu=getVal("system.netSettings.mtu"); %>
               <td width="200"><input class="formText" type="text" name="mtu" size="15" value="<%if (mtu!=0) write(mtu);%>"></td>
         		<td width="200">&nbsp;</td>
         	</tr>            
         </table>         
         <table border="0" cellpadding="0" cellspacing="5" align="center" width="400">
            <tr>
         		<td height="30" valign="middle" colspan="2">
               <font class="tabsTextBig">DNS Service</font>
               </td>		
         	</tr>
            <tr>		
         		<td valign="bottom"><font class="label">Primary DNS Server</font></td>
         		<td  valign="bottom"><font class="label">Secondary DNS Server</font></td>
            </tr>
            <tr>
               <td><input class="formText" type="text" maxlength="15" name="DNSServer" size="15" value="<%get("system.netSettings.dns1");%>"></td>
         		<td>
               <input class="formText" type="text" maxlength="15" name="secDNSServer" size="15" value="<%get("system.netSettings.dns2");%>">&nbsp;&nbsp;&nbsp;
               </td>
         	</tr>
            <tr>		
         		<td colspan="2"><font class="label">Domain Name</font></td>
            </tr>
            <tr>
               <td colspan="2"><input  class="formText" type="text" maxlength="39" name="domain_name" size="15" value="<%get("system.netSettings.domain");%>"></td>
         	</tr>
            <tr>		
         		<td colspan="2" valign="bottom" align="left"  height="30"><font class="label"><%getLabel("system.netSettings.gateway");%></font></td>
         	</tr>
			<tr>		
         		<td colspan="2" valign="middle" align="left"><input class="formText" type="text" maxlength="15" name="gatewayIP" size="15" value="<%get("system.netSettings.gateway");%>"></td>
         	</tr>
         </table>
      </td>
   </tr>
</table>

<%if (BONDING_ENABLED == "yes") {
write("\
<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" align=\"center\" width=\"100%\">
  <tr valign=\"top\">
      <td>
         <table border=\"0\" cellpadding=\"0\" cellspacing=\"5\" align=\"center\"  width=\"400\">
               <tr>
                       <td valign=\"bottom\" width=\"200\"><font class=\"tabsTextBig\">Bonding</font></td>
                       <td valign=\"bottom\" width=\"200\"></td>
                       </tr>
         </table>
         <table border=\"0\" cellpadding=\"0\" cellspacing=\"10\" align=\"center\"  width=\"400\">
                     <tr valign=\"middle\" align=\"center\">
                        <td><input type=\"checkbox\"");
if (getVal("system.netSettings.bondenabled") != 0) write("checked");
                        write(" name=\"bondingEnabled\" value=\"1\" onclick=\"showBondingOrNot()\">
                        <font class=\"label\">Enabled</font>&nbsp;&nbsp;&nbsp;&nbsp;</td>
                     </tr>
         </table>
         <table id=\"Layer2\" style=\"visibility: hidden\" border=\"0\" cellpadding=\"0\" cellspacing=\"5\" align=\"center\"  width=\"400\">
               <tr>
                       <td valign=\"bottom\" width=\"200\"><font class=\"label\">Miimon</font></td>
                       <td valign=\"bottom\" width=\"200\"><font class=\"label\">Updelay</font></td>
                       </tr>
                       <tr>
                       <td valign=\"top\" width=\"200\"><input class=\"formText\" type=\"text\" maxlength=\"30\" name=\"miimonValue\" size=\"15\" value=\"");get("system.netSettings.bondmiimon");write("\"></td>
                       <td valign=\"top\" width=\"200\"><input class=\"formText\" type=\"text\" maxlength=\"49\" name=\"updelayValue\" size=\"15\" value=\"");get("system.netSettings.bondupdelay");write("\"></td>
               </tr>
         </table>
      </td>
   </tr>
</table>");
} %>
</div>

</form>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/network/netHostSettings.asp">
   <input type="hidden" name="urlError" value="/normal/network/netHostSettings.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
   <!--***********************************************************-->
   <input type="hidden" name="system.netSettings.mode" value="1">
   <!--***********************************************************-->
   <input type="hidden" name="system.netSettings.dhcp" value="">
   <input type="hidden" name="system.netSettings.hostName" value="">
   <input type="hidden" name="system.netSettings.ipAddress1" value="">
   <input type="hidden" name="system.netSettings.netMask1" value="">
   <input type="hidden" name="system.netSettings.dns1" value="">
   <input type="hidden" name="system.netSettings.domain" value="">
   <input type="hidden" name="system.netSettings.gateway" value="">
   <input type="hidden" name="system.netSettings.consBanner" value="">
   <input type="hidden" name="system.netSettings.ipAddress2" value="">
   <input type="hidden" name="system.netSettings.netMask2" value="">
   <input type="hidden" name="system.netSettings.mtu" value="">
   <input type="hidden" name="system.netSettings.dns2" value="">
<%if (BONDING_ENABLED == "yes")
write("\
   <input type=\"hidden\" name=\"system.netSettings.bondenabled\" value=\"\">
   <input type=\"hidden\" name=\"system.netSettings.bondmiimon\" value=\"\">
   <input type=\"hidden\" name=\"system.netSettings.bondupdelay\" value=\"\">");
%>
</form>
<%set("_sid_","0");%>
<script type="text/javascript">
parent.mainReady = true;
</script>
</body>
</html>
