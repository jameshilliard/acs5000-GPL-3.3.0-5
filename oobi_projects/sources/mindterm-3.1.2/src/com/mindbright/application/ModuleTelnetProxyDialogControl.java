/******************************************************************************
 *
 * Copyright (c) 2005 AppGate Network Security AB. All Rights Reserved.
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

package com.mindbright.application;

import java.awt.event.ActionListener;

public interface ModuleTelnetProxyDialogControl {
    static String LBL_LISTEN_ADDR     = "Listen address";
    static String LBL_LISTEN_PORT     = "Listen port";
    static String LBL_PROXY_DISABLED = "Bridge disabled";
    static String LBL_ENABLE          = "Enable";
    static String LBL_DISABLE         = "Disable";
    static String LBL_DISMISS         = "Dismiss";

    static String DEFAULT_HOST        = "127.0.0.1";
    static String DEFAULT_PORT        = "23";

    public void initDialog(ActionListener al);
    public void showDialog();
    public void disposeDialog();

    public void setHost(String host);
    public void setPort(String port);
    public void setStatus(String status);

    public void setMode(boolean enable);

    public String getHost();
    public String getPort();    
}
