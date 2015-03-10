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

import junit.framework.TestCase;

import com.mindbright.terminal.DisplayView;
import com.mindbright.terminal.DisplayModel;

import com.mindbright.terminal.tandem6530.Terminal6530Callback;
import com.mindbright.terminal.tandem6530.ConvMode;
import com.mindbright.terminal.tandem6530.Parser;

public class TestConvMode extends TestCase implements AsciiCodes,
            Terminal6530Callback, DisplayView {
    ConvMode mode;
    Parser parser = new Parser();
    StringBuffer buf;
    boolean displayUpdated = false;

    public void setUp() {
        mode = new ConvMode(this, parser);
        mode.setDisplay(this);
        buf = new StringBuffer();
    }

    public void testDuplex() {
        // Full duplex (typed char just sent to host and echoed back
        // there.
        displayUpdated = false;
        mode.setHalfDuplex(false);
        mode.keyHandler('a', 0, 0);
        mode.keyHandler('p', 0, 0);
        mode.keyHandler('a', 0, 0);

        assertEquals("apa", buf.toString());
        assertFalse("Display was updated", displayUpdated);
        buf.setLength(0);

        // Half duplex (typed char sent to host and sent directly to
        // display.
        displayUpdated = false;
        mode.setHalfDuplex(true);
        mode.keyHandler('a', 0, 0);
        mode.keyHandler('p', 0, 0);
        mode.keyHandler('a', 0, 0);

        assertEquals("apa", buf.toString());
        assertTrue("Display was not updated", displayUpdated);
    }

    public void testReadCursor() {
        mode.doSetCursorAddress(true, 1, 1);
        assertEquals(SOH + "_!  " + CR, mode.doReadCursorAddress());

        mode.doSetCursorAddress(true, 24, 80);
        assertEquals(SOH + "_!7o" + CR, mode.doReadCursorAddress());

        mode.doDisplayPage(0);

        mode.doSetCursorAddress(true, 1, 1);
        assertEquals(SOH + "_!  " + CR, mode.doReadCursorAddress());

        mode.doSetCursorAddress(true, 24, 80);
        assertEquals(SOH + "_!7o" + CR, mode.doReadCursorAddress());
    }

    // Terminal6530Callback interface
    public void send(String str) {
        buf.append(str);
    }
    public void send(char c) {
        buf.append(c);
    }
    public void error(String msg) { }
    public void statusLineUpdated() { }

    // DisplayView interface
    public void setModel(DisplayModel model) { }
    public void setVisTopChangeAllowed(boolean set
                                      ) { }
    public void setVisTopDelta(int delta) { }
    public void setVisTopDelta(int delta, boolean force) { }
    public void setVisTop(int visTop) { }
    public void setVisTop(int visTop, boolean force) { }
    public void setPendingVisTopChange(int visTop) { }

    public void updateScrollbarValues() { }
    public void updateDirtyArea(int top, int left, int bottom, int right) {
        displayUpdated = true;
    }
    public void repaint(boolean force) { }
    public void repaint() { }
    public void setGeometry(int row, int col) { }
    public void resetSelection() { }
    public void setSelection(int row1, int col1, int row2, int col2) { }
    public void setNoCursor() { }
    public void setCursorPosition(int row, int col) { }
    public void reverseColors() { }
    public void doBell() { }
    public void doBell(boolean visualBell) { }




}

