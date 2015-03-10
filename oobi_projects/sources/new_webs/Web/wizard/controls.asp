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
      var nextPageNumber = parent.currentPage.page + 1;
      var lastPageNumber = parent.currentPage.page - 1;
      var currPageName = parent.menu.menuPages[parent.currentPage.page].xRef;
   
      if (parent.mainReady && direction == 'back')
      {
	if(lastPageNumber > -1){
         parent.currentPage.document.configForm.urlOk.value = parent.menu.menuPages[lastPageNumber].xRef;
         setQuerySSID(parent.currentPage.document.configForm);
         parent.currentPage.submit();
	}

	show('document', 'next');
	
	if(lastPageNumber == 0){
		hide('document', 'back');
	}
	

      }    

	if (parent.mainReady && direction == 'forward')

      {
	if(parent.currentPage.document.configForm.elements[5].value != 0){
	show('document', 'back');
	}
	if(nextPageNumber ==  parent.menu.menuPages.length-1){
	  		hide('document', 'next');
	}

	if(nextPageNumber < parent.menu.menuPages.length){
         parent.currentPage.document.configForm.urlOk.value = parent.menu.menuPages[nextPageNumber].xRef;
         setQuerySSID(parent.currentPage.document.configForm);
         parent.currentPage.submit();

	}
	

	
      }

      if (direction == 'tryIt')
      {
         parent.currentPage.document.configForm.urlOk.value = currPageName;
         parent.currentPage.document.configForm.elements[0].value = '<%get("_kTry_");%>';
         setQuerySSID(parent.currentPage.document.configForm);
         parent.currentPage.submit();
         if (parent.currentPage.continue_verification == false)
		 {parent.currentPage.document.configForm.elements[0].value = '';}
		 else //(parent.currentPage.continue_verification == true || parent.currentPage.continue_verification == undefined)
		 {alert('<%getLabel("triedChanges");%>');
		 IPmodeChange();}
      }

      if (direction == 'cancelIt')
      {
         parent.currentPage.document.configForm.urlOk.value = currPageName;
         parent.currentPage.document.configForm.elements[0].value = '<%get("_kCancel_");%>';
         setQuerySSID(parent.currentPage.document.configForm);
         parent.currentPage.document.configForm.submit();
		 alert('<%getLabel("CanceledChanges");%>');
      }

      if (direction == 'applyIt')
      {
         parent.currentPage.document.configForm.urlOk.value = currPageName;
         parent.currentPage.document.configForm.elements[0].value = '<%get("_kSet_");%>';
         setQuerySSID(parent.currentPage.document.configForm);
         parent.currentPage.submit();
         if (parent.currentPage.continue_verification == false)
		 {parent.currentPage.document.configForm.elements[0].value = '';}
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
           <td align="right" width="100%" colspan="">
           
           
                <table border="0" cellpadding="0" cellspacing="0" height="98" width="100%" align="right">
                    <tr>
                        <td class="bgColor8" align="right">
                        
                        
                            <table border="0" cellpadding="0" cellspacing="0" width="100%">
                                <tr valign="bottom">
				<td width="60" align="left">
<div id="back" name="back" style="visibility: hidden;" align="left">
	<a href="javascript:submitFormNav('back');"><img src="/Images/back.gif" alt="" width="40" height="40" border="0"></a>
</div>

				</td>
                                    <td width="80" align="left">
                                        <a href="javascript:submitFormNav('tryIt');"><img src="/Images/tryChanges.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>
                                    <td width="50" align="left">
                                        <a href="javascript:submitFormNav('cancelIt');"><img src="/Images/cancelChanges.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>
                                    <td width="50" align="left">
                                        <a href="javascript:submitFormNav('applyIt');"><img src="/Images/applyChanges.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>
									<td width="50" align="left">
<!-- Open a new window to connect to the configured Online Help page.-->
                                        <a href="javascript:newWindow('/onlineHelp.asp', 'Help', 800, 600, 'yes');"><img src="/Images/help.gif" alt="" width="78" height="40" border="0"></a>
                                    </td>
					<td width="50" align="left">
<div id="next" name="next" style="visibility: hidden;" align="left">
	<a href="javascript:submitFormNav('forward');"><img src="/Images/next.gif" alt="" width="40" height="40" border="0"></a> 
</div>

					</td>
                                    <td  width="100" align="left">
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

<script type="text/javascript">
	if (parent.currentPage) {
	if(parent.currentPage.page == 0){
		hide('document', 'back');
                show('document', 'next');

	}
	else{
		show('document', 'back');
                show('document', 'next');

	}
	}
</script>


</body>
</html>
                

