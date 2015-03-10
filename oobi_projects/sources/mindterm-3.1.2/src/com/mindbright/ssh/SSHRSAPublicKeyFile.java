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

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.math.BigInteger;
import java.util.Enumeration;
import java.util.Vector;

import com.mindbright.security.publickey.RSAPublicKey;

public class SSHRSAPublicKeyFile {

    Vector pubKeyList;

    public SSHRSAPublicKeyFile(InputStream fileIn, String name, boolean hostFile) throws IOException {
        BufferedReader  reader = new BufferedReader(new InputStreamReader(fileIn));
        String          row;

        pubKeyList = new Vector();

        while((row = reader.readLine()) != null) {
            row = row.trim();
            if(row.equals("") || row.charAt(0) == '#') // Skip comment-lines and empty lines...
                continue;
            String opts;
            if(hostFile) {
                // If we are reading a 'known_hosts' file we know that first token of line is the host-addr.
                // in this case we store the host-addr in the opts-field of the SSHRSAPublicKeyString
                //
                int i = row.indexOf(' ');
                opts = row.substring(0, i);
                row  = row.substring(i);
            } else {
                opts = ""; // !!! Read options from start of line, we don't support options for now...
            }
            try {
                SSHRSAPublicKeyString pubKey = SSHRSAPublicKeyString.createKey(opts, row);
                pubKeyList.addElement(pubKey);
            } catch (Exception e) {
                throw new IOException("Corrupt public keys file: " + name);
            }
        }
    }

    public static SSHRSAPublicKeyFile loadFromFile(String name, boolean hostFile) throws IOException {
        FileInputStream fileIn = new FileInputStream(name);
        SSHRSAPublicKeyFile keyFile = new SSHRSAPublicKeyFile(fileIn, name, hostFile);
        fileIn.close();
        return keyFile;
    }

    public void saveToFile(String fileName) throws IOException {
        FileWriter            fileOut = new FileWriter(fileName);
        BufferedWriter        writer  = new BufferedWriter(fileOut);
        SSHRSAPublicKeyString pk      = null;
        Enumeration           elmts   = elements();
        String                row;

        try {
            while(elmts.hasMoreElements()) {
                pk = (SSHRSAPublicKeyString) elmts.nextElement();
                row = pk.toString();
                writer.write(row, 0, row.length());
                writer.newLine();
            }
        } catch (Exception e) {
            throw new IOException("Error while writing public-keys-file: " + fileName);
        }
        writer.flush();
        writer.close();
        fileOut.close();
    }

    public Enumeration elements() {
        return pubKeyList.elements();
    }

    public RSAPublicKey getPublic(BigInteger n, String user) {
        SSHRSAPublicKeyString pk = null;

        Enumeration e = pubKeyList.elements();
        while(e.hasMoreElements()) {
            pk = (SSHRSAPublicKeyString) e.nextElement();
            if(pk.getModulus().equals(n))
                break;
            pk = null;
        }

        return pk;
    }

    public int checkPublic(BigInteger n, String host) {
        SSHRSAPublicKeyString pk = null;
        int hostCheck = SSH.SRV_HOSTKEY_NEW;

        Enumeration e = pubKeyList.elements();
        while(e.hasMoreElements()) {
            pk = (SSHRSAPublicKeyString) e.nextElement();
            if(pk.getOpts().equals(host)) {
                if(pk.getModulus().equals(n)) {
                    hostCheck = SSH.SRV_HOSTKEY_KNOWN;
                } else {
                    hostCheck = SSH.SRV_HOSTKEY_CHANGED;
                }
                break;
            }
        }
        return hostCheck;
    }

    public void addPublic(String opts, String user, BigInteger e, BigInteger n) {
        SSHRSAPublicKeyString pubKey = new SSHRSAPublicKeyString(opts, user, e, n);
        pubKeyList.addElement(pubKey);
    }

    public void removePublic(String host) {
        SSHRSAPublicKeyString pk = null;

        Enumeration e = pubKeyList.elements();
        while(e.hasMoreElements()) {
            pk = (SSHRSAPublicKeyString) e.nextElement();
            if(pk.getOpts().equals(host)) {
                pubKeyList.removeElement(pk);
                break;
            }
        }
    }

    /* !!! DEBUG
    public static void main(String[] argv) {
      SSHRSAPublicKeyFile file = null;

      try {
        file = new SSHRSAPublicKeyFile("/home/mats/.ssh/known_hosts", true);

        SSHRSAPublicKeyString pk = null;

        Enumeration e = file.elements();
        while(e.hasMoreElements()) {
    pk = (SSHRSAPublicKeyString) e.nextElement();
    System.out.println(pk);
        }
      } catch (Exception e) {
        e.printStackTrace();
        System.out.println("Error: " + e.toString());
      }
    }
    */

}


