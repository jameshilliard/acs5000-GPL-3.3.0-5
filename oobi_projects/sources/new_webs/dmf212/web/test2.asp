<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: test2.asp,v 1.1.1.1 2005/09/01 02:36:40 edson Exp $-->

<html>
<head>
<%language=javascript%>
<title>Tarkus Test Page #1</title>
</head>


<h1>Very simple test.</h1>

<% 
	count = getErrorCount();
	if (count > 0)
	{
		write("<h2>There were errors while submitting the form:</h2>");
		for (i = 0; i < count; i++)
		{
			write(getErrorMessage(i) + "<br>\n");
		}
	}
%>


<form METHOD="POST" ACTION="/goform/Dmf">

_banana = <input size="5" name="_banana" value=<%get("_banana");%>><br>
_goofus = <input size="5" name="_goofus" value=<%get("_goofus");%>><br>
Language = <input size="5" name="_language" value=<%get("_language");%>><br>


char val = <input size="5" name="test.struct.char" value=<% get("test.struct.char"); %>><br/>
short val = <input size="5" name="test.struct.short" value=<% get("test.struct.short"); %>><br/>
long val = <input size="5" name="test.struct.long" value=<% get("test.struct.long"); %>><br/>

byte array:
<%
	val = 0;
	for (i = 0; __success__; i++)
	{
		val = getVal("test.struct.bytes", i);
		if (__success__)
		{
			write("Array[" + i + "] : ");
			write("<input size=\"5\" name=\"test.struct.bytes_" + i + "\"");
			write("value= \"" + val + "\"><br/>\n");
		}
	}
%>

<input type="hidden" name="request" value=<%write(getRequestNumber());%>>
<input type=hidden name="urlOk" value="test1.asp">
<input type=hidden name="urlError" value="test2.asp">
<input type="submit" name="Submit" value="ok">
</form>





</html>
