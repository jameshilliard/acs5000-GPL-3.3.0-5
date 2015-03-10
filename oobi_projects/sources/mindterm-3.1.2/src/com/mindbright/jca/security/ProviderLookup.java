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

public final class ProviderLookup {

    Provider provider;
    Object   impl;

    private ProviderLookup(Object impl, Provider provider) {
        this.provider = provider;
        this.impl     = impl;
    }

    public Object getImpl() {
        return impl;
    }

    public Provider getProvider() {
        return provider;
    }

    private static String className(String type, String algorithmSpec,
                                    Provider provider) {
        String aliasFor =
            provider.getProperty("Alg.Alias." + type + "." + algorithmSpec);
        if(aliasFor != null) {
            algorithmSpec = aliasFor;
        }
        return provider.getProperty(type + "." + algorithmSpec);
    }

    public static ProviderLookup getImplementation(String type,
            String algorithmSpec,
            String providerName)
    throws NoSuchAlgorithmException, NoSuchProviderException {
        Provider provider = Security.getProvider(providerName);

        if(provider == null) {
            throw new NoSuchProviderException("Provider not found: " +
                                              providerName);
        }

        String className = className(type, algorithmSpec, provider);
        if(className == null) {
            throw new NoSuchAlgorithmException("Algorithm not found: " +
                                               algorithmSpec);
        }

        Object impl = null;
        try {
            impl = Class.forName(className).newInstance();
        } catch (ClassNotFoundException e) {
            throw new NoSuchAlgorithmException("Class " + className +
                                               " not found (" +
                                               provider + " - " +
                                               algorithmSpec + ")");
        } catch (InstantiationException e) {
            throw new NoSuchAlgorithmException("Class " + className +
                                               " can't be instantiated (" +
                                               provider + " - " +
                                               algorithmSpec + ") error: " +
                                               e.getMessage());
        } catch (IllegalAccessException e) {
            throw new NoSuchAlgorithmException("Class " + className +
                                               " can't be accessed (" +
                                               provider + " - " +
                                               algorithmSpec + ") error: " +
                                               e.getMessage());
        }

        return new ProviderLookup(impl, provider);
    }

    public static String findImplementingProvider(String type,
            String algorithmSpec)
    throws NoSuchAlgorithmException {
        Provider[] providers = Security.getProviders();
        for(int i = 0; i < providers.length; i++) {
            if(className(type, algorithmSpec, providers[i]) != null) {
                return providers[i].getName();
            }
        }
        throw new NoSuchAlgorithmException("No implementing provider found: '" +
                                           type + "' - '" + algorithmSpec + "'");
    }

}
