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

package com.mindbright.jca.security;

import java.util.Enumeration;
import java.util.Vector;

public final class Security {

    private static Vector providers;

    static {
        providers = new Vector();
        providers.addElement(new com.mindbright.security.Mindbright());
    };

    public static int addProvider(Provider provider) {
        if(getProvider(provider.getName()) != null)
            return -1;
        providers.addElement(provider);
        return providers.size();
    }

    public static Provider getProvider(String name) {
        Enumeration e = providers.elements();
        while (e.hasMoreElements()) {
            Provider prov = (Provider)e.nextElement();
            if (prov.getName().equals(name))
                return prov;
        }
        return null;
    }

    public static String getAlgorithmProperty(String algName, String propName) {
        return null;
    }

    public static String getProperty(String key) {
        return null;
    }

    public static Provider[] getProviders() {
        Provider[]  list = new Provider[providers.size()];
        Enumeration e    = providers.elements();
        int         i    = 0;
        while(e.hasMoreElements()) {
            list[i++] = (Provider)e.nextElement();
        }
        return list;
    }

    /*
    public static Provider[] getProviders(Map filter) {
    }
    */

    public static Provider[] getProviders(String filter) {
        return null;
    }

    public static int insertProviderAt(Provider provider, int position) {
        return 0;
    }

    public static void removeProvider(String name) {}

    public static void setProperty(String key, String datum) {}

}
