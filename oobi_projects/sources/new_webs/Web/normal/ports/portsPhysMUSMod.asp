<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<title>Multi User Settings</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
	
<% ReqNum = getRequestNumber(); %>	

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	top.currentPage = self;
    top.helpPage = 'helpPages/portsPhysMUSHelp.asp';
	
	function showRest() 
	{
		if (document.MUSForm.multSess.value == 0 && document.MUSForm.sniffMode.value == 3){
			hide('document', 'Layer2');}
		else {
			show('document', 'Layer2');}
	}				

	function init()
	{	
		showRest();
		if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady){
			setUnsavedImage(parent.controls.document, <% get("_changesLed_");%>);
		} else
			setTimeout('init()', 200);
	}
	
	function copyData() 
	{
		for (var i=0; i < 4 ; i++)
			document.configForm.elements[6+i].value = document.MUSForm.elements[i].value;
		if (document.MUSForm.notifyUsers.checked == 1) {
			document.configForm.elements[10].value = 1;
		} else {
			document.configForm.elements[10].value = 0;
		}
	}

	function form_verification ()
	{
		var form_location = document.MUSForm;
		continue_verification = true;

		if ((form_location.elements[0].value != 0 && form_location.elements[0].value != 2 ) && form_location.elements[1].value == 3) {
			alert ("A sniff mode must be selected when multiple sessions are allowed."); 
			form_location.elements[1].focus();
			continue_verification = false;
		}
		if (continue_verification == true && (form_location.elements[0].value != 0  || form_location.elements[1].value != 3)) {
			checkElement (form_location.elements[2], false, false, false, false); 
		}
		if (continue_verification == true && (form_location.elements[0].value != 0  || form_location.elements[1].value != 3)) {
			checkElement (form_location.elements[3], true, false, false, false); 
		}
		if (continue_verification == true && (form_location.elements[0].value != 0  || form_location.elements[1].value != 3)) {
			checkHotKey (form_location.elements[3]); 
		}
	}
	
	function checkHotKey (element_for_verification)
	{
   	var chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; // all roman alphabet letters
      	if (element_for_verification.value.charAt(0) != '^' || element_for_verification.value.charAt(1) == '' || chars.indexOf(element_for_verification.value.charAt(1)) == -1)
      	{
	  	continue_verification = false;
		alert ('Invalid hotkey. Format should be a caret (^) and a letter.');
   	  	element_for_verification.focus();
   	  	element_for_verification.select();
   		}
		else continue_verification = true;  
	}
	
	function submit()
	{
		form_verification();
		if (continue_verification == true)
		{
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
<form name="MUSForm" method="post" action=" " onSubmit="return false;">
<table border="0" align="center" width="500" cellpadding="5">
<tr valign="top"><td align="left" width="170"><font class="label">Allow Multiple Sessions</font></td><td>
<select name="multSess" onChange="showRest();" class="formText">
	<% var sel = getVal("system.ports.physP.mus.multSess");%>
	<option value="0" <% if (sel==0) write("selected");%> >No</option>
	<option value="1" <% if (sel==1) write("selected");%> >Yes (show menu)</option>
	<option value="2" <% if (sel==2) write("selected");%> >Read/Write (do not show menu)</option>
	<option value="4" <% if (sel==4) write("selected");%> >ReadOnly (do not show menu)</option>
	</select>
</td></tr>
</table>
<table border="0" align="center" width="500" cellpadding="5">
<tr><td align="left" width="170"><font class="label">Sniff Mode</font></td><td>
	<select name="sniffMode" onChange="showRest();" class="formText">
	<% var sel=getVal("system.ports.physP.mus.sniffMode");%>
	<option value="0" <% if (sel==0) write("selected");%> >Out</option>
	<option value="1" <% if (sel==1) write("selected");%> >In</option>
	<option value="2" <% if (sel==2) write("selected");%> >In/Out</option>
	<option value="3" <% if (sel==3) write("selected");%> >No</option>
	</select>
</td></tr>
</table>
<div id="Layer2" style="position:absolute; left:0px; top:100px; width:100%; height:27px; z-index:1; visibility: hidden;"align="center">
<table border="0" align="center" width="500" cellpadding="5">
<tr><td align="left" width="170"><font class="label">Privileged Users/Groups</font></td>
<td align="left"><font class="formText">
	<input name="adminUsers" type="text" maxlength="255" size="20" class="formText" value="<% get("system.ports.physP.mus.adminUsers");%>">
</font></td></tr>
<tr><td align="left" width="170"><font class="label">Menu Hotkey</font></td>
<td align="left"><font class="formText">
	<input name="hotKey" type="text" maxlength="2" size="2" class="formText" value="<% get("system.ports.physP.mus.hotKey");%>">
</font></td></tr>
</table>
<table border="0" align="center" width="500" cellpadding="5">
<tr><td align="left" width="170">&nbsp;</td>
	<% var check=getVal("system.ports.physP.mus.notifyUsers");%>
	<td><input type="checkbox" name="notifyUsers" class="formText" <% if (check==1) write("checked");%>>
	&nbsp;&nbsp;&nbsp;&nbsp;
	<font class="label">Notify Users</font></td></tr>
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
   	<input type="hidden" name="system.ports.physP.mus.multSess" value="">
   	<input type="hidden" name="system.ports.physP.mus.sniffMode" value="">
   	<input type="hidden" name="system.ports.physP.mus.adminUsers" value="">
   	<input type="hidden" name="system.ports.physP.mus.hotKey" value="">
   	<input type="hidden" name="system.ports.physP.mus.notifyUsers" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html> 
