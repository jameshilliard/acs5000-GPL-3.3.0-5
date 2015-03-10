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

import java.awt.Frame;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import com.mindbright.gui.GUI;
import com.mindbright.jca.security.KeyPair;
import com.mindbright.jca.security.KeyPairGenerator;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.jca.security.interfaces.RSAPrivateCrtKey;
import com.mindbright.ssh2.SSH2Exception;
import com.mindbright.ssh2.SSH2KeyPairFile;
import com.mindbright.ssh2.SSH2PublicKeyFile;
import com.mindbright.terminal.TerminalWin;
import com.mindbright.terminal.TerminalWindow;

public class SSHKeyGenerationDialog {

    protected final static String EDIT_TITLE = "MindTerm - Select key file to edit";
    protected final static String PASS_TITLE = "MindTerm - File Password";
    
    protected final static String KEY_TYPES[] = {
        "DSA (ssh2)", "RSA (ssh2)", "RSA (ssh1)" 
    };
    protected final static String KEY_LENGTHS[] = {
        "768", "1024", "1536", "2048", "4096", "8192", "16384", "32768"
    };
    
    protected final static String LBL_BTN_GENERATE = "Generate";
    protected final static String LBL_BTN_CLOSE    = "Close";
    protected final static String LBL_BTN_BACK     = "Back";
    protected final static String LBL_BTN_SAVE     = "Save";
    protected final static String LBL_BTN_CANCEL   = "Cancel";

    protected final static String LBL_KEY_TYPE   = "Key type/format";
    protected final static String LBL_KEY_LENGTH = "Key length (bits)";
    protected final static String LBL_IDENTITY   = "Identity file";
    protected final static String LBL_DOT_DOT_DOT = "...";
    protected final static String LBL_PASSWORD   = "Password";
    protected final static String LBL_PASSWORD_AGAIN = "Password again";
    protected final static String LBL_COMMENT    = "Comment";
    protected final static String LBL_SUBJECT    = "Subject";
    protected final static String LBL_OPENSSH    = "OpenSSH .pub format";
    protected final static String LBL_DSA        = "DSA";
    protected final static String LBL_1024       = "1024";
    protected final static String LBL_SSHCOM     = "SSH Comm. private file format";

    protected final static String TEXT_GENERATING = "Generating keypair, please wait...";

    protected final static String keyGenerationHelp =
        "The key is generated using a random number generator, which " +
        "is seeded by mouse movement in the field containing this text. " +
        "Please move the mouse around in here until the progress bar below " +
        "registers 100%.\n" +
        "\n" +
        "This will create a publickey identity which can be used with the " +
        "publickey authentication method. Your identity will consist of two " +
        "parts: public and private keys. Your private key will be saved " +
        "in the location which you specify; the corresponding public key " +
        "is saved in a file with an identical name but with an extension of " +
        "'.pub' added to it.\n" +
        "\n" +
        "Your private key is protected by encryption, if you entered a " +
        "password. If you left the password field blank, the key will " +
        "not be encrypted. This should only be used in protected " +
        "environments where unattended logins are desired. The contents " +
        "of the 'comment' field are stored with your key, and displayed " +
        "each time you are prompted for the key's password.";

    protected final static String keyGenerationComplete =
        "Key Generation Complete\n\n" +
        "To use the key, you must transfer the '.pub' public key " +
        "file to an SSH server and add it to the set of authorized keys. " +
        "See your server documentation for details on this.\n\n" +
        "For convenience, your public key has been copied to the clipboard.\n\n" +
        "Examples:\n" +
        "In ssh2 the '.pub' file should be pointed out in the file " +
        "'authorization' in the config directory (e.g. ~/.ssh2)\n\n" +
        "In OpenSSH's ssh2 the contents of the '.pub' file should be added " +
        "to the file 'authorized_keys2' in your config directory (e.g. ~/.ssh) " +
        "on the server.\n\n" +
        "In ssh1 the contents of the '.pub' file should be added to the " +
        "file 'authorized_keys' in your ssh directory (e.g. ~/.ssh).\n\n" +
        "Press 'Back' to generate a new keypair.";

    protected static SSHInteractiveClient client;
    protected static Frame                parent;


    protected static File getSaveFile() {
        return GUI.selectFile(parent, "MindTerm - Select file to save identity to",
                              client.propsHandler.getSSHHomeDir(), true);
    }

    protected static void alert(String msg) {
        SSHMiscDialogs.alert("MindTerm - Alert", msg, parent);
    }

    protected static String getDefaultFileName() {
        try {
            String fn = client.propsHandler.getSSHHomeDir() + SSHPropertyHandler.DEF_IDFILE;
            File   f  = new File(fn);
            int    fi = 0;
            while(f.exists()) {
                fn = client.propsHandler.getSSHHomeDir() + SSHPropertyHandler.DEF_IDFILE + fi;
                f  = new File(fn);
                fi++;
            }
            fi--;
            return SSHPropertyHandler.DEF_IDFILE + (fi >= 0 ? String.valueOf(fi) : "");
        } catch (Throwable t) {
            // !!!
            // Don't care...
        }
        return "";
    }

    public static KeyPair generateKeyPair(String alg, int bits)
        throws NoSuchAlgorithmException {
        KeyPairGenerator kpg = KeyPairGenerator.getInstance(alg);
        kpg.initialize(bits, client.secureRandom());
        return kpg.generateKeyPair();
    }

    protected static void saveKeyPair(KeyPair kp, String passwd, String fileName, 
                                      String comment, String type, boolean openssh)
        throws IOException, SSH2Exception, NoSuchAlgorithmException {
        String subject   = client.propsHandler.getProperty("usrname");
        String pubKeyStr = null;

        if (subject == null) {
            subject = SSH.VER_MINDTERM;
        }

        if ("RSA (ssh1)".equals(type)) {
            RSAPrivateCrtKey key = (RSAPrivateCrtKey)kp.getPrivate();
            SSHRSAKeyFile.createKeyFile(client, key, 
                                        passwd, expandFileName(fileName), comment);
            SSHRSAPublicKeyString pks =
                new SSHRSAPublicKeyString("", comment,
                                          key.getPublicExponent(), key.getModulus());        
            pks.toFile(expandFileName(fileName) + ".pub");
            pubKeyStr = pks.toString();
        } else {
            SSH2PublicKeyFile pkif = new SSH2PublicKeyFile(kp.getPublic(),
                                                           subject, comment);

            // When key is unencrypted OpenSSH doesn't tolerate headers...
            //
            if(passwd == null || passwd.length() == 0) {
                subject = null;
                comment = null;
            }

            SSH2KeyPairFile kpf = new SSH2KeyPairFile(kp, subject, comment);

            kpf.store(expandFileName(fileName), client.secureRandom(), passwd);

            pubKeyStr = pkif.store(expandFileName(fileName + ".pub"), !openssh);
        }

        TerminalWindow term = client.sshStdIO.getTerminal();
        if (term instanceof TerminalWin) {
            ((TerminalWin)term).getClipboard().setSelection(pubKeyStr);
        }
    }

    protected static boolean checkValues(String passwd, String passwd2, 
                                         String fileName) {
        if (!passwd.equals(passwd2)) {
            alert("Please give same password twice");
            return false;
        }
        if (fileName.length() == 0) {
            alert("Filename can't be empty");
            return false;
        }

        OutputStream out = getOutput(fileName);
        if (out == null) {
            alert("Can't open '" + fileName + "' for saving.");
            return false;
        }
        try {
            out.close();
        } catch (Exception e) { /* don't care */
        }

        return true;
    }

    protected static OutputStream getOutput(String fileName) {
        try {
            return new FileOutputStream(expandFileName(fileName));
        } catch (Throwable t) {
        }
        return null;
    }

    protected static String expandFileName(String fileName) {
        if (fileName.indexOf(File.separator) == -1)
            fileName = client.propsHandler.getSSHHomeDir() + fileName;
        return fileName;
    }
}
