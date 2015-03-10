
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
   <%
      var ReqNum = getRequestNumber();
      var SID = getVal("_sid_");
   %>

   window.name = '<%get("system.device.ip");%>_<%write(SID);%>_<%write(ReqNum);%>';
   parent.currentPage = self;
   var page = 0;
   
   function init()
   {
      if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
      {
		 page = parent.menu.getPageIndex('LAST', parent.menu.menuPages);
		 setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
         parent.menu.takeAwaySelect();
      }
      else
         setTimeout('init()', 200);
   }

   function submit()                                                                        
   {                                                                                         
       document.configForm.submit();
	   parent.mainReady = null;
   }
    </script>
</head>
<body onload="init();" class="bodyMain">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr>
	<td valign="middle">
		<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="450" align="center">
   			<tr><td><table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
       			<tr><td align="left" class="label_for_tables">
			Configuration Wizard has been completed.<br>
			You can either "try changes" (the new configuration 
			starts working but is not saved to flash memory) or  
			"apply changes" (the new configuration starts working
			and is also saved to flash memory).<br>
			Advanced Configuration can be done in Expert Mode. <br>
       			</td></tr>
   			</table></td></tr>
		</table>
	</td>
</tr>
</table>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%write(SID);%>">
   <input type="hidden" name="urlOk" value="/wizard/lastPage.asp">
   <input type="hidden" name="urlError" value="/wizard/lastPage.asp">
   <input type="hidden" name="request" value="0">
   <!--**************************************************** mark the page *******-->
   <input type="hidden" name="system.tablePage.bogus" value="wizLastPage">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_", "0");%>
</body>
</html>
