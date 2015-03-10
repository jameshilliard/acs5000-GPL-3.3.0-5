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

import java.awt.Frame;

public interface GUIInterface {
 // [CYCLADES-START]   	
    /**
     * Shows JOptionPane to user with three buttons : OK, NO and CANCEL.
     * Added by Eduardo Murai Soares.
     * 
     * @param parent
     * @param title
     * @param message
     * @return 0: button OK chosen
     * 		   1: button NO chosen
     * 		   2: button CANCEL chosen	
     */
	public int showOptionPane(Frame parent, String title, String message);
	// [CYCLADES-END]
	
    public void showAlert(String title, String message, Frame parent);
    
    public boolean showConfirm(String title, String message,
                               int rows, int cols,
                               String yesLbl, String noLbl,
                               boolean defAnswer, Frame parent,
                               boolean xscroll, boolean yscroll);

    public void showNotice(Frame parent, String title, String text, 
                           int rows, int cols, boolean scrollbar);
    
    public java.io.File selectFile(Frame parent, String title, 
                                   String cwd, String deffile, boolean save);

    public String textInput(String title, String message, Frame parent,
                            char echo, String defaultValue, String prompt);

    public String setPassword(String title, String message, Frame parent);
}
