// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Bar Chart Functions - barchart.js
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
// ****************************************************************************


// ********************************************************************
// ********************************************************************
// Draws a bar chart with horizontal bars.
// Writes the HTML code to the document.
// 
// barValues: the values to be displayed (array of zero or more nonnegative numbers).
// maxValue: the maximum value to display (positive number).
// barLabels: labels for the values (array of zero or more strings, same length as value array).
// chartWidthPixels: the rough width allowed for the bars on the page (positive number).
// barHeightPixels: the height of each bar on the page (positive number).
// barImageFile: the name of the image used to draw each bar (it will be stretched).
// Returns: whether the function succeeded (true/false).
// 
// If an input parameter has the wrong type, the function returns false.
// The function silently corrects any range errors in the input parameters as specified above.
// If the array sizes don't match, it takes the smaller size.
// If maxValue is violated, though, it truncates the bar and flags the value label.

// In order to use this as a histogram, the following variables are defined in
// file in which the graphs are displayed:

/*
var barLabels = new Array("now", "-10 sec", "-20 sec", "-30 sec", "-40 sec", "-50 sec");
var chartWidthPixels = 250;
var barHeightPixels = 12;
var barImageFile = "bar.gif";
*/

// The page must be set to periodically refresh:
// <META HTTP-EQUIV="Refresh" CONTENT="10">

// and persistent variables for each graph need to be stored in the file's parent:

/*
var maxValue1 = 0;
var theArray1 = new Array(6);
var maxCounter1; 
*/

// All the arrays must be initialized in the parent:

/*
// Build original theArray with 0-initialized values
for ( i=0; i < 6; i++)
{
   theArray1[i] = 0;
   theArray2[i] = 0;
   theArray3[i] = 0; // Add additional lines for additional graphs
}

// Each time the page is re-entered the values must be reset.  This cannot be done onLoad since the 
// page is set to auto-refresh, so this function should be put in the parent and called when the 
// link to the page is clicked, for example: <a href="gr_dat1.htm" onClick="parent.resetArrays()">Data</a>

// Reset all arrays.  This is called by links to graph pages in order to reinitialize the arrays.
function resetArrays()
{
   for ( i=0; i < 6; i++)
   {
      theArray1[i] = 0;
      theArray2[i] = 0;
      theArray3[i] = 0; // Add additional lines for additional graphs
   }
}
*/

function drawHorzBarChart(barValues, maxValue, barLabels, chartWidthPixels,
   barHeightPixels, barImageFile)
{
   var code = "";

   var minValue = 0;
   
   // Make sure the input makes sense.
   if (!barValues.length)
      return false;
   for (var i = 0; i < barValues.length; i++)
   {
      if (isNaN(barValues[i]))
         return false;
   }
   if (isNaN(maxValue))
      return false;
   if (!barLabels.length)
      return false;     
   if (isNaN(chartWidthPixels))
      return false;
   if (isNaN(barHeightPixels))
      return false;

   // Make sure the input parameters are in range.
   for (var i = 0; i < barValues.length; i++)
   {
      barValues[i] = Math.max(barValues[i], 0);
   }
   maxValue = Math.max(maxValue, 1);
   chartWidthPixels = Math.max(chartWidthPixels, 1);
   barHeightPixels = Math.max(barHeightPixels, 1);
   
   // The bar chart is a table with three columns: the middle one is for the bars,
   // and the left and right are for labels.

   code += "\n<table border=0 cellspacing=0 cellpadding=0>";

   // Scale row.
   code += "\n<tr>";
   code += "\n<td></td>";
   code += "\n<td>";
   code += "<table border=0 cellspacing=0 cellpadding=0 width=" + chartWidthPixels+ ">";
   code += "\n<tr>";
   code += "\n<td align=left>" + formatScaleLabel(minValue) + "</td>";
   code += "\n<td align=right>" + formatScaleLabel(maxValue) + "</td>";
   code += "\n</tr>";
   code += "\n</table>";
   code += "\n</td>";
   code += "\n<td></td>";
   code += "\n</tr>";

   // Bar rows.
   var barTotal = Math.min(barValues.length, barLabels.length);
   for (var i = 0; i < barTotal; i++)
   {
      var value = barValues[i];
      var label = barLabels[i];
      var overflow = (value > maxValue);

      code += "\n<tr>";

      // Label.
      code += "\n<td align=right>";
      code += formatBarLabel(label);
      code += "&nbsp;</td>";

      // Bar.
      // The bar lies below both scale labels at the top.
      code += "\n<td colspan=2>";
      code += "<img";
      code += " src=" + barImageFile;
      code += " height=" + barHeightPixels;
      var barWidth = chartWidthPixels;
      if (!overflow)
         barWidth *= (value / maxValue);
      // Navigator 4 PC seems to treat width = 0 as width = height.
      if (barWidth == 0)
         barWidth = 1;
      code += " width=" + barWidth;
      code += ">";
      if (!overflow)
         code += "&nbsp;" + formatBarValueLabel(value);
      code += "</td>";

      // Overflow label if necessary.
      code += "\n<td>"
      if (overflow)
         code += "&nbsp;" + formatBarOverflowValueLabel(value);
      code += "</td>";

      code += "\n</tr>";
   }

   code += "\n</table>";

   document.write(code);
   
   return true;
}

function formatScaleLabel(label)
{
   return "<font size=-1><b>" + label + "</b></font>";
}

function formatBarLabel(label)
{
   return label;
}

function formatBarValueLabel(label)
{
   return "<font size=-1>" + label + "</font>";
}

function formatBarOverflowValueLabel(label)
{
   return "<font size=-1 color=red>" + label + "</font>";
}

function callGraph(theIndex, theValue)
{
   eval("var maxValue = parent.maxValue" + theIndex);
   eval("var oldArray = parent.theArray" + theIndex); // Array of values from last time
   var barValuesArray = new Array(6);
   // Build original barValuesArray with 0-initialized values
   for ( i=0; i < 6; i++)
   {
      barValuesArray[i] = 0;
   }
   barValuesArray[0] = theValue;
   // From barValuesArray[1] on put old values in new array, with values shifted one place down.
   for ( i=1; i < 6; i++)
   {
      barValuesArray[i] = oldArray[(i-1)];
   }
   
   var code = "parent.maxCounter" + theIndex;
   var tempCounter = eval(code);

   if (barValuesArray[0] > maxValue)
   {
      maxValue = barValuesArray[0];
      eval("parent.maxValue" + theIndex + " = maxValue;");
      eval("parent.maxCounter" + theIndex + " = 0;");
   }
   else if (tempCounter > 5)
   {
   // Iterate through the barValuesArray to find the largest value.
      maxValue = barValuesArray[0];
      for ( i=1; i < 6; i++)
      {
         if (barValuesArray[i] > maxValue)
         {
            maxValue = barValuesArray[i];
            eval("parent.maxCounter" + theIndex + " = i;");
         }
      }
      eval("parent.maxValue" + theIndex + " = maxValue;");
   }
   eval("parent.maxCounter" + theIndex + "++;");
   eval("parent.theArray" + theIndex + " = barValuesArray;"); // Put current values in the data storage frame for next time.
   drawHorzBarChart(barValuesArray, maxValue, barLabels, chartWidthPixels, barHeightPixels, barImageFile);
}
