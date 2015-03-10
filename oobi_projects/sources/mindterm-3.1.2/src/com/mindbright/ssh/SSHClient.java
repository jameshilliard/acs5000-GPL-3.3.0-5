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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.math.BigInteger;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Vector;

import com.mindbright.jca.security.MessageDigest;
import com.mindbright.jca.security.SignatureException;
import com.mindbright.jca.security.interfaces.RSAPrivateCrtKey;
import com.mindbright.jca.security.interfaces.RSAPublicKey;
import com.mindbright.net.SocketFactory;
import com.mindbright.security.publickey.RSAAlgorithm;
import com.mindbright.terminal.TerminalWindow;
import com.mindbright.util.SecureRandomAndPad;

/**
 * This class contains the main functionality for setting up a connection to a
 * ssh-server. It can be used both to implement a "full" ssh-client, or it can
 * be used to fire off a single command on the server (both in a background
 * thread and in the current-/foreground-thread). A set of properties can be
 * used to control different aspects of the connection. These are fetched from
 * an object implementing the <code>SSHClientUser</code>-interface.  The
 * authentication can be done in different ways, all which is handled through an
 * object implementing the <code>SSHAuthenticator</code>-interface. The
 * console-output of the <code>SSHClient</code> is (optionally) handled through
 * an object implementing the <code>SSHConsole</code>-interface.  <p>
 *
 * A class realizing a full interactive ssh-client is
 * <code>SSHInteractiveClient</code>. 
 *
 * @author  Mats Andersson
 * @version 0.96, 26/11/98
 * @see     SSHAuthenticator
 * @see     SSHClientUser
 * @see     SSHConsole 
 */
public class SSHClient extends SSH {

    static public class AuthFailException extends IOException {
        public AuthFailException(String msg) {
            super(msg);
        }
        public AuthFailException() {
            this("permission denied");
        }
    }

    static public class ExitMonitor implements Runnable {
        SSHClient client;
        long      msTimeout;
        public ExitMonitor(SSHClient client, long msTimeout) {
            this.msTimeout  = msTimeout;
            this.client     = client;
        }
        public ExitMonitor(SSHClient client) {
            this(client, 0);
        }
        public void run() {
            client.waitForExit(msTimeout);
            // If we have allready exited gracefully don't report...
            //
            if(!client.gracefulExit)
                client.disconnect(false);
        }
    }

    private class KeepAliveThread extends Thread {
        int interval;
        public KeepAliveThread(int i) {
            super("SSH1KeepAlive");
            interval = i;
        }
        public synchronized void setInterval(int i) {
            interval = i;
        }
        public void run() {
            int i;
            SSHPduOutputStream ignmsg;
            while(true) {
                try {
                    synchronized(this) {
                        i = interval;
                    }
                    sleep(1000 * i);
                    if(SSHClient.this.controller != null) {
                        ignmsg = new SSHPduOutputStream(MSG_DEBUG,
                                                        controller.sndCipher,
                                                        controller.sndComp,
                                                        rand);
                        ignmsg.writeString("heartbeat");
                        controller.transmit(ignmsg);
                    }
                } catch (Exception e) {
                    // !!!
                }
            }
        }
    }

    // Local port forwarding
    //
    public static class LocalForward {
        protected String localHost;
        protected int    localPort;
        protected String remoteHost;
        protected int    remotePort;
        protected String plugin;
        public LocalForward(String localHost, int localPort, String remoteHost, int remotePort, String plugin) {
            this.localHost  = localHost;
            this.localPort  = localPort;
            this.remoteHost = remoteHost;
            this.remotePort = remotePort;
            this.plugin     = plugin;
        }
    }

    // Remote port forwarding
    //
    public static class RemoteForward {
        protected String remoteHost;
        protected int    remotePort;
        protected String localHost;
        protected int    localPort;
        protected String plugin;
        public RemoteForward(String remoteHost, int remotePort, String localHost, int localPort, String plugin) {
            this.remoteHost = remoteHost;
            this.remotePort = remotePort;
            this.localHost  = localHost;
            this.localPort  = localPort;
            this.plugin     = plugin;
        }
    }

    protected Thread             myThread;
    protected KeepAliveThread    keepAliveThread;
    protected SecureRandomAndPad rand;

    protected InetAddress serverAddr;
    protected InetAddress serverRealAddr = null;
    protected InetAddress localAddr;
    protected String      srvVersionStr;
    protected int         srvVersionMajor;
    protected int         srvVersionMinor;

    protected Vector localForwards;
    protected Vector remoteForwards;
    protected String commandLine;

    protected SSHChannelController controller;
    protected SSHConsole           console;
    protected SSHAuthenticator     authenticator;
    protected SSHClientUser        user;
    protected SSHInteractor        interactor;

    protected Socket               sshSocket;
    protected BufferedInputStream  sshIn;
    protected BufferedOutputStream sshOut;

    protected boolean gracefulExit;
    protected boolean isConnected;
    protected boolean isOpened;

    boolean usedOTP;

    protected int refCount;

    // !!! KLUDGE
    protected boolean havePORTFtp     = false;
    protected int     firstFTPPort    = 0;
    protected boolean activateTunnels = true;
    // !!! KLUDGE

    /**
     * Exit code. Added by Cyclades.
     */
    protected Integer exitStatus = null;

    public SSHClient(SSHAuthenticator authenticator, SSHClientUser user) {
        this.user           = user;
        this.authenticator  = authenticator;
        this.interactor     = user.getInteractor();
        this.srvVersionStr  = null;
        this.refCount       = 0;
        this.usedOTP        = false;

        try {
            this.localAddr = InetAddress.getByName("0.0.0.0");
        } catch (UnknownHostException e) {
            if(interactor != null)
                interactor.alert("FATAL: Could not create local InetAddress: " + e.getMessage());
        }
        clearAllForwards();
    }

    public void setConsole(SSHConsole console) {
        this.console = console;
        if(controller != null)
            controller.console = console;
    }

    public SSHConsole getConsole() {
        return console;
    }

    public InetAddress getServerAddr() {
        return serverAddr;
    }

    public InetAddress getServerRealAddr() {
        if(serverRealAddr == null)
            return serverAddr;
        return serverRealAddr;
    }

    public void setServerRealAddr(InetAddress realAddr) {
        serverRealAddr = realAddr;
    }

    public InetAddress getLocalAddr() {
        return localAddr;
    }

    public void setLocalAddr(String addr) throws UnknownHostException {
        localAddr = InetAddress.getByName(addr);
    }

    public String getServerVersion() {
        return srvVersionStr;
    }

    public void addLocalPortForward(int localPort, String remoteHost,
                                    int remotePort, String plugin)
        throws IOException {
        addLocalPortForward(localAddr.getHostAddress(), localPort,
                            remoteHost, remotePort, plugin);
    }

    public void addLocalPortForward(String localHost, int localPort,
                                    String remoteHost, int remotePort,
                                    String plugin)
        throws IOException {
        delLocalPortForward(localHost, localPort);
        localForwards.addElement(
            new LocalForward(localHost, localPort,
                             remoteHost, remotePort, plugin));
        if(isOpened) {
            try {
                requestLocalPortForward(localHost, localPort,
                                        remoteHost, remotePort, plugin);
            } catch(IOException e) {
                delLocalPortForward(localHost, localPort);
                throw e;
            }
        }
    }

    public void delLocalPortForward(String localHost, int port) {
        if(port == -1) {
            if(isOpened)
                controller.killListenChannels();
            localForwards = new Vector();
        } else {
            for(int i = 0; i < localForwards.size(); i++) {
                LocalForward fwd = (LocalForward) localForwards.elementAt(i);
                if(fwd.localPort == port && fwd.localHost.equals(localHost)) {
                    localForwards.removeElementAt(i);
                    if(isOpened)
                        controller.killListenChannel(fwd.localHost,
                                                     fwd.localPort);
                    break;
                }
            }
        }
    }

    public void addRemotePortForward(String remoteHost, int remotePort, 
                                     String localHost, int localPort,
                                     String plugin) {
        delRemotePortForward(remoteHost, remotePort);
        remoteForwards.addElement(
            new RemoteForward(remoteHost, remotePort,
                              localHost, localPort, plugin));
    }

    public void delRemotePortForward(String remoteHost, int port) {
        if(port == -1) {
            remoteForwards = new Vector();
        } else {
            for(int i = 0; i < remoteForwards.size(); i++) {
                RemoteForward fwd = (RemoteForward)remoteForwards.elementAt(i);
                if(fwd.remotePort==port && fwd.remoteHost.equals(remoteHost)){
                    remoteForwards.removeElementAt(i);
                    break;
                }
            }
        }
    }

    public void delRemotePortForward(String plugin) {
        for(int i = 0; i < remoteForwards.size(); i++) {
            RemoteForward fwd = (RemoteForward) remoteForwards.elementAt(i);
            if(fwd.plugin.equals(plugin)) {
                remoteForwards.removeElementAt(i);
                i--;
            }
        }
    }

    public void clearAllForwards() {
        this.localForwards  = new Vector();
        this.remoteForwards = new Vector();
    }

    public void startExitMonitor() {
        startExitMonitor(0);
    }

    public void startExitMonitor(long msTimeout) {
        Thread t = new Thread(new ExitMonitor(this, msTimeout));
        t.setName("ExitMonitor");
        t.start();
    }

    public synchronized int addRef() {
        return ++refCount;
    }

    public void forcedDisconnect() {
        if(controller != null) {
            controller.sendDisconnect("exit");
            controller.killAll();
        } else if (interactor != null) {
            interactor.disconnected(this, false);
        }
    }

    public synchronized int delRef() {
        if(--refCount <= 0) {
            forcedDisconnect();
            waitForExit(2000);
        }
        return refCount;
    }

    public void waitForExit(long msTimeout) {
        try {
            controller.waitForExit(msTimeout);
        } catch(InterruptedException e) {
            if(interactor != null)
                interactor.alert("Error when shutting down SSHClient: " + e.getMessage());
            controller.killAll();
        }
        try {
            if(sshSocket != null)
                sshSocket.close();
        } catch (IOException e) {
            // !!!
        }
    }

    public void doSingleCommand(String commandLine, boolean background, long msTimeout)
    throws IOException {
        this.commandLine = commandLine;

        // Cyclades change.
        this.exitStatus = null;

        bootSSH(false);
        if(background)
            startExitMonitor(msTimeout);
        else
            waitForExit(msTimeout);
    }

    public long getConnectTimeout() {
        return 60*1000;
    }
    public long getHelloTimeout() {
        return 10*1000;
    }

    public void bootSSH(boolean haveCnxWatch) throws IOException {
        bootSSH(haveCnxWatch, false);
    }

    public void bootSSH(boolean haveCnxWatch, boolean releaseConnection) throws IOException {
        try {
            myThread = Thread.currentThread();

            rand = secureRandom();

            // Give the interactor a chance to hold us until the user wants to
            // "connect" (e.g. with a dialog with server, username, password,
            // proxy-info)
            //
            if(interactor != null)
                interactor.startNewSession(this);

            // We first ask for the ssh server address since this might
            // typically be a prompt in the SSHClientUser
            //
            String serverAddrStr = user.getSrvHost();

            // When the SSHClientUser has reported which host to
            // connect to we report this to the interactor as
            // sessionStarted
            //
            if(interactor != null)
                interactor.sessionStarted(this);

            // It's the responsibility of the SSHClientUser to
            // establish a proxied connection if that is needed, the
            // SSHClient does not want to know about proxies. If a
            // proxy is not needed getProxyConnection() just returns
            // null.
            //
            sshSocket = user.getProxyConnection();

            if(sshSocket == null) {
                InetAddress serverAddresses[] =
                    InetAddress.getAllByName(serverAddrStr);
                for (int i=0; sshSocket == null && i<serverAddresses.length; i++) {
                    serverAddr = serverAddresses[i];
                    if (SSH.DEBUG) {
                        System.out.println("Connecting to " +
                                           serverAddr.getHostAddress() +
                                           ":" + user.getSrvPort());
                    }
                    sshSocket  = SocketFactory.newSocket
                        (serverAddr, user.getSrvPort(), 
                         getConnectTimeout());
                }
            }
            else {
                serverAddr = sshSocket.getInetAddress();
                if(interactor != null)
                    interactor.report("Connecting through proxy at "
                                      + serverAddr.getHostAddress() +
                                      ":" + sshSocket.getPort());
            }

            if(releaseConnection) {
                return;
            }

            boot(haveCnxWatch, sshSocket);

        } catch (IOException e) {
            if(sshSocket != null)
                sshSocket.close();
            disconnect(false);
            if(controller != null) {
                controller.killListenChannels();
            }
            controller = null;
            throw e;
        }
    }

    public void boot(boolean haveCnxWatch, Socket tpSocket)
        throws IOException {
        try {
            this.sshSocket = tpSocket;

            sshIn  = new BufferedInputStream(sshSocket.getInputStream(), 8192);
            sshOut = new BufferedOutputStream(sshSocket.getOutputStream());

            long hellotimeout = getHelloTimeout();
            int oldtimeout = tpSocket.getSoTimeout();
            if (hellotimeout > 0)
                tpSocket.setSoTimeout((int)hellotimeout);
            negotiateVersion();
            if (hellotimeout > 0)
                tpSocket.setSoTimeout(oldtimeout);

            // We now have a physical connection to a sshd, report this to the SSHClientUser
            //
            isConnected = true;
            if(interactor != null)
                interactor.connected(this);

            String userName = authenticator.getUsername(user);

            receiveServerData();

            initiatePlugins();

            cipherType = authenticator.getCipher(user);

            // Check that selected cipher is supported by server
            //
            if(!isCipherSupported(cipherType))
                throw new IOException("Sorry, server does not support the '" +
                                      getCipherName(authenticator.getCipher(user)) + "' cipher.");

            generateSessionId();
            generateSessionKey();

            initClientCipher();

            sendSessionKey(cipherType);

            // !!!
            // At this stage the communication is encrypted
            // !!!

            authenticateUser(userName);

            //
            //
            requestCompression(user.getCompressionLevel());

            controller = new SSHChannelController(this, sshIn, sshOut,
                                                  sndCipher, sndComp,
                                                  rcvCipher, rcvComp,
                                                  console, haveCnxWatch);

            initiateSession();

            if(console != null)
                console.serverConnect(controller, sndCipher);

            // We now open the SSH-protocol fully, report to SSHClientUser
            //
            isOpened = true;
            if(interactor != null)
                interactor.open(this);

            // Start "heartbeat" if needed
            //
            setAliveInterval(user.getAliveInterval());

            controller.start();

        } catch (IOException e) {
            if(sshSocket != null)
                sshSocket.close();
            disconnect(false);
            if(controller != null) {
                controller.killListenChannels();
            }
            controller = null;
            throw e;
        }
    }

    /**
     * Called from SSHChannelController to set the exit status.
     * Cyclades change.
     */
    protected void setExitStatus(int status) {
        exitStatus = new Integer(status);
    }
    
    /**
     * Returns the exit status if it was set or null otherwise.
     * Cyclades change.
     */
    public Integer getExitStatus() {
        return exitStatus;
    }
    
    protected void disconnect(boolean graceful) {
        if(!isConnected)
            return;
        isConnected   = false;
        isOpened      = false;
        gracefulExit  = graceful;
        srvVersionStr = null;
        setAliveInterval(0); // Stop "heartbeat"...
        if(interactor != null) {
            interactor.disconnected(this, graceful);
            String msg = "";
            if(sndComp != null || rcvComp != null) {
                for(int i = 0; i < 2; i++) {
                    SSHCompressor comp = (i == 0 ? sndComp : rcvComp);
                    if(comp != null) {
                        long compressed, uncompressed;
                        compressed   = comp.numOfCompressedBytes();
                        uncompressed = comp.numOfUncompressedBytes();
                        msg += (i == 0 ? "outgoing" : "\r\nincoming") +
                               " raw data (bytes) = " + uncompressed +
                               ", compressed = " + compressed + " (" +
                               ((compressed * 100) / uncompressed) + "%)";
                    }
                }
                interactor.report(msg);
            }
        }
        rcvComp = sndComp = null;
    }

    void negotiateVersion() throws IOException {
        byte[] buf = new byte[256];
        int    len;
        String verStr;

        try {
            len = sshIn.read(buf);
            srvVersionStr = new String(buf, 0, len);
        } catch (Throwable t) {
            throw new IOException(
                "Could not negotiate SSH version with server");
        }

        try {
            int l = srvVersionStr.indexOf('-');
            int r = srvVersionStr.indexOf('.');
            srvVersionMajor = Integer.parseInt(srvVersionStr.substring(l+1,r));
            l = r;
            r = srvVersionStr.indexOf('-', l);
            if(r == -1) {
                srvVersionMinor=Integer.parseInt(srvVersionStr.substring(l+1));
            } else {
                srvVersionMinor =
                    Integer.parseInt(srvVersionStr.substring(l + 1, r));
            }
        } catch (Throwable t) {
            throw new IOException("Server version string invalid: " +
                                  srvVersionStr);
        }

        if(srvVersionMajor > 1) {
            throw new IOException("This server doesn't support ssh1, connect" +
                                  " with ssh2 enabled");
        } else if(srvVersionMajor < 1 || srvVersionMinor < 5) {
            throw new IOException("Server's protocol version (" +
                                  srvVersionMajor + "-" + srvVersionMinor +
                                  ") is too old, please upgrade");
        }

        // Strip white-space
        srvVersionStr = srvVersionStr.trim();

        verStr = getVersionId(true);
        verStr += "\n";
        buf    = verStr.getBytes();

        sshOut.write(buf);
        sshOut.flush();
    }

    void receiveServerData() throws IOException {
        SSHPduInputStream pdu = new SSHPduInputStream(SMSG_PUBLIC_KEY, null, null);
        pdu.readFrom(sshIn);
        int bits;
        BigInteger e, n;

        srvCookie = new byte[8];
        pdu.read(srvCookie, 0, 8);

        bits = pdu.readInt();
        e = pdu.readBigInteger();
        n = pdu.readBigInteger();
        srvServerKey = new com.mindbright.security.publickey.RSAPublicKey(n, e);

        bits = pdu.readInt();
        e = pdu.readBigInteger();
        n = pdu.readBigInteger();
        srvHostKey = new com.mindbright.security.publickey.RSAPublicKey(n, e);

        int keyLenDiff = Math.abs(srvServerKey.getModulus().bitLength() -
                                  srvHostKey.getModulus().bitLength());

        if(keyLenDiff < 24) {
            throw new IOException("Invalid server keys, difference in sizes must be at least 24 bits");
        }

        if(!authenticator.verifyKnownHosts(srvHostKey)) {
            throw new IOException("Verification of known hosts failed");
        }

        protocolFlags      = pdu.readInt();
        supportedCiphers   = pdu.readInt();
        supportedAuthTypes = pdu.readInt();

        // OUCH: Support SDI patch from ftp://ftp.parc.xerox.com://pub/jean/sshsdi/
        // (we want the types to be in sequence for simplicity, kludge but simple)
        //
        if((supportedAuthTypes & (1 << 16)) != 0) {
            supportedAuthTypes = ((supportedAuthTypes & 0xffff) | (1 << AUTH_SDI));
        }
    }

    void generateSessionKey() {
        sessionKey = new byte[SESSION_KEY_LENGTH / 8];
        rand.nextBytes(sessionKey);
    }

    void sendSessionKey(int cipherType) throws IOException {
        byte[]             key = new byte[sessionKey.length + 1];
        BigInteger         encKey;
        SSHPduOutputStream pdu;

        key[0] = 0;
        System.arraycopy(sessionKey, 0, key, 1, sessionKey.length);

        for(int i = 0; i < sessionId.length; i++)
            key[i + 1] ^= sessionId[i];

        encKey = new BigInteger(key);

        int serverKeyByteLen = (srvServerKey.getModulus().bitLength() + 7) / 8;
        int hostKeyByteLen   = (srvHostKey.getModulus().bitLength() + 7) / 8;

        try {
            if(serverKeyByteLen < hostKeyByteLen) {
                BigInteger padded;
                padded = RSAAlgorithm.addPKCS1Pad(encKey, 2, serverKeyByteLen, rand);
                encKey = RSAAlgorithm.doPublic(padded,
                                               srvServerKey.getModulus(),
                                               srvServerKey.getPublicExponent());
                padded = RSAAlgorithm.addPKCS1Pad(encKey, 2, hostKeyByteLen, rand);
                encKey = RSAAlgorithm.doPublic(padded,
                                               srvHostKey.getModulus(),
                                               srvHostKey.getPublicExponent());
            } else {
                BigInteger padded;
                padded = RSAAlgorithm.addPKCS1Pad(encKey, 2, hostKeyByteLen, rand);
                encKey = RSAAlgorithm.doPublic(padded,
                                               srvHostKey.getModulus(),
                                               srvHostKey.getPublicExponent());
                padded = RSAAlgorithm.addPKCS1Pad(encKey, 2, serverKeyByteLen, rand);
                encKey = RSAAlgorithm.doPublic(padded,
                                               srvServerKey.getModulus(),
                                               srvServerKey.getPublicExponent());
            }
        } catch (SignatureException e) {
            throw new IOException(e.getMessage());
        }

        pdu = new SSHPduOutputStream(CMSG_SESSION_KEY, null, null, rand);
        pdu.writeByte((byte)cipherType);
        pdu.write(srvCookie, 0, srvCookie.length);
        pdu.writeBigInteger(encKey);
        // !!! TODO: check this pdu.writeInt(PROTOFLAG_SCREEN_NUMBER | PROTOFLAG_HOST_IN_FWD_OPEN);
        pdu.writeInt(protocolFlags);
        pdu.writeTo(sshOut);

        // !!!
        // At this stage the communication is encrypted
        // !!!

        if(!isSuccess())
            throw new IOException("Error while sending session key!");
    }

    void authenticateUser(String userName) throws IOException {
        SSHPduOutputStream outpdu;

        usedOTP = false;

        outpdu = new SSHPduOutputStream(CMSG_USER, sndCipher, sndComp, rand);
        outpdu.writeString(userName);
        outpdu.writeTo(sshOut);

        if(isSuccess()) {
            if(interactor != null)
                interactor.report("Authenticated directly by server, no other authentication required");
            return;
        }

        int[] authType = authenticator.getAuthTypes(user);

        for(int i = 0; i < authType.length; i++) {
            try {
                if(!isAuthTypeSupported(authType[i])) {
                    throw new AuthFailException("server does not support '" +
                                                authTypeDesc[authType[i]] + "'");
                }
                switch(authType[i]) {
                case AUTH_PUBLICKEY:
                    doRSAAuth(false, userName);
                    break;
                case AUTH_PASSWORD:
                    doPasswdAuth(userName);
                    break;
                case AUTH_RHOSTS_RSA:
                    doRSAAuth(true, userName);
                    break;
                case AUTH_TIS:
                    doTISAuth(userName);
                    break;
                case AUTH_RHOSTS:
                    doRhostsAuth(userName);
                    break;
                case AUTH_SDI:
                    doSDIAuth(userName);
                    usedOTP = true;
                    break;
                case AUTH_KERBEROS:
                case PASS_KERBEROS_TGT:
                default:
                    throw new IOException("We do not support selected authentication type " +
                                          authTypeDesc[authType[i]]);
                }
                return;
            } catch (AuthFailException e) {
                if(i == (authType.length - 1)) {
                    throw e;
                }
                if(interactor != null) {
                    interactor.report("Authenticating with " + authTypeDesc[authType[i]] + " failed, " +
                                      e.getMessage());
                }
            }
        }
    }

    void doPasswdAuth(String userName) throws IOException {
        SSHPduOutputStream outpdu;
        String password;

        password = authenticator.getPassword(user);

        outpdu = new SSHPduOutputStream(CMSG_AUTH_PASSWORD, sndCipher, sndComp,
                                        rand);
        outpdu.writeString(password);
        outpdu.writeTo(sshOut);

        if(!isSuccess())
            throw new AuthFailException();
    }

    void doRhostsAuth(String userName) throws IOException {
        SSHPduOutputStream outpdu;

        outpdu = new SSHPduOutputStream(CMSG_AUTH_RHOSTS, sndCipher, sndComp, rand);
        outpdu.writeString(userName);
        outpdu.writeTo(sshOut);

        if(!isSuccess())
            throw new AuthFailException();
    }

    void doTISAuth(String userName) throws IOException {
        SSHPduOutputStream outpdu;
        String prompt;
        String response;

        outpdu = new SSHPduOutputStream(CMSG_AUTH_TIS, sndCipher, sndComp, rand);
        outpdu.writeTo(sshOut);
        SSHPduInputStream inpdu = new SSHPduInputStream(MSG_ANY, rcvCipher, rcvComp);
        inpdu.readFrom(sshIn);

        if(inpdu.type == SMSG_FAILURE)
            throw new AuthFailException("TIS authentication server not reachable or user unknown");
        else if(inpdu.type != SMSG_AUTH_TIS_CHALLENGE)
            throw new IOException("Protocol error, expected TIS challenge but got " + inpdu.type);

        prompt = inpdu.readString();
        response = authenticator.getChallengeResponse(user, prompt);

        outpdu = new SSHPduOutputStream(CMSG_AUTH_TIS_RESPONSE, sndCipher, sndComp,
                                        rand);
        outpdu.writeString(response);
        outpdu.writeTo(sshOut);

        if(!isSuccess())
            throw new AuthFailException();
    }

    void doRSAAuth(boolean rhosts, String userName) throws IOException {
        SSHPduOutputStream outpdu;
        SSHRSAKeyFile      keyFile = authenticator.getIdentityFile(user);
        RSAPublicKey       pubKey  = keyFile.getPublic();

        if(rhosts) {
            outpdu = new SSHPduOutputStream(CMSG_AUTH_RHOSTS_RSA, sndCipher, sndComp,
                                            rand);
            outpdu.writeString(userName);
            outpdu.writeInt(pubKey.getModulus().bitLength());
            outpdu.writeBigInteger(pubKey.getPublicExponent());
            outpdu.writeBigInteger(pubKey.getModulus());
        } else {
            outpdu = new SSHPduOutputStream(CMSG_AUTH_RSA, sndCipher, sndComp,
                                            rand);
            outpdu.writeBigInteger(pubKey.getModulus());
        }
        outpdu.writeTo(sshOut);

        SSHPduInputStream inpdu = new SSHPduInputStream(MSG_ANY, rcvCipher, rcvComp);
        inpdu.readFrom(sshIn);
        if(inpdu.type == SMSG_FAILURE)
            throw new  AuthFailException("server refused our key" + (rhosts ? " or rhosts" : ""));
        else if(inpdu.type != SMSG_AUTH_RSA_CHALLENGE)
            throw new IOException("Protocol error, expected RSA-challenge but got " + inpdu.type);

        BigInteger challenge = inpdu.readBigInteger();

        // First try with an empty passphrase...
        //
        RSAPrivateCrtKey privKey = keyFile.getPrivate("");
        if(privKey == null)
            privKey = keyFile.getPrivate(authenticator.getIdentityPassword(user));
        else if(interactor != null)
            interactor.report("Authenticated with password-less rsa-key '" + keyFile.getComment() + "'");

        if(privKey == null)
            throw new AuthFailException("invalid password for key-file '" + keyFile.getComment() + "'");

        rsaChallengeResponse(privKey, challenge);
    }

    private final static int CANNOT_CHOOSE_PIN = 0;
    private final static int USER_SELECTABLE   = 1;
    private final static int MUST_CHOOSE_PIN   = 2;

    void doSDIAuth(String userName) throws IOException {
        SSHPduOutputStream outpdu;
        String password;

        password = authenticator.getChallengeResponse(user, userName +
                   "'s SDI token passcode: ");

        outpdu = new SSHPduOutputStream(CMSG_AUTH_SDI, sndCipher, sndComp,
                                        rand);
        outpdu.writeString(password);
        outpdu.writeTo(sshOut);

        SSHPduInputStream inpdu = new SSHPduInputStream(MSG_ANY, rcvCipher, rcvComp);
        inpdu.readFrom(sshIn);
        switch(inpdu.type) {
        case SMSG_SUCCESS:
            interactor.report("SDI authentication accepted.");
            break;

        case SMSG_FAILURE:
            throw new AuthFailException("SDI authentication failed.");

        case CMSG_ACM_NEXT_CODE_REQUIRED:
            password = interactor.promptPassword("Next token required: ");
            outpdu = new SSHPduOutputStream(CMSG_ACM_NEXT_CODE,
                                            sndCipher, sndComp, rand);
            outpdu.writeString(password);
            outpdu.writeTo(sshOut);
            if(!isSuccess())
                throw new AuthFailException();
            break;

        case CMSG_ACM_NEW_PIN_REQUIRED:
            if(!interactor.askConfirmation("New PIN required, do you want to continue?", false))
                throw new AuthFailException("new PIN not wanted");

            String type       = inpdu.readString();
            String size       = inpdu.readString();
            int    userSelect = inpdu.readInt();

            switch(userSelect) {
            case CANNOT_CHOOSE_PIN:
                break;

            case USER_SELECTABLE:
            case MUST_CHOOSE_PIN:
                String pwdChk;
                do {
                    password =
                        interactor.promptPassword("Please enter new PIN" +
                                                  " containing " + size +
                                                  " " + type);
                    pwdChk =
                        interactor.promptPassword("Please enter new PIN again");
                } while (!password.equals(pwdChk));

                outpdu = new SSHPduOutputStream(CMSG_ACM_NEW_PIN, sndCipher, sndComp,
                                                rand);
                outpdu.writeString(password);
                outpdu.writeTo(sshOut);

                inpdu = new SSHPduInputStream(MSG_ANY, rcvCipher, rcvComp);
                inpdu.readFrom(sshIn);
                if(inpdu.type != CMSG_ACM_NEW_PIN_ACCEPTED) {
                    throw new AuthFailException("PIN rejected by server");
                }
                throw new AuthFailException("new PIN accepted, " +
                                            "wait for the code on your token to change");

            default:
                throw new AuthFailException("invalid response from server");
            }

            break;

        case CMSG_ACM_ACCESS_DENIED:
            // Fall through
        default:
            throw new AuthFailException();
        }
    }

    void rsaChallengeResponse(RSAPrivateCrtKey privKey, BigInteger challenge)
    throws IOException {
        MessageDigest md5;

        try {
            challenge = RSAAlgorithm.doPrivateCrt(challenge,
                                                  privKey.getPrimeP(),
                                                  privKey.getPrimeQ(),
                                                  privKey.getPrimeExponentP(),
                                                  privKey.getPrimeExponentQ(),
                                                  privKey.getCrtCoefficient());
            challenge = RSAAlgorithm.stripPKCS1Pad(challenge, 2);
        } catch(Exception e) {
            throw new IOException(e.getMessage());
        }

        byte[] response = challenge.toByteArray();

        try {
            md5 = MessageDigest.getInstance("MD5");
            if(response[0] == 0)
                md5.update(response, 1, 32);
            else
                md5.update(response, 0, 32);
            md5.update(sessionId);
            response = md5.digest();
        } catch(Exception e) {
            throw new IOException("MD5 not implemented, can't generate session-id");
        }

        SSHPduOutputStream outpdu =	new SSHPduOutputStream(CMSG_AUTH_RSA_RESPONSE,
                                    sndCipher, sndComp,
                                    rand);
        outpdu.write(response, 0, response.length);
        outpdu.writeTo(sshOut);

        if(!isSuccess())
            throw new AuthFailException();
    }

    void initiateSession() throws IOException {
        if(user.wantPTY())
            requestPTY();

        int maxPktSz = user.getMaxPacketSz();
        if(maxPktSz > 0)
            requestMaxPacketSz(maxPktSz);

        if(user.wantX11Forward())
            requestX11Forward();

        if(activateTunnels)
            initiateTunnels();

        if(commandLine != null)
            requestCommand(commandLine);
        else
            requestShell();

        // !!!
        // At this stage we can't send more options/forward-requests
        // the server has now entered it's service-loop.
    }

    void initiatePlugins() {
        SSHProtocolPlugin.initiateAll(this);
    }

    void initiateTunnels() throws IOException {
        int i;
        for(i = 0; i < localForwards.size(); i++) {
            LocalForward fwd = (LocalForward) localForwards.elementAt(i);
            requestLocalPortForward(fwd.localHost, fwd.localPort, fwd.remoteHost, fwd.remotePort, fwd.plugin);
        }
        for(i = 0; i < remoteForwards.size(); i++) {
            RemoteForward fwd = (RemoteForward) remoteForwards.elementAt(i);
            requestRemotePortForward(fwd.remotePort, fwd.localHost, fwd.localPort, fwd.plugin);
        }
    }

    void requestCompression(int level) throws IOException {
        if(level == 0) {
            return;
        }

        SSHPduOutputStream outpdu =
            new SSHPduOutputStream(CMSG_REQUEST_COMPRESSION, sndCipher, sndComp,
                                   rand);

        outpdu.writeInt(level);
        outpdu.writeTo(sshOut);
        if(!isSuccess() && interactor != null)
            interactor.report("Error requesting compression level: " + level);

        sndComp = SSHCompressor.getInstance("zlib", SSHCompressor.COMPRESS_MODE,
                                            level);
        rcvComp = SSHCompressor.getInstance("zlib", SSHCompressor.UNCOMPRESS_MODE,
                                            level);
    }

    void requestMaxPacketSz(int sz) throws IOException {
        SSHPduOutputStream outpdu = new SSHPduOutputStream(CMSG_MAX_PACKET_SIZE,
                                    sndCipher, sndComp,
                                    rand);
        outpdu.writeInt(sz);
        outpdu.writeTo(sshOut);
        if(!isSuccess() && interactor != null)
            interactor.report("Error requesting max packet size: " + sz);
    }

    void requestX11Forward() throws IOException {
        SSHPduOutputStream outpdu =
            new SSHPduOutputStream(CMSG_X11_REQUEST_FORWARDING, sndCipher, sndComp,
                                   rand);

        // !!!
        outpdu.writeString("MIT-MAGIC-COOKIE-1");
        outpdu.writeString("112233445566778899aabbccddeeff00");
        outpdu.writeInt(0);
        // !!!

        outpdu.writeTo(sshOut);

        if(!isSuccess() && interactor != null)
            interactor.report("Error requesting X11 forward");
    }

    void requestPTY() throws IOException {
        SSHPduOutputStream outpdu =
            new SSHPduOutputStream(CMSG_REQUEST_PTY, sndCipher, sndComp, rand);
        TerminalWindow myTerminal = null;
        if(console != null)
            myTerminal = console.getTerminal();
        if(myTerminal != null) {
            outpdu.writeString(myTerminal.terminalType());
            outpdu.writeInt(myTerminal.rows());
            outpdu.writeInt(myTerminal.cols());
            outpdu.writeInt(myTerminal.vpixels());
            outpdu.writeInt(myTerminal.hpixels());
        } else {
            outpdu.writeString("");
            outpdu.writeInt(0);
            outpdu.writeInt(0);
            outpdu.writeInt(0);
            outpdu.writeInt(0);
        }
        outpdu.writeByte((byte)TTY_OP_END);
        outpdu.writeTo(sshOut);

        if(!isSuccess() && interactor != null)
            interactor.report("Error requesting PTY");
    }

    void requestLocalPortForward(String localHost, int localPort, String remoteHost, int remotePort, String plugin)
    throws IOException {
        controller.newListenChannel(localHost, localPort, remoteHost, remotePort, plugin);
    }

    void requestRemotePortForward(int remotePort, String localHost, int localPort, String plugin)
    throws IOException {

        try {
            SSHProtocolPlugin.getPlugin(plugin).remoteListener(remotePort, localHost, localPort,
                    controller);
        } catch (NoClassDefFoundError e) {
            throw new IOException("Plugins not available");
        }

        SSHPduOutputStream outpdu =
            new SSHPduOutputStream(CMSG_PORT_FORWARD_REQUEST, sndCipher, sndComp,
                                   rand);
        outpdu.writeInt(remotePort);
        outpdu.writeString(localHost);
        outpdu.writeInt(localPort);
        outpdu.writeTo(sshOut);

        if(!isSuccess() && interactor != null) {
            interactor.report("Error requesting remote port forward: " + plugin +
                              "/" + remotePort + ":" + localHost + ":" + localPort);

        }
    }

    void requestCommand(String command) throws IOException {
        SSHPduOutputStream outpdu =
            new SSHPduOutputStream(CMSG_EXEC_CMD, sndCipher, sndComp, rand);
        outpdu.writeString(command);
        outpdu.writeTo(sshOut);
    }

    void requestShell() throws IOException {
        SSHPduOutputStream outpdu =
            new SSHPduOutputStream(CMSG_EXEC_SHELL, sndCipher, sndComp, rand);
        outpdu.writeTo(sshOut);
    }

    boolean isSuccess() throws IOException {
        boolean success = false;
        SSHPduInputStream inpdu = null;
        inpdu = new SSHPduInputStream(MSG_ANY, rcvCipher, rcvComp);
        inpdu.readFrom(sshIn);
        if(inpdu.type == SMSG_SUCCESS)
            success = true;
        else if(inpdu.type == SMSG_FAILURE)
            success = false;
        else if(inpdu.type == MSG_DISCONNECT)
            throw new IOException("Server disconnected: " + inpdu.readString());
        else
            throw new IOException("Protocol error: got " + inpdu.type +
                                  " when expecting success/failure");
        return success;
    }

    void setInteractive() {
        try {
            sshSocket.setTcpNoDelay(true);
        } catch (SocketException e) {
            if(interactor != null)
                interactor.report("Error setting interactive mode: " + e.getMessage());
        }
    }

    void setAliveInterval(int i) {
        if(i == 0) {
            if(keepAliveThread != null && keepAliveThread.isAlive())
                keepAliveThread.stop();
            keepAliveThread = null;
        } else {
            if(keepAliveThread != null) {
                keepAliveThread.setInterval(i);
            } else {
                keepAliveThread = new KeepAliveThread(i);
                keepAliveThread.setDaemon(true);
                keepAliveThread.start();
            }
        }
    }

    public boolean isOpened() {
        return isOpened;
    }

    public boolean isConnected() {
        return isConnected;
    }

    void stdinWriteChar(char c) throws IOException {
        stdinWriteString(String.valueOf(c));
    }

    void stdinWriteString(String str) throws IOException {
        stdinWriteString(str.getBytes(), 0, str.length());
    }

    void stdinWriteString(byte[] str) throws IOException {
        stdinWriteString(str, 0, str.length);
    }

    void stdinWriteString(byte[] str, int off, int len) throws IOException {
        SSHPduOutputStream stdinPdu;
        if(isOpened && controller != null) {
            stdinPdu = new SSHPduOutputStream(SSH.CMSG_STDIN_DATA, sndCipher, sndComp,
                                              rand);
            stdinPdu.writeInt(len);
            stdinPdu.write(str, off, len);
            controller.transmit(stdinPdu);
        }
    }

    void signalWindowChanged(int rows, int cols, int vpixels, int hpixels) {
        if(isOpened && controller != null) {
            try {
                SSHPduOutputStream pdu;
                pdu = new SSHPduOutputStream(SSH.CMSG_WINDOW_SIZE, sndCipher, sndComp,
                                             rand);
                pdu.writeInt(rows);
                pdu.writeInt(cols);
                pdu.writeInt(vpixels);
                pdu.writeInt(hpixels);
                controller.transmit(pdu);
            } catch (Exception ex) {
                if(interactor != null)
                    interactor.alert("Error when sending sigWinch: " + ex.toString());
            }
        }
    }

}
