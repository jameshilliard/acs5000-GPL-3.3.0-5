package com.mindterm.util;

public class StringUtil {

	private static String replaceStr(int idx0, String s, String olds, String news) {
		if (idx0==-1) return s;
			
		if (idx0>=0) 
			s = s.substring(0, idx0)+ news + s.substring(idx0+olds.length());
			
		return replaceStr(s.indexOf(olds), s, olds,news);
	}
	
	public static String replaceAll(String s, String olds, String news) {
		return replaceStr(s.indexOf(olds), s, olds,news);
	}
	
}
