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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import com.mindbright.util.CRC32;
import com.mindbright.util.SecureRandomAndPad;

public final class SSHPduOutputStream extends SSHDataOutputStream
    implements SSHPdu {

    public static final class PduByteArrayOutputStream extends ByteArrayOutputStream {
        PduByteArrayOutputStream() {
            super();
        }

        PduByteArrayOutputStream(int size) {
            super(size);
        }

        PduByteArrayOutputStream(byte[] buf) {
            this.buf = buf;
        }

        public byte[] getBuf() {
            return buf;
        }

        public int getCount() {
            return count;
        }

        public void setBuf(byte[] buf) {
            this.buf = buf;
        }

        public void setCount(int count) {
            this.count = count;
        }
    }

    public static final int SSH_DEFAULT_PKT_LEN = 8192;
    public static int mtu = SSH_DEFAULT_PKT_LEN;

    public static synchronized void setMTU(int newMtu) {
        mtu = newMtu;
    }

    byte[]  readFromRawData;
    int     readFromOff;
    int     readFromSize;

    public int                type;
    public SSHCipher          cipher;
    public SSHCompressor      compressor;
    public SecureRandomAndPad rand;

    SSHPduOutputStream(SSHCipher cipher, SSHCompressor compressor,
                       SecureRandomAndPad rand) {
        super(null);
        this.cipher     = cipher;
        this.compressor = compressor;
        this.rand       = rand;
    }

    SSHPduOutputStream(int type, SSHCipher cipher, SSHCompressor compressor,
                       SecureRandomAndPad rand)
    throws IOException {
        super(new PduByteArrayOutputStream(mtu));
        this.type       = type;
        this.cipher     = cipher;
        this.compressor = compressor;
        this.rand       = rand;
        if(cipher != null) {
            PduByteArrayOutputStream bytes = (PduByteArrayOutputStream)out;
            rand.nextPadBytes(bytes.getBuf(), 0, 8);
            bytes.setCount(8);
        } else {
            for(int i = 0; i < 8; i++)
                write(0);
        }
        write(type);
    }

    public SSHPdu createPdu() throws IOException {
        SSHPdu pdu;
        pdu = new SSHPduOutputStream(this.type, this.cipher, this.compressor,
                                     this.rand);
        return pdu;
    }

    public void readFrom(InputStream in) throws IOException {
        if(type != SSH.MSG_CHANNEL_DATA &&
                type != SSH.CMSG_STDIN_DATA)
            throw new IOException("Trying to read raw data into non-data PDU");

        PduByteArrayOutputStream bytes = (PduByteArrayOutputStream) out;

        readFromRawData = bytes.getBuf();
        readFromOff     = bytes.size() + 4; // Leave space for size

        readFromSize = in.read(readFromRawData, readFromOff, mtu - readFromOff);
        if(readFromSize == -1)
            throw new IOException("EOF");

        writeInt(readFromSize);
        bytes.setCount(readFromOff + readFromSize);
    }

    public void writeTo(OutputStream sshOut) throws IOException {
        PduByteArrayOutputStream bytes = (PduByteArrayOutputStream) out;
        byte[]                   padData;
        int                      iSz;
        int                      pad;
        int                      crc32;
        int                      padSz;

        if(compressor != null) {
            compressor.compress(this);
        }

        iSz   = bytes.size();
        pad   = (iSz + 4) % 8;
        crc32 = (int)CRC32.getValue(bytes.getBuf(), pad, iSz - pad);
        padSz = iSz + 4 - pad;
        writeInt(crc32);

        padData = bytes.getBuf();

        if(cipher != null) {
            cipher.encrypt(padData, pad, padData, pad, padSz);
        }

        int sz = padSz - (8 - pad);
        sshOut.write((sz >>> 24) & 0xff);
        sshOut.write((sz >>> 16) & 0xff);
        sshOut.write((sz >>>  8) & 0xff);
        sshOut.write((sz >>>  0) & 0xff);

        sshOut.write(padData, pad, padSz);

        sshOut.flush();
    }

    public PduByteArrayOutputStream getOut() {
        return (PduByteArrayOutputStream)out;
    }

    public byte[] rawData() {
        return readFromRawData;
    }
    public void rawSetData(byte[] raw) {}
    public int rawOffset() {
        return readFromOff;
    }
    public int rawSize() {
        // !!! return readFromSize;
        //
        byte[] bytes = readFromRawData;
        int    off   = readFromOff - 4;
        int ch1 = ((bytes[off++] + 256) & 0xff);
        int ch2 = ((bytes[off++] + 256) & 0xff);
        int ch3 = ((bytes[off++] + 256) & 0xff);
        int ch4 = ((bytes[off]   + 256) & 0xff);
        return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0));
    }
    public void rawAdjustSize(int size) {
        PduByteArrayOutputStream bytes = (PduByteArrayOutputStream) out;
        bytes.setCount(size);
    }

}

