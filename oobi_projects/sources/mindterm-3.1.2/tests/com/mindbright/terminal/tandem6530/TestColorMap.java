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

import com.mindbright.terminal.Display;
import com.mindbright.terminal.DisplayModel;
import com.mindbright.terminal.tandem6530.ColorMap;

public class TestColorMap extends TestCase {

    protected int findColor(String color) {
        for (int i = 0; i < Display.termColorNames.length; i++) {
            if(Display.termColorNames[i].equalsIgnoreCase(color)) {
                return i;
            }
        }
        return -1;
    }

    protected void assertColorEquals(String fg, String bg, int attrib) {
        int fgColor = (attrib & DisplayModel.MASK_FGCOL) >>
                      DisplayModel.SHIFT_FGCOL;
        int bgColor = (attrib & DisplayModel.MASK_BGCOL) >>
                      DisplayModel.SHIFT_BGCOL;
        if ((attrib & DisplayModel.ATTR_FGCOLOR) != DisplayModel.ATTR_FGCOLOR) {
            throw new AssertionFailedError("No foreground color");
        }
        if ((attrib & DisplayModel.ATTR_BGCOLOR) != DisplayModel.ATTR_BGCOLOR) {
            throw new AssertionFailedError("No background color");
        }

        if (fgColor != findColor(fg)) {
            throw new AssertionFailedError("Bad foreground color." +
                                           "Expected "+findColor(fg)+" but was " + fgColor+".");
        }
        if (bgColor != findColor(bg)) {
            throw new AssertionFailedError("Bad background color." +
                                           "Expected "+findColor(bg)+" but was " + bgColor+".");
        }
    }

    public void testManipulate() {
        String defaultMap = "70"+"61"+"52"+"43"+"34"+"25"+"16"+"07"+
                            "78"+"69"+"5a"+"4b"+"3c"+"2d"+"1e"+"0f"+
                            "70"+"61"+"52"+"43"+"34"+"25"+"16"+"07"+
                            "78"+"69"+"5a"+"4b"+"3c"+"2d"+"1e"+"0f";

        byte input[] = { (byte) 0x20,(byte) 0x30, (byte) 0x40, (byte) 0xf0,
                         (byte) 0x21,(byte) 0x31, (byte) 0x41, (byte) 0xf1,
                         (byte) 0x22,(byte) 0x32, (byte) 0x42, (byte) 0xf2,
                         (byte) 0x23,(byte) 0x33, (byte) 0x43, (byte) 0xf3,
                         (byte) 0x24,(byte) 0x34, (byte) 0x44, (byte) 0xf4,
                         (byte) 0x25,(byte) 0x35, (byte) 0x45, (byte) 0xf5,
                         (byte) 0x26,(byte) 0x36, (byte) 0x46, (byte) 0xf6,
                         (byte) 0x27,(byte) 0x37, (byte) 0x47, (byte) 0xf7, };

        String expStr = "203040f0" + "213141f1" + "223242f2" + "233343f3" +
                        "243444f4" + "253545f5" + "263646f6" + "273747f7";

        ColorMap map = new ColorMap();
        assertEquals(defaultMap, map.read());

        map.set(0, input);
        assertEquals(expStr, map.read());

        map.reset();
        assertEquals(defaultMap, map.read());

        byte input1[] = new byte[4];
        input1[0] = (byte) 0xff;
        input1[1] = (byte) 0xff;
        input1[2] = (byte) 0xff;
        input1[3] = (byte) 0xff;
        String exp1 = "ffffffff" + "34251607" + "78695a4b" + "3c2d1e0f"+
                      "70615243" + "34251607" + "78695a4b" + "3c2d1e0f";
        map.set(0, input1);
        assertEquals(exp1, map.read());

        map.reset();

        String exp2 = "70615243" + "34251607" + "78695a4b" + "3c2d1e0f"+
                      "70615243" + "34251607" + "78695a4b" + "ffffffff";
        map.set(28, input1);
        assertEquals(exp2, map.read());
    }

    public void testMapping() {
        ColorMap map = new ColorMap();
        assertColorEquals("black",   "white",   map.map(0));
        assertColorEquals("blue",    "yellow",  map.map(1));
        assertColorEquals("green",   "magenta", map.map(2));
        assertColorEquals("cyan",    "red",     map.map(3));
        assertColorEquals("red",     "cyan",    map.map(4));
        assertColorEquals("magenta", "green",   map.map(5));
        assertColorEquals("yellow",  "blue",    map.map(6));
        assertColorEquals("white",   "black",   map.map(7));

        assertColorEquals("i_black",   "i_white",   map.map(8));
        assertColorEquals("i_blue",    "i_yellow",  map.map(9));
        assertColorEquals("i_green",   "i_magenta", map.map(10));
        assertColorEquals("i_cyan",    "i_red",     map.map(11));
        assertColorEquals("i_red",     "i_cyan",    map.map(12));
        assertColorEquals("i_magenta", "i_green",   map.map(13));
        assertColorEquals("i_yellow",  "i_blue",    map.map(14));
        assertColorEquals("i_white",   "i_black",   map.map(15));
    }
}


