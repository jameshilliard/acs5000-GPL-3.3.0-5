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

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.CardLayout;
import java.awt.Checkbox;
import java.awt.Choice;
import java.awt.Dialog;
import java.awt.FileDialog;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextArea;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import java.io.File;
import java.io.IOException;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.AWTGridBagContainer;
import com.mindbright.gui.BorderDialog;
import com.mindbright.gui.GUI;
import com.mindbright.gui.ProgressBar;
import com.mindbright.jca.security.KeyPair;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.ssh2.SSH2AccessDeniedException;
import com.mindbright.ssh2.SSH2Exception;
import com.mindbright.ssh2.SSH2KeyPairFile;
import com.mindbright.ssh2.SSH2PublicKeyFile;
import com.mindbright.util.RandomSeed;

public final class SSHKeyGenerationDialogAWT extends SSHKeyGenerationDialog {

    private static Choice      choiceBits, choiceType;
    private static TextField   fileText;
    private static TextField   pwdText;
    private static TextField   pwdText2;
    private static TextField   commText;
    private static TextArea    descText;
    private static ProgressBar progBar;
    private static Button      okBut;
    private static Checkbox    cbOpenSSH;
    private static Panel       cardPanel;
    private static CardLayout  cardLayout;
    private static boolean     generatedAndSaved;

    public static void show(String title, Frame par, SSHInteractiveClient cli) {
        client = cli;
        parent = par;

        final Dialog dialog = new BorderDialog(parent, title, true);

        createCardPanel();
        dialog.add(cardPanel, BorderLayout.CENTER);
        
        okBut = new Button(LBL_BTN_GENERATE);
        okBut.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (generatedAndSaved) {
                    resetValues();
                    descText.setCaretPosition(0);
                } else {
                    if(checkValues(pwdText.getText(),
                                   pwdText2.getText(),
                                   fileText.getText())) {
                        cardLayout.show(cardPanel, "second");
                        okBut.setEnabled(false);
                    }
                }
            }
        });
        Button b = new Button(LBL_BTN_CLOSE);
        b.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dialog.dispose();
                resetValues();
            }
        });

        dialog.add(AWTConvenience.newButtonPanel
                   (new Button[] { okBut, b}), BorderLayout.SOUTH);
        dialog.addWindowListener(new AWTConvenience.CloseAdapter(b));

        AWTConvenience.setBackgroundOfChildren(dialog);
        
        dialog.setResizable(true);
        dialog.pack();

        resetValues();
        descText.setCaretPosition(0);

        RandomSeed seed = client.randomSeed();
        seed.addEntropyGenerator(descText);

        AWTConvenience.placeDialog(dialog);
        choiceBits.requestFocus();
        dialog.setVisible(true);
    }

    private static void createCardPanel() {
        cardPanel  = new Panel();
        cardLayout = new CardLayout();
        cardPanel.setLayout(cardLayout);

        Panel p = new Panel();
        AWTGridBagContainer grid = new AWTGridBagContainer(p);
        GridBagConstraints gbc = grid.getConstraints();

        gbc.fill = GridBagConstraints.BOTH;

        descText = new TextArea(keyGenerationHelp, 12, 34, TextArea.SCROLLBARS_VERTICAL_ONLY);
        descText.setEditable(false);
        grid.add(descText, 2, 8);
        descText.addMouseMotionListener(new MouseMotionAdapter() {
            public void mouseMoved(MouseEvent e) {
                if(progBar.isFinished()) {
                    try {
                        client.randomSeed().removeProgress();
                        progBar.setValue(0);
                        int     bits = Integer.valueOf(choiceBits.getSelectedItem()).intValue();
                        String  alg  = choiceType.getSelectedItem().substring(0, 3);

                        descText.setText(TEXT_GENERATING);
                        descText.setCaretPosition(0);
                        Thread.yield();

                        KeyPair kp = generateKeyPair(alg, bits);
                        saveKeyPair(kp);

                        okBut.setEnabled(true);
                        okBut.setLabel(LBL_BTN_BACK);
                        descText.setText(keyGenerationComplete);
                        descText.setCaretPosition(0);
                        generatedAndSaved = true;

                    } catch (Throwable t) {
                        alert("Error while generating/saving key pair: " +
                              t.getMessage());
                        cardLayout.show(cardPanel, "first");
                    }
                }
            }
        });

        gbc.fill   = GridBagConstraints.HORIZONTAL;
        gbc.anchor = GridBagConstraints.CENTER;

        progBar = new ProgressBar(512, 150, 20);
        grid.add(progBar, 3, 8);

        cardPanel.add(p, "second");

        p = new Panel();
        grid = new AWTGridBagContainer(p);
        gbc = grid.getConstraints();

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_KEY_TYPE, Label.RIGHT), 0, 2);
        choiceType = AWTConvenience.newChoice(KEY_TYPES);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(choiceType, 0, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_KEY_LENGTH, Label.RIGHT), 1, 2);
        choiceBits = AWTConvenience.newChoice(KEY_LENGTHS);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(choiceBits, 1, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_IDENTITY, Label.RIGHT), 2, 2);
        fileText = new TextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(fileText, 2, 2);

        Button b = new Button(LBL_DOT_DOT_DOT);
        b.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                File f = getSaveFile();
                if (f != null)
                    fileText.setText(f.getAbsolutePath());
            }
        });
        gbc.fill = GridBagConstraints.NONE;
        grid.add(b, 2, 1);

        gbc.fill   = GridBagConstraints.HORIZONTAL;
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_PASSWORD, Label.RIGHT), 3, 2);
        pwdText = new TextField("", 18);
        pwdText.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(pwdText, 3, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_PASSWORD_AGAIN, Label.RIGHT), 4, 2);
        pwdText2 = new TextField("", 18);
        pwdText2.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(pwdText2, 4, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_COMMENT, Label.RIGHT), 5, 2);
        commText = new TextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(commText, 5, 2);

        cbOpenSSH = new Checkbox(LBL_OPENSSH);
        gbc.fill = GridBagConstraints.NONE;
        grid.add(cbOpenSSH, 6, 4);

        cardPanel.add(p, "first");
    }

    private static void saveKeyPair(KeyPair kp)
        throws IOException, SSH2Exception, NoSuchAlgorithmException {

        saveKeyPair(kp, pwdText.getText(), fileText.getText(), commText.getText(),
                    choiceType.getSelectedItem(), cbOpenSSH.getState());  

        okBut.setEnabled(true);
        pwdText.setText("");
        pwdText2.setText("");
        progBar.setValue(0);
        fileText.setText(getDefaultFileName());
    }

    private static void resetValues() {
        okBut.setEnabled(true);
        choiceType.select(0);
        choiceBits.select("1024");
        fileText.setText(getDefaultFileName());
        generatedAndSaved = false;
        pwdText.setText("");
        pwdText2.setText("");
        descText.setText(keyGenerationHelp);
        okBut.setLabel(LBL_BTN_GENERATE);
        cardLayout.show(cardPanel, "first");

        RandomSeed seed = client.randomSeed();
        seed.resetEntropyCount();
        progBar.setValue(0);
        seed.addProgress(progBar);
    }
   
    private static FileDialog editKeyLoad;
    private static TextField  fileTextEd;
    private static TextField  pwdTextEd;
    private static TextField  pwdText2Ed;
    private static TextField  subjTextEd;
    private static TextField  commTextEd;
    private static Label      typeLbl;
    private static Label      bitLbl;
    private static Checkbox   cbOpenSSHEd;
    private static Checkbox   cbSSHComEd;
    private static Button     okButEd;
    private static Button     cancButEd;

    private static SSH2KeyPairFile   kpf;
    private static SSH2PublicKeyFile pkf;

    public static void editKeyDialog(String title, Frame par, SSHInteractiveClient cli) {
        parent = par;
        client = cli;

        File f = GUI.selectFile(parent, EDIT_TITLE, 
                                client.propsHandler.getSSHHomeDir(), null, false);

        String passwd   = null;
        String fileName = null;

        kpf = new SSH2KeyPairFile();

        if (f != null) {
            fileName = f.getAbsolutePath();
            try {
                pkf = new SSH2PublicKeyFile();
                pkf.load(fileName + ".pub");
            } catch (Exception e) {
                pkf = null;
            }
            boolean retryPasswd = false;
            do {
                try {
                    kpf.load(fileName, passwd);
                    break;
                } catch(SSH2AccessDeniedException e) {
                    /* Retry... */
                    retryPasswd = true;
                } catch(Exception e) {
                    alert("Error loading key file: " + e.getMessage());
                }
            } while((passwd = SSHMiscDialogs.password(PASS_TITLE,
                                                      "Please give password for " +
                                                      fileName,
                                                      parent)) != null);
            if(retryPasswd && passwd == null) {
                return;
            }
        } else {
            return;
        }

        if(pkf == null) {
            pkf = new SSH2PublicKeyFile(kpf.getKeyPair().getPublic(),
                                        kpf.getSubject(), kpf.getComment());
        }

        final Dialog dialog = new Dialog(parent, title, true);
        
        AWTGridBagContainer grid = new AWTGridBagContainer(dialog);
        GridBagConstraints gbc = grid.getConstraints();
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_KEY_TYPE, Label.RIGHT), 0, 2);        
        typeLbl = new Label(LBL_DSA);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(typeLbl, 0, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_KEY_LENGTH, Label.RIGHT), 1, 2);
        bitLbl = new Label(LBL_1024);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(bitLbl, 1, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_IDENTITY, Label.RIGHT), 2, 2);
        fileTextEd = new TextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(fileTextEd, 2, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_PASSWORD, Label.RIGHT), 3, 2);
        pwdTextEd = new TextField("", 18);
        pwdTextEd.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;        
        grid.add(pwdTextEd, 3, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_PASSWORD_AGAIN, Label.RIGHT), 4, 2);        
        pwdText2Ed = new TextField("", 18);
        pwdText2Ed.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(pwdText2Ed, 4, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_SUBJECT, Label.RIGHT), 5, 2);
        subjTextEd = new TextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(subjTextEd, 5, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new Label(LBL_COMMENT, Label.RIGHT), 6, 2);
        commTextEd = new TextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(commTextEd, 6, 2);
        
        cbSSHComEd = new Checkbox(LBL_SSHCOM);
        gbc.fill = GridBagConstraints.NONE;
        grid.add(cbSSHComEd, 7, 4);
        
        cbOpenSSHEd = new Checkbox(LBL_OPENSSH);
        grid.add(cbOpenSSHEd, 8, 4);
        
        okButEd = new Button(LBL_BTN_SAVE);
        okButEd.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String fName = fileTextEd.getText();
                String pwd   = pwdTextEd.getText();
                if(checkValues(pwd, pwdText2Ed.getText(),
                               fName)) {
                    fName = expandFileName(fName);
                    try {
                        String s = subjTextEd.getText();
                        String c = commTextEd.getText();
                        pkf.setSubject(s);
                        pkf.setComment(c);
                        pkf.store(fName + ".pub", !cbOpenSSHEd.getState());
                        if(!cbSSHComEd.getState() &&
                           (pwd == null || pwd.length() == 0)) {
                            s = null;
                            c = null;
                        }
                        kpf.setSubject(s);
                        kpf.setComment(c);
                        kpf.store(fName, client.secureRandom(), pwd,
                                  cbSSHComEd.getState());
                        dialog.dispose();
                    } catch (Exception ee) {
                        alert("Error saving files: " + ee.getMessage());
                    }
                }
            }
        });
        cancButEd = new Button(LBL_BTN_CANCEL);
        cancButEd.addActionListener(new AWTConvenience.CloseAction(dialog));

        Panel bp = AWTConvenience.newButtonPanel(new Button[] { okButEd, cancButEd });
        grid.add(bp, 9, GridBagConstraints.REMAINDER);
        
        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancButEd));
        
        AWTConvenience.setBackgroundOfChildren(dialog);
        AWTConvenience.setKeyListenerOfChildren(dialog,
                                                new AWTConvenience.OKCancelAdapter(okButEd, cancButEd),
                                                null);
        dialog.pack();

        fileTextEd.setText(fileName);
        pwdTextEd.setText(passwd);
        pwdText2Ed.setText(passwd);
        typeLbl.setText(kpf.getAlgorithmName());
        bitLbl.setText(String.valueOf(kpf.getBitLength()));
        subjTextEd.setText(kpf.getSubject());
        commTextEd.setText(kpf.getComment());
        cbSSHComEd.setState(kpf.isSSHComFormat());
        cbOpenSSHEd.setState(!pkf.isSSHComFormat());

        AWTConvenience.placeDialog(dialog);

        dialog.setVisible(true);
    }
}
