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

package com.mindbright.security.publickey;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.math.BigInteger;

import com.mindbright.asn1.ASN1DER;
import com.mindbright.asn1.ASN1Integer;
import com.mindbright.asn1.ASN1Sequence;

public final class DSAWithSHA1 extends RawDSAWithSHA1 {

    /**
     * DSA_SIG ::= SEQUENCE {
     *   r        INTEGER,
     *   s        INTEGER,
     * }
     */
    public static final class DSASIG extends ASN1Sequence {

        public ASN1Integer r;
        public ASN1Integer s;

        public DSASIG() {
            r       = new ASN1Integer();
            s       = new ASN1Integer();
            addComponent(r);
            addComponent(s);
        }

        public DSASIG(BigInteger r, BigInteger s) {
            this();
            this.r.setValue(r);
            this.s.setValue(s);
        }

    }


    public static byte[] toDer(byte[] sig) {
        // Extract integers
        int        dataSz = sig.length / 2;
        byte[]     ra     = new byte[dataSz];
        byte[]     sa     = new byte[dataSz];

        System.arraycopy(sig, 0, ra, 0, dataSz);
        System.arraycopy(sig, dataSz, sa, 0, dataSz);

        BigInteger r  = new BigInteger(1, ra);
        BigInteger s  = new BigInteger(1, sa);

        //  Encode
        DSASIG dsasig= new DSASIG(r, s);
        ByteArrayOutputStream enc = new ByteArrayOutputStream(128);
        ASN1DER               der = new ASN1DER();

        try {
            der.encode(enc, dsasig);
        } catch (IOException e) {
            // This should not happen
        }

        return enc.toByteArray();
    }

    //
    // The RawDSAWithSHA1.sign function leaves two 160 bits long integers
    // in its output buffer. We must recode this into a suitable DER structure.
    //
    protected byte[] sign(byte[] data) {
        return toDer(super.sign(data));
    }
}
