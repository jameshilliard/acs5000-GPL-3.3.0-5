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

public final class ASN1Implicit extends ASN1Object {

    private ASN1Object underlying;

    public ASN1Implicit(int tag, ASN1Object underlying) {
        this(tag, ASN1.CLASS_CONTEXT, underlying);
    }

    public ASN1Implicit(int tag, int cl, ASN1Object underlying) {
        super(tag | cl | (underlying.getTag() & ASN1.TYPE_CONSTRUCTED));
        this.underlying = underlying;
    }

    public void setValue() {
        underlying.setValue();
    }

    public boolean isSet() {
        return underlying.isSet();
    }

    public int encodeValue(ASN1Encoder encoder, OutputStream out)
    throws IOException {
        return underlying.encodeValue(encoder, out);
    }

    public void decodeValue(ASN1Decoder decoder, InputStream in,
                            int tag, int len)
    throws IOException {
        tag = ((tag & ASN1.TYPE_CONSTRUCTED) | underlying.getTag());
        decoder.decodeValue(in, tag, len, underlying);
    }

    public boolean equals(Object obj) {
        return underlying.equals(obj);
    }

    public int hashCode() {
        return underlying.hashCode();
    }

}
