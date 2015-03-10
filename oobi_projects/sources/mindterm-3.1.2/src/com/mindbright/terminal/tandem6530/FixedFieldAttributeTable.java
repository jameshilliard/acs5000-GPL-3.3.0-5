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

/** Fixed Field Definition Table (table 3-9).
 * This table is supposed to be 64 entries, but only the first 24 is
 * listen in table 3-9. When I looked at the x3270 source I found this
 * comment about 3270 attributes:
 * <code>
 * field attribute definitions
 *      The 3270 fonts are based on the 3270 character generator font found on
 *      page 12-2 in the IBM 3270 Information Display System Character Set
 *      Reference.  Characters 0xC0 through 0xCF and 0xE0 through 0xEF
 *      (inclusive) are purposely left blank and are used to represent field
 *      attributes as follows:
 *
 *              11x0xxxx
 *                | ||||
 *                | ||++--- 00 normal intensity/non-selectable
 *                | ||      01 normal intensity/selectable
 *                | ||      10 high intensity/selectable
 *                | ||      11 zero intensity/non-selectable
 *                | |+----- unprotected(0)/protected(1)
 *                | +------ alphanumeric(0)/numeric(1)
 *                +-------- unmodified(0)/modified(1)
 * </code>
 * As you can see, this definition gives 16 possible variations...
 * I don't known what to do, so I copy the 24 entries until all 64
 * positions are filled. :-(
 */

public class FixedFieldAttributeTable extends VariableFieldAttributeTable {
    static {
        try {
            FieldAttributes tmp[] = {
                                        // Nomal video
                                        // Data attrib = unprotected, autotab, MDT not set, data type 0
                                        /*  0 */   new FieldAttributes((char) 0x20, (char) 0x40, (char) 0x44),
                                        // Data attrib = unprotected, autotab, MDT set, data type 0
                                        new FieldAttributes((char) 0x20, (char) 0x41, (char) 0x44),

                                        // Data attrib = unprotected, autotab, MDT not set, data type 4
                                        new FieldAttributes((char) 0x20, (char) 0x48, (char) 0x44),
                                        // Data attrib = unprotected, autotab, MDT set, data type 4
                                        new FieldAttributes((char) 0x20, (char) 0x49, (char) 0x44),

                                        // Data attrib = protected, autotab
                                        new FieldAttributes((char) 0x20, (char) 0x60, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x69, (char) 0x44),

                                        // Normal video + dim intensity
                                        /*   8 */  new FieldAttributes((char) 0x21, (char) 0x40, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x41, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x48, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x49, (char) 0x44),

                                        new FieldAttributes((char) 0x21, (char) 0x60, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x69, (char) 0x44),

                                        // Normal video + invisible
                                        /*  16 */  new FieldAttributes((char) 0x28, (char) 0x40, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x41, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x48, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x49, (char) 0x44),

                                        new FieldAttributes((char) 0x28, (char) 0x60, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x69, (char) 0x44),

                                        // Table repeat
                                        /*  24 */  new FieldAttributes((char) 0x20, (char) 0x40, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x41, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x48, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x49, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x60, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x69, (char) 0x44),

                                        /*  32 */  new FieldAttributes((char) 0x21, (char) 0x40, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x41, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x48, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x49, (char) 0x44),
                                        new FieldAttributes((char) 0x30, (char) 0x40, (char) 0x44), //X
                                        new FieldAttributes((char) 0x21, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x69, (char) 0x44),

                                        /*  40 */  new FieldAttributes((char) 0x28, (char) 0x40, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x41, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x48, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x49, (char) 0x44),
                                        new FieldAttributes((char) 0x30, (char) 0x40, (char) 0x44), // X
                                        new FieldAttributes((char) 0x28, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x28, (char) 0x69, (char) 0x44),

                                        // Table repeat
                                        /*  48 */  new FieldAttributes((char) 0x20, (char) 0x40, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x41, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x48, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x49, (char) 0x44),
                                        new FieldAttributes((char) 0x30, (char) 0x40, (char) 0x44),//X
                                        new FieldAttributes((char) 0x20, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x20, (char) 0x69, (char) 0x44),

                                        /*  56 */  new FieldAttributes((char) 0x21, (char) 0x40, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x41, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x48, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x49, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x60, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x61, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x68, (char) 0x44),
                                        new FieldAttributes((char) 0x21, (char) 0x69, (char) 0x44),
                                    };
            defaultTable = tmp;
        } catch (ParseException e) {
            e.printStackTrace();
        }
    }

    public FixedFieldAttributeTable() {
        reset();
    }

    public void set
        (int startIndex, FieldAttributes entries[]) {
        // This table is fixed
    }
}



