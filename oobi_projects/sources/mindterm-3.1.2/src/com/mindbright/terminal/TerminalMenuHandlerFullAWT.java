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

package com.mindbright.terminal;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Checkbox;
import java.awt.CheckboxMenuItem;
import java.awt.Choice;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Label;
import java.awt.Menu;
import java.awt.MenuBar;
import java.awt.MenuItem;
import java.awt.MenuShortcut;
import java.awt.Panel;
import java.awt.PopupMenu;
import java.awt.SystemColor;
import java.awt.TextArea;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.BorderDialog;
import com.mindbright.gui.Logo;
import com.mindbright.gui.TabPanel;

/**
 * Implements AWT-style menus for a terminal window.
 *
 * @see TerminalWin
 * @see TerminalMenuHandlerFull
 */
public class TerminalMenuHandlerFullAWT extends TerminalMenuHandlerFull {
    private Menu vtOptionsMenu;

    private static final boolean BOX_APPLET = true;
    
    protected void setEnabled(int i, int j, boolean v) {
        ((MenuItem)menuItems[i][j]).setEnabled(v);
    }

    protected void setState(int i, int j, boolean v) {
        ((CheckboxMenuItem)menuItems[i][j]).setState(v);
    }

    protected boolean getState(int i, int j) {
        return ((CheckboxMenuItem)menuItems[i][j]).getState();
    }

    protected void addBasicMenusInternal(Frame frame) {
	MenuBar menubar = frame.getMenuBar();
        if (menubar == null)
            frame.setMenuBar(menubar = new MenuBar());
	menubar.add((Menu)getMenu(MENU_FILE));
	menubar.add((Menu)getMenu(MENU_EDIT));
	menubar.add((Menu)getMenu(MENU_SETTINGS));
	menubar.add((Menu)getMenu(MENU_HELP));
	frame.addWindowListener(new AWTConvenience.
	    CloseAdapter((MenuItem)menuItems[MENU_FILE][M_FILE_CLOSE]));
    }

    public Object getMenu(int idx) {
	Menu m = new Menu(menuTexts[idx][0]);
	int len = menuTexts[idx].length;
	MenuItem mi;
	String   t;

	if (menuItems == null)
	    menuItems = new MenuItem[menuTexts.length][];
	if (menuItems[idx] == null) {
	    menuItems[idx] = new MenuItem[menuTexts[idx].length];
        }

	for (int i = 1; i < len; i++) {
	    t = menuTexts[idx][i];
	    if (t == null) {
		m.addSeparator();
		continue;
	    }
	    if (t.charAt(0) == '_') {
		t = t.substring(1);
		mi = new CheckboxMenuItem(t);
		((CheckboxMenuItem)mi).addItemListener(this);
	    } else {
		mi = new MenuItem(t);
		mi.addActionListener(this);
	    }

	    if (menuShortCuts[idx][i] != NO_SHORTCUT) {
		mi.setShortcut(new MenuShortcut(menuShortCuts[idx][i], true));
	    }

	    menuItems[idx][i] = mi;
	    m.add(mi);
	}

	return m;
    }


    private static Component newColorChoice(String lbl, Choice c, TextField tf) {
        Panel p1 = new Panel(new BorderLayout(0, 2));
        Panel p2 = new Panel(new FlowLayout(FlowLayout.LEFT, 5, 0));
        p2.add(c);
        p2.add(tf);
        p1.add(new Label(lbl), BorderLayout.NORTH);
        p1.add(p2, BorderLayout.CENTER);
        return p1;
    }

    private Component vtoptions[];
    private TerminalOption toptions[];

    private Component getVTOptionsPanel(boolean top) {
        int start, end;
        if (top) {
            toptions = term.getOptions();
            vtoptions = new Component[toptions.length];
            start = 0;
            end = toptions.length/2;
        } else {
            start = toptions.length/2;
            end = toptions.length;
        }

        if (toptions == null || toptions.length == 0)
            return null;

        Panel p = new Panel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.insets    = new Insets(0, 5, 0, 5);
        gridc.fill      = GridBagConstraints.NONE;
        gridc.anchor    = GridBagConstraints.WEST;
        gridc.gridwidth = GridBagConstraints.REMAINDER;

        for (int i=start; i<end; i++) {
            String cs[] = toptions[i].getChoices();
            if (cs == null) {
                vtoptions[i] = new Checkbox(toptions[i].getDescription(),
                                            toptions[i].getValueB());
            } else {
                gridc.gridwidth = 1;
                gridc.fill      = GridBagConstraints.HORIZONTAL;
                p.add(new Label(toptions[i].getDescription(), Label.RIGHT),
                      gridc);
                vtoptions[i] = AWTConvenience.newChoice(cs);
                ((Choice)vtoptions[i]).select(
                    toptions[i].getValue().toLowerCase());
                gridc.gridwidth = GridBagConstraints.REMAINDER;
                gridc.fill      = GridBagConstraints.NONE;
            }
            p.add(vtoptions[i], gridc);
        }
        
        gridc.fill    = GridBagConstraints.BOTH;
        gridc.insets  = new Insets(0, 0, 0, 0);
        gridc.weightx = 1.0;
        gridc.weighty = 1.0;
        p.add(new Panel(), gridc);

        return p;
    }

    private void setVTOptions() {
        if (vtoptions == null) return;
        
        for (int i=0; i<vtoptions.length; i++) {
            String val;
            if (vtoptions[i] instanceof Choice) {
                val = ((Choice)vtoptions[i]).getSelectedItem();
            } else {
                val = ((Checkbox)vtoptions[i]).getState() ? "true" : "false";
            }
            setProperty(toptions[i].getKey(), val);
        }
        
        vtoptions = null;
        toptions = null;
    }

    private Choice choiceTE, choiceEN, choiceFN, choiceSB, choicePB;
    private Choice choiceFG, choiceBG, choiceCC;
    private TextField textFS, textRows, textCols, textInitPos;
    private TextField textSL, textSD;
    private TextField textFG, textBG, textCC;
    private Label lblAlert, labelTE;
    private Checkbox checkIN;

    public void termSettingsDialog(String title) {
        final Dialog dialog = new Dialog(term.ownerFrame, title, true);

        // general tab
        GridBagConstraints gridcl = new GridBagConstraints();
        GridBagConstraints gridcr = new GridBagConstraints();

        Panel mp = new Panel(new GridBagLayout());

        gridcl.fill      = GridBagConstraints.HORIZONTAL;
        gridcl.insets    = new Insets(2, 2, 2, 2);
        gridcl.gridwidth = 1;
        gridcr.anchor    = GridBagConstraints.WEST;
        gridcr.insets    = new Insets(2, 2, 2, 2);
        gridcr.gridwidth = GridBagConstraints.REMAINDER;
        gridcr.weightx   = 1.0;

	    //choiceTE = AWTConvenience.newChoice(TERMINAL_TYPES);
	    mp.add(new Label(LBL_TERMINAL_TYPE, Label.RIGHT), gridcl);
	    //mp.add(choiceTE, gridcr);
	    labelTE = new Label(getProperty("term-type"), Label.LEFT);
	    mp.add(labelTE, gridcr);
	        
        if(!BOX_APPLET) {
	        textCols = new TextField("", 3);
	        mp.add(new Label(LBL_COLUMNS, Label.RIGHT), gridcl);
	        mp.add(textCols, gridcr);
	
	        textRows = new TextField("", 3);
	        mp.add(new Label(LBL_ROWS, Label.RIGHT), gridcl);
	        mp.add(textRows, gridcr);
	
	        choiceEN = AWTConvenience.newChoice(ENCODINGS);
	        mp.add(new Label(LBL_ENCODING, Label.RIGHT), gridcl);
	        mp.add(choiceEN, gridcr);
	
	        choiceFN = AWTConvenience.newChoice(FONT_LIST);
	        mp.add(new Label(LBL_FONT, Label.RIGHT), gridcl);
	        mp.add(choiceFN, gridcr);
	
	        textFS = new TextField("", 3);
	        mp.add(new Label(LBL_SIZE, Label.RIGHT), gridcl);
	        mp.add(textFS, gridcr);
	
	        textSL = new TextField("", 4);
	        mp.add(new Label(LBL_SCROLLBACK_BUFFER, Label.RIGHT), gridcl);
	        mp.add(textSL, gridcr);
        }
        
        choiceSB = AWTConvenience.newChoice(SCROLLBAR_POS);
        mp.add(new Label(LBL_SCROLLBAR_POS, Label.RIGHT), gridcl);
        mp.add(choiceSB, gridcr);

        lblAlert = new Label("", Label.CENTER);
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.insets    = new Insets(0, 0, 0, 0);
        gridc.fill      = GridBagConstraints.BOTH;
        gridc.weighty   = 1.0;
        gridc.anchor    = GridBagConstraints.CENTER;
        mp.add(lblAlert, gridc);

        TabPanel tp = new TabPanel();
        tp.add(LBL_TAB_GENERAL, mp);

        // color tab
        mp = new Panel(new GridBagLayout());

        gridc.weightx   = 0.0;
        gridc.weighty   = 0.0;
        gridc.gridwidth = 1;
        gridc.insets    = new Insets(2, 2, 2, 2);

        ItemListener ilC = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                updateColors();
            }
        };

        choiceFG = new Choice();
        choiceFG.addItemListener(ilC);
        textFG = new TextField("", 10);
        mp.add(new Label(LBL_FG_COLOR, Label.RIGHT), gridc);
        mp.add(choiceFG, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textFG, gridc);

        choiceBG = new Choice();
        choiceBG.addItemListener(ilC);
        textBG = new TextField("", 10);
        gridc.gridwidth = 1;
        mp.add(new Label(LBL_BG_COLOR, Label.RIGHT), gridc);
        mp.add(choiceBG, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textBG, gridc);

        choiceCC = new Choice();
        choiceCC.addItemListener(ilC);
        textCC = new TextField("", 10);
        gridc.gridwidth = 1;
        mp.add(new Label(LBL_CURS_COLOR, Label.RIGHT), gridc);
        mp.add(choiceCC, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textCC, gridc);

        gridc.fill = GridBagConstraints.BOTH;
        gridc.weightx = 1.0;
        gridc.weighty = 1.0;
        mp.add(new Label(""), gridc);
        
        tp.add(LBL_TAB_COLORS, mp);


        // misc tab
        gridc = new GridBagConstraints();
        mp = new Panel(new GridBagLayout());

        gridc.insets    = new Insets(2, 2, 2, 2);
        gridc.anchor    = GridBagConstraints.WEST;
        gridc.gridwidth = 1;
        mp.add(new Label(LBL_PASTE_BUTTON, Label.RIGHT), gridc);

        choicePB = AWTConvenience.newChoice(PASTE_BUTTON);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(choicePB, gridc);

        gridc.gridwidth = 1;
        mp.add(new Label(LBL_SELECT_DELIM, Label.RIGHT), gridc);

        textSD = new TextField("", 8);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textSD, gridc);

        gridc.gridwidth = 1;
        mp.add(new Label(), gridc);

        checkIN = new Checkbox(
            LBL_IGN_NULL, Boolean.valueOf(getProperty("ignore-null")).booleanValue());
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(checkIN, gridc);

        gridc.fill = GridBagConstraints.BOTH;
        gridc.weightx = 1.0;
        gridc.weighty = 1.0;
        mp.add(new Label(""), gridc);
        
        tp.add(LBL_TAB_MISC, mp);

        // VT options tab (top half)
        Component vtp = getVTOptionsPanel(true);
        if (vtp != null) {
            tp.add(LBL_TAB_VTOPTIONS1, vtp);
        }

        // VT options tab (bottom half)
        vtp = getVTOptionsPanel(false);
        if (vtp != null) {
            tp.add(LBL_TAB_VTOPTIONS2, vtp);
        }

        dialog.add(tp, BorderLayout.CENTER);
        tp.show(LBL_TAB_GENERAL);

        Button ok = new Button(LBL_BTN_OK);
        ok.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    setVTOptions();

	                //setProperty("term-type",
	                //            TERMINAL_TYPES[choiceTE.getSelectedIndex()]);
                    if(!BOX_APPLET){
	                    setProperty("encoding",
	                                ENCODINGS[choiceEN.getSelectedIndex()]);
	                    setProperty("font-name",
	                                FONT_LIST[choiceFN.getSelectedIndex()]);
	                    setProperty("font-size", textFS.getText());
	                    setProperty("geometry",
	                            textCols.getText() + "x" + textRows.getText());
                    }
                    setProperty("scrollbar",
                                SCROLLBAR_POS[choiceSB.getSelectedIndex()]);
                    setProperty("save-lines", textSL.getText());
                    setProperty("paste-button",
                                PASTE_BUTTON[choicePB.getSelectedIndex()]);
                    setProperty("select-delim", textSD.getText());
                    setProperty("ignore-null",
                                Boolean.toString(checkIN.getState()));
                    setProperty("fg-color",
                                getSelectedColor(choiceFG, textFG));
                    setProperty("bg-color",
                                getSelectedColor(choiceBG, textBG));
                    setProperty("cursor-color",
                                getSelectedColor(choiceCC, textCC));
                    dialog.dispose();
                } catch (Exception ee) {
                    lblAlert.setText(ee.getMessage());
                }
            }
        });

        Button cancel = new Button(LBL_BTN_CANCEL);
        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));

        Panel p = AWTConvenience.newButtonPanel(new Component[] {ok,cancel});
        dialog.add(p, BorderLayout.SOUTH);

        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancel));

        AWTConvenience.setBackgroundOfChildren(dialog);

	//choiceTE.select(getProperty("term-type"));
	if(!BOX_APPLET) {
		choiceEN.select(getProperty("encoding"));
		choiceFN.select(getProperty("font-name"));
		textFS.setText(getProperty("font-size"));
		textCols.setText(String.valueOf(term.cols()));
		textRows.setText(String.valueOf(term.rows()));
	}
	choiceSB.select(getProperty("scrollbar"));
	textSL.setText(getProperty("save-lines"));

	choicePB.select(getProperty("paste-button"));
	String sdSet = getProperty("select-delim");
	if ((sdSet.charAt(0) == '"'
             && sdSet.charAt(sdSet.length() - 1) == '"')) {
	    sdSet = sdSet.substring(1, sdSet.length() - 1);
	}
	textSD.setText(sdSet);

	choiceBG.add(LBL_CUSTOM_RGB);
	choiceFG.add(LBL_CUSTOM_RGB);
	choiceCC.add(LBL_CUSTOM_RGB);
	for (int i = 0; i < TerminalWin.termColorNames.length; i++) {
	    choiceBG.add(TerminalWin.termColorNames[i]);
	    choiceFG.add(TerminalWin.termColorNames[i]);
	    choiceCC.add(TerminalWin.termColorNames[i]);
	}        
	initColorSelect(choiceFG, textFG, getProperty("fg-color"));
	initColorSelect(choiceBG, textBG, getProperty("bg-color"));
	initColorSelect(choiceCC, textCC, getProperty("cursor-color"));
	updateColors();

	lblAlert.setText("");

        dialog.setResizable(true);
        dialog.pack();

	AWTConvenience.placeDialog(dialog);

	textSL.requestFocus();

	dialog.setVisible(true);
    }


    private static void initColorSelect(Choice c, TextField t, String colStr) {
	if (Character.isDigit(colStr.charAt(0))) {
	    c.select(LBL_CUSTOM_RGB);
	    t.setText(colStr);
	} else {
	    t.setText("");
	    t.setEnabled(false);
	    c.select(colStr);
	}
    }

    private static void checkColorSelect(Choice c, TextField t) {
	int cs = c.getSelectedIndex();
    
	if (cs == 0) {
	    if (!t.isEnabled()) {
		t.setEditable(true);
		t.setEnabled(true);
		t.setBackground(SystemColor.text);
		t.requestFocus();
	    }
	} else {
	    t.setText("");
	    t.setEditable(false);
	    t.setEnabled(false);
	    // on the Mac, Choices can't get keyboard focus
	    // so we may need to move focus away from the TextField
	    t.setBackground(TerminalWin.termColors[cs - 1]);
	}
    }

    private void updateColors() {
	checkColorSelect(choiceFG, textFG);
	checkColorSelect(choiceBG, textBG);
	checkColorSelect(choiceCC, textCC);
    }

    private static String getSelectedColor(Choice c, TextField t) {
	return (c.getSelectedIndex() == 0) ? t.getText() : c.getSelectedItem();
    }

    private TextField  findText;
    private Checkbox   dirCheck, caseCheck;
    private Button     findBut, cancBut;

    public final void findDialog(String title) {
        lastSearch = null;

        Dialog dialog = new BorderDialog(term.ownerFrame, title, false);

        GridBagLayout      grid  = new GridBagLayout();
        GridBagConstraints gridc = new GridBagConstraints();
        dialog.setLayout(grid);
        
        gridc.fill   = GridBagConstraints.NONE;
        gridc.anchor = GridBagConstraints.WEST;
        gridc.gridwidth = 1;

        gridc.gridy = 0;
        Label label = new Label(LBL_FIND);
        dialog.add(label, gridc);
        
        gridc.fill = GridBagConstraints.HORIZONTAL;
        gridc.gridwidth = 5;
        
        findText = new TextField("", 26);
        dialog.add(findText, gridc);
        
        gridc.gridwidth = 4;
        gridc.ipadx = 4;
        gridc.ipady = 4;
        gridc.insets = new Insets(6, 3, 3, 6);
        
        findBut = new Button(LBL_BTN_FIND);
        dialog.add(findBut, gridc);

        gridc.insets = new Insets(0, 0, 0, 0);
        gridc.ipadx = 0;
        gridc.ipady = 0;
        gridc.gridwidth = 3;
        gridc.gridy = 1;
        gridc.fill   = GridBagConstraints.NONE;
        
        caseCheck = new Checkbox(LBL_CASE_SENSITIVE);
        dialog.add(caseCheck, gridc);
        
        dirCheck = new Checkbox(LBL_FIND_BACKWARDS);
        dialog.add(dirCheck, gridc);
        
        gridc.gridwidth = 4;
        gridc.ipadx = 4;
        gridc.ipady = 4;
        gridc.insets = new Insets(3, 3, 6, 6);
        gridc.fill = GridBagConstraints.HORIZONTAL;
        cancBut = new Button(LBL_BTN_CANCEL);
        dialog.add(cancBut, gridc);

        cancBut.addActionListener(new AWTConvenience.CloseAction(dialog));
        findBut.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String txt = findText.getText();
                if (txt != null && txt.length() > 0) 
                    doFind(txt, caseCheck.getState(), dirCheck.getState());
            }
        });
        
        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancBut));
        
        AWTConvenience.setBackgroundOfChildren(dialog);
        AWTConvenience.setKeyListenerOfChildren(
            dialog, new AWTConvenience.OKCancelAdapter(findBut, cancBut),null);
        
        dialog.setResizable(true);
        dialog.pack();

	AWTConvenience.placeDialog(dialog);
	findText.requestFocus();
	dialog.setVisible(true);
    }

    private PopupMenu popupMenu = null;
    public void setPopupMenu(Object menu) {
        popupMenu = (PopupMenu)menu;
        term.getDisplay().getAWTComponent().add(popupMenu);
    }

    public void showPopupMenu(int x, int y) {
        if (popupMenu != null) {
            popupMenu.show(term.ownerFrame, x, y);
        }
    }

    protected void addBasicMenusInternal(Container frame) {
        throw new RuntimeException("not implemented");
    }

    protected void aboutDialog(Frame parent, TerminalWin terminal, 
            String title, String aboutText) {
    	Dialog dialog = new BorderDialog(parent, title, true);

    	Component logo = new Logo((Image)terminal.getLogo());
    	if (logo != null) {
    		Panel p = new Panel();
    		p.add(logo);
    		dialog.add(p, BorderLayout.NORTH);
    	}

    	TextArea textArea =
    		new TextArea(aboutText, 12, 40, TextArea.SCROLLBARS_VERTICAL_ONLY);
    	textArea.setEditable(false);
    	dialog.add(textArea, BorderLayout.CENTER);

    	Button okTextBut = new Button(LBL_BTN_OK);
    	okTextBut.addActionListener(new AWTConvenience.CloseAction(dialog));

    	Panel p = new Panel(new FlowLayout(FlowLayout.RIGHT));
    	p.add(okTextBut);
    	dialog.add(p, BorderLayout.SOUTH);

    	dialog.addWindowListener(new AWTConvenience.CloseAdapter(okTextBut));

    	AWTConvenience.setBackgroundOfChildren(dialog);

    	dialog.setResizable(true);
    	dialog.pack();

    	AWTConvenience.placeDialog(dialog);
    	okTextBut.requestFocus();
    	dialog.setVisible(true);
    }

}
