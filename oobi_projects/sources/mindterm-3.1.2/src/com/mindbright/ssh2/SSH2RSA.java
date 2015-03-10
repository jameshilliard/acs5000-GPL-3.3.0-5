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

package com.mindbright.ssh2;

import java.math.BigInteger;

import com.mindbright.jca.security.KeyFactory;
import com.mindbright.jca.security.PublicKey;
import com.mindbright.jca.security.interfaces.RSAPublicKey;
import com.mindbright.jca.security.spec.RSAPublicKeySpec;

/**
 * Implements "ssh-rsa" signatures according to the ssh standard.
 */
public final class SSH2RSA extends SSH2SimpleSignature {
    public final static String SSH2_KEY_FORMAT = "ssh-rsa";

    public SSH2RSA() {
        super("SHA1withRSA", SSH2_KEY_FORMAT);
    }

    /**
     * Encode the given public key according to the ssh standard.
     *
     * @param publicKey The public key to encode. Must be an instance of
     *                  <code>RSAPublicKey</code>.
     *
     * @return A byte array containing the key suitably encoded.
     */
    public byte[] encodePublicKey(PublicKey publicKey) throws SSH2Exception {
        SSH2DataBuffer buf = new SSH2DataBuffer(8192);

        if(!(publicKey instanceof RSAPublicKey)) {
            throw new SSH2FatalException("SSH2RSA, invalid public key type: " +
                                         publicKey);
        }

        RSAPublicKey rsaPubKey = (RSAPublicKey)publicKey;

        buf.writeString(SSH2_KEY_FORMAT);
        buf.writeBigInt(rsaPubKey.getPublicExponent());
        buf.writeBigInt(rsaPubKey.getModulus());

        return buf.readRestRaw();
    }

    /**
     * Decode a public key encoded according to the ssh standard.
     *
     * @param pubKeyBlob A byte array containing a public key blob.
     *
     * @return A <code>Publickey</code> instance.
     */
    public PublicKey decodePublicKey(byte[] pubKeyBlob) throws SSH2Exception {
        BigInteger e, n;
        SSH2DataBuffer buf = new SSH2DataBuffer(pubKeyBlob.length);

        buf.writeRaw(pubKeyBlob);

        String type = buf.readJavaString();
        if(!type.equals(SSH2_KEY_FORMAT)) {
            throw new SSH2FatalException("SSH2RSA, keyblob type mismatch, got '"
                                         + type + ", (execpted + '" +
                                         SSH2_KEY_FORMAT + "')");
        }

        e = buf.readBigInt();
        n = buf.readBigInt();

        try {
            KeyFactory       rsaKeyFact = KeyFactory.getInstance("RSA");
            RSAPublicKeySpec rsaPubSpec = new RSAPublicKeySpec(n, e);

            return rsaKeyFact.generatePublic(rsaPubSpec);

        } catch (Exception ee) {
            throw new SSH2FatalException("SSH2RSA, error decoding public key blob: " +
                                         ee);
        }
    }

}
