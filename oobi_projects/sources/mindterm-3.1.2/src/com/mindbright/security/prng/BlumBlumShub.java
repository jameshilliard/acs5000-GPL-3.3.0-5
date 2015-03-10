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

package com.mindbright.security.prng;

import java.math.BigInteger;
import java.util.Random;

import com.mindbright.jca.security.MessageDigest;
import com.mindbright.jca.security.SecureRandomSpi;
import com.mindbright.util.RandomSeed;

/**
 * An implementation of the Blum Blum Shub pseudo random number
 * generator. This PRNGD is slow but is believed to be very strong.
 */
public final class BlumBlumShub extends SecureRandomSpi {

    private static final BigInteger one = BigInteger.valueOf(1);
    private static final BigInteger two = BigInteger.valueOf(2);

    private BigInteger n;
    private BigInteger xi;
    private int        bitsTotal;
    private int        bitsLeft;
    private int        entropyBits;

    /**
     * Precalculated big modulus (using the getBlumInteger method with
     * bits = 1024 and using built in SecureRandom in jdk 1.1.8 from IBM)
     */
    public final static byte[] N = {
        (byte)0x78, (byte)0xd0, (byte)0xc8, (byte)0xac, (byte)0xe2, (byte)0x35,
        (byte)0x77, (byte)0x31, (byte)0x3d, (byte)0x33, (byte)0x25, (byte)0x58,
        (byte)0x49, (byte)0x1c, (byte)0x0c, (byte)0x88, (byte)0xbe, (byte)0xf2,
        (byte)0xc2, (byte)0xf0, (byte)0x26, (byte)0x14, (byte)0x20, (byte)0x02,
        (byte)0x0a, (byte)0x75, (byte)0xdf, (byte)0xd4, (byte)0x11, (byte)0x39,
        (byte)0xbf, (byte)0x64, (byte)0x72, (byte)0x8d, (byte)0x85, (byte)0xdd,
        (byte)0xe0, (byte)0xb6, (byte)0x7d, (byte)0x78, (byte)0x24, (byte)0xb6,
        (byte)0x49, (byte)0x0b, (byte)0x23, (byte)0x34, (byte)0x94, (byte)0x8b,
        (byte)0x06, (byte)0x9f, (byte)0x6f, (byte)0x5a, (byte)0x46, (byte)0xfb,
        (byte)0x52, (byte)0x76, (byte)0x81, (byte)0x05, (byte)0x34, (byte)0x23,
        (byte)0xd5, (byte)0xfd, (byte)0xe9, (byte)0x32,	(byte)0xc0, (byte)0xb0,
        (byte)0x7f, (byte)0x16, (byte)0x7e, (byte)0xd3, (byte)0xc1, (byte)0x91,
        (byte)0xda, (byte)0x5e, (byte)0xd1, (byte)0xb2, (byte)0x71, (byte)0x13,
        (byte)0x4e, (byte)0x4e, (byte)0x59, (byte)0x32, (byte)0x30, (byte)0x59,
        (byte)0xba, (byte)0x5b, (byte)0x34, (byte)0xd3, (byte)0x28, (byte)0xfa,
        (byte)0x9c, (byte)0x03, (byte)0xde, (byte)0x96, (byte)0xbf, (byte)0xea,
        (byte)0x84, (byte)0x25, (byte)0x63, (byte)0x7e, (byte)0x41, (byte)0xde,
        (byte)0x29, (byte)0xf0, (byte)0xd0, (byte)0x7c, (byte)0xbe, (byte)0x45,
        (byte)0xe4, (byte)0x0f, (byte)0x77, (byte)0x5e, (byte)0xc8, (byte)0xf6,
        (byte)0xdd, (byte)0x5d, (byte)0x83, (byte)0xc6, (byte)0x5c, (byte)0xe7,
        (byte)0x7f, (byte)0x7e, (byte)0xd7, (byte)0x74, (byte)0x2a, (byte)0xc5,
        (byte)0xab, (byte)0xb5
    };

    /**
     * Creates an instance with a custom modulus (M), do NOT use this
     * unless you know exactly what you are doing. The value of the
     * modulus is very important for the security of the algorithm and
     * there are some specific requirements on how to generate it. Do
     * not use this constructor unless you know exactly what you are
     * doing. 
     *
     * @param n modulus M to use in algorithm.
     * @param seed random bytes used to seed the generator
     */
    public BlumBlumShub(BigInteger n, byte[] seed) {
        this.n = n;
        if(seed != null) {
            engineSetSeed(seed);
        }
    }

    /**
     * Creates an instance which uses the built in modulus N
     *
     * @param seed random bytes used to seed the generator
     */
    public BlumBlumShub(byte[] seed) {
        this(new BigInteger(N), seed);
    }

    /**
     * Creates an unseeded instance which uses the built in modulus N
     */
    public BlumBlumShub() {
        this(null);
    }

    private int log2(int n) {
        int ln = 0;
        while(n != 0) {
            n >>>= 1;
            ln += 1;
        }
        return ln;
    }

    private void nextXi() {
        if(xi == null) {
            RandomSeed seed = new RandomSeed();
            engineSetSeed(seed.getBytes(30));
        }
        xi         = xi.modPow(two, n);
        bitsLeft   = bitsTotal;
        byte[] xib = xi.toByteArray();
        entropyBits = 0;
        for(int i = 0; i < bitsLeft; i += 8) {
            entropyBits <<= i;
            entropyBits |= (xib[(xib.length - 4) + i] & 0xff);
        }
        entropyBits <<= (32 - bitsTotal);
        entropyBits >>>= (32 - bitsTotal);
    }

    private synchronized int nextBits(int bits) {
        int v = 0;
        while(bits >= bitsLeft) {
            v <<= bitsLeft;
            v |= entropyBits;
            bits -= bitsLeft;
            nextXi();
        }
        if(bits > 0) {
            int e = entropyBits;
            v <<= bits;
            e <<= (32 - bits);
            e >>>= (32 - bits);
            v |= e;
            entropyBits >>>= bits;
            bitsLeft -= bits;
        }
        return v;
    }

    private static BigInteger getBlumInteger(int bits, Random rand) {
        BigInteger a;
        BigInteger[] p = new BigInteger[2];
        BigInteger four  = BigInteger.valueOf(4);
        BigInteger three = BigInteger.valueOf(3);
        int found = 0;
        do {
            a = new BigInteger(bits / 2, 128, rand);
            if((a.mod(four)).equals(three)) {
                p[found++] = a;
            }
        } while(found < 2);
        return p[0].multiply(p[1]);
    }

    protected byte[] engineGenerateSeed(int numBytes) {
        byte[] seed = new byte[numBytes];
        engineNextBytes(seed);
        return seed;
    }

    protected void engineNextBytes(byte[] bytes) {
        for(int i = 0; i < bytes.length; i++) {
            int v = nextBits(8);
            bytes[i] = (byte)(v & 0xff);
        }
    }

    protected void engineSetSeed(byte[] seed) {
        try {
            MessageDigest sha1 = MessageDigest.getInstance("SHA1");
            if(xi == null) {
                int seedBits = 8 * seed.length;
                if(seedBits > 160)
                    seedBits = 160;
                bitsTotal = log2(seedBits);
            } else {
                sha1.update(xi.toByteArray());
            }
            sha1.update(seed);
            xi = new BigInteger(1, sha1.digest());
            while(!(xi.gcd(n)).equals(one)) {
                xi = xi.add(one);
            }
            nextXi();
        } catch (Exception e) {
            throw new Error("Error in BlumBlumShub engine: " + e);
        }
    }

    /* !!! DEBUG
    public static void main(String[] argv) {
    BigInteger n = getBlumInteger(1024, new java.security.SecureRandom());
    System.out.println("Example of Blum integer: ");
    com.mindbright.util.HexDump.print(n);

    BlumBlumShub bbs =
     new BlumBlumShub(n, "hejhopp!".getBytes());
    int[] freq = new int[256];
    for(int i = 0; i < 100000; i++) {
     int v = bbs.next(8);
     freq[v] += 1;
    }
    for(int i = 0; i < 256; i++) {
     System.out.print(freq[i] + ", ");
    }
    }
    */

}
