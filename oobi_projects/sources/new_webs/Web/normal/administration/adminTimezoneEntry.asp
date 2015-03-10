<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
  <title><%write('Timezone Configuration');%></title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
  <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
  <script type="text/javascript">

  <%
    var gmtoffm = getVal("system.administration.timeDate.GMToffMinutes");
	var savem = getVal("system.administration.timeDate.DSTsaveMinutes");
	var startm = getVal("system.administration.timeDate.DSTstartMin");
	var endm = getVal("system.administration.timeDate.DSTendMin");
  %>

    var splitGMToff;
    var splitSave;
    var splitStart;
    var splitEnd;
	var startWeek = <%get("system.administration.timeDate.DSTstartWeek");%>;
	var startDay = <%get("system.administration.timeDate.DSTstartDay");%>;
	var endWeek = <%get("system.administration.timeDate.DSTendWeek");%>;
	var endDay = <%get("system.administration.timeDate.DSTendDay");%>;
	var issueAlert = 0;

	if (startWeek == 0 && startDay != 0) {
		startWeek = 1;
		startDay = 1;
		endWeek = 1;
		endDay = 1
		issueAlert = 1;
	}

  function form_verification()
  {
    var form_location1 = document.timezoneForm1;
    var form_location2;

    if (type == 'NN')
      form_location2 = document.LayerDST.document.timezoneForm2;
    else
      form_location2 = document.timezoneForm2;

    splitGMToff = form_location1.elements['gmtOff'].value.split(":");
    splitSave = document.timezoneForm2.elements['dstSave'].value.split(":");
    splitStart = document.timezoneForm2.elements['dstStartHour'].value.split(":");
    splitEnd = document.timezoneForm2.elements['dstEndHour'].value.split(":");

    checkElement(form_location1.elements['timezone'], true, false, false, false);
    if (continue_verification == true)
      checkElement(form_location1.elements['stacronym'], true, true, false, false);
    if (continue_verification == true) {
      if (splitGMToff.length != 2) {
        alert('Please check for valid format: [+|-]hh:mm');
		continue_verification = false;
      } else {
        checkElementValue('Hours', splitGMToff[0], true, true, false, true, -23, 23);
        if (continue_verification == true)
           checkElementValue('Minutes', splitGMToff[1], true, true, false, true, 0, 59);
      }
      if (continue_verification != true) {
        form_location1.elements['gmtOff'].focus();
        form_location1.elements['gmtOff'].select();
      }
    }
    if (continue_verification == true) {
	  if (document.timezoneForm1.elements['dstON'].checked)
        checkElement(form_location2.elements['dstacronym'], true, true, false, false);
	}
    if (continue_verification == true) {
      if (splitSave.length != 2) {
        alert('Please check for valid format: [+|-]hh:mm');
		continue_verification = false;
      } else {
        checkElementValue('Hours', splitSave[0], true, true, false, true, -23, 23);
        if (continue_verification == true)
           checkElementValue('Minutes', splitSave[1], true, true, false, true, 0, 59);
      }
      if (continue_verification != true) {
		form_location2.elements['dstSave'].focus();
		form_location2.elements['dstSave'].select();
      }
    }
//    if (continue_verification == true) {
//      switch(parseInt(form_location2.elements['dstStartMonth'].value)) {
//        case 4:
//        case 6:
//        case 9:
//        case 11:
//          checkElementValue('Start day', form_location2.elements['dstStartDay'].value, true, false, false, true, 1, 30);
//          break;
//        case 2: 
//          checkElementValue('Start day', form_location2.elements['dstStartDay'].value, true, false, false, true, 1, 28);
//          break;
//        default:
//          checkElementValue('Start day', form_location2.elements['dstStartDay'].value, true, false, false, true, 1, 31);
//      }
//      if (continue_verification != true)
//        form_location2.elements['dstStartDay'].focus();
//    }
    if (continue_verification == true) {
      if (splitStart.length != 2) {
        alert('Please check for valid format: hh:mm');
		continue_verification = false;
      } else {
        checkElementValue('Hour', splitStart[0], true, true, false, true, 0, 23);
        if (continue_verification == true)
           checkElementValue('Minute', splitStart[1], true, true, false, true, 0, 59);
      }
      if (continue_verification != true) {
		form_location2.elements['dstStartHour'].focus();
		form_location2.elements['dstStartHour'].select();
      }
    }
//    if (continue_verification == true) {
//      switch(parseInt(form_location2.elements['dstEndMonth'].value)) {
//        case 4:
//        case 6:
//        case 9:
//        case 11:
//          checkElementValue('End day', form_location2.elements['dstEndDay'].value, true, false, false, true, 1, 30);
//          break;
//        case 2: 
//          checkElementValue('End day', form_location2.elements['dstEndDay'].value, true, false, false, true, 1, 28);
//          break;
//        default:
//          checkElementValue('End day', form_location2.elements['dstEndDay'].value, true, false, false, true, 1, 31);
//      }
//      if (continue_verification != true)
//        form_location2.elements['dstEndDay'].focus();
//    }
    if (continue_verification == true) {
      if (splitEnd.length != 2) {
        alert('Please check for valid format: hh:mm');
		continue_verification = false;
      } else {
        checkElementValue('Hour', splitEnd[0], true, true, false, true, 0, 23);
        if (continue_verification == true)
           checkElementValue('Minute', splitEnd[1], true, true, false, true, 0, 59);
      }
      if (continue_verification != true) {
		form_location2.elements['dstEndHour'].focus();
		form_location2.elements['dstEndHour'].select();
      }
    }
  }

  function copyData()
  {
    document.configForm.elements[5].value = opener.document.TimeDateForm.elements[1].value;
    document.configForm.elements[6].value = opener.document.TimeDateForm1.elements[0].value;
    document.configForm.elements[7].value = 'Custom';

    document.configForm.elements[8].value = document.timezoneForm1.elements['timezone'].value;
    document.configForm.elements[9].value = document.timezoneForm1.elements['stacronym'].value;
    document.configForm.elements[10].value = splitGMToff[0];
    document.configForm.elements[11].value = splitGMToff[1];
    document.configForm.elements[13].value = document.timezoneForm2.elements['dstacronym'].value;
	if (document.timezoneForm1.elements['dstON'].checked) {
      document.configForm.elements[12].value = 1;
    } else {
      document.configForm.elements[12].value = 0;
      if (document.configForm.elements[13].value == "")
        document.configForm.elements[13].value = document.configForm.elements[9].value;
    }
    document.configForm.elements[14].value = splitSave[0];
    document.configForm.elements[15].value = splitSave[1];
    document.configForm.elements[16].value = document.timezoneForm2.elements['dstStartMonth'].value;
    document.configForm.elements[17].value = document.timezoneForm2.elements['dstStartDay'].value;
    document.configForm.elements[18].value = splitStart[0];
    document.configForm.elements[19].value = splitStart[1];
    document.configForm.elements[20].value = document.timezoneForm2.elements['dstEndMonth'].value;
    document.configForm.elements[21].value = document.timezoneForm2.elements['dstEndDay'].value;
    document.configForm.elements[22].value = splitEnd[0];
    document.configForm.elements[23].value = splitEnd[1];
    document.configForm.elements[24].value = document.timezoneForm2.elements['dstStartWeek'].value;
    document.configForm.elements[25].value = document.timezoneForm2.elements['dstEndWeek'].value;
  }

  function timezoneSubmit()
  {
	try {
    if (opener.window.name != document.configForm.target) {
      alert("The page which called this form is not available anymore. The changes will not be effective.");
      self.close();
      return;
    }
    form_verification();
    if (continue_verification == true) {
      copyData();
      setQuerySSID(document.configForm);
      document.configForm.submit();
      self.close();
    }
	} catch (error) {
	  self.close();
	}
  }

  function showDSTparams()
  {
    if (document.timezoneForm1.elements['dstON'].checked)   
      show('document', 'LayerDST');
    else   
      hide('document', 'LayerDST');
  }

  function prepareDSTpage()
  {
	continue_verification = true;

    document.timezoneForm1.elements[0].focus();
	showDSTparams();
  }

  </script>
</head>

<body onload="prepareDSTpage();" class="bodyMain">
<form name="timezoneForm1" onSubmit="return false;">
<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
<tr><td align="center">        
	<a href="javascript:timezoneSubmit();">
	<img src="/Images/OK_button.gif" width="35" height="21" border="0"></a>          
	&nbsp;&nbsp;&nbsp;&nbsp;
	<a href="javascript:window.close();">
	<img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
	<br><br>
</td></tr>
<tr><td align="center" valign="middle">
<table cellspacing="0" cellpadding="5" border="0">
	<tr><td align="left"><font class="label">Timezone Name</font></td>
		<td align="left" colspan="2"> 
		<input class="formText" type="text" maxlength="15" name="timezone" size="15"
			value="<%get("system.administration.timeDate.zlabel");%>">
		</td>
	</tr>
	<tr><td align="left"><font class="label">Standard Time Acronym</font></td>
		<td align="left">
		<input class="formText" type="text" maxlength="6" name="stacronym" size="6"
			value="<%get("system.administration.timeDate.acronym");%>">
		</td>
		<td align="center"><font class="label">&nbsp;&nbsp;GMT off&nbsp;</font>
		<input class="formText" type="text" maxlength="6" name="gmtOff" size="6" align="right"
			value="<%get("system.administration.timeDate.GMToffHours");%>:<%if (gmtoffm < 10) write('0'); write(gmtoffm);%>">
		</td>
	</tr>
</table></td><tr>
<tr><td align="center"><font class="label"><br>Enable daylight saving time</font>
	<input type="checkbox" <%if (getVal("system.administration.timeDate.DSTactive") != 0)
		write("checked");%> name="dstON" onclick="showDSTparams()"><br><br>
	</td>
</tr>
</table>
</form>

<div id="LayerDST" style="z-index:1; visibility:hidden;" align="center">
<form name="timezoneForm2" onSubmit="return false;">
<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
<tr><td align="center" valign="middle">
<table cellspacing="0" cellpadding="5" border="0">
	<tr><td align="center"><font class="label">DST Acronym</font></td>
		<td align="center">
		<input class="formText" type="text" maxlength="6" name="dstacronym" size="6"
			value="<%get("system.administration.timeDate.DSTacronym");%>">
		</td>
		<td align="right" colspan="2"><font class="label">Save time&nbsp;</font>
		<input class="formText" type="text" maxlength="6" name="dstSave" size="6" align="right"
			value="<%get("system.administration.timeDate.DSTsaveHours");%>:<%if (savem < 10) write('0'); write(savem);%>">
		</td>
	</tr>
	<tr><td align="center"><font class="label">DST start</font></td>
		<td align="center">
		<select class="formText" name="dstStartMonth">
			<option  value="1">Jan</option>
			<option  value="2">Feb</option>
			<option  value="3">Mar</option>
			<option  value="4">Apr</option>
			<option  value="5">May</option>
			<option  value="6">Jun</option>
			<option  value="7">Jul</option>
			<option  value="8">Aug</option>
			<option  value="9">Sep</option>
			<option value="10">Oct</option>
			<option value="11">Nov</option>
			<option value="12">Dec</option>
<script type="text/javascript">
for (var i=0; i < document.timezoneForm2.elements["dstStartMonth"].length; i++) {
  if (document.timezoneForm2.elements["dstStartMonth"].options[i].value ==
      <%get("system.administration.timeDate.DSTstartMonth");%>) {
    document.timezoneForm2.elements["dstStartMonth"].options[i].selected = 1;
    break;
  }
}
</script>
		</select></td>
		<td align="center">
		<select class="formText" name="dstStartWeek">
			<option  value="1"> 1st</option>
			<option  value="2"> 2nd</option>
			<option  value="3"> 3rd</option>
			<option  value="4"> 4th</option>
			<option  value="5"> last</option>
<script type="text/javascript">
for (var i=0; i < document.timezoneForm2.elements["dstStartWeek"].length; i++) {
  if (document.timezoneForm2.elements["dstStartWeek"].options[i].value == startWeek) {
    document.timezoneForm2.elements["dstStartWeek"].options[i].selected = 1;
    break;
  }
}
</script>
		</select></td>
		<td align="center">
		<select class="formText" name="dstStartDay">
			<option  value="1"> Sun</option>
			<option  value="2"> Mon</option>
			<option  value="3"> Tue</option>
			<option  value="4"> Wed</option>
			<option  value="5"> Thu</option>
			<option  value="6"> Fri</option>
			<option  value="7"> Sat</option>
<script type="text/javascript">
for (var i=0; i < document.timezoneForm2.elements["dstStartDay"].length; i++) {
  if (document.timezoneForm2.elements["dstStartDay"].options[i].value == startDay) {
    document.timezoneForm2.elements["dstStartDay"].options[i].selected = 1;
    break;
  }
}
</script>
		</select></td>
		<td align="right"><font class="label">at&nbsp;</font>
		<input class="formText" type="text" maxlength="6" name="dstStartHour" size="5" align="right"
			value="<%get("system.administration.timeDate.DSTstartHour");%>:<%if (startm < 10) write('0'); write(startm);%>">
		</td>
	</tr>
	<tr><td align="center"><font class="label">DST end</font></td>
		<td align="center">
		<select class="formText" name="dstEndMonth">
			<option  value="1">Jan</option>
			<option  value="2">Feb</option>
			<option  value="3">Mar</option>
			<option  value="4">Apr</option>
			<option  value="5">May</option>
			<option  value="6">Jun</option>
			<option  value="7">Jul</option>
			<option  value="8">Aug</option>
			<option  value="9">Sep</option>
			<option value="10">Oct</option>
			<option value="11">Nov</option>
			<option value="12">Dec</option>
<script type="text/javascript">
for (var i=0; i < document.timezoneForm2.elements["dstEndMonth"].length; i++) {
  if (document.timezoneForm2.elements["dstEndMonth"].options[i].value ==
      <%get("system.administration.timeDate.DSTendMonth");%>) {
    document.timezoneForm2.elements["dstEndMonth"].options[i].selected = 1;
    break;
  }
}
</script>
		</select></td>
		<td align="center">
		<select class="formText" name="dstEndWeek">
			<option  value="1"> 1st</option>
			<option  value="2"> 2nd</option>
			<option  value="3"> 3rd</option>
			<option  value="4"> 4th</option>
			<option  value="5"> last</option>
<script type="text/javascript">
for (var i=0; i < document.timezoneForm2.elements["dstEndWeek"].length; i++) {
  if (document.timezoneForm2.elements["dstEndWeek"].options[i].value == endWeek) {
    document.timezoneForm2.elements["dstEndWeek"].options[i].selected = 1;
    break;
  }
}
</script>
		</select></td>
		<td align="center">
		<select class="formText" name="dstEndDay">
			<option  value="1"> Sun</option>
			<option  value="2"> Mon</option>
			<option  value="3"> Tue</option>
			<option  value="4"> Wed</option>
			<option  value="5"> Thu</option>
			<option  value="6"> Fri</option>
			<option  value="7"> Sat</option>
<script type="text/javascript">
for (var i=0; i < document.timezoneForm2.elements["dstEndDay"].length; i++) {
  if (document.timezoneForm2.elements["dstEndDay"].options[i].value == endDay) {
    document.timezoneForm2.elements["dstEndDay"].options[i].selected = 1;
    break;
  }
}
</script>
		</select></td>
		<td align="right"><font class="label">at&nbsp;</font>
		<input class="formText" type="text" maxlength="6" name="dstEndHour" size="5" align="right"
			value="<%get("system.administration.timeDate.DSTendHour");%>:<%if (endm < 10) write('0'); write(endm);%>">
		</td>
	</tr>
</table></td></tr>
</table>
</form>
</div>

<script type="text/javascript">
	if (issueAlert == 1)
		alert("A new way to define daylight saving dates has been implemented.\n" +
			  "It is better because it is more generic!\n" +
			  "Please, redefine your previous dates.");
</script>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target","main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/administration/adminTimeDate.asp">
	<input type="hidden" name="urlError" value="/normal/administration/adminTimeDate.asp">
	<input type="hidden" name="request" value=<%write(getRequestNumber());%>>
<!--**** 5 to 10 *********************************************************************-->
   <input type="hidden" name="system.administration.timeDate.ntp" value="">
   <input type="hidden" name="system.administration.timeDate.ntpServer" value="">
   <input type="hidden" name="system.administration.timeDate.timezone" value="">
   <input type="hidden" name="system.administration.timeDate.zlabel" value="">
   <input type="hidden" name="system.administration.timeDate.acronym" value="">
   <input type="hidden" name="system.administration.timeDate.GMToffHours" value="">
<!--**** 11 to 20 ********************************************************************-->
   <input type="hidden" name="system.administration.timeDate.GMToffMinutes" value="">
   <input type="hidden" name="system.administration.timeDate.DSTactive" value="">
   <input type="hidden" name="system.administration.timeDate.DSTacronym" value="">
   <input type="hidden" name="system.administration.timeDate.DSTsaveHours" value="">
   <input type="hidden" name="system.administration.timeDate.DSTsaveMinutes" value="">
   <input type="hidden" name="system.administration.timeDate.DSTstartMonth" value="">
   <input type="hidden" name="system.administration.timeDate.DSTstartDay" value="">
   <input type="hidden" name="system.administration.timeDate.DSTstartHour" value="">
   <input type="hidden" name="system.administration.timeDate.DSTstartMin" value="">
   <input type="hidden" name="system.administration.timeDate.DSTendMonth" value="">
<!--**** 21 to 25 ********************************************************************-->
   <input type="hidden" name="system.administration.timeDate.DSTendDay" value="">
   <input type="hidden" name="system.administration.timeDate.DSTendHour" value="">
   <input type="hidden" name="system.administration.timeDate.DSTendMin" value="">
   <input type="hidden" name="system.administration.timeDate.DSTstartWeek" value="">
   <input type="hidden" name="system.administration.timeDate.DSTendWeek" value="">
<!--**********************************************************************************-->
</form>
</body>
</html>
