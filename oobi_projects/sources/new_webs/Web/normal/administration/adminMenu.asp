
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

      if ((parent.family_model != "ACS")&&(parent.family_model != "AVCS")) {
         leftHeadMenu("SYS");
      }
   }

   var pageName = "/normal/administration/adminMenu.asp";

   if ((parent.family_model == "ACS")||(parent.family_model == "AVCS")) {
     var topItem = "ADM";
     var menuPages = new Array;
     var i = 0;


     menuPages[i++] = new menuObj("SYS", "System Information", "/normal/information/infoGeneral.asp");
     menuPages[i++] = new menuObj("NTF", "Notifications", "/normal/administration/adminNotifications.asp");
     menuPages[i++] = new menuObj("TIME", "Time / Date", "/normal/administration/adminTimeDate.asp");
     if (parent.UBOOT_PARAM == "1") {
     menuPages[i++] = new menuObj("BOOT", "Boot Configuration", "/normal/administration/adminUBootConfig.asp");
     } else {
       menuPages[i++] = new menuObj("BOOT", "Boot Configuration", "/normal/administration/adminBootConfig.asp");
     }
     menuPages[i++] = new menuObj("BKP", "Backup Config", "/normal/administration/adminBackupConfig.asp");
     menuPages[i++] = new menuObj("FW", "Upgrade Firmware", "/normal/administration/adminUpgradeFirm.asp");
     menuPages[i++] = new menuObj("REB", "Reboot", "/normal/administration/adminReboot.asp");
     menuPages[i++] = new menuObj("HELP", "Online Help", "/normal/administration/adminOnlineHelp.asp");
   } else {
   if ( (parent.family_model == "ONS")||(parent.family_ext == "KVMNETP") ) {
     var topItem = "CON";

     var menuPages = new Array(
       new menuObj("TIME", "Time / Date", "/normal/administration/adminTimeDate.asp"),
       new menuObj("BOOT", "Boot Configuration", "/normal/administration/adminUBootConfig.asp"),
       new menuObj("HELP", "Online Help", "/normal/administration/adminOnlineHelp.asp")
     );
 
   }
    else {
     var topItem = "CON";

     var menuPages = new Array(
       new menuObj("TIME", "Time / Date", "/normal/administration/adminTimeDate.asp"),
       new menuObj("BOOT", "Boot Configuration", "/normal/administration/adminBootConfig.asp"),
       new menuObj("HELP", "Online Help", "/normal/administration/adminOnlineHelp.asp")
     );
   }
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
  print_head_menu(20, 2, menuPages, "SYS");
// End Hide -->
</script>

</body>
</html>
