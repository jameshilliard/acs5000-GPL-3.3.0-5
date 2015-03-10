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

package com.mindbright.application;

import java.awt.Button;
import java.awt.Checkbox;
import java.awt.CheckboxGroup;
import java.awt.Dialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.IOException;

public class ModulePortFwd implements MindTermModule, ActionListener {

    private MindTermApp mindterm;

    public void init(MindTermApp mindterm) {
        this.mindterm = mindterm;
    }

    Dialog dialog;
    TextField textFwd;
    Checkbox cbRem, cbLoc;
    CheckboxGroup cbg;

    public void activate(MindTermApp mindterm) {
        Frame parent = mindterm.getParentFrame();
        dialog = new Dialog(parent, mindterm.getAppName() + "Port Forward Setup", false);

        GridBagLayout       grid  = new GridBagLayout();
        GridBagConstraints  gridc = new GridBagConstraints();
        Label               lbl;
        Button              b;
        
        dialog.setLayout(grid);
        
        gridc.fill      = GridBagConstraints.HORIZONTAL;
        gridc.anchor    = GridBagConstraints.WEST;
        gridc.gridy     = 0;
        gridc.gridwidth = 1;
        gridc.insets    = new Insets(4, 4, 4, 4);
        
        lbl = new Label("[<src-host>:]<src-port>:<dest-host>:<dest-port>");
        dialog.add(lbl, gridc);
        
        textFwd = new TextField("", 20);
        gridc.gridy     = 1;
        dialog.add(textFwd, gridc);
        
        cbg = new CheckboxGroup();
        
        Panel p = new Panel(new FlowLayout());
        p.add(cbLoc = new Checkbox("Local", cbg, true));
        p.add(cbRem = new Checkbox("Remote", cbg, false));
        
        gridc.anchor = GridBagConstraints.CENTER;
        gridc.gridy     = 2;
        gridc.gridwidth  = 2;
        gridc.fill = GridBagConstraints.NONE;
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        dialog.add(p, gridc);
        
        p = new Panel(new FlowLayout());
        
        p.add(b = new Button("Ok"));
        b.addActionListener(this);
        p.add(b = new Button("Cancel"));
        b.addActionListener(this);
        
        gridc.gridy     = 3;
        dialog.add(p, gridc);
        dialog.setResizable(true);
        dialog.pack();

        dialog.setVisible(true);
    }

    public boolean isAvailable(MindTermApp mindterm) {
        return mindterm.isConnected();
    }

    public void connected(MindTermApp mindterm) {
        int i;
        try {
            for(i = 0; i < 32; i++) {
                String spec = mindterm.getProperty("local" + i);
                if(spec == null)
                    break;
                newLocalForward(mindterm, spec);
            }
            for(i = 0; i < 32; i++) {
                String spec = mindterm.getProperty("remote" + i);
                if(spec == null)
                    break;
                newRemoteForward(mindterm, spec);
            }
        } catch (Exception e) {
            mindterm.alert("Error in forward: " + e.getMessage());
        }
    }

    public void disconnected(MindTermApp mindterm) {}

    public String description(MindTermApp mindterm) {
        return null;
    }

    public void newLocalForward(MindTermApp mindterm, String fwdSpec)
    throws IOException, IllegalArgumentException {
        Object[] components = parseForwardSpec(fwdSpec);

        // !!! TODO handle "plugins", esp. FTP

        mindterm.getConnection().newLocalForward((String)components[1],
                ((Integer)components[2]).intValue(),
                (String)components[3],
                ((Integer)components[4]).intValue());
    }

    public void newRemoteForward(MindTermApp mindterm, String fwdSpec)
    throws IllegalArgumentException {
        Object[] components = parseForwardSpec(fwdSpec);
        mindterm.getConnection().newRemoteForward((String)components[1],
                ((Integer)components[2]).intValue(),
                (String)components[3],
                ((Integer)components[4]).intValue());
    }

    public static Object[] parseForwardSpec(String spec)
    throws IllegalArgumentException {
        int    d1, d2, d3;
        String tmp;
        Object[] components = new Object[5];

        if(spec.startsWith("/")) {
            int i = spec.indexOf('/', 1);
            if(i == 0) {
                throw new IllegalArgumentException("Invalid port forward spec. "
                                                   + spec);
            }
            components[0] = spec.substring(1, i);
            spec = spec.substring(i + 1);
        } else {
            components[0] = "general";
        }

        d1 = spec.indexOf(':');
        d2 = spec.lastIndexOf(':');
        if(d1 == d2)
            throw new IllegalArgumentException("Invalid port forward spec. " +
                                               spec);

        d3 = spec.indexOf(':', d1 + 1);

        if(d3 != d2) {
            components[1] = spec.substring(0, d1);
            components[2] = Integer.valueOf(spec.substring(d1 + 1, d3));
            components[3] = spec.substring(d3 + 1, d2);
        } else {
            components[1] = "127.0.0.1";
            components[2] = Integer.valueOf(spec.substring(0, d1));
            components[3] = spec.substring(d1 + 1, d2);
        }

        tmp = spec.substring(d2 + 1);
        components[4] = Integer.valueOf(tmp);

        return components;
    }

    public void actionPerformed(ActionEvent e) {
        if(e.getActionCommand().equals("Ok")) {
            try {
                String fwdSpec = textFwd.getText();
                if(cbLoc.getState()) {
                    newLocalForward(mindterm, fwdSpec);
                } else {
                    newRemoteForward(mindterm, fwdSpec);
                }
            } catch(Exception ee) {
                mindterm.alert("Error in forward: " + ee.getMessage());
            }
        }
        dialog.dispose();
    }
}
