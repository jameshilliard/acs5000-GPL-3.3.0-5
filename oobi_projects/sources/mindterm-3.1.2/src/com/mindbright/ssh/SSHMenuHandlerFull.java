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

package com.mindbright.ssh;

import java.awt.Frame;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.KeyEvent;
import java.io.File;
import java.util.StringTokenizer;

import com.mindbright.application.MindTerm;
import com.mindbright.application.MindTermModule;
import com.mindbright.gui.GUI;
import com.mindbright.ssh2.SSH2ListUtil;
import com.mindbright.terminal.TerminalMenuHandlerFull;
import com.mindbright.terminal.TerminalMenuListener;
import com.mindbright.terminal.TerminalWin;

public abstract class SSHMenuHandlerFull extends SSHMenuHandler implements 
        ActionListener,
        ItemListener,
        TerminalMenuListener {

    private final static String aboutText =
    	// [CYCLADES-START]
        SSH.VER_MINDTERM + "\n" +
        //Version.licenseMessage + "\n" +
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


    protected final static int ACT_MOD_BASE = 0;

    protected class Actions implements ActionListener {
	private int       action;

	public Actions(int action) {
	    this.action = action;
	}

	public void actionPerformed(ActionEvent e) {
            if(action >= ACT_MOD_BASE)
                modules[action - ACT_MOD_BASE].activate(client);
	}
    }

    protected SSHInteractiveClient client;
    protected Frame                parent;
    protected TerminalWin          term;
    protected MindTerm             mindterm;
    protected MindTermModule[]     modules;
    protected int                  modCnt;
    protected SSHPropertyHandler   ph;

    protected final static int MENU_FILE     = 0;
    protected final static int MENU_SETTINGS = 1;
    protected final static int MENU_TUNNELS  = 2;
    protected final static int MENU_HELP     = 2;//3;

    private final static int M_FILE_NEW          = 1;
    private final static int M_FILE_CLONE        = 2;
    private final static int M_FILE_CONN         = 3;
    private final static int M_FILE_DISC         = 4;
    private final static int M_FILE_LOAD         = 6;
    private final static int M_FILE_SAVE         = 7;
    private final static int M_FILE_SAVEAS       = 8;
    private final static int M_FILE_CREATID      = 10;
    private final static int M_FILE_EDITPKI      = 11;
    private final static int M_FILE_PRINT_SCREEN = 1;//13;
    private final static int M_FILE_PRINT_BUFFER = 2;//14;
    private final static int M_FILE_CAPTURE      = 3;//16;
    private final static int M_FILE_SEND         = 4;//17;
    private final static int M_FILE_CLOSE        = 6;//19;
    private final static int M_FILE_EXIT         = 20;

    // CYCLADES NEW ITENS
    /**
     * Menu ID for "send break" feature
     * Done by Eduardo Murai Soares
     */
    private final static int M_FILE_SENDBREAKSEQ = 16+6; // 15 is the separator!!
    private final static int M_FILE_LOCK = 17+6; 
    private final static int M_FILE_UNLOCK = 18+6; 
    private final static int M_FILE_POWER_ON = 19+6; 
    private final static int M_FILE_POWER_OFF = 20+6; 
    private final static int M_FILE_POWER_CYCLE = 21+6; 
    private final static int M_FILE_POWER_STATUS = 22+6;
    //  [CYCLADES-END]

    private final static int M_SET_SSH_NEW  = 1;
    private final static int M_SET_SSH_PREF = 2;

    private final static int M_SET_TERM     = 1;//3;

    private final static int M_SET_RESET    = 4;
    private final static int M_SET_AUTOSAVE = 6;
    private final static int M_SET_AUTOLOAD = 7;
    private final static int M_SET_SAVEPWD  = 8;

    private final static int M_TUNL_SETUP    = 1;
    private final static int M_TUNL_CURRENT  = 3;

    private final static int M_HELP_ABOUT   = 1;

    protected final static String MENU_HEADER_POPUP   = "MindTerm Menu";
    protected final static String MENU_HEADER_PLUGINS = "Plugins";

    protected final static String[][] menuTexts = {
	{ "File",
	  /*"New Terminal", "Clone Terminal", "Connect...", "Disconnect", null,
	  "Load Settings...", "Save Settings", "Save Settings As...", null,
	  "Create Keypair...", "Edit/Convert Keypair...", null,*/
          "Print screen...", "Print buffer...",/* null,*/
	  "_Capture To File...", "Send ASCII File...", null, "Close"/*, "Exit"
	  // [CYCLADES-START]
	  // Menu separator and "Send break" menu item 
	  // added by Eduardo Murai Soares 
	  , null, "Send break", "Lock outlet","Unlock outlet","Power on","Power off",
	  "Power cycle","Power status"*/
	  // [CYCLADES-END]
	},

	{ "Settings",
	  /*"New Server...", "Connection...",*/ 
          "Terminal..."/*, "Reset To Defaults", null,
	  "_Auto Save Settings", "_Auto Load Settings", "_Save Passwords"*/
	},

	/*{ "Tunnels",
	  "Setup...", null, "Current Connections..."
	},*/

	{ "Help",
	  "About MindTerm"
	},
    };

    protected final static int NO_SHORTCUT = -1;
    private final static int[][] menuShortCuts = {
	/*{ NO_SHORTCUT, KeyEvent.VK_N, KeyEvent.VK_O, KeyEvent.VK_C, NO_SHORTCUT,
	  NO_SHORTCUT, NO_SHORTCUT, KeyEvent.VK_S, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, 
	  NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT,
	  NO_SHORTCUT, KeyEvent.VK_E, KeyEvent.VK_X 
  	  // [CYCLADES-START]
	  // Shortcuts setup for Menu separator and "send break" item
	  // added by Eduardo Murai Soares
	  ,NO_SHORTCUT, 
	  NO_SHORTCUT ,NO_SHORTCUT,NO_SHORTCUT,
	  NO_SHORTCUT,NO_SHORTCUT,
	  NO_SHORTCUT,NO_SHORTCUT
	  // [CYCLADES-END]
},

	{ NO_SHORTCUT, KeyEvent.VK_H, NO_SHORTCUT, KeyEvent.VK_T }*/
    	{NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT, NO_SHORTCUT,
    	 NO_SHORTCUT, NO_SHORTCUT, KeyEvent.VK_E}
    };

    protected static int getMenuShortCut(int m, int s) {
        if (m < 0 || s < 0) return NO_SHORTCUT;
        if (menuShortCuts.length <= m) return NO_SHORTCUT;
        if (menuShortCuts[m].length <= s) return NO_SHORTCUT;
        return menuShortCuts[m][s];
    }

    protected static String LBL_SAVE_AS_ALIAS = "Save as alias";
    protected static String LBL_CUSTOM_LIST   = "custom list...";
    protected static String LBL_SECURID       = "Have SecurID token ready";
    protected static String LBL_CRYPTOCARD    = "Have CryptoCard token";
    protected static String LBL_TIS           = "TIS challenge/response will occur when connecting";
    protected static String LBL_KBDINT        = "Generic keyboard interactive authentication";
    protected static String LBL_TRANS_PREFS   = "Transport prefs.";
    protected static String LBL_ANY_STANDARD  = "any standard";
    protected static String LBL_X11_FORWARD   = "X11 forward";
    protected static String LBL_SEND_KEEP     = "Send keep-alive";
    protected static String LBL_BIND_ADDR     = "Bind address";
    protected static String LBL_STRICT        = "Strict host verify";
    protected static String LBL_ALLOC_PTY     = "Allocate PTY";
    protected static String LBL_KEY_NOISE     = "Key timing noise";
    
    protected static String LBL_AVAIL_HOSTS = "Available hosts/aliases";
    protected static String LBL_SERVER      = "Server";
    protected static String LBL_PORT        = "Port";
    protected static String LBL_USERNAME    = "Username";
    protected static String LBL_AUTH        = "Authentication";
    protected static String LBL_AUTH_REQ    = "Authentication required";
    protected static String LBL_PASSWORD    = "Password";
    protected static String LBL_MODIFY_LIST = "Modify list";
    protected static String LBL_IDENTITY    = "Identity";
    protected static String LBL_HOST_KEY    = "Host key";
    protected static String LBL_PROTOCOL    = "Protocol";
    protected static String LBL_PROTO_SSH1  = "SSHv1";
    protected static String LBL_PROTO_SSH2  = "SSHv2";
    protected static String LBL_HKEY_TYPE   = "Host key type";
    protected static String LBL_HKEY_DSS    = "ssh-dss";
    protected static String LBL_HKEY_RSA    = "ssh-rsa";
    protected static String LBL_C2S         = "Client to Server";
    protected static String LBL_S2C         = "Server to Client";
    protected static String LBL_CIPHER      = "Cipher";
    protected static String LBL_MAC         = "Mac";
    protected static String LBL_COMP        = "Compression";
    protected static String LBL_LOCAL_DISP  = "Local display";
    protected static String LBL_INTERVAL    = "Interval";
    protected static String LBL_SECONDS    = "(s)";
    protected static String LBL_CURR_TUNNELS = "Currently open tunnels";
    protected static String LBL_PROXY_TYPE  = "Proxy type";
    
    
    protected static String LBL_BTN_OK      = "OK";
    protected static String LBL_BTN_DISMISS = "Dismiss";
    protected static String LBL_BTN_CANCEL  = "Cancel";
    protected static String LBL_BTN_NEW     = "New...";
    protected static String LBL_BTN_CONNECT = "Connect";
    protected static String LBL_BTN_PROXY   = "Proxy...";
    protected static String LBL_BTN_PREFS   = "Preferences...";
    protected static String LBL_BTN_BROWSE  = "Browse...";
    protected static String LBL_BTN_CLOSE_TUNNEL = "Disconnect";
    protected static String LBL_BTN_REFRESH = "Refresh";
    protected static String LBL_BTN_ADD     = "Add";
    protected static String LBL_BTN_DELETE  = "Delete";

    protected static String LBL_TAB_GENERAL  = "General";
    protected static String LBL_TAB_PROXY    = "Proxy";
    protected static String LBL_TAB_SECURITY = "Security";
    protected static String LBL_TAB_FEATURES = "Features";

    protected static int IDX_TAB_GENERAL     = 0;
    protected static int IDX_TAB_PROXY       = 1;
    protected static int IDX_TAB_SECURITY    = 2;
    protected static int IDX_TAB_FEATURES    = 3;

    protected static String ERR_NO_PROTOCOL  = "No protocol version selected";
    protected static String ERR_NO_KEYTYPE   = "No host key type selected";

    public static SSHMenuHandler getInstance(Frame frame) {
        try {
            Class c = Class.forName(
                /*AWTConvenience.isSwingJFrame(frame)*/ true ? 
                "com.mindbright.ssh.SSHMenuHandlerFullSwing" :
                "com.mindbright.ssh.SSHMenuHandlerFullAWT");
            return (SSHMenuHandler)c.newInstance();
        } catch (Throwable t) {
        }
        return null;
    }

    public void init(MindTerm mindterm, SSHInteractiveClient client,
                     Frame parent, TerminalWin term) {
	this.mindterm     = mindterm;
	this.client       = client;
	this.parent       = parent;
	this.term         = term;
	this.modCnt       = 0;
        this.ph           = client.propsHandler;

        int i;
        
        for (i=0; ; i++) {
	    String className = client.propsHandler.getProperty("module" + i);
	    if (className == null) {
                modCnt = i+1;
                modules = new MindTermModule[modCnt];
                break;
            }
        }        
            
	for(i = 0; i < modCnt; i++) {
	    String className = client.propsHandler.getProperty("module" + i);
	    if (className == null)
		break;
	    try {
		modules[i] = (MindTermModule)Class.forName(className).newInstance();
		modules[i].init(client);
	    } catch (Exception e) {
		alertDialog("Module class '" + className + "' not found");
	    }
	}
    }

    private void initCiphers() {
        if (ciphers == null) {
            ciphers = SSH2ListUtil.arrayFromList(SSHPropertyHandler.ciphAlgsSort);
            macs    = SSH2ListUtil.arrayFromList(SSHPropertyHandler.macAlgs);
            hktypes = SSH2ListUtil.arrayFromList(SSHPropertyHandler.hostKeyAlgs);
        }
    }

    private int popButtonNum = 3;
    public void setPopupButton(int popButtonNum) {
	term.setPopupButton(popButtonNum);
	this.popButtonNum = popButtonNum;
    }

    public int getPopupButton() {
	return popButtonNum;
    }

    protected String getModuleLabel(int module) {
	return client.propsHandler.getProperty("module" + module + ".label");
    }

    protected void modulesConnect() {
        if (modules == null) return;
	for(int i = 0; i < modules.length; i++)
            if (modules[i] != null)
                modules[i].connected(client);
    }

    protected void modulesDisconnect() {
        if (modules == null) return;
	for(int i = 0; i < modules.length; i++)
            if (modules[i] != null)
                modules[i].disconnected(client);
    }

    private int[] mapAction(String action) {
	int[] id = new int[2];
	int i = 0, j = 0;

	for(i = 0; i < menuTexts.length; i++) {
	    for(j = 1; j < menuTexts[i].length; j++) {
		String mt = menuTexts[i][j];
		if(mt != null && 
                   (action.equals(mt) ||
                    ("_" + action).equals(mt))) {
		    id[0] = i;
		    id[1] = j;
		    i = menuTexts.length;
		    break;
		}
	    }
	}
	return id;
    }

    public void actionPerformed(ActionEvent e) {
	handleMenuAction(mapAction(getMenuLabel(e.getSource())));
    }

    public void itemStateChanged(ItemEvent e) {
	handleMenuAction(mapAction((String)e.getItem()));
    }

    // [CYCLADES-START]
    /**
     * Parse the property as byte sequence and transmit them to the console.
     */
    public void sendBytesFromAPMProperty(String property) {
    	byte seq[] = this.getBytesFromProperty(property);
    	
    	//alertDialog("sendBytesFromAPMProperty, prop="+property+", value="+
    	//		mindterm.getParamSSHProps().getProperty(property));
    	
		if (seq==null) 
			alertDialog("The property "+property+" is undefined. Please configure it at TOMCAT_HOME/apm/WEB-INF/jnlp/mindterm.properties");               			                			
		else term.sendBytes(seq);    	
    }
    
    /**
     * Parse bytes sequence from one single property.
     * sample: #5,c,O,l
     * 
     * The preceding character '#' states that the item should be handled as number.
     * 
     * @param propertyName
     * @return result
     */
    private byte[] getBytesFromProperty(String propertyName) {
        String aux = mindterm.getParamSSHProps().getProperty(propertyName);
        if (aux==null || aux.trim().equals("")) return null;
        
        StringTokenizer tokenlist = new StringTokenizer(aux,",",false);
        String character="";
        byte[] result = new byte[tokenlist.countTokens()];
        int i=0;
        
        while (tokenlist.hasMoreTokens()) {
        	character = tokenlist.nextToken().trim();
        	
        	if (character.startsWith("#"))
        		result[i] = Byte.parseByte(character.substring(1));
        	else 
        		result[i] = (byte) character.charAt(0);
        	
        	i++;
        	}        
        
        return result;
    }
    
    // [CYCLADES-STOP]
    public void menuSendBreak() {
		try {
    		String breakseq = mindterm.getParamSSHProps().getProperty(com.mindterm.util.CYProps.BREAK_SEQ);                		
    		if (breakseq==null) {
    			// send Telnet BREAK following RFC spec rules
    			byte IAC  = (byte) 255; // TELNET COMMAND BYTE
    			byte CMD_BREAK = (byte) 243; // COMMAND IDENTIFIER : NVT character BRK                			
    			term.sendBytes(new byte[] {IAC, CMD_BREAK});
    			}
    		else {
    			//System.out.println("LastTypedChar: "+((int) term.getDisplay().getLastKeyTyped()));
    			int lastKey = term.getLastKeyTyped();
    			if (lastKey!=13 && lastKey!=10) {                			
    				final int opt = GUI.showOptionPane(parent, "break sequence", "The break sequence needs to be sent at the beginning of a line.\nDo you want to send an [enter] before the break sequence?");
    				switch (opt) {
    					// user chose 'YES' : send CR
    					case 0 : term.sendBytes(new byte[] {13});
    						 break;
    						 
    					case 1:  break; // user chose 'NO': do not send CR               				                				
    					case 2:  return; // user chose 'CANCEL': abort operation
   					}                				
   				}
    			// send break sequence
   				sendBytesFromAPMProperty(com.mindterm.util.CYProps.BREAK_SEQ);
   			}
   		}
		catch (Exception e) {
   			e.printStackTrace();                			
   			alertDialog("Error: break failed!");
   		}                		
    }
    
    public void menuDisconnect() {
        if(mindterm.confirmClose()) {
            client.forcedDisconnect();
            client.quiet = client.initQuiet;
        }
    }

    private void handleMenuAction(int[] id) {
	switch(id[0]) {
            case MENU_FILE:
                switch(id[1]) {
                // [CYCLADES-START]
          	  	    // "Send break" action handler added by Eduardo Murai Soares
                	/*case M_FILE_SENDBREAKSEQ:
            			menuSendBreak();
                		break;         
                // [CYCLADES-END]
                    case M_FILE_NEW:
                        mindterm.newWindow();
                        break;
                    case M_FILE_CLONE:
                        mindterm.cloneWindow();
                        break;
                    case M_FILE_CONN:
                        initCiphers();
                        connectDialog("MindTerm - Connect");
                        break;
                    case M_FILE_DISC:
                    	menuDisconnect();
                        break;
                    case M_FILE_LOAD:
                        loadFileDialog();
                        break;
                    case M_FILE_SAVE:
                        try {
                            if(client.propsHandler.savePasswords &&
                               client.propsHandler.emptyPropertyPassword()) {
                                String pwd = setPasswordDialog(
                                    "Please set password for alias " +
                                    client.propsHandler.currentAlias,
                                    "MindTerm - Set File Password");
                                if(pwd == null)
                                    return;
                                client.propsHandler.setPropertyPassword(pwd);
                            }
                            client.propsHandler.saveCurrentFile();
                        } catch (Throwable t) {
                            alertDialog("Error saving settings: " + t.getMessage());
                        }
                        break;
                    case M_FILE_SAVEAS:
                        saveAsFileDialog();
                        break;
                    case M_FILE_CREATID:
                        keyGenerationDialogCreate();
                        break;
                    case M_FILE_EDITPKI:
                        keyGenerationDialogEdit();
                        break;*/
                    case M_FILE_CAPTURE:
                        if(getState(MENU_FILE, M_FILE_CAPTURE)) {
                            if(!((TerminalMenuHandlerFull)term.getMenus()).captureToFileDialog()) {
                                setState(MENU_FILE, M_FILE_CAPTURE, false);
                            }
                        } else {
                            ((TerminalMenuHandlerFull)term.getMenus()).endCapture();
                        }
                        break;
                    case M_FILE_SEND:
                        ((TerminalMenuHandlerFull)term.getMenus()).sendFileDialog();
                        break;
                    case M_FILE_PRINT_SCREEN:
                        ((TerminalMenuHandlerFull)term.getMenus()).printScreen();
                        break;
                    case M_FILE_PRINT_BUFFER:
                        ((TerminalMenuHandlerFull)term.getMenus()).printBuffer();
                        break;
                    case M_FILE_CLOSE:
                        mindterm.close();
                        //mindterm.exit();
                        break;
                    /*case M_FILE_EXIT:
                        mindterm.exit();
                        break;*/
                }
                break;

            case MENU_SETTINGS:
                switch(id[1]) {
                    /*case M_SET_SSH_NEW:
                        sshNewServerDialog();
                        break;
                    case M_SET_SSH_PREF:
                        sshPreferencesDialog();
                        break;*/
                    case M_SET_TERM:
                        ((TerminalMenuHandlerFull)term.getMenus()).termSettingsDialog();
                        break;
                    /*case M_SET_RESET:
                    	// [CYCLADES-START]
                    	if (client.isOpened()) 
                    		alertDialog("You must disconnect from server before performing this task.");
                    	else
                        // [CYCLADES-END]
                    	client.propsHandler.resetToDefaults();
                        break;
                    case M_SET_AUTOSAVE:
                        client.propsHandler.setAutoSaveProps
                            (getState(MENU_SETTINGS, M_SET_AUTOSAVE));
                        update();
                        break;
                    case M_SET_AUTOLOAD:
                        client.propsHandler.setAutoLoadProps
                            (getState(MENU_SETTINGS, M_SET_AUTOLOAD));
                        update();
                        break;
                    case M_SET_SAVEPWD:
                        client.propsHandler.setSavePasswords
                            (getState(MENU_SETTINGS, M_SET_SAVEPWD));
                        if(client.propsHandler.savePasswords && 
                           client.propsHandler.emptyPropertyPassword() &&
                           client.propsHandler.getAlias() != null) {
                            String pwd = setPasswordDialog(
                                "Please set password for alias " +
                                client.propsHandler.currentAlias,
                                "MindTerm - Set File Password");
                            if(pwd == null) {
                                client.propsHandler.setSavePasswords(false);
                                update();
                                return;
                            }
                            client.propsHandler.setPropertyPassword(pwd);
                        }
                        break;*/
                }
                break;

            /*case MENU_TUNNELS:
                switch(id[1]) {
                    case M_TUNL_SETUP:
                        setupTunnelsDialog();
                        break;
                    case M_TUNL_CURRENT:
                        currentTunnelsDialog();
                        break;
                }
                break;*/

            case MENU_HELP:
                switch(id[1]) {
                    case M_HELP_ABOUT:
                        aboutDialog(parent, client,
                                    "About " + SSH.VER_MINDTERM, aboutText);
                        break;
                }
                break;
	}
    }

    // menu handling
    protected abstract void setEnabled(int i, int j, boolean v);
    protected abstract void setState(int i, int j, boolean v);
    protected abstract boolean getState(int i, int j);
    protected abstract void updatePluginMenu();
    public abstract void setupMenuBar(boolean usePopupMenu, boolean appletMode);    
    protected abstract String getMenuLabel(Object o);

	
	 // [CYCLADES-START]
    /**
     * @return MindTerm reference. 
     * @author Eduardo Murai Soares/Cyclades Brasil
     */
	public MindTerm getMindterm() {
		return mindterm;
	}	
	// [CYCLADES-END]
	
    public void update() {
	boolean isOpen      = client.isOpened();
	boolean isConnected = client.isConnected();
	boolean hasHomeDir  = (client.propsHandler.getSSHHomeDir() != null);
        boolean allowNew =client.propsHandler.getPropertyB("allow-new-server");

	/*setEnabled(MENU_FILE, M_FILE_NEW, allowNew);
	setEnabled(MENU_FILE, M_FILE_CLONE, isOpen);
	setEnabled(MENU_FILE, M_FILE_SEND, isOpen);
	setEnabled(MENU_FILE, M_FILE_SAVEAS, isOpen && hasHomeDir);
	setEnabled(MENU_FILE, M_FILE_CONN, !isConnected && allowNew);
	setEnabled(MENU_FILE, M_FILE_DISC, isConnected);
	setEnabled(MENU_FILE, M_FILE_LOAD, !isConnected && allowNew);
	setEnabled(MENU_FILE, M_FILE_SAVE, 
                   client.propsHandler.wantSave() &&
                   client.propsHandler.currentAlias != null);
        
	setEnabled(MENU_SETTINGS, M_SET_SSH_NEW, !isOpen && allowNew);
	setEnabled(MENU_SETTINGS, M_SET_SSH_PREF, isOpen);
	setEnabled(MENU_SETTINGS, M_SET_RESET, !isOpen);

	setEnabled(MENU_SETTINGS, M_SET_AUTOSAVE, hasHomeDir);
	setEnabled(MENU_SETTINGS, M_SET_AUTOLOAD, hasHomeDir);
	setEnabled(MENU_SETTINGS, M_SET_SAVEPWD, hasHomeDir);

	setState(MENU_SETTINGS, M_SET_AUTOSAVE,
                 client.propsHandler.autoSaveProps);
	setState(MENU_SETTINGS, M_SET_AUTOLOAD,
                 client.propsHandler.autoLoadProps);
	setState(MENU_SETTINGS, M_SET_SAVEPWD,
                 client.propsHandler.savePasswords);

	setEnabled(MENU_TUNNELS, M_TUNL_CURRENT, isOpen);
	setEnabled(MENU_TUNNELS, M_TUNL_SETUP, isOpen);*/

	// [CYCLADES-START]
	// Added by Eduardo Murai Soares	 
	/*setEnabled(MENU_FILE, M_FILE_SENDBREAKSEQ, isConnected);
	setEnabled(MENU_FILE, M_FILE_LOCK, isConnected);
	setEnabled(MENU_FILE, M_FILE_UNLOCK, isConnected);
	setEnabled(MENU_FILE, M_FILE_POWER_ON, isConnected);
	setEnabled(MENU_FILE, M_FILE_POWER_OFF, isConnected);
	setEnabled(MENU_FILE, M_FILE_POWER_CYCLE, isConnected);
	setEnabled(MENU_FILE, M_FILE_POWER_STATUS, isConnected);*/
	// [CYCLADES-END]

	//updatePluginMenu();
    }

    public void close() {
	// !!! TODO, note that this can only be generated in SSH2 clone window now
	// so we don't need to care about this...
    }

    protected abstract void sshPreferencesDialog(String title);
    
    protected void sshPreferencesDialog() {
        initCiphers();
        sshPreferencesDialog("MindTerm - SSH Preferences");
    }

    
    protected static String[] hktypes, ciphers, macs;

    protected static final String[] compc2s  = { "none", "low", "medium", "high" };
    protected static final String[] comps2c  = { "none", "medium" };
    protected static final String[] lvl2comp = { "none", "low", "low", "low",
                                                 "medium", "medium", "medium",
                                                 "high", "high", "high" };
    protected static final String[] protos   = { "auto", "ssh2", "ssh1" };
    protected static final int[]    comp2lvl = { 0, 2, 6, 9 };


    protected final static String[] AUTH_METHODS = { "password", "publickey",
                                                     "securid", "cryptocard",
                                                     "tis", "kbd-interact",
                                                     "hostbased", "custom list..." };

    protected abstract void sshNewServerDialog(String title);
    protected final void sshNewServerDialog() {
        initCiphers();
        sshNewServerDialog("MindTerm - New Server");
    }

    protected abstract void currentTunnelsDialog(String title);
    protected final void currentTunnelsDialog() {
        currentTunnelsDialog("MindTerm - Currently Open Tunnels");
    }

    protected abstract void setupTunnelsDialog(String title);
    protected final void setupTunnelsDialog() {
        setupTunnelsDialog("MindTerm - Tunnel Setup");
    }

    protected abstract void connectDialog(String title);

    public final void loadFileDialog() {

        File file = GUI.selectFile(parent, "MindTerm - Select file to load settings from", 
                                   client.propsHandler.getSSHHomeDir(), false);
        
        if (file == null) return;
        
        try {
            String pwd = "";
            do {
                try {
                    client.propsHandler.setPropertyPassword(pwd);
                    client.propsHandler.loadAbsoluteFile(file.getAbsolutePath(), false);
                    client.quiet = true;
                    client.sshStdIO.breakPromptLine("Loaded new settings: " + file.getName());
                    break;
                } catch(SSHClient.AuthFailException ee) {
                }
            } while((pwd = passwordDialog("Please give password for " +
                                          file.getName(), "MindTerm - File Password")) != null);
        } catch (Throwable t) {
            alertDialog("Error loading settings: " + t.getMessage());
        }
    }

    protected abstract void keyGenerationDialogCreate(String title);
    protected final void keyGenerationDialogCreate() {
        keyGenerationDialogCreate("MindTerm - Publickey Keypair Generation");
    }
    
    protected abstract void keyGenerationDialogEdit(String title);
    protected final void keyGenerationDialogEdit() {
        keyGenerationDialogEdit("MindTerm - Publickey Keypair Edit");
    }

    public final void saveAsFileDialog() {
	String fname = client.propsHandler.currentAlias;
	if(fname == null)
	    fname = client.propsHandler.getProperty("server");
        fname += SSHPropertyHandler.PROPS_FILE_EXT;

        File file = GUI.selectFile(parent, "MindTerm - Select file to save settings to", 
                                   client.propsHandler.getSSHHomeDir(), fname, true);        

        if (file == null) return;
        
        try {
            if (client.propsHandler.savePasswords) {
                String pwd = setPasswordDialog("Please set password for " + file.getName(),
                                               "MindTerm - Set File Password");
                if (pwd == null)
                    return;
                client.propsHandler.setPropertyPassword(pwd);
            }
            client.propsHandler.saveAsCurrentFile(file.getAbsolutePath());
        } catch (Throwable t) {
            alertDialog("Error saving settings: " + t.getMessage());
        }
    }

    public final void alertDialog(String message) {
	GUI.showAlert("MindTerm - Alert", message, parent);
    }

    public final String passwordDialog(String message, String title) {
	return SSHMiscDialogs.password(title, message, parent);
    }

    public final String setPasswordDialog(String message, String title) {
	return SSHMiscDialogs.setPassword(title, message, parent);
    }

    public final boolean confirmDialog(String message, boolean defAnswer) {
        return GUI.showConfirm("MindTerm - Confirmation", message,
                               defAnswer, parent);
    }

    public final void textDialog(String title, String text, int rows, int cols, boolean scrollbar) {
        GUI.showNotice(parent, title, text, rows, cols, scrollbar);
    }

    protected abstract void aboutDialog(Frame parent, SSHInteractiveClient client, 
                                        String title, String aboutText);
}
