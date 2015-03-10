<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<% 
	var tabx = getQueryVal("tabx", 0);
	var sniff1 = getQueryVal("sniff1", "");
	var sniff2 = getQueryVal("sniff2", "");
	var sniff3 = getQueryVal("sniff3", "");
	var sniff4 = getQueryVal("sniff4", "");
	var auto = getQueryVal("auto", "0");
	var sid = getQueryVal("SSID", "0");
%>


<html>
<head>
  <title><% getLabel("title"); %></title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
   var tabx = <%write(tabx);%>;
   var sniff = new Array ('<%write(sniff1);%>', '<%write(sniff2);%>', '<%write(sniff3);%>', '<%write(sniff4);%>');
   var target = '<%get("system.device.ip");%>_' + '_POPUP';

<%
	var pagemode = getQueryVal("pagemode", "");
	var action = getQueryVal("action", "");

	if ( pagemode == "submit" ) {
		if ( action == "connect") {
			var newport = getQueryVal("newport", "");
			var sniffopt = getQueryVal("sniffopt", "0");
			var otherstation = getQueryVal("otherstation", "0");
			switchStation(tabx, newport, sniffopt, otherstation);
		} else if ( action == "disconnect") {
			switchStation(tabx, "", "5", "0");
		} else if (action == "cycle") {
			cycleStation(tabx);
		} else if (action == "resync") {
			resyncServer(tabx);
		} else if (action == "power") {
			var power = getQueryVal("power", "");
			pmSwitch(tabx, power);
		} else if (action == "video") {
			var bright = getQueryVal("bright", "");
			var contr = getQueryVal("contr", "");
			stationVideoConf(tabx, "manual", bright, contr);
		} else if (action == "savevid") {
			stationVideoConf(0);
		}
	}
%>
<%userInfo();%>
   var mystation = new Array (0, 0, 0, 0);
   var show = 1;
   var ntabs = 0;
   var auto = <%write(auto);%>;
   var timerID = 0;

   for (ix = 0; ix < 4; ix++) {
     if (port[ix] != 'None' || msg[ix] != 'None') {
	mystation[ntabs] = ix + 1;
        ntabs++;
     }
   }
   if (ntabs == 0) {
      show = 0;
   }
   if (tabx == 0 || (port[tabx-1] == 'None' && msg[tabx-1] == 'None')) {
      tabx=mystation[0];
   }
   if (sniff[tabx-1] == '') {
       pagefile = 'accessConnectPopUpMod.asp';
   } else {
       pagefile = 'accessConnectPopUpSniff.asp';
   }

   function refresh()
   {
	if (auto) {
		location.replace('/normal/applications/accessConnectPopUp.asp?tabx='+tabx+'&sniff1='+sniff[0]+'&sniff2='+sniff[1]+'&sniff3='+sniff[2]+'&sniff4='+sniff[3]+'&auto='+auto+'&SSID=<%write(sid);%>');
	}
   }
   </script>
</head>
<!-- frames -->
   <script type="text/javascript">
      if (show) {
         document.write('<frameset');
         if (auto) {
             document.write(' onload="timerID=setTimeout(\'refresh()\',10000);"');
         }
         document.write(' rows="22,*" border="0" frameborder="0">\
             <frame name="'+target+'tabs" src="accessConnectPopUpTab.asp?tabx='+tabx+'&SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="no" frameborder="0" noresize>\
             <frame name="'+target+'page" src="'+pagefile+'?tabx='+tabx+'&SSID=<%write(sid);%>" marginwidth="0" marginheight="0" scrolling="auto" frameborder="0" noresize>\
             </frameset>');
      } else {
         document.write('<body class="bodyMain"');
         if (auto) {
             document.write(' onload="timerID=setTimeout(\'refresh()\',10000);"');
         }
         document.write('>\
             <table border="0" cellpadding="0" cellspacing="0" align="center" width="100%" height="100%"><tr><td align="center" valign="center">\
             <b>There is no active connection<br>in this session!</b>\
	     </td></tr></table></body>');
      }
   </script>
</html>
