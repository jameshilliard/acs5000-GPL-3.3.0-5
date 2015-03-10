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
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public class Mac implements Cloneable {

    private Provider provider;
    private MacSpi   engine;
    private String   algorithm;
    private boolean  initialized;
    private int      macLength;

    protected Mac(MacSpi macSpi, Provider provider, String algorithm) {
        this.provider    = provider;
        this.engine      = macSpi;
        this.algorithm   = algorithm;
        this.initialized = false;
    }

    public Object clone() throws CloneNotSupportedException {
        Mac clone = new Mac((MacSpi)this.engine.clone(), this.provider,
                            this.algorithm);
        clone.initialized = this.initialized;
        return clone;
    }

    public final byte[] doFinal() throws IllegalStateException {
        if(initialized) {
            byte[] mac = engine.engineDoFinal();
            engine.engineReset();
            return mac;
        } else {
            throw new IllegalStateException(algorithm + " not initialized");
        }
    }

    public final byte[] doFinal(byte[] input) throws IllegalStateException {
        if(initialized)	{
            engine.engineUpdate(input, 0, input.length);
            return doFinal();
        } else {
            throw new IllegalStateException(algorithm + " not initialized");
        }
    }

    public final void doFinal(byte[] output, int outOffset)
    throws ShortBufferException, IllegalStateException {
        if(initialized) {
            if(output.length - outOffset < macLength)
                throw new ShortBufferException("Output buffer is too small for "
                                               + algorithm);
            System.arraycopy(engine.engineDoFinal(), 0, output, outOffset, macLength);
        } else {
            throw new IllegalStateException(algorithm + " not initialized");
        }
    }

    public final String getAlgorithm() {
        return algorithm;
    }

    public final static Mac getInstance(String algorithm)
    throws NoSuchAlgorithmException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("Mac", algorithm);
            return getInstance(algorithm, provider);
        } catch (NoSuchProviderException e) {
            throw new Error("Error in Mac: " + e);
        }
    }

    public final static Mac getInstance(String algorithm, String provider)
    throws NoSuchProviderException, NoSuchAlgorithmException {
        ProviderLookup pl = ProviderLookup.getImplementation("Mac",
                            algorithm,
                            provider);
        return new Mac((MacSpi)pl.getImpl(), pl.getProvider(), algorithm);
    }

    public final int getMacLength() {
        int len = macLength;
        if(!initialized) {
            len = engine.engineGetMacLength();
        }
        return len;
    }

    Provider getProvider() {
        return provider;
    }

    public final void init(Key key) throws InvalidKeyException {
        try {
            init(key, null);
        } catch (InvalidAlgorithmParameterException e) {
            throw new Error("Error in Mac: " + e);
        }
    }

    public final void init(Key key, AlgorithmParameterSpec params)
    throws InvalidKeyException, InvalidAlgorithmParameterException {
        engine.engineInit(key, params);
        macLength   = engine.engineGetMacLength();
        initialized = true;
    }

    public final void reset() {
        engine.engineReset();
    }

    public final void update(byte input) throws IllegalStateException {
        if(initialized) {
            engine.engineUpdate(input);
        } else {
            throw new IllegalStateException(algorithm + " not initialized");
        }
    }

    public final void update(byte[] input) throws IllegalStateException {
        update(input, 0, input.length);
    }

    public final void update(byte[] input, int offset, int len)
    throws IllegalStateException {
        if(initialized) {
            engine.engineUpdate(input, offset, len);
        } else {
            throw new IllegalStateException(algorithm + " not initialized");
        }
    }

}

