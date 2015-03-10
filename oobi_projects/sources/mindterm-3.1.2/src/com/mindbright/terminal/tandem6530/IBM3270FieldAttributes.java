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

public class IBM3270FieldAttributes extends FieldAttributes {
    public static final int DEFAULT_FG_COLOR = 4;
    public static final int DEFAULT_BG_COLOR = 0;

    private int fgColor = DEFAULT_FG_COLOR;
    private int bgColor = DEFAULT_BG_COLOR;

    public IBM3270FieldAttributes(char video, char data, char extData)
    throws ParseException {
        super(video, data, extData);
    }

    public int getFgColor() {
        return fgColor;
    }
    public void setFgColor(int color) throws BadColorException {
        if (color < 0 || color > 15) {
            throw new BadColorException("Invalid color");
        }
        fgColor = color;
    }
    public int getBgColor() {
        return bgColor;
    }
    public void setBgColor(int color) throws BadColorException {
        if (color < 0 || color > 15) {
            throw new BadColorException("Invalid color");
        }
        bgColor = color;
    }

    public int hashCode() {
        return super.hashCode() + 4000 * getFgColor() + 8000 * getBgColor();
    }

    public boolean equals(Object o) {
        IBM3270FieldAttributes other = (IBM3270FieldAttributes) o;
        if (other == null) {
            return false;
        }
        return super.equals(o) &&
               getFgColor() == other.getFgColor() &&
               getBgColor() == other.getBgColor();
    }

    public String toString() {
        return super.toString() +
               " fg="+getFgColor() +
               " bg="+getBgColor();
    }
}



