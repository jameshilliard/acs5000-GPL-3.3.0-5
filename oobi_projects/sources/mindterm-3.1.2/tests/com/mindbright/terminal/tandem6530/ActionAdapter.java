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

import com.mindbright.terminal.tandem6530.ActionHandler;
import com.mindbright.terminal.tandem6530.FieldAttributes;
import com.mindbright.terminal.tandem6530.IBM3270FieldAttributes;
import com.mindbright.terminal.tandem6530.ConfigParameter;

/** Interface with callback methods when Term6530Parser has parsed
 *  a command or a character.
 */
public class ActionAdapter implements ActionHandler {

    static int ACTION_NONE = 0;
    static int ACTION_doBell = 1;
    static int ACTION_doBackspace = 2;
    static int ACTION_doHTab = 3;
    static int ACTION_doLineFeed = 4;
    static int ACTION_doCarriageReturn = 5;
    static int ACTION_doSetConversationalMode = 6;
    static int ACTION_doSetBlockMode = 7;
    static int ACTION_doSetBufferAddress = 8;
    static int ACTION_doDataCompression = 9;
    static int ACTION_doSetCursorAddress = 10;
    static int ACTION_doDefineFieldAttribute = 11;
    static int ACTION_doStartField = 12;
    static int ACTION_doPrintScreenOrPage = 13;
    static int ACTION_doSetTab = 14;
    static int ACTION_doClearTab = 15;
    static int ACTION_doClearAllTabs = 16;
    static int ACTION_doSetVideoAttributes = 17;
    static int ACTION_doSetVideoPriorConditionRegister = 18;
    static int ACTION_doSet40CharLineWidth = 19;
    static int ACTION_doSet80CharLineWidth = 20;
    static int ACTION_doReadCursorAddress = 21;
    static int ACTION_doUnlockKeyboard = 22;
    static int ACTION_doLockKeyboard = 23;
    static int ACTION_doSetStringConfigurationParameter = 24;
    static int ACTION_doReadStringConfigurationParameter = 25;
    static int ACTION_doSimulateFunctionKey = 26;
    static int ACTION_doGetMachineName = 27;
    static int ACTION_doDisconnectModem = 28;
    static int ACTION_doGetCurrentDirectoryAndRedirectionInformation = 29;
    static int ACTION_doReadVTLAUNCHConfigurationParameter = 30;
    static int ACTION_doBackTab = 31;
    static int ACTION_doRTMControl = 32;
    static int ACTION_doRTMDataUpload = 33;
    static int ACTION_doSetEM3270Mode = 34;
    static int ACTION_doReadAllLocations = 35;
    static int ACTION_doReadKeyboardLatch = 36;
    static int ACTION_doWriteToMessageField = 37;
    static int ACTION_doSetMaxPageNumber = 38;
    static int ACTION_doReinitialize = 39;
    static int ACTION_doSetColorMapTable = 40;
    static int ACTION_doResetColorMapTable = 41;
    static int ACTION_doDefineDataTypeTable = 42;
    static int ACTION_doResetVariableTable = 43;
    static int ACTION_doDefineVariableTable = 44;
    static int ACTION_doSet40CharactersScreenWidth = 45;
    static int ACTION_doSetColorConfiguration = 46;
    static int ACTION_doResetColorConfiguration = 47;
    static int ACTION_doReadColorConfiguration = 48;
    static int ACTION_doDefineEnterKeyFunction = 49;
    static int ACTION_doSetTerminalConfiguration = 50;
    static int ACTION_doRead6530ColorMappingTable = 51;
    static int ACTION_doReadColorMappingTable = 52;
    static int ACTION_doSetIODeviceConfiguration = 53;
    static int ACTION_doSet6530ColorMapping = 54;
    static int ACTION_doReadIODeviceConfiguration = 55;
    static int ACTION_doTerminateRemote6530Operation = 56;
    static int ACTION_doCursorUp = 57;
    static int ACTION_doCursorRight = 58;
    static int ACTION_doCursorHomeDown = 59;
    static int ACTION_doCursorHome = 60;
    static int ACTION_doRollUp = 61;
    static int ACTION_doRollDown = 62;
    static int ACTION_doPageUp = 63;
    static int ACTION_doPageDown = 64;
    static int ACTION_doClearMemoryToSpaces = 65;
    static int ACTION_doClearMemoryToSpacesWithAddr = 66;
    static int ACTION_doEraseToEndOfPageOrMemory = 67;
    static int ACTION_doReadWithAddress = 68;
    static int ACTION_doEraseToEndOfLineOrField = 69;
    static int ACTION_doReadWithAddressAll = 70;
    static int ACTION_doInsertLine = 71;
    static int ACTION_doDeleteLine = 72;
    static int ACTION_doDisableLocalLineEditing = 73;
    static int ACTION_doInsertCharacter = 74;
    static int ACTION_doWriteToAux1OrAux2Device = 75;
    static int ACTION_doDeleteCharacter = 76;
    static int ACTION_doReadScreenWithAllAttributes = 77;
    static int ACTION_doLoadAndExecuteAnOperatingSystemProgram = 78;
    static int ACTION_doEnterProtectedSubmode = 79;
    static int ACTION_doReportExecReturnCode = 80;
    static int ACTION_doExitProtectedSubmode = 81;
    static int ACTION_doReadTerminalConfiguration = 82;
    static int ACTION_doReadTerminalStatus = 83;
    static int ACTION_doReadFullRevisionLevel = 84;
    static int ACTION_doDelayOneSecond = 85;
    static int ACTION_doResetMoifiedDataTags = 86;
    static int ACTION_doReadWholePageOrBuffer = 87;
    static int ACTION_doDisplayPage = 88;
    static int ACTION_doSelectPage = 89;
    static int ACTION_doStartEnhancedColorField = 90;
    static int ACTION_doStartFieldExtended = 91;
    static int ACTION_doWriteToFileOrDeviceName = 92;
    static int ACTION_doWriteOrReadToFileOrDeviceName = 93;
    static int ACTION_LAST = 94;

    String actions[] = {
                           "NONE",
                           "doBell",
                           "doBackspace",
                           "doHTab",
                           "doLineFeed",
                           "doCarriageReturn",
                           "doSetConversationalMode",
                           "doSetBlockMode",
                           "doSetBufferAddress",
                           "doDataCompression",
                           "doSetCursorAddress",
                           "doDefineFieldAttribute",
                           "doStartField",
                           "doPrintScreenOrPage",
                           "doSetTab",
                           "doClearTab",
                           "doClearAllTabs",
                           "doSetVideoAttributes",
                           "doSetVideoPriorConditionRegister",
                           "doSet40CharLineWidth",
                           "doSet80CharLineWidth",
                           "doReadCursorAddress",
                           "doUnlockKeyboard",
                           "doLockKeyboard",
                           "doSetStringConfigurationParameter",
                           "doReadStringConfigurationParameter",
                           "doSimulateFunctionKey",
                           "doGetMachineName",
                           "doDisconnectModem",
                           "doGetCurrentDirectoryAndRedirectionInformation",
                           "doReadVTLAUNCHConfigurationParameter",
                           "doBackTab",
                           "doRTMControl",
                           "doRTMDataUpload",
                           "doSetEM3270Mode",
                           "doReadAllLocations",
                           "doReadKeyboardLatch",
                           "doWriteToMessageField",
                           "doSetMaxPageNumber",
                           "doReinitialize",
                           "doSetColorMapTable",
                           "doResetColorMapTable",
                           "doDefineDataTypeTable",
                           "doResetVariableTable",
                           "doDefineVariableTable",
                           "doSet40CharactersScreenWidth",
                           "doSetColorConfiguration",
                           "doResetColorConfiguration",
                           "doReadColorConfiguration",
                           "doDefineEnterKeyFunction",
                           "doSetTerminalConfiguration",
                           "doRead6530ColorMappingTable",
                           "doReadColorMappingTable",
                           "doSetIODeviceConfiguration",
                           "doSet6530ColorMapping",
                           "doReadIODeviceConfiguration",
                           "doTerminateRemote6530Operation",
                           "doCursorUp",
                           "doCursorRight",
                           "doCursorHomeDown",
                           "doCursorHome",
                           "doRollUp",
                           "doRollDown",
                           "doPageUp",
                           "doPageDown",
                           "doClearMemoryToSpaces",
                           "doClearMemoryToSpacesWithAddr",
                           "doEraseToEndOfPageOrMemory",
                           "doReadWithAddress",
                           "doEraseToEndOfLineOrField",
                           "doReadWithAddressAll",
                           "doInsertLine",
                           "doDeleteLine",
                           "doDisableLocalLineEditing",
                           "doInsertCharacter",
                           "doWriteToAux1OrAux2Device",
                           "doDeleteCharacter",
                           "doReadScreenWithAllAttributes",
                           "doLoadAndExecuteAnOperatingSystemProgram",
                           "doEnterProtectedSubmode",
                           "doReportExecReturnCode",
                           "doExitProtectedSubmode",
                           "doReadTerminalConfiguration",
                           "doReadTerminalStatus",
                           "doReadFullRevisionLevel",
                           "doDelayOneSecond",
                           "doResetMoifiedDataTags",
                           "doReadWholePageOrBuffer",
                           "doDisplayPage",
                           "doSelectPage",
                           "doStartEnhancedColorField",
                           "doStartFieldExtended",
                           "doWriteToFileOrDeviceName",
                           "doWriteOrReadToFileOrDeviceName" };

    int lastAction = ACTION_NONE;

    public void resetAction() {
        lastAction = ACTION_NONE;
    }
    public String getActionName(int action) {
        return actions[action];
    }
    public String getLastActionName() {
        return actions[lastAction];
    }
    public int getLastAction() {
        return lastAction;
    }
    public boolean isCalled() {
        return lastAction != ACTION_NONE;
    }


    /* 2-48, 3-84 */
    public void doBell() {
        lastAction = ACTION_doBell;
    }
    /* 2-8, 3-18 */
    public void doBackspace() {
        lastAction = ACTION_doBackspace;
    }
    /* 2-8, 3-18 */
    public void doHTab() {
        lastAction = ACTION_doHTab;
    }
    /* 2-8 */
    public void doLineFeed() {
        lastAction = ACTION_doLineFeed;
    }
    /* 2-8, 3-18 */
    public void doCarriageReturn() {
        lastAction = ACTION_doCarriageReturn;
    }
    /* 1-12 */
    public void doSetConversationalMode() {
        lastAction = ACTION_doSetConversationalMode;
    }
    /* 1-12 */
    public void doSetBlockMode() {
        lastAction = ACTION_doSetBlockMode;
    }
    /* 3-15, 3-16 */
    public void doSetBufferAddress(int row, int column) {
        lastAction = ACTION_doSetBufferAddress;
    }
    /* 3-72, 3-73 */
    public void doDataCompression(int n, char c) {
        lastAction = ACTION_doDataCompression;
    }
    /* 2-6, 3-14 */
    public void doSetCursorAddress(int row, int column) {
        lastAction = ACTION_doSetCursorAddress;
    }
    /* 3-84 */
    public void doDefineFieldAttribute(int row, int column, boolean useFixed,
                                       int tableRow) {
        lastAction = ACTION_doDefineFieldAttribute;
    }
    /* 3-37 */
    public void doStartField(FieldAttributes attribs) {
        lastAction = ACTION_doStartField;
    }

    /* 2-41, 3-77 */
    public void doPrintScreenOrPage() {
        lastAction = ACTION_doPrintScreenOrPage;
    }
    /* 2-9, 3-19 */
    public void doSetTab() {
        lastAction = ACTION_doSetTab;
    }
    /* 2-9, 3-19 */
    public void doClearTab() {
        lastAction = ACTION_doClearTab;
    }
    /* 2-9, 3-19 */
    public void doClearAllTabs() {
        lastAction = ACTION_doClearAllTabs;
    }
    /* 2-14, 3-23 */
    public void doSetVideoAttributes(char videoAttrib) {
        lastAction = ACTION_doSetVideoAttributes;
    }
    /* 2-15, 3-24 */
    public void doSetVideoPriorConditionRegister(char videoAttrib) {
        lastAction = ACTION_doSetVideoPriorConditionRegister;
    }
    /* 3-40 */
    public void doSet40CharLineWidth() {
        lastAction = ACTION_doSet40CharLineWidth;
    }
    /* 3-40 */
    public void doSet80CharLineWidth() {
        lastAction = ACTION_doSet80CharLineWidth;
    }
    /* 2-6, 3-15 */
    public void doReadCursorAddress() {
        lastAction = ACTION_doReadCursorAddress;
    }
    /* 2-49, 3-92 */
    public void doUnlockKeyboard() {
        lastAction = ACTION_doUnlockKeyboard;
    }
    /* 2-49, 3-92 */
    public void doLockKeyboard() {
        lastAction = ACTION_doLockKeyboard;
    }
    /* 2-33, 3-62 */
    public void doSetStringConfigurationParameter(String strs[]) {
        lastAction = ACTION_doSetStringConfigurationParameter;
    }
    /* 2-32, 3-61 */
    public void doReadStringConfigurationParameter(int n) {
        lastAction = ACTION_doReadStringConfigurationParameter;
    }
    /* 2-49, 3-92 */
    public void doSimulateFunctionKey(char keyCode) {
        lastAction = ACTION_doSimulateFunctionKey;
    }
    /* 2-40, 3-76 */
    public void doGetMachineName() {
        lastAction = ACTION_doGetMachineName;
    }
    /* 2-48, 3-84 */
    public void doDisconnectModem() {
        lastAction = ACTION_doDisconnectModem;
    }
    /* 2-40, 3-76 */
    public void doGetCurrentDirectoryAndRedirectionInformation(char drive) {
        lastAction = ACTION_doGetCurrentDirectoryAndRedirectionInformation;
    }
    /* 2-33, 3-63 */
    public void doReadVTLAUNCHConfigurationParameter(int param) {
        lastAction = ACTION_doReadVTLAUNCHConfigurationParameter;
    }
    /* 3-19 */
    public void doBackTab() {
        lastAction = ACTION_doBackTab;
    }
    /* 2-36, 3-65 */
    public void doRTMControl(int startStopEvent, int buckets[]) {
        lastAction = ACTION_doRTMControl;
    }
    /* 2-37, 3-66 */
    public void doRTMDataUpload(int id[]) {
        lastAction = ACTION_doRTMDataUpload;
    }
    /* 3-67 */
    public void doSetEM3270Mode(int mode) {
        lastAction = ACTION_doSetEM3270Mode;
    }
    /* 3-70 */
    public void doReadAllLocations() {
        lastAction = ACTION_doReadAllLocations;
    }
    /* 3-71 */
    public void doReadKeyboardLatch() {
        lastAction = ACTION_doReadKeyboardLatch;
    }
    /* 2-49, 3-93 */
    public void doWriteToMessageField(char msg[], char attribs[]) {
        lastAction = ACTION_doWriteToMessageField;
    }
    /* 3-12 */
    public void doSetMaxPageNumber(int n) {
        lastAction = ACTION_doSetMaxPageNumber;
    }
    /* 2-50, 3-93 */
    public void doReinitialize() {
        lastAction = ACTION_doReinitialize;
    }
    /* 2-19, 3-28 */
    public void doSetColorMapTable(int startIndex, byte entries[]) {
        lastAction = ACTION_doSetColorMapTable;
    }
    /* 2-19, 3-28 */
    public void doResetColorMapTable() {
        lastAction = ACTION_doResetColorMapTable;
    }
    /* 3-39, 3-40 */
    public void doDefineDataTypeTable(int startIndex, byte entries[]) {
        lastAction = ACTION_doDefineDataTypeTable;
    }
    /* 3-90 */
    public void doResetVariableTable() {
        lastAction = ACTION_doResetVariableTable;
    }
    /* 3-90 */
    public void doDefineVariableTable(int startIndex,
                                      FieldAttributes attribs[]) {
        lastAction = ACTION_doDefineVariableTable;
    }
    /* 3-40 */
    public void doSet40CharactersScreenWidth() {
        lastAction = ACTION_doSet40CharactersScreenWidth;
    }
    /* 2-23, 3-32 */
    public void doSetColorConfiguration(int startIndex, byte entries[]) {
        lastAction = ACTION_doSetColorConfiguration;
    }
    /* 2-23, 3-32 */
    public void doResetColorConfiguration() {
        lastAction = ACTION_doResetColorConfiguration;
    }
    /* 2-22, 3-31 */
    public void doReadColorConfiguration() {
        lastAction = ACTION_doReadColorConfiguration;
    }
    /* 2-50 */
    public void doDefineEnterKeyFunction(char str[]) {
        lastAction = ACTION_doDefineEnterKeyFunction;
    }
    /* 2-30, 3-59 */
    public void doSetTerminalConfiguration(ConfigParameter params[]) {
        lastAction = ACTION_doSetTerminalConfiguration;
    }
    /* 2-18, 3-27 */
    public void doRead6530ColorMappingTable() {
        lastAction = ACTION_doRead6530ColorMappingTable;
    }
    /* 2-21, 3-30 */
    public void doReadColorMappingTable() {
        lastAction = ACTION_doReadColorMappingTable;
    }
    /* 2-31, 3-60 */
    public void doSetIODeviceConfiguration(int device,
                                           ConfigParameter params[]) {
        lastAction = ACTION_doSetIODeviceConfiguration;
    }
    /* 3-32 */
    public void doSet6530ColorMapping(boolean setEnhanced) {
        lastAction = ACTION_doSet6530ColorMapping;
    }
    /* 2-31, 3-60 */
    public void doReadIODeviceConfiguration(int device) {
        lastAction = ACTION_doReadIODeviceConfiguration;
    }
    /* 2-50, 3-93 */
    public void doTerminateRemote6530Operation(int exitCode) {
        lastAction = ACTION_doTerminateRemote6530Operation;
    }
    /* 2-7, 3-17 */
    public void doCursorUp() {
        lastAction = ACTION_doCursorUp;
    }
    /* 2-8, 3-17 */
    public void doCursorRight() {
        lastAction = ACTION_doCursorRight;
    }
    /* 2-8, 3-18 */
    public void doCursorHomeDown() {
        lastAction = ACTION_doCursorHomeDown;
    }
    /* 2-8, 3-18 */
    public void doCursorHome() {
        lastAction = ACTION_doCursorHome;
    }
    /* 2-9 */
    public void doRollUp() {
        lastAction = ACTION_doRollUp;
    }
    /* 2-9 */
    public void doRollDown() {
        lastAction = ACTION_doRollDown;
    }
    /* 2-10 */
    public void doPageUp() {
        lastAction = ACTION_doPageUp;
    }
    /* 2-10 */
    public void doPageDown() {
        lastAction = ACTION_doPageDown;
    }
    /* 2-10 */
    public void doClearMemoryToSpaces() {
        lastAction = ACTION_doClearMemoryToSpaces;
    }
    /* 3-48, 3-49 */
    public void doClearMemoryToSpaces(int startRow, int startCol,
                                      int endRow, int endColumn) {
        lastAction = ACTION_doClearMemoryToSpacesWithAddr;
    }
    /* 2-10, 3-49 */
    public void doEraseToEndOfPageOrMemory() {
        lastAction = ACTION_doEraseToEndOfPageOrMemory;
    }
    /* 3-45, 3-46 */
    public void doReadWithAddress(int startRow, int startCol,
                                  int endRow, int endColumn) {
        lastAction = ACTION_doReadWithAddress;
    }
    /* 2-11, 3-49 */
    public void doEraseToEndOfLineOrField() {
        lastAction = ACTION_doEraseToEndOfLineOrField;
    }
    /* 3-46, 3-47 */
    public void doReadWithAddressAll(int startRow, int startCol,
                                     int endRow, int endColumn) {
        lastAction = ACTION_doReadWithAddressAll;
    }
    /* 3-50 */
    public void doInsertLine() {
        lastAction = ACTION_doInsertLine;
    }
    /* 3-50 */
    public void doDeleteLine() {
        lastAction = ACTION_doDeleteLine;
    }
    /* 3-92 */
    public void doDisableLocalLineEditing() {
        lastAction = ACTION_doDisableLocalLineEditing;
    }
    /* 3-51 */
    public void doInsertCharacter() {
        lastAction = ACTION_doInsertCharacter;
    }
    /* 2-42, 3-78 */
    public void doWriteToAux1OrAux2Device(int device, char terminator) {
        lastAction = ACTION_doWriteToAux1OrAux2Device;
    }
    /* 3-51 */
    public void doDeleteCharacter() {
        lastAction = ACTION_doDeleteCharacter;
    }
    /* 3-34 */
    public void doReadScreenWithAllAttributes(int startRow, int startCol,
            int endRow, int endColumn) {
        lastAction = ACTION_doReadScreenWithAllAttributes;
    }
    /* 2-46, 3-82 */
    public void doLoadAndExecuteAnOperatingSystemProgram(String execString) {
        lastAction = ACTION_doLoadAndExecuteAnOperatingSystemProgram;
    }
    /* 3-35 */
    public void doEnterProtectedSubmode() {
        lastAction = ACTION_doEnterProtectedSubmode;
    }
    /* 2-47, 3-83 */
    public void doReportExecReturnCode() {
        lastAction = ACTION_doReportExecReturnCode;
    }
    /* 3-36 */
    public void doExitProtectedSubmode() {
        lastAction = ACTION_doExitProtectedSubmode;
    }
    /* 2-30, 3-57 */
    public void doReadTerminalConfiguration() {
        lastAction = ACTION_doReadTerminalConfiguration;
    }
    /* 2-38, 3-74 */
    public void doReadTerminalStatus() {
        lastAction = ACTION_doReadTerminalStatus;
    }
    /* 2-39, 3-75 */
    public void doReadFullRevisionLevel() {
        lastAction = ACTION_doReadFullRevisionLevel;
    }
    /* 2-48, 3-91 */
    public void doDelayOneSecond() {
        lastAction = ACTION_doDelayOneSecond;
    }
    /* 3-38 */
    public void doResetMoifiedDataTags() {
        lastAction = ACTION_doResetMoifiedDataTags;
    }
    /* 3-44 */
    public void doReadWholePageOrBuffer() {
        lastAction = ACTION_doReadWholePageOrBuffer;
    }
    /* 2-10, 3-11 */
    public void doDisplayPage(int n) {
        lastAction = ACTION_doDisplayPage;
    }
    /* 3-12 */
    public void doSelectPage(int n) {
        lastAction = ACTION_doSelectPage;
    }
    /* 3-33 */
    public void doStartEnhancedColorField(IBM3270FieldAttributes attribs) {
        lastAction = ACTION_doStartEnhancedColorField;
    }
    /* 3-38 */
    public void doStartFieldExtended(FieldAttributes attribs) {
        lastAction = ACTION_doStartFieldExtended;
    }
    /* 2-43, 3-79 */
    public void doWriteToFileOrDeviceName(String device, int opCode,
                                          byte data[]) {
        lastAction = ACTION_doWriteToFileOrDeviceName;
    }
    /* 2-44, 3-80 */
    public void doWriteOrReadToFileOrDeviceName(String device, int opCode,
            byte data[]) {
        lastAction = ACTION_doWriteOrReadToFileOrDeviceName;
    }
}

