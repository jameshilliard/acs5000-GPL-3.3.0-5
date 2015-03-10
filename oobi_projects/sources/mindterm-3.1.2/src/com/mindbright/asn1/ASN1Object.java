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

package com.mindbright.asn1;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public abstract class ASN1Object {

    protected int     tag;
    protected boolean isSet;

    protected ASN1Object(int tag) {
        this.tag = tag;
    }

    public final int getTag() {
        return tag;
    }

    public String getType() {
        String name = this.getClass().getName();
        int i = name.lastIndexOf(".");
        if(i > 0) {
            name = name.substring(i + 1);
        }
        return name;
    }

    public void setValue() {
        isSet = true;
    }

    public boolean isSet() {
        return isSet;
    }

    public void decodeValue(ASN1Decoder decoder, InputStream in,
                            int tag, int len)
    throws IOException {
        if(tag != this.tag) {
            throw new
            IOException("Invalid encoding, tag mismatch when decoding " +
                        getType() + " (expected: " +
                        this.tag + ", got: " + tag + ")");
        }
        decodeValue(decoder, in, len);
    }

    protected void decodeValue(ASN1Decoder decoder, InputStream in, int len)
    throws IOException {
        throw new IOException("ASN1 decoder " + decoder + " couldn't decode " +
                              getType());
    }

    public abstract int encodeValue(ASN1Encoder encoder, OutputStream out)
    throws IOException;

}
