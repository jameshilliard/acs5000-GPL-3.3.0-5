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

import java.awt.Frame;
import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;
import java.util.NoSuchElementException;

import com.mindbright.terminal.TerminalFrameTitle;
import com.mindbright.terminal.TerminalWindow;

public class ModuleDebugConsoleImpl extends ModuleBaseTerminal {

    private TerminalWindow terminal;
    private PrintStream    outOrig;
    private PrintStream    errOrig;

    private volatile boolean isRunning;

    final class DebugOutStream extends OutputStream {
        public void write(int b) throws IOException {
            terminal.write((char)b);
        }
        public void write(byte b[], int off, int len) throws IOException {
            terminal.write(b, off, len);
        }
    }

    protected void runTerminal(MindTermApp mindterm,
                               TerminalWindow terminal, Frame frame,
                               TerminalFrameTitle frameTitle) {
        isRunning     = true;
        this.terminal = terminal;
        outOrig       = System.out;
        errOrig       = System.err;

        try {
            terminal.setProperty("auto-linefeed", "true");
        } catch (NoSuchElementException e) {}

        PrintStream debugOut = new PrintStream(new DebugOutStream());
        try {
            System.setOut(debugOut);
            System.setErr(debugOut);
        } catch (Throwable t) {
            terminal.write("\n\rError, couldn't redirect STDIO: " +
                           t.getMessage());
        }
        synchronized(this) {
            try {
                this.wait();
            } catch (InterruptedException e) {}
        }
    }

    protected boolean closeOnDisconnect() {
        return false;
    }

    protected String getTitle() {
        return mindterm.getAppName() + " - " + "Debug Console";
    }

    protected void doClose() {
        System.setOut(outOrig);
        System.setErr(outOrig);
        this.terminal = null;
        synchronized(this) {
            this.notifyAll();
        }
    }

    public boolean isAvailable(MindTermApp mindterm) {
        return true;
    }

    protected ModuleBaseTerminal newInstance() {
        return this;
    }

}
