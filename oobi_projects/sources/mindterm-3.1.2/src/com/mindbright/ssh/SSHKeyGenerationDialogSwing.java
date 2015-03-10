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
import java.awt.CardLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import java.io.File;
import java.io.IOException;

import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.ScrollPaneConstants;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.AWTGridBagContainer;
import com.mindbright.gui.GUI;
import com.mindbright.gui.SwingConvenience;
import com.mindbright.jca.security.KeyPair;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.ssh2.SSH2AccessDeniedException;
import com.mindbright.ssh2.SSH2Exception;
import com.mindbright.ssh2.SSH2KeyPairFile;
import com.mindbright.ssh2.SSH2PublicKeyFile;
import com.mindbright.util.Progress;
import com.mindbright.util.RandomSeed;

public final class SSHKeyGenerationDialogSwing extends SSHKeyGenerationDialog {

    private static class ProgressBar extends JProgressBar implements Progress {
        ProgressBar(int max, int w, int h) {
            super(0, max);
        }

        public void progress(long value) {
            setValue((int)value);
        }
    }
    
    private static JComboBox    comboBits, comboType;
    private static JTextField   fileText;
    private static JPasswordField pwdText, pwdText2;
    private static JTextField  commText;
    private static JTextArea   descText;
    private static ProgressBar progBar;
    private static JButton     okBut;
    private static JCheckBox   cbOpenSSH;
    private static JPanel      cardPanel;
    private static CardLayout  cardLayout;
    private static boolean     generatedAndSaved;

    public static void show(String title, Frame par, SSHInteractiveClient cli){
        client = cli;
        parent = par;

        final JDialog dialog = SwingConvenience.newBorderJDialog(parent,
                                                                 title, true);

        createCardPanel();
        dialog.getContentPane().add(cardPanel, BorderLayout.CENTER);
        
        okBut = new JButton(LBL_BTN_GENERATE);
        okBut.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if(generatedAndSaved) {
                    resetValues();
                    descText.setCaretPosition(0);
                } else {
                    if(checkValues(new String(pwdText.getPassword()),
                                   new String(pwdText2.getPassword()),
                                   fileText.getText())) {
                        cardLayout.show(cardPanel, "second");
                        okBut.setEnabled(false);
                    }
                }
            }
        });
        JButton closeBut = new JButton(LBL_BTN_CLOSE);
        closeBut.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dialog.dispose();
                resetValues();
            }
        });

        dialog.getContentPane().add(SwingConvenience.newButtonPanel(
                                        new JButton[] { okBut, closeBut }),
                                    BorderLayout.SOUTH);

        resetValues();
        descText.setCaretPosition(0);
        
        dialog.setResizable(false);
        dialog.pack();

        RandomSeed seed = client.randomSeed();
        seed.addEntropyGenerator(descText);

        AWTConvenience.placeDialog(dialog);
        comboBits.requestFocus();
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());
        dialog.setVisible(true);
    }

    private static void createCardPanel() {
        cardPanel  = new JPanel();
        cardLayout = new CardLayout();
        cardPanel.setLayout(cardLayout);

        JPanel p = new JPanel(new BorderLayout(10, 10));

        descText = new JTextArea(keyGenerationHelp, 12, 34);
        descText.setEditable(false);
        descText.setLineWrap(true);
        descText.setWrapStyleWord(true);
        JScrollPane sp = new JScrollPane
            (descText, ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
             ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        p.add(sp, BorderLayout.CENTER);

        descText.addMouseMotionListener(new MouseMotionAdapter() {
            public void mouseMoved(MouseEvent e) {
                if (progBar.getValue() >= progBar.getMaximum()) {
                    try {
                        client.randomSeed().removeProgress();
                        progBar.setValue(0);
                        final int bits = Integer.valueOf
                            ((String)comboBits.getSelectedItem()).intValue();
                        final String alg = 
                            ((String)comboType.getSelectedItem()).substring(0,
                                                                            3);

                        descText.setText(TEXT_GENERATING);
                        descText.setCaretPosition(0);

                        Thread t = new Thread(new Runnable() {
                            public void run() {
                                try {
                                    KeyPair kp = generateKeyPair(alg, bits);
                                    saveKeyPair(kp);
                                    okBut.setEnabled(true);
                                    okBut.setLabel(LBL_BTN_BACK);
                                    descText.setText(keyGenerationComplete);
                                    descText.setCaretPosition(0);
                                    generatedAndSaved = true;
                                } catch (Throwable tt) {
                                    alert("Error while generating/saving key pair: " +
                                          tt.getMessage());
                                    cardLayout.show(cardPanel, "first");
                                }
                            }
                        });
                        t.start();

                    } catch (Throwable tt) {
                        alert("Error while generating/saving key pair: " +
                              tt.getMessage());
                        cardLayout.show(cardPanel, "first");
                    }
                }
            }
        });

        progBar = new ProgressBar(512, 150, 20);
        progBar.setStringPainted(true);
        p.add(progBar, BorderLayout.SOUTH);

        cardPanel.add(p, "second");

        p = new JPanel();
        AWTGridBagContainer grid = new AWTGridBagContainer(p);
        GridBagConstraints gbc = grid.getConstraints();

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_KEY_TYPE, JLabel.RIGHT), 0, 2);
        comboType = new JComboBox(KEY_TYPES);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(comboType, 0, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_KEY_LENGTH, JLabel.RIGHT), 1, 2);
        comboBits = new JComboBox(KEY_LENGTHS);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(comboBits, 1, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_IDENTITY, JLabel.RIGHT), 2, 2);
        fileText = new JTextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(fileText, 2, 2);

        JButton b = new JButton(LBL_DOT_DOT_DOT);
        b.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                File f = getSaveFile();
                if (f != null)
                    fileText.setText(f.getAbsolutePath());
            }
        });
        gbc.fill = GridBagConstraints.NONE;
        grid.add(b, 2, 1);

        gbc.fill = GridBagConstraints.HORIZONTAL;

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_PASSWORD, JLabel.RIGHT), 3, 2);
        pwdText = new JPasswordField("", 18);
        pwdText.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(pwdText, 3, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_PASSWORD_AGAIN, JLabel.RIGHT), 4, 2);
        pwdText2 = new JPasswordField("", 18);
        pwdText2.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(pwdText2, 4, 2);

        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_COMMENT, JLabel.RIGHT), 5, 2);
        commText = new JTextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(commText, 5, 2);

        cbOpenSSH = new JCheckBox(LBL_OPENSSH);
        grid.add(cbOpenSSH, 6, 4);

        cardPanel.add(p, "first");
    }

    private static void saveKeyPair(KeyPair kp)
        throws IOException, SSH2Exception, NoSuchAlgorithmException {

        saveKeyPair(kp, new String(pwdText.getPassword()), fileText.getText(), 
                    commText.getText(),
                    (String)comboType.getSelectedItem(), cbOpenSSH.isSelected());  

        okBut.setEnabled(true);
        pwdText.setText("");
        pwdText2.setText("");
        progBar.setValue(0);
        fileText.setText(getDefaultFileName());
    }

    private static void resetValues() {
        okBut.setEnabled(true);
        comboType.setSelectedIndex(0);
        comboBits.setSelectedItem("1024");
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
   
    private static JTextField  fileTextEd;
    private static JPasswordField  pwdTextEd, pwdText2Ed;
    private static JTextField  subjTextEd;
    private static JTextField  commTextEd;
    private static JLabel      typeLbl;
    private static JLabel      bitLbl;
    private static JCheckBox   cbOpenSSHEd;
    private static JCheckBox   cbSSHComEd;
    private static JButton     okButEd;
    private static JButton     cancButEd;

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

        final JDialog dialog = new JDialog(parent, title, true);

        AWTGridBagContainer grid = new AWTGridBagContainer(dialog.getContentPane());
        GridBagConstraints gbc = grid.getConstraints();
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_KEY_TYPE, JLabel.RIGHT), 0, 2);
        typeLbl = new JLabel(LBL_DSA);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(typeLbl, 0, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_KEY_LENGTH, JLabel.RIGHT), 1, 2);        
        bitLbl = new JLabel(LBL_1024);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(bitLbl, 1, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_IDENTITY, JLabel.RIGHT), 2, 2);        
        fileTextEd = new JTextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(fileTextEd, 2, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_PASSWORD, JLabel.RIGHT), 3, 2);
        pwdTextEd = new JPasswordField("", 18);
        pwdTextEd.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(pwdTextEd, 3, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_PASSWORD_AGAIN, JLabel.RIGHT), 4, 2);        
        pwdText2Ed = new JPasswordField("", 18);
        pwdText2Ed.setEchoChar('*');
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(pwdText2Ed, 4, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_SUBJECT, JLabel.RIGHT), 5, 2);
        subjTextEd = new JTextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(subjTextEd, 5, 2);
        
        gbc.anchor = GridBagConstraints.EAST;
        grid.add(new JLabel(LBL_COMMENT, JLabel.RIGHT), 6, 2);
        commTextEd = new JTextField("", 18);
        gbc.anchor = GridBagConstraints.WEST;
        grid.add(commTextEd, 6, 2);

        cbSSHComEd = new JCheckBox(LBL_SSHCOM);
        grid.add(cbSSHComEd, 7, 4);

        cbOpenSSHEd = new JCheckBox(LBL_OPENSSH);
        grid.add(cbOpenSSHEd, 8, 4);
        
        okButEd = new JButton(LBL_BTN_SAVE);
        okButEd.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String fName = fileTextEd.getText();
                String pwd   = new String(pwdTextEd.getPassword());
                if(checkValues(pwd, new String(pwdText2Ed.getPassword()), fName)) {
                    fName = expandFileName(fName);
                    try {
                        String s = subjTextEd.getText();
                        String c = commTextEd.getText();
                        pkf.setSubject(s);
                        pkf.setComment(c);
                        pkf.store(fName + ".pub", !cbOpenSSHEd.isSelected());
                        if(!cbSSHComEd.isSelected() &&
                           (pwd == null || pwd.length() == 0)) {
                            s = null;
                            c = null;
                        }
                        kpf.setSubject(s);
                        kpf.setComment(c);
                        kpf.store(fName, client.secureRandom(), pwd,
                                  cbSSHComEd.isSelected());
                        dialog.dispose();
                    } catch (Exception ee) {
                        alert("Error saving files: " + ee.getMessage());
                    }
                }
            }
        });

        cancButEd = new JButton(LBL_BTN_CANCEL);        
        cancButEd.addActionListener(new AWTConvenience.CloseAction(dialog));

        JPanel bp = SwingConvenience.newButtonPanel(new JButton[] { okButEd, cancButEd });
        grid.add(bp, 9, GridBagConstraints.REMAINDER);

        dialog.pack();

        fileTextEd.setText(fileName);
        pwdTextEd.setText(passwd);
        pwdText2Ed.setText(passwd);
        typeLbl.setText(kpf.getAlgorithmName());
        bitLbl.setText(String.valueOf(kpf.getBitLength()));
        subjTextEd.setText(kpf.getSubject());
        commTextEd.setText(kpf.getComment());
        cbSSHComEd.setSelected(kpf.isSSHComFormat());
        cbOpenSSHEd.setSelected(!pkf.isSSHComFormat());

        AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());
        dialog.setVisible(true);
    }
}
