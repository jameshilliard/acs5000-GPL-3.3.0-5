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
import java.awt.Button;
import java.awt.CardLayout;
import java.awt.Checkbox;
import java.awt.CheckboxMenuItem;
import java.awt.Choice;
import java.awt.Component;
import java.awt.Dialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.Label;
import java.awt.List;
import java.awt.Menu;
import java.awt.MenuBar;
import java.awt.MenuItem;
import java.awt.MenuShortcut;
import java.awt.Panel;
import java.awt.PopupMenu;
import java.awt.TextArea;
import java.awt.TextField;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.File;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.BorderDialog;
import com.mindbright.gui.GUI;
import com.mindbright.gui.TabPanel;
import com.mindbright.ssh2.SSH2Preferences;
import com.mindbright.terminal.TerminalMenuHandlerFull;

public class SSHMenuHandlerFullAWT extends SSHMenuHandlerFull 
{
//     public SSHMenuHandlerFullAWT() {
//         super();
//     }
    
    public void setupMenuBar(boolean usePopupMenu
	    	// [CYCLADES-START]	
        	, boolean appletMode
        	//  [CYCLADES-END]
		) { 
        if (usePopupMenu) {
            PopupMenu pm = new PopupMenu(MENU_HEADER_POPUP);
            preparePopupMenu(pm);
        } else {
            MenuBar mb = new MenuBar();
            prepareMenuBar(mb);
            parent.setMenuBar(mb);
        }        
    }

    private void prepareMenuBar(MenuBar mb) {
	mb.add(getMenu(MENU_FILE));
	mb.add((Menu)((TerminalMenuHandlerFull)term.getMenus()).getMenu
               (TerminalMenuHandlerFull.MENU_EDIT));
	mb.add(getMenu(MENU_SETTINGS));
	/*Menu pm = getPluginMenu();
	if (pm != null) {
	    mb.add(pm);
	}
	mb.add(getMenu(MENU_TUNNELS));*/
	mb.setHelpMenu(getMenu(MENU_HELP));
	term.updateMenus();
    }

    private void preparePopupMenu(PopupMenu popupmenu) {
	havePopupMenu = true;
	popupmenu.add(getMenu(MENU_FILE));
	popupmenu.add((Menu)((TerminalMenuHandlerFull)term.getMenus()).getMenu
                      (TerminalMenuHandlerFull.MENU_EDIT));
	popupmenu.add(getMenu(MENU_SETTINGS));
	/*Menu pm = getPluginMenu();
	if (pm != null) 
	    popupmenu.add(pm);
	popupmenu.add(getMenu(MENU_TUNNELS));*/
	popupmenu.addSeparator();
	popupmenu.add(getMenu(MENU_HELP));
        term.getMenus().setPopupMenu(popupmenu);
	update();
    }

    private Object[][] menuItems;
    private MenuItem[] modMenuItems;

    private Menu getMenu(int idx) {
	Menu m = new Menu(menuTexts[idx][0]);
	int len = menuTexts[idx].length;
	MenuItem mi;
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
		mi = new CheckboxMenuItem(t);
		((CheckboxMenuItem)mi).addItemListener(this);
	    } else {
		mi = new MenuItem(t);
		mi.addActionListener(this);
	    }

            int sc = getMenuShortCut(idx, i);
	    if (sc != NO_SHORTCUT) {
		mi.setShortcut(new MenuShortcut(sc, true));
	    }

	    menuItems[idx][i] = mi;
	    m.add(mi);
	}
	return m;
    }

    private Menu getPluginMenu() {
        modMenuItems = null;
	Menu m = null;
	if (modCnt > 0) {
            modMenuItems = new MenuItem[modCnt];
	    m = new Menu(MENU_HEADER_PLUGINS);
	    int i = 0;
	    for (; i < modCnt; i++) {
		String label = getModuleLabel(i);
		if (label != null) {
		    MenuItem mi = new MenuItem(label);
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
        ((MenuItem)menuItems[i][j]).setEnabled(v);
    }

    protected void setState(int i, int j, boolean v) {
        ((CheckboxMenuItem)menuItems[i][j]).setState(v);
    }

    protected boolean getState(int i, int j) {
        return ((CheckboxMenuItem)menuItems[i][j]).getState();
    }

    protected void updatePluginMenu() {
	for (int i = 0; i < modCnt; i++)
	    if (getModuleLabel(i) != null)
		modMenuItems[i].setEnabled(modules[i].isAvailable(client));
    }

    protected String getMenuLabel(Object o) {
        return ((MenuItem)o).getLabel();
    }

    public void connectDialog(String title) {
        final Dialog dialog = new Dialog(parent, title, true);
        final List hostList = new List(8);

        dialog.setLayout(new BorderLayout());

        Panel p = new Panel(new GridLayout(0, 1));
        p.add(new Label(LBL_AVAIL_HOSTS));
        p.add(new Label(" (dir: " + ph.getSSHHomeDir() + ")"));
        dialog.add(p, BorderLayout.NORTH);
        dialog.add(hostList, BorderLayout.CENTER);

        final Button newBtn = new Button(LBL_BTN_NEW);

        ActionListener al = new ActionListener() {
            private void actionConnect() {
                String host = hostList.getSelectedItem();
                try {
                    String pwd = "";
                    do {
                        try {
                            ph.setPropertyPassword(pwd);
                            ph.loadAliasFile(host, false);
                            client.quiet = true;
                            client.sshStdIO.breakPromptLine();
                            dialog.dispose();
                            break;
                        } catch(SSHClient.AuthFailException ee) {
                        }
                    } while ((pwd = passwordDialog(
                                  "Please give file password for " +
                                  host, "MindTerm - File Password")) != null);
                } catch (Throwable t) {
                    alertDialog("Error loading settings: " + t.getMessage());
                }
            }
            
            private void actionNew() {
                try {
                    ph.checkSave();
                } catch (Throwable t) {
                    alertDialog("Error saving settings: " + t.getMessage());
                }
                ph.clearServerSetting();
                newBtn.setEnabled(false);
                dialog.dispose();
            }
            
            public void actionPerformed(ActionEvent e) {
                if (e.getSource() == newBtn) {
                    actionNew();
                } else {
                    actionConnect();
                }
            }
        };
        
        Button b;
        p = new Panel(new FlowLayout(FlowLayout.CENTER));
        
        p.add(b = new Button(LBL_BTN_CONNECT));
        b.addActionListener(al);
               
        p.add(newBtn);
        newBtn.addActionListener(al);
               
        p.add(b = new Button(LBL_BTN_CANCEL));
        b.addActionListener(new AWTConvenience.CloseAction(dialog));
        
        dialog.add(p, BorderLayout.SOUTH);
        
        dialog.addWindowListener(new AWTConvenience.CloseAdapter(b));
        
        AWTConvenience.setBackgroundOfChildren(dialog);
        
        dialog.setResizable(true);
        dialog.pack();
	hostList.removeAll();

	String[] l = ph.availableAliases();

	if (l != null) {
	    for (int i = 0; i < l.length; i++)
		hostList.add(l[i]);
	}
	hostList.select(0);
	dialog.pack();

	AWTConvenience.placeDialog(dialog);
	hostList.requestFocus();
	dialog.setVisible(true);

	if (!newBtn.isEnabled())
	    sshNewServerDialog();
    }

    public void keyGenerationDialogCreate(String title) {
        SSHKeyGenerationDialogAWT.show(title, parent, client);
    }
    
    public void keyGenerationDialogEdit(String title) {
        SSHKeyGenerationDialogAWT.editKeyDialog(title, parent, client);
    }


    protected void sshNewServerDialog(String title) {
        sshConfigDialog(title, LBL_TAB_GENERAL);
    }

    public void sshPreferencesDialog(String title) {
        sshConfigDialog(title, LBL_TAB_SECURITY);
    }

    // General stuff
    private Choice    comboAuthTyp;
    private Checkbox  cbSaveAlias;
    private TextField textSrv, textPort, textUser, textAlias, textPrivateKey, textPrivateHostKey;
    private TextField textPwd;
    private Button    customBtn, browseBtn;
    private CardLayout authLabelCL, authCL;
    private Panel     authLabelCP, authCP;
    private String    customAuth;

    // Proxy stuff
    private Choice    comboPrxType;
    private Checkbox  cbNeedAuth;
    private TextField textPrxHost, textPrxPort, textPrxUser;
    private TextField textPrxPasswd;

    // Security stuff
    private Checkbox  cbProto1, cbProto2;
    private Checkbox  cbKeyTypeDSS, cbKeyTypeRSA;
    private Choice    comboCipherC2S, comboCipherS2C, comboMacC2S, comboMacS2C;
    private Choice    comboCompC2S, comboCompS2C;
    private Checkbox  cbX11, cbIdHost, cbKeyNoise, cbAlive, cbForcPty;
    private TextField textDisp, textMtu, textAlive;

    protected void sshConfigDialog(String title, String first) {
        final Dialog dialog = new Dialog(parent, title, true);
        
        GridBagLayout      grid  = new GridBagLayout();
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.fill   = GridBagConstraints.HORIZONTAL;
        gridc.anchor = GridBagConstraints.WEST;
        gridc.insets = new Insets(2,2,2,2);
        gridc.gridheight = 1;
        gridc.weightx = 0.0;
        gridc.weighty = 0.0;

        TabPanel tp = new TabPanel();

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
        Panel mp = new Panel(grid);
        
        Label lbl = new Label(LBL_SERVER, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        textSrv = new TextField("", 20);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        mp.add(textSrv, gridc);
        gridc.weightx = 0.0;
        
        lbl = new Label(LBL_USERNAME, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        textUser = new TextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textUser, gridc);

        lbl = new Label(LBL_AUTH, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        comboAuthTyp = AWTConvenience.newChoice(AUTH_METHODS);
        comboAuthTyp.addItemListener(ilg);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.fill = GridBagConstraints.NONE;
        mp.add(comboAuthTyp, gridc);
        gridc.fill = GridBagConstraints.HORIZONTAL;
        
        gridc.gridwidth = 1;
        mp.add(getAuthLabel(), gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(getAuth(), gridc);

        cbSaveAlias = new Checkbox(LBL_SAVE_AS_ALIAS);
        cbSaveAlias.addItemListener(ilg);
        gridc.insets = new Insets(10, 2, 2, 2);
        gridc.gridwidth = 1;
        mp.add(cbSaveAlias, gridc);
        textAlias = new TextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textAlias, gridc);

        gridc.weighty = 1.0;
        mp.add(new Label(""), gridc);
        gridc.weighty = 0.0;

        tp.add(LBL_TAB_GENERAL, mp);

        /****************************************************************
         * Proxy tab
         */
        mp = new Panel(grid);

        lbl = new Label(LBL_PROXY_TYPE, Label.RIGHT);
        gridc.insets = new Insets(2, 2, 2, 2);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        comboPrxType = AWTConvenience.newChoice(SSH.getProxyTypes());
        comboPrxType.addItemListener(ilp);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.fill = GridBagConstraints.NONE;
        mp.add(comboPrxType, gridc);
        gridc.fill = GridBagConstraints.HORIZONTAL;

        lbl = new Label(LBL_SERVER, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        textPrxHost = new TextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        mp.add(textPrxHost, gridc);
        gridc.weightx = 0.0;

        lbl = new Label(LBL_PORT, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        textPrxPort = new TextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(textPrxPort, gridc);

        Panel ap = new Panel(new GridBagLayout());

        cbNeedAuth = new Checkbox(LBL_AUTH);
        cbNeedAuth.addItemListener(ilp);
        gridc.gridwidth = 1;
        ap.add(cbNeedAuth, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        ap.add(new Panel(), gridc);

        lbl = new Label(LBL_USERNAME, Label.RIGHT);
        gridc.gridwidth = 1;
        ap.add(lbl, gridc);
        textPrxUser = new TextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        ap.add(textPrxUser, gridc);
        gridc.weightx = 0.0;

        lbl = new Label(LBL_PASSWORD, Label.RIGHT);
        gridc.gridwidth = 1;
        ap.add(lbl, gridc);
        textPrxPasswd = new TextField("", 12);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        ap.add(textPrxPasswd, gridc);

        mp.add(ap, gridc);

        gridc.weighty = 1.0;
        mp.add(new Label(""), gridc);
        gridc.weighty = 0.0;

        tp.add(LBL_TAB_PROXY, mp);

        /****************************************************************
         * Security tab
         */
        mp = new Panel(grid);

        lbl = new Label(LBL_PROTOCOL, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        cbProto2 = new Checkbox(LBL_PROTO_SSH2);
        cbProto2.addItemListener(ils);
        mp.add(cbProto2, gridc);
        cbProto1 = new Checkbox(LBL_PROTO_SSH1);
        mp.add(cbProto1, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(new Panel(), gridc);

        lbl = new Label(LBL_HKEY_TYPE, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        cbKeyTypeDSS = new Checkbox(LBL_HKEY_DSS);
        mp.add(cbKeyTypeDSS, gridc);
        cbKeyTypeRSA = new Checkbox(LBL_HKEY_RSA);
        mp.add(cbKeyTypeRSA, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(new Panel(), gridc);

        gridc.gridwidth = 1;
        mp.add(new Panel(), gridc);
        gridc.gridwidth = 2;
        mp.add(new Label(LBL_C2S), gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(new Label(LBL_S2C), gridc);

        comboCipherC2S = AWTConvenience.newChoice(ciphers);
        comboCipherS2C = AWTConvenience.newChoice(ciphers);
        comboMacC2S    = AWTConvenience.newChoice(macs);
        comboMacS2C    = AWTConvenience.newChoice(macs);
        comboCompC2S   = AWTConvenience.newChoice(compc2s);
        comboCompS2C   = AWTConvenience.newChoice(comps2c);
        
        comboCipherC2S.insert(LBL_ANY_STANDARD, 0);
        comboCipherS2C.insert(LBL_ANY_STANDARD, 0);
        comboMacC2S.insert(LBL_ANY_STANDARD, 0);
        comboMacS2C.insert(LBL_ANY_STANDARD, 0);

        comboCipherC2S.addItemListener(ils);
        comboMacC2S.addItemListener(ils);
        comboCompC2S.addItemListener(ils);

        lbl = new Label(LBL_CIPHER, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        gridc.gridwidth = 2;
        mp.add(comboCipherC2S, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(comboCipherS2C, gridc);

        lbl = new Label(LBL_MAC, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        gridc.gridwidth = 2;
        mp.add(comboMacC2S, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(comboMacS2C, gridc);

        lbl = new Label(LBL_COMP, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        gridc.gridwidth = 2;
        mp.add(comboCompC2S, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(comboCompS2C, gridc);

        gridc.weighty = 1.0;
        mp.add(new Label(""), gridc);
        gridc.weighty = 0.0;

        tp.add(LBL_TAB_SECURITY, mp);

        /****************************************************************
         * features tab
         */
        mp = new Panel(grid);

        lbl = new Label(LBL_X11_FORWARD, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        Panel p = new Panel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        cbX11 = new Checkbox();
        cbX11.addItemListener(ilf);
        textDisp = new TextField("", 12);
        p.add(cbX11);
        p.add(new Label(LBL_LOCAL_DISP));
        p.add(textDisp);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.weightx = 1.0;
        mp.add(p, gridc);
        gridc.weightx = 0.0;

        lbl = new Label(LBL_SEND_KEEP, Label.RIGHT);
        gridc.gridwidth = 1;
        mp.add(lbl, gridc);
        p = new Panel(new FlowLayout(FlowLayout.LEFT, 0, 0));
        cbAlive = new Checkbox();
        cbAlive.addItemListener(ilf);
        textAlive = new TextField("", 12);
        p.add(cbAlive);
        p.add(new Label(LBL_INTERVAL));
        p.add(textAlive);
        p.add(new Label(LBL_SECONDS));
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(p, gridc);

        gridc.gridwidth = 1;
        mp.add(new Panel(), gridc);
        cbIdHost = new Checkbox(LBL_STRICT);
        mp.add(cbIdHost, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(new Panel(), gridc);

        gridc.gridwidth = 1;
        mp.add(new Panel(), gridc);
        cbKeyNoise = new Checkbox(LBL_KEY_NOISE);
        mp.add(cbKeyNoise, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(new Panel(), gridc);

        gridc.gridwidth = 1;
        mp.add(new Panel(), gridc);
        cbForcPty = new Checkbox(LBL_ALLOC_PTY);
        mp.add(cbForcPty, gridc);
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        mp.add(new Panel(), gridc);

        gridc.weighty = 1.0;
        mp.add(new Label(""), gridc);
        gridc.weighty = 0.0;

        tp.add(LBL_TAB_FEATURES, mp);

        /****************************************************************
         * Add stuff to dialog
         */
        dialog.add(tp, BorderLayout.CENTER);
        tp.show(first);

        Button ok = new Button();
        if (client.isConnected()) {
            ok.setLabel(LBL_BTN_OK);
        } else {
            ok.setLabel(LBL_BTN_CONNECT);
        }
        Button cancel = new Button(LBL_BTN_CANCEL);
        Panel bp = AWTConvenience.newButtonPanel(new Button[] {ok,cancel});
        dialog.add(bp, BorderLayout.SOUTH);

        dialog.setResizable(true);
        dialog.pack();

	AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancel));

        populate();
	updateChoicesGeneral();
	updateChoicesProxy(null);
	updateChoicesSecurity(null);
	updateChoicesFeatures();

	if (textSrv.isEnabled()) {
	    textSrv.requestFocus();
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

    private void okPressed(Dialog dialog) throws Exception {
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

        host = textSrv.getText();
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

        if (cbSaveAlias.getState()) {
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
        ph.clearAllForwards();

        String authType = (String)comboAuthTyp.getSelectedItem();
         if (authType.equals(LBL_CUSTOM_LIST)) {
             ph.setProperty("authtyp", customAuth);
         } else {
             ph.setProperty("authtyp", authType);
         }

        String pwd = textPwd.getText();
        if (pwd.length() == 0) {
            pwd = null;
        }
        ph.setProperty("password", pwd);
        ph.setProperty("private-key", textPrivateKey.getText());
        ph.setProperty("private-host-key", textPrivateHostKey.getText());

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
        if (cbNeedAuth.getState()) {
            ph.setProperty("proxyuser", textPrxUser.getText());
            String pwd = textPwd.getText();
            if (pwd.length() == 0) {
                pwd = null;
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

        if (cbProto1.getState() && cbProto2.getState()) {
            ph.setProperty("protocol", "auto");
        } else if (cbProto1.getState()) {
            ph.setProperty("protocol", "ssh1");
        } else if (cbProto2.getState()) {
            ph.setProperty("protocol", "ssh2");
        } else {
            throw new Exception(ERR_NO_PROTOCOL);
        }
        if (cbKeyTypeDSS.getState() && cbKeyTypeRSA.getState()) {
            ph.setProperty("server-host-key-algorithms", "ssh-rsa,ssh-dss");
        } else if (cbKeyTypeDSS.getState()) {
            ph.setProperty("server-host-key-algorithms", "ssh-dss");
        } else if (cbKeyTypeRSA.getState()) {
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
        if (cbX11.getState()) {
            ph.setProperty("display", textDisp.getText());
        } else {
            ph.setProperty("display", "");
        }
        ph.setProperty("x11fwd", cbX11.getState());
        ph.setProperty("stricthostid", cbIdHost.getState());
        ph.setProperty("key-timing-noise", cbKeyNoise.getState());
        ph.setProperty("forcpty", cbForcPty.getState());
        if (cbAlive.getState()) {
            ph.setProperty("alive", textAlive.getText());
        } else {
            ph.setProperty("alive", "0");
        }
    }

    private Panel getAuthLabel() {
	authLabelCP = new Panel();
	authLabelCP.setLayout(authLabelCL = new CardLayout());

        int r = Label.RIGHT;
	authLabelCP.add(new Label(LBL_PASSWORD, r), "password");
	authLabelCP.add(new Panel(), LBL_CUSTOM_LIST);
	authLabelCP.add(new Label(LBL_IDENTITY, r), "publickey");
	authLabelCP.add(new Panel(), "securid");
	authLabelCP.add(new Panel(), "tis");
	authLabelCP.add(new Panel(), "kbd-interact");
	authLabelCP.add(new Label(LBL_HOST_KEY, r), "hostbased");

        return authLabelCP;
    }

    private Component makeCenter(Component c) {
        Panel p = new Panel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();        
        gridc.fill = GridBagConstraints.HORIZONTAL;
        gridc.weightx = 1.0;
        p.add(c, gridc);
        return p;
    }

    private Panel getAuth() {
	authCP = new Panel();
	authCP.setLayout(authCL = new CardLayout());

        textPwd = new TextField("", 12);
	textPwd.setEchoChar('*');

        customBtn = new Button(LBL_MODIFY_LIST);
        customBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                customAuthDialog();
            }
        });      

        textPrivateKey = new TextField("", 12);
        Button browseBtn = new Button(LBL_BTN_BROWSE);
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
        Panel p = new Panel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();        
        gridc.fill = GridBagConstraints.HORIZONTAL;
        gridc.weightx = 1.0;
        p.add(textPrivateKey, gridc);
        gridc.weightx = 0.0;
        p.add(browseBtn, gridc);

	authCP.add(makeCenter(textPwd), "password");
	authCP.add(makeCenter(customBtn), LBL_CUSTOM_LIST);
	authCP.add(p, "publickey");
	authCP.add(new Label(), "tis");
	authCP.add(new Label(), "kbd-interact");
 	authCP.add(new Panel(), "securid");
 	authCP.add(new Panel(), "tis");
 	authCP.add(new Panel(), "kbd-interact");

        textPrivateHostKey = new TextField("", 12);
        browseBtn = new Button(LBL_BTN_BROWSE);
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
        p = new Panel(new GridBagLayout());
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
        textSrv.setEnabled(!client.isOpened());
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
	if (cbSaveAlias.getState()) {
            textAlias.setEnabled(true);
	    String t = textAlias.getText();
	    if (!textAlias.isEnabled() &&
                (t == null || t.trim().length() == 0)) {
		textAlias.setText(textSrv.getText());
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
            cbNeedAuth.setState(false);

        textPrxUser.setEnabled(cbNeedAuth.getState());
        textPrxPasswd.setEnabled(cbNeedAuth.getState());
    }

    private void updateChoicesSecurity(ItemEvent e) {
	boolean isOpen = client.isOpened();

        boolean isSSH2;
        if (isOpen) {
            isSSH2 = client.isSSH2;
        } else {
            isSSH2 = cbProto2.getState();
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
            comboCipherS2C.select((String)e.getItem());
        } else if (e != null && e.getSource() == comboMacS2C) {
            comboMacS2C.select((String)e.getItem());
        } else if (e != null && e.getSource() == comboCompS2C) {
            if ("none".equals(e.getItem())) {
                comboCompS2C.select("none");
            } else {
                comboCompS2C.select("medium");
            }
        }
    }

    private void updateChoicesFeatures() {
	boolean isOpen = client.isOpened();
	cbX11.setEnabled(!isOpen);
	cbIdHost.setEnabled(!isOpen);
	cbForcPty.setEnabled(!isOpen);

        textAlive.setEnabled(cbAlive.getState());
        textDisp.setEnabled(cbX11.getState());
    }

    // Populate the new server dialog
    private void populate() {
        // General
        String port = ph.getProperty("port");
        if (port.length() > 0 && !port.equals("22")) {
            textSrv.setText(ph.getProperty("server") + ":" + port);
        } else {
            textSrv.setText(ph.getProperty("server"));
        }
        textUser.setText(ph.getProperty("username"));
        textPrivateKey.setText(ph.getProperty("private-key"));
        textPrivateHostKey.setText(ph.getProperty("private-host-key"));
        customAuth = ph.getProperty("auth-method");
        if (-1 == customAuth.indexOf(',')) {
            comboAuthTyp.select(customAuth);
        } else {
            comboAuthTyp.select(LBL_CUSTOM_LIST);
        }

        // Proxy
        textPrxHost.setText(ph.getProperty("proxyhost"));
        textPrxPort.setText(ph.getProperty("proxyport"));
        textPrxUser.setText(ph.getProperty("proxyuser"));
    
        // Security
        cbKeyTypeDSS.setState(true);
        cbKeyTypeRSA.setState(true);
	comboCipherC2S.select(0);
	comboCipherS2C.select(0);
	comboMacC2S.select(0);
	comboMacS2C.select(0);

        cbProto1.setState(!ph.getProperty("protocol").equals("ssh2"));
        cbProto2.setState(!ph.getProperty("protocol").equals("ssh1"));
        cbKeyTypeDSS.setState(
            -1!=ph.getProperty("server-host-key-algorithms").indexOf("ssh-dss"));
        cbKeyTypeRSA.setState(
            -1!=ph.getProperty("server-host-key-algorithms").indexOf("ssh-rsa"));
	comboCipherC2S.select(ph.getProperty("enc-algorithms-cli2srv"));
	comboCipherS2C.select(ph.getProperty("enc-algorithms-srv2cli"));
	comboMacC2S.select(ph.getProperty("mac-algorithms-cli2srv"));
	comboMacS2C.select(ph.getProperty("mac-algorithms-srv2cli"));

	int compLevel = ph.getCompressionLevel();
	comboCompC2S.select(lvl2comp[compLevel]);
	String s2cComp = ph.getProperty("comp-algorithms-srv2cli");
	if ("none".equals(s2cComp)) {
	    comboCompS2C.select("none");
	} else {
	    comboCompS2C.select("medium");
	}

        // Features
	textDisp.setText(ph.getProperty("display"));
	textAlive.setText(ph.getProperty("alive"));

	cbX11.setState(ph.getPropertyB("x11fwd"));
	cbAlive.setState(!ph.getProperty("alive").equals("0"));

	cbIdHost.setState(ph.getPropertyB("stricthostid"));
	cbKeyNoise.setState(ph.getPropertyB("key-timing-noise"));
	cbForcPty.setState(ph.getPropertyB("forcpty"));
    }

    private void updateCheckedText(Checkbox cb, TextField text,
                                   String propName) {
	if (!text.isEnabled()) {
	    if (cb.getState()) {
		text.setText(ph.getProperty(propName));
	    } else {
		text.setText(ph.getDefaultProperty(propName));
	    }
	}
	text.setEnabled(cb.isEnabled() && cb.getState());
	if (!text.isEnabled()) {
	    text.setText(ph.getDefaultProperty(propName));
	}
    }

    private void checkSupportedByPeer() throws Exception {
	checkSupportedByPeer(SSH2Preferences.CIPHERS_C2S, comboCipherC2S);
	checkSupportedByPeer(SSH2Preferences.CIPHERS_S2C, comboCipherS2C);
	checkSupportedByPeer(SSH2Preferences.MACS_C2S, comboMacC2S);
	checkSupportedByPeer(SSH2Preferences.MACS_S2C, comboMacS2C);
	if ((!comboCompC2S.getSelectedItem().equals("none") &&
	    !client.transport.getPeerPreferences().
	    isSupported(SSH2Preferences.COMP_C2S, "zlib"))
	   ||
	   (!comboCompS2C.getSelectedItem().equals("none") &&
	    !client.transport.getPeerPreferences().
	    isSupported(SSH2Preferences.COMP_S2C, "zlib"))) {
	    throw new Exception("Peer doesn't support 'zlib'");
	}
    }

    private void checkSupportedByPeer(String type, Choice c) throws Exception {
	if (c.getSelectedIndex() == 0) {
	    return;
	}
	String item = c.getSelectedItem();
	if (!client.transport.getPeerPreferences().isSupported(type, item)) {
	    throw new Exception("Peer doesn't support: " + item);
	}
    }

    private Checkbox[] authMethod;
    private void customAuthDialog() {
        final Dialog dialog = new BorderDialog(parent, LBL_CUSTOM_LIST, true);

        Panel p = new Panel(new GridBagLayout());
        GridBagConstraints gridc = new GridBagConstraints();
        gridc.anchor    = GridBagConstraints.WEST;
        gridc.gridwidth = GridBagConstraints.REMAINDER;
        gridc.insets    = new Insets(2,2,2,2);

        authMethod = new Checkbox[AUTH_METHODS.length];
        // The -1 is to exclude the custom auth-method
        for (int i=0; i< authMethod.length-1; i++) {
            authMethod[i] = new Checkbox(AUTH_METHODS[i]);
            authMethod[i].setState(-1 != customAuth.indexOf(AUTH_METHODS[i]));
            p.add(authMethod[i], gridc);
        }
        
        dialog.add(p, BorderLayout.CENTER);

        Button ok = new Button(LBL_BTN_OK);
        Button cancel = new Button(LBL_BTN_CANCEL);
        Panel bp = AWTConvenience.newButtonPanel(new Button[] {ok,cancel});
        dialog.add(bp, BorderLayout.SOUTH);

        dialog.setResizable(true);
        dialog.pack();

	AWTConvenience.placeDialog(dialog);
        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancel));

        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));
        ok.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                StringBuffer auths = new StringBuffer();
                for (int i=0; i< authMethod.length-1; i++) {
                    if (authMethod[i].getState()) {
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

    private List currList;
    public final void currentTunnelsDialog(String title) {
        final Dialog dialog = new BorderDialog(parent, title, false);

        dialog.add(new Label(LBL_CURR_TUNNELS), BorderLayout.NORTH);
        currList = new List(8);
        dialog.add(currList, BorderLayout.CENTER);

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
                dialog.invalidate();
            }
        };
        
        final Button close   = new Button(LBL_BTN_CLOSE_TUNNEL);
        Button refresh = new Button(LBL_BTN_REFRESH);
        Button cancel  = new Button(LBL_BTN_DISMISS);

        ItemListener ll = new ItemListener() {
            public void itemStateChanged(ItemEvent e) {
                int i = currList.getSelectedIndex();
                close.setEnabled(i >= 0);
            }
        };
        currList.addItemListener(ll);

        close.setActionCommand("close");
        close.addActionListener(al);
        close.setEnabled(false);
        refresh.setActionCommand("refresh");
        refresh.addActionListener(al);
        cancel.addActionListener(new AWTConvenience.CloseAction(dialog));

        Panel bp = AWTConvenience.newButtonPanel(
            new Button[] { close, refresh, cancel });
        dialog.add(bp, BorderLayout.SOUTH);

        dialog.addWindowListener(new AWTConvenience.CloseAdapter(cancel));
        
        AWTConvenience.setBackgroundOfChildren(dialog);
        
        dialog.setResizable(true);
        dialog.pack();

        refreshCurrList();
    
	AWTConvenience.placeDialog(dialog);
	currList.requestFocus();
	dialog.setVisible(true);
    }

    private void refreshCurrList() {
	currList.removeAll();
	String[] l = client.listTunnels();
	for(int i = 0; i < l.length; i++) {
	    currList.add(l[i]);
	}
    }


    protected void setupTunnelsDialog(String title) {
        SSHTunnelDialogAWT.show(title, client, ph, parent);
    }
    
    protected void aboutDialog(Frame parent, SSHInteractiveClient client, 
                               String title, String aboutText) {
	Dialog dialog = new BorderDialog(parent, title, true);

	Component logo = client.getLogo();
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
