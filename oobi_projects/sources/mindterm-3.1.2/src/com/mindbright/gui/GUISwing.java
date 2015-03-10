/******************************************************************************
 *
 * Copyright (c) 2005 AppGate Network Security AB. All Rights Reserved.
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

package com.mindbright.gui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

public class GUISwing implements GUIInterface {
    public GUISwing() { }
    
 // [CYCLADES-START]    
    /**
     * Added by Eduardo Murai Soares.
     * @see GUIInterface.showOptionPane for documentation
     */
    public int showOptionPane(Frame parent, String title, String message) {
    	int opt = JOptionPane.showConfirmDialog(parent, message, title, JOptionPane.YES_NO_CANCEL_OPTION,
    		JOptionPane.QUESTION_MESSAGE);
    	return opt;
    }
    // [CYCLADES-END]
	
    public void showAlert(String title, String message, Frame parent) {
        JOptionPane.showMessageDialog(parent, message, title,
                                      JOptionPane.ERROR_MESSAGE);
    }

    public boolean showConfirm(String title, String message,
                               int rows, int cols,
                               String yesLbl, String noLbl,
                               boolean defAnswer, Frame parent,
                               boolean xscroll, boolean yscroll) {
        final JDialog dialog = SwingConvenience.newBorderJDialog(parent,
                                                                 title, true);

        Component confirmText;

        if (rows == 0 || cols == 0) {
            confirmText = new JLabel(message);
        } else {
            JTextArea ta = new JTextArea(message);
            if (rows > 0) ta.setRows(rows);
            if (cols > 0) ta.setColumns(cols);
            ta.setEditable(false);
            if (!xscroll) {
                ta.setLineWrap(true);
                ta.setWrapStyleWord(true);
            }
            if (xscroll || yscroll) {
                confirmText = new JScrollPane
                    (ta, 
                     yscroll ? JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED : 
                     JScrollPane.VERTICAL_SCROLLBAR_NEVER,
                     xscroll ? JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED : 
                     JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            } else {
                confirmText = ta;
            }
            
        }
        dialog.getContentPane().add(confirmText, BorderLayout.CENTER);

        JButton yes = new JButton(yesLbl);
        JButton no = new JButton(noLbl);

        ActionListener al = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                ((JButton)e.getSource()).setEnabled(false); // ;^)
                dialog.dispose();
            }
        };
        
        yes.addActionListener(al);
        no.addActionListener(al);

        dialog.getContentPane().add(
            SwingConvenience.newButtonPanel(new JButton[] { yes, no }),
            BorderLayout.SOUTH);

        dialog.setResizable(true);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);

        if (defAnswer)
            yes.requestFocus();
        else
            no.requestFocus();

        dialog.addWindowListener(SwingConvenience.getWindowDisposer());
        dialog.setVisible(true);

        return !yes.isEnabled();
    }

    public void showNotice(Frame parent, String title, String text, 
                           int rows, int cols, boolean scrollbar) {
        JDialog dialog = SwingConvenience.newBorderJDialog(parent,
                                                           title, true);
        
        JTextArea ta = new JTextArea(text, rows, cols);
        Component comp;
        ta.setEditable(false);
        ta.setLineWrap(true);
        ta.setWrapStyleWord(true);
        if (scrollbar) {
            comp = new JScrollPane
                (ta, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                 JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        } else {
            comp = ta;
        }

        dialog.getContentPane().add(comp, BorderLayout.CENTER);

        JButton okBut = new JButton("OK");
        okBut.addActionListener(new AWTConvenience.CloseAction(dialog));

        JPanel p = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        p.add(okBut);
        dialog.getContentPane().add(p, BorderLayout.SOUTH);

        dialog.setResizable(true);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);
        okBut.requestFocus();
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());
        dialog.setVisible(true);
    }    

    public File selectFile(Frame parent, String title, String cwd,
                           String deffile, boolean save) {
        JFileChooser fc = new JFileChooser();
        fc.setDialogTitle(title);
        if (cwd != null) fc.setCurrentDirectory(new File(cwd));
        if (deffile != null) fc.setSelectedFile(new File(deffile));
        fc.setDialogType(save ?
                         JFileChooser.SAVE_DIALOG : JFileChooser.OPEN_DIALOG);
        int ret = fc.showOpenDialog(parent);
        return (ret == JFileChooser.APPROVE_OPTION) ?
            fc.getSelectedFile() : null;
    }

    private String textInput;
    public String textInput(String title, String message, Frame parent,
                            char echo, String defaultValue, String prompt) {
        final JTextField textTxtInp;
        final JDialog dialog = SwingConvenience.newBorderJDialog(parent,
                                                                 title, true);

        if (message != null && message.trim().length() > 0)
            dialog.getContentPane().add(
                new JLabel(message), BorderLayout.NORTH);

        dialog.getContentPane().add(new JLabel(prompt), BorderLayout.WEST);

        if (echo > (char)0) {
            JPasswordField pwd = new JPasswordField();
            pwd.setEchoChar(echo);
            textTxtInp = pwd;
        } else {
            textTxtInp = new JTextField();
        }
        textTxtInp.setText(defaultValue);
        textTxtInp.setColumns(10);
        dialog.getContentPane().add(textTxtInp, BorderLayout.CENTER);

        JButton okBut = new JButton("OK");
        ActionListener al;
        okBut.addActionListener(al = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if(e.getActionCommand().equals("OK")) {
                    if (textTxtInp instanceof JPasswordField) {
                        textInput = new String(((JPasswordField)textTxtInp).getPassword());
                    } else {
                        textInput = textTxtInp.getText();
                    }
                } else {
                    textInput = null;
                }
                dialog.dispose();
            }
        });
        JButton cancBut = new JButton("Cancel");
        cancBut.addActionListener(al);

        JPanel bp = SwingConvenience.newButtonPanel(
            new JButton[] { okBut, cancBut });
        dialog.getContentPane().add(bp, BorderLayout.SOUTH);

        dialog.setResizable(false);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());
        dialog.setVisible(true);

        return textInput;
    }

    private static boolean arrayequals(char a[], char b[]) {
        if (a == b) return true;
        if (a == null || b == null) return false;
        if (a.length != b.length) return false;
        for (int i=0; i<a.length; i++)
            if (a[i] != b[i]) return false;
        return true;
    }

    private String setPwdAnswer;
    public String setPassword(String title, String message, Frame parent) {
        final JPasswordField setPwdText, setPwdText2;
        final JDialog dialog = SwingConvenience.newBorderJDialog(
            parent, title, true);

        dialog.getContentPane().add(new JLabel(message), BorderLayout.NORTH);

        JPanel p = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();

        gbc.fill      = GridBagConstraints.NONE; 
        gbc.insets    = new Insets(10, 4, 2, 4);
        gbc.anchor    = GridBagConstraints.EAST;;
        gbc.gridwidth = GridBagConstraints.RELATIVE;
        gbc.weightx   = 0.0;
        p.add(new JLabel("Password"), gbc);
        
        setPwdText = new JPasswordField("", 12);
        setPwdText.setEchoChar('*');
        gbc.anchor    = GridBagConstraints.WEST;
        gbc.fill      = GridBagConstraints.HORIZONTAL;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx   = 1.0;
        p.add(setPwdText, gbc);

        gbc.fill      = GridBagConstraints.NONE; 
        gbc.insets    = new Insets(2, 4, 4, 4);
        gbc.anchor    = GridBagConstraints.EAST;;
        gbc.gridwidth = GridBagConstraints.RELATIVE;
        gbc.weightx   = 0.0;
        p.add(new JLabel("Password again"), gbc);

        setPwdText2 = new JPasswordField("", 12);
        setPwdText2.setEchoChar('*');
        gbc.anchor    = GridBagConstraints.WEST;
        gbc.fill      = GridBagConstraints.HORIZONTAL;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx   = 1.0;
        p.add(setPwdText2, gbc);

        dialog.getContentPane().add(p, BorderLayout.CENTER);

        JButton okBut = new JButton("OK");
        ActionListener al;
        okBut.addActionListener(al = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if (e.getActionCommand().equals("OK")) {
                    char[] a1 = setPwdText.getPassword();
                    char[] a2 = setPwdText2.getPassword();
                    if (arrayequals(a1, a2)) {
                        setPwdText.setText("");
                        setPwdText2.setText("");
                        setPwdText.requestFocus();
                        return;
                    }
                    setPwdAnswer = new String(a1);
                } else {
                    setPwdAnswer = null;
                }
                dialog.dispose();
            }
        });

        JButton cancBut = new JButton("Cancel");
        cancBut.addActionListener(al);

        JPanel bp = SwingConvenience.newButtonPanel(
            new JButton[] { okBut, cancBut });
        dialog.getContentPane().add(bp, BorderLayout.SOUTH);

        dialog.setResizable(false);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());
        dialog.setVisible(true);

        return setPwdAnswer;
    }
}
