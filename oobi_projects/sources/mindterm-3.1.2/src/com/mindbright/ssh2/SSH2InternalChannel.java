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

package com.mindbright.ssh2;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.mindbright.util.InputStreamPipe;
import com.mindbright.util.OutputStreamPipe;

/**
 * Implements an internal channel which is connected ot a pair of pipes.
 */
public class SSH2InternalChannel extends SSH2StreamChannel {
    protected InputStreamPipe  rxPipe;
    protected OutputStreamPipe txPipe;

    /**
     * Create a new internal channel of the given type. The channel is
     * associated with an ssh connection. Channel types are
     * defined in <code>SSH2Connection</code> and starts with
     * <code>CH_TYPE</code>.
     *
     * @param channelType Type of channel to create.
     * @param connection The ssh connection to associate the channel with.
     */
    public SSH2InternalChannel(int channelType, SSH2Connection connection) {
        super(channelType, connection, connection,
              null, null);

        int ioBufSz = connection.getPreferences().
                      getIntPreference(SSH2Preferences.INT_IO_BUF_SZ);
        in  = new InputStreamPipe(ioBufSz);
        out = new OutputStreamPipe();

        try {
            this.txPipe = new OutputStreamPipe();
            this.rxPipe = new InputStreamPipe(ioBufSz);
            this.rxPipe.connect((OutputStreamPipe)out);
            this.txPipe.connect((InputStreamPipe)in);
        } catch (IOException e) {
            connection.getLog().error("SSH2InternalChannel", "<constructor>",
                                      "can't happen, bug somewhere!?!");
        }
    }

    /**
     * Get the input stream of the channel.
     *
     * @return The input stream.
     */
    public InputStream getInputStream() {
        return rxPipe;
    }

    /**
     * Get the output stream of the channel.
     *
     * @return The output stream.
     */
    public OutputStream getOutputStream() {
        return txPipe;
    }

}
