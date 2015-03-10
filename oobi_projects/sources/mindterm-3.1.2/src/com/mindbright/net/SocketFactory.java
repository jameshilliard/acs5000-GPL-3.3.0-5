/******************************************************************************
 *
 * Copyright (c) 2006 AppGate Network Security AB. All Rights Reserved.
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
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Vector;

public class SocketFactory {

    public static Socket newSocket(InetAddress ia, int port) throws IOException {
        return newSocket(ia, port, 0);
    }

    public static Socket newSocket(String host, int port) 
        throws UnknownHostException, IOException {
        return newSocket(host, port, 0);
    }

    public static Socket newSocket(String host, int port, long mstimeout) 
        throws UnknownHostException, IOException {
        return newSocket(InetAddress.getByName(host), port, 0);
    }

    public static Socket newSocket(InetAddress ia, int port, long mstimeout) throws IOException {
        if (mstimeout <= 0) return new Socket(ia, port);
        
        final Vector v = new Vector();
        final InetAddress iia = ia;
        final int iport = port;
        Thread connector = new Thread(new Runnable() {
            public void run() {
                Socket s;
                try {
                    s = new Socket(iia, iport);
                    v.addElement(s);
                } catch (IOException e) {
                    v.addElement(e);
                }
            }
        }, "SocketFactory.connector[" + ia + ", " + port + "]");
        connector.start();

        for (long t=0; t<mstimeout/100; t++) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException ie) {
                ie.printStackTrace();
            }
            
            if (v.size() > 0)
                break;
        }
        if (v.size() > 0) {
            Object o = v.firstElement();
            if (o instanceof Socket)
                return (Socket)o;
            throw (IOException)o;
        }
 
        try { 
            connector.stop();
        } catch (Throwable t){
        }
        
        throw new IOException("timeout when connecting");
    }    
}
