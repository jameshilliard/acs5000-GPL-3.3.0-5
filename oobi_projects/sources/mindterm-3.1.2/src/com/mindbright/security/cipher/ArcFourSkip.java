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

package com.mindbright.security.cipher;

import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.Key;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;
import com.mindbright.jce.crypto.CipherSpi;
import com.mindbright.jce.crypto.spec.SecretKeySpec;

public final class ArcFourSkip extends CipherSpi {
    ArcFour rc4;

    public ArcFourSkip() {
        rc4 = new ArcFour();
    }

    protected int engineDoFinal(byte[] input, int inputOffset, int inputLen,
                                byte[] output, int outputOffset) {
        return rc4.engineDoFinal(input, inputOffset, inputLen,
                                 output, outputOffset);
    }

    public void initializeKey(byte[] key) {
        rc4.initializeKey(key);

        byte[] buf = new byte[1536];
        for (int i=0; i<1536; i++) {
            buf[i] = (byte)rc4.arcfour_byte();
        }
    }

    protected int engineGetBlockSize() {
        return rc4.engineGetBlockSize();
    }

    protected byte[] engineGetIV() {
        return rc4.engineGetIV();
    }

    protected int engineGetOutputSize(int inputLen) {
        return rc4.engineGetOutputSize(inputLen);
    }

    protected void engineInit(int opmode, Key key,
                              AlgorithmParameterSpec params,
                              SecureRandom random)
        throws InvalidKeyException {
        initializeKey(((SecretKeySpec)key).getEncoded());
    }

    protected void engineInit(int opmode, Key key,
                              SecureRandom random) throws InvalidKeyException {
        engineInit(opmode, key, (AlgorithmParameterSpec)null, random);
    }

    protected void engineSetMode(String mode) {
        rc4.engineSetMode(mode);
    }

    protected void engineSetPadding(String padding) {
        rc4.engineSetPadding(padding);
    }
}
