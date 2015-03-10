// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Parameter Controls - controls.js
//
// Version: 1.02
//
// Art & Logic, Inc.
// http://www.artlogic.com
// info@artlogic.com
//
// COPYRIGHT
//    The cookies library is available freely to the public, as per the
//    license below.
//
// NOTES
//    Alternative markup delemeters (single square brackets [...]) are used
//    throughout this library to describe function usage, so that the markups
//    won't get resolved accidentally.
//
//
// ****************************************************************************


// ****************************************************************************
// Name:        Cookies (gGetCookie, gSetCookie, gDeleteCookie)
// Description: Everything you need to set and get cookies.
//              These cookie functions were originally released into
//              the public domain by Bill Dortch <bdortch@netw.com>
//              and were modified by Art & Logic.
//              For more information, see Netscape's cookie spec at:
//              http://www.netscape.com/newsref/std/cookie_spec.html
// Usage:       var expdate = new Date ();
//              expdate.setTime (expdate.getTime() + (100 * 365 * 24 * 60 * 60 * 1000)); // 24 hrs from now
//              alert(expdate);
//              gSetCookie ("ccpath", "http://www.hidaho.com/colorcenter/", expdate);
//              gSetCookie ("ccname", "hIdaho Design ColorCenter", expdate);
//              gSetCookie ("tempvar", "This is a temporary cookie.");
//              gSetCookie ("ubiquitous", "This cookie will work anywhere in this domain",null,"/");
//              gSetCookie ("paranoid", "This cookie requires secure communications",expdate,"/",null,true);
//              gSetCookie ("goner", "This cookie must die!");
//              document.write (document.cookie + "<br>");
//              gDeleteCookie ("goner");
//              document.write ("ccpath = " + gGetCookie("ccpath") + "<br>");
//              document.write ("ccname = " + gGetCookie("ccname") + "<br>");
//              document.write ("tempvar = " + gGetCookie("tempvar") + "<br>");
//              alert (document.cookie)
// ****************************************************************************

// "Internal" function to return the decoded value of a cookie.

function getCookieVal(offset)
{
   var endstr = document.cookie.indexOf (";", offset);
   if (endstr == -1)
      endstr = document.cookie.length;
   return unescape(document.cookie.substring(offset, endstr));
}


// Function to return the value of the cookie specified by "name".
// name - String object containing the cookie name.
// returns - String object containing the cookie value, or null if
// the cookie does not exist.

function gGetCookie(name)
{
   var arg = name + "=";
   var alen = arg.length;
   var clen = document.cookie.length;
   var i = 0;
   while (i < clen)
   {
      var j = i + alen;
      if (document.cookie.substring(i, j) == arg)
         return getCookieVal (j);
      i = document.cookie.indexOf(" ", i) + 1;
      if (i == 0) break;
   }
   return "";
}


// Function to create or update a cookie.
// name -      String object object containing the cookie name.
// value -     String object containing the cookie value.  May contain
//             any valid string characters.
// [expires] - Date object containing the expiration data of the cookie.  If
//             omitted or null, expires the cookie at the end of the current session.
// [path] -    String object indicating the path for which the cookie is valid.
//             If omitted or null, uses the path of the calling document.
// [domain] -  String object indicating the domain for which the cookie is
//             valid.  If omitted or null, uses the domain of the calling document.
// [secure] -  Boolean (true/false) value indicating whether cookie transmission
//             requires a secure channel (HTTPS).
//
// The first two parameters are required.  The others, if supplied, must
// be passed in the order listed above.  To omit an unused optional field,
// use null as a place holder.  For example, to call gSetCookie using name,
// value and path, you would code:
//
//      gSetCookie ("myCookieName", "myCookieValue", null, "/");
//
// Note that trailing omitted parameters do not require a placeholder
//
// To set a secure cookie for path "/myPath", that expires after the
// current session, you might code:
//
//      gSetCookie (myCookieVar, cookieValueVar, null, "/myPath", null, true);

function gSetCookie(name, value)
{
   var argv = gSetCookie.arguments;
   var argc = gSetCookie.arguments.length;
   var expires = (argc > 2) ? argv[2] : null;
   var path = (argc > 3) ? argv[3] : null;
   var domain = (argc > 4) ? argv[4] : null;
   var secure = (argc > 5) ? argv[5] : false;
   var auth = gGetCookie ("auth");

   document.cookie = name + "=" + escape (value) +
      ((expires == null) ? "" : ("; expires=" + expires.toGMTString())) +
      ((path == null) ? "" : ("; path=" + path)) +
      ((domain == null) ? "" : ("; domain=" + domain)) +
      ((secure == true) ? "; secure" : "");

   if ( name == "auth" ) gSetCookie (name, auth);
}

// Function to delete a cookie. (Sets expiration date to current date/time)
// name - String object containing the cookie name


function gDeleteCookie (name)
{
   var exp = new Date();
   exp.setTime (exp.getTime() - 1);  // This cookie is history
   var cval = gGetCookie (name);
   document.cookie = name + "=" + cval + "; expires=" + exp.toGMTString();
}
