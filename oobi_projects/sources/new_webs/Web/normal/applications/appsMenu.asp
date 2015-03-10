<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script language="JavaScript" src="/menu.js" type="text/javascript"></script>
   <script language="JavaScript">

   function init()
   {
      parent.menuReady = 1;
   }

   var Local = <%write(getVal("system.regUser.authLocal"));%>;
   var pageName = "/normal/applications/appsMenu.asp";
   var menuPages = new Array;
   var i = 0;

   if ((parent.family_model != "ACS")&&(parent.family_model != "AVCS"))
      var topItem = "ACC";
   else
      var topItem = "APP";

   if (parent.family_model == "ONS" ) {
      if (parent.access == "easy" || parent.ConnectBox == 1) {
         menuPages[i++] = new menuObj("CONS", "Connect to Server", "/normal/applications/appsConnect.asp");
      } else {
         menuPages[i++] = new menuObj("CONS", "Connect to Server", "/normal/applications/appsConnectPortOnly.asp");
      }
   } else if (parent.family_model == "KVM" ) {
      if (parent.kvmAnalog == 0) {
         //if (parent.access == "easy" || parent.ConnectBox == 1) {
           //menuPages[i++] = new menuObj("CONS", "Connect to Server", "/normal/applications/appsConnect.asp");
         //} else {
           menuPages[i++] = new menuObj("CONK", "Connect to Server", "/normal/applications/accessConnect.asp");
         //}
      }
   } else {
      if (parent.access == "easy" || parent.ConnectBox == 1) {
         menuPages[i++] = new menuObj("CONS", "Connect", "/normal/applications/appsConnect.asp");
      } else {
         menuPages[i++] = new menuObj("CONS", "Connect", "/normal/applications/appsConnectPortOnly.asp");
      }
   }

   if (parent.family_model != "AVCS") {
      menuPages[i++] = new menuObj("PWIPDU", "<%getLabel("system.application.IPDU");%> Power Mgmt.", "/normal/applications/appsPMOutletsMan.asp");

      if (parent.access != "restricted") {
         menuPages[i++] = new menuObj("PMDC", "PMD Configuration", "/normal/applications/appsPMDconfGen.asp");
      }
   }

   if (parent.family_model != "KVM" && parent.family_model != "AVCS") {
      if (parent.access != "restricted") {
       menuPages[i++] = new menuObj("PWIPMI", "IPMI Power Mgmt.", "/normal/applications/appsPMIPMI.asp");
      }
   }

   if (parent.family_model != "KVM" && parent.access == "easy") {
      menuPages[i++] = new menuObj("TPM", "Terminal Profile Menu", "/normal/applications/appsTermProMenu.asp");
   }

   if (Local == "1" && parent.access == "restricted") {
      menuPages[i++] = new menuObj("PASS", "Security", "/normal/applications/regularSecurity.asp")
   }

   if (parent.family_model == "ONS" ) {
      menuPages[i++] = new menuObj("TEMP", "Temperature Sensors", "/normal/applications/appsTempSensor.asp")
   }
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
  print_table_menu(20, 2, menuPages);
// End Hide -->
</script>

</body>
</html>
