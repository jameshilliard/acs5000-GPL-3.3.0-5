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

import com.isnetworks.ssh.LocalFileBrowser;
import com.mindbright.gui.AWTConvenience;
import com.mindbright.ssh2.SSH2SFTPFactory;
import com.mindbright.ssh2.SSH2SFTPFileBrowser;
import com.mindbright.sshcommon.SSHChdirEventHandler;
import com.mindbright.sshcommon.SSHFileTransferDialogControl;
import com.mindbright.sshcommon.SSHFileTransferFactory;

public class ModuleSFTP implements MindTermModule, SSHChdirEventHandler {

    private MindTermApp mindtermapp;

    public void init(MindTermApp mindterm) {}

    public void activate(MindTermApp mindterm) {
        mindtermapp = mindterm;

        String title = mindterm.getAppName() +
                       " - SFTP (" + mindterm.getHost() + ")";

        String lcwd = mindterm.getProperty("module.sftp.cwd-local");
        String rcwd = mindterm.getProperty("module.sftp.cwd-remote");
        if(rcwd == null) {
            rcwd = ".";
        }
        try {
            if(lcwd == null) {
                lcwd = System.getProperty("user.home");
                if(lcwd == null)
                    lcwd = System.getProperty("user.dir");
                if(lcwd == null)
                    lcwd = System.getProperty("java.home");
            }
        } catch (Throwable t) {
            // !!!
        }

        SSHFileTransferDialogControl dialog = null;

        try {
            java.awt.Frame f = mindterm.getParentFrame();
            Class c = 
                Class.forName(AWTConvenience.isSwingJFrame(f) ? 
                              "com.mindbright.sshcommon.SSHFileTransferDialogSwing" :
                              "com.mindbright.sshcommon.SSHFileTransferDialogAWT");
            java.lang.reflect.Constructor cons =
                c.getConstructor(new Class[] { String.class, MindTermApp.class, 
                                               SSHFileTransferFactory.class});
            dialog = (SSHFileTransferDialogControl)cons.newInstance
                (new Object[] { title, mindterm, new SSH2SFTPFactory() });
        } catch (Throwable t) {
        }

        dialog.setLocalFileBrowser
            (new LocalFileBrowser(dialog.getLocalFileDisplay(), lcwd));

        dialog.setRemoteFileBrowser
            (new SSH2SFTPFileBrowser(mindterm.getConnection(),
                                     dialog.getRemoteFileDisplay(), rcwd));

        dialog.setLocalChdirCallback(this);

        dialog.doShow();
    }

    public boolean isAvailable(MindTermApp mindterm) {
        return (mindterm.isConnected() && (mindterm.getConnection() != null));
    }

    public void connected(MindTermApp mindterm) {}

    public void disconnected(MindTermApp mindterm) {}

    public String description(MindTermApp mindterm) {
        return null;
    }

    public void chdir(String newdir) {
        mindtermapp.setProperty("module.sftp.cwd-local", newdir);
    }
}
