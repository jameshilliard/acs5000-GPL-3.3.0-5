<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: error400.asp,v 1.1.1.1 2005/09/01 02:36:39 edson Exp $-->

<html>
<head>
<%language=javascript%>
<title>Rich Error page</title>
</head>


<h1>400 Error</h1>

<p>
	This page is served when the server encounters error code 400. 
</p>

<%	now = getTime();
	timeString = formatTime(now, "%c");
	write("Current time = " + timeString + "<br />");
	write("Error Code = " + getVal("_errorCode_") + "<br />");
	write("Error Text = " + getVal("_errorText_") + "<br />");
	write("Error Message = " + getVal("_errorMessage_") + "<br />");
	write("Error URL = " + getVal("_errorUrl_") + "<br />");
%>

</html>
