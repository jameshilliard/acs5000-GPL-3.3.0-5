/*
 * SpecialKeysUTF8.java
 *
 * Created on May 11, 2007, 11:27
 *
 */

package com.mindbright.terminal;

import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author ivan.gardino
 */
public class SpecialKeysUTF8 {
    
    private static Map map = null;

    static{
        map = new HashMap();
        //map for ctrl+key events...
        map.put(new Character(((char)(byte)1)), new Character('a'));
        map.put(new Character(((char)(byte)2)), new Character('b'));
        map.put(new Character(((char)(byte)3)), new Character('c'));
        map.put(new Character(((char)(byte)4)), new Character('d'));
        map.put(new Character(((char)(byte)5)), new Character('e'));
        map.put(new Character(((char)(byte)6)), new Character('f'));
        map.put(new Character(((char)(byte)7)), new Character('g'));
        map.put(new Character(((char)(byte)8)), new Character('h'));
        map.put(new Character(((char)(byte)9)), new Character('i'));
        map.put(new Character(((char)(byte)10)), new Character('j'));
        map.put(new Character(((char)(byte)11)), new Character('k'));
        map.put(new Character(((char)(byte)12)), new Character('l'));
        map.put(new Character(((char)(byte)13)), new Character('m'));
        map.put(new Character(((char)(byte)14)), new Character('n'));
        map.put(new Character(((char)(byte)15)), new Character('o'));
        map.put(new Character(((char)(byte)16)), new Character('p'));
        map.put(new Character(((char)(byte)17)), new Character('q'));
        map.put(new Character(((char)(byte)18)), new Character('r'));
        map.put(new Character(((char)(byte)19)), new Character('s'));
        map.put(new Character(((char)(byte)20)), new Character('t'));
        map.put(new Character(((char)(byte)21)), new Character('u'));
        map.put(new Character(((char)(byte)22)), new Character('v'));
        map.put(new Character(((char)(byte)24)), new Character('x'));
        map.put(new Character(((char)(byte)23)), new Character('w'));
        map.put(new Character(((char)(byte)25)), new Character('y'));
        map.put(new Character(((char)(byte)26)), new Character('z'));
        map.put(new Character(((char)(byte)27)), new Character('['));
        map.put(new Character(((char)(byte)28)), new Character(']'));
        map.put(new Character(((char)(byte)29)), new Character('�'));
        map.put(new Character(((char)(byte)31)), new Character('-'));

        //map for alt+key events
        map.put(new Character(((char)(byte)97)), new Character('a'));
        map.put(new Character(((char)(byte)98)), new Character('b'));
        map.put(new Character(((char)(byte)99)), new Character('c'));
        map.put(new Character(((char)(byte)100)), new Character('d'));
        map.put(new Character(((char)(byte)101)), new Character('e'));
        map.put(new Character(((char)(byte)102)), new Character('f'));
        map.put(new Character(((char)(byte)103)), new Character('g'));
        map.put(new Character(((char)(byte)104)), new Character('h'));
        map.put(new Character(((char)(byte)105)), new Character('i'));
        map.put(new Character(((char)(byte)106)), new Character('j'));
        map.put(new Character(((char)(byte)107)), new Character('k'));
        map.put(new Character(((char)(byte)108)), new Character('l'));
        map.put(new Character(((char)(byte)109)), new Character('m'));
        map.put(new Character(((char)(byte)110)), new Character('n'));
        map.put(new Character(((char)(byte)111)), new Character('o'));
        map.put(new Character(((char)(byte)112)), new Character('p'));
        map.put(new Character(((char)(byte)113)), new Character('q'));
        map.put(new Character(((char)(byte)114)), new Character('r'));
        map.put(new Character(((char)(byte)115)), new Character('s'));
        map.put(new Character(((char)(byte)116)), new Character('t'));
        map.put(new Character(((char)(byte)117)), new Character('u'));
        map.put(new Character(((char)(byte)118)), new Character('v'));
        map.put(new Character(((char)(byte)119)), new Character('w'));
        map.put(new Character(((char)(byte)120)), new Character('x'));
        map.put(new Character(((char)(byte)121)), new Character('y'));
        map.put(new Character(((char)(byte)122)), new Character('z'));
        map.put(new Character(((char)(byte)39)), new Character('\''));
        map.put(new Character(((char)(byte)49)), new Character('1'));
        map.put(new Character(((char)(byte)50)), new Character('2'));
        map.put(new Character(((char)(byte)51)), new Character('3'));
        map.put(new Character(((char)(byte)52)), new Character('4'));
        map.put(new Character(((char)(byte)53)), new Character('5'));
        map.put(new Character(((char)(byte)54)), new Character('6'));
        map.put(new Character(((char)(byte)55)), new Character('7'));
        map.put(new Character(((char)(byte)56)), new Character('8'));
        map.put(new Character(((char)(byte)57)), new Character('9'));
        map.put(new Character(((char)(byte)48)), new Character('0'));
        map.put(new Character(((char)(byte)45)), new Character('-'));
        map.put(new Character(((char)(byte)61)), new Character('='));
        map.put(new Character(((char)(byte)92)), new Character('\\'));
        map.put(new Character(((char)(byte)-76)), new Character('�'));
        map.put(new Character(((char)(byte)91)), new Character('['));
        map.put(new Character(((char)(byte)126)), new Character('~'));
        map.put(new Character(((char)(byte)93)), new Character(']'));
        map.put(new Character(((char)(byte)44)), new Character(','));
        map.put(new Character(((char)(byte)46)), new Character('.'));
        map.put(new Character(((char)(byte)59)), new Character(';'));
        map.put(new Character(((char)(byte)47)), new Character('/'));
        //numpad keys...
        map.put(new Character(((char)(byte)58)), new Character('1'));
        map.put(new Character(((char)(byte)59)), new Character('2'));
        map.put(new Character(((char)(byte)101)), new Character('3'));
        map.put(new Character(((char)(byte)102)), new Character('4'));
        map.put(new Character(((char)(byte)99)), new Character('5'));
        map.put(new Character(((char)(byte)96)), new Character('6'));
        map.put(new Character(((char)(byte)34)), new Character('7'));
        map.put(new Character(((char)(byte)-40)), new Character('8'));
        map.put(new Character(((char)(byte)-53)), new Character('9'));
        map.put(new Character(((char)(byte)47)), new Character('/'));
        map.put(new Character(((char)(byte)42)), new Character('*'));
        map.put(new Character(((char)(byte)43)), new Character('+'));
        map.put(new Character(((char)(byte)46)), new Character('.'));
        map.put(new Character(((char)(byte)44)), new Character(','));

        //shift+key events...
        map.put(new Character(((char)(byte)65)), new Character('a'));
        map.put(new Character(((char)(byte)66)), new Character('b'));
        map.put(new Character(((char)(byte)67)), new Character('c'));
        map.put(new Character(((char)(byte)68)), new Character('d'));
        map.put(new Character(((char)(byte)69)), new Character('e'));
        map.put(new Character(((char)(byte)70)), new Character('f'));
        map.put(new Character(((char)(byte)71)), new Character('g'));
        map.put(new Character(((char)(byte)72)), new Character('h'));
        map.put(new Character(((char)(byte)73)), new Character('i'));
        map.put(new Character(((char)(byte)74)), new Character('j'));
        map.put(new Character(((char)(byte)75)), new Character('k'));
        map.put(new Character(((char)(byte)76)), new Character('l'));
        map.put(new Character(((char)(byte)77)), new Character('m'));
        map.put(new Character(((char)(byte)78)), new Character('n'));
        map.put(new Character(((char)(byte)79)), new Character('o'));
        map.put(new Character(((char)(byte)80)), new Character('p'));
        map.put(new Character(((char)(byte)81)), new Character('q'));
        map.put(new Character(((char)(byte)82)), new Character('r'));
        map.put(new Character(((char)(byte)83)), new Character('s'));
        map.put(new Character(((char)(byte)84)), new Character('t'));
        map.put(new Character(((char)(byte)85)), new Character('u'));
        map.put(new Character(((char)(byte)86)), new Character('v'));
        map.put(new Character(((char)(byte)87)), new Character('w'));
        map.put(new Character(((char)(byte)88)), new Character('x'));
        map.put(new Character(((char)(byte)89)), new Character('y'));
        map.put(new Character(((char)(byte)90)), new Character('z'));
        map.put(new Character(((char)(byte)-57)), new Character('�'));
        map.put(new Character(((char)(byte)124)), new Character('|'));
        map.put(new Character(((char)(byte)33)), new Character('!'));
        map.put(new Character(((char)(byte)64)), new Character('@'));
        map.put(new Character(((char)(byte)35)), new Character('#'));
        map.put(new Character(((char)(byte)36)), new Character('$'));
        map.put(new Character(((char)(byte)37)), new Character('%'));
        map.put(new Character(((char)(byte)-88)), new Character('�'));
        map.put(new Character(((char)(byte)38)), new Character('&'));
        map.put(new Character(((char)(byte)40)), new Character('('));
        map.put(new Character(((char)(byte)41)), new Character(')'));
        map.put(new Character(((char)(byte)95)), new Character('_'));
        map.put(new Character(((char)(byte)96)), new Character('`'));
        map.put(new Character(((char)(byte)123)), new Character('{'));
        map.put(new Character(((char)(byte)94)), new Character('^'));
        map.put(new Character(((char)(byte)125)), new Character('}'));
        map.put(new Character(((char)(byte)58)), new Character(':'));
        map.put(new Character(((char)(byte)63)), new Character('?'));
        map.put(new Character(((char)(byte)60)), new Character('<'));
        map.put(new Character(((char)(byte)62)), new Character('>'));
        map.put(new Character(((char)(byte)47)), new Character('/'));
        map.put(new Character(((char)(byte)42)), new Character('*'));
        map.put(new Character(((char)(byte)45)), new Character('-'));
        map.put(new Character(((char)(byte)43)), new Character('+'));
        map.put(new Character(((char)(byte)46)), new Character('.'));
    }
    
    public static char getKeyValue(char key){
        return ((Character) map.get(new Character(key))).charValue();
    }
}
