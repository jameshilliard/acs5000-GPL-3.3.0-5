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

public final class GUI {
    
    private static GUIInterface impl;
    
    private static void init(Frame parent) {
        if (impl != null || parent == null) return;
        try {
			//  [CYCLADES-START]  
			// if (parent instanceof javax.swing.JFrame) {
			if (AWTConvenience.isSwingJFrame(parent)) {
			// [CYCLADES-END]
            
                impl = new GUISwing();
            }
        } catch (Throwable t) {}
        if (impl == null) {
            impl = new GUIAWT();
        }

    }

    public static void showAlert(String title, String message, Frame parent) {
        init(parent);
        impl.showAlert(title, message, parent);
    }

    public static boolean showConfirm(String title, String message,
                                      boolean defAnswer, Frame parent) {
        return showConfirm(title, message, 0, 0, "Yes", "No", 
                           defAnswer, parent, false, false);
    }

    public static boolean showConfirm(String title, String message,
                                      int rows, int cols,
                                      String yesLbl, String noLbl,
                                      boolean defAnswer, Frame parent,
                                      boolean xscroll, boolean yscroll) {
        init(parent);
		
		 // [CYCLADES-START] : 	showConfig causes error when Mindterm runs in the same window as applet
		 // 					thus a try/catch block was added.
        // Done by Eduardo Murai Soares
        try {
        return impl.showConfirm(title, message, rows, cols, yesLbl, noLbl, 
                                defAnswer, parent, xscroll, yscroll);
        }
        catch (Exception e) {
        	System.out.println("WARNING in GUI.showConfirm: "+e.getMessage());
        	return true;
        }
        // [CYCLADES-END]
    }

    public static java.io.File selectFile(Frame parent, String title, boolean save) {
        return selectFile(parent, title, null, null, save);
    }

    public static java.io.File selectFile(Frame parent, String title, String cwd, boolean save) {
        return selectFile(parent, title, cwd, null, save);
    }

    public static java.io.File selectFile(Frame parent, String title, String cwd, String deffile, boolean save) {
        init(parent);
        if (cwd != null && cwd.equals("")) cwd = null;
        if (deffile != null && deffile.equals("")) deffile = null;
        return impl.selectFile(parent, title, cwd, deffile, save);
    }

    public static void showNotice(Frame parent, String title, String text, int rows, int cols,
                                  boolean scrollbar) {
        init(parent);
        impl.showNotice(parent, title, text, rows, cols, scrollbar);
    }

    public static String textInput(String title, String message, Frame parent,
                                   char echo, String defaultValue, String prompt) {
        init(parent);
        return impl.textInput(title, message, parent, echo, defaultValue, prompt);
    }

    public static String setPassword(String title, String message, Frame parent) {
        init(parent);
        return impl.setPassword(title, message, parent);
    }

    public static String[] getFontList() {
        return java.awt.Toolkit.getDefaultToolkit().getFontList();
    }
	
    //  [CYCLADES-START]    
    /**
     * Shows JOptionPane to user.
     * Added by Eduardo Murai Soares.
     * 
     * @param parent
     * @param title
     * @param message
     * @return Options done by user
     */
    public static int showOptionPane(Frame parent, String title, String message) {
    	init(parent);    	
    	return impl.showOptionPane(parent, title, message);    	
    }
    // [CYCLADES-END]
	
}
