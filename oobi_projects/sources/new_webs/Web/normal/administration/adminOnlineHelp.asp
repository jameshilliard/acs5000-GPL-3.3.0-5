
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
  
  <%
    var online = "system.administration.onlineHelp.";
    ReqNum = getRequestNumber();

    var model = getVal('_familymodel_');
    var ext = getVal('_familyext_');
    var analog = getVal("system.device.kvmanalog");
    var version = getVal("system.device.swVersion");
    var sx_prod = "undefined";

    if ((model == "ACS")||(model == "AVCS")) {
        sx_prod = getLabelVal("help");
    }
    if (model == "ONS") {
        sx_prod = "ons";
    }
    if (model == "KVM") {
        sx_prod = "kvm";
        if (analog == 0) {
            sx_prod = "kvmnet";
        }
        if (ext == "KVMNETP") {
            sx_prod = "kvmnetplus";
        }
    }
  %>

  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = 'helpPages/adminOnlineHelp.asp';

  function init()
  {
    if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    {
      if (checkLeftMenu('/normal/administration/adminMenu.asp') == 0) {
         parent.menuReady = 0;
         setTimeout('init()', 200);
         return;
      }
      setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
      parent.topMenu.selectItem(parent.menu.topItem);
      parent.menu.selectItem("HELP");
    }
    else
      setTimeout('init()', 200);
  }

  function copyData()
  {
    document.configForm.elements[5].value = document.onlineHelpForm.elements[0].value;
  }
  
  function form_verification()
  {
    var form_location = document.onlineHelpForm;
    checkElement (form_location.elements[0], true, false, false, false);
  }

      
  function submit()
  {
    form_verification();
    if (continue_verification == true)
    {    
      copyData();
      document.configForm.submit();
      parent.mainReady = null;
    }
  }
      
    </script>
</head>

<body onload="init();" class="bodyMain">
   <form name="onlineHelpForm" onSubmit="return false;">
      <table border="0" width="100%" height="70%" align="center">
         <tr><td>
         <table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">
            <tr><td>
            <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
               <tr><td align="center" class="label_for_tables" >
     Configures the Online Help path.<br>
     Paths ending in '/' will be appended with the product name and version.<br>
     Otherwise the entire path will be used to access the help file.<br><br>
     Example: http://www.MyHttpServer.com/online-help/ will be extended to<br>
<script language="JavaScript">
     var version = '<%write(version);%>';
     var sx_ver = version.split(" ");

     version = sx_ver[0].toLowerCase();

     if (version.charAt(version.length-1) < '0' ||
         version.charAt(version.length-1) > '9') {
         version = version.substring(0, version.length-1);
     }

     document.write("http://www.MyHttpServer.com/online-help/<%write(sx_prod);%>/" + version + "/en/index.html");
</script>
              </td></tr>
            </table>
            </td></tr>
         </table> 
         </td></tr>
         <tr><td>
            <table border="0" cellpadding="2" cellspacing="0" align="center">
               <tr><td><font class="label">Online Help Path</font></td></tr>
               <tr><td>
                  <input class="formText" type="text" maxlength="120" name="path" size="60" value='<%get(online + "path");%>'>
               </td></tr>
            </table>
         </td></tr>
      </table>
   </form>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/administration/adminOnlineHelp.asp">
   <input type="hidden" name="urlError" value="/normal/administration/adminOnlineHelp.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
 <!--*************************************************************************-->
  <!--these hidden inputs are used to store values of form elements inside layers-->
   <input type="hidden" name="<%write(online);%>path" value='<%get(online + "path");%>'>
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
