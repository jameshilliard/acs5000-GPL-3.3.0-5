<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
  <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
  <script type="text/javascript">

  <%
    ReqNum = getRequestNumber();
	Min = getVal("system.administration.timeDate.minute");
	Sec = getVal("system.administration.timeDate.second");
  %>

  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
  parent.currentPage = self;
  parent.helpPage = 'helpPages/adminTimeDateHelp.asp';

  var tz = "<%get("system.administration.timeDate.timezone");%>";
  var zlabel = "<%get("system.administration.timeDate.zlabel");%>";
  var localMonth = 0; var localDay = 0; var localYear = 0;
  var localHour = 0; var localMin = 0; var localSec = 0;

  function init()
  {
    continue_verification = true;
    if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
    {
      if (checkLeftMenu('/normal/administration/adminMenu.asp') == 0) {
         parent.menuReady = 0;
         setTimeout('init()', 200);
         return;
      }
      setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
      parent.topMenu.selectItem(parent.menu.topItem);
      parent.menu.selectItem("TIME");
      showLayers();
      if (!tz)
        alert("Your current timezone configuration is based on the old method (/etc/TIMEZONE).\n" +
              "If you select a timezone from this page you will start\n" +
              "using the new method (/etc/localtime).");
    }
    else
      setTimeout('init()', 200);
  }

  function showLayers() 
  {
    if (document.TimeDateForm.elements[1].value == 1)   
    {      
      show('document', 'Layer1');
      hide('document', 'Layer2');
    }
    else   
    {
      hide('document', 'Layer1');
      show('document', 'Layer2');
    }
  }

  function copyData()
  {
    document.configForm.elements[5].value = document.TimeDateForm.elements[1].value;
    document.configForm.elements[6].value = document.TimeDateForm1.elements[0].value;
    document.configForm.elements[13].value = document.TimeDateForm.elements[0].value;
    if (localYear) {
      //have to check if time info changed after pop-up closed without commit
      if ((document.TimeDateForm2.elements[0].value == localMonth) && 
          (document.TimeDateForm2.elements[1].value == localDay) && 
          (document.TimeDateForm2.elements[2].value == localYear) && 
          (document.TimeDateForm2.elements[3].value == localHour) && 
          (document.TimeDateForm2.elements[4].value == localMin) && 
          (document.TimeDateForm2.elements[5].value == localSec)) {
        document.configForm.elements[9].value = 0;
        return;
      }
    }
    for (var i=0; i<6; i++) {
      document.configForm.elements[i+7].value = document.TimeDateForm2.elements[i].value;
    }
  }

  function form_verification()
  {
    var form_location1;
    var form_location2;

    if (type == 'NN') {
      form_location1 = document.Layer1.document.TimeDateForm1;
      form_location2 = document.Layer2.document.TimeDateForm2;
    } else {
      form_location1 = document.TimeDateForm1;
      form_location2 = document.TimeDateForm2;
    }

    if (document.TimeDateForm.elements[1].value == 1)
    {
      checkElement(form_location1.elements[0], true, false, false, false);
    }
    else
	{
      checkElement(form_location2.elements[0], true, false, false, true, 1, 12); //check month
      if (continue_verification == true)
        checkElement(form_location2.elements[2], true, false, false, true, 1970, 9999); //check year
      if (continue_verification == true)
        switch(form_location2.elements[0].value) { //check day (28, 29, 30 or 31)
          case "4": case "04":
          case "6": case "06":
          case "9": case "09":
          case "11":
            checkElement(form_location2.elements[1], true, false, false, true, 1, 30);
            break;
          case "2": case "02": 
            if (form_location2.elements[2].value%4 == 0 && (form_location2.elements[2].value%100 != 0 || form_location2.elements[2].value%400 == 0))
              //leap-years (February has 29 days) (if yaer can be divided by 4 without remainder but not years with 00 on the end (1900) (exception: if yaer can be divided by 400 without remainder, like 2000)
              checkElement(form_location2.elements[1], true, false, false, true, 1, 29);
            else if (form_location2.elements[2].value%4 != 0 || form_location2.elements[2].value%100 == 0)
              //normal year (February has 28 days)
              checkElement(form_location2.elements[1], true, false, false, true, 1, 28);
            break;
          default:
            checkElement(form_location2.elements[1], true, false, false, true, 1, 31);
        }
      if (continue_verification == true)
        checkElement(form_location2.elements[3], true, false, false, true, 'no', 23); //check hour
      if (continue_verification == true)
        checkElement(form_location2.elements[4], false, false, false, true, 'no', 59); //check minute
      if (continue_verification == true)
        checkElement(form_location2.elements[5], false, false, false, true, 'no', 59); //check second
    }
  }  

  function submit()
  {
    form_verification();
    if (continue_verification == true)
    {
      copyData();
      setQuerySSID(document.configForm);
      document.configForm.submit();
    }
  }

  function callTzPopUp()
  {
    var newRef = "/normal/administration/adminTimezoneEntry.asp?target="+window.name+"&SSID=<%get("_sid_");%>";

    form_verification();
    if (continue_verification == true) {
      localMonth = document.TimeDateForm2.elements[0].value;
      localDay = document.TimeDateForm2.elements[1].value;
      localYear = document.TimeDateForm2.elements[2].value;
      localHour = document.TimeDateForm2.elements[3].value;
      localMin = document.TimeDateForm2.elements[4].value;
      localSec = document.TimeDateForm2.elements[5].value;
      newWindow(newRef, 'name', 450, 300, 'no');
    }
  }

  </script>
</head>

<body onload="init();" class="bodyMain">
<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0" height="40%">
<tr><td align="center" valign="middle">
<table width="450" border="0" align="center" cellpadding="0" cellspacing="5">
<form name="TimeDateForm"  onSubmit="return false;">
	<tr align="center" valign="middle" height="30"> 
	<td align="right"><font class="label">Timezone</font>&nbsp;&nbsp;
	<select name="timezone" size="1" class="formText">
<script type="text/javascript">
if (!tz) {
  document.write('<option name="disable" value="">Old Style</option>');
} else {
  if (tz == "Custom")
    document.write('<option name="Custom" value="Custom">' + zlabel + ': <%get("system.administration.timeDate.tzacronym");%></option>');
  else
    if (zlabel)
      document.write('<option name="Custom" value="Custom">' + zlabel + '</option>');
}
</script>
		<option name="GMT"       value="gmt">GMT</option>
		<option name="GMT-1"   value="gmt-1">GMT 01h West</option>
		<option name="GMT-2"   value="gmt-2">GMT 02h West</option>
		<option name="GMT-3"   value="gmt-3">GMT 03h West</option>
		<option name="GMT-4"   value="gmt-4">GMT 04h West</option>
		<option name="GMT-5"   value="gmt-5">GMT 05h West</option>
		<option name="GMT-6"   value="gmt-6">GMT 06h West</option>
		<option name="GMT-7"   value="gmt-7">GMT 07h West</option>
		<option name="GMT-8"   value="gmt-8">GMT 08h West</option>
		<option name="GMT-9"   value="gmt-9">GMT 09h West</option>
		<option name="GMT-10" value="gmt-10">GMT 10h West</option>
		<option name="GMT-11" value="gmt-11">GMT 11h West</option>
		<option name="GMT-12" value="gmt-12">GMT 12h West</option>
		<option name="GMT+1"   value="gmt+1">GMT 01h East</option>
		<option name="GMT+2"   value="gmt+2">GMT 02h East</option>
		<option name="GMT+3"   value="gmt+3">GMT 03h East</option>
		<option name="GMT+4"   value="gmt+4">GMT 04h East</option>
		<option name="GMT+5"   value="gmt+5">GMT 05h East</option>
		<option name="GMT+6"   value="gmt+6">GMT 06h East</option>
		<option name="GMT+7"   value="gmt+7">GMT 07h East</option>
		<option name="GMT+8"   value="gmt+8">GMT 08h East</option>
		<option name="GMT+9"   value="gmt+9">GMT 09h East</option>
		<option name="GMT+10" value="gmt+10">GMT 10h East</option>
		<option name="GMT+11" value="gmt+11">GMT 11h East</option>
		<option name="GMT+12" value="gmt+12">GMT 12h East</option>
		<option name="GMT+13" value="gmt+13">GMT 13h East</option>
		<option name="GMT+14" value="gmt+14">GMT 14h East</option>
<script type="text/javascript">
if (tz) {
  for (var i=0; i < document.TimeDateForm.elements["timezone"].length; i++) {
    if (document.TimeDateForm.elements["timezone"].options[i].value == tz) {
      document.TimeDateForm.elements["timezone"].options[i].selected = 1;
      break;
    }
  }
}
</script>
	</select></td>
	<td align="left">&nbsp;&nbsp;&nbsp;<a href="javascript:callTzPopUp();"
	class="mainButtons" style="padding: 2px 8px 2px 8px" onfocus="blur()">Edit Custom</a></td>
	</tr>
	<tr align="center" valign="middle" height="30"> 
	<td colspan="2"><font class="label">Network Time Protocol</font>&nbsp;&nbsp;
	<%selected = getVal("system.administration.timeDate.ntp");%>
	<select name="time_protocol" size="1" class="formText" onChange="showLayers();">
		<option value="0" <%if (0 == selected) write("SELECTED");%>> Disable </option>
		<option value="1" <%if (1 == selected) write("SELECTED");%>> Enable </option>
	</select></td></tr>
</form>
</table></td></tr>
</table>

<div id="Layer1" name="enable" style="position:absolute; z-index:1; visibility: hidden;" align="center">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr><td align="center" valign="middle">
<table width="300" border="0" align="center" cellpadding="0" cellspacing="0"  class="colorTRowAlt">
<form name="TimeDateForm1"  onSubmit="return false;">
	<tr><td  align="center" valign="middle"><font class="label">NTP Server</font>&nbsp;&nbsp;
	<input name="ntp_server" type="text" id="ntp_server" maxlength="99" size="20" class="formText" value="<%get("system.administration.timeDate.ntpServer");%>">
	</td></tr>
</form>
</table></td></tr>
</table>
</div>

<div id="Layer2" name="disable" style="position:absolute; z-index:1; visibility: hidden;" align="center">
<table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<tr><td align="center" valign="middle">
<table width="350" border="0" align="center" cellpadding="0" cellspacing="0">
<form name="TimeDateForm2"  onSubmit="return false;">
	<tr align="center" valign="middle"> 
	<td colspan="6" height="20" class="colorTHeader" valign="middle"><font class="colorTHeader">Date</font>
	</td></tr>
	<tr align="center" valign="middle"  class="colorTRowAlt">
		<td width="70" height="40" valign="middle"><font class="label">Month</font></td>
		<td height="40" valign="middle"><input name="month" type="text" align="right" value="<%get("system.administration.timeDate.month");%>" class="formText" size="2" maxlength="2"></td>
		<td width="70" height="40" valign="middle"><font class="label">Day</font></td>
		<td height="40" valign="middle"><input name="day" type="text" align="right" value="<%get("system.administration.timeDate.day");%>" class="formText" size="2" maxlength="2"></td>
		<td width="70" height="40" valign="middle"><font class="label">Year</font></td>
		<td height="40" valign="middle"><input name="year" type="text" align="right" value="<%get("system.administration.timeDate.year");%>" type="text" class="formText" size="4" maxlength="4"></td>
	</tr>
	<tr align="center" valign="middle"> 
    <td colspan="6" height="20"  class="colorTHeader"  valign="middle"><font class="colorTHeader">Time</font>
	</td></tr>
	<tr align="center" valign="middle"  class="colorTRowAlt"> 
		<td width="70" height="40" valign="middle"><font class="label">Hour</font></td>
		<td height="40" valign="middle"><input name="hour" type="text" align="right" value="<%get("system.administration.timeDate.hour");%>" type="text" class="formText" size="2" maxlength="2"></td>
		<td width="70" height="40" valign="middle"><font class="label">Minute</font></td>
		<td height="40" valign="middle"><input name="minute" type="text" align="right" value="<%if (Min < 10) write('0'); write(Min);%>" type="text" class="formText" size="2" maxlength="2"></td>
		<td width="70" height="40" valign="middle"><font class="label">Second</font></td>
		<td height="40" valign="middle"><input name="second" type="text" align="right" value="<%if (Sec < 10) write('0'); write(Sec);%>" class="formText" size="2" maxlength="2"></td>
	</tr>
</form>
</table></td></tr>
</table>
</div>

<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/administration/adminTimeDate.asp">
   <input type="hidden" name="urlError" value="/normal/administration/adminTimeDate.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
<!--**** 5 to 10 *********************************************************************-->
   <input type="hidden" name="system.administration.timeDate.ntp" value="">
   <input type="hidden" name="system.administration.timeDate.ntpServer" value="">
   <input type="hidden" name="system.administration.timeDate.month" value="">
   <input type="hidden" name="system.administration.timeDate.day" value="">
   <input type="hidden" name="system.administration.timeDate.year" value="">
   <input type="hidden" name="system.administration.timeDate.hour" value="">
<!--**** 11 to 13 ********************************************************************-->
   <input type="hidden" name="system.administration.timeDate.minute" value="">
   <input type="hidden" name="system.administration.timeDate.second" value="">
   <input type="hidden" name="system.administration.timeDate.timezone" value="">
<!--**********************************************************************************-->
</form>

<script type="text/javascript">
  parent.mainReady = true;
</script>
  <%set("_sid_","0");%>
</body>
</html>
