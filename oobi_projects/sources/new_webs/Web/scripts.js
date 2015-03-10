
function doNothing(){
	;
}

var type = "IE";	//Variable used to hold the browser name

BrowserSniffer();

//detects the capabilities of the browser
function BrowserSniffer() {
	if (navigator.userAgent.indexOf("Opera")!=-1 && document.getElementById)
		type="OP";   //Opera
	else if (document.all)
		type="IE";   //Internet Explorer e.g. IE4 upwards
	else if (document.layers)
		type="NN";  //Netscape Communicator 4
	else if (!document.all && document.getElementById)
		type="MO";	//Mozila e.g. Netscape 6 upwards
	else type = "IE";		
}

//***********************************************************************************************
//**************************************begin menu loading script********************************
//***********************************************************************************************

function menuObj(nickname, textItem, xRef, lPos) {
	this.nickname = nickname;
	this.textItem = textItem;
	this.xRef = xRef;
	if (menuObj.arguments.length > 3)
		this.lPos = lPos;
	else
		this.lPos = 0;
	this.selBut = 0;
}

function getPageIndex(nickname, Pages)
{
	for(var i=0; i < Pages.length; i++) {
		if (Pages[i].nickname == nickname) {
			return(i);
		}
	}
	return(-1);
}

function checkLeftMenu(page) {
	if (top.menu.pageName != page){
		if (document.images){
			top.menu.location.replace(page+"?SSID="+top.sid);
		} else {
			top.menu.location.href = page+"?SSID="+top.sid;
		}
		return 0;
	}
	return 1;
}

//***********************************end menu loading script***************************************

//*************************************************************************************************
//**********************************begin mini-wizard popup scripts********************************
//*************************************************************************************************

function newWindow(mypage, myname, w, h, scroll)
{
	var screenH = (screen.height - 100)	
	if (h >= screenH) {
		wint = 20;
		h = screenH;
	} else {
		var wint = (screen.height - h) / 2;
		if (wint < 80) {
			wint = screen.height - h - 80;
		}
	}
	var winl = (screen.width - w) / 2;
	winprops =
		'height='+h+',width='+w+',top='+wint+',left='+winl+
		', scrollbars='+scroll;
	win = window.open(mypage, myname, winprops);
	if (win && parseInt(navigator.appVersion) >= 4) {
		win.window.focus();
	}
}

function getSelectedEdit(popUp,group,width,height,extraArgs,selectName,scroll,val)
{  
	var Current = selectName.selectedIndex;	  
	if (selectName.selectedIndex == -1 || selectName.options[Current].value == '-1') {
		alert('Please select an item to edit.');  
	} else {
		if (!val)
			newRef = popUp + "?" + group + "=" + selectName.selectedIndex;
		else
			newRef = popUp + "?" + group + "=" +
				selectName.options[selectName.selectedIndex].value;
		if ('' != extraArgs)
			newRef += "&" + extraArgs;
		if (scroll != null)
			newWindow(newRef,'name',width,height,scroll);
		else
			newWindow(newRef,'name',width,height,'no');
	}
}

function getSelectedAdd(popUp,group,width,height,extraArgs,scroll)
{
	newRef = popUp + "?" + group + "=-1";
	if ('' != extraArgs)
		newRef += "&" + extraArgs
	if (scroll != null)
		newWindow(newRef,'name',width,height,scroll);
	else
		newWindow(newRef,'name',width,height,'no');
}
      
function getValueSelectedEdit(popUp,group,width,height,extraArgs,selectName,scroll)
{	  
   if (selectName.selectedIndex == -1 || selectName.options[selectName.selectedIndex].value == '-1')
   {
      alert('Please select an item to edit.');  
   }
   else
   {
      newRef = popUp + "?" + group + "=" + selectName.options[selectName.selectedIndex].value;
      if ('' != extraArgs)
         newRef += "&" + extraArgs
	  if (scroll != null)
		 newWindow(newRef,'name',width,height,scroll);
	  else
         newWindow(newRef,'name',width,height,'no');
   }
}

//**************************************end mini-wizard popup script*********************************

//***************************************************************************************************
//**************************************begin form validation scripts********************************
//***************************************************************************************************
	var reAlpha = new RegExp(/^[a-zA-Z]+$/);
	var reNum   = new RegExp(/^[0-9]+$/);
	var reAlNum = new RegExp(/^[0-9a-zA-Z]+$/);
	var reInt   = new RegExp(/^-?[0-9]+$/);

	var reIP        = new RegExp(/^(((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])|((([0-9a-fA-F]{1,4}:){7}(:|[0-9a-fA-F]{1,4})|([0-9a-fA-F]{1,4}:){6}(:([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:){5}(:|(:[0-9a-fA-F]{1,4}){1,2})|([0-9a-fA-F]{1,4}:){4}(:|(:[0-9a-fA-F]{1,4}){1,3})|([0-9a-fA-F]{1,4}:){3}(:|(:[0-9a-fA-F]{1,4}){1,4})|([0-9a-fA-F]{1,4}:){2}(:|(:[0-9a-fA-F]{1,4}){1,5})|([0-9a-fA-F]{1,4}:)(:|(:[0-9a-fA-F]{1,4}){1,6})|:(:|(:[0-9a-fA-F]{1,4}){1,7}))|(([0-9a-fA-F]{1,4}:){5}(:|([0-9a-fA-F]{1,4}:))|([0-9a-fA-F]{1,4}:){4}(:|:([0-9a-fA-F]{1,4}:)?)|([0-9a-fA-F]{1,4}:){3}(:|:([0-9a-fA-F]{1,4}:){1,2})|([0-9a-fA-F]{1,4}:){2}(:|:([0-9a-fA-F]{1,4}:){1,3})|([0-9a-fA-F]{1,4}:)(:|:([0-9a-fA-F]{1,4}:){1,4})|:(:|:([0-9a-fA-F]{1,4}:){1,5}))((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])))$/);
	var reIPv4      = new RegExp(/^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])$/);
	var reIPv4p     = new RegExp(/^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(:([1-9][0-9]{3}|[1-9][0-9]{2}|[1-9][0-9]|[0-9]))?$/);
	var reIPv4m     = new RegExp(/^((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])(\/(3[0-2]|[12][0-9]|[0-9]))$/);
	var reIPv6      = new RegExp(/^((([0-9a-fA-F]{1,4}:){7}(:|[0-9a-fA-F]{1,4})|([0-9a-fA-F]{1,4}:){6}(:([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:){5}(:|(:[0-9a-fA-F]{1,4}){1,2})|([0-9a-fA-F]{1,4}:){4}(:|(:[0-9a-fA-F]{1,4}){1,3})|([0-9a-fA-F]{1,4}:){3}(:|(:[0-9a-fA-F]{1,4}){1,4})|([0-9a-fA-F]{1,4}:){2}(:|(:[0-9a-fA-F]{1,4}){1,5})|([0-9a-fA-F]{1,4}:)(:|(:[0-9a-fA-F]{1,4}){1,6})|:(:|(:[0-9a-fA-F]{1,4}){1,7}))|(([0-9a-fA-F]{1,4}:){5}(:|([0-9a-fA-F]{1,4}:))|([0-9a-fA-F]{1,4}:){4}(:|:([0-9a-fA-F]{1,4}:)?)|([0-9a-fA-F]{1,4}:){3}(:|:([0-9a-fA-F]{1,4}:){1,2})|([0-9a-fA-F]{1,4}:){2}(:|:([0-9a-fA-F]{1,4}:){1,3})|([0-9a-fA-F]{1,4}:)(:|:([0-9a-fA-F]{1,4}:){1,4})|:(:|:([0-9a-fA-F]{1,4}:){1,5}))((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9]))$/);
	var reIPv6p     = new RegExp(/^((([0-9a-fA-F]{1,4}:){7}(:|[0-9a-fA-F]{1,4})|([0-9a-fA-F]{1,4}:){6}(:([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:){5}(:|(:[0-9a-fA-F]{1,4}){1,2})|([0-9a-fA-F]{1,4}:){4}(:|(:[0-9a-fA-F]{1,4}){1,3})|([0-9a-fA-F]{1,4}:){3}(:|(:[0-9a-fA-F]{1,4}){1,4})|([0-9a-fA-F]{1,4}:){2}(:|(:[0-9a-fA-F]{1,4}){1,5})|([0-9a-fA-F]{1,4}:)(:|(:[0-9a-fA-F]{1,4}){1,6})|:(:|(:[0-9a-fA-F]{1,4}){1,7}))|(([0-9a-fA-F]{1,4}:){5}(:|([0-9a-fA-F]{1,4}:))|([0-9a-fA-F]{1,4}:){4}(:|:([0-9a-fA-F]{1,4}:)?)|([0-9a-fA-F]{1,4}:){3}(:|:([0-9a-fA-F]{1,4}:){1,2})|([0-9a-fA-F]{1,4}:){2}(:|:([0-9a-fA-F]{1,4}:){1,3})|([0-9a-fA-F]{1,4}:)(:|:([0-9a-fA-F]{1,4}:){1,4})|:(:|:([0-9a-fA-F]{1,4}:){1,5}))((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9]))(\/(12[0-8]|1[01][0-9]|[1-9][0-9]|[4-9]))?$/);
	var reIPv6P     = new RegExp(/^((([0-9a-fA-F]{1,4}:){7}(:|[0-9a-fA-F]{1,4})|([0-9a-fA-F]{1,4}:){6}(:([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:){5}(:|(:[0-9a-fA-F]{1,4}){1,2})|([0-9a-fA-F]{1,4}:){4}(:|(:[0-9a-fA-F]{1,4}){1,3})|([0-9a-fA-F]{1,4}:){3}(:|(:[0-9a-fA-F]{1,4}){1,4})|([0-9a-fA-F]{1,4}:){2}(:|(:[0-9a-fA-F]{1,4}){1,5})|([0-9a-fA-F]{1,4}:)(:|(:[0-9a-fA-F]{1,4}){1,6})|:(:|(:[0-9a-fA-F]{1,4}){1,7}))|(([0-9a-fA-F]{1,4}:){5}(:|([0-9a-fA-F]{1,4}:))|([0-9a-fA-F]{1,4}:){4}(:|:([0-9a-fA-F]{1,4}:)?)|([0-9a-fA-F]{1,4}:){3}(:|:([0-9a-fA-F]{1,4}:){1,2})|([0-9a-fA-F]{1,4}:){2}(:|:([0-9a-fA-F]{1,4}:){1,3})|([0-9a-fA-F]{1,4}:)(:|:([0-9a-fA-F]{1,4}:){1,4})|:(:|:([0-9a-fA-F]{1,4}:){1,5}))((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9]))(\/(12[0-8]|1[01][0-9]|[1-9][0-9]|[4-9]))$/);
	var reIPv6m     = new RegExp(/^((([0-9a-fA-F]{1,4}:){7}(:|[0-9a-fA-F]{1,4})|([0-9a-fA-F]{1,4}:){6}(:([0-9a-fA-F]{1,4})?)|([0-9a-fA-F]{1,4}:){5}(:|(:[0-9a-fA-F]{1,4}){1,2})|([0-9a-fA-F]{1,4}:){4}(:|(:[0-9a-fA-F]{1,4}){1,3})|([0-9a-fA-F]{1,4}:){3}(:|(:[0-9a-fA-F]{1,4}){1,4})|([0-9a-fA-F]{1,4}:){2}(:|(:[0-9a-fA-F]{1,4}){1,5})|([0-9a-fA-F]{1,4}:)(:|(:[0-9a-fA-F]{1,4}){1,6})|:(:|(:[0-9a-fA-F]{1,4}){1,7}))|(([0-9a-fA-F]{1,4}:){5}(:|([0-9a-fA-F]{1,4}:))|([0-9a-fA-F]{1,4}:){4}(:|:([0-9a-fA-F]{1,4}:)?)|([0-9a-fA-F]{1,4}:){3}(:|:([0-9a-fA-F]{1,4}:){1,2})|([0-9a-fA-F]{1,4}:){2}(:|:([0-9a-fA-F]{1,4}:){1,3})|([0-9a-fA-F]{1,4}:)(:|:([0-9a-fA-F]{1,4}:){1,4})|:(:|:([0-9a-fA-F]{1,4}:){1,5}))((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9]))(\/(12[0-8]|1[01][0-9]|[1-9][0-9]|[0-9]))$/);
	var reNetMask   = new RegExp(/^(((0|1(28|92)|2(24|4(0|8)|5(2|4|5)))(\.0){3})|(255\.(0|1(28|92)|2(24|4(0|8)|5(2|4|5)))(\.0){2})|((255\.){2}(0|1(28|92)|2(24|4(0|8)|5(2|4|5)))\.0)|((255\.){3}(0|1(28|92)|2(24|4(0|8)|5(2|4|5)))))$/);
	var rePPP       = new RegExp(/^(%[ij])$/);
	var reHostName  = new RegExp(/(^[0-9a-zA-Z]{1,2}$|^[0-9a-zA-Z][0-9a-zA-Z\._-]+[0-9a-zA-Z]$)/);
	var reChainName = new RegExp(/^[0-9a-zA-Z][-_0-9a-zA-Z]*?$/);
	var reOID       = new RegExp(/^(\.?[0-9]+)(\.[0-9]+)*?$/);
	var reFill      = new RegExp(/^.+$/);
	var reSpace     = new RegExp(/^[^\s]+$/);
	var reKey       = new RegExp(/(^s:.*?$|^([0-9a-fA-F]+?|([0-9a-fA-F]{4}-)+[0-9a-fA-F]{1,4})$)/);
	var reCtrlKey   = new RegExp(/^\^[a-zA-Z]$/);
	var reFloat = new RegExp(/^[1-9]?[0-9]+(\.[0-9]+)?$/);

function _check(value) {
	return this.re.test(value);
}

function _validate(element) {
	var r = this.check(element.value);
	if (!r) {
		element.focus();
		element.select();
		alert(this.message);
	}
	return r;
}

function _check_ip(value) {
	var ip_mode = getIPmode();
	switch (getIPmode()) {
		case 1:
		case "1":
			return reIPv4.test(value);
		break;
		case 2:
		case "2":
			return reIPv6.test(value);
		break;
		case 3:
		case "3":
			return reIP.test(value);
		break;
		default:
			return reIPv4.test(value);
		break;
	}
}

function _check_ipm(value) {
	var ip_mode = getIPmode();
	switch (getIPmode()) {
		case 1:
		case "1":
			return reIPv4m.test(value);
		break;
		case 2:
		case "2":
			return reIPv6m.test(value);
		break;
		case 3:
		case "3":
			return (reIPv4m.test(value) || reIPv6m.test(value));
		break;
		default:
			return reIPv4m.test(value);
		break;
	}
}

function _check_vpn_id(value) {
	return (/^@[^\s]+$/.test(value) || _check_ip(value));
}

function _check_bound(value, bound) {
	if (bound.length == 2) {
		if ((bound[0] == undefined) && (bound[1] == undefined)) {
			throw "At least one of the bound values must be defined.";
		} else if (bound[0] == undefined) {
			return (value <= bound[1]);
		} else if (bound[1] == undefined) {
			return (bound[0] <= value);
		} else {
			return (bound[0] <=value) && (value <= bound[1]);
		}
	} else {
		throw "An array with the upper and lower bound values is expected.";
	}
}

function _validate_bound(element, bound) {
	var msg;
	var r;
	var val = parseFloat(element.value);
	if (!reFloat.test(element.value)) {
		element.focus();
		element.select();
		msg = "a positive number";
		alert(this.message.replace(/%1/, msg));
		return r;
	}
	r = this.check(val, bound);
	if (!r) {
		element.focus();
		element.select();
		if (bound[1] == undefined) {
			msg = "no lesser than " + bound[0];
		} else if (bound[0] == undefined) {
			msg = "no greater than " + bound[1];
		} else {
			msg = "between " + bound[0] + " and " + bound[1];
		}
		alert(this.message.replace(/%1/, msg));
	}
	return r;
}

function _check_length(value, len) {
	if (len.length == 1) {
		if (len[0] == undefined) {
			throw "The length must be defined.";
		} else {
			return (value.length <= len);
		}
	} else {
		throw "An array with the length is expected.";
	}
}

function _validate_length(element, len) {
	var r = this.check(element.value, len);
	var s = (len==1)? "" : "s"
	if (!r) {
		element.focus();
		element.select();
		alert(this.message.replace(/%1/, len).replace(/%2/, s));
	}
	return r;
}

function validator(id, re, opt, message) {
	this.id = id;
	this.re = re;
	this.opt = opt;
	this.message = message;
	this.validate = _validate;
	this.check = _check;
}

var validators = new Array();

validators["IP"]        = new validator("IP",        reIP,        false, "Invalid IP address.");
	validators["IP"].check = _check_ip;
validators["IPm"]       = new validator("IPm",       null,        false, "This field must have an IP address with bitmask.");
	validators["IPm"].check = _check_ipm;
validators["IPv4"]      = new validator("IPv4",      reIPv4,      false, "Invalid IPv4 address.");
validators["IPv4p"]     = new validator("IPv4p",     reIPv4p,     false, "Invalid IPv4 address.");
validators["IPv4m"]     = new validator("IPv4m",     reIPv4m,     false, "This field must have an IPv4 address with bitmask.");
validators["IPv6"]      = new validator("IPv6",      reIPv6,      false, "Invalid IPv6 address.");
validators["IPv6p"]     = new validator("IPv6p",     reIPv6p,     false, "Invalid IPv6 address.");
validators["IPv6P"]     = new validator("IPv6P",     reIPv6P,     false, "This field must have an IPv6 address with prefix.");
validators["IPv6m"]     = new validator("IPv6m",     reIPv6m,     false, "This field must have an IPv6 address with bitmask.");
validators["NetMask"]   = new validator("NetMask",   reNetMask,   false, "Invalid network mask.");
validators["PPP"]       = new validator("PPP",       rePPP,       false, "Invalid IPv4 address.");
validators["VpnId"]     = new validator("VpnId",     null,        false, "Invalid VPN ID.");
	validators["VpnId"].check = _check_vpn_id;
validators["Fill"]      = new validator("Fill",      reFill,      false, "This field must be filled.");
validators["HostName"]  = new validator("HostName",  reHostName,  false, "Invalid Host name.");
validators["ChainName"] = new validator("ChainName", reChainName, false, "Invalid Chain name.");
validators["OID"]       = new validator("OID",       reOID,       false, "Invalid OID.");
validators["Alias"]     = new validator("Alias",     reHostName,  false, "Invalid Alias.");
validators["Key"]       = new validator("Key",       reKey,       false, "Invalid Encryption Key.");
validators["CtrlKey"]   = new validator("CtrlKey",   reCtrlKey,   false, "This field must have a caret (^) followed by a character.");
validators["Space"]     = new validator("Space",     reSpace,     false, "This field can not have blank spaces.");
validators["Bound"]     = new validator("Bound",     null,        true,  "This field must be %1.");
	validators["Bound"].check = _check_bound;
	validators["Bound"].validate = _validate_bound;
validators["Length"]    = new validator("Length",    null,        true,  "This field can not be more than %1 character%2 long.");
	validators["Length"].check = _check_length;
	validators["Length"].validate = _validate_length;
validators["Alpha"]     = new validator("Alpha",     reAlpha,     false, "This field must have only alphabetic characters.");
validators["Num"]       = new validator("Num",       reNum,       false, "This field must have only numeric characters.");
validators["AlNum"]     = new validator("AlNum",     reAlNum,     false, "This field must have only alphanumeric characters.");

function validate(element, test) {
	var tests = test.split(/,/);
	var i;
	var opts = validate.arguments;
	var opti = 2;
	for (i in tests) {
		var check = /\?$/.test(tests[i]);
		var tests_i = tests[i].match(/(.+?)\??$/)[1];
		var val = validators[tests_i];
		try {
			if (check) {
				if (val.opt) {
					if (opti < opts.length) {
						if (!val.check(element.value, opts[opti++])) {
							return true;
						}
					} else {
						throw "Optional argument is missing.";
					}
				} else if (!val.check(element.value)) {
					return true;
				}
			} else {
				if (val.opt) {
					if (opti < opts.length) {
						if (!val.validate(element, opts[opti++])) {
							return false;
						}
					} else {
						throw "Optional argument is missing.";
					}
				} else if (!val.validate(element)) {
					return false;
				}
			}
		} catch (ex) {
			alert("Unable to validate \"" + tests_i + "\".\n" + ex);
			return false;
		}
	}
	return true;
}

function check(element, test) {
	var tests = test.split(/,/);
	var i;
	for (i in tests) {
		try {
			if (!validators[tests[i]].check(element)) {
				return false;
			}
		} catch (ex) {
			alert("Unable to check \""+tests[i]+"\".\n" + ex);
		}
	}
	return true;
}

function verify(elements) {
	var i;
	for (i in elements) {
		var l = elements[i].length;
		var line = "validate(";
		if (l) {
			var j = 0;
			line += "elements[i][" + j + "]";
			for (j=1; j<l; j++) {
				line += ", elements[i][" + j + "]";
			}
		}
		line += ")";
		if (!eval(line)) {
			return false;
		}
	}
	return true;
}

function isAlpha(c) {
	//if((c < "a" || c > "z") && (c < "A" || c > "Z"))
	//	return false;
	//return true;
	return /[a-zA-Z]/.test(c);
}

function isDigit(c) {
	//if(c < "0" || c > "9")
	//	return false;
	//return true;
	return /[0-9]/.test(c);
}

function isAlNum(c) {
	//return (isAlpha(c) || isDigit(c));
	return /[a-zA-Z0-9]/.test(c);
}

/*function validateIP(what, required, ppp) 
{
   var validChars = '.:0123456789';
   var portMin = 0;
   var portMax = 9999;
   var portLoc = what.indexOf(":");
   var port = parseInt(what.substring(portLoc+1));
   var dots = 0;
   var ports = 0;
   var ipValid = true;

   if (!what)
      return (!required);
	  
   if (ppp && (what == "%i" || what == "%j")) return (true);

   for (var i = 0; i < what.length; i++) 
   {
      var chr = what.substring(i,i+1);
	   
      if (validChars.indexOf(chr) == -1)
         return false;
      if (chr == '.') 
      {
         dots++;
            eval('dot' + dots + ' = ' + i);
       }
	   if (chr == ':') 
      {
         ports++;
           
      }
   }
	
	if (ports == 1)
   {
      if (port < portMin || port > portMax || !port)
         return false;
   }
		
   if (dots != 3)
      return false;
   
   if (what.substring(0,1) == '.' || what.substring(what.length,what.length+1) == '.')
      return false;
   
   ip1 = what.substring(0,dot1);
   
   if (ip1 == '' || ip1 >255)
      return false;
      
   ip2 = what.substring(dot1+1,dot2);
   
   if (ip2 == '' || ip2 >255)
      return false;
     
   ip3 = what.substring(dot2+1,dot3);
   
   if (ip3 == '' || ip3 >255)
      return false;
     
   ip4 = what.substring(dot3+1,what.length+1);
   
   if (ip4 == '' || ip4 >255)
      return false;
   
   if (required && ip1 == 0 && ip2 == 0 && ip3 == 0 && ip4 == 0)
      return false;
      
   return true;
}*/

function validateIP(address, required, ppp) {
	if (!address) {
		return !required;
	} else {
		//var val = new Array("IPv4", "IPv4", "IPv6", "IP");
		//var IPmode = getIPmode();
		//IPmode = (IPmode==undefined)? 0 : IPmode;
		return check(address, "IPv4") || (ppp ? check(address, "PPP") : false);

	}
}

function validateNumeric(what) 
{
   var string = what.value;
   var numValid = true;
   
   var Chars = "0123456789";

   for (var i = 0; i < string.length; i++) 
   {
      if (Chars.indexOf(string.charAt(i)) == -1)
      {
	if (isArray(what) && (typeof(what.type)=="undefined")) {what=what[0];}
	if(what.type=="text"||what.type=="textarea"||what.type=="password") { what.select(); }
	what.focus();
         return false;
      }
   }
   return true;
} 

function validatePassword(pass1, pass2)
{
	if (pass1.value == "") {
		alert("You must enter a password.");
		pass1.focus();
		return false;
	}
	if (pass1.value != pass2.value) {
		alert("Entered passwords did not match.");
		pass1.focus();
		pass2.focus();
		return false;
	}
	return true;
}

function validateStringWithoutBlank(what)
{
	var invalid = " "; // Invalid character is a space

	if (what.value.indexOf(invalid) > -1) {
		alert("Sorry, spaces are not allowed.");
		if (isArray(what) && (typeof(what.type)=="undefined")) {what=what[0];}
		if(what.type=="text"||what.type=="textarea"||what.type=="password") { what.select(); }
		what.focus();
		return false;
	} else {
		return true;
   	}
}

//-------------------------------------------------------------------
// isArray(obj)
// Returns true if the object is an array, else false
//-------------------------------------------------------------------
function isArray(obj){return(typeof(obj.length)=="undefined")?false:true;}

//-------------------------------------------------------------------
// isBlank(value)
//   Returns true if value only contains spaces
//-------------------------------------------------------------------
function isBlank(val)
{
	if(val==null){return true;}
	for(var i=0;i<val.length;i++) {
		if ((val.charAt(i)!=' ')&&(val.charAt(i)!="\t")&&(val.charAt(i)!="\n")&&(val.charAt(i)!="\r")){return false;}
		}
	return true;
}

//-------------------------------------------------------------------
// commifyArray(array)
//   Take an array of values and turn it into a comma-separated string
//-------------------------------------------------------------------
function commifyArray(obj)
{
	var s="";
	if(obj==null||obj.length<=0){return s;}
	for(var i=0;i<obj.length;i++){
		s=s+((s=="")?"":",")+obj[i].toString();
		}
	return s;
}

//-------------------------------------------------------------------
// disallowBlank(input_object[,message[,true]])
//   Checks a form field for a blank value. Optionally alerts if 
//   blank and focuses
//-------------------------------------------------------------------
function disallowBlank(obj)
{
	var msg=(arguments.length>1)?arguments[1]:"";
	var dofocus=(arguments.length>2)?arguments[2]:false;
	if (isBlank(getInputValue(obj))){
		if(!isBlank(msg)){alert(msg);}
		if(dofocus){
			if (isArray(obj) && (typeof(obj.type)=="undefined")) {obj=obj[0];}
			if(obj.type=="text"||obj.type=="textarea"||obj.type=="password") { obj.select(); }
			obj.focus();
			}
		return true;
		}
	return false;
}

//-------------------------------------------------------------------
// getInputValue(input_object)
//   Get the value of any form input field
//   Multiple-select fields are returned as comma-separated values
//   (Doesn't support input types: button,file,reset,submit)
//-------------------------------------------------------------------
function getInputValue(obj) 
{
	var use_default=(arguments.length>1)?arguments[1]:false;
	if (isArray(obj) && (typeof(obj.type)=="undefined")) {
		var values=new Array();
		for(var i=0;i<obj.length;i++){
			var v=getSingleInputValue(obj[i],use_default);
			if(v!=null){values[values.length]=v;}
			}
		return commifyArray(values);
		}
	return getSingleInputValue(obj,use_default);
}

//-------------------------------------------------------------------
// getSingleInputValue(input_object,use_default)
//   Utility function used by others
//-------------------------------------------------------------------
function getSingleInputValue(obj,use_default) 
{
	switch(obj.type){
		case 'radio': case 'checkbox': return(((use_default)?obj.defaultChecked:obj.checked)?obj.value:null);
		case 'text': case 'hidden': case 'textarea': return(use_default)?obj.defaultValue:obj.value;
		case 'password': return((use_default)?null:obj.value);
		case 'select-one':
			if (obj.options==null) { return null; }
			if(use_default){
				var o=obj.options;
				for(var i=0;i<o.length;i++){if(o[i].defaultSelected){return o[i].value;}}
				return o[0].value;
				}
			if (obj.selectedIndex<0){return null;}
			return(obj.options.length>0)?obj.options[obj.selectedIndex].value:null;
		case 'select-multiple': 
			if (obj.options==null) { return null; }
			var values=new Array();
			for(var i=0;i<obj.options.length;i++) {
				if((use_default&&obj.options[i].defaultSelected)||(!use_default&&obj.options[i].selected)) {
					values[values.length]=obj.options[i].value;
					}
				}
			return (values.length==0)?null:commifyArray(values);
		}
	alert("FATAL ERROR: Field type "+obj.type+" is not supported for this function");
	return null;
}

function getMaxLength(formName,fieldType,field)
{         
   if (fieldType == 'kSwVersionLength')
      field.maxLength = 31;
   if (fieldType == 'kHostNameLength')
      field.maxLength = 31;
   if (fieldType == 'kUserNameLength')
      field.maxLength = 31;
   if (fieldType == 'kPasswordLength')
      field.maxLength = 31;
   if (fieldType == 'kIpLength')
      field.maxLength = 21;
   if (fieldType == 'kDomainNameLength')
      field.maxLength = 25;
   if (fieldType == 'kDnsLength')
      field.maxLength = 20;
   if (fieldType == 'kSttyCommandLength')
      field.maxLength = 65;
   if (fieldType == 'kBreakSequenceLength')
      field.maxLength = 31;
   if (fieldType == 'kServerFarmLength')
      field.maxLength = 31;
   if (fieldType == 'kUserListLength')
      field.maxLength = 512;
   if (fieldType == 'kPathLength')
      field.maxLength = 512;             
} 
      
//**************************************end form validation scripts********************************

//*************************************************************************************************
//**************************************begin listbox modification scripts*************************
//*************************************************************************************************

function deleteOption(object,index) {
    object.options[index] = null;
}

function deleteSelOpt(object)
{ 
    var Current = object.selectedIndex;
	if (object.selectedIndex == -1 || object.options[Current].value == '-1')
	{
		alert('Please select an item to delete.');
		return false;
	}
	else
	{
		object.options[Current] = null;
		object.selectedIndex = 0;
		return true;
	}
}

function getValues(table,text)
{         
   var string = '';
   
   for (var i=0;i<table.options.length;i++)
   {
      if (table.options[i].value != -1)
      if (1 == text)
         string += table.options[i].text + ',';
      else
         string += table.options[i].value + ',';
   }
   return string;
}
       
function passSessionID()
{
   this.document.configForm.elements[1].value = window.opener.document.configForm.elements[1].value;

}

function optionUp(what,id) 	 
{  	     
   var Current = what.selectedIndex;	     
   if (what.selectedIndex == -1 || what.options[Current].value == '-1')
   {
   alert('Please select an item.');
   }else if (id != 0)	// id can't be 0, it avoid errors when top of list was reached
   {
   var text1 = what.options[id-1].text;
   var value1 = what.options[id-1].value;
   var text2 = what.options[id].text;
   var value2 = what.options[id].value;
   var newSelected = id-1;   
   what.options[id].text = text1;  
   what.options[id].value = value1; 
   what.options[id-1].text = text2;  
   what.options[id-1].value = value2;
   what.selectedIndex = newSelected;}
}

function optionDown(what,id) 
{  
   var Current = what.selectedIndex;	     
   if (what.selectedIndex == -1 || what.options[Current].value == '-1')
   {
   alert('Please select an item.');
   }else if (id != what.length-2)	// id can't be == what.length-2, it avoid errors when end of list was reached
   // and don't let any option be below option '-1' (option -1 is used for set up option width, this option can not be deleted)  
   {
   var text1 = what.options[id+1].text;
   var value1 = what.options[id+1].value;
   var text2 = what.options[id].text;
   var value2 = what.options[id].value;
   var newSelected = id+1;
   
   what.options[id].text = text1; 
   what.options[id].value = value1;  
   what.options[id+1].text = text2;    
   what.options[id+1].value = value2;
   what.selectedIndex = newSelected;}
}

function addOption(object,text,value) {
    var defaultSelected = true;
    var selected = true;
    var optionName = new Option(text, value, defaultSelected, selected)
    object.options[object.length] = optionName;
}

function copySelected(fromObject,toObject) {
    for (var i=0, l=fromObject.options.length;i<l;i++) {
        if (fromObject.options[i].selected)
            addOption(toObject,fromObject.options[i].text,fromObject.options[i].value);
    }
    for (var i=fromObject.options.length-1;i>-1;i--) {
        if (fromObject.options[i].selected)
            deleteOption(fromObject,i);
    }
}

function copyAll(fromObject,toObject) {
    for (var i=0, l=fromObject.options.length;i<l;i++) {
        addOption(toObject,fromObject.options[i].text,fromObject.options[i].value);
    }
    for (var i=fromObject.options.length-1;i>-1;i--) {
        deleteOption(fromObject,i);
    }
}

//**************************************end listbox modification scripts*****************************



//***************************************************************************************************
//**************************************Begin layer modification scripts******************************
//***************************************************************************************************

function getVisible(id) {
    if (type == 'NN')
        return document.layers[id].visibility == 'show' ? true : false;
    if (type == 'MO')    
        return document.getElementById(id).style.visibility == 'visible' ? true : false;   
    else if (type == 'IE')
        return document.all[document.all.tags('div')[id].id].style.visibility == 'visible' ? true : false;
    }
    
 function getRadioValue(radioGroup) {
	for (var i = 0; i < radioGroup.length; i++) {
		if (radioGroup[i].checked) {
			return radioGroup[i].value
		}	
	}
	return ""
}

function resetAll(doc) {
	for (var i = 0; i < doc.forms.length; i++) {
		doc.forms[i].reset()	
	}
	if (document.layers) {
		for (var j = 0; j < doc.layers.length; j++) {
			resetAll(doc.layers[j].document)
		}		
	}
}

function hide(doc, layerName)
{
   if(type == 'NN')        //NN4+
      eval(doc).layers[layerName].visibility = "hide";
   else if(type == 'MO')          //gecko + IE 5+
      document.getElementById(layerName).style.visibility = "hidden";
   else if(type == 'IE')   // IE 4
      document.all[layerName].style.visibility = "hidden";
}

function show(doc, layerName) 
{
   if(type == 'NN')
      eval(doc).layers[layerName].visibility = "show";
   else if(type == 'MO')          //gecko + IE 5+
      document.getElementById(layerName).style.visibility = "visible";
   else if(type == 'IE')   // IE 4
      document.all[layerName].style.visibility = "visible";
}

function hide2(doc, layerList) {
	var i;
	var layerNames = layerList.split(',');
		
	if (type == 'NN') {
		for (i in layerNames) {
			eval(doc).layers[layerNames[i]].display = "none";
		}
	} else if (type == 'MO') {
		for (i in layerNames) {
			document.getElementById(layerNames[i]).style.display = "none";
		}
	} else if (type == 'IE') {
		for (i in layerNames) {
			document.all[layerNames[i]].style.display = "none";
		}
	}
}

function show2(doc, layerList) {
	var i;
	var layerNames = layerList.split(',');

	if(type == 'NN') {
		for (i in layerNames) {
			eval(doc).layers[layerNames[i]].display = "inline";
		}
	} else if(type == 'MO') {
		for (i in layerNames) {
			document.getElementById(layerNames[i]).style.display = "inline";
		}
	} else if(type == 'IE') {
		for (i in layerNames) {
			document.all[layerNames[i]].style.display = "inline";
		}
	}
}

function getObj(name) {
    if (document.getElementById) {
        return document.getElementById(name).style;
    } else
    if (document.all) {
        return document.all[name].style;
    } else
    if (document.layers) {
        return document.layers[name];
    }
}

function toggleVisibility(option) {

    for (var i=0; i < layers.length; i++) {
		if (layers[i] == arguments[0]) { 
			getObj(layers[i]).visibility = 'visible';
		} else {
			getObj(layers[i]).visibility = 'hidden';
		}
	}
}

function set(What,Value) {

    if (document.layers && document.layers[What] != null) document.layers[What].visibility = Value;
    else if (document.all) eval('document.all.'+What+'.style.visibility ="'+ Value+'"');
}

function clicked(Form,Radio,LayerToHide,LayerToShow) {
    for (var i=0; i<Form[Radio].length; i++) {
        if (Form[Radio][i].checked) {
            set(LayerToHide,'hidden');
            set(LayerToShow,'visible');
        }
    }
}


function showSelectedForm(object,Layer1,Layer2,Layer3) {
	if(object.options[object.selectedIndex].value == "1"){
    	set(Layer1,'visible');
    	set(Layer2,'hidden');
		set(Layer3,'hidden');
		}
	if(object.options[object.selectedIndex].value == "2"){
   		set(Layer1,'hidden');
		set(Layer3,'hidden');
    	set(Layer2,'visible');
		}
	if(object.options[object.selectedIndex].value == "3"){
    	set(Layer3,'visible');
    	set(Layer1,'hidden');
		set(Layer2,'hidden');
		}
}

//**************************************end form modification scripts******************************


//*************************************************************************************************
//**************************************Begin navigation scripts***********************************
//*************************************************************************************************

function CheckAddSSID(url, ssid)
{
	var splitPage;
	var splitSid;

	if (url.search(/SSID=/) >= 0) return url;

	splitSid = ssid.split(";");
    splitPage = url.split("?");

    if (splitPage.length == 1) {
	  return url + "?SSID=" + splitSid[0];
    }
	return splitPage[0] + "?SSID=" + splitSid[0] + "&" + splitPage[1];
}

function setQuerySSID(form)
{

    form.urlOk.value = CheckAddSSID(form.urlOk.value, form.elements[1].value);

    form.urlError.value = CheckAddSSID(form.urlError.value, form.elements[1].value);

}

function submitForm(page, newPage)
{
	var oldUrlOk = page.document.configForm.urlOk.value;

	page.document.configForm.urlOk.value = newPage;
	setQuerySSID(page.document.configForm);
   	page.submit();
	if (!continue_verification || continue_verification != undefined)
		{page.document.configForm.urlOk.value = oldUrlOk}
}

function loadNextPage (newPage)
{
	if (top.mainReady)
	{submitForm(top.currentPage, newPage);}				
}

function loadNextPagePhysP (newPage)
{
   if (top.mainReady) {
      top.currentPage.document.configForm.elements[5].value = "0";
      submitForm(top.currentPage, newPage);
   }
}

//**************************************end navigation scripts******************************

function setUnsavedImage(doc, state)
{
   if (state)
      doc["unsaved"].src = "/Images/unsavedChanges.gif";
   else
      doc["unsaved"].src = "/Images/noUnsaved.gif";
}

function initCheck(checkbox, hiddenField)
{
   if ('0' != hiddenField.value)
      checkbox.checked = true
   else
      checkbox.checked = false
}

function setCheck(hiddenField, checkbox)
{
   if (true == checkbox.checked)
      hiddenField.value = '1';
   else
      hiddenField.value = '0';
}

//**************************************Form validation script***********************************
var continue_verification;

function checkElement(element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old, ppp)
{
	continue_verification = true;
if (completing == true) // completing
{if (element_for_verification.value == '')
		{alert ('Please fill in this field.');
		continue_verification = false;
		element_for_verification.focus();
		element_for_verification.select();}
}

if (white_spaces == true && element_for_verification.value != '') {	// white spaces
	    for (var i=0; i<element_for_verification.value.length; i++)
		{
		if (element_for_verification.value.charAt(i)== ' ')
		{
		alert("Sorry, spaces are not allowed.");
		continue_verification = false;
		element_for_verification.focus();
		element_for_verification.select();
		break;}
		}
}

if (ip == true && element_for_verification.value != '') // ip 
{
    if (ppp == null) ppp = false;
    if (!validateIP(element_for_verification.value, false, ppp)) 
		{
		alert('The IP address you entered is not valid.');
		continue_verification = false;
		element_for_verification.focus();
		element_for_verification.select();}
}

if (numerical == true && element_for_verification.value != '') // numerical
{if (!validateNumeric(element_for_verification))
		{
		alert('Please enter only positive numbers.');
		continue_verification = false;
		element_for_verification.focus();
		element_for_verification.select();
		}
else if (minNum == 'no' || maxNum == 'no' || minNum == undefined || maxNum == undefined)
		{   if ((minNum == 'no' || minNum == undefined) && element_for_verification.value > maxNum)
			{
			alert ('The value should not be more than ' + maxNum + '.');
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
			}
			
			else if ((maxNum == 'no' || maxNum == undefined) && element_for_verification.value < minNum)
			{
			alert ('The value should not be less than ' + minNum + '.');
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
			}
		}
else if ((minNum != 'no' && maxNum != 'no') || (minNum != undefined && maxNum != undefined))					
		{			
			if (element_for_verification.value < minNum || element_for_verification.value > maxNum)
		    {
			alert ('The value should be between ' + minNum +' and ' + maxNum + '.');
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
			}
        }
}

if (password == true && element_for_verification.value != '') // password 
{
        if (element_for_verification.value != password_old) 
		{
		alert('Entered passwords did not match.');
		continue_verification = false;	  
		element_for_verification.focus();
		element_for_verification.select();}
}
}

function checkElementValue(element_name, element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old, ppp)
{
	continue_verification = true;
if (completing == true) // completing
{if (element_for_verification == '')
		{alert ('Please fill in this field.');
		continue_verification = false;
		}
}

if (white_spaces == true && element_for_verification != '') {	// white spaces
	    for (var i=0; i<element_for_verification.length; i++)
		{
		if (element_for_verification.charAt(i)== ' ')
		{
		alert("Sorry, spaces are not allowed.");
		continue_verification = false;
		break;}
		}
}

if (ip == true && element_for_verification != '') // ip 
{
    if (ppp == null) ppp = false;
    if (!validateIP(element_for_verification, false, ppp)) 
		{
		alert('The IP address you entered is not valid.');
		continue_verification = false;
		}
}

if (numerical == true && element_for_verification != '') // numerical
{if (isNaN(parseInt(element_for_verification)))
		{
		alert('Please enter only numbers.');
		continue_verification = false;
		}
else if (minNum == 'no' || maxNum == 'no' || minNum == undefined || maxNum == undefined)
		{   if ((minNum == 'no' || minNum == undefined) && element_for_verification > maxNum)
			{
			alert (element_name + ' should not be more than ' + maxNum + '.');
			continue_verification = false;
			}
			
			else if ((maxNum == 'no' || maxNum == undefined) && element_for_verification < minNum)
			{
			alert (element_name + ' should not be less than ' + minNum + '.');
			continue_verification = false;
			}
		}
else if ((minNum != 'no' && maxNum != 'no') || (minNum != undefined && maxNum != undefined))					
		{			
			if (element_for_verification < minNum || element_for_verification > maxNum)
		    {
			alert (element_name + ' should be between ' + minNum + ' and ' + maxNum + '.');
			continue_verification = false;
			}
        }
}

if (password == true && element_for_verification != '') // password 
{		
        if (element_for_verification != password_old) 
		{
		alert('Entered passwords did not match.');
		continue_verification = false;	  
		}
}
}

function checkPermitedChars (element_for_verification, chars, alertmessage)
{ 
continue_verification = true;
if (element_for_verification.value != '')
  {
   var string = element_for_verification.value;
   for (var i = 0; i < string.length; i++) 
   {
      if (chars.indexOf(string.charAt(i)) == -1)
      {
	  continue_verification = false;
	  alert (alertmessage);
   	  element_for_verification.focus();
   	  element_for_verification.select();
	  break;
	  }
    }
  }   
}

function checkNotPermitedChars (element_for_verification, chars, alertmessage)
{
continue_verification = true;
if (element_for_verification.value != '')
  {
   var string = element_for_verification.value;
   for (var i = 0; i < string.length; i++)
   {
      if (chars.indexOf(string.charAt(i)) != -1)
      {
          continue_verification = false;
          alert (alertmessage);
          element_for_verification.focus();
          element_for_verification.select();
          break;
          }
    }
  }
}

function checkStringChars (element_for_verification, chars, alertmessage)
{ 
continue_verification = true;
if (element_for_verification != '')
  {
   for (var i = 0; i < element_for_verification.length; i++) 
   {
      if (chars.indexOf(element_for_verification.charAt(i)) == -1)
      {
	  continue_verification = false;
	  alert (alertmessage);
	  break;
	  }
    }
  }   
}

function checkHexa (element_for_verification) {
	continue_verification = true;
	var string = element_for_verification.value;
	var cinco = 5;
	for (var i = 1; i <= string.length; i++) {
		if (string.charAt(i-1)=='-') {
			for(var j=0;j<i;j=j+cinco) 
			;	
			if(j!=i) {
				alert('Please type the - separator between each four numbers.');
				continue_verification = false;
				element_for_verification.focus();
				element_for_verification.select();
				break;	
			} 
		}
	}
}

 function checkNetmask (element_for_verification)
 {
 dat = element_for_verification.value;
 index1 = dat.indexOf('.')
 index2 = dat.indexOf('.', index1+1)
 index3 = dat.indexOf('.', index2+1) 
 chars = '1234567890.'
 
 for (var i = 0; i < dat.length; i++)// there are unallowed chars 
   {
     if (chars.indexOf(dat.charAt(i)) == -1)
     {
	  continue_verification = false;
	  alert ('The Network Mask you entered is not valid.');
   	  element_for_verification.focus();
   	  element_for_verification.select();
	  break;
	 }
	 else  continue_verification = true;	  
   }
 if (continue_verification == true)// there aren't points      
    { if (index1 == -1 || index2 == -1 || index3 == -1)
		{ 
	  	continue_verification = false;
	  	alert ('The Network Mask you entered is not valid.');
   	  	element_for_verification.focus();
   	  	element_for_verification.select();
		}
	  else 
		{
		dat_1 = dat.slice(0, index1); 
 		dat_2 = dat.slice(index1+1, index2);
 		dat_3 = dat.slice(index2+1, index3);
 		dat_4 = dat.slice(index3+1, dat.length);
		error = checkMaskError (dat_1, dat_2, dat_3, dat_4) 
			if (error == true)
			{
        	continue_verification = false;
	  		alert ('The Network Mask you entered is not valid.');
   	  		element_for_verification.focus();
   	  		element_for_verification.select();
			}
			else 
			{
			continue_verification = true;
			}		
		//alert (index1+'//'+index2+'//'+index3);
		//alert (dat_1+'//'+dat_2+'//'+dat_3+'//'+dat_4);
		}
	} 	
 }
 
 function checkMaskError (dat_1, dat_2, dat_3, dat_4) {

  if (dat_1 > 255 || dat_1 < 0) {return true;}
  if (dat_2 > 255 || dat_2 < 0) {return true;}
  if (dat_3 > 255 || dat_3 < 0) {return true;}
  if (dat_4 > 255 || dat_4 < 0) {return true;}
  var last_value_before_0 =5;

  if (dat_1 < 255) 
  	{
    if (dat_2 > 0 || dat_3 > 0 || dat_4 > 0) {return true;}
    last_value_before_0 = dat_1;
    } 
  else 
  	{
    if (dat_2 < 255) 
		{
        if(dat_3 > 0 || dat_4 > 0) {return true;}
        last_value_before_0 = dat_2;
        }
	else 
		{
        if (dat_3 < 255) 
			{
          	if ((dat_4 > 0)) {return true;}
	  		last_value_before_0 = dat_3;
        	} 
		else { last_value_before_0 = dat_4; }
      }
    }
  switch (last_value_before_0) {
      case "128":
	  case "192":
	  case "224":
	  case "240":
	  case "248":
	  case "252":
	  case "254":
	  case "255":
      case "0":
        return false;
      default:
        return true;
    } return 0;
  }
  
  function checkGrpMember(element_for_verification, usertable)
{
		var found = 1;
		dat = element_for_verification.value;
		while ((found) && (dat != '')) {
			index1 = dat.indexOf(',');
			if (index1 >= 0) {
				dat_1 = dat.slice(0, index1);
				dat_2 = dat.slice(index1+1, dat.length);
			} else {
				dat_1 = dat;
				dat_2 = '';
			}
			for (var i=0; i<usertable.options.length;i++)
			{
				if (usertable.options[i].text == dat_1) {
					found = 1;
					break;
				} else {
					found = 0;
				}
			}	
			dat = dat_2;
		}
		if (found) {
			continue_verification = true;
		} else {
			alert("Group member entered is invalid");
			continue_verification = false;
		}
}
  function checkBitMask (element_for_verification)
  {			
		dat = element_for_verification.value;
   		chars = '1234567890';
   		var bitmask1 = true;
   		var bitmask2 = true;
 		index1 = dat.indexOf('/')
 		dat_1 = dat.slice(0, index1); 
 		dat_2 = dat.slice(index1+1, dat.length);

    	if (!validateIP(dat_1, false, false) || dat_1 == '') {bitmask1 = false}// check ip in dat_1
	 
    	for (var i = 0; i < dat_2.length; i++)// check numeric in dat_2
    	{
    		if (chars.indexOf(dat_2.charAt(i)) == -1)
     		{bitmask2 = false;
	  		break;}
    	}
		
		if ((dat_2 > 32) || (dat_2 == '')) {bitmask2 = false;}// check number (0-32)
			
		if (bitmask1 == false || bitmask2 == false) 	
			{
			alert('The Network Address you entered is not valid. Correct format xxx.xxx.xxx.xxx/xx');
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
			}
		else {continue_verification = true;}   
  }
  
  function checkMaxLength (text_box, maxi)// for check maxLength in text box
  {
  if (text_box.value.length > maxi)
  {
  alert ("The text you entered shouldn't be more than "+maxi+" characters long.")
  continue_verification = false;
  text_box.focus();
  text_box.select();
  }
  }
 
  function checkOutletName (element_for_verification)
  {
        continue_verification = true;
	inp = element_for_verification.value;
        if (inp != '') {
                for (var i = 0; i < inp.length; i++) {
			if (!isAlNum(inp.charAt(i)) && inp.charAt(i) != '_' && inp.charAt(i) != '-') { 
				continue_verification = false;
                                alert ('Invalid Outlet Name');
                                element_for_verification.focus();
                                element_for_verification.select();
				break;
			}
                }
        }
  }

  function checkSequence (element_for_verification, key)
  {
	var wrongSequence = 0;
   	var chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-=\\][;'/.,"; 
	var modifiers = new Array ('[ALT]', '[CTRL]', '[SHIFT]', '[WIN]');
	var modifiers2 = '$^#*';
	var modifiersset = new Array (0, 0, 0, 0);
	//[SHIFT] cannot be escape sequence only,
	// that means we don't support [SHIFT] or  [SHIFT]K, but for example, 
	//we can support [Alt][SHIFT] or [Alt][SHIFT]k

	var hasShift = 0; 
	var hasCAD = 0; 
	var numOfModifier = 0;
	var pos = 0;
	var len = element_for_verification.value.length;
	var ch, i, j;
	
	var numOfKey = 0;

	if (len == 0) {
		wrongSequence = 1;
	} else {
		ch = element_for_verification.value.charAt(0);
		if (ch == '@') { 
			hasCAD = 1;
			pos++;
		} else if (element_for_verification.value.indexOf("[CTRLALTDEL]") == 0) {
			hasCAD = 1;
			pos += 12;
		}
	}

	while(pos < len) {
		ch = element_for_verification.value.charAt(pos);
		for (i = 0; i < 4; i ++) {
			if (ch == modifiers2.charAt(i)) {
				numOfModifier++;
				if( i== 2)
					hasShift = 1;
				pos ++;
				break;
			}
		}
		if (i == 4) {
			for (i = 0; i < 4; i ++) {
				if (pos + modifiers[i].length > len) {
					continue;
				}
				for (j = 0; j < modifiers[i].length; j ++) {
					if (element_for_verification.value.charAt(pos + j) != modifiers[i].charAt(j)) {
						break;
					}
				}
				if (j == modifiers[i].length) {
					numOfModifier++;
					if(i==2)
						hasShift = 1;
					pos += j;
					break;
				}
			}
		}
		if (i == 4) {
			break;
		} else if (modifiersset[i] != 0) {
			wrongSequence = 1;
			break;
		} else {
			modifiersset[i] = 1;
		}
	}
	if( key == 2){
		if(hasCAD == 1)
			wrongSequence = 2;
		if(numOfModifier==1 && hasShift == 1)
			wrongSequence = 2;
		if (wrongSequence == 0) {
			if (pos == len) {
				wrongSequence = 2;
			} else {
				ch = element_for_verification.value.charAt(pos);
				if (chars.indexOf(ch) == -1) {
					wrongSequence = 2;
				} else {
					pos ++;
				}
			}
		}
	}
	else if( key == 1){
		if(hasCAD == 1)
			wrongSequence = 2;
		if (wrongSequence == 0) {
			ch = element_for_verification.value.charAt(pos);
			if (chars.indexOf(ch) == -1) {
				wrongSequence = 2;
			} else {
				pos ++;
			}
		}

	}
	else if( key == 3){ 
		if(numOfModifier==1 && hasShift == 1)
			wrongSequence = 2;
		while (wrongSequence == 0 && (pos != len)) {
			ch = element_for_verification.value.charAt(pos);
			if (chars.indexOf(ch) == -1) {
				wrongSequence = 2;
			} else {
				pos ++;
				numOfKey ++;
				if(numOfKey > 15)
					wrongSequence = 2;
			}
		}
	}
	else{
	}
	if (wrongSequence == 0 && pos != len) {
		wrongSequence = 3;
	}
	if (wrongSequence != 0) {
		continue_verification = false;
	  	alert ('Wrong key sequence typed.');
   	  	element_for_verification.focus();
   	  	element_for_verification.select();
   	} else {
		continue_verification = true;
	}
  }
  
function checkWrongHost (element_for_verification)
{
	var string = element_for_verification.value;
	// Minimum 2 characters
	if (string.length < 2) {
		return 1;
	}
	// First char alphanumeric 	
	if (string.charAt(0).search(/[0-9a-z]/i) == -1) {
		return 2;
	}
	// Last char alphanumeric
	if (string.charAt(string.length-1).search(/[0-9a-z]/i) == -1) {
		return 3;
	}				
	// accept only alphanumeric characters, hyphen (-) and period (.).		
	for (var i = 0; i < string.length; i++) {
		if (string.charAt(i).search(/\w/) == -1 &&
			string.charAt(i) != '-' && string.charAt(i) != '.') {
			return 4;
		} else if (i == string.length -1) {
			return 0;
		}
	}
}

function checkHostOrIP (element_for_verification)
//  will check host domain name or an IP address 
{
	var string = element_for_verification.value;
	var point = 0;
	var expReg = /[a-z]/i;

	continue_verification = true;
	if (element_for_verification.value != '') {
		for (var i = 0; i < string.length; i++) {
			if (string.charAt(i) == '.') point++;
		}
	
		if (string.search(expReg) == -1 && point >= 3) {
			// check alpha (FQDN must contain at least one alpha)
			// will check IP
			if (!validateIP(element_for_verification.value, false, false)) {
				alert('The IP address you entered is not valid.');
				continue_verification = false;
				element_for_verification.focus();
				element_for_verification.select();
			}
		} else {
			// will check FQDN or HostName
			if (checkWrongHost(element_for_verification) != 0) {
				alert('The Host Name you entered is not valid.');
				continue_verification = false;
				element_for_verification.focus();
				element_for_verification.select();
			}
		}
	}
}
/*  
function checkUsername(elm)
{
	var string = elm.value;
	var i;
	
	if (string.length < 1) {
		return false;
	}
	if (! isAlpha(string.charAt(0))) {
		return false;
	}
	for(i = 1; i < string.length; i++){
		if(! isAlNum(string.charAt(i))) {
			return false;
		}
	}

	if (string == "default") {
       return false;
    }
	return true;
}
*/
/*
Author:Christine Qiu
Date: 11/20/2005
Description: "_-." should be valid in username
*/
function checkUsername(element_for_verification)
{
	var string = element_for_verification.value;
	// Minimum 1 character
	if (string.length < 1) {
		return false;
	}
	// First char alphanumeric
	if (string.charAt(0).search(/[0-9a-z]/i) == -1) {
		return false;
	}
	// Last char alphanumeric
	if (string.charAt(string.length-1).search(/[0-9a-z]/i) == -1) {
		return false;
	}
	// accept alphanumeric chars and .-_
	for (var i = 0; i < string.length - 1; i++) {
		if ((string.charAt(i).search(/\w/) == -1) && 
			(string.charAt(i) != '.') && (string.charAt(i) != '_') &&
			(string.charAt(i) != '-')) {
			return false;
		}
	}
	return true;
}

// Check for range formats: X[,X,...,X][,X+] 
//                          Y1-Y2[,Y3-Y4,...][,X+]
//                          minVal <= {X or Yn} <= maxVal
//                          X+ only if more is true
//                          Yn-Ym only if range is true
function checkRanges (element_for_verification, minitems, maxitems, minVal, maxVal, more, range)
{
	var string = element_for_verification.value;
	continue_verification = true;

	moretxt = "";
	if (more == true)
		moretxt = " and optionally followed by '+'";

	rangetxt = "";
	if (range == true)
		rangetxt = " and/or '-'";

	mintxt = "";
	if (minitems-0 > 0)
		mintxt = "There must have at least " + minitems + " different ports defined in the range.";

	if (element_for_verification.value != '')
	{
		j = 0;
		nitems = 0;
		prev = '-';
		len = string.length;
		var listitems = new Array();
		for (var i = 0; i < len; i++) {
			charAt = string.charAt(i);
			if (charAt == '+' && (more != true)) break;
			if (charAt == '-' && (range != true)) break;
			if (charAt == '+' && (i < (len-1))) break;
			if ((charAt == ',' || charAt == '-') && i >= (len-1)) break;
			if (charAt == '+' || charAt == ',' || charAt == '-') {
				if (i == 0 || i <= j) break;
				num = string.substring(j, i);
				if (num-0 < minVal-0 || num-0 > maxVal-0) break;
				if ((charAt == '+' || charAt == '-') && prev != '-') break;
				if (charAt == '+') {
					j = i+1;
					if ((num-1)+(minitems-nitems) > maxVal) break;
					for (ix = nitems; ix < minitems; ix++) {
						if (++nitems > maxitems) break;
						listitems[nitems-1] = num++;
					}
					if (nitems > maxitems) break;
					continue;
				}
				if (prev != '-') {
					if (num-0 < prev-0 ) break;
					for (ix = 0; ix <= num-prev; ix++) {
						if (++nitems > maxitems) break;
						listitems[nitems-1] = (prev - 0) + ix;
					}
					if (nitems > maxitems) break;
					prev = '-';
				}
				else if (charAt == '-') prev = num;
				else {
					if (++nitems > maxitems) break;
					listitems[nitems-1] = num-0;
				}
				if (nitems > maxitems) break;
				j = i+1;
			} else {
				if (charAt < '0' || charAt > '9') break;
			}
		} 
		if (i < len) {
			if (nitems > maxitems) {
				alert("Too many ports defined in the range.\nPlease configure at most " + maxitems + " ports.");
			} else {
				alert("Invalid Range.\nPlease fix it using only numbers between " + minVal + " and " + maxVal + " delimited by ','" + rangetxt + moretxt + ".\n" + mintxt);
			}
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
			return;
		}
		if (i > j) {
			num = string.substring(j, i);
			if (num-0 < minVal-0 || num-0 > maxVal-0 ||
				(prev != '-' && num-0 < prev-0)) {
				alert("Invalid Range.\nPlease fix it using only numbers between " + minVal + " and " + maxVal + " delimited by ','" + rangetxt + moretxt + ".\n" + mintxt);
				continue_verification = false;
				element_for_verification.focus();
				element_for_verification.select();
				return;
			}
			if (prev != '-') {
				for (ix = 0; ix <= num-prev; ix++) {
					if (++nitems > maxitems) break;
					listitems[nitems-1] = (prev-0)+ix;
				}
			}
			else if (++nitems <= maxitems) {
				listitems[nitems-1] = num-0;
			}
		}
		if (nitems > maxitems) {
			alert("Too many ports defined in the range.\nPlease configure at most " + maxitems + " ports.");
			continue_verification = false;
			element_for_verification.focus();
			element_for_verification.select();
			return;
		}
		for (ix = 0; ix < nitems-1; ix++) {
			for (iy = ix+1; iy < nitems; iy++) {
				if (listitems[ix] == listitems[iy]) {
					alert("There are ports repeated in the range.\n" + mintxt);
					continue_verification = false;
					element_for_verification.focus();
					element_for_verification.select();
					return;
				}
			}
		}
		if (nitems < minitems) {
				alert("Too few ports defined in the range.\n" + mintxt);
				continue_verification = false;
				element_for_verification.focus();
				element_for_verification.select();
				return;
		}
	}
}

function checkPortDevice (element_for_verification)
{
	var string = element_for_verification.value;
	// Minimum 1 character
	if (string.length < 1) {
		return 1;
	}
	// First char alphanumeric
	if (string.charAt(0).search(/[0-9a-z]/i) == -1) {
		return 2;
	}
	// Last char alphanumeric
	if (string.charAt(string.length-1).search(/[0-9a-z]/i) == -1) {
		return 3;
	}
	// accept alphanumeric chars and .-_
	for (var i = 0; i < string.length - 1; i++) {
		if ((string.charAt(i).search(/\w/) == -1) && 
			(string.charAt(i) != '.') && (string.charAt(i) != '_') &&
			(string.charAt(i) != '-')) {
			return 4;
		}
	}
	return 0;
}

function chgSeparator(list)
{
	var newlist;
	var chgsep = true;
	var nopen = 0;
	var nclose = 0;
	for (var i=0; i<list.length; i++) {
		if (list.charAt(i) == ';') {
			alert("Invalid separator: ';'.");
			continue_verification = false;
			break;
		} else if (list.charAt(i) == '[') {
			nopen = nopen + 1;
			if (chgsep == true)
				chgsep = false;
			else {
				alert("Two consecutives '['.");
				continue_verification = false;
				break;
			}
		} else if (list.charAt(i) == ']') {
			nclose = nclose + 1;
			if (chgsep == false)
				chgsep = true;
			else {
				alert("Two consecutives ']'.");
				continue_verification = false;
				break;
			}
		} else if ((chgsep == true) && (list.charAt(i) == ',')) {
			newlist = list.slice(0, i);
			newlist = newlist.concat(';');
			newlist = newlist.concat(list.slice(i+1));
			list = newlist;
		}
	}
	if (continue_verification == true)
		if (nopen != nclose) {
			alert('Not matched brackets.');
			continue_verification = false;
		}
	return list;
}

function findChar(character,word)
{
	for (var x=0; x < word.length; x++) {
		if (word.charAt(x) == character)
			return true;
	}
	return false;
}

function checktty(word)
{
	var type = word.charAt(4);
	if (word.indexOf("!tty") == 0) {
		switch (family_model) {
			case "ONS":
				if (type != 'S' && type != 'A') {
					return false;
				}
			break;
			case "KVM":
				if (type != 'A') {
					return false;
				}
			break;
			default:
				if (type != 'S') {
					return false;
				}
			break;
		}
		var port = word.slice(5);
		var portmax = (type == 'S')? 48 : (family_model == "KVM")? 1 : 2;
		if ((port >= 1) && (port <= portmax))
			return true;
	}
	return false;
}

function validateNameOutIpdu(string,index)
{
	// The following are the rules for IPDU/outlet names: 
	// a - First char alpha
	// b - only alphanumeric and '_'
	if (string.charAt(index).search(/[a-z]/i) == -1) {
		return false;
	}

	for (var i=index+1; i < string.length; i++) {
		if (!isAlNum(string.charAt(i)) && string.charAt(i) != '_' && string.charAt(i) != '-' ) { 
			return false;
                }
        }

	return true;
}

function checkOutlets(input)
{
	var range = input.split(',');
	var Chars = "0123456789";
	var outlist = new Array(129);

	for (j = 0; j < 129; j ++) {
		outlist[j] = 0;
	}
	for (i = 0; i < range.length; i++) {
		if (range[i].length == 0) {
			alert('No outlet number or range entered');
			break;
		}
		var out = range[i].split('-');
		if (out.length != 1 && out.length != 2) {
			alert('Invalid outlet number or range entered: ' + range[i]);
			break;
		}
		for (j = 0; j < out.length; j ++) {
			for (k = 0; k < out[j].length; k ++) {
				if (Chars.indexOf(out[j].charAt(k)) == -1) {
					break;
				}
			}
			if (k < out[j].length) {
				alert('Invalid outlet number: ' + out[j]);
				break;
			}
			if (out[j] < 1) {
				alert ('Outlet is out of range: ' + out[j]); 
				break;
			}
		}
		if (j < out.length) {
			break;
		}
		outletini = parseInt(out[0]);
		if (out.length == 1) {
			outletend = outletini;
		} else {
			if (parseInt(out[1]) < outletini) {
				outletend = outletini;
				outletini = parseInt(out[1]);
			} else {
				outletend = parseInt(out[1]);
			}
		}
		for (j = outletini; j <= outletend; j ++) {
			if (outlist[j-1]) {
				alert('Duplicate outlet in input: ' + range[i]);
				break;
			}
			outlist[j-1] = 1;
		}
		if (j <= outletend) {
			break;
		}
	}
	if (i < range.length) {
		continue_verification = false;
		return false;
	}
	return true;
}

function validateListElements(list)
{
    pieces = list.split(";");
   	result = true;
	for (var x=0; x < pieces.length; x++) {
		if (!findChar("[",pieces[x])) {
			if (pieces[x].charAt(0) == '$') {
           			if (!validateNameOutIpdu(pieces[x],1)) {
					alert('Invalid group name: ' + pieces[x]);
					result = false;
					break;
           			}
				continue;
			}
           		if (!validateNameOutIpdu(pieces[x],0)) {
				alert('Invalid outlet name: ' + pieces[x]);
				result = false;
				break;
           		}
		} else {
			subpieces = pieces[x].split("[");
			if (subpieces.length != 2) {
				alert('Bad element formation: ' + pieces[x]);
				result = false;
				break;
			}
			if (subpieces[0].charAt(0) == '!') {
				if (!checktty(subpieces[0])) {
					alert('Invalid serial port reference: ' + subpieces[0]);
					result = false;
					break;
				}
			} else {
				if (!validateNameOutIpdu(subpieces[0],0)) {
					alert('Invalid pdu name: ' + subpieces[0]);
					result = false;
					break;
				}
			}
			outlets = subpieces[1].split("]");
			if (!checkOutlets(outlets[0])) {
				result = false;
				break;
			}
		}
	}
	if (result == false)
		continue_verification = false;
}


//****************************************end of form validation script***************************

	function _draw_tab() {
		document.write('\
		<td id="' + this.id + '" class="' + this.Color[this.state] + '" align="center" width="' + this.percentage + '" nowrap>\
		');
		if (this.state == 1) {
			document.write('\
			<a href=\'javascript:loadNextPage("' + this.href + '")\' class="tabsTextAlt">\
				<font class="tabsText">' + this.label + '</font>\
			</a>\
			');
		} else {
			document.write('\
			<font class="tabsText">' + this.label + '</font>\
			');
		}
		document.write('\
		</td>');
	}
	
	function _draw_bott() {
		document.write('\
		<td class="' + this.ColorBott[this.state] + '" width="' + this.percentage + '"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		');
	}
	
	function _draw_div() {
		document.write('\
		<td class="bgColor4" align="left" height="20" width="1">\
			<img src="/Images/brownLine.gif" alt="" width="1" height="20" border="0">\
		</td>\
		');
	}
	
	function _draw_div_bott() {
		document.write('\
		<td class="bgColor3" align="center"><img src="/Images/1pixelTrans.gif" alt="" width="1" height="1" border="0"></td>\
		');
	}
	
	function _tab(id, state, label, href, percentage) {
		this.Color       = ["bgColor1", "bgColor4", "bgColor9"];
		this.ColorBott   = ["bgColor1", "bgColor3", "bgColor3"];
		this.id          = id;
		this.state       = state;
		this.label       = label;
		this.href        = href;
		this.percentage  = percentage;
		this.drawTab     = _draw_tab;
		this.drawBott    = _draw_bott;
		this.drawDiv     = _draw_div;
		this.drawDivBott = _draw_div_bott;
	}
	
	function _draw_tabs() {
		document.write('\
		<table id="' + this.id + '" border="0" cellpadding="0" cellspacing="0" width="100%">\
			<tr>\
		');
		for (i=0; i<this.divisions.length; i++) {
			this.divisions[i].drawTab();
			if (i != this.divisions.length - 1) {
				this.divisions[i].drawDiv();
			}
		}
		document.write('\
			</tr>\
			<tr>\
		');
		for (i=0; i<this.divisions.length; i++) {
			this.divisions[i].drawBott();
			if (i != this.divisions.length - 1) {
				this.divisions[i].drawDivBott();
			}
		}
		document.write('\
			</tr>\
		</table>\
		');
	}
	
	function _tabs(id, divisions) {
		this.id        = id;
		this.divisions = divisions;
		this.drawTabs  = _draw_tabs;
	}
	
	function _col(label, data_len, full_len) {
		this.label    = label;
		this.data_len = data_len;
		this.full_len = full_len;
	}
	
	function _draw_sel_table() {
		var i;

		document.write('\
			<table border="0" align="center" cellpadding="0" cellspacing="0" class="tableColor">\
				<tr align="left" valign="bottom">\
					<td>\
						<table border="0" cellpadding="0" cellspacing="0">\
							<tr style="visibility:collapse">\
								<td width="5"></td>\
		');
		
		for (i=0; i<this.cols.length; i++) {
			var j, line = this.space;
			for (j=1; j<this.cols[i].full_len; j++) {
				line += this.space;
			}
			document.write('\
								<td>\
									<font class="formText">'
										+ line +
									'</font>\
								</td>\
			');
		}
		
		document.write('\
							</tr>\
							<tr align="left">\
								<td width="5"></td>\
		');

		for (i=0; i<this.cols.length; i++) {
			document.write('\
								<td><font class="tabsTextBig">' + this.cols[i].label + '</font></td>\
			');
		}
		
		document.write('\
							</tr>\
						</table>\
						<table border="0" cellpadding="0" cellspacing="0">\
							<tr>\
								<td align="left" valign="top">\
									<select name="' + this.name + '" size="' + this.size + '" class="formText"></select>\
								</td>\
							</tr>\
						</table>\
					</td>\
				</tr>\
			</table>\
		');

	}
	
	function _fill_sel_table(opts) {
		var i, len = 0;
		for (i=0; i<this.cols.length; i++) {
			len += this.cols[i].full_len;
		}
		var line = this.space;
		for (i=1; i<len; i++) {
			line += this.space;
		}
		var fields = new Array();
		var o = new Array();
		for (i=0; i<opts.length; i++) {
			if (opts[i] != undefined) {
				var j;
				var find = "/^";
				var repl = "";
				var idx = "";
				for (j=0; j<this.cols.length; j++) {
					var len = (opts[i][j]==undefined)? 0 : opts[i][j].length;
					var field;
					field = (len==0)? "" : ((len > this.cols[i].data_len)? opts[i][j].substr(0, this.cols[i].data_len - 1) + this.ellipsis : opts[i][j]);
					find += "(" + this.space + "{" + field.length + "})(.{" + (this.cols[j].full_len - field.length) + "})";
					repl += field + "$" + (2*(j+1));
					idx += ((len==0)? " " : opts[i][j]);
					if (j!=(this.cols.length-1)) {
						idx += " ";
					}
				}
				find += "/";
				o[i] = [idx, line.replace(eval(find), repl)];
			}
		}
		o[opts.length] = [-1, line];
		fillSelect(document.HostTableForm.table, o);
	}
	
	function selTable(name, cols, size) {
		this.space        = String.fromCharCode(160);
		this.ellipsis     = String.fromCharCode(8230);
		this.name         = name;
		this.cols         = cols;
		this.size         = size;
		this.drawSelTable = _draw_sel_table;
		this.fillSelTable = _fill_sel_table;
	}

	function setIPmode(IPmode) {
		top.oldIPmode = top.IPmode;
		top.IPmode = IPmode;
	}
	
	function getIPmode() {
		if (top.IPmode == undefined) {
			if (opener.top.IPmode == undefined) {
				return 1;
			} else {
				return opener.top.IPmode;
			}
		} else {
			return top.IPmode;
		}
	}

   var disable4 = 
       "Please note that by disabling IPv4 you are enabling IPv6 only. " +
       "The services listed below are not currently supported under IPv6:\n\n" +
       "\t- NIS Authentication,\n" +
       "\t- ISDN Dial Up,\n" +
       "\t- NFS Logging,\n" +
       "\t- Access to Virtual ports.\n\n" +
       "Therefore, Avocent recommends the use of Dual Stack option in case you need any of the above services.";
   var ipmodechg =
       "For the new IP Mode to be completely active, please reboot the ACS appliance after saving configuration.";

	function alertIPmodeChange() {
		if (top.oldIPmode != top.IPmode) {
			switch (parseInt(top.oldIPmode) & parseInt(top.IPmode)) {
				case 0:
					if (parseInt(top.IPmode)==1) {
						//alert("IPv6 to IPv4");
					} else {
						//alert("IPv4 to IPv6");
						alert(disable4);
					}
				break;
				case 1:
					if (parseInt(top.IPmode)==1) {
						//alert("Dual-Stack to IPv4");
					} else {
						//alert("Pv4 to Dual-Stack");
					}
				break;
				case 2:
					if (parseInt(top.IPmode)==2) {
						//alert("Dual-Stack to IPv6");
						alert(disable4);
					} else {
						//alert("IPv6 to Dual-Stack");
					}
				break;
			}
		} else {
			return false;
		}
	}
	
	function IPmodeChange() {
		if (top.oldIPmode != top.IPmode) {
				alert(ipmodechg);
		} else {
			return false;
		}
	}
	
	function moveData(dst, src) {
		var i;
		//i=2 to skip action and SSID
		for (i=2; i<dst.elements.length; i++) {
			var dname = dst.elements[i].name.split(/\./);
			var sname = dname[dname.length - 1];
			if (src.elements[sname] != null && src.elements[sname].type != undefined) {
				dst.elements[i].value = src.elements[sname].value;
			}
		}
	}

	function fillSelect(sel, opts) {

		var o, opt;

		for (o in opts) {
			opt = document.createElement('option');
			opt.value = opts[o][0];
			opt.text  = opts[o][1];
			if (type == 'IE') {
				sel.add(opt);
			} else {
				sel.add(opt, null);
			}
		}

	}

	function fillSelTable(doc, table, opts) {
		var i, len = 0;
		for (i=0; i<table.cols.length; i++) {
			len += table.cols[i].full_len;
		}
		var line = table.space;
		for (i=1; i<len; i++) {
			line += table.space;
		}
		var o = new Array();
		for (i=0; i<opts.length; i++) {
			if (opts[i] != undefined) {
				var j, g;
				var find = "/^";
				var repl = "";
				var idx = "";
				for (j=0, g=1; j<table.cols.length; j++) {
					var len = (opts[i][j]==undefined)? 0 : opts[i][j].length;
					var field;
					field = (len==0)? "" : ((len > table.cols[j].data_len)? opts[i][j].substr(0, table.cols[j].data_len - 1) + table.ellipsis : opts[i][j]);
					if (field.length) {
						find += "(" + table.space + "{" + field.length + "})";
						repl += field;
						g++;
					}
					if (table.cols[j].full_len - field.length) {
						find += "(.{" + (table.cols[j].full_len - field.length) + "})";
						repl += "$" + g++;
					}
					idx += ((len==0)? " " : opts[i][j]);
					if (j!=(table.cols.length-1)) {
						idx += " ";
					}
				}
				find += "/";
				o[i] = [idx, line.replace(eval(find), repl)];
			}
		}
		o[opts.length] = [-1, line];
		fillSelect(doc.elements[table.name], o);
	}
	
	
