// Copyright © 1996-2002 by Art & Logic, Inc. All rights reserved.


// ****************************************************************************
// Art & Logic Advanced JavaScript Control Suite (AJCS)
// Table Sort Functions - tablsort.js
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
// Name:        Table sorting functions
// Description: Functions used for sorting table rows, similarly to
//              the sorting seen in popular file managers like Windows
//              Explorer.  Currently this code only supports ascending
//              sorting, but it can easily be upgraded to provide
//              ascending and descending sorting.  The standard HTML
//              form buttons should be replaced with small up/down
//              arrows at the top of each table column.
// Usage:       Call from HTML <body>:
//              <body>
//              <script language="JavaScript">
//              var buttons = new Array("Column 0","Column 1","Column 2","Column 3","Column 4")
//              // Put the [markups] into the array.
//              var data = new Array
//              (
//                new Array("James","Red","one","1","cat"),
//                new Array("Angel","Blue","two","2","dog"),
//                new Array("Tom","Green","three","3","rat"),
//                new Array("Brett","Orange","four","4","chicken"),
//                new Array("Paul","Grayish-black","five","5","slug")
//              )
//              var table = new SortableTable("myTable",5,5,1,"100%","center")
//              table.setData(data)
//              table.setButtons(buttons)
//              // Use table.setNumeric(n) to sort column n in numerical order.
//              table.display()
//              </script>
//              </body>
// ****************************************************************************


function SortableTable(id,rows,cols,border,width,align)
{
   this.id = id
   this.rows = rows
   this.cols = cols
   this.border = border
   this.width = width
   this.align = align
   this.buttons = new Array(cols)
   this.numeric = new Array(cols)
   for(var i=0;i<cols;++i)
      this.buttons[i]=" "
   for(var i=0;i<cols;++i)
      this.numeric[i]=false
   this.data = new Array(rows)
   for(var i=0;i<rows;++i)
   {
      this.data[i] = new Array(cols)
      for(var j=0;j<cols;++j)
         this.data[i][j] = " "
   }
   this.display = SortableTable_display
   this.setButtons = SortableTable_setButtons
   this.setData = SortableTable_setData
   this.setNumeric = SortableTable_setNumeric
   this.sort = SortableTable_sort
}

function SortableTable_setData(data)
{
   if(data == null) return
   if(data.length > 0)
   {
      for(var i=0;i<this.rows;++i)
      {
         if(i>=data.length) break
         if(data[i] != null)
         {
            var n = data[i].length
            if(n > this.cols) n = this.cols
            for(var j=0;j<n;++j)
            if(data[i][j]!=null) this.data[i][j] = data[i][j]
         }
      }
   }
}

function SortableTable_setButtons(buttons)
{
   if(buttons == null) return
   var n = buttons.length
   if(n > this.cols) n = this.cols
   for(var i=0;i<n;++i)
      if(buttons[i]!=null) this.buttons[i] = buttons[i]
}

function SortableTable_display()
{
   // Check for tableSortBy cookie
   if(document.cookie.length > 0)
   {
      var search = this.id + "="
      var offset = document.cookie.indexOf(this.id+"=")
      if(offset != -1)
      {
         offset += search.length
         var end = document.cookie.indexOf(";", offset)
         if(end == -1) end = document.cookie.length
         this.sort(document.cookie.substring(offset, end))
      }
   }
   document.writeln('<form name="tableForm">')
   document.write('<table border="'+this.border+'" width="')
   document.writeln(this.width+'" align="'+this.align+'">')
   document.writeln('<tr>')
   // Display buttons
   for(var i=0;i<this.cols;++i)
   {
      document.write('<td>')
      document.write('<input type="button" value="')
      document.write(this.buttons[i])
      document.write('" onclick="SortableTable_handleColumnButton(\'')
      document.write(this.id+'\','+i+')">')
      document.writeln('</td>')
   }
   document.writeln('</tr>')
   // Display sorted/unsorted table
   for(var i=0;i<this.rows;++i)
   {
      document.writeln('<tr>')
      for(var j=0;j<this.cols;++j)
      {
         document.write('<td>')
         document.write(this.data[i][j])
         document.writeln('</td>')
      }
      document.writeln('</tr>')
   }
   document.writeln('</table>')
   document.writeln('</form>')
}

function SortableTable_setNumeric(n)
{
   this.numeric[n] = true
}

function SortableTable_sort(n)
{
// Sort rows
   var changes=true
   for(;changes;)
   {
      changes = false
      for(var i=0;i<this.rows-1;++i)
      {
         if(this.numeric[n])
         {
            var v1 = parseFloat(this.data[i][n])
            if(isNaN(v1)) v1 = 0;
            var v2 = parseFloat(this.data[i+1][n])
            if(isNaN(v2)) v2 = 0;
            if(v1 > v2)
            {
               changes = true
               var temp = this.data[i]
               this.data[i] = this.data[i+1]
               this.data[i+1] = temp
            }
         }
            else
         {
            if(this.data[i][n] > this.data[i+1][n])
            {
               changes = true
               var temp = this.data[i]
               this.data[i] = this.data[i+1]
               this.data[i+1] = temp
            }
         }
      }
   }
}

function SortableTable_handleColumnButton(id,n)
{
   // Set cookie and reload
   document.cookie = id + "="+n
   window.location.reload()
}
