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
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

public abstract class SwingConvenience {

    public static JPanel newButtonPanel(JComponent[] b) {
        JPanel p1 = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        JPanel p2 = new JPanel(new GridLayout(1, 0, 5, 0));
        p1.add(p2);
        for (int i=0; i<b.length; i++)
            p2.add(b[i]);
        return p1;
    }

    public static JDialog newBorderJDialog(Frame parent, String title,
                                           boolean modal) {
        JDialog d = new JDialog(parent, title, modal);
        ((JComponent)d.getContentPane()).setLayout(new BorderLayout(10, 10));
        ((JComponent)d.getContentPane()).setBorder(
            new EmptyBorder(5, 5, 5, 5));
        return d;
    }

    public static WindowAdapter disposer = null;
    public static WindowAdapter getWindowDisposer() {
        if (disposer == null) {
            disposer = new WindowAdapter() {
                public void windowClosing(WindowEvent e) {
                    e.getWindow().dispose();
                }
            };
        }
        return disposer;
    }

    public final static JFrame newJFrameWithMenuBar() {
        JFrame frame = new JFrame();
        frame.setJMenuBar(new JMenuBar());
        frame.validate();
        return frame;
    }
}
