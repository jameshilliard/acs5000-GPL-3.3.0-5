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


import junit.framework.TestCase;
import junit.framework.AssertionFailedError;

import com.mindbright.terminal.tandem6530.DataType;
import com.mindbright.terminal.tandem6530.Position;
import com.mindbright.terminal.tandem6530.FieldAttributes;
import com.mindbright.terminal.tandem6530.ProtectedScreen;

public class TestProtectedScreen extends TestCase implements AsciiCodes {
    int COLS = 15;
    int ROWS = 10;

    protected Position HOME = new Position(0, 0);
    protected Position END = new Position(ROWS, COLS);

    protected DataType dataTypeTable;
    protected ProtectedScreen screen;
    protected Position p1;
    protected Position p2;
    protected Position p3;
    protected Position p4;
    protected Position p5;
    protected Position p6;

    protected String header(int row, int col) {
        return DC1 + String.valueOf((char) (0x1f + row + 1)) +
               String.valueOf((char) (0x1f + col + 1));
    }

    public void setUp() {
        /* Define a screen that looks like:
         * ----------------- 
         * |PPPPPPPPPPPPPPP|
         * |Pdjur:_________|
         * |Pmat: _________|
         * |PPPPPPPPPPPPPPP|
         * |PPPPPPPPPPPPPPP|
         * |PPPPPPPPPPPPPPP|
         * |PPPPPPPPPPPPPPP|
         * |PPPPPPPPPPPPPPP|
         * |PPPPPPPPPPPPPPP|
         * |PPPPPPPPPPPPPPP|
         * -----------------
         * P = protected position
         * _ = unprotected position
         * labels are protected as well
         */

        dataTypeTable = new DataType();
        screen = new ProtectedScreen(ROWS, COLS, dataTypeTable);

        FieldAttributes a1= new FieldAttributes();
        a1.setProtect(true);
        a1.setVideoAttrib((char) (0x20 + 10));
        p1 = new Position(1, 1);
        screen.addField(p1, a1);
        screen.setBufferAddress(1, 2);
        screen.bufferWrite('d');
        screen.bufferWrite('j');
        screen.bufferWrite('u');
        screen.bufferWrite('r');
        screen.bufferWrite(':');

        FieldAttributes a2= new FieldAttributes();
        a2.setVideoAttrib((char) (0x20 + 20));
        p2 = new Position(1, 7);
        screen.addField(p2, a2);

        FieldAttributes a3= new FieldAttributes();
        a3.setProtect(true);
        p3 = new Position(2, 0);
        screen.addField(p3, a3);

        FieldAttributes a4= new FieldAttributes();
        a4.setProtect(true);
        p4 = new Position(2, 1);
        screen.addField(p4, a4);
        screen.setBufferAddress(2, 2);
        screen.bufferWrite('m');
        screen.bufferWrite('a');
        screen.bufferWrite('t');
        screen.bufferWrite(':');

        FieldAttributes a5= new FieldAttributes();
        p5 = new Position(2, 7);
        screen.addField(p5, a5);

        FieldAttributes a6= new FieldAttributes();
        a6.setProtect(true);
        p6 = new Position(3, 0);
        screen.addField(p6, a6);
    }

    protected void assertArrayEquals(char exp[], char a[]) {
        if (exp == null && a == null) {
            return;
        }
        if (exp == null && a != null) {
            throw new AssertionFailedError("Array was not null as expected");
        }
        if (exp != null && a == null) {
            throw new AssertionFailedError("Array was null");
        }

        if (exp.length != a.length) {
            throw new AssertionFailedError("Length did not match." +
                                           " Expected " + exp.length + " but got " + a.length);
        }

        for (int i = 0; i < exp.length; i++) {
            if (exp[i] != a[i]) {
                throw new AssertionFailedError("Arrays was unequal at offset "+
                                               i + ". Expected " + (int) exp[i] +
                                               " but got " + (int) a[i]);
            }
        }
    }

    protected void assertArrayEquals(int exp[], int a[]) {
        if (exp == null && a == null) {
            return;
        }
        if (exp == null && a != null) {
            throw new AssertionFailedError("Array was not null as expected");
        }
        if (exp != null && a == null) {
            throw new AssertionFailedError("Array was null");
        }

        if (exp.length != a.length) {
            throw new AssertionFailedError("Length did not match." +
                                           " Expected " + exp.length + " but got " + a.length);
        }

        for (int i = 0; i < exp.length; i++) {
            if (exp[i] != a[i]) {
                throw new AssertionFailedError("Arrays was unequal at offset "+
                                               i + ". Expected " + exp[i] +
                                               " but got " + a[i]);
            }
        }
    }

    public void testReadWithAddress() {
        String exp;

        exp = "" + EOT;
        assertEquals(exp, screen.readWithAddress(HOME, END));

        screen.setCursorAddress(1, 8);
        screen.cursorWrite('a');
        screen.cursorWrite('p');
        screen.cursorWrite('a');

        exp = header(1, 8) + "apa" + EOT;
        assertEquals(exp, screen.readWithAddress(HOME, END));

        screen.setCursorAddress(2, 8);
        screen.cursorWrite('f');
        screen.cursorWrite('o');
        screen.cursorWrite('d');
        screen.cursorWrite('e');
        screen.cursorWrite('r');

        exp = header(1, 8) + "apa" +
              header(2, 8) + "foder" + EOT;
        assertEquals(exp, screen.readWithAddress(HOME, END));

        exp = header(1, 8) + "apa" + EOT;
        Position p = new Position(1, 8);
        assertEquals(exp, screen.readWithAddress(p, p));
    }

    public void testReadWithAddressAll() {
        String exp;

        exp = header(0, 1) +
              header(1, 2) + "djur:" + header(1, 8) +
              header(2,1) + header(2, 2) + "mat:" + header(2, 8) + EOT;
        assertEquals(exp, screen.readWithAddressAll(HOME, END));

        screen.setCursorAddress(1, 8);
        assertTrue(screen.cursorWrite('a'));
        assertTrue(screen.cursorWrite('p'));
        assertTrue(screen.cursorWrite('a'));

        exp = header(0, 1) +
              header(1, 2) + "djur:" + header(1, 8) + "apa" +
              header(2, 1) + header(2, 2) + "mat:" + header(2, 8) + EOT;
        assertEquals(exp, screen.readWithAddressAll(HOME, END));

        screen.setCursorAddress(2, 8);
        assertTrue(screen.cursorWrite('f'));
        assertTrue(screen.cursorWrite('o'));
        assertTrue(screen.cursorWrite('d'));
        assertTrue(screen.cursorWrite('e'));
        assertTrue(screen.cursorWrite('r'));

        exp = header(0, 1) +
              header(1, 2) + "djur:" + header(1, 8) + "apa" +
              header(2, 1) + header(2, 2) + "mat:" + header(2, 8) + "foder" +
              EOT;
        assertEquals(exp, screen.readWithAddressAll(HOME, END));

        exp = header(1, 8) + "apa" + EOT;
        Position p = new Position(1, 8);
        assertEquals(exp, screen.readWithAddressAll(p, p));
    }

    public void testReadWholePageOrBuffer() {
        String exp;

        exp = header(1, 8) +
              header(2, 8) + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());

        screen.setCursorAddress(1, 8);
        assertTrue(screen.cursorWrite('a'));
        assertTrue(screen.cursorWrite('p'));
        assertTrue(screen.cursorWrite('a'));

        exp = header(1, 8) + "apa" +
              header(2, 8) + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());

        screen.setCursorAddress(2, 8);
        assertTrue(screen.cursorWrite('f'));
        assertTrue(screen.cursorWrite('o'));
        assertTrue(screen.cursorWrite('d'));
        assertTrue(screen.cursorWrite('e'));
        assertTrue(screen.cursorWrite('r'));

        exp = header(1, 8) + "apa" +
              header(2, 8) + "foder" + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());
    }

    public void testReadWholePageOrBufferSpaceFilled() {
        String exp;

        screen.setBufferAddress(1, 8);
        for (int i = 0; i < 8; i++) {
            screen.bufferWrite(' ');
        }
        screen.setBufferAddress(1, 9);
        for (int i = 0; i < 8; i++) {
            screen.bufferWrite(' ');
        }

        exp = header(1, 8) +
              header(2, 8) + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());

        screen.setCursorAddress(1, 8);
        assertTrue(screen.cursorWrite('a'));
        assertTrue(screen.cursorWrite('p'));
        assertTrue(screen.cursorWrite('a'));

        exp = header(1, 8) + "apa" +
              header(2, 8) + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());

        screen.setCursorAddress(2, 8);
        assertTrue(screen.cursorWrite('f'));
        assertTrue(screen.cursorWrite('o'));
        assertTrue(screen.cursorWrite('d'));
        assertTrue(screen.cursorWrite('e'));
        assertTrue(screen.cursorWrite('r'));

        exp = header(1, 8) + "apa" +
              header(2, 8) + "foder" + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());
    }

    public void testReadWholePageWithAttributes() {
        String exp;
        char attr = '!';

        exp = header(1, 8) +
              header(2, 8) + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());

        screen.setCursorAddress(1, 8);
        assertTrue(screen.cursorWrite('a'));
        assertTrue(screen.cursorWrite('p'));
        assertTrue(screen.cursorWrite('a'));

        screen.setBufferAddress(1, 11);
        screen.setAttribute(attr & 0x1f);
        screen.setCursorAddress(1, 12);
        assertTrue(screen.cursorWrite('e'));
        assertTrue(screen.cursorWrite('p'));
        assertTrue(screen.cursorWrite('a'));

        exp = header(1, 8) + "apa" + ESC + "6" + attr + "epa" +
              header(2, 8) + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());

        screen.setCursorAddress(2, 8);
        assertTrue(screen.cursorWrite('f'));
        assertTrue(screen.cursorWrite('o'));
        assertTrue(screen.cursorWrite('d'));
        assertTrue(screen.cursorWrite('e'));
        assertTrue(screen.cursorWrite('r'));

        screen.setBufferAddress(2, 13);
        screen.setAttribute(attr & 0x1f);

        exp = header(1, 8) + "apa" + ESC + "6" + attr + "epa" +
              header(2, 8) + "foder" + ESC + "6" + attr + EOT;
        assertEquals(exp, screen.readWholePageOrBuffer());
    }

    public void testGetCharsAndAttribs() {
        /*      0   1   2   3   4   5   6   7   8   9  10  11  12  13  14 */
        int attribs1[] =
            {   0, 10, 10, 10, 10, 10, 10, 20, 20, 20, 20, 20, 20, 20, 20 };
        char chars1[] =
            { ' ',' ','d','j','u','r',':',' ',' ',' ',' ',' ',' ',' ',' ' };

        assertArrayEquals(attribs1, screen.getAttribs(1));
        assertArrayEquals(chars1, screen.getChars(1));

        screen.setCursorAddress(1, 8);
        assertTrue(screen.cursorWrite('a'));
        screen.setBufferAddress(1, 9);
        screen.setAttribute(11);

        screen.setCursorAddress(1, 11);
        assertTrue(screen.cursorWrite('b'));
        screen.setBufferAddress(1, 12);
        screen.setAttribute(21);

        /*      0   1   2   3   4   5   6   7   8   9  10  11  12  13  14 */
        int attribs2[] =
            {   0, 10, 10, 10, 10, 10, 10, 20, 20, 11, 11, 11, 21, 21, 21 };
        char chars2[] =
            { ' ',' ','d','j','u','r',':',' ','a',' ',' ','b',' ',' ',' ' };

        assertArrayEquals(attribs2, screen.getAttribs(1));
        assertArrayEquals(chars2, screen.getChars(1));
    }

    public void testAutoTab() {
        // Redefine field 5 to have auto tab enabled
        FieldAttributes attribs = new FieldAttributes();
        attribs.setAutoTab(true);
        screen.addField(p5, attribs);

        // Position cursor at next to last position in field
        screen.setCursorAddress(1, 13);
        assertTrue(screen.cursorWrite('X'));
        assertEquals(new Position(1, 14), screen.getCursorAddress());
        assertTrue(screen.cursorWrite('X'));
        assertEquals(new Position(2, 0), screen.getCursorAddress());
        // Ignore input when in this position
        screen.cursorWrite('X');
        assertEquals(new Position(2, 0), screen.getCursorAddress());

        screen.doCursorRight();
        // Cursor should be in first unprotected position of field 5
        assertEquals(new Position(2, 8), screen.getCursorAddress());

        // Move the cursor to next to last position in field 5
        screen.setCursorAddress(2, 13);
        assertTrue(screen.cursorWrite('X'));
        assertEquals(new Position(2, 14), screen.getCursorAddress());
        assertTrue(screen.cursorWrite('X'));
        // Now the cursor has moved to first unprotected position in
        // next unprotected field (field 1 because of wrap)
        assertEquals(new Position(1, 8), screen.getCursorAddress());
    }

    public void testCursorMovementOneField() {
        screen = new ProtectedScreen(ROWS, COLS, dataTypeTable);

        // Define one protected field and one unprotected field
        FieldAttributes a1= new FieldAttributes();
        a1.setProtect(true);
        p1 = new Position(1, 1);
        screen.addField(p1, a1);
        screen.setBufferAddress(1, 2);
        screen.bufferWrite('d');
        screen.bufferWrite('j');
        screen.bufferWrite('u');
        screen.bufferWrite('r');
        screen.bufferWrite(':');

        assertEquals(new Position(0, 0), screen.getCursorAddress());
        FieldAttributes a2= new FieldAttributes();
        p2 = new Position(1, 7);
        screen.addField(p2, a2);
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.setCursorAddress(0, 0);
        screen.doHTab();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.doHTab();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.doBackTab();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.doCursorHomeDown();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        // Set cursor below the protected field
        screen.setCursorAddress(2, 2);
        screen.doCursorUp();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.doCursorHome();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.setCursorAddress(ROWS - 1, 0);
        screen.doCursorDown();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.setCursorAddress(ROWS - 1, COLS - 1);
        screen.doCursorRight();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.cursorWrite('k');
        screen.cursorWrite('a');
        screen.cursorWrite('l');
        screen.cursorWrite('l');
        screen.cursorWrite('e');
        screen.setCursorAddress(1, 8);
        screen.cursorToLastCharInField();
        assertEquals(new Position(1, (8+5)), screen.getCursorAddress());
    }

    public void testCursorMovementTwoFields() {
        screen = new ProtectedScreen(ROWS, COLS, dataTypeTable);

        FieldAttributes a1= new FieldAttributes();
        a1.setProtect(true);
        p1 = new Position(1, 1);
        screen.addField(p1, a1);
        screen.setBufferAddress(1, 2);
        screen.bufferWrite('d');
        screen.bufferWrite('j');
        screen.bufferWrite('u');
        screen.bufferWrite('r');
        screen.bufferWrite(':');

        FieldAttributes a2= new FieldAttributes();
        p2 = new Position(1, 7);
        screen.addField(p2, a2);

        FieldAttributes a3= new FieldAttributes();
        a3.setProtect(true);
        p3 = new Position(2, 0);
        screen.addField(p3, a3);

        FieldAttributes a4= new FieldAttributes();
        a4.setProtect(true);
        p4 = new Position(2, 1);
        screen.addField(p4, a4);
        screen.setBufferAddress(2, 2);
        screen.bufferWrite('m');
        screen.bufferWrite('a');
        screen.bufferWrite('t');
        screen.bufferWrite(':');

        FieldAttributes a5= new FieldAttributes();
        p5 = new Position(2, 7);
        screen.addField(p5, a5);

        screen.setCursorAddress(0, 0);
        assertEquals(new Position(1, 8), screen.getCursorAddress());
        screen.doHTab();
        assertEquals(new Position(2, 8), screen.getCursorAddress());
        screen.doHTab();
        assertEquals(new Position(1, 8), screen.getCursorAddress());

        screen.doBackTab();
        assertEquals(new Position(2, 8), screen.getCursorAddress());
        screen.doBackTab();
        assertEquals(new Position(1, 8), screen.getCursorAddress());
    }

    public void testClearMemory() {
        screen = new ProtectedScreen(ROWS, COLS, dataTypeTable);

        // Define one protected field and one unprotected field
        FieldAttributes a1= new FieldAttributes();
        a1.setProtect(true);
        p1 = new Position(1, 1);
        screen.addField(p1, a1);

        FieldAttributes a2= new FieldAttributes();
        p2 = new Position(1, 7);
        screen.addField(p2, a2);

        screen.reset();

        screen.doClearMemoryToSpaces(0, 0, ROWS - 1, COLS - 1);
    }
}

