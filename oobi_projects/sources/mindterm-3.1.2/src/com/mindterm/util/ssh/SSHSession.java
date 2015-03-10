/*
 * SSHSession.java
 *
 * Created on April 13, 2006, 11:37 AM
 *
 * Copyright 2006, Avocent Corporation
 */

package com.mindterm.util.ssh;

import java.io.IOException;
import java.io.InputStream;

/**
 * Class used to issue SSH and SCP commands.
 * @author Constantino
 */
public interface SSHSession {

    /**
     * Default SSH port;
     */
    static int SSH_PORT = 22;
    
    /**
     * String sent when the transport is disconnected.
     */
    static String MSG_DISCONNECT = "Disconnected by user";
    
    /**
     * Close the conection.
     */
    public void disconnect();
    
    /**
     * Executes the given command waiting at most for the given amount of time.
     */
    public ExecResponse execute(String command, long timeout) 
        throws TimeoutException, IOException;
    
    /*
     * Method used when the given command needs to read something from the
     * standard input. This works with the creation of a temporary file in
     * the appliance that is then redirected to the given command.
     * It was not possible to send data directly to the command because
     * it would not be possible for the command to know when there is no more
     * data to read. That happens because if we close the output stream, that
     * would make the command receive an EOF, the entire connection is closed 
     * and then it is not possible to read the output of the command. I don't
     * know if this is something specific to Mindterm of if this is SSH regular
     * behaviour.
     */
    public ExecResponse execute(String command, InputStream in, long timeout)
        throws TimeoutException, IOException;
    
    /**
     * Copy a file or files from the local machine to the remote machine.
     */
    public void sendFile(String localFile, String remoteFile, boolean recursive) 
        throws IOException;
    
    /**
     * Copy a file or files from the remote machine to the local machine.
     */
    public void receiveFile(String localFile, String remoteFile, boolean recursive) 
        throws IOException;
}
