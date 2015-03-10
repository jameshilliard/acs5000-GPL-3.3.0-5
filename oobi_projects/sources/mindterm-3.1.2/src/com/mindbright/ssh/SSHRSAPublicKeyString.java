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

package com.mindbright.ssh;

import java.io.FileOutputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;

import com.mindbright.security.publickey.RSAPublicKey;

public class SSHRSAPublicKeyString extends RSAPublicKey {

    String user;
    String opts;

    public SSHRSAPublicKeyString(String opts, String user, BigInteger e, BigInteger n) {
        super(n, e);
        this.opts = opts;
        this.user = user;
    }

    public static SSHRSAPublicKeyString createKey(String opts, String pubKey) throws NoSuchElementException {
        StringTokenizer tok  = new StringTokenizer(pubKey);
        String          user = null;
        String bits;
        String e;
        String n;

        bits = tok.nextToken();
        e    = tok.nextToken();
        n    = tok.nextToken();
        if(tok.hasMoreElements())
            user = tok.nextToken();

        return new SSHRSAPublicKeyString(opts, user, new BigInteger(e), new BigInteger(n));
    }

    public String getOpts() {
        return opts;
    }

    public String getUser() {
        return user;
    }

    public String toString() {
        int bitLen = getModulus().bitLength();
        return ((opts != null ? (opts + " ") : "") +
                bitLen + " " + getPublicExponent() + " " + getModulus() + " " +
                (user != null ? user : ""));
    }

    public void toFile(String fileName) throws IOException {
        FileOutputStream    fileOut = new FileOutputStream(fileName);
        SSHDataOutputStream dataOut = new SSHDataOutputStream(fileOut);
        dataOut.writeBytes(toString());
        dataOut.close();
    }

}
