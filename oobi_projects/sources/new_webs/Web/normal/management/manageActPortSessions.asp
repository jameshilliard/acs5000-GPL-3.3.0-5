<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>Untitled</title>
   <link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">

   <%
	  ReqNum = getRequestNumber();
      self = "system.security.session.";
      self2 = "system.administration.unitInfo.";
   %>
   
   top.currentPage = self;
   top.helpPage = 'helpPages/secActPortSessionsHelp.asp';
  window.name = '<%get("system.device.ip");%>_<%get("_sid_");%>_<%write(ReqNum);%>';

   function init()
   {
      if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == top.controlsReady)
      {
         if (checkLeftMenu('/normal/management/manageMenu.asp') == 0) {
            parent.menuReady = 0;
            setTimeout('init()', 200);
            return;
         }
         setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
         parent.topMenu.selectItem(parent.menu.topItem);
         parent.menu.selectItem("ACTS");
      }
      else
         setTimeout('init()', 200);
   }

   function killSession(sess)
   {
      document.configForm.elements[5].value = 1;
      document.configForm.elements[6].value = sess;
	  setQuerySSID(document.configForm);
      submit();
   }

   function refresh()
   {
	  setQuerySSID(document.configForm);
      submit();
   }
      
   function submit()
   {
      document.configForm.submit();
	  parent.mainReady = null;
   }
    
    </script>
</head>
<body onload="init();" class="bodyMain">
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%get("_sid_");%>">
   <input type="hidden" name="urlOk" value="/normal/management/manageActPortSessions.asp">
   <input type="hidden" name="urlError" value="/normal/management/manageActPortSessions.asp">
   <input type="hidden" name="request" value=<%write(ReqNum);%>>
<!----------------- elements 5-6 below ----------------------------------------->
   <input type="hidden" name="<%write(self);%>action" value="0">
   <input type="hidden" name="<%write(self);%>selectedOption" value="">
</form>	
<form name="sessionForm" method="POST" action=" ">
<table border="0" cellpadding="0" cellspacing="0" align="center">
<tr>
   <td align="center" valign="middle" height="40"><font class="tabsTextBig">Uptime: <% get(self2 + "upTime"); %></font>
   &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font class="tabsTextBig"># Users: <% get(self + "numSessions"); %></font></td>
</tr>
</table>      
<table border="0" cellpadding="0" cellspacing="0" align="center" class="tableColor">
<tr>
<td>      
<table border="0" cellpadding="0" cellspacing="0">
   <tr class="tableColor">
      <td width="95"><font class="tabsTextBig">&nbsp;User</font></td>
      <td width="114"><font class="tabsTextBig">To</font></td>
      <td width="153"><font class="tabsTextBig">From</font></td>
      <td width="91"><font class="tabsTextBig">Login@</font></td>
      <td width="80"><font class="tabsTextBig">IDLE</font></td>
      <td width="81"><font class="tabsTextBig">JCPU</font></td>
      <td width="80"><font class="tabsTextBig">PCPU</font></td>
      <td><font class="tabsTextBig">What</font></td>
    </tr>
</table>
</td>
</tr>
   <tr>
      <td>
<select name="table" size="10" multiple class="formText">
<% get(self + "sessionHtml"); %>
   </select>
      </td>
   </tr>
</table>
<table border="0" cellpadding="0" cellspacing="0" align="center">
<tr>
   <td align="center" valign="bottom">
<table align="center" cellspacing="15" cellpadding="0" border="0">
<tr>
	<td><a href="javascript:killSession(document.sessionForm.table.selectedIndex);"><img src="/Images/kill_sessions_button.gif" alt="" width="100" height="21" border="0"></a></td>
	<td><a href="javascript:refresh();"><img src="/Images/refresh_button.gif" alt="" width="100" height="21" border="0"></a></td>
</tr>
</table>   
   </td>
</tr>
</table>    
</form>	
<script type="text/javascript">
parent.mainReady = true;
</script>
<%set("_sid_","0");%>
</body>
</html>
