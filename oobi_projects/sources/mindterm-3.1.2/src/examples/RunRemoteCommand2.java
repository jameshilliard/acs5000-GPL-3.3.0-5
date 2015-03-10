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

package examples;

import java.net.Socket;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;

import com.mindbright.jca.security.SecureRandom;
import com.mindbright.util.SecureRandomAndPad;
import com.mindbright.util.RandomSeed;

import com.mindbright.ssh2.SSH2Transport;
import com.mindbright.ssh2.SSH2Preferences;
import com.mindbright.ssh2.SSH2SimpleClient;
import com.mindbright.ssh2.SSH2ConsoleRemote;

/**
 * This is a variant of RunRemoteCommand which processes the stdio streams
 * slightly differently, otherwise it does exactly the same thing.
 * <p>
 * Usage:
 * <code> java -cp examples.jar examples.RunRemoteCommand2
 * <em>server</em>[:<em>port</em>] <em>username</em> <em>password</em>
 * <em>command_line</em>
 *
 * @see RemoteShellScript
 * @see RunRemoteCommand
 */
public class RunRemoteCommand2 {

    /**
     * Run the application
     */
    public static void main(String[] argv) {
        if(argv.length < 4) {
            System.out.println("usage: RunRemoteCommand2 <server[:port]> <username> <password> <command-line>");
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

            String cmdLine = "";
            for(i = 3; i < argv.length; i++) {
                cmdLine += argv[i] + " ";
            }
            cmdLine = cmdLine.trim();

            SSH2Preferences prefs = new SSH2Preferences();

            /*
             * Set some preferences, i.e. force usage of blowfish
             * cipher and set loglevel to 6 (debug) and log output to the
             * file ssh2out.log
             */
            prefs.setPreference(SSH2Preferences.CIPHERS_C2S, "blowfish-cbc");
            prefs.setPreference(SSH2Preferences.CIPHERS_S2C, "blowfish-cbc");
            prefs.setPreference(SSH2Preferences.LOG_LEVEL, "6");
            prefs.setPreference(SSH2Preferences.LOG_FILE, "ssh2out.log");

            /*
             * Connect to the server and authenticate using plain password
             * authentication (if other authentication method needed
             * check other constructors for SSH2SimpleClient).
             */
            Socket serverSocket     = new Socket(server, port);
            SSH2Transport transport = new SSH2Transport(serverSocket, prefs,
                                      createSecureRandom());
            SSH2SimpleClient client = new SSH2SimpleClient(transport,
                                      user, passwd);

            /*
             * Create the remote console to use for command execution. Here we
             * redirect stderr of all sessions started with this console to our
             * own stderr (NOTE: stdout is NOT redirected here but is instead
             * fetched below).
             */
            SSH2ConsoleRemote console =
                new SSH2ConsoleRemote(client.getConnection(), null,System.err);

            int exitStatus = -1;

            /*
             * Run the command. Here we don't redirect stdout and
             * stderr but use the internal streams of the session channel
             * instead.
             */
            if (console.command(cmdLine)) {
                /*
                 * Fetch the internal stdout stream and wrap it in a
                 * BufferedReader for convenience.
                 */
                BufferedReader stdout = new BufferedReader(
                    new InputStreamReader(console.getStdOut()));

                /*
                 * Read all output sent to stdout (line by line) and
                 * print it to our own stdout.
                 */
                String line;
                while((line = stdout.readLine()) != null) {
                    System.out.println(line);
                }

                /*
                 * Retrieve the exit status of the command (from the
                 * remote end).
                 */
                exitStatus = console.waitForExitStatus();
            } else {
                System.err.println("failed to execute command: " + cmdLine);
            }

            /*
             * Disconnect the transport layer gracefully
             */
            client.getTransport().normalDisconnect("User disconnects");

            /*
             * Exit with same status as remote command did
             */
            System.exit(exitStatus);

        } catch (Exception e) {
            System.err.println("An error occured: " + e);
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
