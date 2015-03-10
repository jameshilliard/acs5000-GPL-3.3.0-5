<!-- (c) 2002 Art & Logic Software Development. All Rights Reserved. -->
<!-- $Id: test1.asp,v 1.1.1.1 2005/09/01 02:36:40 edson Exp $-->

<html>
<head>
<%language=javascript%>
<title>Tarkus Test Page #1</title>
</head>

<h1><%getLabel("h1");%></h1>
Language = <%getLanguage();%><br/>
Small = <%getLabel("small");%><br />
Medium = <%getLabel("medium");%><br />
Large = <%getLabel("large");%><br />

<h1><%getLabel("h2");%></h1>
_banana = <%get("_banana");%><br />
_goofus = <%get("_goofus");%><br />
_language=<%get("_language");%><br />
<h1><%getLabel("h3");%></h1>

<p>char val = <% get("test.struct.char"); %></p>
<p>short val = <% get("test.struct.short"); %></p>
<p>long val = <% get("test.struct.long"); %></p>
<p>Byte Array:</p>

<!-- 29 May 02 BgP: adding test for magic global ejScript variables 
	__success__ and __errorCode__.
-->

<%
	done = 0;
	val = 0;
	for (i = 0; __success__; i++)
	{
		
		val = getVal("test.struct.bytes", i);
		
		if (__error__ == "Success")
		{
			write(i + " : " + val + "<br />\n");
		}
	}
%>


<a href="test2.asp">Edit these values!</a>

<%
	if (userCanAccess("admin"))
	{
		write("You already have access to the secret pages, " + userName() + "<br />");
	}
	else
	{
		write("You will need to login before accessing the secret page!<br />");
	}
%>
<a href="/admin/test1.asp">Secret Admin page!</a>

</html>
