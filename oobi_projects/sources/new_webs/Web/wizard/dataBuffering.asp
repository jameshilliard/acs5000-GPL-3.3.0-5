
<!doctype html public "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
   <title>dataBuffering</title>
	<link rel="STYLESHEET" type="text/css" href="/stylesLayout.css">
   <script language="JavaScript" src="/scripts.js" type="text/javascript"></script>
   <script type="text/javascript">
   <%
      // Set page-wide variables:
      var self = "system.dataBuffering.";
      var device = "system.device.";
      var model = getVal(device + "model");

      var ReqNum = getRequestNumber();
      var SID = getVal("_sid_");
   %> 

      var page = 0;
	  parent.helpPage = 'helpPages/dataBufferingHelp.asp'; 
      window.name = '<%get("system.device.ip");%>_<%write(SID);%>_<%write(ReqNum);%>';
      parent.currentPage = self;
	 
      function copyData() 
      {    
		 var my_form;
         var selector;
       
         if (type == 'NN')
            selector = document.buffering.document.destinationForm.elements[0];
         else
            selector = document.destinationForm.elements[0];

         if (type == 'NN')
			 my_form = document.local.document.localForm;
         else
			 my_form = document.localForm;

         if (document.configForm.elements[10].checked == true) {
             document.configForm.elements[5].value = eval(my_form).elements[1].value;
         } else {
             document.configForm.elements[5].value = 0;
		 }

         document.configForm.elements[9].value = eval(my_form).elements[0].selectedIndex;

         if (document.configForm.elements[5].value == 0 && 
		 document.configForm.elements[10].checked == true && selector.selectedIndex == 1) 
		 {
        	 document.configForm.elements[5].value = 1024;
		 }
		 
         if (type == 'NN')
			 my_form = document.remote.document.remoteForm;
		 else
			 my_form = document.remoteForm;

         if (document.configForm.elements[10].checked == true) {
        	if (selector.selectedIndex == 0)
         		document.configForm.elements[6].value = '';
			else
         		document.configForm.elements[6].value = eval(my_form).elements[0].value;               
		 } else {
         	document.configForm.elements[6].value = eval(my_form).elements[0].value;               
		 }


         if (type == 'NN') 
			 my_form = document.other.document.otherForm;
		 else
			 my_form = document.otherForm;

         document.configForm.elements[7].value = eval(my_form).elements[2].selectedIndex;
         setCheck(eval(my_form).elements[1], eval(my_form).elements[0]);
         document.configForm.elements[8].value = eval(my_form).elements[1].value;
		 return true;
      }
   
      function init()
      {
         if (1 == parent.menuReady && 1 == parent.topMenuReady && 1 == parent.controlsReady)
         {
		    page = parent.menu.getPageIndex('BUF', parent.menu.menuPages);
            parent.menu.selectItem('BUF');
            setUnsavedImage(parent.controls.document, <%get("_changesLed_");%>);
        	if (type == 'NN')
            	initCheck(document.configForm.elements[10], document.local.document.localForm.elements[1]);
			else
            	initCheck(document.configForm.elements[10], document.localForm.elements[1]);
        	if (type == 'NN')
            	initCheck(document.other.document.otherForm.elements[0], document.other.document.otherForm.elements[1]);
			else
            	initCheck(document.otherForm.elements[0], document.otherForm.elements[1]);
			if (type =='NN') {
				if (document.remote.document.remoteForm.elements[0].value != '')
					document.buffering.document.destinationForm.elements[0].selectedIndex = 1
				else
					document.buffering.document.destinationForm.elements[0].selectedIndex = 0
			} else {
				if (document.remoteForm.elements[0].value != '')
					document.destinationForm.elements[0].selectedIndex = 1
				else
					document.destinationForm.elements[0].selectedIndex = 0
			}
			showEnabled();
         }
         else
            setTimeout('init()', 200);
      }

      function showEnabled()
      {
         if (document.configForm.elements[10].checked == true)
         {
            show('document', 'buffering');
            show('document', 'other');
            showSelectedDestination();
         }
         else
         {
            hide('document', 'buffering');
            hide('document', 'other');
            hide('document', 'local');
            hide('document', 'remote');  
            hide('document', 'remoteNOK');  
         }
      }

      function showSelectedDestination()
      {    
        var selector = 0;
        if (type == 'NN')
           selector = document.buffering.document.destinationForm.elements[0];
        else if (type == 'IE' || type == 'MO')
           selector = document.destinationForm.elements[0];

        var remoteDiv = (parseInt(getIPmode()) == 2)? 'remoteNOK' : 'remote';
        if (selector.options[selector.selectedIndex].value == '0')
        {               
           show('document', 'local');
           hide('document', remoteDiv);
        }
        else //if (selector.options[selector.selectedIndex].value == '1')
        {
           show('document', remoteDiv);
           hide('document', 'local');
        } 
      }
	  
	  	  function form_verification()
	  {
	  		var form_location // Dif IE or NN
			var form_location_local
			var form_location_remote
			var selector
			continue_verification = true;
	  		if (type == 'NN') 
	  		{
			form_location = document.buffering.document.destinationForm;
			form_location_local = document.local.document.localForm;
			form_location_remote = document.remote.document.remoteForm;
			selector = document.remote.document.remoteForm.elements[0];
			}
	  		else 
	  		{
			form_location = document.destinationForm;
			form_location_local = document.localForm;
			form_location_remote = document.remoteForm;
			selector = document.destinationForm.elements[0];
			}
	  if ((document.configForm.elements[10].checked) && (parseInt(getIPmode()) == 2) && (selector.value == '1'))
	  {
	        continue_verification = false;
	  }
	  if (continue_verification && document.configForm.elements[10].checked == true && selector.options[selector.selectedIndex].value == '0')
	  {
	  	    checkElement (form_location_local.elements[1], true, false, false, true, 1, 'no');
	  		// checkElement (element_for_verification, completing, white_spaces, ip, numerical, minNum, maxNum, password, password_old)
	  }
	  else if (continue_verification && document.configForm.elements[10].checked == true && selector.options[selector.selectedIndex].value == '1')
	  {
	  	    checkElement (form_location_remote.elements[0], true, false, false, false);
	  }
	  /*else 
	  {continue_verification = true;}*/
	  }  	  
		  
	  
      function submit()         
	  {	 form_verification();
		 if (continue_verification == true)
		 { copyData();
           document.configForm.submit();
		   parent.mainReady = null;
		 }  
      }	   
      
</script>

</head>
<body onload="init();" class="bodyForLayers">
   <form name="configForm" onsubmit="copyData();" method="POST" action="/goform/Dmf">
         <input type="hidden" name="system.req.action" value="">
         <input type="hidden" name="system.req.sid" value="<%write(SID);%>">
         <input type="hidden" name="urlOk" value="">
         <input type="hidden" name="urlError" value="/wizard/dataBuffering.asp">
         <input type="hidden" name="request" value="<%write(ReqNum);%>">

         <!--these hidden inputs are used to store values of form elements inside layers-->
         <input type="hidden" name="<%write(self);%>fileSize" value="">
         <input type="hidden" name="<%write(self);%>nfsPath" value="">
         <input type="hidden" name="<%write(self);%>showMenu" value="">
         <input type="hidden" name="<%write(self);%>timeStamp" value="">
         <input type="hidden" name="<%write(self);%>mode" value="">
	<table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="400" align="center">	      
		<tr>		        
			<td>		            
				<table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">		                
					<tr>		                    
						<td align="center" class="label_for_tables">
                      Set up data buffering to the output from the consoles in a console log file.
		   <font color="red" weigth="bold">
                   The previous port-specific parameters will be discarded.<br>
		   </font>
                   </td>
                </tr>
				</table>		        
			</td>		    
		</tr>	      
	</table>	
         <!--*************************************************************************-->           
	      <table border="0" cellpadding="2" cellspacing="0" align="center">		    
		      <tr valign="top">		        
			      <td  height="35" valign="bottom">	
	              <font class="label"><%getLabel(self + "enable");%></font>&nbsp;
                  <input type="checkbox" name="system.req.bogus" value="1"
                   onclick="showEnabled()">
			      </td>		    
		      </tr>	      
	      </table>
	   </form>
<div id="buffering" style="position:absolute; left:0px; top:180px; width:100%; z-index:1; visibility: hidden; height: 30;">	
      <form name="destinationForm"  onSubmit="return false;">	
		<table border="0" cellpadding="2" cellspacing="0" align="center">
			<tr valign="top">				
				<td align="center">
					<font class="label"><%getLabel(self + "destination");%></font>
	            &nbsp;&nbsp;&nbsp;&nbsp;            
					<select name="destination" onChange="showSelectedDestination()" class="formText">
						<option value="0"> <%getLabel(self + "local");%> </option>
						<option value="1"> <%getLabel(self + "remote");%> </option>
					</select>
				</td>
			</tr>         	
		</table>
      </form>
</div>	  		
<div id="local" style="position:absolute; left:0px; top:220px; width:100%; z-index:1; visibility: hidden; height: 60;">	      
			<form name="localForm"  onSubmit="return false;">	         
				<table border="0" cellpadding="0" cellspacing="0" align="center">		    
					<tr valign="middle">	   			
						<td align="left">	                  
	                  <font class="label"><%getLabel(self + "mode");%></font>&nbsp;        
						</td>				
						<td  align="left">            
							<select name="<%write(self);%>mode" class="formText">
				               <% selected = getVal(self + "mode"); %>
								<option value="0" <% if (0 == selected) write("selected"); %>> <%getLabel(self + "circular");%> </option>
								<option value="1"<% if (1 == selected) write("selected"); %>> <%getLabel(self + "linear");%> </option>
							</select>
						</td>		        
						<td align="right">                  
                     &nbsp;&nbsp;&nbsp;&nbsp;<font class="label"><%getLabel(self + "fileSize");%></font>&nbsp;
						</td>				
						<td align="right">							
							<input type="text" size="10" class="formText" name="<%write(self);%>fileSize" value="<%get(self + "fileSize");%>">
						</td>		    
					</tr>
            </table>	
         </form>	
		</div>	
<div id="remote" style="position:absolute; left:0px; top:220px; width:100%; z-index:1; visibility: hidden; height: 60;">
   <form name="remoteForm"  onSubmit="return false;">				
      <table border="0" cellpadding="0" cellspacing="0" align="center">			
         <tr>
            <td align="center">
               <font class="label"><%getLabel(self + "nfsPath");%></font>
               &nbsp;&nbsp;&nbsp;&nbsp;<input type="text" size="30" maxlength="99" class="formText" name="<%write(self);%>nfsPath" value="<%get(self + "nfsPath");%>">
            </td>
         </tr>
      </table>
   </form>
</div>
<div id="remoteNOK" style="position:absolute; left:0px; top:220px; width:100%; z-index:1; visibility: hidden; height: 60;">
<table width="500" border="0" cellpadding="0" cellspacing="0" align="center">
   <tr>
      <td height="100%" align="center">
         <table height="100%" width="100%" cellpadding="0" cellspacing="0" border="0">
            <tr height="100%" align="center">
               <td align="center" valign="middle">
                  <table class="bgColor7" border="0" cellpadding="2" cellspacing="0" width="500" align="center">
                     <tr>
                        <td align="center" valign="middle">
                           <table class="bgColor1" border="0" cellpadding="15" cellspacing="0" class="helpBox" width="100%" align="center">
                              <tr>
                                 <td align="center" valign="middle" class="label_for_tables">
                                    You have selected a service that is currently not supported in IPv6.<br>
                                    If you like to use this service please enable IPv4 for NFS to work
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
</table>	
</div>
<div id="other"style="position:absolute; left:0px; top:290px; width:100%; z-index:1; visibility: hidden; height: 60;">
      <form name="otherForm"  onSubmit="return false;">
<table border="0" cellpadding="0" cellspacing="0" align="center">   
   <tr>
      <td align="center" colspan="4" height="35" valign="middle">
         <font class="label"><%getLabel(self + "timeStamp");%></font>&nbsp;&nbsp;&nbsp;&nbsp;       
         <input type="checkbox" name="system.req.bogus" value="1">
         <input type="hidden" name="<%write(self);%>timeStamp" value="<%get(self + "timeStamp");%>">
       </td>
   </tr>
   <tr>
      <td align="center" colspan="4" height="40"  valign="bottom">
	    <font class="label"><%getLabel(self + "showMenu");%></font>&nbsp;        
		<select name="<%write(self);%>showMenu"  class="formText">
		<% selected = getVal(self + "showMenu"); %>
		<option value="0"<% if (0 == selected) write("selected"); %>> <%getLabel(self + "showMenu.val0");%> </option>
		<option value="1"<% if (1 == selected) write("selected"); %>> <%getLabel(self + "showMenu.val1");%> </option>
		<option value="2"<% if (2 == selected) write("selected"); %>> <%getLabel(self + "showMenu.val2");%> </option>
		<option value="3"<% if (3 == selected) write("selected"); %>> <%getLabel(self + "showMenu.val3");%> </option>
		</select>	
       </td>
   </tr>
</table>
      </form>	
	</div>
<script type="text/javascript">
parent.mainReady = true;
parent.controls.document.getElementById('back').style.visibility = "visible";
parent.controls.document.getElementById('next').style.visibility = "visible";

</script>	
<% set("_sid_", "0"); %>
</body>
</html>



