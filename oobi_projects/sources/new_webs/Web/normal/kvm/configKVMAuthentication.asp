<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
	
	top.currentPage = self;

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
            top.menu.selectItem("AUTH");
         }
         else
            setTimeout('init()', 200);
      }

      function submit()
      {
         document.configForm.submit();
      }
      
     </script>
</head>

<body onload="init();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/security/secAuthentication.asp">
   <input type="hidden" name="urlError" value="/normal/security/secAuthentication.asp">
   <input type="hidden" name="request" value=<%write(getRequestNumber());%>>
</form>
<p><br><p><br>
<div class="alignCenter">
<font class="label"> secAuthentication.asp - Authentication ID 27</font>
</div>
</body>
</html>
