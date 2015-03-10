<!doctype html public "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>dataBuffering</title>
  	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

<%	
	ReqNum = getRequestNumber();
	var input_size = 15;
	var input_maxlength = 15;
	if (getVal('_IPv6_')) {
		input_size = 25;
		input_maxlength = 50;
	}
	
%>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	top.currentPage = self;
	top.helpPage = 'helpPages/portsPhysDataBuffHelp.asp';

	function copyData() {
		if (document.bufferingForm.enableDb.checked == false) {
			document.configForm.elements[8].value = 0;
			document.configForm.elements[18].value = 0;
		} else {
			document.configForm.elements[6].value = document.bufferingForm.destType.value;
			if (document.bufferingForm.destType.value == 0) {
				document.configForm.elements[8].value = document.bufferingForm.fileSize.value;
				document.configForm.elements[7].value = document.bufferingForm.mode.value;
			} else {
				document.configForm.elements[8].value = 1024;
				document.configForm.elements[9].value = document.bufferingForm.nfsFile.value;
			}
			if (document.bufferingForm.timeStamp.checked == true) {
				document.configForm.elements[10].value = 1;
			} else {
				document.configForm.elements[10].value = 0;
			}
			if (document.bufferingForm.notAllTime.checked == true) {
				document.configForm.elements[17].value = 1;
			} else {
				document.configForm.elements[17].value = 0;
			}
			if (document.bufferingForm.ADEnable.checked == true) {
				document.configForm.elements[18].value = 1;
			} else {
				document.configForm.elements[18].value = 0;
			}
			document.configForm.elements[19].value = document.bufferingForm.ADTimeOut.value;
			document.configForm.elements[11].value = document.bufferingForm.showMenu.value;
		}
		if (document.bufferingForm.enableSys.checked == false) {
			document.configForm.elements[12].value = 0;
		} else {
			document.configForm.elements[12].value = 1;
			document.configForm.elements[14].value = document.bufferingForm.facility.value;
			document.configForm.elements[15].value = document.bufferingForm.bufferSize.value;
			if (document.bufferingForm.notSysAllTime.checked) {
				document.configForm.elements[16].value = 1;
			} else {
				document.configForm.elements[16].value = 0;
			}
		}
		document.configForm.elements[13].value = document.bufferingForm.sysServer.value;
		return true;
	}
   
	function init() {
		showEnabled();
		showBuffOrNot();
		if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady){
			setUnsavedImage(parent.controls.document, <% get("_changesLed_");%>);
		} else {
			setTimeout('init()', 200);
		}
	}

	function setDivStyleDisplay(val, divId) {
		nav = document.getElementById(divId).style;
		nav.display = val;
	}

	function showEnabled() {
		if (document.bufferingForm.enableDb.checked == true) {
			setDivStyleDisplay('block', 'buffering');
			setDivStyleDisplay('block', 'other');
			showSelectedDestination();
		} else {
			setDivStyleDisplay('none', 'buffering');
			setDivStyleDisplay('none', 'other');
			setDivStyleDisplay('none', 'local');
			setDivStyleDisplay('none', 'remoteOK');  
			setDivStyleDisplay('none', 'remoteNOK');  
		}
	}

	function showSelectedDestination() {
		var selector = document.bufferingForm.destType;
		var remoteDiv = (parseInt(getIPmode()) == 2)? 'remoteNOK' : 'remoteOK';
		if (selector.options[selector.selectedIndex].value == '0') {
			setDivStyleDisplay('block', 'local');
			setDivStyleDisplay('none', remoteDiv);
		} else {
			setDivStyleDisplay('block', remoteDiv);
			setDivStyleDisplay('none', 'local');
		}
	}
	
	function showBuffOrNot() {
		if (document.bufferingForm.enableSys.checked == true) {
			setDivStyleDisplay('block', 'other2');
		} else {
			setDivStyleDisplay('none', 'other2');
		}
	}

	function form_verification() {
		continue_verification = true;
		var form_location = document.bufferingForm;
		if ((form_location.enableDb.checked) && (parseInt(getIPmode()) == 2) && (form_location.destType.value == '1')) {
			continue_verification = false;
		}
		if (form_location.enableDb.checked == true && form_location.destType.value == 0) {
			checkElement (form_location.fileSize, true, false, false, true,  1, 'no');
		}
		if (continue_verification == true && form_location.enableDb.checked == true && form_location.destType.value == 1) {
			checkElement (form_location.nfsFile, true, false, false, false);
		}
		if (continue_verification == true && form_location.enableSys.checked == true) {
			continue_verification = validate(form_location.sysServer, "Fill,IP");
		}
		if (continue_verification == true && form_location.enableSys.checked == true) {
			checkElement (form_location.bufferSize, true, false, false, false);
		}
		if (continue_verification == true && form_location.enableSys.checked == true && form_location.bufferSize.value != 0) {
			checkBufferSize (form_location.bufferSize);
		}
		if (continue_verification == true && form_location.enableDb.checked == true && form_location.ADTimeOut.value <= 0) {
			if (form_location.ADEnable.checked == true) {
				continue_verification = false;
				alert('Hostame discovery timeout value must be greater than zero.');
				form_location.ADTimeOut.focus();
				form_location.ADTimeOut.select();
			} else {
				if (form_location.ADTimeOut.value < 0) {
					continue_verification = false;
					alert('Hostame discovery timeout value cannot be negative.');
					form_location.ADTimeOut.focus();
					form_location.ADTimeOut.select();
				}
			}
		}
	}
	
	function checkBufferSize(element_for_verification) {
		if (!validateNumeric(element_for_verification)) {
			alert('Please enter only numbers (without spaces).');
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
		} else if (element_for_verification.value < 40 || element_for_verification.value > 254) {
			alert('Value must be greater than 39 and less than 255');
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
		}
	}
	
	function submit() {
		form_verification();
		if (continue_verification == true) {
			copyData();
			if (document.configForm.elements[0].value != 0) {
				document.configForm.urlOk.value="/normal/ports/physPorts.asp";
			}
			setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = null;
		}
	}

</script>
</head>

<body onload="init();" class="bodyForLayers">
<form action="" name="bufferingForm" onSubmit="return false;">	
<table width="400" border="0" align="center" cellpadding="0" cellspacing="0">	      
	<tr valign="middle">
		<td width="20">&nbsp;</td>
		<td width="190">
<% var status = getVal("system.ports.physP.dataBuf.fileSize");%>
			<input type="checkbox" name="enableDb" <% if (status!=0) write("checked");%> onclick="showEnabled()">
			&nbsp;<font class="label">Enable Data Buffering</font>
		</td>
		<td width="190">
<% var status = getVal("system.ports.physP.sysBuf.enable");%>
			<input type="checkbox" name="enableSys" <% if (status==1) write("checked");%> onclick="showBuffOrNot()">
			&nbsp;<font class="label">Buffer to Syslog</font>
		</td>
	</tr>
</table>   
<div id="buffering" style="display: none">	
<table width="500" border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>				
		<td align="left">
			<font class="label"><br>Destination</font>
			<select name="destType" class="formText" onChange="showSelectedDestination()">
<% var sel = getVal("system.ports.physP.dataBuf.destType");%>
				<option value="0" <% if (sel=="0") write("selected");%>> Local </option>
				<option value="1" <% if (sel=="1") write("selected");%>> Remote </option>
			</select>
		</td>
	</tr>
</table>
</div>
<div id="local" style="display: none">	      
<table width="500" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<br>
		<td align="left" width="12%">
			<font class="label">Mode</font>
		</td>
		<td align="left" width="32%">
			<select name="mode" class="formText">
<% sel = getVal("system.ports.physP.dataBuf.mode"); %>
				<option value="0" <% if (0 == sel) write("selected"); %>> Circular </option>
				<option value="1"<% if (1 == sel) write("selected"); %>> Linear </option>
			<script type="text/javascript">
				if (top.family_model != "AVCS") document.write('\
					<option value="2"<% if (2 == sel) write("selected"); %>> DSView </option>\
					<option value="3"<% if (3 == sel) write("selected"); %>> DSView-Local </option>');
			</script>
			</select>
		</td>
		<td align="left" width="28%">
			<font class="label">File Size (Bytes)</font>
		</td>
		<td align="left" width="28%">
			<input type="text" size="10" class="formText" name="fileSize" value="<% get("system.ports.physP.dataBuf.fileSize");%>" >
		</td>
	</tr>
</table>
</div>	
<div id="remoteOK" style="display: none">
<table width="500" border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>
		<td align="left">
			<font class="label"><br>NFS File Path</font>
			&nbsp;<input type="text" size="20" class="formText" maxlength="99" name="nfsFile" value="<% get("system.ports.physP.dataBuf.nfsFile");%>">
		</td>
	</tr>			
</table>	
</div>	      
<div id="remoteNOK" style="display: none">
<table width="500" border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>
		<br>
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
												If you like to use this service please enable IPv4 for NFS to work
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
<div id="other"style="display: none">
<table width="500" border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>
		<td align="left" colspan="2">
			<font class="label"><br>Record the timestamp in the data buffering file</font>
<% var sel=getVal("system.ports.physP.dataBuf.timeStamp");%>
			&nbsp;<input type="checkbox" name="timeStamp" class="formText" <% if (sel==1) write("checked");%>>
		</td>
	</tr>
	<tr>
		<td align="left" colspan="2">
			<font class="label"><br>Buffering data only when nobody is connected to the port</font>
<% var sel = getVal("system.ports.physP.dataBuf.allTime");%>
			&nbsp;<input type="checkbox" name="notAllTime" class="formText" <% if (sel==1) write("checked");%>>
		</td>
	</tr>
	<tr>
		<td align="left" colspan="2">
		    &nbsp;
		</td>
	</tr>
	<tr>
		<td width="50%" align="left">
			<font class="label">Hostname Discovery</font>
<% var sel = getVal("system.ports.physP.dataBuf.ADEnable");%>
			&nbsp;<input type="checkbox" name="ADEnable" class="formText" <% if (sel==1) write("checked");%>>
		</td>
		<td width="50%" align="left">
			<font class="label">Timeout (seconds)</font>
			&nbsp;<input type="text" size="5" class="formText" name="ADTimeOut" value="<% get("system.ports.physP.dataBuf.ADTimeOut");%>">
		</td>
	</tr>
	<tr>
		<td align="left" colspan="2">
			<font class="label"><br>Show Menu</font>
			&nbsp;<select name="showMenu"  class="formText">
<% sel = getVal("system.ports.physP.dataBuf.showMenu"); %>
				<option value="0"<% if (0 == sel) write("selected"); %>>Show all options </option>
				<option value="1"<% if (1 == sel) write("selected"); %>>No</option>
				<option value="2"<% if (2 == sel) write("selected"); %>>Show data buffering file only</option>
				<option value="3"<% if (3 == sel) write("selected"); %>>Show without the erase options</option>
				<option value="4"<% if (4 == sel) write("selected"); %>>Show and erase data buffering file</option>
			</select>
		</td>
	</tr>
</table>
</div>
<div id="other2" style="display: none">
<table width="500" border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>
		<td align="left">
			<font class="label"><br>Syslog Server</font>
			<input type="text" name="sysServer" maxlength="<%write(input_maxlength);%>" value="<%get("system.ports.physP.sysBuf.sysServer");%>" size="<%write(input_size);%>" class="formText">
		</td>
	</tr>
</table>
<table width="500" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr align="left">
		<br>
		<td width="20%">
			<font class="label">Facility Number</font>
		</td>
		<td width="20%">
			<select name="facility" class="formText">
<% var sel=getVal("system.ports.physP.sysBuf.facility");%>
				<option value="0" <% if (sel==0) write("selected");%>><font class="formText">Local0</font></option>
				<option value="1" <% if (sel==1) write("selected");%>><font class="formText">Local1</font></option>
				<option value="2" <% if (sel==2) write("selected");%>><font class="formText">Local2</font></option>
				<option value="3" <% if (sel==3) write("selected");%>><font class="formText">Local3</font></option>
				<option value="4" <% if (sel==4) write("selected");%>><font class="formText">Local4</font></option>
				<option value="5" <% if (sel==5) write("selected");%>><font class="formText">Local5</font></option>
				<option value="6" <% if (sel==6) write("selected");%>><font class="formText">Local6</font></option>
				<option value="7" <% if (sel==7) write("selected");%>><font class="formText">Local7</font></option>
			</select>
		</td>
		<td width="34%">
			<font class="label">Syslog Buffer Size (Bytes)</font>
		</td>
		<td width="20%">
			<input type="text" name="bufferSize" maxlength="3" size="3" class="formText" value="<% get("system.ports.physP.sysBuf.bufferSize");%>">
		</td>
	</tr>
</table>
<table width="500" border="0" align="center" cellpadding="4" cellspacing="0">
	<tr>
		<td align="left">
			<font class="label"><br>Buffer syslog only when nobody is connected to the port</font>
<% var sel = getVal("system.ports.physP.sysBuf.allTime");%>
			&nbsp;<input type="checkbox" name="notSysAllTime" class="formText" <% if (sel==1) write("checked");%>>
		</td>
	</tr>
</table>
</div>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/ports/physPorts.asp">
	<input type="hidden" name="urlError" value="/normal/ports/physPorts.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name="system.ports.physP.action" value="2">
<!--these hidden inputs are used to store values of form elements inside layers-->
	<input type="hidden" name="system.ports.physP.dataBuf.destType" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.mode" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.fileSize" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.nfsFile" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.timeStamp" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.showMenu" value="">
	<input type="hidden" name="system.ports.physP.sysBuf.enable" value="">
	<input type="hidden" name="system.ports.physP.sysBuf.sysServer" value="">
	<input type="hidden" name="system.ports.physP.sysBuf.facility" value="">
	<input type="hidden" name="system.ports.physP.sysBuf.bufferSize" value="">
	<input type="hidden" name="system.ports.physP.sysBuf.allTime" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.allTime" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.ADEnable" value="">
	<input type="hidden" name="system.ports.physP.dataBuf.ADTimeOut" value="">
<!--*************************************************************************-->           
</form>

<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%> 
</body>
</html>

