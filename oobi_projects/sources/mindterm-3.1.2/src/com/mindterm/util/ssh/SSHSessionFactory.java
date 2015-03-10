/*
 * SSHSessionFactory.java
 *
 * Created on 18 de Abril de 2006, 17:24
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package com.mindterm.util.ssh;

/**
 *
 * @author Constantino
 */
public class SSHSessionFactory {
    
    private static SSHSessionFactory instance = new SSHSessionFactory();
    
    /** Creates a new instance of SSHSessionFactory */
    private SSHSessionFactory() {
    }
    
    public static SSHSessionFactory getInstance() {
        return instance;
    }
    
    public SSHSession createSession(String hostname, String username, String password) 
            throws Exception {
        return new SSH2SessionImpl(hostname,username,password);
    }
}
