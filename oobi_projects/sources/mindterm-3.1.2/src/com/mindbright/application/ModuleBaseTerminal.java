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

import java.awt.BorderLayout;
import java.awt.Frame;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.util.Enumeration;
import java.util.Vector;

import com.mindbright.gui.AWTConvenience;
import com.mindbright.terminal.GlobalClipboard;
import com.mindbright.terminal.TerminalFrameTitle;
import com.mindbright.terminal.TerminalMenuHandler;
import com.mindbright.terminal.TerminalMenuHandlerFull;
import com.mindbright.terminal.TerminalMenuListener;
import com.mindbright.terminal.TerminalWin;
import com.mindbright.terminal.TerminalWindow;

public abstract class ModuleBaseTerminal extends WindowAdapter
    implements MindTermModule, Runnable, TerminalMenuListener {

    protected MindTermApp mindterm;
    protected Thread      myThread;

    protected Vector instances;

    public void init(MindTermApp mindterm) {
        this.mindterm = mindterm;
    }

    protected boolean haveMenus() {
        return Boolean.valueOf(mindterm.getProperty("havemenus")).
            booleanValue();
    }

    protected boolean useChaff() {
        return Boolean.valueOf(mindterm.getProperty("key-timing-noise")).
               booleanValue();
    }

    public void activate(MindTermApp mindterm) {
        if(instances == null) {
            instances = new Vector();
        }
        ModuleBaseTerminal bt = newInstance();
        instances.addElement(bt);
        bt.init(mindterm);
        Thread t = new Thread(bt, "Terminal_" + this.getClass());
        t.start();
        try {
            t.join();
        } catch (InterruptedException ex) {
            ex.printStackTrace();
            bt.doClose();
        }
    }

    public void run() {
        Frame frame = AWTConvenience.tryToAllocateJFrame();

        if (frame == null) {
            frame = haveMenus() ? AWTConvenience.newFrameWithMenuBar() : new Frame();
        } else {
            if (haveMenus()) {
                try {
                    Class c = Class.forName("com.mindbright.gui.SwingConvenience");
                    java.lang.reflect.Method m = c.getMethod
                        ("newJFrameWithMenuBar", new Class[] {});
                    frame = (Frame)m.invoke(c, (Object[])null);
                } catch (Throwable t) {
                }
            }
        }

        TerminalWin terminal = new TerminalWin(frame,
                                               mindterm.getProperties());

        terminal.addAsEntropyGenerator(mindterm.getRandomSeed());

        AWTConvenience.getContentPane(frame).setLayout(new BorderLayout());
        AWTConvenience.getContentPane(frame).add(
            terminal.getPanelWithScrollbar(), BorderLayout.CENTER);

        TerminalFrameTitle frameTitle =
            new TerminalFrameTitle(frame, getTitle());
        frameTitle.attach(terminal);

        TerminalMenuHandler tmenus = null;

        if(haveMenus()) {
            try {
                tmenus = getTerminalMenuHandler(frame);
                tmenus.setTitleName(mindterm.getAppName());
                tmenus.addBasicMenus(terminal, frame);
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

        try {
            runTerminal(mindterm, terminal, frame, frameTitle);
        } finally {
            frame.dispose();
            if(haveMenus() && tmenus != null) {
                GlobalClipboard.getClipboardHandler().removeMenuHandler(tmenus);
            }
            instances = null;
            mindterm = null;
        }
    }

    protected TerminalMenuHandler getTerminalMenuHandler(Frame frame) {
        try {
            return TerminalMenuHandlerFull.getInstance(frame);
        } catch (Throwable t) {}
        return null;
    }

    public void connected(MindTermApp mindterm) {}

    public void disconnected(MindTermApp mindterm) {
        if(instances != null) {
            Enumeration e = instances.elements();
            while(e.hasMoreElements()) {
                ModuleBaseTerminal bt = (ModuleBaseTerminal)e.nextElement();
                if(bt.closeOnDisconnect()) {
                    bt.doClose();
                    instances.removeElement(bt);
                }
            }
        }
    }

    public String description(MindTermApp mindterm) {
        return null;
    }

    public void windowClosing(WindowEvent e) {
        doClose();
    }

    public void close(TerminalMenuHandler originMenu) {
        doClose();
    }

    public void update() {}

    protected abstract void runTerminal(MindTermApp mindterm,
                                        TerminalWindow terminal, Frame frame,
                                        TerminalFrameTitle frameTitle);
    protected abstract boolean closeOnDisconnect();
    protected abstract String getTitle();
    protected abstract void doClose();
    protected abstract ModuleBaseTerminal newInstance();

}
