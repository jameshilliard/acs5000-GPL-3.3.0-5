/*
 * SSH2SessionImpl.java
 *
 * Created on 18 de Abril de 2006, 17:15
 *
 */

package com.mindterm.util.ssh;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.logging.Level;
import java.util.logging.Logger;

import com.mindbright.jca.security.SecureRandom;
import com.mindbright.ssh2.SSH2SCP1Client;
import com.mindbright.ssh2.SSH2SimpleClient;
import com.mindbright.ssh2.SSH2Transport;
import com.mindbright.util.RandomSeed;
import com.mindbright.util.SecureRandomAndPad;

/**
 * Class used to issue SSH and SCP commands.
 * @author Constantino
 */
public class SSH2SessionImpl implements SSHSession {

    /**
     * Logger for this class.
     */
    private static Logger log = Logger.getLogger(SSH2SessionImpl.class.getName());
    
    /**
     * Class used to do SCP.
     */
    private SSH2SCP1Client sshClient;
    
    /**
     * SSH transport.
     */
    private SSH2Transport transport;
    
    /**
     * Socket connection to the SSH server.
     */
    private Socket serverSocket;
    
    /**
     * Counter used to generated temporary file names.
     */
    private static int tmpCounter = 0;
    
    /**
     * Current class loader hash code used to generated temporary file names.
     */
    private static String classLoaderHashCode = 
            "" + SSH2SessionImpl.class.getClassLoader().hashCode();
    
    /**
     * Establishes a new SSH connection to the default port (22).
     */
    SSH2SessionImpl(String hostname, String username, String password) 
            throws Exception {
        this(hostname,username,password,SSH_PORT);
    }
    
    /**
     * Establishes a new SSH connection.
     * In a near future it will receive a certificate instead of a password.
     */
    SSH2SessionImpl(String hostname, String username, String password, int port) 
            throws Exception {
        
            serverSocket = new Socket(hostname, port);
            transport       = new SSH2Transport(serverSocket,createSecureRandom());
            
            SSH2SimpleClient client = new SSH2SimpleClient(transport,username,password);

            // there is no point in setting the stderr because when a command
            // is executed is is changed
            sshClient = new SSH2SCP1Client(
                    new File(System.getProperty("user.dir")), 
                    client.getConnection(), null, false);
        
    }
    
    /**
     * Close the conection.
     */
    public void disconnect() {
        
        if(sshClient != null) sshClient.close();
        
        if(transport != null) transport.normalDisconnect(MSG_DISCONNECT);
        
        try {
            serverSocket.close();
        } catch(Exception ex) {
            log.log(Level.WARNING,"error closing SSH socket",ex);
        }
    }

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
    public ExecResponse execute(String command, InputStream in, long timeout) throws TimeoutException, IOException {
                
        ByteArrayOutputStream bout = new ByteArrayOutputStream();
        ByteArrayOutputStream berr = new ByteArrayOutputStream();
        
        int rc = -1;
        String cmdPrefix = "";
        String cmdSufix = "";
        
        if (in != null) {
            String tmpFileName = createTempFileName();
            cmdPrefix = "cat " + tmpFileName + " | ";
            cmdSufix = " ; rm -f " + tmpFileName;
            
            sshClient.command("cat > " + tmpFileName, bout, berr);
            
            OutputStream out = sshClient.getStdIn();
            byte buffer[] = new byte[512];
            int n;
            while ((n = in.read(buffer)) >= 0) {
                out.write(buffer,0,n);
            }
            
            out.flush();
            out.close(); // send EOF
            try {
                rc = sshClient.waitForExitStatus(timeout);
            } catch (com.mindbright.sshcommon.TimeoutException ex) {
                throw new TimeoutException(ex.getMessage() + berr.toString(),ex);
            }
            if (rc != 0) {
                throw new IOException("error creating temporary file " + tmpFileName + 
                        " - " + berr.toString() + " - " + bout.toString());
            }
        }
        
        bout.reset();
        berr.reset();
        
        sshClient.command(cmdPrefix + command + cmdSufix, bout, berr);
        
        rc = -1;
        
        try {
            rc = sshClient.waitForExitStatus(timeout);
        } catch (com.mindbright.sshcommon.TimeoutException ex) {
            throw new TimeoutException(ex.getMessage(),ex);
        }
        
        return new ExecResponse(bout.toByteArray(),berr.toByteArray(),rc);
    }

    /**
     * Executes the given command waiting at most the given amount of time.
     */
    public ExecResponse execute(String command, long timeout) throws TimeoutException, IOException {
        return execute(command,null,timeout);
    }

    /**
     * Copy a file or files from the local machine to the remote machine.
     */
    public void sendFile(String localFile, String remoteFile, boolean recursive) 
            throws IOException {
        
        if(sshClient == null) {
            throw new NullPointerException("object not initialized properly");
        }
            
        sshClient.scp1().copyToRemote(localFile, remoteFile, recursive);
    }

    /**
     * Copy a file or files from the remote machine to the local machine.
     */
    public void receiveFile(String localFile, String remoteFile, boolean recursive) 
            throws IOException {
        
        if(sshClient == null) {
            throw new NullPointerException("object not initialized properly");
        }
        
        sshClient.scp1().copyToLocal(localFile, remoteFile, recursive);
    }
    
   /**
     * Create a random number generator. This implementation uses the
     * system random device if available to generate good random
     * numbers. Otherwise it falls back to some low-entropy garbage.
     */
    private SecureRandomAndPad createSecureRandom() {
   
        byte[] seed;
        File devRandom = new File("/dev/random");
        if (devRandom.exists()) {
            RandomSeed rs = new RandomSeed("/dev/random", "/dev/urandom");
            seed = rs.getBytesBlocking(20);
        } else {
            seed = RandomSeed.getSystemStateHash();
        }
        return new SecureRandomAndPad(new SecureRandom(seed));
    }

    /**
     * Create a temporary file name to be used when there is some data to be 
     * sent to the command's standard input.
     * The file name has a hash code of the current class loader, a timestamp
     * and a counter to make sure it is unique.
     */
    synchronized private String createTempFileName() {
        String name = "/tmp/tmp_" + classLoaderHashCode +
                "_" + System.currentTimeMillis() + "_" + tmpCounter;
        tmpCounter ++;
        return name;
    }
}
