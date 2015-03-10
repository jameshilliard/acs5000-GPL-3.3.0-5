<html>
<head>
<title>Set KVM Permissions for <%
        if (getVal("system.kvm.conf.acclist.currusergroup") == 1) {
		write("group ");
	} else {
		write("user ");
	}
	get("system.kvm.conf.acclist.currname");%></title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">

    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

    <% ReqNum = getRequestNumber(); %>

	function done()
	{
		if (document.kvmSetPermissions.elements[0].checked == true) {
			document.configForm.elements[5].value = "1";
		} else {
			document.configForm.elements[5].value = "0";
		}
		document.configForm.elements[6].value = opt[document.kvmSetPermissions.elements[1].selectedIndex];
		document.configForm.elements[7].value = 0;
		document.configForm.urlOk.value = '/normal/kvm/configKVMUsersGroups.asp';
		setQuerySSID(document.configForm);
	      	submit();
	} 	
	
	function setPerms()
	{
		document.configForm.elements[6].value = opt[document.kvmSetPermissions.elements[1].selectedIndex];
	} 	
	

	function cancel()
	{
		document.configForm.elements[7].value = 1;
	  	document.configForm.urlOk.value = '/normal/kvm/configKVMUsersGroups.asp';
		setQuerySSID(document.configForm);
	      	submit();
	}
	
	function modify ()
	{	
	
	if (document.kvmSetPermissions.elements[0].checked == false) {
		var table = document.kvmSetPermissions.access;
		if (table.selectedIndex == -1 || 
			table.options[table.selectedIndex].value == '-1') {
 			alert('Please select an item to modify.');
		} else {
			getSelectedEdit('configKVMSetPermissionsEntry.asp','device','450','280',target + '&defperm='+opt[document.kvmSetPermissions.elements[1].selectedIndex]+'&SSID=<%get("_sid_");%>',table,'yes');
		}
	}
	}
	
	function disableElements()
	{
	if (document.kvmSetPermissions.elements[0].checked == true) {
		document.kvmSetPermissions.elements[1].disabled = true;
		document.kvmSetPermissions.elements[2].disabled = true;
		defperm = defperm_defuser;
		perm = perm_defuser;
	} else {
		document.kvmSetPermissions.elements[1].disabled = false;
		document.kvmSetPermissions.elements[2].disabled = false;
		defperm = defperm_user;
		perm = perm_user;
	}
        if (defperm == 0) {
                document.kvmSetPermissions.elements[1].selectedIndex = 0;
        } else if (defperm == 1) {
                document.kvmSetPermissions.elements[1].selectedIndex = 1;
        } else if (defperm == 3) {
                document.kvmSetPermissions.elements[1].selectedIndex = 2;
        } else if (defperm == 7) {
                document.kvmSetPermissions.elements[1].selectedIndex = 3;
        } else if (defperm == 128) {
		 document.kvmSetPermissions.elements[1].selectedIndex = 4;
	}
	
	document.kvmSetPermissions.elements[2].length = perm.length + 1;
	for (var i = 0; i < perm.length; i ++) {
		document.kvmSetPermissions.elements[2].options[i+1].value = document.kvmSetPermissions.elements[2].options[i].value;
		document.kvmSetPermissions.elements[2].options[i+1].text = document.kvmSetPermissions.elements[2].options[i+1].text;
		document.kvmSetPermissions.elements[2].options[i].value = i;
		document.kvmSetPermissions.elements[2].options[i].text = perm[i];
	}
	document.kvmSetPermissions.elements[2].selectedIndex = perm.length;
    }
	
    function submit()
    {
	document.configForm.submit();
	   parent.mainReady = null;
    }	

    </script>
</head>
<body class="bodyForLayers">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr valign="middle">
	<td>
<form name="kvmSetPermissions" method="POST" action=" ">
<table align="center" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td align="center" height="30" valign="top">
	<font class="tabsTextBig">KVM Access List for <% 
        if (getVal("system.kvm.conf.acclist.currusergroup") == 1) {
		write("group ");
	} else {
		write("user ");
	}
	get("system.kvm.conf.acclist.currname");%></font></td>
</tr>

<tr>
  <td align="center" height="30" valign="top">
    <input type="checkbox" name="custom" value="" onclick="disableElements()">
  &nbsp;&nbsp;<font class="label">Default Access List</font></td>
</tr>
</table>

<table border="0" align="center" cellpadding="2" cellspacing="0">
  <tr>
    <td nowrap height="40" valign="top"><font class="label">Default Permission</font></td>
    <td nowrap height="40" valign="top">
      <select name="permission_port_def" size="1" class="formText" OnChange="setPerms()">
         <option value="0">No access</option>
         <option value="1">Read only</option>
         <option value="3">Read/Write</option>
         <option value="7">Full access</option>
	 <option value="128">Not defined</option>

      </select>
	</td>
	</tr>
</table>

<table border="0" align="center" cellpadding="0" cellspacing="0" class="tableColor" height="100">
<tr align="left" valign="bottom"> 
    <td>
	<table border="0" cellpadding="0" cellspacing="0">
       <tr align="left" class="tableColor">         		
          <td><font class="tabsTextBig">&nbsp;Device&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Permissions</font></td>
        </tr>
    </table>
	</td>
 </tr>
 <tr align="left">
 	<!-- option = 
	Device (1-20 characters)
	--> 
    <td align="left" valign="top">
	<select name="access" size="8" class="formText">
	<option value="-1" selected>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</option>
	</select>
	</td>
 </tr>
</table>
   	<table border="0" cellpadding="5" cellspacing="0" align="center">
	   	<tr> 
          	<td colspan="2" align="center" valign="bottom">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
	<td><a href="javascript:done();"><img src="/Images/OK_button.gif" alt="" width="35" height="21" border="0"></a></td>
	<td><a href="javascript:cancel();"><img src="/Images/cancelButton.gif" alt="" width="47" height="21" border="0"></a></td>
	<td><a href="javascript:modify();"><img src="/Images/setPermForDevice.gif" alt="" width="180" height="21" border="0"></a></td>
</tr>
</table>					        
			</td>
        </tr>
	</table>
</td>
</tr>
<tr>
	<td>
</form>   

<script type="text/javascript">
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    var target='target=' + window.name;
	var defperm_user = <%get("system.kvm.conf.acclist.user.defperm");%>;
	var defperm_defuser = <%get("system.kvm.conf.acclist.default.defperm");%>;
	var defuser = <%get("system.kvm.conf.acclist.currdefault");%>;
	var opt = new Array(0, 1, 3, 7, 128);
	var perm_user = new Array (<%get("system.kvm.conf.acclist.KVMPermHtml");%>);
	var perm_defuser = new Array (<%get("system.kvm.conf.acclist.KVMDefPermHtml");%>);
	var perm = new Array;
	var curruser = '<%get("system.kvm.conf.acclist.currname");%>';

	if (curruser == "Generic User"){
		perm = perm_defuser;
		defperm = defperm_defuser;
		document.kvmSetPermissions.elements[0].checked = false;
		document.kvmSetPermissions.elements[0].disabled = true;
		document.kvmSetPermissions.elements[1].disabled = false;
		document.kvmSetPermissions.elements[2].disabled = false;
	}else{
		if (defuser != 0) {
			perm = perm_defuser;
			defperm = defperm_defuser;
			document.kvmSetPermissions.elements[0].checked = true;
			document.kvmSetPermissions.elements[1].disabled = true;
			document.kvmSetPermissions.elements[2].disabled = true;
		} else {
			perm = perm_user;
			defperm = defperm_user;
			document.kvmSetPermissions.elements[0].checked = false;
			document.kvmSetPermissions.elements[1].disabled = false;
			document.kvmSetPermissions.elements[2].disabled = false;
		}
	}
        if (defperm == 0) {
                document.kvmSetPermissions.elements[1].selectedIndex = 0;
        } else if (defperm == 1) {
                document.kvmSetPermissions.elements[1].selectedIndex = 1;
        } else if (defperm == 3) {
                document.kvmSetPermissions.elements[1].selectedIndex = 2;
        } else if (defperm == 7) {
                document.kvmSetPermissions.elements[1].selectedIndex = 3;
        }
	else if (defperm == 128) {
                document.kvmSetPermissions.elements[1].selectedIndex = 4;
        }

	document.kvmSetPermissions.elements[2].length = perm.length + 1;
	for (var i = 0; i < perm.length; i ++) {
		document.kvmSetPermissions.elements[2].options[i+1].value = document.kvmSetPermissions.elements[2].options[i].value;
		document.kvmSetPermissions.elements[2].options[i+1].text = document.kvmSetPermissions.elements[2].options[i].text;
		document.kvmSetPermissions.elements[2].options[i].value = i;
		document.kvmSetPermissions.elements[2].options[i].text = perm[i];
	}
	document.kvmSetPermissions.elements[2].selectedIndex = perm.length;
</script>

	<form name="configForm" method="POST" action="/goform/Dmf">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/kvm/configKVMUsersGroups.asp">
	<input type="hidden" name="urlError" value="/normal/kvm/configKVMUsersGroups.asp">
	<input type="hidden" name="request" value=<%write(ReqNum);%>>   
	<input type="hidden" name="system.kvm.conf.acclist.currdefault" value="">   
	<input type="hidden" name="system.kvm.conf.acclist.user.defperm" value="">   
	<input type="hidden" name="system.kvm.conf.acclist.cancel" value="">   
</form>	
	</td>
</tr>
</table>	
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
