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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketException;
import java.net.UnknownHostException;

/**
 * Socket that implements web proxy tunnelling (using CONNECT).
 *
 * Described in an Internet Draft (expired Feb. 1999) titled
 * 'Tunneling TCP based protocols through Web proxy servers'
 * by: Ari Luotonen.
 *
 * Proxy-authentication is described in RFC2616 and RFC2617.
 *
 * @author  Mats Andersson (originally by John Pallister)
 */
public class WebProxyTunnelSocket {

    private String proxyHost;
    private int    proxyPort;
    private String targetHost;
    private int    targetPort;
    private Socket socket;

    HttpHeader responseHeader;
    String     serverDesc;

    public HttpHeader getResponseHeader() {
        return responseHeader;
    }

    public String getServerDesc() {
        return serverDesc;
    }

    private WebProxyTunnelSocket(String host, int port,
                                 String proxyHost, int proxyPort, long proxyTimeout)
    throws IOException, UnknownHostException {
        socket = SocketFactory.newSocket(proxyHost, proxyPort, proxyTimeout);
        
        this.proxyHost  = proxyHost;
        this.proxyPort  = proxyPort;
        this.targetHost = targetHost;
        this.targetPort = targetPort;
    }

    public static Socket getProxy(
        String host, int port, String proxyHost, int proxyPort,
        ProxyAuthenticator authenticator, String userAgent)
    throws IOException, UnknownHostException {
        return getProxy(host, port, proxyHost, proxyPort, 0, 
                        null, authenticator, userAgent);
    }
    public static Socket getProxy(
        String host, int port, String proxyHost, int proxyPort, long proxyTimeout,
        ProxyAuthenticator authenticator, String userAgent)
    throws IOException, UnknownHostException {
        return getProxy(host, port, proxyHost, proxyPort, proxyTimeout,
                        null, authenticator, userAgent);
    }

    public static Socket getProxy(
        String host, int port, String proxyHost, int proxyPort,
        String protoStr, ProxyAuthenticator authenticator, String userAgent)
        throws IOException, UnknownHostException {
        return getProxy(host, port, proxyHost, proxyPort, 0,
                        protoStr, authenticator, userAgent);
    }
    
    public static Socket getProxy(
        String host, int port, String proxyHost, int proxyPort, long proxyTimeout,
        String protoStr, ProxyAuthenticator authenticator, String userAgent)
    throws IOException, UnknownHostException {
        WebProxyTunnelSocket proxySocket =
            new WebProxyTunnelSocket(host, port, proxyHost, proxyPort, proxyTimeout);
        int status = -1;
        String serverDesc;

        try {
            InputStream  proxyIn  = proxySocket.socket.getInputStream();
            OutputStream proxyOut = proxySocket.socket.getOutputStream();
            HttpHeader   requestHeader = new HttpHeader();

            if(protoStr == null)
                protoStr = "";

            requestHeader.setStartLine("CONNECT " + protoStr + host + ":" +
                                       port + " HTTP/1.0");
            requestHeader.setHeaderField("User-Agent", userAgent);
            requestHeader.setHeaderField("Pragma", "No-Cache");
            requestHeader.setHeaderField("Proxy-Connection", "Keep-Alive");

            requestHeader.writeTo(proxyOut);
            proxySocket.responseHeader = new HttpHeader(proxyIn);

            serverDesc = proxySocket.responseHeader.getHeaderField("server");

            // If proxy requires authentication
            //
            if(proxySocket.responseHeader.getStatus() == 407
                    && authenticator != null) {
                String method =proxySocket.responseHeader.getProxyAuthMethod();

                if ("basic".equalsIgnoreCase(method)) {
                    // OK
                } else if ("digest".equalsIgnoreCase(method)) {
                    throw new IOException("We don't support 'Digest' HTTP " +
                                          "Authentication");
                } else {
                    throw new IOException("Unknown HTTP Authentication " +
                                          "method '" + method + "'");
                }

                String realm = proxySocket.responseHeader.getProxyAuthRealm();

                if(realm == null)
                    realm = "";

                proxySocket.socket.close();
                proxySocket = new WebProxyTunnelSocket(host, port,
                                                       proxyHost, proxyPort,
                                                       proxyTimeout);
                proxyIn  = proxySocket.socket.getInputStream();
                proxyOut = proxySocket.socket.getOutputStream();

                String username =
                    authenticator.getProxyUsername("HTTP Proxy", realm);
                String password =
                    authenticator.getProxyPassword("HTTP Proxy", realm);

                requestHeader.setBasicProxyAuth(username, password);
                requestHeader.writeTo(proxyOut);
                proxySocket.responseHeader = new HttpHeader(proxyIn);
            }

            status = proxySocket.responseHeader.getStatus();

        } catch (SocketException e) {
            throw new SocketException("Error communicating with proxy server "+
                                      proxyHost + ":" + proxyPort + " (" +
                                      e.getMessage()  + ")");
        }

        if((status < 200) || (status > 299))
            throw new WebProxyException("Proxy tunnel setup failed: " +
                                        proxySocket.responseHeader.getStartLine());

        proxySocket.serverDesc = serverDesc;

        return proxySocket.socket;
    }

    public String toString() {
        return "WebProxyTunnelSocket[addr=" + socket.getInetAddress() +
               ",port=" + socket.getPort() +
               ",localport=" + socket.getLocalPort() + "]";
    }
}
