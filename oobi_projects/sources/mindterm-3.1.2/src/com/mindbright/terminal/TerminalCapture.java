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

package com.mindbright.terminal;

import java.io.IOException;
import java.io.OutputStream;

public final class TerminalCapture implements TerminalOutputListener {

    private OutputStream captureTarget;
    private TerminalWin     terminal;

    public TerminalCapture(OutputStream captureTarget) {
        this.captureTarget = captureTarget;
    }

    public void startCapture(TerminalWin terminal) {
        this.terminal = terminal;
        terminal.addOutputListener(this);
    }

    public void endCapture() {
        terminal.removeOutputListener(this);
    }

    public OutputStream getTarget() {
        return captureTarget;
    }

    public void write(char c) {
        write(new byte[] { (byte)c }, 0, 1);
    }

    private void write(byte[] c, int off, int len) {
        try {
            captureTarget.write(c, off, len);
        } catch (IOException e) {
            // !!! TODO report this to someone...
        }
    }

}
