/******************************************************************************
 *
 * Copyright (c) 1999-2006 AppGate Network Security AB. All Rights Reserved.
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

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PushbackInputStream;
import java.util.NoSuchElementException;
import java.util.StringTokenizer;

import com.mindbright.jca.security.PublicKey;
import com.mindbright.jca.security.interfaces.DSAPublicKey;
import com.mindbright.jca.security.interfaces.RSAPublicKey;
import com.mindbright.util.ASCIIArmour;
import com.mindbright.util.Base64;

/**
 * This class implements the file formats commonly used for storing public keys
 * for public key authentication. It can handle both OpenSSH's proprietary file
 * format aswell as the (draft) standard format. When importing/exporting use
 * the appropriate constructor and the load/store methods. Note that this class
 * can also be used to convert key pair files between the formats.
 *
 * @see SSH2KeyPairFile
 */
public class SSH2PublicKeyFile {

    public final static String BEGIN_PUB_KEY = "---- BEGIN SSH2 PUBLIC KEY ----";
    public final static String END_PUB_KEY   = "---- END SSH2 PUBLIC KEY ----";

    private PublicKey publicKey;
    private String    subject;
    private String    comment;
    private boolean   sshComFormat;
    private boolean   puttyFormat;

    public final static String FILE_SUBJECT = "Subject";
    public final static String FILE_COMMENT = "Comment";

    /**
     * This is the constructor used for storing a public key.
     *
     * @param publicKey the public key to store
     * @param subject   the subject name of the key owner
     * @param comment   a comment to accompany the key
     */
    public SSH2PublicKeyFile(PublicKey publicKey,
                             String subject, String comment) {
        this.publicKey = publicKey;
        this.subject   = subject;
        this.comment   = comment;
    }

    /**
     * This is the constructor used for loading a public key.
     */
    public SSH2PublicKeyFile() {
        this(null, null, null);
    }

    public String getAlgorithmName() {
        String alg = null;
        if(publicKey instanceof DSAPublicKey) {
            alg = "ssh-dss";
        } else if(publicKey instanceof RSAPublicKey) {
            alg = "ssh-rsa";
        }
        return alg;
    }

    public boolean isSSHComFormat() {
        return sshComFormat;
    }

    public boolean isPuttyFormat() {
        return puttyFormat;
    }

    public void load(String fileName) throws IOException, SSH2Exception {
        load(fileName, null);
    }

    public void load(String fileName, String password) throws IOException, SSH2Exception {
        FileInputStream in  = new FileInputStream(fileName);
        load(in);
        in.close();
    }

    public void load(InputStream in) throws IOException, SSH2Exception {
        load(in, null);
    }

    public void load(InputStream in, String password) throws IOException, SSH2Exception {
        PushbackInputStream pbi = new PushbackInputStream(in);

        int c = pbi.read();
        pbi.unread(c);

        byte[] keyBlob = null;
        String format  = null;

        if(c == 's') {
            int    l   = pbi.available();
            byte[] buf = new byte[l];
            int    o   = 0;
            while(o < l) {
                int n = pbi.read(buf, o, l - o);
                if(n == -1)
                    throw new SSH2FatalException("Corrupt public key file");
                o += n;
            }
            StringTokenizer st     = new StringTokenizer(new String(buf));
            String          base64 = null;
            try {
                format       = st.nextToken();
                base64       = st.nextToken();
                this.comment = st.nextToken();
            } catch (NoSuchElementException e) {
                throw new SSH2FatalException("Corrupt openssh public key string");
            }
            keyBlob = Base64.decode(base64.getBytes());
        } else if(c == '-') {
            ASCIIArmour armour = new ASCIIArmour(BEGIN_PUB_KEY, END_PUB_KEY);

            keyBlob = armour.decode(pbi);
            format  = SSH2SimpleSignature.getKeyFormat(keyBlob);

            this.subject      = armour.getHeaderField(FILE_SUBJECT);
            this.comment      = stripQuotes(armour.getHeaderField(FILE_COMMENT));
            this.sshComFormat = true;
        } else if(c == 'P') { // PuTTY
	    try {
		SSH2PuttyKeyFile pkf = new SSH2PuttyKeyFile(pbi);
                if (!pkf.validate(password))
                    throw new SSH2AccessDeniedException("Failed to validate PuTTY key file");

		keyBlob = pkf.getPublicKeyBlob();
		if (keyBlob == null)
		    throw new SSH2FatalException("Corrupt PuTTY key file");

		format = pkf.getFormat();
		comment = pkf.getComment();		
		puttyFormat = true;
	    } catch (NoSuchElementException e) {
		throw new SSH2FatalException("Corrupt PuTTY key file");
	    }
        } else {
            throw new SSH2FatalException("Corrupt or unknown public key file format");
        }

        SSH2Signature decoder = SSH2Signature.getEncodingInstance(format);

        this.publicKey = decoder.decodePublicKey(keyBlob);
    }

    public String store(String fileName) throws IOException, SSH2Exception {
	if (puttyFormat)
	    throw new SSH2FatalException("No support for writing putty key files");
        return store(fileName, sshComFormat);
    }

    public String store(String fileName, boolean sshComFormat)
    throws IOException, SSH2Exception {
        FileOutputStream out       = new FileOutputStream(fileName);
        String           keyString = store(sshComFormat);
        out.write(keyString.getBytes());
        out.close();
        return keyString;
    }

    public String store(boolean sshComFormat) throws SSH2Exception {
        String format = getAlgorithmName();

        if(format == null) {
            throw new SSH2FatalException("Unknown publickey alg: " + publicKey);
        }

        byte[]        keyBlob   = getRaw();
        String        keyString = null;

	if(sshComFormat) {
            ASCIIArmour armour = new ASCIIArmour(BEGIN_PUB_KEY, END_PUB_KEY);
            armour.setCanonicalLineEnd(false);
            armour.setHeaderField(FILE_SUBJECT, subject);
            armour.setHeaderField(FILE_COMMENT, "\"" + comment + "\"");
            keyString = new String(armour.encode(keyBlob));
        } else {
            byte[]       base64  = Base64.encode(keyBlob);
            StringBuffer buf     = new StringBuffer();
            buf.append(format);
            buf.append(" ");
            buf.append(new String(base64));
            buf.append(" ");
            buf.append(comment);
            buf.append("\n");
            keyString = buf.toString();
        }

        return keyString;
    }

    public PublicKey getPublicKey() {
        return publicKey;
    }

    public byte[] getRaw() throws SSH2Exception {
        SSH2Signature encoder =
            SSH2Signature.getEncodingInstance(getAlgorithmName());
        return encoder.encodePublicKey(publicKey);
    }

    public String getSubject() {
        return subject;
    }

    public void setSubject(String subject) {
        this.subject = subject;
    }

    public String getComment() {
        return comment;
    }

    public void setComment(String comment) {
        this.comment = comment;
    }

    public boolean sameAs(PublicKey other) {
        return SSH2HostKeyVerifier.comparePublicKeys(publicKey, other);
    }

    private String stripQuotes(String str) throws SSH2FatalException {
        if(str != null && str.charAt(0) == '"') {
            if(str.charAt(str.length() - 1) != '"') {
                throw new SSH2FatalException("Unbalanced quotes in key file comment");
            }
            str = str.substring(1, str.length() - 1);
        }
        return str;
    }

    /* !!! DEBUG
    public static void main(String[] argv) {
    try {
     SSH2PublicKeyFile key = new SSH2PublicKeyFile();
     key.load("/home/mats/.ssh/id_dsa.pub");
     key.store("/home/mats/id_dsa.pub");
     key.load("/home/mats/.ssh/id_rsa.pub");
     key.store("/home/mats/id_rsa.pub");
     key.load("/home/mats/.ssh2/id_dsa_1024_a.pub");
     key.store("/home/mats/id_dsa_a.pub");
    } catch (Exception e) {
     e.printStackTrace();
    }
    } */

}
