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
	parent.helpPage = 'helpPages/physPortsHelp.asp';

      function init()
      {
         if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
         {
			if (checkLeftMenu('/normal/ports/portsMenu.asp') == 0) {
				parent.menuReady = 0;
   				setTimeout('init()', 200);
				return;
   			}
            setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
            parent.menu.selectItem('PHYS');
            parent.topMenu.selectItem(parent.menu.topItem);
         }
         else
            setTimeout('init()', 200);
      }

	function copySelectedPorts()
	{
		var sel=0;
		var table = document.physPortsForm.physTable;

		document.configForm.elements[7].value = "";

		for (var i=0; i < table.options.length; i++)
		{
			if (table.options[i].selected) {
				sel += 1;
				document.configForm.elements[7].value += table.options[i].value + ",";
				if (sel == 1) {
					parent.selectedPorts = table.options[i].value;
				} else {
					parent.selectedPorts += "," + table.options[i].value;
				}
			}	
		}
		parent.numPorts = sel;
		return sel;
	}

	function modSelPorts()
	{
		if (copySelectedPorts() == "0") {
			alert('Please select a port to modify');
			document.configForm.elements[7].value = "";
			return;
		}
		document.configForm.elements[6].value = "3";
        document.configForm.urlOk.value = "/normal/ports/portsPhysPortSett.asp";
		setQuerySSID(document.configForm);
        document.configForm.submit();
	}

	function modAllPorts()
	{
		parent.numPorts = '1';
		parent.selectedPorts = 'all';
		document.configForm.elements[6].value = "3";
		document.configForm.elements[7].value = ",0,";
        document.configForm.urlOk.value = "/normal/ports/portsPhysPortSett.asp";
		setQuerySSID(document.configForm);
        document.configForm.submit();
	}

	function disableSelPorts()
	{
		if (copySelectedPorts() == "0") {
			alert('Please select a port to disable');
			document.configForm.elements[7].value = "";
			return;
		}
		document.configForm.elements[6].value = "1";
        document.configForm.urlOk.value = "/normal/ports/physPorts.asp";
		setQuerySSID(document.configForm);
        document.configForm.submit();
	}

	function enableSelPorts()
	{
		if (copySelectedPorts() == "0") {
			alert('Please select a port to enable');
			document.configForm.elements[7].value = "";
			return;
		}
		document.configForm.elements[6].value = "2";
        document.configForm.urlOk.value = "/normal/ports/physPorts.asp";
		setQuerySSID(document.configForm);
        document.configForm.submit();
	}

      function submit()
      {
		document.configForm.elements[6].value = "0";
		document.configForm.elements[7].value = "0";
        document.configForm.submit();
		parent.mainReady = null;
      }

     </script>
</head>
<body onload="init();" class="bodyMain" marginheight="0" marginwidth="0">
<form name="physPortsForm" method="POST" action=" " onSubmit="return false;">
<table border="0" cellpadding="0" cellspacing="5" align="center">
     <tr>
		<td valign="middle" align="right">
		<a href="javascript:modSelPorts();"><img src="/Images/modSelPortsButton.gif" width="140" height="21" border="0" alt=""></a></td>
        <td valign="middle" align="left">
		<a href="javascript:modAllPorts();"><img src="/Images/modAllPortsButton.gif" width="140" height="21" border="0" alt=""></a></td>
		</tr>
		<tr>
	    <td valign="middle" align="right">
		<a href="javascript:enableSelPorts();"><img src="/Images/enable_selected_ports.gif" width="140" height="21" border="0" alt=""></a></td>
		<td valign="middle" align="left">
		<a href="javascript:disableSelPorts();"><img src="/Images/disable_selected_ports.gif" width="140" height="21" border="0" alt=""></a></td>
    </tr>	
</table>	
<table border="0" cellpadding="0" cellspacing="0" align="center" class="tableColor">
<tr>
<td>
<table border="0" cellpadding="0" cellspacing="0">
   	<tr class="tableColor" align="left">
	   	<td width="42"><font class="tabsTextBig">Port</font></td>
		<td width="55"><font class="tabsTextBig">Disable</font></td>
		<td width="160"><font class="tabsTextBig">Alias</font></td>
		<td width="173"><font class="tabsTextBig">Connection Protocol</font></td>
		<td ><font class="tabsTextBig">Serial Config</font></td>
	</tr>
</table>
</td>
</tr>
	<tr>
		<td>
<!-- options width = 
	1 (0 ... 2); 3 whitespace; (60px) 
	2 (0 ... 3); 3 whitespace;	(71px)
	3 (0 ...17); 1 whitespace;	(160px)
	4 (0 ... 16); 3 whitespace;	(173px)
	5 (0 ... 14); ()-->		
<select name="physTable" size="10" multiple class="formText">
	<%get("system.ports.physTableHtml");%>
</select>
		</td>
	</tr>
</table>	
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/ports/physPorts.asp">
   <input type="hidden" name="urlError" value="/normal/ports/physPorts.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
   <!--*************************************************************************-->
   <input type="hidden" name="system.ports.placeholder" value="">
   <input type="hidden" name="system.ports.physAction.action" value="">
   <input type="hidden" name="system.ports.physAction.selected" value="">
</form> <!-- maybe can use form below once hooked up correctly; maybe not. -->
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
