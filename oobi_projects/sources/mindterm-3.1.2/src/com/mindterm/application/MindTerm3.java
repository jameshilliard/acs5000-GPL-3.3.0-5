package com.mindterm.application;

import java.awt.Dimension;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Properties;
import java.util.StringTokenizer;

import com.mindbright.application.MindTerm;
import com.mindbright.gui.AlertDialog;
import com.mindbright.ssh.SSHPropertyHandler;
import com.mindbright.terminal.DisplaySwing;
import com.mindterm.util.CYProps;
import com.mindterm.util.CryptoUtil;
import com.mindterm.util.StringUtil;
import com.mindterm.util.Utils;

import netscape.javascript.JSObject;

/**
 * MindTerm applet powered with specific Cyclades business logic.
 *
 * SESSION TIMEOUT is explained bellow :
 *
 * HttpSession ID is passed to the SSH server by setting it in Mindterm version attribute.
 * This approach is OK since this setting is done very early in the Mindterm startup.
 * Thus be carefull regarding the setMindtermVersion method call - now it is placed
 * in strategic points of the code allowing Applet and Application to work fine.
 *
 * This sessionId is received by SSHD daemon and exported to the SSH_CLIENT environment variable.
 * Then such variable is used inside the security profiles shells to pass it to the Java counter part (SecurityProfilesCommand).
 * Conserver calls apm_conserver.security_profile.sh every minute to check user access.
 *
 * SecurityProfilesCommand returns false ou true verifying if session timed out as well.
 * Depending on column USERS.cliTimeoutEnabled value (0 or 1), APM session timeout can be propagated to Mindterm or not.
 *
 *
 * Applet parameters :
 * 
 * app : contains all parameters and properties restored from database/static file.
 * 		 Warning: These parameters will be moved to paramSSHProps in the methods 
 * 				getAppletParams and getApplicationParams however any code running
 * 				before such calls will not see any parameter provided by 'app'.
 * 				For this reason, parma and parmb were put in separated applet parameters
 * 				(they are accessed before 'app' parse). 
 * 				Applet.getParameter method was overriden to lookup in paramSSHProps.
 *   
 * parma: hash of session-id 
 * parmb: crypted authentication information
 * 
 * @author Eduardo Murai Soares / Cyclades
 * @since October/2005
 */
public class MindTerm3 extends MindTerm
{
    private static final String APPLETID_START="MindTerm3-";

	private final String CLOSE_PAGE = "close.asp";	
	private boolean debugMode;
	
	private boolean ie = false;
	
	private String URL_CHECK_SESSION="/cli-service/checkSessionId?id=$1";
	private String URL_SAVE_USER_PREFS="/cli-service/saveCLIPreferences?uid=$1";

	
	public MindTerm3() {
		super();
		// setting sshhome to ' ' prevents from storing local keys
		// Do not comment this code. 	
		setSshHomeDir(" ");
		getParamSSHProps().setProperty(CYProps.SSH_HOME," ");
		}

	public MindTerm3(Properties sshProps, Properties termProps) {
		super(sshProps, termProps);
		// setting sshhome to ' ' prevents from storing local keys
		// Do not comment this code. 
		setSshHomeDir(" ");
		getParamSSHProps().setProperty(CYProps.SSH_HOME," ");
	}

	private void setDebugMode() {
		String sdebug = getParamSSHProps().getProperty(CYProps.DEBUG_CYCLADES);
		debugMode = sdebug!=null && sdebug.equalsIgnoreCase("true");
	}
	
	public void init() {
		// mindterm version set up with sessionId for Applet
		setMindtermVersion( getParameter(CYProps.PARMA) );	
		super.init();
	}

	/**
	 * Override the default getParameter method.
	 * APM will sent all properties in a single applet parameter called 'app'.
	 * getAppletParams method will parse 'app' parameter into paramSSHProps.
	 * Firstly, the parameter is looked up in paramSSHProps. if not found, it goes 
	 * to Applet.getParameter method.
	 */
	public String getParameter(String pname) {				
		String pvalue = getParamSSHProps().getProperty(pname);
		if (pvalue==null) return super.getParametersFromBaseClass(pname); 
		else return pvalue;	
	}
	
	/*public void actionPerformed(ActionEvent ae) {
		String ac = ae.getActionCommand();
		if(ac.equals("Disconnect")) {
			System.out.println("Pressed the "+ac+" button");
		}
		else if(ac.equals("Send Break")) {
			System.out.println("Pressed the "+ac+" button");
		}
		else {
			System.out.println("Unknown event intercepted");
		}
	}*/
	
	public void initGUI() {
		super.initGUI();
		String wsize = getClient().propsHandler.getProperty(CYProps.WINDOW_SIZE);
		
		// Handle window resizing for Swing window
		if (super.getTerm().getDisplayView() instanceof DisplaySwing) {
		  final DisplaySwing display = (DisplaySwing) super.getTerm().getDisplayView();						
		  int width=0,height=0;
		  
		  if (wsize!=null && !wsize.equals("")) {
			  wsize = wsize.trim();
			  StringTokenizer tklist = new StringTokenizer(wsize,"x",false);
			  String sdim[] = new String[tklist.countTokens()];
			  int s=0;
			  
			  if (tklist.hasMoreTokens()) sdim[s++]= tklist.nextToken();  
			  if (tklist.hasMoreTokens()) sdim[s++]= tklist.nextToken();			  
			
			  if (sdim!=null && sdim.length==2) {
					width = Integer.parseInt(sdim[0],10);
					height = Integer.parseInt(sdim[1],10);
					}
		  		}		  
		   
		  if (isAppletMode()) { 
			  if (width>0 && height>0) 
				  display.setSize(width, height);  
		  }		  
		  else if (!isAppletMode()) {
			    if (width>0 && height>0) {
			    	// restore the window size		
			    	if (debugMode) System.out.println("initGUI, restoring window size: "+wsize);				
					super.getFrame().setSize(width, height);
					}			
		  
			// register window resizing event in order to store the new size as property
			// this property can be saved as user preferences 
			display.addComponentListener(new ComponentAdapter() {
				public void componentResized(ComponentEvent e) {					
					Dimension dim = display.getSize();
					int w = (int) dim.getWidth();
					int h = (int) dim.getHeight();
					getClient().propsHandler.setProperty(CYProps.WINDOW_SIZE, w+"x"+h);
					if (debugMode) System.out.println("DisplaySwing.componentResized: RESIZE HAPPENED, PROPERTY UPDATED: "+
							getClient().propsHandler.getProperty(CYProps.WINDOW_SIZE));
					}
			});			
		  	}
		}		
	}	

	private void setMindtermVersion(String id) {
		// the code bellow does not work well for Applets at second time
		//Version.version = "Cyclades-"+id;

		// WARNING: Do not change the prefix stored in APPLETID_START because it is recognized by
		// APMWEB (SecurityProfilesCommand)
		SSHPropertyHandler.defaultProperties.put("package-version", APPLETID_START+id);
	}

   /**
	 * Validates session-id and retrieve login information from applet parameters.
	 */
	public void getAppletParams() {
		super.getAppletParams();
		setDebugMode();
		try {
		decryptParams();
		
		// this method has no effect here
		// setMindtermVersion( getParameter("parma") );
		}
		catch (Exception e) {
			StringWriter sw = new StringWriter();			
			e.printStackTrace(new PrintWriter(sw));			
			throw new RuntimeException(sw.getBuffer().toString());
		}
	}

	/*public void getApplicationParams() throws Exception {
		super.getApplicationParams();
		setDebugMode();
		decryptParams();
		setMindtermVersion( getParamSSHProps().getProperty(CYProps.PARMA) );
	}*/

	private String[] getTokens(String s, String sep) {
		StringTokenizer tokens = new StringTokenizer(s, sep,false);
		String t;
		String resp[] = new String[tokens.countTokens()];
		int i=0;
		while (tokens.hasMoreTokens())
			resp[i++] = tokens.nextToken();

		return resp;
	}
	
	/**
	 * Decrypt authentication parameters.
	 * @throws UnsupportedEncodingException 
	 */
	public void decryptParams() throws UnsupportedEncodingException  {
        String parma = null,parmb=null,unit=null;
        parma = getParamSSHProps().getProperty(CYProps.PARMA);
		parmb = getParamSSHProps().getProperty(CYProps.PARMB);
		unit =  getParamSSHProps().getProperty(CYProps.UNIT);

		if (debugMode) System.out.println("parma="+parma+", parmb="+parmb);
		// check if sessionid is valid
		/*if ((!unit.equals("acs"))&&(!unit.equals("apm"))) {
			if (parma!=null) checkSessionId(parma);							
			
			if (parmb!=null)
				try {			
				String dec[] = CryptoUtil.getInstance().decryptMindterm(parmb, parma);
				if (debugMode && dec!=null) 
					for (int x=0; x<dec.length; x++) 
						System.out.println("decrypted DATA: #"+dec[x]+"#,");	
				
				if (dec!=null && dec.length>0) {
					String usrinfo ="";
	
					if (dec.length>=3) {
						usrinfo = dec[0] + ":" + dec[2];
						}
					else usrinfo = dec[0];
	
					getParamSSHProps().put("username", usrinfo);
					if (debugMode) System.out.println("decrypted username: "+usrinfo);
					
					if (dec.length>=2) {
						getParamSSHProps().put("password", dec[1]);
						if (debugMode) System.out.println("decrypted password: "+dec[1]);
						}
					
					if (dec.length>=4) {
						getParamSSHProps().put("userid", dec[3]);
						if (debugMode) System.out.println("decrypted userId: "+dec[3]);
						}
					}
				} 
				catch (Exception e) {
					AlertDialog.show("Error","An error ocurred while decrypting parameters. Check the java console for details.", new java.awt.Frame());
					System.out.println("Error when decrypting parma/parmb: ");
					e.printStackTrace();
				}
			}
			else*/ {
				/*if ((unit.equals("apm") && parma != null) && (parmb != null)) {
		           String[] dec = Utils.decrypt(parmb, parma);
		           String consoleChosen = dec[2];
					getParamSSHProps().put("username", dec[0]);
					getParamSSHProps().put("password", dec[1]);
		        }*/

		        if (unit != null && unit.equals("acs")) {
		           boolean noAuthPopup = true;
		           byte[] myByte1 = Utils.hexDecode(parma);
		           parma = new String(myByte1);
		           parma = Utils.myDecrypt(null, parma, true);
		           if(debugMode) System.out.println("Decrypted parma: " + parma); 


		           byte[] myByte2 = Utils.hexDecode(parmb);
		           parmb = new String(myByte2);
		           if(debugMode) System.out.println("Decrypt hex parmb: [" + parmb + "]");
		           String password = Utils.myDecrypt(parma, parmb, false);
		           if(debugMode) System.out.println("Decrypt Password: [" + password + "]");
					getParamSSHProps().put("password", password);
		        }
			}
		}

	/*public String getHTTPProtocol() {
		String flag = getParamSSHProps().getProperty(CYProps.HTTPS_FLAG);
		
		if (flag!=null && flag.trim().equalsIgnoreCase("true")) 
			return "https://";
		else return "http://";						
	}*/
	
	/**
	 * Check if user session is valid, avoiding bogus connection.
	 * 
	 * @param parma Crypted session-id.
	 */
	/*private void checkSessionId(String parma) {
		//AlertDialog.show("Message","checkSessionId", new java.awt.Frame());
		
	      int returnCode = 404;
	      URL url = null;	      
	      try {
	    	  String urlParams = StringUtil.replaceAll(URL_CHECK_SESSION,"$1",parma);
	    	  
	    	  if (debugMode) System.out.println("checkSessionId, JWS server: "+getParamSSHProps().getProperty(CYProps.SERVER));
	    	  url = new URL(getHTTPProtocol()+getParamSSHProps().getProperty(CYProps.SERVER)+urlParams);   

	    	  if (debugMode) System.out.println("checkSessionId, url: "+url.toExternalForm());

	        HttpURLConnection con = (HttpURLConnection)url.openConnection();
	        returnCode = con.getResponseCode();
	        
	        if (debugMode) System.out.println("checkSessionId, returnCode: "+returnCode);
	      } catch (Exception ex) {
	    	  ex.printStackTrace();
	    	  //StringWriter buf = new StringWriter();	    	   
	          //ex.printStackTrace(new PrintWriter(buf));
	          //AlertDialog.show("STACK TRACE",buf.getBuffer().toString(), new java.awt.Frame());
	      }

	      if (returnCode != 200) {
	    	  AlertDialog.show("Connection Failed","This session is invalid. Make sure you are logged in. returnCode="+returnCode, new java.awt.Frame());
	          throw new RuntimeException("Connection Failed");
	      }
	   }*/

		private String getWindowSizeJS() {
			int windowWidth=0, windowHeight=0;
			JSObject window = JSObject.getWindow(this);
			if (window==null) return null;
			
			// try Mozilla
			Object jsobj = window.eval("window.innerWidth");						
			if (jsobj!=null) windowWidth= ((Number) jsobj).intValue();
			else { 
				System.out.println("getWidth-IE ...");
				// try IE
				jsobj = window.eval("document.body.clientWidth");
				if (jsobj==null) return null;
				windowWidth= ((Number) jsobj).intValue();
				}											
			
			// try Mozilla
			jsobj = window.eval("window.innerHeight");						
			if (jsobj!=null) windowHeight= ((Number) jsobj).intValue();
			else {
				System.out.println("getHeight-IE ...");
				// try IE
				jsobj = window.eval("document.body.clientHeight");			
				
				if (jsobj==null) return null;
				else windowHeight= ((Number) jsobj).intValue();																			
				}
			
			System.out.println("window-size: "+windowWidth+"x"+windowHeight);
			return (windowWidth+"x"+windowHeight);
		}
		
		/**
		 * Handle window size for Applet.
		 * 
		 * @param propsToSave
		 */
		private void handleWindowSizeForApplet(Properties propsToSave) {
			try {
				// capture current window size
				String wsize = getWindowSizeJS();
				
				if (wsize!=null) propsToSave.setProperty(CYProps.WINDOW_SIZE, wsize);
				else System.out.println("Warning: it was not possible to capture browser window size- JS problems");
				
				} catch (Exception ex) {
					System.out.println("Warning: It was not possible to capture browser window size - ");
					ex.printStackTrace();
				}
		}
		/**
		 * Save mindterm preferences into APM.
		 */
		/*public void savePreferences(Properties propsToSave) {
			int returnCode=404;
			String msg=null;
			String[] excludeParams={"parma","parmb","userid","password","username","server","debug"};
			try {
			if (isAppletMode()) handleWindowSizeForApplet(propsToSave);
			
			if (debugMode) System.out.println("Saving window-size: "+propsToSave.getProperty(CYProps.WINDOW_SIZE));
			//------------------------------------------------------------------
			// get parameters	
			Enumeration keys = propsToSave.keys();
			String k,v;
			int i=0;
			StringBuffer params = new StringBuffer();
			
			PARAMS_LOOP: 
			while (keys.hasMoreElements()) {
				k = keys.nextElement().toString();
				
				if (excludeParams!=null)
					for (i=0; i<excludeParams.length; i++) 
						if (excludeParams[i].equals(k)) continue PARAMS_LOOP;				
				
				v = propsToSave.getProperty(k);
				params.append(k);
				params.append("=");
				params.append(URLEncoder.encode(v)); //,"utf-8"));
				params.append("&");
				}
			
			// get the userID
			String userid = (String) getParamSSHProps().get("userid");
			if (userid==null || Integer.parseInt(userid,10)<=0) 
				throw new InvalidUserException("Invalid userid sent by APM : "+userid);
				
			// call the service URL to save the data into database
			URL url = new URL(getHTTPProtocol()+getParamSSHProps().getProperty(CYProps.SERVER)+
					StringUtil.replaceAll(URL_SAVE_USER_PREFS,"$1",userid)+"&"+params.toString());      	  	

			if (debugMode) System.out.println("savePreferences, url: "+url.toExternalForm());

			HttpURLConnection con = (HttpURLConnection)url.openConnection();
			returnCode = con.getResponseCode();			
			msg = con.getResponseMessage();
			
			if (debugMode) System.out.println("savePreferences, returnCode: "+returnCode+", msg: "+msg);
			} 
			catch (InvalidUserException e) {
				throw e;
			}
			catch (Exception e) {
				System.out.println("Error in MindTerm3.savePreferences");
				e.printStackTrace();
			}
			
			 if (returnCode != 200) 
		          throw new RuntimeException("Failed to save preferences in APM");		      	
			 else AlertDialog.show("Message","Preferences has been saved successfully!", new java.awt.Frame());
		}*/
	/**
	 * Release allocated resources
	 */
	public void destroy() {
		// original destroy method would close all windows :
		// When you start two applets by clicking twice on the CLI button in the webpage
		// if you exit one applet, the second would be killed as well.
		// Calling close method prevents from such behavior
		setConfirmedClose(true);
		super.close();
	}
	
	/**
	 * Method called by Mindterm.close().
	 * It checks if is necessary to call the regular close method or close the html page in case of
	 * main Applet window.
	 */	
	public void checkAppletWindow(MindTerm mt) { 
		if (mt.isAppletMode() && mt.isMainWindow()) 
			try {

				mt.getParamSSHProps().clear();
			
				//	Do not close browser window through JSObject
				// because it leads Firefox to hang
				// OBS: In case you need to use the netscape library, put ${java.home}/lib/plugin.jar in the CLASSPATH
				JSObject window = JSObject.getWindow(this);
				
				if (!(window instanceof netscape.javascript.JSObject)) {
					window.eval("self.close();");
				} else if (mt.getAppletContext()!=null) {
					mt.getAppletContext().showDocument(new URL(mt.getCodeBase(), CLOSE_PAGE));
				}
				

			} catch (Exception e) {
				e.printStackTrace();
				try { 
					if (mt.getAppletContext()!=null)
						mt.getAppletContext().showDocument(new URL(mt.getCodeBase(), CLOSE_PAGE));
				} catch (Exception ee) {
					ee.printStackTrace();
				}
			}
	}
	
	public void cloneWindow() {
        MindTerm3 mindterm = new MindTerm3(getSshProps(), getTermProps());
        mindterm.initParams(this);
        mindterm.setupClone( getClient() );

        // cloned window is child of the main window
        mindterm.setMainWindow(false);
        (new Thread(mindterm, "MindTerm.clone")).start();
    }

    public void newWindow() {
    	MindTerm3 mindterm = new MindTerm3(getParamSSHProps(), getParamTermProps());
        mindterm.initParams(this);

        // new window is child of the main window
        mindterm.setMainWindow(false);
        (new Thread(mindterm, "MindTerm.window")).start();
    }

	 /*public static void main(String[] argv) {
	        MindTerm3 controller    = new MindTerm3(getParamSSHProps(), getParamTermProps());
	        controller.cmdLineArgs = argv;        
	        try {
	            controller.getApplicationParams();
	            
	            //System.out.println("---- PARAMETERS ----");
                //Enumeration keys = controller.getParamSSHProps().keys();
                //String k,v;
                //while (keys.hasMoreElements()) {
                //    k = keys.nextElement().toString();
                //    v = controller.getParamSSHProps().getProperty(k);
                //    System.out.println(k+"=."+v+".");
                //}
                            	            
	        } catch (Exception e) {
	            System.out.println("Error: ");
	            e.printStackTrace();
	            System.exit(1);
	        }

	        try {
	            controller.run();
	        } catch (Exception e) {
	            System.out.println("Internal error running controller");
	            e.printStackTrace();
	        }
	    }*/


}
