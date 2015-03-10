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

import com.mindbright.terminal.tandem6530.ParseException;
import com.mindbright.terminal.tandem6530.ConfigParameter;

public class TestConfigParameter extends TestCase {
    ConfigParameter result[] = null;

    public void setUp() {}

    public void tearDown() {}


    protected void assertParseOK(String input) {
        result = null;
        try {
            result = ConfigParameter.parse(input);
        } catch (ParseException e) {
            throw new AssertionFailedError(
                "Unexpected exception: " + e.getMessage());
        }
    }

    protected void assertParseFail(String input, String expMessage) {
        String exceptMsg = null;
        try {
            ConfigParameter.parse(input);
        } catch (ParseException e) {
            exceptMsg = e.getMessage();
        }
        if (exceptMsg == null) {
            throw new AssertionFailedError(
                "Expected an exception with message '"
                + expMessage + "' here.");
        }
        if (!expMessage.equals(expMessage)) {
            throw new AssertionFailedError(
                "Expected an exception with message '" +
                expMessage + "' here, got message '" +
                exceptMsg +"' instead.");
        }
    }


    public void testParse() {
        assertParseOK("A0 ");
        assertNotNull("Result was null",           result);
        assertEquals("Result length was wrong", 1, result.length);
        assertEquals("Code was wrong", 'A',        result[0].getCode());
        assertEquals("Value was wrong", 0,         result[0].getValue());

        assertParseOK("A 0 B72");
        assertNotNull("Result was null",           result);
        assertEquals("Result length was wrong", 2, result.length);
        assertEquals("Code was wrong", 'A',        result[0].getCode());
        assertEquals("Not integer", false,         result[0].hasStringValue());
        assertEquals("Value was wrong", 0,         result[0].getValue());
        assertEquals("Code was wrong", 'B',        result[1].getCode());
        assertEquals("Not integer", false,         result[1].hasStringValue());
        assertEquals("Value was wrong", 72,        result[1].getValue());

        assertParseOK("A  0B72 I apa   ");
        assertNotNull("Result was null",           result);
        assertEquals("Result length was wrong", 3, result.length);
        assertEquals("Code was wrong", 'A',        result[0].getCode());
        assertEquals("Not integer", false,         result[0].hasStringValue());
        assertEquals("Value was wrong", 0,         result[0].getValue());
        assertEquals("Code was wrong", 'B',        result[1].getCode());
        assertEquals("Not integer", false,         result[1].hasStringValue());
        assertEquals("Value was wrong", 72,        result[1].getValue());
        assertEquals("Code was wrong", 'I',        result[2].getCode());
        assertEquals("Not string", true,           result[2].hasStringValue());
        assertEquals("Value was wrong", " apa   ", result[2].getStringValue());

        assertParseFail("0", "Missing code");
        assertParseFail("A", "Missing value");
    }
}

