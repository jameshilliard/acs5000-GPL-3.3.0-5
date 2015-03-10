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
import com.mindbright.jca.security.spec.DSAPrivateKeySpec;
import com.mindbright.jca.security.spec.DSAPublicKeySpec;
import com.mindbright.jca.security.spec.InvalidKeySpecException;
import com.mindbright.jca.security.spec.KeySpec;

public class DSAKeyFactory extends KeyFactorySpi {

    protected PublicKey engineGeneratePublic(KeySpec keySpec)
    throws InvalidKeySpecException {
        if(!(keySpec instanceof DSAPublicKeySpec)) {
            throw new InvalidKeySpecException("KeySpec " + keySpec +
                                              ", not supported");
        }
        DSAPublicKeySpec dsaPub = (DSAPublicKeySpec)keySpec;
        return new DSAPublicKey(dsaPub.getY(),
                                dsaPub.getP(), dsaPub.getQ(), dsaPub.getG());
    }

    protected PrivateKey engineGeneratePrivate(KeySpec keySpec)
    throws InvalidKeySpecException {
        if(!(keySpec instanceof DSAPrivateKeySpec)) {
            throw new InvalidKeySpecException("KeySpec " + keySpec +
                                              ", not supported");
        }
        DSAPrivateKeySpec dsaPrv = (DSAPrivateKeySpec)keySpec;
        return new DSAPrivateKey(dsaPrv.getX(),
                                 dsaPrv.getP(), dsaPrv.getQ(), dsaPrv.getG());
    }

    protected KeySpec engineGetKeySpec(Key key, Class keySpec)
    throws InvalidKeySpecException {
        // !!! TODO
        return null;
    }

    protected Key engineTranslateKey(Key key)
    throws InvalidKeyException {
        // !!! TODO
        return null;
    }

}
