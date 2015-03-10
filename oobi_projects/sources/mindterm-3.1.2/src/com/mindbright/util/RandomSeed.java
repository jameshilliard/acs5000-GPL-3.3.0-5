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

package com.mindbright.util;

import java.awt.Component;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.util.Enumeration;
import java.util.Properties;

import com.mindbright.jca.security.MessageDigest;

/**
 * Generates a few high quality random numbers which are useful to seed
 * other pseudo random number generators. This class will read frome
 * system devices if possible. But it also has code to collect
 * and handle randomness from user interaction.
 * <p>
 * The whole concept of cryptographically strong random numbers is
 * very complicated. This class holds a pool of randomness and always
 * estimates how much entropy (randomness) the pool contains. The
 * internal pool is always stirred whenever entropy is extracted.
 */
public final class RandomSeed implements MouseMotionListener, MouseListener,
                             KeyListener, FocusListener, ComponentListener {
    private final static boolean DEBUG = false;

    private InputStream  devRand;
    private InputStream  devURand;
    private String       devRandName;
    private String       devURandName;
    private byte[]       entropyPool;
    private int          entropyRIdx;
    private int          entropyWIdx;
    private volatile int entropyCount;

    private boolean haveEntropyGenerator;
    private int entropyGeneratorCount = 0;

    private long tickT;
    private int  evtCnt;
    private int  mouseCnt;
    private int  evtHash;
    private int  keyHash;
    private int  mouseHash;
    private int  lastX;
    private int  lastY;

    private Progress progress;

    /**
     * This class is used in the spin function to help generate a
     * random number. It basically just starts a thread which lives
     * for a specified number of milliseconds.
     */
    private static class Sleeper extends Thread {
        long sleepTime;

        /**
         * Create an instance which just lives for the specified time
         * and then exits.
         *
         * @param sleepTime how many milliseconds the thread should live
         */
        public Sleeper(long sleepTime) {
            super("RandomSeed.Sleeper");
            this.sleepTime = sleepTime;
            this.start();
        }

        public void run() {
            Thread.yield();
            try {
                Thread.sleep(sleepTime);
            } catch (InterruptedException ex) {}
        }
    }

    /**
     * Create an instance which does not load random numbers from
     * system devices.
     */
    public RandomSeed() {
        init();
        // Generate 64 bits of entropy as default
        for(int i = 0; i < 16; i++) {
            addEntropyBits((byte)spin(8), 4);
        }
    }

    /**
     * Create an instance which tries to use system random devices.
     *
     * @param devRandName name of blocking random
     *                    device. Typically <code>/dev/random</code>
     * @param devURandName name of non-blocking random
     *                     device. Typically <code>/dev/urandom</code>
     */
    public RandomSeed(String devRandName, String devURandName) {
        init();
        this.devRandName  = devRandName;
        this.devURandName = devURandName;
        this.devRand      = openRandFile(devRandName);
        this.devURand     = openRandFile(devURandName);
    }

    private void init() {
        entropyPool  = new byte[1200];
        entropyCount = 16;
        entropyRIdx  = 0;
        entropyWIdx  = 0;
        tickT        = 0L;
        evtCnt       = 0;
        mouseCnt     = 0;
        devRand      = null;
        devURand     = null;
        progress     = null;
        byte[] sysState = getSystemStateHash();
        System.arraycopy(sysState, 0, entropyPool, 0, sysState.length);
    }

    /**
     * Open a file but swallow an eventual exception.
     */
    private InputStream openRandFile(String name) {
        InputStream in = null;
        try {
            File file = new File(name);
            if(file.exists() && file.canRead()) {
                in = new FileInputStream(file);
            }
        } catch (Throwable t) {
            in = null;
        }
        return in;
    }

    /**
     * Connect to a progress meeter which gives feedback how the
     * initialization goes.
     *
     * @param progress progress indicator
     */
    public void addProgress(Progress progress) {
        this.progress = progress;
    }

    /**
     * Detach from progress meter.
     */
    public void removeProgress() {
        this.progress = null;
    }

    /**
     * Register event handles for a component so entropy can be
     * collected from events.
     *
     * @param c component to attach event handles to
     */
    public void addEntropyGenerator(Component c) {
        if(c != null) {
            c.addComponentListener(this);
            c.addKeyListener(this);
            c.addFocusListener(this);
            c.addMouseMotionListener(this);
            c.addMouseListener(this);
            haveEntropyGenerator = true;
            entropyGeneratorCount++;
        }
    }

    /**
     * Unregister event handles for a component.
     *
     * @param c component to detach from
     */
    public void removeEntropyGenerator(Component c) {
        if(c != null) {
            c.removeComponentListener(this);
            c.removeKeyListener(this);
            c.removeFocusListener(this);
            c.removeMouseMotionListener(this);
            c.removeMouseListener(this);
            if (0 == --entropyGeneratorCount) {
                haveEntropyGenerator = false;
            }
        }
    }

    /**
     * Add entropy to the pool of randomness.
     *
     * @param bits byte containing the random data to add
     * @param count estimate how many bits of entropy the byte holds
     */
    public synchronized void addEntropyBits(byte bits, int count) {
        entropyPool[entropyWIdx++] ^= bits;
        if(entropyWIdx == entropyPool.length) {
            entropyWIdx = 0;
        }
        entropyCount += count;
        if(progress != null) {
            progress.progress(entropyCount);
        }
    }

    /**
     * Checks if a blocking random device is available.
     *
     * @return true if a blocking random device is available
     */
    public boolean haveDevRandom() {
        return (devRand != null);
    }

    /**
     * Checks if a non-blocking random device is available.
     *
     * @return true if a non-blocking random device is available
     */
    public boolean haveDevURandom() {
        return (devURand != null);
    }

    /**
     * Checks if an entropy generator has been added
     *
     * @return true if an entropy generator has been added
     */
    public boolean haveEntropyGenerator() {
        return haveEntropyGenerator;
    }

    /**
     * Gets the estimate of how much entropy the randomness pool
     * contains.
     *
     * @return the estimated entropy measured in bits
     */
    public int getAvailableBits() {
        return entropyCount;
    }

    /**
     * Reset the entropy count to zero.
     */
    public void resetEntropyCount() {
        entropyCount = 0;
    }

    /**
     * Gets random bytes in a non-blocking way. Note that this
     * function may generate random numbers of lower quality of there
     * is not enough entropy in the pool.
     *
     * @param numBytes number of bytes to extract
     * @return an array containing the requested number of random bytes.
     */
    public byte[] getBytes(int numBytes) {
        if(haveDevURandom()) {
            try {
                for(int i = 0; i < numBytes; i++) {
                    int bits = devURand.read();
                    addEntropyBits((byte)bits, 8);
                }
            } catch (IOException e) {
                throw new Error("Error reading '" + devURandName + "'");
            }
        }

        return getBytesInternal(numBytes);
    }

    /**
     * Gets random bytes in a blocking way. This function will block
     * if there is not enough entropy available.
     *
     * @param numBytes number of bytes to extract
     * @return an array containing the requested number of random bytes.
     */
    public byte[] getBytesBlocking(int numBytes) {
        return getBytesBlocking(numBytes, true);
    }

    /**
     * Gets random bytes in a blocking way. This function will block
     * if there is not enough entropy available. With this function is
     * possible to control how entropy is collected if there is not
     * enough available.
     *
     * @param numBytes number of bytes to extract
     * @param generatorIfPresent if true then the function waits for
     *        the attached generators to generate enough
     *        randomness. If false the function will use the local
     *        spin generator.
     * @return an array containing the requested number of random bytes.
     */
    public byte[] getBytesBlocking(int numBytes, boolean generatorIfPresent) {
        int bits = (numBytes * 8);
        while(entropyCount < bits) {
            if(haveDevRandom()) {
                try {
                    int b = devRand.read();
                    addEntropyBits((byte)b, 8);
                } catch (IOException e) {
                    throw new Error("Error reading '" + devRandName + "'");
                }
            } else if(generatorIfPresent && haveEntropyGenerator) {
                try {
                    Thread.sleep(500);
                } catch (InterruptedException ex) {}
            }
            else {
                addEntropyBits((byte)spin(8), 4);
            }
        }

        return getBytesInternal(numBytes);
    }

    private synchronized byte[] getBytesInternal(int numBytes) {
        MessageDigest sha1 = null;
        try {
            sha1 = MessageDigest.getInstance("SHA1");
        } catch (Exception e) {
            throw new Error("Error in RandomSeed, no sha1 hash");
        }

        int    curLen = 0;
        byte[] bytes  = new byte[numBytes];
        int    offset = entropyRIdx;
        while(curLen < numBytes) {
            sha1.update((byte)System.currentTimeMillis());
            sha1.update(entropyPool, offset, 40); // estimate 4 bits/byte
            sha1.update((byte)evtCnt);
            byte[] material = sha1.digest();
            System.arraycopy(material, 0, bytes, curLen,
                             ((numBytes - curLen > material.length) ?
                              material.length : (numBytes - curLen)));
            curLen += material.length;
            offset += 40;
            offset %= entropyPool.length;
        }

        entropyRIdx  = offset;
        entropyCount -= (numBytes * 8);
        if(entropyCount < 0) {
            entropyCount = 0;
        }

        return bytes;
    }

    /**
     * Get a hash-value which reflects the current system state. The
     * value is the SHA1 hash over a bunch of data while includes the
     * current time, memory information, all system properties, IP
     * address etc. This hash can be used as a random seed, it is not
     * cryptographically strong but is a lot better than nothing.
     *
     * @return a byte array containg data which reflects the current
     *         system state.
     */
    public synchronized static byte[] getSystemStateHash() {
        
        MessageDigest sha1;
        try {
            sha1 = MessageDigest.getInstance("SHA1");
        } catch (Exception e) {
            throw new Error("Error in RandomSeed, no sha1 hash");
        }

        sha1.update((byte)System.currentTimeMillis());
        sha1.update((byte)Runtime.getRuntime().totalMemory());
        sha1.update((byte)Runtime.getRuntime().freeMemory());
        sha1.update(stackDump(new Throwable()));

        try {
            Properties  props = System.getProperties();
            Enumeration names = props.propertyNames();
            while(names.hasMoreElements()) {
                String name = (String)names.nextElement();
                sha1.update(name.getBytes("US-ASCII"));
                sha1.update(props.getProperty(name).getBytes("US-ASCII"));
            }
        } catch (Throwable t) {
            sha1.update(stackDump(t));
        }
        sha1.update((byte)System.currentTimeMillis());

        try {
            sha1.update(InetAddress.getLocalHost().toString().getBytes("US-ASCII"));
        } catch (Throwable t) {
            sha1.update(stackDump(t));
        }
        sha1.update((byte)System.currentTimeMillis());

        Runtime.getRuntime().gc();
        sha1.update((byte)Runtime.getRuntime().freeMemory());
        sha1.update((byte)System.currentTimeMillis());

        return sha1.digest();
    }

    /**
     * Creates a random number by checking how many times we can run a
     * loop and yield during a set time.
     *
     * @param t how many milliseconds the loop should run for
     */
    public static int spin(long t) {
        int counter = 0;
        Sleeper s = new Sleeper(t);
        do {
            counter++;
            Thread.yield();
        } while(s.isAlive());
        return counter;
    }

    /**
     * Extracts entropy from key presses.
     */
    public void keyPressed(KeyEvent e) {
        keyHash ^= e.getModifiers();
        keyHash += (e.getKeyCode() ^ evtHash);
        eventTick(e);
    }

    /**
     * Extracts entropy from key releases.
     */
    public void keyReleased(KeyEvent e) {
        eventTick(e);
    }

    /**
     * Extracts entropy from key typed events.
     */
    public void keyTyped(KeyEvent e) {
        keyHash ^= e.getKeyChar();
        keyHash ^= e.hashCode();
        if((evtCnt % 7) == 0) {
            addEntropyBits((byte)keyHash, 4);
        }
    }

    /**
     * Extract entropy from a component event.
     */
    public void componentHidden(ComponentEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a component event.
     */
    public void componentMoved(ComponentEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a component event.
     */
    public void componentResized(ComponentEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a component event.
     */
    public void componentShown(ComponentEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a focus event.
     */
    public void focusGained(FocusEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a focus event.
     */
    public void focusLost(FocusEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a mouse event.
     */
    public void mouseClicked(MouseEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a mouse event.
     */
    public void mouseEntered(MouseEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a mouse event.
     */
    public void mouseExited(MouseEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a mouse event.
     */
    public void mousePressed(MouseEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a mouse event.
     */
    public void mouseReleased(MouseEvent e) {
        eventTick(e);
    }

    /**
     * Extract entropy from a mouse event.
     */
    public void mouseDragged(MouseEvent e) {
        mouseMoved(e);
    }

    /**
     * Extract entropy from a mouse event.
     */
    public void mouseMoved(MouseEvent e) {
        if(DEBUG)
            System.out.println("RandomSeed.mouseMoved: " + e);
        mouseCnt++;
        int dX = lastX - e.getX();
        int dY = lastY - e.getY();
        lastX  = e.getX();
        lastY  = e.getY();

        mouseHash ^= e.hashCode();
        mouseHash ^= e.getX();
        mouseHash ^= dY;
        mouseHash ^= e.getY();
        mouseHash ^= dX;

        if((mouseCnt % 3) == 0) {
            addEntropyBits((byte)mouseHash, 4);
        }
    }

    /**
     * Print a stack-dump into a byte array.
     *
     * @param t throwable to generate stack dump from
     *
     * @return a byte-array containing the stack dump
     */
    public static byte[] stackDump(Throwable t) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        PrintWriter pw = new PrintWriter(baos);
        t.printStackTrace(pw);
        return baos.toByteArray();
    }

    /**
     * Add entropy to the pool. This function is called from all event
     * handlers above. It stirrs the pool a bit, sometimes it adds the
     * system time and every now and then.
     */
    private void eventTick(Object o) {
        if(DEBUG)
            System.out.println("RandomSeed.eventTick: " + o);

        // Keep tracvk of how many events have happened and update the evtHash
        evtCnt++;
        evtHash ^= o.hashCode();

        // Every five events we add the current time to the entropy pool
        if((evtCnt % 5) == 0) {
            long now = System.currentTimeMillis();
            addEntropyBits((byte)(now - tickT), 4);
            tickT = now;
            evtHash ^= now;
        }

        // Every 17 events we add the evtHash to the pool
        if((evtCnt % 17) == 0) {
            addEntropyBits((byte)evtHash, 4);
        }
    }
}
