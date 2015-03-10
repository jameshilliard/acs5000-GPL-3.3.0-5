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

package com.mindbright.ssh;

import java.awt.BorderLayout;
import java.awt.CardLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.File;
import java.util.Arrays;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPasswordField;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.ListSelectionModel;
import javax.swing.ScrollPaneConstants;
import javax.swing.SwingConstants;
import javax.swing.border.TitledBorder;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.GUI;
import com.mindbright.gui.SwingConvenience;
import com.mindbright.ssh2.SSH2Preferences;
import com.mindbright.terminal.TerminalMenuHandlerFull;

public class SSHMenuHandlerFullSwing extends SSHMenuHandlerFull 
{
    
//     public SSHMenuHandlerFullSwing() {
//         super();
//     }

    public void setupMenuBar(boolean usePopupMenu, boolean appletMode) { 
        if (usePopupMenu) {
            JPopupMenu pm = new JPopupMenu(MENU_HEADER_POPUP);
            preparePopupMenu(pm);
        } else {
            JMenuBar mb = new JMenuBar();
            prepareMenuBar(mb);
            
	      // [CYCLADES-START]
            // Modified by Eduardo Murai Soares 
            // Swing Menu is no longer set directly in the Frame but
            // it is added to the container instead (for applet).
            
            //((JFrame)parent).setJMenuBar(mb); // CODE COMMENTED
            
            if (appletMode) {
            	if (super.getMindterm().getAppContainer() instanceof javax.swing.JFrame) {
            		javax.swing.JFrame frm = (javax.swing.JFrame) super.getMindterm().getAppContainer();
            		frm.getContentPane().add(mb, BorderLayout.NORTH);
            		}            		
            	else super.getMindterm().getAppContainer().add(mb, BorderLayout.NORTH);
            	}
            else 
            //((JFrame)parent).setJMenuBar(mb);
            mindterm.add(mb,BorderLayout.NORTH);
            // [CYCLADES-END]
	    
        }        
    }

    private void prepareMenuBar(JMenuBar mb) {
	mb.add(getMenu(MENU_FILE));
	mb.add((JMenuItem) ((TerminalMenuHandlerFull)term.getMenus()).getMenu
               (TerminalMenuHandlerFull.MENU_EDIT));
	mb.add(getMenu(MENU_SETTINGS));
	/*JMenu pm = getPluginMenu();
	if (pm != null)
	    mb.add(pm);
	mb.add(getMenu(MENU_TUNNELS));*/
	mb.add(getMenu(MENU_HELP));
	term.updateMenus();
    }

    private void preparePopupMenu(JPopupMenu popupmenu) {
	havePopupMenu = true;
	popupmenu.add(getMenu(MENU_FILE));
	popupmenu.add(
            (JMenuItem)((TerminalMenuHandlerFull)term.getMenus()).getMenu
            (TerminalMenuHandlerFull.MENU_EDIT));
	popupmenu.add(getMenu(MENU_SETTINGS));
	/*JMenu pm = getPluginMenu();
	if (pm != null) {
	    popupmenu.add(pm);
	}
	popupmenu.add(getMenu(MENU_TUNNELS));*/
	popupmenu.addSeparator();
	popupmenu.add(getMenu(MENU_HELP));
        term.getMenus().setPopupMenu(popupmenu);
	update();
    }    

    private Object[][] menuItems;
    private JMenuItem[] modMenuItems;

    private JMenu getMenu(int idx) {
	JMenu m = new JMenu(menuTexts[idx][0]);
	int len = menuTexts[idx].length;
	JMenuItem mi;
	String   t;

	if (menuItems == null)
	     menuItems = new Object[menuTexts.length][];
	if (menuItems[idx] == null)
	    menuItems[idx] = new Object[menuTexts[idx].length];

	for (int i = 1; i < len; i++) {
	    t = menuTexts[idx][i];
	    if (t == null) {
		m.addSeparator();
		continue;
	    }

	    if (t.charAt(0) == '_') {
		t = t.substring(1);
		mi = new JCheckBoxMenuItem(t);
		((JCheckBoxMenuItem)mi).addActionListener(this);
	    } else {
		mi = new JMenuItem(t);
		mi.addActionListener(this);
	    }

            int sc = getMenuShortCut(idx, i);
 	    if (sc != NO_SHORTCUT) {
 		mi.setAccelerator
                    (KeyStroke.getKeyStroke
                     (sc,
                      java.awt.Toolkit.getDefaultToolkit().getMenuShortcutKeyMask() | 
                      java.awt.event.InputEvent.SHIFT_MASK));
 	    }

	    menuItems[idx][i] = mi;
	    m.add(mi);
	}
	return m;
    }

    private JMenu getPluginMenu() {
        modMenuItems = null;
	JMenu m = null;
	if (modCnt > 0) {
            modMenuItems = new JMenuItem[modCnt];
	    m = new JMenu(MENU_HEADER_PLUGINS);
	    int i = 0;
	    for (; i < modCnt; i++) {
		String label = getModuleLabel(i);
		if (label != null) {
		    JMenuItem mi = new JMenuItem(label);
	    	    modMenuItems[i] = mi;
		    mi.addActionListener(new Actions(ACT_MOD_BASE + i));
		    m.add(mi);
		}
	    }
	    if (i == 0) m = null;
	}
	return m;
    }

    protected void setEnabled(int i, int j, boolean v) {
    	if(menuItems!=null) {
    		((JMenuItem)menuItems[i][j]).setEnabled(v);
    	}
    }

    protected void setState(int i, int j, boolean v) {
        ((JCheckBoxMenuItem)menuItems[i][j]).setSelected(v);
    }

    protected boolean getState(int i, int j) {
        return ((JCheckBoxMenuItem)menuItems[i][j]).isSelected();
    }

    protected void updatePluginMenu() {
	for (int i = 0; i < modCnt; i++)
	    if (getModuleLabel(i) != null)
			// [CYCLADES-START] : this code is necessary to avoid NullPointer exception (!??)
			{
			if (modMenuItems[i]==null || modules[i]==null) continue;
	    	// [CYCLADES-END]
			modMenuItems[i].setEnabled(modules[i].isAvailable(client));
			// [CYCLADES-START]
			}
			// [CYCLADES-END]
    }

    protected String getMenuLabel(Object o) {
        return ((JMenuItem)o).getText();
    }

    public void connectDialog(String title) {
        sshConfigDialog(title, IDX_TAB_GENERAL);
    }

    public void keyGenerationDialogCreate(String title) {
        SSHKeyGenerationDialogSwing.show(title, parent, client);
    }
    
    public void keyGenerationDialogEdit(String title) {
        SSHKeyGenerationDialogSwing.editKeyDialog(title, parent, client);
    }

    protected void sshNewServerDialog(String title) {
        sshConfigDialog(title, IDX_TAB_GENERAL);
    }

    public void sshPreferencesDialog(String title) {
        sshConfigDialog(title, IDX_TAB_SECURITY);
    }

    // General stuff
    private JComboBox   comboAuthTyp, comboSrv;
    private JCheckBox   cbSaveAlias;
    private JTextField  textPort, textUser, textAlias, textPrivateKey, textPrivateHostKey;
    private JPasswordField textPwd;
    private JButton     customBtn, browseBtn;
    private CardLayout  authLabelCL, authCL;
    private JPanel      authLabelCP, authCP;
    private String      customAuth;

    // Proxy stuff
    private JComboBox   comboPrxType;
    private JCheckBox   cbNeedAuth;
    private JTextField  textPrxHost, textPrxPort, textPrxUser;
    private JPasswordField textPrxPasswd;

    // Security stuff
    private JCheckBox cbProto1, cbProto2;
    private JCheckBox cbKeyTypeDSS, cbKeyTypeRSA;
    private JComboBox comboCipherC2S, comboCipherS2C, comboMacC2S, comboMacS2C;
    private JComboBox comboCompC2S, comboCompS2C;
    private JCheckBox cbX11, cbIdHost, cbKeyNoise, cbAlive, cbForcPty;
    private JTextField textDisp, textMtu, textAlive;

    protected void sshConfigDialog(String title, int first) {
        final JDialog dialog = new JDialog(parent, title, true);

        GridBagLayout      grid  = new GridBagLayout();
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.fill   = GridBagConstraints.HORIZONTAL;
        gridc.anchor = GridBagConstraints.WEST;
        gridc.insets = new Insets(2,2,2,2);
        gridc.gridheight = 1;
        gridc.weightx = 0.0;
        gridc.weighty = 0.0;

        JTabbedPane tp = new JTabbedPane();

        ItemListener ilserv = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                serverSelected();
            }
        };
        ItemListener ilg = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                updateChoicesGeneral();
            }
        };
        ItemListener ilp = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                updateChoicesProxy(e);
            }
        };
        ItemListener ils = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                updateChoicesSecurity(e);
            }
        };
        ItemListener ilf = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                updateChoicesFeatures();
            }
        };

        /****************************************************************
         * General tab
         */
        JPanel mp = new JPanel(grid);
        
        JLabel lbl = new JLabel(LBL_SERVER, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        comboSrv = new JComboBox(getAvailableAliases());
        comboSrv.setEditable(true);
        comboSrv.addItemListener(ilserv);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        mp.add(comboSrv, gridc);
        gridc.weightx = 0.0;
        
        lbl = new JLabel(LBL_USERNAME, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        textUser = new JTextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textUser, gridc);

        lbl = new JLabel(LBL_AUTH, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        comboAuthTyp = new JComboBox(AUTH_METHODS);
        comboAuthTyp.addItemListener(ilg);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.fill = GridBagConstraints.NONE;
        mp.add(comboAuthTyp, gridc);
        gridc.fill = GridBagConstraints.HORIZONTAL;
        
        gridc.gridwidth = 1;
        mp.add(getAuthLabel(), gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(getAuth(), gridc);

        cbSaveAlias = new JCheckBox(LBL_SAVE_AS_ALIAS);
        cbSaveAlias.addItemListener(ilg);
        gridc.insets = new Insets(10, 2, 2, 2);
        gridc.gridwidth = 1;
        mp.add(cbSaveAlias, gridc);
        textAlias = new JTextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textAlias, gridc);

        gridc.weighty = 1.0;
        mp.add(Box.createVerticalGlue(), gridc);
        gridc.weighty = 0.0;

        tp.addTab(LBL_TAB_GENERAL, mp);

        /****************************************************************
         * Proxy tab
         */
        mp = new JPanel(grid);

        lbl = new JLabel(LBL_PROXY_TYPE, SwingConstants.RIGHT);
        gridc.insets = new Insets(2, 2, 2, 2);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        comboPrxType = new JComboBox(SSH.getProxyTypes());
        comboPrxType.addItemListener(ilp);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.fill = GridBagConstraints.NONE;
        mp.add(comboPrxType, gridc);
        gridc.fill = GridBagConstraints.HORIZONTAL;

        lbl = new JLabel(LBL_SERVER, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        textPrxHost = new JTextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        mp.add(textPrxHost, gridc);
        gridc.weightx = 0.0;

        lbl = new JLabel(LBL_PORT, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        textPrxPort = new JTextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textPrxPort, gridc);

        JPanel ap = new JPanel(new GridBagLayout());
	TitledBorder titleBorder 
	    = BorderFactory.createTitledBorder(BorderFactory.
					       createEtchedBorder());
	titleBorder.setTitle(LBL_AUTH_REQ);
	ap.setBorder(titleBorder);

        cbNeedAuth = new JCheckBox(LBL_AUTH);
        cbNeedAuth.addItemListener(ilp);
        ap.add(cbNeedAuth, gridc);

        lbl = new JLabel(LBL_USERNAME, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        ap.add(lbl, gridc);
        textPrxUser = new JTextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        ap.add(textPrxUser, gridc);
        gridc.weightx = 0.0;

        lbl = new JLabel(LBL_PASSWORD, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        ap.add(lbl, gridc);
        textPrxPasswd = new JPasswordField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        ap.add(textPrxPasswd, gridc);

        mp.add(ap, gridc);

        gridc.weighty = 1.0;
        mp.add(Box.createVerticalGlue(), gridc);
        gridc.weighty = 0.0;

        tp.addTab(LBL_TAB_PROXY, mp);

        /****************************************************************
         * Security tab
         */
        mp = new JPanel(grid);

        lbl = new JLabel(LBL_PROTOCOL, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        cbProto2 = new JCheckBox(LBL_PROTO_SSH2);
        cbProto2.addItemListener(ils);
        mp.add(cbProto2, gridc);
        cbProto1 = new JCheckBox(LBL_PROTO_SSH1);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(cbProto1, gridc);

        lbl = new JLabel(LBL_HKEY_TYPE, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        cbKeyTypeDSS = new JCheckBox(LBL_HKEY_DSS);
        mp.add(cbKeyTypeDSS, gridc);
        cbKeyTypeRSA = new JCheckBox(LBL_HKEY_RSA);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(cbKeyTypeRSA, gridc);

        gridc.gridwidth = 1;
        mp.add(new JPanel(), gridc);
        gridc.gridwidth = 2;
        mp.add(new JLabel(LBL_C2S), gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(new JLabel(LBL_S2C), gridc);

        comboCipherC2S = new JComboBox(ciphers);
        comboCipherS2C = new JComboBox(ciphers);
        comboMacC2S    = new JComboBox(macs);
        comboMacS2C    = new JComboBox(macs);
        comboCompC2S   = new JComboBox(compc2s);
        comboCompS2C   = new JComboBox(comps2c);
        
        comboCipherC2S.insertItemAt(LBL_ANY_STANDARD, 0);
        comboCipherS2C.insertItemAt(LBL_ANY_STANDARD, 0);
        comboMacC2S.insertItemAt(LBL_ANY_STANDARD, 0);
        comboMacS2C.insertItemAt(LBL_ANY_STANDARD, 0);

        comboCipherC2S.addItemListener(ils);
        comboMacC2S.addItemListener(ils);
        comboCompC2S.addItemListener(ils);

        lbl = new JLabel(LBL_CIPHER, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        gridc.gridwidth = 2;
        mp.add(comboCipherC2S, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(comboCipherS2C, gridc);

        lbl = new JLabel(LBL_MAC, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        gridc.gridwidth = 2;
        mp.add(comboMacC2S, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(comboMacS2C, gridc);

        lbl = new JLabel(LBL_COMP, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        gridc.gridwidth = 2;
        mp.add(comboCompC2S, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(comboCompS2C, gridc);

        gridc.weighty = 1.0;
        mp.add(Box.createVerticalGlue(), gridc);
        gridc.weighty = 0.0;

        tp.addTab(LBL_TAB_SECURITY, mp);

        /****************************************************************
         * features tab
         */
        mp = new JPanel(grid);

        lbl = new JLabel(LBL_X11_FORWARD, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        JPanel p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        cbX11 = new JCheckBox();
        cbX11.addItemListener(ilf);
        textDisp = new JTextField("", 12);
        p.add(cbX11);
        p.add(new JLabel(LBL_LOCAL_DISP));
        p.add(Box.createRigidArea(new Dimension(4, 0)));
        p.add(textDisp);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        mp.add(p, gridc);
        gridc.weightx = 0.0;

        lbl = new JLabel(LBL_SEND_KEEP, SwingConstants.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        p = new JPanel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        cbAlive = new JCheckBox();
        cbAlive.addItemListener(ilf);
        textAlive = new JTextField("", 12);
        p.add(cbAlive);
        p.add(new JLabel(LBL_INTERVAL));
        p.add(Box.createRigidArea(new Dimension(4, 0)));
        p.add(textAlive);
        p.add(new JLabel(LBL_SECONDS));
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(p, gridc);

        gridc.gridwidth = 1;
        mp.add(new JPanel(), gridc);
        cbIdHost = new JCheckBox(LBL_STRICT);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(cbIdHost, gridc);

        gridc.gridwidth = 1;
        mp.add(new JPanel(), gridc);
        cbKeyNoise = new JCheckBox(LBL_KEY_NOISE);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(cbKeyNoise, gridc);

        gridc.gridwidth = 1;
        mp.add(new JPanel(), gridc);
        cbForcPty = new JCheckBox(LBL_ALLOC_PTY);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(cbForcPty, gridc);

        gridc.weighty = 1.0;
        mp.add(Box.createVerticalGlue(), gridc);
        gridc.weighty = 0.0;

        tp.addTab(LBL_TAB_FEATURES, mp);

        /****************************************************************
         * Add stuff to dialog
         */
        dialog.getContentPane().add(tp, BorderLayout.CENTER);
        tp.setSelectedIndex(first);

        JButton ok = new JButton();
        if (client.isConnected()) {
            ok.setText(LBL_BTN_OK);
        } else {
            ok.setText(LBL_BTN_CONNECT);
        }
        JButton cancel = new JButton(LBL_BTN_CANCEL);
        JPanel bp = SwingConvenience.newButtonPanel(new JButton[] {ok,cancel});
        dialog.getContentPane().add(bp, BorderLayout.SOUTH);

        dialog.getRootPane().setDefaultButton(ok);

        dialog.setResizable(true);
        dialog.pack();

	AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());        

        ph.clearAllForwards();
        populate();
	updateChoicesGeneral();
	updateChoicesProxy(null);
	updateChoicesSecurity(null);
	updateChoicesFeatures();

	if (comboSrv.isEnabled()) {
	    comboSrv.requestFocus();
        } else {
	    textUser.requestFocus();
        }

        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));
        ok.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                try {
                    okPressed(dialog);
                } catch (Exception ee) {
                    alertDialog("Error: " + ee.getMessage());
                }
            }
        });      

	dialog.setVisible(true);
    }

    private void serverSelected() {
        Object item = comboSrv.getModel().getSelectedItem();
        if (item == null)
            return;

        String host = item.toString().trim();
        if (host.length() == 0)
            return;

        try {
            String pwd = "";
            do {
                try {
                    // If menu item was selected then elect save alias
                    if (-1 != comboSrv.getSelectedIndex()) {
                        cbSaveAlias.setSelected(true);
                        textAlias.setText(host);
                    }

                    ph.setPropertyPassword(pwd);
                    ph.loadAliasFile(host, false);
                    try {
                        ph.saveAsCurrentFile(ph.getSSHHomeDir() + host + ph.PROPS_FILE_EXT);
                    } catch (java.io.IOException e) {
                        System.err.println("Failed to save");
                    }
                    populate();
                    break;
                } catch(SSHClient.AuthFailException ee) {
                }
            } while((pwd = passwordDialog(
                         "Please give file password for " +
                         host, "MindTerm - File Password")) != null);
        } catch (Throwable t) {
        }
    }

    private void okPressed(JDialog dialog) throws Exception {
        if (!client.isConnected()) {
            if (!extractDataFromGeneral()) {
                return;
            }
            extractDataFromProxy();
        }
        extractDataFromSecurity();
        extractDataFromFeatures();

        dialog.dispose();
    }

    private boolean extractDataFromGeneral() {
        String host = null;
        int port = 22, i;

        host = comboSrv.getModel().getSelectedItem().toString().trim();
        if (host.length() == 0) {
            alertDialog("Please specify a server to connect to");
            return false;
        }
        if ((i = host.indexOf(':')) > 0) {
            port = Integer.parseInt(host.substring(i + 1));
            host = host.substring(0, i);
        }
        ph.setProperty("server", host);
        ph.setProperty("port", port);
        ph.setProperty("usrname", textUser.getText());

        if (cbSaveAlias.isSelected()) {
            String alias = textAlias.getText();
            if (alias == null || alias.trim().length() == 0) {
                alertDialog("Please specify an alias name for these settings");
                return false;
            }
            if (ph.savePasswords) {
                String pwd =
                    setPasswordDialog(
                        "Please set password for alias " + host,
                        "MindTerm - Set File Password");
                if (pwd == null)
                    return false;
                ph.setPropertyPassword(pwd);
            }
            ph.setAlias(alias);
        }

        client.quiet = true;

        String prxPasswd = ph.getProperty("prxpassword");
        ph.clearPasswords();
        if (prxPasswd != null)
            ph.setProperty("prxpassword", prxPasswd);

        String authType = (String)comboAuthTyp.getSelectedItem();
         if (authType.equals(LBL_CUSTOM_LIST)) {
             ph.setProperty("authtyp", customAuth);
         } else {
             ph.setProperty("authtyp", authType);
         }

        String pwd = null;
        char pwdc[] = textPwd.getPassword();
        if (pwdc != null && pwdc.length > 0) {
            pwd = new String(pwdc);
        }
        ph.setProperty("password", pwd);
        ph.setProperty("private-key", textPrivateKey.getText());
        ph.setProperty("private-host-key", textPrivateHostKey.getText());

        try {
            ph.saveAsCurrentFile(ph.getSSHHomeDir() + host + ph.PROPS_FILE_EXT);
        } catch (java.io.IOException e) {
            System.err.println("Failed to save");
        }
        // This causes the connect to commence
        client.sshStdIO.breakPromptLine();
        return true;
    }

    private void extractDataFromProxy() {
        String prxTypeStr = (String)comboPrxType.getSelectedItem();
        ph.setProperty("proxytype", prxTypeStr);
        if (!"none".equalsIgnoreCase(prxTypeStr)) {
            ph.setProperty("proxyhost", textPrxHost.getText());
            ph.setProperty("proxyport", textPrxPort.getText());
        }
        if (cbNeedAuth.isSelected()) {
            ph.setProperty("proxyuser", textPrxUser.getText());
            String pwd = null;
            char pwdc[] = textPrxPasswd.getPassword();
            if (pwdc != null && pwdc.length > 0) {
                pwd = new String(pwdc);
            }
            ph.setProperty("prxpassword", pwd);
        } else if ("socks4".equals(prxTypeStr)) {
            ph.setProperty("proxyuser", textPrxUser.getText());
        }
    }

    private void extractDataFromSecurity() throws Exception {
        String cipherC2S = (String)comboCipherC2S.getSelectedItem();
        String cipherS2C = (String)comboCipherS2C.getSelectedItem();
        boolean doReKey  = (client.isConnected() &&
                            client.isSSH2 &&
                            !client.transport.incompatibleCantReKey);
        if (doReKey) {
            checkSupportedByPeer();
        }

        if (cbProto1.isSelected() && cbProto2.isSelected()) {
            ph.setProperty("protocol", "auto");
        } else if (cbProto1.isSelected()) {
            ph.setProperty("protocol", "ssh1");
        } else if (cbProto2.isSelected()) {
            ph.setProperty("protocol", "ssh2");
        } else {
            throw new Exception(ERR_NO_PROTOCOL);
        }
        if (cbKeyTypeDSS.isSelected() && cbKeyTypeRSA.isSelected()) {
            ph.setProperty("server-host-key-algorithms", "ssh-rsa,ssh-dss");
        } else if (cbKeyTypeDSS.isSelected()) {
            ph.setProperty("server-host-key-algorithms", "ssh-dss");
        } else if (cbKeyTypeRSA.isSelected()) {
            ph.setProperty("server-host-key-algorithms", "ssh-rsa");
        } else {
            throw new Exception(ERR_NO_KEYTYPE);
        }
        if (comboCipherC2S.getSelectedIndex() > 0) {
            ph.setProperty("enc-algorithms-cli2srv", cipherC2S);
            ph.setProperty("cipher", cipherC2S);
        } else {
            ph.resetProperty("enc-algorithms-cli2srv");
            ph.resetProperty("cipher");
        }
        if (comboCipherS2C.getSelectedIndex() > 0) {
            ph.setProperty("enc-algorithms-srv2cli", cipherS2C);
        } else {
            ph.resetProperty("enc-algorithms-srv2cli");
        }
        if (comboMacC2S.getSelectedIndex() > 0) {
            ph.setProperty("mac-algorithms-cli2srv", 
                           (String)comboMacC2S.getSelectedItem());
        } else {
            ph.resetProperty("mac-algorithms-cli2srv");
        }
        if (comboMacS2C.getSelectedIndex() > 0) {
            ph.setProperty("mac-algorithms-srv2cli", 
                           (String)comboMacS2C.getSelectedItem());
        } else {
            ph.resetProperty("mac-algorithms-srv2cli");
        }
                    
        int compLevel = comp2lvl[comboCompC2S.getSelectedIndex()];
        if (compLevel > 0) {
            ph.setProperty("comp-algorithms-cli2srv", "zlib,zlib@openssh.com");
        } else {
            ph.setProperty("comp-algorithms-cli2srv", "none");
        }
        ph.setProperty("compression", compLevel);
        compLevel = comboCompS2C.getSelectedIndex();
        if (compLevel > 0) {
            ph.setProperty("comp-algorithms-srv2cli", "zlib,zlib@openssh.com");
        } else {
            ph.setProperty("comp-algorithms-srv2cli", "none");
        }
                    
        if (doReKey) {
            SSH2Preferences prefs = new SSH2Preferences(ph.getProperties());
            client.transport.startKeyExchange(prefs);
        }
    }

    private void extractDataFromFeatures() {
        if (cbX11.isSelected()) {
            ph.setProperty("display", textDisp.getText());
        } else {
            ph.setProperty("display", "");
        }
        ph.setProperty("x11fwd", cbX11.isSelected());
        ph.setProperty("stricthostid", cbIdHost.isSelected());
        ph.setProperty("key-timing-noise", cbKeyNoise.isSelected());
        ph.setProperty("forcpty", cbForcPty.isSelected());
        if (cbAlive.isSelected()) {
            ph.setProperty("alive", textAlive.getText());
        } else {
            ph.setProperty("alive", "0");
        }
    }

    private JPanel getAuthLabel() {
	authLabelCP = new JPanel();
	authLabelCP.setLayout(authLabelCL = new CardLayout());

        int r = SwingConstants.RIGHT;
	authLabelCP.add(new JLabel(LBL_PASSWORD, r), "password");
	authLabelCP.add(new JPanel(), LBL_CUSTOM_LIST);
	authLabelCP.add(new JLabel(LBL_IDENTITY, r), "publickey");
	authLabelCP.add(new JPanel(), "securid");
	authLabelCP.add(new JPanel(), "tis");
	authLabelCP.add(new JPanel(), "kbd-interact");
	authLabelCP.add(new JLabel(LBL_HOST_KEY, r), "hostbased");

        return authLabelCP;
    }

    private Component makeCenter(Component c) {
        JPanel p = new JPanel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();        
        gridc.fill = GridBagConstraints.HORIZONTAL;
        gridc.weightx = 1.0;
        p.add(c, gridc);
        return p;
    }

    private JPanel getAuth() {
	authCP = new JPanel();
	authCP.setLayout(authCL = new CardLayout());

        textPwd = new JPasswordField("", 12);
	textPwd.setEchoChar('*');

        customBtn = new JButton(LBL_MODIFY_LIST);
        customBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                customAuthDialog();
            }
        });      

        textPrivateKey = new JTextField("", 12);
        JButton browseBtn = new JButton(LBL_BTN_BROWSE);
        browseBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                File file = GUI.selectFile(
                    parent,
                    "MindTerm - Select file with identity (private)",
                    ph.getSSHHomeDir(), false);
                if (file != null)
                    textPrivateKey.setText(file.getAbsolutePath());
            }
        });
        JPanel p = new JPanel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();        
        gridc.fill = GridBagConstraints.HORIZONTAL;
        gridc.weightx = 1.0;
        p.add(textPrivateKey, gridc);
        gridc.weightx = 0.0;
        p.add(browseBtn, gridc);

	authCP.add(makeCenter(textPwd), "password");
	authCP.add(makeCenter(customBtn), LBL_CUSTOM_LIST);
	authCP.add(p, "publickey");
	authCP.add(new JLabel(), "tis");
	authCP.add(new JLabel(), "kbd-interact");
 	authCP.add(new JPanel(), "securid");
 	authCP.add(new JPanel(), "tis");
 	authCP.add(new JPanel(), "kbd-interact");

        textPrivateHostKey = new JTextField("", 12);
        browseBtn = new JButton(LBL_BTN_BROWSE);
        browseBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                File file = GUI.selectFile(
                    parent,
                    "MindTerm - Select file with private host key",
                    ph.getSSHHomeDir(), false);
                if (file != null)
                    textPrivateHostKey.setText(file.getAbsolutePath());
            }
        });
        p = new JPanel(new GridBagLayout());
        gridc = new GridBagConstraints();        
        gridc.fill = GridBagConstraints.HORIZONTAL;
        gridc.weightx = 1.0;
        p.add(textPrivateHostKey, gridc);
        gridc.weightx = 0.0;
        p.add(browseBtn, gridc);

        authCP.add(p, "hostbased");

        return authCP;
    }

    private void updateChoicesGeneral() {
        comboSrv.setEnabled(!client.isOpened());
        textUser.setEnabled(!client.isOpened());
        cbSaveAlias.setEnabled(!client.isOpened());
        textAlias.setEnabled(!client.isOpened());
        comboAuthTyp.setEnabled(!client.isOpened());
        textPwd.setEnabled(!client.isOpened());
        customBtn.setEnabled(!client.isOpened());
        textPrivateKey.setEnabled(!client.isOpened());
        textPrivateHostKey.setEnabled(!client.isOpened());

	String auth = (String)comboAuthTyp.getSelectedItem();
	authLabelCL.show(authLabelCP, auth);
	authCL.show(authCP, auth);
	if (cbSaveAlias.isSelected()) {
            textAlias.setEnabled(true);
	    String t = textAlias.getText();
	    if (!textAlias.isEnabled() &&
                (t == null || t.trim().length() == 0)) {
		textAlias.setText(
                    comboSrv.getModel().getSelectedItem().toString().trim());
		textAlias.requestFocus();
	    }
	} else {
	    textAlias.setText("");
	    textAlias.setEnabled(false);
	}
    }

    private void updateChoicesProxy(ItemEvent e) {
        if (e == null || e.getSource() == comboPrxType) {
            int pt = SSH.getProxyType((String)comboPrxType.getSelectedItem());
            textPrxPort.setText(String.valueOf(SSH.defaultProxyPorts[pt]));
        }

        boolean proxyEnable = true;
        boolean authEnable  = true;
        String  proxyType   = (String)comboPrxType.getSelectedItem();
        int     type        = SSH.PROXY_NONE;

        try {
            type = SSH.getProxyType(proxyType);
        } catch (Exception ee) {} // Ignored
        if (type == SSH.PROXY_NONE) {
            proxyEnable = false;
        }
        if (type == SSH.PROXY_NONE || type == SSH.PROXY_SOCKS4) {
            authEnable = false;
        }
        if (client.isConnected()) {
            proxyEnable = false;
            comboPrxType.setEnabled(false);
        } else {
            comboPrxType.setEnabled(true);
        }
        textPrxHost.setEnabled(proxyEnable);
        textPrxPort.setEnabled(proxyEnable);
        cbNeedAuth.setEnabled(authEnable);

        if (!authEnable)
            cbNeedAuth.setSelected(false);

        textPrxUser.setEnabled(cbNeedAuth.isSelected());
        textPrxPasswd.setEnabled(cbNeedAuth.isSelected());
    }

    private void updateChoicesSecurity(ItemEvent e) {
	boolean isOpen = client.isOpened();

        boolean isSSH2;
        if (isOpen) {
            isSSH2 = client.isSSH2;
        } else {
            isSSH2 = cbProto2.isSelected();
        }

        cbProto1.setEnabled(!isOpen);
        cbProto2.setEnabled(!isOpen);
        cbKeyTypeDSS.setEnabled(!isOpen);
        cbKeyTypeRSA.setEnabled(!isOpen);

	boolean incompat = false;
	if (client.transport != null) {
	    incompat = client.transport.incompatibleCantReKey;
	}
	boolean tpset  = !isOpen || (isSSH2 && isOpen && !incompat);
	comboCipherS2C.setEnabled(tpset && isSSH2);
	comboMacS2C.setEnabled(tpset && isSSH2);
	comboCompS2C.setEnabled(tpset && isSSH2);
	comboCipherC2S.setEnabled(tpset);
	comboMacC2S.setEnabled(tpset && isSSH2);
	comboCompC2S.setEnabled(tpset);

        if (e != null && e.getSource() == comboCipherC2S) {
            comboCipherS2C.setSelectedItem((String)e.getItem());
        } else if (e != null && e.getSource() == comboMacS2C) {
            comboMacS2C.setSelectedItem((String)e.getItem());
        } else if (e != null && e.getSource() == comboCompS2C) {
            if ("none".equals(e.getItem())) {
                comboCompS2C.setSelectedItem("none");
            } else {
                comboCompS2C.setSelectedItem("medium");
            }
        }
    }

    private void updateChoicesFeatures() {
	boolean isOpen = client.isOpened();
	cbX11.setEnabled(!isOpen);
	cbIdHost.setEnabled(!isOpen);
	cbForcPty.setEnabled(!isOpen);

        textAlive.setEnabled(cbAlive.isSelected());
        textDisp.setEnabled(cbX11.isSelected());
    }

    // Populate the new server dialog
    private void populate() {
        // General
        String port = ph.getProperty("port");
        String server;
        if (port.length() > 0 && !port.equals("22")) {
            server = ph.getProperty("server") + ":" + port;
        } else {
            server = ph.getProperty("server");
        }
        if (server != null)
            comboSrv.getModel().setSelectedItem(server);
        textUser.setText(ph.getProperty("username"));
        textPrivateKey.setText(ph.getProperty("private-key"));
        textPrivateHostKey.setText(ph.getProperty("private-host-key"));
        customAuth = ph.getProperty("auth-method");
        if (-1 == customAuth.indexOf(',')) {
            comboAuthTyp.setSelectedItem(customAuth);
        } else {
            comboAuthTyp.setSelectedItem(LBL_CUSTOM_LIST);
        }

        // Proxy
        comboPrxType.getModel().setSelectedItem(ph.getProperty("proxytype"));
        textPrxHost.setText(ph.getProperty("proxyhost"));
        textPrxPort.setText(ph.getProperty("proxyport"));
        textPrxUser.setText(ph.getProperty("proxyuser"));
    
        // Security
        cbKeyTypeDSS.setSelected(true);
        cbKeyTypeRSA.setSelected(true);
	comboCipherC2S.setSelectedIndex(0);
	comboCipherS2C.setSelectedIndex(0);
	comboMacC2S.setSelectedIndex(0);
	comboMacS2C.setSelectedIndex(0);

        cbProto1.setSelected(!ph.getProperty("protocol").equals("ssh2"));
        cbProto2.setSelected(!ph.getProperty("protocol").equals("ssh1"));
        cbKeyTypeDSS.setSelected(
            -1!=ph.getProperty("server-host-key-algorithms").indexOf("ssh-dss"));
        cbKeyTypeRSA.setSelected(
            -1!=ph.getProperty("server-host-key-algorithms").indexOf("ssh-rsa"));
	comboCipherC2S.setSelectedItem(ph.getProperty("enc-algorithms-cli2srv"));
	comboCipherS2C.setSelectedItem(
            ph.getProperty("enc-algorithms-srv2cli"));
	comboMacC2S.setSelectedItem(ph.getProperty("mac-algorithms-cli2srv"));
	comboMacS2C.setSelectedItem(ph.getProperty("mac-algorithms-srv2cli"));

	int compLevel = ph.getCompressionLevel();
	comboCompC2S.setSelectedItem(lvl2comp[compLevel]);
	String s2cComp = ph.getProperty("comp-algorithms-srv2cli");
	if ("none".equals(s2cComp)) {
	    comboCompS2C.setSelectedItem("none");
	} else {
	    comboCompS2C.setSelectedItem("medium");
	}

        // Features
	textDisp.setText(ph.getProperty("display"));
	textAlive.setText(ph.getProperty("alive"));

	cbX11.setSelected(ph.getPropertyB("x11fwd"));
	cbAlive.setSelected(!ph.getProperty("alive").equals("0"));

	cbIdHost.setSelected(ph.getPropertyB("stricthostid"));
	cbKeyNoise.setSelected(ph.getPropertyB("key-timing-noise"));
	cbForcPty.setSelected(ph.getPropertyB("forcpty"));
    }

    private void updateCheckedText(JCheckBox cb, JTextField text,
                                   String propName) {
	if (!text.isEnabled()) {
	    if (cb.isSelected()) {
		text.setText(ph.getProperty(propName));
	    } else {
		text.setText(ph.getDefaultProperty(propName));
	    }
	}
	text.setEnabled(cb.isEnabled() && cb.isSelected());
	if (!text.isEnabled()) {
	    text.setText(ph.getDefaultProperty(propName));
	}
    }

    private void checkSupportedByPeer() throws Exception {
	checkSupportedByPeer(SSH2Preferences.CIPHERS_C2S, comboCipherC2S);
	checkSupportedByPeer(SSH2Preferences.CIPHERS_S2C, comboCipherS2C);
	checkSupportedByPeer(SSH2Preferences.MACS_C2S, comboMacC2S);
	checkSupportedByPeer(SSH2Preferences.MACS_S2C, comboMacS2C);
	if ((!((String)comboCompC2S.getSelectedItem()).equals("none") &&
	    !client.transport.getPeerPreferences().
	    isSupported(SSH2Preferences.COMP_C2S, "zlib"))
	   ||
	   (!((String)comboCompS2C.getSelectedItem()).equals("none") &&
	    !client.transport.getPeerPreferences().
	    isSupported(SSH2Preferences.COMP_S2C, "zlib"))) {
	    throw new Exception("Peer doesn't support 'zlib'");
	}
    }

    private void checkSupportedByPeer(String type, JComboBox c)
        throws Exception {
	if (c.getSelectedIndex() == 0) {
	    return;
	}
	String item = (String)c.getSelectedItem();
	if (!client.transport.getPeerPreferences().isSupported(type, item)) {
	    throw new Exception("Peer doesn't support: " + item);
	}
    }

    private JCheckBox[] authMethod;
    private void customAuthDialog() {
        final JDialog dialog =
            SwingConvenience.newBorderJDialog(parent, LBL_CUSTOM_LIST, true);

        JPanel p = new JPanel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.anchor    = GridBagConstraints.WEST;
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.insets    = new Insets(2,2,2,2);

        authMethod = new JCheckBox[AUTH_METHODS.length];
        // The -1 is to exclude the custom auth-method
        for (int i=0; i< authMethod.length-1; i++) {
            authMethod[i] = new JCheckBox(AUTH_METHODS[i]);
            authMethod[i].setSelected(
                -1 != customAuth.indexOf(AUTH_METHODS[i]));
            p.add(authMethod[i], gridc);
        }
        
        dialog.getContentPane().add(p, BorderLayout.CENTER);

        JButton ok = new JButton(LBL_BTN_OK);
        JButton cancel = new JButton(LBL_BTN_CANCEL);
        JPanel bp = SwingConvenience.newButtonPanel(new JButton[] {ok,cancel});
        dialog.getContentPane().add(bp, BorderLayout.SOUTH);

        dialog.getRootPane().setDefaultButton(ok);

        dialog.setResizable(true);
        dialog.pack();

	AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());        

        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));
        ok.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                StringBuffer auths = new StringBuffer();
                for (int i=0; i< authMethod.length-1; i++) {
                    if (authMethod[i].isSelected()) {
                        if (auths.length() > 0) {
                            auths.append(',');
                        }
                        auths.append(AUTH_METHODS[i]);
                    }
                }
                customAuth = auths.toString();
                System.err.println(customAuth);
                dialog.dispose();
            }
        });      

	dialog.setVisible(true);
    }

    private JList currList;
    public final void currentTunnelsDialog(String title) {
        final JDialog dialog = SwingConvenience.newBorderJDialog(parent,
                                                                 title, false);

        dialog.getContentPane().add(
            new JLabel(LBL_CURR_TUNNELS), BorderLayout.NORTH);
        
        currList = new JList();
        currList.setVisibleRowCount(8);
        currList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);

        JScrollPane sp = new JScrollPane(currList);
        dialog.getContentPane().add(sp, BorderLayout.CENTER);
        
        ActionListener al = new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                String cmd = e.getActionCommand();
                if ("close".equals(cmd)) {
                    int i = currList.getSelectedIndex();
                    if (i == -1) {
                        term.ringBell();
                        return;
                    }
                    client.closeTunnelFromList(i);
                    Thread.yield();
                } else if ("refresh".equals(cmd)) {
                    // Nothing to do, the refresh happens below
                }
                refreshCurrList();
            }
        };

        final JButton close   = new JButton(LBL_BTN_CLOSE_TUNNEL);
        JButton refresh = new JButton(LBL_BTN_REFRESH);
        JButton cancel  = new JButton(LBL_BTN_DISMISS);

        ListSelectionListener ll = new ListSelectionListener() {
            public void valueChanged(ListSelectionEvent e) {
                int i = currList.getSelectedIndex();
                close.setEnabled(i >= 0);
            }
        };
        currList.addListSelectionListener(ll);
        
        close.setActionCommand("close");
        close.addActionListener(al);
        close.setEnabled(false);
        refresh.setActionCommand("refresh");
        refresh.addActionListener(al);
        
        dialog.getContentPane().add(SwingConvenience.newButtonPanel
                   (new JButton[] { close, refresh, cancel }),
                   BorderLayout.SOUTH);

        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));
        
        dialog.setResizable(true);
        dialog.pack();

        refreshCurrList();
    
	AWTConvenience.placeDialog(dialog);
	currList.requestFocus();
        dialog.addWindowListener(SwingConvenience.getWindowDisposer());        
	dialog.setVisible(true);
    }

    private void refreshCurrList() {
        currList.setListData(client.listTunnels());
        currList.setSelectedIndex(0);
    }

    protected void setupTunnelsDialog(String title) {
        SSHTunnelDialogSwing.show(title, client, ph, parent);
    }

    protected void aboutDialog(Frame parent, SSHInteractiveClient client, 
                               String title, String aboutText) {
	final JDialog dialog = SwingConvenience.newBorderJDialog(parent,
                                                                 title, true);
        
 	Component logo = client.getLogo();
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

    private String[] getAvailableAliases() {
        String[] al = ph.availableAliases();
        if (al != null) {
            Arrays.sort(al);
        } else {
            al = new String[0];
        }
        return al;
    }
}
