<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">


<% var user = getQueryVal("user", 0); %>

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
	
<% ReqNum = getRequestNumber(); %>
<% var ipsecurity = getVal("system.kvm.conf.station.ipsecurity"); %>
<% var tripledes = getVal("system.kvm.conf.station.des"); %>

	top.currentPage = self;
	top.helpPage = 'helpPages/generalConfigUsersHelp.asp';
  	parent.window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  
  	function init()
  	{
  		if (1 == top.menuReady && 1 == top.topMenuReady && 1 == top.controlsReady)
  			{
            if (checkLeftMenu('/normal/kvm/configKVMMenu.asp') == 0) {
               top.menuReady = 0;
               setTimeout('init()', 200);
               return;
            }
    		setUnsavedImage(top.controls.document, <%get("_changesLed_");%>);
            top.topMenu.selectItem(top.menu.topItem);
            top.menu.selectItem("GC");
  			}
  		else
    	setTimeout('init()', 200);
    }
	 
    function checkDupSeqs()
    {
        var form = document.stationForm;
        var seqs = new Array(
            form.quitseq.value.toLowerCase(),
            form.powerseq.value.toLowerCase(),
            form.srvsyncseq.value.toLowerCase(),
            form.videoconfseq.value.toLowerCase(),
            form.switchnextseq.value.toLowerCase(),
            form.switchprevseq.value.toLowerCase(),
            form.portinfoseq.value.toLowerCase());

        for(i = 0; i < seqs.length; i++) {
            for(j = 0; j < seqs.length; j++) {
                if(i != j) {
                    if(seqs[i] == seqs[j]) {
                        alert("duplicate key sequences");
                        continue_verification = false;
                        return;
                    }
                }
            }
        }
    }

	function form_verification ()
	{
		continue_verification = true;
		var form_location = document.stationForm;
				checkElement (form_location.idletimeout, false, false, false, true, 'no', 60);
			//element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old
			if (continue_verification == true && parent.tabs.user != 'ip')
				checkElement (form_location.screensavertime, false, false, false, true, 'no', 60);
//			if (continue_verification == true && parent.tabs.user != 'ip')
				checkElement (form_location.cycletime, false, false, false, true, 3, 60);
			if (continue_verification == true && parent.tabs.user != 'ip')
				checkSequence (form_location.quitseq, 1);
<%
var nIPstations = 0;
if (getVal("system.device.kvmanalog") == 0) {
    nIPstations = 2;
    if (getVal("system.kvm.numstations") > 4) {
        nIPstations = 4;
    }
}
%>
            if (continue_verification == true && parent.tabs.user == 'ip')
                checkRanges (form_location.viewerport, <%write(nIPstations);%>,
								4, 1024, 65535, true, true);
			if (continue_verification == true)
				checkSequence (form_location.powerseq, 1);
			if (continue_verification == true)
				checkSequence (form_location.srvsyncseq, 1);
			if (continue_verification == true)
				checkSequence (form_location.videoconfseq, 1);
			if (continue_verification == true)
				checkSequence (form_location.switchnextseq, 1);
			if (continue_verification == true)
				checkSequence (form_location.switchprevseq, 1);
			if (continue_verification == true)
				checkSequence (form_location.portinfoseq, 1);
			if (continue_verification == true)
				checkDupSeqs();
	}
	
	function submit()
	{		 
		form_verification();
		if (continue_verification == true)
		{
			
			document.configForm.elements[5].value = document.stationForm.idletimeout.value;
			if (parent.tabs.user != 'ip')
			{
				document.configForm.elements[6].value = document.stationForm.screensavertime.value;
				document.configForm.elements[7].value = document.stationForm.kbdtype.value;
				document.configForm.elements[8].value = document.stationForm.cycletime.value;
				document.configForm.elements[16].value = "<% get("system.kvm.conf.station.viewerport"); %>";
				document.configForm.elements[17].value = "<% write(ipsecurity); %>";
				document.configForm.elements[18].value = "<% write(tripledes); %>";
			} else {
				document.configForm.elements[6].value = <% get("system.kvm.conf.station.screensavertime", user); %>;
				document.configForm.elements[7].value = <% get("system.kvm.conf.station.kbdtype", user); %>;
				document.configForm.elements[8].value = document.stationForm.cycletime.value;
				document.configForm.elements[16].value = document.stationForm.viewerport.value;

				if (document.stationForm.ipsecurity[0].checked == true)
					document.configForm.elements[17].value = 0;
				else if (document.stationForm.ipsecurity[1].checked == true)
					document.configForm.elements[17].value = 1;
				else if (document.stationForm.ipsecurity[2].checked == true)
					document.configForm.elements[17].value = 2;

				if (document.stationForm.tripledes.checked == true)
					document.configForm.elements[18].value = 1;
				else
					document.configForm.elements[18].value = 0;
			}
			document.configForm.elements[9].value = document.stationForm.quitseq.value;
			document.configForm.elements[10].value = document.stationForm.powerseq.value;
			document.configForm.elements[11].value = document.stationForm.srvsyncseq.value;
			document.configForm.elements[12].value = document.stationForm.videoconfseq.value;
			document.configForm.elements[13].value = document.stationForm.switchnextseq.value;
			document.configForm.elements[14].value = document.stationForm.switchprevseq.value;
			document.configForm.elements[15].value = document.stationForm.portinfoseq.value;

			document.configForm.submit();
			top.mainReady = null;
		}
	} 

/* I use same page for User1, User2, and Web User
   You can know what user is configured by parent.tabs.user (gen; u1; u2; ip)
*/
	
</script>	 
</head>
<body class="bodyMiniWizard" onload="init();" >
<form name="stationForm" onSubmit="return false;">
<table align="center" cellspacing="0" cellpadding="0" border="0" width="100%" height="100%" >
<tr>
	<td valign="middle" align="center">
<script type="text/javascript">
if (parent.tabs.user == 'ip') {
	document.write('\
	<table class="bgColor7" align="center" width="460px" cellpadding="2" cellspacing="0" border="0"><tr><td> \
	<table class="bgColor1" width="460px" cellpadding="15" cellspacing="0" class="securityBox" border="0"><tr><td> \
Verify that your hosts and user workstations can reach the KVM WMI and TCP viewer ports using the very same IP address.<br>\
Firewall and proxies may need special configuration in order to meet this requirement.\
	</td></tr></table></td></tr></table>');
}
</script>
</td></tr>
<tr><td>
	<table align="center" cellspacing="0" cellpadding="3" border="0" colspan="4" valign="middle">
<script type="text/javascript">
// WEB User Version	
if (parent.tabs.user == 'ip') {
	document.write('\
	<tr>\
		<td align="left" colspan="4" valign="middle"><font class="label">Idle Timeout (min)</font>\
		<input name="idletimeout" type="text" size="3" maxlength="3" class="formText" value=<% get("system.kvm.conf.station.idletimeout", user); %>>\
		&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\
		<font class="label">Cycle Time (sec)</font>\
		<input name="cycletime" type="text" size="3" maxlength="3" class="formText" value=<% get("system.kvm.conf.station.cycletime", user); %>>\
		</td>\
	</tr>\
	<tr>\
		<td align="center" colspan="4" valign="middle"><font class="label">TCP Viewer Ports</font>\
		&nbsp;&nbsp;&nbsp;&nbsp;\
		<input name="viewerport" type="text" size="25" maxlength="24" class="formText" value="<% get("system.kvm.conf.station.viewerport"); %>">\
		</td>\
	</tr>');
	document.write('\
	<tr><td colspan="4" valign="middle" align="center" class="colorTHeader">\
		<table align="center" cellspacing="0" cellpadding="3" border="0" class="colorTHeader">\
			<tr>\
				<td><font class="tabsTextBig">IP security</font></td>\
			</tr>\
		</table>\
	</td>\
</tr>');
	document.write('\
	<tr><td colspan="4"><table align="center" cellspacing="0" cellpadding="0" border="0">\
	<tr><td valign="middle"><input type="radio" name="ipsecurity" value="0" \
		<% if (ipsecurity == 0) write("checked"); %> ></td>\
		<td><font class="label">Level 0 (No Encryption)</font></td>\
	</tr>\
	<tr><td><input type="radio" name="ipsecurity" value="1" \
		<% if (ipsecurity == 1) write("checked"); %> ></td>\
		<td><font class="label">Level 1 (Encrypt Keyboard and Mouse data)</font></td>\
	</tr>\
	<tr><td><input type="radio" name="ipsecurity" value="2" \
		<% if (ipsecurity == 2) write("checked"); %> ></td>\
		<td><font class="label">Level 2 (Encrypt Video, Keyboard and Mouse Data)</font></td>\
	</tr>\
	<tr>\
	<td valign="middle"><input type="checkbox" name="tripledes" \
		<% if (tripledes != 0) write("checked");%> ></td>\
	<td><font class="label">Use 3DES Encryption on Video Session</font></td>\
	</tr></table></td>\
</tr>');
} else {
    document.write('\
<tr>\
	<td align="left"><font class="label">Idle Timeout (min)</font></td>\
	<td><input name="idletimeout" type="text" size="3" maxlength="3" class="formText" value=<% get("system.kvm.conf.station.idletimeout", user); %>></td>\
	<td align="left"><font class="label">Screen Saver<br> Timeout (min)</font></td>\
	<td><input name="screensavertime" type="text" size="3" maxlength="3" class="formText" value=<% get("system.kvm.conf.station.screensavertime", user); %>></td>\
</tr>\
<tr>\
	<td align="left"><font class="label">Keyboard Type</font></td>\
	<td>\
	<% var selkbdtype=getVal("system.kvm.conf.station.kbdtype", user);%>\
	<select name="kbdtype" size="1" class="formText">\
		<option value="0" <% if (selkbdtype == "0") write("selected");%> >US</option>\
		<option value="1" <% if (selkbdtype == "1") write("selected");%> >BR-ABNT</option>\
		<option value="2" <% if (selkbdtype == "2") write("selected");%> >BR-ABNT2</option>\
		<option value="3" <% if (selkbdtype == "3") write("selected");%> >Japanese</option>\
		<option value="4" <% if (selkbdtype == "4") write("selected");%> >German</option>\
		<option value="5" <% if (selkbdtype == "5") write("selected");%> >Italian</option>\
		<option value="6" <% if (selkbdtype == "6") write("selected");%> >French</option>\
		<option value="7" <% if (selkbdtype == "7") write("selected");%> >Spanish</option>\
  	</select>\
	</td>\
	<td align="left"><font class="label">Cycle Time (sec)</font></td>\
	<td><input name="cycletime" type="text" size="3" maxlength="3" class="formText" value=<% get("system.kvm.conf.station.cycletime", user); %>></td>\
</tr>');
}

var quitseq = "<% get("system.kvm.conf.station.quitseq", user); %>";
var powerseq = "<% get("system.kvm.conf.station.powerseq", user); %>";
var srvsyncseq = "<% get("system.kvm.conf.station.srvsyncseq", user); %>";
var videoconfseq = "<% get("system.kvm.conf.station.videoconfseq", user); %>";
var switchnextseq = "<% get("system.kvm.conf.station.switchnextseq", user); %>";
var switchprevseq = "<% get("system.kvm.conf.station.switchprevseq", user); %>";
var portinfoseq = "<% get("system.kvm.conf.station.portinfoseq", user); %>";

var family_ext = "<% get('_familyext_'); %>";

if (parent.tabs.user != 'ip' || (family_ext != "KVMNETP" && family_ext != "ONS-V2"))
        document.write('\
	<tr><td colspan="4" valign="middle" align="center" class="colorTHeader">\
		<table align="center" cellspacing="0" cellpadding="3" border="0" class="colorTHeader">\
			<tr>\
				<td><font class="tabsTextBig">Escape Sequences</font></td>\
			</tr>\
		</table>\
	</td>\
</tr>\
<tr>\
	<td valign="middle" align="left"><font class="label">Quit</font></td>\
	<td valign="bottom"><input name="quitseq" type="text" size="1" maxlength="1" class="formText" value="' + quitseq + '"></td>\
	<td valign="middle" align="left"><font class="label">Power<br> Management</font></td>\
	<td valign="bottom"><input name="powerseq" type="text" size="1" maxlength="1" class="formText" value="' + powerseq + '"></td>\
</tr>\
<tr>\
	<td align="left"><font class="label">Mouse/Keyboard<br> Reset</font></td>\
	<td><input name="srvsyncseq" type="text" size="1" maxlength="1" class="formText" value="' + srvsyncseq + '"></td>\
	<td align="left"><font class="label">Video Control</font></td>\
	<td><input name="videoconfseq" type="text" size="1" maxlength="1" class="formText" value="' + videoconfseq + '"></td>\
</tr>\
<tr>\
	<td align="left"><font class="label">Switch Next</font></td>\
	<td><input name="switchnextseq" type="text" size="1" maxlength="1" class="formText" value="' + switchnextseq + '"></td>\
	<td align="left"><font class="label">Switch Previous</font></td>\
	<td><input name="switchprevseq" type="text" size="1" maxlength="1" class="formText" value="' + switchprevseq + '"></td>\
</tr>\
<tr>\
	<td align="left"><font class="label">Port Info</font></td>\
	<td><input name="portinfoseq" type="text" size="1" maxlength="1" class="formText" value="' + portinfoseq + '"></td>\
	<td></td>\
	<td></td>\
</tr>');
else {
        document.write('\
	<tr><input name="quitseq" type="hidden" value="' + quitseq + '">\
	<input name="powerseq" type="hidden" value="' + powerseq + '">\
	<input name="srvsyncseq" type="hidden" value="' + srvsyncseq + '">\
	<input name="videoconfseq" type="hidden" value="' + videoconfseq + '">\
	<input name="switchnextseq" type="hidden" value="' + switchnextseq + '">\
	<input name="switchprevseq" type="hidden" value="' + switchprevseq + '">\
	<input name="portinfoseq" type="hidden" value="' + portinfoseq + '"></tr>');
}
</script>

</table>
	</td>
</tr>
</table>
</form>
<form name="configForm" onsubmit="copyData();" method="POST" action="/goform/Dmf" target="_parent">
	<input type="hidden" name="system.req.action" value="0">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/kvm/configKVMGeneral.asp">
	<input type="hidden" name="urlError" value="/normal/kvm/configKVMGeneral.asp">
	<input type="hidden" name="request" value=<% write(ReqNum);%>>
	<input type="hidden" name=<% write("system.kvm.conf.station.idletimeout_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.screensavertime_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.kbdtype_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.cycletime_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.quitseq_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.powerseq_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.srvsyncseq_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.videoconfseq_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.switchnextseq_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.switchprevseq_"+user); %> value="">
	<input type="hidden" name=<% write("system.kvm.conf.station.portinfoseq_"+user); %> value="">
	<input type="hidden" name="system.kvm.conf.station.viewerport" value="">
    <input type="hidden" name="system.kvm.conf.station.ipsecurity" value="">
    <input type="hidden" name="system.kvm.conf.station.des" value="">
</form>
<script type="text/javascript">
top.mainReady = true;
</script>
<% set("_sid_","0");%>
</body>
</html>

