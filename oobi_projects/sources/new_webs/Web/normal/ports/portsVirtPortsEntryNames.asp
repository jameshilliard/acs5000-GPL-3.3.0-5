<html>
<head>
	<title>New/Modify Port</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">


  <%
		var nports = getQueryVal("nports",-1);
		
		set("system.ports.virtualP.nports",nports);

		write("portName = new Array("+nports+");");

		for ( i=0; i < nports ; i++) {
			write('portName['+i+']="'+
				getVal("system.ports.virtualP.virtualPort.name",i)+'";\n');
		}
	    ReqNum = getRequestNumber();
	%>

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	var nPorts = <%write(nports);%>;

	var sid = opener.document.configForm.elements[1].value;
	var virtualList = opener.document.configForm.elements[5].value;
	var localPort = opener.document.configForm.elements[7].value;
	var firstlocalPort = opener.document.configForm.elements[7].value;
	var localIp = opener.document.configForm.elements[8].value;
	var localTcpPort = opener.document.configForm.elements[9].value;
	var firstlocalTcpPort = opener.document.configForm.elements[9].value;
	var remoteIp = opener.document.configForm.elements[10].value;
	var remoteTcpPort = opener.document.configForm.elements[11].value;
	var firstremoteTcpPort = opener.document.configForm.elements[11].value;
	var prot = opener.document.configForm.elements[12].value; 
	var firstopener = opener.opener;

	function form_verification()
	{
	continue_verification = true
	for (var i=0; i< nPorts; i++)
	{
	if (document.VirtPortsEntryNamesForm.elements[i].value != '' && continue_verification == true)
	{checkElement (document.VirtPortsEntryNamesForm.elements[i], false, true, false, false);}
	}
	} 
	
	function okNameEntry()
	{
	try {
	     if (firstopener.window.name != document.configForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
		form_verification();
		if (continue_verification == true)
		{		
		document.configForm.elements[1].value=sid;
		document.configForm.elements[5].value=virtualList;
		document.configForm.elements[7].value = firstlocalPort;
		document.configForm.elements[8].value = localIp;
		document.configForm.elements[9].value = firstlocalTcpPort;
		document.configForm.elements[10].value = remoteIp;
		document.configForm.elements[11].value = firstremoteTcpPort;
		document.configForm.elements[12].value = prot; 

		for (var i=0; i< nPorts; i++) {
		  document.configForm.elements[13+i].value = document.VirtPortsEntryNamesForm.elements[i].value;
		}
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
   <input type="hidden" name="system.ports.virtualP.virtualPort.numOfPorts" value="<%write(nports);%>">
   <input type="hidden" name="system.ports.virtualP.virtualPort.localPort" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.clusteringIp" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.localTcpPort" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.remoteIp" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.remoteTcpPort" value="">
   <input type="hidden" name="system.ports.virtualP.virtualPort.protocol" value="">
   <%
	for (i=0; i< nports; i++) {
		write('<input type="hidden" name="system.ports.virtualP.virtualPort.name_'+i+'" value="">\n');
	}
    %>
</form>
<form name="VirtPortsEntryNamesForm"  onSubmit="return false;">
  <table width="100%" height="100%" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
  <td align="center" valign="middle">
  <table>
	        <td align="center" colspan="2" height="40" valign="bottom">       
		    <!--<a href="javascript:alert(document.VirtPortsEntryNamesForm.elements[5].value);">-->
			<a href="javascript:okNameEntry();">
            <img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a>
            &nbsp;&nbsp;&nbsp;&nbsp;
		    <a href="javascript:window.close();">
            <img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
             </td>
      </tr>
   </table>
  <table width="580" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
  <td align="left" valign="top" colspan="7">&nbsp;</td>
  </tr>
    <tr align="left" valign="top"> 
    <td nowrap class="colorTHeader" width="50"><font class="label_for_tables">&nbsp;Port #</font></td>
    <td nowrap class="colorTHeader" width="100"><font class="label_for_tables">Port<br>Name</font></td>
    <td nowrap class="colorTHeader" width="110"><font class="label_for_tables">Local IP</font></td>
    <td nowrap class="colorTHeader" width="70" align="center"><font class="label_for_tables">Local<br>TCP Port</font></td>
    <td nowrap class="colorTHeader" width="110"><font class="label_for_tables">Remote IP </font></td>
    <td nowrap class="colorTHeader" width="70" align="center"><font class="label_for_tables">Remote<br>TCP Port</font></td>
    <td nowrap class="colorTHeader" width="70"><font class="label_for_tables">Protocol</font></td>
  </tr>
<script type="text/javascript">

	opener.close();
	if ( prot == "87") 
		protocol = "telnet";
	else 
		protocol = "SSH";

    for (var i=0; i<nPorts; i++)
	{	 
	if (i%2 != 0)
	{ 	
	document.write('<tr align="left" valign="top">');
	document.write('<td nowrap class="colorTRow" width="50"><font class="label">&nbsp;'+localPort);
	document.write('&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRow" width="100"><font class="formText"><input type="text" size="10" maxlength="30" name="port_name"', i, ' class="formText" value='+portName[i]+'></font></td>');
	document.write('<td nowrap class="colorTRow" width="110"><font class="label"&nbsp;&nbsp;>'+ localIp +'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRow" width="70" align="center"><font class="label">'+ localTcpPort +'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRow" width="110"><font class="label">'+remoteIp+'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRow" width="70" align="center"><font class="label">'+ remoteTcpPort +'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRow" width="70"><font class="label">'+ protocol +'&nbsp;&nbsp;</font></td>');
	document.write('</tr>');
	}
	else
	{
	document.write('<tr align="left" valign="top">');
	document.write('<td nowrap class="colorTRowAlt" width="50"><font class="label">&nbsp;'+ localPort);
	document.write('&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRowAlt" width="100"><font class="formText"><input type="text" size="10" maxlength="30" name="port_name"', i, ' class="formText" value='+portName[i]+'></font></td>');
	document.write('<td nowrap class="colorTRowAlt" width="110"><font class="label"&nbsp;&nbsp;>'+ localIp +'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRowAlt" width="70" align="center"><font class="label">'+ localTcpPort +'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRowAlt" width="110"><font class="label">'+remoteIp +'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRowAlt" width="70" align="center"><font class="label">'+ remoteTcpPort +'&nbsp;&nbsp;</font></td>');
	document.write('<td nowrap class="colorTRowAlt" width="70"><font class="label">'+ protocol +'&nbsp;&nbsp;</font></td>');
	document.write('</tr>');	
	}
	localPort++;
	localTcpPort++;
	remoteTcpPort++;
	
	}
</script>   
</table>
</td>
  </tr>
</table> 
</form>
</body>
</html>
