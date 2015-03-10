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

import com.mindbright.terminal.tandem6530.Field;
import com.mindbright.terminal.tandem6530.FieldAttributes;
import com.mindbright.terminal.tandem6530.FieldMap;
import com.mindbright.terminal.tandem6530.Position;
import com.mindbright.terminal.tandem6530.DataType;

public class TestFieldMap extends TestCase {
    private static final int ROWS   = 10;
    private static final int COLS   = 15;

    private FieldMap map ;
    private Position HOME = new Position(0, 0);
    private Position END = new Position(ROWS - 1, COLS - 1);

    public void setUp() {}

    protected Position compareRow(int row, int start, int end, Field field) {
        Position tmp = new Position(row, start);
        for (int i = start; i < end; i++) {
            if (map.getFieldAt(tmp.setCol(i)) != field) {
                return tmp;
            }
        }
        return null;
    }

    protected void assertMapContainsField(Position start, Position end,
                                          Field field) {
        Position badPos;
        if (start.getRow() == end.getRow()) {
            badPos = compareRow(start.getRow(), start.getCol(),
                                end.getCol(), field);
            if (badPos != null) {
                throw new AssertionFailedError(
                    "field did not match at " + badPos);
            }
        } else {
            badPos = compareRow(start.getRow(), start.getCol(), COLS-1, field);
            if (badPos != null) {
                throw new AssertionFailedError(
                    "field did not match at " + badPos);
            }

            for (int i = start.getRow() + 1; i < end.getRow(); i++) {
                badPos = compareRow(i, 0, COLS - 1, field);
                if (badPos != null) {
                    throw new AssertionFailedError(
                        "field did not match at " + badPos);
                }
            }

            badPos = compareRow(end.getRow(), 0, end.getCol(), field);
            if (badPos != null) {
                throw new AssertionFailedError(
                    "field did not match at " + badPos);
            }

        }
    }

    public void testAddFields() {
        Position pos = new Position(HOME);

        // Add first field (covers whole screen)
        map = new FieldMap(ROWS, COLS, new FieldAttributes(), new DataType());
        Field field1 = map.getFieldAt(pos);
        Position start1 = new Position(HOME);
        Position end1 = new Position(END);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);

        assertSame(map.nextField(HOME), field1);
        assertSame(map.prevField(HOME), field1);

        // Add second field
        pos.set(2, 0);
        map.addField(pos, new FieldAttributes());
        Field field2 = map.getFieldAt(pos);
        assertNotSame(field1, field2);

        end1 = new Position(1, COLS - 1);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);

        Position start2 = new Position(2, 0);
        Position end2 = new Position(END);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);

        assertSame(map.prevField(start1), field1);
        assertSame(map.nextField(start1), field2);

        assertSame(map.prevField(start2), field1);
        assertSame(map.nextField(start2), field2);

        // Add third field
        pos.set(5, ROWS - 1);
        map.addField(pos, new FieldAttributes());
        Field field3 = map.getFieldAt(pos);
        assertNotSame(field1, field2);
        assertNotSame(field2, field3);

        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);

        end2 = new Position(5, ROWS - 2);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);

        Position start3 = new Position(5, ROWS - 1);
        Position end3 = new Position(END);
        assertEquals(start3, field3.getStart());
        assertEquals(end3, field3.getEnd());
        assertMapContainsField(start3, end3, field3);


        assertSame(map.prevField(start1), field1);
        assertSame(map.nextField(start1), field2);

        assertSame(map.prevField(start2), field1);
        assertSame(map.nextField(start2), field3);

        assertSame(map.prevField(start3), field2);
        assertSame(map.nextField(start3), field3);
    }

    public void testInsertRows() {
        map = new FieldMap(ROWS, COLS, new FieldAttributes(), new DataType());

        Position start1 = new Position(HOME);
        Position end1 = new Position(1, COLS - 1);
        Position start2 = new Position(2, 0);
        Position end2 = new Position(5, COLS - 2);
        Position start3 = new Position(5, COLS - 1);
        Position end3 = new Position(END);

        map.addField(start2, new FieldAttributes());
        map.addField(start3, new FieldAttributes());
        Field field1 = map.getFieldAt(start1);
        Field field2 = map.getFieldAt(start2);
        Field field3 = map.getFieldAt(start3);


        // Insert a row
        map.insertRow(1);

        // Test that all positions have moved one line down
        end1 = new Position(2, COLS - 1);
        start2 = new Position(3, 0);
        end2 = new Position(6, COLS - 2);
        start3 = new Position(6, COLS - 1);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);
        assertEquals(start3, field3.getStart());
        assertEquals(end3, field3.getEnd());
        assertMapContainsField(start3, end3, field3);

        // Insert enough rows for field3 start position to be
        // "outshifted"
        for (int i = 0; i < ROWS - 6; i++) {
            map.insertRow(5);
        }

        end2 = new Position(END);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);

        // insert a row last, nothing should change
        map.insertRow(ROWS - 1);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);

        // Insert a row first
        map.insertRow(0);
        end1 = new Position(3, COLS - 1);
        start2 = new Position(4, 0);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);
    }

    public void testDeleteRows() {
        map = new FieldMap(ROWS, COLS, new FieldAttributes(), new DataType());

        Position start1 = new Position(HOME);
        Position end1 = new Position(1, COLS - 1);
        Position start2 = new Position(2, 0);
        Position end2 = new Position(5, COLS - 2);
        Position start3 = new Position(5, COLS - 1);
        Position end3 = new Position(END);

        map.addField(start2, new FieldAttributes());
        map.addField(start3, new FieldAttributes());
        Field field1 = map.getFieldAt(start1);
        Field field2 = map.getFieldAt(start2);
        Field field3 = map.getFieldAt(start3);

        // Delete a row
        map.deleteRow(1);

        // Test that all positions have moved one line up
        end1 = new Position(0, COLS - 1);
        start2 = new Position(1, 0);
        end2 = new Position(4, COLS - 2);
        start3 = new Position(4, COLS - 1);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);
        assertEquals(start3, field3.getStart());
        assertEquals(end3, field3.getEnd());
        assertMapContainsField(start3, end3, field3);

        // Delete three more rows, field 2 should be totaly deleted
        map.deleteRow(1);
        map.deleteRow(1);
        map.deleteRow(1);
        map.deleteRow(1);
        end1 = new Position(0, COLS - 1);
        start3 = new Position(1, 0);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start3, field3.getStart());
        assertEquals(end3, field3.getEnd());
        assertMapContainsField(start3, end3, field3);
    }

    public void testClearAreaFirstField() {
        map = new FieldMap(ROWS, COLS, new FieldAttributes(), new DataType());

        Position start1 = new Position(HOME);
        Position end1 = new Position(1, COLS - 1);
        Position start2 = new Position(2, 0);
        Position end2 = new Position(4, COLS - 2);
        Position start3 = new Position(4, COLS - 1);
        Position end3 = new Position(END);

        map.addField(start2, new FieldAttributes());
        map.addField(start3, new FieldAttributes());
        Field field1 = map.getFieldAt(start1);
        Field field2 = map.getFieldAt(start2);
        Field field3 = map.getFieldAt(start3);

        // field1 should be deleted
        Position clearStart = new Position(0, 0);
        Position clearEnd = new Position(0, 5);
        map.clear(clearStart, clearEnd);

        start2 = new Position(HOME);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
        assertMapContainsField(start2, end2, field2);
        assertEquals(start3, field3.getStart());
        assertEquals(end3, field3.getEnd());
        assertMapContainsField(start3, end3, field3);
    }

    public void testClearAreaMiddleField() {
        map = new FieldMap(ROWS, COLS, new FieldAttributes(), new DataType());

        Position start1 = new Position(HOME);
        Position end1 = new Position(1, COLS - 1);
        Position start2 = new Position(2, 0);
        Position end2 = new Position(4, COLS - 2);
        Position start3 = new Position(4, COLS - 1);
        Position end3 = new Position(END);

        map.addField(start2, new FieldAttributes());
        map.addField(start3, new FieldAttributes());
        Field field1 = map.getFieldAt(start1);
        Field field2 = map.getFieldAt(start2);
        Field field3 = map.getFieldAt(start3);

        // field2 should be deleted
        Position clearStart = new Position(0, 12);
        Position clearEnd = new Position(3, 9);
        map.clear(clearStart, clearEnd);

        end1 = end2;
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start3, field3.getStart());
        assertEquals(end3, field3.getEnd());
        assertMapContainsField(start3, end3, field3);

    }

    public void testClearAreaLastField() {
        map = new FieldMap(ROWS, COLS, new FieldAttributes(), new DataType());

        Position start1 = new Position(HOME);
        Position end1 = new Position(1, COLS - 1);
        Position start2 = new Position(2, 0);
        Position end2 = new Position(4, COLS - 2);
        Position start3 = new Position(4, COLS - 1);
        Position end3 = new Position(END);

        map.addField(start2, new FieldAttributes());
        map.addField(start3, new FieldAttributes());
        Field field1 = map.getFieldAt(start1);
        Field field2 = map.getFieldAt(start2);
        Field field3 = map.getFieldAt(start3);

        // field3 should be deleted
        Position clearStart = new Position(4, 12);
        Position clearEnd = new Position(4, 14);
        map.clear(clearStart, clearEnd);

        end2 = new Position(END);
        assertEquals(start1, field1.getStart());
        assertEquals(end1, field1.getEnd());
        assertMapContainsField(start1, end1, field1);
        assertEquals(start2, field2.getStart());
        assertEquals(end2, field2.getEnd());
    }

    public void testReset() {
        map = new FieldMap(ROWS, COLS, new FieldAttributes(), new DataType());

        Position start1 = new Position(HOME);
        Position end1 = new Position(1, COLS - 1);
        Position start2 = new Position(2, 0);
        Position end2 = new Position(4, COLS - 2);
        Position start3 = new Position(4, COLS - 1);
        Position end3 = new Position(END);

        map.addField(start2, new FieldAttributes());
        map.addField(start3, new FieldAttributes());
        Field field1 = map.getFieldAt(start1);
        Field field2 = map.getFieldAt(start2);
        Field field3 = map.getFieldAt(start3);

        map.reset(new FieldAttributes());

        // Test that a new default field covers the entire screen
        Field newDefault = map.getFieldAt(HOME);
        assertNotSame(newDefault, field1);
        assertSame(newDefault, map.getFieldAt(END));
    }
}

