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

import java.awt.BorderLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.SwingConstants;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.SwingConvenience;

public class ModuleTelnetProxyDialogSwing extends JDialog implements
   ModuleTelnetProxyDialogControl
{
    private JTextField   proxyHost;
    private JTextField   proxyPort;
    private JLabel       lblStatus;
    private JButton      startBut, closeBut;

    public ModuleTelnetProxyDialogSwing(Frame parent, String title,
                                        boolean modal) {
        super(parent, title, modal);
    }
    
    public void initDialog(ActionListener al) {
        JPanel p = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        
        gbc.fill   = GridBagConstraints.HORIZONTAL;
        gbc.insets = new Insets(2, 2, 2, 2);
        gbc.gridwidth = 1;
        
        JLabel lbl = new JLabel(LBL_LISTEN_ADDR);
        p.add(lbl, gbc);
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        lbl = new JLabel(LBL_LISTEN_PORT);
        p.add(lbl, gbc);
        
        gbc.gridwidth = 1;
        proxyHost = new JTextField(DEFAULT_HOST, 20);
        p.add(proxyHost, gbc);
        
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        proxyPort = new JTextField(DEFAULT_PORT, 5);
        p.add(proxyPort, gbc);
        
        gbc.anchor = GridBagConstraints.CENTER;
        lblStatus = new JLabel(LBL_PROXY_DISABLED, SwingConstants.CENTER);
        p.add(lblStatus, gbc);
        
        getContentPane().add(p, BorderLayout.CENTER);

        startBut = new JButton(LBL_ENABLE);
        startBut.addActionListener(al);

        closeBut = new JButton(LBL_DISMISS);
        closeBut.addActionListener(new AWTConvenience.CloseAction(this));
        
        getContentPane().add(SwingConvenience.newButtonPanel(
                                 new JComponent[] {startBut, closeBut}),
                             BorderLayout.SOUTH);
        
        setResizable(true);
        pack();

        AWTConvenience.placeDialog(this);
        addWindowListener(SwingConvenience.getWindowDisposer());
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
