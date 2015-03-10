/******************************************************************************
 *
 * Copyright (c) 1999-2006 AppGate Network Security AB. All Rights Reserved.
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

import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.io.OutputStream;

import com.mindbright.util.Queue;

/**
 * Class implementing streams-based channels. That is channels which
 * locally are connected to a pair of Input/Output streams. It is also
 * possible to apply filters to the channels.
 */
public class SSH2StreamChannel extends SSH2Channel {
    protected InputStream  in;
    protected OutputStream out;

    protected Thread transmitter;
    protected Thread receiver;
    protected Queue  rxQueue;
    protected long   txCounter;
    protected long   rxCounter;

    /*
     * NOTE, if enabled can cause dead-lock when doing re-keyexchange
     * if we initiate it, hence SSH2Transport.incompatibleCantReKey is
     * set accordingly
     */
    private boolean rxChanIsQueued;

    /**
     * Create a new stream channel of the given type. The channel is
     * associated with an ssh connection. Channel types are
     * defined in <code>SSH2Connection</code> and starts with
     * <code>CH_TYPE</code>.
     *
     * @param channelType Type of channel to create.
     * @param connection The ssh connection to associate the channel with.
     * @param creator The object the channel is created from.
     * @param in The input stream from which data to be sent over the
     * channel is read.
     * @param out The output stream onto which data received from the
     * channel is written.
     */
    protected SSH2StreamChannel(int channelType, SSH2Connection connection,
                                Object creator,
                                InputStream in, OutputStream out) {
        super(channelType, connection, creator);

        rxChanIsQueued =
            "true".equals(connection.getPreferences().
                          getPreference(SSH2Preferences.QUEUED_RX_CHAN));

        this.in  = in;
        this.out = out;
        createStreams();
    }

    /**
     * Apply the given filter to this channel.
     *
     * @param filter Filter to apply.
     */
    public void applyFilter(SSH2StreamFilter filter) {
        if(filter != null) {
            in  = filter.getInputFilter(in);
            out = filter.getOutputFilter(out);
        }
    }

    private void channelTransmitLoop() {
        connection.getLog().debug("SSH2StreamChannel",
                                  "starting ch. #" + channelId +
                                  " (" + getType() + ") transmitter");
        Thread.yield();
        try {
            SSH2TransportPDU pdu;
            int              maxSz = 0;
            int              rcvSz = 0;
            boolean          interrupted = false;
            while(!eofSent && !closeSent) {
                pdu =
                    SSH2TransportPDU.createOutgoingPacket(SSH2.MSG_CHANNEL_DATA,
                                                          txMaxPktSz + 256);
                pdu.writeInt(peerChanId);
                maxSz = checkTxWindowSize(rcvSz);
                do {
                    try {
                        rcvSz = in.read(pdu.data, pdu.wPos + 4, maxSz);
                        interrupted = false;
                    } catch (InterruptedIOException e) {
                        interrupted = true;
                    }
                } while (interrupted);
                if(rcvSz == -1) {
                    sendEOF();
                } else if (!eofSent && !closeSent) {
                    pdu.writeInt(rcvSz);
                    pdu.wPos  += rcvSz;
                    txCounter += rcvSz;
                    transmit(pdu);
                }
            }
        } catch (IOException e) {
            if(!eofSent) {
                connection.getLog().error("SSH2StreamChannel",
                                          "channelTransmitLoop",
                                          e.toString());
            }
        } finally {
            try {
                if (in != null)
                    in.close();
            } catch (IOException e) { /* don't care */
            }
            sendClose();
        }
        connection.getLog().debug("SSH2StreamChannel",
                                  "exiting ch. #" +
                                  channelId + " (" + getType() +
                                  ") transmitter, " + txCounter +
                                  " bytes tx");
    }

    private void channelReceiveLoop() {
        connection.getLog().debug("SSH2StreamChannel",
                                  "starting ch. #" + channelId +
                                  " (" + getType() + ") receiver");
        Thread.yield();
        try {
            SSH2TransportPDU pdu;
            while((pdu = (SSH2TransportPDU)rxQueue.getFirst()) != null) {
                rxWrite(pdu);
            }
        } catch (IOException e) {
            connection.getLog().error("SSH2StreamChannel",
                                      "channelReceiveLoop",
                                      e.toString());
        } finally {
            rxClosing();
        }
        connection.getLog().debug("SSH2StreamChannel",
                                  "exiting ch. #" +
                                  channelId + " (" + getType() +
                                  ") receiver, " + rxCounter +
                                  " bytes rx");
    }

    private final void rxWrite(SSH2TransportPDU pdu) throws IOException {
        int len = pdu.readInt();
        int off = pdu.getRPos();
        rxCounter += len;
        out.write(pdu.data, off, len);
        pdu.release();
        checkRxWindowSize(len);
    }

    private final void rxClosing() {
        // Signal to transmitter that this is an orderly shutdown
        //
        eofSent = true;
        try {
            if (out != null)
                out.close();
        } catch (IOException e) { /* don't care */
        }
        try {
            if (in != null)
                in.close();
        } catch (IOException e) { /* don't care */
        }
        outputClosed();

        // there is a slight chance that the transmitter is waiting for
        // window adjust in which case we must interrupt it here so it
        // doesn't hang
        //
        if(txCurrWinSz == 0) {
            txCurrWinSz = -1;
            transmitter.interrupt();
        }
    }

    private final synchronized int checkTxWindowSize(int lastSz) {
        txCurrWinSz -= lastSz;
        while(txCurrWinSz == 0) {
            // Our window is full, wait for ACK from peer
            try {
                this.wait();
            } catch (InterruptedException e) {
                if(!eofSent) {
                    connection.getLog().error("SSH2StreamChannel",
                                              "checkTxWindowSize",
                                              "window adjust wait interrupted");
                }
            }
        }
        // Try sending remaining window size or max packet size before ACK
        //
        int dataSz = (txCurrWinSz < txMaxPktSz ? txCurrWinSz : txMaxPktSz);
        return dataSz;
    }

    protected final void checkRxWindowSize(int len) {
        rxCurrWinSz -= len;
        if(rxCurrWinSz < 0) {
            connection.fatalDisconnect(SSH2.DISCONNECT_PROTOCOL_ERROR,
                                       "Peer overflowed window");
        } else if(rxCurrWinSz <= (rxInitWinSz >>> 1)) {
            // ACK on >= 50% of window received
            SSH2TransportPDU pdu =
                SSH2TransportPDU.createOutgoingPacket(SSH2.MSG_CHANNEL_WINDOW_ADJUST);
            pdu.writeInt(peerChanId);
            pdu.writeInt(rxInitWinSz - rxCurrWinSz);
            transmit(pdu);
            rxCurrWinSz = rxInitWinSz;
        }
    }

    protected void data(SSH2TransportPDU pdu) {
        if(rxChanIsQueued) {
            rxQueue.putLast(pdu);
        } else {
            try {
                rxWrite(pdu);
            } catch (IOException e) {
                connection.getLog().error("SSH2StreamChannel",
                                          "data",
                                          e.toString());
                rxClosing();
            }
        }
    }

    protected void openConfirmationImpl(SSH2TransportPDU pdu) {
        startStreams();
    }

    protected boolean openFailureImpl(int reasonCode, String reasonText,
                                      String langTag) {
        // Just return false since we don't want to keep the channel,
        // handle in derived class if needed
        return false;
    }

    protected synchronized void windowAdjustImpl(int inc) {
        txCurrWinSz += inc;
        this.notify();
    }

    protected void eofImpl() {
        if(rxChanIsQueued) {
            rxQueue.setBlocking(false);
        } else {
            rxClosing();
        }
    }

    protected void closeImpl() {
        eofImpl();
    }

    /**
     * Called when no more data can be written to the channel.
     */
    protected void outputClosed() {
        // Do nothing, handle in derived class if needed
    }

    protected void handleRequestImpl(String type, boolean wantReply,
                                     SSH2TransportPDU pdu) {
        // Do nothing, handle in derived class if needed
    }

    /**
     * Create the transmitter and receiver threads.
     */
    protected void createStreams() {
        if(rxChanIsQueued) {
            receiver = new Thread(new Runnable() {
                                      public void run() {
                                          channelReceiveLoop();
                                      }
                                  }
                                  , "SSH2StreamRX_" + getType() + "_" + channelId);
            receiver.setDaemon(false);
            rxQueue =
                new Queue(connection.getPreferences().
                          getIntPreference(SSH2Preferences.QUEUE_DEPTH),
                          connection.getPreferences().
                          getIntPreference(SSH2Preferences.QUEUE_HIWATER));
        }
        transmitter = new Thread(new Runnable() {
                                     public void run() {
                                         channelTransmitLoop();
                                     }
                                 }
                                 , "SSH2StreamTX_" + getType() + "_" + channelId);
        transmitter.setDaemon(false);
    }

    /**
     * Starts the transmitter and receiver threads.
     */
    protected void startStreams() {
        transmitter.start();
        if(rxChanIsQueued) {
            receiver.start();
        }
    }

    public void waitUntilClosed() {
        super.waitUntilClosed();
        if(rxChanIsQueued) {
            try {
                receiver.join();
            } catch (InterruptedException e) { }
        }
    }

}


