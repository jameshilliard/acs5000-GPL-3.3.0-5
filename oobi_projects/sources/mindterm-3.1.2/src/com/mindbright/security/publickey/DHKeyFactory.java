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


import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.Key;
import com.mindbright.jca.security.KeyFactorySpi;
import com.mindbright.jca.security.PrivateKey;
import com.mindbright.jca.security.PublicKey;
import com.mindbright.jca.security.spec.InvalidKeySpecException;
import com.mindbright.jca.security.spec.KeySpec;
import com.mindbright.jce.crypto.spec.DHPrivateKeySpec;
import com.mindbright.jce.crypto.spec.DHPublicKeySpec;

public final class DHKeyFactory extends KeyFactorySpi {

    protected PublicKey engineGeneratePublic(KeySpec keySpec)
    throws InvalidKeySpecException {
        if(!(keySpec instanceof DHPublicKeySpec)) {
            throw new InvalidKeySpecException("KeySpec " + keySpec +
                                              ", not supported");
        }
        DHPublicKeySpec dhPub = (DHPublicKeySpec)keySpec;
        return new DHPublicKey(dhPub.getY(), dhPub.getP(), dhPub.getG());
    }

    protected PrivateKey engineGeneratePrivate(KeySpec keySpec)
    throws InvalidKeySpecException {
        if(!(keySpec instanceof DHPrivateKeySpec)) {
            throw new InvalidKeySpecException("KeySpec " + keySpec +
                                              ", not supported");
        }
        DHPrivateKeySpec dhPrv = (DHPrivateKeySpec)keySpec;
        return new DHPrivateKey(dhPrv.getX(), dhPrv.getP(), dhPrv.getG());
    }

    protected KeySpec engineGetKeySpec(Key key, Class keySpec)
    throws InvalidKeySpecException {
        // !!! TODO
        throw new Error("DHKeyFactory.engineGetKeySpec() not implemented");
    }

    protected Key engineTranslateKey(Key key) throws InvalidKeyException {
        // !!! TODO
        throw new Error("DHKeyFactory.engineTranslateKey() not implemented");
    }

}
