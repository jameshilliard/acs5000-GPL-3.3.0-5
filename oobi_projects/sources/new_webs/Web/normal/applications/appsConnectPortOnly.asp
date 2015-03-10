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
        if (parent.access == "restricted")// for regular user
    	  setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
    	parent.topMenu.selectItem(parent.menu.topItem);
    	parent.menu.selectItem("CONS");
  	} else {
    	setTimeout('init()', 200);
	}	

  }

  function connectport(url, form) {
    MenuIndex = form.port.selectedIndex;
    PortName = form.port.options[MenuIndex].text;
    PortNum = form.port.options[MenuIndex].value;
    winArray = PortNum.split("/");
    Url = url + "&port=" + winArray[0] + "&win=" + winArray[1] + "&name=" + PortName;
    if (winArray[1] == 1) {
      if(screen.width <= 800) {
        window.open(Url, "_blank","height=550,width=630,resizable=yes,scrollbars=yes");
      } else if (screen.width >= 1024) {
        window.open(Url, "_blank","height=730,width=630,resizable=yes,scrollbars=yes");
      }
    } else {
      window.open(Url, "_blank","height=510,width=610,resizable=yes");
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
<table border="0" cellpadding="0" cellspacing="0" align="center" height="100%">
   <tr valign="middle">
      <td>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/applications/appsConnectPortOnly.asp">
   <input type="hidden" name="urlError" value="/normal/applications/appsConnectPortOnly.asp">
   <input type="hidden" name="request" value="0">
   <input type="hidden" name="system.tablePage.bogus" value="">
</form>    
    </td>
   </tr>
   <tr valign="middle">
      <td>
         <form name="connectForm" action="" onSubmit="return false;">
           <table border="0" cellpadding="0" cellspacing="0" align="center">
            <tr height="45" align="left"  valign="middle">
               <td>
						<select name="port" class="formText">
							<% get("system.ports.connectPortsHtml"); %>
                  </select>
               </td>
            </tr>
            <tr height="45" align="center"  valign="middle">
               <td>
                  <a href="appl/appsConnectPorts.asp?id=<%get("_sid_");%>" onclick="connectport(this.href, document.connectForm);return false;">
                  <img src="/Images/connectButton.gif" alt="" width="58" height="21" border="0"></a>
               </td>
            </tr>
         </table>
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
