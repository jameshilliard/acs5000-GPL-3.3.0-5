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

package com.mindbright.ssh;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Checkbox;
import java.awt.CheckboxGroup;
import java.awt.Choice;
import java.awt.Component;
import java.awt.Dialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Label;
import java.awt.List;
import java.awt.Panel;
import java.awt.TextField;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.BorderDialog;

public final class SSHTunnelDialogAWT {

    private final static String LBL_CURRENT_TUNNELS = "Current tunnels";
    private final static String LBL_LOCAL      = "Local";
    private final static String LBL_REMOTE     = "Remote";
    private final static String LBL_LOCALHOST  = "localhost";
    private final static String LBL_ALL_HOSTS  = "all (0.0.0.0)";
    private final static String LBL_IP         = "ip";

    private final static String LBL_TYPE       = "Type";
    private final static String LBL_PLUGIN     = "Plugin";
    private final static String LBL_SRC_PORT   = "Bind port";
    private final static String LBL_BIND_ADDR  = "Bind address";
    private final static String LBL_DEST_ADDR  = "Dest. address";
    private final static String LBL_DEST_PORT  = "Dest. port";

    private final static String LBL_BTN_ADD    = "Add...";
    private final static String LBL_BTN_DEL    = "Delete";
    private final static String LBL_BTN_DISMISS= "Dimiss";
    private final static String LBL_BTN_OK     = "OK";
    private final static String LBL_BTN_CANCEL = "Cancel";

    private final static String   PLUGIN_NONE  = "none";
    private final static String   PLUGIN_FTP   = "ftp";

    private final static String[] PLUGIN_NAMES = { PLUGIN_NONE, PLUGIN_FTP };
    private final static int[]    PLUGIN_PORTS = {  0, 21 };

    private static SSHPropertyHandler   propsHandler;
    private static Frame                parent;
    private static SSHInteractiveClient client;

    private static List   tunnelList;
    private static Button delButton;

    private static void showAddDialog() {
        final Dialog dialog = new BorderDialog(parent, "Add tunnel", true);

        Panel p = new Panel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        
        gbc.fill   = GridBagConstraints.NONE;
        gbc.insets = new Insets(2, 2, 2, 2);

        CheckboxGroup bg = new CheckboxGroup();
        final Checkbox local = new Checkbox(LBL_LOCAL, bg, true);
        final Checkbox remote = new Checkbox(LBL_REMOTE, bg, false); 

        Panel p1 = new Panel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        p1.add(local);
        p1.add(remote);
        gbc.gridwidth = 1;
        gbc.anchor    = GridBagConstraints.EAST;
        p.add(new Label(LBL_TYPE), gbc);
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.anchor    = GridBagConstraints.WEST;
        p.add(p1, gbc);

        bg = new CheckboxGroup();
        final Checkbox localhost = new Checkbox(LBL_LOCALHOST, bg, true);
        final Checkbox allhosts  = new Checkbox(LBL_ALL_HOSTS, bg, false);
        final Checkbox spechost  = new Checkbox(LBL_IP, bg, false);

        gbc.gridwidth = 1;
        gbc.anchor    = GridBagConstraints.EAST;
        gbc.insets    = new Insets(2, 2, 0, 2);
        p.add(new Label(LBL_BIND_ADDR), gbc);

        gbc.anchor    = GridBagConstraints.WEST;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        p.add(localhost, gbc);

        gbc.gridx     = 1;
        p.add(allhosts, gbc);

        gbc.insets    = new Insets(2, 2, 2, 2);
        p1 = new Panel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        p1.add(spechost);
        final TextField listenip = new TextField("", 16);
        listenip.setEnabled(false);
        p1.add(listenip);
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        p.add(p1, gbc);

        gbc.gridx     = GridBagConstraints.RELATIVE;
        gbc.gridwidth = 1;
        gbc.anchor    = GridBagConstraints.EAST;
        p.add(new Label(LBL_SRC_PORT), gbc);
        final TextField srcport = new TextField("", 5);
        gbc.anchor    = GridBagConstraints.WEST;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        p.add(srcport, gbc);

        gbc.gridwidth = 1;
        gbc.anchor    = GridBagConstraints.EAST;
        p.add(new Label(LBL_DEST_ADDR), gbc);
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.anchor    = GridBagConstraints.WEST;
        final TextField destaddr = new TextField("", 16);
        p.add(destaddr, gbc);

        gbc.gridwidth = 1;
        gbc.anchor    = GridBagConstraints.EAST;
        p.add(new Label(LBL_DEST_PORT), gbc);
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.anchor    = GridBagConstraints.WEST;
        final TextField destport = new TextField("", 5);
        p.add(destport, gbc);

        gbc.gridwidth = 1;
        gbc.anchor    = GridBagConstraints.EAST;
        final Label pluginlbl = new Label(LBL_PLUGIN);
        p.add(pluginlbl, gbc);
        gbc.anchor    = GridBagConstraints.WEST;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        final Choice plugins = AWTConvenience.newChoice(PLUGIN_NAMES);
        p.add(plugins, gbc);

        gbc.fill      = GridBagConstraints.BOTH;
        gbc.weightx   = 1.0;
        gbc.weighty   = 1.0;
        p.add(new Panel(), gbc);

        Button ok = new Button(LBL_BTN_OK);
        ok.setActionCommand("ok");

        Button cancel = new Button(LBL_BTN_CANCEL);
        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));

        dialog.add(p, BorderLayout.CENTER);
        dialog.add(AWTConvenience.newButtonPanel
                   (new Component[] { ok, cancel }), BorderLayout.SOUTH);

        ActionListener al = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                int lp = -1, rp = -1;
                try {
                    lp = Integer.valueOf(srcport.getText()).intValue();
                    rp = Integer.valueOf(destport.getText()).intValue();
                    if (lp < 1 || lp > 65535) {
                            lp = -1;
                            throw new NumberFormatException();
                    }
                    if (rp < 1 || rp > 65535) {
                        rp = -1;
                        throw new NumberFormatException();
                    }
                } catch (NumberFormatException ee) {
                    if (lp == -1) {
                        srcport.setText("");
                        srcport.requestFocus();
                    } else {
                        destport.setText("");
                        destport.requestFocus();
                    }
                    Toolkit.getDefaultToolkit().beep();
                    return;
                }
                
                String plug = plugins.getSelectedItem();
                if (plug.equals(PLUGIN_NONE))
                    plug = "general";
                String daddr = destaddr.getText().trim();
                if (daddr.equals("")) {
                    destport.requestFocus();
                    Toolkit.getDefaultToolkit().beep();
                    return;
                }
                
                String key;
                
                try {
                    if (local.getState()) {
                        key = "local" + client.localForwards.size();
                    } else {
                        key = "remote" + client.remoteForwards.size();
                    }
                    String baddr = "";
                    if (localhost.getState()) {
                        baddr = "127.0.0.1";
                    } else if (allhosts.getState()) {
                        baddr = "0.0.0.0";
                    } else {
                        baddr = listenip.getText().trim();
                        if (baddr.equals("")) {
                            listenip.requestFocus();
                            Toolkit.getDefaultToolkit().beep();
                            return;
                        }
                    }
                    
                    propsHandler.setProperty(key, "/" + plug + "/" +
                                             baddr + ":" + lp + ":" +
                                             daddr + ":" +  rp);
                } catch (Throwable ee) {
                    SSHMiscDialogs.alert("Tunnel Notice",
                                             "Could not open tunnel: " +
                                         ee.getMessage(), parent);
                    return;
                }
                dialog.dispose();
            }
        };
        ok.addActionListener(al);

        ItemListener il = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                if (local.getState()) {
                    pluginlbl.setEnabled(true);
                    plugins.setEnabled(true);
                } else {
                    pluginlbl.setEnabled(false);
                    plugins.setEnabled(false);
                    plugins.select(0);
                } 

                if (localhost.getState()) {
                    listenip.setEnabled(false);
                    listenip.setText("");
                } else if (allhosts.getState()) {
                    listenip.setEnabled(false);
                    listenip.setText("");
                } else {
                    listenip.setEnabled(true);
                }
            }
        };

        local.addItemListener(il);
        remote.addItemListener(il);        
        localhost.addItemListener(il);
        allhosts.addItemListener(il);
        spechost.addItemListener(il);        

        srcport.requestFocus();

        dialog.setResizable(false);
        dialog.pack();
        AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancel));
        dialog.setVisible(true);
    }
    
    private static class Action implements 
          ActionListener, ItemListener 
    {
        public void actionPerformed(ActionEvent e) {
            String cmd = e.getActionCommand();
            if ("add".equals(cmd)) {
                showAddDialog();
            } else if ("del".equals(cmd)) {
                int i = tunnelList.getSelectedIndex();
                if (i < 0) return;
                int len = client.localForwards.size();
                if (i < len) {
                    propsHandler.removeLocalTunnelAt(i, true);
                } else {
                    propsHandler.removeRemoteTunnelAt(i-len);
                }
            }
            updateTunnelList();
        }

        public void itemStateChanged(ItemEvent e) {
            int i = tunnelList.getSelectedIndex();
            delButton.setEnabled(i >= 0);
        }
    }
    
    public static void show(String title, SSHInteractiveClient cli,
                            SSHPropertyHandler props, Frame p) {
        propsHandler = props;
        parent       = p;
        client       = cli;

        Dialog dialog = new BorderDialog(parent, title, true);
        
        Action al = new Action();

        tunnelList = new List(8);
        dialog.add(tunnelList, BorderLayout.CENTER);
        tunnelList.addItemListener(al);

        Button add = new Button(LBL_BTN_ADD);
        add.setActionCommand("add");
        add.addActionListener(al);

        delButton = new Button(LBL_BTN_DEL);
        delButton.setActionCommand("del");
        delButton.addActionListener(al);
        delButton.setEnabled(false);

        Button ok  = new Button(LBL_BTN_DISMISS);
        ok.addActionListener(new AWTConvenience.CloseAction(dialog));

        dialog.add(AWTConvenience.newButtonPanel(new Component[] {
            new Label(""), new Label(""), add, delButton, ok
        }), BorderLayout.SOUTH);


        updateTunnelList();

        dialog.setResizable(true);
        dialog.pack();
        AWTConvenience.placeDialog(dialog);
        dialog.setVisible(true);
    }
    
    private static void updateTunnelList() {
        int llen = client.localForwards.size();
        int rlen = client.remoteForwards.size();
        String[] s = new String[llen+rlen];

        for(int i = 0; i < llen; i++) {
            SSHClient.LocalForward fwd = (SSHClient.LocalForward) client.localForwards.elementAt(i);
            String plugStr = (fwd.plugin.equals("general")) ? 
                "" : (" (plugin: " + fwd.plugin + ")");
            s[i] = "L: " + fwd.localHost + ":" + fwd.localPort + 
                " --> " + fwd.remoteHost + ":" + fwd.remotePort + plugStr;
        }

        for(int i = 0; i < rlen; i++) {
            SSHClient.RemoteForward fwd = (SSHClient.RemoteForward) client.remoteForwards.elementAt(i);
            s[i+llen] = "R: " + fwd.localHost + ":" + fwd.localPort + 
                " <-- " + fwd.remoteHost + ":" + fwd.remotePort;
        }

        tunnelList.removeAll();
        for (int i=0; i<s.length; i++)
            tunnelList.addItem(s[i]);
    }
}
