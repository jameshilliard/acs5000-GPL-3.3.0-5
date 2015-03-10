/******************************************************************************
 *
 * Copyright (c) 1999-2005 AppGate Network Security AB. All Rights Reserved.
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
import java.awt.Button;
import java.awt.Dialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.Panel;

public final class AlertDialog {

    public static void show(String title, String message, Frame parent) {

        Dialog dialog = new BorderDialog(parent, title, true);

        MultiLineLabel alertLabel = new MultiLineLabel(message, 2, 2);
        dialog.add(alertLabel, BorderLayout.CENTER);

        Button okAlertBut = new Button("OK");
        okAlertBut.addActionListener(new AWTConvenience.CloseAction(dialog));

        Panel bp = new Panel(new FlowLayout());
        bp.add(okAlertBut);        
        dialog.add(bp, BorderLayout.SOUTH);

        dialog.addWindowListener(new AWTConvenience.CloseAdapter(okAlertBut));

        dialog.setResizable(true);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);
        okAlertBut.requestFocus();
        dialog.setVisible(true);
    }
}

