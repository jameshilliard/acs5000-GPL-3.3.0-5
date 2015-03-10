<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help/Step 2</title>
	<link rel="STYLESHEET" type="text/css" href="../../stylesLayout.css">
    <script language="JavaScript" src="../../scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
      function submit()
      {
         document.configForm.submit();
      }
    </script>
</head>
<body  class="bodyHelp">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr> 
    <td width="50" height="50" align="left" valign="top" nowrap><img src="../../Images/helpImage.gif" alt="" width="50" height="50" border="0"></td>
    <td width="365" align="center" valign="middle" class="titleHelp" nowrap>Help<br>
      Step 2: CAS Profile Port Settings</td>
  </tr>
  <tr align="center" valign="middle"> 
    <td colspan="2"><table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"> <font class="paragHelp">Introduction</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify"><font class="textHelp"> 
              This page allows the setup of generic serial port parameters and 
              applies to all ports on the unit. <br> In Wizard mode this assumes all 
              devices will be connected at the same speed etc. Where specific 
              devices have different values then use the expert mode under Ports->Physical 
              Ports. </font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"> <font class="paragHelp">Configuration</font></td>
        </tr>
	<script>
		document.write("<tr> <td width=\"25\">&nbsp;</td>")
                document.write("<td width=\"25\" align=\"center\" valign=\"top\">&nbsp;</td>")
                document.write("<td width=\"365\" align=\"left\" valign=\"middle\"> <font class=\"textHelp\">")
                document.write("<div align=\"justify\">Connection Protocol - Is the method you will use to access the serial ports.")
		document.write("&nbsp;Recommend SSH to ensure all data and authentication information is encrypted.")
                document.write("&nbsp;Other options are Telnet, TelnetSSH or Raw Data (for un-negotiated plain socket connections).<p>")
                document.write("Baud Rate - Is the serial speed on each console port and should match the equipment you will connect to.</p><p>")
                document.write("Data Size - Number of data bits used by the attached devices.</p><p>")
                document.write("Stop Bits - Number of stop bits used by the attached devices.</p><p>")
                document.write("Parity - Parity used by the attached devices.</p><p>")
                document.write("Flow Control - Method of flow control used by the attached devices.</p><p>")
                document.write("Authentication Required - This selects if authentication is required to access the ports.")
                document.write("&nbsp;If it is selected then authentication is done in the local database in the unit.")
                document.write("&nbsp;You must add users in the Wizard step 3 if you will use port authentication.")

              if ((opener.top.family_model == "ACS")||(opener.top.family_model == "AVCS")) {
                document.write("&nbsp;Other authentication methods such as LDAP, RADIUS, TACACS, KERBEROS, ... can also be chosen and configured in Expert Mode under Security -> Authentication")
                document.write("</p></div></font></td></tr>")
	      } 
	      else {
                document.write("&nbsp;Other authentication methods such as LDAP, RADIUS, TACACS, KERBEROS, ... can also be chosen and configured in Expert Mode under Configuration -> Authentication")
                document.write("</p></div></font></td></tr>")
              }
	</script>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" class="tabsTextBig" height="40"> <font class="paragHelp">Additional 
            Information</font></td>
        </tr>
		<tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify"> 
		  <font class="textHelp">See the user manual</font></div> </td>
        </tr>
        <tr>
          <td width="25"  height="40">&nbsp;</td>
          <td align="center" valign="top" width="25" height="40">&nbsp;</td>
          <td width="365" height="40" align="center" valign="middle">
		  <a href="javascript:window.close();">
          <img src="../../Images/closeButton.gif" width="47" height="21" border="0" alt=""></a></td>
        </tr>        
      </table></td>
  </tr>
</table>
</body>
</html>
