<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help/Syslog</title>
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
    Network | Syslog
 </td>
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
          Here you can configure how the <%getLabel("nickname");%> will handle its syslog messages. 
          The <%getLabel("nickname");%> generates syslog messages related to users connecting to ports, 
          login failures and other information that can be used for audit and control purposes.
          </font></div></td>
                     
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
          Select the Syslog facility number that the <%getLabel("nickname");%> will use to send out syslog messages.
          This number can be used by the syslog servers to identify the messages origin.
          <p>Choose the destination of the messages, selecting to send them to the console port (that will log the messages even if no user logged in),
          to all sessions where the root user is logged and/or to a list of remote syslog servers. 
          In this case, enter the IP address of a syslog server and click "Add".
          You may repeat this step for additional syslog servers.</p>
          <p>Choose the priority level of the messages. 
          Only the messages that matches the selected levels of each application will be logged.</p>
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

