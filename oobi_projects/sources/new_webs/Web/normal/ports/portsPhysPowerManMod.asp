<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
 
<html>
<head>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript"> 

	<% ReqNum = getRequestNumber(); 
	   numPM = getVal("system.ports.physAction.numPowerMgm");
	   numIPMI = getVal("system.applications.pmIpmi.numActiveIpmiDev");
	%>		   

	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	top.currentPage = self;
	top.helpPage = 'helpPages/portsPhysPowerManHelp.asp';
	var target='target=' + window.name;
	var ssid="&SSID=<%get("_sid_");%>";
	var pmen=<% write(numPM);%>;
	var ipmien=<% write(numIPMI);%>;
	var family_model = "<%get('_familymodel_');%>";
	
	var enablePMlabel = (pmen==0)? "labelInactive" : "label";
	var enableIPMIlabel = (ipmien==0)? "labelInactive" : "label";

	function copyData()
	{
		if (document.PowerMgmForm.enablePM.checked == true) {
			document.configForm.elements[6].value = 1;
		} else {
			document.configForm.elements[6].value = 0;
		}
		document.configForm.elements[7].value =  document.PowerMgmForm.pmoutlets.value;
		document.configForm.elements[8].value = document.PowerMgmForm.pmKey.value;
		if (document.PowerMgmForm.elements[6].checked) {
			document.configForm.elements[9].value = 1;
			document.configForm.elements[10].value = "";
		} 
		if (document.PowerMgmForm.elements[7].checked) {
			document.configForm.elements[9].value = 0;
			buildString(document.PowerMgmForm.usersTable);
			document.configForm.elements[10].value = users_and_groups
		}
		if (document.PowerMgmForm.enableIPMI.checked == true) {
			document.configForm.elements[11].value = 1;
		} else {
			document.configForm.elements[11].value = 0;
		}
		/* IPMI key */
		document.configForm.elements[12].value = document.PowerMgmForm.ipmiKey.value;
		/* IPMI Server Id */
		document.configForm.elements[13].value = document.PowerMgmForm.ipmidevice.value;
	}
	  
	function showEnabled()
	{
		if (pmen == 0) {
			document.PowerMgmForm.enablePM.setAttribute('enabled',false);
			document.PowerMgmForm.enablePM.setAttribute('disabled',true);
			document.PowerMgmForm.enablePM.checked = false;
			enablePMlabel = "labelInactive";
			hide ('document', 'enablePMdiv');
			hide ('document', 'choose_users');
			return;	
		}
		if (document.PowerMgmForm.enablePM.checked == true) {
			show ('document', 'enablePMdiv');
			showchoose_users();
		} else {
			hide ('document', 'enablePMdiv');
			hide ('document', 'choose_users');
		}
	}
	  
	function showIPMI()
	{
		if (ipmien == 0) {
			document.PowerMgmForm.enableIPMI.setAttribute('enabled',false);
			document.PowerMgmForm.enableIPMI.setAttribute('disabled',true);
			document.PowerMgmForm.enableIPMI.checked = false;
			enableIPMIlabel = "labelInactive";
			hide ('document', 'enableIPMIdiv');
			return;	
		}
		if (document.PowerMgmForm.enableIPMI.checked == true) {
			show ('document', 'enableIPMIdiv');
		} else {
			hide ('document', 'enableIPMIdiv');
		}
	}

	function showchoose_users()
	{
		if (document.PowerMgmForm.elements[7].checked == true && 
			document.PowerMgmForm.enablePM.checked == true) {
			show ('document', 'choose_users');
		} else {
			hide ('document', 'choose_users');
		}
	}
	
	function checkOutletList(outlist)
	{
		var list = chgSeparator(outlist.value);

		if (continue_verification == true)
			validateListElements(list);

		if (continue_verification != true) {
			outlist.focus();
			outlist.select();
		}
	}

	function form_verification()
	{
		var form_location = document.PowerMgmForm;
		continue_verification = true;
		if (form_location.enablePM.checked == true) {
			if (form_location.pmKey.value != '') {
				checkHotKey (form_location.pmKey);
			}
			if (continue_verification == true && 
			    form_location.elements[7].checked == true) {
				if (document.PowerMgmForm.usersTable.length <= 1) {
					alert ('Please add Users.');
					continue_verification = false;
					document.PowerMgmForm.newUserGroup.focus();
				}				
			}	
			if (continue_verification == true)
				checkElement(form_location.pmoutlets, true, true, false, false);
			if (continue_verification == true)
				checkOutletList(form_location.pmoutlets);
			if (continue_verification != true)
				return;
		}
		if (form_location.enableIPMI.checked == true) {
			if (form_location.ipmiKey.value != '') {
				checkHotKey (form_location.ipmiKey);
			}
			if (form_location.ipmidevice.value == '-1') {
				alert('Please select one IPMI server');
			}
		}
	} 
	
	function checkHotKey (element_for_verification)
	{
		var chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"; // all roman alphabet letters
		if (element_for_verification.value.charAt(0) != '^' || 
		    element_for_verification.value.charAt(1) == '' || 
		    chars.indexOf(element_for_verification.value.charAt(1)) == -1) {
			continue_verification = false;
			alert ('Invalid hotkey. Format should be a caret (^) and a letter.');
			element_for_verification.focus();
			element_for_verification.select();
		} else 
			continue_verification = true;  
	}	
	
	function check_this()
	{ 
		var chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
		if (document.PowerMgmForm.elements[5].value.charAt(0) != '^' || 
		    chars.indexOf(document.PowerMgmForm.elements[5].value.charAt(1)) == -1) {
			return false;
		} else {
			return true;
		}		
	}
		 
/************Functions for Add allowed users and groups******************/      
	 function form_verification_add()
	 {	   
		checkElement (document.PowerMgmForm.newUserGroup, true, true, false, false);
		if (continue_verification == true) {
			if (document.PowerMgmForm.newUserGroup.value.indexOf(',') != -1) {
				alert ('Sorry, commas are not allowed.')
				continue_verification = false;
				document.PowerMgmForm.newUserGroup.focus();
				document.PowerMgmForm.newUserGroup.select();
			}
		}
		buildString(document.PowerMgmForm.usersTable);
		if (continue_verification == true && users_and_groups != '') {
			// check if list is too much big 
			if (users_and_groups.length+document.PowerMgmForm.newUserGroup.value.length+1 > 1024) {
				alert ('The List of Users is too big (greater than 1024 chars). You can not add more.')
				continue_verification = false;
				document.PowerMgmForm.newUserGroup.focus();
				document.PowerMgmForm.newUserGroup.select();
			} else {
				// check if you try to add user that is added already
				var users = users_and_groups.split(',');
				for (var i = 0; i < users.length; i++) {
					if (document.PowerMgmForm.newUserGroup.value == users[i]) {
						alert ('This user is already in list');
						continue_verification = false;
						document.PowerMgmForm.newUserGroup.focus();
						document.PowerMgmForm.newUserGroup.select();
						break;
					}
				}
			}
		}		
	}
	
	function deleteUser() {
		deleteSelOpt(document.PowerMgmForm.usersTable);
	} 

	function addUserGroup(object) {	 
		form_verification_add();
		if (continue_verification == true) {
			var defaultSelected = true;
			var selected = true;
			var newOption = new Option(object.newUserGroup.value.slice(0, 20), 
						   object.newUserGroup.value, 
						   defaultSelected, selected);
			var length = object.usersTable.length;
			object.usersTable.options[length-1] = newOption;
			var widthOption = new Option("                    ", -1, false, false);
			object.usersTable.options[length] = widthOption;
			object.newUserGroup.value='';
			object.newUserGroup.focus();
		}
	}
	  
	var users_and_groups;
	  
	function buildString (selectTable)
	{
		if (selectTable.length == 1 && selectTable.options[0].value == -1) {
			users_and_groups = '';
		} else {
			users_and_groups = selectTable.options[0].value;
			for (var i = 1; i < selectTable.length-1; i++) {
				users_and_groups = users_and_groups + ',' +selectTable.options[i].value
			}
		 }
	  }	 
/******End of Functions for Add allowed users***********/   		

	function submit()
	{
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
<body class="bodyMiniWizard" onload="showEnabled(); showIPMI();">
<form action="" name="PowerMgmForm" onSubmit="return false;">
<table width=80% height="45" border="0" align="center" cellpadding="0" cellspacing="0">	      
<tr valign="middle">
	<td width=60%>
	<% var check=getVal("system.ports.physP.powerMgm.enable");%>
	<input type="checkbox" name="enablePM" <% if (check==1) write("checked");%> onclick="showEnabled();">
	&nbsp;
	<script language="JavaScript" type="text/javascript">
	document.write('<font class='+enablePMlabel+'>Enable Power Management on this port</font>');
	</script>
    </td>
    <td width=40%>	 
	<% var checkI=getVal("system.ports.physP.powerMgm.enableIPMI");%>
	<input type="checkbox" name="enableIPMI" <% if (checkI==1) write("checked");%> onclick="showIPMI();">
	&nbsp;
	<script language="JavaScript" type="text/javascript">
	document.write('<font class="'+enableIPMIlabel+'">Enable IPMI on this port</font>');
	</script>
    </td>
</tr>
</table>   

<div id="enableIPMIdiv" style="display: block;  visibility: hidden;">
<table width=60% height="30" border="0" cellpadding="2" cellspacing="0" align="center">
	<tr valign="middle">				
	<td width=50% align="middle"> <font class="label">IPMI key</font></td>				
	<td align="left" valign="middle" height="50">
	<input type="text" name="ipmiKey" class="formText" size="2" maxlength="2" value="<%get("system.ports.physP.powerMgm.ipmiKey");%>"></td>
   	<td width="10">&nbsp;</td>
        <td  width=50% align="middle" valign="middle" height="50"><font class="label">IPMI Server</font></td>
	<td><select name="ipmidevice" size="1" class="formText">  
	&nbsp;&nbsp;&nbsp;&nbsp;
	<% get("system.ports.physP.powerMgm.ipmiDeviceListHtml");%>
	</select></td></tr>
</table>
</div>
<div id="enablePMdiv" style="position:relative; top:0px; width:100%; z-index:1; visibility: hidden;" align="center">
<table align="center" cellspacing="0" cellpadding="0" border="0">
<tr>
<td align="center" valign="middle" height="50"><font class="label">Outlets</font>
<input type="text" name="pmoutlets" class="formText" size="30" maxlength="50" value="<%get("system.ports.physP.powerMgm.pmOutlets");%>"></td>
</tr>
</table>
<table align="center" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td align="left" valign="middle" height="50" width="170"><font class="label">Power Management Key</font></td>
	<td align="left" valign="middle" height="50"><input type="text" name="pmKey" class="formText" size="2" maxlength="2" value="<%get("system.ports.physP.powerMgm.pmKey");%>"></td>
</tr>
<tr>
	<% var check=getVal("system.ports.physP.powerMgm.allUsers");%>
	<td align="left" valign="middle" height="25" colspan="2"><input type="radio" name="users" value="1" <% if (check==1) write("checked");%> onclick="showchoose_users();">
	&nbsp;&nbsp;&nbsp;&nbsp;<font class="label">Allow All Users</font></td>
</tr>
<tr>
	<td align="left" valign="top" height="30" colspan="2"><input type="radio" name="users" value="0" <% if (check==0) write("checked");%> onclick="showchoose_users();">
	&nbsp;&nbsp;&nbsp;&nbsp;<font class="label">Allow Users</font></td>
</tr>
</table>
</div>
<div id="choose_users" style="position:relative; top:0px; width:100%; z-index:1; visibility: hidden;" align="center">
<input type="hidden"  class="formText" name="userList" size="20" value="<% get("system.ports.physP.powerMgm.userList");%>">
<!-- my modif -->
<table width="460" border="0" align="center" cellpadding="0" cellspacing="5">
  <tr> 
    <td width="220" align="center" valign="middle">
<table border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td align="center" valign="middle"  height="30">
		  <font class="label">New User</font>
   </td>
 </tr>
 <tr>
   <td align="center" valign="middle">		  
		  <input class="formText" type="text" maxlength="30" name="newUserGroup" size="20">
    </td>
  </tr>
 </table>
 <table border="0" align="center" cellpadding="0" cellspacing="0">
   <tr>
	<td height="40" align="center" valign="middle">
	      <input type="image" src="/Images/addToButton.gif" onClick="addUserGroup(this.form);">
	</td>
   </tr>
</table>
<td width="220" align="center" valign="middle">
<table border="0" align="center" cellpadding="0" cellspacing="0">
        <tr  class="tableColor">
          <td align="left" valign="top">&nbsp;
		  <font class="tabsTextBig">Allowed Users</font></td>
        </tr>
        <tr>
          <td align="left" valign="top">
<!-- I used Pre WS for this class (NN and MO)-->
<select class="formPre" name="usersTable" size="5">
<script type="text/javascript">
	var added_users = document.PowerMgmForm.userList.value;
	if (added_users != '')
	{
		var users = added_users.split(',');
		for (var i = 0; i < users.length; i++)
			{
			document.write('<option value="'+users[i]+'" selected>'+users[i].slice(0, 20)+'</option>');	
			}
	}		
</script>		  
<option value="-1">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
</select> 
		  </td>
        </tr>
</table>
<table border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
          <td height="40" align="center" valign="middle">
		<a href="javascript:deleteUser();">
		<img src="/Images/deleteButton.gif" alt="" width="47" height="21" border="0"></a>  
		  </td>
        </tr>
</table>
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
   	<input type="hidden" name="system.ports.physP.powerMgm.enable" value=""> 
   	<input type="hidden" name="system.ports.physP.powerMgm.pmOutlets" value=""> 
   	<input type="hidden" name="system.ports.physP.powerMgm.pmKey" value=""> 
   	<input type="hidden" name="system.ports.physP.powerMgm.allUsers" value=""> 
   	<input type="hidden" name="system.ports.physP.powerMgm.userList" value=""> 
   	<input type="hidden" name="system.ports.physP.powerMgm.enableIPMI" value=""> 
   	<input type="hidden" name="system.ports.physP.powerMgm.ipmiKey" value=""> 
   	<input type="hidden" name="system.ports.physP.powerMgm.ipmiDeviceId" value=""> 
</form>
<script type="text/javascript">
top.mainReady = true;
</script>	
<%set("_sid_","0");%>
</body>
</html> 
