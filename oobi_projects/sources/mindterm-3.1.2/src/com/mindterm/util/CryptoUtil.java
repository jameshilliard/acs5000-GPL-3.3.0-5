package com.mindterm.util;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;

import com.mindbright.jca.security.InvalidKeyException;
import com.mindbright.jca.security.NoSuchAlgorithmException;
import com.mindbright.util.Base64;

//import sun.misc.BASE64Decoder;

/**
 * Cryptography utility which provides access to Mindterm crypto classes. 
 * 
 * WARNING: APMWEB JCECrypto class will encrypt login data and pass it to Mindterm. 
 * Mindterm CryptoUtil class will decrypt this data.
 * Thus if you modify this class, take a look at JCECrypto to keep it in sync.    
 *  
 * This class must be compliant with JDK 1.2.2.
 *  
 * @author eduardo.soares@cyclades.com
 * @see [APMWEB PROJECT]: com.mindterm.utils.crypto.JCECrypto
 */
public class CryptoUtil {
	private static CryptoUtil singleton = new CryptoUtil();
	public final int KEY_SIZE_BITS = 128;  
	public final String ALGORITHM = "AES/ECB";  
	
	private  CryptoUtil() { }
	
	public static CryptoUtil getInstance() {
		return singleton;
	}
	
	/**
	 * Set the String size to the nearest multiple number.
	 * 
	 * @param m
	 * @param mult
	 * @param ch Character to fill
	 * @return
	 */
	private String ajustMultipleChars(String m, int mult, char ch) {
		if (m==null) return null;
		
		int r = m.length() % mult;
		if (r != 0) {
			int div = m.length() / mult;			
			return ajustSize(m, (mult * (div+1)), ch); 
			}
		else return m;
	}
	

	/**
	 * Truncate or fill the 'information' to reach the size provided.
	 * 
	 * @param m Information
	 * @param size 
	 * @return Information modified to the size provided.
	 */
	private String ajustSize(String m, int size, char ch) {
		if (m==null) return null;
		
		if (m.length()<size) {			
			StringBuffer buf = new StringBuffer(m);
			
			for (int i=0; i<(size-m.length()); i++) {
				if (ch=='\0') buf.append(m.charAt( i % (m.length()-1) ));
				else buf.append(ch);
				}
			
			return buf.toString();
			}			
		else if (m.length()>size) 
			return m.substring(0, size);		
		else return m;		
	}	

	/**
	 * Use characters of string m to fill.
	 * 
	 * @param m
	 * @param mult
	 * @return
	 */
	private String ajustSize(String m, int size) {
		return ajustSize(m, size, '\0');
	}
	
	/**
	 * Decrypts data previously encrypted by JCE API.
	 * 
	 * @param encdata encrypted data with base64 encoding
	 * @param password Flat string
	 * @return decrypted values
	 * @throws NoSuchAlgorithmException 
	 * @throws IOException 
	 * @throws NoSuchAlgorithmException 
	 * @throws InvalidKeyException 
	 * @throws IOException 
	 * @throws ClassNotFoundException 
	 * @throws Exception
	 */
	public String[] decryptMindterm(String encdata, String password) throws NoSuchAlgorithmException, InvalidKeyException, IOException, ClassNotFoundException   {
		// set key size of 128 bits (16 bytes)
		password  = ajustSize(password, KEY_SIZE_BITS/8);
		
		// decode the base64 encoded data
		byte[] bx = Base64.decode(encdata.getBytes());		
		//-----------------------------------
		// decrypts the data
		com.mindbright.jce.crypto.Cipher cipher = com.mindbright.jce.crypto.Cipher.getInstance(ALGORITHM);
		cipher.init(com.mindbright.jce.crypto.Cipher.DECRYPT_MODE,
             new com.mindbright.jce.crypto.spec.SecretKeySpec(password.getBytes(), cipher.getAlgorithm()));           
		
		byte[] dx = cipher.doFinal(bx, 0, bx.length);
		//-----------------------------------
		// convert decrypted bytes into an Object 
		String str[] = null;
		ByteArrayInputStream bin = new ByteArrayInputStream(dx);
        ObjectInputStream oin = new ObjectInputStream(bin);
        try {
        	return (String[])oin.readObject();
        } finally {
        	bin.close();
        	}        		
	}		

	// WARNING: the data to be encrypted must be multiple of 8
	// PROBLEM: Not possible to encrypt a serialized array of bytes, just Strings
	// For this reason, APMWEB uses JCECrypto to encrypt	
/*	public String encryptMindterm(String data, String password) throws InvalidKeyException, NoSuchAlgorithmException, IOException  {	 
		data = ajustMultipleChars(data,KEY_SIZE_BITS/8, ' ');

		byte objData[] = data.getBytes(); 
		//----------------------------------------------------
		password  = ajustSize(password, KEY_SIZE_BITS/8,'\0');
		
		com.mindbright.jce.crypto.Cipher cipher = com.mindbright.jce.crypto.Cipher.getInstance(ALGORITHM);
		cipher.init(com.mindbright.jce.crypto.Cipher.ENCRYPT_MODE,
             new com.mindbright.jce.crypto.spec.SecretKeySpec(password.getBytes(), cipher.getAlgorithm()));             

		byte[] dx = cipher.doFinal(objData, 0, objData.length);
		return new String(Base64.encode(dx));
	}
*/	
}
