<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help/Step 4</title>
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
      Step 4: Data Buffering </td>
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
          <td width="365" align="left" valign="middle"><div align="justify"><font class="textHelp">
		  This part of the wizard enables you to configure data buffering. This allows all 
		  console data to be logged to a data buffer file either locally in the unit or to 
		  an external storage source such as NFS server or Syslog server. More advanced 
		  configuration of this feature is available in the Expert mode.</font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"> <font class="paragHelp">Configuration</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify"><font class="textHelp">
		  To enable data buffering
		  <ul type="square">
		  <li>Click the checkbox.</li>
		  <li>Select the location for the data files either local (in ramdisk) or remote.</li>
		  <li>For local specify the file size.</li>
		  <li>For remote specify the nfs mount point. 
		  Note the NFS server must be already configured with the mount point exported.</li>
		  </ul></font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" class="tabsTextBig" height="40"> <font class="paragHelp">
		  Additional Information</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"> <font class="textHelp"> 
            <div align="justify">
			You also have the choice to use linear or circular data buffering 
			and have a timestamp recorded in the data buffer file. See the user 
			manual for more details of these features.
			</div></font></td>
        </tr>
        <tr>
          <td width="25" height="40">&nbsp;</td>
          <td align="center" valign="top" width="25" height="50">&nbsp;</td>
          <td width="365" height="40" align="center" valign="middle">
		  <a href="javascript:window.close();">
          <img src="../../Images/closeButton.gif" width="47" height="21" border="0" alt=""></a></td>
        </tr>        
      </table></td>
  </tr>
</table>
</body>
</html>

