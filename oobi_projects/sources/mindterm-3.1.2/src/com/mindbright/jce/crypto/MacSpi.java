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
import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public abstract class MacSpi {

    public MacSpi() {}

    public Object clone() throws CloneNotSupportedException {
        if(this instanceof Cloneable) {
            return super.clone();
        } else {
            throw new CloneNotSupportedException();
        }
    }

    protected abstract byte[] engineDoFinal();

    protected abstract int engineGetMacLength();

    protected abstract void engineInit(Key key, AlgorithmParameterSpec params)
    throws InvalidKeyException, InvalidAlgorithmParameterException;

    protected abstract void engineReset();

    protected abstract void engineUpdate(byte input);

    protected abstract void engineUpdate(byte[] input, int offset, int len);

}
