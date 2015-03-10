
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
  
  <%
    var self = "system.administration.bootInfo.";
	ReqNum = getRequestNumber();
  %>  

  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = 'helpPages/adminBootConfigHelp.asp';

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
      parent.menu.selectItem("BOOT");
    }
    else
      setTimeout('init()', 200);
  }

  function copyData()
  {
    for (var i=0; i < 8; i++) 
    {
      var temp = document.bootconfigForm.elements[i].value;
      document.configForm.elements[i+5].value = document.bootconfigForm.elements[i].value;
    }
  }
 
  function form_verification_ons()
  {
  var form_location = document.bootconfigForm;
  checkElement (form_location.elements[0], true, false, true, false);
  if (continue_verification == true)    
  {checkElement (form_location.elements[7], false, false, false, true);} 
  	if (form_location.elements[2].value == 1)
	  {	
	    if (continue_verification == true)	 
		{checkElement (form_location.elements[3], true, true, false, false);}
		if (continue_verification == true)    
		{checkElement (form_location.elements[4], true, false, true, false);}
	  }  
  }
      
  function submit()
  {
	form_verification_ons();
    if (continue_verification == true)
    {    
        copyData();
        document.configForm.submit();
        parent.mainReady = null;
    } 
 }
      
</script>
</head>

<body onload="init();" class="bodyMain">
   <form name="bootconfigForm" onSubmit="return false;">
      <table border="0" cellpadding="2" cellspacing="0" width="560" align="center">
         <tr>
            <td colspan="3"><font class="label">IP Address assigned to Ethernet</font></td>
         </tr>
         <tr valign="top">
            <td colspan="3" height="40"><input class="formText" type="text" maxlength="15" name="ipAddr" size="15" value="<%get(self + "ipAddr");%>"></td>
         </tr>
         <tr>
            <td><font class="label">Watchdog Timer</font></td>
			<td>&nbsp</td>
            <td><font class="label">Unit boot from</font></td>
         </tr>
         <tr valign="top">
            <td height="40">
               <font class="formText">
               <select class="formText" name="wdTimer">
               <% selected = getVal(self + "wdTimer"); %>
               <option value="0" <% if (0 == selected) write("SELECTED"); %>> InActive </option>
               <option value="1" <% if (1 == selected) write("SELECTED"); %>> Active </option>
               </select>
               </font>
            </td>         
			<td>&nbsp</td>
            <td>
               <font class="formText">
               <select class="formText" name="uBoot">
                 <%get(self + "uBootHtml");%>
               </select>
               </font>
            </td>
         </tr>
         <tr>
            <td colspan="2"><font class="label">Boot File Name</font></td>
            <td><font class="label">Server's IP Address</font></td>
         </tr>
         <tr valign="top">
            <td colspan="2" height="40"><input type="text" name="bootFilename" maxlength="60" size="19" value="<%get(self + "bootFilename");%>" class="formText"></td>
            <td><input class="formText" type="text" name="serverIpAddr" maxlength="15" size="15" value="<%get(self + "serverIpAddr");%>"></td>
         </tr>
         <tr>
            <td><font class="label">Console Speed</font></td>
         </tr>
         <tr valign="top">
            <td height="40">
               <font class="formText">
               <% selected = getVal(self + "speed"); %>
               <select class="formText" name="speed">
               <option value="0" <% if (0 == selected) write("SELECTED"); %>> 4800 </option>
               <option value="1" <% if (1 == selected) write("SELECTED"); %>> 9600 </option>
               <option value="2" <% if (2 == selected) write("SELECTED"); %>> 19200 </option>
               <option value="3" <% if (3 == selected) write("SELECTED"); %>> 38400 </option>
               <option value="4" <% if (4 == selected) write("SELECTED"); %>> 57600 </option>
               <option value="5" <% if (5 == selected) write("SELECTED"); %>> 115200 </option>
               </select>
               </font>
            </td>         
         </tr>
         <tr>
            <td colspan="2"><font class="label">Fast Ethernet</font></td>
            <td><font class="label">Fast Ethernet Max <br>Interrupt Events</font></td>
         </tr>
         <tr valign="top">
            <td colspan="2" height="40">
               <font class="formText">
               <% selected = getVal(self + "fastEthernet"); %>
               <select class="formText" name="fastEthernet">
               <option value="0" <% if (0 == selected) write("SELECTED"); %>> Auto Negotiation </option>
               <option value="1" <% if (1 == selected) write("SELECTED"); %>> 100BaseT Half-Duplex </option>
               <option value="3" <% if (3 == selected) write("SELECTED"); %>> 100BaseT Full-Duplex </option>
               <option value="4" <% if (4 == selected) write("SELECTED"); %>> 10BaseT Half-Duplex </option>
               <option value="2" <% if (2 == selected) write("SELECTED"); %>> 10BaseT Full-Duplex </option>
               </select>
               </font>
            </td>
            <td><input class="formText" type="text" name="maxEvent" size="15" value="<%get(self + "maxEvent");%>"></td>
         </tr>
      </table>
   </form>

<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/administration/adminUBootConfig.asp">
   <input type="hidden" name="urlError" value="/normal/administration/adminUBootConfig.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
 <!--*************************************************************************-->
  <!--these hidden inputs are used to store values of form elements inside layers-->
   <input type="hidden" name="<%write(self);%>ipAddr" value="">
   <input type="hidden" name="<%write(self);%>wdTimer" value="">
   <input type="hidden" name="<%write(self);%>uBoot" value="">
   <input type="hidden" name="<%write(self);%>bootFilename" value="">
   <input type="hidden" name="<%write(self);%>serverIpAddr" value="">
   <input type="hidden" name="<%write(self);%>speed" value="">
   <input type="hidden" name="<%write(self);%>fastEthernet" value="">
   <input type="hidden" name="<%write(self);%>maxEvent" value="">
</form>

<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
