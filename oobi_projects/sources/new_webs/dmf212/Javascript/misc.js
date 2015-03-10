// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Miscellaneous Functions - misc.js
//
// Version: 1.02
//
// Art & Logic, Inc.
// http://www.artlogic.com
// info@artlogic.com
//
// COPYRIGHT
//    This library is not freely available to the public. If you would like to
//    purchase a license, contact Art & Logic.
//
// NOTES
//    Alternative markup delemeters (single square brackets [...]) are used
//    throughout this library to describe function usage, so that the markups
//    won't get resolved accidentally.
//
//
// ****************************************************************************


// ****************************************************************************
// Name:        Table Functions
// Description: These functions are used to create HTML tables.  Why
//              use functions to create tables?  Because, unless we do
//              it this way, many JavaScript bugs will occur under
//              Netscape 3.0 for UNIX.  We implemented these functions
//              because most of our clients want Netscape 3.0 for UNIX
//              compliance.  There are dozens of cross-issues, but this one was worth it.
//
//              Furthermore, these functions make it easy to design
//              and modify consistent looking tables across the
//              entire web application.
// Usage:       drawIpAccessGroup(); - creates normal form elements.
//              drawIpAccessGroup(); - creates hidden form elements.
// ****************************************************************************

function tableBegin(width)
{
   document.write('<table width=' + width + ' cellpadding=0 cellspacing=0 border=0><tr><td bgcolor=#000000>');
   document.write('<table width=100% cellpadding=2 cellspacing=2 border=0>');
}

function tableEnd()
{
   document.write('</table></td></tr></table>');
}

function rowBegin()
{
   document.write('<tr>');
}

function rowEnd()
{
   document.write('</tr>');
}

// If aligment is not specified, the cell contents will align left, the default value.
function dataBegin(alignment)
{
   document.write('<td bgcolor="#ffffff" align=' + alignment + '><font face="arial,helvetica">');
}

function displayPageHeader(heading)
{
   document.write('<table width=100% cellpadding=0 cellspacing=0 border=0><tr><td bgcolor=#000000>');
   document.write('<table width=100% cellpadding=4 cellspacing=2 border=0><tr><td bgcolor="#cccccc">');
   document.write('<font face="courier new,courier,arial" size="+2" color="#000000"><b>');
   document.write(heading);
   document.write('</b></font></td></tr></table></td></tr></table>');
}

function dataEnd()
{
   document.write('</font></td>');
}

function displayTableHeading(heading)
{
   document.write('<td bgcolor="#6699cc" align="center"><font face="arial,helvetica"><b>',heading,'</b></font></td>');
}

function displayListHeading(heading)
{
   document.write('<td bgcolor="#6699cc" valign="top"><font face="arial,helvetica"><b>',heading,'</b></font></td>');
}


function displayUrl(url,visualLink)
{
   document.write('<a href="',url,'">',visualLink,'</a>');
}


// ****************************************************************************
// Name:        Fix array bug in Netscape 4.05 vs. 4.08.
// Description: 
// Usage:
// ****************************************************************************


  // fix for horrible bug in Netscape 4.05 split function
  var versionString = navigator.appVersion;
  var versionArray = versionString.split(' ');
  var theVersion = versionArray[0];
  if (theVersion <= "4.03")
  {
    i = 3;
  } else {
    i = 4; // !!!
  }
  re = /\s*\s/;
  for (x=0; x <= numImages-1; x++)
  {
    var fileValues = fileArray[x].split(re);
    fileName = fileValues[i];
    createImageListEntry(fileName,fileArray[x]);
  }


// ****************************************************************************
// Name:        printArray()
// Description: Prints an array, for testing
// Usage:
// ****************************************************************************

function printArray()
{
   // print the contents of the array for testing purposes
   var x;
   var y;
   for (x=1; x <= 21; x++)
   {
      str = "row (slot number) "+x+":"
      for (y=1; y <= 3; y++)
      {
         str += a[x][y];
         str += "&nbsp;&nbsp;&nbsp;&nbsp;";
      }
      document.write(str,"<br>")
   }
}


// ****************************************************************************
// Name:        gUnloadCacheFix and gLoadCacheFix
// Description: This pair of functions forces a page to refresh once so that cached
//       documents are never displayed.  This is a workaround for IE's lack
//       of support for the "Pragma" META tag, and incorrect implementation
//       of the "Expires" tag.
// Usage:       <body onLoad="gLoadCacheFix()" onUnload="gUnloadCacheFix()">
// ****************************************************************************

var timerID = null;
var getTotal = 0;
var timerRunning = false;

function gUnloadCacheFix()
{
   if (timerRunning)
         clearTimeout(timerID);
   timerRunning = false;
}

function gLoadCacheFix()
{
      var now = new Date();
      gUnloadCacheFix();
      getTotal = now.getMinutes();
}


// ****************************************************************************
// Name:        adminType
// Description: Code for systems in which there is more than one user level, and
//       certain features are restricted to users of certain levels.  In the example
//       users of levels 2 and 3 only will see the "Set" button.
// Usage:    Test for the user level, then use document.write to write the
//       restricted feature.
// ****************************************************************************

var adminType;
x = '[userLevel]';
if ((x == 2) || (x == 3))
{
   adminType = true;
} else
{
   adminType = false;
}

// Use within HTML code
if (adminType)
{
   document.write("<td bgcolor=\"#FFFFFF\"  align=center><input id=\"button\" type=\"button\" name=\"set\" value=\"Set\" onClick=\"validateAndSubmit()\"></td>");
}


// ****************************************************************************
// Name:        formatTime
// Description: This function converts time as returned by markups such as sysUpTime
//       to days:hours:minutes:seconds format.
// Usage:       ticks = "[sysUpTime]";
//       formatTime(ticks);
// ****************************************************************************

function formatTime(ticks)
{
   if (isNaN(ticks))
      ticks = 0;
   days = Math.floor(ticks / (100*60*60*24));
   ticks = ticks - (days*100*60*60*24);
   hours = Math.floor(ticks / (100*60*60));
   ticks = ticks - (hours*100*60*60);
   minutes = Math.floor(ticks / (100*60));
   ticks = ticks - (minutes*100*60);
   seconds = Math.floor(ticks / 100);
   document.write(days);
   document.write(":");
   if (hours < 10)
      document.write("0");
   document.write(hours);
   document.write(":");
   if (minutes < 10)
      document.write("0");
   document.write(minutes);
   document.write(":");
   if (seconds < 10)
      document.write("0");
   document.write(seconds);
}


