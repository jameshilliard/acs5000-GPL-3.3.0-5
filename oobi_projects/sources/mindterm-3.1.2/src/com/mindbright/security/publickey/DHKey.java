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

import com.mindbright.jca.security.Key;
import com.mindbright.jce.crypto.spec.DHParameterSpec;

public class DHKey extends DHParameterSpec
    implements com.mindbright.jce.crypto.interfaces.DHKey, Key {

    protected DHKey(BigInteger p, BigInteger g) {
        super(p, g);
    }

    public String getAlgorithm() {
        return "DiffieHellman";
    }

    public String getFormat() {
        return null;
    }

    public byte[] getEncoded() {
        return null;
    }

    public DHParameterSpec getParams() {
        return this;
    }

}
