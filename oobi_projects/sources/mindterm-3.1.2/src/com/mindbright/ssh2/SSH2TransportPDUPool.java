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

package com.mindbright.ssh2;

/**
 * Implements a pool of PDUs which can be reused. This class holds a
 * pool of PDUs and tries to reuse them whenever possible.
 */
public class SSH2TransportPDUPool extends SSH2TransportPDU {

    static int POOL_SIZE = 32;

    protected class PoolPDU extends SSH2TransportPDU {
        protected PoolPDU(int pktType, int bufSize) {
            super(pktType, bufSize);
        }
        public void release() {
            this.reset();
            this.pktSize = 0;
            releasePDU(this);
        }
    }

    int cnt;

    SSH2TransportPDU[] pool;

    protected SSH2TransportPDUPool() {
        pool = new SSH2TransportPDU[POOL_SIZE];
        cnt  = 0;
    }

    protected SSH2TransportPDU createPDU(int bufSize) {
        return createPDU(0, bufSize);
    }

    protected SSH2TransportPDU createPDU(int pktType, int bufSize) {
        synchronized(pool) {
            if (cnt == 0) {
                return new PoolPDU(pktType, bufSize);
            }
            int best = -1;
            for (int i = 0; i < cnt; i++) {
                if (pool[i].getMaxSize() >= bufSize
                    &&  (best < 0
                         || pool[i].getMaxSize() < pool[best].getMaxSize())) {
                    best = i;
                }
            }
            if (best >= 0) {
                SSH2TransportPDU b = pool[best];
                b.pktType = pktType;
                pool[best] = pool[cnt-1];
                cnt--;
                return b;
            } else {
                return new PoolPDU(pktType, bufSize);
            }
        }
    }

    /**
     * Internal class which releases an incoming PDU.
     */
    protected void releasePDU(PoolPDU pdu) {
        synchronized(pool) {
            if(cnt < pool.length) {
                pool[cnt++] = pdu;
            } else {
                /* stat */
            }
        }
    }
}
