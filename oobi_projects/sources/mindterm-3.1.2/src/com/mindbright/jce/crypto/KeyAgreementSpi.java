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


import com.mindbright.jca.security.InvalidAlgorithmParameterException;
import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.Key;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public abstract class KeyAgreementSpi {

    public KeyAgreementSpi() {}

    protected abstract void engineInit(Key key, SecureRandom random)
    throws InvalidKeyException;

    protected abstract void engineInit(Key key, AlgorithmParameterSpec params,
                                       SecureRandom random)
    throws InvalidKeyException, InvalidAlgorithmParameterException;

    protected abstract Key engineDoPhase(Key key, boolean lastPhase)
    throws InvalidKeyException, IllegalStateException;

    protected abstract byte[] engineGenerateSecret()
    throws IllegalStateException;

    protected abstract int engineGenerateSecret(byte[] sharedSecret, int offset)
    throws IllegalStateException, ShortBufferException;

    protected abstract SecretKey engineGenerateSecret(String algorithm)
    throws IllegalStateException, NoSuchAlgorithmException,
                InvalidKeyException;

}
