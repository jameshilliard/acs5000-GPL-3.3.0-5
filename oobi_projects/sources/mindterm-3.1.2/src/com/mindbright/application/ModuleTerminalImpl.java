/******************************************************************************
 *
 * Copyright (c) 1999-2006 AppGate Network Security AB. All Rights Reserved.
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

import com.mindbright.ssh2.SSH2Channel;
import com.mindbright.ssh2.SSH2SessionChannel;
import com.mindbright.ssh2.SSH2TerminalAdapterImpl;
import com.mindbright.terminal.TerminalFrameTitle;
import com.mindbright.terminal.TerminalWindow;

public class ModuleTerminalImpl extends ModuleBaseTerminal {

    private SSH2SessionChannel session;

    public void runTerminal(MindTermApp mindterm, TerminalWindow terminal,
                            Frame frame, TerminalFrameTitle frameTitle) {
        SSH2TerminalAdapterImpl termAdapter =
            new SSH2TerminalAdapterImpl(terminal);

        session = mindterm.getConnection().newTerminal(termAdapter);

        if(useChaff()) {
            termAdapter.startChaff();
        }

        if(session.openStatus() != SSH2Channel.STATUS_OPEN) {
            mindterm.alert("Failed to open session channel");
            frame.dispose();
            return;
        }

        boolean wantX11 =
            Boolean.valueOf(mindterm.getProperty("x11-forward")).booleanValue();

        if(wantX11) {
            mindterm.getTransport().getLog().info(mindterm.getAppName(),
                                                  "got X11 forward? " +
                                                  session.requestX11Forward(false, 0));
        }
        mindterm.getTransport().getLog().info(mindterm.getAppName(),
                                              "got pty? " +
                                              session.requestPTY(terminal.terminalType(),
                                                                 terminal.rows(),
                                                                 terminal.cols(),
                                                                 null));
        mindterm.getTransport().getLog().info(mindterm.getAppName(),
                                              "got shell? " +
                                              session.doShell());

        session.waitForExit(0);

        termAdapter.stopChaff();
    }

    protected boolean closeOnDisconnect() {
        return true;
    }

    public String getTitle() {
        return mindterm.getUserName() + "@" +
               mindterm.getHost() +
               (mindterm.getPort() != 22 ?
                (":" + mindterm.getPort()) : "");
    }

    public boolean isAvailable(MindTermApp mindterm) {
        return mindterm.isConnected();
    }

    protected boolean haveMenus() {
        return Boolean.valueOf(mindterm.getProperty("module.terminal.havemenus")).
               booleanValue();
    }

    public void doClose() {
        if(session != null) {
            session.close();
            session = null;
        }
    }

    protected ModuleBaseTerminal newInstance() {
        return new ModuleTerminalImpl();
    }

}

