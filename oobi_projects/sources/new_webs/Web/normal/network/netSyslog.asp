<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>

<%
	var IPv6 = getVal('_IPv6_');
	var width = 15;
	var input_size = 15;
	var input_maxlength = 15;
	if (IPv6) {
		width = 46;
		input_size = 25;
		input_maxlength = 50;
	}
%>

	var width = <%write(width);%>;
	var cols = new Array(
			new _col("Syslog Servers", width, width)
		);
	var serverTable = new selTable("serverTable", cols, 5);
	var line = String.fromCharCode(160);
	for (i=1; i<width; i++) {
		line += String.fromCharCode(160);
	}

</script>

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">

	<% ReqNum = getRequestNumber(); %>
	<% FamilyModel = getVal('_familymodel_'); %>

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	parent.currentPage = self;
	parent.helpPage = 'helpPages/netSyslogHelp.asp';
	
	var servers = <%get("system.systemLog.serversHtml");%>;
	
	function fill() {
		fillSelTable(document.serverForm, serverTable, servers);
		document.serverForm.serverTable.selectedIndex = servers.length;
	}

	function init()	{
		if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
			if (checkLeftMenu('/normal/network/networkMenu.asp') == 0) {
				parent.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
			parent.topMenu.selectItem(parent.menu.topItem);
			parent.menu.selectItem("LOG");
		} else {
			setTimeout('init()', 200);
		}
	}

	function form_verification() {
		continue_verification = verify([
			[document.serverForm.newServer, "Fill,IP"]
		]);
	}
	  
	function addServer(object) {	 
		form_verification();
		// check for duplication
		for (var i = 0; i < object.serverTable.length-1; i++) {
			if (object.serverTable.options[i].value == object.newServer.value) {
				alert('This server is already in the list!');
				continue_verification = false;
				document.serverForm.newServer.focus();
				document.serverForm.newServer.select();
				break;
			}
		}
		if (continue_verification == true) {
			var defaultSelected = true;
			var selected = true;
			var newOption = new Option(object.newServer.value, object.newServer.value, defaultSelected, selected);
			var length = object.serverTable.length;
			object.serverTable.options[length-1] = newOption;
			var widthOption = new Option(line, -1, false, false);
			object.serverTable.options[length] = widthOption;
			object.newServer.value='';
		}
	}

	function delServer() {
		deleteSelOpt(document.serverForm.serverTable);
	}

	function submit() {
		elemX = 0;
		document.configForm.elements[6].value = document.serverForm.CASfacility.value;
		if (parent.family_model == "KVM" || parent.family_model == "ONS") {
			document.configForm.elements[7].name = "system.systemLog.KVMfacility";
			document.configForm.elements[7].value = document.serverForm.KVMfacility.value;
			elemX++;
		}
		document.configForm.elements[8].value = 0;
		if (document.serverForm.console.checked == true) {
			document.configForm.elements[8].value = document.configForm.elements[8].value-0 + 1;
		}
		if (document.serverForm.root.checked == true) {
			document.configForm.elements[8].value = document.configForm.elements[8].value-0 + 2;
		}
		if (document.serverForm.server.checked == true) {
			document.configForm.elements[8].value = document.configForm.elements[8].value-0 + 4;
		}
		document.configForm.elements[9].value = getValues(document.serverForm.serverTable,1);
		elemX += 6;
		if (parent.family_model == "ACS" || parent.family_model == "ONS" || parent.family_model == "AVCS") {
			document.configForm.elements[10].value = 0;
			for (ix=0; ix<8; ix++) {
				if (document.serverForm.elements[elemX++].checked == true) {
					document.configForm.elements[10].value = document.configForm.elements[10].value-0 + (1<<ix);
				}
			}
		}
		if (parent.family_model == "KVM" || parent.family_model == "ONS") {
			document.configForm.elements[11].value = 0;
			document.configForm.elements[11].name = "system.systemLog.KVMlevel";
			for (ix=0; ix<8; ix++) {
				if (document.serverForm.elements[elemX++].checked == true) {
					document.configForm.elements[11].value =
						document.configForm.elements[11].value-0 + (1<<ix);
				}
			}
		}
		if (parent.family_model == "KVM") {
			document.configForm.elements[10].value = 0;
			for (ix=0; ix<8; ix++) {
				if (document.serverForm.elements[elemX++].checked == true) {
					document.configForm.elements[10].value =
						document.configForm.elements[10].value-0 + (1<<ix);
				}
			}
		}
		document.configForm.elements[12].value = 0;
		for (ix=0; ix<8; ix++) {
			if (document.serverForm.elements[elemX++].checked == true) {
				document.configForm.elements[12].value =
					document.configForm.elements[12].value-0 + (1<<ix);
			}
		}
		document.configForm.elements[13].value = 0;
		for (ix=0; ix<8; ix++) {
			if (document.serverForm.elements[elemX++].checked == true) {
				document.configForm.elements[13].value =
					document.configForm.elements[13].value-0 + (1<<ix);
			}
		}
		document.configForm.elements[14].value = 0;
		for (ix=0; ix<8; ix++) {
			if (document.serverForm.elements[elemX++].checked == true) {
				document.configForm.elements[14].value =
					document.configForm.elements[14].value-0 + (1<<ix);
			}
		}
		document.configForm.submit();
		parent.mainReady = null;
	}
	</script>
</head>

<body onload="init();" class="bodyMain">
<table align="center" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr><td align="center" valign="middle">
<form name="serverForm" action="" onSubmit="return false;">
<table width="100%" height="50" border="0" align="center" cellpadding="0">
<tr>
<script type="text/javascript">
  if (parent.family_model == "ACS" || parent.family_model == "ONS" || parent.family_model == "AVCS") {
    document.write('<td align="center" valign="top">');
    if ((parent.family_model == "ACS")||(parent.family_model == "AVCS"))
      document.write('<font class="label">CAS Ports Facility</font>&nbsp;&nbsp;');
    else
      document.write('<font class="label">CAS/AUX Ports Facility</font>&nbsp;&nbsp;');
    document.write('<select name="CASfacility" class="formText">');
    var selected = <%get("system.systemLog.facilityNumber");%>;
    for (ix = 0; ix <=7; ix++) {
      if (selected != ix)
        document.write('<option value=\"'+ix+'\">Local'+ix+'</option>');
      else
        document.write('<option value=\"'+ix+'\" selected>Local'+ix+'</option>');
    }
    document.write('</select></td>');
  }

  if (parent.family_model == "KVM" || parent.family_model == "ONS") {
<%
if (FamilyModel == "KVM" || FamilyModel == "ONS") {
  var selected = getVal("system.systemLog.KVMfacility");
} else {
  var selected = 0;
}
%>
     document.write('<td align="center" valign="top">');
     document.write('<font class="label">KVM Ports Facility</font>&nbsp;&nbsp;');
     document.write('<select name="KVMfacility" class="formText">');
     for (ix = 0; ix <=7; ix++) {
       if ( <%write(selected);%> != ix) 
         document.write('<option value=\"'+ix+'\">Local'+ix+'</option>');
       else 
         document.write('<option value=\"'+ix+'\" selected>Local'+ix+'</option>');
     }
   	 document.write('</select></td>');
  }

  if (parent.family_model == "KVM") {
    document.write('<td align="center" valign="top">');
    document.write('<font class="label">AUX Port Facility</font>&nbsp;&nbsp;');
    document.write('<select name="CASfacility" class="formText">');
    var selected = <%get("system.systemLog.facilityNumber");%>;
    for (ix = 0; ix <=7; ix++) {
      if (selected != ix) 
        document.write('<option value=\"'+ix+'\">Local'+ix+'</option>');
      else 
        document.write('<option value=\"'+ix+'\" selected>Local'+ix+'</option>');
    }
    document.write('</select></td>');
  }
</script>
</tr>
</table>

<table align="center" width="100%" cellspacing="5" cellpadding="0" border="0">
<tr><td>
  <table align="center"><tr><td align="center" valign="bottom" height="30" width="100%" nowrap>
    <font class="label">Syslog Destination</font></td></tr>
  </table>
<script type="text/javascript">

  dest = <%get("system.systemLog.dest");%>;
  document.write('<table align="center" valign="bottom" height="30" width="100%">');
  document.write('<tr><td><font class="label">');
  chk=(dest & 1)?" checked ":" ";
  document.write('<input type="checkbox" '+chk+' name="console" id="console" value="">');
  document.write('&nbsp;Console</font></td>');
  document.write('<td><font class="label">');
  chk=(dest & 2)?" checked ":" ";
  document.write('<input type="checkbox" '+chk+' name="root" id="root" value="">');
  document.write('&nbsp;Root User</font></td>');
  document.write('<td><font class="label">');
  chk=(dest & 4)?" checked ":" ";
  document.write('<input type="checkbox" '+chk+' name="server" id="server" value="">');
  document.write('&nbsp;Server</font></td></tr>');
  document.write('</table>');
</script>
</td></tr>
</table>

<%if (IPv6) {
write('\
<table width="460" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr align="center">
		<td align="center" valign="middle">
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td align="center" valign="middle" height="30" colspan="2">
						<font class="label">New Syslog Server</font>
					</td>
				</tr>
				<tr>
					<td align="center" valign="middle">
						<input class="formText" type="text" maxlength="50" name="newServer" size="25">
					</td>
					<td height="40" align="center" valign="middle">
						&nbsp;&nbsp;<input type="image" src="/Images/addToButton.gif" onClick="addServer(this.form);" align="texttop" border="0">
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr align="center">
		<td align="center" valign="middle">
			<script>
				serverTable.drawSelTable();
			</script>
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td height="40" align="center" valign="middle">
						<a href="javascript:delServer();">
						<img src="/Images/deleteButton.gif" alt="" width="47" height="21" border="0"></a>  
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>');
} else {
write('\
<table width="460" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr align="center">
		<td align="center" valign="middle">
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td align="center" valign="middle" height="30" colspan="2">
						<font class="label">New Syslog Server</font>
					</td>
				</tr>
				<tr>
					<td align="center" valign="middle">
						<input class="formText" type="text" maxlength="15" name="newServer" size="15">
					</td>
				</tr>
				<tr>
					<td height="40" align="center" valign="middle">
						<input type="image" src="/Images/addToButton.gif" onClick="addServer(this.form);" align="texttop" border="0">
					</td>
				</tr>
			</table>
		</td>
		<td align="center" valign="middle">
			<script>
				serverTable.drawSelTable();
			</script>
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td height="40" align="center" valign="middle">
						<a href="javascript:delServer();">
						<img src="/Images/deleteButton.gif" alt="" width="47" height="21" border="0"></a>  
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>');
};%>

<table align="center" width="100%" cellspacing="5" cellpadding="0" border="0">
<tr><td>

<script type="text/javascript">
                                                                                
  var lvlName = new Array('Emergency', 'Alert',  'Critical', 'Error',
                          'Warning',   'Notice', 'Info',     'Debug');
  var lvlid = new Array('emerg',   'alert',  'crit', 'err',
                        'warning', 'notice', 'info', 'debug');

  if (parent.family_model == "ACS" || parent.family_model == "ONS" || parent.family_model == "AVCS") {
    document.write('<table align="center"><tr><td align="center" valign="bottom" height="30" width="100%" nowrap>');

    if ((parent.family_model == "ACS")||(parent.family_model == "AVCS"))
      document.write('<font class="label">Filter CAS Log Messages by Level</font>');
    else 
      document.write('<font class="label">Filter CAS/AUX Log Messages by Level</font>');

    document.write('</td></tr></table>');
    document.write('<table align="center" valign="bottom" height="30" width="100%"><tr>');
                                                                                
    var level = <%get("system.systemLog.CASlevel");%>;
    for (ix=0; ix<8; ix++) {
      chk=(level & (1<<ix))?" checked ":" ";
      document.write('<td><font class="label"><input type="checkbox"'+chk+'name="CAS'+lvlid[ix]+'" id="CAS'+lvlid[ix]+'" value="">&nbsp;'+lvlName[ix]+'</font></td>');
      if (ix == 3) document.write('</tr><tr>');
    }
    document.write('</tr></table>');
  }

  if (parent.family_model == "KVM" || parent.family_model == "ONS") {
    document.write('<table align="center"><tr><td align="center" valign="bottom" height="30" width="100%" nowrap>');
    document.write('<font class="label">Filter KVM Log Messages by Level</font>');
    document.write('</td></tr></table>');
    document.write('<table align="center" valign="bottom" height="30" width="100%"><tr>');
                                                                                
    var level = <%get("system.systemLog.KVMlevel");%>;
    for (ix=0; ix<8; ix++) {
      chk=(level & (1<<ix))?" checked ":" ";
      document.write('<td><font class="label"><input type="checkbox"'+chk+'name="KVM'+lvlid[ix]+'" id="KVM'+lvlid[ix]+'" value="">&nbsp;'+lvlName[ix]+'</font></td>');
      if (ix == 3) document.write('</tr><tr>');
    }
    document.write('</tr></table>');
  }

  if (parent.family_model == "KVM") {
    document.write('<table align="center"><tr><td align="center" valign="bottom" height="30" width="100%" nowrap>');
    document.write('<font class="label">Filter AUX Log Messages by Level</font>');
    document.write('</td></tr></table>');
    document.write('<table align="center" valign="bottom" height="30" width="100%"><tr>');
                                                                                
    var level = <%get("system.systemLog.CASlevel");%>;
    for (ix=0; ix<8; ix++) {
      chk=(level & (1<<ix))?" checked ":" ";
      document.write('<td><font class="label"><input type="checkbox"'+chk+'name="CAS'+lvlid[ix]+'" id="CAS'+lvlid[ix]+'" value="">&nbsp;'+lvlName[ix]+'</font></td>');
      if (ix == 3) document.write('</tr><tr>');
    }
    document.write('</tr></table>');
  }

  document.write('<table align="center"><tr><td align="center" valign="bottom" height="30" width="100%" nowrap>');
  document.write('<font class="label">Filter Data Buffering Log Messages by Level</font>');
  document.write('</td></tr></table>');
  document.write('<table align="center" valign="bottom" height="30" width="100%"><tr>');
                                                                                
  var level = <%get("system.systemLog.Buflevel");%>;
  for (ix=0; ix<8; ix++) {
    chk=(level & (1<<ix))?" checked ":" ";
    document.write('<td><font class="label"><input type="checkbox"'+chk+'name="Buf'+lvlid[ix]+'" id="Buf'+lvlid[ix]+'" value="">&nbsp;'+lvlName[ix]+'</font></td>');
    if (ix == 3) document.write('</tr><tr>');
  }
  document.write('</tr></table>');

  document.write('<table align="center"><tr><td align="center" valign="bottom" height="30" width="100%" nowrap>');
  document.write('<font class="label">Filter Web Log Messages by Level</font>');
  document.write('</td></tr></table>');
  document.write('<table align="center" valign="bottom" height="30" width="100%"><tr>');
                                                                                
  var level = <%get("system.systemLog.Weblevel");%>;
  for (ix=0; ix<8; ix++) {
    chk=(level & (1<<ix))?" checked ":" ";
    document.write('<td><font class="label"><input type="checkbox"'+chk+'name="Web'+lvlid[ix]+'" id="Web'+lvlid[ix]+'" value="">&nbsp;'+lvlName[ix]+'</font></td>');
    if (ix == 3) document.write('</tr><tr>');
  }
  document.write('</tr></table>');

  document.write('<table align="center"><tr><td align="center" valign="bottom" height="30" width="100%" nowrap>');
  document.write('<font class="label">Filter System Log Messages by Level</font>');
  document.write('</td></tr></table>');
  document.write('<table align="center" valign="bottom" height="30" width="100%"><tr>');
                                                                                
  var level = <%get("system.systemLog.Syslevel");%>;
  for (ix=0; ix<8; ix++) {
    chk=(level & (1<<ix))?" checked ":" ";
    document.write('<td><font class="label"><input type="checkbox"'+chk+'name="Sys'+lvlid[ix]+'" id="Sys'+lvlid[ix]+'" value="">&nbsp;'+lvlName[ix]+'</font></td>');
    if (ix == 3) document.write('</tr><tr>');
  }
  document.write('</tr></table>');
</script>
</td></tr></table>

</form>
<form name="configForm" method="POST" action="/goform/Dmf">
      <input type="hidden" name="system.req.action" value="">
      <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
      <input type="hidden" name="urlOk" value="/normal/network/netSyslog.asp">
      <input type="hidden" name="urlError" value="/normal/network/netSyslog.asp">
      <input type="hidden" name="request" value="<%write(ReqNum);%>">
      <input type="hidden" name="system.systemLog.mode" value="2">
<!--*************************************************************************-->
      <input type="hidden" name="system.systemLog.facilityNumber" value="">
      <input type="hidden" name="system.systemLog.dummy" value="0">
      <input type="hidden" name="system.systemLog.dest" value="">
      <input type="hidden" name="system.systemLog.serversCs" value="">
      <input type="hidden" name="system.systemLog.CASlevel" value="">
      <input type="hidden" name="system.systemLog.dummy" value="0">
      <input type="hidden" name="system.systemLog.Buflevel" value="">
      <input type="hidden" name="system.systemLog.Weblevel" value="">
      <input type="hidden" name="system.systemLog.Syslevel" value="">
</form>
</td>
</tr>
</table>
<script type="text/javascript">
fill();
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
