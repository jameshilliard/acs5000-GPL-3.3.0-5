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
   var pageName = "regUserMenu.asp";

   if (parent.family_model == "KVM" ) {
      var topItem = "ACC";
      var menuPages = new Array(
        new menuObj("CONN", "Connect to server", "/normal/applications/accessConnect.asp"),
        new menuObj("PW", "Power Management", "/normal/applications/appsPMOutletsMan.asp")
      );
   } else {
      var topItem = "APP";
      var menuPages = new Array(
        new menuObj("CONN", "Connect", "/normal/applications/appsConnect.asp"),
        new menuObj("PW", "Power Management", "/normal/applications/appsPMOutletsMan.asp")
      );
      if (Local == "1") {
        i = menuPages.length;
        menuPages[i] = new menuObj("PASS", "Security", "/normal/regularUser/regularSecurity.asp")
      }
   }

   </script> 

    <script language="JavaScript" src="/menuNav.js" type="text/javascript"></script>

</head>

<body onload="init();" class="bgColor11" bottommargin="0" topmargin="0" leftmargin="0" rightmargin="0" marginheight="0" marginwidth="0">

<table border="0" cellpadding="0" cellspacing="0" width="204" height="100%">    
    <tr>
        <td class="bgColor5">   
            <table border="0" cellpadding="0" cellspacing="0" width="16" height="100%">
                <tr>
                    <td background="/Images/leftEdge.gif" height="100%"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
                </tr>
            </table>
        </td>
        
        <td class="bgColor5" valign="top">
            <table border="0" cellpadding="0" cellspacing="0" width="163" height="100%">
                <tr>
                    <td class="bgColor5" align="center" valign="bottom"><img src="/Images/menuTop.gif" alt="" width="163" height="4" border="0"></td>
                </tr>
                <tr>
                    <td background="/Images/menuBackground.gif" height="100%" width="100%" valign="top">
                        &nbsp;
                    </td>                    
                </tr>
            </table>
        </td>
        <td class="bgColor5" width="25">
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
