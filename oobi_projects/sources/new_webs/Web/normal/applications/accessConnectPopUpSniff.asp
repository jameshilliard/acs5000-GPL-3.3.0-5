<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<% var tabx = getQueryVal("tabx", 0);%>
<% var sid = getQueryVal("SSID", "0");%>

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

var tabx=<%write(tabx);%>;
function connect()
{
	document.connectForm.sniff1.value= parent.sniff[0];
	document.connectForm.sniff2.value= parent.sniff[1];
	document.connectForm.sniff3.value= parent.sniff[2];
	document.connectForm.sniff4.value= parent.sniff[3];
   	document.connectForm.submit();
}

   </script>
</head>

<body class="bodyMain">


<form name="connectForm"
	action="/normal/applications/accessConnectPopUp.asp"
	target="_parent">
<input type="hidden" name="tabx" value="<%write(tabx);%>">
<input type="hidden" name="pagemode" value="submit">
<input type="hidden" name="action" value= "connect">
<input type="hidden" name="newport">
<input type="hidden" name="otherstation">
<input type="hidden" name="sniff1">
<input type="hidden" name="sniff2">
<input type="hidden" name="sniff3">
<input type="hidden" name="sniff4">
<input type="hidden" name="SSID" value="<%write(sid);%>">

<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%" height="100%">
<tr>
   <td align="center" valign="center">
      <table>
      <tr>
         <td>
<script type="text/javascript">
var cols = parent.sniff[tabx-1].split(' ');
if (cols.length >= 2) {
	document.connectForm.otherstation.value = cols[0];
	document.connectForm.newport.value = cols[1];
        document.write('<b>The server ' + cols[1] + ' is being used in another session!</b>');
}
</script>
         </td>
      </tr>
      <tr>
         <td align="center">
            <table>
            <tr>
               <td><input type="radio" name="sniffopt" value="1" checked></td>
               <td>Quit this session</td>
            </tr><tr>
               <td><input type="radio" name="sniffopt" value="2"></td>
               <td>Connect read only</td>
            </tr><tr>
               <td><input type="radio" name="sniffopt" value="3"></td>
               <td>Connect read write</td>
            </tr><tr>
               <td><input type="radio" name="sniffopt" value="4"></td>
               <td>Kill other session</td>
            </tr>
            </table>
         </td>
      </tr>
      <tr>
         <td align="center">
            <a href="javascript:connect();"><img src="/Images/OK_button.gif" alt="OK" width="35" height="21" border="0"></a>
         </td>
      </tr>
      </table>
   </td>
</tr>
</table>

</form>

</body>
</html>
