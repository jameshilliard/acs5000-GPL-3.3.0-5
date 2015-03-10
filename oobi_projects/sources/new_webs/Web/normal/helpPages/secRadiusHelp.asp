<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help</title>
	<link rel="STYLESHEET" type="text/css" href="../../stylesLayout.css">
    <script language="JavaScript" src="../../scripts.js" type="text/javascript"></script>
    <script type="text/javascript">
	<% FamilyModel = getVal('_familymodel_'); %>
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
    <%if ((FamilyModel == "ACS")||(FamilyModel == "AVCS"))
		write("Security | Authentication | Radius</td>");
	  else
		write("Configuration | Authentication | Radius</td>");
	%>
  </tr>
  <tr align="center" valign="middle"> 
    <td colspan="2"><table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr> 
          <td width="25" height="20">&nbsp;</td>
          <td width="25" align="center" valign="top" height="20"> &nbsp;</td>
          <td width="365" align="left" valign="middle" height="20"><div align="justify"><font class="textHelp"></font></div></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify"><font class="textHelp">
		  <p>Configure Radius related parameters. Please consult your
		  network administrator for authentication servers in your network.</p></font></div></td>
        </tr>
        <tr> 
          <td width="25" height="20">&nbsp;</td>
          <td width="25" align="center" valign="top" height="20"> &nbsp;</td>
          <td width="365" align="left" valign="middle" height="20"><div align="justify"><font class="textHelp"></font></div></td>
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
