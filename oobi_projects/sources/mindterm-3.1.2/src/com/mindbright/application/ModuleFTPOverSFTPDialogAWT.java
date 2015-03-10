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

package com.mindbright.application;

import java.awt.Button;
import java.awt.Checkbox;
import java.awt.CheckboxGroup;
import java.awt.Dialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextField;
import java.awt.event.ActionListener;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.AWTGridBagContainer;

public class ModuleFTPOverSFTPDialogAWT extends Dialog 
    implements ModuleFTPOverSFTPDialogControl {

    private TextField ftpHost;
    private TextField ftpPort;
    private Checkbox  unixCB, windowsCB;
    private Label     lblStatus;
    private Button    startBut, closeBut, browseBut;

    public ModuleFTPOverSFTPDialogAWT(Frame parent, String title, boolean modal) {
        super(parent, title, modal);
    }
    
    public void initDialog(ActionListener al, boolean showBrowse) {
        AWTGridBagContainer grid = new AWTGridBagContainer(this);
        
        Label lbl = new Label(LBL_LISTEN_ADDR);
        grid.add(lbl, 0, 1);
        
        lbl = new Label(LBL_LISTEN_PORT);
        grid.add(lbl, 0, 1);
        
        ftpHost = new TextField(DEFAULT_HOST, 20);
        grid.add(ftpHost, 1, 1);
        
        ftpPort = new TextField(DEFAULT_PORT, 5);
        grid.add(ftpPort, 1, 1);

        CheckboxGroup bg = new CheckboxGroup();
        Panel p1 = new Panel(new FlowLayout(FlowLayout.LEFT));
        p1.add(new Label(LBL_REMOTE_SYSTEM));
        p1.add(unixCB = new Checkbox(LBL_REMOTE_IS_UNIX, true, bg));
        p1.add(windowsCB = new Checkbox(LBL_REMOTE_IS_WINDOWS, false, bg));
        grid.add(p1, 2, GridBagConstraints.REMAINDER);
        
        grid.getConstraints().anchor = GridBagConstraints.CENTER;

        lblStatus = new Label(LBL_BRIDGE_DISABLED, Label.CENTER);
        grid.add(lblStatus, 3, 2);
        
        Panel p = new Panel(new FlowLayout());
        
        p.add(startBut = new Button(LBL_ENABLE));
        startBut.addActionListener(al);
        
        if (showBrowse) {
            p.add(browseBut = new Button(LBL_BROWSER));
            browseBut.addActionListener(al);
        }

        p.add(closeBut = new Button(LBL_DISMISS));
        closeBut.addActionListener(new AWTConvenience.CloseAction(this));

        grid.add(p, 4, GridBagConstraints.REMAINDER);
        
        addWindowListener(new AWTConvenience.CloseAdapter(closeBut));
        AWTConvenience.setBackgroundOfChildren(this);
        AWTConvenience.setKeyListenerOfChildren(
            this, new AWTConvenience.OKCancelAdapter(startBut, closeBut),null);
        
        setResizable(true);
        pack();

        AWTConvenience.placeDialog(this);        
    }
    
    public void showDialog() {
        setVisible(true);
    }
    
    public void disposeDialog() {
        dispose();
    }
    
    public void setHost(String host) {
        ftpHost.setText(host);
    }
    
    public void setPort(String port) {
        ftpPort.setText(port);
    }

    public void setRemoteSystemIsUnix(boolean yes) {
        if (yes) {
            unixCB.setState(true);
        } else {
            windowsCB.setState(true);
        }
    }

    public void setStatus(String status) {
        lblStatus.setText(status);
    }

    public void setMode(boolean enable) {
        ftpPort.setEnabled(enable);
        ftpHost.setEnabled(enable);
        if (browseBut != null)
            browseBut.setEnabled(!enable);
        startBut.setLabel(enable ? LBL_ENABLE : LBL_DISABLE);
     }

    public String getHost() {
        return ftpHost.getText();
    }
    
    public String getPort() {
        return ftpPort.getText();
    }

    public boolean isRemoteSystemUnix() {
        return unixCB.getState();
    }
}
