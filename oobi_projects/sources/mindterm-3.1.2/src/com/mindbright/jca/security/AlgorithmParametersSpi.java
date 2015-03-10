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

import java.io.IOException;

import com.mindbright.jca.security.spec.AlgorithmParameterSpec;
import com.mindbright.jca.security.spec.InvalidParameterSpecException;

// !!! TODO

public abstract class AlgorithmParametersSpi {

    public AlgorithmParametersSpi() {}

    protected abstract byte[] engineGetEncoded() throws IOException;

    protected abstract byte[] engineGetEncoded(String format)
    throws IOException;

    protected abstract AlgorithmParameterSpec
    engineGetParameterSpec(Class paramSpec)
    throws InvalidParameterSpecException;

    protected abstract void engineInit(AlgorithmParameterSpec paramSpec)
    throws InvalidParameterSpecException;

    protected abstract void engineInit(byte[] params) throws IOException;

    protected abstract void engineInit(byte[] params, String format)
    throws IOException;

    protected abstract String engineToString();

}
