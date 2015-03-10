package com.mindterm.util;

import java.util.StringTokenizer;
import java.util.Vector;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.PBEParameterSpec;




/**
 * utils for apm
 */
public class Utils {
    public static boolean DEBUG = false;       
    /** This array is used to convert from bytes to hexadecimal numbers */
    static final char[] digits = { '0', '1', '2', '3', '4', '5', '6', '7',
                                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    /**
     * Encyrpts str (an array of strings) using key supplied. Uses some predfined salt
     * and an MD5 digest and DES encryption
     */
    public static String encrypt(String[] str, String key) {
        try {
            PBEKeySpec pbeKeySpec;
            PBEParameterSpec pbeParamSpec;
            SecretKeyFactory keyFac;
            
            // Salt
            byte[] salt = {
                (byte)0xc7, (byte)0x73, (byte)0x21, (byte)0x8c,
                (byte)0x7e, (byte)0xc8, (byte)0xee, (byte)0x99
            };
            
            // Iteration count
            int count = 20;
            //sessionId = "abc4592";
            
            // Create PBE parameter set
            pbeParamSpec = new PBEParameterSpec(salt, count);
            char[] sa = new char[key.length()];
            key.getChars(0, key.length(), sa, 0);
            pbeKeySpec = new PBEKeySpec(sa);
            keyFac = SecretKeyFactory.getInstance("PBEWithMD5AndDES");
            SecretKey pbeKey = keyFac.generateSecret(pbeKeySpec);
            
            // Create PBE Cipher
            Cipher pbeCipher = Cipher.getInstance("PBEWithMD5AndDES");
            
            // Initialize PBE Cipher with key and parameters
            pbeCipher.init(Cipher.ENCRYPT_MODE, pbeKey, pbeParamSpec);
            
            // Our cleartext
            String cleartextStr = "";
            for (int i = 0; i < str.length; i++) {
                cleartextStr = cleartextStr + str[i] + ";";
            }
            
            byte[] cleartext = cleartextStr.getBytes();
            if (DEBUG) {
                System.out.println("v5. unencrpyted text = >" + cleartextStr + "<");
            }
            // Encrypt the cleartext
            byte[] ciphertext = pbeCipher.doFinal(cleartext);
            String ciphertextStr = new String(Utils.hexEncode(ciphertext));
            //String ciphertextStr = new String(ciphertext, "ISO-8859-1");
            if (DEBUG) {
                System.out.println("v5. encrypted text = >" + ciphertextStr + "<");
                System.out.println("length = " + ciphertextStr.length());
            }
            
            return ciphertextStr;
            
        } catch (Exception e) {
            System.out.println("exception= " + e);
        }
        
        return null;
    }
    
    public static String[] decrypt(String ciphertextStr, String key) {
        try {
            byte[] ciphertext = ciphertextStr.getBytes();
            
            PBEKeySpec pbeKeySpec;
            PBEParameterSpec pbeParamSpec;
            SecretKeyFactory keyFac;
            
            // Salt
            byte[] salt = {
                (byte)0xc7, (byte)0x73, (byte)0x21, (byte)0x8c,
                (byte)0x7e, (byte)0xc8, (byte)0xee, (byte)0x99
            };
            
            // Iteration count
            int count = 20;
            //sessionId = "abc4592";
            
            // Create PBE parameter set
            pbeParamSpec = new PBEParameterSpec(salt, count);
            char[] sa = new char[key.length()];
            key.getChars(0, key.length(), sa, 0);
            pbeKeySpec = new PBEKeySpec(sa);
            keyFac = SecretKeyFactory.getInstance("PBEWithMD5AndDES");
            SecretKey pbeKey = keyFac.generateSecret(pbeKeySpec);
            
            // Create PBE Cipher
            Cipher pbeCipher = Cipher.getInstance("PBEWithMD5AndDES");
            
            // Initialize PBE Cipher with key and parameters
            pbeCipher.init(Cipher.DECRYPT_MODE, pbeKey, pbeParamSpec);
            if (DEBUG) {
                System.out.println("v5. decrypting= >" + ciphertextStr + "<");
                System.out.println("v5. length = " + ciphertextStr.length());
            }
            byte[] ciphertextDecrypt = pbeCipher.doFinal(Utils.hexDecode(ciphertextStr));
            //byte[] ciphertextDecrypt = pbeCipher.doFinal(ciphertextStr.getBytes("ISO-8859-1"));
            String decrpytedStr = new String(ciphertextDecrypt);
            if (DEBUG) {
                System.out.println("v5. decrpyted text= >" + decrpytedStr + "<");
            }
            //split it up
            StringTokenizer st = new StringTokenizer(decrpytedStr, ";");
            Vector v = new Vector();
            while (st.hasMoreElements()) {
                v.addElement((String) st.nextElement());
            }
            String[] res = new String[v.size()];
            v.copyInto(res);
            return res;
            
        } catch (Exception e) {
            System.out.println("exception= " + e);
        }
        
        return null;
    }

    public static String hexEncode(byte[] bytes) {
        StringBuffer s = new StringBuffer(bytes.length * 2);
        for (int i = 0; i < bytes.length; i++) {
            byte b = bytes[i];
            s.append(digits[(b & 0xf0) >> 4]);
            s.append(digits[b & 0x0f]);
        }
        return s.toString();
    }

    /**
     * A convenience method to convert in the other direction, from a string
     * of hexadecimal digits to an array of bytes.
     **/
    public static byte[] hexDecode(String s) throws IllegalArgumentException {
        try {
            int len = s.length();
            byte[] r = new byte[len/2];
            for (int i = 0; i < r.length; i++) {
                int digit1 = s.charAt(i*2);
                int digit2 = s.charAt(i*2 + 1);
                if ((digit1 >= '0') && (digit1 <= '9')) {
                    digit1 -= '0';
                } else if ((digit1 >= 'a') && (digit1 <= 'f')) {
                    digit1 -= 'a' - 10;
                }
                if ((digit2 >= '0') && (digit2 <= '9')) {
                    digit2 -= '0'; 
                } else if ((digit2 >= 'a') && (digit2 <= 'f')) digit2 -= 'a' - 10;
                    r[i] = (byte)((digit1 << 4) + digit2);
                }
                return r;
        } catch (Exception e) {
            throw new IllegalArgumentException("hexDecode(): invalid input");
        }
    }

    public static String myDecrypt(String key, String text, boolean def) {
        try {
            String enMask;
            short nChar;
            char   enChar;
            int    numChars = 0, minLen,numMask=0;
            String textString = "";

            
            if (def) {
               key = "ioHrEw(L#yZfjsfSdfHfSgGjMjq;Ss;d)(*&^#@$avsig";
            }
            //minLen = Math.min(text.length(), key.length());
            minLen = text.length();
            enMask = key;

				//System.out.println("Key: " + key);
				//System.out.println("Text: " + text);
            
            while (numChars < minLen) { 
               nChar = (short)((short)text.charAt(numChars) ^ (short)enMask.charAt(numMask));
               //System.out.println("nChar: " + nChar);
               enChar = (char)nChar;
					//enChar = Character.forDigit(nChar, 10);
               //System.out.println("enChar: [" + enChar + "]");
               
               textString += enChar;
		if (++numMask == key.length()) {
			numMask=0;
		}
               numChars++;
           
            }
            
            return (textString);
                     
        } catch (Exception e) {
            System.out.println("exception= " + e);
        }
        
        return null;
    }	
    
}


