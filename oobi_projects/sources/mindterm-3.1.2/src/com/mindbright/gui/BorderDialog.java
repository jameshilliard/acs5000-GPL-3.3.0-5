/******************************************************************************
 *
 * Copyright (c) 2005 AppGate Network Security AB. All Rights Reserved.
 * 
 * This file contains Original Code and/or Modifications of Original Code as
 * defined in and that are subject to the MindTerm Public Source License,
 * Version 2.0, (the 'License'). You may not use this file except in compliance
 * with the License.
 * 
 * You should have received a copy of the MindTerm Public Source License
 * along with this software; see the file LICENSE.  If not, write to
 * AppGate Network Security AB, Otterhallegatan 2, SE-41118 Goteborg, SWEDEN
 *
 *****************************************************************************/

package com.mindbright.gui;

import java.awt.BorderLayout;
import java.awt.Dialog;
import java.awt.Frame;
import java.awt.Insets;

/* Dialog with nice insets and default BorderLayout with some spacing */

public class BorderDialog extends Dialog {
    private Insets myinsets = null;

    public BorderDialog(Frame owner, String title, boolean modal) {
        super(owner, title, modal);
        setLayout(new BorderLayout(10, 10));
    }

    public Insets getInsets() {
        if (myinsets != null) 
            return myinsets;

        Insets i = super.getInsets();
        i.top    += 5;
        i.left   += 5;
        i.bottom += 5;
        i.right  += 5;
        return i;
    }

    public void setInsets(int top, int left, int bottom, int right) {
        myinsets = new Insets(top, left, bottom, right);
    }
}
