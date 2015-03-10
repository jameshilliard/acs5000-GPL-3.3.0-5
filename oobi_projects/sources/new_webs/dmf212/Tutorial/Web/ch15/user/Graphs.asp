<HTML>
<HEAD>
<meta http-equiv=Content-Type content="text/html;  charset=ISO-8859-1">
<title><%getLabel("page.title"); get("_chapter_");%></title>
</HEAD>
<body bgcolor=<%get("_bgcolor");%>>
<!-- URL's used in the movie-->
<!-- text used in the movie-->
<!-- image map -->
<MAP NAME="">

</MAP>
<h1>Data Values: Macromedia Flash Graphs
(requires <a href="http://www.macromedia.com/go/getflashplayer">Flash 6</a>)
</h1>

<SCRIPT LANGUAGE=JavaScript1.1>
<!--
var MM_contentVersion = 6;
var plugin = (navigator.mimeTypes && navigator.mimeTypes["application/x-shockwave-flash"]) ? navigator.mimeTypes["application/x-shockwave-flash"].enabledPlugin : 0;
if ( plugin ) {
		var words = navigator.plugins["Shockwave Flash"].description.split(" ");
	    for (var i = 0; i < words.length; ++i)
	    {
		if (isNaN(parseInt(words[i])))
		continue;
		var MM_PluginVersion = words[i]; 
	    }
	var MM_FlashCanPlay = MM_PluginVersion >= MM_contentVersion;
}
else if (navigator.userAgent && navigator.userAgent.indexOf("MSIE")>=0 
   && (navigator.appVersion.indexOf("Win") != -1)) {
	document.write('<SCR' + 'IPT LANGUAGE=VBScript\> \n'); //FS hide this from IE4.5 Mac by splitting the tag
	document.write('on error resume next \n');
	document.write('MM_FlashCanPlay = ( IsObject(CreateObject("ShockwaveFlash.ShockwaveFlash." & MM_contentVersion)))\n');
	document.write('</SCR' + 'IPT\> \n');
}
if ( MM_FlashCanPlay ) {
		document.write('<OBJECT classid="clsid:D27CDB6E-AE6D-11cf-96B8-444553540000"');
	document.write('  codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=6,0,0,0" ');
	document.write(' ID="graphs" WIDTH="482" HEIGHT="269" ALIGN="">');
	document.write(' <PARAM NAME=movie VALUE="graphs.swf"> <PARAM NAME=quality VALUE=high> <PARAM NAME=salign VALUE=LT> <PARAM NAME=bgcolor VALUE=#FFFFFF>  '); 
	document.write(' <EMBED src="graphs.swf" quality=high salign=LT bgcolor=#FFFFFF  ');
	document.write(' swLiveConnect=FALSE WIDTH="482" HEIGHT="269" NAME="graphs" ALIGN=""');
	document.write(' TYPE="application/x-shockwave-flash" PLUGINSPAGE="http://www.macromedia.com/go/getflashplayer">');
	document.write(' </EMBED>');
	document.write(' </OBJECT>');
} else{
	document.write('<IMG SRC="" WIDTH="482" HEIGHT="269" usemap="#" BORDER=0>');
}
//-->
</SCRIPT><NOSCRIPT><IMG SRC="" WIDTH="482" HEIGHT="269" usemap="#" BORDER=0></NOSCRIPT>
</BODY>
</HTML>
