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
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Date;
import java.util.Enumeration;

import com.mindbright.jca.security.cert.Certificate;
import com.mindbright.jca.security.cert.CertificateException;

public class KeyStore {

    private KeyStoreSpi keyStoreSpi;
    private Provider    provider;
    private String      type;

    protected KeyStore(KeyStoreSpi keyStoreSpi, Provider provider, String type) {
        this.keyStoreSpi = keyStoreSpi;
        this.provider    = provider;
        this.type        = type;
    }

    public static KeyStore getInstance(String type) throws KeyStoreException {
        try {
            String provider =
                ProviderLookup.findImplementingProvider("KeyStore", type);
            return getInstance(type, provider);
        } catch(Exception e) {
            throw new KeyStoreException("KeyStore " + type + " not found");
        }
    }

    public static KeyStore getInstance(String type, String provider)
    throws KeyStoreException, NoSuchProviderException {
        try {
            ProviderLookup pl = ProviderLookup.getImplementation("KeyStore",
                                type,
                                provider);
            return new KeyStore((KeyStoreSpi)pl.getImpl(), pl.getProvider(),
                                type);
        } catch(NoSuchAlgorithmException e) {
            throw new KeyStoreException("KeyStore " + type +
                                        " not found in provider " + provider);
        }
    }

    public final Provider getProvider() {
        return provider;
    }

    public final String getType() {
        return type;
    }

    public final Key getKey(String alias, char[] password)
    throws KeyStoreException, NoSuchAlgorithmException,
        UnrecoverableKeyException {
        return keyStoreSpi.engineGetKey(alias, password);
    }

    public final Certificate[] getCertificateChain(String alias)
    throws KeyStoreException {
        return keyStoreSpi.engineGetCertificateChain(alias);
    }

    public final Certificate getCertificate(String alias)
    throws KeyStoreException {
        return keyStoreSpi.engineGetCertificate(alias);
    }

    public final Date getCreationDate(String alias) throws KeyStoreException {
        return keyStoreSpi.engineGetCreationDate(alias);
    }

    public final void setKeyEntry(String alias, Key key,
                                  char[] password, Certificate[] chain)
    throws KeyStoreException {
        keyStoreSpi.engineSetKeyEntry(alias, key, password, chain);
    }

    public final void setKeyEntry(String alias, byte[] key, Certificate[] chain)
    throws KeyStoreException {
        keyStoreSpi.engineSetKeyEntry(alias, key, chain);
    }

    public final void setCertificateEntry(String alias, Certificate cert)
    throws KeyStoreException {
        keyStoreSpi.engineSetCertificateEntry(alias, cert);
    }

    public final void deleteEntry(String alias) throws KeyStoreException {
        keyStoreSpi.engineDeleteEntry(alias);
    }

    public final Enumeration aliases() throws KeyStoreException {
        return keyStoreSpi.engineAliases();
    }

    public final boolean containsAlias(String alias) throws KeyStoreException {
        return keyStoreSpi.engineContainsAlias(alias);
    }

    public final int size() throws KeyStoreException {
        return keyStoreSpi.engineSize();
    }

    public final boolean isKeyEntry(String alias) throws KeyStoreException {
        return keyStoreSpi.engineIsKeyEntry(alias);
    }

    public final boolean isCertificateEntry(String alias)
    throws KeyStoreException {
        return keyStoreSpi.engineIsCertificateEntry(alias);
    }

    public final String getCertificateAlias(Certificate cert)
    throws KeyStoreException {
        return keyStoreSpi.engineGetCertificateAlias(cert);
    }

    public final void store(OutputStream stream, char[] password)
    throws KeyStoreException, IOException, NoSuchAlgorithmException,
        CertificateException {
        keyStoreSpi.engineStore(stream, password);
    }

    public final void load(InputStream stream, char[] password)
    throws IOException, NoSuchAlgorithmException, CertificateException {
        keyStoreSpi.engineLoad(stream, password);
    }

    public static final String getDefaultType() {
        return "PKCS12";
    }

}
