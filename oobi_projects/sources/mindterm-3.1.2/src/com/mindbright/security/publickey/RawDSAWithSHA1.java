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

import java.math.BigInteger;

import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.interfaces.DSAParams;
import com.mindbright.jca.security.interfaces.DSAPrivateKey;
import com.mindbright.jca.security.interfaces.DSAPublicKey;

public class RawDSAWithSHA1 extends BaseSignature {

    public RawDSAWithSHA1() {
        super("SHA1");
    }

    protected void initVerify() throws InvalidKeyException {
        if(publicKey == null || !(publicKey instanceof DSAPublicKey)) {
            throw new InvalidKeyException("Wrong key for DSAWithSHA1 verify: " +
                                          publicKey);
        }
    }

    protected void initSign() throws InvalidKeyException {
        if(privateKey == null || !(privateKey instanceof DSAPrivateKey)) {
            throw new InvalidKeyException("Wrong key for DSAWithSHA1 sign: " +
                                          privateKey);
        }
    }

    protected byte[] sign(byte[] data) {
        DSAPrivateKey key  = (DSAPrivateKey)privateKey;
        DSAParams     parm = key.getParams();
        BigInteger    x    = key.getX();
        BigInteger    p    = parm.getP();
        BigInteger    q    = parm.getQ();
        BigInteger    g    = parm.getG();
        return DSAAlgorithm.sign(x, p, q, g, data);
    }

    protected boolean verify(byte[] signature, byte[] data) {
        DSAPublicKey key  = (DSAPublicKey)publicKey;
        DSAParams    parm = key.getParams();
        BigInteger   y    = key.getY();
        BigInteger   p    = parm.getP();
        BigInteger   q    = parm.getQ();
        BigInteger   g    = parm.getG();
        return DSAAlgorithm.verify(y, p, q, g, signature, data);
    }

}
