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

import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.spec.DSAParameterSpec;

public final class DSAAlgorithm {

    private final static BigInteger one = BigInteger.valueOf(1L);

    public static byte[] sign(BigInteger x,
                              BigInteger p, BigInteger q, BigInteger g,
                              byte[] data) {
        BigInteger hM = new BigInteger(1, data);

        hM = hM.mod(q);

        BigInteger r = g.modPow(x, p).mod(q);
        BigInteger s = x.modInverse(q).multiply(hM.add(x.multiply(r))).mod(q);

        int dataSz = data.length;
        byte[] signature = new byte[dataSz * 2];
        byte[] tmp;

        tmp = unsignedBigIntToBytes(r, dataSz);
        System.arraycopy(tmp, 0, signature, 0, dataSz);

        tmp = unsignedBigIntToBytes(s, dataSz);
        System.arraycopy(tmp, 0, signature, dataSz, dataSz);

        return signature;
    }

    public static boolean verify(BigInteger y,
                                 BigInteger p, BigInteger q, BigInteger g,
                                 byte[] signature, byte[] data) {
        int        dataSz = signature.length / 2;
        byte[]     ra     = new byte[dataSz];
        byte[]     sa     = new byte[dataSz];

        System.arraycopy(signature, 0, ra, 0, dataSz);
        System.arraycopy(signature, dataSz, sa, 0, dataSz);

        BigInteger hM = new BigInteger(1, data);
        BigInteger r  = new BigInteger(1, ra);
        BigInteger s  = new BigInteger(1, sa);

        hM = hM.mod(q);

        BigInteger w  = s.modInverse(q);
        BigInteger u1 = hM.multiply(w).mod(q);
        BigInteger u2 = r.multiply(w).mod(q);
        BigInteger v  = g.modPow(u1, p).multiply(y.modPow(u2, p)).mod(p).mod(q);

        return (v.compareTo(r) == 0);
    }

    private static byte[] unsignedBigIntToBytes(BigInteger bi, int size) {
        byte[] tmp  = bi.toByteArray();
        byte[] tmp2 = null;
        if(tmp.length > size) {
            tmp2 = new byte[size];
            System.arraycopy(tmp, tmp.length - size, tmp2, 0, size);
        } else if(tmp.length < size) {
            tmp2 = new byte[size];
            System.arraycopy(tmp, 0, tmp2, size - tmp.length, tmp.length);
        } else {
            tmp2 = tmp;
        }
        return tmp2;
    }

    public static DSAParameterSpec
    generateParams(int pBits, int qBits, SecureRandom random) {
        BigInteger[] pq;
        BigInteger   g;

        pq = com.mindbright.util.Math.findRandomStrongPrime(pBits, qBits, random);
        g  = com.mindbright.util.Math.findRandomGenerator(pq[1], pq[0], random);

        return new DSAParameterSpec(pq[0], pq[1], g);
    }

    public static BigInteger generatePrivateKey(BigInteger q,
            SecureRandom random) {
        BigInteger x;
        do {
            x = new BigInteger(q.bitLength(), random);
        } while((x.compareTo(one) < 0) || (x.compareTo(q) > 0));
        return x;
    }

    public static BigInteger generatePublicKey(BigInteger g, BigInteger p,
            BigInteger x) {
        return g.modPow(x, p);
    }

}
