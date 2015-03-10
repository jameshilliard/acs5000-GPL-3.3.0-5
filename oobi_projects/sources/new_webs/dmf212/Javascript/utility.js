// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Utilities - utility.js
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

function debugWrite(x) {
   debugWriteArray(arguments);
}

function debugWriteArray(x)
{
   for (var i = 0; i < x.length; i++)
      write(x[i] + " ");
   write("<br>");
}

function debugAssert(expression)
{
   debugAssertEqual(expression, true);
}

function debugAssertEqual(expression0, expression1)
{
   if (expression0 != expression1) {
      debugWrite("!!!DEBUG ASSERTION FAILED");
      debugWrite("expecting: " + expression1);
      debugWrite("found: " + expression0);
      // Stop the interpreter.
      eval("(;");
   }
}

function stringCompare(string0, string1) {
   if (string0 < string1)
      return -1;
   else if (string0 > string1)
      return 1;
   else
      return 0;
}

function stringCompareNoCase(string0, string1) {
   var string0Lower = string0.toLowerCase();
   var string1Lower = string1.toLowerCase();
   return stringCompare(string0Lower, string1Lower);
}

function numberCompare(x0, x1) {
   if (x0 < x1)
      return -1;
   else if (x0 > x1)
      return 1;
   else
      return 0;
}

// The objects should define the "compareTo" function.
function doHeapSort(list) {
   // Heapsort, adapted from _Numerical Recipes in C_, Press, second edition.
   // Changed format for Java (made indexes 0-based), renamed variables.
   // Changed format slightly for JavaScript.

   var n = list.length;

   // We first insert elements into the heap to put them in order
   //  (it's enough to insert the top half), then we remove all of them from the
   //  heap in that order.
   if (n < 2)
      return;
   // Note that JavaScript will not do integer arithmetic here!
   var insert = Math.floor(n / 2);
   var remove = n - 1;
   for (;;) {
      var entry;
      if (insert > 0) {
         --insert;
         entry = list[insert];
      } else {
         entry = list[remove];
         list[remove] = list[0];
         --remove;
         if (remove == 0) {
            list[0] = entry;
            break;
         }
      }
      var parent = insert;
      var child = (insert * 2) + 1;
      while (child <= remove) {
         if (child < remove && list[child].compareTo(list[child + 1]) < 0)
            child++;
         if (entry.compareTo(list[child]) < 0) {
            list[parent] = list[child];
            parent = child;
            child = child * 2 + 1;
         } else {
            child = remove + 1;
         }
      }
      list[parent] = entry;
   }
}

//testHeapSort();
function testHeapSort() {
   debugWrite("testHeapSort");

   // Check that two randomly scrambled lists sort to the same list.
   // Try small array sizes.
   var testCaseList = new Array(
    new Array(
    ),
    new Array(
    "1/1/99"
    ),
    new Array(
    "1/1/99",
    "1/2/99"
    ),
    new Array(
    "1/1/99",
    "1/2/99",
    "1/3/99"
    ),
    new Array(
    "1/1/99",
    "1/2/99",
    "1/3/99",
    "1/4/99"
    ),
    // Include a bunch of duplicates in the list, for fun.
    new Array(
    "10/3/98",
    "10/3/98",
    "10/5/99",
    "7/8/98",
    "3/4/98",
    "10/6/99",
    "11/5/99",
    "7/8/98",
    "3/6/98",
    "10/12/99",
    "10/3/98",
    "1/3/98",
    "10/5/99",
    "7/8/98"
    )
    );
   for (var caseIndex = 0; caseIndex < testCaseList.length; caseIndex++) {
      debugWrite("case " + caseIndex + ":");
      var stringList = testCaseList[caseIndex];
      var length = stringList.length;
      var dateList0 = new Array();
      var dateList1 = new Array();
      for (var i = 0; i < length; i++) {
         dateList0.push(new MyDate(stringList[i]));
         dateList1.push(new MyDate(stringList[i]));
      }
      // Scramble.
      for (var i = 0; i < length; i++) {
         var temp;
         var iNew;

         iNew = i + Math.floor(Math.random() * (length - i));
         temp = dateList0[i]; dateList0[i] = dateList0[iNew]; dateList0[iNew] = temp;

         iNew = i + Math.floor(Math.random() * (length - i));
         temp = dateList1[i]; dateList1[i] = dateList1[iNew]; dateList1[iNew] = temp;
      }
      // Print.
      debugWrite("list 0:");
      for (var i = 0; i < length; i++) {
         debugWrite("&nbsp;" + dateList0[i].toString());
      }
      debugWrite("list 1:");
      for (var i = 0; i < length; i++) {
         debugWrite("&nbsp;" + dateList1[i].toString());
      }
      // Sort.
      doHeapSort(dateList0);
      doHeapSort(dateList1);
      // Print.
      debugWrite("after sort:");
      debugWrite("list 0:");
      for (var i = 0; i < length; i++) {
         debugWrite("&nbsp;" + dateList0[i].toString());
      }
      debugWrite("list 1:");
      for (var i = 0; i < length; i++) {
         debugWrite("&nbsp;" + dateList1[i].toString());
         debugAssert(dateList0[i].compareTo(dateList1[i]) == 0);
      }
   }

   debugWrite("testHeapSort succeeded");
}

// Text breaker class.
// This could probably be replaced by the JS 1.2 regular expression functions.

function textBreakerGetNextPiece() {
   // Move past the old delimiter.
   this.pieceStart = this.pieceEnd + this.delimiter.length;
   this.pieceEnd = this.text.indexOf(this.delimiter, this.pieceStart);
   // If there are no more delimiters, take the rest of the string.
   if (this.pieceEnd == -1)
      this.pieceEnd = this.text.length;
}

function textBreakerCurrentPiece() {
   if (this.pieceStart > this.pieceEnd)
      return null;
   return this.text.substring(this.pieceStart, this.pieceEnd);
}

// Breaks a string into pieces using a delimiter string.
function TextBreaker(text, delimiter) {
   this.text = text;
   this.delimiter = delimiter;
   this.pieceStart = 0;
   // Set to get the first piece.
   this.pieceEnd = -this.delimiter.length;

   this.getNextPiece = textBreakerGetNextPiece;
   this.currentPiece = textBreakerCurrentPiece;

   this.getNextPiece();
}

//testTextBreaker();
function testTextBreaker() {
   debugWrite("testTextBreaker");

   var testCaseArray = new Array(
    new Array("fiji islands", " ", "fiji", "islands"),
    new Array(" string0 string1 ", " ", "", "string0", "string1", ""),
    new Array("", " ", ""),
    new Array("x", "x", "", ""),
    new Array("xx", "x", "", "", ""),
    new Array(" a, b, c,d, e", ", ", " a", "b", "c,d", "e"),
    new Array(", ", ", ", "", ""),
    new Array("\ts\t", "\t", "", "s", ""),
    new Array("\ns\n", "\n", "", "s", "")
    );

   for (var i = 0; i < testCaseArray.length; i++) {
      var testCase = testCaseArray[i];
      debugWriteArray(testCase);

      var text = testCase[0];
      var delimiter = testCase[1];
      var breaker = new TextBreaker(text, delimiter);
      var piece = null;
      for (var j = 2; j < testCase.length; j++) {
         piece = breaker.currentPiece();
         debugAssertEqual(piece, testCase[j]);
         breaker.getNextPiece();
      }
      piece = breaker.currentPiece();
      debugAssertEqual(piece, null);
   }

   debugWrite("testTextBreaker succeeded");
}

// MyDate class.
// We define our own date class instead of using Date to guarantee features
// like well-defined string format, good Y2K behavior, and some arithmetic
// operations such as comparison.

// Converts from "mm/dd/yy" or "mm/dd/yyyy".
// This is the format used by the Lasso Server_Date substitution tag.
// Years 0-49 mean 2000-2049; years 50-99 mean 1950-1999.
function myDateParseShort(text) {
   var fieldBreaker = new TextBreaker(text, "/");
   var numberList = new Array();
   for (var i = 0; i < 3; i++) {
      var field = fieldBreaker.currentPiece();
      if (null == field)
         return false;
      numberList[i] = parseInt(field, 10);
      fieldBreaker.getNextPiece();
   }
   if (null != fieldBreaker.currentPiece())
      return false;

   var month = numberList[0];
   if (isNaN(month) || month < 1 || month > 12)
      return false;
   var day = numberList[1];
   if (isNaN(day) || day < 1 || day > 31)
      return false;
   var year = numberList[2];
   if (isNaN(year) || year < 0 || year > 9999)
      return false;
   // Two-digit year?
   if (year <= 99)
      year += (year < 50 ? 2000 : 1900);

   this.month = month;
   this.day = day;
   this.year = year;

   // Check number of days in the month.
   if (this.day > this.monthDays())
      return false;

   return true;
}

function myDateMonthDays()
{
   var isLeapYear = ((this.year % 4 == 0 && this.year % 100 != 0) ||
    (this.year % 400) == 0);
   var monthDaysList = new Array(
      31,
      28 + (isLeapYear ? 1 : 0),
      31,
      30,
      31,
      30,
      31,
      31,
      30,
      31,
      30,
      31
   );

   return monthDaysList[this.month - 1];
}

// Like Java method: -1 for "less than", 1 for "greater than", 0 for "equal".
function myDateCompareTo(otherDate) {
   if (this.year != otherDate.year)
      return (this.year > otherDate.year ? 1 : -1);
   if (this.month != otherDate.month)
      return (this.month > otherDate.month ? 1 : -1);
   if (this.day != otherDate.day)
      return (this.day > otherDate.day ? 1 : -1);
   return 0;
}

// Outputs as "mm/dd/yyyy".
function myDateToString() {
   var monthString = "" + this.month;
   var dayString = "" + this.day;
   var yearString = "" + this.year;
   while (monthString.length < 2)
      monthString = "0" + monthString;
   while (dayString.length < 2)
      dayString = "0" + dayString;
   while (yearString.length < 4)
      yearString = "0" + yearString;
   return monthString + "/" + dayString + "/" + yearString;
}

function myDateAddYears(change)
{
   this.year += change;
}

function myDateAddMonths(change)
{
   this.month += change;
   this.normalize();
}

function myDateAddDays(change)
{
   this.day += change;
   this.normalize();
}

function myDateNormalize()
{
   this.normalizeMonth();

   while (this.day > this.monthDays())
   {
      this.day -= this.monthDays();
      this.month++;
      this.normalizeMonth();
   }
   while (this.day < 1)
   {
      this.month--;
      this.normalizeMonth();
      this.day += this.monthDays();
   }
}

function myDateNormalizeMonth()
{
   while (this.month < 1)
   {
      this.month += 12;
      this.year--;
   }
   while (this.month > 12)
   {
      this.month -= 12;
      this.year++;
   }
}

function myDateSubtract(otherDate)
{
   if (this.compareTo(otherDate) == -1)
      return -otherDate.subtract(this);
   var largerDate = new MyDate(this.toString());

   // Move the larger date back one month at a time
   // until it becomes the smaller.
   var difference = 0;
   while (largerDate.compareTo(otherDate) >= 0)
   {
      difference += largerDate.day;
      largerDate.day = 0;
      largerDate.normalize();
   }
   difference -= otherDate.day;

   return difference;
}

// Returns an integer: 0 for Sunday up to 6 for Saturday.
function myDateDayOfTheWeek()
{
   var referenceDate = new MyDate("1/1/01");
   var referenceDayOfTheWeek = 1;

   var day = referenceDayOfTheWeek + this.subtract(referenceDate);
   if (day < 0)
      return 7 - ((-day) % 7);
   return day % 7;
}

function MyDate()
{
   this.parseShort = myDateParseShort;
   this.monthDays = myDateMonthDays;
   this.toString = myDateToString;
   this.compareTo = myDateCompareTo;
   this.addYears = myDateAddYears;
   this.addMonths = myDateAddMonths;
   this.addDays = myDateAddDays;
   this.normalize = myDateNormalize;
   this.normalizeMonth = myDateNormalizeMonth;
   this.subtract = myDateSubtract;
   this.dayOfTheWeek = myDateDayOfTheWeek;

   // The fields are conventional 1-based values.

   if (arguments.length == 1)
      this.parseShort(arguments[0]);
   else
   {
      var oldDate = new Date();
      this.parseShort((oldDate.getMonth() + 1) + "/" + oldDate.getDate() + "/" +
       oldDate.getFullYear());
   }
}

//testMyDate();
function testMyDate()
{
   debugWrite("testMyDate");

   var testCaseArray;

   // Test parsing and validation.
   testCaseArray = new Array(
    new Array("10/25/99", 10, 25, 1999, "10/25/1999"),
    new Array("010/25/0", 10, 25, 2000, "10/25/2000"),
    new Array("10/12/1999", 10, 12, 1999, "10/12/1999"),
    new Array("12/31/2049", 12, 31, 2049, "12/31/2049"),
    new Array("12/31/670", 12, 31, 670, "12/31/0670"),
    new Array("12/31/000000006", 12, 31, 2006, "12/31/2006"),
    new Array("12/31/49", 12, 31, 2049, "12/31/2049"),
    new Array("1/1/50", 1, 1, 1950, "01/01/1950"),
    new Array("x/25/99", null),
    new Array("x/25/20001", null),
    new Array("/25/99", null),
    new Array("10/11/12/", null),
    new Array("0/25/99", null),
    new Array("-1/25/99", null),
    new Array("13/25/99", null),
    new Array("10/0/99", null),
    new Array("10/32/99", null),
    new Array("10/12/-1", null),
    // Test the different months (including leap years for Feb.).
    new Array("1/31/2000", 1, 31, 2000, "01/31/2000"),
    new Array("1/32/2000", null),
    new Array("2/28/2001", 2, 28, 2001, "02/28/2001"),
    new Array("2/29/2001", null),
    new Array("2/29/2004", 2, 29, 2004, "02/29/2004"),
    new Array("2/30/2004", null),
    new Array("2/28/2100", 2, 28, 2100, "02/28/2100"),
    new Array("2/29/2100", null),
    new Array("2/29/2000", 2, 29, 2000 , "02/29/2000"),
    new Array("2/30/2000", null),
    new Array("3/31/2000", 3, 31, 2000, "03/31/2000"),
    new Array("3/32/2000", null),
    new Array("4/30/2000", 4, 30, 2000, "04/30/2000"),
    new Array("4/31/2000", null),
    new Array("5/31/2000", 5, 31, 2000, "05/31/2000"),
    new Array("5/32/2000", null),
    new Array("6/30/2000", 6, 30, 2000, "06/30/2000"),
    new Array("6/31/2000", null),
    new Array("7/31/2000", 7, 31, 2000, "07/31/2000"),
    new Array("7/32/2000", null),
    new Array("8/31/2000", 8, 31, 2000, "08/31/2000"),
    new Array("8/32/2000", null),
    new Array("9/30/2000", 9, 30, 2000, "09/30/2000"),
    new Array("9/31/2000", null),
    new Array("10/31/2000", 10, 31, 2000, "10/31/2000"),
    new Array("10/32/2000", null),
    new Array("11/30/2000", 11, 30, 2000, "11/30/2000"),
    new Array("11/31/2000", null),
    new Array("12/31/2000", 12, 31, 2000, "12/31/2000"),
    new Array("12/32/2000", null)
    );
   // !!! Can we write "static" functions for a class?
   var date = new MyDate();
   for (var i = 0; i < testCaseArray.length; i++)
   {
      var testCase = testCaseArray[i];
      debugWriteArray(testCase);

      var text = testCase[0];
      var status = date.parseShort(text);
      if (null == testCase[1])
      {
         debugAssert(false == status);
      }
      else
      {
         debugAssert(true == status);
         debugAssert(date.month == testCase[1]);
         debugAssert(date.day == testCase[2]);
         debugAssert(date.year == testCase[3]);
         debugAssert(date.toString() == testCase[4]);
      }
   }

   // Test comparison.
   testCaseArray = new Array(
    new Array("6/9/99", "06/09/99", 0),
    new Array("6/9/99", "6/9/98", 1),
    new Array("6/9/99", "6/9/00", -1),
    new Array("6/9/99", "6/11/99", -1),
    new Array("6/11/99", "6/6/99", 1),
    new Array("7/9/99", "6/11/99", 1),
    new Array("6/11/99", "7/11/99", -1),
    new Array("1/1/99", "9/26/99", -1),
    new Array("10/18/34", "9/10/98", 1),
    new Array("11/25/14", "05/2/5", 1),
    new Array("02/10/99", "9/04/99", -1),
    new Array("9/25/99", "04/26/96", 1),
    new Array("3/31/97", "9/5/99", -1),
    new Array("10/5/02", "12/14/88", 1),
    new Array("8/13/0", "07/29/99", 1)
    );
   for (var i = 0; i < testCaseArray.length; i++)
   {
      var testCase = testCaseArray[i];
      debugWriteArray(testCase);

      var date0 = new MyDate(testCase[0]);
      var date1 = new MyDate(testCase[1]);
      debugAssertEqual(date0.compareTo(date1), testCase[2]);
   }

   // Test year addition.
   testCaseArray = new Array(
    new Array("2/17/91", 0, "2/17/91"),
    new Array("2/17/91", 1, "2/17/92"),
    new Array("2/17/91", 10, "2/17/01"),
    new Array("2/17/91", 100, "2/17/2091"),
    new Array("2/17/91", -1, "2/17/90"),
    new Array("2/17/91", -10, "2/17/81"),
    new Array("2/17/91", -100, "2/17/1891")
    );
   for (var i = 0; i < testCaseArray.length; i++)
   {
      var testCase = testCaseArray[i];
      debugWriteArray(testCase);

      var date0 = new MyDate(testCase[0]);
      var change = testCase[1];
      var date1 = new MyDate(testCase[2]);

      date0.addYears(change);
      debugAssertEqual(date0.toString(), date1.toString());
   }

   // Test month addition.
   testCaseArray = new Array(
    new Array("2/17/91", 9, "11/17/91"),
    new Array("2/17/91", 10, "12/17/91"),
    new Array("2/17/91", 11, "1/17/92"),
    new Array("2/17/91", 12, "2/17/92"),
    new Array("2/17/91", 24, "2/17/93"),
    new Array("2/11/01", 0, "2/11/01"),
    new Array("2/11/01", -1, "1/11/01"),
    new Array("2/11/01", -2, "12/11/00"),
    new Array("2/11/01", -12, "2/11/00"),
    new Array("2/11/01", -24, "2/11/99")
    );
   for (var i = 0; i < testCaseArray.length; i++)
   {
      var testCase = testCaseArray[i];
      debugWriteArray(testCase);

      var date0 = new MyDate(testCase[0]);
      var change = testCase[1];
      var date1 = new MyDate(testCase[2]);

      date0.addMonths(change);
      debugAssertEqual(date0.toString(), date1.toString());
   }

   // Test day addition.
   testCaseArray = new Array(
    new Array("9/13/01", 0, "9/13/01"),
    new Array("9/13/01", 1, "9/14/01"),
    new Array("9/13/01", 2, "9/15/01"),
    new Array("9/13/01", -1, "9/12/01"),
    new Array("9/13/01", -2, "9/11/01"),
    new Array("9/13/01", 17, "9/30/01"),
    new Array("9/13/01", 18, "10/1/01"),
    new Array("9/13/01", 19, "10/2/01"),
    new Array("9/13/01", -12, "9/1/01"),
    new Array("9/13/01", -13, "8/31/01"),
    new Array("9/13/01", -14, "8/30/01"),
    new Array("1/1/00", (31 + 29 + 31 + 30), "5/1/00"),
    new Array("11/1/99", (31 + 30), "1/1/00"),
    new Array("1/1/00", 366, "1/1/01"),
    new Array("1/1/01", -366, "1/1/00"),
    new Array("1/1/01", 365, "1/1/02"),
    new Array("1/1/02", -365, "1/1/01")
    );
   for (var i = 0; i < testCaseArray.length; i++)
   {
      var testCase = testCaseArray[i];
      debugWriteArray(testCase);

      var date0 = new MyDate(testCase[0]);
      var change = testCase[1];
      var date1 = new MyDate(testCase[2]);

      debugAssertEqual(date1.subtract(date0), change);
      date0.addDays(change);
      debugAssertEqual(date0.toString(), date1.toString());
   }

   // Test day of the week.
   testCaseArray = new Array(
    new Array("1/1/01", 1), // 2001 began on a Monday.
    new Array("1/2/01", 2),
    new Array("1/3/01", 3),
    new Array("1/7/01", 0),
    new Array("1/8/01", 1),
    new Array("12/31/00", 0),
    new Array("12/30/00", 6),
    new Array("10/13/00", 5), // A Friday the 13th (see _Numerical Recipes_).
    new Array("10/19/87", 1), // "Black Monday" stock market crash.
    new Array("10/29/1929", 2) // "Black Tuesday" stock market crash.
    );
   for (var i = 0; i < testCaseArray.length; i++)
   {
      var testCase = testCaseArray[i];
      debugWriteArray(testCase);

      debugAssertEqual((new MyDate(testCase[0])).dayOfTheWeek(), testCase[1]);
   }

   debugWrite("testMyDate succeeded");
}

