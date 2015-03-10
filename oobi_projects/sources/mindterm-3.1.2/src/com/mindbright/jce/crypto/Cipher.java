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
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.jca.security.NoSuchProviderException;
import com.mindbright.jca.security.Provider;
import com.mindbright.jca.security.ProviderLookup;
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public class Cipher {

    public final static int DECRYPT_MODE = 1;
    public final static int ENCRYPT_MODE = 2;

    CipherSpi cipherSpi;
    Provider  provider;
    String    transformation;
    String    algorithm;

    protected Cipher(CipherSpi cipherSpi, Provider provider,
                     String transformation) {
        this.cipherSpi      = cipherSpi;
        this.provider       = provider;
        this.transformation = transformation;
        int i = transformation.indexOf('/');
        if(i != -1) {
            this.algorithm = transformation.substring(0, i);
        } else {
            this.algorithm = transformation;
        }
    }

    /*
    public final byte[] doFinal() {
    }
    */

    /*
    public final int doFinal(byte[] output, int outputOffset) {
    }
    */

    public final byte[] doFinal(byte[] input) {
        return doFinal(input, 0, input.length);
    }

    public final byte[] doFinal(byte[] input, int inputOffset, int inputLen) {
        byte[] output = new byte[getOutputSize(inputLen)];
        doFinal(input, inputOffset, inputLen, output, 0);
        return output;
    }

    public final int doFinal(byte[] input, int inputOffset, int inputLen,
                             byte[] output) {
        return doFinal(input, inputOffset, inputLen, output, 0);
    }

    public final int doFinal(byte[] input, int inputOffset, int inputLen,
                             byte[] output, int outputOffset) {
        return cipherSpi.engineDoFinal(input, inputOffset, inputLen,
                                       output, outputOffset);
    }

    public final String getAlgorithm() {
        return algorithm;
    }

    public final int getBlockSize() {
        return cipherSpi.engineGetBlockSize();
    }

    public static final Cipher getInstance(String transformation)
    throws NoSuchAlgorithmException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("Cipher",
                                                        transformation);
            return getInstance(transformation, provider);
        } catch (NoSuchProviderException e) {
            throw new Error("Error in Cipher: " + e);
        }
    }

    public static final Cipher getInstance(String transformation,
                                           String provider)
    throws NoSuchAlgorithmException, NoSuchProviderException {
        ProviderLookup pl = ProviderLookup.getImplementation("Cipher",
                            transformation,
                            provider);

        CipherSpi cipherSpi = (CipherSpi)pl.getImpl();

        int i = transformation.indexOf('/');
        if(i != -1) {
            int j = transformation.indexOf('/', i + 1);
            if(j != -1) {
                cipherSpi.engineSetPadding(transformation.substring(j + 1));
            } else {
                j = transformation.length();
            }
            cipherSpi.engineSetMode(transformation.substring(i + 1, j));
        }

        return new Cipher(cipherSpi, pl.getProvider(), transformation);
    }

    public final byte[] getIV() {
        return cipherSpi.engineGetIV();
    }

    public final int getOutputSize(int inputLen) {
        return cipherSpi.engineGetOutputSize(inputLen);
    }

    /*
    public final AlgorithmParameters getParameters() {
    return cipherSpi.engineGetParameters();
    }
    */

    public final Provider getProvider() {
        return provider;
    }

    public final void init(int opmode, Key key) throws InvalidKeyException {
        init(opmode, key, null);
    }

    public final void init(int opmode, Key key, AlgorithmParameterSpec params)
    throws InvalidKeyException {
        cipherSpi.engineInit(opmode, key, params, null);
    }

    /*
    public final void init(int opmode, Key key, AlgorithmParameters params) {
    }

    public final void init(int opmode, Key key, AlgorithmParameters params,
      SecureRandom random) {
    }
    */

    /*
    public final void init(int opmode, Key key, AlgorithmParameterSpec params,
      SecureRandom random) {
    }

    public final void init(int opmode, Key key, SecureRandom random) {
    }
    */

    /*
    public final byte[] update(byte[] input) {
    }

    public final byte[] update(byte[] input, int inputOffset, int inputLen) {
    }

    public final int update(byte[] input, int inputOffset, int inputLen,
        byte[] output) {
    }

    public final int update(byte[] input, int inputOffset, int inputLen,
        byte[] output, int outputOffset) {
    }
    */
}
