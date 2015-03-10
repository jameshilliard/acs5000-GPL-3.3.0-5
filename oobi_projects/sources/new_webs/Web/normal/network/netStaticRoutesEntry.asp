<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>

<head>
	<title>New/Modify Route</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
	<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
	<script language="JavaScript" type="text/javascript">

	<%
		var SaddCall = 0;

		// I'm using sysMsg as a communication channel to the middleware
		set("system.network.staticRoutes.stRoute.sysMsg", getQueryVal("route",-1));

		// This get() makes the appropriate query function to be called in the middleware
		var SrouteType      = getVal("system.network.staticRoutes.stRoute.routeType");
		var SgoTo           = getVal("system.network.staticRoutes.stRoute.goTo");
		var interfaces      = getVal("system.administration.unitInfo.ifInfo.allif");
		var gateWay         = getVal("system.network.staticRoutes.stRoute.gateWay");
		var metric          = getVal("system.network.staticRoutes.stRoute.metric");
		var destinationIp   = getVal("system.network.staticRoutes.stRoute.destinationIp");
		var destinationMask = getVal("system.network.staticRoutes.stRoute.destinationMask");
		var routeInterface  = getVal("system.network.staticRoutes.stRoute.routeInterface");

		if (SrouteType == 99) {
			SaddCall        = 1;
			SrouteType      = 0;
			SgoTo           = 0;
			gateWay         = "";
			metric          = "";
			destinationIp   = "";
			destinationMask = "";
			routeInterface  = "";
		}
				
		var IPv6 = getVal('_IPv6_');
		var len1  = 15;
		var mlen  = 15;
		var len2  = 15;
	
		if (IPv6) {
			len1  = 30;
			mlen  = 50;
			len2  = 25;
		}
		
		ReqNum = getRequestNumber();
	%>

	var interfaces = '<%write(interfaces);%>';
	window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';
	
	var optGoTo      = [[0,"Gateway"],[1,"Interface"]];
	var optRouteType = [[0,"Default"],[1,"Network"],[2,"Host"]];
	var optRouteInterface = new Array();
	
	function fill() {
		fillSelect(document.netStaticRoutesEntryForm.goTo, optGoTo);
		document.netStaticRoutesEntryForm.goTo.selectedIndex = <%write(SgoTo);%>;
		fillSelect(document.netStaticRoutesEntryForm.RouteType, optRouteType);
		document.netStaticRoutesEntryForm.RouteType.selectedIndex = <%write(SrouteType);%>;
		var i;
		var idx = -1;
		var ifs = interfaces.split(',');
		for (i=0; i<ifs.length; i++) {
			if ("<%write(routeInterface);%>"==ifs[i]) {
				idx = i;
			}
			optRouteInterface[i] = [ifs[i], ifs[i]];
		}
		if (idx == -1) {
			idx = ifs.length;
			optRouteInterface[idx] = [-1, " "];
		}
		fillSelect(document.netStaticRoutesEntryForm.routeInterface, optRouteInterface);
		document.netStaticRoutesEntryForm.routeInterface.selectedIndex = idx;
		document.netStaticRoutesEntryForm.gateWay.value = "<%write(gateWay);%>";
		document.netStaticRoutesEntryForm.metric.value = "<%write(metric);%>";
		document.netStaticRoutesEntryForm.destinationIp.value = "<%write(destinationIp);%>";
		document.netStaticRoutesEntryForm.destinationMask.value = "<%write(destinationMask);%>";
		showLayer('ip if');
	}

	function showLayer(my_action) {
		if (my_action == 'clear') {
			document.netStaticRoutesEntryForm.destinationIp.value = "";
			document.netStaticRoutesEntryForm.destinationMask.value = "";
		} else if (my_action == 'ip if') {
			switch (document.netStaticRoutesEntryForm.goTo.value) {
			case "0":
				if ((getIPmode() == 2)
				|| ((getIPmode() & 2) && check(document.netStaticRoutesEntryForm.gateWay.value,"IPv6"))) {
					show2('document', 'label_br,goto_ip,goto_br,goto_if');
				} else {
					show2('document', 'goto_ip');
					hide2('document', 'label_br,goto_br,goto_if');
				}
			break;
			case "1":
				show2('document', 'goto_if');
				hide2('document', 'label_br,goto_ip,goto_br');
			break;
			}
		}
		switch(document.netStaticRoutesEntryForm.RouteType.value) {
			case "0": //Default
				hide2('document',  'route_ip');
				hide2('document',  'route_mask');
			break;
			case "1": //Network
				show2('document',  'route_ip');
				show2('document', 'route_network');
				hide2('document', 'route_host');
				if ((parseInt(getIPmode()) == 2)
				|| ((parseInt(getIPmode()) & 2) 
				&& check(document.netStaticRoutesEntryForm.destinationIp.value, "IPv6p"))) {
					document.netStaticRoutesEntryForm.destinationIp.blur();
					hide2('document',  'route_mask');
				} else {
					show2('document',  'route_mask');
				}
			break;
			case "2": //Host
				show2('document',  'route_ip');
				show2('document', 'route_host');
				hide2('document', 'route_network');
				hide2('document',  'route_mask');
			break;
			default:
				alert("VALUE: " + document.netStaticRoutesEntryForm.RouteType.options[index].value);
		}
	}

	function copy_data() {
		document.configForm.elements[1].value = opener.document.configForm.elements[1].value;
		document.configForm.elements[4].value = opener.document.configForm.elements[4].value;

		document.configForm.elements[5].value = opener.document.configForm.elements[5].value;

		var index = document.forms.netStaticRoutesEntryForm.RouteType.selectedIndex;
		switch(document.forms.netStaticRoutesEntryForm.RouteType.options[index].value) {
			case "0": //Default
				document.configForm.elements[6].value = '0';
				document.configForm.elements[7].value = 'default';
				document.configForm.elements[8].value = '';
			break;
			case "1": //Network
				document.configForm.elements[6].value = '1';
				document.configForm.elements[7].value = document.forms.netStaticRoutesEntryForm.destinationIp.value;
				document.configForm.elements[8].value = ((parseInt(getIPmode()) == 2) || ((parseInt(getIPmode()) & 2) 
					&& check(document.netStaticRoutesEntryForm.destinationIp.value, "IPv6p")))?
					'' : document.forms.netStaticRoutesEntryForm.destinationMask.value;
			break;
			case "2": //Host
				document.configForm.elements[6].value = '2';
				document.configForm.elements[7].value = document.forms.netStaticRoutesEntryForm.destinationIp.value;
				document.configForm.elements[8].value = '';
			break;
			default:
				alert("VALUE:" + document.forms.netStaticRoutesEntryForm.RouteType.options[index].value);
		}

		switch(document.forms.netStaticRoutesEntryForm.goTo.value) {
			case "0": //Gateway
				document.configForm.elements[9].value = '0';
				document.configForm.elements[10].value = document.forms.netStaticRoutesEntryForm.gateWay.value;
				document.configForm.elements[11].value = '';
				if (parseInt(getIPmode()) & 2) {
				document.configForm.elements[11].value = 
					(check(document.forms.netStaticRoutesEntryForm.gateWay.value,"IPv6")
					&& document.forms.netStaticRoutesEntryForm.routeInterface.value!="-1")?
					document.forms.netStaticRoutesEntryForm.routeInterface.value : '';
				}
			break;
			case "1": //Interface
				document.configForm.elements[9].value = '1';
				document.configForm.elements[10].value = '';
				document.configForm.elements[11].value = document.forms.netStaticRoutesEntryForm.routeInterface.value;
			break;
			default:
				alert("VALUE:" + document.forms.netStaticRoutesEntryForm.goTo.options[index].value);
		}

		document.configForm.elements[12].value = document.forms.netStaticRoutesEntryForm.metric.value;

	}

	function form_verification() {
		continue_verification = true;
		form_location = document.netStaticRoutesEntryForm;
		if (form_location.RouteType.value == 1) {
			switch (parseInt(getIPmode())) {
				case 1:
					continue_verification = verify([
						[form_location.destinationIp,   "Fill,IPv4"   ],
						[form_location.destinationMask, "Fill,NetMask"],
					]);
				break;
				case 2:
					continue_verification = verify([
						[form_location.destinationIp, "Fill,IPv6P"],
					]);
				break;
				case 3:
					if (check(form_location.destinationIp.value, "IPv6p")) {
						continue_verification = verify([
							[form_location.destinationIp, "Fill,IPv6P"],
						]);
					} else {
						continue_verification = verify([
							[form_location.destinationIp,   "Fill,IPv4"   ],
							[form_location.destinationMask, "Fill,NetMask"],
						]);
					}
				break;
			}
		} else if (form_location.RouteType.value == 2) {
			continue_verification = verify([
				[form_location.destinationIp, "Fill,IP"],
			]);
		}
		if (continue_verification) {
			switch (form_location.goTo.value) {
				case "0":
					continue_verification = verify([
						[form_location.gateWay, "Fill,IP" ],
						[form_location.metric,  "Fill?,Num"],
					]);
					if (continue_verification && (parseInt(getIPmode()) & 2)) {
						if (check(form_location.gateWay.value, "IPv6")
						&&  /^[fF][eE][89aAbB]/.test(form_location.gateWay.value)) {
							if (form_location.routeInterface.value == "-1") {
								alert("An interface must be defined for Link-Local Gateways.");
								continue_verification = false;
							}
						}
					}
				break;
				case "1":
					if (form_location.routeInterface.value != "-1") {
						continue_verification = verify([
							[form_location.metric, "Fill?,Num"],
						]);
					} else {
						alert("An interface must be defined.");
						continue_verification = false;
					}
				break;
			
			}
		}
		return continue_verification;
	}

	function addEditRoute() {
		try {
			if (opener.window.name != document.configForm.target) {
				alert("The page which called this form is not available anymore. The changes will not be effective.");
				self.close();
				return;
			}
			if (form_verification()) {
				if (<%write(SaddCall);%>) {
					copy_data();
					document.configForm.elements[13].value = 'entryADDED';
				} else {
					copy_data();
					document.configForm.elements[13].value = 'entryEDITED';
				}
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
<form name="netStaticRoutesEntryForm" id="netStaticRoutesEntryForm" method="post" action=" "onSubmit="return false;">
	<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
		<tr>
			<td height="40" align="center" valign="middle">
				<a href="javascript:addEditRoute();">
				<img src="/Images/applyButton.gif" border="0"></a>
				&nbsp;&nbsp;&nbsp;&nbsp;
				<a href="javascript:window.close();">
				<img src="/Images/cancelButton.gif" border="0"></a>
			</td>
		</tr>
		<tr>
			<td height="40" align="center" valign="middle">
				<font class="label">Route</font>&nbsp;&nbsp;&nbsp;&nbsp;
				<select name="RouteType" class="formText" onChange="showLayer('clear');">
				</select>
			</td>
		</tr>
		<tr>
			<td height="100" align="center" valign="middle">
				<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
					<tr>
						<td>
							<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
								<tr align="center" valign="middle" height="40">
									<td>
										<table id="route_ip" width="100%" style="display:none;">
											<tr>
												<td width="30%" align="right">
													<div id="route_network" style="display:none;">
														<font class="label">
															Network IP&nbsp;&nbsp;&nbsp;&nbsp;
														</font>
													</div>
													<div id="route_host" style="display:none;">
														<font class="label">
															Host IP&nbsp;&nbsp;&nbsp;&nbsp;
														</font>
													</div>
												</td>
												<td align="left">
													<input name="destinationIp" type="text" maxlength="<%write(mlen);%>" size="<%write(len1);%>" class="formText" onChange="showLayer('');">
												</td>
											</tr>
										</table>
									</td>
								</tr>
								<tr align="center" valign="middle" height="40">
									<td>
										<table id="route_mask" width="100%" style="display:none;">
											<tr>
												<td width="30%" align="right">
													<font class="label"><%getLabel("system.netSettings.netMask1");%>&nbsp;&nbsp;&nbsp;&nbsp;</font>
												</td>
												<td align="left">
													<input name="destinationMask" type="text" maxlength="<%write(mlen);%>" size="<%write(len1);%>" class="formText">
												</td>
											</tr>
										</table>
									</td>
								</tr>
							</table>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<td>
				<table align="center" width="100%" cellspacing="0" cellpadding="0" border="0">
					<tr align="center" valign="middle">
						<td width="40%" height="40" align="right" valign="middle">
							<font class="label">Go to</font>&nbsp;&nbsp;&nbsp;&nbsp;
							<select name="goTo" id="goTo" class="formText" onChange="showLayer('ip if');">
							</select>&nbsp;&nbsp;&nbsp;&nbsp;
							<div id="label_br" style="display:none;">
								<br><font class="label">Interface</font>&nbsp;&nbsp;&nbsp;&nbsp;
							</div>
						</td>
						<td width="60%" height="40" align="left" valign="middle">
							<div id="goto_ip" style="display:none;">
								<input name="gateWay" type="text" maxlength="<%write(mlen);%>" size="<%write(len2);%>" class="formText" onChange="showLayer('ip if')">
							</div>
							<div id="goto_br" style="display:none;">
								<br>
							</div>
							<div id="goto_if" style="display:none;">
								<select name="routeInterface" class="formText">
								</select>
							</div>
						</td>
					</tr>
				</table>
			</td>
		</tr>
		<tr>
			<td height="40" align="center" valign="middle">
				<font class="label">Metric</font>&nbsp;&nbsp;&nbsp;&nbsp;
				<input name="metric" type="text" id="metric" size="15" class="formText">
			</td>
		</tr>
	</table>
</form>

<form name="configForm" method="POST" action="/goform/Dmf" target="<%write(getQueryVal("target", "main"));%>">
	<input type="hidden" name="system.req.action" value="">
	<input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
	<input type="hidden" name="urlOk" value="/normal/network/netStaticRoutes.asp">
	<input type="hidden" name="urlError" value="/normal/network/netStaticRoutes.asp">
	<input type="hidden" name="request" value="<%write(ReqNum);%>">
	<!--*******************************************************************-->
	<input type="hidden" name="system.network.staticRoutes.entriesList" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.routeType" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.destinationIp" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.destinationMask" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.goTo" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.gateWay" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.routeInterface" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.metric" value="">
	<input type="hidden" name="system.network.staticRoutes.stRoute.sysMsg" value="">
</form>

<script>
fill();
</script>

</body>
</html>