<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>New/Modify Connection</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">

  <%
     ReqNum = getRequestNumber();
     set ("_sel1_" , getQueryVal("vpnIndex",0));
  %>

    window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
    function showSelectedLayer(object)
    {    
        if (object.options[object.selectedIndex].value == 0)
         {
            show('document','rsaRight');
            show('document','rsaLeft');
			hide('document','secret');
         }
        else if (object.options[object.selectedIndex].value == 1)      
         {      
            hide('document','rsaRight');
            hide('document','rsaLeft');
			show('document','secret');
         }
    }       
    
    function copyData() 
    {        
		var my_form;

		if (type == 'NN') 
			my_form = document.Remote.document.RemoteForm;
		else
			my_form = document.RemoteForm;

		document.configForm.elements[5].value = eval(my_form).connectionName.value;
		document.configForm.elements[6].value = eval(my_form).authProtocol.selectedIndex;
		document.configForm.elements[7].value = eval(my_form).authMethod.selectedIndex;
        document.configForm.elements[8].value = eval(my_form).idRht.value;
        document.configForm.elements[9].value = eval(my_form).ipAddressRht.value;
        document.configForm.elements[10].value = eval(my_form).nextHopRht.value;
        document.configForm.elements[11].value = eval(my_form).subnetMaskRht.value;

		if (type == 'NN') 
			my_form = document.rsaRight.document.keyRht;
		else
			my_form = document.keyRht;

		document.configForm.elements[12].value = eval(my_form).rsaKeyRht.value;

		if (type == 'NN') 
			my_form = document.local.document.localForm;
		else
			my_form = document.localForm;

		document.configForm.elements[13].value = eval(my_form).idLft.value;
		document.configForm.elements[14].value = eval(my_form).ipAddressLft.value;
		document.configForm.elements[15].value = eval(my_form).nextHopLft.value;
		document.configForm.elements[16].value = eval(my_form).subnetMaskLft.value;

		if (type == 'NN') 
			my_form = document.rsaLeft.document.keyLft;
		else
			my_form = document.keyLft;

		document.configForm.elements[17].value = eval(my_form).rsaKeyLft.value;

		if (type == 'NN')
			my_form = document.boot.document.bootForm;
		else
			my_form = document.bootForm;

		document.configForm.elements[18].value = eval(my_form).bootAction.selectedIndex;
		document.configForm.elements[19].value = eval(my_form).sharedSecret.value;
		document.configForm.elements[20].value = opener.document.configForm.elements[5].value;
    }     
    
	function init( )
	{
		var my_form;

		if (type == 'NN')
			my_form = document.Remote.document.RemoteForm;
		else
			my_form = document.RemoteForm;

		eval(my_form).authProtocol.selectedIndex = document.configForm.elements[6].value;
		eval(my_form).authMethod.selectedIndex = document.configForm.elements[7].value;

		showSelectedLayer(eval(my_form).authMethod);

		if (type == 'NN')
			my_form = document.boot.document.bootForm;
		else
			my_form = document.bootForm;

		eval(my_form).bootAction.selectedIndex = document.configForm.elements[18].value;
	}

	function form_verification()
	{
	var form_remote
	var form_remote_rsa
	var form_local
	var form_local_rsa
	var form_boot
	if (type == 'NN') 
	  		{
			form_remote = document.Remote.document.RemoteForm;
			form_remote_rsa = document.rsaRight.document.keyRht;
			form_local = document.local.document.localForm;
			form_local_rsa = document.rsaLeft.document.keyLft;
			form_boot = document.boot.document.bootForm;
			}
	  		else 
	  		{
			form_remote = document.RemoteForm;
			form_remote_rsa = document.keyRht;
			form_local = document.localForm;
			form_local_rsa = document.keyLft;
			form_boot = document.bootForm;
			}
	checkElement (form_remote.elements[0], true, true, false, false);
	if (continue_verification == true)
	{checkElement (form_remote.elements[3], true, false, false, false);}
	if (continue_verification == true)
	{checkElementID (form_remote.elements[3]);}
	if (continue_verification == true)
	{checkElement (form_remote.elements[4], true, false, true, false);}
	if (continue_verification == true)
	{checkElement (form_remote.elements[5], false, false, true, false);}
	if (continue_verification == true && form_remote.elements[6].value != '')
	{checkBitMask (form_remote.elements[6]);}
	if (continue_verification == true && form_remote.elements[2].value == 0)
	{checkElement (form_remote_rsa.elements[0], true, true, false, false);}
	if (continue_verification == true && form_remote.elements[2].value == 0)
	{checkMaxLength (form_remote_rsa.elements[0], 255)}
	//
	if (continue_verification == true)
	{checkElement (form_local.elements[0], true, false, false, false);}
	if (continue_verification == true)
	{checkElementID (form_local.elements[0]);}
	if (continue_verification == true)
	{checkElement (form_local.elements[1], true, false, true, false);}
	if (continue_verification == true)
	{checkElement (form_local.elements[2], false, false, true, false);}
	if (continue_verification == true && form_local.elements[3].value != '')
	{checkBitMask (form_local.elements[3]);}
	if (continue_verification == true && form_remote.elements[2].value == 0)
	{checkElement (form_local_rsa.elements[0], true, true, false, false);}
	if (continue_verification == true && form_remote.elements[2].value == 0)
	{checkMaxLength (form_local_rsa.elements[0], 255)}
	if (form_remote.authMethod.value == 1 && continue_verification == true)
	{checkElement (form_boot.elements[1], false, true, false, false);}
					
	}
	
	function checkElementID (elem) 
	{
	if (elem.value.charAt(0) == '@')
	{
	for (var i=0; i<elem.value.length; i++)
		{
		if (elem.value.charAt(i)== ' ')
		{
		alert("Sorry, spaces are not allowed.");
		continue_verification = false;
		elem.focus();
		elem.select();
		break;}
	}}	
	else
		{if (!validateIP(elem.value, false, false)) 
			{
			alert('The ID you entered is not valid.');
			continue_verification = false;
			elem.focus();
			elem.select();
		}
		else
			{continue_verification = true;}
    }
	}
	
	function submit( )
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
		document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
        setQuerySSID(document.configForm);
		document.configForm.submit();
		self.close();
		}
	  }
	  catch (error) {
		self.close();
	  }
	}
 
    </script>
</head>

<body onload="init();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/network/netVPNConnect.asp">
   <input type="hidden" name="urlError" value="/normal/network/netVPNConnect.asp">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">

   <!--hidden inputs used to store values of form elements inside layers-->           
   <input type="hidden" name="system.network.VPNEntry.connectionName" value="">
   <input type="hidden" name="system.network.VPNEntry.authProtocol"  value="<%get("system.network.VPNEntry.authProtocol");%>">
   <input type="hidden" name="system.network.VPNEntry.authMethod" value="<%get("system.network.VPNEntry.authMethod");%>">
   <input type="hidden" name="system.network.VPNEntry.idRht" value="">
   <input type="hidden" name="system.network.VPNEntry.ipAddressRht" value="">
   <input type="hidden" name="system.network.VPNEntry.nextHopRht" value="">
   <input type="hidden" name="system.network.VPNEntry.subnetMaskRht" value="">
   <input type="hidden" name="system.network.VPNEntry.rsaKeyRht" value="">
   <input type="hidden" name="system.network.VPNEntry.idLft" value="">
   <input type="hidden" name="system.network.VPNEntry.ipAddressLft" value="">
   <input type="hidden" name="system.network.VPNEntry.nextHopLft" value="">
   <input type="hidden" name="system.network.VPNEntry.subnetMaskLft" value="">
   <input type="hidden" name="system.network.VPNEntry.rsaKeyLft" value="">
   <input type="hidden" name="system.network.VPNEntry.bootAction" value="<%get("system.network.VPNEntry.bootAction");%>">
   <input type="hidden" name="system.network.VPNEntry.sharedSecret" value="">
   <input type="hidden" name="system.network.VPNEntry.VPNTableDel" value="">
   <!--*************************************************************************-->
<table border="0" cellpadding="0" cellspacing="0" align="center">
	<tr>
        <td align="center" height="40" valign="middle">
		<a href="javascript:submit();">
		<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
        &nbsp;&nbsp;&nbsp;&nbsp;
        <a href="javascript:window.close();">
		<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
    </tr>
</table>   
</form>
<div id="Remote" class="visible">
   <form name="RemoteForm"  onSubmit="return false;">
<table border="0" cellpadding="2" cellspacing="0" width="500" align="center">	
   <tr>
      <td  colspan="4" align="center" height="50" valign="middle">
      <font class="label">Connection Name</font>
      &nbsp;&nbsp;&nbsp;&nbsp;
      <input type="text" maxlength="19" class="formText" name="connectionName" size="15" value="<%get("system.network.VPNEntry.connectionName");%>"> 
      </td>     
   </tr>
   <tr>
      <td >
         <font class="label">Authentication <br>Protocol</font>
      </td>
      <td>
         <select name="authProtocol"  class="formText">
         	<option value="0" selected> ESP </option>
         	<option value="1"> AH </option>
         </select>        
      </td>
      <td >
         <font class="label">Authentication <br>Method</font>
      </td>
      <td>
         <select name="authMethod" class="formText"
            onChange="showSelectedLayer(this)">
         	<option value="0" SELECTED> RSA Public Keys </option>
         	<option value="1"> Shared Secret </option>
         </select>
      </td>
   </tr>
</table>
<table border="0" cellpadding="2" cellspacing="0" width="500" align="center">	
   <tr>
       <td colspan="2" height="40" valign="middle">         
         <font class="tabsTextBig">Remote ("Right")</font>
      </td>
   </tr>
   <tr>
      <td>
         <font class="label">ID</font>
      </td>
      <td>
         <input class="formText" maxlength="39" type="text" name="idRht" size="15" value="<%get("system.network.VPNEntry.idRht");%>">
      </td>
      <td>
         <font class="label">IP Address</font>
      </td>
      <td>
         <input class="formText" maxlength="15" type="text" name="ipAddressRht" size="15" value="<%get("system.network.VPNEntry.ipAddressRht");%>">
      </td>
   </tr>
   <tr>
      <td>
         <font class="label">NextHop</font>
      </td>
      <td>
         <input class="formText" maxlength="15" type="text" name="nextHopRht" size="15" value="<%get("system.network.VPNEntry.nextHopRht");%>">
      </td>      
      <td>
         <font class="label"><%getLabel("system.netSettings.subnet");%></font>
      </td>
      <td>
         <input class="formText"  maxlength="18" type="text" name="subnetMaskRht" size="18" value="<%get("system.network.VPNEntry.subnetMaskRht");%>">
      </td>      
   </tr>
</table>
</form>
<div id="rsaRight" class="visible">
   <form name="keyRht"  onSubmit="return false;">
      <table border="0" cellpadding="2" cellspacing="0" width="500" align="center">	
         <tr>     
            <td valign="top">
               <font class="label">RSA Key</font>
            </td>
            <td>
               <textarea wrap=soft cols="43" rows="2" id="rsaKeyRht"  class="formText"><%get("system.network.VPNEntry.rsaKeyRht");%></textarea>
            </td>
         </tr>
      </table>
   </form>
</div>
<div id="local">
<form name="localForm"  onSubmit="return false;">
<table border="0" cellpadding="2" cellspacing="0" width="500" align="center">	
   <tr>
      <td colspan="2" height="40" valign="middle">
         <font class="tabsTextBig">Local ("Left")</font>
      </td>
   </tr>
   <tr>
      <td>
         <font class="label">ID</font>
      </td>
      <td>
         <input class="formText" maxlength="39" type="text" name="idLft" size="15" value="<%get("system.network.VPNEntry.idLft");%>">
      </td>
      <td>
         <font class="label">IP Address</font>
      </td>
      <td>
         <input class="formText"  maxlength="15" type="text" name="ipAddressLft" size="15" value="<%get("system.network.VPNEntry.ipAddressLft");%>">
      </td>
   </tr>
   <tr>
      <td>
         <font class="label">NextHop</font>
      </td>
      <td>
         <input class="formText" maxlength="15" type="text" name="nextHopLft" size="15" value="<%get("system.network.VPNEntry.nextHopLft");%>">
      </td>      
      <td>
         <font class="label"><%getLabel("system.netSettings.subnet");%></font>
      </td>
      <td>
         <input class="formText" maxlength="18" type="text" name="subnetMaskLft" size="18" value="<%get("system.network.VPNEntry.subnetMaskLft");%>">
      </td>      
   </tr>
</table>
</form>
<div id="rsaLeft" class="visible">
   <form name="keyLft"  onSubmit="return false;">
      <table border="0" cellpadding="2" cellspacing="0" width="500" align="center">	
         <tr>     
            <td valign="top">
               <font class="label">RSA Key</font>
            </td>
            <td>
               <textarea wrap=soft cols="43" rows="2" id="rsaKeyLft"  class="formText"><%get("system.network.VPNEntry.rsaKeyLft");%></textarea>
            </td>
         </tr>
      </table>
   </form>
</div>
<div id="boot">
<form name="bootForm" action="" onSubmit="return false;">
   <table border="0" cellpadding="2" cellspacing="0" width="500" align="center">	
      <tr>
         <td align="center" height="40" valign="top">
            <font class="label">Boot Action</font>
			&nbsp;&nbsp;&nbsp;
            <select name="bootAction" class="formText">

            	<option value="1" SELECTED>Ignore</option>
            	<option value="2">Add</option>
            	<option value="3">Start</option>
            </select>
         </td>
		 </tr>
 </table>
 <div id="secret">
    <table border="0" cellpadding="2" cellspacing="0" width="500" align="center">	
      <tr>       
		 <td align="center" height="40" valign="top">
            <font class="label">Pre Shared Secret</font>
			&nbsp;&nbsp;&nbsp;
            <input class="formText" maxlength="30" type="password" name="sharedSecret" size="15" value="<%get("system.network.VPNEntry.sharedSecret");%>">
         </td>
      </tr>
   </table>		 
 </div>		 
</form>
</div>
</body>
</html>
