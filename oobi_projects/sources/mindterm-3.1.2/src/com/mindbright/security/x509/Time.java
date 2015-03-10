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

package com.mindbright.security.x509;

import com.mindbright.asn1.ASN1Choice;
import com.mindbright.asn1.ASN1GeneralizedTime;
import com.mindbright.asn1.ASN1UTCTime;

public class Time extends ASN1Choice {

    public Time() {
        super();
        setMember(new ASN1UTCTime());
        setMember(new ASN1GeneralizedTime());
    }

}
