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

package com.mindbright.jca.security.cert;

import java.io.ObjectStreamException;

import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.jca.security.NoSuchProviderException;
import com.mindbright.jca.security.PublicKey;
import com.mindbright.jca.security.SignatureException;

public abstract class Certificate {

    protected Certificate(String type) {}

    public final String getType() {
        return null;
    }

    public boolean equals(Object other) {
        return false;
    }

    public int hashCode() {
        return 0;
    }

    public abstract byte[] getEncoded() throws CertificateEncodingException;

    public abstract void verify(PublicKey key)
    throws CertificateException, NoSuchAlgorithmException,
                InvalidKeyException, NoSuchProviderException, SignatureException;

    public abstract void verify(PublicKey key, String sigProvider)
    throws CertificateException, NoSuchAlgorithmException,
                InvalidKeyException, NoSuchProviderException, SignatureException;

    public abstract String toString();

    public abstract PublicKey getPublicKey();

protected Object writeReplace() throws ObjectStreamException {
        return null;
    }

}
