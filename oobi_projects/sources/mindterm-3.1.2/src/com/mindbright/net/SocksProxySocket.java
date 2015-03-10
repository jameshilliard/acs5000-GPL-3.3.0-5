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

package com.mindbright.net;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;

public class SocksProxySocket {

    private final static String[] replyErrorV5 = {
                "Success",
                "General SOCKS server failure",
                "Connection not allowed by ruleset",
                "Network unreachable",
                "Host unreachable",
                "Connection refused",
                "TTL expired",
                "Command not supported",
                "Address type not supported"
            };

    private final static String[] replyErrorV4 = {
                "Request rejected or failed",
                "SOCKS server cannot connect to identd on the client",
                "The client program and identd report different user-ids"
            };

    private String proxyHost;
    private int    proxyPort;
    private String targetHost;
    private int    targetPort;
    private Socket socket;

    String     serverDesc;

    public String getServerDesc() {
        return serverDesc;
    }

    private SocksProxySocket(String targetHost, int targetPort,
                             String proxyHost, int proxyPort, long proxyTimeout)
    throws IOException, UnknownHostException {

        socket = SocketFactory.newSocket(proxyHost, proxyPort, proxyTimeout);

        this.proxyHost  = proxyHost;
        this.proxyPort  = proxyPort;
        this.targetHost = targetHost;
        this.targetPort = targetPort;
    }

    public static Socket getSocks4Proxy(String host, int port,
                                        String proxyHost,
                                        int proxyPort,
                                        ProxyAuthenticator auth,
                                        String userAgent)
    throws IOException, UnknownHostException {
        return getSocks4Proxy(host, port, proxyHost, proxyPort, 0,
                              auth.getProxyUsername(null, null));
    }

    public static Socket getSocks4Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort, 
                                        String userId)
        throws IOException, UnknownHostException {
        return getSocks4Proxy(targetHost, targetPort, proxyHost, proxyPort, 
                              0, userId);
    }
    
    public static Socket getSocks4Proxy(String host, int port,
                                        String proxyHost,
                                        int proxyPort,
                                        long proxyTimeout,
                                        ProxyAuthenticator auth,
                                        String userAgent)
    throws IOException, UnknownHostException {
        return getSocks4Proxy(host, port, proxyHost, proxyPort, proxyTimeout,
                              auth.getProxyUsername(null, null));
    }

    public static Socket getSocks4Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort, 
                                        long proxyTimeout,
                                        String userId)
        throws IOException, UnknownHostException {

        SocksProxySocket proxySocket =
            new SocksProxySocket(targetHost, targetPort, proxyHost, proxyPort, proxyTimeout);

        try {
            InputStream  proxyIn  = proxySocket.socket.getInputStream();
            OutputStream proxyOut = proxySocket.socket.getOutputStream();
            InetAddress  hostAddr = InetAddress.getByName(targetHost);

            ByteArrayOutputStream buf = new ByteArrayOutputStream();
            buf.write(0x04); // V4
            buf.write(0x01); // CONNECT
            buf.write((targetPort >>> 8) & 0xff);
            buf.write(targetPort & 0xff);
            buf.write(hostAddr.getAddress());
            buf.write(userId.getBytes());
            buf.write(0x00); // NUL terminate userid string
            proxyOut.write(buf.toByteArray());
            proxyOut.flush();

            int res = proxyIn.read();
            if(res == -1) {
                throw new IOException("SOCKS4 server " + proxyHost + ":" + proxyPort +
                                      " disconnected");
            }
            if(res != 0x00)
                throw new IOException("Invalid response from SOCKS4 server (" + res + ") " +
                                      proxyHost + ":" + proxyPort);

            int code = proxyIn.read();
            if(code != 90) {
                if(code > 90 && code < 93)
                    throw new IOException("SOCKS4 server unable to connect, reason: " +
                                          replyErrorV4[code - 91]);
                else
                    throw new IOException("SOCKS4 server unable to connect, reason: " +
                                          code);
            }

            byte[] data = new byte[6];

            if(proxyIn.read(data, 0, 6) != 6)
                throw new IOException("SOCKS4 error reading destination address/port");

            proxySocket.serverDesc = data[2]  + "." + data[3] +  "." + data[4] +
                                     "." + data[5] + ":" + ((data[0] << 8) | data[1]);

        } catch (SocketException e) {
            throw new SocketException("Error communicating with SOCKS4 server " +
                                      proxyHost + ":" + proxyPort + ", " +
                                      e.getMessage());
        }

        return proxySocket.socket;
    }

    public static Socket getSocks5Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort, 
                                        ProxyAuthenticator auth,
                                        String userAgent)
    throws IOException, UnknownHostException {
        return getSocks5Proxy(targetHost, targetPort, proxyHost, proxyPort,
                              0, false, auth);
    }

    public static Socket getSocks5Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort,
                                        ProxyAuthenticator authenticator)
        throws IOException, UnknownHostException {
        return getSocks5Proxy(targetHost, targetPort, proxyHost, proxyPort,
                              0, false, authenticator);
    }

    public static Socket getSocks5Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort,
                                        boolean localLookup,
                                        ProxyAuthenticator authenticator)
        throws IOException, UnknownHostException {
        return getSocks5Proxy(targetHost, targetPort, proxyHost, proxyPort,
                              0, localLookup, authenticator);
    }
    
    public static Socket getSocks5Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort, long proxyTimeout,
                                        ProxyAuthenticator auth,
                                        String userAgent)
    throws IOException, UnknownHostException {
        return getSocks5Proxy(targetHost, targetPort, proxyHost, proxyPort,
                              false, auth);
    }

    public static Socket getSocks5Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort, long proxyTimeout,
                                        ProxyAuthenticator authenticator)
        throws IOException, UnknownHostException {
        return getSocks5Proxy(targetHost, targetPort, proxyHost, proxyPort,
                              proxyTimeout, false, authenticator);
    }

    public static Socket getSocks5Proxy(String targetHost,
                                        int targetPort,
                                        String proxyHost,
                                        int proxyPort,
                                        long proxyTimeout,
                                        boolean localLookup,
                                        ProxyAuthenticator authenticator)
        throws IOException, UnknownHostException {
        SocksProxySocket proxySocket =
            new SocksProxySocket(targetHost, targetPort, proxyHost, proxyPort, proxyTimeout);

        try {
            InputStream  proxyIn  = proxySocket.socket.getInputStream();
            OutputStream proxyOut = proxySocket.socket.getOutputStream();

            // Simplest form, only no-auth and cleartext username/password
            //
            byte[] request = { (byte) 0x05, (byte) 0x02, (byte) 0x00, (byte) 0x02 };

            proxyOut.write(request);
            proxyOut.flush();

            int res = proxyIn.read();
            if(res == -1) {
                throw new IOException("SOCKS5 server " + proxyHost + ":" + proxyPort +
                                      " disconnected");
            }
            if(res != 0x05) {
                throw new IOException("Invalid response from SOCKS5 server (" + res + ") " +
                                      proxyHost + ":" + proxyPort);
            }

            int method = proxyIn.read();
            switch(method) {
            case 0x00:
                break;
            case 0x02:
                doAuthentication(proxyIn, proxyOut, authenticator, proxyHost, proxyPort);
                break;
            default:
                throw new IOException("SOCKS5 server does not support our authentication methods");
            }

            ByteArrayOutputStream buf = new ByteArrayOutputStream();

            if(localLookup) {
                // Request connect to targetHost (as 'ip-number') : targetPort
                //
                InetAddress hostAddr;
                try {
                    hostAddr = InetAddress.getByName(targetHost);
                } catch (UnknownHostException e) {
                    throw new IOException("Can't do local lookup on: " +
                                          targetHost +
                                          ", try socks5 without local lookup");
                }
                request = new byte[] { (byte) 0x05, (byte) 0x01, (byte) 0x00, (byte) 0x01 };
                buf.write(request);
                buf.write(hostAddr.getAddress());
            } else {
                // Request connect to targetHost (as 'domain-name') : targetPort
                //
                request = new byte[] { (byte) 0x05, (byte) 0x01, (byte) 0x00, (byte) 0x03 };
                buf.write(request);
                buf.write(targetHost.length());
                buf.write(targetHost.getBytes());
            }
            buf.write((targetPort >>> 8) & 0xff);
            buf.write(targetPort & 0xff);
            proxyOut.write(buf.toByteArray());
            proxyOut.flush();

            res = proxyIn.read();
            if(res != 0x05)
                throw new IOException("Invalid response from SOCKS5 server (" + res + ") " +
                                      proxyHost + ":" + proxyPort);

            int status = proxyIn.read();
            if(status != 0x00) {
                if(status > 0 && status < 9)
                    throw new IOException("SOCKS5 server unable to connect, reason: " +
                                          replyErrorV5[status]);
                else
                    throw new IOException("SOCKS5 server unable to connect, reason: " + status);
            }

            proxyIn.read(); // 0x00 RSV

            int aType = proxyIn.read();
            byte[] data = new byte[255];
            switch(aType) {
            case 0x01:
                if(proxyIn.read(data, 0, 4) != 4)
                    throw new IOException("SOCKS5 error reading address");
                proxySocket.serverDesc = data[0]  + "." + data[1] +  "." + data[2] +
                                         "." + data[3];
                break;
            case 0x03:
                int n = proxyIn.read();
                if(proxyIn.read(data, 0, n) != n)
                    throw new IOException("SOCKS5 error reading address");
                proxySocket.serverDesc = new String(data);
                break;
            default:
                throw new IOException("SOCKS5 gave unsupported address type: " + aType);
            }

            if(proxyIn.read(data, 0, 2) != 2)
                throw new IOException("SOCKS5 error reading port");
            proxySocket.serverDesc += ":" + ((data[0] << 8) | data[1]);

        } catch (SocketException e) {
            throw new SocketException("Error communicating with SOCKS5 server " +
                                      proxyHost + ":" + proxyPort + ", " +
                                      e.getMessage());
        }

        return proxySocket.socket;
    }

    private static void doAuthentication(InputStream proxyIn, OutputStream proxyOut,
                                         ProxyAuthenticator authenticator,
                                         String proxyHost, int proxyPort)
        throws IOException {

        String username = authenticator.getProxyUsername("SOCKS5", null);
        String password = authenticator.getProxyPassword("SOCKS5", null);

        ByteArrayOutputStream buf = new ByteArrayOutputStream();
        buf.reset();
        buf.write(0x01);
        buf.write(username.length());
        buf.write(username.getBytes());
        buf.write(password.length());
        buf.write(password.getBytes());
        proxyOut.write(buf.toByteArray());
        proxyOut.flush();

        int res = proxyIn.read();

        /*
         * We accept both 0x05 and 0x01 as version in the response here. 0x01
         * is the right response but some buggy servers will respond with 0x05
         * (i.e. not complying with rfc1929).
         */
        if(res != 0x01 && res != 0x05)
            throw new IOException("Invalid response from SOCKS5 server (" + res + ") " +
                                  proxyHost + ":" + proxyPort);

        if(proxyIn.read() != 0x00)
            throw new IOException("Invalid username/password for SOCKS5 server");
    }

    public String toString() {
        return "SocksProxySocket[addr=" + socket.getInetAddress() +
               ",port=" + socket.getPort() +
               ",localport=" + socket.getLocalPort() + "]";
    }
}
