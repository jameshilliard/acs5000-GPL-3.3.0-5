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

package com.mindbright.jce.crypto.spec;

import java.math.BigInteger;

import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public class DHParameterSpec extends DHParamsImpl
    implements AlgorithmParameterSpec {
    protected int l;

    public DHParameterSpec(BigInteger p, BigInteger g) {
        this(p, g, 0);
    }

    public DHParameterSpec(BigInteger p, BigInteger g, int l) {
        super(p, g);
        this.l = l;
    }

    public int getL() {
        return l;
    }

}
