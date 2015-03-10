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

public abstract class MessageDigest extends MessageDigestSpi {

    protected String           algorithm;
    protected Provider         provider;

    protected MessageDigest(String algorithm) {
        this.algorithm = algorithm;
    }

    public static MessageDigest getInstance(String algorithm)
    throws NoSuchAlgorithmException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("MessageDigest",
                                                        algorithm);
            return getInstance(algorithm, provider);
        } catch (NoSuchProviderException e) {
            throw new Error("Error in MessageDigest: " + e);
        }
    }

    public static MessageDigest getInstance(String algorithm,
                                            String provider)
    throws NoSuchAlgorithmException, NoSuchProviderException {
        ProviderLookup pl = ProviderLookup.getImplementation("MessageDigest",
                            algorithm,
                            provider);
        MessageDigest md = (MessageDigest) pl.getImpl();
        md.provider      = pl.getProvider();
        md.engineReset();
        return md;
    }

    public byte[] digest() {
        return engineDigest();
    }

    public byte[] digest(byte[] input) {
        engineUpdate(input, 0, input.length);
        return engineDigest();
    }

    public int digest(byte[] buf, int offset, int len) throws DigestException {
        return engineDigest(buf, offset, len);
    }

    public void reset() {
        engineReset();
    }

    public void update(byte input) {
        engineUpdate(input);
    }

    public void update(byte[] input) {
        engineUpdate(input, 0, input.length);
    }

    public void update(byte[] input, int offset, int len) {
        engineUpdate(input, offset, len);
    }

    public final Provider getProvider() {
        return provider;
    }

    public final String getAlgorithm() {
        return algorithm;
    }

    public final int getDigestLength() {
        return engineGetDigestLength();
    }

    public static boolean isEqual(byte[] digesta, byte[] digestb) {
        if(digesta.length != digestb.length)
            return false;

        for(int i = 0; i < digesta.length; i++) {
            if(digesta[i] != digestb[i]) {
                return false;
            }
        }
        return true;
    }

    public String toString() {
        return provider.getName() + " (" + provider.getVersion() +
               ") MessageDigest." + algorithm;
    }

    public Object clone() throws CloneNotSupportedException {
        if(this instanceof Cloneable) {
            return super.clone();
        } else {
            throw new CloneNotSupportedException();
        }
    }

}
