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

import java.util.Vector;

public class ConfigParameter {
    private char code;
    private int value = -1;
    private String strValue = null;

    public static ConfigParameter[] parse(String input)
    throws ParseException {
        Vector params = new Vector();
        StringBuffer buf = new StringBuffer(input);

        while (buf.length() > 0) {
            params.add(parseNextParam(buf));
        }

        ConfigParameter array[] = new ConfigParameter[params.size()];
        for (int i = 0; i < params.size(); i++) {
            array[i] = (ConfigParameter) params.elementAt(i);
        }
        return array;
    }

    private static ConfigParameter parseNextParam(StringBuffer buf)
    throws ParseException {
        char code;

        code = buf.charAt(0);
        if (!Character.isLetter(code)) {
            throw new ParseException("Missing code");
        }
        removeFirstChar(buf);

        if (buf.length() == 0) {
            throw new ParseException("Missing value");
        }


        /* Find out if first non-whitespace char is a digit */
        boolean isNumberValue = false;
        for (int i = 0; i < buf.length(); i++) {
            if (buf.charAt(i) == ' ') {
                continue;
            } else if (Character.isDigit(buf.charAt(i))) {
                isNumberValue = true;
                break;
            } else {
                break;
            }
        }

        ConfigParameter ret;
        if (isNumberValue) {
            /* remove whitespace at the beginning */
            while (buf.length() > 0 && buf.charAt(0) == ' ') {
                removeFirstChar(buf);
            }

            StringBuffer numBuf = new StringBuffer();
            while (buf.length() > 0 && Character.isDigit(buf.charAt(0))) {
                numBuf.append(removeFirstChar(buf));
            }

            int num = 0;
            try {
                num = Integer.parseInt(numBuf.toString(), 10);
            } catch (NumberFormatException e) {
                throw new ParseException("Value must be a number");
            }
            ret = new ConfigParameter(code, num);

            /* remove trailing whitespace */
            while (buf.length() > 0 && buf.charAt(0) == ' ') {
                removeFirstChar(buf);
            }
        } else {
            /* Must be a string then. Grab all characters to end of string */
            ret = new ConfigParameter(code, buf.toString());
            buf.setLength(0);
        }
        return ret;
    }

    /** Remove first character in buf. StringBuffer as of java 1.1 don't
     * have this operation. :-(
     */
    private static char removeFirstChar(StringBuffer buf) {
        char first = buf.charAt(0);
        buf.reverse();
        buf.setLength(buf.length() - 1);
        buf.reverse();
        return first;
    }

    public ConfigParameter(char code, int value) {
        this.code = code;
        this.value = value;
    }
    public ConfigParameter(char code, String value) {
        this.code = code;
        this.strValue = value;
    }

    public char getCode() {
        return code;
    }
    public int getValue() {
        return value;
    }
    public String getStringValue() {
        return strValue;
    }
    public boolean hasStringValue() {
        return strValue != null;
    }

    public String toString() {
        if (hasStringValue()) {
            return code + " " + strValue;
        }
        return code + " " + String.valueOf(value);
    }
}


