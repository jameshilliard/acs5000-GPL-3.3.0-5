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

package tests.com.mindbright.terminal.tandem6530;

import java.lang.StringBuffer;
import java.util.Properties;
import java.util.NoSuchElementException;

import com.mindbright.util.RandomSeed;
import com.mindbright.terminal.TerminalWindow;
import com.mindbright.terminal.TerminalInputListener;
import com.mindbright.terminal.TerminalOutputListener;
import com.mindbright.terminal.TerminalPrinter;
import com.mindbright.terminal.TerminalOption;
import com.mindbright.terminal.SearchContext;

/** This class pretends to be a TerminalWindow. All it does is
 * collecting characters that the terminal sends to the host.
 */
public class MockTerminalWindow implements TerminalWindow {

    private StringBuffer buf = new StringBuffer();

    public String getSentChars() {
        String ret = buf.toString();
        if (ret.length() == 0) {
            return null;
        }
        buf.setLength(0);
        return ret;
    }
    public void typedChar(char c) {
        buf.append(c);
    }
    public void sendBytes(byte[] b) {
        if (b == null) {
            return;
        }
        for (int i = 0; i < b.length; i++) {
            buf.append((char) b[i]);
        }
    }
    public void sendBreak() {}

    //
    // These methods are not interesting when testing the 6530 emulator
    //
    public void setTitle(String title) {}
    public String getTitle() {
        return null;
    }
    public int rows() {
        return -1;
    }
    public int cols() {
        return -1;
    }
    public int vpixels() {
        return -1;
    }
    public int hpixels() {
        return -1;
    }

    public void write(char c) {}
    public void write(char[] c, int off, int len) {}
    public void write(byte[] c, int off, int len) {}
    public void write(String str) {}

    public void addInputListener(TerminalInputListener listener) {}
    public void removeInputListener(TerminalInputListener inListener) {}
    public void addOutputListener(TerminalOutputListener listener) {}
    public void removeOutputListener(TerminalOutputListener listener) {}
    public void attachPrinter(TerminalPrinter printer) {}
    public void detachPrinter() {}

    public void reset() {}
    public void printScreen() {}
    public void startPrinter() {}
    public void stopPrinter() {}

    public String terminalType() {
        return "mock6530";
    }
    public void setProperties(Properties newProps, boolean merge)
    throws IllegalArgumentException, NoSuchElementException {}
    public void setProperty(String key, String value)
    throws IllegalArgumentException, NoSuchElementException {}
    public Properties getProperties() {
        return null;
    }
    public String getProperty(String key) {
        return null;
    }
    public void resetToDefaults() {}
    public boolean getPropsChanged() {
        return false;
    }
    public void  setPropsChanged(boolean value) {}
    public void enableOptionInMenu(String key, String value) {}
    public TerminalOption[] getOptions() {
        return null;
    }
    public SearchContext search(SearchContext lastContext, String key,
                                boolean reverse, boolean caseSens) {
        return null;
    }

    public void addAsEntropyGenerator(RandomSeed seed) {}

    // XXX Silly methods because they are used around code.
    public void setAttributeBold(boolean set
                                ) {}
    public void clearScreen() {}
    public void ringBell() {}
    public void setCursorPos(int row, int col) {}
    public void clearLine() {}
}
