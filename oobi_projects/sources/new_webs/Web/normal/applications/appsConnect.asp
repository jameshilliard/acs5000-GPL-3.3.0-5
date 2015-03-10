<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

  <%
	 var hostname = getVal("system.device.hostname");
	 ReqNum = getRequestNumber();
     var family_model = getVal('_familymodel_');
  %>
  
  var hostName = "<%write(hostname);%>";

  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = '/normal/helpPages/appsConnectHelp.asp';
  
  function init()
  {
	if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
       if (checkLeftMenu('/normal/applications/appsMenu.asp') == 0) {
          parent.menuReady = 0;
          setTimeout('init()', 200);
          return;
       }
       document.connectForm.connect[1].checked = true;
       if (parent.access != "restricted") // for regular user
          setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
       parent.topMenu.selectItem(parent.menu.topItem);
       parent.menu.selectItem("CONS");
  	}
  	else
       setTimeout('init()', 200);
  }

  function connectSerialPort(form) {
    MenuIndex = form.serialPort.selectedIndex;
    PortName = form.serialPort.options[MenuIndex].text;
    PortNum = form.serialPort.options[MenuIndex].value;
    winArray = PortNum.split("/");
    Url = "appl/appsConnectPorts.asp?id=" +
          document.configForm.elements[1].value +
          "&port=" + winArray[0] + "&win=" + winArray[1] + "&name=" + PortName;
    if (winArray[1] == 1) {
      if(screen.width <= 800) {
        window.open(Url, "_blank","height=560,width=630,resizable=yes,scrollbars=yes");
      } else if (screen.width >= 1024) {
        window.open(Url, "_blank","height=740,width=630,resizable=yes,scrollbars=yes");
      }
    } else {
      window.open(Url, "_blank","height=520,width=610,resizable=yes");
    }
  }

  function connectKvmPort(form) {
    MenuIndex = form.kvmPort.selectedIndex;
    PortNum = form.kvmPort.options[MenuIndex].value;
	PortName = PortNum.split("#");

	if (PortName.length > 1 && PortName[1].length > 0) {
		Url = "/normal/applications/appl/accessConnectRDP.asp" +
		"?name=" + PortName[0] + "&connect=" + PortName[1] + 
		"&rc=appsConnect.asp";
	} else {
		Url = "/normal/applications/appl/accessConnectPorts.asp" +
				"?name=" + PortName + "&rc=appsConnect.asp";
	} 
    submitForm(self, Url)
 }

  function connectbox() {
     Url = "appl/appsConnectPorts.asp?id=" +
           document.configForm.elements[1].value +
           "&name=" + hostName;
     window.open(Url, "_blank","height=510,width=610,resizable=yes");
     return false;
  }
        
  function  selectRadius(form, val)
  {
     for (var i=0; i < form.connect.length; i++) {
        if (form.connect[i].value == val) form.connect[i].checked = true;
     }
  }

  function connection(form)
  {
     for (var i=0; i < form.connect.length; i++) {
        if (form.connect[i].checked) {
            if (form.connect[i].value == 2) {
		       return  connectKvmPort(form);
			}
            if (form.connect[i].value == 1) {
		       return  connectSerialPort(form);
			}
            if (form.connect[i].value == 0) {
               connectbox();
            }
        }
     }
  }

  function submit()
  {
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
   <input type="hidden" name="urlOk" value="/normal/applications/appsConnect.asp">
   <input type="hidden" name="urlError" value="/normal/applications/appsConnect.asp">
   <input type="hidden" name="request" value="0">
   <input type="hidden" name="system.tablePage.bogus" value="">
</form>    
    </td>
   </tr>
   <tr valign="middle">
      <td>
         <form name="connectForm" action="" onSubmit="return false;">
           <table border="0" cellpadding="0" cellspacing="0" align="center">
			<tr align="left">
               <td>
                  <input type="radio" name="connect" value="0">&nbsp;<font class="tableText">Connect to <%write(getVal("_boardname_"));%></font>
                  <p>
               </td>
            </tr>
<%
            if (family_model != "KVM") {
              write('<tr height="45" align="left"  valign="middle"><td>');
              write('<input type="radio" name="connect" value="1">&nbsp;<font class="tableText">Serial&nbsp;</font>');
              write('<select name="serialPort" class="formText" onChange="selectRadius(document.connectForm,1)">');
              write(get("system.ports.connectPortsHtml"));
              write('</select></td></tr>');
            }

            if ((family_model != "ACS")&&(family_model != "AVCS")) {
            write('<tr height="45" align="left"  valign="middle"><td>');
              write('<input type="radio" name="connect" value="2">&nbsp;<font class="tableText">KVM&nbsp;&nbsp;&nbsp;</font>');
              write('<select name="kvmPort" class="formText" onChange="selectRadius(document.connectForm,2)">');
              write(get("system.kvm.KVMconnectPortsHtml"));
              write('</select></td></tr>');
            }
%>
            <tr height="45" align="left"  valign="middle">
               <td>
                  <a href="javascript:connection(document.connectForm);">
                  <img src="/Images/connectButton.gif" alt="" width="58" height="21" border="0"></a>
               </td>
            </tr>
         </table>
<%
var family_ext = getVal('_familyext_');
if (family_ext == "ONS-V2") {
   write('<tr valign="bottom" align="right"><td>\n');
   write(' <a href="javascript:newWindow(\'/normal/applications/accessConnectPopUp.asp?tabx=0&SSID=' + getVal("_sid_") + '\', \'kvmcontrol\', 600, 400, \'yes\');">Show Connections</a>\n');
   write('</td></tr>\n');
}
%>
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
