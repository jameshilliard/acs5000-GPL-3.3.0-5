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

import java.io.Serializable;

/**
 * Interface for random number generators
 */
public abstract class SecureRandomSpi implements Serializable {

    public SecureRandomSpi() {}

    /**
     * Generate a number of random seed bytes.
     *
     * @param numBytes how many bytes to generate
     *
     * @return an array filled with the requested number of random bytes
     */
    protected abstract byte[] engineGenerateSeed(int numBytes);

    /**
     * Generate random bytes
     *
     * @param bytes array which should be filled with random bytes
     */
    protected abstract void engineNextBytes(byte[] bytes);

    /**
     * Set the seed used by the random number generator.
     *
     * @param seed an array of random bytes
     */
    protected abstract void engineSetSeed(byte[] seed);

}
