<!-- Begin Hide

var posTopInc = 20;
var butSize = 20;

if (screen.height < 768) {
  posTopInc = 14;
  butSize = 14;
}

function left_menu(nItem, posLeft, posTop, textItem, xRef, butSel)
{
   document.write('<div style="z-index: 0; position: absolute;',
      ' left:', posLeft, '; top:', posTop, '" id="text', nItem, '">',
      '<a href="javascript:loadNextPage(', "'", xRef, "');",
      '" class="menuText" onmouseover="showItem(',
      "'textBut", nItem, "', previousText); ",
      "showItem('But", nItem, "', previousText);", '" onmouseout="hideItem(',
      "'textBut", nItem, "', previousText); hideItem('But", nItem,
      "', previousBut)", '">', textItem, "</a></div>");

   document.write('<div style="z-index: 15; visibility: hidden;',
      ' position: absolute;',
      ' left:', posLeft, '; top:', posTop, '" id="textBut', nItem, '">',
      '<a href="javascript:loadNextPage(', "'", xRef, "');",
      '" class="menuTextSelected"', ' onmouseover="showItem(',
      "'textBut", nItem, "', previousText); ",
      "showItem('But", nItem, "', previousText);", '" onmouseout="hideItem(',
      "'textBut", nItem, "', previousText); hideItem('But", nItem,
      "', previousBut)", '">', textItem, "</a></div>");

   posLeft -= 3;
   size = 178 - posLeft;
   document.write('<div style="z-index: 10; position: absolute;',
      ' left:', posLeft, '; top:', posTop, '" class="menuButton" id="But',
      nItem, '">', '<a target="main"', 
      ' onmouseover="showItem(',
      "'textBut", nItem, "', previousText); ",
      "showItem('But", nItem, "', previousText);", '" onmouseout="hideItem(',
      "'textBut", nItem, "', previousText); hideItem('But", nItem,
      "', previousBut)", '">',
      '<img src="/Images/menuButton.jpg" alt="" width="', size,
      '" height="', butSize, '" border="0">', "</a></div>");
   if (butSel == 1) {
       posLeft -= 9;
       size = 178 - posLeft;
       document.write('<div style="z-index: 10; position: absolute;',
         ' left:', posLeft, '; top:', posTop, '" class="menuButton" ',
         'id="ButSel', nItem, '">',
         '<img src="/Images/menuSubButton.gif" alt="" width="', size,
         '" height="', butSize, '" border="0">', "</div>");
   }
}

function head_left_menu(nItem, posLeft, posTop, textItem, xRef)
{
   document.write('<div style="z-index: 0; position: absolute;',
      ' left:', posLeft, '; top:', posTop, '" id="Htext', nItem, '">',
      '<a href="javascript:loadNextPage(', "'", xRef, "');",
      '" class="menuText" onmouseover="showItem(',
      "'textHBut", nItem, "', previousHText); ",
      "showItem('HBut", nItem, "', previousHText);", '" onmouseout="hideItem(',
      "'textHBut", nItem, "', previousHText); hideItem('HBut", nItem,
      "', previousHBut)", '">', textItem, "</a></div>");

   document.write('<div style="z-index: 15; visibility: hidden;',
      ' position: absolute;',
      ' left:', posLeft, '; top:', posTop, '" id="textHBut', nItem, '">',
      '<a href="javascript:loadNextPage(', "'", xRef, "');",
      '" class="menuTextSelected"', ' onmouseover="showItem(',
      "'textHBut", nItem, "', previousHText); ",
      "showItem('HBut", nItem, "', previousHText);", '" onmouseout="hideItem(',
      "'textHBut", nItem, "', previousHText); hideItem('HBut", nItem,
      "', previousHBut)", '">', textItem, "</a></div>");

   posLeft -= 3;
   size = 178 - posLeft;
   document.write('<div style="z-index: 10; position: absolute;',
      ' left:', posLeft, '; top:', posTop, '" class="menuButton" id="HBut',
      nItem, '">', '<a target="main"', 
      ' onmouseover="showItem(',
      "'textHBut", nItem, "', previousHText); ",
      "showItem('HBut", nItem, "', previousHText);", '" onmouseout="hideItem(',
      "'textHBut", nItem, "', previousHText); hideItem('HBut", nItem,
      "', previousHBut)", '">',
      '<img src="/Images/menuButton.jpg" alt="" width="', size,
      '" height="', butSize, '" border="0">', "</a></div>");
}

function top_menu(nItem, posLeft, posTop, textItem, xRef, last)
{
    var tLast;

    if (last == 1)
      tLast = '</font></div>';
    else
      tLast = '&nbsp;|</font></div>';

   document.write('<div style="visibility:hidden; position: absolute; left:',
      posLeft, '; top:', posTop, '" id="text', nItem, '"><a href="',
      "javascript:loadNextPage('", xRef, "');", '" class="topMenuText" ',
      'onmouseover="', "showItem('textHigh", nItem, "'); ",
      "hideItem('text", nItem, "');", '" onmouseout="',
      "hideItem('textHigh", nItem, "'); showItem('text",
      nItem, "')", '">', textItem, '</a><font class="topMenuMono">', tLast);


   document.write('<div style="visibility:hidden; position: absolute; left:',
      posLeft, '; top:', posTop, '" id="textHigh', nItem, '"><a href="',
      "javascript:loadNextPage('", xRef, "');", '" class="topMenuTextHigh" ',
      'onmouseover="', "showItem('textHigh", nItem, "'); ",
      "hideItem('text", nItem, "');", '" onmouseout="',
      "hideItem('textHigh", nItem, "'); showItem('text",
      nItem, "')", '">', textItem, '</a><font class="topMenuMono">', tLast);
}
 
function print_table_menu(posLeft, posTop, Pages)
{
   for(var i=0; i < Pages.length; i++) {
     left_menu(i, posLeft, posTop, Pages[i].textItem, Pages[i].xRef, 0);
     posTop += posTopInc;
   }
   return posTop;
}

function print_table_selBut(posLeft, posTop, Pages)
{
   for(var i=0; i < Pages.length; i++) {
     Pages[i].selBut = 1;
     left_menu(i, posLeft, posTop, Pages[i].textItem, Pages[i].xRef, 1);
     posTop += posTopInc;
   }
   return posTop;
}

function print_head_menu(posLeft, posTop, Pages, nickname)
{
   if ((parent.family_model == "ACS")||(parent.family_model == "AVCS")) {
      print_table_menu(posLeft, posTop, Pages);
      return;
   }

   for(var i=0; i < parent.headMenuPages.length; i++) {
     head_left_menu(i, posLeft, posTop, parent.headMenuPages[i].textItem, parent.headMenuPages[i].xRef);
     posTop += posTopInc;
     if (nickname == parent.headMenuPages[i].nickname) {
        posTop = print_table_selBut(posLeft+20, posTop, Pages);
     }
   }
}

function print_top_table_menu(posTop, Pages)
{
   for(var i=0; i < Pages.length-1; i++) {
     top_menu(i, Pages[i].lPos, posTop, Pages[i].textItem, Pages[i].xRef, 0);
   }
   top_menu(i, Pages[i].lPos, posTop, Pages[i].textItem, Pages[i].xRef, 1);
}

// End Hide -->

