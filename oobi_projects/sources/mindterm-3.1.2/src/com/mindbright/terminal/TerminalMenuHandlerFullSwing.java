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
import java.awt.Component;
import java.awt.Container;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Image;
import java.awt.Insets;
import java.awt.SystemColor;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingConstants;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.Logo;
import com.mindbright.gui.SwingConvenience;

/**
 * Implements Swing-style menus for a terminal window.
 *
 * @see TerminalWin
 * @see TerminalMenuHandlerFull
 */
public class TerminalMenuHandlerFullSwing extends TerminalMenuHandlerFull {
    private JMenu vtOptionsMenu;
    
    private static final boolean BOX_APPLET = true;

    protected void setEnabled(int i, int j, boolean v) {
        ((JMenuItem)menuItems[i][j]).setEnabled(v);
    }

    protected void setState(int i, int j, boolean v) {
        ((JCheckBoxMenuItem)menuItems[i][j]).setSelected(v);
    }

    protected boolean getState(int i, int j) {
        return ((JCheckBoxMenuItem)menuItems[i][j]).isSelected();
    }

    protected void addBasicMenusInternal(Container frame) {
        JMenuBar menubar = null;
        if (frame instanceof JFrame) {
            menubar = ((JFrame)frame).getJMenuBar();
            if (menubar == null) {
                ((JFrame)frame).setJMenuBar(menubar = new JMenuBar());
            }
        } else {
            menubar = new JMenuBar();
            frame.add(menubar,BorderLayout.NORTH);
        }
	menubar.add((JMenu)getMenu(MENU_FILE));
	menubar.add((JMenu)getMenu(MENU_EDIT));
	menubar.add((JMenu)getMenu(MENU_SETTINGS));
	menubar.add((JMenu)getMenu(MENU_HELP));
    }

    public Object getMenu(int idx) {
	JMenu m = new JMenu(menuTexts[idx][0]);
	int len = menuTexts[idx].length;
	JMenuItem mi;
	String   t;

	if (menuItems == null)
	    menuItems = new JMenuItem[menuTexts.length][];
	if (menuItems[idx] == null) {
	    menuItems[idx] = new JMenuItem[menuTexts[idx].length];
        }

	for (int i = 1; i < len; i++) {
	    t = menuTexts[idx][i];
	    if (t == null) {
		m.addSeparator();
		continue;
	    }
	    if (t.charAt(0) == '_') {
		t = t.substring(1);
		mi = new JCheckBoxMenuItem(t);
		((JCheckBoxMenuItem)mi).addItemListener(this);
	    } else {
		mi = new JMenuItem(t);
		mi.addActionListener(this);
	    }

	    if (menuShortCuts[idx][i] != NO_SHORTCUT) {
		mi.setAccelerator
                    (KeyStroke.getKeyStroke
                     (menuShortCuts[idx][i],
                      java.awt.Toolkit.getDefaultToolkit().getMenuShortcutKeyMask() | 
                      java.awt.event.InputEvent.SHIFT_MASK));
	    }

	    menuItems[idx][i] = mi;
	    m.add(mi);
	}
	return m;
    }

    private JComponent vtoptions[];
    private TerminalOption toptions[];
    
    private Component getVTOptionsPanel(boolean top) {
        int start, end;
        if (top) {
            toptions = term.getOptions();
            vtoptions = new JComponent[toptions.length];
            start = 0;
            end = toptions.length/2;
        } else {
            start = toptions.length/2;
            end = toptions.length;
        }

        if (toptions == null || toptions.length == 0)
            return null;

        JPanel p = new JPanel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.fill      = GridBagConstraints.NONE;
        gridc.anchor    = GridBagConstraints.WEST;
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.insets    = new Insets(2, 5, 2, 5);

        for (int i=start; i<end; i++) {
            String cs[] = toptions[i].getChoices();
            if (cs == null) {
                vtoptions[i] = new JCheckBox(toptions[i].getDescription(),
                                             toptions[i].getValueB());
            } else {
                gridc.gridwidth = 1;
                gridc.fill      = GridBagConstraints.HORIZONTAL;
                p.add(new JLabel(toptions[i].getDescription(),
                                 SwingConstants.RIGHT), gridc);
                vtoptions[i] = new JComboBox(cs);
                ((JComboBox)vtoptions[i]).setSelectedItem(
                    toptions[i].getValue().toLowerCase());
                gridc.gridwidth = GridBagConstraints.REMAINDER;
                gridc.fill      = GridBagConstraints.NONE;
            }
            p.add(vtoptions[i], gridc);
        }

        gridc.fill      = GridBagConstraints.BOTH;
        gridc.weightx   = 1.0;
        gridc.weighty   = 1.0;
        p.add(new JLabel(""), gridc);
        
        JScrollPane sp = new JScrollPane
            (p, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
             JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        sp.getViewport().setBackground(new java.awt.Color(p.getBackground().getRGB()));
        return sp;
    }
    
    private void setVTOptions() {
        if (vtoptions == null) return;
        
        for (int i=0; i<vtoptions.length; i++) {
            String val;
            if (vtoptions[i] instanceof JComboBox) {
                val = (String)((JComboBox)vtoptions[i]).getSelectedItem();
            } else {
                val = ((JCheckBox)vtoptions[i]).isSelected() ? "true" : "false";
            }
            setProperty(toptions[i].getKey(), val);
        }        

        vtoptions = null;
        toptions = null;
    }

    private JComboBox comboTE, comboEN, comboFN, comboSB, comboPB;
    private JComboBox comboFG, comboBG, comboCC;
    private JTextField textFS, textRows, textCols, textInitPos;
    private JTextField textSL, textSD;
    private JTextField textFG, textBG, textCC;
    private JLabel lblAlert, labelTE;
    private JCheckBox checkIN;

    public void termSettingsDialog(String title) {
        final JDialog dialog = new JDialog(term.ownerFrame, title, true);

        // general tab
        GridBagConstraints gridcl = new GridBagConstraints();
        GridBagConstraints gridcr = new GridBagConstraints();

        JPanel mp = new JPanel(new GridBagLayout());

        gridcl.fill      = GridBagConstraints.HORIZONTAL;
        gridcl.insets    = new Insets(2, 2, 2, 2);
        gridcl.gridwidth = 1;
        gridcr.anchor    = GridBagConstraints.WEST;
        gridcr.insets    = new Insets(2, 2, 2, 2);
        gridcr.gridwidth = GridBagConstraints.REMAINDER;
        gridcr.weightx   = 1.0;

	    //comboTE = new JComboBox(TERMINAL_TYPES);
	    mp.add(new JLabel(LBL_TERMINAL_TYPE, SwingConstants.RIGHT), gridcl);
	    //mp.add(comboTE, gridcr);
	    labelTE = new JLabel(getProperty("term-type"), SwingConstants.LEFT);
	    mp.add(labelTE, gridcr);
	        
        if(!BOX_APPLET) {
	        textCols = new JTextField("", 3);
	        mp.add(new JLabel(LBL_COLUMNS, SwingConstants.RIGHT), gridcl);
	        mp.add(textCols, gridcr);
	
	        textRows = new JTextField("", 3);
	        mp.add(new JLabel(LBL_ROWS, SwingConstants.RIGHT), gridcl);
	        mp.add(textRows, gridcr);
	
	        comboEN = new JComboBox(ENCODINGS);
	        comboEN.setEditable(true);
	        mp.add(new JLabel(LBL_ENCODING, SwingConstants.RIGHT), gridcl);
	        mp.add(comboEN, gridcr);
	
	        comboFN = new JComboBox(FONT_LIST);
	        mp.add(new JLabel(LBL_FONT, SwingConstants.RIGHT), gridcl);
	        mp.add(comboFN, gridcr);
	
	        textFS = new JTextField("", 3);
	        mp.add(new JLabel(LBL_SIZE, SwingConstants.RIGHT), gridcl);
	        mp.add(textFS, gridcr);
        }
        
        textSL = new JTextField("", 3);
        mp.add(new JLabel(LBL_SCROLLBACK_BUFFER, SwingConstants.RIGHT),gridcl);
        mp.add(textSL, gridcr);

        comboSB = new JComboBox(SCROLLBAR_POS);
        mp.add(new JLabel(LBL_SCROLLBAR_POS, SwingConstants.RIGHT), gridcl);
        mp.add(comboSB, gridcr);

        lblAlert = new JLabel("", SwingConstants.CENTER);
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.fill      = GridBagConstraints.BOTH;
        gridc.weighty   = 1.0;
        gridc.anchor    = GridBagConstraints.CENTER;
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(lblAlert, gridc);

        JTabbedPane tp = new JTabbedPane();
        tp.addTab(LBL_TAB_GENERAL, mp);

        // color tab
        mp = new JPanel(new GridBagLayout());

        gridc.weightx   = 0.0;
        gridc.weighty   = 0.0;
        gridc.gridwidth = 1;
        gridc.insets    = new Insets(2, 2, 2, 2);

        ItemListener ilC = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                updateColors();
            }
        };
        comboFG = new JComboBox();
        comboFG.addItemListener(ilC);
        textFG = new JTextField("", 10);
        mp.add(new JLabel(LBL_FG_COLOR, SwingConstants.RIGHT), gridc);
        mp.add(comboFG, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textFG, gridc);

        gridc.gridwidth = 1;
        comboBG = new JComboBox();
        comboBG.addItemListener(ilC);
        textBG = new JTextField("", 10);
        mp.add(new JLabel(LBL_BG_COLOR, SwingConstants.RIGHT), gridc);
        mp.add(comboBG, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textBG, gridc);

        gridc.gridwidth = 1;
        comboCC = new JComboBox();
        comboCC.addItemListener(ilC);
        textCC = new JTextField("", 10);
        mp.add(new JLabel(LBL_CURS_COLOR, SwingConstants.RIGHT), gridc);
        mp.add(comboCC, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textCC, gridc);

        gridc.fill = GridBagConstraints.BOTH;
        gridc.weightx = 1.0;
        gridc.weighty = 1.0;
        mp.add(new JLabel(""), gridc);
        
        tp.addTab(LBL_TAB_COLORS, mp);


        // misc tab
        gridc = new GridBagConstraints();
        mp = new JPanel(new GridBagLayout());
        mp.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

        gridc.insets    = new Insets(2, 2, 2, 2);
        gridc.anchor    = GridBagConstraints.WEST;
        gridc.gridwidth = 1;
        mp.add(new JLabel(LBL_PASTE_BUTTON, SwingConstants.RIGHT), gridc);

        comboPB = new JComboBox(PASTE_BUTTON);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(comboPB, gridc);

        gridc.gridwidth = 1;
        mp.add(new JLabel(LBL_SELECT_DELIM, SwingConstants.RIGHT), gridc);

        textSD = new JTextField("", 8);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textSD, gridc);

        gridc.gridwidth = 1;
        mp.add(new JLabel(), gridc);

        checkIN = new JCheckBox(
            LBL_IGN_NULL, Boolean.valueOf(getProperty("ignore-null")).booleanValue());
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(checkIN, gridc);

        gridc.fill = GridBagConstraints.BOTH;
        gridc.weightx = 1.0;
        gridc.weighty = 1.0;
        mp.add(new JLabel(""), gridc);
        
        tp.addTab(LBL_TAB_MISC, mp);


        // VT options tab (top half)
        Component vtp = getVTOptionsPanel(true);
        if (vtp != null) {
            tp.addTab(LBL_TAB_VTOPTIONS1, vtp);
        }

        // VT options tab (bottom half)
        vtp = getVTOptionsPanel(false);
        if (vtp != null) {
            tp.addTab(LBL_TAB_VTOPTIONS2, vtp);
        }

        dialog.getContentPane().add(tp, BorderLayout.CENTER);
        tp.setSelectedIndex(0);

        JButton ok = new JButton(LBL_BTN_OK);
        JButton cancel = new JButton(LBL_BTN_CANCEL);
        JPanel bp = SwingConvenience.newButtonPanel(new JButton[] {ok,cancel});
        ok.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    setVTOptions();

	                //setProperty("term-type",
	                //            TERMINAL_TYPES[comboTE.getSelectedIndex()]);
                    if(!BOX_APPLET) {
	                    setProperty("encoding", (String)comboEN.getSelectedItem());
	                    setProperty("font-name",
	                                FONT_LIST[comboFN.getSelectedIndex()]);
	                    setProperty("font-size", textFS.getText());
	                    setProperty("geometry",
	                            textCols.getText() + "x" + textRows.getText());
                    }
                    setProperty("scrollbar",
                                SCROLLBAR_POS[comboSB.getSelectedIndex()]);
                    setProperty("save-lines", textSL.getText());
                    setProperty("paste-button",
                                PASTE_BUTTON[comboPB.getSelectedIndex()]);
                    setProperty("select-delim", textSD.getText());
                    setProperty("ignore-null",
                                Boolean.toString(checkIN.isSelected()));
                    setProperty("fg-color", getSelectedColor(comboFG, textFG));
                    setProperty("bg-color", getSelectedColor(comboBG, textBG));
                    setProperty("cursor-color",
                                getSelectedColor(comboCC, textCC));
                    dialog.dispose();
                } catch (Exception ee) {
                    lblAlert.setText(ee.getMessage());
                }
            }
        });        
        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));
        
        dialog.getContentPane().add(bp, BorderLayout.SOUTH);

	//comboTE.setSelectedItem(getProperty("term-type"));
    if(!BOX_APPLET) {
		comboEN.setSelectedItem(getProperty("encoding"));
		comboFN.setSelectedItem(getProperty("font-name"));
		textFS.setText(getProperty("font-size"));
		textCols.setText(String.valueOf(term.cols()));
		textRows.setText(String.valueOf(term.rows()));
    }
	comboSB.setSelectedItem(getProperty("scrollbar"));
	textSL.setText(getProperty("save-lines"));

	comboPB.setSelectedItem(getProperty("paste-button"));
	String sdSet = getProperty("select-delim");
	if ((sdSet.charAt(0) == '"'
             && sdSet.charAt(sdSet.length() - 1) == '"')) {
	    sdSet = sdSet.substring(1, sdSet.length() - 1);
	}
	textSD.setText(sdSet);

	comboBG.addItem(LBL_CUSTOM_RGB);
	comboFG.addItem(LBL_CUSTOM_RGB);
	comboCC.addItem(LBL_CUSTOM_RGB);
	for (int i = 0; i < TerminalWin.termColorNames.length; i++) {
	    comboBG.addItem(TerminalWin.termColorNames[i]);
	    comboFG.addItem(TerminalWin.termColorNames[i]);
	    comboCC.addItem(TerminalWin.termColorNames[i]);
	}        
	initColorSelect(comboFG, textFG, getProperty("fg-color"));
	initColorSelect(comboBG, textBG, getProperty("bg-color"));
	initColorSelect(comboCC, textCC, getProperty("cursor-color"));
	updateColors();

	lblAlert.setText("");

        dialog.setResizable(true);
        dialog.pack();

        AWTConvenience.placeDialog(dialog);

    textSD.requestFocus();

	dialog.addWindowListener(SwingConvenience.getWindowDisposer());        
	dialog.setVisible(true);
    }

    private static void initColorSelect(JComboBox c, JTextField t, String colStr) {
	if (Character.isDigit(colStr.charAt(0))) {
	    c.setSelectedItem(LBL_CUSTOM_RGB);
	    t.setText(colStr);
	} else {
	    t.setText("");
	    t.setEnabled(false);
	    c.setSelectedItem(colStr);
	}
    }

    private static void checkColorSelect(JComboBox c, JTextField t) {
	int cs = c.getSelectedIndex();
    
	if (cs <= 0) {
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
	    // on the Mac, Combos can't get keyboard focus
	    // so we may need to move focus away from the JTextField
	    t.setBackground(TerminalWin.termColors[cs - 1]);
	}
    }

    private void updateColors() {
	checkColorSelect(comboFG, textFG);
	checkColorSelect(comboBG, textBG);
	checkColorSelect(comboCC, textCC);
    }

    private static String getSelectedColor(JComboBox c, JTextField t) {
	return (c.getSelectedIndex() == 0) ? t.getText() : (String)c.getSelectedItem();
    }

    private JTextField  findText;
    private JCheckBox   dirCheck, caseCheck;
    private JButton     findBut, cancBut;

    public void findDialog(String title) {
        lastSearch = null;

        JDialog dialog = SwingConvenience.newBorderJDialog(term.ownerFrame,
                                                           title, false);
        
        dialog.getContentPane().setLayout(new FlowLayout(FlowLayout.LEFT));
        
        Box b = new Box(BoxLayout.Y_AXIS);
        JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT));
        p.add(new JLabel(LBL_FIND));
        p.add(findText  = new JTextField("", 20));
        b.add(p);

        p = new JPanel(new FlowLayout(FlowLayout.LEFT));
        p.add(caseCheck = new JCheckBox(LBL_CASE_SENSITIVE));
        p.add(dirCheck  = new JCheckBox(LBL_FIND_BACKWARDS));
        b.add(p);

        dialog.getContentPane().add(b);

        p = new JPanel(new GridLayout(0, 1, 0, 5));
        p.add(findBut = new JButton(LBL_BTN_FIND));
        p.add(cancBut = new JButton(LBL_BTN_CANCEL));
        dialog.getContentPane().add(p);

        cancBut.addActionListener(new AWTConvenience.CloseAction(dialog));
        findBut.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String txt = findText.getText();
                if (txt != null && txt.length() > 0) 
                    doFind(txt, caseCheck.isSelected(), dirCheck.isSelected());
            }
        });
        
        dialog.setResizable(true);
        dialog.pack();

	AWTConvenience.placeDialog(dialog);
	findText.requestFocus();
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());        
	dialog.setVisible(true);
    }

    private JPopupMenu popupMenu = null;
    public void setPopupMenu(Object menu) {
        popupMenu = (JPopupMenu)menu;
    }

    public void showPopupMenu(int x, int y) {
        if (popupMenu != null) {
            popupMenu.show(term.ownerFrame, x, y);
        }
    }

    protected void aboutDialog(Frame parent, TerminalWin terminal, 
            String title, String aboutText) {
    	final JDialog dialog = SwingConvenience.newBorderJDialog(parent,
                                              title, true);

    	Component logo = new Logo((Image)terminal.getLogo());
    	if (logo != null) {
    		JPanel p = new JPanel();
    		p.add(logo);
    		dialog.getContentPane().add(p, BorderLayout.NORTH);
    	}

    	JTextArea textArea = new JTextArea(aboutText, 12, 40);
    	textArea.setEditable(false);
    	JScrollPane sp = new JScrollPane
    					(textArea, ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED, 
    								ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

    	dialog.getContentPane().add(sp, BorderLayout.CENTER);

    	JButton okTextBut = new JButton(LBL_BTN_OK);
    	okTextBut.addActionListener(new AWTConvenience.CloseAction(dialog));

    	JPanel p = new JPanel(new FlowLayout(FlowLayout.RIGHT));
    	p.add(okTextBut);
    	dialog.getContentPane().add(p, BorderLayout.SOUTH);

    	dialog.setResizable(true);
    	dialog.pack();

    	AWTConvenience.placeDialog(dialog);
    	okTextBut.requestFocus();
    	dialog.addWindowListener(SwingConvenience.getWindowDisposer());
    	dialog.setVisible(true);
    }

}
