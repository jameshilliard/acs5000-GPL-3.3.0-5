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

import java.io.File;

import com.mindbright.application.MindTermApp;
import com.mindbright.sshcommon.SSHFileTransfer;
import com.mindbright.sshcommon.SSHFileTransferFactory;

public class SSHSCPFactory implements SSHFileTransferFactory {
    public SSHFileTransfer create(final MindTermApp client, File cwd)
    throws Exception {
        SSHInteractor interactAdapter = new SSHInteractorAdapter() {
                                            public void alert(String msg) {
                                                client.alert(msg);
                                            }
                                        };
        SSHInteractiveClient ic = (SSHInteractiveClient)client;
        SSHSCPClient scpClient = new SSHSCPClient(client.getHost(),
                                 client.getPort(),
                                 ic.propsHandler,
                                 interactAdapter,
                                 cwd, false);
        scpClient.setClientUser(ic.propsHandler);
        return scpClient.scp1();
    }
}
