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

package com.mindbright.terminal.tandem6530;

public class FieldMap {
    protected int rows;
    protected int cols;
    protected Field map[][];
    protected Position HOME;
    protected Position END;
    protected DataType dataTypeTable;
    protected boolean haveUnprotectedFields = false;

    public FieldMap(int rows, int cols, FieldAttributes defaultFieldAttribs,
                    DataType dataTypeTable) {
        this.rows = rows;
        this.cols = cols;
        this.dataTypeTable = dataTypeTable;

        HOME = new Position(0, 0);
        END = new Position(rows - 1, cols - 1);

        map = new Field[rows][cols];
        Field field = new Field(cols, HOME, END, defaultFieldAttribs,
                                dataTypeTable);
        setField(HOME, END, field);
    }

    public void addField(Position p, FieldAttributes attribs) {
        Field oldField = getFieldAt(p);
        if (p.equals(oldField.getStart())) {
            // Start address is same as an existing field, redefine it
            oldField.redefine(attribs);
            return;
        }

        Position tmp = new Position(p);
        Field prev = prevField(p);
        Field next = nextField(p);

        Position oldFieldEnd = new Position(p).decCol(rows, cols);
        Position fieldEnd;
        if (next == oldField) {
            // oldField is the last field on screen
            fieldEnd = new Position(END);
        } else {
            fieldEnd = new Position(next.getStart()).decCol(rows, cols);
        }
        Field field = new Field(cols, p, fieldEnd, attribs, dataTypeTable);

        setField(field.getStart(), field.getEnd(), field);
        adjustFields();
    }

    public void reset(FieldAttributes defaultFieldAttribs) {
        Field field = new Field(cols, HOME, END, defaultFieldAttribs,
                                dataTypeTable);
        setField(HOME, END, field);
        adjustFields();
    }

    public boolean haveUnprotectedFields() {
        return haveUnprotectedFields;
    }

    protected void setField(Position start, Position end, Field field) {
        Position i = new Position(start);
        setFieldAt(i, field);
        while (!i.equals(end)) {
            i.incCol(rows, cols);
            setFieldAt(i, field);
        }
    }

    protected void setFieldAt(Position p, Field field) {
        map[p.getRow()][p.getCol()] = field;
    }

    public Field getFieldAt(Position p) {
        return map[p.getRow()][p.getCol()];
    }

    protected Position prevFieldEnd(Position p) {
        Field f = getFieldAt(p);
        Position pos = new Position(p).decCol(rows, cols);

        while (f == getFieldAt(pos) && !pos.equals(HOME)) {
            pos.decCol(rows, cols);
        }
        return pos;
    }

    public Field prevField(Position p) {
        return getFieldAt(prevFieldEnd(p));
    }

    protected Position nextFieldStart(Position p) {
        Field f = getFieldAt(p);
        Position pos = new Position(p).incCol(rows, cols);

        while (f == getFieldAt(pos) && !pos.equals(END)) {
            pos.incCol(rows, cols);
        }
        return pos;
    }

    public Field nextField(Position p) {
        return getFieldAt(nextFieldStart(p));
    }

    /** Set the fields start/end values according to the map.
     */
    protected void adjustFields() {
        Position fieldStart = new Position(HOME);
        Position fieldEnd = null;

        haveUnprotectedFields = false;
        while (!END.equals(fieldStart)) {
            fieldEnd = nextFieldStart(fieldStart);
            if (!END.equals(fieldEnd)) {
                fieldEnd.decCol(rows, cols);
            }

            getFieldAt(fieldStart).setPosition(fieldStart, fieldEnd);
            if (!getFieldAt(fieldStart).isProtected()) {
                haveUnprotectedFields = true;
            }

            fieldStart = nextFieldStart(fieldStart);
        }
    }

    public void insertRow(int row) {
        Position rowPos = new Position(row, 0);
        Field prev = getFieldAt(rowPos);

        if (rowPos.equals(prev.getStart())) {
            // This field started at rowPos, must choose the
            // previous field to be the one before that.
            prev = prevField(rowPos);
        }

        // Move down the fields below row, and let the previous field
        // occupy the new row.
        //
        // I don't want to leave the new row just blank, since the
        // whole point of a default field seems to be that all positions
        // on screen should belong to a field.
        System.arraycopy(map, row, map, row + 1, (rows - row - 1));
        setField(rowPos, new Position(row, cols - 1), prev);

        adjustFields();

    }

    public void deleteRow(int row) {
        Field lastField = getFieldAt(END);

        // Move all rows below row up, and let the last field fill up
        // the empty row at the bottom.
        System.arraycopy(map, row + 1, map, row, (rows - row - 1));
        setField(new Position(rows - 1, 0), END, lastField);

        adjustFields();
    }

    protected void clearFields(Position start, Position end) {
        Field fillField;

        if (HOME.equals(start)) {
            fillField = nextField(end);
            if (getFieldAt(start) == fillField) {
                // This field is the only one on screen, can't remove it
                return;
            }
        } else {
            fillField = getFieldAt(start);
            if (fillField.getStart().equals(start)) {
                fillField = prevField(start);
            }
        }

        Position p = new Position(start);
        Position fillEnd = getFieldAt(end).getEnd();
        while (p.le(end)) {
            if (getFieldAt(p).getStart().equals(p)) {
                // A field started inside the interval [start,end], replace
                // all positions until fillEnd with the fillField
                while (p.le(fillEnd)) {
                    setFieldAt(p, fillField);
                    if (p.equals(END)) {
                        break;
                    }
                    p.incCol(rows, cols);
                }
                break;
            }
            p.incCol(rows, cols);
        }

        adjustFields();
    }

    public void clear(Position start, Position end) {
        // Remove any fields that starts in the interval [start,end]
        clearFields(start, end);

        Position p = new Position(start);
        while (!p.equals(end)) {
            getFieldAt(p).clearAt(p);
            p.incCol(rows, cols);
        }
    }

    public String toString() {
        StringBuffer buf = new StringBuffer();
        Position fieldStart = new Position(HOME);
        Field field = null;
        int i = 0;

        while (!END.equals(fieldStart)) {
            field = getFieldAt(fieldStart);
            buf.append("Field ").append(i++).append(' ');
            buf.append(field.getStart()).append('-').append(field.getEnd());

            fieldStart = nextFieldStart(fieldStart);
        }
        return buf.toString();
    }
}


