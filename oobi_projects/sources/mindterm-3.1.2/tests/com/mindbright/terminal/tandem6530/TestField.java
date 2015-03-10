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
import com.mindbright.terminal.tandem6530.Field;
import com.mindbright.terminal.tandem6530.FieldAttributes;

public class TestField extends TestCase implements AsciiCodes {
    int COLS = 10;
    int ROWS = 10;

    DataType dataTypeTable;

    public void setUp() {
        dataTypeTable = new DataType();
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


    protected Field createField(Position start, Position end,
                                FieldAttributes attribs) {
        Field f = new Field(COLS, start, end, attribs, dataTypeTable);

        char c = 0;
        Position cursor = new Position(start).incCol(ROWS, COLS);

        while (!end.equals(cursor)) {
            f.writeCursorChar(cursor, (char) ('a' + c));
            cursor.incCol(ROWS, COLS);
            c = (char) ((c + 1) % 32);
        }
        f.writeCursorChar(cursor, (char) ('a' + c));
        return f;
    }

    public void testWriteCursorChar() {
        Position cursor = new Position(0, 0);
        Position start = new Position(0, 0);
        Position end = new Position(0, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = new Field(80, start, end, attribs, dataTypeTable);

        assertFalse(f.getMdt());
        assertFalse("Manage to write into field-start address",
                    f.writeCursorChar(cursor, 'a'));
        assertFalse(f.getMdt());
        cursor.setCol(5);
        assertFalse("Mange to write outside field",
                    f.writeCursorChar(cursor, 'X'));
        assertFalse(f.getMdt());

        cursor.setCol(1);
        assertTrue(f.writeCursorChar(cursor, 'a'));
        assertTrue(f.getMdt());
        cursor.setCol(2);
        assertTrue(f.writeCursorChar(cursor, 'p'));
        cursor.setCol(3);
        assertTrue(f.writeCursorChar(cursor, 'a'));
        assertEquals("apa", f.getContents());

        attribs = new FieldAttributes();
        attribs.setUpShift(true);
        f.redefine(attribs);
        cursor.setCol(1);
        assertTrue(f.writeCursorChar(cursor, 'a'));
        cursor.setCol(2);
        assertTrue(f.writeCursorChar(cursor, 'p'));
        cursor.setCol(3);
        assertTrue(f.writeCursorChar(cursor, 'a'));
        cursor.setCol(4);
        assertTrue(f.writeCursorChar(cursor, 'n'));
        assertEquals("APAN", f.getContents());

        attribs = new FieldAttributes();
        attribs.setDataType(DataType.ALPHA);
        attribs.setMdt(true);
        f.redefine(attribs);

        assertTrue(f.getMdt());
        f.resetMdt();
        assertFalse(f.getMdt());

        cursor.setCol(1);
        assertFalse("Don't care about data types",
                    f.writeCursorChar(cursor, '1'));
        assertFalse(f.getMdt());
        assertEquals("APAN", f.getContents());
    }

    public void testWriteBufferChar() {
        Position cursor = new Position(0, 0);
        Position start = new Position(0, 0);
        Position end = new Position(0, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = new Field(80, start, end, attribs, dataTypeTable);

        assertFalse(f.getMdt());
        assertTrue("Fail to write into field-start address",
                   f.writeBufferChar(cursor, 'a'));
        assertFalse(f.getMdt());

        cursor.setCol(0);
        f.writeBufferChar(cursor, ' ');

        cursor.setCol(5);
        assertFalse("Mange to write outside field",
                    f.writeBufferChar(cursor, 'X'));
        assertFalse(f.getMdt());

        cursor.setCol(1);
        assertTrue(f.writeBufferChar(cursor, 'a'));
        assertFalse("MDT set for buffer writes", f.getMdt());
        cursor.setCol(2);
        assertTrue(f.writeBufferChar(cursor, 'p'));
        cursor.setCol(3);
        assertTrue(f.writeBufferChar(cursor, 'a'));
        assertEquals("apa", f.getContents());

        attribs = new FieldAttributes();
        attribs.setUpShift(true);
        f.redefine(attribs);
        cursor.setCol(1);
        assertTrue(f.writeBufferChar(cursor, 'a'));
        cursor.setCol(2);
        assertTrue(f.writeBufferChar(cursor, 'p'));
        cursor.setCol(3);
        assertTrue(f.writeBufferChar(cursor, 'a'));
        assertEquals("Upshift affected buffer write", "apa", f.getContents());

        attribs = new FieldAttributes();
        attribs.setDataType(DataType.ALPHA);
        attribs.setMdt(true);
        f.redefine(attribs);

        assertTrue(f.getMdt());
        f.resetMdt();
        assertFalse(f.getMdt());

        cursor.setCol(1);
        assertTrue("Data types affected buffer write",
                   f.writeBufferChar(cursor, '1'));
        assertFalse(f.getMdt());
        assertEquals("1pa", f.getContents());
    }

    public void testInsertDelete() {
        Position cursor = new Position(0, 0);
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);
        f.setAttrib(new Position(4, 5), 1);

        char expLine3[] =                             { ' ','a','b' };
        char expLine4[] = { 'c','d','e','f','g',' ','i','j','k','l' };
        char expLine5[] = { 'm','n','o','p','q' };
        int  attribs3[] =                             {   0,  0,  0 };
        int  attribs4[] = {   0,  0,  0,  0,  0,  1,  1,  1,  1,  1 };
        int  attribs5[] = {   1,  1,  1,  1,  1 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(attribs3, f.getAttribs(3));
        assertArrayEquals(attribs4, f.getAttribs(4));
        assertArrayEquals(attribs5, f.getAttribs(5));


        cursor.set(4, 4);
        f.insertChar(cursor);

        char expLine3_2[] =                             { ' ','a','b' };
        char expLine4_2[] = { 'c','d','e','f',' ','g',' ','i','j','k' };
        char expLine5_2[] = { 'l','m','n','o','p' };
        int  attribs3_2[] =                             {   0,  0,  0 };
        int  attribs4_2[] = {   0,  0,  0,  0,  0,  0,  1,  1,  1,  1 };
        int  attribs5_2[] = {   1,  1,  1,  1,  1 };
        assertArrayEquals(expLine3_2, f.getChars(3));
        assertArrayEquals(expLine4_2, f.getChars(4));
        assertArrayEquals(expLine5_2, f.getChars(5));
        assertArrayEquals(attribs3_2, f.getAttribs(3));
        assertArrayEquals(attribs4_2, f.getAttribs(4));
        assertArrayEquals(attribs5_2, f.getAttribs(5));

        cursor.set(3, 9);
        f.deleteChar(cursor);
        char expLine3_3[] =                             { ' ','a','c' };
        char expLine4_3[] = { 'd','e','f',' ','g',' ','i','j','k','l' };
        char expLine5_3[] = { 'm','n','o','p',' ' };
        int  attribs3_3[] =                             {   0,  0,  0 };
        int  attribs4_3[] = {   0,  0,  0,  0,  0,  1,  1,  1,  1,  1 };
        int  attribs5_3[] = {   1,  1,  1,  1,  1 };
        assertArrayEquals(expLine3_3, f.getChars(3));
        assertArrayEquals(expLine4_3, f.getChars(4));
        assertArrayEquals(expLine5_3, f.getChars(5));
        assertArrayEquals(attribs3_3, f.getAttribs(3));
        assertArrayEquals(attribs4_3, f.getAttribs(4));
        assertArrayEquals(attribs5_3, f.getAttribs(5));

        // Test overwrite attribute char
        cursor.set(4, 5);
        f.writeCursorChar(cursor, 'h');
        char expLine3_4[] =                             { ' ','a','c' };
        char expLine4_4[] = { 'd','e','f',' ','g','h','i','j','k','l' };
        char expLine5_4[] = { 'm','n','o','p',' ' };
        int  attribs3_4[] =                             {   0,  0,  0 };
        int  attribs4_4[] = {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
        int  attribs5_4[] = {   0,  0,  0,  0,  0 };
        assertArrayEquals(expLine3_4, f.getChars(3));
        assertArrayEquals(expLine4_4, f.getChars(4));
        assertArrayEquals(expLine5_4, f.getChars(5));
        assertArrayEquals(attribs3_4, f.getAttribs(3));
        assertArrayEquals(attribs4_4, f.getAttribs(4));
        assertArrayEquals(attribs5_4, f.getAttribs(5));
    }

    public void testGetContents() {
        Position cursor = new Position(0, 0);
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        char attr = '$';
        Field f = createField(start, end, attribs);
        f.setAttrib(new Position(4, 5), attr & 0x1f);

        String exp = "ab" + "cdefg"+ESC+"6"+attr+"ijkl" + "mnopq";
        assertEquals(exp, f.getContents());
    }

    public void testGetContentsSpaceFilled() {
        Position start = new Position(3, 0);
        Position end = new Position(3, 7);
        FieldAttributes attribs = new FieldAttributes();

        Field f = new Field(COLS, start, end, attribs, dataTypeTable);

        // Fill field with space
        Position p = new Position(3, 1);
        for (int i = 0; i < 6; i++) {
            f.writeBufferChar(p, ' ');
            p.incCol(ROWS, COLS);
        }

        // Write "apa" in the middle, with spaces on both sides.
        p.setCol(2);
        f.writeBufferChar(p, 'a');
        p.incCol(ROWS, COLS);
        f.writeBufferChar(p, 'p');
        p.incCol(ROWS, COLS);
        f.writeBufferChar(p, 'a');

        String exp = " apa";
        assertEquals(exp, f.getContents());
    }



    public void testDisplayOneRows() {
        Position start = new Position(3, 2);
        Position end = new Position(3, 7);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);

        char expLine3[] = { ' ', 'a', 'b', 'c', 'd', 'e' };
        int attribs3[] = {    0,   0,   0,   0,   0,   0 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(attribs3, f.getAttribs(3));

        f.setAttrib(new Position(3, 4), 1);

        char expLine3_2[] = { ' ', 'a', ' ', 'c', 'd', 'e' };
        int attribs3_2[] = {    0,   0,   1,   1,   1,   1 };
        assertArrayEquals(expLine3_2, f.getChars(3));
        assertArrayEquals(attribs3_2, f.getAttribs(3));
    }

    public void testDisplayManyRows() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);

        char expLine3[] =                             { ' ','a','b' };
        char expLine4[] = { 'c','d','e','f','g','h','i','j','k','l' };
        char expLine5[] = { 'm','n','o','p','q' };
        int  attribs3[] =                             {   0,  0,  0 };
        int  attribs4[] = {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
        int  attribs5[] = {   0,  0,  0,  0,  0 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(attribs3, f.getAttribs(3));
        assertArrayEquals(attribs4, f.getAttribs(4));
        assertArrayEquals(attribs5, f.getAttribs(5));

        f.setAttrib(new Position(3, 9), 1);
        f.setAttrib(new Position(4, 3), 2);
        f.setAttrib(new Position(4, 5), 3);
        f.setAttrib(new Position(5, 0), 0);

        char expLine3_2[] =                             { ' ','a',' ' };
        char expLine4_2[] = { 'c','d','e',' ','g',' ','i','j','k','l' };
        char expLine5_2[] = { ' ','n','o','p','q' };
        int  attribs3_2[] =                             {   0,  0,  1 };
        int  attribs4_2[] = {   1,  1,  1,  2,  2,  3,  3,  3,  3,  3 };
        int  attribs5_2[] = {   0,  0,  0,  0,  0 };
        assertArrayEquals(expLine3_2, f.getChars(3));
        assertArrayEquals(expLine4_2, f.getChars(4));
        assertArrayEquals(expLine5_2, f.getChars(5));
        assertArrayEquals(attribs3_2, f.getAttribs(3));
        assertArrayEquals(attribs4_2, f.getAttribs(4));
        assertArrayEquals(attribs5_2, f.getAttribs(5));
    }

    public void testResizeDeleteRow() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);
        f.setAttrib(new Position(4, 4), 1);

        end.decRow(COLS, ROWS);
        f.setPosition(start, end); // delete row 4
        char expLine3[] =                                    { ' ', 'a', 'b' };
        char expLine4[] = { 'c', 'd', 'e', 'f', ' ' };
        int  attribs3[] =                                    {   0,   0,   0 };
        int  attribs4[] = {   0,   0,   0,   0,   1 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(attribs3, f.getAttribs(3));
        assertArrayEquals(attribs4, f.getAttribs(4));
    }

    public void testResizeDeleteCharsAtEnd() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);

        end.set(5, 1);
        f.setPosition(start, end); // delete row 4
        char expLine3[] =                                    { ' ', 'a', 'b' };
        char expLine4[] = { 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
        char expLine5[] = { 'm', 'n' };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
    }

    public void testResizeInsertRowLast() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);

        end.incRow(COLS, ROWS);
        f.setPosition(start, end); // add new row last

        char expLine3[] =                                    { ' ', 'a', 'b' };
        char expLine4[] = { 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
        char expLine5[] = { 'm', 'n', 'o', 'p', 'q', ' ', ' ', ' ', ' ', ' ' };
        char expLine6[] = { ' ', ' ', ' ', ' ', ' ' };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(expLine6, f.getChars(6));
    }

    public void testResizeInsertRowFirst() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);

        start.decRow(COLS, ROWS);
        f.setPosition(start, end); // add new row last

        char expLine2[] =                                    { ' ', 'a', 'b' };
        char expLine3[] = { 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
        char expLine4[] = { 'm', 'n', 'o', 'p', 'q', ' ', ' ', ' ', ' ', ' ' };
        char expLine5[] = { ' ', ' ', ' ', ' ', ' ' };
        assertArrayEquals(expLine2, f.getChars(2));
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
    }

    public void testClearAt() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);
        f.setAttrib(new Position(5, 1), 1);

        Position p = new Position(5, 0);
        while (p.le(end)) {
            f.clearAt(p);
            p.incCol(ROWS, COLS);
        }

        char expLine3[] =                                    { ' ', 'a', 'b' };
        char expLine4[] = { 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
        char expLine5[] = { ' ', ' ', ' ', ' ', ' ' };
        int  attribs5[] = {   0,   0,   0,   0,   0 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(attribs5, f.getAttribs(5));
    }

    public void testClearField() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);
        f.setAttrib(new Position(5, 1), 1);
        f.clearField();

        char expLine3[] =                                    { ' ', ' ', ' ' };
        char expLine4[] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };
        char expLine5[] = { ' ', ' ', ' ', ' ', ' ' };
        int  attribs5[] = {   0,   0,   0,   0,   0 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(attribs5, f.getAttribs(5));
    }

    public void testClearToEnd() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = createField(start, end, attribs);

        Position p = new Position(5, 0);
        f.setAttrib(new Position(5, 2), 1);
        f.clearToEnd(p);

        char expLine3[] =                                    { ' ', 'a', 'b' };
        char expLine4[] = { 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
        char expLine5[] = { ' ', ' ', ' ', ' ', ' ' };
        int  attribs5[] = {   0,   0,   0,   0,   0 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(attribs5, f.getAttribs(5));
    }

    public void testSetLength() {
        Position start = new Position(3, 7);
        Position end = new Position(5, 4);
        FieldAttributes attribs = new FieldAttributes();

        Field f = new Field(COLS, start, end, attribs, dataTypeTable);

        char expLine3[] =                             { ' ',' ',' ' };
        char expLine4[] = { ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
        char expLine5[] = { ' ',' ',' ',' ',' ' };
        int  attribs3[] =                             {   0,  0,  0 };
        int  attribs4[] = {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
        int  attribs5[] = {   0,  0,  0,  0,  0 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(attribs3, f.getAttribs(3));
        assertArrayEquals(attribs4, f.getAttribs(4));
        assertArrayEquals(attribs5, f.getAttribs(5));

        f.setStart(new Position(3, 8));
        f.setEnd(new Position(5, 3));

        expLine3 = new char[]                                 { ' ',' ' };
        expLine4 = new char[] { ' ',' ',' ',' ',' ',' ',' ',' ',' ',' ' };
        expLine5 = new char[] { ' ',' ',' ',' ' };
        attribs3 = new int[]                                   { 0,  0 };
        attribs4 = new int[] {   0,  0,  0,  0,  0,  0,  0,  0,  0,  0 };
        attribs5 = new int[] {   0,  0,  0,  0 };
        assertArrayEquals(expLine3, f.getChars(3));
        assertArrayEquals(expLine4, f.getChars(4));
        assertArrayEquals(expLine5, f.getChars(5));
        assertArrayEquals(attribs3, f.getAttribs(3));
        assertArrayEquals(attribs4, f.getAttribs(4));
        assertArrayEquals(attribs5, f.getAttribs(5));
    }


}



