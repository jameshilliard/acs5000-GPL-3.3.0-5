package com.mindterm.util;

import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.Enumeration;
import java.util.Properties;
import java.util.StringTokenizer;

/**
 * Data formatter class.
 * 
 * @author Eduardo Murai Soares
 */
public class Formatter {
	/**
	 * DO NOT use '&' as delimiter. It has a issue in Windows which will interpret 
	 * any string with '&copy' and convert to another char. 
	 */
	public static String DELIM_PARAM="#";
	//public static String ENCODING="utf-8";
	
	/**
	 * Mount an URL of all parameters stored in appParams, encoding the values.
	 * @param appParams
	 * @param excludePattern
	 * @return mounted URL
	 * @throws UnsupportedEncodingException
	 */
	public static String encodeParams(Properties appParams, String excludePattern) throws UnsupportedEncodingException {
		 Enumeration keys = appParams.keys();
		 String k, v;
		 StringBuffer buffer = new StringBuffer();
		 		  
		 while (keys.hasMoreElements()) {
			k = keys.nextElement().toString();
			if (excludePattern!=null && k.startsWith(excludePattern)) continue;
			
			v = appParams.getProperty(k);					
			
			buffer.append(k);
			buffer.append("=");
			buffer.append(URLEncoder.encode(v)); //,ENCODING));
			buffer.append(DELIM_PARAM);
		  	}
		 
		 if (buffer.length()>0) buffer.setLength(buffer.length()-1); // remove last DELIM
		 return buffer.toString();
	}
	
	/**
	 * Parse the URL parameters back to the targets properties.
	 * @param rawParams URL 
	 * @param targets List of Properties 
	 * @throws UnsupportedEncodingException
	 */
	public static void parseCycladesParams(String rawParams, Properties targets[]) throws UnsupportedEncodingException {
		if (targets==null) return;
		
		String pname="",pvalue="",v="",token="";
    	StringTokenizer tkparams;
    	StringTokenizer tklist = new StringTokenizer(rawParams,DELIM_PARAM,false);    	
    	//System.out.println("#2 - parseCycladesParams, rawParams: "+rawParams);
    	
		int itarg=0;		
    	while (tklist.hasMoreTokens()) {
    		token = tklist.nextToken();
    		//System.out.println("parseCycladesParams, token: "+token);
    		
    		tkparams= new StringTokenizer(token,"=",false);                    		
    		pname = (tkparams.hasMoreTokens()?tkparams.nextToken():"");
    		pvalue = (tkparams.hasMoreTokens()?tkparams.nextToken():"");
    		tkparams= null;
    		
    		if (pvalue!=null) v = URLDecoder.decode(pvalue); //,ENCODING);
    		else v="";
    		
    		//System.out.println("parseCycladesParams, key: "+pname+", value: ("+v+")");
    		
    		if (v==null || v.trim().equals("")) continue;    		
    		
    		for (itarg=0; itarg<targets.length; itarg++)              		
    			targets[itarg].setProperty(pname, v);    		
    		}		
	}
}
