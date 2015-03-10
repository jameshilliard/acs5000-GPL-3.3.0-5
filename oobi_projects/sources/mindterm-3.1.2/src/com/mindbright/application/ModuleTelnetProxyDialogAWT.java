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

public class ModuleTelnetProxyDialogAWT extends Dialog implements
   ModuleTelnetProxyDialogControl
{
    private TextField   proxyHost;
    private TextField   proxyPort;
    private Label       lblStatus;
    private Button      startBut, closeBut;

    public ModuleTelnetProxyDialogAWT(Frame parent,String title,boolean modal){
        super(parent, title, modal);
    }
    
    public void initDialog(ActionListener al) {
        AWTGridBagContainer grid = new AWTGridBagContainer(this);
        
        Label lbl = new Label(LBL_LISTEN_ADDR);
        grid.add(lbl, 0, 1);
        
        lbl = new Label(LBL_LISTEN_PORT);
        grid.add(lbl, 0, 1);
        
        proxyHost = new TextField(DEFAULT_HOST, 20);
        grid.add(proxyHost, 1, 1);
        
        proxyPort = new TextField(DEFAULT_PORT, 5);
        grid.add(proxyPort, 1, 1);
        
        grid.getConstraints().anchor = GridBagConstraints.CENTER;
        
        lblStatus = new Label(LBL_PROXY_DISABLED, Label.CENTER);
        grid.add(lblStatus, 2, 2);
        
        Panel p = new Panel(new FlowLayout());
        
        p.add(startBut = new Button(LBL_ENABLE));
        startBut.addActionListener(al);

        p.add(closeBut = new Button(LBL_DISMISS));
        closeBut.addActionListener(
            new AWTConvenience.CloseAction(this));
        
        grid.add(p, 3, GridBagConstraints.REMAINDER);
        
        addWindowListener(new AWTConvenience.CloseAdapter(closeBut));
        AWTConvenience.setBackgroundOfChildren(this);
        AWTConvenience.setKeyListenerOfChildren
            (this, new AWTConvenience.OKCancelAdapter(startBut, closeBut), null);

        setResizable(true);
        pack();

        AWTConvenience.placeDialog(this);
    }

    public void showDialog() {
        setVisible(true);
    }

    public void setMode(boolean enable) {
        proxyPort.setEnabled(enable);
        proxyHost.setEnabled(enable);
        startBut.setLabel(enable ? LBL_ENABLE : LBL_DISABLE);
    }

    public void disposeDialog() {
        dispose();
    }
    
    public void setHost(String host) {
        proxyHost.setText(host);
    }
    
    public void setPort(String port) {
        proxyPort.setText(port);
    }

    public void setStatus(String status) {
        lblStatus.setText(status);
    }

    public String getHost() {
        return proxyHost.getText();
    }

    public String getPort() {
        return proxyPort.getText();
    }
}
