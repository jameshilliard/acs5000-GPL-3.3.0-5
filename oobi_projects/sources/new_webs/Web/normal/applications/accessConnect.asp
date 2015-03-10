<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<% var connect = getQueryVal("connect", 0);%>

<html>
<head>
<title>Untitled</title>
<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script type="text/javascript">

	<% ReqNum = getRequestNumber(); %>
  
	parent.currentPage = self;
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	top.helpPage = "/normal/helpPages/appsConnectHelp.asp";
 
	function init()
	{
		if (1 == parent.menuReady && 1 == parent.topMenuReady &&
				1 == parent.controlsReady) {
			if (checkLeftMenu('/normal/applications/appsMenu.asp') == 0) {
				parent.menuReady = 0;
				setTimeout('init()', 200);
				return;
			}
			if (parent.access != "restricted") // for regular user
			setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
			parent.topMenu.selectItem(parent.menu.topItem);
			parent.menu.selectItem("CONK");	
		} else {
			setTimeout('init()', 200);
		}
	}

	function connect() {
		MenuIndex = document.connectForm.port.selectedIndex;
		PortNum = document.connectForm.port.options[MenuIndex].value;

		PortName = PortNum.split("#");
		if (PortName.length > 1 && PortName[1].length > 0) {
			Url = "/normal/applications/appl/accessConnectRDP.asp" +
			"?name=" + PortName[0] + "&connect=" + PortName[1] + 
			"&rc=accessConnect.asp";
		} else {
			Url = "/normal/applications/appl/accessConnectPorts.asp" +
					"?name=" + PortName + "&rc=accessConnect.asp";
		} 

		submitForm(self, Url);
	}

	function submit()
	{
		//document.configForm.elements[5].value = document.connectForm.elements[0].value;
		document.configForm.submit();
		parent.mainReady = null;
	}
</script>

</head>
<body onload="init();" class="bodyMain">
<table border="0" cellpadding="0" cellspacing="0" align="center" height="100%" width="100%">
   <tr valign="middle">
      <td>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/applications/accessConnect.asp">
   <input type="hidden" name="urlError" value="/normal/applications/accessConnect.asp">
   <input type="hidden" name="request" value="0">
   <input type="hidden" name="system.tablePage.bogus" value="">
   <!--<input type="hidden" name="system.kvm.connport" value="">-->
</form>    
    </td>
   </tr>
   <tr valign="middle">
      <td>
         <form name="connectForm" action="">
           <table border="0" cellpadding="0" cellspacing="0" align="center">
            <tr height="45" align="left"  valign="middle">
               <td>
                  <select name="port" class="formText">
<% get("system.kvm.KVMconnectPortsHtml"); %>
                  </select>   
               </td>
            </tr>
            <tr height="45" align="center"  valign="middle">
               <td>
                  <a href="javascript:connect();">
				  <img src="/Images/connectButton.gif" alt="" width="58" height="21" border="0"></a>
               </td>
            </tr>
         </table>
     </form>  

      </td>
   </tr>
<%
var family_ext = getVal('_familyext_');
if (family_ext == "KVMNETP") {
   write('<tr valign="bottom" align="right"><td>\n');
   write(' <a href="javascript:newWindow(\'/normal/applications/accessConnectPopUp.asp?tabx=0&SSID=' + getVal("_sid_") + '\', \'kvmcontrol\', 600, 400, \'yes\');">Show Connections</a>\n');
   write('</td></tr>\n');
}
%>
</table>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
