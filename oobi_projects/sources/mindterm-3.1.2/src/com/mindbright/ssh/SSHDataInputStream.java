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

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.math.BigInteger;

public class SSHDataInputStream extends DataInputStream {

    SSHDataInputStream(InputStream in) {
        super(in);
    }

    public BigInteger readBigInteger() throws IOException {
        short  bits = readShort();
        byte[] raw  = new byte[(bits + 7) / 8 + 1];

        raw[0] = 0;
        read(raw, 1, raw.length - 1);

        return new BigInteger(raw);
    }

    public String readString() throws IOException {
        int    len = readInt();
        byte[] raw = new byte[len];
        read(raw, 0, raw.length);
        return new String(raw);
    }

    public byte[] readStringAsBytes() throws IOException {
        int    len = readInt();
        byte[] raw = new byte[len];
        read(raw, 0, raw.length);
        return raw;
    }

}
