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

package com.mindbright.terminal;

/**
 * Holds the context of a search, so that a new search can be started
 * from the last position
 */
public class SearchContext {
    private int startRow;
    private int startCol;
    private int endRow;
    private int endCol;

    public SearchContext(int startRow, int startCol, int endRow, int endCol) {
        this.startRow = startRow;
        this.startCol = startCol;
        this.endRow = endRow;
        this.endCol = endCol;
    }

    public int getStartRow() {
        return startRow;
    }
    public int getStartCol() {
        return startCol;
    }
    public int getEndRow() {
        return endRow;
    }
    public int getEndCol() {
        return endCol;
    }

    public String toString() {
        return getStartRow() + "," + getStartCol() + " - " +
               getEndRow() + "," + getEndCol();
    }
}


