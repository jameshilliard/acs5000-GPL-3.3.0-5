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
import junit.framework.AssertionFailedError;

import com.mindbright.terminal.tandem6530.Parser;
import com.mindbright.terminal.tandem6530.ParseException;
import com.mindbright.terminal.tandem6530.FieldAttributes;
import com.mindbright.terminal.tandem6530.IBM3270FieldAttributes;
import com.mindbright.terminal.tandem6530.ConfigParameter;

public class TestParser extends TestCase implements AsciiCodes {
    Parser parser;

    public void setUp() {
        parser = new Parser();
    }

    public void tearDown() {}

    /** Assert that no output is generated, and that the test callback
     * method is called for last input if calledLast is true.
     */
    protected void assertParseOK(String input, int action) {
        assertParseOK(input.toCharArray(), action);
    }
    protected void assertParseOK(char input[], int action) {
        ActionAdapter ah = (ActionAdapter) parser.getActionHandler();
        char ret;
        int i;
        for (i = 0; i < input.length; i++) {
            try {
                ret = parser.parse(input[i]);
            } catch (ParseException e) {
                throw new AssertionFailedError("i = " + i + ": " +
                                               "Unexpected exception with message '" +
                                               e.getMessage() + "'");
            }

            if (ret != Parser.IGNORE) {
                throw new AssertionFailedError("i = " + i + ": " +
                                               "Should not generate output ("+(int) ret+") " +
                                               "for input (" + (int)input[i] + ")");
            }
            if (i == (input.length - 1)) {
                if (ah.getLastAction() != action) {
                    throw new AssertionFailedError("i = " + i + ": " +
                                                   "The expected callback method ('" +
                                                   ah.getActionName(action)+"') was not called. "+
                                                   "'" + ah.getLastActionName() + "'" +
                                                   " was called instead.");
                }
            } else {
                if (ah.isCalled()) {
                    throw new AssertionFailedError("i = " + i + ": " +
                                                   "No method should be called here, '" +
                                                   ah.getLastActionName() + "' was called.");
                }
            }
        }

        try {
            ret = parser.parse((char) 0xa0);
        } catch (ParseException e) {
            throw new AssertionFailedError("i = " + i + ": " +
                                           "Unexpected exception with message '" +
                                           e.getMessage() + "'");
        }

        if (ret != 0xa0) {
            throw new AssertionFailedError(
                "Parser did not return to ground state");
        }
    }

    /** Assert that no output is generated, and that a exception is
     * thrown.
     */
    protected void assertParseFail(String input, String expMsg) {
        assertParseFail(input.toCharArray(), expMsg);
    }
    protected void assertParseFail(String input, int exceptionIndex,
                                   String expMsg) {
        assertParseFail(input.toCharArray(), exceptionIndex, expMsg);
    }

    protected void assertParseFail(char input[], String expMsg) {
        assertParseFail(input, input.length - 1, expMsg);
    }

    protected void assertParseFail(char input[], int exceptionIndex,
                                   String expMsg) {
        ActionAdapter ah = (ActionAdapter) parser.getActionHandler();
        char ret;
        int i;
        for (i = 0; i < input.length; i++) {
            if (i == exceptionIndex) {
                try {
                    ret = parser.parse(input[i]);
                } catch (ParseException e) {
                    if (!expMsg.equals(e.getMessage())) {
                        throw new AssertionFailedError("i = " + i + ": " +
                                                       "Bad message in exception. Expected '" +
                                                       expMsg + "' but got '" + e.getMessage()+"'");
                    }
                    break;
                }
                throw new AssertionFailedError("i = " + i + ": " +
                                               "Expected an exception with message '"+expMsg+"' here");
            } else {
                try {
                    ret = parser.parse(input[i]);
                } catch (ParseException e) {
                    throw new AssertionFailedError("i = " + i + ": " +
                                                   "Unexpected exception with message '" +
                                                   e.getMessage() + "'");
                }

                if (ret != Parser.IGNORE) {
                    throw new AssertionFailedError("i = " + i + ": " +
                                                   "Should not generate output ("+(int) ret+") " +
                                                   "for input (" + (int)input[i] + ")");
                }
                if (ah.isCalled()) {
                    throw new AssertionFailedError("i = " + i + ": " +
                                                   "No method should be called here, '" +
                                                   ah.getLastActionName() + "' was called.");
                }
            }
        }

        if (i == input.length) {
            throw new AssertionFailedError(
                "Could parse all input without exceptions," +
                " badly coded test!");
        }

        try {
            ret = parser.parse((char) 0xa0);
        } catch (ParseException e) {
            throw new AssertionFailedError("Unexpected exception with" +
                                           " message '" + e.getMessage() +
                                           "'. Is parser state returned to ground state?");
        }

        if (ret != 0xa0) {
            throw new AssertionFailedError(
                "Parser did not return to ground state after failure");
        }
    }

    protected void assertBytesEquals(String msg, byte expArray[], byte array[]) {
        assertBytesEquals(msg, expArray, 0, expArray.length, array);
    }
    protected void assertBytesEquals(String msg, byte expArray[],
                                     int offset, int len, byte array[]) {
        if (len != array.length) {
            throw new AssertionFailedError(msg +
                                           " Length was unexpected. Was " + array.length +
                                           " expected " + len);
        }
        for (int i = 0; i < len; i++) {
            if (expArray[offset + i] != array[i]) {
                throw new AssertionFailedError(msg +
                                               " Arrays was unequal at index " + i +
                                               " was " + (int) array[i] + " expected " +
                                               (int) expArray[offset + i]);
            }
        }
    }

    protected void assertAttribsEquals(String msg, FieldAttributes expArray[],
                                       int offset, int len,
                                       FieldAttributes array[]) {
        if (len != array.length) {
            throw new AssertionFailedError(msg +
                                           " Length was unexpected. Was " + array.length +
                                           " expected " + len);
        }
        for (int i = 0; i < len; i++) {
            if (!expArray[offset + i].equals(array[i])) {
                throw new AssertionFailedError(msg +
                                               " Arrays was unequal at index " + i);
            }
        }
    }



    private void setG1CharSet() {
        try {
            parser.parse((char) 0x0e);
        } catch (ParseException e) {
            throw new AssertionFailedError(
                "Unexpected exception with message '" +
                e.getMessage() + "' when swithing to G1");
        }
    }

    private void setG0CharSet() {
        try {
            parser.parse((char) 0x0f);
        } catch (ParseException e) {
            throw new AssertionFailedError(
                "Unexpected exception with message '" +
                e.getMessage() + "' when swithing to G0");
        }
    }

    public void testBadControlCharacters() {
        char input[] = new char[1];
        for (int i = 0x00; i < 0x9f; i++) {
            if (i == 0x07 || i == 0x08 || i == 0x09 || i == 0x0a ||
                    i == 0x0d || i == 0x0e || i == 0x0f || i == 0x11 ||
                    i == 0x12 || i == 0x13 || i == 0x14 || i == ESC ||
                    i == 0x1c || i == 0x1d || i == SOH  || i == NUL ||
                    i == EOT  || i == ENQ) {
                // Known control char
                continue;
            }

            if (0x20 <= i && i <= 0x7f) {
                // Graphics characters
                continue;
            }

            input[0] = (char) i;
            assertParseFail(input, "Unknown control character 0x" +
                            Integer.toString(i, 16));
        }

    }

    public void testUnknownEscapeSequences() {
        char input[] = new char[2];
        ActionAdapter ah = new ActionAdapter();
        parser.setActionHandler(ah);

        input[0] = ESC;
        for (int i = 0x20; i <= 0x7f; i++) {
            switch (i) {
            case ' ':
            case '0':
            case '1':
            case '2':
            case '3':
            case '6':
            case '7':
            case '8':
            case '9':
            case ':':
            case ';':
            case '`':
            case '<':
            case '=':
            case '>':
            case '?':
            case '@':
            case 'A':
            case 'C':
            case 'F':
            case 'H':
            case 'I':
            case 'J':
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case '[':
            case ']':
            case '^':
            case '_':
            case '-':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'f':
            case 'i':
            case 'o':
            case 'p':
            case 'q':
            case 'r':
            case 't':
            case 'u':
            case 'v':
            case 'x':
            case 'y':
            case 'z':
            case '{':
            case '}':
                // Known escape sequence
                continue;
            }

            input[1] = (char) i;
            assertParseFail(input, "Unknown escape sequence Esc " +
                            (char) i);
        }

    }

    public void testUnknownEscapeDashSequences() {
        char input[] = new char[3];
        ActionAdapter ah = new ActionAdapter();
        parser.setActionHandler(ah);

        input[0] = ESC;
        input[1] =        '-';
        for (int i = 0x20; i <= 0x7f; i++) {
            switch (i) {
            case 'C':
            case 'D':
            case 'I':
            case 'J':
            case 'K':
            case 'O':
            case 'V':
            case 'W':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'i':
            case 'j':
            case 'm':
            case 'n':
            case 'o':
            case 'q':
            case 'r':
            case 's':
            case 't':
            case 'u':
            case 'v':
            case 'x':
            case 'y':
            case 'z':
                // Known escape sequence
                continue;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case ';':
                // Used for parameters
                continue;
            }

            input[2] = (char) i;
            assertParseFail(input, "Unknown escape sequence Esc - " +
                            (char) i);
        }
    }

    public void testOrdinaryChars() {
        char ret = 0;
        ActionAdapter ah = new ActionAdapter();
        parser.setActionHandler(ah);

        int charSetSize = 95;
        char G0[] = new char[charSetSize];
        char G1[] = new char[charSetSize];

        for (int i = 0; i < charSetSize; i++) {
            G0[i] = (char) ((int) 0x20 + i);
            G1[i] = (char) ((int) 0xa0 + i);
        }

        /* Test G0 */
        for (int i = 0; i < charSetSize; i++) {
            try {
                ret = parser.parse(G0[i]);
            } catch (ParseException e) {
                fail("Unexpected exception with message '"+e.getMessage()+
                     "' when parsing i = " + i);
            }
            assertEquals("Fail to parse G0 char. Input "+(int)G0[i]+
                         " became "+(int)ret, G0[i], ret);
        }

        /* Test G1 as 8-bit characters */
        for (int i = 0; i < charSetSize; i++) {
            try {
                ret = parser.parse(G1[i]);
            } catch (ParseException e) {
                fail("Unexpected exception with message '"+e.getMessage()+
                     "' when parsing i = " + i);
            }
            assertEquals("Fail to parse 8-bit G1 char. Input "+(int)G1[i]+
                         " became "+(int)ret, G1[i], ret);
        }

        /* Test G1 as 8-bit characters when charset is shifted
         * (should be unaffected) */
        setG1CharSet();
        for (int i = 0; i < charSetSize; i++) {
            try {
                ret = parser.parse(G1[i]);
            } catch (ParseException e) {
                fail("Unexpected exception with message '"+e.getMessage()+
                     "' when parsing i = " + i);
            }
            assertEquals("Fail to parse 8-bit G1 char when shifted. "+
                         "Input "+(int)G1[i]+" became "+(int)ret, G1[i], ret);
        }
        setG0CharSet();

        /* Test G1 as 7-bit characters */
        setG1CharSet();
        for (int i = 0; i < charSetSize; i++) {
            try {
                ret = parser.parse(G0[i]);
            } catch (ParseException e) {
                fail("Unexpected exception with message '"+e.getMessage()+
                     "' when parsing i = " + i);
            }
            assertEquals("Fail to parse 7-bit G1 char. Input "+(int)G0[i]+
                         " became "+(int)ret, G1[i], ret);
        }
        setG0CharSet();
    }

    /** Test that basic control characters works (without arguments)
     */
    public void testBasicControlCharacter() {
        ActionAdapter ah = new ActionAdapter();
        parser.setActionHandler(ah);

        /* BELL */
        assertParseOK("\u0007", ActionAdapter.ACTION_doBell);
        ah.resetAction();

        /* BS */
        assertParseOK("\u0008", ActionAdapter.ACTION_doBackspace);
        ah.resetAction();

        /* HT (tab) */
        assertParseOK("\u0009", ActionAdapter.ACTION_doHTab);
        ah.resetAction();

        /* LF */
        assertParseOK("\n", ActionAdapter.ACTION_doLineFeed);
        ah.resetAction();

        /* CR */
        assertParseOK("\r", ActionAdapter.ACTION_doCarriageReturn);
        ah.resetAction();

        /* SO and SI are tested with ordinary characters and with some
         * ESC sequencies
         */
    }

    public void testDoSetBufferAddress() {
        char input[] = new char[3];
        class SetBufferAddrAction extends ActionAdapter {
            public int row = -1;
            public int column = -1;
            public void doSetBufferAddress(int row, int column) {
                super.doSetBufferAddress(row, column);
                this.row = row;
                this.column = column;
            }

        }
        SetBufferAddrAction ah = new SetBufferAddrAction();
        parser.setActionHandler(ah);

        /* DC1 to set 1,1*/
        input[0] = (char) 0x11;
        input[1] = (char) (0x1f + 1);
        input[2] = (char) (0x1f + 1);
        assertParseOK(input, ActionAdapter.ACTION_doSetBufferAddress);
        assertEquals("Value of row was unexpected", 1, ah.row);
        assertEquals("Value of column was unexpected", 1, ah.column);
        ah.resetAction();

        /* DC1 to set 24,80 */
        input[0] = (char) 0x11;
        input[1] = (char) (0x1f + 24);
        input[2] = (char) (0x1f + 80);
        assertParseOK(input, ActionAdapter.ACTION_doSetBufferAddress);
        assertEquals("Value of row was unexpected", 24, ah.row);
        assertEquals("Value of column was unexpected", 80, ah.column);
        ah.resetAction();

        /* DC1 to set 1,1 when G1 is shifted in (should be unaffected) */
        setG1CharSet();
        input[0] = (char) 0x11;
        input[1] = (char) (0x1f + 1);
        input[2] = (char) (0x1f + 1);
        assertParseOK(input, ActionAdapter.ACTION_doSetBufferAddress);
        assertEquals("Value of row was unexpected", 1, ah.row);
        assertEquals("Value of column was unexpected", 1, ah.column);
        ah.resetAction();
        setG0CharSet();

        /* DC1 to set 25,1 (should not work) */
        input[0] = (char) 0x11;
        input[1] = (char) (0x1f + 25);
        input[2] = (char) (0x1f + 1);
        assertParseFail(input, "DC1: Row argument was out of range");
        ah.resetAction();

        /* DC1 to set 1,81 (should not work) */
        input[0] = (char) 0x11;
        input[1] = (char) (0x1f + 1);
        input[2] = (char) (0x1f + 81);
        assertParseFail(input, "DC1: Column argument was out of range");
        ah.resetAction();

        /* Test extended buffer address set */
        assertParseOK(ESC + "-12;49C", ActionAdapter.ACTION_doSetBufferAddress);
        assertEquals("Value of row was unexpected", 12, ah.row);
        assertEquals("Value of column was unexpected", 49, ah.column);
        ah.resetAction();

        /* Test that too big values are beeing set to max allowed
         * values */
        assertParseOK(ESC + "-32;81C", ActionAdapter.ACTION_doSetBufferAddress);
        assertEquals("Value of row was unexpected", 24, ah.row);
        assertEquals("Value of column was unexpected", 80, ah.column);
        ah.resetAction();

        /* Test bad values */
        assertParseFail(ESC + "-;81C", "Esc-C: Row argument is missing");
        ah.resetAction();
        assertParseFail(ESC + "-25;C", "Esc-C: Column argument is missing");
        ah.resetAction();
        assertParseFail(ESC + "-0;5C", "Esc-C: Row argument is too low");
        ah.resetAction();
        assertParseFail(ESC + "-1;0C", "Esc-C: Column argument is too low");
        ah.resetAction();
    }

    public void testDoSetCursorAddress() {
        char input[] = new char[3];
        class SetCursorAddrAction extends ActionAdapter {
            public int row = -1;
            public int column = -1;
            public void doSetCursorAddress(int row, int column) {
                super.doSetCursorAddress(row, column);
                this.row = row;
                this.column = column;
            }

        }
        SetCursorAddrAction ah = new SetCursorAddrAction();
        parser.setActionHandler(ah);

        /* DC3 to set 1,1*/
        input[0] = (char) 0x13;
        input[1] = (char) (0x1f + 1);
        input[2] = (char) (0x1f + 1);
        assertParseOK(input, ActionAdapter.ACTION_doSetCursorAddress);
        assertEquals("Value of row was unexpected", 1, ah.row);
        assertEquals("Value of column was unexpected", 1, ah.column);
        ah.resetAction();

        /* DC3 to set 24,80 */
        input[0] = (char) 0x13;
        input[1] = (char) (0x1f + 24);
        input[2] = (char) (0x1f + 80);
        assertParseOK(input, ActionAdapter.ACTION_doSetCursorAddress);
        assertEquals("Value of row was unexpected", 24, ah.row);
        assertEquals("Value of column was unexpected", 80, ah.column);
        ah.resetAction();

        /* DC3 to set 1,1 when G1 is shifted in (should be unaffected) */
        setG1CharSet();
        input[0] = (char) 0x13;
        input[1] = (char) (0x1f + 1);
        input[2] = (char) (0x1f + 1);
        assertParseOK(input, ActionAdapter.ACTION_doSetCursorAddress);
        assertEquals("Value of row was unexpected", 1, ah.row);
        assertEquals("Value of column was unexpected", 1, ah.column);
        ah.resetAction();
        setG0CharSet();

        /* DC3 to set 25,1 (should not work) */
        input[0] = (char) 0x13;
        input[1] = (char) (0x1f + 25);
        input[2] = (char) (0x1f + 1);
        assertParseFail(input, "DC3: Row argument was out of range");
        ah.resetAction();

        /* DC3 to set 1,81 (should not work) */
        input[0] = (char) 0x13;
        input[1] = (char) (0x1f + 1);
        input[2] = (char) (0x1f + 81);
        assertParseFail(input, "DC3: Column argument was out of range");
        ah.resetAction();


        /* Test extended buffer address set */
        assertParseOK(ESC + "-12;49D", ActionAdapter.ACTION_doSetCursorAddress);
        assertEquals("Value of row was unexpected", 12, ah.row);
        assertEquals("Value of column was unexpected", 49, ah.column);
        ah.resetAction();

        /* Test that too big values are beeing set to max allowed
         * values */
        assertParseOK(ESC + "-99;81D", ActionAdapter.ACTION_doSetCursorAddress);
        assertEquals("Value of row was unexpected", 24, ah.row);
        assertEquals("Value of column was unexpected", 80, ah.column);
        ah.resetAction();

        /* Test bad values */
        assertParseFail(ESC + "-;81D", "Esc-D: Row argument is missing");
        ah.resetAction();
        assertParseFail(ESC + "-25;D", "Esc-D: Column argument is missing");
        ah.resetAction();
        assertParseFail(ESC + "-0;5D", "Esc-D: Row argument is too low");
        ah.resetAction();
        assertParseFail(ESC + "-1;0D", "Esc-D: Column argument is too low");
        ah.resetAction();
    }

    public void testDoDataCompression() {
        class DataCompression extends ActionAdapter {
            public int n = -1;
            public char c = 0;
            public void doDataCompression(int n, char c) {
                super.doDataCompression(n, c);
                this.n = n;
                this.c = c;
            }

        }
        DataCompression ah = new DataCompression();
        parser.setActionHandler(ah);

        /* Simple compression */
        char simpleInput[] = new char[2];
        simpleInput[0] = (char) 0x14;
        simpleInput[1] = (char) (0x40 + 63); // Repeat space 63 times
        assertParseOK(simpleInput, ActionAdapter.ACTION_doDataCompression);
        assertEquals("Character was bad.", ' ', ah.c);
        assertEquals("Repeat number was bad.", 63, ah.n);
        ah.resetAction();

        simpleInput[0] = (char) 0x14;
        simpleInput[1] = (char) 31;          // Repeat zero 31 times
        assertParseOK(simpleInput, ActionAdapter.ACTION_doDataCompression);
        assertEquals("Character was bad.", '0', ah.c);
        assertEquals("Repeat number was bad.", 31, ah.n);
        ah.resetAction();

        /* Make sure that simple compression don't care about S1 */
        setG1CharSet();
        simpleInput[0] = (char) 0x14;
        simpleInput[1] = (char) 31;          // Repeat zero 31 times
        assertParseOK(simpleInput, ActionAdapter.ACTION_doDataCompression);
        assertEquals("Character was bad.", '0', ah.c);
        assertEquals("Repeat number was bad.", 31, ah.n);
        ah.resetAction();
        setG0CharSet();

        /* Extended compression */
        char extInput[] = new char[3];
        extInput[0] = (char) 0x12;
        extInput[1] = (char) (0x20 + 95); // Repeat 95 times
        extInput[2] = 'a';                // Repeat 'a'
        assertParseOK(extInput, ActionAdapter.ACTION_doDataCompression);
        assertEquals("Character was bad.", 'a', ah.c);
        assertEquals("Repeat number was bad.", 95, ah.n);
        ah.resetAction();

        /* Make sure that control characters can't be repeated */
        extInput[0] = (char) 0x12;
        extInput[1] = (char) (0x20 + 127); // Repeat 127 times
        extInput[2] = (char) 0x0a;         // Repeat LF
        assertParseOK(extInput, ActionAdapter.ACTION_NONE);
        ah.resetAction();

        /* Make sure that SO works */
        setG1CharSet();
        extInput[0] = (char) 0x12;
        extInput[1] = (char) (0x20 + 95); // Repeat 95 times
        extInput[2] = 'a';                // Repeat 'a'
        assertParseOK(extInput, ActionAdapter.ACTION_doDataCompression);
        assertEquals("Character was bad.", (char) ('a' + 0x80), ah.c);
        assertEquals("Repeat number was bad.", 95, ah.n);
        setG0CharSet();
        ah.resetAction();

        /* Make sure that S0 interleaved with compression works */
        extInput = new char[4];
        extInput[0] = (char) 0x12;
        extInput[1] = (char) (0x20 + 95); // Repeat 95 times
        extInput[2] = (char) 0x0e;        // SO
        extInput[3] = 'a';                // Repeat 'a'
        assertParseOK(extInput, ActionAdapter.ACTION_doDataCompression);
        setG0CharSet();
        assertEquals("Character was bad.", (char) ('a' + 0x80), ah.c);
        assertEquals("Repeat number was bad.", 95, ah.n);
        setG0CharSet();
        ah.resetAction();
    }

    public void testDoDefineFieldAttribute() {
        char input[];
        class DefineField extends ActionAdapter {
            public int row = -1;
            public int column = -1;
            public boolean useFixed = false;
            public int n = -1;

            public void doDefineFieldAttribute(int row, int column,
                                               boolean useFixed, int n) {
                super.doDefineFieldAttribute(row, column, useFixed, n);
                this.row = row;
                this.column = column;
                this.useFixed = useFixed;
                this.n = n;
            }
            public void resetAction() {
                super.resetAction();
                row = -1;
                column = -1;
                useFixed = false;
                n = -1;
            }
        }
        DefineField ah = new DefineField();
        parser.setActionHandler(ah);

        /* Test fixed attribute table */
        input = new char[4];
        input[0] = (char) 0x1c;
        input[1] = (char) (0x1f + 10); // row = 10
        input[2] = (char) (0x1f + 20); // row = 20
        input[3] = (char) (0x40 + 50); // 50th row in fixed table
        assertParseOK(input, ActionAdapter.ACTION_doDefineFieldAttribute);
        assertEquals("Value of row was unexpected", 10, ah.row);
        assertEquals("Value of column was unexpected", 20, ah.column);
        assertEquals("Value of table selector was unexpected",
                     true, ah.useFixed);
        assertEquals("Value of table row was unexpected", 50, ah.n);
        ah.resetAction();

        /* Test variable attribute table */
        input = new char[4];
        input[0] = (char) 0x1c;
        input[1] = (char) (0x1f + 10); // row = 10
        input[2] = (char) (0x1f + 20); // row = 20
        input[3] = (char) (0x20 + 31); // 31th row in variable table
        assertParseOK(input, ActionAdapter.ACTION_doDefineFieldAttribute);
        assertEquals("Value of row was unexpected", 10, ah.row);
        assertEquals("Value of column was unexpected", 20, ah.column);
        assertEquals("Value of table selector was unexpected",
                     false, ah.useFixed);
        assertEquals("Value of table row was unexpected", 31, ah.n);
        ah.resetAction();
    }

    public void testDoStartField() {
        char input[];
        class StartField extends ActionAdapter {
            public FieldAttributes attrib = null;
            public IBM3270FieldAttributes ibmAttrib = null;

            public void doStartField(FieldAttributes attrib) {
                super.doStartField(attrib);
                this.attrib = attrib;
            }
            public void doStartFieldExtended(FieldAttributes attrib) {
                super.doStartFieldExtended(attrib);
                this.attrib = attrib;
            }
            public void doStartEnhancedColorField(
                IBM3270FieldAttributes attrib) {
                super.doStartEnhancedColorField(attrib);
                ibmAttrib = attrib;
            }

            public void resetAction() {
                super.resetAction();
                attrib = null;
                ibmAttrib = null;
            }
        }

        StartField ah = new StartField();
        parser.setActionHandler(ah);

        input = new char[3];
        input[0] = (char) 0x1d;
        input[1] =        ' ';    // normal video
        input[2] = (char) (0x20 | // protected
                           0x10 | // auto-tab disabled
                           0x0e | // data type 7
                           0x01); // MDT set
        assertParseOK(input, ActionAdapter.ACTION_doStartField);
        assertEquals("Value of video attrib was unexpected",
                     ' ', ah.attrib.getVideoAttrib());
        assertEquals("Value of protect was unexpected",
                     true, ah.attrib.getProtect());
        assertEquals("Value of auto-tab was unexpected",
                     false, ah.attrib.getAutoTab());
        assertEquals("Value of MDT was unexpected",
                     true, ah.attrib.getMdt());
        assertEquals("Value of data type was unexpected",
                     7, ah.attrib.getDataType());
        ah.resetAction();

        input[0] = (char) 0x1d;
        input[1] =        '?';    // normal video
        input[2] = (char) (0x00 | // non Protected
                           0x00 | // auto-tab enables
                           0x02 | // data type 1
                           0x00); // MDT unset
        assertParseOK(input, ActionAdapter.ACTION_doStartField);
        assertEquals("Value of video attrib was unexpected",
                     '?', ah.attrib.getVideoAttrib());
        assertEquals("Value of protect was unexpected",
                     false, ah.attrib.getProtect());
        assertEquals("Value of auto-tab was unexpected",
                     true, ah.attrib.getAutoTab());
        assertEquals("Value of MDT was unexpected",
                     false, ah.attrib.getMdt());
        assertEquals("Value of data type was unexpected",
                     1, ah.attrib.getDataType());
        ah.resetAction();

        input = new char[5];
        input[0] =        ESC;
        input[1] =        '[';
        input[2] =        ' ';    // normal video
        input[3] = (char) (0x20 | // protected
                           0x10 | // auto-tab disabled
                           0x0e | // data type 7
                           0x01); // MDT set
        input[4] = (char) (0x00 | // keyboard only
                           0x00); // no upshift
        assertParseOK(input, ActionAdapter.ACTION_doStartFieldExtended);
        assertEquals("Value of video attrib was unexpected",
                     ' ', ah.attrib.getVideoAttrib());
        assertEquals("Value of protect was unexpected",
                     true, ah.attrib.getProtect());
        assertEquals("Value of auto-tab was unexpected",
                     false, ah.attrib.getAutoTab());
        assertEquals("Value of MDT was unexpected",
                     true, ah.attrib.getMdt());
        assertEquals("Value of data type was unexpected",
                     7, ah.attrib.getDataType());
        assertEquals("Value of upshift was unexpected",
                     false, ah.attrib.getUpShift());
        assertEquals("Value of keyboard was unexpected",
                     true, ah.attrib.getKeyboard());
        assertEquals("Value of aid was unexpected",
                     false, ah.attrib.getAid());
        ah.resetAction();

        input = new char[5];
        input[0] =        ESC;
        input[1] =        '[';
        input[2] =        ' ';    // normal video
        input[3] = (char) (0x20 | // protected
                           0x10 | // auto-tab disabled
                           0x0e | // data type 7
                           0x01); // MDT set
        input[4] = (char) (0x04 | // both keyboard and AID
                           0x01); // upshift
        assertParseOK(input, ActionAdapter.ACTION_doStartFieldExtended);
        assertEquals("Value of video attrib was unexpected",
                     ' ', ah.attrib.getVideoAttrib());
        assertEquals("Value of protect was unexpected",
                     true, ah.attrib.getProtect());
        assertEquals("Value of auto-tab was unexpected",
                     false, ah.attrib.getAutoTab());
        assertEquals("Value of MDT was unexpected",
                     true, ah.attrib.getMdt());
        assertEquals("Value of data type was unexpected",
                     7, ah.attrib.getDataType());
        assertEquals("Value of upshift was unexpected",
                     true, ah.attrib.getUpShift());
        assertEquals("Value of keyboard was unexpected",
                     true, ah.attrib.getKeyboard());
        assertEquals("Value of aid was unexpected",
                     true, ah.attrib.getAid());
        ah.resetAction();

        input = new char[5];
        input[0] =        ESC;
        input[1] =        '[';
        input[2] =        ' ';    // normal video
        input[3] = (char) (0x20 | // protected
                           0x10 | // auto-tab disabled
                           0x0e | // data type 7
                           0x01); // MDT set
        input[4] = (char) (0x06 | // both keyboard and AID bits
                           0x01); // upshift
        assertParseFail(input, "Esc[: Invalid extended data attribute");
        ah.resetAction();

        /* Test enhanced fields */

        /* set foreground */
        assertParseOK(ESC + "` P@01"+"422c",
                      ActionAdapter.ACTION_doStartEnhancedColorField);
        assertEquals("Value of video attrib was unexpected",
                     ' ', ah.ibmAttrib.getVideoAttrib());
        assertEquals("Value of protect was unexpected",
                     false, ah.ibmAttrib.getProtect());
        assertEquals("Value of auto-tab was unexpected",
                     false, ah.ibmAttrib.getAutoTab());
        assertEquals("Value of MDT was unexpected",
                     false, ah.ibmAttrib.getMdt());
        assertEquals("Value of data type was unexpected",
                     0, ah.ibmAttrib.getDataType());
        assertEquals("Value of upshift was unexpected",
                     false, ah.ibmAttrib.getUpShift());
        assertEquals("Value of keyboard was unexpected",
                     true, ah.ibmAttrib.getKeyboard());
        assertEquals("Value of aid was unexpected",
                     false, ah.ibmAttrib.getAid());
        assertEquals("Value of foreground color was unexpected",
                     12, ah.ibmAttrib.getFgColor());
        assertEquals("Value of background color was unexpected",
                     0, ah.ibmAttrib.getBgColor());
        ah.resetAction();

        /* set background */
        assertParseOK(ESC + "` P@01"+"452c",
                      ActionAdapter.ACTION_doStartEnhancedColorField);
        assertEquals("Value of video attrib was unexpected",
                     ' ', ah.ibmAttrib.getVideoAttrib());
        assertEquals("Value of protect was unexpected",
                     false, ah.ibmAttrib.getProtect());
        assertEquals("Value of auto-tab was unexpected",
                     false, ah.ibmAttrib.getAutoTab());
        assertEquals("Value of MDT was unexpected",
                     false, ah.ibmAttrib.getMdt());
        assertEquals("Value of data type was unexpected",
                     0, ah.ibmAttrib.getDataType());
        assertEquals("Value of upshift was unexpected",
                     false, ah.ibmAttrib.getUpShift());
        assertEquals("Value of keyboard was unexpected",
                     true, ah.ibmAttrib.getKeyboard());
        assertEquals("Value of aid was unexpected",
                     false, ah.ibmAttrib.getAid());
        assertEquals("Value of foreground color was unexpected",
                     4, ah.ibmAttrib.getFgColor());
        assertEquals("Value of background color was unexpected",
                     12, ah.ibmAttrib.getBgColor());
        ah.resetAction();

        /* set both foreground and background */
        assertParseOK(ESC + "` P@02"+"452c"+"4225",
                      ActionAdapter.ACTION_doStartEnhancedColorField);
        assertEquals("Value of video attrib was unexpected",
                     ' ', ah.ibmAttrib.getVideoAttrib());
        assertEquals("Value of protect was unexpected",
                     false, ah.ibmAttrib.getProtect());
        assertEquals("Value of auto-tab was unexpected",
                     false, ah.ibmAttrib.getAutoTab());
        assertEquals("Value of MDT was unexpected",
                     false, ah.ibmAttrib.getMdt());
        assertEquals("Value of data type was unexpected",
                     0, ah.ibmAttrib.getDataType());
        assertEquals("Value of upshift was unexpected",
                     false, ah.ibmAttrib.getUpShift());
        assertEquals("Value of keyboard was unexpected",
                     true, ah.ibmAttrib.getKeyboard());
        assertEquals("Value of aid was unexpected",
                     false, ah.ibmAttrib.getAid());
        assertEquals("Value of foreground color was unexpected",
                     5, ah.ibmAttrib.getFgColor());
        assertEquals("Value of background color was unexpected",
                     12, ah.ibmAttrib.getBgColor());
        ah.resetAction();

        assertParseFail(ESC + "` P@00", "Esc`: Bad pair count value");
        ah.resetAction();
        assertParseFail(ESC + "` P@03", "Esc`: Bad pair count value");
        ah.resetAction();
        assertParseFail(ESC + "` P@02"+"452x"+"4225", "Esc`: Not a hex digit");
        ah.resetAction();
        assertParseFail(ESC + "` P@01"+"4530", "Esc`: Bad color");
        ah.resetAction();
        assertParseFail(ESC + "` P@01"+"4230", "Esc`: Bad color");
        ah.resetAction();
        assertParseFail(ESC + "` P@01"+"4720", "Esc`: Invalid color selector");
        ah.resetAction();
    }

    public void testBasicEscCommands() {
        char input[] = new char[2];
        int tests[][] = {
                            { '0', ActionAdapter.ACTION_doPrintScreenOrPage },
                            { '1', ActionAdapter.ACTION_doSetTab },
                            { '2', ActionAdapter.ACTION_doClearTab },
                            { '3', ActionAdapter.ACTION_doClearAllTabs },
                            { '8', ActionAdapter.ACTION_doSet40CharLineWidth },
                            { '9', ActionAdapter.ACTION_doSet80CharLineWidth },
                            { '<', ActionAdapter.ACTION_doReadWholePageOrBuffer },
                            { '>', ActionAdapter.ACTION_doResetMoifiedDataTags },
                            { '@', ActionAdapter.ACTION_doDelayOneSecond },
                            { 'A', ActionAdapter.ACTION_doCursorUp },
                            { 'C', ActionAdapter.ACTION_doCursorRight },
                            { 'F', ActionAdapter.ACTION_doCursorHomeDown },
                            { 'H', ActionAdapter.ACTION_doCursorHome },
                            { 'J', ActionAdapter.ACTION_doEraseToEndOfPageOrMemory },
                            { 'K', ActionAdapter.ACTION_doEraseToEndOfLineOrField },
                            { 'L', ActionAdapter.ACTION_doInsertLine },
                            { 'M', ActionAdapter.ACTION_doDeleteLine },
                            { 'N', ActionAdapter.ACTION_doDisableLocalLineEditing },
                            { 'O', ActionAdapter.ACTION_doInsertCharacter },
                            { 'P', ActionAdapter.ACTION_doDeleteCharacter },
                            { 'S', ActionAdapter.ACTION_doRollUp },
                            { 'T', ActionAdapter.ACTION_doRollDown },
                            { 'U', ActionAdapter.ACTION_doPageDown },
                            { 'V', ActionAdapter.ACTION_doPageUp },
                            { 'W', ActionAdapter.ACTION_doEnterProtectedSubmode },
                            { 'X', ActionAdapter.ACTION_doExitProtectedSubmode },
                            { '^', ActionAdapter.ACTION_doReadTerminalStatus },
                            { '_', ActionAdapter.ACTION_doReadFullRevisionLevel },
                            { 'a', ActionAdapter.ACTION_doReadCursorAddress },
                            { 'b', ActionAdapter.ACTION_doUnlockKeyboard },
                            { 'c', ActionAdapter.ACTION_doLockKeyboard },
                            { 'f', ActionAdapter.ACTION_doDisconnectModem },
                            { 'i', ActionAdapter.ACTION_doBackTab },
                            { 'q', ActionAdapter.ACTION_doReinitialize },
                            { 't', ActionAdapter.ACTION_doSet40CharactersScreenWidth },
                            { '?', ActionAdapter.ACTION_doReadTerminalConfiguration },
                        };
        ActionAdapter ah = new ActionAdapter();
        parser.setActionHandler(ah);

        input[0] = ESC; // Esc
        for (int i = 0; i < tests.length; i++) {
            input[1] = (char) tests[i][0];
            assertParseOK(input, tests[i][1]);
            ah.resetAction();
        }
    }
    public void testSettingOfVideoAttributes() {
        class SetVideoAttributes extends ActionAdapter {
            public char attrib;
            public void doSetVideoAttributes(char videoAttrib) {
                super.doSetVideoAttributes(videoAttrib);
                attrib = videoAttrib;
            }
            public void doSetVideoPriorConditionRegister(char videoAttrib) {
                super.doSetVideoPriorConditionRegister(videoAttrib);
                attrib = videoAttrib;
            }
            public void resetAction() {
                super.resetAction();
                attrib = 0;
            }
        }

        SetVideoAttributes ah = new SetVideoAttributes();
        parser.setActionHandler(ah);

        /* Esc 6 1*/
        assertParseOK(ESC + "61", ActionAdapter.ACTION_doSetVideoAttributes);
        assertEquals("Value of video attribute was unexpected.",
                     0x11, ah.attrib);
        ah.resetAction();

        /* Esc 7 :*/
        assertParseOK(ESC + "7:",
                      ActionAdapter.ACTION_doSetVideoPriorConditionRegister);
        assertEquals("Value of video attribute was unexpected.",
                     0x1a, ah.attrib);
        ah.resetAction();
    }

    public void testSimulateFunctionKey() {
        class SimulateFunctionKey extends ActionAdapter {
            public char keyCode;
            public void doSimulateFunctionKey(char keyCode) {
                super.doSimulateFunctionKey(keyCode);
                this.keyCode = keyCode;
            }
            public void resetAction() {
                super.resetAction();
                keyCode = (char) 0;
            }
        }

        SimulateFunctionKey ah = new SimulateFunctionKey();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "da", ActionAdapter.ACTION_doSimulateFunctionKey);
        assertEquals("Key code was unexpected", 'a', ah.keyCode);
        ah.resetAction();
    }

    public void testPageSelectionAndSetting() {
        class PageStuff extends ActionAdapter {
            public int display = -1;
            public int select = -1;
            public int numPage = -1;

            public void doDisplayPage(int n) {
                super.doDisplayPage(n);
                display = n;
            }
            public void doSelectPage(int n) {
                super.doSelectPage(n);
                select = n;
            }
            public void doSetMaxPageNumber(int n) {
                super.doSetMaxPageNumber(n);
                numPage = n;
            }
            public void resetAction() {
                super.resetAction();
                display = -1;
                select = -1;
                numPage = -1;
            }
        }

        PageStuff ah = new PageStuff();
        parser.setActionHandler(ah);

        assertParseOK(ESC + ";\u0020", ActionAdapter.ACTION_doDisplayPage);
        assertEquals("Value of display page was unexpected.", 0, ah.display);
        ah.resetAction();
        assertParseOK(ESC + ";\u001f", ActionAdapter.ACTION_NONE);
        ah.resetAction();

        assertParseOK(ESC + ":\u0021", ActionAdapter.ACTION_doSelectPage);
        assertEquals("Value of selected page was unexpected.", 1, ah.select);
        ah.resetAction();
        assertParseOK(ESC + ":\u0020", ActionAdapter.ACTION_NONE);
        ah.resetAction();
        assertParseOK(ESC + ":\u001f", ActionAdapter.ACTION_NONE);
        ah.resetAction();

        assertParseOK(ESC + "p7", ActionAdapter.ACTION_doSetMaxPageNumber);
        assertEquals("Value of max page was unexpected.", 7, ah.numPage);
        ah.resetAction();
        assertParseOK(ESC + "p\u001f", ActionAdapter.ACTION_NONE);
        ah.resetAction();
        assertParseOK(ESC + "p0", ActionAdapter.ACTION_NONE);
        ah.resetAction();
    }

    public void testBasicEscDashCommands() {
        char input[] = new char[3];
        int tests[][] = {
                            { 'e', ActionAdapter.ACTION_doGetMachineName },
                            { 'o', ActionAdapter.ACTION_doReadKeyboardLatch },
                            { 'u', ActionAdapter.ACTION_doReadColorConfiguration },
                            { 'v', ActionAdapter.ACTION_doReadColorMappingTable },
                            { 'W', ActionAdapter.ACTION_doReportExecReturnCode },
                        };
        ActionAdapter ah = new ActionAdapter();
        parser.setActionHandler(ah);

        input[0] = ESC; // Esc
        input[1] = '-';  // -
        for (int i = 0; i < tests.length; i++) {
            input[2] = (char) tests[i][0];
            assertParseOK(input, tests[i][1]);
            ah.resetAction();
        }
    }

    public void testDoRead6530ColorMappingTable() {
        char input[] = new char[3];
        class Color6530 extends ActionAdapter {
            public boolean setEnhanced = false;

            public void doSet6530ColorMapping(boolean setEnhanced) {
                super.doSet6530ColorMapping(setEnhanced);
                this.setEnhanced = setEnhanced;
            }
            public void resetAction() {
                super.resetAction();
                setEnhanced = false;
            }
        }
        Color6530 ah = new Color6530();
        parser.setActionHandler(ah);

        input[0] = ESC; // Esc
        input[1] = ' ';  // space!! On page 3-27 it says Esc-v in the
        // header, but the hex dump says 0x20. Since
        // Esc-v is allocated to ReadColorMappingTable
        // with a diffrent response, I guess it's correct.
        input[2] = 'v';  // -
        assertParseOK(input, ActionAdapter.ACTION_doRead6530ColorMappingTable);
        ah.resetAction();


        assertParseOK(ESC + "-0x", ActionAdapter.ACTION_doSet6530ColorMapping);
        assertEquals("Bad setting", false, ah.setEnhanced);
        ah.resetAction();

        assertParseOK(ESC + "-1x", ActionAdapter.ACTION_doSet6530ColorMapping);
        assertEquals("Bad setting", true, ah.setEnhanced);
        ah.resetAction();


        assertParseFail(ESC + "-x", "Esc-x: Wrong number of arguments");
        ah.resetAction();
        assertParseFail(ESC + "-2x", "Esc-x: Invalid mode");
        ah.resetAction();
    }

    public void testDoClearMemoryToSpaces() {
        char input[];
        class ClearMemoryToSpaces extends ActionAdapter {
            public int startRow = -1;
            public int startColumn = -1;
            public int endRow = -1;
            public int endColumn = -1;

            public void doClearMemoryToSpaces() {
                super.doClearMemoryToSpaces();
            }
            public void doClearMemoryToSpaces(int startRow, int startColumn,
                                              int endRow, int endColumn) {
                super.doClearMemoryToSpaces(startRow, startColumn,
                                            endRow, endColumn);
                this.startRow = startRow;
                this.startColumn = startColumn;
                this.endRow = endRow;
                this.endColumn = endColumn;
            }
            public void resetAction() {
                super.resetAction();
                startRow = startColumn = endRow = endColumn = -1;
            }
        }

        ClearMemoryToSpaces ah = new ClearMemoryToSpaces();
        parser.setActionHandler(ah);
        parser.setBlockMode();

        /* Test normal clear memory to space */
        input = new char[6];
        input[0] = ESC;
        input[1] = 'I';

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseOK(input,
                      ActionAdapter.ACTION_doClearMemoryToSpacesWithAddr);
        assertEquals("Value of start row was unexpected", 1, ah.startRow);
        assertEquals("Value of start column was unexpected", 1,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        input[2] = (char) (0x1f + 0);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "EscI: Row argument was out of range");
        ah.resetAction();
        input[2] = (char) (0x1f + 25);
        assertParseFail(input, "EscI: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 0);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "EscI: Column argument was out of range");
        ah.resetAction();
        input[3] = (char) (0x1f + 81);
        assertParseFail(input, "EscI: Column argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 0);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "EscI: Row argument was out of range");
        ah.resetAction();
        input[4] = (char) (0x1f + 25);
        assertParseFail(input, "EscI: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 80);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 0);
        assertParseFail(input, "EscI: Column argument was out of range");
        ah.resetAction();
        input[5] = (char) (0x1f + 81);
        assertParseFail(input, "EscI: Column argument was out of range");
        ah.resetAction();

        /* Test clear memory to spaces in conversational mode */
        parser.setConversationalMode();
        assertParseOK(ESC + "I", ActionAdapter.ACTION_doClearMemoryToSpaces);
        parser.setBlockMode();
        ah.resetAction();

        /* Test extended clear memory to spaces with default values,
         * this is not documented but feedback from Verizon suggests
         * that this form exist. */
        assertParseOK(ESC + "-I",
                      ActionAdapter.ACTION_doClearMemoryToSpacesWithAddr);
        assertEquals("Value of start row was unexpected", 1, ah.startRow);
        assertEquals("Value of start column was unexpected", 1,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        /* Test extended clear memory to spaces */
        assertParseOK(ESC + "-12;49;17;1I",
                      ActionAdapter.ACTION_doClearMemoryToSpacesWithAddr);
        assertEquals("Value of start row was unexpected", 12, ah.startRow);
        assertEquals("Value of start column was unexpected", 49,ah.startColumn);
        assertEquals("Value of end row was unexpected", 17, ah.endRow);
        assertEquals("Value of end column was unexpected", 1, ah.endColumn);
        ah.resetAction();

        /* Test with values that are out of range */
        assertParseOK(ESC + "-30;90;99;99I",
                      ActionAdapter.ACTION_doClearMemoryToSpacesWithAddr);
        assertEquals("Value of start row was unexpected", 24, ah.startRow);
        assertEquals("Value of start column was unexpected", 80,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();
    }

    public void testDoReadWithAddress() {
        char input[];
        class ReadWithAddresses extends ActionAdapter {
            public int startRow = -1;
            public int startColumn = -1;
            public int endRow = -1;
            public int endColumn = -1;

            public void doReadWithAddress(int startRow, int startColumn,
                                          int endRow, int endColumn) {
                super.doReadWithAddress(startRow, startColumn,
                                        endRow, endColumn);
                this.startRow = startRow;
                this.startColumn = startColumn;
                this.endRow = endRow;
                this.endColumn = endColumn;
            }

            public void doReadWithAddressAll(int startRow, int startColumn,
                                             int endRow, int endColumn) {
                super.doReadWithAddressAll(startRow, startColumn,
                                           endRow, endColumn);
                this.startRow = startRow;
                this.startColumn = startColumn;
                this.endRow = endRow;
                this.endColumn = endColumn;
            }

            public void doReadScreenWithAllAttributes(int startRow,
                    int startColumn,
                    int endRow,
                    int endColumn) {
                super.doReadScreenWithAllAttributes(startRow, startColumn,
                                                    endRow, endColumn);
                this.startRow = startRow;
                this.startColumn = startColumn;
                this.endRow = endRow;
                this.endColumn = endColumn;
            }

            public void resetAction() {
                super.resetAction();
                startRow = startColumn = endRow = endColumn = -1;
            }
        }

        ReadWithAddresses ah = new ReadWithAddresses();
        parser.setActionHandler(ah);

        /* Test extended read with address with default values,
         * this is not documented but feedback from Verizon suggests
         * that this form exist. */
        assertParseOK(ESC + "-J",
                      ActionAdapter.ACTION_doReadWithAddress);
        assertEquals("Value of start row was unexpected", 1, ah.startRow);
        assertEquals("Value of start column was unexpected", 1,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        /* Test extended read with address */
        assertParseOK(ESC + "-12;49;17;1J",
                      ActionAdapter.ACTION_doReadWithAddress);
        assertEquals("Value of start row was unexpected", 12, ah.startRow);
        assertEquals("Value of start column was unexpected", 49,ah.startColumn);
        assertEquals("Value of end row was unexpected", 17, ah.endRow);
        assertEquals("Value of end column was unexpected", 1, ah.endColumn);
        ah.resetAction();

        /* Test with values that are out of range */
        assertParseOK(ESC + "-30;90;99;99J",
                      ActionAdapter.ACTION_doReadWithAddress);
        assertEquals("Value of start row was unexpected", 24, ah.startRow);
        assertEquals("Value of start column was unexpected", 80,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        /* Test extended read with address all with default values,
         * this is not documented but feedback from Verizon suggests
         * that this form exist. */
        assertParseOK(ESC + "-K",
                      ActionAdapter.ACTION_doReadWithAddressAll);
        assertEquals("Value of start row was unexpected", 1, ah.startRow);
        assertEquals("Value of start column was unexpected", 1,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        /* Test extended read with address all */
        assertParseOK(ESC + "-12;49;17;1K",
                      ActionAdapter.ACTION_doReadWithAddressAll);
        assertEquals("Value of start row was unexpected", 12, ah.startRow);
        assertEquals("Value of start column was unexpected", 49,ah.startColumn);
        assertEquals("Value of end row was unexpected", 17, ah.endRow);
        assertEquals("Value of end column was unexpected", 1, ah.endColumn);
        ah.resetAction();

        /* Test with values that are out of range */
        assertParseOK(ESC + "-30;90;99;99K",
                      ActionAdapter.ACTION_doReadWithAddressAll);
        assertEquals("Value of start row was unexpected", 24, ah.startRow);
        assertEquals("Value of start column was unexpected", 80,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();


        /* test read with addresses */
        input = new char[6];
        input[0] = ESC;
        input[1] = '=';

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseOK(input,
                      ActionAdapter.ACTION_doReadWithAddress);
        assertEquals("Value of start row was unexpected", 1, ah.startRow);
        assertEquals("Value of start column was unexpected", 1,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        input[2] = (char) (0x1f + 0);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "Esc=: Row argument was out of range");
        ah.resetAction();
        input[2] = (char) (0x1f + 25);
        assertParseFail(input, "Esc=: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 0);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "Esc=: Column argument was out of range");
        ah.resetAction();
        input[3] = (char) (0x1f + 81);
        assertParseFail(input, "Esc=: Column argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 0);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "Esc=: Row argument was out of range");
        ah.resetAction();
        input[4] = (char) (0x1f + 25);
        assertParseFail(input, "Esc=: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 80);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 0);
        assertParseFail(input, "Esc=: Column argument was out of range");
        ah.resetAction();
        input[5] = (char) (0x1f + 81);
        assertParseFail(input, "Esc=: Column argument was out of range");
        ah.resetAction();

        /* test read with addresses all */
        input = new char[6];
        input[0] = ESC;
        input[1] = ']';

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseOK(input,
                      ActionAdapter.ACTION_doReadWithAddressAll);
        assertEquals("Value of start row was unexpected", 1, ah.startRow);
        assertEquals("Value of start column was unexpected", 1,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        input[2] = (char) (0x1f + 0);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "Esc]: Row argument was out of range");
        ah.resetAction();
        input[2] = (char) (0x1f + 25);
        assertParseFail(input, "Esc]: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 0);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "Esc]: Column argument was out of range");
        ah.resetAction();
        input[3] = (char) (0x1f + 81);
        assertParseFail(input, "Esc]: Column argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 0);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "Esc]: Row argument was out of range");
        ah.resetAction();
        input[4] = (char) (0x1f + 25);
        assertParseFail(input, "Esc]: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 80);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 0);
        assertParseFail(input, "Esc]: Column argument was out of range");
        ah.resetAction();
        input[5] = (char) (0x1f + 81);
        assertParseFail(input, "Esc]: Column argument was out of range");
        ah.resetAction();

        /* Test read screen with all attributes */
        input = new char[6];
        input[0] = ESC;
        input[1] = 'Q';

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseOK(input,
                      ActionAdapter.ACTION_doReadScreenWithAllAttributes);
        assertEquals("Value of start row was unexpected", 1, ah.startRow);
        assertEquals("Value of start column was unexpected", 1,ah.startColumn);
        assertEquals("Value of end row was unexpected", 24, ah.endRow);
        assertEquals("Value of end column was unexpected", 80, ah.endColumn);
        ah.resetAction();

        input[2] = (char) (0x1f + 0);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 24);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "EscQ: Row argument was out of range");
        ah.resetAction();
        input[2] = (char) (0x1f + 25);
        assertParseFail(input, "EscQ: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 0);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "EscQ: Column argument was out of range");
        ah.resetAction();
        input[3] = (char) (0x1f + 81);
        assertParseFail(input, "EscQ: Column argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 1);
        input[4] = (char) (0x1f + 0);
        input[5] = (char) (0x1f + 80);
        assertParseFail(input, "EscQ: Row argument was out of range");
        ah.resetAction();
        input[4] = (char) (0x1f + 25);
        assertParseFail(input, "EscQ: Row argument was out of range");
        ah.resetAction();

        input[2] = (char) (0x1f + 1);
        input[3] = (char) (0x1f + 80);
        input[4] = (char) (0x1f + 1);
        input[5] = (char) (0x1f + 0);
        assertParseFail(input, "EscQ: Column argument was out of range");
        ah.resetAction();
        input[5] = (char) (0x1f + 81);
        assertParseFail(input, "EscQ: Column argument was out of range");
        ah.resetAction();

    }

    public void testDoReadVTLAUNCHConfigurationParameter() {
        char input[];
        class ReadVTLAUNCH extends ActionAdapter {
            public int n = -1;

            public void doReadVTLAUNCHConfigurationParameter(int n) {
                super.doReadVTLAUNCHConfigurationParameter(n);
                this.n = n;
            }

            public void resetAction() {
                super.resetAction();
                n = -1;
            }
        }

        ReadVTLAUNCH ah = new ReadVTLAUNCH();
        parser.setActionHandler(ah);

        /* Test VTLAUNCH 6530 Config parameter */
        assertParseOK(ESC + "-2g",
                      ActionAdapter.ACTION_doReadVTLAUNCHConfigurationParameter);
        assertEquals("Value of start parameter unexpected", 2, ah.n);
        ah.resetAction();

        assertParseOK(ESC + "-3g",
                      ActionAdapter.ACTION_doReadVTLAUNCHConfigurationParameter);
        assertEquals("Value of start parameter unexpected", 3, ah.n);
        ah.resetAction();

        /* Ignore parameters not 2 and 3 */
        assertParseOK(ESC + "-4g", ActionAdapter.ACTION_NONE);
        ah.resetAction();
    }

    public void testDoRTMControl() {
        class RTMStuff extends ActionAdapter {
            public int startStopEvent = -1;
            public int buckets[] = null;
            public int id[] = null;

            public void doRTMControl(int startStopEvent, int buckets[]) {
                super.doRTMControl(startStopEvent, buckets);
                this.startStopEvent = startStopEvent;
                this.buckets = buckets;
            }
            public void doRTMDataUpload(int id[]) {
                super.doRTMDataUpload(id);
                this.id = id;
            }
            public void resetAction() {
                super.resetAction();
                startStopEvent = -1;
                buckets = null;
                id = null;
            }
        }

        RTMStuff ah = new RTMStuff();
        parser.setActionHandler(ah);

        /* Test RTM control (one bucket) */
        assertParseOK(ESC + "-1;1i", ActionAdapter.ACTION_doRTMControl);
        assertEquals("Value of start/stop parameter unexpected",
                     1, ah.startStopEvent);
        assertEquals("Value of buckets was unexpected", null, ah.buckets);
        ah.resetAction();

        /* Test RTM control with 10 buckets (max) */
        int expBuckets[] = { 1, 2, 3, 4, 5, 6, 7, 10, 100 };
        assertParseOK(ESC + "-2;10;1;2;3;4;5;6;7;10;100i",
                      ActionAdapter.ACTION_doRTMControl);
        assertEquals("Value of start/stop parameter unexpected",
                     2, ah.startStopEvent);
        assertEquals("Value of buckets length was unexpected",
                     expBuckets.length, ah.buckets.length);
        for (int i = 0; i < expBuckets.length; i++) {
            assertEquals("Value of bucket " + i + " was unexpected",
                         expBuckets[i], ah.buckets[i]);
        }
        ah.resetAction();

        /* Test RTM control by say 11 buckets (larger than max)
         * Should be ignored */
        assertParseOK(ESC + "-2;11;1;2;3;4;5;6;7;10;100;1000i",
                      ActionAdapter.ACTION_NONE);
        assertEquals("Value of start/stop parameter unexpected",
                     -1, ah.startStopEvent);
        assertEquals("Value of buckets was unexpected", null, ah.buckets);
        ah.resetAction();

        /* Test RTM control by say 10 buckets but specify 11
         * Should be ignored */
        assertParseOK(ESC + "-2;10;1;2;3;4;5;6;7;10;100;1000;1500i",
                      ActionAdapter.ACTION_NONE);
        assertEquals("Value of start/stop parameter unexpected",
                     -1, ah.startStopEvent);
        assertEquals("Value of buckets was unexpected", null, ah.buckets);
        ah.resetAction();


        /* Test RTM control by say 10 buckets, but only specify 6
         * Should be ignored */
        assertParseOK(ESC + "-2;10;1;2;3;4;5;6i",
                      ActionAdapter.ACTION_NONE);
        assertEquals("Value of start/stop parameter unexpected",
                     -1, ah.startStopEvent);
        assertEquals("Value of buckets was unexpected", null, ah.buckets);
        ah.resetAction();

        /* Test RTM data upload */
        assertParseOK(ESC + "-0;5;2;255j",
                      ActionAdapter.ACTION_doRTMDataUpload);
        assertNotNull("No id", ah.id);
        assertEquals("Bad id part",   0, ah.id[0]);
        assertEquals("Bad id part",   5, ah.id[1]);
        assertEquals("Bad id part",   2, ah.id[2]);
        assertEquals("Bad id part", 255, ah.id[3]);
        ah.resetAction();

        /* Test RTM data upload with bad values */
        assertParseOK(ESC + "-0;5;2j",
                      ActionAdapter.ACTION_NONE);
        ah.resetAction();
        assertParseOK(ESC + "-0;5;2;256j",
                      ActionAdapter.ACTION_NONE);
        ah.resetAction();
    }

    public void testDoSetEM3270Mode() {
        class SetEM3270 extends ActionAdapter {
            public int n = -1;

            public void doSetEM3270Mode(int n) {
                super.doSetEM3270Mode(n);
                this.n = n;
            }
            public void resetAction() {
                super.resetAction();
                n = -1;
            }
        }

        SetEM3270 ah = new SetEM3270();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-0m", ActionAdapter.ACTION_doSetEM3270Mode);
        assertEquals("Value of parameter unexpected", 0, ah.n);
        ah.resetAction();
        assertParseOK(ESC + "-1m", ActionAdapter.ACTION_doSetEM3270Mode);
        assertEquals("Value of parameter unexpected", 1, ah.n);
        ah.resetAction();
        assertParseOK(ESC + "-2m", ActionAdapter.ACTION_doSetEM3270Mode);
        assertEquals("Value of parameter unexpected", 2, ah.n);
        ah.resetAction();
        assertParseOK(ESC + "-3m", ActionAdapter.ACTION_doSetEM3270Mode);
        assertEquals("Value of parameter unexpected", 3, ah.n);
        ah.resetAction();

        assertParseFail(ESC + "-4m", "Esc-m: Bad parameter 4");
        ah.resetAction();
        assertParseFail(ESC + "-m", "Esc-m: One parameter expected");
        ah.resetAction();

    }

    public void testDoWriteToAux1OrAux2Device() {
        class WriteToAux1Or2 extends ActionAdapter {
            public int device = -1;
            public char terminator = 0;

            public void doWriteToAux1OrAux2Device(int device, char terminator) {
                super.doWriteToAux1OrAux2Device(device, terminator);
                this.device = device;
                this.terminator = terminator;
            }
            public void resetAction() {
                super.resetAction();
                device = -1;
                terminator = 0;
            }
        }

        WriteToAux1Or2 ah = new WriteToAux1Or2();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-;O",
                      ActionAdapter.ACTION_doWriteToAux1OrAux2Device);
        assertEquals("Value of device unexpected", 1, ah.device);
        assertEquals("Value of terminator unexpected", 0x12, ah.terminator);
        ah.resetAction();

        assertParseOK(ESC + "-;3O",
                      ActionAdapter.ACTION_doWriteToAux1OrAux2Device);
        assertEquals("Value of device unexpected", 1, ah.device);
        assertEquals("Value of terminator unexpected", 0x03, ah.terminator);
        ah.resetAction();

        assertParseOK(ESC + "-4;O",
                      ActionAdapter.ACTION_doWriteToAux1OrAux2Device);
        assertEquals("Value of device unexpected", 1, ah.device);
        assertEquals("Value of terminator unexpected", 0x12, ah.terminator);
        ah.resetAction();

        assertParseOK(ESC + "-2;128O",
                      ActionAdapter.ACTION_doWriteToAux1OrAux2Device);
        assertEquals("Value of device unexpected", 2, ah.device);
        assertEquals("Value of terminator unexpected", 0x12, ah.terminator);
        ah.resetAction();

        assertParseFail(ESC + "-O", "Esc-O: Two parameters expected");
        ah.resetAction();
    }

    public void testDoTerminateRemote6530Operation() {
        class TerminateOperation extends ActionAdapter {
            public int exitCode = -1;

            public void doTerminateRemote6530Operation(int exitCode) {
                super.doTerminateRemote6530Operation(exitCode);
                this.exitCode = exitCode;
            }
            public void resetAction() {
                super.resetAction();
                exitCode = -1;
            }
        }

        TerminateOperation ah = new TerminateOperation();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-0z",
                      ActionAdapter.ACTION_doTerminateRemote6530Operation);
        assertEquals("Value of exit code unexpected", 0, ah.exitCode);
        ah.resetAction();
    }

    public void testDoReadIODeviceConfiguration() {
        class IODeviceConfiguration extends ActionAdapter {
            public int device = -1;

            public void doReadIODeviceConfiguration(int device) {
                super.doReadIODeviceConfiguration(device);
                this.device = device;
            }
            public void resetAction() {
                super.resetAction();
                device = -1;
            }
        }

        IODeviceConfiguration ah = new IODeviceConfiguration();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "yT2",
                      ActionAdapter.ACTION_doReadIODeviceConfiguration);
        assertEquals("Value of device unexpected", 1, ah.device);
        ah.resetAction();

        assertParseOK(ESC + "yT3",
                      ActionAdapter.ACTION_doReadIODeviceConfiguration);
        assertEquals("Value of device unexpected", 2, ah.device);
        ah.resetAction();

        assertParseFail(ESC + "yT1", "Escy: Bad device number");
        ah.resetAction();

        assertParseFail(ESC + "yT4", "Escy: Bad device number");
        ah.resetAction();

        assertParseFail(ESC + "yK2", "Escy: Bad format");
        ah.resetAction();
    }

    public void testDoReadStringConfigurationParameter() {
        class StringConfig extends ActionAdapter {
            public int n = -1;

            public void doReadStringConfigurationParameter(int n) {
                super.doReadStringConfigurationParameter(n);
                this.n = n;
            }
            public void resetAction() {
                super.resetAction();
                n = -1;
            }
        }

        StringConfig ah = new StringConfig();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-d",
                      ActionAdapter.ACTION_doReadStringConfigurationParameter);
        assertEquals("Value of param unexpected", 0, ah.n);
        ah.resetAction();

        assertParseOK(ESC + "-0d",
                      ActionAdapter.ACTION_doReadStringConfigurationParameter);
        assertEquals("Value of param unexpected", 0, ah.n);
        ah.resetAction();

        assertParseOK(ESC + "-7d",
                      ActionAdapter.ACTION_doReadStringConfigurationParameter);
        assertEquals("Value of param unexpected", 7, ah.n);
        ah.resetAction();

        assertParseOK(ESC + "-100d",
                      ActionAdapter.ACTION_doReadStringConfigurationParameter);
        assertEquals("Value of param unexpected", 0, ah.n);
        ah.resetAction();

        assertParseFail(ESC + "-1;1d", "Esc-d: Bad format");
        ah.resetAction();
    }

    public void testDoGetCurrentDirectoryAndRedirectionInformation() {
        class CurrentDirInfo extends ActionAdapter {
            public char drive = 0x00;

            public void doGetCurrentDirectoryAndRedirectionInformation(
                char drive) {
                super.doGetCurrentDirectoryAndRedirectionInformation(drive);
                this.drive = drive;
            }
            public void resetAction() {
                super.resetAction();
                drive = 0x00;
            }
        }

        CurrentDirInfo ah = new CurrentDirInfo();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-f@",
                      ActionAdapter.ACTION_doGetCurrentDirectoryAndRedirectionInformation);
        assertEquals("Value of drive unexpected", '@', ah.drive);
        ah.resetAction();

        assertParseOK(ESC + "-fZ",
                      ActionAdapter.ACTION_doGetCurrentDirectoryAndRedirectionInformation);
        assertEquals("Value of drive unexpected", 'Z', ah.drive);
        ah.resetAction();

        assertParseFail(ESC + "-f?", "Esc-f: Bad drive");
        ah.resetAction();

        assertParseFail(ESC + "-f[", "Esc-f: Bad drive");
        ah.resetAction();
    }

    public void testDoDefineEnterKeyFunction() {
        class DefineEnterKey extends ActionAdapter {
            public char str[] = null;

            public void doDefineEnterKeyFunction(char str[]) {
                super.doDefineEnterKeyFunction(str);
                this.str = str;
            }

            public void resetAction() {
                super.resetAction();
                str = null;
            }
        }

        DefineEnterKey ah = new DefineEnterKey();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "u#\r\n:",
                      ActionAdapter.ACTION_doDefineEnterKeyFunction);
        assertNotNull("No string defined", ah.str);
        assertEquals("Bad length of string", 3, ah.str.length);
        assertEquals("Wrong character", 0x0d, ah.str[0]);
        assertEquals("Wrong character", 0x0a, ah.str[1]);
        assertEquals("Wrong character",  ':', ah.str[2]);
        ah.resetAction();

        assertParseOK(ESC + "u!\r",
                      ActionAdapter.ACTION_doDefineEnterKeyFunction);
        assertNotNull("No string defined", ah.str);
        assertEquals("Bad length of string", 1, ah.str.length);
        assertEquals("Wrong character", 0x0d, ah.str[0]);
        ah.resetAction();

        assertParseOK(ESC + "u(abcdefgh",
                      ActionAdapter.ACTION_doDefineEnterKeyFunction);
        assertNotNull("No string defined", ah.str);
        assertEquals("Bad length of string", 8, ah.str.length);
        assertEquals("Wrong character", 'a', ah.str[0]);
        assertEquals("Wrong character", 'b', ah.str[1]);
        assertEquals("Wrong character", 'c', ah.str[2]);
        assertEquals("Wrong character", 'd', ah.str[3]);
        assertEquals("Wrong character", 'e', ah.str[4]);
        assertEquals("Wrong character", 'f', ah.str[5]);
        assertEquals("Wrong character", 'g', ah.str[6]);
        assertEquals("Wrong character", 'h', ah.str[7]);
        ah.resetAction();

        assertParseFail(ESC + "u ", "Escu: Wrong character count");
        ah.resetAction();
        assertParseFail(ESC + "u)", "Escu: Wrong character count");
        ah.resetAction();
        assertParseFail(ESC + "u!\u0000", "Escu: Invalid character");
        ah.resetAction();
        assertParseFail(ESC + "u!\u0005", "Escu: Invalid character");
        ah.resetAction();
    }

    private String createString(char c, int n) {
        StringBuffer buf = new StringBuffer();
        for (int i = 0; i < n; i++) {
            buf.append(c);
        }
        return buf.toString();
    }
    public void testDoSetStringConfigurationParameter() {
        class SetStringConfiguration extends ActionAdapter {
            public String strs[] = null;
            public void doSetStringConfigurationParameter(String strs[]) {
                super.doSetStringConfigurationParameter(strs);
                this.strs = strs;
            }
            public void resetAction() {
                super.resetAction();
                strs = null;
            }
        }

        SetStringConfiguration ah = new SetStringConfiguration();
        parser.setActionHandler(ah);

        /* Test to configure all strings and test to truncate them */
        assertParseOK(ESC + "-0c" + createString('a', 150) + DC2 +
                      createString('b', 150) + DC2 +
                      createString('c', 150) + DC2 +
                      createString('d', 150) + DC2 +
                      createString('e', 150) + DC2 +
                      createString('f', 150) + DC2 +
                      createString('g', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Fewer strings than expected", 7, ah.strs.length);
        assertEquals("Bad string length",  26, ah.strs[0].length());
        assertEquals("Bad string content", createString('a',  26), ah.strs[0]);
        assertEquals("Bad string length",  34, ah.strs[1].length());
        assertEquals("Bad string content", createString('b',  34), ah.strs[1]);
        assertEquals("Bad string length",   8, ah.strs[2].length());
        assertEquals("Bad string content", createString('c',   8), ah.strs[2]);
        assertEquals("Bad string length", 128, ah.strs[3].length());
        assertEquals("Bad string content", createString('d', 128), ah.strs[3]);
        assertEquals("Bad string length",  80, ah.strs[4].length());
        assertEquals("Bad string content", createString('e',  80), ah.strs[4]);
        assertEquals("Bad string length",   8, ah.strs[5].length());
        assertEquals("Bad string content", createString('f',   8), ah.strs[5]);
        assertEquals("Bad string length",  16, ah.strs[6].length());
        assertEquals("Bad string content", createString('g',  16), ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-c" + createString('a', 150) + DC2 +
                      createString('b', 150) + DC2 +
                      createString('c', 150) + DC2 +
                      createString('d', 150) + DC2 +
                      createString('e', 150) + DC2 +
                      createString('f', 150) + DC2 +
                      createString('g', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Fewer strings than expected", 7, ah.strs.length);
        assertEquals("Bad string length",  26, ah.strs[0].length());
        assertEquals("Bad string content", createString('a',  26), ah.strs[0]);
        assertEquals("Bad string length",  34, ah.strs[1].length());
        assertEquals("Bad string content", createString('b',  34), ah.strs[1]);
        assertEquals("Bad string length",   8, ah.strs[2].length());
        assertEquals("Bad string content", createString('c',   8), ah.strs[2]);
        assertEquals("Bad string length", 128, ah.strs[3].length());
        assertEquals("Bad string content", createString('d', 128), ah.strs[3]);
        assertEquals("Bad string length",  80, ah.strs[4].length());
        assertEquals("Bad string content", createString('e',  80), ah.strs[4]);
        assertEquals("Bad string length",   8, ah.strs[5].length());
        assertEquals("Bad string content", createString('f',   8), ah.strs[5]);
        assertEquals("Bad string length",  16, ah.strs[6].length());
        assertEquals("Bad string content", createString('g',  16), ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-8c" + createString('a', 150) + DC2 +
                      createString('b', 150) + DC2 +
                      createString('c', 150) + DC2 +
                      createString('d', 150) + DC2 +
                      createString('e', 150) + DC2 +
                      createString('f', 150) + DC2 +
                      createString('g', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Fewer strings than expected", 7, ah.strs.length);
        assertEquals("Bad string length",  26, ah.strs[0].length());
        assertEquals("Bad string content", createString('a',  26), ah.strs[0]);
        assertEquals("Bad string length",  34, ah.strs[1].length());
        assertEquals("Bad string content", createString('b',  34), ah.strs[1]);
        assertEquals("Bad string length",   8, ah.strs[2].length());
        assertEquals("Bad string content", createString('c',   8), ah.strs[2]);
        assertEquals("Bad string length", 128, ah.strs[3].length());
        assertEquals("Bad string content", createString('d', 128), ah.strs[3]);
        assertEquals("Bad string length",  80, ah.strs[4].length());
        assertEquals("Bad string content", createString('e',  80), ah.strs[4]);
        assertEquals("Bad string length",   8, ah.strs[5].length());
        assertEquals("Bad string content", createString('f',   8), ah.strs[5]);
        assertEquals("Bad string length",  16, ah.strs[6].length());
        assertEquals("Bad string content", createString('g',  16), ah.strs[6]);
        ah.resetAction();

        /* Send empty strings */
        assertParseOK(ESC + "-0c" + DC2 + DC2 + DC2 + DC2 + DC2 + DC2 + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Fewer strings than expected", 7, ah.strs.length);
        assertEquals("Bad string length", 0, ah.strs[0].length());
        assertEquals("Bad string length", 0, ah.strs[1].length());
        assertEquals("Bad string length", 0, ah.strs[2].length());
        assertEquals("Bad string length", 0, ah.strs[3].length());
        assertEquals("Bad string length", 0, ah.strs[4].length());
        assertEquals("Bad string length", 0, ah.strs[5].length());
        assertEquals("Bad string length", 0, ah.strs[6].length());
        ah.resetAction();

        /* Send single strings */
        assertParseOK(ESC + "-1c" + createString('a', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Number of strings unexpected", 7, ah.strs.length);
        assertEquals("Bad string length",  26, ah.strs[0].length());
        assertEquals("Bad string content", createString('a',  26), ah.strs[0]);
        assertNull("No string expected", ah.strs[1]);
        assertNull("No string expected", ah.strs[2]);
        assertNull("No string expected", ah.strs[3]);
        assertNull("No string expected", ah.strs[4]);
        assertNull("No string expected", ah.strs[5]);
        assertNull("No string expected", ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-2c" + createString('b', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Number of strings unexpected", 7, ah.strs.length);
        assertNull("No string expected", ah.strs[0]);
        assertEquals("Bad string length",  34, ah.strs[1].length());
        assertEquals("Bad string content", createString('b',  34), ah.strs[1]);
        assertNull("No string expected", ah.strs[2]);
        assertNull("No string expected", ah.strs[3]);
        assertNull("No string expected", ah.strs[4]);
        assertNull("No string expected", ah.strs[5]);
        assertNull("No string expected", ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-3c" + createString('c', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Number of strings unexpected", 7, ah.strs.length);
        assertNull("No string expected", ah.strs[0]);
        assertNull("No string expected", ah.strs[1]);
        assertEquals("Bad string length", 8, ah.strs[2].length());
        assertEquals("Bad string content", createString('c',   8), ah.strs[2]);
        assertNull("No string expected", ah.strs[3]);
        assertNull("No string expected", ah.strs[4]);
        assertNull("No string expected", ah.strs[5]);
        assertNull("No string expected", ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-4c" + createString('d', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Number of strings unexpected", 7, ah.strs.length);
        assertNull("No string expected", ah.strs[0]);
        assertNull("No string expected", ah.strs[1]);
        assertNull("No string expected", ah.strs[2]);
        assertEquals("Bad string length", 128, ah.strs[3].length());
        assertEquals("Bad string content", createString('d', 128), ah.strs[3]);
        assertNull("No string expected", ah.strs[4]);
        assertNull("No string expected", ah.strs[5]);
        assertNull("No string expected", ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-5c" + createString('e', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Number of strings unexpected", 7, ah.strs.length);
        assertNull("No string expected", ah.strs[0]);
        assertNull("No string expected", ah.strs[1]);
        assertNull("No string expected", ah.strs[2]);
        assertNull("No string expected", ah.strs[3]);
        assertEquals("Bad string length",  80, ah.strs[4].length());
        assertEquals("Bad string content", createString('e',  80), ah.strs[4]);
        assertNull("No string expected", ah.strs[5]);
        assertNull("No string expected", ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-6c" + createString('f', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Number of strings unexpected", 7, ah.strs.length);
        assertNull("No string expected", ah.strs[0]);
        assertNull("No string expected", ah.strs[1]);
        assertNull("No string expected", ah.strs[2]);
        assertNull("No string expected", ah.strs[3]);
        assertNull("No string expected", ah.strs[4]);
        assertEquals("Bad string length", 8, ah.strs[5].length());
        assertEquals("Bad string content", createString('f',   8), ah.strs[5]);
        assertNull("No string expected", ah.strs[6]);
        ah.resetAction();

        assertParseOK(ESC + "-7c" + createString('g', 150) + CR,
                      ActionAdapter.ACTION_doSetStringConfigurationParameter);
        assertEquals("Number of strings unexpected", 7, ah.strs.length);
        assertNull("No string expected", ah.strs[0]);
        assertNull("No string expected", ah.strs[1]);
        assertNull("No string expected", ah.strs[2]);
        assertNull("No string expected", ah.strs[3]);
        assertNull("No string expected", ah.strs[4]);
        assertNull("No string expected", ah.strs[5]);
        assertEquals("Bad string length", 16, ah.strs[6].length());
        assertEquals("Bad string content", createString('g',  16), ah.strs[6]);
        ah.resetAction();

        /* Send bad arguments */
        assertParseFail(ESC + "-7c" + createString('g', 150) + DC2,
                        "Esc-c: Too many strings");
        ah.resetAction();
        assertParseFail(ESC + "-0c" + CR, "Esc-c: Too few strings");
        ah.resetAction();
    }

    public void testDoWriteToMessageField() {
        class SetMessageField extends ActionAdapter {
            public char msg[] = null;
            public char attribs[] = null;

            public void doWriteToMessageField(char msg[], char attribs[]) {
                super.doWriteToMessageField(msg, attribs);
                this.msg = msg;
                this.attribs = attribs;
            }
            public void resetAction() {
                super.resetAction();
                msg = null;
                attribs = null;
            }
        }

        SetMessageField ah = new SetMessageField();
        parser.setActionHandler(ah);

        /* Basic message */
        assertParseOK(ESC + "o SELECT OPTION" + CR,
                      ActionAdapter.ACTION_doWriteToMessageField);
        assertEquals("Bad message", " SELECT OPTION", new String(ah.msg));
        assertEquals("Bad attributes",
                     createString(' ', 14), new String(ah.attribs));
        ah.resetAction();

        /* Embedded Esc6 message */
        assertParseOK(ESC + "oA" +ESC+"6!"+ "P"+ ESC+"6#" +"AN" + CR,
                      ActionAdapter.ACTION_doWriteToMessageField);
        assertEquals("Bad message", "APAN", new String(ah.msg));
        assertEquals("Bad attributes", " !##", new String(ah.attribs));
        ah.resetAction();

        /* Truncated message */
        String longMsg = createString('a', 80);
        String longAttr = createString(' ', 20) + createString('0', 60);
        StringBuffer buf = new StringBuffer();
        buf.append(ESC).append('o');
        for (int i = 0; i < longMsg.length(); i++) {
            buf.append(ESC).append('6').append(longAttr.charAt(i));
            buf.append(longMsg.charAt(i));
        }
        buf.append(CR);
        assertParseOK(buf.toString(),
                      ActionAdapter.ACTION_doWriteToMessageField);
        assertEquals("Bad message",
                     longMsg.substring(0, 63), new String(ah.msg));
        assertEquals("Bad attributes",
                     longAttr.substring(0, 63), new String(ah.attribs));
        ah.resetAction();

        /* Test G1 message */
        assertParseOK(ESC + "oSELECT " + SO + "O" + SI + "PTION" + CR,
                      ActionAdapter.ACTION_doWriteToMessageField);
        assertEquals("Bad message",
                     "SELECT " + (char) ('O' + 0x80) + "PTION", new String(ah.msg));
        assertEquals("Bad attributes",
                     createString(' ', 13), new String(ah.attribs));
        ah.resetAction();
    }

    private String getHexString(byte a[], int startIndex, int length) {
        StringBuffer buf = new StringBuffer();
        for (int i = startIndex; i < (startIndex + length); i++) {
            buf.append(Integer.toString((a[i] >> 4) & 0x0f, 16));
            buf.append(Integer.toString((a[i]     ) & 0x0f, 16));
        }
        return buf.toString();
    }

    public void testDoSetOrResetColorMapTable() {
        class SetResetColorMapTable extends ActionAdapter {
            public int startIndex = -1;
            public byte colorMap[] = null;

            public void doSetColorMapTable(int startIndex, byte entries[]) {
                super.doSetColorMapTable(startIndex, entries);
                this.startIndex = startIndex;
                this.colorMap = entries;
            }
            public void doResetColorMapTable() {
                super.doResetColorMapTable();
            }
            public void resetAction() {
                super.resetAction();
                startIndex = -1;
                colorMap = null;
            }
        }
        // Fill test color map with something
        byte colorMap[] = new byte[32];
        for (int i = 0; i < colorMap.length; i++) {
            colorMap[i] = (byte) ((i * 5) % 256);
        }

        SetResetColorMapTable ah = new SetResetColorMapTable();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-1;70;70q",
                      ActionAdapter.ACTION_doResetColorMapTable);
        ah.resetAction();

        assertParseOK(ESC + "-0;0;70q"+getHexString(colorMap, 0, 32),
                      ActionAdapter.ACTION_doSetColorMapTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad color map table", colorMap, 0, 32, ah.colorMap);
        ah.resetAction();

        assertParseOK(ESC + "-0;;q"+getHexString(colorMap, 0, 32),
                      ActionAdapter.ACTION_doSetColorMapTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad color map table", colorMap, 0, 32, ah.colorMap);
        ah.resetAction();

        assertParseOK(ESC + "-0;10;10q"+getHexString(colorMap, 9, 1),
                      ActionAdapter.ACTION_doSetColorMapTable);
        assertEquals("Startindex is bad", 10, ah.startIndex);
        assertBytesEquals("Bad color map table", colorMap, 9, 1, ah.colorMap);
        ah.resetAction();

        assertParseFail(ESC + "-0;q", "Esc-q: Too few arguments");
        ah.resetAction();

        assertParseFail(ESC + "-0;;;q", "Esc-q: Too many arguments");
        ah.resetAction();

        assertParseFail(ESC + "-3;;q", "Esc-q: Invalid mode");
        ah.resetAction();

        assertParseFail(ESC + "-0;1;2q"+getHexString(colorMap, 0, 1)+"XX",
                        "Esc-q: Not hex digits");
        ah.resetAction();
    }

    public void testDoDefineDataTypeTable() {
        class DefineDataTypeTable extends ActionAdapter {
            public int startIndex = -1;
            public byte dataTypes[] = null;

            public void doDefineDataTypeTable(int startIndex, byte entries[]) {
                super.doDefineDataTypeTable(startIndex, entries);
                this.startIndex = startIndex;
                this.dataTypes = entries;
            }
            public void resetAction() {
                super.resetAction();
                startIndex = -1;
                dataTypes = null;
            }
        }
        // Fill test data type table with something
        byte dataTypes[] = new byte[192];
        for (int i = 0; i < dataTypes.length; i++) {
            dataTypes[i] = (byte) ((i * 5) % 256);
        }

        DefineDataTypeTable ah = new DefineDataTypeTable();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "r" + getHexString(dataTypes, 0, 96),
                      ActionAdapter.ACTION_doDefineDataTypeTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad data type table", dataTypes, 0, 96,
                          ah.dataTypes);
        ah.resetAction();

        assertParseFail(ESC + "r" + getHexString(dataTypes, 0, 95) + "XX",
                        "Escr: Not hex digits");
        ah.resetAction();

        assertParseOK(ESC + "-1;96r" + getHexString(dataTypes, 0, 96),
                      ActionAdapter.ACTION_doDefineDataTypeTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad data type table", dataTypes, 0, 96,
                          ah.dataTypes);
        ah.resetAction();

        assertParseOK(ESC + "-1;192r" + getHexString(dataTypes, 0, 192),
                      ActionAdapter.ACTION_doDefineDataTypeTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad data type table", dataTypes, 0, 192,
                          ah.dataTypes);
        ah.resetAction();

        assertParseOK(ESC + "-;r" + getHexString(dataTypes, 0, 192),
                      ActionAdapter.ACTION_doDefineDataTypeTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad data type table", dataTypes, 0, 192,
                          ah.dataTypes);
        ah.resetAction();

        assertParseOK(ESC + "-1000;1000r" + getHexString(dataTypes, 0, 192),
                      ActionAdapter.ACTION_doDefineDataTypeTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad data type table", dataTypes, 0, 192,
                          ah.dataTypes);
        ah.resetAction();

        assertParseFail(ESC + "-;r" + getHexString(dataTypes, 0, 191)+"XX",
                        "Esc-r: Not hex digits");
        ah.resetAction();
    }

    public void testDoDefineOrUpdateVariableTable() {
        class DefineVariableTable extends ActionAdapter {
            public int startIndex = -1;
            public FieldAttributes attribs[] = null;

            public void doDefineVariableTable(int startIndex,
                                              FieldAttributes attribs[]) {
                super.doDefineVariableTable(startIndex, attribs);
                this.startIndex = startIndex;
                this.attribs = attribs;
            }
            public void resetAction() {
                super.resetAction();
                startIndex = -1;
                attribs = null;
            }
        }
        // Attribute characters
        char chars[][] = {
                             { ' ', ' ', ' ' }, { '0', ' ', ' ' }, { '.', ' ', ' ' },
                             { '!', ' ', ' ' }, { '1', ' ', ' ' }, { '-', ' ', ' ' },
                             { '"', ' ', ' ' }, { '2', ' ', ' ' }, { ':', ' ', ' ' },
                             { '#', ' ', ' ' }, { '3', ' ', ' ' }, { ';', ' ', ' ' },
                             { '$', ' ', ' ' }, { '4', ' ', ' ' }, { '<', ' ', ' ' },
                             { '&', ' ', ' ' }, { '5', ' ', ' ' }, { '=', ' ', ' ' },
                             { '(', ' ', ' ' }, { '6', ' ', ' ' }, { '>', ' ', ' ' },
                             { ')', ' ', ' ' }, { '7', ' ', ' ' }, { '*', ' ', ' ' },
                             { '*', ' ', ' ' }, { '8', ' ', ' ' }, { '0', ' ', ' ' },
                             { '+', ' ', ' ' }, { '9', ' ', ' ' }, { '(', ' ', ' ' },
                             { '/', ' ', ' ' }, { '?', ' ', ' ' } };
        StringBuffer hexAttribs = new StringBuffer(); // attributes as ASCII hex
        FieldAttributes attribs[] = new FieldAttributes[chars.length];
        for (int i = 0; i < chars.length; i++) {
            /* all characters in 'char' are larger than 0x0f, so we don't
             * need to pad the hex value to two characters */
            hexAttribs.append(Integer.toString(chars[i][0], 16));
            hexAttribs.append(Integer.toString(chars[i][1], 16));
            hexAttribs.append(Integer.toString(chars[i][2], 16));

            try {
                attribs[i] = new FieldAttributes(chars[i][0],
                                                 chars[i][1],
                                                 chars[i][2]);
            } catch (ParseException e) {
                fail("Failed to create FieldAttributes for i = " + i +
                     ". Msg = " + e.getMessage());
            }
        }

        DefineVariableTable ah = new DefineVariableTable();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-1s",
                      ActionAdapter.ACTION_doResetVariableTable);
        ah.resetAction();
        assertParseOK(ESC + "-1;;s",
                      ActionAdapter.ACTION_doResetVariableTable);
        ah.resetAction();

        assertParseOK(ESC + "-0;1;32s" + hexAttribs.toString(),
                      ActionAdapter.ACTION_doDefineVariableTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertAttribsEquals("Bad variable table:", attribs, 0, 32, ah.attribs);
        ah.resetAction();

        assertParseOK(ESC + "-0;1;2s" + hexAttribs.substring(0*6, 2*6),
                      ActionAdapter.ACTION_doDefineVariableTable);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertAttribsEquals("Bad variable table:", attribs, 0, 2, ah.attribs);
        ah.resetAction();

        assertParseOK(ESC + "-0;5;10s" + hexAttribs.substring(4*6, 10*6),
                      ActionAdapter.ACTION_doDefineVariableTable);
        assertEquals("Startindex is bad", 5, ah.startIndex);
        assertAttribsEquals("Bad variable table:", attribs, 4, 6, ah.attribs);
        ah.resetAction();

        assertParseFail(ESC + "-s", "Esc-s: Too few arguments");
        ah.resetAction();

        assertParseFail(ESC + "-;;;s", "Esc-s: Too many arguments");
        ah.resetAction();

        assertParseFail(ESC + "-5;10;20s", "Esc-s: Invalid mode");
        ah.resetAction();

        assertParseFail(ESC + "-0;1;1sXXXXXX", "Esc-s: Not hex digits");
        ah.resetAction();
    }
    public void testDoSetColorConfiguration() {
        class SetColorConfiguration extends ActionAdapter {
            public int startIndex = -1;
            public byte colors[] = null;

            public void doSetColorConfiguration(int startIndex,
                                                byte entries[]) {
                super.doSetColorConfiguration(startIndex, entries);
                this.startIndex = startIndex;
                this.colors = entries;
            }
            public void doResetColorConfiguration() {
                super.doResetColorConfiguration();
            }
            public void resetAction() {
                super.resetAction();
                startIndex = -1;
                colors = null;
            }
        }
        // Fill test color map with something
        byte colors[] = new byte[16];
        for (int i = 0; i < colors.length; i++) {
            colors[i] = (byte) ((i * 5) % 256);
        }

        SetColorConfiguration ah = new SetColorConfiguration();
        parser.setActionHandler(ah);

        assertParseOK(ESC + "-1;70;70t",
                      ActionAdapter.ACTION_doResetColorConfiguration);
        ah.resetAction();

        assertParseOK(ESC + "-1t",
                      ActionAdapter.ACTION_doResetColorConfiguration);
        ah.resetAction();

        assertParseOK(ESC + "-0;0;70t"+getHexString(colors, 0, 16),
                      ActionAdapter.ACTION_doSetColorConfiguration);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad color configuration", colors, 0, 16, ah.colors);
        ah.resetAction();

        assertParseOK(ESC + "-0;1;16t"+getHexString(colors, 0, 16),
                      ActionAdapter.ACTION_doSetColorConfiguration);
        assertEquals("Startindex is bad", 1, ah.startIndex);
        assertBytesEquals("Bad color configuration", colors, 0, 16, ah.colors);
        ah.resetAction();

        assertParseOK(ESC + "-0;5;7t"+getHexString(colors, 4, 3),
                      ActionAdapter.ACTION_doSetColorConfiguration);
        assertEquals("Startindex is bad", 5, ah.startIndex);
        assertBytesEquals("Bad color configuration", colors, 4, 3, ah.colors);
        ah.resetAction();

        assertParseFail(ESC + "-;;;t", "Esc-t: Too many arguments");
        ah.resetAction();

        assertParseFail(ESC + "-5;10;20t", "Esc-t: Invalid mode");
        ah.resetAction();

        assertParseFail(ESC + "-0;1;1tXX", "Esc-t: Not hex digits");
        ah.resetAction();
    }

    public void testDoSetTerminalConfiguration() {
        class SetConfiguration extends ActionAdapter {
            public ConfigParameter params[] = null;
            public int device = -1;

            public void doSetTerminalConfiguration(ConfigParameter params[]) {
                super.doSetTerminalConfiguration(params);
                this.params = params;
            }
            public void doSetIODeviceConfiguration(int device,
                                                   ConfigParameter params[]) {
                super.doSetIODeviceConfiguration(device, params);
                this.params = params;
                this.device = device;
            }
            public void resetAction() {
                super.resetAction();
                params = null;
                device = -1;
            }
        }

        SetConfiguration ah = new SetConfiguration();
        parser.setActionHandler(ah);

        /* Test terminal configuration */
        assertParseOK(ESC + "vA0B72" + CR,
                      ActionAdapter.ACTION_doSetTerminalConfiguration);
        assertEquals("Wrong number of parameters", 2, ah.params.length);
        assertEquals("Bad parameter code", 'A', ah.params[0].getCode());
        assertEquals("Bad parameter value", 0, ah.params[0].getValue());
        assertEquals("Bad parameter code", 'B', ah.params[1].getCode());
        assertEquals("Bad parameter value", 72, ah.params[1].getValue());
        ah.resetAction();

        assertParseOK(ESC + "v" + CR,
                      ActionAdapter.ACTION_doSetTerminalConfiguration);
        assertEquals("Parameter array not empty", 0, ah.params.length);
        ah.resetAction();

        assertParseFail(ESC + "v_1" + CR, "Escv: Missing code");
        ah.resetAction();
        assertParseFail(ESC + "vA" + CR, "Escv: Missing value");
        ah.resetAction();
        assertParseFail(ESC + "vAA" + CR, "Escv: Value must be a number");
        ah.resetAction();
        assertParseFail(ESC + "vAAB72" + CR, "Escv: Value must be a number");
        ah.resetAction();

        /* Test IO device configuration */
        assertParseOK(ESC + "xT2G0H1" + CR,
                      ActionAdapter.ACTION_doSetIODeviceConfiguration);
        assertEquals("Wrong number of parameters", 2, ah.params.length);
        assertEquals("Wrong device", 1, ah.device);
        assertEquals("Bad parameter code", 'G', ah.params[0].getCode());
        assertEquals("Bad parameter value", 0, ah.params[0].getValue());
        assertEquals("Bad parameter code", 'H', ah.params[1].getCode());
        assertEquals("Bad parameter value", 1, ah.params[1].getValue());
        ah.resetAction();

        assertParseOK(ESC + "xT3G0H1Ithis is my name" + CR,
                      ActionAdapter.ACTION_doSetIODeviceConfiguration);
        assertEquals("Wrong number of parameters", 3, ah.params.length);
        assertEquals("Wrong device", 2, ah.device);
        assertEquals("Bad parameter code", 'G', ah.params[0].getCode());
        assertEquals("Bad parameter value", 0, ah.params[0].getValue());
        assertEquals("Bad parameter code", 'H', ah.params[1].getCode());
        assertEquals("Bad parameter value", 1, ah.params[1].getValue());
        assertEquals("Bad parameter code", 'I', ah.params[2].getCode());
        assertEquals("Bad parameter type",
                     true, ah.params[2].hasStringValue());
        assertEquals("Bad parameter value", -1, ah.params[2].getValue());
        assertEquals("Bad parameter value",
                     "this is my name", ah.params[2].getStringValue());
        ah.resetAction();

        assertParseOK(ESC + "xT2" + CR,
                      ActionAdapter.ACTION_doSetIODeviceConfiguration);
        assertEquals("Parameter array not empty", 0, ah.params.length);
        ah.resetAction();

        assertParseFail(ESC + "x" + CR, "Escx: Invalid device");
        ah.resetAction();
        assertParseFail(ESC + "xT5A0" + CR, "Escx: Invalid device");
        ah.resetAction();
        assertParseFail(ESC + "xT2_1" + CR, "Escx: Missing code");
        ah.resetAction();
        assertParseFail(ESC + "xT2A" + CR, "Escx: Missing value");
        ah.resetAction();

    }

    /** Adds a Longitudinal Redundancy Check to in string */
    private String addLRC(String in) {
        return in + (char) 0xff;    // XXX fix me!!!
    }

    public void testOSCommands() {
        String input;
        class OSCommands extends ActionAdapter {
            public String execString = null;

            public String device = null;
            public int opCode = -1;
            public byte data[] = null;

            public void doLoadAndExecuteAnOperatingSystemProgram(
                String execString) {
                super.doLoadAndExecuteAnOperatingSystemProgram(execString);
                this.execString = execString;
            }

            public void doWriteToFileOrDeviceName(String device,
                                                  int opCode,
                                                  byte data[]) {
                super.doWriteToFileOrDeviceName(device, opCode, data);
                this.device = device;
                this.opCode = opCode;
                this.data = data;
            }

            public void doWriteOrReadToFileOrDeviceName(String device,
                    int opCode,
                    byte data[]) {
                super.doWriteOrReadToFileOrDeviceName(device, opCode, data);
                this.device = device;
                this.opCode = opCode;
                this.data = data;
            }

            public void resetAction() {
                super.resetAction();
                execString = null;
                device = null;
                opCode = -1;
                data = null;
            }
        }

        OSCommands ah = new OSCommands();
        parser.setActionHandler(ah);

        /* Test execute OS program */
        input = ESC + "-Vfilename.exe-" + ETX;
        assertParseOK(addLRC(input),
                      ActionAdapter.ACTION_doLoadAndExecuteAnOperatingSystemProgram);
        assertEquals("Bad exec string", "filename.exe", ah.execString);
        ah.resetAction();

        input = ESC + "-Vfilename.exe param1 param2-" + ETX;
        assertParseOK(addLRC(input),
                      ActionAdapter.ACTION_doLoadAndExecuteAnOperatingSystemProgram);
        assertEquals("Bad filename",
                     "filename.exe param1 param2", ah.execString);
        ah.resetAction();

        input = ESC + "-V-" + ETX;
        assertParseFail(addLRC(input), "Esc-V: Filename missing");
        ah.resetAction();

        /* Test write to file or device */
        byte trasanApanson[] = {
                                   (byte) 0x74, (byte) 0x72, (byte) 0x61, (byte) 0x73, (byte) 0x61,
                                   (byte) 0x6e, (byte) 0x20, (byte) 0x61, (byte) 0x70, (byte) 0x61,
                                   (byte) 0x6e, (byte) 0x73, (byte) 0x6f, (byte) 0x6e
                               };
        assertParseOK(ESC + "{\"dev\"3f " + ESC + "a" +
                      "trasan apanson" + CR,
                      ActionAdapter.ACTION_doWriteToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x3f, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseOK(ESC + "{\"dev\"42 " + ESC + "b" +
                      "74726173616e206170616e736f6e" + CR,
                      ActionAdapter.ACTION_doWriteToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x42, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseOK(ESC + "{\"dev\"42 " +
                      ESC + "b" + "74726173616e20" +
                      ESC + "a" + "apanson" + CR,
                      ActionAdapter.ACTION_doWriteToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x42, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseOK(ESC + "{\"dev\"42 " +
                      ESC + "a" + "trasan " +
                      ESC + "b" + "6170616e736f6e" + CR,
                      ActionAdapter.ACTION_doWriteToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x42, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseFail(ESC + "{\"dev\"42 " + ESC + "b" + "a" + CR,
                        "Esc{: Uneven number of data characters");
        ah.resetAction();
        assertParseFail(ESC + "{\"dev\"42 " + ESC + "b" + "apan" + CR,
                        13, "Esc{: Invalid data character");
        ah.resetAction();
        assertParseFail(ESC + "{\"dev\"ff " + ESC + "a" + "apa" + CR,
                        "Esc{: Invalid opcode");
        ah.resetAction();

        /* Test write or read to file or device */
        assertParseOK(ESC + "}\"dev\"3f " + ESC + "a" +
                      "trasan apanson" + CR,
                      ActionAdapter.ACTION_doWriteOrReadToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x3f, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseOK(ESC + "}\"dev\"42 " + ESC + "b" +
                      "74726173616e206170616e736f6e" + CR,
                      ActionAdapter.ACTION_doWriteOrReadToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x42, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseOK(ESC + "}\"dev\"42 " +
                      ESC + "b" + "74726173616e20" +
                      ESC + "a" + "apanson" + CR,
                      ActionAdapter.ACTION_doWriteOrReadToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x42, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseOK(ESC + "}\"dev\"42 " +
                      ESC + "a" + "trasan " +
                      ESC + "b" + "6170616e736f6e" + CR,
                      ActionAdapter.ACTION_doWriteOrReadToFileOrDeviceName);
        assertEquals("Bad device", "dev", ah.device);
        assertEquals("Bad opCode", 0x42, ah.opCode);
        assertBytesEquals("Bad data", trasanApanson, ah.data);
        ah.resetAction();

        assertParseFail(ESC + "}\"dev\"42 " + ESC + "b" + "a" + CR,
                        "Esc}: Uneven number of data characters");
        ah.resetAction();
        assertParseFail(ESC + "}\"dev\"42 " + ESC + "b" + "apan" + CR,
                        13, "Esc}: Invalid data character");
        ah.resetAction();
        assertParseFail(ESC + "}\"dev\"ff " + ESC + "a" + "apa" + CR,
                        "Esc}: Invalid opcode");
        ah.resetAction();
    }

    public void testModeSwitch() {
        String input;

        class ModeSwitch extends ActionAdapter {
            public boolean convMode = true;
            public boolean blockMode = false;

            public void doSetConversationalMode() {
                super.doSetConversationalMode();
                convMode = true;
                blockMode = false;
            }
            public void doSetBlockMode() {
                super.doSetBlockMode();
                convMode = false;
                blockMode = true;
            }
            public void resetAction() {
                super.resetAction();
                convMode = true;
                blockMode = false;
            }
        }

        ModeSwitch ah = new ModeSwitch();
        parser.setActionHandler(ah);

        input = SOH + "C" + ETX;
        assertParseOK(addLRC(input),
                      ActionAdapter.ACTION_doSetConversationalMode);
        assertTrue("Conversational mode not set", ah.convMode && !ah.blockMode);
        ah.resetAction();

        input = SOH + "B" + ETX;
        assertParseOK(addLRC(input), ActionAdapter.ACTION_doSetBlockMode);
        assertTrue("Block mode not set", !ah.convMode && ah.blockMode);
        ah.resetAction();

        input = SOH + "X" + ETX;
        assertParseFail(addLRC(input), 1, "Mode switch: Unknown mode 'X'");
    }

}


