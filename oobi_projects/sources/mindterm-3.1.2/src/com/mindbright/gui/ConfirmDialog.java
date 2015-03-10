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
import java.awt.Component;
import java.awt.Frame;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextArea;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public final class ConfirmDialog extends BorderDialog implements ActionListener {

    private boolean confirmRet;
    private String  yesLbl;

    private ConfirmDialog(String title, Frame parent, String yesLbl) {
        super(parent, title, true);
        this.yesLbl = yesLbl;
    }

    public static boolean show(String title, String message,
                               int rows, int cols,
                               String yesLbl, String noLbl,
                               boolean defAnswer, Frame parent,
                               boolean xscroll, boolean yscroll) {

        ConfirmDialog dialog = new ConfirmDialog(title, parent, yesLbl);

        Component confirmText;

        if (rows == 0 || cols == 0) {
            confirmText = new Label(message);
        } else {
            int scrollbars;
            if (xscroll && yscroll) {
                scrollbars = TextArea.SCROLLBARS_BOTH;
            } else if (xscroll) {
                scrollbars = TextArea.SCROLLBARS_HORIZONTAL_ONLY;
            } else if (yscroll) {
                scrollbars = TextArea.SCROLLBARS_VERTICAL_ONLY;
            } else {
                scrollbars = TextArea.SCROLLBARS_NONE;
            }
            TextArea ta = new TextArea(message, rows, cols, scrollbars);
            ta.setEditable(false);
            confirmText = ta;
        }
        dialog.add(confirmText, BorderLayout.CENTER);

        Button yesBut = new Button(yesLbl);
        Button noBut = new Button(noLbl);
        yesBut.addActionListener(dialog);
        noBut.addActionListener(dialog);

        Panel bp = AWTConvenience.newButtonPanel
            (new Button[] { yesBut, noBut });
        dialog.add(bp, BorderLayout.SOUTH);

        dialog.addWindowListener(new AWTConvenience.CloseAdapter(noBut));

        dialog.setResizable(true);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);

        if(defAnswer)
            yesBut.requestFocus();
        else
            noBut.requestFocus();

        dialog.setVisible(true);

        return dialog.confirmRet;
    }

    public void actionPerformed(ActionEvent e) {
        confirmRet = e.getActionCommand().equals(yesLbl);
        this.setVisible(false);
    }
}
