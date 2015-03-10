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

import com.mindbright.gui.GUI;

public final class SSHMiscDialogs {

    public static void alert(String title, String message, Frame parent) {
        GUI.showAlert(title, message, parent);
    }

    public static String password(String title, String message, Frame parent) {
        return textInput(title, message, parent, '*', "", "Password:");
    }

    public static String textInput(String title, String message, Frame parent) {
        return textInput(title, null, parent, (char)0, "", message);
    }

    public static String textInput(String title, String message, Frame parent,
                                   String defaultValue) {
        return textInput(title, null, parent, (char)0, defaultValue, message);
    }

    public static String textInput(String title, String message, Frame parent,
                                   char echo, String defaultValue, String prompt) {
        return GUI.textInput(title, message, parent, echo, defaultValue, prompt);
    }
    
    public static String setPassword(String title, String message, Frame parent) {
        return GUI.setPassword(title, message, parent);
    }
    
    public static boolean confirm(String title, String message, boolean defAnswer,
                                  Frame parent) {
        return GUI.showConfirm(title, message, 0, 0, "Yes", "No", 
                               defAnswer, parent, false, false);
    }

    public static boolean confirm(String title, String message,
                                  int rows, int cols,
                                  String yesLbl, String noLbl,
                                  boolean defAnswer, Frame parent,
                                  boolean scrollbar) {
        return GUI.showConfirm(title, message, rows, cols, yesLbl, noLbl,
                               defAnswer, parent, false, scrollbar);
    }
}
