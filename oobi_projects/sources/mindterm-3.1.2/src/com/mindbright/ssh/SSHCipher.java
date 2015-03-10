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

import com.mindbright.jca.security.MessageDigest;

public abstract class SSHCipher {

    public static SSHCipher getInstance(String algorithm) {
        Class c;
        try {
            c = Class.forName("com.mindbright.ssh." + algorithm);
            return (SSHCipher)c.newInstance();
        } catch(Throwable t) {
            return null;
        }
    }

    public abstract void encrypt(byte[] src, int srcOff,
                                 byte[] dest, int destOff, int len);
    public abstract void decrypt(byte[] src, int srcOff,
                                 byte[] dest, int destOff, int len);
    public abstract void setKey(byte[] key);

    public byte[] encrypt(byte[] src) {
        byte[] dest = new byte[src.length];
        encrypt(src, 0, dest, 0, src.length);
        return dest;
    }

    public byte[] decrypt(byte[] src) {
        byte[] dest = new byte[src.length];
        decrypt(src, 0, dest, 0, src.length);
        return dest;
    }

    public void setKey(String key) {
        MessageDigest md5;
        byte[] mdKey = new byte[32];
        try {
            md5 = MessageDigest.getInstance("MD5");
            md5.update(key.getBytes());
            byte[] digest = md5.digest();
            System.arraycopy(digest, 0, mdKey, 0, 16);
            System.arraycopy(digest, 0, mdKey, 16, 16);
        } catch(Exception e) {
            // !!!
            System.out.println("MD5 not implemented, can't generate key out of string!");
            System.exit(1);
        }
        setKey(mdKey);
    }

}
