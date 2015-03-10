<!-- Begin Hide

   var previousBut;
   var previousText;
   var previousHBut;
   var previousHText;

   function takeAwaySelect ()
   {
      hideSelected(previousBut);
      hideSelected(previousText);
      previousBut = null;
      previousText = null;
   }

   function leftHeadMenu(name)
   {
     page = getPageIndex(name, parent.headMenuPages);

     if (page == -1) return;

     selectedBut="HBut" + page;
     selectedText="textHBut" + page;

      if (selectedBut != previousHBut) {
         showItem(selectedBut, previousHBut);
         showItem(selectedText, previousHText);
    
         if (previousHBut != null && previousHText != null) {
            hideSelected(previousHBut);
            hideSelected(previousHText);
         }
         previousHBut = selectedBut;
         previousHText = selectedText;
      }      
   }

   function selectItem(name)
   {
      page = getPageIndex(name, menuPages);

      if (page == -1) return;

      if (menuPages[page].selBut == 1)
        selectedBut="ButSel" + page;
      else
        selectedBut="But" + page;
      selectedText="textBut" + page;

      if (selectedBut != previousBut) {
         showItem(selectedBut, previousBut);
         showItem(selectedText, previousText);
    
         if (previousBut != null && previousText != null) {
            hideSelected(previousBut);
            hideSelected(previousText);
         }
         previousBut = selectedBut;
         previousText = selectedText;
      }      
   }

   function hideSelected(object)
   {
      if (type == 'NN') {
         if (this.document.layers[object] != null)
            this.document.layers[object].visibility = 'hidden';
      } else if (type == 'MO')          
         document.getElementById(object).style.visibility = "hidden";   
      else if (type == 'IE')
         this.document.all[object].style.visibility = 'hidden';
   } 
 
   
   function showItem(object, previousObj)
   {
      if (object != previousObj) {
         if (type == 'NN') {
            if (this.document.layers[object] != null)
               this.document.layers[object].visibility = 'visible';
          } else if (type == 'MO')          
               document.getElementById(object).style.visibility = "visible";   
          else if (type == 'IE')
            this.document.all[object].style.visibility = 'visible';
      }
   }

   function hideItem(object, previousObj)
   {
      if (object != previousObj) {
         if (type == 'NN') {
            if (this.document.layers[object] != null)
               this.document.layers[object].visibility = 'hidden';
         } else if (type == 'MO')          
            document.getElementById(object).style.visibility = "hidden";   
         else if (type == 'IE')
            this.document.all[object].style.visibility = 'hidden';
      }
   } 

// End Hide -->
