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
import java.awt.Button;
import java.awt.Choice;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridBagConstraints;
import java.awt.GridLayout;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Label;
import java.awt.Menu;
import java.awt.MenuBar;
import java.awt.MenuItem;
import java.awt.Panel;
import java.awt.TextArea;
import java.awt.TextField;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.Socket;
import java.util.Properties;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.AWTGridBagContainer;
import com.mindbright.gui.GUI;
import com.mindbright.gui.Logo;
import com.mindbright.jca.security.SecureRandom;
import com.mindbright.net.ProxyAuthenticator;
import com.mindbright.ssh2.SSH2;
import com.mindbright.ssh2.SSH2Connection;
import com.mindbright.ssh2.SSH2ConsoleRemote;
import com.mindbright.ssh2.SSH2Exception;
import com.mindbright.ssh2.SSH2HostKeyVerifier;
import com.mindbright.ssh2.SSH2Interactor;
import com.mindbright.ssh2.SSH2KeyFingerprint;
import com.mindbright.ssh2.SSH2Preferences;
import com.mindbright.ssh2.SSH2Signature;
import com.mindbright.ssh2.SSH2SignatureException;
import com.mindbright.ssh2.SSH2SimpleClient;
import com.mindbright.ssh2.SSH2Transport;
import com.mindbright.ssh2.SSH2TransportEventAdapter;
import com.mindbright.ssh2.SSH2UserCancelException;
import com.mindbright.sshcommon.SSHConsoleRemote;
import com.mindbright.util.JarLoader;
import com.mindbright.util.RandomSeed;
import com.mindbright.util.SecureRandomAndPad;

/**
 * This is a more light-weight version of MindTerm. It has less
 * UI and at startup it only shows a basic login window. All other
 * functionality like port forwards and terminal windows are available
 * through plugins (in the plugins menu).
 * <p>
 * In order to keep the jar-file smaller some functionality
 * (terminal-window and proxy pass through) has been split out into
 * separate jar-files which are loaded if needed. When looking for
 * these classes the code first checks to see if they are already
 * loaded otherwise it checks the <code>jar-path</code> (a
 * ':'-separated list of directories) for the needed jar-files.
 */
public class MindTermLite extends Applet
    implements SSH2Interactor, MindTermApp, ProxyAuthenticator, Runnable {
    private final static String VERSION = "3.0.1";

    private class Actions implements ActionListener, ItemListener {
        private int       action;

        public Actions(int action) {
            this.action = action;
        }

        public void actionPerformed(ActionEvent e) {
            if(action == -1) {
                (new Thread(MindTermLite.this)).start();
            } else if(action == -2) {
                try {
                    client.getTransport().normalDisconnect("Disconnect by user");
                } catch (Exception ee) {
                    // !!!
                }
            } else if(action == -3) {
                if(e.getActionCommand().equals("Cancel")) {
                    pressedCancel = true;
                } else {
                    pressedCancel = false;
                }
                promptDialog.setVisible(false);
            } else {
                modules[action].activate(MindTermLite.this);
            }
        }

        public void itemStateChanged(ItemEvent e) {
            if("password".equals(choiceAuthTyp.getSelectedItem())) {
                textPwd.setEnabled(true);
            } else {
                textPwd.setEnabled(false);
            }
            textPwd.setText("");
        }

    }

    boolean   separateFrame = true;

    Container container;
    Frame     frame;
    Logo      logo;
    Image     discImg;
    Image     connImg;

    boolean weAreAnApplet;
    boolean haveMenus;
    boolean autoSave;
    String  saveLocation;

    Properties settings;

    String host;
    int    port;
    String username;
    String password;

    SSH2SimpleClient client;

    MindTermModule[] modules;
    int              modCnt;
    Button[]         modButs;
    MenuItem[]       modMenuItems;

    Choice     choiceAuthTyp;
    TextField  textUser, textSrv, textPwd;
    Button     connBut, discBut;


    public RandomSeed         randomSeed;
    public SecureRandomAndPad secureRandom;

    public MindTermLite() {
        settings = new Properties();

        settings.put("package-version", SSH2.getPackageVersion(getAppName(),
                     1, 0, "(non-commercial)"));

        modButs      = new Button[32];
        modMenuItems = new MenuItem[32];
        modules      = new MindTermModule[32];
        modCnt       = 0;
        logo         = null;
    }

    public void init() {
        weAreAnApplet = true;
        startMeUp();
    }

    public static void main(String[] argv) {
        final MindTermLite mindterm = new MindTermLite();
        mindterm.startMeUp();
    }

    private void getDefaultParams() {
        try {
            try {
                Boolean sep = new Boolean(getParameter("sepframe"));
                separateFrame = sep.booleanValue();
            } catch (Exception e) {
                separateFrame = true;
            }

            InputStream in =
                getClass().getResourceAsStream("/defaults/settings.txt");
            if(in != null) {
                settings.load(in);
            }

            saveLocation = settings.getProperty("savelocation");

            if(saveLocation == null) {
                saveLocation = System.getProperty("user.home");
                if(saveLocation == null) {
                    saveLocation = System.getProperty("user.dir");
                }
                if(saveLocation == null) {
                    saveLocation = System.getProperty("java.home");
                }
                if(!saveLocation.endsWith(File.separator)) {
                    saveLocation = saveLocation + File.separator;
                }
                saveLocation = saveLocation + "mtlast.txt";
            }

            File f = new File(saveLocation);
            if(f.exists() && f.canRead()) {
                settings.load(new FileInputStream(f));
            }
        } catch (Throwable t) {
            System.err.println("Failed to load local settings: " + t);
        }
    }

    private Container getContainer() {
        if (!weAreAnApplet || separateFrame) {
            frame = new Frame();
            frame.addWindowListener(new WindowAdapter() {
                public void windowClosing(WindowEvent e)  {
                    if(!isConnected()) {
                        if(!weAreAnApplet || separateFrame) {
                            frame.dispose();
                        }
                        if(!weAreAnApplet) {
                            System.exit(0);
                        }
                    }
                }
            });
            frame.setTitle(getAppName() + " " + VERSION);
            return frame;
        } else {
            return this;
        }
    }

    public void startMeUp() {
        try {
            getDefaultParams();

            container = getContainer();

            haveMenus = (Boolean.valueOf(settings.getProperty("havemenus")).
                         booleanValue() && (frame != null));

            autoSave = Boolean.valueOf(settings.getProperty("autosave")).
                       booleanValue();

            discImg = createImage("/defaults/logo_disc.gif");
            connImg = createImage("/defaults/logo_conn.gif");

            if(discImg != null) {
                logo = new Logo(discImg);
            }

            initSeedGenerator();

            container.setLayout(new BorderLayout());

            Label lbl;
            Panel panel = new Panel();

            AWTGridBagContainer grid = new AWTGridBagContainer(panel);

            if(logo != null) {
                Panel p = new Panel();
                p.add(logo);
                container.add(p, BorderLayout.NORTH);
            }

            grid.getConstraints().anchor = GridBagConstraints.WEST;

            lbl = new Label("Server[:port] :");
            grid.add(lbl, 1, 2);

            textSrv = new TextField("", 16);
            grid.add(textSrv, 1, 2);

            lbl = new Label("Username :");
            grid.add(lbl, 2, 2);

            textUser = new TextField("", 16);
            grid.add(textUser, 2, 2);

            lbl = new Label("Authentication :");
            grid.add(lbl, 3, 2);

            choiceAuthTyp = AWTConvenience.newChoice(new String[] { "password", "kbd-interactive"});
            choiceAuthTyp.addItemListener(new Actions(0));
            grid.add(choiceAuthTyp, 3, 2);

            lbl = new Label("Password :");
            grid.add(lbl, 4, 2);

            textPwd = new TextField("", 16);
            textPwd.setEchoChar('*');
            grid.add(textPwd, 4, 2);

            grid.getConstraints().anchor = GridBagConstraints.CENTER;
            grid.getConstraints().fill   = GridBagConstraints.HORIZONTAL;

            Panel bp = new Panel(new FlowLayout());
            bp.add(connBut = new Button("Connect"));
            connBut.addActionListener(new Actions(-1));
            bp.add(discBut = new Button("Disconnect"));
            discBut.addActionListener(new Actions(-2));

            grid.getConstraints().insets = new Insets(4, 24, 12, 24);

            Panel modPanel = getModulesPanel();
            if(modPanel != null) {
                grid.add(modPanel, 5, 4);
            }

            grid.getConstraints().insets = new Insets(4, 4, 0, 4);

            grid.add(bp, 6, 4);

            container.add(panel, BorderLayout.CENTER);

            enableButtons();

            String server = settings.getProperty("server");
            String port   = settings.getProperty("port");
            String user   = settings.getProperty("username");
            String auth   = settings.getProperty("auth-method");
            if(server != null) {
                if(port != null && server.indexOf(':') == -1) {
                    server += ":" + port;
                }
                textSrv.setText(server);
            }
            if(user != null) {
                textUser.setText(user);
            }
            if(frame != null) {
                frame.pack();
                frame.setVisible(true);
            }

            if(server == null) {
                textSrv.requestFocus();
            } else if(user == null) {
                textUser.requestFocus();
            } else if(auth == null || auth.equals("password")) {
                textPwd.requestFocus();
            } else {
                choiceAuthTyp.select(auth);
                connBut.requestFocus();
                (new Actions(0)).itemStateChanged(null);
            }

        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    public void run() {
        try {
            String srvStr = textSrv.getText();
            int i;
            if((i = srvStr.indexOf(':')) > 0) {
                host = srvStr.substring(0, i);
                port = Integer.parseInt(srvStr.substring(i + 1));
            } else {
                host = srvStr;
                port = 22;
            }
            username = textUser.getText();
            password = textPwd.getText();

            connect();

            setProperty("server", host);
            setProperty("port", String.valueOf(port));
            setProperty("username", username);
            setProperty("auth-method", choiceAuthTyp.getSelectedItem());

            enableButtons();

            textPwd.setText("");

        } catch (Exception ee) {
            String msg = ee.getMessage();
            if (null == msg) {
                msg = ee.toString();
            }
            alert("Error connecting: " + msg);
            disconnect();
        }
    }

    private String getLabel(int module) {
        return getProperty("module" + module + ".label");
    }

    private void enableButtons() {
        boolean isConnected = isConnected();
        connBut.setEnabled(!isConnected);
        discBut.setEnabled(isConnected);
        textSrv.setEnabled(!isConnected);
        textUser.setEnabled(!isConnected);
        textPwd.setEnabled(!isConnected);
        choiceAuthTyp.setEnabled(!isConnected);
        for(int i = 0; i < modCnt; i++) {
            if(modButs[i] != null) {
                modButs[i].setEnabled(modules[i].isAvailable(this));
            }
            if(modMenuItems[i] != null) {
                try {
                    modMenuItems[i].setEnabled(modules[i].isAvailable(this));
                } catch (java.lang.NullPointerException e) {
                    // This seems to happen sometimes with 1.5
                }
            }
        }
    }

    private Panel getModulesPanel() {
        int i;
        boolean haveVisible = false;
        for(i = 0; i < modules.length; i++) {
            if(getLabel(i) != null) {
                haveVisible = true;
            }
            String className = settings.getProperty("module" + i);
            if(className == null) {
                break;
            }
            try {
                modules[i] = (MindTermModule)Class.forName(className).newInstance();
                modules[i].init(this);
                modCnt++;
            } catch (Exception e) {
                // !!! TODO
                throw new Error("Module class '" + className + "' not found");
            }
        }
        Panel p = null;
        if(haveVisible) {
            Menu menu = null;
            if(haveMenus) {
                frame.setMenuBar(new MenuBar());
                menu = new Menu("Plugins");
                frame.getMenuBar().add(menu);
                frame.addNotify();
                frame.validate();
            } else {
                p = new Panel(new GridLayout(modCnt, 1));
            }
            for(i = 0; i < modCnt; i++) {
                String label = getLabel(i);
                if(label != null) {
                    if(haveMenus) {
                        modMenuItems[i] = new MenuItem(label);
                        modMenuItems[i].addActionListener(new Actions(i));
                        menu.add(modMenuItems[i]);
                    } else {
                        modButs[i] = new Button(label);
                        modButs[i].addActionListener(new Actions(i));
                        p.add(modButs[i]);
                    }
                }
            }
        }
        return p;
    }

    public void connect() throws SSH2Exception, IOException {
        Socket socket  = null;
        String prxType = getProperty("proxy-type");
        if(prxType != null && !prxType.equals("none")) {
            JarLoader cl = new JarLoader(getProperty("jar-path"),
                                         "lite_proxy.jar");
            try {
                String proxyClass;
                String method;
                if("http".equals(prxType)) {
                    proxyClass = "com.mindbright.net.WebProxyTunnelSocket";
                    method = "getProxy";
                } else if("socks4".equals(prxType)) {
                    proxyClass = "com.mindbright.net.SocksProxySocket";
                    method = "getSocks4Proxy";
                } else if("socks5".equals(prxType)) {
                    proxyClass = "com.mindbright.net.SocksProxySocket";
                    method = "getSocks5Proxy";
                } else {
                    throw new IOException("Unknown proxy type " + prxType);
                }

                Class c = cl.loadClass(proxyClass);

                Class paramTypes[] = {
                    String.class,
                    int.class,
                    String.class,
                    int.class,
                    long.class,
                    Class.forName("com.mindbright.net.ProxyAuthenticator"),
                    String.class,
                };
                Method getProxy = c.getMethod(method, paramTypes);

                int    prxPort = Integer.parseInt(getProperty("proxy-port"));
                int    prxTimeout = Integer.parseInt(getProperty("proxy-timeout"));
                Object params[] = {host, new Integer(port),
                                   getProperty("proxy-host"),
                                   new Integer(prxPort),
                                   new Long(prxTimeout),
                                   this, getAppName()};
                socket = (Socket)getProxy.invoke(null, params);
            } catch (InvocationTargetException e) {
                throw new IOException("Error in proxy connect: " +
                                      e.getTargetException());
            } catch (Exception e) {
                throw new IOException("Error in proxy connect: " + e);
            }
        } else {
            socket = new Socket(host, port);
        }

        SSH2Transport transport =
            new SSH2Transport(socket, new SSH2Preferences(settings),
                              secureRandom);

        transport.setEventHandler(new SSH2TransportEventAdapter() {
            public boolean kexAuthenticateHost(SSH2Transport tp,
                                               SSH2Signature serverHostKey) {
                return fingerprintMatch(serverHostKey);
            }
            public void gotConnectInfoText(SSH2Transport tp, String text) {
                alert(text);
            }
            public void normalDisconnect(SSH2Transport tp,
                                         String description,
                                         String languageTag) {
                if(autoSave) {
                    try {
                        FileOutputStream file =
                            new FileOutputStream(saveLocation);
                        settings.save(file, getAppName() + " settings");
                    } catch (IOException e) {
                        alert("Error saving settings: " + e.getMessage());
                    }
                }
                disconnect();
            }
            public void fatalDisconnect(SSH2Transport tp, int reason,
                                        String description,
                                        String languageTag) {
                alert("Disconnecting: " + description);
                disconnect();
            }
            public void peerDisconnect(SSH2Transport tp, int reason,
                                       String description,
                                       String languageTag) {
                alert("Server disconnected: " + description);
                disconnect();
            }
        });
        
        if("password".equals(choiceAuthTyp.getSelectedItem())) {
            client = new SSH2SimpleClient(transport,
                                          username,
                                          password);
            password = null;
        } else if("publickey".equals(choiceAuthTyp.getSelectedItem())) {
            /* !!! TODO publickey */
        } else {
            client = new SSH2SimpleClient(transport,
                                          username,
                                          this);
        }

        if(frame != null) {
            frame.setTitle(getAppName() + " " + VERSION + " (connected)");
        }

        if(logo != null) {
            logo.setImage(connImg);
        }

        for(int i = 0; i < modCnt; i++) {
            modules[i].connected(this);
        }
    }

    public void disconnect() {
        if(logo != null) {
            logo.setImage(discImg);
        }

        for(int i = 0; i < modCnt; i++) {
            modules[i].disconnected(this);
        }
        if(frame != null) {
            frame.setTitle(getAppName() + " " + VERSION);
        }
        enableButtons();
    }

    public void initSeedGenerator() {
        RandomSeed seed = getRandomSeed();
        seed.addEntropyGenerator(container);
        if(secureRandom == null) {
            byte[] s = seed.getBytesBlocking(20, false);
            secureRandom = new SecureRandomAndPad(new SecureRandom(s));
        } else {
            int bytes = seed.getAvailableBits() / 8;
            secureRandom.setSeed(seed.getBytesBlocking(bytes > 20 ? 20 : bytes));
        }
        secureRandom.setPadSeed(seed.getBytes(20));
    }

    public boolean fingerprintMatch(SSH2Signature serverHostKey) {
        String fp = settings.getProperty("fingerprint");
        if(fp == null) {
            fp = settings.getProperty("fingerprint." + host + "." + port);
        }
        byte[] blob = null;
        try {
            blob = serverHostKey.getPublicKeyBlob();
        } catch (SSH2SignatureException e) {
            alert(e.toString());
            return false;
        }
        String  fpMD5Hex  = SSH2KeyFingerprint.md5Hex(blob);
        boolean confirmed = false;
        if(fp != null) {
            if(SSH2HostKeyVerifier.compareFingerprints(fp, serverHostKey)) {
                return true;
            }
            if(strictHostKeyCheck()) {
                return false;
            }
            confirmed = confirm("WARNING: HOST IDENTIFICATION HAS CHANGED!\n" +
                                "Do you want to replace existing key with:\n" +
                                "'" + fpMD5Hex + "'", false);
        } else {
            confirmed = confirm("Accept key:\n'" + fpMD5Hex + "'\nfor host " +
                                host + "?", true);
        }
        if(confirmed) {
            setProperty("fingerprint", fpMD5Hex);
            setProperty("fingerprint." + host + "." + port, fpMD5Hex);
        }
        return confirmed;
    }

    boolean strictHostKeyCheck() {
        return Boolean.valueOf(getProperty("strict-hostid")).booleanValue();
    }

    public ByteArrayOutputStream readResource(String name) {
        InputStream in = getClass().getResourceAsStream(name);
        ByteArrayOutputStream baos = null;
        if(in != null) {
            baos = new ByteArrayOutputStream();
            try {
                int c;
                while((c = in.read()) >= 0)
                    baos.write(c);
            } catch(IOException e) {
                // !!!
                System.err.println("ERROR reading resource " + name + " : " + e);
            }
        }
        return baos;
    }

    public Image createImage(String file) {
        ByteArrayOutputStream baos = readResource(file);
        Image  img = null;
        if(baos != null) {
            byte[] raw = baos.toByteArray();
            img = Toolkit.getDefaultToolkit().createImage(raw);
        }
        return img;
    }

    public String promptLine(String prompt, boolean echo)
    throws SSH2UserCancelException {
        String[] answer =
            promptMulti(new String[] { prompt }, new boolean[] { echo });
        ;
        return answer[0];
    }

    public String[] promptMulti(String[] prompts, boolean[] echos)
    throws SSH2UserCancelException {
        return promptMultiFull(getAppName(), null, prompts, echos);
    }

    // !!! OUCH Clean out and move to gui or sshcommon or some such
    static volatile boolean pressedCancel;
    static volatile Dialog  promptDialog;
    public String[] promptMultiFull(String name, String instruction,
                                    String[] prompts, boolean[] echos)
    throws SSH2UserCancelException {
        promptDialog = new Dialog(getParentFrame(), name, true);
        AWTGridBagContainer grid  = new AWTGridBagContainer(promptDialog);

        Label               lbl;
        int                 i;
        TextField[]         fields = new TextField[prompts.length];
        ActionListener      al;

        if(instruction != null) {
            if(instruction.length() > 40) {
                grid.add(new TextArea(instruction, 4, 40,
                                      TextArea.SCROLLBARS_VERTICAL_ONLY), 0, 4);
            } else {
                grid.add(new Label(instruction), 0, 4);
            }
        }

        for(i = 0; i < prompts.length; i++) {
            lbl = new Label(prompts[i]);
            grid.add(lbl, 1 + i, 2);
            fields[i] = new TextField("", 16);
            grid.add(fields[i], 1 + i, 2);
        }

        Button okBut, cancBut;
        Panel bp = new Panel(new FlowLayout());
        bp.add(okBut = new Button("OK"));
        okBut.addActionListener(al = new Actions(-3));

        bp.add(cancBut = new Button("Cancel"));
        cancBut.addActionListener(al);

        grid.add(bp, prompts.length + 2, GridBagConstraints.REMAINDER);

        promptDialog.setResizable(true);
        promptDialog.pack();
        promptDialog.setVisible(true);

        if(pressedCancel) {
            throw new SSH2UserCancelException("User cancel");
        }

        String[] answers = new String[prompts.length];
        for(i = 0; i < answers.length; i++) {
            answers[i] = fields[i].getText();
            fields[i].setText("");
        }

        return answers;
    }

    public int promptList(String name, String instruction, String[] choices)
    throws SSH2UserCancelException {
        promptDialog = new Dialog(getParentFrame(), name, true);
        AWTGridBagContainer grid  = new AWTGridBagContainer(promptDialog);

        ActionListener al;

        if(instruction != null) {
            if(instruction.length() > 40) {
                grid.add(new TextArea(instruction, 4, 40,
                                      TextArea.SCROLLBARS_VERTICAL_ONLY), 0, 4);
            } else {
                grid.add(new Label(instruction), 0, 4);
            }
        }

        Choice choice = new Choice();
        for(int i = 0; i < choices.length; i++) {
            choice.add(choices[i]);
        }
        grid.add(choice, 1, 4);

        Button okBut, cancBut;
        Panel bp = new Panel(new FlowLayout());
        bp.add(okBut = new Button("OK"));
        okBut.addActionListener(al = new Actions(-3));

        bp.add(cancBut = new Button("Cancel"));
        cancBut.addActionListener(al);

        grid.add(bp, 2, GridBagConstraints.REMAINDER);

        promptDialog.setResizable(true);
        promptDialog.pack();
        promptDialog.setVisible(true);

        if(pressedCancel) {
            throw new SSH2UserCancelException("User cancel");
        }

        return choice.getSelectedIndex();
    }

    //
    // MindTermApp interface implementation
    //

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public Properties getProperties() {
        return settings;
    }

    public String getProperty(String name) {
        return settings.getProperty(name);
    }

    public void setProperty(String name, String value) {
        settings.put(name, value);
    }

    public String getUserName() {
        return username;
    }

    public Frame getParentFrame() {
        Frame parent = frame;
        if(parent == null) {
            Component comp = this;
            do {
                comp = comp.getParent();
            } while(!(comp instanceof Frame));
            parent = (Frame)comp;
        }

        return parent;
    }

    public String getAppName() {
        return "MindTermLT";
    }

    public synchronized RandomSeed getRandomSeed() {
        if(randomSeed == null) {
            randomSeed = new RandomSeed("/dev/urandom", "/dev/urandom");
        }
        return randomSeed;
    }

    public SSH2Interactor getInteractor() {
        return this;
    }

    public void alert(String message) {
        GUI.showAlert(getAppName() + " - Alert", message, getParentFrame());
    }

    public boolean confirm(String message, boolean defAnswer) {
        boolean ret = false;
        if(message.length() > 35) {
            boolean scrollbar = (message.length() > 120);
            ret = GUI.showConfirm(getAppName() + " - Confirmation",  message,
                                  3, 40, "Yes", "No", defAnswer,
                                  getParentFrame(), scrollbar, false);
        } else {
            ret = GUI.showConfirm(getAppName() + " - Confirmation",  message,
                                  0, 0, "Yes", "No", defAnswer,
                                  getParentFrame(), false, false);
        }
        return ret;
    }

    public boolean isConnected() {
        return (client != null && client.getTransport() != null &&
                client.getTransport().isConnected());
    }

    public boolean isApplet() {
        return weAreAnApplet;
    }

    public AppletContext getAppletContext() {
        AppletContext ctx = null;
        if(weAreAnApplet) {
            ctx = super.getAppletContext();
        }
        return ctx;
    }

    public SSH2Transport getTransport() {
        return client.getTransport();
    }

    public SSH2Connection getConnection() {
        return client.getConnection();
    }

    public SSHConsoleRemote getConsoleRemote() {
        return new SSH2ConsoleRemote(getConnection());
    }

    //
    // ProxyAuthenticator interface implementation
    //

    public String getProxyUsername(String type, String challenge)
    throws IOException {
        return getProperty("proxy-user");
    }

    public String getProxyPassword(String type, String challenge)
    throws IOException {
        return getProperty("proxy-password");
    }
    
    public Applet getApplet() {
        return this;
    }
}
