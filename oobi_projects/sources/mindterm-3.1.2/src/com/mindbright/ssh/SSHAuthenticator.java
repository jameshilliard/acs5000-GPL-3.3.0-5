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

package com.mindbright.ssh;

import java.io.IOException;

import com.mindbright.jca.security.interfaces.RSAPublicKey;

public interface SSHAuthenticator {
    public String        getUsername(SSHClientUser origin) throws IOException;
    public String        getPassword(SSHClientUser origin) throws IOException;
    public String        getChallengeResponse(SSHClientUser origin, String challenge) throws IOException;
    public int[]         getAuthTypes(SSHClientUser origin);
    public int           getCipher(SSHClientUser origin);
    public SSHRSAKeyFile getIdentityFile(SSHClientUser origin) throws IOException;
    public String        getIdentityPassword(SSHClientUser origin) throws IOException;
    public boolean       verifyKnownHosts(RSAPublicKey hostPub) throws IOException;
}
