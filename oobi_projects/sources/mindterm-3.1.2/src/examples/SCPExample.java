/******************************************************************************
 *
 * Copyright (c) 2005-2006 AppGate Network Security AB. All Rights Reserved.
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

import java.net.Socket;

import com.mindbright.jca.security.SecureRandom;
import com.mindbright.util.SecureRandomAndPad;
import com.mindbright.util.RandomSeed;

import com.mindbright.sshcommon.SSHSCP1;

import com.mindbright.ssh2.SSH2Transport;
import com.mindbright.ssh2.SSH2SimpleClient;
import com.mindbright.ssh2.SSH2SCP1Client;

/**
 * Copy a file to/from an SSH2 server using the scp protocol. The scp
 * protocol was introduced with ssh1 and is very similar to the rcp
 * protocol. This example used the scp protocol over an ssh2 connection.
 * <p>
 * Usage:
 * <code> java -cp examples.jar examples.SCPExample
 * <em>server</em>[:<em>port</em>] <em>username</em> <em>password</em>
 * to|from <em>src_file</em> <em>dst_file</em>
 *
 * @see SSHSCP1
 * @see SFTPCopyFile
 */
public class SCPExample {

    /**
     * Run the application
     */
    public static void main(String[] argv) {
        if(argv.length < 6) {
            System.out.println
            ("Usage: SCPExample <server:port> <username> <password> to|from <src_file> <dst_file>");
            System.exit(1);
        }

        try {
            String server = argv[0];
            String user   = argv[1];
            String passwd = argv[2];
            int    port   = 22;

            int i = server.indexOf(':');
            if(i != -1) {
                String p = server.substring(i + 1);
                server = server.substring(0, i);
                port = Integer.parseInt(p);
            }

            /*
             * Connect to the server and authenticate using plain password
             * authentication (if other authentication method needed check
             * other constructors for SSH2SimpleClient).
             */
            Socket serverSocket   = new Socket(server, port);
            SSH2Transport transport = new SSH2Transport(serverSocket, 
                                                        createSecureRandom());
            SSH2SimpleClient client = new SSH2SimpleClient(transport,
                                      user, passwd);
            /*
             * Create SSH2SCP1 client which is used for the file transfer.
             */
            SSH2SCP1Client scpClient =
                new SSH2SCP1Client(new File(System.getProperty("user.dir")),
                                   client.getConnection(), System.err, false);

            /*
             * Extract more command line arguments
             */
            String direction = argv[3];
            String srcFile = argv[4];
            String dstFile = argv[5];

            /*
             * Convert the given paths to use '/' as separators
             */
            srcFile = srcFile.replace(File.separatorChar, '/');
            dstFile = dstFile.replace(File.separatorChar, '/');

            /*
             * Copy the given file
             */
            SSHSCP1 scp = scpClient.scp1();

            if (direction.equals("to")) {
                scp.copyToRemote(srcFile, dstFile, false);
            } else if (direction.equals("from")) {
                scp.copyToLocal(dstFile, srcFile, false);
            }
            
            /*
             * Disconnect the transport layer gracefully
             */
            transport.normalDisconnect("User disconnects");

        } catch (Exception e) {
            e.printStackTrace();
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
