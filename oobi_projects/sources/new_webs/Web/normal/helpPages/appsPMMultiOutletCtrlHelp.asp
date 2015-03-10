<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help</title>
	<link rel="STYLESHEET" type="text/css" href="../../stylesLayout.css">
    <script language="JavaScript" src="../../scripts.js" type="text/javascript"></script>
</head>
<body  class="bodyHelp">
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr> 
    <td width="50" height="60" align="left" valign="top" nowrap><img src="../../Images/helpImage.gif" alt="" width="50" height="50" border="0"></td>
    <td width="365" align="center" valign="middle" class="titleHelp" nowrap>Help<br>
    Applications | IPDU Multi-Outlet Ctrl</td>
  </tr>
  <tr align="center" valign="middle"> 
    <td colspan="2"><table width="100%" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="top" height="40"> &nbsp;</td>
          <td width="365" align="left" valign="middle" height="40"><div align="justify">
		  <font class="textHelp"><br>
			This page allows you to manage in a single operation all power outlets of a
			multi-outlet device connected to the <%getLabel("powername");%> family of
			Intelligent Power Distribution Units.
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"><font class="paragHelp">Operation</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify">
		  <font class="textHelp">
		  	The display shows groups of outlets that you are allowed to access and manage. 
			The lamp and the padlock icons indicate the current status of the whole group
			or the status of individual power outlets.
			<br>Click the group lamp icon to toggle the state of the group if all outlets
			are already ON or all outlets are already OFF.
			A group is ON if at least one outlet in the group is turned on. If the group is
			ON but there is at least one outlet in the group that is turned off, then
			clicking the group lamp icon will turn all outlets ON.
			<br>Click the "Cycle" button of a group which is ON and has all outlets turned on
			to momentarily power it off. 
			<br>Click the group lock icon to lock/unlock the group; locking the outlets of
			a group prevents accidental changes to their state. A group is Unlocked if
			at least one outlet in the group is unlocked. If the group is Unlocked but
			there is at least one outlet in the group that is locked, then clicking the
			group padlock icon will turn all outlets Unlocked.
			<br>The sequence of outlets affects Power ON and Cycle operations. You can change
			the sequence of outlets by changing the definition order in:
			<br>Ports | Physical Ports | Modify | Power Management<br>
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
