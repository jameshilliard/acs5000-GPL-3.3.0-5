
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

      function init()
      {
         parent.controlsReady = 1;
      }

      function submitFormNav(direction)
      {
 
		 if (direction == 'tryIt')
         {
            parent.currentPage.document.configForm.elements[0].value = '<%get("_kTry_");%>';
            setQuerySSID(parent.currentPage.document.configForm);
            parent.currentPage.submit();
			if (parent.currentPage.continue_verification == false)
			{parent.currentPage.document.configForm.elements[0].value = ''}
			else
	        {alert('<%getLabel("triedChanges");%>');
	        IPmodeChange();}
         }

         if (direction == 'cancelIt')
         {
            parent.currentPage.document.configForm.elements[0].value = '<%get("_kCancel_");%>';
            setQuerySSID(parent.currentPage.document.configForm);
            parent.currentPage.document.configForm.submit();
	        alert('<%getLabel("CanceledChanges");%>');
         }
		 
         if (direction == 'reloadIt')
         {
            // modifications for reload
	        parent.currentPage.document.configForm.elements[0].value = '<%get("_kReloadPage_");%>';
            setQuerySSID(parent.currentPage.document.configForm);
            parent.currentPage.document.configForm.submit();
         }		 

         if (direction == 'applyIt')
         {
            parent.currentPage.document.configForm.elements[0].value = '<%get("_kSet_");%>';
            setQuerySSID(parent.currentPage.document.configForm);
            parent.currentPage.submit();
			if (parent.currentPage.continue_verification == false)
			{parent.currentPage.document.configForm.elements[0].value = ''}
			else
	        {alert('<%getLabel("appliedChanges");%>');
	        IPmodeChange();}
         }
      }

    </script>
</head>

<body onload="init();" class="bgColor11" bottommargin="0" topmargin="0" leftmargin="0" rightmargin="0" marginheight="0" marginwidth="0">

<div>
    <table class="bgColor8" border="0" cellpadding="0" cellspacing="0" width="100%" height="98">
        <tr>
           <td align="right" valign="top" width="100%" colspan="">
                <table border="0" cellpadding="0" cellspacing="0" height="98" width="100%" align="right">
                    <tr>
                        <td class="bgColor8" align="right">
                            <table border="0" cellpadding="0" cellspacing="0" width="100%">
                                <tr valign="bottom">                                   
                                    <td width="72">
                                        <a href="javascript:submitFormNav('tryIt');"><img src="/Images/tryChanges.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>
                                    <td width="72">
                                        <a href="javascript:submitFormNav('cancelIt');"><img src="/Images/cancelChanges.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>
                                    <td width="72">
                                        <a href="javascript:submitFormNav('applyIt');"><img src="/Images/applyChanges.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>
									<td width="72">
                                        <a href="javascript:submitFormNav('reloadIt');"><img src="/Images/reload.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>																	
                                    <td width="72">
<!-- Open a new window to connect to the configured Online Help page.-->
                                        <a href="javascript:newWindow('/onlineHelp.asp', 'Help', 800, 600, 'yes');"><img src="/Images/help.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>                                     
                                    <td width="85">
                                    	<img name="unsaved" src="/Images/noUnsaved.gif" alt="" border="0">
                                    </td>
                                </tr>
                            </table>
                            <table border="0" cellpadding="1" cellspacing="1" width="100%" height="42">
       							<tr valign="bottom" align="right"><td><img name="goAhead" src="/Images/goAheadWS.gif" alt="" border="0"></td></tr>
       						</table>
                        </td>
                        <td background="/Images/rightEdge.gif" width="6" height="98" rowspan="3"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
                    </tr>
                </table>               
            </td>
        </tr>
        <tr>
            <td background="/Images/botEdge.gif" width="100%" height="2" align="right"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>
        </tr>
    </table>


</div>

</body>
</html>
                
