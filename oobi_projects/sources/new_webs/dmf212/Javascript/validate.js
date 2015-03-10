// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Form Validation Functions - validate.js
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
// Name:        validateAndSubmit
// Description: Validations form elements according to their extensions, then strips
//              off the extensions before submitting them.
// Usage:       <input type="button" name="set" value="Set"
//              onClick="validateAndSubmit()">
//
// ****************************************************************************


function validateAndSubmit()
{
   // Add validation here.
   for (i = 0; i < document.theForm.length; i++)
   {
      var theName = document.theForm.elements[i].name;
      // Validation form element according to its extension
      if ((theName.indexOf(".txt")) != -1) // Text elements cannot be blank.
      {
         if (document.theForm.elements[i].value == "")
         {
            alert("Please fill in the form completely.");
            return;
         }
         theName = theName.substring(0, theName.length - 4)
         document.theForm.elements[i].name = theName;
      }
   }
   document.theForm.submit();
}
