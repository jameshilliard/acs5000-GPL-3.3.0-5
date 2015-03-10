/******************************************************************************
 *
 * Copyright (c) 2005 AppGate Network Security AB. All Rights Reserved.
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

import java.math.BigInteger;

import com.mindbright.jca.security.KeyFactory;
import com.mindbright.jca.security.KeyPair;
import com.mindbright.jca.security.KeyPairGenerator;
import com.mindbright.jca.security.MessageDigest;
import com.mindbright.jce.crypto.KeyAgreement;
import com.mindbright.jce.crypto.interfaces.DHPrivateKey;
import com.mindbright.jce.crypto.interfaces.DHPublicKey;
import com.mindbright.jce.crypto.spec.DHParameterSpec;
import com.mindbright.jce.crypto.spec.DHPublicKeySpec;

/**
 * Implements diffie hellman key exchange using a predefined group. This
 * algorithm is known as 'diffie-hellman-group14-sha1'
 */
public class SSH2KEXDHGroup14SHA1 extends SSH2KeyExchanger {

    public final static BigInteger group14P =
        com.mindbright.security.publickey.ModPGroups.oakleyGroup14P;

    public final static BigInteger group14G =
        com.mindbright.security.publickey.ModPGroups.oakleyGroup14G;

    protected SSH2Transport transport;
    protected DHPublicKey   dhPublicKey;
    protected DHPrivateKey  dhPrivateKey;
    protected byte[]        serverHostKey;
    protected BigInteger    serverF;
    protected BigInteger    clientE;
    protected byte[]        sharedSecret_K;
    protected byte[]        exchangeHash_H;
    protected MessageDigest sha1;

    public void init(SSH2Transport transport) throws SSH2Exception {
        this.transport = transport;
        this.sha1      = createHash();

        DHParameterSpec group14Params = new DHParameterSpec(group14P, group14G);
        generateDHKeyPair(group14Params);

        if(!transport.isServer()) {
            sendDHINIT(SSH2.MSG_KEXDH_INIT);
        }
    }

    public void processKEXMethodPDU(SSH2TransportPDU pdu) throws SSH2Exception {
        if(pdu.getType() == SSH2.MSG_KEXDH_REPLY) {
            if(transport.isServer()) {
                throw new SSH2KEXFailedException("Unexpected KEXDH_REPLY");
            }

            serverHostKey      = pdu.readString();
            serverF            = pdu.readBigInt();
            byte[] serverSigH  = pdu.readString();

            DHPublicKeySpec srvPubSpec = new DHPublicKeySpec
                (serverF, group14P, group14G);
            
            computeSharedSecret_K(srvPubSpec);
            computeExchangeHash_H();

            transport.authenticateHost(serverHostKey, serverSigH,
                                       exchangeHash_H);

            transport.sendNewKeys();

        } else if(pdu.getType() == SSH2.MSG_KEXDH_INIT) {
            // !!! TODO SERVER
        }
    }

    public MessageDigest getExchangeHashAlgorithm() {
        sha1.reset();
        return sha1;
    }

    public byte[] getSharedSecret_K() {
        SSH2DataBuffer buf = new SSH2DataBuffer(1024);
        buf.writeString(sharedSecret_K);
        return buf.readRestRaw();
    }

    public byte[] getExchangeHash_H() {
        return exchangeHash_H;
    }

    public String getHostKeyAlgorithms() {
        return "ssh-dss,ssh-rsa";
    }

    protected void computeExchangeHash_H() {
        SSH2DataBuffer buf = new SSH2DataBuffer(64*1024);

        if(transport.isServer()) {
            serverF = dhPublicKey.getY();
        } else {
            clientE = dhPublicKey.getY();
        }

        buf.writeString(transport.getClientVersion());
        buf.writeString(transport.getServerVersion());
        buf.writeString(transport.getClientKEXINITPDU().getData(),
                        transport.getClientKEXINITPDU().getPayloadOffset(),
                        transport.getClientKEXINITPDU().getPayloadLength());
        buf.writeString(transport.getServerKEXINITPDU().getData(),
                        transport.getServerKEXINITPDU().getPayloadOffset(),
                        transport.getServerKEXINITPDU().getPayloadLength());
        buf.writeString(serverHostKey);
        buf.writeBigInt(clientE);
        buf.writeBigInt(serverF);
        buf.writeString(sharedSecret_K);

        sha1.reset();
        sha1.update(buf.getData(), 0, buf.getWPos());
        exchangeHash_H = sha1.digest();

        transport.getLog().debug2("SSH2KEXDHGroup14SHA1",
                                  "computeExchangeHash_H", "E: ",
                                  clientE.toByteArray());
        transport.getLog().debug2("SSH2KEXDHGroup14SHA1",
                                  "computeExchangeHash_H", "F: ",
                                  serverF.toByteArray());
        transport.getLog().debug2("SSH2KEXDHGroup14SHA1",
                                  "computeExchangeHash_H", "K: ",
                                  sharedSecret_K);
        transport.getLog().debug2("SSH2KEXDHGroup14SHA1",
                                  "computeExchangeHash_H", "Hash over: ",
                                  buf.getData(), 0, buf.getWPos());
        transport.getLog().debug2("SSH2KEXDHGroup14SHA1",
                                  "computeExchangeHash_H", "H: ",
                                  exchangeHash_H);
    }

    protected void computeSharedSecret_K(DHPublicKeySpec peerPubSpec)
    throws SSH2Exception {
        try {
            KeyFactory   dhKeyFact = KeyFactory.getInstance("DH");
            KeyAgreement dhKEX     = KeyAgreement.getInstance("DH");

            DHPublicKey  peerPubKey =
                (DHPublicKey)dhKeyFact.generatePublic(peerPubSpec);

            dhKEX.init(dhPrivateKey);
            dhKEX.doPhase(peerPubKey, true);

            sharedSecret_K = dhKEX.generateSecret();
        } catch (Exception e) {

            e.printStackTrace();

            throw new SSH2FatalException("Error computing shared secret: "
                                         + e);
        }
    }

    protected void sendDHINIT(int type) throws SSH2Exception {
        SSH2TransportPDU pdu =
            SSH2TransportPDU.createOutgoingPacket(type);

        pdu.writeBigInt(dhPublicKey.getY());

        transport.transmitInternal(pdu);
    }

    protected MessageDigest createHash() throws SSH2Exception {
        try {
            return MessageDigest.getInstance("SHA1");
        } catch (Exception e) {
            throw new SSH2KEXFailedException("SHA1 not implemented", e);
        }
    }

    protected void generateDHKeyPair(DHParameterSpec dhParams) throws SSH2Exception {
        try {
            KeyPairGenerator dhKeyPairGen = KeyPairGenerator.getInstance("DH");

            dhKeyPairGen.initialize(dhParams, transport.getSecureRandom());

            KeyPair dhKeyPair = dhKeyPairGen.generateKeyPair();

            dhPrivateKey = (DHPrivateKey)dhKeyPair.getPrivate();
            dhPublicKey  = (DHPublicKey)dhKeyPair.getPublic();
        } catch (Exception e) {
            throw new SSH2FatalException("Error generating DiffieHellman keys: "
                                         + e);
        }
    }

}
