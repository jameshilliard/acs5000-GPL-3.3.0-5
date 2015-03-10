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

public class SSHInteractorAdapter implements SSHInteractor {

    public void startNewSession(SSHClient client) {}
    public void sessionStarted(SSHClient client) {}

    public void connected(SSHClient client) {}
    public void open(SSHClient client) {}
    public void disconnected(SSHClient client, boolean graceful) {}

    public void report(String msg) {}
    public void alert(String msg) {}

    public void propsStateChanged(SSHPropertyHandler props) {}
    public boolean askConfirmation(String message, boolean defAnswer) {
        return defAnswer;
    }
    public boolean licenseDialog(String license) {
        return false;
    }

    public boolean quietPrompts() {
        return true;
    }
    public String promptLine(String prompt, String defaultVal) throws IOException {
        return null;
    }
    public String promptPassword(String prompt) throws IOException {
        return null;
    }

    public boolean isVerbose() {
        return false;
    }

}
