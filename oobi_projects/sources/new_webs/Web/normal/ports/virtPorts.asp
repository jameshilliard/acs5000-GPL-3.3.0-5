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
    parent.helpPage = 'helpPages/virtPortsHelp.asp';
    var target='target=' + window.name;
	     
	function init()
	{
		if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
			if (checkLeftMenu('/normal/ports/portsMenu.asp') == 0) {
				parent.menuReady = 0;
   				setTimeout('init()', 200);
				return;
   			}
			setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
			parent.menu.selectItem('VIRT');
			parent.topMenu.selectItem(parent.menu.topItem);
			if (parseInt(getIPmode()) == 2) {
				show2('document', 'virtPortsNOK');
				hide2('document', 'virtPortsOK');
			} else {
				show2('document', 'virtPortsOK');
				hide2('document', 'virtPortsNOK');
			}
		} else {
   			setTimeout('init()', 200);
		}
	}
 
	function editVirtualPort(selectedOption)
	{
		copyData();
		getValueSelectedEdit('portsVirtPortsEntry.asp','virtPort','500','450',target, document.virtPortForm.virtPortTable,'yes');
	}

	function addVirtualPort()
	{
		copyData();
		getSelectedAdd('portsVirtPortsEntry.asp','virtPort','500','450',target,'yes');
	}

	function deleteVirtualPort() {
		deleteSelOpt(document.virtPortForm.virtPortTable);
	}

	function copyData()
	{
		if (document.virtPortForm.virtPortTable.options.length > 1) {
			document.configForm.elements[5].value = "," + 
						getValues(document.virtPortForm.virtPortTable,1);
		}
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
<div id="virtPortsOK" style="display:none;">
<table align="center" height="100%" cellspacing="0" cellpadding="0" border="0">
	<tr>
		<td align="center" valign="middle">
			<form name="virtPortForm" action="" onSubmit="return false;">
				<table border="0" cellpadding="0" cellspacing="0" align="center">
					<tr>
						<td>&nbsp;</td>
					</tr>
					<tr>
						<td align="left" class="tableColor">         
							<font class="tabsTextBig">Slave</font>      	
						</td>
					</tr>
					<tr>
						<td align="center">
							<select name="virtPortTable" size="5" class="formText">
								<%get("system.ports.virtualP.virtualInfoHtml");%>
								<option value="-1">
									&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 
								</option>
							</select>
						</td>
					</tr>
				</table>
				<table border="0" cellpadding="0" cellspacing="0" align="center">
					<tr>
						<td align="center" height="40" valign="middle">
							<a href="javascript:addVirtualPort()">
								<img src="/Images/addButton.gif" width="47" height="21" border="0" alt=""></a>
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
							<a href="javascript:deleteVirtualPort()">
								<img src="/Images/deleteButton.gif" width="47" height="21" border="0" alt=""></a>
							&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
							<a href="javascript:editVirtualPort(document.virtPortForm.virtPortTable.selectedIndex)">
								<img src="/Images/editButton.gif" width="47" height="21" border="0" alt=""></a>
						</td>
					</tr>
				</table>
			</form>
		</td>
	</tr>
</table>
</div>
<div id="virtPortsNOK" style="display:none;">
<table align="center" height="100%" cellspacing="0" cellpadding="0" border="0">
	<tr>
		<td height="100%" align="center">
			<table height="100%" width="100%" cellpadding="0" cellspacing="0" border="0">
				<tr height="100%" align="center">
					<td align="center" valign="middle">
						<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">
							<tr>
								<td align="center" valign="middle">
									<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
										<tr>
											<td align="center" valign="middle" class="label_for_tables">
												You have selected a service that is currently not supported in IPv6.<br>
												If you like to use this service please enable IPv4 for Virtual Ports to work
											</td>
										</tr>
									</table>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</div>
	
<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/ports/virtPorts.asp">
	<input type="hidden" name="urlError" value="/normal/ports/virtPorts.asp">
	<input type="hidden" name="request" value=<%write(ReqNum);%>>
	<input type="hidden" name="system.ports.virtualP.virtualInfoSet" value="">
</form>
<script type="text/javascript">
	parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
