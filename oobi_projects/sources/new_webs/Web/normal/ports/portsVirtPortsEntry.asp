<html>
<head>
	<title>New/Modify Port</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

  <%
	var remoteIp = getQueryVal("virtPort",-1);

	var family_model = getVal('_familymodel_');
	if (family_model == "ONS")
	var nphysports = getVal("system.device.number2");
	else
	var nphysports = getVal("system.device.number");

	if (remoteIp == "-1") {
		set("system.ports.virtualP.action",2);
	} else {
		set("system.ports.virtualP.action",0);
		set("system.ports.virtualP.remoteIp",remoteIp);
		name = getVal("system.ports.virtualP.virtualPort.name",0);
	}
	ReqNum = getRequestNumber();
  %>
	
    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	var remIp = "<%write(remoteIp);%>";
    	
	function copyData()
	{   
	    document.configForm.elements[1].value=opener.document.configForm.elements[1].value;
		document.configForm.elements[5].value=opener.document.configForm.elements[5].value;

		for (var i=0; i < 7 ; i++) {
			document.configForm.elements[6+i].value = document.VirtPortsEntryForm.elements[i].value;
		}

                //[LMT] Cybts BUG 5005 - Edit mode 
                if(remIp != -1)
                {
                        var str = document.configForm.elements[5].value;
                        str = str.replace(remIp,document.configForm.elements[10].value);
                        document.configForm.elements[5].value=str;
                }
	}

    function form_verification()
     {
     var form_location = document.VirtPortsEntryForm;
     checkElement (form_location.elements[1], true, false, false, true, <%write(nphysports+1);%>, 1023);
     if (continue_verification == true)
	 {checkElement (form_location.elements[2], true, false, true, false);}
     if (continue_verification == true)
	 {checkElement (form_location.elements[3], true, false, false, true);}
     if (continue_verification == true)
	 {checkElement (form_location.elements[4], true, false, true, false);}
     if (continue_verification == true)
	 {checkElement (form_location.elements[5], true, false, false, true);}  
     }
	
	function winNameEntry()
	{
	try {
	     if (opener.window.name != document.configForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
    form_verification();
    if (continue_verification == true)
	{			
		var newRef;
		copyData();
		newRef='portsVirtPortsEntryNames.asp' + "?nports=" + document.VirtPortsEntryForm.ports_number.value + "&target=" + document.configForm.target;
		newWindow(newRef,"ports",620,590,"yes");
	}
	} catch (error) {
	  self.close();
	}
	}

	function okVirtualPortEntry()
	{ 
	try {
	     if (opener.window.name != document.configForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
    form_verification();
    if (continue_verification == true)
	{		
		copyData();
        setQuerySSID(document.configForm);
		document.configForm.submit();
		self.close();
	}
	} catch (error) {
	  self.close();
	}
	}

    </script>
</head>
<body class="bodyForLayers">
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/ports/virtPorts.asp">
   <input type="hidden" name="urlError" value="/normal/ports/virtPorts.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
   <input type="hidden" name="system.ports.virtualP.virtualInfoSet" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.numOfPorts" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.localPort" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.clusteringIp" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.localTcpPort" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.remoteIp" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.remoteTcpPort" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.protocol" value="">
</form>
<form name="VirtPortsEntryForm" method="POST" action=" " onSubmit="return false;">
<table width="350" border="0" align="center" cellpadding="0" cellspacing="10">
   <tr>
      <td height="40" valign="middle" align="center">
    	 <a href="javascript:okVirtualPortEntry();">
         <img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a>
         &nbsp;&nbsp;&nbsp;&nbsp;
		 <a href="javascript:winNameEntry();">
         <img src="/Images/portNames.gif" width="108" height="21" border="0" alt=""></a> 
		 &nbsp;&nbsp;&nbsp;&nbsp;
		 <a href="javascript:window.close();">
         <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
      </td>
   </tr>
</table>
<table width="350" border="0" align="center" cellpadding="10" cellspacing="0">
  <tr align="left" valign="middle"> 
    <td width="50%"><font class="label">Number of ports</font></td>
    <td width="50%"><font class="formText">
	<select name="ports_number" size="1" class="formText">
	<% var selected = getVal("system.ports.virtualP.virtualPort.numOfPorts");%>
         		 <option value="1" <%if (1==selected) write("selected");%>> 1 </option>
         		 <option value="4" <%if (4==selected) write("selected");%>> 4 </option>
         		 <option value="8" <%if (8==selected) write("selected");%>> 8 </option>
         		 <option value="16" <%if (16==selected) write("selected");%>> 16 </option>
         		 <%if (family_model != "AVCS") { write("<option value=\"32\" "); if (32==selected) write("selected"); write("> 32 </option>");}%>
         		 <%if (family_model != "AVCS") { write("<option value=\"48\" "); if (48==selected) write("selected"); write("> 48 </option>");}%>
	 </select> 
	 </font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="50%"><font class="label">First Local Port Number</font></td>
    <td width="50%"><font class="formText"><input type="text" size="10" name="localPort" class="formText" value="<%get("system.ports.virtualP.virtualPort.localPort");%>"></font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="50%"><font class="label">Local IP</font></td>
    <td width="50%"><font class="formText"><input type="text" maxlength="15" size="15" name="localIp" class="formText" value="<%get("system.ports.virtualP.virtualPort.clusteringIp");%>"></font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="50%"><font class="label">First Local TCP Port Number</font></td>
    <td width="50%"><font class="formText"><input type="text" size="10" name="localTcpPort" class="formText" value="<%get("system.ports.virtualP.virtualPort.localTcpPort");%>"></font></td>
  </tr>	
  <tr align="left" valign="middle"> 
    <td width="50%"><font class="label">Remote IP</font></td>
    <td width="50%"><font class="formText"><input type="text" size="15" maxlength="15" name="remoteIp" class="formText"value="<%get("system.ports.virtualP.virtualPort.remoteIp");%>"></font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="50%"><font class="label">First Remote TCP Port Number</font></td>
    <td width="50%"><font class="formText"><input type="text" size="15" name="remoteTcpPort" class="formText" value="<%get("system.ports.virtualP.virtualPort.remoteTcpPort");%>"></font></td>
  </tr>
  <tr align="left" valign="middle"> 
    <td width="50%"><font class="label">Protocol</font></td>
    <td width="50%"><font class="formText">
	<select name="protocol" size="1" class="formText">
	<% var selected = getVal("system.ports.virtualP.virtualPort.protocol");%>
         		 <option value="87" <%if (selected==87) write("selected");%>>Telnet&nbsp;&nbsp;</option>
		 	 <option value="89" <%if (selected==89) write("selected");%>> SSH&nbsp;&nbsp; </option>
	 </select>
	</font></td>
  </tr>
</table>
</form>
</body>
</html>
