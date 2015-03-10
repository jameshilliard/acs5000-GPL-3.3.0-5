<html>
<head>
<title>Edit Outlet</title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/JavaScript">

	<% ReqNum = getRequestNumber(); %> 
window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

<%
	var Sname  = getQueryVal("name",-1);
	var Slock  = getQueryVal("lock",-1);
	var SminOn  = getQueryVal("minOn",-1);
	var SminOff  = getQueryVal("minOff",-1);
	var SpoDelay  = getQueryVal("poDelay",-1);
	var SwuState  = getQueryVal("wuState",-1);
	var SrealName  = getQueryVal("realName",-1);
	var Show_minOn  = getQueryVal("sminOn",-1);
	var Show_minOff  = getQueryVal("sminOff",-1);
	var Show_poDelay  = getQueryVal("spoDelay",-1);
	var Show_wuState  = getQueryVal("swuState",-1);
	var SisServerTech  = getQueryVal("isST",-1);
	var SpoffDelay  = getQueryVal("poffDelay",-1);
	var Show_poffDelay  = getQueryVal("spoffDelay",-1);
	var Sthre_hc = getQueryVal("thre_hc",-1);
	var Sthre_lc = getQueryVal("thre_lc",-1);
	var Sthre_hw = getQueryVal("thre_hw",-1);
	var Sthre_lw = getQueryVal("thre_lw",-1);
	var Show_threshold  = getQueryVal("sthre",-1);
%>

//for ServerTech, Post-On Delay is 0..900 sec
//for PM, 0..100 sec (0.1 multiples)
	function checkPoDelay(element_for_verification)
	{
		var interv = element_for_verification.value;
		var chars = '0123456789';
		var integer = interv.slice(0, interv.indexOf('.'));
		var fraction2 = interv.slice(interv.indexOf('.')+1, interv.length);
		var maxPODelay;
		if (<%write(SisServerTech);%>)
			maxPODelay = 900;
		else
			maxPODelay = 100;
		if (interv.indexOf('.') == -1)
		{
		integer = interv;
		fraction2 = 0;
		}
		if (<%write(SisServerTech);%> && fraction2 != 0) {
			continue_verification = false;
			alert('For ServerTech, Post On Delay must be an integer value.');
			element_for_verification.focus();
			element_for_verification.select();			
			return;
		}
   		for (var i = 0; i < integer.length; i++) 
   			{
      			if (chars.indexOf(integer.charAt(i)) == -1)
      			{
	  				continue_verification = false;
					alert('The Post On Delay you entered is not valid.');
   	  				element_for_verification.focus();
   	  				element_for_verification.select();
	  				break;
	  			}
   			}
		for (var i = 0; i < fraction2.length; i++) 
   			{
      			if (chars.indexOf(fraction2.charAt(i)) == -1 && continue_verification != false)
      			{
	  				continue_verification = false;
					alert('The Post On Delay you entered is not valid.');
   	  				element_for_verification.focus();
   	  				element_for_verification.select();
	  				break;
	  			}
   			}
		if (integer > maxPODelay || (integer == maxPODelay && fraction2 != 0))
			{
 					continue_verification = false;
					if (<%write(SisServerTech);%>)
						alert('For ServerTech, Maximum Post On Delay is '+maxPODelay+' seconds.');
					else
						alert('Maximum Post On Delay is '+maxPODelay+' seconds.');
   	  				element_for_verification.focus();
   	  				element_for_verification.select();			
			}							
	}	

	function form_verification()
	{
		var form_location = document.PMOutletsManEntryForm;
		continue_verification = true;
		var chars = '0123456789!$';
		
		if (<%write(SrealName);%>) {
			{checkElement (form_location.outlet_name, false, true, false, false);}
			if (continue_verification == true) {
				var name = form_location.outlet_name.value;
				if (validateNameOutIpdu(name,0) == false) {
	  				continue_verification = false;
					alert('Invalid Outlet Name.');
   	  				form_location.outlet_name.focus();
   	  				form_location.outlet_name.select();
					return;
				}
			}
		}
		if (<%write(Show_poDelay);%>) {
			if (continue_verification == true)
				{checkElement (form_location.post_on_delay, true, false, false, false);}
			if (continue_verification == true)
				{checkPoDelay(form_location.post_on_delay);}
		}
		if (<%write(Show_poffDelay);%>) {
			if (continue_verification == true)
				continue_verification = verify([
					[document.PMOutletsManEntryForm.post_off_delay,  
					"Fill,Bound", [0,2500]    ]
				]);
		}
		if (<%write(Show_threshold);%>) {
			minC = 0; maxC = 20;
			if (continue_verification == true) {
				continue_verification = verify([
					[document.PMOutletsManEntryForm.thre_lc,  
					"Fill,Bound", [minC, maxC]    ]
				]);
				minC = document.PMOutletsManEntryForm.thre_lc.value;
			}
			if (continue_verification == true) {
				continue_verification = verify([
					[document.PMOutletsManEntryForm.thre_lw,  
					"Fill,Bound", [minC, maxC]    ]
				]);
				minC = document.PMOutletsManEntryForm.thre_lw.value;
			}
			if (continue_verification == true) {
				continue_verification = verify([
					[document.PMOutletsManEntryForm.thre_hw,  
					"Fill,Bound", [minC, maxC]    ]
				]);
				minC = document.PMOutletsManEntryForm.thre_hw.value;
			}
			if (continue_verification == true) {
				if (minC == 0) minC = 2;
				continue_verification = verify([
					[document.PMOutletsManEntryForm.thre_hc,  
					"Fill,Bound", [minC, maxC]    ]
				]);
			}
		}
	}

	function sendOutletInfo()
	{
	     if (opener.parent.window.name != document.configForm.target) {
            alert("The page which called this form is not available anymore. The changes will not be effective.");
            self.close();
            return;
         }
		form_verification();
		if (continue_verification == true)		
		{
//			document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
			document.configForm.elements[5].value = 7; //Edit
			document.configForm.elements[6].value = opener.document.configForm.elements[6].value;
			document.configForm.elements[7].value = opener.document.configForm.elements[7].value;

			if (<%write(SrealName);%>) {
				if (document.PMOutletsManEntryForm.outlet_name.value)
					document.configForm.elements[6].value += " "+
						document.PMOutletsManEntryForm.outlet_name.value;
				else
					document.configForm.elements[6].value += " blankblank"; //name was erased
			} else {
				document.configForm.elements[6].value += " editedit"; //name cannot be changed
			}

		if (<%write(Show_minOn);%>)
			document.configForm.elements[6].value += " "+
				document.PMOutletsManEntryForm.min_on_time.value;
		else
			document.configForm.elements[6].value += " 0";
//-------------------------------------------------------------------------
		if (<%write(Show_minOff);%>)
			document.configForm.elements[6].value += " "+
				document.PMOutletsManEntryForm.min_off_time.value;
		else
			document.configForm.elements[6].value += " 0";
//-------------------------------------------------------------------------
		if (<%write(Show_poDelay);%>)
			document.configForm.elements[6].value += " "+
				document.PMOutletsManEntryForm.post_on_delay.value;
		else
			document.configForm.elements[6].value += " 0";
//-------------------------------------------------------------------------
		if (<%write(Show_wuState);%>)
			document.configForm.elements[6].value += " "+
				document.PMOutletsManEntryForm.wake_up_state.value;
		else
			document.configForm.elements[6].value += " 0";
//-------------------------------------------------------------------------
		if (<%write(Show_poffDelay);%>)
			document.configForm.elements[6].value += " "+
				document.PMOutletsManEntryForm.post_off_delay.value;
		else
			document.configForm.elements[6].value += " 0";
//-------------------------------------------------------------------------
		if (<%write(Show_threshold);%>) {
			document.configForm.elements[6].value += " "+
				document.PMOutletsManEntryForm.thre_hc.value +
				" " + document.PMOutletsManEntryForm.thre_hw.value +
				" " + document.PMOutletsManEntryForm.thre_lw.value +
				" " + document.PMOutletsManEntryForm.thre_lc.value ;
		} else
			document.configForm.elements[6].value += " 0 0 0 0";

			setQuerySSID(document.configForm);
			document.configForm.submit();
			top.mainReady = false;
			self.close();
		}	
	}

</script>
</head>

<body class="bodyMain">

<script type="text/javascript">
	if (<%write(Slock);%>) {
		alert('Outlet is locked, and some of its properties cannot be changed.');
		edit = 0;
		inputparam = ' disabled';
	} else {
		edit = 1;
		inputparam = '';
	}
</script>

<table align="center" border="0" cellspacing="5" cellpadding="0">
<tr valign="top"> 
	<td height="40" colspan="2" align="center">
		<a href="javascript:sendOutletInfo();">
		<img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
		&nbsp;&nbsp;&nbsp;&nbsp;
		<a href="javascript:window.close();">
		<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
	</td>
</tr>
<form name="PMOutletsManEntryForm" method="POST" action=" " onSubmit="return false;">
<tr>
	<td align="left" width="240"><font class="label">Outlet Name</font></td>
	<td align="left">
<script type="text/javascript">
	var sname = "<%write(Sname);%>";
		sname = sname.replace(/-nuMber-/, "#");
	if (<%write(SrealName);%>) { // write edit box when name is outlet name in PM
document.write(
		'<input class="formText" name="outlet_name" type="text" maxlength="16" size="17"',
			'value="'+sname+'">');
	} else { // write static text when name is server alias
document.write(
		'<font class="label">'+sname+'</font>');
	}
</script>
	</td>
</tr>
<script type="text/javascript">
if (<%write(Show_minOn);%>)
document.write(
	'<tr><td align="left"><font class="label">Min. On Time</font></td>',
	'<td align="left"><select name="min_on_time" size="1" class="formText"'+inputparam+'>',
		'<option value="0"    <%if (SminOn==0)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0s</option>',
		'<option value="15"   <%if (SminOn==1)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;15s</option>',
		'<option value="30"   <%if (SminOn==2)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;30s</option>',
		'<option value="45"   <%if (SminOn==3)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;45s</option>',
		'<option value="60"   <%if (SminOn==4)  write("selected");%>>&nbsp;1min</option>',
		'<option value="75"   <%if (SminOn==5)  write("selected");%>>&nbsp;1min15s</option>',
		'<option value="90"   <%if (SminOn==6)  write("selected");%>>&nbsp;1min30s</option>',
		'<option value="105"  <%if (SminOn==7)  write("selected");%>>&nbsp;1min45s</option>',
		'<option value="120"  <%if (SminOn==8)  write("selected");%>>&nbsp;2min</option>',
		'<option value="180"  <%if (SminOn==9)  write("selected");%>>&nbsp;3min</option>',
		'<option value="240"  <%if (SminOn==10) write("selected");%>>&nbsp;4min</option>',
		'<option value="300"  <%if (SminOn==11) write("selected");%>>&nbsp;5min</option>',
		'<option value="600"  <%if (SminOn==12) write("selected");%>>10min</option>',
		'<option value="900"  <%if (SminOn==13) write("selected");%>>15min</option>',
		'<option value="1800" <%if (SminOn==14) write("selected");%>>30min</option>',
		'<option value="3600" <%if (SminOn==15) write("selected");%>>&nbsp;1h</option>',
		'</select>',
	'</td></tr>');
if (<%write(Show_minOff);%>)
document.write(
	'<tr><td align="left"><font class="label">Min. Off Time</font></td>',
	'<td align="left"><select name="min_off_time" size="1" class="formText"'+inputparam+'>',
		'<option value="0"    <%if (SminOff==0)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;0s</option>',
		'<option value="15"   <%if (SminOff==1)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;15s</option>',
		'<option value="30"   <%if (SminOff==2)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;30s</option>',
		'<option value="45"   <%if (SminOff==3)  write("selected");%>>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;45s</option>',
		'<option value="60"   <%if (SminOff==4)  write("selected");%>>&nbsp;1min</option>',
		'<option value="75"   <%if (SminOff==5)  write("selected");%>>&nbsp;1min15s</option>',
		'<option value="90"   <%if (SminOff==6)  write("selected");%>>&nbsp;1min30s</option>',
		'<option value="105"  <%if (SminOff==7)  write("selected");%>>&nbsp;1min45s</option>',
		'<option value="120"  <%if (SminOff==8)  write("selected");%>>&nbsp;2min</option>',
		'<option value="180"  <%if (SminOff==9)  write("selected");%>>&nbsp;3min</option>',
		'<option value="240"  <%if (SminOff==10) write("selected");%>>&nbsp;4min</option>',
		'<option value="300"  <%if (SminOff==11) write("selected");%>>&nbsp;5min</option>',
		'<option value="600"  <%if (SminOff==12) write("selected");%>>10min</option>',
		'<option value="900"  <%if (SminOff==13) write("selected");%>>15min</option>',
		'<option value="1800" <%if (SminOff==14) write("selected");%>>30min</option>',
		'<option value="3600" <%if (SminOff==15) write("selected");%>>&nbsp;1h</option>',
		'</select>',
	'</td></tr>');
if (<%write(Show_poDelay);%>)
document.write(
	'<tr><td align="left"><font class="label">Post On Delay</font></td>',
	'<td align="left"><input class="formText" name="post_on_delay" type="text" size="8" maxlength="5"',
		'value="<%write(SpoDelay);%>"'+inputparam+'><font class="label">&nbsp;(sec)</font></td></tr>');
if (<%write(Show_poffDelay);%>)
document.write(
	'<tr><td align="left"><font class="label">Post Off Delay</font></td>',
	'<td align="left"><input class="formText" name="post_off_delay" type="text" size="8" maxlength="5"',
		'value="<%write(SpoffDelay);%>"'+inputparam+'><font class="label">&nbsp;(sec)</font></td></tr>');
if (<%write(Show_wuState);%>)
document.write(
	'<tr><td align="left"><font class="label">Wake State</font></td>',
	'<td align="left"><select name="wake_up_state" size="1" class="formText"'+inputparam+'>',
		'<option value="0" <%if (SwuState==0) write("selected");%>>Off</option>',
		'<option value="1" <%if (SwuState==1) write("selected");%>>On</option>',
		'<%if (SisServerTech==1) {
			write("<option value=\"2\"");
				if (SwuState==2) write("selected");
			write(">Last</option>");
		}%>',
		'</select>',
	'</td></tr>');
if (<%write(Show_threshold);%>) {
document.write(
	'<tr><td align="left"><font class="label">Current High Critical Threshold</font></td>',
	'<td align="left"><input class="formText" name="thre_hc" type="text" size="10" maxlength="8"',
		'value="<%write(Sthre_hc);%>"'+inputparam+'><font class="label">&nbsp;(A)</font></td></tr>');
document.write(
	'<tr><td align="left"><font class="label">Current High Warning Threshold</font></td>',
	'<td align="left"><input class="formText" name="thre_hw" type="text" size="10" maxlength="8"',
		'value="<%write(Sthre_hw);%>"'+inputparam+'><font class="label">&nbsp;(A)</font></td></tr>');
document.write(
	'<tr><td align="left"><font class="label">Current Low Warning Threshold</font></td>',
	'<td align="left"><input class="formText" name="thre_lw" type="text" size="10" maxlength="8"',
		'value="<%write(Sthre_lw);%>"'+inputparam+'><font class="label">&nbsp;(A)</font></td></tr>');
document.write(
	'<tr><td align="left"><font class="label">Current Low Critical Threshold)</font></td>',
	'<td align="left"><input class="formText" name="thre_lc" type="text" size="10" maxlength="8"',
		'value="<%write(Sthre_lc);%>"'+inputparam+'><font class="label">&nbsp;(A)</font></td></tr>');
}
</script>
</tr>
</form>
</table>
<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
    <input type="hidden" name="system.req.action" value="">
    <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
    <input type="hidden" name="urlOk" value="/normal/applications/appsPMOutletsMan.asp">
    <input type="hidden" name="urlError" value="/normal/applications/appsPMOutletsMan.asp">
    <input type="hidden" name="request" value="0">
    <!--*******************************************************************-->
	<input type="hidden" name="system.applications.IpduOutMan.action" value="">
	<input type="hidden" name="system.applications.IpduOutMan.sysMsg" value="">
	<input type="hidden" name="system.applications.IpduOutMan.memSignature" value="">
</form>
</body>
</html>
