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

import java.util.Random;

import com.mindbright.util.RandomSeed;

public class SecureRandom extends Random {
    SecureRandomSpi engine;
    Provider        provider;

    public SecureRandom() {
        this(new com.mindbright.security.prng.BlumBlumShub(), null);
    }

    public SecureRandom(byte[] seed) {
        this(new com.mindbright.security.prng.BlumBlumShub(seed), null);
    }

    protected SecureRandom(SecureRandomSpi secureRandomSpi, Provider provider) {
        super(0); // Just to trick Random, we don't want this seed...
        this.engine   = secureRandomSpi;
        this.provider = provider;
    }

    public byte[] generateSeed(int numBytes) {
        return engine.engineGenerateSeed(numBytes);
    }

    public static SecureRandom getInstance(String algorithm)
    throws NoSuchAlgorithmException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("SecureRandom",
                                                        algorithm);
            return getInstance(algorithm, provider);
        } catch (NoSuchProviderException e) {
            throw new Error("Error in SecureRandom: " + e);
        }
    }

    public static SecureRandom getInstance(String algorithm, String provider)
    throws NoSuchAlgorithmException, NoSuchProviderException {
        ProviderLookup pl = ProviderLookup.getImplementation("SecureRandom",
                            algorithm,
                            provider);
        return new SecureRandom((SecureRandomSpi)pl.getImpl(),
                                pl.getProvider());
    }

    public Provider getProvider() {
        return provider;
    }

    public static byte[] getSeed(int numBytes) {
        // !!! TODO
        RandomSeed seed = new RandomSeed("/dev/urandom", "/dev/urandom");
        return seed.getBytesBlocking(numBytes);
    }

    protected final int next(int numBits) {
        int    numBytes = (numBits + 7) / 8;
        byte[] b        = new byte[numBytes];
        int    next     = 0;
        nextBytes(b);
        for(int i = 0; i < numBytes; i++)
            next = (next << 8) + (b[i] & 0xff);

        return next >>> (numBytes * 8 - numBits);
    }

    public void nextBytes(byte[] bytes) {
        engine.engineNextBytes(bytes);
    }

    public void setSeed(byte[] seed) {
        engine.engineSetSeed(seed);
    }

    public void setSeed(long seed) {
        if(seed == 0) {
            // The Random constructor will call us with zero seed, ignore it
            return;
        }
        byte[] seedBytes = new byte[8];
        for (int i = 0; i < 8; i++) {
            seedBytes[i] = (byte)(seed & 0xff);
            seed >>= 8;
        }
        setSeed(seedBytes);
    }

}
