<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
    <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script language="JavaScript" src="/menu.js" type="text/javascript"></script>
    <script language="JavaScript">

    //*********************begin side menu tracking script**********************

   if (parent.family_model == "KVM" ) {
     var menuPages = new Array(
      new menuObj("SRV", "Step 1: Security Profile", "/wizard/secProfile.asp"),
      new menuObj("NET", "Step 2: Network Settings", "/wizard/netSettings.asp"),
      new menuObj("ACC", "Step 3: Access", "/wizard/access.asp"),
      new menuObj("LOG", "Step 4: <%getLabel("wizardMenu5");%>", "/wizard/systemLog.asp"),
      new menuObj("LAST", "", "/wizard/lastPage.asp")
     );
  } else if (parent.family_model == "ONS") {
     var menuPages = new Array(
      new menuObj("SRV", "Step 1: Security Profile", "/wizard/secProfile.asp"),
      new menuObj("NET", "Step 2: Network Settings", "/wizard/netSettings.asp"),
      new menuObj("PORT", "Step 3: Serial Port Profile", "/wizard/CASProSettings.asp"),
      new menuObj("ACC", "Step 4: Access", "/wizard/access.asp"),
      new menuObj("BUF", "Step 5: Data Buffering", "/wizard/dataBuffering.asp"),
      new menuObj("LOG", "Step 6: <%getLabel("wizardMenu5");%>", "/wizard/systemLog.asp"),
      new menuObj("LAST", "", "/wizard/lastPage.asp")
     );
  } else if (parent.family_model == "AVCS") {
     var menuPages = new Array(
      new menuObj("NET", "Step 1: Network Settings", "/wizard/netSettings.asp"),
      new menuObj("PORT", "Step 2: Port Profile", "/wizard/CASProSettings.asp"),
      new menuObj("ACC", "Step 3: Access", "/wizard/access.asp"),
      new menuObj("BUF", "Step 4: Data Buffering", "/wizard/dataBuffering.asp"),
      new menuObj("LOG", "Step 5: <%getLabel("wizardMenu5");%>", "/wizard/systemLog.asp"),
      new menuObj("LAST", "", "/wizard/lastPage.asp")
     );
  } else {
     var menuPages = new Array(
      new menuObj("SRV", "Step 1: Security Profile", "/wizard/secProfile.asp"),
      new menuObj("NET", "Step 2: Network Settings", "/wizard/netSettings.asp"),
      new menuObj("PORT", "Step 3: Port Profile", "/wizard/CASProSettings.asp"),
      new menuObj("ACC", "Step 4: Access", "/wizard/access.asp"),
      new menuObj("BUF", "Step 5: Data Buffering", "/wizard/dataBuffering.asp"),
      new menuObj("LOG", "Step 6: <%getLabel("wizardMenu5");%>", "/wizard/systemLog.asp"),
      new menuObj("LAST", "", "/wizard/lastPage.asp")
     );
   }
    
   function init()
   {
      parent.menuReady = 1;
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
  print_table_menu(19, 2, menuPages);
// End Hide -->
</script> 

</body>
</html>
