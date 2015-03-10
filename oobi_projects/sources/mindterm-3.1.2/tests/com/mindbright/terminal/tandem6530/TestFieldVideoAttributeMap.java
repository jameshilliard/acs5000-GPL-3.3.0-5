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

import com.mindbright.terminal.tandem6530.FieldVideoAttributeMap;

public class TestFieldVideoAttributeMap extends TestCase {

    int DEFAULT_ATTRIB = 10;
    FieldVideoAttributeMap map;
    int attrib0 = 0;
    int attrib1 = 1;
    int attrib2 = 2;
    int attrib3 = 3;
    int newDefault = 11;

    public void setUp() {
        map = new FieldVideoAttributeMap(DEFAULT_ATTRIB);
    }

    public void test() {
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(DEFAULT_ATTRIB, map.get(1));
        assertEquals(DEFAULT_ATTRIB, map.get(1000));

        map.set(0, attrib0);
        assertEquals(attrib0, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(10));
        assertEquals(attrib0, map.get(20));

        map.set(5, attrib1);
        assertEquals(attrib0, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));

        map.set(15, attrib2);
        assertEquals(attrib0, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib2, map.get(15));
        assertEquals(attrib2, map.get(16));

        map.clearAt(1); // Nothing should happend
        assertEquals(attrib0, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib2, map.get(15));
        assertEquals(attrib2, map.get(16));

        map.clearAt(0);
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(DEFAULT_ATTRIB, map.get(1));
        assertEquals(DEFAULT_ATTRIB, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib2, map.get(15));
        assertEquals(attrib2, map.get(16));

        map.clearFrom(0);
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(DEFAULT_ATTRIB, map.get(1));
        assertEquals(DEFAULT_ATTRIB, map.get(4));
        assertEquals(DEFAULT_ATTRIB, map.get(5));
        assertEquals(DEFAULT_ATTRIB, map.get(6));
        assertEquals(DEFAULT_ATTRIB, map.get(14));
        assertEquals(DEFAULT_ATTRIB, map.get(15));
        assertEquals(DEFAULT_ATTRIB, map.get(16));

    }

    public void testInsert() {
        map.set(15, attrib2);
        map.set(5, attrib1);
        map.set(0, attrib0);

        assertEquals(attrib0, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib2, map.get(15));
        assertEquals(attrib2, map.get(16));

        map.insertAt(0);
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(2));
        assertEquals(attrib0, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(7));
        assertEquals(attrib1, map.get(15));
        assertEquals(attrib2, map.get(16));
        assertEquals(attrib2, map.get(17));

        map.insertAt(3);
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(2));
        assertEquals(attrib0, map.get(6));
        assertEquals(attrib1, map.get(7));
        assertEquals(attrib1, map.get(8));
        assertEquals(attrib1, map.get(16));
        assertEquals(attrib2, map.get(17));
        assertEquals(attrib2, map.get(18));

        map.insertAt(30); // Nothing should happend
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(2));
        assertEquals(attrib0, map.get(6));
        assertEquals(attrib1, map.get(7));
        assertEquals(attrib1, map.get(8));
        assertEquals(attrib1, map.get(16));
        assertEquals(attrib2, map.get(17));
        assertEquals(attrib2, map.get(18));
    }

    public void testDelete() {
        map.set(15, attrib2);
        map.set(5, attrib1);
        map.set(1, attrib0);

        map.deleteAt(15);
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(2));
        assertEquals(attrib0, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib1, map.get(15));
        assertEquals(attrib1, map.get(16));

        map.deleteAt(2);
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(2));
        assertEquals(attrib0, map.get(3));
        assertEquals(attrib1, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(13));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib1, map.get(15));

        map.deleteAt(0);
        assertEquals(attrib0, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(2));
        assertEquals(attrib1, map.get(3));
        assertEquals(attrib1, map.get(4));
        assertEquals(attrib1, map.get(12));
        assertEquals(attrib1, map.get(13));
        assertEquals(attrib1, map.get(14));

        map.deleteAt(0);
        assertEquals(DEFAULT_ATTRIB, map.get(0));
        assertEquals(DEFAULT_ATTRIB, map.get(1));
        assertEquals(attrib1, map.get(2));
        assertEquals(attrib1, map.get(3));
        assertEquals(attrib1, map.get(11));
        assertEquals(attrib1, map.get(12));
        assertEquals(attrib1, map.get(13));
    }

    public void testNewDefault() {
        map.set(15, attrib2);
        map.set(5, attrib1);
        map.set(1, attrib0);

        int newDefault = 11;
        map.setDefaultAttrib(newDefault);
        assertEquals(newDefault, map.get(0));
        assertEquals(attrib0, map.get(1));
        assertEquals(attrib0, map.get(2));
        assertEquals(attrib0, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib2, map.get(15));
        assertEquals(attrib2, map.get(16));
    }

    public void testReplace() {
        map.set(15, attrib2);
        map.set(5, attrib1);
        map.set(0, attrib0);

        map.set(0, attrib3);
        assertEquals(attrib3, map.get(0));
        assertEquals(attrib3, map.get(1));
        assertEquals(attrib3, map.get(2));
        assertEquals(attrib3, map.get(4));
        assertEquals(attrib1, map.get(5));
        assertEquals(attrib1, map.get(6));
        assertEquals(attrib1, map.get(14));
        assertEquals(attrib2, map.get(15));
        assertEquals(attrib2, map.get(16));
    }
}
