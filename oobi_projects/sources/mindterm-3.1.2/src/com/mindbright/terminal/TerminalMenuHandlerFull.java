/******************************************************************************
 *
 * Copyright (c) 1999-2006 AppGate Network Security AB. All Rights Reserved.
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

package com.mindbright.terminal;

import java.awt.Component;
import java.awt.Container;
import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import com.mindbright.application.MindTerm;
import com.mindbright.gui.GUI;
import com.mindbright.ssh.SSH;
import com.mindbright.ssh.Version;

/**
 * The actual implementation of the standard terminal menus. Or rather
 * the front-end to the actual implementation since this class depends
 * on its two subclasses <code>TerminalMenuHandlerFullAWT</code> and
 * <code>TerminalMenuHandlerFullSwing</code> for the actual
 * handling. That means that this class, together with the suitable
 * subclass, can handle bit AWT and Swing menus.
 *
 * @see TerminalWin
 * @see TerminalMenuHandlerFullAWT
 * @see TerminalMenuHandlerFullSwing
 */
public abstract class TerminalMenuHandlerFull extends TerminalMenuHandler 
    implements ActionListener, ItemListener, TerminalPrinter
{
    protected void setProperty(String key, String value) {
        try {
            term.setProperty(key, value);
        } catch (IllegalArgumentException e) {
            GUI.showAlert("Failed to set property", e.getMessage(),
                          term.ownerFrame);
        }
    }

    protected String getProperty(String key) {
        return term.getProperty(key);
    }

    private final static String aboutText =
    	// [CYCLADES-START]
        SSH.VER_MINDTERM + "\n" +
        "\n" +
        Version.copyright + "\n" +    	
        "\t" + Version.url + "\n" +
        "\n"+
        "This product includes cryptographic software written by,\n" +
        "Eric Young (eay@cryptsoft.com)\n" +
        "\n" +
        "JVM vendor:\t" + MindTerm.javaVendor  + "\n" +
        "JVM version:\t" + MindTerm.javaVersion  + "\n" +
        "OS name:\t" + MindTerm.osName  + "\n" +
        "OS arch.:\t" + MindTerm.osArch  + "\n" +
        "OS version:\t" + MindTerm.osVersion  + "\n";
     
    protected TerminalWin term;
    protected String titleName;
    private TerminalMenuListener listener;
    
    protected Frame frame;

    public final static int MENU_FILE     = 0;
    public final static int MENU_EDIT     = 1;
    public final static int MENU_SETTINGS = 2;
    public final static int MENU_HELP     = 3;

    final static int M_FILE_PRINT_SCREEN = 1;
    final static int M_FILE_PRINT_BUFFER = 2;
    final static int M_FILE_CAPTURE      = 3;
    final static int M_FILE_SEND         = 4;
    final static int M_FILE_CLOSE        = 6;

    final static int M_SET_TERM     = 1;

    final static int M_EDIT_COPY    = 1;
    final static int M_EDIT_PASTE   = 2;
    final static int M_EDIT_CPPASTE = 3;
    final static int M_EDIT_SELALL  = 4;
    final static int M_EDIT_FIND    = 5;
    final static int M_EDIT_CLS     = 7;
    final static int M_EDIT_CLEARSB = 8;
    final static int M_EDIT_VTRESET = 9;
    
    final static int M_HELP_ABOUT   = 1;

    protected final static String[][] menuTexts = {
	{ "File", 
          "Print screen...", "Print buffer...",
	  "_Capture To File...", "Send ASCII File...", null, "Close"
	},
	{ "Edit",
	  "Copy Ctrl+Ins", "Paste Shift+Ins", "Copy & Paste", "Select All",
	  "Find...", null,
	  "Clear Screen", "Clear Scrollback", "VT Reset"
	},
	{ "Settings",
	  "Terminal...", 
	},
	{ "Help",
		  "About MindTerm", 
	}
    };

    protected final static int NO_SHORTCUT = -1;
    protected final static int[][] menuShortCuts = {
	{ NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT,
          NO_SHORTCUT, KeyEvent.VK_E },

	{ NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, KeyEvent.VK_A,
	  KeyEvent.VK_F, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT },

	{ NO_SHORTCUT, NO_SHORTCUT },

	{ NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT,
	  NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT,
	  NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT,
	  NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT },
    };

    protected TerminalMenuHandlerFull() {
	this("MindTerm");
    }

    protected TerminalMenuHandlerFull(String titleName) {
	setTitleName(titleName);
    }

    /**
     * Factory class which constructs a suitable instance. This will
     * actually create an instance of one of the subclasses. Which
     * subclass is used depends on wether the frame argument is
     * actually a <code>Frame</code> or a <code>JFrame</code>
     *
     * @param frame top frame of window to handle menus of. But this
     * is only used to select which implementation (AWT or Swing) to
     * instantiate.
     *
     * @return a new instance suitable for the given frame
     */
    public static TerminalMenuHandler getInstance(Frame frame) {
        try {
            Class c = Class.forName(
                /*AWTConvenience.isSwingJFrame(frame)*/ true ? 
                "com.mindbright.terminal.TerminalMenuHandlerFullSwing" :
                "com.mindbright.terminal.TerminalMenuHandlerFullAWT");
            return (TerminalMenuHandler)c.newInstance();
        } catch (Throwable t) {
        }
        return null;
    }

    public void setTitleName(String titleName) {
	this.titleName = titleName;	
    }

    public void setTerminalWin(TerminalWin term) {
	this.term = term;
	term.attachPrinter(this);
    }

    public void setTerminalMenuListener(TerminalMenuListener listener) {
	this.listener = listener;
    }

    public void addBasicMenus(TerminalWin terminal, Container frame) {
        Component comp = frame;
        do {
            comp = comp.getParent();
        } while(!(comp instanceof Frame));
        this.frame = (Frame)comp;
    	setTerminalWin(terminal);
    	addBasicMenusInternal(frame);
    	terminal.setMenus(this);
    	terminal.setClipboard(GlobalClipboard.getClipboardHandler(this));
    	terminal.updateMenus();
    }

    public void updateSelection(boolean selectionAvailable) {
        setEnabled(MENU_EDIT, M_EDIT_COPY, selectionAvailable);
        setEnabled(MENU_EDIT, M_EDIT_CPPASTE, selectionAvailable);
    }

    public void update() {
	if (listener != null)
	    listener.update();
    }

    // stuff for terminal settings dialog
    protected final static String[] PASTE_BUTTON   = { "middle", "right", "shift+left" };
    protected final static String[] SCROLLBAR_POS  = { "left", "right", "none" };
    protected final static String[] TERMINAL_TYPES = TerminalWin.getTerminalTypes();
    protected final static String[] FONT_LIST      = GUI.getFontList();
    protected final static String[] ENCODINGS      = {"iso-8859-1", "utf-8", "euc-jp", "euc-kr", "us-ascii"};

    protected final static String LBL_TERMINAL_TYPE     = "Terminal type";
    protected final static String LBL_COLUMNS           = "Columns";
    protected final static String LBL_ROWS              = "Rows";
    protected final static String LBL_FONT              = "Font";
    protected final static String LBL_ENCODING          = "Encoding";
    protected final static String LBL_SIZE              = "Size";
    protected final static String LBL_SCROLLBACK_BUFFER = "Scrollback buffer";
    protected final static String LBL_SCROLLBAR_POS     = "Scrollbar position";
    protected final static String LBL_FG_COLOR          = "Foreground color";
    protected final static String LBL_BG_COLOR          = "Background color";
    protected final static String LBL_CURS_COLOR        = "Cursor color";
    protected final static String LBL_PASTE_BUTTON      = "Paste button";
    protected final static String LBL_SELECT_DELIM      = "Select delim.";
    protected final static String LBL_IGN_NULL          = "Ignore null bytes";
    protected final static String LBL_TAB_GENERAL       = "General";
    protected final static String LBL_TAB_MISC          = "Misc";
    protected final static String LBL_TAB_COLORS        = "Colors";
    protected final static String LBL_TAB_VTOPTIONS1    = "VT 1";
    protected final static String LBL_TAB_VTOPTIONS2    = "VT 2";
    protected final static String LBL_CUSTOM_RGB        = "custom rgb";
    protected final static String LBL_FIND              = "Find";
    protected final static String LBL_CASE_SENSITIVE    = "Case sensitive";
    protected final static String LBL_FIND_BACKWARDS    = "Find backwards";
    protected final static String LBL_BTN_OK            = "OK";
    protected final static String LBL_BTN_CANCEL        = "Cancel";
    protected final static String LBL_BTN_FIND          = "Find";
    
    /**
     * Show the terminal settings dialog.
     *
     * @param title desired title of dialog
     */
    public abstract void termSettingsDialog(String title);

    /**
     * Show the terminal settings dialog with the default title.
     */
    public void termSettingsDialog() {
        termSettingsDialog("Terminal Settings");
    }

    /**
     * Show the find dialog.
     *
     * @param title desired title of dialog
     */
    public abstract void findDialog(String title);

    protected SearchContext lastSearch;

    protected void doFind(String findStr, boolean caseSens, boolean revFind) {
        lastSearch = term.search(lastSearch, findStr, revFind, caseSens);
        if (lastSearch == null)
            term.ringBell();
    }

    /**
     * Show the send file dialog.
     */
    public final void sendFileDialog() {
        File file = GUI.selectFile(term.ownerFrame, titleName +
                                   " - Select ASCII-file to send", false);
        
        if (file == null) return;
        
        try {
            FileInputStream fileIn = new FileInputStream(file);
            byte[] bytes = new byte[fileIn.available()];
            fileIn.read(bytes);
            term.sendBytes(bytes);
        } catch (Throwable t) {
            GUI.showAlert(titleName + " - Alert", t.getMessage(),
                          term.ownerFrame);
        }
    }

    /**
     * Shows a save dialog with the specified title. The selected file
     * is returned as an opened <code>FileOutputStream</code>
     *
     * @param title title of dialog
     * @return the opened file or null if the user aborted or an error
     * ocurred.
     *
     */
    public final FileOutputStream chooseFileDialog(String title) {
        File file = GUI.selectFile
            (term.ownerFrame, titleName + " - " + title, true);

        if (file == null) return null;

        try {
            boolean append = false;
            if (file.exists()) {
                append = GUI.showConfirm(titleName + " - File exists",
                                         "File exists, overwrite or append?",
                                         0, 0, "Append", "Overwrite",
                                         true, term.ownerFrame, false, false);
            }
            return new FileOutputStream(file.getCanonicalPath(), append);
        } catch (Throwable t) {
            GUI.showAlert(titleName + " - Alert", t.getMessage(),
                          term.ownerFrame);
        }
	return null;
    }

    private TerminalCapture termCapture;

    /**
     * Start capturing data to file. This function will cause a save
     * dialog to appear and if a file was successfully specified then a
     * log of the terminal session from now on will be stored in the file.
     */
    public final boolean captureToFileDialog() {
	FileOutputStream fileOut =
            chooseFileDialog("Select file to capture to");
	if (fileOut != null) {
	    termCapture = new TerminalCapture(fileOut);
	    termCapture.startCapture(term);
	    return true;
	}

	return false;
    }

    /**
     * Stop capturing data in a file. Capturing is started by calling
     * <code>captureToFileDialog()</code>.
     */
    public void endCapture() {
	if(termCapture != null) {
	    termCapture.endCapture();
	    try {
		termCapture.getTarget().close();
	    } catch (IOException e) {
		GUI.showAlert(titleName + " - Alert", e.getMessage(),
                              term.ownerFrame);
	    }
	}
    }

    protected Object[][] menuItems;

    // menu handling stuff
    protected abstract void setEnabled(int i, int j, boolean v);
    protected abstract void setState(int i, int j, boolean v);
    protected abstract boolean getState(int i, int j);
    protected abstract void addBasicMenusInternal(Container frame);
    public abstract Object getMenu(int idx);

    private int[] mapAction(Object o) {
	int[] id = new int[2];
	int i = 0, j = 0;

        for(i = 0; i < menuItems.length; i++) {
            for(j = 1; menuItems[i] != null && j < menuItems[i].length;
                j++)
            {
                if(menuItems[i][j] == o) {
                    id[0] = i;
                    id[1] = j;
                    return id;
                }
            }
        }
        return id;
    }

    /**
     * Called when a menu item was selected
     */
    public void actionPerformed(ActionEvent e) {
	handleMenuAction(mapAction(e.getSource()));
    }

    /**
     * Called when a checkbox menu item has changed state
     */
    public void itemStateChanged(ItemEvent e) {
	handleMenuAction(mapAction(e.getSource()));
    }

    /**
     * Actually handle the selection of all the menu elements.
     */
    public void handleMenuAction(int[] id) {
	switch(id[0]) {
            case MENU_FILE:
                switch(id[1]) {
	                case M_FILE_PRINT_SCREEN:
	                    printScreen();
	                    break;
	                case M_FILE_PRINT_BUFFER:
	                    printBuffer();
	                    break;
                	case M_FILE_CAPTURE:
                        if (getState(MENU_FILE, M_FILE_CAPTURE)) {
                            if(!captureToFileDialog())
                                setState(MENU_FILE, M_FILE_CAPTURE, false);
                        } else {
                            endCapture();
                        }
                        break;
                    case M_FILE_SEND:
                        sendFileDialog();
                        break;
                    case M_FILE_CLOSE:
                        if (listener != null)
                            listener.close(this);
                        break;
                }
                break;

            case MENU_EDIT:
                switch(id[1]) {
                    case M_EDIT_COPY:
                        term.doCopy();
                        break;
                    case M_EDIT_PASTE:
                        term.doPaste();
                        break;
                    case M_EDIT_CPPASTE:
                        term.doCopy();
                        term.doPaste();
                        break;
                    case M_EDIT_SELALL:
                        term.selectAll();
                        break;
                    case M_EDIT_FIND:
                        findDialog(titleName + " - Find");
                        break;
                    case M_EDIT_CLS:
                        term.clearScreen();
                        term.setCursorPos(0, 0);
                        break;
                    case M_EDIT_CLEARSB:
                        term.clearSaveLines();
                        break;
                    case M_EDIT_VTRESET:
                        term.reset();
                        break;
                }
                break;

            case MENU_SETTINGS:
                switch(id[1]) {
                    case M_SET_TERM:
                        termSettingsDialog();
                        break;
                }
                break;
                
            case MENU_HELP:
                switch(id[1]) {
                    case M_HELP_ABOUT:
                        aboutDialog((Frame)frame, term,
                                "About " + SSH.VER_MINDTERM, aboutText);
                        break;
                }
                break;
           
	}
    }

    protected FileOutputStream printerOut;

    /**
     * Write a character to the printer.
     *
     * @param c charcter to write
     */
    public void write(char c) {
	if(printerOut != null) {
	    try {
		printerOut.write((int)c);
	    } catch (IOException e) {
		GUI.showAlert(titleName + " - Alert", e.getMessage(),
                              term.ownerFrame);
	    }
	}
    }

    /**
     * Start printing data to printer. That is currently it only saves
     * data in a file.
     */
    public void startPrinter() {
	printerOut = chooseFileDialog("Passthrough print to file");
    }

    /**
     * Stop sending data to printer.
     */
    public void stopPrinter() {
	if(printerOut != null) {
	    try { printerOut.close(); }
	    catch (IOException e) { /* don't care... */ }
	    printerOut = null;
	    GUI.showAlert(titleName + " - Alert",
                          "Passthrough printing ended, file saved.",
                          term.ownerFrame);
	}
    }

    public void printScreen() {
        doPrint(TerminalPrintable.SCREEN);
    }

    public void printBuffer() {
        doPrint(TerminalPrintable.BUFFER);
    }

    private void doPrint(int what) {
        PrinterJob job = PrinterJob.getPrinterJob();

        job.setPrintable(
            new TerminalPrintable(term.getDisplay().getModel(), what,
                                  term.printFontName, term.printFontSize));

        if (job.printDialog()) {
            try {
                job.print();
            } catch (PrinterException e) {
                // Handle Exception
            }
        }
    }

    protected abstract void aboutDialog(Frame parent, TerminalWin terminal, 
            String title, String aboutText);

}
