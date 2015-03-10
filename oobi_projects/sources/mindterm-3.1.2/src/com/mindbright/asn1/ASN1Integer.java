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
import java.math.BigInteger;

public class ASN1Integer extends ASN1Object {

    private BigInteger value;

    public ASN1Integer() {
        super(ASN1.TAG_INTEGER);
    }

    public int encodeValue(ASN1Encoder encoder, OutputStream out)
    throws IOException {
        return encoder.encodeInteger(out, value);
    }

    public void decodeValue(ASN1Decoder decoder, InputStream in, int len)
    throws IOException {
        setValue(decoder.decodeInteger(in, len));
    }

    public void setValue(BigInteger value) {
        setValue();
        this.value = value;
    }

    public void setValue(long value) {
        setValue(BigInteger.valueOf(value));
    }

    public BigInteger getValue() {
        return value;
    }

    public boolean equals(Object obj) {
        if(obj == null || !(obj instanceof ASN1Integer)) {
            return false;
        }
        ASN1Integer other = (ASN1Integer)obj;
        return value.equals(other);
    }

    public int hashCode() {
        int hc = super.hashCode();
        if(value != null) {
            hc = value.hashCode();
        }
        return hc;
    }

}
