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

public abstract class KeyStoreSpi {

    public abstract Key engineGetKey(String alias, char[] password)
    throws NoSuchAlgorithmException, UnrecoverableKeyException;

    public abstract Certificate[] engineGetCertificateChain(String alias);

    public abstract Certificate engineGetCertificate(String alias);

    public abstract Date engineGetCreationDate(String alias);

    public abstract void engineSetKeyEntry(String alias, Key key,
                                           char[] password, Certificate[] chain)
    throws KeyStoreException;

    public abstract void engineSetKeyEntry(String alias, byte[] key,
                                           Certificate[] chain)
    throws KeyStoreException;

    public abstract void engineSetCertificateEntry(String alias,
            Certificate cert)
    throws KeyStoreException;

    public abstract void engineDeleteEntry(String alias)
    throws KeyStoreException;

    public abstract Enumeration engineAliases();

    public abstract boolean engineContainsAlias(String alias);

    public abstract int engineSize();

    public abstract boolean engineIsKeyEntry(String alias);

    public abstract boolean engineIsCertificateEntry(String alias);

    public abstract String engineGetCertificateAlias(Certificate cert);

    public abstract void engineStore(OutputStream stream, char[] password)
    throws IOException, NoSuchAlgorithmException, CertificateException;

    public abstract void engineLoad(InputStream stream, char[] password)
    throws IOException, NoSuchAlgorithmException, CertificateException;

}
