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

import com.mindbright.jca.security.InvalidAlgorithmParameterException;
import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.Key;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;
import com.mindbright.jce.crypto.KeyAgreementSpi;
import com.mindbright.jce.crypto.SecretKey;
import com.mindbright.jce.crypto.ShortBufferException;

public final class DHKeyAgreement extends KeyAgreementSpi {

    private DHPrivateKey prvKey;
    private SecureRandom random;
    private BigInteger   lastKey;
    private boolean      lastPhase;

    protected void engineInit(Key key, SecureRandom random)
    throws InvalidKeyException {
        if(!(key instanceof DHPrivateKey)) {
            throw new InvalidKeyException("DHKeyAgreement got: " + key);
        }
        this.prvKey  = (DHPrivateKey)key;
        this.random  = random;
    }

    protected void engineInit(Key key, AlgorithmParameterSpec params,
                              SecureRandom random)
    throws InvalidKeyException, InvalidAlgorithmParameterException {
        throw new InvalidAlgorithmParameterException(
            "DHKeyAgreement params not supported: " +
            params);
    }

    protected Key engineDoPhase(Key key, boolean lastPhase)
    throws InvalidKeyException, IllegalStateException {
        if(!(key instanceof DHPublicKey)) {
            throw new InvalidKeyException("Invalid key: " + key);
        }
        this.lastPhase = lastPhase;

        BigInteger y  = ((DHPublicKey)key).getY();

        lastKey = DiffieHellman.computeKey(prvKey.getX(),
                                           y, prvKey.getP());

        return new DHPublicKey(lastKey, prvKey.getP(), prvKey.getG());
    }

    protected byte[] engineGenerateSecret() throws IllegalStateException {
        if(!lastPhase) {
            throw new IllegalStateException("DHKeyAgreement not final");
        }
        byte[] sharedSecret = lastKey.toByteArray();
        lastPhase = false;
        lastKey   = null;
        return sharedSecret;
    }

    protected int engineGenerateSecret(byte[] sharedSecret, int offset)
    throws IllegalStateException, ShortBufferException {
        byte[] genSecret = engineGenerateSecret();
        if(genSecret.length > (sharedSecret.length - offset)) {
            throw new ShortBufferException("DHKeyAgreement, buffer too small");
        }
        System.arraycopy(genSecret, 0, sharedSecret, offset, genSecret.length);
        return genSecret.length;
    }

    protected SecretKey engineGenerateSecret(String algorithm)
    throws IllegalStateException, NoSuchAlgorithmException,
        InvalidKeyException {
        throw new Error("DHKeyAgreement.engineGenerateSecret(String) not " +
                        "implemented");
    }

}
