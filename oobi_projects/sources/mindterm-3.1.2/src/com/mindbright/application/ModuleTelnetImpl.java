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
import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Frame;
import java.awt.Image;
import java.awt.Panel;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Properties;

import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JMenuBar;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.net.telnet.TelnetTerminalAdapter;
import com.mindbright.ssh2.SSH2Channel;
import com.mindbright.ssh2.SSH2InternalChannel;
import com.mindbright.terminal.GlobalClipboard;
import com.mindbright.terminal.LineReaderTerminal;
import com.mindbright.terminal.TerminalFrameTitle;
import com.mindbright.terminal.TerminalMenuHandler;
import com.mindbright.terminal.TerminalMenuHandlerFull;
import com.mindbright.terminal.TerminalWin;
import com.mindbright.terminal.TerminalWindow;
import com.mindterm.util.CYProps;

import com.mindbright.util.HandleEMSMessage;

public class ModuleTelnetImpl extends ModuleBaseTerminal {
    private String                remoteHost;
    private int                   remotePort;
    private SSH2InternalChannel   channel;
    private TelnetTerminalAdapter telnetAdapter;
    private InputStream           telnetIn;
    private OutputStream          telnetOut;
    private LineReaderTerminal    lineReader;
    private boolean               localConnect;
    private Socket                socket;
    private Properties            telnetProps;
    private TerminalWin           terminal;
    private HandleEMSMessage      emsMsg = null;
    
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

    protected class ChaffedTelnet extends TelnetTerminalAdapter {

        public ChaffedTelnet(InputStream in, OutputStream out,
                             TerminalWindow terminalWin) {
            super(in, out, terminalWin);
        }

        public ChaffedTelnet(InputStream in, OutputStream out,
                TerminalWindow terminalWin,
                String username, String password, 
                String phys_port_auth, String typed_input) {
        	super(in, out, terminalWin, username, password, phys_port_auth, typed_input);
        }

        protected void sendFakeChar() {
            if(channel != null) {
                /*
                 * 5 bytes of "ignore-payload" makes packet same size as one
                 * byte channel data (i.e. a key-press). 
                 */
                byte[] chaff = new byte[] { 1, 2, 3, 4, 5 };
                channel.getConnection().getTransport().sendIgnore(chaff);
            }
        }

    }

    public ModuleTelnetImpl() {super();}
    
    public ModuleTelnetImpl(Properties props) {
    	this.telnetProps = props;
    }

    public Frame initJFrame(Applet applet) {
        boolean separateFrame = false;
        Frame frame = null;
        boolean useAWT = false;
        boolean useSwing = true;
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
        } else {
            Component comp = applet;
            do {
                comp = comp.getParent();
            } while(!(comp instanceof Frame));
            frame = (Frame)comp;
            // Force AWT-layout when embedded in a webpage since this
            // class still extends Applet
            useSwing = false;
        }
        return frame;
    }
    
    public void run() {
        
        Applet applet = mindterm.getApplet();
        
        //Frame frame = AWTConvenience.tryToAllocateJFrame();
        Frame frame = initJFrame(applet);
        
        if (frame == null) {
            frame = haveMenus() ? AWTConvenience.newFrameWithMenuBar() : new Frame();
        } /*else {
            if (haveMenus()) {
                try {
                    Class c = Class.forName("com.mindbright.gui.SwingConvenience");
                    java.lang.reflect.Method m = c.getMethod
                        ("newJFrameWithMenuBar", new Class[] {});
                    frame = (Frame)m.invoke(c, (Object[])null);
                    frame.setMenuBar(new MenuBar());
                    frame.validate();
                } catch (Throwable t) {
                }
            }
        }*/
        frame.validate();

        /*TerminalWin*/terminal = new TerminalWin(frame, this.telnetProps!=null ? this.telnetProps
                                                                             : mindterm.getProperties());
        
        if ("enable".equals(applet.getParameter("ems"))) {
        	this.emsMsg = new HandleEMSMessage();
        	terminal.addOutputListener(this.emsMsg);        	
        }
        
        installLogo(this.terminal);
        
        terminal.addAsEntropyGenerator(mindterm.getRandomSeed());

        AWTConvenience.getContentPane(applet/*frame*/).setLayout(new BorderLayout());
        AWTConvenience.getContentPane(applet/*frame*/).add(
            terminal.getPanelWithScrollbar(), BorderLayout.CENTER);
        AWTConvenience.getContentPane(applet/*frame*/).add(
                new JMenuBar(),BorderLayout.NORTH);

        Panel  p = new Panel();
        if("enable".equals(applet.getParameter("ems")))
            terminal.setProperty("backspace-send", "bs", true);
            
        boolean specialKey = false;
        try{
            specialKey = "UTF-8".equals(applet.getParameter("encoding"));
        }catch (Exception e){
            specialKey = false;
        }
        
        //[CYCLADES-START]
        terminal.setSpecialKeysUTF8(specialKey);
        addButtons(p, createActionListener(), applet, "enable".equals(applet.getParameter("ems")));
        //[CYCLADES-END]
        AWTConvenience.getContentPane(applet).add(p, BorderLayout.SOUTH);

        TerminalFrameTitle frameTitle =
            new TerminalFrameTitle(frame, getTitle());
        frameTitle.attach(terminal);

        TerminalMenuHandler tmenus = null;

        if(haveMenus()) {
            try {
                tmenus = getTerminalMenuHandler(frame);
                tmenus.setTitleName(mindterm.getAppName());
                tmenus.addBasicMenus(terminal, applet/*frame*/);
                tmenus.setTerminalMenuListener(this);
            } catch (Throwable t) {
                /* no menus... */
                t.printStackTrace();
            }
        } else {
            terminal.setClipboard(GlobalClipboard.getClipboardHandler());
        }

        frame.addWindowListener(this);

        frame.pack();
        frame.setVisible(true);
        AWTConvenience.getContentPane(applet).setVisible(true);
        p.setVisible(true);
        p.repaint();
        terminal.requestFocus();

        try {
            runTerminal(mindterm, terminal, frame, frameTitle);
        } finally {
            frame.dispose();
            if(haveMenus() && tmenus != null) {
                GlobalClipboard.getClipboardHandler().removeMenuHandler(tmenus);
            }
            instances = null;
            mindterm = null;
            doClose();
        }
    }

    protected TerminalMenuHandler getTerminalMenuHandler(Frame frame) {
        try {
            return TerminalMenuHandlerFull.getInstance(frame);
        } catch (Throwable t) {}
        return null;
    }

    public void runTerminal(MindTermApp mindterm, TerminalWindow terminal,
                            Frame frame, TerminalFrameTitle frameTitle) {
        try {
        	remoteHost = mindterm.getProperty("server");
            remotePort = getPort(mindterm.getProperty("port"));

            localConnect = true;

            if(remoteHost == null) {
                lineReader = new LineReaderTerminal(terminal);
                while (remoteHost == null) {
                    terminal.clearScreen();
                    if(!(mindterm.isConnected() &&
                            (mindterm.getConnection() != null))) {
                        terminal.write("Not connected to ssh2 server, " +
                                       "can only make direct connections.\r\n");
                    } else {
                        terminal.write("Prefix hostname with '|' to make"+
                                       " a tunneled connection.\r\n");
                    }
                    lineReader.print("\r\n");
                    remoteHost = lineReader.promptLine(
                                     "remote host[:port] : ", null, false);
                }
                if(remoteHost.trim().length() == 0) {
                    return;
                }

                if(remoteHost.charAt(0) == '|') {
                    localConnect = false;
                    remoteHost = remoteHost.substring(1, remoteHost.length());
                }

                lineReader.detach();
            }

            int j = remoteHost.indexOf(']');
            int i = remoteHost.indexOf(':', (j!=-1)? j : 0);
            if(i != -1) {
                remotePort = getPort(remoteHost.substring(i + 1));
                remoteHost = remoteHost.substring(0, i);
            }

            if(localConnect) {
                try {
                    socket    = new Socket(remoteHost, remotePort);
                    telnetIn  = socket.getInputStream();
                    telnetOut = socket.getOutputStream();
                } catch (Exception e) {
                    mindterm.alert("Local connection failed: " +
                                   e.getMessage());
                    return;
                }
            } else {
                channel =
                    mindterm.getConnection().newLocalInternalForward(remoteHost,
                            remotePort);

                if(channel.openStatus() != SSH2Channel.STATUS_OPEN) {
                    mindterm.alert("Failed to open tunnel for telnet");
                    return;
                }

                telnetIn  = channel.getInputStream();
                telnetOut = channel.getOutputStream();
            }

            frameTitle.setTitleName("telnet@" + remoteHost + (remotePort != 23 ?
                                    (":" + remotePort)
                                    : ""));

            telnetAdapter = new ChaffedTelnet(telnetIn, telnetOut, terminal,
            		                          telnetProps.getProperty("username"),
            		                          telnetProps.getProperty("password"),
            		                          telnetProps.getProperty(CYProps.PHYS_PORT_AUTH),
            		                          telnetProps.getProperty("typed-input"));

            if(!localConnect && useChaff()) {
                telnetAdapter.startChaff();
            }

            telnetAdapter.getTelnetNVT().getThread().join();
            if(telnetAdapter.isBuffered()) {
                lineReader = new LineReaderTerminal(terminal);
                lineReader.promptLine("\n\rTelnet session was closed, press <return> to close window", null,
                                      false);
            }

            telnetAdapter.stopChaff();

        } catch (Exception e) {
            /* don't care */
        }
    }

    protected boolean closeOnDisconnect() {
        return !localConnect;
    }

    private int getPort(String port) {
        int p;
        try {
            p = Integer.parseInt(port);
        } catch (Exception e) {
            p = 23;
        }
        return p;
    }

    public String getTitle() {
        return mindterm.getAppName() + " - " + "Telnet (not connected)";
    }

    public boolean isAvailable(MindTermApp mindterm) {
        return true;
    }

    protected boolean haveMenus() {
        return Boolean.valueOf(mindterm.getProperty("module.telnet.havemenus")).
               booleanValue();
    }

    public void doClose() {
        if(telnetOut != null) {
            try {
                telnetOut.close();
            } catch (IOException ee) { }
        }
        if(telnetIn != null) {
            try {
                telnetIn.close();
            } catch (IOException ee) { }
        }
        if(channel != null) {
            channel.close();
        }
        if(socket != null) {
            try {
                socket.close();
            } catch (IOException ee) { }
        }
        if(lineReader != null) {
            lineReader.breakPromptLine("");
        }
        if (emsMsg != null) {
        	terminal.removeOutputListener(this.emsMsg);
        }
    }

    protected ModuleBaseTerminal newInstance() {
    	if(this.telnetProps.getProperty("username")!=null){
    		return this;
    	}
    	else{
    		return new ModuleTelnetImpl();
    	}
    }
	
	public boolean installLogo(TerminalWin terminal) {
	    boolean isPresent = false;
	
	    if(terminal != null) {
	        ByteArrayOutputStream baos = readResource("/defaults/logo.gif");
	        if(baos != null) {
	            byte[] img = baos.toByteArray();
	            Image logo = Toolkit.getDefaultToolkit().createImage(img);
	            int width  = -1;
	            int height = -1;
	            boolean ready = false;
	
	            while (!ready) {
	                width  = logo.getWidth(null);
	                height = logo.getHeight(null);
	                if(width != -1 && height != -1) {
	                    ready = true;
	                }
	                Thread.yield();
	            }
	
	            terminal.setLogo(logo, -1, -1, width, height);
	
	            isPresent = true;
	        }
	    }
	
	    return isPresent;
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

    
    private void addButtons(Panel p, ActionListener al, Applet applet, boolean isEMS){
        
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
        
        p.setSize(applet.getWidth(), 285);
        p.setLayout(null);

        applet.setSize(applet.getWidth(), applet.getHeight() + 285);
        
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
                        doClose();
                    }
                    if(ac.toUpperCase().equals("SEND BREAK")) {
                        String breakseq = telnetProps.getProperty(com.mindterm.util.CYProps.BREAK_SEQ);                		
                        if (breakseq==null) {
                        // send Telnet BREAK following RFC spec rules
                        //byte IAC  = (byte) 255; // TELNET COMMAND BYTE
                        //byte CMD_BREAK = (byte) 243; // COMMAND IDENTIFIER : NVT character BRK                			
                        //terminal.sendBytes(new byte[] {IAC, CMD_BREAK});
                        terminal.sendBreak();
                        }
                        else {
                            int lastKey = terminal.getLastKeyTyped();
                            if (lastKey!=13 && lastKey!=10) {                			
                                terminal.sendBytes(new byte[] {13});
                            }
                        }
                        return; 
                    }
                    if(ac.toUpperCase().equals("SERVER")){
                        terminal.sendBytes("id\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("GETTIMEDATE")){
                        terminal.sendBytes("s\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("IP")){
                        terminal.sendBytes("i\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("KERNELLOG")){
                        terminal.sendBytes("d\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("TIMEDATE")){
                        terminal.sendString("s " + jTextField10.getText() + "/" + jTextField2.getText()
                                        + "/" + jTextField3.getText() + " " + jTextField4.getText()
                                        + ":" + jTextField5.getText() + "\r");
                        return;
                    }
                    
                    if(ac.toUpperCase().equals("IPINFO")){
                        terminal.sendString("i " + jTextField6.getText() + " " + jTextField7.getText()
                                        + " " + jTextField8.getText() + " " + jTextField9.getText()
                                        + "\r");
                        return;
                    }
                    if(ac.toUpperCase().equals("CHANNELLIST")){
                        terminal.sendBytes("ch\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("CREATE")){
                        terminal.sendBytes("cmd\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("LOCK")){
                        terminal.sendBytes("lock\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("SWITCH")){
                        terminal.sendBytes(String.valueOf("ch -si " + jTextField1.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("CLOSE")){
                        terminal.sendBytes(String.valueOf("ch -ci " + jTextField11.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("TASKLIST")){
                        terminal.sendBytes("t\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("DETAILONOFF")){
                        terminal.sendBytes("f\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("PAGEONOFF")){
                        terminal.sendBytes("p\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("KILL")){
                        terminal.sendBytes(String.valueOf("k " + jTextField12.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("LOWER")){
                        terminal.sendBytes(String.valueOf("l " + jTextField13.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("RAISE")){
                        terminal.sendBytes(String.valueOf("r " + jTextField14.getText()+ "\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("LIMIT")){
                        terminal.sendBytes(String.valueOf("m " + jTextField15.getText()+ " " + jTextField16.getText() +"\r").getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("RESTART")){
                        terminal.sendBytes("restart\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("SHUTDOWN")){
                        terminal.sendBytes("shutdown\r".getBytes());
                        return;
                    }
                    if(ac.toUpperCase().equals("CRASHDUMP")){
                        terminal.sendBytes("crashdump\r".getBytes());
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