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

import com.mindbright.terminal.tandem6530.Screen6530;
import com.mindbright.terminal.tandem6530.Screen6530Buffer;

public class TestScreen6530 extends TestCase implements AsciiCodes {
    int COLS = 15;
    int ROWS = 10;

    protected Screen6530 screen;

    public void setUp() {
        screen = new Screen6530(ROWS, COLS, ' ', false);

        screen.cursorSetPos(0, 0);

        for (int i = 0; i < COLS; i++) {
            screen.cursorWrite( (i % 2 == 0) ? '_' : 'X');
        }

        screen.cursorSetPos(2, 0);
        write("K. ");
        screen.setAttribute('$' & 0x1f);
        write(" Anka");

        screen.cursorSetPos(4, 0);
        screen.setAttribute('0' & 0x1f);
        screen.setAttribute('1' & 0x1f);
        screen.setAttribute('2' & 0x1f);
        screen.setAttribute('3' & 0x1f);
        screen.setAttribute('4' & 0x1f);
    }

    protected void write(String str) {
        for (int i = 0; i < str.length(); i++) {
            screen.cursorWrite(str.charAt(i));
        }
    }

    public void testReadPage() {
        String expOneLine = ". " + ESC+"6$" + " An" + CR;
        assertEquals(expOneLine, screen.read(2, 1, 2, 6));

        String expTwoLines = CR +
                             "K. " + ESC+"6$" + " Anka" + CR;
        assertEquals(expTwoLines, screen.read(1, 0, 2, COLS - 1));
        assertEquals(expTwoLines, screen.read(1, COLS - 1, 2, 8));

        String expWhole = "_X_X_X_X_X_X_X_" + CR +
                          CR +
                          "K. " + ESC+"6$" + " Anka" + CR +
                          CR +
                          ESC+"60"+ESC+"61"+ESC+"62"+ESC+"63"+ESC+"64"+ CR +
                          CR +
                          CR +
                          CR +
                          CR +
                          CR;
        assertEquals(expWhole, screen.readWhole());
    }

    private void display(String s1, String s2) {
        System.out.println("len1="+s1.length()+" len2="+s2.length());
        for (int i = 0; i < s1.length() && i < s2.length(); i++) {
            if (s1.charAt(i) != s2.charAt(i)) {
                System.out.println("i="+i+" "+(int)s1.charAt(i)+" "+
                                   (int) s2.charAt(i) +" !!");
            } else {
                System.out.println("i="+i+" "+(int)s1.charAt(i)+" "+
                                   (int) s2.charAt(i));
            }
        }
    }

    public void testAltBuffer() {
        String exp = screen.readWhole();
        String empty = String.valueOf(CR)+CR+CR+CR+CR+CR+CR+CR+CR+CR;

        Screen6530Buffer buf = screen.getBuffer();
        assertEquals(ROWS, buf.getRows());
        assertEquals(COLS, buf.getCols());

        Screen6530 screen2;

        // Normal screen
        screen2 = new Screen6530(ROWS, COLS, ' ', false);
        assertEquals(empty, screen2.readWhole());
        screen2.setBuffer(buf);
        assertEquals(exp, screen2.readWhole());

        // Screen with savelines
        screen2 = new Screen6530(ROWS, COLS, ' ', false);
        screen2.setSaveLines(100);
        assertEquals(empty, screen2.readWhole());
        screen2.setBuffer(buf);
        assertEquals(exp, screen2.readWhole());

        screen2.setVisTop(ROWS);
        assertEquals(empty, screen2.readWhole());
        screen2.setBuffer(buf);
        assertEquals(exp, screen2.readWhole());

        // Screen that is larger
        String bigEmpty = empty + empty;
        String bigExp = exp + empty;
        screen2 = new Screen6530(ROWS*2, COLS*2, ' ', false);
        assertEquals(bigEmpty, screen2.readWhole());
        screen2.setBuffer(buf);
        assertEquals(bigExp, screen2.readWhole());

        // Screen that is smaller
        String smallEmpty = String.valueOf(CR) + CR + CR + CR + CR;
        String smallExp = "_X_X_X_X" + CR +
                          CR +
                          "K. " + ESC+"6$" + " Ank" + CR +
                          CR +
                          ESC+"60"+ESC+"61"+ESC+"62"+ESC+"63"+ESC+"64"+ CR;
        screen2 = new Screen6530(5, 8, ' ', false);
        assertEquals(smallEmpty, screen2.readWhole());
        screen2.setBuffer(buf);
        assertEquals(smallExp, screen2.readWhole());
    }

}


