
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
	<title><%getLabel("title");%></title>
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
<%
	// To navigate straight to the "ports" page (expert admins), set x to "e" (expert)
    var UBOOT_PARAM = getVal('_ubootparam_');
    var family_model = getVal('_familymodel_');
    var family_ext = getVal('_familyext_');
    var sid = getVal('_sid_');
    var controls;
    var menuBottom;
    var menuasp;
    var topMenu;
    var where;
    var access;
    var ConnectBox = getUserBio(sid);
    var kvmAnalog = 0;
    var rdp_connect = getVal('_rdpconnect_');

	sid = getNewSid(sid);

    if (family_model == "KVM")
       kvmAnalog = getVal("system.device.kvmanalog");


    if ("e" == getQueryVal("x", 0)) {
	   access = "easy";
       controls = "controls.asp";
       menuBottom = "menuBottom.asp";
       topMenu = "topMenu.asp";
       if ((family_model == "ACS")||(family_model == "AVCS")) {
          menuasp = "ports/portsMenu.asp";
          where = "ports/portsStatus.asp";
       } else {
          menuasp = "applications/appsMenu.asp";
          if (family_model == "KVM") {
            if (kvmAnalog != 0) {
               where = "/normal/applications/appsPMOutletsMan.asp";
            } else {
               where = "/normal/applications/accessConnect.asp";
            }
          } else {
    	    where = "applications/appsConnect.asp";
          }
       }
    } else {
       access = "restricted";
       controls = "justReload.asp";
       menuBottom = "menuBottomPlain.html";
       menuasp = "applications/appsMenu.asp";
       topMenu = "notopMenu.asp";
       if (family_model != "KVM") {
          if (0 == ConnectBox) {
    	     where = "applications/appsConnectPortOnly.asp";
          } else {
    	     where = "applications/appsConnect.asp";
          }
       } else {
          if (kvmAnalog != 0) {
             where = "/normal/applications/appsPMOutletsMan.asp";
          } else {
             where = "/normal/applications/accessConnect.asp";
          }
       }
    }
   controls = controls + "?SSID=" + sid;
   menuBottom = menuBottom + "?SSID=" + sid;
   menuasp = menuasp + "?SSID=" + sid;
   topMenu = topMenu + "?SSID=" + sid;
   where = where + "?SSID=" + sid;
%>
    var ConnectBox = <%write(ConnectBox);%>;
    var kvmAnalog = <%write(kvmAnalog);%>;
    var access = "<%write(access);%>";
    var UBOOT_PARAM = "<%write(UBOOT_PARAM);%>";
    var family_model = "<%write(family_model);%>";
    var family_ext = "<%write(family_ext);%>";
    var sid = "<%write(sid);%>";
    var headMenuPages = new Array;
    var rdp_connect = "<%write(rdp_connect);%>";
    var i = 0;

	if (family_model == "ONS") {
		headMenuPages[i++] = new menuObj("KVM", "KVM", "/normal/kvm/configKVMGeneral.asp");
		headMenuPages[i++] = new menuObj("PORT", "Serial/AUX", "/normal/ports/physPorts.asp");
		if (rdp_connect == "enable" && kvmAnalog == 0) {
			headMenuPages[i++] = new menuObj("RDP", "Inband", "/normal/rdp/configRDPGeneral.asp");
		}
		headMenuPages[i++] = new menuObj("SEC", "Security", "normal/security/secAccessConf.asp");
		headMenuPages[i++] = new menuObj("NET", "Network", "/normal/network/netHostSettings.asp");
		headMenuPages[i++] = new menuObj("SYS", "System", "/normal/administration/adminTimeDate.asp");

	} else if (family_model == "KVM") {

		headMenuPages[i++] = new menuObj("KVM", "KVM", "/normal/kvm/configKVMGeneral.asp");
		if (rdp_connect == "enable" && kvmAnalog == 0) {
			headMenuPages[i++] = new menuObj("RDP", "Inband", "/normal/rdp/configRDPGeneral.asp");
		}
		headMenuPages[i++] = new menuObj("SEC", "Security", "normal/security/secAccessConf.asp");
		headMenuPages[i++] = new menuObj("NET", "Network", "/normal/network/netHostSettings.asp");
		if (family_ext == "KVMNETP") {
			headMenuPages[i++] = new menuObj("AUX", "AUX Ports", "/normal/auxport/configKVMAuxPort1.asp");
		} else { 
			headMenuPages[i++] = new menuObj("AUX", "AUX Port", "/normal/auxport/configAuxPorts.asp");
		}
		headMenuPages[i++] = new menuObj("SYS", "System", "/normal/administration/adminTimeDate.asp");
	}

    var topMenuReady = 0;
    var menuReady = 0;
    var controlsReady = 0;
    var currentPage = 0;
    var helpPage = 0;
	  
//this variables define link state of phisical ports tables (active or not) 
    var accessActive = true;
    var dataBuffActive = true;
    var multiUserActive = true;
    var powManActive = false;
    var otherActive = true;
	var bidirectActive = true;
    var mainReady;	  

   </script>
</head>


<!-- frames -->
<frameset  rows="*,590,*" framespacing="0" frameborder="0" border="0">
    <frame name="topBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
    <frameset  cols="*,900,*" framespacing="0" frameborder="0" border="0">
        <frame name="leftBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
        <frameset  rows="98,*,102" cols="202,*" framespacing="0" frameborder="0" border="0">
        
                <frame name="logo" src="/logo.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                <frame name="topMenu" src="<%write(topMenu);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                <frame name="menu" src="<%write(menuasp);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                <frameset  cols="*,55" framespacing="0" frameborder="0" border="0">
    
                        <frameset  rows="6,*,6">
                            <frame name="frameTop" src="/frameTop.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                            <frameset  cols="6,*,6">
                                <frame name="frameLeft" src="/frameLeft.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                                <frame name="main" src="<%write(where);%>" marginwidth="0" marginheight="0" scrolling="auto" frameborder="0" noresize>
                                <frame name="frameRight" src="/frameRight.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                            </frameset>
                            <frame name="frameBottom" src="/frameBottom.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                        </frameset>

                    <frame name="rightMargin" src="/rightMargin.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                </frameset>
                <frame name="menuBottom" src="<%write(menuBottom);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
                <frame name="controls" src="<%write(controls);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
</frameset>


        <frame name="rightBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
    </frameset>
    <frame name="bottomBlank" src="/blank.html" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>
</frameset>

</html>
