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
import java.io.OutputStream;

import java.net.Socket;

import com.mindbright.jca.security.SecureRandom;
import com.mindbright.util.SecureRandomAndPad;
import com.mindbright.util.RandomSeed;

import com.mindbright.ssh2.SSH2Transport;
import com.mindbright.ssh2.SSH2SessionChannel;
import com.mindbright.ssh2.SSH2SimpleClient;

/**
 * Login and start a subsystem. Needs to be expanded to actually handle
 * the communication with the subsystem. See place marked XXX in the code.
 * <p>
 * Usage:
 * <code> java -cp examples.jar examples.SubSystem
 * <em>server</em>[:<em>port</em>] <em>username</em> <em>password</em>
 * <em>subsystem</em>
 *
 */
public class SubSystem {

    /**
     * Run the application
     */
    public static void main(String[] argv) {
        if(argv.length < 4) {
            System.out.println("Usage: SubSystem <server:port> <username> <password> <subsystem>");
            System.exit(1);
        }

        try {
            String server = argv[0];
            String user   = argv[1];
            String passwd = argv[2];
            int    port   = 22;
            String subsystem = argv[3];

            int i = server.indexOf(':');
            if(i != -1) {
                String p = server.substring(i + 1);
                server = server.substring(0, i);
                port = Integer.parseInt(p);
            }

            String cmdLine = "";
            for(i = 3; i < argv.length; i++) {
                cmdLine += argv[i] + " ";
            }
            cmdLine = cmdLine.trim();

            /*
             * Connect to the server and authenticate using plain password
             * authentication (if other authentication method needed check
             * other constructors for SSH2SimpleClient).
             */
            Socket serverSocket     = new Socket(server, port);
            SSH2Transport transport = new SSH2Transport(serverSocket,
                                      createSecureRandom());
            SSH2SimpleClient client = new SSH2SimpleClient(transport,
                                      user, passwd);


            SSH2SessionChannel session = client.getConnection().newSession();
            if (!session.doSubsystem(subsystem)) {
                throw new Exception("failed to start subsystem '" + subsystem + "'");
            }

            OutputStream stdin = session.getStdIn();
            InputStream stdout = session.getStdOut();

            /* XXX: Do something with the subsystem */


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
