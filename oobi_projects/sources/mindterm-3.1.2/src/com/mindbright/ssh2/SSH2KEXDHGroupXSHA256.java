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

package com.mindbright.ssh2;


import com.mindbright.jca.security.MessageDigest;

/**
 * Implements diffie hellman key exchange with group negotiation. This
 * algorithm is known as 'diffie-hellman-group-exchange-sha256'
 */
public class SSH2KEXDHGroupXSHA256 extends SSH2KEXDHGroupXSHA1 {

    protected MessageDigest createHash() throws SSH2Exception {
        try {
            return MessageDigest.getInstance("SHA256");
        } catch (Exception e) {
	    e.printStackTrace();
            throw new SSH2KEXFailedException("SHA256 not implemented", e);
        }
    }
}
