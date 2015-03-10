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
import com.mindbright.jca.security.NoSuchProviderException;
import com.mindbright.jca.security.Provider;
import com.mindbright.jca.security.ProviderLookup;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public class KeyAgreement {

    protected KeyAgreementSpi keyAgreementSpi;
    Provider  provider;
    String    algorithm;

    protected KeyAgreement(KeyAgreementSpi keyAgreeSpi, Provider provider,
                           String algorithm) {
        this.keyAgreementSpi = keyAgreeSpi;
        this.provider        = provider;
        this.algorithm       = algorithm;
    }

    public final String getAlgorithm() {
        return algorithm;
    }

    public static final KeyAgreement getInstance(String algorithm)
    throws NoSuchAlgorithmException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("KeyAgreement",
                                                        algorithm);
            return getInstance(algorithm, provider);
        } catch (NoSuchProviderException e) {
            throw new Error("Error in Signature: " + e);
        }

    }

    public static final KeyAgreement getInstance(String algorithm,
            String provider)
    throws NoSuchAlgorithmException, NoSuchProviderException {
        ProviderLookup pl = ProviderLookup.getImplementation("KeyAgreement",
                            algorithm,
                            provider);
        return new KeyAgreement((KeyAgreementSpi)pl.getImpl(),
                                pl.getProvider(), algorithm);
    }

    public final Provider getProvider() {
        return provider;
    }

    public final void init(Key key) throws InvalidKeyException {
        init(key, (SecureRandom)null);
    }

    public final void init(Key key, SecureRandom random)
    throws InvalidKeyException {
        keyAgreementSpi.engineInit(key, random);
    }

    public final void init(Key key, AlgorithmParameterSpec params)
    throws InvalidKeyException, InvalidAlgorithmParameterException {
        init(key, params, null);
    }

    public final void init(Key key, AlgorithmParameterSpec params,
                           SecureRandom random)
    throws InvalidKeyException, InvalidAlgorithmParameterException {
        keyAgreementSpi.engineInit(key, params, random);
    }

    public final Key doPhase(Key key, boolean lastPhase)
    throws InvalidKeyException, IllegalStateException {
        return keyAgreementSpi.engineDoPhase(key, lastPhase);
    }

    public final byte[] generateSecret() throws IllegalStateException {
        return keyAgreementSpi.engineGenerateSecret();
    }

    public final int generateSecret(byte[] sharedSecret, int offset)
    throws IllegalStateException, ShortBufferException {
        return keyAgreementSpi.engineGenerateSecret(sharedSecret, offset);
    }

    public final SecretKey generateSecret(String algorithm)
    throws IllegalStateException, NoSuchAlgorithmException,
        InvalidKeyException {
        return keyAgreementSpi.engineGenerateSecret(algorithm);
    }

}
