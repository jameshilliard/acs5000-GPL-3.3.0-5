// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Image Functions - images.js
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
// Name:        changeImage
// Description: Swaps a PNG, GIF, or JPG image with a new one.
//              This will only work if all the images are pre-loaded
//              into the browser's memory.
// Usage:
// ****************************************************************************


function changeImage()
{
   if (document.images)
   {
      for (var i=0; i<changeImage.arguments.length; i+=2)
      {
         document[changeImage.arguments[i]].src = eval(changeImage.arguments[i+1] + ".src");
      }
   }
}


// ****************************************************************************
// Name:        Load images
// Description: Loads images into the browser's memory.  In most cases
//              this code will need to be placed in the HTML file
//              where the images are to be displayed.
// Usage:       <a href="file.html" onMouseOver="changeImages('image2'
//              ,'image2on')" onMouseOut="changeImages('image2'
//              ,'image2off')"><img name="image2" src="x.gif"></a>
// ****************************************************************************


if (document.images)
{
   notch_on = new Image();
   notch_off = new Image();
   notch_transparent = new Image();
   notch_tick = new Image();

// notch_on.src = "image1.gif";
// notch_off.src = "image2.gif";
// notch_transparent.src = "image3.gif";
// notch_tick.src = "image4.gif";
}
