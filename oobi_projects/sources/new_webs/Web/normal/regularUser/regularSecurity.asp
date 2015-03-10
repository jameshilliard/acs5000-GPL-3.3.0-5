<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<title>Untitled</title>
<link rel="STYLESHEET" type="text/css" href="../../../stylesLayout.css">
<script language="JavaScript" src="../../../scripts.js" type="text/javascript"></script>
<script type="text/javascript">
<%
	var res = getVal("system.regUser.operResult");
	set("system.regUser.operResult",0);
%>	
    
var result = '<% write(res); %>';

parent.currentPage = self;
parent.helpPage = '../helpPages/regularSecurityHelp.asp';

function init()
{
	if (1 == parent.menuReady) {
		checkLeftMenu('regUserMenu.asp');
        parent.menu.selectItem('VIRT');
        parent.topMenu.selectItem(parent.menu.topItem);
		if (result == '1') {
			alert("Success : The password was changed and this operation was reported.");
		} else {
			if (result == '2') {
				alert("Failed : The password is not valid and this operation was reported.");
			} else {
				if (result == '3') {
					alert("Failed : An error occurred updating the password file");
				}
			}
		}	
	} else
		setTimeout('init()', 200);

}

function formVerify()
{
	// old passwrd : need to filled and spaces are not allowed
	checkElement (document.passwdForm.elements[0], true, true, false, false,
			'no','no',false,0);
	if (continue_verification == true) {
		// new passwrd : need to filled, spaces are not allowed
		checkElement (document.passwdForm.elements[1], true, true, false, 
				false,'no','no',false,0);
		if (continue_verification == true) {
			// repeated : need to filled, spaces are not allowed
			//            need to equal to first
			checkElement (document.passwdForm.elements[2], true, true, 
					false, false,'no','no',
					true,document.passwdForm.elements[1].value);
		}
	}
}

function submitPasswd()
{
	formVerify();
	if (continue_verification == true) {
		document.configForm.elements[5].value = document.passwdForm.elements[0].value;
		document.configForm.elements[6].value = document.passwdForm.elements[1].value;
		document.configForm.submit();
		parent.mainReady = false;
	}
}

function submit()
{
	document.configForm.submit();
	parent.mainReady = false;
}

</script>
</head>

<body onload="init();" class="bodyMain">
    <table align="center" width="100%" height="100%" cellspacing="0" cellpadding="0" border="0">
<form name="passwdForm" action="" onSubmit="return false;">
      <tr>
        <td align="center" valign="middle">
          <table border="0" cellpadding="5" cellspacing="0">
            <tr  height="40"  valign="middle">
              <td align="left"><font class="label">Current Password</font></td>
              <td>
                <input class="formText" type="password" maxlength="30" name="currentPassword" size="15" value="">
              </td>
            </tr>
            <tr  height="40"  valign="middle">
              <td align="left"><font class="label">New Password</font></td>
              <td>
                <input class="formText" type="password" maxlength="30" name="newPassword" size="15" value="">
              </td>
            </tr>
            <tr  height="40" valign="middle">
              <td align="left"><font class="label">Repeat <br> New Password</font></td>
              <td>
                <input class="formText" type="password" maxlength="30" name="newPassword2" size="15" value="">
              </td>
            </tr>
            <tr>
              <td colspan="2" align="center"  height="40"  valign="top">
                <a href="javascript:submitPasswd();">
                   <img src="../../Images/OK_button.gif" alt="" width="35" height="21" border="0">
                </a> 
              </td>
            </tr>
          </table>
        </td>
      </tr>
</form>
    </table>

<form name="configForm" method="POST" action="/goform/Dmf">
  <input type="hidden" name="system.req.action" value="">
  <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
  <input type="hidden" name="urlOk" value="/normal/regularUser/regularSecurity.asp">
  <input type="hidden" name="urlError" value="/normal/regularUser/regularSecurity.asp">
  <input type="hidden" name="request" value=<%write(getRequestNumber());%>>
  <input type="hidden" name="system.regUser.passwd.oldPasswd" value="">
  <input type="hidden" name="system.regUser.passwd.newPasswd" value="">
</form>	
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
