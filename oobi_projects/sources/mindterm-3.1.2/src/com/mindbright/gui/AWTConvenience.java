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

package com.mindbright.gui;

import java.awt.Button;
import java.awt.Choice;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.List;
import java.awt.MenuBar;
import java.awt.MenuComponent;
import java.awt.MenuItem;
import java.awt.Panel;
import java.awt.SystemColor;
import java.awt.TextField;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.lang.reflect.Method;

import javax.swing.JFrame;

public abstract class AWTConvenience {

    public static class CloseAction implements ActionListener {
        Dialog dialog;
        public CloseAction(Dialog dialog) {
            this.dialog = dialog;
        }
        public void actionPerformed(ActionEvent e) {
            dialog.setVisible(false);
            dialog.dispose();
        }
    }

    public static class CloseAdapter extends WindowAdapter {
        Object source;
        String action;
        public CloseAdapter(MenuItem mi) {
            this(mi, mi.getLabel());
        }
        public CloseAdapter(Button b) {
            this(b, b.getActionCommand());
        }
        private CloseAdapter(Object source, String action) {
            this.source = source;
            this.action = action;
        }
        public void windowClosing(WindowEvent e) {
            if(source instanceof Component) {
                ((Component)source).dispatchEvent(new ActionEvent(source,
                                                  ActionEvent.ACTION_PERFORMED,
                                                  action));
            } else if(source instanceof MenuComponent) {
                ((MenuComponent)source).dispatchEvent(new ActionEvent(source,
                                                      ActionEvent.ACTION_PERFORMED,
                                                      action));
            }
        }
    }

    public static class OKCancelAdapter extends KeyAdapter {

        protected static boolean isMRJ = false;

        static {
            try { // see <http://developer.apple.com/qa/java/java17.html>
                isMRJ = (System.getProperty("mrj.version") != null);
            } catch (Throwable e) {
                // applets may not be able to do this
            }
        }

        Button butOK;
        Button butCancel;

        public OKCancelAdapter(Button ok, Button cancel) {
            this.butOK = ok;
            this.butCancel = cancel;
        }

        protected void pushButton(Button target) {
            if (isMRJ) { // see <http://developer.apple.com/qa/java/java01.html>
                target.dispatchEvent(new KeyEvent(target, KeyEvent.KEY_PRESSED,
                                                  System.currentTimeMillis(), 0, KeyEvent.VK_ENTER,
                                                  (char)KeyEvent.VK_ENTER));
            } else { // still can work, just no visual feedback
                target.dispatchEvent(new ActionEvent(target, ActionEvent.ACTION_PERFORMED,
                                                     target.getActionCommand()));
            }
        }

        public void keyReleased(KeyEvent e) {
            switch(e.getKeyCode()) {
            case KeyEvent.VK_ENTER :
                if (butOK != null)
                    pushButton(butOK);
                break;
            case KeyEvent.VK_ESCAPE :
                if (butCancel != null)
                    pushButton(butCancel);
                break;
            }
        }
    }

    public static Panel newButtonPanel(Component[] b) {
        Panel p1 = new Panel(new FlowLayout(FlowLayout.RIGHT));
        Panel p2 = new Panel(new GridLayout(1, 0, 5, 0));
        p1.add(p2);
        for (int i=0; i<b.length; i++)
            p2.add(b[i]);
        return p1;
    }

    public final static void placeDialog(Container c) {
        Dimension sDim = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension mDim = c.getSize();
        int x, y;
        x = ((sDim.width / 2) - (mDim.width / 2));
        y = ((sDim.height / 2) - (mDim.height / 2));
        c.setLocation(x, y);
    }

    public final static void setBackgroundOfChildren(Container container) {
        Component[] children = container.getComponents();
        container.setBackground(SystemColor.menu);
        for(int i = 0; i < children.length; i++) {
            if(children[i] instanceof Choice)
                continue;
            children[i].setBackground(SystemColor.menu);

            if(children[i] instanceof Container) {
                setBackgroundOfChildren((Container)children[i]);
            } else if(children[i] instanceof Choice) {
                continue;
            } else if(children[i] instanceof TextField || children[i] instanceof List) {
                children[i].setBackground(SystemColor.text);
            } else {
                children[i].setBackground(SystemColor.menu);
            }
        }
    }

    public final static void setKeyListenerOfChildren(Container container, KeyListener listener,
            Class typeOfChild) {
        Component[] children = container.getComponents();
        for(int i = 0; i < children.length; i++) {
            if(children[i] instanceof Choice)
                continue;
            if(children[i] instanceof Container) {
                setKeyListenerOfChildren((Container)children[i], listener, typeOfChild);
            } else if(children[i] != null && (typeOfChild == null ||
                                              typeOfChild.isInstance(children[i]))) {
                children[i].addKeyListener(listener);
            }
        }
    }

    public final static Choice newChoice(String[] opts) {
        Choice c = new Choice();
        for(int i = 0; i < opts.length; i++)
            c.add(opts[i]);
        return c;
    }

    public final static Frame newFrameWithMenuBar() {
        Frame frame = new Frame();
        frame.setMenuBar(new MenuBar());
        frame.validate();
        return frame;
    }

    public final static boolean isSwingJFrame(Frame f) {
		// [CYCLADES-START]
        	// force-swing added by Eduardo Murai Soares
    		String forceSwing = com.mindterm.application.MindTerm3.getParamSSHProps().getProperty(com.mindterm.util.CYProps.FORCE_SWING);

    		if (forceSwing!=null && forceSwing.equalsIgnoreCase("true"))
        		try {
        		Class.forName("javax.swing.JFrame").newInstance();
        		return true;
        		} catch (Exception e) {
        			// Swing recognition failed!
        		}
        	//}
        	// [CYCLADES-END]
        	//---------------------------------------------------------	
        try {			
            Method m = f.getClass().getMethod("getJMenuBar", (Class[])null);
            return m != null;
        } catch (Throwable t) {
        }
        return false;
    }

    public final static Frame tryToAllocateJFrame() {
        try {
        	// [CYCLADES-START]
        	// force-swing added by Eduardo Murai Soares
        	String deflaf = null; // moved
    		String forceSwing = com.mindterm.application.MindTerm3.getParamSSHProps().getProperty(com.mindterm.util.CYProps.FORCE_SWING);

    		if (forceSwing!=null && forceSwing.equalsIgnoreCase("false")) {                		
            //String deflaf = null;
    			// [CYCLADES-END]	
            try {
                String p = System.getProperty("useAWT");
                if (p != null && Boolean.valueOf(p).booleanValue())
                    return null;
                deflaf = System.getProperty("swing.defaultlaf");
            } catch (Throwable tt) {
            }
            // [CYCLADES-START]
    		} 
            // [CYCLADES-END]
            try {
                Class c = Class.forName("com.mindbright.gui.GUISwing");
            } catch (Throwable tt) {
                return null;
            }

            Method m;
            
            if (deflaf == null) {
                try {
                    Class c = Class.forName("javax.swing.UIManager");
                    m = c.getMethod("getSystemLookAndFeelClassName", 
                                    new Class[] {} );
                    deflaf = (String)m.invoke(c, (Object[])null);
                } catch (Throwable tt) {
                }
            }
            
            if (deflaf != null) {
                try {
                    Class c = Class.forName("javax.swing.UIManager");
                    m = c.getMethod("setLookAndFeel", new Class[] {String.class} );
                    m.invoke(c, new Object[] { deflaf });
                } catch (Throwable tt) {
                }
            }
            
            // [CYCLADES-START] : set default close operation to NOTHING
            //return (Frame)Class.forName("javax.swing.JFrame").newInstance();            
            JFrame fr = (JFrame) Class.forName("javax.swing.JFrame").newInstance();
            fr.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);
            return fr;
            // [CYCLADES-STOP]
        } catch (Throwable t) {
        }
        return null;
    }


    public final static Container getContentPane(Container cont) {
        if (! (cont instanceof Frame)) 
            return cont;

        Frame f = (Frame)cont;
        if (isSwingJFrame(f)) {
            try {
                Class c = Class.forName("javax.swing.JFrame");
                Method m = c.getMethod("getContentPane", new Class[] {} );
                return (Container)m.invoke(f, (Object[])null);
            } catch (Throwable tt) {
            }
        }
        return f;
    }
}
