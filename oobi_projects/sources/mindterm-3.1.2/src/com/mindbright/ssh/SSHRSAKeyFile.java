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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.math.BigInteger;

import com.mindbright.jca.security.interfaces.RSAPrivateCrtKey;
import com.mindbright.jca.security.interfaces.RSAPublicKey;

public class SSHRSAKeyFile {

    //
    //
    int                cipherType;
    RSAPublicKey       pubKey;
    String             fileComment;

    byte[]             encrypted;

    final static String privFileId = "SSH PRIVATE KEY FILE FORMAT 1.1\n";

    static public void createKeyFile(SSH ssh, RSAPrivateCrtKey privKey,
                                     String passwd, String name, String comment)
    throws IOException {
        ByteArrayOutputStream baos  = new ByteArrayOutputStream(8192);
        SSHDataOutputStream dataOut = new SSHDataOutputStream(baos);

        byte[] c = new byte[2];
        ssh.secureRandom().nextBytes(c);
        dataOut.writeByte((int)c[0]);
        dataOut.writeByte((int)c[1]);
        dataOut.writeByte((int)c[0]);
        dataOut.writeByte((int)c[1]);
        dataOut.writeBigInteger(privKey.getPrivateExponent());
        dataOut.writeBigInteger(privKey.getCrtCoefficient());
        dataOut.writeBigInteger(privKey.getPrimeQ());
        dataOut.writeBigInteger(privKey.getPrimeP());

        byte[] encrypted = baos.toByteArray();
        c = new byte[(8 - (encrypted.length % 8)) + encrypted.length];
        System.arraycopy(encrypted, 0, c, 0, encrypted.length);
        encrypted = c;

        int cipherType = SSH.CIPHER_DEFAULT;

        SSHCipher cipher = SSHCipher.getInstance(SSH.cipherClasses[cipherType][0]);
        cipher.setKey(passwd);
        encrypted = cipher.encrypt(encrypted);

        FileOutputStream fileOut = new FileOutputStream(name);
        dataOut = new SSHDataOutputStream(fileOut);

        dataOut.writeBytes(privFileId);
        dataOut.writeByte(0);

        dataOut.writeByte(cipherType);
        dataOut.writeInt(0);
        dataOut.writeInt(0);
        dataOut.writeBigInteger(privKey.getModulus());
        dataOut.writeBigInteger(privKey.getPublicExponent());
        dataOut.writeString(comment);

        dataOut.write(encrypted, 0, encrypted.length);
        dataOut.close();
    }

    public SSHRSAKeyFile(String name) throws IOException {
        FileInputStream    fileIn = new FileInputStream(name);
        SSHDataInputStream dataIn = new SSHDataInputStream(fileIn);

        byte[] id = new byte[privFileId.length()];
        dataIn.readFully(id);
        String idStr = new String(id);
        dataIn.readByte(); // Skip end-of-string (?!)

        if(!idStr.equals(privFileId))
            throw new IOException("RSA key file corrupt");

        cipherType = dataIn.readByte();
        if(SSH.cipherClasses[cipherType][0] == null)
            throw new IOException("Ciphertype " + cipherType + " in key-file not supported");

        dataIn.readInt(); // Skip a reserved int

        dataIn.readInt(); // Skip bits... (!?)

        BigInteger n = dataIn.readBigInteger();
        BigInteger e = dataIn.readBigInteger();
        pubKey       = new com.mindbright.security.publickey.RSAPublicKey(n, e);

        fileComment  = dataIn.readString();

        byte[] rest = new byte[8192];
        int    len  = dataIn.read(rest);
        dataIn.close();

        encrypted = new byte[len];
        System.arraycopy(rest, 0, encrypted, 0, len);
    }

    public String getComment() {
        return fileComment;
    }

    public RSAPublicKey getPublic() {
        return pubKey;
    }

    public RSAPrivateCrtKey getPrivate(String passwd) {
        RSAPrivateCrtKey privKey = null;

        SSHCipher cipher = SSHCipher.getInstance(SSH.cipherClasses[cipherType][0]);
        cipher.setKey(passwd);
        byte[] decrypted = cipher.decrypt(encrypted);
        SSHDataInputStream dataIn = new SSHDataInputStream(new ByteArrayInputStream(decrypted));

        try {
            byte c1  = dataIn.readByte();
            byte c2  = dataIn.readByte();
            byte c11 = dataIn.readByte();
            byte c22 = dataIn.readByte();

            if(c1 != c11 || c2 != c22)
                return null;

            BigInteger d = dataIn.readBigInteger();
            BigInteger u = dataIn.readBigInteger();
            BigInteger q = dataIn.readBigInteger();
            BigInteger p = dataIn.readBigInteger();
            dataIn.close();

            // Older versions of MindTerm wrote the private key file with p and q
            // reversed. This test unreverses them if needed.
            if (p.compareTo(q) < 0) {
                BigInteger t = q;
                q = p;
                p = t;
            }

            privKey =
                new com.mindbright.security.publickey.RSAPrivateCrtKey(pubKey.getModulus(),
                        pubKey.getPublicExponent(),
                        d, p, q, u);
        } catch (IOException e) {
            privKey = null;
        }

        return privKey;
    }

    /* !!! DEBUG
    public static void main(String[] argv) {
      SSHRSAKeyFile file = null;

      try {
        file = new SSHRSAKeyFile("/home/mats/.ssh/identity");
        file.getPrivate("********");
      } catch (Exception e) {
        System.out.println("Error: " + e.toString());
      }
      System.out.println("Comment: " + file.fileComment);
    }
    */

}


