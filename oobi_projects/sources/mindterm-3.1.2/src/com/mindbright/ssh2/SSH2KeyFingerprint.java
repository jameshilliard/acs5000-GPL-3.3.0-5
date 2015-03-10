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

import com.mindbright.jca.security.MessageDigest;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.util.HexDump;

/**
 * Contains static methods for calculating fingerprints on keys.
 */
public final class SSH2KeyFingerprint {

    /**
     * Calculates the MD5 checksum of the given blob.
     *
     * @param blob Data to calculate checksum of.
     *
     * @return The checksum expressed as a hexadecimal string.
     */
    public static String md5Hex(byte[] blob) {
        MessageDigest md5 = null;
        try {
            md5 = MessageDigest.getInstance("MD5");
            md5.update(blob);
        } catch (NoSuchAlgorithmException e) {
            throw new Error("SSH2KeyFingerprint.md5Hex: " + e);
        }

        byte[] raw       = md5.digest();
        String hex       = HexDump.toString(raw);
        StringBuffer fps = new StringBuffer();
        for(int i = 0; i < hex.length(); i += 2) {
            fps.append(hex.substring(i, i + 2));
            if(i < hex.length() - 2) {
                fps.append(":");
            }
        }
        return fps.toString();
    }

    /**
     * Calculates the bubble-babble checksum of the given blob.
     *
     * @param blob Data to calculate checksum of.
     *
     * @return The bubble-babble checksum string.
     */
    public static String bubbleBabble(byte[] blob) {
        MessageDigest sha1 = null;
        try {
            sha1 = MessageDigest.getInstance("SHA1");
            sha1.update(blob);
        } catch (NoSuchAlgorithmException e) {
            throw new Error("SSH2KeyFingerprint.bubbleBabble: " + e);
        }

        byte[]       raw        = sha1.digest();
        StringBuffer retval     = new StringBuffer();
        char[]       consonants = { 'b', 'c', 'd', 'f', 'g', 'h', 'k', 'l', 'm',
                                    'n', 'p', 'r', 's', 't', 'v', 'z', 'x' };
        char[]       vowels     = { 'a', 'e', 'i', 'o', 'u', 'y' };
        int          rounds     = (raw.length / 2) + 1;
        int          seed       = 1;

        retval.append('x');

        for(int i = 0; i < rounds; i++) {
            int idx0, idx1, idx2, idx3, idx4;
            if((i + 1 < rounds) || ((raw.length % 2) != 0)) {
                idx0 = ((((((int)(raw[2 * i])) & 0xff) >>> 6) & 3) + seed) % 6;
                idx1 = ((((int)(raw[2 * i])) & 0xff) >>> 2) & 15;
                idx2 = (((((int)(raw[2 * i])) & 0xff) & 3) + (seed / 6)) % 6;
                retval.append(vowels[idx0]);
                retval.append(consonants[idx1]);
                retval.append(vowels[idx2]);
                if((i + 1) < rounds) {
                    idx3 = ((((int)(raw[(2 * i) + 1])) & 0xff) >>> 4) & 15;
                    idx4 = (((int)(raw[(2 * i) + 1])) & 0xff) & 15;
                    retval.append(consonants[idx3]);
                    retval.append('-');
                    retval.append(consonants[idx4]);
                    seed = ((seed * 5) +
                            (((((int)(raw[2 * i])) & 0xff) * 7) +
                             (((int)(raw[(2 * i) + 1])) & 0xff))) % 36;
                }
            } else {
                idx0 = seed % 6;
                idx1 = 16;
                idx2 = seed / 6;
                retval.append(vowels[idx0]);
                retval.append(consonants[idx1]);
                retval.append(vowels[idx2]);
            }
        }
        retval.append('x');

        return retval.toString();
    }
}
