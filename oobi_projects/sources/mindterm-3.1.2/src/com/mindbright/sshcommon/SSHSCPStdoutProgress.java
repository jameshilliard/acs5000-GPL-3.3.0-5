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

package com.mindbright.sshcommon;

/**
 * Report progress of a file transfer to System.out
 */
public class SSHSCPStdoutProgress implements SSHFileTransferProgress {
    public void startFile(String file, long size) {
        System.out.print("Transfering " + file + " (" + size + " bytes)...");
    }
    public void startDir(String file) {
        System.out.println("Entering directory " + file);
    }
    public void endFile() {
        System.out.println("done");
    }
    public void endDir() {}
    public void progress(long size) {}
}
