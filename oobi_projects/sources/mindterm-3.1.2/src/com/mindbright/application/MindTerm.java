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

package com.mindbright.application;

import java.applet.Applet;
import java.applet.AppletContext;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Frame;
import java.awt.Panel;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Properties;

import javax.swing.JButton;
import javax.swing.JLabel;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.ssh.SSH;
import com.mindbright.ssh.SSHClient;
import com.mindbright.ssh.SSHInteractiveClient;
import com.mindbright.ssh.SSHMenuHandler;
import com.mindbright.ssh.SSHMenuHandlerFull;
import com.mindbright.ssh.SSHPropertyHandler;
import com.mindbright.ssh.SSHSCPClient;
import com.mindbright.ssh.SSHStdIO;
import com.mindbright.sshcommon.SSHSCPStdoutProgress;
import com.mindbright.terminal.GlobalClipboard;
import com.mindbright.terminal.TerminalMenuHandler;
import com.mindbright.terminal.TerminalMenuHandlerFull;
import com.mindbright.terminal.TerminalWin;
import com.mindbright.terminal.TerminalWindow;
import com.mindterm.util.CYProps;
import com.mindterm.util.Formatter;

import com.mindbright.util.HandleEMSMessage;

public class MindTerm extends Applet implements Runnable, WindowListener {

	// [CYCLADES-START]
	/**
	 * Container of the Swing/AWT main component.
	 * Added by Eduardo Murai Soares 
	 */
	private Container appContainer;	
	
	private boolean mainWindow = true;
	
	private Properties mindtermProps = new Properties();
	private HandleEMSMessage emsMsg = null;
    // [CYCLADES-END]
	
    static Properties paramTermProps = new Properties();
    static Properties paramSSHProps  = new Properties();

    public static String javaVersion = "<unknown>";
    public static String javaVendor  = "<unknown>";
    public static String osName      = "<unknown>";
    public static String osArch      = "<unknown>";
    public static String osVersion   = "<unknown>";

    Frame                frame;
    TerminalWin          term;
    SSHInteractiveClient client;
    SSHInteractiveClient sshClone;
    SSHStdIO             console;
    Thread               clientThread;
    
    boolean    mergedTermProps;
    Properties sshProps;
    Properties termProps;

	// [CYCLADES-START]
    public 
    //  [CYCLADES-END]
    	String[]   cmdLineArgs;

    String  commandLine = null;
    String  sshHomeDir  = null;
    String  propsFile   = null;

    boolean usePopMenu   = false;
    boolean haveMenus    = true;
    boolean haveGUI      = true;
    boolean exitOnLogout = false;
    boolean quiet        = true;
    boolean useSwing     = false;

    boolean doSCP        = false;
    boolean recursiveSCP = false;
    boolean toRemote     = true;
    int     firstArg     = 0;

    boolean autoSaveProps = true;
    boolean autoLoadProps = true;

    boolean savePasswords = false;

    int     popButtonNum = 3;

    boolean isClosing  = false;

    // !!!
    boolean        separateFrame = true;
    public boolean weAreAnApplet = false;
    boolean        useAWT = false;
    
    //Text fields used for getting information to send to the server
    CustomJTextField jTextField1 = new CustomJTextField();
    CustomJTextField jTextField2 = new CustomJTextField();
    CustomJTextField jTextField3 = new CustomJTextField();
    CustomJTextField jTextField4 = new CustomJTextField();
    CustomJTextField jTextField5 = new CustomJTextField();
    CustomJTextField jTextField6 = new CustomJTextField();
    CustomJTextField jTextField7 = new CustomJTextField();
    CustomJTextField jTextField8 = new CustomJTextField();
    CustomJTextField jTextField9 = new CustomJTextField();
    CustomJTextField jTextField10 = new CustomJTextField();
    CustomJTextField jTextField11 = new CustomJTextField();        
    CustomJTextField jTextField12 = new CustomJTextField();
    CustomJTextField jTextField13 = new CustomJTextField();
    CustomJTextField jTextField14 = new CustomJTextField();
    CustomJTextField jTextField15 = new CustomJTextField();
    CustomJTextField jTextField16 = new CustomJTextField();

    static Hashtable terminals = new Hashtable();

    static synchronized boolean isLastTerminal() {
        return terminals.isEmpty();
    }

    static synchronized void addTerminal(MindTerm mindterm) {
        terminals.put(mindterm, mindterm);
    }

    static synchronized void removeTerminal(MindTerm mindterm) {
        terminals.remove(mindterm);
    }

    public MindTerm() {
        super();
        this.sshProps  = paramSSHProps;
        this.termProps = paramTermProps;
        addTerminal(this);
    }

    public MindTerm(Properties sshProps, Properties termProps) {
        this.sshProps  = sshProps;
        this.termProps = termProps;
        addTerminal(this);
    }

    /*public static void main(String[] argv) {
        MindTerm controller    = new MindTerm(paramSSHProps, paramTermProps);
        controller.cmdLineArgs = argv;

        try {
            controller.getApplicationParams();
        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
            System.exit(1);
        }

        try {
            controller.run();
        } catch (Exception e) {
            System.out.println("Internal error running controller");
            e.printStackTrace();
        }
    }*/

    public void init() {
        weAreAnApplet  = true;
        autoSaveProps  = false;
        autoLoadProps  = false;
        savePasswords  = false;

        getAppletParams();
        (new Thread(this, "MindTerm.init")).start();
    }

    public void run() {
        try {
            if(sshClone != null) {
                if(!sshClone.isSSH2) {
                    client = new SSHInteractiveClient(sshClone, this);
                    sshClone = null;
                } else {
                    client = null;
                    sshClone.newShell();
                    removeTerminal(this);
                    return;
                }
            } else {
                // Default when running single command is not to allocate a PTY
                //
                if(commandLine != null
                        && sshProps.getProperty("force-pty") == null) {
                    sshProps.put("force-pty", "false");
                }

                SSHPropertyHandler propsHandler =
                    new SSHPropertyHandler(sshProps, true);
					
				// [CYCLADES-START] 
                propsHandler.mergeAPMProperties(mindtermProps);                                
                // [CYCLADES-END]					

                if(propsFile != null) {
                    try {
                        propsHandler =
                            SSHPropertyHandler.fromFile(propsFile, "");
                    } catch (SSHClient.AuthFailException e) {
                        throw new Exception("Sorry, can only use passwordless"
                                            + "settings files for now");
                    }
                    propsHandler.mergeProperties(sshProps);
                }

                client = new SSHInteractiveClient(quiet, exitOnLogout,
                                                  propsHandler, this);
            }

            SSHPropertyHandler props = client.getPropertyHandler();
            if(weAreAnApplet && props.getDefaultProperty("server") == null){
                props.setDefaultProperty("server", getCodeBase().getHost());
            }
            if (props.getDefaultProperty("username") == null) {
                String auto = props.getDefaultProperty("auto-username");
                if (Boolean.valueOf(auto).booleanValue()) {
                    try {
                        String user = System.getProperty("user.name", "");
                        props.setDefaultProperty("username", user);
                    } catch (Throwable t) {
                        System.err.println("Failed to get username: " + t);
                    }
                }
            }

            console = (SSHStdIO)client.getConsole();

            // If we loaded a specific property-file we merge the
            // termProps from there
            //
            if(props.getInitTerminalProperties()!=null) {
                Properties newTermProps = new Properties(
                    props.getInitTerminalProperties());
                if(termProps != null && !termProps.isEmpty()) {
                    Enumeration e = termProps.keys();
                    while(e.hasMoreElements()) {
                        String name = (String)e.nextElement();
                        newTermProps.put(name, termProps.getProperty(name));
                    }
                    mergedTermProps = true;
                }
                termProps = newTermProps;
            }

            // First we initialize the GUI if we have one (to be able to set
            // properties to terminal
            //
            
            String proto = getParameter("proto");
            
            if(haveGUI&&proto.equals("89")) {
                initGUI();
                console.setTerminal(term);
                console.setOwnerContainer(frame);
                console.setOwnerName(SSH.VER_MINDTERM);
                console.updateTitle();
                try {
                    while(!frame.isShowing())
                        Thread.sleep(50);
                } catch(InterruptedException e) {
                    // !!!
                }

                // !!! When we are in the Panel of the Applet we don't get a
                // componentShown event so we have to "emulate" it since we
                // depend on it to be correctly drawn
                //
                if(!separateFrame) {
                    term.emulateComponentShown();
                }
            }

            if(!props.setSSHHomeDir(sshHomeDir)) {
                throw new Exception("License not accepted, exiting");
            }
            props.setAutoSaveProps(autoSaveProps);
            props.setAutoLoadProps(autoLoadProps);
            props.setSavePasswords(savePasswords);
            client.updateMenus();

            /*if(commandLine != null) {
                if(!doSCP) {
                    client.doSingleCommand(commandLine);
                } else {
                    if((cmdLineArgs.length - firstArg) < 2)
                        throw new Exception(
                            "scp must have at least two arguments (<source> "
                            + "<destination>)");
                    String[] fileList =
                        new String[cmdLineArgs.length - firstArg - 1];
                    String  target, source =
                        commandLine.substring(0, commandLine.lastIndexOf(' '));
                    for(int i = firstArg; i < cmdLineArgs.length - 1; i++) {
                        fileList[i - firstArg] = cmdLineArgs[i];
                    }
                    target = cmdLineArgs[cmdLineArgs.length - 1];
                    String srvHost = props.getSrvHost();
                    int    srvPort = props.getSrvPort();
                    SSHSCPClient scp = new SSHSCPClient(
                                           srvHost, srvPort, props,
                                           (SSH.DEBUG ? client : null), new File("."), SSH.DEBUG);
                    if(SSH.DEBUG) {
                        scp.scp1().setProgress(new SSHSCPStdoutProgress());
                    }
                    if(toRemote) {
                        scp.scp1().copyToRemote(fileList, target,recursiveSCP);
                    } else {
                        scp.scp1().copyToLocal(target, source, recursiveSCP);
                    }
                }
            } else*/
            if(proto.equals("89")) {
                try {
                    clientThread = new Thread(client, "MindTerm");
                    clientThread.start();
                    clientThread.join();
                } catch(InterruptedException e) {
                    // !!!
                }
            } else {
            	Properties telnetProps = new Properties();
                String name, value;
                Enumeration e = termProps.keys();
                while(e.hasMoreElements()) {
                    name  = (String)e.nextElement();
                    value = termProps.getProperty(name);
                    if(value != null) {
                        telnetProps.put(name, value);
                    }
                }
            	telnetProps.put("username", props.getProperty("username"));
            	telnetProps.put("password", props.getProperty("password"));
            	telnetProps.put(CYProps.PHYS_PORT_AUTH, props.getProperty(CYProps.PHYS_PORT_AUTH));
            	telnetProps.put("typed-input", proto.equals("86")? "raw" : "buffered");
            	
                ModuleTelnetImpl telnet = new ModuleTelnetImpl(telnetProps);
                telnet.activate(client);
                client.forcedDisconnect();
                telnetProps = null;
                termProps= null;
                close();
            }

        } catch (IllegalArgumentException ae) {
            if(client != null)
                client.alert(ae.getMessage());
            System.out.println(ae.getMessage());
        } catch (FileNotFoundException fe) {
            System.out.println("Settings-file not found: " + fe.getMessage());
        } catch (Exception e) {
            if(client != null)
                client.alert("Error: "  + e.getMessage());
            System.out.println("Error: " + e.getMessage());
            if(SSH.DEBUGMORE) {
                e.printStackTrace();
            }
        }

        // Do not ask for confirmation in this case
        confirmedClose = true;

        windowClosing(null);
        if(isLastTerminal())
            doExit();
    }

    public void getAppletParams() {
        String    name;
        String    value;
        String    param;
        int       i;

        // [CYCLADES-START]
        String cvalue="";
        try {        
        cvalue = getParametersFromBaseClass("app");
        if (cvalue!=null) parseCycladesParams(cvalue);
        } catch (Exception e) {
        	System.out.println("Error while parsing parameters sent by APM : "+cvalue+":");
        	e.printStackTrace();
        }
        // [CYCLADES-END]
        try {
            useAWT = (new Boolean(getParameter("useAWT"))).booleanValue();
        } catch (Exception e) {
        }

        try {
            separateFrame = (new Boolean(getParameter("sepframe"))).booleanValue();
        } catch (Exception e) {
            separateFrame = true;
        }
        try {
            SSH.DEBUG = (new Boolean(getParameter("verbose"))).booleanValue();
        } catch (Exception e) {
            SSH.DEBUG = false;
        }

        try {
            SSH.DEBUGMORE = (new Boolean(getParameter("debug"))).booleanValue();
            SSH.DEBUG = SSH.DEBUGMORE;
        } catch (Exception e) {}

        try {
            quiet = (new Boolean(getParameter("quiet"))).booleanValue();
        } catch (Exception e) {
            quiet = true;
        }

        try {
            exitOnLogout = (new Boolean(getParameter("exit-on-logout"))).booleanValue();
        } catch (Exception e) {
            exitOnLogout = false;
        }

        try {
            savePasswords = (new Boolean(getParameter("savepasswords"))).booleanValue();
        } catch (Exception e) {
            savePasswords = false;
        }

        param = getParameter("menus");
        if(param != null) {
            if(param.equals("no"))
                haveMenus = false;
            else if(param.startsWith("pop")) {
                getPopupButtonNumber(param);
                usePopMenu = true;
            }
        }

        param = getParameter("autoprops");
        if(param != null) {
            if(param.equals("save")) {
                autoSaveProps = true;
                autoLoadProps = false;
            } else if(param.equals("load")) {
                autoSaveProps = false;
                autoLoadProps = true;
            } else if(param.equals("both")) {
                autoSaveProps = true;
                autoLoadProps = true;
            }
        }

        sshHomeDir  = getParameter("sshhome");
        propsFile   = getParameter("propsfile");
        commandLine = getParameter("commandline");

        getDefaultParams();

        for(i = 0; i < SSHPropertyHandler.defaultPropDesc.length; i++) {
            name  = SSHPropertyHandler.defaultPropDesc[i][SSHPropertyHandler.PROP_NAME];
            value = getParametersFromBaseClass(name);
            if(value != null)
                paramSSHProps.put(name, value);
        }
        i = 0;
        while((value = getParameter("local" + i)) != null) {
            paramSSHProps.put("local" + i, value);
            i++;
        }
        i = 0;
        while((value = getParameter("remote" + i)) != null) {
            paramSSHProps.put("remote" + i, value);
            i++;
        }

        String tPropNames[] = TerminalWin.getPropertyNames();
        for(i = 0; i < tPropNames.length; i++) {
            name  = tPropNames[i];
            value = getParametersFromBaseClass(name);
            if(value != null)
                termProps.put(name, value);
        }

        param = getParameter("appletbg");
        if(param != null) {
            Color c;
            try {
                c = TerminalWin.getTermColor(param);
            } catch (IllegalArgumentException e) {
                try {
                    c = TerminalWin.getTermRGBColor(param);
                } catch (Throwable t) {
                    c = null;
                    // !!!
                }
            }
            if(c != null)
                this.setBackground(c);
        }
    }

    public AppletContext getAppletContext() {
        AppletContext ctx = null;
        if(weAreAnApplet) {
            ctx = super.getAppletContext();
        }
        return ctx;
    }

    /*public void getApplicationParams() throws Exception {
        String    name;
        String    value;
        int       numOfOpts;
        int       i;

        // First we check the MindTerm options (i.e. not the
        // ssh/terminal-properties)
        //
        try {
        	// [CYCLADES-START]
        	if (cmdLineArgs==null) return;
        	// [CYCLADES-END]
        	
            for(i = 0; i < cmdLineArgs.length; i++) {
                String arg = cmdLineArgs[i];
                if(!arg.startsWith("--"))
                    break;
                switch(arg.charAt(2)) {
                case 'h':
                    sshHomeDir = cmdLineArgs[++i];
                    break;
                case 'f':
                    propsFile = cmdLineArgs[++i];
                    break;
                case 'd':
                    haveGUI = false;
                    break;
                case 'e':
                    exitOnLogout = true;
                    break;
                case 'm': {
                        String typ = cmdLineArgs[++i];
                        if(typ.equals("no"))
                            haveMenus = false;
                        else if(typ.startsWith("pop")) {
                            getPopupButtonNumber(typ);
                            usePopMenu = true;
                        } else
                            throw new Exception("value of '--m' must be 'no', "
                                                + "'pop1', 'pop2', or 'pop3'");
                        break;
                    }
                case 'p': {
                        String typ = cmdLineArgs[++i];
                        if(typ.equals("save")) {
                            autoSaveProps = true;
                            autoLoadProps = false;
                        } else if(typ.equals("load")) {
                            autoSaveProps = false;
                            autoLoadProps = true;
                        } else if(typ.equals("both")) {
                            autoSaveProps = true;
                            autoLoadProps = true;
                        } else if(typ.equals("none")) {
                            autoSaveProps = false;
                            autoLoadProps = false;
                        } else
                            throw new Exception("value of '--p' must be 'save', "
                                                + "'load', 'both', or 'none'");
                        break;
                    }
                case 'q':
                    String val = cmdLineArgs[++i];
                    if(val.equalsIgnoreCase("true")
                            || val.equalsIgnoreCase("false")) {
                        quiet = Boolean.valueOf(val).booleanValue();
                    } else {
                        throw new Exception("value of '--q' must be 'true' or "
                                            + "'false'");
                    }
                    break;
                case 'r':
                    recursiveSCP = true;
                    break;
                case 's':
                    haveGUI = false;
                    doSCP   = true;
                    String direction = cmdLineArgs[++i];
                    if(direction.equalsIgnoreCase("toremote")) {
                        toRemote = true;
                    } else if(direction.equalsIgnoreCase("tolocal")) {
                        toRemote = false;
                    } else {
                        throw new Exception("value of '--s' must be "
                                            + "'toremote' or 'tolocal'");
                    }
                    break;
                case 'v':
                    System.out.println("verbose mode selected...");
                    SSH.DEBUG = true;
                    break;
                case 'x':
                    savePasswords = true;
                    break;
                case 'V':
                    System.out.println(SSH.VER_MINDTERM);
                    System.out.println(
                        "SSH protocol version "
                        + SSH.SSH_VER_MAJOR + "." + SSH.SSH_VER_MINOR);
                    System.exit(0);
                    break;
                case 'D':
                    SSH.DEBUG     = true;
                    SSH.DEBUGMORE = true;
                    break;
                case '?':
                    printHelp();
                    System.exit(0);
                default:
				// [CYCLADES-START]              
                	// --app receives application parameters in form <paramName>=<paramValue>&
                	// sample: -app parma=xxx&parmb=yyy
                    if ("--app".equals(arg.trim())) 
                    	parseCycladesParams(cmdLineArgs[++i]);
                    	                    
                    else // [CYCLADES-END]
                    throw new Exception("unknown parameter '" + arg + "'");
                }
            }
        } catch (Exception e) {
            printHelp();
            throw e;
        }

        getDefaultParams();

        numOfOpts = i;
        for(i = numOfOpts; i < cmdLineArgs.length; i += 2) {
            name = cmdLineArgs[i];
            if((name.charAt(0) != '-') || ((i + 1) == cmdLineArgs.length))
                break;
            name  = name.substring(1);
            value = cmdLineArgs[i + 1];
            if(SSHPropertyHandler.isProperty(name)) {
                paramSSHProps.put(name, value);
            } else if(TerminalWin.isProperty(name))
                paramTermProps.put(name, value);
            else
                System.out.println("Unknown property '" + name + "'");
        }

        if(i < cmdLineArgs.length) {
            firstArg = i;
            commandLine = "";
            for(; i < cmdLineArgs.length; i++) {
                commandLine += cmdLineArgs[i] + " ";
            }
            commandLine = commandLine.trim();
        }
    }

    void printHelp() {
        System.out.println("usage: MindTerm [options] [properties] [commandline]");
        System.out.println("Options:");
        System.out.println("  --d            No terminal-window, only dumb command-line and port-forwarding.");
        System.out.println("  --e            Exit MindTerm after logout (i.e. single session).");
        System.out.println("  --f <file>     Use settings from the given file as default.");
        System.out.println("  --h dir        Name of the MindTerm home-dir (default: ~/mindterm/).");
        System.out.println("  --m <no | pop | popN>");
        System.out.println("                 Use no menus or popup (on mouse-button N) menu.");
        System.out.println("  --p <save | load | both | none>");
        System.out.println("                 Sets automatic save/load flags for property-files.");
        System.out.println("  --q <true | false>");
        System.out.println("                 Quiet; don't query for server/username if given.");
        System.out.println("  --v            Verbose; display verbose messages.");
        System.out.println("  --x            Save passwords in encrypted property-files.");
        System.out.println("  --D            Debug; display extra debug info.");
        System.out.println("  --V            Version; display version number only.");
        System.out.println("  --?            Help; display this help.");
    }*/

    void getPopupButtonNumber(String param) {
        if(param.length() == 4) {
            try {
                popButtonNum = Integer.valueOf(param.substring(3)).intValue();
                if(popButtonNum < 1 || popButtonNum > 3)
                    popButtonNum = 3;
            } catch (NumberFormatException e) {
                // !!!
            }
        }
    }

    void getDefaultParams() {
        try {
            javaVersion = System.getProperty("java.version");
            javaVendor  = System.getProperty("java.vendor");
            osName      = System.getProperty("os.name");
            osArch      = System.getProperty("os.arch");
            osVersion   = System.getProperty("os.version");
        } catch (Throwable t) {
            // !!!
        }

        try {
            if (sshHomeDir == null) {
                String hDir = System.getProperty("user.home");
                if (hDir == null)
                    hDir = System.getProperty("user.dir");
                if (hDir == null)
                    hDir = System.getProperty("java.home");

                sshHomeDir = hDir + File.separator + "mindterm";
                File f = new File(sshHomeDir);
                if (!f.exists()) {
                    if (osName.toLowerCase().startsWith("win")) {
                        sshHomeDir = hDir + File.separator + "Application Data";
                        f = new File(sshHomeDir);
                        if (!f.exists())
                            sshHomeDir = hDir;
                        sshHomeDir += File.separator + "MindTerm";
                    } else {
                        sshHomeDir = hDir + File.separator + ".mindterm";
                    }
                }
                sshHomeDir += File.separator;
            }
        } catch (Throwable t) {
            // !!!
        }
    }

    public void initGUI() {
        Container container;
        if (separateFrame) {
            if (!useAWT) {
                frame = AWTConvenience.tryToAllocateJFrame();
            }
            if (frame == null) { // fallback to use AWT
                useSwing = false;
                frame = new Frame();
            } else {
                useSwing = true;
            }
            
            frame.addWindowListener(this);
            container = frame;
        } else {
            Component comp = this;
            do {
                comp = comp.getParent();
            } while(!(comp instanceof Frame));
            frame = (Frame)comp;
            container = this;
            // Force AWT-layout when embedded in a webpage since this
            // class still extends Applet
            useSwing = false;
        }

		// [CYCLADES-START] 
        // appContainer added by Eduardo Murai Soares
        this.appContainer = container; 
        // [CYCLADES-END]

        term = new TerminalWin(frame, termProps, (sshClone == null));
        if (separateFrame) {
            term.setIgnoreClose();
        }

        if(mergedTermProps)
            term.setPropsChanged(true);
        
        if ("enable".equals(getParameter("ems"))) {
        	this.emsMsg = new HandleEMSMessage();
        	term.addOutputListener(this.emsMsg);
        }
		
        // [CYCLADES-START]
        // Panel setup moved here by Eduardo Murai Soares
        // This action was necessary because menus.setupMenuBar will add MenuBar to 
        // the main panel in case of Swing API
        AWTConvenience.getContentPane(container).setLayout(new BorderLayout());
        AWTConvenience.getContentPane(container).add(term.getPanelWithScrollbar(), BorderLayout.CENTER);
        
        Panel  p = new Panel();
        
        if("enable".equals(getParameter("ems")))
            term.setProperty("backspace-send", "bs", true);
            
        boolean specialKey = false;
        try{
            specialKey = "UTF-8".equals(getParameter("encoding"));
        }catch (Exception e){
            specialKey = false;
        }
        
        //[CYCLADES-START]
        term.setSpecialKeysUTF8(specialKey);
        addButtons(p, createActionListener(), "enable".equals(getParameter("ems")));
        //[CYCLADES-END]
        
        AWTConvenience.getContentPane(container).add(p, BorderLayout.SOUTH);

        // [CYCLADES-END]
        		
        if(haveMenus) {
            SSHMenuHandler      menus;
            TerminalMenuHandler tmenus;
            try {
                menus = SSHMenuHandlerFull.getInstance(frame);
                menus.init(this, client, frame, term);
                tmenus = TerminalMenuHandlerFull.getInstance(frame);
                tmenus.setTerminalWin(term);
                term.setMenus(tmenus);
                client.setMenus(menus);
                menus.setupMenuBar(usePopMenu
					// [CYCLADES-START]	
                	,isAppletMode()
                	// [CYCLADES-STOP]
					);
                tmenus.setTerminalMenuListener(menus);
                term.setClipboard(GlobalClipboard.getClipboardHandler(tmenus));
            } catch (Throwable t) {
                t.printStackTrace();
                System.out.println("Full menus can't be enabled since classes "
                                   + "are missing");
                term.setMenus(null);
                client.setMenus(null);
                term.setClipboard(GlobalClipboard.getClipboardHandler());
            }
        } else {
            term.setClipboard(GlobalClipboard.getClipboardHandler());
        }

        term.addAsEntropyGenerator(client.randomSeed());

		// [CYCLADES-START]
        // lines comented because they were moved to be before "if (haveMenus)"        
        // AWTConvenience.getContentPane(container).setLayout(new BorderLayout());
        // AWTConvenience.getContentPane(container).add(term.getPanelWithScrollbar(), BorderLayout.CENTER);
		// [CYCLADES-END]        
        
        frame.pack();
        frame.setVisible(true);
        AWTConvenience.getContentPane(container).setVisible(true);
        p.setVisible(true);
        p.repaint();
        term.requestFocus();
    }

    public void doExit() {
        
        if(!separateFrame && term != null) {
        	// [CYCLADES-START] : clear screen commented
            // term.clearScreen();
             
            
            //term.setAttributeBold(true);
            //term.write("Thank you for using MindTerm...");
            // [CYCLADES-END]
        }
        if(!weAreAnApplet) {
            System.exit(0);
        }
    }

    boolean confirmedClose = true;
    public boolean confirmClose() {
        if(client != null && !confirmedClose) {
            try {
                client.getPropertyHandler().checkSave();
            } catch(IOException ee) {
                client.alert("Error saving settings: " + ee.getMessage());
            }
            if(client.isOpened() &&
                    !client.askConfirmation(
                        "Do you really want to disconnect from " +
                        client.getPropertyHandler().getProperty("server") + "?",
                        false)) {
                confirmedClose = false;
            } else {
                confirmedClose = true;
            }
        }
        return confirmedClose;
    }

    // [CYCLADES-START]
    public
    //  [CYCLADES-END]
    	void initParams(MindTerm mindterm) {
        this.sshHomeDir     = mindterm.sshHomeDir;
        this.propsFile      = mindterm.propsFile;
        this.usePopMenu     = mindterm.usePopMenu;
        this.haveMenus      = mindterm.haveMenus;
        this.haveGUI        = mindterm.haveGUI;
        this.exitOnLogout   = mindterm.exitOnLogout;
        this.quiet          = mindterm.quiet;
        this.separateFrame  = true;
        this.weAreAnApplet  = mindterm.weAreAnApplet;
        this.autoLoadProps  = mindterm.autoLoadProps;
        this.popButtonNum   = mindterm.popButtonNum;
    }

    public void cloneWindow() {
        MindTerm mindterm = new MindTerm(this.sshProps, this.termProps);
        mindterm.initParams(this);
        mindterm.sshClone = this.client;
        //  [CYCLADES-START]
        //  new window is child of the main window
        mindterm.setMainWindow(false);
        // [CYCLADES-END]        
        (new Thread(mindterm, "MindTerm.clone")).start();
    }

    public void newWindow() {
        MindTerm mindterm = new MindTerm(paramSSHProps, paramTermProps);
        mindterm.initParams(this);
        
        //  [CYCLADES-START]
        //  new window is child of the main window
        mindterm.setMainWindow(false);
        // [CYCLADES-END]
        
        (new Thread(mindterm, "MindTerm.window")).start();
    }

    public void destroy() {
        Enumeration e = terminals.elements();
        while(e.hasMoreElements()) {
            MindTerm mt = (MindTerm)e.nextElement();
            if(mt.client != null && mt.client.isSSH2) {
                mt.client.transport.normalDisconnect("User exited");
            }
            
            // [CYCLADES-START]
            checkAppletWindow(mt);
            // [CYCLADES-END]
            
            if(mt.clientThread != null) {
                mt.clientThread.stop();
            }
        }
    }

    
    //  [CYCLADES-START]
    public void checkAppletWindow(MindTerm term) { }
    //  [CYCLADES-END]  
    
    public void close() {
        if(!confirmClose())
            return;
        
        // [CYCLADES-START]
        if (client!=null)
        // [CYCLADES-END]        
        
        if(client.isSSH2) {
            client.transport.normalDisconnect("User closed connection");
        }
        
        // [CYCLADES-START]
        checkAppletWindow(this);
        //  [CYCLADES-END]
        
        if(clientThread != null)
            clientThread.stop();
        
        if(this.emsMsg != null)
        	term.removeOutputListener(this.emsMsg);
        
        if(separateFrame && haveGUI && frame != null) {
            frame.dispose();
        }

    }

    public void exit() {
        if(!confirmClose())
            return;
        destroy();
    }

    public synchronized void windowClosing(WindowEvent e) {
        if(isClosing)
            return;
        isClosing = true;

        if(!confirmClose()) {
            isClosing = false;
            return;
        }

        // [CYCLADES-START]: close the current connection
        close();
        // [CYCLADES-STOP]
        
        if(separateFrame && haveGUI && frame != null) {
            frame.dispose();
        }

        // [CYCLADES-START]: task done inside the close method
        //if(clientThread != null && clientThread.isAlive())
        //   clientThread.stop();
        // [CYCLADES-STOP]
        
        removeTerminal(this);        
    }

    public void windowDeiconified(WindowEvent e) {
        term.requestFocus();
    }

    public void windowOpened(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowActivated(WindowEvent e) {}
    public void windowDeactivated(WindowEvent e) {}
	
	 // [CYCLADES-START]
    /*class StateTO {    	
    	public StateTO(MindTerm parent) {
    		sshHomeDir     = parent.sshHomeDir;
            propsFile      = parent.propsFile;
            usePopMenu     = parent.usePopMenu;
            haveMenus      = parent.haveMenus;
            haveGUI        = parent.haveGUI;
            exitOnLogout   = parent.exitOnLogout;
            quiet          = parent.quiet;
            separateFrame  = true;
            weAreAnApplet  = parent.weAreAnApplet;
            autoLoadProps  = parent.autoLoadProps;
            popButtonNum   = parent.popButtonNum;    		
    		}
    	
        public String sshHomeDir;
        public String propsFile;
        public boolean usePopMenu;
        public boolean haveMenus;
        public boolean haveGUI;
        public boolean exitOnLogout;
        public boolean quiet;
        public boolean separateFrame;
        public boolean weAreAnApplet;
        public boolean autoLoadProps;
        public int popButtonNum;    	
    }
    
    public void initParams(StateTO src) {
        this.sshHomeDir     = src.sshHomeDir;
        this.propsFile      = src.propsFile;
        this.usePopMenu     = src.usePopMenu;
        this.haveMenus      = src.haveMenus;
        this.haveGUI        = src.haveGUI;
        this.exitOnLogout   = src.exitOnLogout;
        this.quiet          = src.quiet;
        this.separateFrame  = src.separateFrame;
        this.weAreAnApplet  = src.weAreAnApplet;
        this.autoLoadProps  = src.autoLoadProps;
        this.popButtonNum   = src.popButtonNum;
    }*/
    
	// ------------------------------------------------------------------------------
    // the reason of these methods is to export some attributes
    // necessary for mindterm codes done by Cyclades' staff
    /**
     * Export SSH properties. 
     * @return Properties
     * @author Eduardo Murai Soares
     */
	public static Properties getParamSSHProps() {
		return paramSSHProps;
	}

    /**
     * Export Terminal properties. 
     * @return Properties
     * @author Eduardo Murai Soares
     */
	public static Properties getParamTermProps() {
		return paramTermProps;
	}
	
	/**
	 * @return Frame which holds this applet
	 * @author Eduardo Murai Soares
	 */
	public Frame getFrame() {
		return frame;
	}

	/**
	 * @return Container which holds Swing/AWT main  panel/component
	 * @author Eduardo Murai Soares
	 */
	public Container getAppContainer() {
		return appContainer;
	}
	
	/**
	 * @return True if user confirmed close 
	 * @author Eduardo Murai Soares
	 */
	public boolean getConfirmedClose() {
		return this.confirmedClose;
	}
	
	public void setConfirmedClose(boolean v) {
		this.confirmedClose = v;
	}
	
	public SSHInteractiveClient getClient() {
		return client;	
	}
	
	public boolean isAppletMode() {
		return weAreAnApplet;
	}
	
	public void setSshHomeDir(String path) {
		this.sshHomeDir = path;
	}
	
	public void setupClone(SSHInteractiveClient client) {
		this.sshClone = client;
	}	

	public Properties getSshProps() {
		return sshProps;
	}

	public Properties getTermProps() {
		return termProps;
	}
	
	public TerminalWindow getTerm() {
		return term;
	}	
	
	public boolean isMainWindow() {
		return mainWindow;
	}

	public void setMainWindow(boolean mainWindow) {
		this.mainWindow = mainWindow;
	}
	
	/**
	 * Save mindterm preferences into APM.
	 * 
	 * @see MindTerm3.savePreferences
	 */
	/*public void savePreferences(Properties propsToSave) { }*/

	/**
	 * Parse parameters sent by APM and merge them into Mindterm. 
	 * 
	 * @param rawParams
	 * @throws UnsupportedEncodingException
	 */
    private void parseCycladesParams(String rawParams) throws UnsupportedEncodingException {
    	Formatter.parseCycladesParams(rawParams, new Properties[] {paramSSHProps, mindtermProps});
    }
	
    public String getParametersFromBaseClass(String pname) {
    	return super.getParameter(pname);
    }       
    //  [CYCLADES-END]0
    
    private void addButtons(Panel p, ActionListener al, boolean isEMS){
        
        if (!isEMS){
            JButton b1 = new JButton("Send Break");
            JButton b2 = new JButton("Disconnect");
            
            b1.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
            b2.setFont(new java.awt.Font("MS Sans Serif", 1, 11));

            b1.addActionListener(al);
            b2.addActionListener(al);
            
            p.add(b1, BorderLayout.WEST); p.add(b2, BorderLayout.EAST);
            return;
        }
        JLabel jLabel1 = new JLabel();
        JLabel jLabel2 = new JLabel();
        JLabel jLabel3 = new JLabel();
        JLabel jLabel4 = new JLabel();
        JLabel jLabel5 = new JLabel();        
        JButton jButton1 = new JButton();
        JButton jButton2 = new JButton();
        JButton jButton3 = new JButton();
        JButton jButton4 = new JButton();
        JButton jButton5 = new JButton();
        JButton jButton6 = new JButton();
        JButton jButton7 = new JButton();
        JButton jButton8 = new JButton();
        JButton jButton9 = new JButton();
        JButton jButton10 = new JButton();
        JButton jButton11 = new JButton();
        JButton jButton12 = new JButton();
        JButton jButton13 = new JButton();
        JButton jButton14 = new JButton();
        JButton jButton15 = new JButton();
        JButton jButton16 = new JButton();
        JButton jButton17 = new JButton();
        JButton jButton18 = new JButton();
        JButton jButton19 = new JButton();
        JButton jButton20 = new JButton();
        JButton jButton21 = new JButton();
        JButton jButton22 = new JButton();
        
        p.setSize(this.getWidth(), 285);
        p.setLayout(null);

        this.setSize(this.getWidth(), this.getHeight() + 285);
        
        jLabel1.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jLabel1.setText("System Information:");
        p.add(jLabel1);
        jLabel1.setBounds(0, 0, 130, 15);

        jButton1.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton1.setText("Server");
        p.add(jButton1);
        jButton1.setBounds(0, 20, 140, 20);

        jButton2.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton2.setText("getTimeDate");
        p.add(jButton2);
        jButton2.setBounds(140, 20, 140, 20);

        jButton3.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton3.setText("IP");
        p.add(jButton3);
        jButton3.setBounds(280, 20, 130, 20);

        jButton4.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton4.setText("KernelLog");
        p.add(jButton4);
        jButton4.setBounds(410, 20, 170, 20);

        jLabel2.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jLabel2.setText("System Settings:");
        p.add(jLabel2);
        jLabel2.setBounds(0, 50, 130, 15);

        jButton5.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton5.setText("TimeDate");
        p.add(jButton5);
        jButton5.setBounds(0, 70, 130, 20);

        jButton6.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton6.setText("IPInfo");
        p.add(jButton6);
        jButton6.setBounds(0, 90, 130, 20);

        jTextField1.setText("#");
        p.add(jTextField1);
        jTextField1.setBounds(370, 140, 60, 20);

        jTextField2.setText("dd");
        p.add(jTextField2);
        jTextField2.setBounds(230, 70, 80, 20);
        jTextField2.setNext(jTextField3);

        jTextField10.setText("mm");
        p.add(jTextField10);
        jTextField10.setBounds(140, 70, 80, 20);
        jTextField10.setNext(jTextField2);
        
        jTextField3.setText("yyyy");
        p.add(jTextField3);
        jTextField3.setBounds(320, 70, 80, 20);
        jTextField3.setNext(jTextField4);

        jTextField4.setText("hh");
        p.add(jTextField4);
        jTextField4.setBounds(410, 70, 80, 20);
        jTextField4.setNext(jTextField5);

        jTextField5.setText("mm");
        p.add(jTextField5);
        jTextField5.setBounds(500, 70, 80, 20);
        jTextField5.setNext(jTextField6);

        jTextField6.setText("interface#");
        p.add(jTextField6);
        jTextField6.setBounds(140, 90, 80, 20);
        jTextField6.setNext(jTextField7);

        jTextField7.setText("ip");
        p.add(jTextField7);
        jTextField7.setBounds(230, 90, 80, 20);
        jTextField7.setNext(jTextField8);

        jTextField8.setText("subnet");
        p.add(jTextField8);
        jTextField8.setBounds(320, 90, 80, 20);
        jTextField8.setNext(jTextField9);

        jTextField9.setText("gateway");
        p.add(jTextField9);
        jTextField9.setBounds(410, 90, 170, 20);

        jLabel3.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jLabel3.setText("Channels:");
        p.add(jLabel3);
        jLabel3.setBounds(0, 120, 130, 15);

        jButton7.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton7.setText("ChannelList");
        p.add(jButton7);
        jButton7.setBounds(0, 140, 100, 20);

        jButton8.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton8.setText("Create");
        p.add(jButton8);
        jButton8.setBounds(100, 140, 90, 20);

        jButton9.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton9.setText("Lock");
        p.add(jButton9);
        jButton9.setBounds(190, 140, 90, 20);

        jButton10.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton10.setText("Close");
        p.add(jButton10);
        jButton10.setBounds(440, 140, 70, 20);

        jButton11.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton11.setText("Switch");
        p.add(jButton11);
        jButton11.setBounds(290, 140, 80, 20);

        jTextField11.setText("#");
        p.add(jTextField11);
        jTextField11.setBounds(510, 140, 70, 20);

        jLabel4.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jLabel4.setText("Tasks:");
        p.add(jLabel4);
        jLabel4.setBounds(0, 170, 130, 15);

        jButton12.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton12.setText("TaskList");
        p.add(jButton12);
        jButton12.setBounds(0, 190, 110, 20);

        jButton13.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton13.setText("DetailOnOff");
        p.add(jButton13);
        jButton13.setBounds(110, 190, 130, 20);

        jButton14.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton14.setText("PageOnOff");
        p.add(jButton14);
        jButton14.setBounds(240, 190, 120, 20);

        jButton15.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton15.setText("Kill");
        p.add(jButton15);
        jButton15.setBounds(360, 190, 90, 20);

        jTextField12.setText("pid");
        p.add(jTextField12);
        jTextField12.setBounds(450, 190, 130, 20);

        jButton16.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton16.setText("Lower");
        p.add(jButton16);
        jButton16.setBounds(0, 210, 110, 20);

        jTextField13.setText("pid");
        p.add(jTextField13);
        jTextField13.setBounds(110, 210, 60, 20);

        jTextField14.setText("pid");
        p.add(jTextField14);
        jTextField14.setBounds(280, 210, 60, 20);

        jButton17.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton17.setText("Raise");
        p.add(jButton17);
        jButton17.setBounds(170, 210, 110, 20);

        jButton18.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton18.setText("Limit");
        p.add(jButton18);
        jButton18.setBounds(340, 210, 110, 20);

        jTextField15.setText("pid");
        p.add(jTextField15);
        jTextField15.setBounds(450, 210, 60, 20);

        jTextField16.setText("mb");
        p.add(jTextField16);
        jTextField16.setBounds(510, 210, 70, 20);

        jLabel5.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jLabel5.setText("Administrative Tasks:");
        p.add(jLabel5);
        jLabel5.setBounds(0, 240, 140, 15);

        jButton19.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton19.setText("Restart");
        p.add(jButton19);
        jButton19.setBounds(0, 260, 130, 20);

        jButton20.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton20.setText("Shutdown");
        p.add(jButton20);
        jButton20.setBounds(130, 260, 150, 20);

        jButton21.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton21.setText("CrashDump");
        p.add(jButton21);
        jButton21.setBounds(280, 260, 140, 20);

        jButton22.setFont(new java.awt.Font("MS Sans Serif", 1, 11));
        jButton22.setText("Disconnect");
        p.add(jButton22);
        jButton22.setBounds(420, 260, 160, 20);
        
        jButton1.addActionListener(al);
        jButton2.addActionListener(al);
        jButton3.addActionListener(al);
        jButton4.addActionListener(al);
        jButton5.addActionListener(al);
        jButton6.addActionListener(al);
        jButton7.addActionListener(al);
        jButton8.addActionListener(al);
        jButton9.addActionListener(al);
        jButton10.addActionListener(al);
        jButton11.addActionListener(al);
        jButton12.addActionListener(al);
        jButton13.addActionListener(al);
        jButton14.addActionListener(al);
        jButton15.addActionListener(al);
        jButton16.addActionListener(al);
        jButton17.addActionListener(al);
        jButton18.addActionListener(al);
        jButton19.addActionListener(al);
        jButton20.addActionListener(al);
        jButton21.addActionListener(al);
        jButton22.addActionListener(al);
    }
    
    private ActionListener createActionListener(){
        ActionListener al = new ActionListener() {
            public void actionPerformed(ActionEvent ae) {
                String ac = ae.getActionCommand();
                
                try{
                    if(ac.toUpperCase().equals("DISCONNECT")) {
                        client.forcedDisconnect();
                        close();
                        return;
                    }
                    if(ac.toUpperCase().equals("SEND BREAK")) {
                        String breakseq = termProps.getProperty(com.mindterm.util.CYProps.BREAK_SEQ);                		
                        if (breakseq==null) {
                        // send Telnet BREAK following RFC spec rules
                        //byte IAC  = (byte) 255; // TELNET COMMAND BYTE
                        //byte CMD_BREAK = (byte) 243; // COMMAND IDENTIFIER : NVT character BRK                			
                        //term.sendBytes(new byte[] {IAC, CMD_BREAK});
                        term.sendBreak();
                        }
                        else {
                            int lastKey = term.getLastKeyTyped();
                            if (lastKey!=13 && lastKey!=10) {                			
                                term.sendBytes(new byte[] {13});
                            }
                        }
                        return; 
                    }
                    if(ac.toUpperCase().equals("SERVER")){
                        term.sendBytes("id\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("GETTIMEDATE")){
                        term.sendBytes("s\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("IP")){
                        term.sendBytes("i\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("KERNELLOG")){
                        term.sendBytes("d\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("TIMEDATE")){
                        term.sendString("s " + jTextField10.getText() + "/" + jTextField2.getText()
                                        + "/" + jTextField3.getText() + " " + jTextField4.getText()
                                        + ":" + jTextField5.getText() + "\r");
                        return;
                    }
                    
                    if(ac.toUpperCase().equals("IPINFO")){
                        term.sendString("i " + jTextField6.getText() + " " + jTextField7.getText()
                                        + " " + jTextField8.getText() + " " + jTextField9.getText()
                                        + "\r");
                        return;
                    }
                    if(ac.toUpperCase().equals("CHANNELLIST")){
                        term.sendBytes("ch\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("CREATE")){
                        term.sendBytes("cmd\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("LOCK")){
                        term.sendBytes("lock\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("SWITCH")){
                        term.sendBytes(String.valueOf("ch -si " + jTextField1.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("CLOSE")){
                        term.sendBytes(String.valueOf("ch -ci " + jTextField11.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("TASKLIST")){
                        term.sendBytes("t\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("DETAILONOFF")){
                        term.sendBytes("f\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("PAGEONOFF")){
                        term.sendBytes("p\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("KILL")){
                        term.sendBytes(String.valueOf("k " + jTextField12.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("LOWER")){
                        term.sendBytes(String.valueOf("l " + jTextField13.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("RAISE")){
                        term.sendBytes(String.valueOf("r " + jTextField14.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("LIMIT")){
                        term.sendBytes(String.valueOf("m " + jTextField15.getText()+ " " + jTextField16.getText() +"\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("RESTART")){
                        term.sendBytes("restart\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("SHUTDOWN")){
                        term.sendBytes("shutdown\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("CRASHDUMP")){
                        term.sendBytes("crashdump\r".getBytes());
                        return;
                    }
                }
                catch (Exception e) {
                    e.printStackTrace();
                }
                //if comes here, command unknown recivied
                System.out.println("Unknown event intercepted");
            }
        };
        return al;
    }
}