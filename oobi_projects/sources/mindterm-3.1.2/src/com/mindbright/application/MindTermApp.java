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

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.Frame;
import java.util.Properties;

import com.mindbright.ssh2.SSH2Connection;
import com.mindbright.ssh2.SSH2Interactor;
import com.mindbright.ssh2.SSH2Transport;
import com.mindbright.sshcommon.SSHConsoleRemote;
import com.mindbright.util.RandomSeed;

public interface MindTermApp {
    public String     getHost();
    public int        getPort();
    public Properties getProperties();
    public String     getProperty(String name);
    public void       setProperty(String name, String value);
    public String     getUserName();

    public Frame getParentFrame();
    public String getAppName();
    public RandomSeed getRandomSeed();

    public SSH2Interactor getInteractor();
    public void alert(String msg);

    public boolean isConnected();

    public boolean isApplet();
    public AppletContext getAppletContext();
    public Applet getApplet();

    public SSH2Transport    getTransport();
    public SSH2Connection   getConnection();
    public SSHConsoleRemote getConsoleRemote();
}
