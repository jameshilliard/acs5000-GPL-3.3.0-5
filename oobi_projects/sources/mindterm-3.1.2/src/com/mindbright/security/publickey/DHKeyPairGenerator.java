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
import com.mindbright.jca.security.KeyPair;
import com.mindbright.jca.security.KeyPairGenerator;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;
import com.mindbright.jce.crypto.spec.DHParameterSpec;

public class DHKeyPairGenerator extends KeyPairGenerator {

    protected DHParameterSpec params;
    protected SecureRandom    random;

    public DHKeyPairGenerator() {
        super("DH");
    }

    public void initialize(int keysize, SecureRandom random) {
        throw new Error("Not implemented: " +
                        "'DHKeyPairGenerator.initialize(int, SecureRandom)'");
    }

    public void initialize(AlgorithmParameterSpec params, SecureRandom random)
    throws InvalidAlgorithmParameterException {
        if(!(params instanceof DHParameterSpec)) {
            throw new InvalidAlgorithmParameterException("Invalid params: " +
                    params);
        }
        this.params = (DHParameterSpec)params;
        this.random = random;
    }

    public KeyPair generateKeyPair() {
        BigInteger g = params.getG();
        BigInteger p = params.getP();
        int        l = params.getL();

        if(l == 0) {
            l = p.bitLength();
        }

        if(random == null) {
            random = new SecureRandom();
        }

        BigInteger x = DiffieHellman.generateX(p, l, random);
        BigInteger y = DiffieHellman.generateY(x, g, p);

        return new KeyPair(new DHPublicKey(y, p, g), new DHPrivateKey(x, p, g));
    }

}
