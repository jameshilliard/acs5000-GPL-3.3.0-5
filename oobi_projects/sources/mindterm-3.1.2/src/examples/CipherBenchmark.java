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
package examples;

/**
 * Benchmark the different encryption algorithms. The code as shipped
 * tests the MindTerm implementations of the algorithms. By
 * uncommenting relevant portions, and building with Java 1.4 or
 * later, it is possible to also benchmark some of the implementations
 * in the Java runtime.
 * <p>
 * Usage:
 * <code> java -cp examples.jar examples.CipherBenchmark</code>
 */
public class CipherBenchmark {
    /**
     * Fill the given array with zeros
     */
    private static void clear(byte[] b) {
        for (int i=0; i<b.length; i++)
            b[i] = 0x00;
    }

    /**
     * How long to test each algorithm
     */
    private final static double RUNTIME = 5.0;

    /************************************************************************
     * The following two routines are used to test the algorithms
     * provided by Java. However they require java 1.4 or later to
     * even compile so they have been commented out.
     * To test the performance in java uncomment the bench14 routines
     * as well as the calls to them in the main function.
     */
//     private static void bench14(String name, String cipher, boolean encrypt,
//                                int keylen, int blocksize) {
//         bench14(name, cipher, encrypt, keylen, blocksize, 0);
//     }

//     /**
//      * Benchmark an algorithm in the Java runtime
//      *
//      * @param name name to print when presenting result
//      * @param cipher formal name of algorithm to test
//      * @param encrypt true to encrypt and false to decrypt
//      * @param keylen length of keys (bytes)
//      * @param blocksize length of block to encrypt (bytes)
//      * @param ivlength length of IV, 0 means no IV is needed
//      */
//     private static void bench14(String name, String cipher, boolean encrypt,
//                                int keylen, int blocksize, int ivlength)
//     {
//         try {
//             javax.crypto.Cipher c = javax.crypto.Cipher.getInstance(cipher);
//             byte[] in  = new byte[blocksize];
//             byte[] out = new byte[blocksize];
//             byte[] keydata = new byte[keylen];
//             javax.crypto.spec.SecretKeySpec key;

//             clear(in);
//             clear(out);
//             clear(keydata);

//             String algShortName;
//             if (cipher.indexOf('/') > 0) {
//                 algShortName = cipher.substring(0, cipher.indexOf('/'));
//             } else {
//                 algShortName = cipher;
//             }
//             key = new javax.crypto.spec.SecretKeySpec(keydata, algShortName);
//             if (ivlength > 0) {
//                 byte[] iv  = new byte[ivlength];
//                 clear(iv);
//                 c.init(encrypt ? javax.crypto.Cipher.ENCRYPT_MODE :
//                        javax.crypto.Cipher.DECRYPT_MODE, key,
//                        new javax.crypto.spec.IvParameterSpec(iv));
//             } else {
//                 c.init(encrypt ? javax.crypto.Cipher.ENCRYPT_MODE :
//                        javax.crypto.Cipher.DECRYPT_MODE, key);
//             }
            
//             long start = System.currentTimeMillis();
//             for (int i=0; i<5000; i++)
//                 c.doFinal(in, 0, in.length, out, 0);
//             long end = System.currentTimeMillis();

//             int m = (int)((RUNTIME / ((double)(end-start)/1000.0)) * 5000);

//             start = System.currentTimeMillis();
//             long size = 0;
//             for (int i=0; i<m; i++) {
//                 size += in.length;
//                 c.doFinal(in, 0, in.length, out, 0);
//             }
//             end = System.currentTimeMillis();

//             double t = end - start;
//             double sz = size;
//             double mbps = ((double)((int)(size / t))) / 1000.0;
            
//             System.out.println(name+", "+(encrypt?"en":"de")+"crypt: "
//                                + (float)mbps + " MBps");
//         } catch(Throwable t) {
//             System.out.println(t);
//         }
//     }


    /**
     * Benchmark an algorithm
     *
     * @param name name to print when presenting result
     * @param cipher formal name of algorithm to test
     * @param encrypt true to encrypt and false to decrypt
     * @param keylen length of keys (bytes)
     * @param blocksize length of block to encrypt (bytes)
     */
    public static void bench(String name, String cipher, boolean encrypt,
                             int keylen, int blocksize)
    throws com.mindbright.jca.security.GeneralSecurityException {
        com.mindbright.jce.crypto.Cipher c =
            com.mindbright.jce.crypto.Cipher.getInstance(cipher);
        byte[] in  = new byte[blocksize];
        byte[] out = new byte[blocksize];
        byte[] key = new byte[keylen];

        clear(in);
        clear(out);
        clear(key);

        c.init(encrypt ? com.mindbright.jce.crypto.Cipher.ENCRYPT_MODE :
               com.mindbright.jce.crypto.Cipher.DECRYPT_MODE,
               new com.mindbright.jce.crypto.spec.SecretKeySpec(key, c.getAlgorithm()));

        long start = System.currentTimeMillis();
        for (int i=0; i<5000; i++)
            c.doFinal(in, 0, in.length, out, 0);
        long end = System.currentTimeMillis();

        int m = (int)((RUNTIME / ((double)(end-start)/1000.0)) * 5000);

        start = System.currentTimeMillis();
        long size = 0;
        for (int i=0; i<m; i++) {
            size += in.length;
            c.doFinal(in, 0, in.length, out, 0);
        }
        end = System.currentTimeMillis();

        double t = end - start;
        double sz = size;
        double mbps = ((double)((int)(size / t))) / 1000.0;

        System.out.println(name+", "+(encrypt?"en":"de")+"crypt: "
                           + (float)mbps + " MBps");
    }

    /**
     * Run the benchmark
     */
    public static void main(String[] argv) {
        try {
            System.out.println("MindTerm implementations");
            bench("3des-cbc", "3DES/CBC", true,  24, 1024);
            bench("3des-cbc", "3DES/CBC", false, 24, 1024);

            bench("blowfish-cbc", "Blowfish/CBC", true,  16, 1024);
            bench("blowfish-cbc", "Blowfish/CBC", false, 16, 1024);

            bench("cast128-cbc", "CAST128/CBC", true,  16, 1024);
            bench("cast128-cbc", "CAST128/CBC", false, 16, 1024);

            bench("idea-cbc", "IDEA/CBC", true,  16, 1024);
            bench("idea-cbc", "IDEA/CBC", false, 16, 1024);

            bench("arcfour", "RC4/OFB", true,  16, 1024);
            bench("arcfour", "RC4/OFB", false, 16, 1024);

            bench("aes128-cbc", "AES/CBC", true,  16, 1024);
            bench("aes128-cbc", "AES/CBC", false, 16, 1024);

            bench("aes192-cbc", "AES/CBC", true,  24, 1024);
            bench("aes192-cbc", "AES/CBC", false, 24, 1024);

            bench("aes256-cbc", "AES/CBC", true,  32, 1024);
            bench("aes256-cbc", "AES/CBC", false, 32, 1024);

            bench("twofish128-cbc", "Twofish/CBC", true,  16, 1024);
            bench("twofish128-cbc", "Twofish/CBC", false, 16, 1024);

            bench("twofish192-cbc", "Twofish/CBC", true,  24, 1024);
            bench("twofish192-cbc", "Twofish/CBC", false, 24, 1024);

            bench("twofish256-cbc", "Twofish/CBC", true,  32, 1024);
            bench("twofish256-cbc", "Twofish/CBC", false, 32, 1024);

//             System.out.println("Builtin implementations");
//             bench14("3des-cbc", "DESede/CBC/NoPadding", true, 24, 1024, 8);
//             bench14("3des-cbc", "DESede/CBC/NoPadding", false, 24, 1024, 8);
//             bench14("arcfour", "RC4", true,  16, 1024);
//             bench14("arcfour", "RC4", false, 16, 1024);
//             bench14("aes128", "AES/CBC/NoPadding", true,  16, 1024, 16);
//             bench14("aes128", "AES/CBC/NoPadding", false, 16, 1024, 16);

        } catch (Throwable t) {
            t.printStackTrace();
        }
    }
}
