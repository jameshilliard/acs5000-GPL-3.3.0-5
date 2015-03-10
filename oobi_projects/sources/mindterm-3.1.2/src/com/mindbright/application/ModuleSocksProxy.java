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




import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.ssh2.SSH2SocksListener;

public class ModuleSocksProxy implements MindTermModule, ActionListener {

    MindTermApp mindterm;

    SSH2SocksListener listener;
    
    ModuleSocksProxyDialogControl dialog;

    public void init(MindTermApp mindterm) {
        this.mindterm = mindterm;
    }

    public void activate(MindTermApp mindterm) {
        try {
            Frame f = mindterm.getParentFrame();
            Class c = 
                Class.forName(AWTConvenience.isSwingJFrame(f) ? 
                              "com.mindbright.application.ModuleSocksProxyDialogSwing" :
                              "com.mindbright.application.ModuleSocksProxyDialogAWT");
            java.lang.reflect.Constructor cons =
                c.getConstructor(new Class[] { Frame.class, String.class, boolean.class});
            dialog = (ModuleSocksProxyDialogControl)cons.newInstance
                (new Object[] {f, mindterm.getAppName() + " - SOCKS Proxy",
                               new Boolean(false) });
        } catch (Throwable t) {
            t.printStackTrace();
        }

        dialog.initDialog(this);

        String host = mindterm.getProperty("socksproxy-host");
        String port = mindterm.getProperty("socksproxy-port");
        if (host != null && !host.equals("")) 
            dialog.setHost(host);
        if (port != null && !port.equals(""))
            dialog.setPort(port);

        updateSocksDialog(false);

        dialog.showDialog();
    }

    public boolean isAvailable(MindTermApp mindterm) {
        return (mindterm.isConnected() && (mindterm.getConnection() != null));
    }

    public void connected(MindTermApp mindterm) {
        String socksHost = mindterm.getProperty("socksproxy-host");
        String socksPort = mindterm.getProperty("socksproxy-port");
        if (socksHost != null && socksHost.trim().length() > 0) {
            try {
                startSocksListener(socksHost, socksPort);
                mindterm.alert("Starting SOCKS proxy on " +
                               socksHost + ":" + socksPort);
            } catch (Exception e) {
                mindterm.alert("Error starting SOCKS proxy on " +
                               socksHost + ":" + socksPort + " - " +
                               e.getMessage());
            }
        }
    }

    public void disconnected(MindTermApp mindterm) {
        stopSocksLoop();
    }

    public String description(MindTermApp mindterm) {
        return null;
    }

    private void updateSocksDialog(boolean preserveMsg) {
        if (listener == null) {
            if (!preserveMsg)
                dialog.setStatus("SOCKS proxy disabled");
            dialog.setMode(true);
        } else {
            if (!preserveMsg)
                dialog.setStatus("SOCKS proxy enabled: " +
                                 dialog.getHost() + ":" +
                                 dialog.getPort());
            dialog.setMode(false);
        }
    }

    public void startSocksListener(String host, String portStr) throws Exception {
        listener = new SSH2SocksListener(host, Integer.parseInt(portStr),
                                         mindterm.getConnection());
    }

    public void stopSocksLoop() {
        if (listener != null) {
            listener.stop();
            listener = null;
        }
    }

    public void actionPerformed(ActionEvent e) {
        if (listener != null) {
            stopSocksLoop();
            updateSocksDialog(false);
            mindterm.setProperty("socksproxy-host", "");
            mindterm.setProperty("socksproxy-port", "");
        } else {
            boolean err = false;
            dialog.setStatus("Starting...");
            try {
                String host = dialog.getHost();
                String port = dialog.getPort();
                startSocksListener(host, port);
                mindterm.setProperty("socksproxy-host", host);
                mindterm.setProperty("socksproxy-port", port);
            } catch (Exception ex) {
                err = true;
                dialog.setStatus("Error: " + ex.getMessage());
            }
            updateSocksDialog(err);
        }
    }
}
