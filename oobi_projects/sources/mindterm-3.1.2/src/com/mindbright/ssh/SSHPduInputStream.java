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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.mindbright.util.CRC32;

public final class SSHPduInputStream extends SSHDataInputStream implements SSHPdu {

    public static final class PduByteArrayInputStream extends ByteArrayInputStream {
        PduByteArrayInputStream(byte[] data) {
            super(data);
        }
        public int getPos() {
            return pos;
        }
        public void setPos(int pos) {
            this.pos = pos;
        }
        public byte[] getBuf() {
            return buf;
        }
        public void setBuf(byte[] buf) {
            this.buf   = buf;
            this.count = buf.length;
        }
    }

    public int type;
    public int length;

    byte[]        bytes;
    SSHCipher     cipher;
    SSHCompressor compressor;

    SSHPduInputStream(int type, SSHCipher cipher, SSHCompressor compressor) {
        super(null);
        this.type       = type; // This is the expected type (checked in readFrom())
        this.cipher     = cipher;
        this.compressor = compressor;
    }

    boolean validChecksum() throws IOException {
        int padLen = ((length + 8) & ~7);
        int stored, calculated;

        skip(padLen - 4);
        stored = readInt();
        reset();

        calculated = (int)CRC32.getValue(bytes, 0, padLen - 4);

        if(calculated != stored)
            return false;

        return true;
    }

    public SSHPdu createPdu() {
        return new SSHPduInputStream(this.type, this.cipher, this.compressor);
    }

    public void readFrom(InputStream in) throws IOException {
        SSHDataInputStream dIn    = new SSHDataInputStream(in);
        int                len    = dIn.readInt();
        int                padLen = ((len + 8) & ~7);
        int                type;
        byte[]             data;

        if(padLen > 256000)
            throw new IOException("Corrupt incoming packet, too large");

        data = new byte[padLen];

        dIn.readFully(data);
        if(cipher != null)
            cipher.decrypt(data, 0, data, 0, padLen);

        this.in     = new PduByteArrayInputStream(data);
        this.bytes  = data;
        this.length = len;

        if(!this.validChecksum())
            throw new IOException("Invalid checksum in packet");

        this.skip(8 - (len % 8));

        if(compressor != null) {
            compressor.uncompress(this);
        }

        type = (int)this.readByte();

        if(type == SSH.MSG_DEBUG) {
            SSH.logDebug("MSG_DEBUG: " + this.readString());
            this.readFrom(in);
        } else if(type == SSH.MSG_IGNORE) {
            SSH.logIgnore(this);
            this.readFrom(in);
        } else {
            if((this.type != SSH.MSG_ANY) && (this.type != type)) {
                if(type == SSH.MSG_DISCONNECT)
                    throw new IOException("Server disconnected: " + this.readString());
                else
                    throw new IOException("Invalid type: " + type + " (expected: " +
                                          this.type + ")");
            }
            this.type = type;
        }

    }

    public void writeTo(OutputStream sshOut) throws IOException {
        if(type != SSH.MSG_CHANNEL_DATA &&
                type != SSH.SMSG_STDOUT_DATA &&
                type != SSH.SMSG_STDERR_DATA)
            throw new IOException("Trying to write raw data from non-data PDU");

        // Here we assume that the content left is readable through readString
        // which is the case if this is SSH-data
        //
        int len = readInt();

        PduByteArrayInputStream is = (PduByteArrayInputStream)in;

        sshOut.write(bytes, is.getPos(), len);
        sshOut.flush();
    }

    public PduByteArrayInputStream getIn() {
        return (PduByteArrayInputStream)in;
    }

    public byte[] rawData() {
        return bytes;
    }
    public void rawSetData(byte[] raw) {
        PduByteArrayInputStream is = (PduByteArrayInputStream)in;
        bytes = new byte[raw.length + 4];
        is.setPos(0);
        int len = raw.length;
        int off = 0;
        bytes[off++] = (byte)((len >>> 24) & 0xff);
        bytes[off++] = (byte)((len >>> 16) & 0xff);
        bytes[off++] = (byte)((len >>> 8)  & 0xff);
        bytes[off++] = (byte)(len & 0xff);
        System.arraycopy(raw, 0, bytes, off, raw.length);
        is.setBuf(bytes);
    }
    public int rawOffset() {
        PduByteArrayInputStream is = (PduByteArrayInputStream)in;
        return is.getPos() + 4; // The first four bytes is the length of the data
    }
    public int rawSize() {
        PduByteArrayInputStream is = (PduByteArrayInputStream)in;
        int off = is.getPos();
        int ch1 = ((bytes[off++] + 256) & 0xff);
        int ch2 = ((bytes[off++] + 256) & 0xff);
        int ch3 = ((bytes[off++] + 256) & 0xff);
        int ch4 = ((bytes[off]   + 256) & 0xff);
        return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0));
    }
    // !!! Only allowed to shrink for now !!!
    public void rawAdjustSize(int size) {
        PduByteArrayInputStream is = (PduByteArrayInputStream)in;
        int oldSz = rawSize();
        if(size >= oldSz)
            return;
        int pos = is.getPos() + (oldSz - size);
        is.setPos(pos);
        bytes[pos++] = (byte)((size >>> 24) & 0xff);
        bytes[pos++] = (byte)((size >>> 16) & 0xff);
        bytes[pos++] = (byte)((size >>> 8)  & 0xff);
        bytes[pos++] = (byte) (size & 0xff);
    }

}
