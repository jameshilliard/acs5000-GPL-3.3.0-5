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
import java.awt.Button;
import java.awt.Dialog;
import java.awt.FileDialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Label;
import java.awt.Panel;
import java.awt.TextArea;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

public class GUIAWT implements GUIInterface {
    public GUIAWT() { }

	 // [CYCLADES-START]    
    /**
     * Added by Eduardo Murai Soares.
     * @see GUIInterface.showOptionPane for documentation
     */
    public int showOptionPane(Frame parentComp, String title, String message) {
    	AlertDialog.show(title, "WARNING- This should not be called in AWT!", parentComp);
    	return -1;
    }
    // [CYCLADES-END]
	
    public void showAlert(String title, String message, Frame parent) {
        AlertDialog.show(title, message, parent);
    }

    public boolean showConfirm(String title, String message,
                               int rows, int cols,
                               String yesLbl, String noLbl,
                               boolean defAnswer, Frame parent,
                               boolean xscroll, boolean yscroll) {
        return ConfirmDialog.show(title, message, rows, cols, yesLbl, noLbl,
                                  defAnswer, parent, xscroll, yscroll);
    }

    public void showNotice(Frame parent, String title, String text, 
                           int rows, int cols, boolean scrollbar) {
        Dialog dialog = new BorderDialog(parent, title, true);

        TextArea textArea = new TextArea
            (text, rows, cols, scrollbar ? TextArea.SCROLLBARS_BOTH : TextArea.SCROLLBARS_NONE);
        textArea.setEditable(false);
        dialog.add(textArea, BorderLayout.CENTER);

        Button okBut = new Button("OK");
        okBut.addActionListener(new AWTConvenience.CloseAction(dialog));

        Panel p = new Panel(new FlowLayout(FlowLayout.CENTER));
        p.add(okBut);

        dialog.add(p, BorderLayout.SOUTH);

        dialog.addWindowListener(new AWTConvenience.CloseAdapter(okBut));

        AWTConvenience.setBackgroundOfChildren(dialog);

        dialog.setResizable(true);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);
        okBut.requestFocus();
        dialog.setVisible(true);        
    }

    public File selectFile(Frame parent, String title, String cwd, String deffile, boolean save) {
        FileDialog dialog = new FileDialog(parent, title, save ? FileDialog.SAVE : FileDialog.LOAD);
        if (cwd != null) dialog.setDirectory(cwd);
        if (deffile != null) dialog.setFile(deffile);
        dialog.setVisible(true);
        String fileName = dialog.getFile();
        if (fileName == null || fileName.length() == 0) return null;
        return new File(dialog.getDirectory(), fileName);        
    }


    private String textInput;
    public String textInput(String title, String message, Frame parent,
                            char echo, String defaultValue, String prompt) {
        final TextField textTxtInp;
        final Dialog dialog = new BorderDialog(parent, title, true);

        if (message != null && message.trim().length() > 0)
            dialog.add(new Label(message), BorderLayout.NORTH);

        dialog.add(new Label(prompt), BorderLayout.WEST);

        textTxtInp = new TextField();
        textTxtInp.setText(defaultValue);
        if (echo > (char)0)
            textTxtInp.setEchoChar(echo);
        textTxtInp.setColumns(16);
        dialog.add(textTxtInp, BorderLayout.CENTER);

        Button okBut = new Button("OK");
        ActionListener al;
        okBut.addActionListener(al = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if(e.getActionCommand().equals("OK")) {
                    textInput = textTxtInp.getText();
                } else {
                    textInput = null;
                }
                dialog.dispose();
            }
        });

        Button cancBut = new Button("Cancel");
        cancBut.addActionListener(al);

        Panel bp = AWTConvenience.newButtonPanel(
            new Button[] { okBut, cancBut });
        dialog.add(bp, BorderLayout.SOUTH);

        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancBut));

        AWTConvenience.setKeyListenerOfChildren
            (dialog, new AWTConvenience.OKCancelAdapter(okBut, cancBut), null);
        AWTConvenience.setBackgroundOfChildren(dialog);

        dialog.setResizable(false);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);

        dialog.setVisible(true);

        return textInput;
    }

    private String setPwdAnswer;
    public String setPassword(String title, String message, Frame parent) {
        final TextField setPwdText, setPwdText2;
        final Dialog dialog = new BorderDialog(parent, title, true);

        dialog.add(new Label(message), BorderLayout.NORTH);

        Panel p = new Panel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();

        gbc.fill      = GridBagConstraints.NONE; 
        gbc.insets    = new Insets(10, 4, 2, 4);
        gbc.anchor    = GridBagConstraints.EAST;;
        gbc.gridwidth = GridBagConstraints.RELATIVE;
        gbc.weightx   = 0.0;
        p.add(new Label("Password"), gbc);
        
        setPwdText = new TextField("", 12);
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
        p.add(new Label("Password again"), gbc);

        setPwdText2 = new TextField("", 12);
        setPwdText2.setEchoChar('*');
        gbc.anchor    = GridBagConstraints.WEST;
        gbc.fill      = GridBagConstraints.HORIZONTAL;
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.weightx   = 1.0;
        p.add(setPwdText2, gbc);

        dialog.add(p, BorderLayout.CENTER);

        Button okBut = new Button("OK");
        ActionListener al;
        okBut.addActionListener(al = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                if(e.getActionCommand().equals("OK")) {
                    setPwdAnswer = setPwdText.getText();
                    if(!setPwdAnswer.equals(setPwdText2.getText())) {
                        setPwdText.setText("");
                        setPwdText2.setText("");
                        setPwdText.requestFocus();
                        return;
                    }
                } else {
                    setPwdAnswer = null;
                }
                dialog.dispose();
            }
        });

        Button cancBut = new Button("Cancel");
        cancBut.addActionListener(al);

        Panel bp = AWTConvenience.newButtonPanel(
            new Button[] { okBut, cancBut });        
        dialog.add(bp, BorderLayout.SOUTH);

        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancBut));

        AWTConvenience.setKeyListenerOfChildren
            (dialog, new AWTConvenience.OKCancelAdapter(okBut, cancBut), null);
        AWTConvenience.setBackgroundOfChildren(dialog);

        dialog.setResizable(false);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);

        dialog.setVisible(true);

        return setPwdAnswer;
    }

}
