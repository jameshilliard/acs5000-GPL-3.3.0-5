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

import java.awt.Component;
import java.awt.Container;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

public final class AWTGridBagContainer {

    private Container          container;
    private GridBagLayout      grid;
    private GridBagConstraints gridc;

    public AWTGridBagContainer(Container container) {
        grid  = new GridBagLayout();
        gridc = new GridBagConstraints();
        this.container = container;
        container.setLayout(grid);

        gridc.fill   = GridBagConstraints.HORIZONTAL;
        gridc.anchor = GridBagConstraints.WEST;
        gridc.insets = new Insets(4, 4, 0, 4);
    }

    public GridBagConstraints getConstraints() {
        return gridc;
    }

    public void add
        (Component comp, int gridx, int gridy, int gridwidth) {
        gridc.gridx = gridx;
        add(comp, gridy, gridwidth);
    }

    public void add
        (Component comp, int gridy, int gridwidth) {
        gridc.gridy     = gridy;
        gridc.gridwidth = gridwidth;
        container.add(comp, gridc);
    }
}

