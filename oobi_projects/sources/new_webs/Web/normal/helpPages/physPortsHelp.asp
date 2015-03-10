<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help</title>
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
    <td width="50" height="60" align="left" valign="top" nowrap><img src="../../Images/helpImage.gif" alt="" width="50" height="50" border="0"></td>
    <td width="365" align="center" valign="middle" class="titleHelp" nowrap>Help<br>
    Ports | Physical Ports</td>
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
          <td width="365" align="left" valign="middle" height="40"><div align="justify"><font class="textHelp">
		  <p>This section activates/deactivates serial ports and sets 
		  the parameters for each or all ports. Specific parameters 
		  are configured for the serial ports where IPDU devices 
		  (<%getLabel("powername");%> or similar) are connected. Select one port to 
		  modify its settings, or set all ports to have the same settings.</p>
		  </font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"><font class="paragHelp">Configuration</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify"><font class="textHelp">
		  	<p>To modify the parameters for a specific port, select the port and click on [Modify Selected Ports] to bring up a dialog screen. Click on the header buttons and enter your settings for that port (speed, access permissions, IP alias, etc.).</p> 
			<p>To set the same parameters for all ports, click on [Modify all ports] and enter the parameters. All ports will then have the same settings.</p> 
			<p>If you are connecting an IPDU device, select that port and click [Modify Selected Ports] and select the power management protocol. To manage the IPDU, go to "Applications>Power Management".</p> 
			<p>To manage the outlet one server is powered from, select the serial port where that specific server's console is connected and click [Modify Selected Ports] and click [Power Management] tab. Check the [Enable power management on this port] and enter the specific outlet of the IPDU the server is plugged in, the hot key to start the integrated console and power management process and the users with permission to manage this outlet.</p> 
			<p>Please see the user manual for a description of the unique Integrated Console And Power Management feature.</p>

		  </font></div></td>
        </tr>
        <tr>
          <td width="25" height="50">&nbsp;</td>
          <td align="center" valign="top" width="25" height="50">&nbsp;</td>
          <td width="365" height="50" align="center" valign="middle">
		  <a href="javascript:window.close();">
          <img src="../../Images/closeButton.gif" width="47" height="21" border="0" alt=""></a></td>
        </tr>        
      </table></td>
  </tr>
</table>
</body>
</html>


