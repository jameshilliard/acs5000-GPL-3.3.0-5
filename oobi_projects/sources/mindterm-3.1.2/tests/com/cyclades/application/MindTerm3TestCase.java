package com.mindterm.application;

import java.net.URLEncoder;
import java.util.Properties;

import com.mindbright.application.MindTerm;

public class MindTerm3TestCase {
	private MindTerm3 cyApp = new MindTerm3();
	
	private Runnable task = new Runnable() {					

		public void run() {
			try {
			testApplet();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	};
	
	public void testConcurrentApplets() throws Exception {
		Thread thApplet1 = new Thread(task);
		Thread thApplet2 = new Thread(task);
		
		thApplet1.start();
		thApplet2.start();
	}
	
	public void testApplet() throws Exception {
		MindTerm3 cyLocalApp = new MindTerm3();
		AppletRunner appRunner = new AppletRunner();
		String server = "172.20.0.87";
		Properties params = new Properties();
		params.setProperty("app","exit-on-logout=true#module6=com.mindbright.application.ModuleTerminal#breakseq=%7E%2Cb%2Cr%2Ce%2Ca%2Ck#module5=com.mindbright.application.ModuleTelnetProxy#module4=com.mindbright.application.ModuleSocksProxy#compression=0#module3=com.mindbright.application.ModuleFTPOverSFTP#auto-linefeed=false#module2=com.mindbright.application.ModuleSCP#module1=com.mindbright.application.ModuleSFTP#module0=com.mindbright.application.ModuleTelnet#module.terminal.havemenus=true#scrollbar=right#ssh1-cipher=blowfish-cbc#mtu=0#line-space-delta=0#lockOutlet=%235%2Cc%2CO%2Cl#select-delim=%22+%22#repos-output=true#font-name=Dialog#debug=false#server="+server+
				"#repos-input=true#module2.label=SCP+File+Transfer...#sftpbridge-host=#copy-crnl=false#allow-new-server=true#x11-display=127.0.0.1%3A0#comp-algorithms-srv2cli=none#visible-cursor=true#term-type=xterm#portftp=false#module.telnet.havemenus=true#local-bind=127.0.0.1#auth-method=password%2Ckbd-interact#local-pgkeys=false#debug-mindterm=true#backspace-send=del#port=22#module1.label=SFTP+File+Transfer...#force-swing=true#save-lines=512#private-key=identity#rev-autowrap=false#powerOff=%235%2Cc%2CO%2Cf%2Cf#paste-button=middle#module.telnet.inhibit=false#mac-algorithms-cli2srv=hmac-md5%2Chmac-sha1%2Chmac-sha1-96%2Chmac-md5-96%2Chmac-ripemd160#strict-hostid=false#socksproxy-host=#window-size=581x399#quiet=true#languages-cli2srv=#filelist-remote-command=ls+-A+-L+-F+-1%0A#local-echo=false#enc-algorithms-cli2srv=aes128-ctr%2Caes128-cbc%2Caes192-ctr%2Caes192-cbc%2Caes256-ctr%2Caes256-cbc%2Cblowfish-ctr%2Cblowfish-cbc%2Ctwofish1#mac-algorithms-srv2cli=hmac-md5%2Chmac-sha1%2Chmac-sha1-96%2Chmac-md5-96%2Chmac-ripemd160#copy-select=true#remfwd=false#alive=10#unlockOutlet=%235%2Cc%2CO%2Cu#fg-color=green#sftpbridge-port=#module5.label=Telnet+Proxy...#module0.label=Telnet+Terminal#insert-mode=false#protocol=auto#powerOn=%235%2Cc%2CO%2Cn#languages-srv2cli=#sepframe=false#server-host-key-algorithms=ssh-rsa%2Cssh-dss#enc-algorithms-srv2cli=aes128-ctr%2Caes128-cbc%2Caes192-ctr%2Caes192-cbc%2Caes256-ctr%2Caes256-cbc%2Cblowfish-ctr%2Cblowfish-cbc%2Ctwofish1#map-ctrl-space=true#key-timing-noise=false#auto-username=false#bg-color=i_black#delete-send=bs#ascii-line=false#geometry=54x22#autowrap=true#socksproxy-port=#input-charset=none#cursor-color=i_blue#module4.label=SOCKS+Proxy...#powerStatus=%235%2Cc%2CO%2Cs#x11-forward=false#powerCycle=%235%2Cc%2CO%2Cc#force-pty=true#proxy-type=none#rev-video=false#font-size=12#visual-bell=false#module3.label=FTP+To+SFTP+Bridge...#passthru-prn-enable=true#kex-algorithms=diffie-hellman-group1-sha1%2Cdiffie-hellman-group14-sha1%2Cdiffie-hellman-group-exchange-sha1#comp-algorithms-cli2srv=none#jar-path=.#encoding=iso-8859-1");
		
		params.setProperty("parma","27258cf4fbcb93a34eba20b7d7d13b9f2f94fa34");
		params.setProperty("parmb","7FIf/WIiGDvMOMb0FhFTyXhneJsG65qdfUNMFqlT1xDeor4UYURcsYZntDF/aocL9811NxQHJHPpi6YANVipxVtVtcxrODN+pR1+RwhuCEE=");
		
		appRunner.execute(cyLocalApp, server,server,564,364,params);
	}
	
	public void testConnectionAPM() throws Exception {
		cyApp.getParamSSHProps().setProperty("userid","1");
		cyApp.getParamSSHProps().setProperty("server","172.22.78.2");
		//cyApp.getParamSSHProps().setProperty("debug","true");
		cyApp.getParamSSHProps().setProperty("username","admin:acs-32-sup_02");
		cyApp.getParamSSHProps().setProperty("password","eduardo");
		
		cyApp.main(null);
	}
	
	public void testOutletShortcuts() throws Exception {
        
        String[] property = new String[] { "lockOutlet", "unlockOutlet", "powerOn", "powerOff", "powerCycle", 
                "powerStatus", "breakseq" };
        
        // the '#' character identifies the string that already is a byte value.
        String[] value = new String[] { "#5,c,O,l", "#5,c,O,u", "#5,c,O,n", "#5,c,O,f,f", "#5,c,O,c", 
                "#5,c,O,s", "~,b,r,e,a,k" };

        String arg = "";
        for (int i = 0; i < property.length; i++) {
            arg += property[i] + "=" + URLEncoder.encode(value[i],"utf-8") + "&";    
        }
        arg += "encoding=utf-8";
        
		cyApp.main(new String[]{"--app",arg});
	}

    public void testPropertiesLoading() throws Exception {
    	String parmb="";
    	String parma="";
    	String arg = "parma="+parma+"#parmb="+parmb; //+"#breakseq=%7E%2Cb%2Cr%2Ce%2Ca%2Ck#line-space-delta=0#window-size=607x425#local-echo=false#alive=10#sepframe=false#rev-video=false#powerOff=%235%2Cc%2CO%2Cf%2Cf#font-size=12#autowrap=true#delete-send=bs#ascii-line=false#local-pgkeys=false#font-name=Monospaced#powerOn=%235%2Cc%2CO%2Cn#visual-bell=false#geometry=80x24#backspace-send=del#paste-button=middle#rev-autowrap=false#passthru-prn-enable=true#lockOutlet=%235%2Cc%2CO%2Cl#copy-select=true#debug=false#unlockOutlet=%235%2Cc%2CO%2Cu#powerCycle=%235%2Cc%2CO%2Cc#save-lines=512#term-type=xterm-color#insert-mode=false#force-swing=true#server=172.22.78.2#copy-crnl=false#bg-color=white#input-charset=none#port=22#repos-output=true#repos-input=true#exit-on-logout=true#quiet=true#map-ctrl-space=true#encoding=utf-8#cursor-color=i_blue#debug-mindterm=true#scrollbar=right#auto-linefeed=false#powerStatus=%235%2Cc%2CO%2Cs#select-delim=%22+%22#visible-cursor=true#fg-color=black";
        cyApp.main(new String[]{"--q","true","--e","--app",arg});
    }

	public void testSavePreferences() throws Exception {				
		cyApp.getParamSSHProps().setProperty("userid","1");
		cyApp.getParamSSHProps().setProperty("server","172.22.78.2");
		cyApp.getParamSSHProps().setProperty("debug","true");
		cyApp.getParamSSHProps().setProperty("username","admin:ACS_05");
		cyApp.getParamSSHProps().setProperty("password","apm");
		
		Properties props = new Properties();
		props.setProperty("test-one","one");
		props.setProperty("test-two","two");
		props.setProperty("test-three","three");
		props.setProperty("test-four","four");
		
		cyApp.getApplicationParams();
		cyApp.savePreferences(props);
	}
	
	public void testMindTerm() throws Exception {
		MindTerm mind = new MindTerm();				
		mind.main(new String[]{"--q","true","--e"});
	}
	
	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception {
		MindTerm3TestCase testCase = new MindTerm3TestCase();		
		//testCase.testSavePreferences();		
		//testCase.testConnectionAPM();
		//testCase.testPropertiesLoading();
		testCase.testApplet();		
		//testCase.testConcurrentApplets();		
		//testCase.testMindTerm();
		
		System.out.println("the end...");
	}

}
