package com.mindterm.application;

import java.applet.Applet;
import java.applet.AppletContext;
import java.applet.AppletStub;
import java.applet.AudioClip;
import java.awt.AWTEvent;
import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Properties;

import javax.swing.JFrame;
import javax.swing.UIManager;


public class AppletRunner {
	private Properties params = new Properties();
	private JFrame frame = new JFrame();
	private Applet applet;
	private String codebase="localhost";
	private String documentbase="localhost";
	
	class MockFrame extends JFrame {
		public void test() {
			
		}
	}
	
	class MockAppletStub implements AppletStub {		
		
		public MockAppletStub() {
		}
		
		public void appletResize(int width, int height) {
		}

		public AppletContext getAppletContext() {
			return new MockAppletContext();
		}

		public URL getCodeBase() {
			try {						
			URL u = new URL("http://"+codebase);
			return u;
			} catch (Exception e) {
				throw new RuntimeException(e.toString());
			}
		}

		public URL getDocumentBase() {
			try {						
				URL u = new URL("http://"+documentbase);
				return u;
				} catch (Exception e) {
					throw new RuntimeException(e.toString());
				}
		}

		public String getParameter(String name) {
			return params.getProperty(name);
		}

		public boolean isActive() {		
			return applet.isActive();			
		}		
	}
	//-------------------------------------------------------------------
	class MockAppletContext implements AppletContext {

		public Applet getApplet(String name) {
			// TODO Auto-generated method stub
			return null;
		}

		public Enumeration getApplets() {
			// TODO Auto-generated method stub
			return null;
		}

		public AudioClip getAudioClip(URL url) {
			// TODO Auto-generated method stub
			return null;
		}

		public Image getImage(URL url) {
			// TODO Auto-generated method stub
			return null;
		}

		public InputStream getStream(String key) {
			// TODO Auto-generated method stub
			return null;
		}

		public Iterator getStreamKeys() {
			// TODO Auto-generated method stub
			return null;
		}

		public void setStream(String key, InputStream stream) throws IOException {
			// TODO Auto-generated method stub
			
		}

		public void showDocument(URL url, String target) {
			System.out.println("AppletContext.showDocument: "+url);
			//WindowEvent e = new WindowEvent(frame, WindowEvent.WINDOW_CLOSING);
			//frame.dispatchEvent(e);			
		}

		public void showDocument(URL url) {
			System.out.println("AppletContext.showDocument: "+url);
			//WindowEvent e = new WindowEvent(frame, WindowEvent.WINDOW_CLOSING);
			//frame.dispatchEvent(e);			
		}

		public void showStatus(String status) {
			System.out.println("AppletContext.showStatus: "+status);
		}
		
	}
//	-------------------------------------------------------------------
    public AppletRunner() {        
    }

    public void execute(final Applet pApplet, 	String pCodebase,
    		String pDocumentbase, int width, int height, Properties pParams) {
    	this.codebase = pCodebase;
    	this.documentbase = pDocumentbase;
    	this.applet = pApplet;
    	
    	if (pParams!=null) this.params = pParams;    	
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        } catch (Exception e) {
            e.printStackTrace();
        }        
        
        frame.getContentPane().setLayout(new BorderLayout());
        frame.setSize( new Dimension(width, height) );
        frame.setTitle( "Applet Runner" );
        
        Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension frameSize = frame.getSize();
        if (frameSize.height > screenSize.height) {
            frameSize.height = screenSize.height;
        }
        if (frameSize.width > screenSize.width) {
            frameSize.width = screenSize.width;
        	}
        frame.setLocation( ( screenSize.width - frameSize.width ) / 2, ( screenSize.height - frameSize.height ) / 2 );
        frame.setDefaultCloseOperation( JFrame.EXIT_ON_CLOSE );
        frame.setVisible(true);
        frame.addWindowListener(new WindowAdapter() {

			public void windowClosing(WindowEvent e) {
				applet.stop();
				applet.destroy();
				super.windowClosing(e);
			}
        	
        });
        frame.getContentPane().add(applet, BorderLayout.CENTER);
        
        applet.setStub(new MockAppletStub());
        applet.init();
        applet.start();
    }    
}
