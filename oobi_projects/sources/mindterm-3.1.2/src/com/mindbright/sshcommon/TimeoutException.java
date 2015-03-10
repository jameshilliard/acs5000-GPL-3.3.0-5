/*
 * TimeoutException.java
 *
 * Created on April 3, 2006, 11:43 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package com.mindbright.sshcommon;

/**
 * Exception thrown when a command does not finish in the given period of time.
 * @author Constantino
 */
public class TimeoutException extends Exception {
    
    /** Creates a new instance of TimeoutException */
 
    public TimeoutException() {
        super();
    }

    public TimeoutException(String message) {
        super(message);
    }
    
    public TimeoutException(String message, Throwable cause) {
        super(message,cause);
    }

    public TimeoutException(Throwable cause) {
        super(cause);
    }

}
