<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

<% ReqNum = getRequestNumber(); %>	
	
   window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
   parent.currentPage = self;
   parent.helpPage = 'helpPages/portsStatusHelp.asp';
   var topMenu = '/normal/ports/portsMenu.asp';
   var adProbeStrings = '<%get("system.autoDiscovery.ADProbeString");%>';
   var adAnswerStrings = '<%get("system.autoDiscovery.ADAnswerString");%>';
   var adProbeString;
   var adAnswerString;
   
   function init()
   {
      if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
      {
         if (checkLeftMenu(topMenu) == 0) {
            parent.menuReady = 0;
            setTimeout('init()', 200);
            return;
         }
         setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
         parent.menu.selectItem('AD');
         parent.topMenu.selectItem(parent.menu.topItem);
      }
      else
         setTimeout('init()', 200);
   }
   
   function form_verification() {
      var i;
      var check, strings = [document.adProbeStringsForm.ADProbeStrings.value,
                            document.adAnswerStringsForm.ADAnswerStrings.value];
      var object;
      var errMsg = [ 'Please provide a list of double-quote (") delimited, blank space separated strings',
                     'Please precede by a blackslash (\\) quotes (\') and double-quotes (") that are part of the string' ];

      continue_verification = true;

      for (i=0; i<strings.length; i++) {
         strings[i] = strings[i].replace(/(^[ \r\n]+)|([ \r\n]+$)/g, "");
         if (strings[i].length <= 0) {
            document.configForm.elements[5+i].value = '';
            continue;
         }
         check = strings[i].match(/(^\")|(\"$)/g);
         if (check == null) {
            continue_verification = false;
            alert(errMsg[0]);
            break;
         } else if (check.length != 2) {
            continue_verification = false;
            alert(errMsg[0]);
            break;
         } else {
            check = strings[i].replace(/(^\")|(\"$)/g, "").replace(/\"[ \t\r\n]+\"/g, " ");
            if (check.match(/[^\\][\"\']/g) != null) {
               continue_verification = false;
               if (check.match(/( [\"\'])|([\"\'] )/g) == null) {
                  alert(errMsg[0]);
               } else {
                  alert(errMsg[1]);
               }
               break;
            } else {
               strings[i] = strings[i].replace(/\"[ \t\r\n]+\"/g, "\" \"");
               document.configForm.elements[5+i].value = strings[i];
            }
         }
      }
      
      if (continue_verification == false) {
         object = (i==0)? document.adProbeStringsForm.ADProbeStrings
                        : document.adAnswerStringsForm.ADAnswerStrings;
         object.focus();
         object.select();
      }
      
      return continue_verification;
   }

   function submit()
   {
      if (form_verification()) {
         document.configForm.submit();
         parent.mainReady = null;
      }
   }
    
    </script>
</head>
<body onload="init();" class="bodyMain">
<form name="adProbeStringsForm" onSubmit="return false;">
<table border="0" cellpadding="0" cellspacing="0" align="center">
 <tr align="left" valign="top"> 
    <td>
  <table border="0" cellpadding="0" cellspacing="0">
       <tr>             
          <td align="center"><font class="label">&nbsp;Hostname Discovery Probe Strings</font></td>
       </tr>
 </table></td>
 </tr>
 <tr align="left" valign="top"> 
  <td align="left"><font class="formText">
  <!-- options width = 
  1 (0 ... 20); 2 whitespace; (220px) 
  2 (0 ... 10); 2 whitespace;  (110px)
  3 (0 ... 10); 2 whitespace;  (105px)
  4 (0 ... 10); 2 whitespace;  
  -->
  <script type="text/javascript">
    document.write('<textarea name="ADProbeStrings" cols="60" rows="3" class="formText">');
    document.write(adProbeStrings.replace(/\"[ \t]+\"/g,"\"\n\""));
    document.write('</textarea>');
  </script>
  </td>
 </tr>
</table>
</form>
<form name="adAnswerStringsForm" onSubmit="return false;">
<table border="0" cellpadding="0" cellspacing="0" align="center">
 <tr align="left" valign="top"> 
    <td>
  <table border="0" cellpadding="0" cellspacing="0">
       <tr>             
          <td align="center"><font class="label">&nbsp;Hostname Discovery Answer Strings</font></td>
       </tr>
 </table></td>
 </tr>
 <tr align="left" valign="top"> 
  <td align="left"><font class="formText">
  <!-- options width = 
  1 (0 ... 20); 2 whitespace; (220px) 
  2 (0 ... 10); 2 whitespace;  (110px)
  3 (0 ... 10); 2 whitespace;  (105px)
  4 (0 ... 10); 2 whitespace;  
  -->
  <script type="text/javascript">
    document.write('<textarea name="ADAnswerStrings" cols="60" rows="6" class="formText">');
    document.write(adAnswerStrings.replace(/\"[ \t]+\"/g,"\"\n\""));
    document.write('</textarea>');
  </script>
  </td>
 </tr>
</table>
<br>
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="450" align="center">
	<tr>
		<td>
			<table class="bgColor1" border="0" cellpadding="10" cellspacing="0" class="helpBox" width="100%" align="center">
				<tr>
					<td align="center" class="tabsTextBig">
						<script type="text/javascript">
							document.write('Default probe string is "\\n".<br>\
Default answer string is "([A-Za-z0-9\\._-]+)[ ]+[Ll]ogin[:]?[ ]?$".<br>\
If an answer string has more than one group, only the first group is matched.');
						</script>
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>
</form>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/ports/portsDiscovery.asp">
   <input type="hidden" name="urlError" value="/normal/ports/portsDiscovery.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
   <input type="hidden" name="system.autoDiscovery.ADProbeString" value="">
   <input type="hidden" name="system.autoDiscovery.ADAnswerString" value="">
</form>
<script type="text/javascript">
parent.mainReady = true;
</script>   
<%set("_sid_","0");%>
</body>
</html>
