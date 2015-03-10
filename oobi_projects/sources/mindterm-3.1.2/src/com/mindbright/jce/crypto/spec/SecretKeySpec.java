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

package com.mindbright.jce.crypto.spec;

import com.mindbright.jca.security.spec.KeySpec;
import com.mindbright.jce.crypto.SecretKey;

public class SecretKeySpec implements KeySpec, SecretKey {

    private byte[] key;
    private String algorithm;

    public SecretKeySpec(byte[] key, int offset, int len, String algorithm) {
        this.key = new byte[len];
        System.arraycopy(key, offset, this.key, 0, len);
        this.algorithm = algorithm;
    }

    public SecretKeySpec(byte[] key, java.lang.String algorithm) {
        this(key, 0, key.length, algorithm);
    }

    public boolean equals(java.lang.Object obj) {
        return false;
    }

    public String getAlgorithm() {
        return algorithm;
    }

    public byte[] getEncoded() {
        return key;
    }

    public String getFormat() {
        return null;
    }

    public int hashCode() {
        return 0;
    }
}
