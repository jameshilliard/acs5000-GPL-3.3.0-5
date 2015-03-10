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
 * Interface for printers which may be attached to terminals.
 */
public interface TerminalPrinter extends TerminalOutputListener {
    /**
     * Print a dump of the current screen.
     */
    public void printScreen();

    /**
     * Start printing everything which is shown on the terminal from
     * now on.
     */
    public void startPrinter();

    /**
     * Stop dumping data to the printer.
     */
    public void stopPrinter();
}
