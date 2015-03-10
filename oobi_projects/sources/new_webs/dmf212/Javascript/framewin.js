// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Frame and Window Functions - framewin.js
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
// Name:        openNewWindow
// Description: Opens a new browser window with custom window features.
// Usage:       openNewWindow('file.htm?instance=5','newWindowName'
//              ,800,500,'scrollbars','resizable','status');
// ****************************************************************************


function openNewWindow(url, name)
{
   var l = openNewWindow.arguments.length;
   var w = "";
   var h = "";
   var features = "";
   for (i=2; i<l; i++)
   {
      var param = openNewWindow.arguments[i];
      if ( (parseInt(param) == 0) || (isNaN(parseInt(param))) )
      {
         features += param + ',';
      }
         else
      {
         (w == "") ? w = "width=" + param + "," : h = "height=" + param;
      }
   }
   features += w + h;
   var code = "popupWin = window.open(url, name";
   if (l > 2) code += ", '" + features + "'";
   code += ")";
   eval(code);
}


// ****************************************************************************
// Name:        Window Focus
// Description: Bug fix for cross-browser window focus problems.
// Usage:       For use with openNewWindow().  This single line of
//              code should appear at the top of every page that is
//              opened with openNewWindow.
// ****************************************************************************


if (!((navigator.appName != 'Netscape') && (navigator.appVersion.indexOf('Win') == -1)))
{
   if (window.focus) self.focus();
}


// ****************************************************************************
// Name:        reDo
// Description: A patch for redraw problem in NN when the user resizes the Window.
// Usage:    Needs to be called onResize in <body> or outermost <frameset> tag.
//
// ****************************************************************************
  var nn;
  if (navigator.appName == "Netscape") nn=true;

  if (nn)
  {
    origWidth = innerWidth;
    origHeight = innerHeight;
  }

  function reDo()
  {
    if (innerWidth != origWidth || innerHeight != origHeight)
    location.replace("Index.htm?tab=$%QueryValue(tab)#$");
  }
