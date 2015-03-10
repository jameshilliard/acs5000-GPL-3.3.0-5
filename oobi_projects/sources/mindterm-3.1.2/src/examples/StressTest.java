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

package examples;

import java.io.File;
import java.io.InputStream;
import java.io.IOException;
import java.io.OutputStream;

import java.net.Socket;
import java.util.Random;

import com.mindbright.jca.security.SecureRandom;

import com.mindbright.ssh2.*;

import com.mindbright.util.SecureRandomAndPad;
import com.mindbright.util.RandomSeed;

/*
 * Simple program to stress test communication over port
 * forwards. It creates a connection to a server (which should
 * have an ECHO service available), then starts <nforwards>
 * port forwards, and <nthreads> threads which in turn uses the
 * pool of port forwards to send (and retrieve) <iter> messages 
 * of random sizes to the ECHO service.
 *
 * Try running something like:
 *
 *  java -cp examples.jar examples.StressTest <server:port> <user> <pass> 16 40 100
 *
 */

public class StressTest {

    private static volatile int thrcount = 0;
    private static volatile long bcount = 0;

    public static void main(String[] argv) {

        if (argv.length < 6) {
            System.out.println("Usage: StressTest <server:port> <username> " + 
                               "<password> <nforwards> <nthreads> <iters>");
            System.exit(1);
        }

        try {
            String server = argv[0];
            String user   = argv[1];
            String passwd = argv[2];
            int    port   = 22;
            int nforwards = Integer.parseInt(argv[3]);
            int nthreads  = Integer.parseInt(argv[4]);
            final int iters = Integer.parseInt(argv[5]);
            int lports[] = new int[nforwards];

            int i = server.indexOf(':');
            if (i != -1) {
                String p = server.substring(i + 1);
                server = server.substring(0, i);
                port = Integer.parseInt(p);
            }

            /*
             * Connect to the server and authenticate using plain password
             * authentication (if other authentication method needed check
             * other constructors for SSH2SimpleClient).
             */
            SSH2Preferences prefs = new SSH2Preferences();
            prefs.setPreference(SSH2Preferences.CIPHERS_C2S, "arcfour");
            prefs.setPreference(SSH2Preferences.CIPHERS_S2C, "arcfour");
            Socket serverSocket = new Socket(server, port);
            SSH2Transport transport = new SSH2Transport
                (serverSocket, prefs, createSecureRandom());
            SSH2SimpleClient client = new SSH2SimpleClient
                (transport, user, passwd);
            
            /* Start port forward */
            SSH2Connection conn = client.getConnection();
            for (i=0; i<nforwards; i++) {
                SSH2Listener l = conn.newLocalForward("127.0.0.1", 0, "127.0.0.1", 7) ;
                lports[i] = l.getListenPort();
            }
            
            /*
             * Start threads that talks to remote end        
             */
            thrcount = nthreads;
            for (i=0; i<nthreads; i++) {
                final int lp = lports[i%nforwards];
                Thread t = new Thread(new Runnable() {
                    public void run() {
                        talk(lp, iters);
                    }}, "T" + i);
                t.start();
            }

            try {
                for (;;) {
                    Thread.sleep(1000);
                    if (thrcount <= 0) break;
                }
            } catch (Throwable t) {
                t.printStackTrace();
            }

            System.out.println("============================");
            System.out.println("Sum bytes="+bcount);
            /*
             * Disconnect the transport layer gracefully
             */
            transport.normalDisconnect("User disconnects");            
        } catch (Exception e) {
            System.err.println("An error occured: " + e);
            e.printStackTrace();
            System.exit(1);
        }
    }
    
    private static void talk(int port, int iters) {
        try {
            int sum = 0;
            Random r = new Random();
            Socket sock;
            InputStream in;
            OutputStream out;
            String s;
            byte [] b = new byte[256];
            byte [] c = new byte[256];
            for (int i=0; i<b.length; i++) b[i] = 'A';
            
            for (int i=0; i<iters; i++) {
                sock = new Socket("127.0.0.1", port);
                in = sock.getInputStream();
                out = sock.getOutputStream();
                int jj = 1 + r.nextInt(100);
                
                for (int j=0; j<jj; j++) {
                    int l = r.nextInt(40);
                    b[l] = '\n';
                    out.write(b, 0, l+1);
                        b[l] = 'A';
                    while (l >= 0) {
                        int rl = in.read(c, 0, l+1);
                        if (rl <= 0) throw new IOException("read short");
                        sum += rl;
                        l -= rl;
                    }
                }
                sock.close();
            }
            System.out.println(Thread.currentThread().getName() + "/" + 
                               port + ": bytes=" + sum);
            bcount += sum;
        } catch (Throwable t) {
            t.printStackTrace();
        }
        thrcount --;
    }
    
    /**
     * Create a random number generator. This implementation uses the
     * system random device if available to generate good random
     * numbers. Otherwise it falls back to some low-entropy garbage.
     */
    private static SecureRandomAndPad createSecureRandom() {
        byte[] seed;
        File devRandom = new File("/dev/urandom");
        if (devRandom.exists()) {
            RandomSeed rs = new RandomSeed("/dev/urandom", "/dev/urandom");
            seed = rs.getBytesBlocking(20);
        } else {
            seed = RandomSeed.getSystemStateHash();
        }
        return new SecureRandomAndPad(new SecureRandom(seed));
    }
}
