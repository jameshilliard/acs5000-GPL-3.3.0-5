<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
    <script language="JavaScript" src="/scripts.js" type="text/javascript">
    </script>
    <script type="text/javascript">

var sid = '<% write(getQueryVal("SSID", "0")); %>';

function changetab(index)
{
	parent.tabx = index;
        if (parent.sniff[index - 1] == '') {
		document.connectForm.action = '/normal/applications/accessConnectPopUpMod.asp';
	} else {
		document.connectForm.action = '/normal/applications/accessConnectPopUpSniff.asp';
	}
	document.connectForm.target = parent.target + 'page';
	document.connectForm.tabx.value = index;
	document.connectForm.SSID.value = sid;
	document.connectForm.submit();
	location.replace('/normal/applications/accessConnectPopUpTab.asp?tabx=' + index + '&SSID=' + sid);
}
    </script>
</head>

<body class="bodyMainTabs" marginheight="0" marginwidth="0">

<script type="text/javascript">

// Maybe it would be faster call it just once and split the result in the array.

   tabx = <%write(getQueryVal("tabx", 0));%>;

   document.write('\
    <table border="0" cellpadding="0" cellspacing="0" width="100%"><tr>');

    for (ix = 0; ix < 4; ix++) {
      if (parent.mystation[ix] == 0) {

        document.write('\
          <td class="bgColor9" align="center" width="25%" height="20" nowrap></td>');

      } else if (parent.mystation[ix] == tabx) {

        document.write('\
          <td class="bgColor1" align="center" width="25%" nowrap>\
          <font class="tabsText">IP ' + parent.mystation[ix] + ' (' + (parent.port[parent.mystation[ix] - 1]) + ')</font>&nbsp;');
	if (parent.status[parent.mystation[ix] - 1]) {
		document.write('<img src="/Images/LEDGreen.gif" alt="Connected" width="10" height="10" border="0">')
	} else {
		document.write('<img src="/Images/LEDRed.gif" alt="Not connected" width="10" height="10" border="0">')
	}
	//if (parent.msg[parent.mystation[ix] - 1] != 'None') {
	//	document.write('&nbsp;<img src="/Images/excl.gif" alt="Event: '+ parent.msg[parent.mystation[ix] - 1] +'" width="10" height="10" border="0">');
	//}
        document.write('</td>\
          <td class="bgColor4" align="left" height="20" width="1">\
          <img src="../../Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>');

      } else {

        document.write('<td class="bgColor4" align="center" width="25%" nowrap>\
            <a href="javascript:changetab(' + parent.mystation[ix] + ');" class="tabsTextAlt">\
            <font class="tabsText">IP ' + parent.mystation[ix] + ' (' + (parent.port[parent.mystation[ix] - 1]) + ')</font></a>&nbsp;');
	if (parent.status[parent.mystation[ix] - 1]) {
		document.write('<img src="/Images/LEDGreen.gif" alt="Connected" width="10" height="10" border="0">')
	} else {
		document.write('<img src="/Images/LEDRed.gif" alt="Not connected" width="10" height="10" border="0">')
	}
	//if (parent.msg[parent.mystation[ix] - 1] != 'None') {
	//	document.write('&nbsp;<img src="/Images/excl.gif" alt="Event: '+ parent.msg[parent.mystation[ix] - 1] +'" width="10" height="10" border="0">');
	//}
        document.write('</td>\
          <td class="bgColor4" align="left" height="20" width="1">\
          <img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0"></td>');

      }
    }

    document.write('</tr><tr>');

    for (ix = 0; ix < 4; ix++) { 
      if (parent.mystation[ix] == 0) {

        document.write('\
	      <td class="bgColor2" width="25%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');
        document.write('\
	      <td class="bgColor2" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');

      } else if (parent.mystation[ix] == tabx) {

        document.write('\
          <td class="bgColor1" width="25%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');
        document.write('\
          <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');

      } else {

        document.write('\
          <td class="bgColor3" width="25%><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');
        document.write('\
	      <td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>');

      }
    }

    document.write('</tr></table>');
</script>

<form name="connectForm">
    <input type="hidden" name="tabx">
    <input type="hidden" name="SSID">
</form>');
</body>
</html>
