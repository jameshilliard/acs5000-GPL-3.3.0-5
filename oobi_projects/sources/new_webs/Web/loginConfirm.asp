<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title><%getLabel("title");%> | <%getLabel("page.login");%></title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
   <%
      // Set page-wide variables:
      var self = "system.login."; 
   %>
	function confirm() {
		document.mainForm.elements[6].value = 1;
		document.mainForm.submit();
	}
	function noconfirm() {
		document.mainForm.elements[6].value = 0;
		document.mainForm.submit();
	}
   </script>
</head>

<body class="bodyMain">
<table height="100%" align="center">
   <tr valign="middle" align="center">
      <td> 
         <form name="mainForm" method="POST" action="/goform/Dmf">
         <input type="hidden" name="system.req.action" value="">
         <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
         <input type="hidden" name="urlOk" value=""> <!-- an arg will be put to this by LoginConfirm() in UserFunc.c -->
         <input type="hidden" name="urlError" value="/login.asp">
         <input type="hidden" name="request" value=<%write(getRequestNumber());%>>
         <input type="hidden" name="<%write(self + "userlogin");%>" value='<%get(self + "userlogin");%>'>
         <input type="hidden" name="<%write(self + "confirm");%>" value="0">      
         <table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="650" align="center">
             <tr>
                 <td>
                     <table class="bgColor1" border="0" cellpadding="20" cellspacing="0" class="helpBox" 
                     width="100%" align="center">
                         <tr>
                             <td align="center" class="labelBold" >
Another administrator is logged into this device.<br>
Do you want to log in and force the other administrator to be logged off ?<br><br>
			<a href="javascript:confirm();" class="yellowButton">&nbsp;&nbsp;Yes&nbsp;&nbsp;</a>
			<a href="javascript:noconfirm();" class="yellowButton">&nbsp;&nbsp;No&nbsp;&nbsp;</a>
               </td>
                         </tr>
                     </table>
                 </td>
             </tr>
         </table>
         </form>
      </td>
   </tr>
</table>

</body>
</html>
