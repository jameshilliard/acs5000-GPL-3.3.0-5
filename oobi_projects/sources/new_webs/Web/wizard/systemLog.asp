
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
<script>

<%
	var IPv6 = getVal('_IPv6_');
	var width = 15;
	var input_size = 15;
	var input_maxlength = 15;
	if (IPv6) {
		width = 46;
		input_size = 25;
		input_maxlength = 50;
	}
%>

	var width = <%write(width);%>;
	var cols = new Array(
			new _col("Syslog Servers", width, width)
		);
	var serverTable = new selTable("serverTable", cols, 5);
	var line = String.fromCharCode(160);
	for (i=1; i<width; i++) {
		line += String.fromCharCode(160);
	}

</script>


<html>
<head>
	<title>Untitled</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
      <%
         // Set page-wide variables:
         var self = "system.systemLog.";
         var device = "system.device.";
         var model = getVal(device + "model");

         var ReqNum = getRequestNumber();
         var SID = getVal("_sid_");
         trial = getVal(self + "facilityNumber"); // representative of the rest (could test all if anal)
      %>
	
      var page = 0;
	  parent.helpPage = 'helpPages/systemLogHelp.asp';
      window.name = '<%get("system.device.ip");%>_<%write(SID);%>_<%write(ReqNum);%>';
      parent.currentPage = self;

      var servers = <%get("system.systemLog.serversHtml");%>;
	
      function fill() {
         fillSelTable(document.serverForm, serverTable, servers);
         document.serverForm.serverTable.selectedIndex = servers.length;
      }

      function form_verification() {
         continue_verification = verify([
            [document.serverForm.newServer, "Fill,IP"]
         ]);
      }
	  
      function addServer(object) {	 
         form_verification();
         // check for duplication
         for (var i = 0; i < object.serverTable.length-1; i++) {
            if (object.serverTable.options[i].value == object.newServer.value) {
               alert('This server is already in the list!');
               continue_verification = false;
               document.serverForm.newServer.focus();
               document.serverForm.newServer.select();
               break;
            }
         }
         if (continue_verification == true) {
            var defaultSelected = true;
            var selected = true;
            var newOption = new Option(object.newServer.value, object.newServer.value, defaultSelected, selected);
            var length = object.serverTable.length;
            object.serverTable.options[length-1] = newOption;
            var widthOption = new Option(line, -1, false, false);
            object.serverTable.options[length] = widthOption;
            object.newServer.value='';
         }
      }

      function delServer() {
         deleteSelOpt(document.serverForm.serverTable);
      }


      function init() {
         if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady) {
		    page = parent.menu.getPageIndex('LOG', parent.menu.menuPages);
            parent.menu.selectItem('LOG');
            setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
         } else
            setTimeout('init()', 200);
      }

      function submit()
      {
         document.configForm.elements[6].value = document.serverForm.elements[0].value;
         document.configForm.elements[7].value = getValues(document.serverForm.serverTable,1);
         document.configForm.submit();
		 parent.mainReady = null;
      }

   </script>
</head>
<body onload="init();" class="bodyMain">
<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="400" align="center">
    <tr>
        <td>
            <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
               <tr>
                   <td align="center" class="label_for_tables">
                     Configure external syslog server location to receive unit's syslog messages.
                  </td>
               </tr>
            </table>
        </td>
    </tr>
</table>
<form name="configForm" method="POST" action="/goform/Dmf">
   <input type="hidden" name="system.req.action" value="">
   <input type="hidden" name="system.req.sid" value="<%write(SID);%>">
   <input type="hidden" name="urlOk" value="/wizard/systemLog.asp">
   <input type="hidden" name="urlError" value="/wizard/systemLog.asp">
   <input type="hidden" name="request" value="<%write(ReqNum);%>">
   <input type="hidden" name="<%write(self);%>mode" value="0">
<!--*************************************************************************-->
   <input type="hidden" name="<%write(self);%>facilityNumber" value="">
   <input type="hidden" name="<%write(self);%>serversCs" value="">
</form>
<form name="serverForm" action="" onSubmit="return false;">
<table width="100%" height="40" border="0" align="center" cellpadding="0">
   <tr>
      <td align="center" valign="top">
         <font class="label"><%getLabel(self + "facilityNumber");%></font>
         &nbsp;&nbsp;&nbsp;&nbsp;
         <select name="<%write(self);%>facilityNumber" class="formText">
            <% var selected = getVal(self + "facilityNumber"); %>
            <option value="0" <%if (0 == selected) write("selected");%>>Local 0</option>
            <option value="1" <%if (1 == selected) write("selected");%>>Local 1</option>
            <option value="2" <%if (2 == selected) write("selected");%>>Local 2</option>
            <option value="3" <%if (3 == selected) write("selected");%>>Local 3</option>
            <option value="4" <%if (4 == selected) write("selected");%>>Local 4</option>
            <option value="5" <%if (5 == selected) write("selected");%>>Local 5</option>
            <option value="6" <%if (6 == selected) write("selected");%>>Local 6</option>
            <option value="7" <%if (7 == selected) write("selected");%>>Local 7</option>
         </select>	
      </td>
   </tr>
</table>
<%if (IPv6) {
write('\
<table width="460" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr align="center">
		<td align="center" valign="middle">
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td align="center" valign="middle" height="30" colspan="2">
						<font class="label">New Syslog Server</font>
					</td>
				</tr>
				<tr>
					<td align="center" valign="middle">
						<input class="formText" type="text" maxlength="50" name="newServer" size="25">
					</td>
					<td height="40" align="center" valign="middle">
						&nbsp;&nbsp;<input type="image" src="/Images/addToButton.gif" onClick="addServer(this.form);" align="texttop" border="0">
					</td>
				</tr>
			</table>
		</td>
	</tr>
	<tr align="center">
		<td align="center" valign="middle">
			<script>
				serverTable.drawSelTable();
			</script>
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td height="40" align="center" valign="middle">
						<a href="javascript:delServer();">
						<img src="/Images/deleteButton.gif" alt="" width="47" height="21" border="0"></a>  
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>');
} else {
write('\
<table width="460" border="0" align="center" cellpadding="0" cellspacing="0">
	<tr align="center">
		<td align="center" valign="middle">
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td align="center" valign="middle" height="30" colspan="2">
						<font class="label">New Syslog Server</font>
					</td>
				</tr>
				<tr>
					<td align="center" valign="middle">
						<input class="formText" type="text" maxlength="15" name="newServer" size="15">
					</td>
				</tr>
				<tr>
					<td height="40" align="center" valign="middle">
						<input type="image" src="/Images/addToButton.gif" onClick="addServer(this.form);" align="texttop" border="0">
					</td>
				</tr>
			</table>
		</td>
		<td align="center" valign="middle">
			<script>
				serverTable.drawSelTable();
			</script>
			<table border="0" align="center" cellpadding="0" cellspacing="0">
				<tr>
					<td height="40" align="center" valign="middle">
						<a href="javascript:delServer();">
						<img src="/Images/deleteButton.gif" alt="" width="47" height="21" border="0"></a>  
					</td>
				</tr>
			</table>
		</td>
	</tr>
</table>');
};%>
</form>

<script type="text/javascript">
fill();
parent.mainReady = true;
parent.controls.document.getElementById('back').style.visibility = "visible";
parent.controls.document.getElementById('next').style.visibility = "visible";
</script>
<% set("_sid_", "0"); %>
</body>
</html> 
