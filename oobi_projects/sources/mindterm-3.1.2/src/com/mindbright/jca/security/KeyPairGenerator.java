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

package com.mindbright.jca.security;

import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public class KeyPairGenerator extends KeyPairGeneratorSpi {

    KeyPairGeneratorSpi keyPairGenSpi;
    Provider            provider;
    String              algorithm;

    protected KeyPairGenerator(String algorithm) {
        this.algorithm = algorithm;
    }

    public String getAlgorithm() {
        return algorithm;
    }

    public static KeyPairGenerator getInstance(String algorithm)
    throws NoSuchAlgorithmException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("KeyPairGenerator",
                                                        algorithm);
            return getInstance(algorithm, provider);
        } catch(NoSuchProviderException e) {
            throw new NoSuchAlgorithmException(algorithm + " not found");
        }
    }

    public static KeyPairGenerator getInstance(String algorithm,
            String provider)
    throws NoSuchAlgorithmException, NoSuchProviderException {
        ProviderLookup pl = ProviderLookup.getImplementation("KeyPairGenerator",
                            algorithm,
                            provider);

        KeyPairGeneratorSpi keyPairGenSpi = (KeyPairGeneratorSpi)pl.getImpl();
        KeyPairGenerator    keyPairGen    = new KeyPairGenerator(algorithm);

        keyPairGen.provider      = pl.getProvider();
        keyPairGen.keyPairGenSpi = keyPairGenSpi;

        return keyPairGen;
    }

    public final Provider getProvider() {
        return provider;
    }

    public void initialize(int keysize) {
        initialize(keysize, null);
    }

    public void initialize(int keysize, SecureRandom random) {
        keyPairGenSpi.initialize(keysize, random);
    }

    public void initialize(AlgorithmParameterSpec params)
    throws InvalidAlgorithmParameterException {
        initialize(params, null);
    }

    public void initialize(AlgorithmParameterSpec params, SecureRandom random)
    throws InvalidAlgorithmParameterException {
        keyPairGenSpi.initialize(params, random);
    }

    public final KeyPair genKeyPair() {
        return generateKeyPair();
    }

    public KeyPair generateKeyPair() {
        return keyPairGenSpi.generateKeyPair();
    }

}
