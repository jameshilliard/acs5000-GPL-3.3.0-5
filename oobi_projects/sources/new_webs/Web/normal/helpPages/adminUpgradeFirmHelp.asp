

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
<body  class="bodyHelp" link="#7a0e20" vlink="#7a0e20" alink="#7a0e20">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr> 
    <td width="50" height="60" align="left" valign="top" nowrap><img src="../../Images/helpImage.gif" alt="" width="50" height="50" border="0"></td>
    <td width="365" align="center" valign="middle" class="titleHelp" nowrap>Help<br>
    Administration | Upgrade Firm</td>
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
		  <p><%getLabel("prodname");%> firmware is upgradeable using FTP server. 
		  This section configures the FTP connection and the new file to be downloaded.</p>
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
		  <p>If you have access to the <%getLabel("upgsite");%> site (<a href="<%getLabel("upgsiteurl");%>" target="_blank"><%getLabel("upgsiteurl");%></a>) and know the complete path 
		  to the file, you can upgrade the firmware directly. If you do not have direct access to 
		  the internet, download the new firmware file to one local FTP server and upgrade from there.</p>
          <p>Please double-check the file name for the version you are to upgrade, and read carefully 
		  the upgrade instructions; distinct procedures are required depending on the version you are 
		  upgrading from. MD5 checksum file is also available in the same directory as the firmware file.</p>

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


