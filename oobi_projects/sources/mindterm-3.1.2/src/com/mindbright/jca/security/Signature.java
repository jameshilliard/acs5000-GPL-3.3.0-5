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

import com.mindbright.jca.security.spec.AlgorithmParameterSpec;

public class Signature extends SignatureSpi {

    protected static final int UNINITIALIZED = 0;
    protected static final int SIGN          = 1;
    protected static final int VERIFY        = 2;

    private final String[] states = {
                                        "<uninitialized>", "<sign>", "<verify>"
                                    };

    protected int state;

    private SignatureSpi signatureSpi;
    private Provider     provider;
    private String       algorithm;

    protected Signature(String algorithm) {
        this.algorithm = algorithm;
    }

    public static Signature getInstance(String algorithm)
    throws NoSuchAlgorithmException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("Signature", algorithm);
            return getInstance(algorithm, provider);
        } catch (NoSuchProviderException e) {
            throw new Error("Error in Signature: " + e);
        }
    }

    public static Signature getInstance(String algorithm,
                                        String provider)
    throws NoSuchAlgorithmException, NoSuchProviderException {
        ProviderLookup pl = ProviderLookup.getImplementation("Signature",
                            algorithm,
                            provider);

        SignatureSpi signatureSpi = (SignatureSpi)pl.getImpl();
        Signature    signature    = new Signature(algorithm);

        signature.provider     = pl.getProvider();
        signature.signatureSpi = signatureSpi;

        return signature;
    }

    public final Provider getProvider() {
        return provider;
    }

    public void initVerify(PublicKey publicKey) throws InvalidKeyException {
        signatureSpi.engineInitVerify(publicKey);
        state = VERIFY;
    }

    /*
    public final void initVerify(Certificate certificate)
    throws InvalidKeyException
    {
    }
    */

    public void initSign(PrivateKey privateKey) throws InvalidKeyException {
        signatureSpi.engineInitSign(privateKey);
        state = SIGN;
    }

    public void initSign(PrivateKey privateKey, SecureRandom random)
    throws InvalidKeyException {
        signatureSpi.engineInitSign(privateKey, random);
        state = SIGN;
    }

    public byte[] sign() throws SignatureException {
        if(state != SIGN) {
            throw new SignatureException("Signature not initialized for " +
                                         "signing");
        }
        return signatureSpi.engineSign();
    }

    public int sign(byte[] outbuf, int offset, int len)
    throws SignatureException {
        if(state != SIGN) {
            throw new SignatureException("Signature not initialized for " +
                                         "signing");
        }
        return signatureSpi.engineSign(outbuf, offset, len);
    }

    public boolean verify(byte[] signature) throws SignatureException {
        if(state != VERIFY) {
            throw new SignatureException("Signature not initialized for " +
                                         "verification");
        }
        return signatureSpi.engineVerify(signature);
    }

    public void update(byte b) throws SignatureException {
        if (state == UNINITIALIZED) {
            throw new SignatureException("Signature not initialized");
        }
        signatureSpi.engineUpdate(b);
    }

    public final void update(byte[] data) throws SignatureException {
        update(data, 0, data.length);
    }

    public void update(byte[] data, int off, int len)
    throws SignatureException {
        if(state == UNINITIALIZED) {
            throw new SignatureException("Signature not initialized");
        }
        signatureSpi.engineUpdate(data, off, len);
    }

    public final String getAlgorithm() {
        return algorithm;
    }

    public final String toString() {
        return "Signature " + getAlgorithm() + " " + states[state] +
               " (" + provider.getName() + ")";
    }

    public void setParameter(String param, Object value)
    throws InvalidParameterException {
        signatureSpi.engineSetParameter(param, value);
    }

    public void setParameter(AlgorithmParameterSpec params)
    throws InvalidAlgorithmParameterException {
        signatureSpi.engineSetParameter(params);
    }

    public Object getParameter(String param)
    throws InvalidParameterException {
        return signatureSpi.engineGetParameter(param);
    }

    public Object clone() throws CloneNotSupportedException {
        throw new CloneNotSupportedException();
    }

    protected void engineInitVerify(PublicKey publicKey)
    throws InvalidKeyException {
        notImplemented();
    }

    protected void engineInitSign(PrivateKey privateKey)
    throws InvalidKeyException {
        notImplemented();
    }

    protected void engineInitSign(PrivateKey privateKey,
                                  SecureRandom random)
    throws InvalidKeyException {
        notImplemented();
    }

    protected void engineUpdate(byte b)
    throws SignatureException {
        notImplemented();
    }

    protected void engineUpdate(byte[] b, int off, int len)
    throws SignatureException {
        notImplemented();
    }

    protected byte[] engineSign() throws SignatureException {
        notImplemented();
        return null;
    }

    protected boolean engineVerify(byte[] sigBytes) throws SignatureException {
        notImplemented();
        return false;
    }

    protected void engineSetParameter(String param, Object value)
    throws InvalidParameterException {
        notImplemented();
    }

    protected Object engineGetParameter(String param)
    throws InvalidParameterException {
        notImplemented();
        return null;
    }

    private final void notImplemented() {
        throw
        new Error("Abstract method in SignatureSpi implementation missing");
    }

}
