<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Help/Temperature Sensor</title>
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
    Access | Temperature Sensor </td>
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
		  You can use the Temperature Sensor form to view graphs of temperatures read from any of the <%getLabel("nickname");%>'s three internal temperature sensors. You can reconfigure the graph's display settings, and you can save the settings in a profile for later use.
		</font></div></td>
        </tr>
        <tr> 
          <td width="25" height="40">&nbsp;</td>
          <td width="25" align="center" valign="middle" height="40"> <img src="../../Images/paragHelp.gif" alt="" width="25" height="25" border="0"></td>
          <td align="left" valign="middle" height="40"> <font class="paragHelp">Access</font></td>
        </tr>
        <tr> 
          <td width="25">&nbsp;</td>
          <td width="25" align="center" valign="top">&nbsp;</td>
          <td width="365" align="left" valign="middle"><div align="justify"><font class="textHelp">
		<P>Select either "FPGA" (field programmable gate array), "Power supply" or "CPU" from the pull-down menu and click the "Connect" button.<P>The graph displays a new reading at a specified interval that is shown in the heading on the graph. The graph displays a new reading at 5-second intervals as default.<P> To clear the temperature display and start the plotting again at zero seconds, select "Clear Graph."<P>To exit, click the X box at the upper right of the window.
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
<l1>To Change the Graph</l1>
<ol type=1>
<li>Change any of the parameters using the options and fields
on the left of the Java applet.</li>
<li>Click the "Apply Changes" button.</li>
</ol>

<l1>To Save Changes in a Profile</l1>
<ol type=1>
<li>Select "Save Profile."</li>
The "Enter Filename to Save Profile" dialog box appears.
<li>Enter a profile name in the "File Name" field.</li>
<li>Press OK.</li>
</ol>
<l1>To Apply an Existing Profile</l1>
<ol type=1>
<li>Click "Set Profile from File..</li>
<li>The .Select File to Set Profile From. dialog box appears.</li>
<li>Select the desired profile.s file name from the pulldown menu.</li>
<li>Press OK.</li>
The temperature graph display changes to the format defined in the selected profile.
</ol>
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

