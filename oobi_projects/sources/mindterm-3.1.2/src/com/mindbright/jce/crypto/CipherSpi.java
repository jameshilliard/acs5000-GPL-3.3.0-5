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

package com.mindbright.jce.crypto;

import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.Key;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public abstract class CipherSpi {

    public CipherSpi() {}

    /*
    protected abstract byte[] engineDoFinal(byte[] input, int inputOffset,
    	    int inputLen);
    */

    protected abstract int engineDoFinal(byte[] input, int inputOffset,
                                         int inputLen,
                                         byte[] output, int outputOffset);

    protected abstract int engineGetBlockSize();

    protected abstract byte[] engineGetIV();

    protected abstract int engineGetOutputSize(int inputLen);

    /*
    protected abstract AlgorithmParameters engineGetParameters();
    */

    /*
    protected abstract void engineInit(int opmode, Key key,
           AlgorithmParameters params,
           SecureRandom random);
    */

    protected abstract void engineInit(int opmode, Key key,
                                       AlgorithmParameterSpec params,
                                       SecureRandom random) throws
                InvalidKeyException;

    protected abstract void engineInit(int opmode, Key key,
                                       SecureRandom random) throws
                InvalidKeyException;

    protected abstract void engineSetMode(String mode);

    protected abstract void engineSetPadding(String padding);

    /*
    protected abstract byte[] engineUpdate(byte[] input, int inputOffset,
    	   int inputLen);

    protected abstract int engineUpdate(byte[] input, int inputOffset,
    	int inputLen,
    	byte[] output, int outputOffset);
    */

}
