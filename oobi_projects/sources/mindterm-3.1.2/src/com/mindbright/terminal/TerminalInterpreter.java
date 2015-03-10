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

public abstract class TerminalInterpreter {

    protected CompatTerminal term;

    public final static int IGNORE = -1;

    abstract public String terminalType();
    abstract public int interpretChar(char c);

    public void vtReset() {}

    public void keyHandler(char c, int virtualKey, int modifiers) {
        if (term != null) {
            term.typedChar(c);
        }
    }
    public void mouseHandler(int x, int y, boolean press, int modifiers) {}

    public final void setTerminal(CompatTerminal term) {
        this.term = term;
    }

    public void setDumbMode(boolean dumb) {}
}
