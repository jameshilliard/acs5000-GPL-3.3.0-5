
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
   <%
      // Set page-wide variables:
      var self = "system.casProSettings.";
      var device = "system.device.";
      var model = getVal(device + "model");

      var ReqNum = getRequestNumber();
      var SID = getVal("_sid_");
      trial = getVal(self + "protocol"); // representative of the rest (could test all if anal)
   %>

      var page = 0;
	  parent.helpPage = 'helpPages/CASProSettingsHelp.asp';     
    window.name = '<%get("system.device.ip");%>_<%write(SID);%>_<%write(ReqNum);%>';
    parent.currentPage = self;

	function copyData()
	{
		for (var i=1; i < 7 ; i++) {
			 if (type == 'NN') {
				document.configForm.elements[i+4].value = document.casProSettings.document.settings.elements[i].value;
			} 
			if (type == 'IE' || type == 'MO') {
				document.configForm.elements[i+4].value = document.settings.elements[i].value;
			} else {
				document.configForm.elements[i+4].value = '';
			}
		}
		if (document.settings.elements[7].checked == true) {
			document.configForm.elements[11].value = "128";
		} else {
			document.configForm.elements[11].value = "0";
		}
		if (document.settings.elements[0].checked == true) {
			document.configForm.elements[12].value = "77";
		}
	}

      function init()
      {
         if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
         {
		    page = parent.menu.getPageIndex('PORT', parent.menu.menuPages);
            parent.menu.selectItem('PORT');
            setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
         }
         else
            setTimeout('init()', 200);
      }

      function submit()
      {
	     copyData();
         document.configForm.submit();
		 parent.mainReady = null;
      }
	  
</script>
</head>

<body onload="init();" class="bodyMain">

<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="400" align="center">
    <tr>
        <td>
            <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
                <tr>
                    <td align="center" class="label_for_tables" >
		   Set up the CAS (Console Access Server) profile, for the<br>
		   serial ports. Specify the serial parameters for all ports.
		   <font color="red" weigth="bold">
                   The previous port-specific parameters will be discarded.<br>
		   </font>
                   </td>
                </tr>
            </table>
        </td>
    </tr>
</table>
        
<form name="configForm" onsubmit="copyData();" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="0">
        <input type="hidden" name="system.req.sid" value="<%write(SID);%>">
        <input type="hidden" name="urlOk" value="">
        <input type="hidden" name="urlError" value="/wizard/CASProSettings.asp">
        <input type="hidden" name="request" value="<%write(ReqNum);%>">
   
        <!--these hidden inputs are used to store values of form elements inside layers-->
        <input type="hidden" name="<%write(self);%>protocol" value="">
        <input type="hidden" name="<%write(self);%>baudRate" value="">
        <input type="hidden" name="<%write(self);%>flowControl" value="">
        <input type="hidden" name="<%write(self);%>dataSize" value="">
        <input type="hidden" name="<%write(self);%>parity" value="">
        <input type="hidden" name="<%write(self);%>stopBits" value="">
        <input type="hidden" name="<%write(self);%>authReq" value="">
        <input type="hidden" name="<%write(self);%>enableAll" value="0">
        <!--*************************************************************************-->
</form>

<form name="settings" onSubmit="return false;">
<table border="0" cellpadding="2" cellspacing="0" width="460" align="center">
        <tr align="left" valign="top"> 
           <td height="45" align="center" colspan="4">
		<font class="label">Enable all ports </font>
            <input name="<%write(self);%>enableAll" type="checkbox" checked>
	   </td>
        </tr>
  <tr align="left" valign="top"> 
	<td height="45" align="left" width="80">
        <font class="label"><%getLabel(self + "protocol");%></font></td>
          <td height="45" width="180">  
            <select name="<%write(self);%>protocol" class="formText">
	    <% var selected = getVal(self + "protocol"); %>
		<option value="87" <% if (87 == selected) write("selected"); %>>Console (Telnet)</option>
		<option value="89" <% if (89 == selected) write("selected"); %>>Console (SSH)</option>
                <option value="81" <% if (81 == selected) write("selected"); %>>Console (TelnetSSH)</option>
		<option value="86" <% if (86 == selected) write("selected"); %>>Console (Raw)</option>
            </select>
            </td>
          <td height="45" width="80"><font class="label"><%getLabel(self + "baudRate");%></font></td>
          <td height="45" width="120"> 
            <select name="<%write(self);%>baudRate" class="formText">
	    <% var selected = getVal(self + "baudRate"); %>
              <option value="2400" <%if (2400 == selected) write("selected");%>> 2400 </option>
              <option value="4800" <%if (4800 == selected) write("selected");%>> 4800 </option>
              <option value="9600" <%if (9600 == selected) write("selected");%>> 9600 </option>
              <option value="14400" <%if (14400 == selected) write("selected");%>> 14400 </option>
              <option value="19200" <%if (19200 == selected) write("selected");%>> 19200 </option>
              <option value="28800" <%if (28800 == selected) write("selected");%>> 28800 </option>
              <option value="38400" <%if (38400 == selected) write("selected");%>> 38400 </option>
              <option value="57600" <%if (57600 == selected) write("selected");%>> 57600 </option>
              <option value="76800" <%if (76800 == selected) write("selected");%>> 76800 </option>
              <option value="115200" <%if (115200 == selected) write("selected");%>> 115200 </option>
              <option value="230400" <%if (230400 == selected) write("selected");%>> 230400 </option>
            </select> </td>
        </tr>
        <tr align="left" valign="top"> 
          <td height="45" align="left" width="80">
		<font class="label"><%getLabel(self + "flowControl");%></font></td>
          <td height="45"> 
            <select name="<%write(self);%>flowControl" class="formText">
	    <% var selected = getVal(self + "flowControl"); %>
              <option value="0" <%if (0 == selected) write("selected");%>>None</option>
              <option value="1" <%if (1 == selected) write("selected");%>>Hardware</option>
              <option value="2" <%if (2 == selected) write("selected");%>>Software</option>
            </select></td>
          <td height="45" align="left">
		<font class="label"><%getLabel(self + "dataSize");%></font></td>
          <td height="45">
            <select name="<%write(self);%>dataSize" class="formText">
	    <% var selected = getVal(self + "dataSize"); %>
              <option value="5" <%if (5 == selected) write("selected");%>> 5 </option>
              <option value="6" <%if (6 == selected) write("selected");%>> 6 </option>
              <option value="7" <%if (7 == selected) write("selected");%>> 7 </option>
              <option value="8" <%if (8 == selected) write("selected");%>> 8 </option>
            </select></td>
        </tr>
        <tr align="left" valign="top"> 
          <td height="45" align="left" width="80">
		<font class="label"><%getLabel(self + "parity");%></font></td>
          <td height="45"> 
            <select name="<%write(self);%>parity" class="formText">
	    <% var selected = getVal(self + "parity");%>
              <option value="1" <%if (1 == selected) write("selected");%>>None </option>
              <option value="2" <%if (2 == selected) write("selected");%>>Odd </option>
              <option value="3" <%if (3 == selected) write("selected");%>>Even </option>
            </select></td>
          <td height="45">
		<font class="label"><%getLabel(self + "stopBits");%></font></td>
          <td height="45">
            <select name="<%write(self);%>stopBits" class="formText">
	    <% var selected = getVal(self + "stopBits");%>
              <option value="1" <%if (1 == selected) write("selected");%>>1</option>
              <option value="2" <%if (2 == selected) write("selected");%>>2</option>
            </select></td>
        </tr>
        <tr align="left" valign="top"> 
           <td height="45" align="center" colspan="4">
		<font class="label"><%getLabel(self + "authReq");%></font>
            <% var checked = getVal(self + "authReq");%>
            <input name="<%write(self);%>authReq" type="checkbox" <%if (0 != checked) write("checked");%>>
	   </td>
        </tr>
  </tr>
</table>
</form>
<script type="text/javascript">
parent.mainReady = true;
parent.controls.document.getElementById('back').style.visibility = "visible";
parent.controls.document.getElementById('next').style.visibility = "visible";

</script>
<%set("_sid_", "0");%>
</body>
</html>
