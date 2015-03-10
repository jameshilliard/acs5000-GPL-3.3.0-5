<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script language="JavaScript" src="/menu.js" type="text/javascript"></script>
   <script language="JavaScript">

   if ((parent.family_model == "ACS" ) || (parent.family_model == "AVCS" ))
      var topItem = "PORT";
   else
      var topItem = "CON";

   var i = 0;
   var menuPages = new Array;

   menuPages[i++] = new menuObj("PHYS", "Physical Ports", "/normal/ports/physPorts.asp");
   if ((parent.family_model == "ACS" ) || (parent.family_model == "AVCS" ))
   {
     menuPages[i++] = new menuObj("VIRT", "Virtual Ports", "/normal/ports/virtPorts.asp");
     menuPages[i++] = new menuObj("ST", "Ports Status", "/normal/ports/portsStatus.asp");
     menuPages[i++] = new menuObj("PS", "Ports Statistics", "/normal/ports/portsStatistics.asp");
   } else {
     menuPages[i++] = new menuObj("AUX", "Aux/Modem Port", "/normal/auxport/configAuxPort1.asp");
     menuPages[i++] = new menuObj("NTF", "Notifications", "/normal/administration/adminNotifications.asp");
   }
   menuPages[i++] = new menuObj("AD", "Hostname Discovery", "/normal/ports/portsDiscovery.asp");

   function init()
   {
      parent.menuReady = 1;
   }

   var pageName = "/normal/ports/portsMenu.asp";
    
   </script> 

   <script language="JavaScript" src="/menuNav.js" type="text/javascript"></script>

</head>

<body onload="init();" class="bgColor11" bottommargin="0" topmargin="0" leftmargin="0" rightmargin="0" marginheight="0" marginwidth="0">

<table border="0" cellpadding="0" cellspacing="0" width="204" height="100%">    
    <tr>
        <td class="bgColor8">   
            <table border="0" cellpadding="0" cellspacing="0" width="16" height="100%">
                <tr>
                    <td background="/Images/leftEdge.gif" height="100%"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
                </tr>
            </table>
        </td>
        
        <td class="bgColor8" valign="top">
            <table border="0" cellpadding="0" cellspacing="0" width="163" height="100%">
                <tr>
                    <td class="bgColor8" align="center" valign="bottom"><img src="/Images/menuTop.gif" alt="" width="163" height="4" border="0"></td>
                </tr>
                <tr>
                    <td background="/Images/menuBackground.gif" height="100%" width="100%" valign="top">
                        &nbsp;
                    </td>                    
                </tr>
            </table>
        </td>
        <td class="bgColor8" width="25">
            &nbsp;
        </td>
    </tr>
</table>     

<script language="JavaScript">
<!-- Begin Hide
  print_head_menu(20, 2, menuPages, "PORT");
// End Hide -->
</script>

</body>
</html>
