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

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PushbackInputStream;
import java.math.BigInteger;

import com.mindbright.asn1.ASN1DER;
import com.mindbright.asn1.ASN1Integer;
import com.mindbright.asn1.ASN1OIDRegistry;
import com.mindbright.asn1.ASN1Object;
import com.mindbright.asn1.ASN1Sequence;
import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.KeyFactory;
import com.mindbright.jca.security.KeyPair;
import com.mindbright.jca.security.MessageDigest;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.jca.security.PublicKey;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.jca.security.interfaces.DSAParams;
import com.mindbright.jca.security.interfaces.DSAPrivateKey;
import com.mindbright.jca.security.interfaces.DSAPublicKey;
import com.mindbright.jca.security.interfaces.RSAPrivateCrtKey;
import com.mindbright.jca.security.interfaces.RSAPublicKey;
import com.mindbright.jca.security.spec.DSAPrivateKeySpec;
import com.mindbright.jca.security.spec.DSAPublicKeySpec;
import com.mindbright.jca.security.spec.InvalidKeySpecException;
import com.mindbright.jca.security.spec.KeySpec;
import com.mindbright.jca.security.spec.RSAPrivateCrtKeySpec;
import com.mindbright.jca.security.spec.RSAPrivateKeySpec;
import com.mindbright.jca.security.spec.RSAPublicKeySpec;
import com.mindbright.jce.crypto.Cipher;
import com.mindbright.jce.crypto.spec.IvParameterSpec;
import com.mindbright.jce.crypto.spec.SecretKeySpec;
import com.mindbright.security.pkcs1.RSAPrivateKey;
import com.mindbright.util.ASCIIArmour;
import com.mindbright.util.HexDump;

/**
 * This class implements the file formats commonly used for storing key pairs
 * for public key authentication. It can handle both OpenSSH's PEM file format
 * as well as SSH Communications proprietary format for DSA keys. It
 * can also read the PuTTY key file format. When importing/exporting
 * use the appropriate constructor and the load/store methods. Note
 * that this class can also be used to convert key pair files between
 * the formats.
 *
 * @see SSH2PublicKeyFile
 */
public class SSH2KeyPairFile {

    private final static int TYPE_PEM_DSA    = 0;
    private final static int TYPE_PEM_RSA    = 1;
    private final static int TYPE_SSHCOM_DSA = 2;

    public final static String[] BEGIN_PRV_KEY = {
                "-----BEGIN DSA PRIVATE KEY-----",
                "-----BEGIN RSA PRIVATE KEY-----",
                "---- BEGIN SSH2 ENCRYPTED PRIVATE KEY ----"
            };

    public final static String[] END_PRV_KEY   = {
                "-----END DSA PRIVATE KEY-----",
                "-----END RSA PRIVATE KEY-----",
                "---- END SSH2 ENCRYPTED PRIVATE KEY ----"
            };

    public final static int SSH_PRIVATE_KEY_MAGIC = 0x3f6ff9eb;

    public final static String PRV_PROCTYPE = "Proc-Type";
    public final static String PRV_DEKINFO  = "DEK-Info";

    public final static String FILE_SUBJECT = "Subject";
    public final static String FILE_COMMENT = "Comment";

    private KeyPair       keyPair;
    private ASCIIArmour   armour;
    private String        subject;
    private String        comment;
    private boolean       sshComFormat;
    private boolean       puttyFormat;

    static {
        ASN1OIDRegistry.addModule("com.mindbright.security.pkcs1");
    }

    /**
     * Handles PEM encoding of a DSA key. From OpenSSL doc for dsa.
     * <p>
     * <pre>
     * PEMDSAPrivate ::= SEQUENCE {
     *   version  Version,
     *   p        INTEGER,
     *   q        INTEGER,
     *   g        INTEGER,
     *   y        INTEGER,
     *   x        INTEGER
     * }
     *
     * Version ::= INTEGER { openssl(0) }
     * </pre>
     * (OpenSSL currently hardcodes version to 0)
     */
    public static final class PEMDSAPrivate extends ASN1Sequence {

        public ASN1Integer version;
        public ASN1Integer p;
        public ASN1Integer q;
        public ASN1Integer g;
        public ASN1Integer y;
        public ASN1Integer x;

        public PEMDSAPrivate() {
            version = new ASN1Integer();
            p       = new ASN1Integer();
            q       = new ASN1Integer();
            g       = new ASN1Integer();
            y       = new ASN1Integer();
            x       = new ASN1Integer();
            addComponent(version);
            addComponent(p);
            addComponent(q);
            addComponent(g);
            addComponent(y);
            addComponent(x);
        }

        public PEMDSAPrivate(int version,
                             BigInteger p, BigInteger q, BigInteger g,
                             BigInteger y, BigInteger x) {
            this();
            this.version.setValue(version);
            this.p.setValue(p);
            this.q.setValue(q);
            this.g.setValue(g);
            this.y.setValue(y);
            this.x.setValue(x);
        }

    }

    /**
     * This is the constructor used for storing a key pair.
     *
     * @param keyPair the key pair to store
     * @param subject the subject name of the key owner
     * @param comment a comment to accompany the key
     */
    public SSH2KeyPairFile(KeyPair keyPair, String subject, String comment) {
        this.keyPair = keyPair;
        this.armour  = new ASCIIArmour("----");
        this.subject = subject;
        this.comment = comment;
    }

    /**
     * This is the constructor used for loading a key pair.
     */
    public SSH2KeyPairFile() {
        this(null, null, null);
    }

    public KeyPair getKeyPair() {
        return keyPair;
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

    public ASCIIArmour getArmour() {
        return armour;
    }

    public boolean isSSHComFormat() {
        return sshComFormat;
    }

    public boolean isPuttyFormat() {
        return puttyFormat;
    }

    public String getAlgorithmName() {
        PublicKey publicKey = keyPair.getPublic();
        String    alg       = null;
        if(publicKey instanceof DSAPublicKey) {
            alg = "ssh-dss";
        } else if(publicKey instanceof RSAPublicKey) {
            alg = "ssh-rsa";
        }
        return alg;
    }

    public int getBitLength() {
        PublicKey publicKey = keyPair.getPublic();
        if(publicKey instanceof DSAPublicKey) {
            return ((DSAPublicKey)publicKey).getParams().getP().bitLength();
        } else {
            return ((RSAPublicKey)publicKey).getModulus().bitLength();
        }
    }

    public static byte[] writeKeyPair(ASCIIArmour armour, String password,
                                      SecureRandom random,
                                      KeyPair keyPair)
    throws SSH2FatalException {
        ASN1Object pem;
        PublicKey  publicKey = keyPair.getPublic();
        int        headType;

        if(publicKey instanceof DSAPublicKey) {
            DSAPublicKey  pubKey = (DSAPublicKey)keyPair.getPublic();
            DSAPrivateKey prvKey = (DSAPrivateKey)keyPair.getPrivate();
            DSAParams     params = pubKey.getParams();

            PEMDSAPrivate dsa = new PEMDSAPrivate(0,
                                                  params.getP(),
                                                  params.getQ(),
                                                  params.getG(),
                                                  pubKey.getY(),
                                                  prvKey.getX());
            pem = dsa;
            headType = TYPE_PEM_DSA;

        } else if(publicKey instanceof RSAPublicKey) {
            RSAPublicKey     pubKey = (RSAPublicKey)keyPair.getPublic();
            RSAPrivateCrtKey prvKey = (RSAPrivateCrtKey)keyPair.getPrivate();

            RSAPrivateKey rsa = new RSAPrivateKey(0,
                                                  pubKey.getModulus(),
                                                  pubKey.getPublicExponent(),
                                                  prvKey.getPrivateExponent(),
                                                  prvKey.getPrimeP(),
                                                  prvKey.getPrimeQ(),
                                                  prvKey.getCrtCoefficient());
            pem = rsa;
            headType = TYPE_PEM_RSA;

        } else {
            throw new SSH2FatalException("Unsupported key type: " + publicKey);
        }

        armour.setHeaderLine(BEGIN_PRV_KEY[headType]);
        armour.setTailLine(END_PRV_KEY[headType]);

        ByteArrayOutputStream enc = new ByteArrayOutputStream(128);
        ASN1DER               der = new ASN1DER();

        try {
            der.encode(enc, pem);
        } catch (IOException e) {
            throw new SSH2FatalException("Error while DER encoding");
        }

        byte[] keyBlob = enc.toByteArray();

        if(password != null && password.length() > 0) {
            byte[] iv = new byte[8];
            byte[] key;

            random.setSeed(keyBlob);

            for(int i = 0; i < 8; i++) {
                byte[] r = new byte[1];
                do {
                    random.nextBytes(r);
                    iv[i] = r[0];
                } while(iv[i] == 0x00);
            }

            key = expandPasswordToKey(password, 192 / 8, iv);

            armour.setHeaderField(PRV_PROCTYPE, "4,ENCRYPTED");
            armour.setHeaderField(PRV_DEKINFO, "DES-EDE3-CBC," +
                                  HexDump.toString(iv).toUpperCase());

            int    encLen = (8 - (keyBlob.length % 8)) + keyBlob.length;
            byte[] encBuf = new byte[encLen];

            doCipher(Cipher.ENCRYPT_MODE, password,
                     keyBlob, keyBlob.length, encBuf, iv);

            keyBlob = encBuf;
        }

        return keyBlob;
    }

    public static byte[] writeKeyPairSSHCom(String password,
                                            String cipher, KeyPair keyPair)
    throws SSH2FatalException {
        SSH2DataBuffer toBeEncrypted = new SSH2DataBuffer(8192);
        int            totLen        = 0;

        DSAPublicKey  pubKey = (DSAPublicKey)keyPair.getPublic();
        DSAPrivateKey prvKey = (DSAPrivateKey)keyPair.getPrivate();
        DSAParams     params = pubKey.getParams();

        if(!(pubKey instanceof DSAPublicKey)) {
            throw new SSH2FatalException("Unsupported key type: " + pubKey);
        }

        toBeEncrypted.writeInt(0); // unenc length (filled in below)

        toBeEncrypted.writeInt(0); // type 0 is explicit params (as opposed to predefined)
        toBeEncrypted.writeBigIntBits(params.getP());
        toBeEncrypted.writeBigIntBits(params.getG());
        toBeEncrypted.writeBigIntBits(params.getQ());
        toBeEncrypted.writeBigIntBits(pubKey.getY());
        toBeEncrypted.writeBigIntBits(prvKey.getX());

        totLen = toBeEncrypted.getWPos();
        toBeEncrypted.setWPos(0);
        toBeEncrypted.writeInt(totLen - 4);

        if(!cipher.equals("none")) {
            try {
                int    keyLen     = SSH2Preferences.getCipherKeyLen(cipher);
                String cipherName = SSH2Preferences.ssh2ToJCECipher(cipher);
                byte[] key = expandPasswordToKeySSHCom(password, keyLen);
                Cipher encrypt = Cipher.getInstance(cipherName);
                encrypt.init(Cipher.ENCRYPT_MODE,
                             new SecretKeySpec(key, encrypt.getAlgorithm()));
                byte[] data = toBeEncrypted.getData();
                int    bs   = encrypt.getBlockSize();
                totLen += (bs - (totLen % bs)) % bs;
                totLen = encrypt.doFinal(data, 0, totLen, data, 0);
            } catch (NoSuchAlgorithmException e) {
                throw new SSH2FatalException("Invalid cipher in " +
                                             "SSH2KeyPairFile.writeKeyPair: " + cipher);
            } catch (InvalidKeyException e) {
                throw new SSH2FatalException("Invalid key derived in " +
                                             "SSH2KeyPairFile.writeKeyPair: " + e);
            }
        }

        SSH2DataBuffer buf = new SSH2DataBuffer(512 + totLen);

        buf.writeInt(SSH_PRIVATE_KEY_MAGIC);
        buf.writeInt(0); // total length (filled in below)
        buf.writeString("dl-modp{sign{dsa-nist-sha1},dh{plain}}");
        buf.writeString(cipher);
        buf.writeString(toBeEncrypted.getData(), 0, totLen);

        totLen = buf.getWPos();
        buf.setWPos(4);
        buf.writeInt(totLen);

        byte[] keyBlob = new byte[totLen];
        System.arraycopy(buf.data, 0, keyBlob, 0, totLen);

        return keyBlob;
    }

    public static KeyPair readKeyPair(ASCIIArmour armour, byte[] keyBlob,
                                      String password)
    throws SSH2Exception {
        String procType = armour.getHeaderField(PRV_PROCTYPE);

        if(procType != null && password != null) {
            String dekInfo = armour.getHeaderField(PRV_DEKINFO);
            if(dekInfo == null || !dekInfo.startsWith("DES-EDE3-CBC,")) {
                throw new SSH2FatalException("Proc type not supported: " +
                                             procType);
            }
            dekInfo = dekInfo.substring(13);
            BigInteger dekI = new BigInteger(dekInfo, 16);

            byte[] iv = dekI.toByteArray();
            if(iv.length > 8) {
                byte[] tmp = iv;
                iv = new byte[8];
                System.arraycopy(tmp, 1, iv, 0, 8);
            }
            doCipher(Cipher.DECRYPT_MODE, password,
                     keyBlob, keyBlob.length, keyBlob, iv);
        }

        ByteArrayInputStream enc         = new ByteArrayInputStream(keyBlob);
        ASN1DER              der         = new ASN1DER();
        KeySpec              prvSpec     = null;
        KeySpec              pubSpec     = null;
        String               keyFactType = null;

        String head = armour.getHeaderLine();
        if(head.indexOf("DSA") != -1) {
            keyFactType = "DSA";
        } else if(head.indexOf("RSA") != -1) {
            keyFactType = "RSA";
        }

        try {
            if("DSA".equals(keyFactType)) {
                PEMDSAPrivate dsa = new PEMDSAPrivate();
                der.decode(enc, dsa);

                BigInteger p, q, g, x, y;
                p = dsa.p.getValue();
                q = dsa.q.getValue();
                g = dsa.g.getValue();
                y = dsa.y.getValue();
                x = dsa.x.getValue();

                prvSpec = new DSAPrivateKeySpec(x, p, q, g);
                pubSpec = new DSAPublicKeySpec(y, p, q, g);

            } else if("RSA".equals(keyFactType)) {
                RSAPrivateKey rsa = new RSAPrivateKey();
                der.decode(enc, rsa);

                BigInteger n, e, d, p, q, pe, qe, u;

                n =  rsa.modulus.getValue();
                e =  rsa.publicExponent.getValue();
                d =  rsa.privateExponent.getValue();
                p =  rsa.prime1.getValue();
                q =  rsa.prime2.getValue();
                pe = rsa.exponent1.getValue();
                qe = rsa.exponent2.getValue();
                u =  rsa.coefficient.getValue();

                prvSpec = new RSAPrivateCrtKeySpec(n, e, d, p, q, pe, qe, u);
                pubSpec = new RSAPublicKeySpec(n, e);

            } else {
                throw new SSH2FatalException("Unsupported key type: " + keyFactType);
            }
        } catch (IOException e) {
            throw new SSH2AccessDeniedException("Invalid password or corrupt key blob");
        }

        try {
            KeyFactory keyFact = KeyFactory.getInstance(keyFactType);
            return new KeyPair(keyFact.generatePublic(pubSpec),
                               keyFact.generatePrivate(prvSpec));
        } catch (Exception e) {
            throw new SSH2FatalException("Error in readKeyPair: " + e );
        }
    }

    public static KeyPair readKeyPairSSHCom(byte[] keyBlob, String password)
    throws SSH2Exception {
        SSH2DataBuffer buf = new SSH2DataBuffer(keyBlob.length);

        buf.writeRaw(keyBlob);

        int    magic         = buf.readInt();
        int    privateKeyLen = buf.readInt();
        String type          = buf.readJavaString();
        String cipher        = buf.readJavaString();
        int    bufLen        = buf.readInt();

        if(type.indexOf("dl-modp") == -1) {
            // !!! TODO: keyformaterror exception?
            throw new SSH2FatalException("Unknown key type '" + type + "'");
        }

        if(magic != SSH_PRIVATE_KEY_MAGIC) {
            // !!! TODO: keyformaterror exception?
            throw new SSH2FatalException("Invalid magic in private key: " +
                                         magic);
        }

        if(!cipher.equals("none")) {
            try {
                int    keyLen     = SSH2Preferences.getCipherKeyLen(cipher);
                String cipherName = SSH2Preferences.ssh2ToJCECipher(cipher);
                byte[] key = expandPasswordToKeySSHCom(password, keyLen);
                Cipher decrypt = Cipher.getInstance(cipherName);
                decrypt.init(Cipher.DECRYPT_MODE,
                             new SecretKeySpec(key, decrypt.getAlgorithm()));
                byte[] data   = buf.getData();
                int    offset = buf.getRPos();
                decrypt.doFinal(data, offset, bufLen, data, offset);
            } catch (NoSuchAlgorithmException e) {
                throw new SSH2FatalException("Invalid cipher in " +
                                             "SSH2KeyPairFile.readKeyPairSSHCom: " +
                                             cipher);
            } catch (InvalidKeyException e) {
                throw new SSH2FatalException("Invalid key derived in " +
                                             "SSH2KeyPairFile.readKeyPairSSHCom: " + e);
            }
        }

        int parmLen = buf.readInt();
        if(parmLen > buf.getMaxReadSize() || parmLen < 0) {
            throw new SSH2AccessDeniedException("Invalid password or corrupt key blob");
        }

        int value = buf.readInt();
        BigInteger p, q, g, x, y;

        if(value == 0) {
            p = buf.readBigIntBits();
            g = buf.readBigIntBits();
            q = buf.readBigIntBits();
            y = buf.readBigIntBits();
            x = buf.readBigIntBits();
        } else {
            // !!! TODO: predefined params
            throw new Error("Predefined DSA params not implemented (" +
                            value + ") '" + buf.readJavaString() + "'");
        }

        try {
            KeyFactory keyFact = KeyFactory.getInstance("DSA");
            return new KeyPair(keyFact.generatePublic(
                                   new DSAPublicKeySpec(y, p, q, g)),
                               keyFact.generatePrivate(
                                   new DSAPrivateKeySpec(x, p, q, g)));
        } catch (Exception e) {
            throw new SSH2FatalException(
                "Error in SSH2KeyPairFile.readKeyPair: " + e );
        }
    }

    private void readKeyPairPutty(PushbackInputStream in, String password) 
      throws IOException, SSH2Exception, NoSuchAlgorithmException, 
             InvalidKeyException, InvalidKeySpecException {
	SSH2PuttyKeyFile pkf = new SSH2PuttyKeyFile(in);
	if (!pkf.validate(password))
            throw new SSH2AccessDeniedException("Failed to validate PuTTY key file");

	puttyFormat = true;
	comment = pkf.getComment();
	String format = pkf.getFormat();
	SSH2Signature decoder = SSH2Signature.getEncodingInstance(format);
	PublicKey pubkey = decoder.decodePublicKey(pkf.getPublicKeyBlob());

	byte [] b = pkf.getPrivateKeyBlob();
	if (format.equals("ssh-dss")) {
	    SSH2DataBuffer buf = new SSH2DataBuffer(b.length);
	    buf.writeRaw(b);
	    DSAParams dsaparams = ((DSAPublicKey)pubkey).getParams();
	    keyPair = new KeyPair
		(pubkey,
		 KeyFactory.getInstance("DSA").generatePrivate
		 (new DSAPrivateKeySpec(buf.readBigInt(), dsaparams.getP(), 
                                        dsaparams.getQ(), dsaparams.getG())));
	} else if (format.equals("ssh-rsa")) {
	    SSH2DataBuffer buf = new SSH2DataBuffer(b.length);
	    buf.writeRaw(b);
	    keyPair = new KeyPair
		(pubkey,
		 KeyFactory.getInstance("RSA").generatePrivate
		 (new RSAPrivateKeySpec(((RSAPublicKey)pubkey).getModulus(), 
                                        buf.readBigInt())));
	} else {
            throw new  SSH2FatalException("Unsupported key type: " + format);
        }
    }

    /**
     * Store the key pair in the given file
     *
     * @param fileName name of file to store keys in
     * @param random random number generator used when encrypting the
     * keys
     * @param password password to use when encrypting the keys
     */
    public void store(String fileName, SecureRandom random, String password)
        throws IOException, SSH2FatalException {
	if (puttyFormat)
	    throw new SSH2FatalException("No support for writing PuTTY key files");
        store(fileName, random, password, sshComFormat);
    }

    /**
     * Store the key pair in the given file with more format control
     *
     * @param fileName name of file to store keys in
     * @param random random number generator used when encrypting the
     * keys
     * @param password password to use when encrypting the keys
     * @param sshComFormat if tru store the key in the ssh.com format
     */
    public void store(String fileName, SecureRandom random, String password,
                      boolean sshComFormat)
        throws IOException, SSH2FatalException {
        FileOutputStream out = new FileOutputStream(fileName);
        store(out, random, password, sshComFormat);
        out.close();
    }

    /**
     * Store the key pair in the given stream with more format control
     *
     * @param out output stream to store keys to. Note that this
     * stream will not be closed.
     * @param random random number generator used when encrypting the
     * keys
     * @param password password to use when encrypting the keys
     * @param sshComFormat if tru store the key in the ssh.com format
     */
    public void store(OutputStream out, SecureRandom random, String password,
                      boolean sshComFormat)
        throws IOException, SSH2FatalException {
        armour.setBlankHeaderSep(!sshComFormat);
        armour.setLineLength(sshComFormat ?
                             ASCIIArmour.DEFAULT_LINE_LENGTH : 64);

        armour.setHeaderField(PRV_PROCTYPE, null);
        armour.setHeaderField(PRV_DEKINFO, null);
        armour.setHeaderField(FILE_SUBJECT, null);
        armour.setHeaderField(FILE_COMMENT, null);

        byte[] keyBlob = null;

        if(sshComFormat) {
            if(!(keyPair.getPublic() instanceof DSAPublicKey)) {
                throw new SSH2FatalException(
                    "Only DSA keys supported when saving in compatibility mode");
            }
            String cipher = ((password != null && password.length() > 0) ?
                             "3des-cbc" : "none");
            armour.setHeaderLine(BEGIN_PRV_KEY[TYPE_SSHCOM_DSA]);
            armour.setTailLine(END_PRV_KEY[TYPE_SSHCOM_DSA]);
            comment = "\"" + comment + "\"";
            keyBlob = writeKeyPairSSHCom(password, cipher, keyPair);
        } else {
            keyBlob = writeKeyPair(armour, password, random, keyPair);
        }

        armour.setHeaderField(FILE_SUBJECT, subject);
        armour.setHeaderField(FILE_COMMENT, comment);

        armour.setCanonicalLineEnd(false);
        armour.encode(out, keyBlob);
    }

    /**
     * Load key pair from file.
     *
     * @param fileName name of file to load keys from
     * @param password password used to encrypt the file
     */
    public void load(String fileName, String password)
        throws IOException, SSH2Exception {
        FileInputStream in = new FileInputStream(fileName);
        load(in, password);
        in.close();
    }

    /**
     * Load key pair from stream.
     *
     * @param in input stream from which the key pair is read. It will
     * be wrapped in a PushbackInputStream, but not closed.
     * @param password password used to encrypt the file
     */
    public void load(InputStream in, String password)
        throws IOException, SSH2Exception {
        PushbackInputStream pbi = new PushbackInputStream(in);

        int c = pbi.read();
	if (c == 'P') {
	    try {
		readKeyPairPutty(pbi, password);
            } catch (NoSuchAlgorithmException e) {
                throw new SSH2FatalException
		    ("Invalid cipher in " +
		     "SSH2KeyPairFile.readKeyPairPutty");
            } catch (InvalidKeyException e) {
                throw new SSH2FatalException
		    ("Invalid key derived in " +
		     "SSH2KeyPairFile.readKeyPairPutty: " + e);
            } catch (InvalidKeySpecException e) {
                throw new SSH2FatalException
		    ("Invalid key spec. derived in " +
		     "SSH2KeyPairFile.readKeyPairPutty: " + e);
            }
	    return;
	} else if( c != '-') {
            throw new SSH2FatalException("Corrupt or unsupported key file");
        }
        pbi.unread(c);

        armour         = new ASCIIArmour("----");
        byte[] keyBlob = armour.decode(pbi);

        if(armour.getHeaderLine().indexOf("SSH2") != -1) {
            this.sshComFormat = true;
            this.keyPair      = readKeyPairSSHCom(keyBlob, password);
        } else {
            this.keyPair = readKeyPair(armour, keyBlob, password);
        }

        this.subject = armour.getHeaderField(FILE_SUBJECT);
        this.comment = stripQuotes(armour.getHeaderField(FILE_COMMENT));
    }

    public static byte[] expandPasswordToKey(String password, int keyLen,
            byte[] salt) {
        try {
            MessageDigest md5    = MessageDigest.getInstance("MD5");
            int           digLen = md5.getDigestLength();
            byte[]        mdBuf  = new byte[digLen];
            byte[]        key    = new byte[keyLen];
            int           cnt    = 0;

            while(cnt < keyLen) {
                if(cnt > 0) {
                    md5.update(mdBuf);
                }
                md5.update(password.getBytes());
                md5.update(salt);
                md5.digest(mdBuf, 0, digLen);
                int n = ((digLen > (keyLen - cnt)) ? keyLen - cnt : digLen);
                System.arraycopy(mdBuf, 0, key, cnt, n);
                cnt += n;
            }

            return key;

        } catch (Exception e) {
            throw new Error("Error in SSH2KeyPairFile.expandPasswordToKey: " +
                            e);
        }
    }

    public static byte[] expandPasswordToKeySSHCom(String password, int keyLen) {
        try {
            if(password == null) {
                password = "";
            }
            MessageDigest md5    = MessageDigest.getInstance("MD5");
            int           digLen = md5.getDigestLength();
            byte[]        buf    = new byte[((keyLen + digLen) / digLen) *
                                            digLen];
            int           cnt    = 0;
            while(cnt < keyLen) {
                md5.update(password.getBytes());
                if(cnt > 0) {
                    md5.update(buf, 0, cnt);
                }
                md5.digest(buf, cnt, digLen);
                cnt += digLen;
            }
            byte[] key = new byte[keyLen];
            System.arraycopy(buf, 0, key, 0, keyLen);
            return key;
        } catch (Exception e) {
            throw new Error("Error in SSH2KeyPairFile.expandPasswordToKeySSHCom: " + e);
        }
    }

    private static void doCipher(int mode, String password,
                                 byte[] input, int len, byte[] output,
                                 byte[] iv)
    throws SSH2FatalException {
        byte[] key = expandPasswordToKey(password, 192 / 8, iv);

        try {
            Cipher cipher = Cipher.getInstance("3DES/CBC/PKCS5Padding");
            cipher.init(mode, new SecretKeySpec(key, cipher.getAlgorithm()),
                        new IvParameterSpec(iv));
            cipher.doFinal(input, 0, len, output, 0);
        } catch (NoSuchAlgorithmException e) {
            throw new SSH2FatalException("Invalid algorithm in " +
                                         "SSH2KeyPairFile.doCipher: " + e);
        } catch (InvalidKeyException e) {
            throw new SSH2FatalException("Invalid key derived in " +
                                         "SSH2KeyPairFile.doCipher: " + e);
        }
    }

    private String stripQuotes(String str) throws SSH2FatalException {
        if(str != null && str.length() > 0 && str.charAt(0) == '"') {
            if(str.charAt(str.length() - 1) != '"') {
                throw new SSH2FatalException("Unbalanced quotes in key file comment");
            }
            str = str.substring(1, str.length() - 1);
        }
        return str;
    }

}
