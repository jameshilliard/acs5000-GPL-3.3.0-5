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

import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.security.cipher.DES;

public final class SSHDES extends SSHCipher {
    DES    des;
    byte[] IV;
    byte[] enc;
    byte[] dec;

    public SSHDES() {
        des = new DES();
        IV  = new byte[8];
        enc = new byte[8];
        dec = new byte[8];
    }

    public void setKey(byte[] key) {
        try {
            des.initializeKey(key);
        } catch (InvalidKeyException e) {
            throw new Error("Internal error, invalid key in SSHDES");
        }
    }

    public synchronized void encrypt(byte[] src, int srcOff, byte[] dest, int destOff, int len) {
        int end = srcOff + len;
        int i;

        for(int si = srcOff, di = destOff; si < end; si += 8, di += 8) {
            for(i = 0; i < 8; i++) {
                IV[i] ^= src[si + i];
            }
            des.blockEncrypt(IV, 0, IV, 0);
            for(i = 0; i < 8; i++) {
                dest[di + i] = IV[i];
            }
        }
    }

    public synchronized void decrypt(byte[] src, int srcOff, byte[] dest, int destOff, int len) {
        int    end = srcOff + len;
        int    i;

        for(int si = srcOff, di = destOff; si < end; si += 8, di += 8) {
            for(i = 0; i < 8; i++) {
                enc[i] = src[si + i];
            }
            des.blockDecrypt(enc, 0, dec, 0);
            for(i = 0; i < 8; i++) {
                dest[di + i] = (byte)((IV[i] ^ dec[i]) & 0xff);
                IV[i] = enc[i];
            }
        }
    }

}

