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


import java.awt.event.KeyEvent;
import java.awt.event.InputEvent;

import junit.framework.TestCase;


import com.mindbright.terminal.tandem6530.Terminal6530;

public class TestTerminal6530 extends TestCase implements AsciiCodes {
    Terminal6530 term;
    MockTerminalWindow mockWin = new MockTerminalWindow();

    String expDefaultConfig = SOH + "!" +
                              "A 3" +     // cursor block blinking
                              "B72" +     // bell column number
                              "C 1" +     // Bell on
                              "D 0" +     // key click off
                              "E 0" +     // status line border off
                              "F 0" +     // Language ASCII
                              "G 0" +     // Mode: conversersational
                              "H 8" +     // Baud rate: no change
                              "I 3" +     // Parity: none
                              "J 0" +     // Duplex full
                              "K 0" +     // Default device ID: none
                              "L 1" +     // Packet blocking: 256 (on)
                              "M 0" +     // Return key function: off
                              "N 0" +     // Single-page submode: off
                              "O 0" +     // Save configuration: temporary
                              "P 0" +     // Screen saver: 0
                              "S 0" +     // Screen format: 25*80
                              "T 0" +     // Normal intensity: high
                              "U 1" +     // Local transmit column: 1
                              "V 1" +     // Character size: 8 bits
                              "W 0" +     // Character set: single byte
                              "X 7" +     // Keyboard: PSX105
                              "e 0" +     // Color support: none
                              "f 0" +     // compression enhance: no
                              "i 0" +     // MLAN host init IXF 6530
                              "j 0" +     // RTM support: off
                              "h00";      // EM3270 support: no PFKey and no EM3270 support

    String setConfig = ESC + "v" +
                       "A 1" +     // cursor block blinking
                       "B50" +     // bell column number
                       "C 0" +     // Bell off
                       "D 1" +     // key click on
                       "E 0" +     // status line border off **
                       "F27" +     // Language: Swedish
                       "G 3" +     // Mode: ANSI??
                       "H16" +     // Baud rate: 38400
                       "I 2" +     // Parity: odd
                       "J 1" +     // Duplex half
                       "K 1" +     // Default device ID: BAD **
                       "L 8" +     // Packet blocking: 2048 (on)
                       "M 1" +     // Return key function: on
                       "N 1" +     // Single-page submode: on **
                       "O 1" +     // Save configuration: BAD **
                       "P 1" +     // Screen saver: 1 min
                       "S 1" +     // Screen format: BAD **
                       "T 1" +     // Normal intensity: low
                       "U10" +     // Local transmit column: 10
                       "V 0" +     // Character size: 7 bits
                       "W 1" +     // Character set: BAD **
                       "X 0" +     // Keyboard: 654X **
                       "e 3" +     // Color support: VGA
                       "f 1" +     // compression enhance: on
                       "i 1" +     // MLAN host init IXF: TTE
                       "j 1" +     // RTM support: on
                       "h11" +     // EM3270 support: PFKey and EM3270 support
                       CR;

    String expConfig = SOH + "!" +
                       "A 1" +     // cursor block blinking
                       "B50" +     // bell column number
                       "C 0" +     // Bell on
                       "D 0" +     // key click off
                       "E 0" +     // status line border off
                       "F27" +     // Language: swedish
                       "G 3" +     // Mode: ANSI
                       "H16" +     // Baud rate: 38400
                       "I 2" +     // Parity: odd
                       "J 1" +     // Duplex half
                       "K 0" +     // Default device ID: none
                       "L 8" +     // Packet blocking: 2048 (on)
                       "M 0" +     // Return key function: off
                       "N 0" +     // Single-page submode: off
                       "O 0" +     // Save configuration: temporary
                       "P 1" +     // Screen saver: 1 min
                       "S 0" +     // Screen format: 25*80
                       "T 1" +     // Normal intensity: low
                       "U10" +     // Local transmit column: 10
                       "V 0" +     // Character size: 7 bits
                       "W 0" +     // Character set: single byte
                       "X 7" +     // Keyboard: PSX105
                       "e 0" +     // Color support: none
                       "f 0" +     // compression enhance: no
                       "i 0" +     // MLAN host init IXF 6530
                       "j 0" +     // RTM support: off
                       "h00";      // EM3270 support: no PFKey and no EM3270 support

    public void setUp() {
        term = new Terminal6530();
        term.setTerminalWindow(mockWin);
    }

    public void tearDown() {}

    private String send(String str) {
        char ca[] = str.toCharArray();

        for (int i = 0; i < ca.length; i++) {
            term.fromHost(ca[i]);
        }
        return recieve();
    }
    private String recieve() {
        return mockWin.getSentChars();
    }

    public void testConvConfiguration() {
        String config;

        send(SOH + "C" + ETX + LRC); // Set conv mode
        // Get the default configuration
        config = send(ESC + "?"); // Read configuration

        String exp = expDefaultConfig + CR;
        assertEquals("Default configuration differs", exp, config);

        // Set values for all parameters
        assertNull("Terminal should not send back anything", send(setConfig));

        // Check that the new values are returned
        config = send(ESC + "?"); // Read configuration
        exp = expConfig + CR;
        assertEquals("Configuration differs", exp, config);
    }

    public void testBlockConfiguration() {
        String config;

        send(SOH + "B" + ETX + LRC); // Set block mode
        // Get the default configuration
        config = send(ESC + "?"); // Read configuration

        String exp = expDefaultConfig + ETX + LRC;
        assertEquals("Default configuration differs", exp, config);

        // Set values for all parameters
        assertNull("Terminal should not send back anything", send(setConfig));

        // Check that the new values are returned
        config = send(ESC + "?"); // Read configuration
        exp = expConfig + ETX + LRC;
        assertEquals("Configuration differs", exp, config);
    }

    class KeyPress {
        char expKeyCode;
        int keyEvent;
        boolean shift;
        boolean alt;
        boolean ctrl;

        KeyPress(char expKeyCode, int event, boolean shift, boolean alt,
                 boolean ctrl) {
            this.expKeyCode = expKeyCode;
            this.keyEvent = event;
            this.shift = shift;
            this.alt = alt;
            this.ctrl = ctrl;
        }

        void press(Terminal6530 term) {
            int mod = 0;
            mod |= shift ? InputEvent.SHIFT_MASK : 0;
            mod |= alt   ? InputEvent.ALT_MASK : 0;
            mod |= ctrl  ? InputEvent.CTRL_MASK : 0;
            term.keyHandler((char) 65535, keyEvent, mod);
        }

        String getExpKeyCode() {
            return String.valueOf(expKeyCode);
        }
        public String toString() {
            return "keyCode = " + expKeyCode + "(" +
                   (shift ? "S" : "_") +
                   (alt   ? "A" : "_") +
                   (ctrl  ? "C" : "_") + ")";
        }
    }

    public void testConvFunctionKeys() {
        String tail = "  " + CR; // cursor at 1,1

        send(SOH + "C" + ETX + LRC); // Set block mode

        KeyPress tests[] = {// key                     shift  alt    ctrl
                               new KeyPress('@',  KeyEvent.VK_F1,         false, false, false),
                               new KeyPress('`',  KeyEvent.VK_F1,         true,  false, false),
                               new KeyPress('J',  KeyEvent.VK_F1,         false, true,  false),
                               new KeyPress('j',  KeyEvent.VK_F1,         true,  true,  false),

                               new KeyPress('A',  KeyEvent.VK_F2,         false, false, false),
                               new KeyPress('a',  KeyEvent.VK_F2,         true,  false, false),
                               new KeyPress('K',  KeyEvent.VK_F2,         false, true,  false),
                               new KeyPress('k',  KeyEvent.VK_F2,         true,  true,  false),

                               new KeyPress('B',  KeyEvent.VK_F3,         false, false, false),
                               new KeyPress('b',  KeyEvent.VK_F3,         true,  false, false),
                               new KeyPress('L',  KeyEvent.VK_F3,         false, true,  false),
                               new KeyPress('l',  KeyEvent.VK_F3,         true,  true,  false),

                               new KeyPress('C',  KeyEvent.VK_F4,         false, false, false),
                               new KeyPress('c',  KeyEvent.VK_F4,         true,  false, false),
                               new KeyPress('M',  KeyEvent.VK_F4,         false, true,  false),
                               new KeyPress('m',  KeyEvent.VK_F4,         true,  true,  false),

                               new KeyPress('D',  KeyEvent.VK_F5,         false, false, false),
                               new KeyPress('d',  KeyEvent.VK_F5,         true,  false, false),
                               new KeyPress('N',  KeyEvent.VK_F5,         false, true,  false),
                               new KeyPress('n',  KeyEvent.VK_F5,         true,  true,  false),

                               new KeyPress('E',  KeyEvent.VK_F6,         false, false, false),
                               new KeyPress('e',  KeyEvent.VK_F6,         true,  false, false),
                               new KeyPress('O',  KeyEvent.VK_F6,         false, true,  false),
                               new KeyPress('o',  KeyEvent.VK_F6,         true,  true,  false),

                               new KeyPress('F',  KeyEvent.VK_F7,         false, false, false),
                               new KeyPress('f',  KeyEvent.VK_F7,         true,  false, false),

                               new KeyPress('G',  KeyEvent.VK_F8,         false, false, false),
                               new KeyPress('g',  KeyEvent.VK_F8,         true,  false, false),

                               new KeyPress('H',  KeyEvent.VK_F9,         false, false, false),
                               new KeyPress('h',  KeyEvent.VK_F9,         true,  false, false),

                               new KeyPress('I',  KeyEvent.VK_F10,        false, false, false),
                               new KeyPress('i',  KeyEvent.VK_F10,        true,  false, false),

                           };

        String exp = null;
        String actual = null;
        for (int i = 0; i < tests.length; i++) {
            term.doUnlockKeyboard();
            tests[i].press(term);

            exp = SOH + tests[i].getExpKeyCode() + tail;
            actual = recieve();
            assertEquals("i="+i+" "+tests[i], exp, actual);
        }
    }

    public void testBlockFunctionKeys() {
        String tail = "!  " + ETX + LRC; // 1:st page, cursor at 1,1

        send(SOH + "B" + ETX + LRC); // Set block mode
        term.doDisableLocalLineEditing(); // Make delete, insert function keys
        send(ESC + "v" + "M 1" + CR);     // Make enter function key

        KeyPress tests[] = {// key                     shift  alt    ctrl
                               new KeyPress('@',  KeyEvent.VK_F1,         false, false, false),
                               new KeyPress('`',  KeyEvent.VK_F1,         true,  false, false),
                               new KeyPress('J',  KeyEvent.VK_F1,         false, true,  false),
                               new KeyPress('j',  KeyEvent.VK_F1,         true,  true,  false),

                               new KeyPress('A',  KeyEvent.VK_F2,         false, false, false),
                               new KeyPress('a',  KeyEvent.VK_F2,         true,  false, false),
                               new KeyPress('K',  KeyEvent.VK_F2,         false, true,  false),
                               new KeyPress('k',  KeyEvent.VK_F2,         true,  true,  false),

                               new KeyPress('B',  KeyEvent.VK_F3,         false, false, false),
                               new KeyPress('b',  KeyEvent.VK_F3,         true,  false, false),
                               new KeyPress('L',  KeyEvent.VK_F3,         false, true,  false),
                               new KeyPress('l',  KeyEvent.VK_F3,         true,  true,  false),

                               new KeyPress('C',  KeyEvent.VK_F4,         false, false, false),
                               new KeyPress('c',  KeyEvent.VK_F4,         true,  false, false),
                               new KeyPress('M',  KeyEvent.VK_F4,         false, true,  false),
                               new KeyPress('m',  KeyEvent.VK_F4,         true,  true,  false),

                               new KeyPress('D',  KeyEvent.VK_F5,         false, false, false),
                               new KeyPress('d',  KeyEvent.VK_F5,         true,  false, false),
                               new KeyPress('N',  KeyEvent.VK_F5,         false, true,  false),
                               new KeyPress('n',  KeyEvent.VK_F5,         true,  true,  false),

                               new KeyPress('E',  KeyEvent.VK_F6,         false, false, false),
                               new KeyPress('e',  KeyEvent.VK_F6,         true,  false, false),
                               new KeyPress('O',  KeyEvent.VK_F6,         false, true,  false),
                               new KeyPress('o',  KeyEvent.VK_F6,         true,  true,  false),

                               new KeyPress('F',  KeyEvent.VK_F7,         false, false, false),
                               new KeyPress('f',  KeyEvent.VK_F7,         true,  false, false),

                               new KeyPress('G',  KeyEvent.VK_F8,         false, false, false),
                               new KeyPress('g',  KeyEvent.VK_F8,         true,  false, false),

                               new KeyPress('H',  KeyEvent.VK_F9,         false, false, false),
                               new KeyPress('h',  KeyEvent.VK_F9,         true,  false, false),

                               new KeyPress('I',  KeyEvent.VK_F10,        false, false, false),
                               new KeyPress('i',  KeyEvent.VK_F10,        true,  false, false),

                               new KeyPress('P',  KeyEvent.VK_UP,         false, true,  false),
                               new KeyPress('p',  KeyEvent.VK_UP,         true,  true,  false),

                               new KeyPress('Q',  KeyEvent.VK_DOWN,       false, true,  false),
                               new KeyPress('q',  KeyEvent.VK_DOWN,       true,  true,  false),

                               new KeyPress('R',  KeyEvent.VK_PAGE_DOWN,  false, false, false),
                               new KeyPress('r',  KeyEvent.VK_PAGE_DOWN,  false, true,  false),

                               new KeyPress('S',  KeyEvent.VK_PAGE_UP,    false, false, false),
                               new KeyPress('s',  KeyEvent.VK_PAGE_UP,    false, true,  false),

                               new KeyPress('T',  KeyEvent.VK_INSERT,     false, false, true),
                               new KeyPress('t',  KeyEvent.VK_DELETE,     false, false, true),

                               new KeyPress('V',  KeyEvent.VK_ENTER,      false, false, false),
                               new KeyPress('v',  KeyEvent.VK_ENTER,      true,  false, false),
                           };

        String exp = null;
        String actual = null;
        for (int i = 0; i < tests.length; i++) {
            term.doUnlockKeyboard();
            tests[i].press(term);

            exp = SOH + tests[i].getExpKeyCode() + tail;
            actual = recieve();
            assertEquals("i="+i+" "+tests[i], exp, actual);
        }

        KeyPress bogus =
            new KeyPress('O',  KeyEvent.VK_F6,         false, true,  true);
        term.doUnlockKeyboard();
        bogus.press(term);
        assertNull("Bad keypressed generated response", recieve());

        /*
        for (int j = 0; j < exp.length() && j < actual.length(); j++) {
            System.out.println(
                    "exp="+Integer.toHexString((int) exp.charAt(j)) + "  " +
                    "actual="+Integer.toHexString((int) actual.charAt(j)));
        }
        */
    }

    public void testConvTerminalStatus() {
        String actual;

        String expFirstResponse =  SOH + "?" + "A" + "F" + "G" + CR;
        String expSecondResponse = SOH + "?" + "C" + "F" + "G" + CR;

        term.doReadTerminalStatus();
        actual = recieve();
        assertEquals(expFirstResponse, actual);

        term.doReadTerminalStatus();
        actual = recieve();
        assertEquals(expSecondResponse, actual);
    }

    public void testBlockTerminalStatus() {
        term.doSetBlockMode();
        String actual;

        String expFirstResponse =  SOH + "?" + "A" + "F" + "G" +
                                   "*" + "?>" + ETX + LRC;
        String expSecondResponse = SOH + "?" + "C" + "F" + "G" +
                                   "*" + "?>" + ETX + LRC;

        term.doReadTerminalStatus();
        actual = recieve();
        assertEquals(expFirstResponse, actual);

        term.doReadTerminalStatus();
        actual = recieve();
        assertEquals(expSecondResponse, actual);
    }

}
