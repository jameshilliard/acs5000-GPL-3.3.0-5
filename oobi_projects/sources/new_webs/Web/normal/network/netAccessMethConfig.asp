<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title>Slot <%if (slot == "0") write('1'); else write('2');%></title>	  
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script type="text/javascript">	

	<%
		var slot = getQueryVal("slot",1);
		ReqNum = getRequestNumber();
		var family_model = getVal('_familymodel_');
		// modem var
		var modemPPP = getVal("system.network.accessMethod.modemPPP",slot);
		var version2 = getVal("system.network.accessMethod.version2",slot);
		var modemGsmLocalIp = getVal("system.network.accessMethod.modemGsmLocalIp", slot);
		var modemGsmRemoteIp = getVal("system.network.accessMethod.modemGsmRemoteIp", slot);
		var callback = getVal("system.network.accessMethod.callback");
		var callbackNum = getVal("system.network.accessMethod.callbackNum");
		var pin = getVal("system.network.accessMethod.pin", slot);
		var speed = getVal("system.network.accessMethod.speed", slot);
		var addinit = getVal("system.network.accessMethod.addinit", slot);
		var OTP = getVal("system.network.accessMethod.OTP");

		// ISDN
		var isdnLocalIp = getVal("system.network.accessMethod.isdnLocalIp");
		var isdnRemoteIp = getVal("system.network.accessMethod.isdnRemoteIp");
		var isdnCallback = getVal("system.network.accessMethod.isdnCallback");
		var isdnCallbackNum = getVal("system.network.accessMethod.isdnCallbackNum");

		// LAN / wireless
		var ethIpAddr = getVal("system.network.accessMethod.ethIpAddr");
		var ethNetMask = getVal("system.network.accessMethod.ethNetMask");
		var essid = getVal("system.network.accessMethod.essid", slot);
		var channel = getVal("system.network.accessMethod.channel", slot);
		var encryption = getVal("system.network.accessMethod.encryption",slot);
		var key = getVal("system.network.accessMethod.key",slot);

		// Compact Flash / Hard Disk
		var cfEnable = getVal("system.network.accessMethod.cfEnable",slot);
		var cfDataBuf = getVal("system.network.accessMethod.cfDataBuf");
		
		var IPv6 = getVal('_IPv6_');
		var input_size = 15;
		var input_maxlength = 15;
		if (IPv6) {
			input_size = 25;
			input_maxlength = 50;
		}
	%>

	var family_model = "<%write(family_model);%>";
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

	var original_values = new Array();
	original_values["modemPPP"] =
		[ "system.network.accessMethod.modemPPP_<%write(slot);%>", <%write(modemPPP);%> ];
	original_values["pin"] =
		[ "system.network.accessMethod.pin_<%write(slot);%>", "<%write(pin);%>" ];
	original_values["modemGsmLocalIp"] =
		[ "system.network.accessMethod.modemGsmLocalIp_<%write(slot);%>", "<%write(modemGsmLocalIp);%>" ];
	original_values["modemGsmRemoteIp"] =
		[ "system.network.accessMethod.modemGsmRemoteIp_<%write(slot);%>", "<%write(modemGsmRemoteIp);%>" ];
	original_values["callback"] =
		[ "system.network.accessMethod.callback", <%write(callback);%> ];
	original_values["callbackNum"] =
		[ "system.network.accessMethod.callbackNum", "<%write(callbackNum);%>" ];
	original_values["speed"] =
		[ "system.network.accessMethod.speed_<%write(slot);%>", <%write(speed);%> ];
	original_values["addinit"] =
		[ "system.network.accessMethod.addinit_<%write(slot);%>", "<%write(addinit);%>" ];
	original_values["OTP"] =
		[ "system.network.accessMethod.OTP", <%write(OTP);%> ];
	original_values["isdnLocalIp"] =
		[ "system.network.accessMethod.isdnLocalIp", "<%write(isdnLocalIp);%>" ];
	original_values["isdnRemoteIp"] =
		[ "system.network.accessMethod.isdnRemoteIp", "<%write(isdnRemoteIp);%>" ];
	original_values["isdnCallback"] =
		[ "system.network.accessMethod.isdnCallback", <%write(isdnCallback);%> ];
	original_values["isdnCallbackNum"] =
		[ "system.network.accessMethod.isdnCallbackNum", "<%write(isdnCallbackNum);%>" ];
	original_values["ethIpAddr"] =
		[ "system.network.accessMethod.ethIpAddr", "<%write(ethIpAddr);%>" ];
	original_values["ethNetMask"] =
		[ "system.network.accessMethod.ethNetMask", "<%write(ethNetMask);%>" ];
	original_values["key"] =
		[ "system.network.accessMethod.key_<%write(slot);%>", "<%write(key);%>" ];
	original_values["essid"] =
		[ "system.network.accessMethod.essid_<%write(slot);%>", "<%write(essid);%>" ];
	original_values["channel"] =
		[ "system.network.accessMethod.channel_<%write(slot);%>", "<%write(channel);%>" ];
	original_values["encryption"] =
		[ "system.network.accessMethod.encryption_<%write(slot);%>", <%write(encryption);%> ];
	original_values["cfEnable"] =
		[ "system.network.accessMethod.cfEnable_<%write(slot);%>", <%write(cfEnable);%> ];
	original_values["cfDataBuf"] =
		[ "system.network.accessMethod.cfDataBuf", <%write(cfDataBuf);%> ];

	var version2 = "<% write(version2); %>";
	
	var divShowHide = [
		[ "option0,option_OTP", "option1,option2,option3,option4,option5,option6" ],
		[ "option1", "option0,option2,option3,option4,option5,option6,option_OTP" ],
		[ "option2,option_OTP", "option0,option1,option3,option4,option5,option6" ],
		[ "option3", "option0,option1,option2,option4,option5,option6,option_OTP" ],
		[ "option4", "option0,option1,option2,option3,option5,option6,option_OTP" ],
		[ "option5", "option0,option1,option2,option3,option4,option6,option_OTP" ],
		[ "option6,option_OTP", "option0,option1,option2,option3,option4,option5" ],
	];
	
	var optSpeed = [
		[ "300",    "300"    ],
		[ "1200",   "1200"   ],
		[ "2400",   "2400"   ],
		[ "4800",   "4800"   ],
		[ "9600",   "9600"   ],
		[ "14400",  "14400"  ],
		[ "19200",  "19200"  ],
		[ "28800",  "28800"  ],
		[ "38400",  "38400"  ],
		[ "57600",  "57600"  ],
		[ "76800",  "76800"  ],
		[ "115200", "115200" ],
		[ "230400", "230400" ],
		[ "460800", "460800" ],
		[ "0",      "**"     ],
	];

	var optSelectForm = [
			[0, "Modem"                      ]
	];
	if (family_model != "KVM") {
		optSelectForm = optSelectForm.concat([
			[ 1, "ISDN"                      ],
			[ 2, "GSM"                       ],
			[ 3, "Ethernet"                  ],
			[ 4, "Compact Flash / Hard Disk" ],
			[ 5, "Wireless LAN"              ],
			[ 6, "CDMA"                      ]
		]);
	}

	function setDivStyleDisplay(val, divId) {
		nav = document.getElementById(divId).style;
		nav.display = val;
	}
	
	function fill() {

		fillSelect(document.pcmciaForm.speed, optSpeed);
		for (i=0; i<document.pcmciaForm.speed.options.length; i++) {
			if (document.pcmciaForm.speed.options[i].value == original_values["speed"][1]) {
				document.pcmciaForm.speed.selectedIndex = document.pcmciaForm.speed.options[i].index;
			}
		}

		fillSelect(document.pcmciaForm.selectForm, optSelectForm);
		
		document.pcmciaForm.modemLocalIp.value     = original_values["modemGsmLocalIp"][1];
		document.pcmciaForm.gsmLocalIp.value       = original_values["modemGsmLocalIp"][1];
		document.pcmciaForm.cdmaLocalIp.value      = original_values["modemGsmLocalIp"][1];
		document.pcmciaForm.isdnLocalIp.value      = original_values["isdnLocalIp"][1];

		document.pcmciaForm.modemRemoteIp.value    = original_values["modemGsmRemoteIp"][1];
		document.pcmciaForm.gsmRemoteIp.value      = original_values["modemGsmRemoteIp"][1];
		document.pcmciaForm.cdmaRemoteIp.value     = original_values["modemGsmRemoteIp"][1];
		document.pcmciaForm.isdnRemoteIp.value     = original_values["isdnRemoteIp"][1];
		
		document.pcmciaForm.modemCallback.checked  = (original_values["callback"][1])? true : false;
		document.pcmciaForm.gsmCallback.checked    = (original_values["callback"][1])? true : false;
		document.pcmciaForm.cdmaCallback.checked   = (original_values["callback"][1])? true : false;
		document.pcmciaForm.isdnCallback.checked   = (original_values["isdnCallback"][1])? true : false;

		document.pcmciaForm.modemCallbackNum.value = original_values["callbackNum"][1];
		document.pcmciaForm.gsmCallbackNum.value   = original_values["callbackNum"][1];
		document.pcmciaForm.cdmaCallbackNum.value  = original_values["callbackNum"][1];
		document.pcmciaForm.isdnCallbackNum.value  = original_values["isdnCallbackNum"][1];
		
		document.pcmciaForm.modemPPP.checked       = (original_values["modemPPP"][1])? true : false;
		document.pcmciaForm.pin.value              = original_values["pin"][1];
		document.pcmciaForm.addinit.value          = original_values["addinit"][1];
		document.pcmciaForm.OTP.checked            = (original_values["OTP"][1])? true : false;

		document.pcmciaForm.ethIpAddr.value        = original_values["ethIpAddr"][1];
		document.pcmciaForm.wlanIpAddr.value       = original_values["ethIpAddr"][1];

		document.pcmciaForm.ethNetMask.value       = original_values["ethNetMask"][1];
		document.pcmciaForm.wlanNetMask.value      = original_values["ethNetMask"][1];
		
		document.pcmciaForm.essid.value            = original_values["essid"][1];
		document.pcmciaForm.channel.value          = original_values["channel"][1];
		document.pcmciaForm.encryption.checked     = (original_values["encryption"][1])? true : false;
		document.pcmciaForm.key.value              = original_values["key"][1];
		
		document.pcmciaForm.cfEnable.checked       = (original_values["cfEnable"][1])? true : false;
		document.pcmciaForm.cfDataBuf.checked      = (original_values["cfDataBuf"][1])? true : false;

		showMainLayer(document.pcmciaForm.selectForm);

	}

	function showMainLayer(sel) { 	
		var index = sel.selectedIndex;
		var value = sel.options[index].value;
		if (family_model == "KVM" && value != "0") {
			alert("Invalid Option: " + sel.options[index].text);
			return;
		}
		if ((version2 == "1") && 
			((value == "0") ||
			 (value == "2") ||
			 (value == "6"))) {
			 	hide2('document', "option0,option1,option2,option3,option4,option5,option6,option_OTP");
				alert("The MODEM configuration is under DSView control.");
			return;
		}
		
		show2('document', divShowHide[parseInt(value)][0]);
		hide2('document', divShowHide[parseInt(value)][1]);

		switch(value) {
			case "0": //MODEM
				showLayer(document.pcmciaForm.modemPPP, 'option0_PPP');
				showLayer(document.pcmciaForm.modemCallback, 'option0_CALL');
			break;
			case "1": // ISDN
				showLayer(document.pcmciaForm.isdnCallback, 'option1_CALL');
			break;
			case "2": // GSM
				showLayer(document.pcmciaForm.gsmCallback, 'option2_CALL');
			break;
			case "3": // ETH
				if ((parseInt(getIPmode()) == 2)
				|| ((parseInt(getIPmode()) & 2) && check(document.pcmciaForm.ethIpAddr.value,"IPv6"))) {
					show2('document', "ethPrefixLabel");
					hide2('document', "ethMaskLabel");
				} else {
					show2('document', "ethMaskLabel");
					hide2('document', "ethPrefixLabel");
				}
			break;
			case "4": //FLASH
				showLayer(document.pcmciaForm.cfEnable, 'option4_enable');
			break;
			case "5": // WLAN
				if ((parseInt(getIPmode()) == 2)
				|| ((parseInt(getIPmode()) & 2) && check(document.pcmciaForm.wlanIpAddr.value,"IPv6"))) {
					show2('document', "wlanPrefixLabel");
					hide2('document', "wlanMaskLabel");
				} else {
					show2('document', "wlanMaskLabel");
					hide2('document', "wlanPrefixLabel");
				}
				showLayer(document.pcmciaForm.encryption, 'option5_encrypted');
			break;
			case "6": //CDMA
				showLayer(document.pcmciaForm.cdmaCallback, 'option6_CALL');
			break;
			default:
				alert("Invalid Option:" + sel.options[index].text);
		}
	}
	
	function showMaskPrefixLabel(value) {
		var maskLabel;
		var prefixLabel;
		var sel;
		if (parseInt(getIPmode()) == 2) {
			return;
		}
		switch(parseInt(document.pcmciaForm.selectForm.value)) {
			case 3:
				sel = "eth";
			break;
			case 5:
				sel = "wlan";
			break;
			default:
				return;
		}
		maskLabel   = sel + "MaskLabel"
		prefixLabel = sel + "PrefixLabel"
		if ((parseInt(getIPmode()) & 2) && check(value, "IPv6")) {
			show2('document', prefixLabel);
			hide2('document', maskLabel);
		} else {
			show2('document', maskLabel);
			hide2('document', prefixLabel);
		}
	}
	  
	function showLayer(sel, layerid) {
		if (sel.checked == true)
			setDivStyleDisplay('block', layerid);
		else
			setDivStyleDisplay('none', layerid);
	}
	
	function copyModem() {
		if (document.pcmciaForm.modemPPP.checked == true) {
			document.configForm.elements["system.network.accessMethod.modemPPP_<%write(slot);%>"].value = 1;
			document.configForm.elements["system.network.accessMethod.modemGsmLocalIp_<%write(slot);%>"].value = document.pcmciaForm.modemLocalIp.value;
			document.configForm.elements["system.network.accessMethod.modemGsmRemoteIp_<%write(slot);%>"].value = document.pcmciaForm.modemRemoteIp.value;
			document.configForm.elements["system.network.accessMethod.pin_<%write(slot);%>"].value = document.pcmciaForm.pin.value;
			document.configForm.elements["system.network.accessMethod.speed_<%write(slot);%>"].value = document.pcmciaForm.speed.value;
			document.configForm.elements["system.network.accessMethod.addinit_<%write(slot);%>"].value = document.pcmciaForm.addinit.value;
		} else {
			document.configForm.elements["system.network.accessMethod.modemPPP_<%write(slot);%>"].value = 0;
		}
		if (document.pcmciaForm.modemCallback.checked == true) {
			document.configForm.elements["system.network.accessMethod.callback"].value = 1;
			document.configForm.elements["system.network.accessMethod.callbackNum"].value = document.pcmciaForm.modemCallbackNum.value;
		} else {
			document.configForm.elements["system.network.accessMethod.callback"].value = 0;
		}
		document.configForm.elements["system.network.accessMethod.OTP"].value = document.pcmciaForm.OTP.checked? 1 : 0;
	}

	function copyISDN() {
		document.configForm.elements["system.network.accessMethod.isdnLocalIp"].value = document.pcmciaForm.isdnLocalIp.value;
		document.configForm.elements["system.network.accessMethod.isdnRemoteIp"].value = document.pcmciaForm.isdnRemoteIp.value;
		if (document.pcmciaForm.isdnCallback.checked == true) {
			document.configForm.elements["system.network.accessMethod.isdnCallback"].value = 1;
			document.configForm.elements["system.network.accessMethod.isdnCallbackNum"].value = document.pcmciaForm.isdnCallbackNum.value;
		} else {
			document.configForm.elements["system.network.accessMethod.isdnCallback"].value = 0;
		}
	}

	function copyGSM() {
		document.configForm.elements["system.network.accessMethod.modemPPP_<%write(slot);%>"].value = 1;
		document.configForm.elements["system.network.accessMethod.modemGsmLocalIp_<%write(slot);%>"].value = document.pcmciaForm.gsmLocalIp.value;
		document.configForm.elements["system.network.accessMethod.modemGsmRemoteIp_<%write(slot);%>"].value = document.pcmciaForm.gsmRemoteIp.value;
		document.configForm.elements["system.network.accessMethod.pin_<%write(slot);%>"].value = document.pcmciaForm.pin.value;
		if (document.pcmciaForm.gsmCallback.checked == true) {
			document.configForm.elements["system.network.accessMethod.callback"].value = 1;
			document.configForm.elements["system.network.accessMethod.callbackNum"].value = document.pcmciaForm.gsmCallbackNum.value;
		} else {
			document.configForm.elements["system.network.accessMethod.callback"].value = 0;
		}
		document.configForm.elements["system.network.accessMethod.speed_<%write(slot);%>"].value = document.pcmciaForm.speed.value;
		document.configForm.elements["system.network.accessMethod.addinit_<%write(slot);%>"].value = document.pcmciaForm.addinit.value;
		document.configForm.elements["system.network.accessMethod.OTP"].value = document.pcmciaForm.OTP.checked? 1 : 0;
	}

	function copyCDMA() {
		document.configForm.elements["system.network.accessMethod.modemPPP_<%write(slot);%>"].value = 1;
		document.configForm.elements["system.network.accessMethod.modemCardtype"].value = 2;
		document.configForm.elements["system.network.accessMethod.modemGsmLocalIp_<%write(slot);%>"].value = document.pcmciaForm.cdmaLocalIp.value;
		document.configForm.elements["system.network.accessMethod.modemGsmRemoteIp_<%write(slot);%>"].value = document.pcmciaForm.cdmaRemoteIp.value;
		document.configForm.elements["system.network.accessMethod.speed_<%write(slot);%>"].value = document.pcmciaForm.speed.value;
		document.configForm.elements["system.network.accessMethod.addinit_<%write(slot);%>"].value = document.pcmciaForm.addinit.value;
		if (document.pcmciaForm.cdmaCallback.checked == true) {
			document.configForm.elements["system.network.accessMethod.callback"].value = 1;
			document.configForm.elements["system.network.accessMethod.callbackNum"].value = document.pcmciaForm.cdmaCallbackNum.value;
		} else {
			document.configForm.elements["system.network.accessMethod.callback"].value = 0;
		}
		document.configForm.elements["system.network.accessMethod.pin_<%write(slot);%>"].value = document.pcmciaForm.pin.value;
		document.configForm.elements["system.network.accessMethod.OTP"].value = document.pcmciaForm.OTP.checked? 1 : 0;
	}

	function copyEth() {
		document.configForm.elements["system.network.accessMethod.ethIpAddr"].value = document.pcmciaForm.ethIpAddr.value;
		document.configForm.elements["system.network.accessMethod.ethNetMask"].value = document.pcmciaForm.ethNetMask.value;
	}

	function copyFlash() {
		if (document.pcmciaForm.cfEnable.checked == true) {
			document.configForm.elements["system.network.accessMethod.cfEnable_<%write(slot);%>"].value = 1;
			document.configForm.elements["system.network.accessMethod.cfDataBuf"].value = (document.pcmciaForm.cfDataBuf.checked)? 1 : 0;
		} else {
			document.configForm.elements["system.network.accessMethod.cfEnable_<%write(slot);%>"].value = 0;
		}
	}

	function copyWlan() {
		document.configForm.elements["system.network.accessMethod.ethIpAddr"].value = document.pcmciaForm.wlanIpAddr.value;
		document.configForm.elements["system.network.accessMethod.ethNetMask"].value = document.pcmciaForm.wlanNetMask.value;
		document.configForm.elements["system.network.accessMethod.essid_<%write(slot);%>"].value = document.pcmciaForm.essid.value;
		document.configForm.elements["system.network.accessMethod.channel_<%write(slot);%>"].value = document.pcmciaForm.channel.value;
		if (document.pcmciaForm.encryption.checked == true) {
			document.configForm.elements["system.network.accessMethod.encryption_<%write(slot);%>"].value = 1;
			document.configForm.elements["system.network.accessMethod.key_<%write(slot);%>"].value = document.pcmciaForm.key.value;
		} else {
			document.configForm.elements["system.network.accessMethod.encryption_<%write(slot);%>"].value = 0;
		}
	}

	function form_verification () {

		var form_location = document.pcmciaForm;
		continue_verification = true;
		
		switch (parseInt(form_location.selectForm.value)) {
			case 0: // Modem
				if (form_location.modemPPP.checked) {
					continue_verification = verify([
						[form_location.modemLocalIp,  "Fill,IP"],
						[form_location.modemRemoteIp, "Fill,IP"],
					]);
					if (continue_verification && form_location.modemCallback.checked) {
						continue_verification = verify([
							[form_location.modemCallbackNum, "Fill,Num"],
						]);
					}
				}
			break;
			case 1: // ISDN
				continue_verification = verify([
					[form_location.isdnLocalIp,  "Fill,IP"],
					[form_location.isdnRemoteIp, "Fill,IP"],
				]);
				if (continue_verification && form_location.isdnCallback.checked) {
					continue_verification = verify([
						[form_location.isdnCallbackNum, "Fill,Num"],
					]);
				}
			break;
			case 2: // GSM
				continue_verification = verify([
					[form_location.gsmLocalIp,  "Fill,IP" ],
					[form_location.gsmRemoteIp, "Fill,IP" ],
					[form_location.pin,         "Fill,Num"]
				]);
				if (continue_verification && form_location.gsmCallback.checked) {
					continue_verification = verify([
						[form_location.gsmCallbackNum, "Fill,Num"],
					]);
				}
			break;
			case 3: // Ethernet
				if ((parseInt(getIPmode()) & 2) && check(form_location.ethIpAddr.value,"IPv6")) {
					continue_verification = verify([
						[form_location.ethIpAddr,  "Fill,IPv6"              ],
						[form_location.ethNetMask, "Fill,Num,Bound", [4,128]]
					]);
				} else {
					continue_verification = verify([
						[form_location.ethIpAddr,  "Fill,IPv4"   ],
						[form_location.ethNetMask, "Fill,NetMask"]
					]);
				}
			break;
			case 4: // CF / HD
			break;
			case 5: // WLAN
				if ((parseInt(getIPmode()) & 2) && check(form_location.wlanIpAddr.value,"IPv6")) {
					continue_verification = verify([
						[form_location.wlanIpAddr,  "Fill,IPv6"              ],
						[form_location.wlanNetMask, "Fill,Num,Bound", [4,128]],
						[form_location.essid,       "Fill,Space"  ],
						[form_location.channel,     "Num"         ],
					]);
				} else {
					continue_verification = verify([
						[form_location.wlanIpAddr,  "Fill,IPv4"   ],
						[form_location.wlanNetMask, "Fill,NetMask"],
						[form_location.essid,       "Fill,Space"  ],
						[form_location.channel,     "Num"         ],
					]);
				}
				if (continue_verification && form_location.encryption.checked) {
					continue_verification = verify([
						[form_location.key,         "Fill,Key" ],
					]);
				}
			break;
			case 6: // CDMA
				continue_verification = verify([
					[form_location.cdmaLocalIp,  "Fill,IP"],
					[form_location.cdmaRemoteIp, "Fill,IP"],
				]);
				if (continue_verification && form_location.cdmaCallback.checked) {
					continue_verification = verify([
						[form_location.cdmaCallbackNum, "Fill,Num"],
					]);
				}
			break;
		}
		
		return continue_verification;

	}		

	function copy_original_values() {
		var i;
		for (i in original_values) {
			document.configForm.elements[original_values[i][0]].value = original_values[i][1];
		}
	}

	function submit() {
		try {
			if (opener.window.name != document.configForm.target) {
				alert("The page which called this form is not available anymore. The changes will not be effective.");
				self.close();
				return;
			}
			if (form_verification()) {
				var index = document.pcmciaForm.selectForm.selectedIndex;
				// copy original values
				copy_original_values();
				if (family_model == "KVM" && document.pcmciaForm.selectForm.options[index].value != "0") {
					return;
				}
				switch(document.pcmciaForm.selectForm.options[index].value) {
					case "0" : copyModem(); break;
					case "1" : copyISDN(); break;
					case "2" : copyGSM(); break;
					case "3" : copyEth(); break;
					case "4" : copyFlash(); break;
					case "5" : copyWlan(); break;
					case "6" : copyCDMA(); break;
				}
				document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
				setQuerySSID(document.configForm);
				document.configForm.submit();
				self.close();
			}
		} catch (error) {
			self.close();
		}
	}

	</script>
</head>

<body class="bodyForLayers">
<form name="pcmciaForm" onSubmit="return false;">
<table border="0" cellpadding="0" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="center">   
			<table border="0" cellpadding="0" cellspacing="0" align="center">
				<tr>
					<td align="center" height="30" valign="middle">
						<a href="javascript:submit();"><img src="/Images/OK_button.gif" width="35" height="21" border="0" alt=""></a>
						&nbsp;&nbsp;&nbsp;&nbsp;
						<a href="javascript:window.close();"><img src="/Images/cancelButton.gif" width="47" height="21" border="0" alt=""></a>
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr valign="middle">	
		<td align="center"  height="40" valign="middle">
			<select name="selectForm" onchange="showMainLayer(this)" class="formText">
			</select>
		</td>
	</tr> 
</table>

<div id="option0" style="display: none">
<table border="0" cellpadding="5" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="left" height="50" valign="middle">
			<input type="checkbox" name="modemPPP" id="modemPPP" onclick="showLayer (this, 'option0_PPP');">
				&nbsp;&nbsp;&nbsp;&nbsp;
			<font class="label">PPP</font>
		</td>
	</tr>
</table>
	<div id="option0_PPP" style="display: none">
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr valign="middle">
			<td align="right">
				<font class="label">Local IP</font>
			</td>
			<td>
				<input type="text" class="formText" name="modemLocalIp" id="modemLocalIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
			</td>
		</tr>
		<tr>
			<td align="right">
				<font class="label">Remote IP</font>
			</td>
			<td>
				<input type="text" class="formText" name="modemRemoteIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
			</td>
		</tr>
	</table>
	</div>
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr valign="middle">
			<td align="left" height="50" valign="middle">
				<input type="checkbox" name="modemCallback" id="modemCallback" onclick="showLayer(this,'option0_CALL');">
					&nbsp;&nbsp;&nbsp;&nbsp;
				<font class="label">Call Back</font>
			</td>
		</tr>
	</table>
	<div id="option0_CALL" style="display: none">
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr>
			<td align="right"><font class="label">Phone Number</font></td>
			<td>
				<input type="text" maxlength="19" size="10" class="formText" name="modemCallbackNum">
			</td>
		</tr>
	</table>
	</div>
</div>

<div id="option1" style="display: none">
<table border="0" cellpadding="5" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="right">
			<font class="label">Local IP</font>
		</td>
		<td>
			<input type="text" class="formText" name="isdnLocalIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
		</td>
	</tr>
	<tr>
		<td align="right">
			<font class="label">Remote IP</font>
		</td>
		<td>
			<input type="text" class="formText" name="isdnRemoteIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
		</td>
	</tr>
	<tr>
		<td align="center" colspan="2">
			<input type="checkbox" name="isdnCallback" id="isdnCallback" onclick="showLayer(this,'option1_CALL');">
				&nbsp;&nbsp;&nbsp;&nbsp;
			<font class="label">Call Back</font>
		</td>
	</tr>
</table>
	<div id="option1_CALL" style="display: none">
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr>
			<td align="right">
				<font class="label">Phone Number</font>
			</td>
			<td>
				<input type="text" maxlength="19" size="10" class="formText" name="isdnCallbackNum">
			</td>
		</tr>
	</table>
	</div>
</div>

<div id="option2" style="display: none">
<table border="0" cellpadding="5" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="right">
			<font class="label">Local IP</font>
		</td>
		<td>
			<input type="text" class="formText" name="gsmLocalIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
		</td>
	</tr>
	<tr>
		<td align="right">
			<font class="label">Remote IP</font>
		</td>
		<td>
			<input type="text" class="formText" name="gsmRemoteIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
		</td>
	</tr>
	<tr>
		<td align="right">
			<font class="label">Pin Number</font>
		</td>
		<td>
			<input type="text" size="15" class="formText" name="pin">
		</td>
	</tr>
	<tr>
		<td align="center" colspan="2">
			<input type="checkbox" name="gsmCallback" onclick="showLayer(this,'option2_CALL');">
				&nbsp;&nbsp;&nbsp;&nbsp;
			<font class="label">Call Back</font>
		</td>
	</tr>
</table>
	<div id="option2_CALL" style="display: none">
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr>
			<td align="right">
				<font class="label">Phone Number</font>
			</td>
			<td>
				<input type="text" maxlength="19" size="10" class="formText" name="gsmCallbackNum">
			</td>
		</tr>
	</table>
	</div>
</div>

<div id="option3" style="display: none">
<table border="0" cellpadding="5" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="left">
			<font class="label">IP Address</font>
		</td>
		<td>
			<input type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" name="ethIpAddr" onChange="showMaskPrefixLabel(this.value);">
		</td>
	</tr>
	<tr>
		<td align="left">
			<div id="ethMaskLabel" style="display:none;">
				<font class="label">Network Mask</font>
			</div>
			<div id="ethPrefixLabel" style="display:none;">
				<font class="label">Prefix</font>
			</div>
		</td>
		<td>
			<input type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" name="ethNetMask">
		</td>
	</tr>
</table>
</div>

<div id="option4" style="display: none">
<table border="0" cellpadding="5" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="center">
			<input type="checkbox" name="cfEnable" id="cfEnable" onclick="showLayer(this, 'option4_enable');">
				&nbsp;&nbsp;&nbsp;&nbsp;
			<font class="label">Enable</font>
		</td>
	</tr>
</table>
	<div id="option4_enable" style="display: none">
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr>
			<td align="center">
				<input type="checkbox" name="cfDataBuf" id="cfDataBuf">
					&nbsp;&nbsp;&nbsp;&nbsp;
				<font class="label">Use for data buffering</font>
			</td>
		</tr>
	</table>
	</div>
</div>

<div id="option5" style="display: none">
<table border="0" cellpadding="3" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="left">
			<font class="label">IP Address</font>
		</td>
		<td>
			<input type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" name="wlanIpAddr" onChange="showMaskPrefixLabel(this.value);">
		</td>
	</tr>
	<tr>
		<td align="left">
			<div id="wlanMaskLabel" style="display:none;">
				<font class="label">Network Mask</font>
			</div>
			<div id="wlanPrefixLabel" style="display:none;">
				<font class="label">Prefix</font>
			</div>
		</td>
		<td>
			<input type="text" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>" class="formText" name="wlanNetMask">
		</td>
	</tr>
	<tr>        
		<td align="left">
			<font class="label">MyPrivateNet<br> (ESSID)</font>
		</td>
		<td>
			<input type="text" maxlength="29" size="15" class="formText" name="essid">
		</td>
	</tr> 
	<tr>        
		<td align="left">
			<font class="label">Channel</font>
		</td>
		<td>
			<input type="text" size="15" class="formText" name="channel">
		</td>
	</tr>
	<tr>
		<td align="center" colspan="2">
			<input type="checkbox" name="encryption" id="encryption" onclick="showLayer(this,'option5_encrypted');">
				&nbsp;&nbsp;&nbsp;&nbsp;
			<font class="label">Encrypted </font>
		</td>
	</tr>
</table>
	<div id="option5_encrypted" style="display: none">
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr>
			<td align="right">
				<font class="label">Key</font>
			</td>
			<td>
				<input type="text" maxlength="255" size="20" class="formText" name="key">
			</td>
		</tr>
	</table>
	</div>
</div>

<div id="option6" style="display: none">
<table border="0" cellpadding="5" cellspacing="0" align="center">
	<tr valign="middle">
		<td align="right">
			<font class="label">Local IP</font>
		</td>
		<td>
			<input type="text" class="formText" name="cdmaLocalIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
		</td>
	</tr>
	<tr>
		<td align="right">
			<font class="label">Remote IP</font>
		</td>
		<td>
			<input type="text" class="formText" name="cdmaRemoteIp" maxlength="<%write(input_maxlength);%>" size="<%write(input_size);%>">
		</td>
	</tr>
	<tr valign="top">
		<td align="right">
			<font class="label">Speed</font>
		</td>
		<td align="left">
			<select name="speed" class="formText">
			</select>
		</td>
	</tr>
	<tr>
		<td align="right">
			<font class="label">Additional Initialization</font>
		</td>
		<td>
			<input type="text" maxlength="15" size="15" class="formText" name="addinit">
		</td>
	</tr>
	<tr>
		<td align="center" colspan="2">
			<input type="checkbox" name="cdmaCallback" onclick="showLayer(this,'option6_CALL');">
				&nbsp;&nbsp;&nbsp;&nbsp;
			<font class="label">Call Back</font>
		</td>
	</tr>
</table>
	<div id="option6_CALL" style="display: none">
	<table border="0" cellpadding="5" cellspacing="0" align="center">
		<tr>
			<td align="right">
				<font class="label">Phone Number</font>
			</td>
			<td>
				<input type="text" maxlength="19" size="10" class="formText" name="cdmaCallbackNum">
			</td>
		</tr>
	</table>
	</div>
</div>

<div id="option_OTP" style="display: none">
<table border="0" cellpadding="5" cellspacing="0" align="center">
	<tr>
		<td align="center">
			<font class="label">One Time Password (OTP) Authentication Required</font>
		</td>
		<td>
			<input type="checkbox" name="OTP">
		</td>
	</tr>
</table>
</div>

</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netAccessMeth.asp">
	<input type="hidden" name="urlError" value="/normal/network/netAccessMeth.asp">
	<input type="hidden" name="request" value=<%write(ReqNum);%>>
<!---------------------- element 5-9 below---------------------------------->
	<input type="hidden" name="system.network.accessMethod.action" value="0">
	<input type="hidden" name="system.network.accessMethod.modemPPP_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.pin_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.modemGsmLocalIp_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.modemGsmRemoteIp_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.callback" value="">
	<input type="hidden" name="system.network.accessMethod.callbackNum" value="">
	<input type="hidden" name="system.network.accessMethod.isdnLocalIp" value="">
	<input type="hidden" name="system.network.accessMethod.isdnRemoteIp" value="">
	<input type="hidden" name="system.network.accessMethod.isdnCallback" value="">
	<input type="hidden" name="system.network.accessMethod.isdnCallbackNum" value="">
	<input type="hidden" name="system.network.accessMethod.ethIpAddr" value="">
	<input type="hidden" name="system.network.accessMethod.ethNetMask" value="">
	<input type="hidden" name="system.network.accessMethod.cfEnable_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.cfDataBuf" value="">
	<input type="hidden" name="system.network.accessMethod.key_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.essid_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.channel_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.encryption_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.speed_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.addinit_<%write(slot);%>" value="">
	<input type="hidden" name="system.network.accessMethod.modemCardtype" value="">
	<input type="hidden" name="system.network.accessMethod.OTP" value="">
</form>
<script>
fill();
</script>
</body>
</html>