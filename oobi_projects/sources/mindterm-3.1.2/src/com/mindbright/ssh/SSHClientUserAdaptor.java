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

package com.mindbright.ssh;

import java.io.IOException;
import java.net.Socket;

public class SSHClientUserAdaptor implements SSHClientUser {

    protected String sshHost;
    protected int    sshPort;
    protected SSHInteractor interactor;

    public SSHClientUserAdaptor(String server, int port) {
        this.sshHost = server;
        this.sshPort = port;
        this.interactor = null;
    }

    public SSHClientUserAdaptor(String server) {
        this(server, SSH.DEFAULTPORT);
    }

    public String getSrvHost() {
        return sshHost;
    }

    public int getSrvPort() {
        return sshPort;
    }

    public Socket getProxyConnection() throws IOException {
        return null;
    }

    public String getDisplay() {
        return "";
    }

    public int getMaxPacketSz() {
        return 0;
    }

    public int getAliveInterval() {
        return 0;
    }

    public int getCompressionLevel() {
        return 0;
    }

    public boolean wantX11Forward() {
        return false;
    }

    public boolean wantPTY() {
        return false;
    }

    public SSHInteractor getInteractor() {
        return interactor;
    }

}
