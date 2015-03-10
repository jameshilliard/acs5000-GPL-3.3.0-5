package com.mindbright.util;

import javax.swing.JOptionPane;
import com.mindbright.terminal.TerminalOutputListener;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class HandleEMSMessage implements TerminalOutputListener {
    
    private boolean bufferingBP;
    private boolean bufferingMI;
	private StringBuffer buffer;
    private StringBuffer msgBuffer;
    private String message;
    private static Pattern tagMI;
    private static Pattern tagInstance;
    private static Pattern tagProperty;
    private static Pattern regionMI;
    
	public HandleEMSMessage() {
		this.buffer      = new StringBuffer();
		this.message     = null;
		this.bufferingBP = false;
		this.bufferingMI = false;
		this.tagMI       = Pattern.compile("<(.+?)>([^<>]+?)</\\1>");
		this.tagInstance = Pattern.compile("<INSTANCE CLASSNAME=\"(.+?)\">", Pattern.CASE_INSENSITIVE);
		this.tagProperty = Pattern.compile("<PROPERTY NAME=\"(.+?)\".*?><VALUE>(.+?)</VALUE></PROPERTY>", Pattern.CASE_INSENSITIVE);
		this.regionMI    = Pattern.compile("<MACHINE-INFO>(.+?)</MACHINE-INFO>", Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
		
	}

	private int findMI() {
		return findMI(0);
	}
	
	private int findMI(int start) {
		return findMI(start, msgBuffer.length());
	}
	
	private int findMI(int start, int end) {
		Matcher mi = tagMI.matcher(msgBuffer.substring(start, end));
		String g1, g2;
		int r = -1;
		while (mi.find()) {
			g1 = mi.group(1);
			g2 = mi.group(2);
			if (g1 != null && g2 != null) {
				message += g1 + ": " + g2 + "\n";
			}
			r = mi.end();
		}
		return r;
	}

	private int findInstance() {
		return findInstance(0);
	}
	
	private int findInstance(int start) {
		Matcher instance = tagInstance.matcher(msgBuffer.substring(start, msgBuffer.length()));
		int r = -1;
		while (instance.find()) {
			if (instance.group(1) != null) {
				message += "Instance: " + instance.group(1) + "\n";
			}
			r = instance.end();
		}
		return r;
	}
    
	private int findProperty() {
		return findProperty(0);
	}
	
	private int findProperty(int start) {
		Matcher property = tagProperty.matcher(msgBuffer.substring(start, msgBuffer.length()));
		String g1, g2;
		int r = -1;
		while (property.find()) {
			g1 = property.group(1);
			g2 = property.group(2);
			if (g1 != null && g2 != null) {
				message += g1 + ": " + g2 + "\n";
			}
			r = property.end();
		}
		return r;
	}
    
	public void write(char c) {
		int idx;
		boolean stop_bufferingBP = false;
		boolean stop_bufferingMI = false;
		buffer.append(c);
		if (c == '\n') {
			if (!bufferingBP && !bufferingMI) {
				if (bufferingBP = (idx=buffer.toString().toUpperCase().indexOf("<BP>")) != -1) {
					msgBuffer = new StringBuffer();
					msgBuffer.append(buffer.substring(idx));
				} else if (bufferingMI = (idx=buffer.toString().toUpperCase().indexOf("<MACHINE-INFO>")) != -1) {
					msgBuffer = new StringBuffer();
					msgBuffer.append(buffer.substring(idx));
				}
			} else {
				if (bufferingBP && (stop_bufferingBP = (idx=buffer.toString().toUpperCase().indexOf("</BP>")) != -1)) {
					msgBuffer.append(buffer.substring(0, idx + "</BP>".length()));
					bufferingBP = false;
				} else if (bufferingMI && (stop_bufferingMI = (idx=buffer.toString().toUpperCase().indexOf("</MACHINE-INFO>")) != -1)) {
					msgBuffer.append(buffer.substring(0, idx + "</MACHINE-INFO>".length()));
					bufferingMI = false;
				} else {
					msgBuffer.append(buffer);
				}
			}
			buffer.setLength(0);
		}
		if (!bufferingBP && stop_bufferingBP) {
			int next[] = new int[4];
			Matcher mi;
			message = new String("");
			next[0] = 0;
			next[1] = findInstance(next[0]);
			next[1] = (next[1]>=0)? next[1] : next[0];
			message += "\nProperties:\n\n";
			next[2] = findProperty(next[1]);
			next[2] = (next[2]>=0)? next[2] : next[1];
			mi = regionMI.matcher(msgBuffer);
			if (mi.find(next[2])) {
				message += "\nMachine Information:\n\n";
				next[3] = findMI(mi.start(), mi.end());
			}
			if (message.length() > 0) {
				new Thread(new DisplayEMSMessage("Breakpoint", message)).start();
			}
			msgBuffer = null;
			message = null;
		} else if (!bufferingMI && stop_bufferingMI) {
			message = new String("");
			findMI();
			if (message.length() > 0) {
				new Thread(new DisplayEMSMessage("Machine Information", message)).start();
			}
			msgBuffer = null;
			message = null;
		}
	}
}

class DisplayEMSMessage implements Runnable {
    private String title;
    private String message;
    
    public DisplayEMSMessage(String title, String message) {
    	this.title   = new String(title);
    	this.message = new String(message);
    }
    
	public void run() {
		JOptionPane.showMessageDialog(null, message, "EMS Event: " + title, JOptionPane.WARNING_MESSAGE);
	}
    
}

