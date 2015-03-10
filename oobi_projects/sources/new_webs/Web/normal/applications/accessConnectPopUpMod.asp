<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<% var tabx = getQueryVal("tabx", 0);%>
<% var sid = getQueryVal("SSID", "0");%>

<html>
<head>
  <title>Untitled</title>
  <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

var tabx=<%write(tabx);%>;
function reconnect(port)
{
	document.connectForm.newport.value = port;
	document.connectForm.action.value = 'connect';
	document.connectForm.submit();
}

function reconnecti(index)
{
	for (i = 0; i < document.connectForm.port.length; i ++) {
		if (parent.port[tabx-1] == document.connectForm.port[i].value) {
			nport = i + index;
			if (nport < 0) {
				nport = nport + document.connectForm.port.length;
			}
			if (nport >= document.connectForm.port.length) {
				nport = nport - document.connectForm.port.length;
			}
			document.connectForm.newport.value = document.connectForm.port[nport].value;
			document.connectForm.action.value = 'connect';
			document.connectForm.submit();
			break;
		}
	}
}

function connectport(port)
{
	if (parent.cycle[tabx-1] != 0) {
		return;
	}
	document.connectForm.newport.value = document.connectForm.port[document.connectForm.port.selectedIndex].value;
	document.connectForm.action.value = 'connect';
	document.connectForm.submit();
}

function disconnect()
{
	if (parent.cycle[tabx-1] != 0) {
		return;
	}
	document.connectForm.action.value = 'disconnect';
	document.connectForm.submit();
}

function cycle()
{
	document.connectForm.action.value = 'cycle';
	document.connectForm.submit();
}

function resync()
{
	if (parent.cycle[tabx-1] != 0) {
		return;
	}
	document.connectForm.action.value = 'resync';
	document.connectForm.submit();
}


function setbright(increment)
{
	if (parent.cycle[tabx-1] != 0) {
		return;
	}
	bright = bright + increment;
	if (bright < 0) {
		bright = 0;
	}
	if (bright > 255) {
		bright = 255;
	}
	document.connectForm.bright.value = bright;
	document.connectForm.contr.value = contr;
	document.connectForm.action.value = 'video';
	document.connectForm.submit();
}

function setcontr(increment)
{
	if (parent.cycle[tabx-1] != 0) {
		return;
	}
	contr = contr + increment;
	if (contr < 0) {
		contr = 0;
	}
	if (contr > 255) {
		contr = 255;
	}
	document.connectForm.bright.value = bright;
	document.connectForm.contr.value = contr;
	document.connectForm.action.value = 'video';
	document.connectForm.submit();
}

function savevideo(increment)
{
	document.connectForm.action.value = 'savevid';
	document.connectForm.submit();
}

function setoutlet(comm) {
	if (parent.cycle[tabx-1] != 0) {
		return;
	}
	document.connectForm.power.value = comm;
	document.connectForm.action.value = 'power';
	document.connectForm.submit();
}

function refresh() {
	document.connectForm.pagemode.value = '';
	document.connectForm.submit();
}

function setauto() {
	if (document.connectForm.auto.checked) {
            parent.auto = 1;
	    if (parent.timerID == 0) {
                parent.timerID = parent.setTimeout("refresh()", 5000);
	    }
	} else {
            parent.auto = 0;
	    if (parent.timerID) {
                parent.clearTimeout(parent.timerID);
                parent.timerID = 0;
 	    }
	}
	location.replace('/normal/applications/accessConnectPopUpMod.asp?tabx=' + tabx + '&SSID=<%write(sid);%>');
} 
   </script>

</head>
<body class="bodyMain">

<form name="connectForm"
		action="/normal/applications/accessConnectPopUp.asp"
		target="_parent">
<input type="hidden" name="tabx" value="<%write(tabx);%>">
<input type="hidden" name="pagemode" value="submit">
<input type="hidden" name="action">
<input type="hidden" name="newport">
<input type="hidden" name="power">
<input type="hidden" name="bright">
<input type="hidden" name="contr">
<input type="hidden" name="sniff1">
<input type="hidden" name="sniff2">
<input type="hidden" name="sniff3">
<input type="hidden" name="sniff4">
<input type="hidden" name="SSID" value="<%write(sid);%>">

<table border="0" cellpadding="0" cellspacing="0" align="center" width="100%" height="100%">
<script type="text/javascript">
      if (parent.msg[tabx-1] != 'None') {
          document.write('<tr><td valign="top"><b>Event</b></td><td colspan="2" valign="top"><b>' + 
		parent.msg[tabx-1]+ '</b></td></tr>');
      }
      if (parent.port[tabx-1] != 'None') {

      document.write('\
        <tr>\
            <td colspan="3" class="colorTHeader">\
            <font class="tabsTextBig">Server</font></td>\
        </tr>\
        <tr>\
           <td width="25%"><b>Server name</b></td>\
           <td width="25%">' + parent.port[tabx-1] + '</td>\
           <td rowspan="2" align="center" valign="center" width="50%">');
      if (parent.cycle[tabx-1] != 0) {
           document.write('<a href="javascript:cycle();">Stop Cycle</a>');
      } else {
           document.write('\
		<a href="javascript:reconnecti(-1);">Prev</a>&nbsp;');
           document.write('\
                <select name="port" class="formText" onchange="javascript:connectport();"');
           if (parent.auto) {
                document.write(' disabled');
           }
           document.write('></select>\
		&nbsp;<a href="javascript:reconnecti(1);">Next</a><br>\
      		<a href="javascript:cycle();">Start Cycle</a>&nbsp;&nbsp;&nbsp;\
		<a href="javascript:disconnect();">Disconnect</a>');
      }
      document.write('\
        </td>\
      </tr>');
      document.write('\
      <tr>\
        <td><b>Physical ID</b></td>\
	<td>' + parent.phys[tabx-1] + '</td>\
      </tr>');
      document.write('\
      <tr>\
        <td colspan="3" class="colorTHeader"><font class="tabsTextBig">Access</font></td>\
      </tr>');
      document.write('\
      <tr>\
        <td><b>Username</b></td>\
	<td>' + parent.username[tabx-1] + '</td>\
        <td rowspan="2">\
        </td>\
      </tr>');
      document.write('\
      <tr>\
        <td><b>Permission</b></td>\
	<td>');
      perm = parent.perm[tabx-1];
      if (perm == 'NONE') {
          document.write('none');
      } else if (perm == 'RO') {
          document.write('read only');
      } else if (perm == 'RW') {
          document.write('read/write');
      } else if (perm == 'FULL') {
          document.write('read/write/power');
      } else {
          document.write(perm);
      }
      document.write('\
        </td>\
      </tr>\
      <tr>\
        <td colspan="3" class="colorTHeader"><font class="tabsTextBig">Connections</font></td>\
      </tr>\
      <tr>\
        <td><b>Status</b></td>\
        <td colspan="2">');
	if (parent.status[tabx-1]) {
		document.write('Connected with ' + parent.conn[tabx-1]);
	} else {
		document.write('Not connected.');
	}
      document.write('\
	</td>\
      </tr>\
      <tr>\
        <td colspan="3" align="center">');
	if (parent.cycle[tabx-1] == 0) {
   		document.write('<a href="javascript:resync();">Reset Mouse and Keyboard</a>');
	}
      document.write('\
	</td>\
      </tr>\
      <tr>\
        <td colspan="3" class="colorTHeader"><font class="tabsTextBig">Power Management</font></td>\
      </tr>');
     var family_model = '<%get("_familymodel_");%>';
     if (family_model == "ONS") {
      document.write('<tr><td colspan="3" align="center">Power management is not available through KVM sessions.</td></tr>');
     } else {
      j = 0;
      if (parent.outlet[tabx-1] != 'none') {
          var outls = parent.outlet[tabx-1].split(' ');
          for (i = 0; i < outls.length; i ++) {
              var cols = outls[i].split(':');
              if (cols.length >= 2) {
                  j ++;
              }
          }
      }
      if (j == 0) {
          document.write('<tr><td colspan="3" align="center">No outlet configured.</td></tr>');
      } else {
          var outls = parent.outlet[tabx-1].split(' ');
          k = 0;
          for (i = 0; i < outls.length; i ++) {
              var cols = outls[i].split(':');
              if (cols.length >= 2) {
                  document.write('<tr><td><b>Outlet ' + cols[0] + '</b></td>');
                  document.write('<td align="left"><img name="bulb1" src="');
                  if (cols[1] == 'on') {
                      document.write('/Images/bulbOn.gif');
                  } else if (cols[1] == 'off') {
                      document.write('/Images/bulbOff.gif');
                  } else if (cols[1] == 'error') {
                      document.write('/Images/bulbUK.gif');
                  } else {
                      document.write('/Images/bulbUK.gif');
                  }
                  document.write('" width="15" height="20" border="0" alt="'+cols[1]+'">');
                  if (cols[2] == 'locked') {
                      document.write('&nbsp;<img name="lock1" src="/Images/locked.gif" width="15" height="20" border="0" alt="' + cols[2] + '">');
                  }
                  document.write('</td>');
                  if (k == 0) {
                      document.write('<td rowspan="' + j + '" align="center">');
                      if (parent.cycle[tabx-1] == 0 && perm == 'FULL') {
                          document.write('<a href="javascript:setoutlet(\'on\');">On</a>&nbsp;');
                          document.write('<a href="javascript:setoutlet(\'off\');">Off</a>&nbsp;');
                          document.write('<a href="javascript:setoutlet(\'cycle\');">Cycle</a>');
                      }
                      document.write('</td>');
                  }
                  document.write('</tr>');
                  k ++;
              }
          }
      }
     }
      
      document.write('\
      </tr>\
      <tr>\
        <td colspan="3" class="colorTHeader"><font class="tabsTextBig">Video Configuration</font></td>\
      </tr>\
      <tr>\
        <td><b>Brightness</b></td>\
        <td align="center">');
         bright = parent.bright[tabx-1]; 
         if (parent.cycle[tabx-1] == 0) {
            document.write('<a href="javascript:setbright(-16);">&lt;&lt;</a>&nbsp;<a href="javascript:setbright(-1);">&lt;</a>&nbsp;<b>' + bright + '</b>&nbsp;<a href="javascript:setbright(1);">&gt;</a>&nbsp;<a href="javascript:setbright(16);">&gt;&gt;</a>');
         } else {
            document.write(bright);
         }
      document.write('\
        </td>\
        <td rowspan="2" align="center">');
	if (parent.cycle[tabx-1] == 0) {
           document.write('<a href="javascript:savevideo();">Save Settings</a>');
	}
      document.write('\
        </td>\
      </tr>\
      <tr>\
        <td><b>Cable Length Adjustment</b></td>\
        <td align="center">');
         contr = parent.contr[tabx-1]; 
         if (parent.cycle[tabx-1] == 0) {
            document.write('<a href="javascript:setcontr(-16);">&lt;&lt;</a>&nbsp;<a href="javascript:setcontr(-1);">&lt;</a>&nbsp;<b>' + contr + '</b>&nbsp;<a href="javascript:setcontr(1);">&gt;</a>&nbsp;<a href="javascript:setcontr(16);">&gt;&gt;</a>');
         } else {
            document.write(contr);
         }
        document.write('</td></tr>');

       if (parent.cycle[tabx-1] == 0) {
        document.connectForm.port.length = parent.portlist[tabx-1].length;
        for (i = 0; i < parent.portlist[tabx-1].length; i ++) {
          document.connectForm.port[i].value = parent.portlist[tabx-1][i];
          document.connectForm.port[i].text = parent.portlist[tabx-1][i];
          if (parent.port[tabx-1] == parent.portlist[tabx-1][i]) {
            document.connectForm.port.selectedIndex = i;
          }
        }
       }
      }
</script>
      <tr>
        <td colspan="3" valign="bottom" align="center">
<script type="text/javascript">
          if (parent.auto == 0) {
              document.write('<a href=javascript:refresh();><img src="/Images/refresh_button.gif" alt="refresh" width="100" height="21" border="0"></a>&nbsp;&nbsp;&nbsp;&nbsp;');
          }
          document.write('<input type="checkbox" name="auto" value="1" onClick="javascript:setauto();"');
          if (parent.auto) {
              document.write(' checked');
	  }
          document.write('>');
</script>
          &nbsp;<b>Automatic refresh</b>
        </td>
      </tr>
    </table>
   </form>
<script type="text/javascript">
document.connectForm.sniff1.value= parent.sniff[0];
document.connectForm.sniff2.value= parent.sniff[1];
document.connectForm.sniff3.value= parent.sniff[2];
document.connectForm.sniff4.value= parent.sniff[3];
</script>
</body>
</html>
