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

package com.mindbright.security;

import com.mindbright.jca.security.Provider;

public class Mindbright extends Provider {

    public Mindbright() {
        super("Mindbright", 1.1, "Mindbright JCA/JCE provider v1.1");

        put("Cipher.DES",          "com.mindbright.security.cipher.DES");
        put("Cipher.DES/ECB",      "com.mindbright.security.cipher.DES");
        put("Cipher.DES/CBC",      "com.mindbright.security.cipher.DES");
        put("Cipher.DES/CFB",      "com.mindbright.security.cipher.DES");
        put("Cipher.DES/OFB",      "com.mindbright.security.cipher.DES");
        put("Cipher.DES/CBC/PKCS5Padding", "com.mindbright.security.cipher.DES");
        put("Cipher.3DES",         "com.mindbright.security.cipher.DES3");
        put("Cipher.3DES/ECB",     "com.mindbright.security.cipher.DES3");
        put("Cipher.3DES/CBC",     "com.mindbright.security.cipher.DES3");
        put("Cipher.3DES/CFB",     "com.mindbright.security.cipher.DES3");
        put("Cipher.3DES/OFB",     "com.mindbright.security.cipher.DES3");
        put("Cipher.3DES/CTR",     "com.mindbright.security.cipher.DES3");
        put("Cipher.3DES/CBC/PKCS5Padding", "com.mindbright.security.cipher.DES3");
        put("Cipher.Blowfish",     "com.mindbright.security.cipher.Blowfish");
        put("Cipher.Blowfish/ECB", "com.mindbright.security.cipher.Blowfish");
        put("Cipher.Blowfish/CBC", "com.mindbright.security.cipher.Blowfish");
        put("Cipher.Blowfish/CFB", "com.mindbright.security.cipher.Blowfish");
        put("Cipher.Blowfish/OFB", "com.mindbright.security.cipher.Blowfish");
        put("Cipher.Blowfish/CTR", "com.mindbright.security.cipher.Blowfish");
        put("Cipher.Twofish",      "com.mindbright.security.cipher.Twofish");
        put("Cipher.Twofish/ECB",  "com.mindbright.security.cipher.Twofish");
        put("Cipher.Twofish/CBC",  "com.mindbright.security.cipher.Twofish");
        put("Cipher.Twofish/CFB",  "com.mindbright.security.cipher.Twofish");
        put("Cipher.Twofish/OFB",  "com.mindbright.security.cipher.Twofish");
        put("Cipher.Twofish/CTR",  "com.mindbright.security.cipher.Twofish");
        put("Cipher.Rijndael",     "com.mindbright.security.cipher.Rijndael");
        put("Cipher.Rijndael/ECB", "com.mindbright.security.cipher.Rijndael");
        put("Cipher.Rijndael/CBC", "com.mindbright.security.cipher.Rijndael");
        put("Cipher.Rijndael/CFB", "com.mindbright.security.cipher.Rijndael");
        put("Cipher.Rijndael/OFB", "com.mindbright.security.cipher.Rijndael");
        put("Cipher.Rijndael/CTR", "com.mindbright.security.cipher.Rijndael");
        put("Cipher.IDEA",         "com.mindbright.security.cipher.IDEA");
        put("Cipher.IDEA/ECB",     "com.mindbright.security.cipher.IDEA");
        put("Cipher.IDEA/CBC",     "com.mindbright.security.cipher.IDEA");
        put("Cipher.IDEA/CFB",     "com.mindbright.security.cipher.IDEA");
        put("Cipher.IDEA/OFB",     "com.mindbright.security.cipher.IDEA");
        put("Cipher.CAST128",      "com.mindbright.security.cipher.CAST128");
        put("Cipher.CAST128/ECB",  "com.mindbright.security.cipher.CAST128");
        put("Cipher.CAST128/CBC",  "com.mindbright.security.cipher.CAST128");
        put("Cipher.CAST128/CFB",  "com.mindbright.security.cipher.CAST128");
        put("Cipher.CAST128/OFB",  "com.mindbright.security.cipher.CAST128");
        put("Cipher.RC2",          "com.mindbright.security.cipher.RC2");
        put("Cipher.RC2/ECB",      "com.mindbright.security.cipher.RC2");
        put("Cipher.RC2/CBC",      "com.mindbright.security.cipher.RC2");
        put("Cipher.RC2/CFB",      "com.mindbright.security.cipher.RC2");
        put("Cipher.RC2/OFB",      "com.mindbright.security.cipher.RC2");
        put("Cipher.RC2/CBC/PKCS5Padding", "com.mindbright.security.cipher.RC2");
        put("Cipher.RC4/OFB",      "com.mindbright.security.cipher.ArcFour");
        put("Cipher.RC4/OFB/PKCS5Padding", "com.mindbright.security.cipher.ArcFour");
        put("Cipher.RC4Skip/OFB", "com.mindbright.security.cipher.ArcFourSkip");
        put("Alg.Alias.Cipher.AES",     "Rijndael/ECB");
        put("Alg.Alias.Cipher.AES/ECB", "Rijndael/ECB");
        put("Alg.Alias.Cipher.AES/CBC", "Rijndael/CBC");
        put("Alg.Alias.Cipher.AES/CFB", "Rijndael/CFB");
        put("Alg.Alias.Cipher.AES/OFB", "Rijndael/OFB");
        put("Alg.Alias.Cipher.AES/CTR", "Rijndael/CTR");
        put("Alg.Alias.Cipher.DESede",     "3DES/ECB");
        put("Alg.Alias.Cipher.DESede/ECB", "3DES/ECB");
        put("Alg.Alias.Cipher.DESede/CBC", "3DES/CBC");
        put("Alg.Alias.Cipher.DESede/CFB", "3DES/CFB");
        put("Alg.Alias.Cipher.DESede/OFB", "3DES/OFB");
        put("Alg.Alias.Cipher.DESede/CBC/PKCS5Padding", "3DES/CBC/PKCS5Padding");
        put("Alg.Alias.Cipher.RC4",     "RC4/OFB");
        put("Alg.Alias.Cipher.ArcFour", "RC4/OFB");
        put("Alg.Alias.Cipher.ArcFourSkip", "RC4Skip/OFB");
        put("Alg.Alias.Cipher.CAST5",   "CAST128");
        put("Alg.Alias.Cipher.CAST5/ECB", "CAST128/ECB");
        put("Alg.Alias.Cipher.CAST5/CBC", "CAST128/CBC");
        put("Alg.Alias.Cipher.CAST5/CFB", "CAST128/CFB");
        put("Alg.Alias.Cipher.CAST5/OFB", "CAST128/OFB");

        put("MessageDigest.MD2",       "com.mindbright.security.digest.MD2");
        put("MessageDigest.MD5",       "com.mindbright.security.digest.MD5");
        put("MessageDigest.SHA",       "com.mindbright.security.digest.SHA1");
        put("MessageDigest.SHA256",    "com.mindbright.security.digest.SHA256");
        put("MessageDigest.RIPEMD160", "com.mindbright.security.digest.RIPEMD160");
        put("Alg.Alias.MessageDigest.SHA-1",              "SHA");
        put("Alg.Alias.MessageDigest.SHA1",               "SHA");
        put("Alg.Alias.MessageDigest.1.3.14.3.2.26",      "SHA");
        put("Alg.Alias.MessageDigest.1.2.840.113549.2.5", "MD5");
        put("Alg.Alias.MessageDigest.1.2.840.113549.2.2", "MD2");
        put("Alg.Alias.MessageDigest.1.3.36.3.2.1",       "RIPEMD160");
        put("Alg.Alias.MessageDigest.SHA-256",            "SHA256");
        put("Alg.Alias.MessageDigest.SHA2-256",           "SHA256");

        put("Mac.HmacSHA1",      "com.mindbright.security.mac.HMACSHA1");
        put("Mac.HmacMD5",       "com.mindbright.security.mac.HMACMD5");
        put("Mac.HmacRIPEMD160", "com.mindbright.security.mac.HMACRIPEMD160");
        put("Mac.HmacSHA1-96",   "com.mindbright.security.mac.HMACSHA1_96");
        put("Mac.HmacMD5-96",    "com.mindbright.security.mac.HMACMD5_96");
        put("Mac.HmacRIPEMD160-96", "com.mindbright.security.mac.HMACRIPEMD160_96");
        /* Convenience, not id of MAC itself but of the used HASH */
        put("Alg.Alias.Mac.1.2.840.113549.2.5", "HmacMD5");
        put("Alg.Alias.Mac.1.3.14.3.2.26",      "HmacSHA1");
        /* From IANA numbers for ipsec */
        put("Alg.Alias.Mac.1.3.6.1.5.5.8.1.1",  "HmacMD5");
        put("Alg.Alias.Mac.1.3.6.1.5.5.8.1.2",  "HmacSHA1");
        put("Alg.Alias.Mac.1.3.6.1.5.5.8.1.4",  "HmacRIPEMD160");

        put("SecureRandom.BlumBlumShub", "com.mindbright.security.prng.BlumBlumShub");

        put("Signature.SHA1withRawDSA", "com.mindbright.security.publickey.RawDSAWithSHA1");
        put("Signature.SHA1withDSA", "com.mindbright.security.publickey.DSAWithSHA1");
        put("Signature.SHA1withRSA", "com.mindbright.security.publickey.RSAWithSHA1");
        put("Signature.MD5withRSA", "com.mindbright.security.publickey.RSAWithMD5");
        put("Signature.MD2withRSA", "com.mindbright.security.publickey.RSAWithMD2");
        put("Signature.RIPEMD160withRSA", "com.mindbright.security.publickey.RSAWithRIPEMD160");
        put("Alg.Alias.Signature.1.3.14.3.2.13", "SHA1withDSA");
        put("Alg.Alias.Signature.1.3.14.3.2.27", "SHA1withDSA");
        put("Alg.Alias.Signature.1.2.840.10040.4.3", "SHA1withDSA");
        put("Alg.Alias.Signature.1.3.14.3.2.29",  "SHA1withRSA");
        put("Alg.Alias.Signature.1.2.840.113549.1.1.5", "SHA1withRSA");
        put("Alg.Alias.Signature.1.3.14.3.2.3", "MD5withRSA");
        put("Alg.Alias.Signature.1.3.14.3.2.25", "MD5withRSA");
        put("Alg.Alias.Signature.1.2.840.113549.1.1.4", "MD5withRSA");
        put("Alg.Alias.Signature.1.3.14.3.2.24", "MD2withRSA");
        put("Alg.Alias.Signature.1.2.840.113549.1.1.2", "MD2withRSA");
        put("Alg.Alias.Signature.1.3.36.3.3.1.2", "RIPEMD160withRSA");

        put("KeyFactory.RSA", "com.mindbright.security.publickey.RSAKeyFactory");
        put("KeyFactory.DSA", "com.mindbright.security.publickey.DSAKeyFactory");
        put("KeyFactory.DH",  "com.mindbright.security.publickey.DHKeyFactory");

        put("KeyPairGenerator.DH", "com.mindbright.security.publickey.DHKeyPairGenerator");
        put("KeyPairGenerator.RSA", "com.mindbright.security.publickey.RSAKeyPairGenerator");
        put("KeyPairGenerator.DSA", "com.mindbright.security.publickey.DSAKeyPairGenerator");

        put("KeyAgreement.DH", "com.mindbright.security.publickey.DHKeyAgreement");

        put("KeyStore.PKCS12", "com.mindbright.security.keystore.PKCS12KeyStore");
        put("KeyStore.Netscape", "com.mindbright.security.keystore.NetscapeKeyStore");
    }

}
