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

public final class ArcFour extends CipherSpi {
    int    x;
    int    y;
    int[]  state = new int[256];

    int arcfour_byte() {
        int x;
        int y;
        int sx, sy;
        x = (this.x + 1) & 0xff;
        sx = state[x];
        y = (sx + this.y) & 0xff;
        sy = state[y];
        this.x = x;
        this.y = y;
        state[y] = (sx & 0xff);
        state[x] = (sy & 0xff);
        return state[((sx + sy) & 0xff)];
    }

    protected int engineDoFinal(byte[] input, int inputOffset, int inputLen,
                                byte[] output, int outputOffset) {
        int end = inputOffset + inputLen;
        for(int si = inputOffset, di = outputOffset; si < end; si++, di++)
            output[di] = (byte)(((int)input[si] ^ arcfour_byte()) & 0xff);
        return inputLen;
    }

    public void initializeKey(byte[] key) {
        int t, u = 0;
        int keyindex;
        int stateindex;
        int counter;

        for(counter = 0; counter < 256; counter++)
            state[counter] = (byte)counter;
        keyindex = 0;
        stateindex = 0;
        for(counter = 0; counter < 256; counter++) {
            t = state[counter];
            stateindex = (stateindex + key[keyindex] + t) & 0xff;
            u = state[stateindex];
            state[stateindex] = t;
            state[counter] = u;
            if(++keyindex >= key.length)
                keyindex = 0;
        }
    }

    protected int engineGetBlockSize() {
        return 1;
    }

    protected byte[] engineGetIV() {
        return null;
    }

    protected int engineGetOutputSize(int inputLen) {
        return inputLen;
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

    protected void engineSetMode(String mode) {}

    protected void engineSetPadding(String padding) {}

}
