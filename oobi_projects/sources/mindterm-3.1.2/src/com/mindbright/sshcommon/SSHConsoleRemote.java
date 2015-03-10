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

import java.io.InputStream;
import java.io.OutputStream;

/**
 * Basic interface to classes implementing a console to a remote
 * command or shell.
 */
public interface SSHConsoleRemote {
    /**
     * Runs single command on server.
     *
     * @param command command line to run
     *
     * @return a boolean indicating success or failure
     */
    public boolean command(String command);

    /**
     * Starts an interactive shell on the server, note that no PTY is
     * allocated.
     *
     * @return a boolean indicating success or failure
     */
    public boolean shell();

    /**
     * Closes the session channel. That is cancels a command/shell in
     * progress if it hasn't already exited.
     */
    public void close();

    /**
     * Closes the session channel. If waitforcloseconfirm is true it
     * waits for the remote end to acknowledge the close.
     */
    public void close(boolean waitforcloseconfirm);

    /**
     * Changes the output stream where stdout is written to in the underlying
     * session channel.
     *
     * @param out new stdout stream
     */
    public void changeStdOut(OutputStream out);

    /**
     * Gets the stdin stream of the underlying session channel. Note, this is
     * an output stream since one wants to <b>write</b> to stdin.
     *
     * @return the input stream of stdout stream
     */
    public OutputStream getStdIn();

    /**
     * Gets the stdout stream of the underlying session channel. Note, this is
     * an input stream since one wants to <b>read</b> from stdout.
     *
     * @return the input stream of stdout stream
     */
    public InputStream  getStdOut();
}
