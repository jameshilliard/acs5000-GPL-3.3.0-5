<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help/Step 1</title>
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
      Step 1: Network Settings</td>
  </tr>
  <tr align="center" valign="middle"> 
    <td colspan="2"><table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"> <font class="paragHelp">Introduction</font></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="top" height="40"> &nbsp;</td>
          <td width="365" align="left" valign="middle" height="40"><div align="justify"> <font class="textHelp">This 
            page allows configuring network parameters to 
            make the unit accessible on the network.</font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"> <font class="paragHelp">DHCP 
            Configuration</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify"> <font class="textHelp">You 
            may choose the DHCP option which is the default and allows all network 
            parameters to be set from a DHCP server.</font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" class="tabsTextBig" height="40"> <font class="paragHelp">Manual 
            Configuration</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"> <font class="textHelp"> 
            <div align="justify"><p>Uncheck the DHCP checkbox to perform manual configuration. You will 
            need to enter:</p>
            Host Name - The host name of this console server<br>
            Domain Name - domain name of this console server<br>
            IP Address - IP address of this console server<br>
            <%getLabel("system.netSettings.netMask1");%> - Network Mask of this console server<br>
            <%getLabel("system.netSettings.gateway");%> - Default gateway in this network<br>
            Domain Name Server IP address - IP address of a domain name server 
            </font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" class="tabsTextBig" height="40"> <font class="paragHelp">Additional 
            Information</font></td>
        </tr>
		<tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="top" height="40">&nbsp;</td>
          <td width="365" align="left" valign="middle" height="40"><div align="justify"> <font class="textHelp">More 
            advanced network configuration can be performed in expert mode under 
            Network->Host Settings</font></div> </td>
        </tr>
        <tr>
          <td width="25" height="40">&nbsp;</td>
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
