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
import com.mindbright.security.cipher.Blowfish;

public final class SSHBlowfish extends SSHCipher {
    Blowfish blowfish;
    byte[]   IV;
    byte[]   enc;
    byte[]   dec;

    public SSHBlowfish() {
        blowfish = new Blowfish();
        IV       = new byte[8];
        enc      = new byte[8];
        dec      = new byte[8];
    }

    public void setKey(String skey) {
        if(skey.length() == 0) {
        }
        else {
            byte[] key = skey.getBytes();
            setKey(key);
        }
    }

    public void setKey(byte[] key) {
        try {
            blowfish.initializeKey(key);
        } catch (InvalidKeyException e) {
            throw new Error("Internal error, invalid key in SSHBlowfish");
        }
    }

    public synchronized void encrypt(byte[] src, int srcOff, byte[] dest, int destOff, int len) {
        int end = srcOff + len;
        int i, j;

        for(int si = srcOff, di = destOff; si < end; si += 8, di += 8) {
            for(i = 0; i < 4; i++) {
                j = 3 - i;
                IV[i]     ^= src[si + j];
                IV[i + 4] ^= src[si + 4 + j];
            }
            blowfish.blockEncrypt(IV, 0, IV, 0);
            for(i = 0; i < 4; i++) {
                j = 3 - i;
                dest[di + i]     = IV[j];
                dest[di + i + 4] = IV[4 + j];
            }
        }
    }

    public synchronized void decrypt(byte[] src, int srcOff, byte[] dest, int destOff, int len) {
        int    end = srcOff + len;
        int    i, j;

        for(int si = srcOff, di = destOff; si < end; si += 8, di += 8) {
            for(i = 0; i < 4; i++) {
                j = (3 - i);
                enc[i]     = src[si + j];
                enc[i + 4] = src[si + 4 + j];
            }
            blowfish.blockDecrypt(enc, 0, dec, 0);
            for(i = 0; i < 4; i++) {
                j = 3 - i;
                dest[di + i] = (byte)((IV[j] ^ dec[j]) & 0xff);
                IV[j] = enc[j];
                dest[di + i + 4] = (byte)((IV[4 + j] ^ dec[4 + j]) & 0xff);
                IV[4 + j] = enc[4 + j];
            }
        }
    }

}
