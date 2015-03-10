<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<!--meta http-equiv="refresh" content="5;url=/normal/applications/accessConnect.asp"-->
    <%
        // Set page-wide variables:
        var portName = getQueryVal("portname", 0);
        var kvmip = getQueryVal("rdpaddr", 0);
        var rdpport = getQueryVal("rdpport", 0);
    %>

<SCRIPT LANGUAGE="VBScript">
sub launchMSRDP
	MsTsControl.Server = "<%write(kvmip);%>"
	MsTsControl.FullScreen = FALSE
	MsTsControl.AdvancedSettings.RDPPort = "<%write(rdpport);%>"
	MsTsControl.AdvancedSettings.RedirectPrinters = TRUE
	MsTsControl.AdvancedSettings.RedirectDrives = TRUE
	MsTsControl.FullScreenTitle = "<% getLabel("title");%> - RDP Server: <%write(portName);%>"
	MsTsControl.Connect
end sub

sub closeMSRDP
	if MsTsControl.Connected = 1 then
		MsTsControl.Disconnect
	end if
end sub

</SCRIPT>
</head>

<body>

<OBJECT ID="MsTsControl"
CLASSID="CLSID:9059f30f-4eb1-4bd2-9fdc-36f43a218f4a"
CODEBASE="msrdp.cab#version=5,1,2600,1095"
WIDTH=1024  HEIGHT=768>
</OBJECT>

</body>
</html>
