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

import com.mindbright.terminal.DisplayView;
import com.mindbright.terminal.DisplayModel;

import com.mindbright.terminal.tandem6530.VideoAttributeMap;

public class TestVideoAttributeMap extends TestCase implements DisplayView {
    static final int COLS           = 80;
    static final int TOTAL_ROWS     = 48;
    static final int DEFAULT_ATTRIB = 10;

    protected int dirtyTop;
    protected int dirtyLeft;
    protected int dirtyBottom;
    protected int dirtyRight;

    protected VideoAttributeMap map;

    protected void assertNoDirt() {
        assertDirtEquals(-1, -1, -1, -1);
    }
    protected void assertDirtEquals(int top, int left, int bottom, int right) {
        if (top == dirtyTop && left == dirtyLeft && bottom == dirtyBottom &&
                right == dirtyRight) {
            return;
        }

        throw new AssertionFailedError("Expected dirt to be " +
                                       top + "," + left + "-" + bottom + "," + right + " but was " +
                                       dirtyTop + "," + dirtyLeft + "-" +
                                       dirtyBottom + "," + dirtyRight);
    }

    protected void assertAttribsEquals(int exp[], int visTop, int row) {
        int a[] = map.getAttribsAt(visTop, row);
        if (exp.length != a.length) {
            throw new AssertionFailedError("Length differs, expected " +
                                           exp.length + " but was " + a.length);
        }

        for (int i = 0; i < exp.length; i++) {
            if (exp[i] != a[i]) {
                throw new AssertionFailedError("Mismatch at i = " + i +
                                               ", expected " + exp[i] + " but was " + a[i]);
            }
        }
    }

    public void setUp() {
        map = new VideoAttributeMap(TOTAL_ROWS, COLS, DEFAULT_ATTRIB);
        resetDirt();
        map.setDisplay(this);
    }

    public void testAddAttributes() {
        assertEquals(DEFAULT_ATTRIB, map.attribAt(0, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 10));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10000, COLS));
        assertFalse(map.isAttrib(0, 0));
        assertNoDirt();

        int a1 = 0;
        map.add(10, 11, a1);
        assertEquals(DEFAULT_ATTRIB, map.attribAt(0, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 10));
        assertEquals(a1, map.attribAt(10, 11));
        assertEquals(a1, map.attribAt(11, 0));
        assertEquals(a1, map.attribAt(10000, COLS));

        assertFalse(map.isAttrib(0, 0));
        assertTrue(map.isAttrib(10, 11));
        assertDirtEquals(10, 11, TOTAL_ROWS, COLS);

        resetDirt();
        int a2 = 1;
        map.add(10, 15, a2);
        assertEquals(DEFAULT_ATTRIB, map.attribAt(0, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 10));
        assertEquals(a1, map.attribAt(10, 11));
        assertEquals(a1, map.attribAt(10, 14));
        assertEquals(a2, map.attribAt(10, 15));
        assertEquals(a2, map.attribAt(11, 0));
        assertEquals(a2, map.attribAt(10000, COLS));

        assertFalse(map.isAttrib(0, 0));
        assertTrue(map.isAttrib(10, 11));
        assertTrue(map.isAttrib(10, 15));
        assertDirtEquals(10, 15, TOTAL_ROWS, COLS);

        resetDirt();
        int a3 = 2;
        map.add(0, 0, a3);
        assertEquals(a3, map.attribAt(0, 0));
        assertEquals(a3, map.attribAt(10, 10));
        assertEquals(a1, map.attribAt(10, 11));
        assertEquals(a1, map.attribAt(10, 14));
        assertEquals(a2, map.attribAt(10, 15));
        assertEquals(a2, map.attribAt(11, 0));
        assertEquals(a2, map.attribAt(10000, COLS));

        assertTrue(map.isAttrib(0, 0));
        assertTrue(map.isAttrib(10, 11));
        assertTrue(map.isAttrib(10, 15));
        assertDirtEquals(0, 0, 11, 11);

        assertFalse(map.isAttrib(0, 1));
        assertFalse(map.isAttrib(10, 10));
        assertFalse(map.isAttrib(10, 12));
        assertFalse(map.isAttrib(10, 14));
        assertFalse(map.isAttrib(10, 16));
    }

    public void testAddAttributes2() {
        int a1 = 0;
        map.add(10, 15, a1);
        int a2 = 1;
        map.add(10, 11, a2);
        int a3 = 2;
        map.add(0, 0, a3);

        assertEquals(a3, map.attribAt(0, 0));
        assertEquals(a3, map.attribAt(10, 10));
        assertEquals(a2, map.attribAt(10, 11));
        assertEquals(a2, map.attribAt(10, 14));
        assertEquals(a1, map.attribAt(10, 15));
        assertEquals(a1, map.attribAt(11, 0));
        assertEquals(a1, map.attribAt(10000, COLS));
        assertTrue(map.isAttrib(0, 0));
        assertTrue(map.isAttrib(10, 11));
        assertTrue(map.isAttrib(10, 15));
    }

    public void testSetDefaultAttrib() {
        assertEquals(DEFAULT_ATTRIB, map.attribAt(0, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 10));

        int default1 = 4;
        map.setDefault(default1);
        assertEquals(default1, map.attribAt(0, 0));
        assertEquals(default1, map.attribAt(10, 10));
        assertDirtEquals(0, 0, TOTAL_ROWS, COLS);

        int a1 = 6;
        map.add(10, 10, a1);
        resetDirt();

        int default2 = 7;
        map.setDefault(default2);
        assertEquals(default2, map.attribAt(0, 0));
        assertEquals(a1, map.attribAt(10, 10));
        assertDirtEquals(0, 0, 11, 11);
    }

    public void testDeleteAttributes() {
        int a1 = 0;
        map.add(10, 11, a1);
        int a2 = 1;
        map.add(10, 15, a2);
        int a3 = 2;
        map.add(0, 0, a3);

        assertEquals(a3, map.attribAt(0, 0));
        assertEquals(a3, map.attribAt(10, 10));
        assertEquals(a1, map.attribAt(10, 11));
        assertEquals(a1, map.attribAt(10, 14));
        assertEquals(a2, map.attribAt(10, 15));
        assertEquals(a2, map.attribAt(11, 0));
        assertEquals(a2, map.attribAt(10000, COLS));

        resetDirt();
        map.delete(0,0);
        assertEquals(DEFAULT_ATTRIB, map.attribAt(0, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 10));
        assertEquals(a1, map.attribAt(10, 11));
        assertEquals(a1, map.attribAt(10, 14));
        assertEquals(a2, map.attribAt(10, 15));
        assertEquals(a2, map.attribAt(11, 0));
        assertEquals(a2, map.attribAt(10000, COLS));
        assertFalse(map.isAttrib(0, 0));
        assertTrue(map.isAttrib(10, 11));
        assertTrue(map.isAttrib(10, 15));
        assertDirtEquals(0, 0, 11, 12);


        resetDirt();
        map.delete(10,15);
        assertEquals(DEFAULT_ATTRIB, map.attribAt(0, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 10));
        assertEquals(a1, map.attribAt(10, 11));
        assertEquals(a1, map.attribAt(10, 14));
        assertEquals(a1, map.attribAt(10, 15));
        assertEquals(a1, map.attribAt(11, 0));
        assertEquals(a1, map.attribAt(10000, COLS));
        assertFalse(map.isAttrib(0, 0));
        assertTrue(map.isAttrib(10, 11));
        assertFalse(map.isAttrib(10, 15));
        assertDirtEquals(10, 15, TOTAL_ROWS, COLS);


        resetDirt();
        map.delete(10, 11);
        assertEquals(DEFAULT_ATTRIB, map.attribAt(0, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 10));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 11));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 14));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10, 15));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(11, 0));
        assertEquals(DEFAULT_ATTRIB, map.attribAt(10000, COLS));
        assertFalse(map.isAttrib(0, 0));
        assertFalse(map.isAttrib(10, 11));
        assertFalse(map.isAttrib(10, 15));
        assertDirtEquals(10, 11, TOTAL_ROWS, COLS);
    }

    public void testInsertChars() {
        int a1 = 0;
        map.add(9, 20, a1);
        int a2 = 1;
        map.add(10, 20, a2);
        int a3 = 2;
        map.add(10, COLS - 10, a3);
        int a4 = 3;
        map.add(11, 1, a4);

        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a2, map.attribAt(10, 20));
        assertEquals(a3, map.attribAt(10, COLS - 10));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));

        // The attributes at line 10 should be shifted one position to
        // the right
        resetDirt();
        map.insertChars(10, 19, 1);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 20));
        assertEquals(a2, map.attribAt(10, 21));
        assertEquals(a2, map.attribAt(10, COLS - 10));
        assertEquals(a3, map.attribAt(10, COLS - 9));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertDirtEquals(10, 20, 11, COLS - 9);

        // Insert enough characters for a3 to fall of the end of line
        resetDirt();
        map.insertChars(10, 21, 9);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 29));
        assertEquals(a2, map.attribAt(10, 30));
        assertEquals(a2, map.attribAt(10, COLS - 1));
        assertEquals(a2, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertDirtEquals(10, 21, 12, 2);

        // Insert enough characters for a4 to fall of the end of line
        resetDirt();
        map.insertChars(11, 0, 80);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 29));
        assertEquals(a2, map.attribAt(10, 30));
        assertEquals(a2, map.attribAt(10, COLS - 1));
        assertEquals(a2, map.attribAt(11, 0));
        assertEquals(a2, map.attribAt(11, 1));
        assertDirtEquals(11, 1, TOTAL_ROWS, COLS);
    }

    public void testDeleteChars() {
        int a1 = 0;
        map.add(9, 20, a1);
        int a2 = 1;
        map.add(10, 20, a2);
        int a3 = 2;
        map.add(10, COLS - 10, a3);
        int a4 = 3;
        map.add(11, 1, a4);

        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a2, map.attribAt(10, 20));
        assertEquals(a3, map.attribAt(10, COLS - 10));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));

        // The attributes at line 10 should be shifted one position to
        // the left
        resetDirt();
        map.deleteChars(10, 19, 1);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 18));
        assertEquals(a2, map.attribAt(10, 19));
        assertEquals(a3, map.attribAt(10, COLS - 11));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertDirtEquals(10, 19, 11, COLS - 9);

        // The a2 attribute should be deleted
        resetDirt();
        map.deleteChars(10, 19, 1);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 18));
        assertEquals(a1, map.attribAt(10, 19));
        assertEquals(a1, map.attribAt(10, COLS - 13));
        assertEquals(a3, map.attribAt(10, COLS - 12));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertDirtEquals(10, 19, 11, COLS - 10);

        // Delete the a3 attribute and make sure that the display is
        // repainted all the way to attribute a4
        resetDirt();
        map.deleteChars(10, 19, 60);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 18));
        assertEquals(a1, map.attribAt(10, 19));
        assertEquals(a1, map.attribAt(10, COLS - 13));
        assertEquals(a1, map.attribAt(10, COLS - 12));
        assertEquals(a1, map.attribAt(11, 0));
        assertDirtEquals(10, COLS - 12, 12, 2);

        // Delete the a4 attribute as well and make sure that the
        // display is repainted all the way to the bottom of the
        // buffer.
        resetDirt();
        map.deleteChars(11, 1, 1);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 18));
        assertEquals(a1, map.attribAt(10, 19));
        assertEquals(a1, map.attribAt(10, COLS - 13));
        assertEquals(a1, map.attribAt(10, COLS - 12));
        assertEquals(a1, map.attribAt(11, 0));
        assertEquals(a1, map.attribAt(TOTAL_ROWS - 1, COLS -1));
        assertDirtEquals(11, 1, TOTAL_ROWS, COLS);
    }

    public void testClearRight() {
        int a1 = 0;
        map.add(9, 20, a1);
        int a2 = 1;
        map.add(10, 20, a2);
        int a3 = 2;
        map.add(10, COLS - 10, a3);
        int a4 = 3;
        map.add(11, 1, a4);

        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a2, map.attribAt(10, 20));
        assertEquals(a3, map.attribAt(10, COLS - 10));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));

        resetDirt();
        map.clear(10, COLS - 5, COLS - 1);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a2, map.attribAt(10, 20));
        assertEquals(a3, map.attribAt(10, COLS - 10));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertNoDirt();

        resetDirt();
        map.clear(10, COLS - 10, COLS - 1);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a2, map.attribAt(10, 20));
        assertEquals(a2, map.attribAt(10, COLS - 10));
        assertEquals(a2, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertDirtEquals(10, COLS - 10, 12, 2);
    }

    public void testClearLeft() {
        int a1 = 0;
        map.add(9, 20, a1);
        int a2 = 1;
        map.add(10, 20, a2);
        int a3 = 2;
        map.add(10, COLS - 10, a3);
        int a4 = 3;
        map.add(11, 1, a4);

        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a2, map.attribAt(10, 20));
        assertEquals(a3, map.attribAt(10, COLS - 10));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));

        resetDirt();
        map.clear(10, 0, 19);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a2, map.attribAt(10, 20));
        assertEquals(a3, map.attribAt(10, COLS - 10));
        assertEquals(a3, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertNoDirt();

        resetDirt();
        map.clear(10, 0, COLS - 10);
        assertEquals(a1, map.attribAt(9, COLS - 1));
        assertEquals(a1, map.attribAt(10, 20));
        assertEquals(a1, map.attribAt(10, COLS - 10));
        assertEquals(a1, map.attribAt(11, 0));
        assertEquals(a4, map.attribAt(11, 1));
        assertDirtEquals(10, 20, 12, 2);
    }

    public void testInsertAndDeleteLine() {
        int a1 = 0;
        map.add( 9, 1, a1);
        int a2 = 1;
        map.add(10, 2, a2);
        int a3 = 2;
        map.add(11, 3, a3);
        int a4 = 3;
        map.add(12, 4, a4);

        assertEquals(a1, map.attribAt( 9, 1));
        assertEquals(a2, map.attribAt(10, 2));
        assertEquals(a3, map.attribAt(11, 3));
        assertEquals(a4, map.attribAt(12, 4));

        resetDirt();
        map.insertLine(8);
        assertEquals(a1, map.attribAt(10, 1));
        assertEquals(a2, map.attribAt(11, 2));
        assertEquals(a3, map.attribAt(12, 3));
        assertEquals(a4, map.attribAt(13, 4));
        assertTrue(map.isAttrib(10, 1));
        assertTrue(map.isAttrib(11, 2));
        assertTrue(map.isAttrib(12, 3));
        assertTrue(map.isAttrib(13, 4));
        assertDirtEquals(8, 0, TOTAL_ROWS, COLS);

        resetDirt();
        map.insertLine(12);
        assertEquals(a1, map.attribAt(10, 1));
        assertEquals(a2, map.attribAt(11, 2));
        assertEquals(a3, map.attribAt(13, 3));
        assertEquals(a4, map.attribAt(14, 4));
        assertTrue(map.isAttrib(10, 1));
        assertTrue(map.isAttrib(11, 2));
        assertTrue(map.isAttrib(13, 3));
        assertTrue(map.isAttrib(14, 4));
        assertDirtEquals(12, 0, TOTAL_ROWS, COLS);

        resetDirt();
        map.deleteLine(0);
        assertEquals(a1, map.attribAt( 9, 1));
        assertEquals(a2, map.attribAt(10, 2));
        assertEquals(a3, map.attribAt(12, 3));
        assertEquals(a4, map.attribAt(13, 4));
        assertTrue(map.isAttrib( 9, 1));
        assertTrue(map.isAttrib(10, 2));
        assertTrue(map.isAttrib(12, 3));
        assertTrue(map.isAttrib(13, 4));
        assertDirtEquals(0, 0, TOTAL_ROWS, COLS);

        // Delete the line with the a3 attribute
        resetDirt();
        map.deleteLine(12);
        assertEquals(a1, map.attribAt( 9, 1));
        assertEquals(a2, map.attribAt(10, 2));
        assertEquals(a2, map.attribAt(11, 3));
        assertEquals(a4, map.attribAt(12, 4));
        assertTrue(map.isAttrib( 9, 1));
        assertTrue(map.isAttrib(10, 2));
        assertFalse(map.isAttrib(11, 3));
        assertTrue(map.isAttrib(12, 4));
        assertDirtEquals(12, 0, TOTAL_ROWS, COLS);

        // Insert enough lines for the line with attribute a4 to
        // dissapear
        for (int i = 0; i < (TOTAL_ROWS - 12); i++) {
            map.insertLine(12);
        }
        assertEquals(a2, map.attribAt(TOTAL_ROWS - 1, 0));

        // Delete the same number of lines and check that a4 have not
        // come back
        for (int i = 0; i < (TOTAL_ROWS - 12); i++) {
            map.deleteLine(12);
        }
        assertEquals(a2, map.attribAt(TOTAL_ROWS - 1, 0));
    }

    public void testClearLine() {
        int a1 = 0;
        map.add( 9, 1, a1);
        int a2 = 1;
        map.add(10, 2, a2);
        int a3 = 2;
        map.add(11, 3, a3);
        int a4 = 3;
        map.add(11, 20, a4);
        int a5 = 4;
        map.add(12, 4, a5);

        assertEquals(a1, map.attribAt( 9, 1));
        assertEquals(a2, map.attribAt(10, 2));
        assertEquals(a3, map.attribAt(11, 3));
        assertEquals(a4, map.attribAt(11, 20));
        assertEquals(a5, map.attribAt(12, 4));

        resetDirt();
        map.clearLine(8);
        assertEquals(a1, map.attribAt( 9, 1));
        assertEquals(a2, map.attribAt(10, 2));
        assertEquals(a3, map.attribAt(11, 3));
        assertEquals(a4, map.attribAt(11, 20));
        assertEquals(a5, map.attribAt(12, 4));
        assertNoDirt();

        resetDirt();
        map.clearLine(11);
        assertEquals(a1, map.attribAt( 9, 1));
        assertEquals(a2, map.attribAt(10, 2));
        assertEquals(a2, map.attribAt(11, 3));
        assertEquals(a2, map.attribAt(11, 20));
        assertEquals(a5, map.attribAt(12, 4));
        assertDirtEquals(11, 3, 13, 5);

        resetDirt();
        map.clearLine(12);
        assertEquals(a1, map.attribAt( 9, 1));
        assertEquals(a2, map.attribAt(10, 2));
        assertEquals(a2, map.attribAt(11, 3));
        assertEquals(a2, map.attribAt(11, 20));
        assertEquals(a2, map.attribAt(12, 4));
        assertDirtEquals(12, 4, TOTAL_ROWS, COLS);
    }

    public void testResize() {
        int a1 = 0;
        map.add( 0, 1, a1);
        int a2 = 1;
        map.add(10, 30, a2);
        int a3 = 2;
        map.add(20, 60, a3);
        int a4 = 3;
        map.add(30, 50, a4);
        int a5 = 4;
        map.add(40, 79, a5);

        assertEquals(a1, map.attribAt( 0,  1));
        assertEquals(a2, map.attribAt(10, 30));
        assertEquals(a3, map.attribAt(20, 60));
        assertEquals(a4, map.attribAt(30, 50));
        assertEquals(a5, map.attribAt(40, 79));
        resetDirt();

        map.resize(35, 70);
        assertEquals(a1, map.attribAt( 0,  1));
        assertEquals(a2, map.attribAt(10, 30));
        assertEquals(a3, map.attribAt(20, 60));
        assertEquals(a4, map.attribAt(30, 50));
        assertEquals(a4, map.attribAt(40, 79));
        resetDirt();

        map.resize(35, 60);
        assertEquals(a1, map.attribAt( 0,  1));
        assertEquals(a2, map.attribAt(10, 30));
        assertEquals(a2, map.attribAt(20, 60));
        assertEquals(a4, map.attribAt(30, 50));
        assertEquals(a4, map.attribAt(40, 79));
        resetDirt();

        map.resize(TOTAL_ROWS, COLS);
        assertEquals(a1, map.attribAt( 0,  1));
        assertEquals(a2, map.attribAt(10, 30));
        assertEquals(a2, map.attribAt(20, 50));
        assertEquals(a4, map.attribAt(30, 60));
        assertEquals(a4, map.attribAt(40, 79));
        resetDirt();
    }

    public void testGetAttribs() {
        // Make the map small, to avoid too much typing
        map.resize(10, 10);

        int a1 = 0;
        map.add(0, 1, a1);
        int a2 = 1;
        map.add(5, 3, a2);
        int a3 = 2;
        map.add(5, 5, a3);
        int a4 = 3;
        map.add(5, 7, a4);

        int exp1[] = { DEFAULT_ATTRIB, a1, a1, a1, a1, a1, a1, a1, a1, a1};
        assertAttribsEquals(exp1, 0, 0);

        int exp2[] = { DEFAULT_ATTRIB, DEFAULT_ATTRIB, DEFAULT_ATTRIB,
                       DEFAULT_ATTRIB, DEFAULT_ATTRIB, DEFAULT_ATTRIB,
                       DEFAULT_ATTRIB, DEFAULT_ATTRIB, DEFAULT_ATTRIB,
                       DEFAULT_ATTRIB };
        assertAttribsEquals(exp2, 1, 0);

        int exp3[] = { a1, a1, a1, a2, a2, a3, a3, a4, a4, a4 };
        assertAttribsEquals(exp3, 0, 5);

        int exp4[] = { DEFAULT_ATTRIB, DEFAULT_ATTRIB, DEFAULT_ATTRIB,
                       a2, a2, a3, a3, a4, a4, a4 };
        assertAttribsEquals(exp4, 1, 4);

        assertAttribsEquals(exp2, 6, 0);
    }

    public void testReplace() {
        int a1 = 0;
        map.add(10, 30, a1);
        assertEquals(a1, map.attribAt(10, 30));
        assertEquals(a1, map.attribAt(10, 31));

        int a2 = 1;
        map.add(10, 30, a2);
        assertEquals(a2, map.attribAt(10, 30));
        assertEquals(a2, map.attribAt(10, 31));
    }


    protected void resetDirt() {
        dirtyTop = -1;
        dirtyLeft = -1;
        dirtyBottom = -1;
        dirtyRight = -1;
    }

    protected boolean isLarger(int row1, int col1, int row2, int col2) {
        return (row1*COLS + col1) > (row2*COLS + col2);
    }

    protected boolean isSmaller(int row1, int col1, int row2, int col2) {
        return (row1*COLS + col1) < (row2*COLS + col2);
    }


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
        // Increase the dirty area
        if (dirtyTop == -1) {
            dirtyTop = top;
            dirtyLeft = left;
            dirtyBottom = bottom;
            dirtyRight = right;
        } else {
            if (isSmaller(top, left, dirtyTop, dirtyLeft)) {
                dirtyTop = top;
                dirtyLeft = left;
            }
            if (isLarger(bottom, right, dirtyBottom, dirtyRight)) {
                dirtyBottom = bottom;
                dirtyRight = right;
            }
        }
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

