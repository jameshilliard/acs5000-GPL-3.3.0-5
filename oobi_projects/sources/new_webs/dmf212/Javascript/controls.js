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
// Name:        displayROString
// Description: Displays a simple string, or a forced blank space.
//              This comes in handy when displaying data in table cells.
// Usage:       displayROEnum(2,"cat","dog","spider","horse");
//
// ****************************************************************************


function displayROString(string)
{
   if (string == '')
   {
      document.write('&nbsp;');
   }
      else
   {
      document.write(string);
   }
}


// ****************************************************************************
// Name:        displayROEnum
// Description: Displays a string given a list of strings and a number.
//
// Usage:       displayROEnum(2,"cat","dog","spider","horse");
//
// ****************************************************************************


function displayROEnum()
{
   var l = displayROEnum.arguments.length;
   var a = new Array();
   var i;
   for (i=0; i<l; i++)
   {
      a[i] = displayROEnum.arguments[i];
   }
   i = 2;
   while ((i<l) && (a[0]!=a[i]))
   {
      i = i+2;
   }
   if (i == l)
   {
      document.write(a[1]);
   }
      else
   {
      document.write(a[i+1]);
   }
}


// ****************************************************************************
// Name:        writeSelectionListOption
// Description: Creates selection list option tags with the default
//              automatically selected.
// Usage:       writeSelectionListOption('[sysColor]',1,'Orange');
//              writeSelectionListOption('[sysColor]',2,'Blue');
//              writeSelectionListOption('[sysColor]',3,'Black');
// ****************************************************************************


function writeSelectionListOption(resolvedMarkup,value,label)
{
   if (resolvedMarkup == value)
   {
      var code = 'document.write("<option value=' + value + ' selected>' + label + '");';
      eval(code);
   }
      else
   {
      var code = 'document.write("<option value=' + value + '>' + label + '");';
      eval (code);
   }
}


// ****************************************************************************
// Name:        writeAdvancedCheckbox
// Description: Creates a checkbox and decides whether or not it
//              should be checked by default.
// Usage:       writeAdvancedCheckbox('[city]','city','Seattle');
//              writeAdvancedCheckbox('[city]','city','Boston');
// ****************************************************************************


function writeAdvancedCheckbox(resolvedMarkup,objectName,value)
{
   if (resolvedMarkup == value)
   {
      // create a checked checkbox.
      var code = 'document.write("';
      code += '<input type=checkbox name=' + objectName + ' value=' + value + ' checked>';
      code += '");';
      eval(code);
   }
      else
   {
      // create an unchecked checkbox.
      var code = 'document.write("';
      code += '<input type=checkbox name=' + objectName + ' value=' + value;
      code += '>");';
      eval(code);
   }
}


// ****************************************************************************
// Name:        writeAdvancedRadio
// Description: Creates a radio button and decides whether or not it
//              should be checked (selected) by default.
// Usage:       writeAdvancedRadio('[city]','city','Seattle');
//              writeAdvancedRadio('[city]','city','Boston');
// ****************************************************************************


function writeAdvancedRadio(resolvedMarkup,objectName,value)
{
   if (resolvedMarkup == value)
   {
      // create a checked radio button.
      var code = 'document.write("';
      code += '<input type=radio name=' + objectName + ' value=' + value + ' checked>';
      code += '");';
      eval(code);
   }
      else
   {
      // create an unchecked radio button.
      var code = 'document.write("';
      code += '<input type=radio name=' + objectName + ' value=' + value;
      code += '>");';
      eval(code);
   }
}


// ****************************************************************************
// Name:        drawFormSection
// Description: This code would typically appear on an HTML file which
//              contains form elements that are visible/invisible
//              depending on [QueryValue()] which was passed into
//              the page.
//              Variations of this function should be used in
//              conjunction with writeSelectionListOption(), ### others also like displayListHeading
//              writeAdvancedRadio(), and writeAdvancedCheckbox().
// Usage:       drawIpAccessGroup(); - creates normal form elements.
//              drawIpAccessGroup(); - creates hidden form elements.
// ****************************************************************************


function drawFormSection(visibility)
{
   if (visibility == 'hidden')
   {
      document.write('<input type="hidden" name="subscriberAccessGroupIn" value="">');
      document.write('<input type="hidden" name="subscriberAccessGroupOut" value="1234">');
   }
      else
   {
      rowBegin();
         displayListHeading('IP Access Group:');
         dataBegin();
            document.write('<table border=0 cellspacing=0>');
               document.write('<tr><td>In:</td><td><input type="text" name="subscriberAccessGroupIn" value=""></td></tr>');
               document.write('<tr><td>Out:</td><td><input type="text" name="subscriberAccessGroupOut" value="1234"></td></tr>');
            document.write('</table>');
         dataEnd();
      rowEnd();
   }
}
