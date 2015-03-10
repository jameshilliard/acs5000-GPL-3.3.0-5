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
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

import com.mindbright.util.Queue;

/**
 * Implements the client side of the sftp protocol. File reads and writes
 * can be either synchronous (blocking) or asynchronous
 * (non-blocking). Asynchronous operation is much faster but is more
 * complicated to use when the interface is used for reading
 * parts of files. In asynchronous mode <code>read</code> and
 * <code>write</code> return immediately before the operation has
 * finished. The <code>asyncWait</code> method waits until all
 * operations have been completed and must be called before read
 * buffers are accessed.
 */
public final class SSH2SFTPClient extends SSH2SFTP {

    private class ReplyLock {
        protected int        expectType;
        protected SFTPPacket replyPkt;

        protected ReplyLock(int expectType) {
            this.expectType = expectType;
            this.replyPkt   = null;
        }

        protected synchronized SFTPPacket expect()
        throws SFTPException {
            while(replyPkt == null) {
                try {
                    this.wait();
                } catch (InterruptedException e) {}
            }
            checkType(replyPkt, expectType);
            return replyPkt;
        }

        protected synchronized void received(SFTPPacket replyPkt) {
            this.replyPkt = replyPkt;
            this.notify();
        }

        protected synchronized void cancel() {
            this.replyPkt = createPacket(SSH_FXP_STATUS);
            this.replyPkt.writeInt(SSH_FX_CONNECTION_LOST);
            this.notify();
        }

    }

    private class WriteReplyLock extends ReplyLock {

        private FileHandle handle;
        private int        len;

        protected WriteReplyLock(FileHandle handle, int len) {
            super(SSH_FXP_STATUS);
            this.handle = handle;
            this.len    = len;
            handle.asyncStart(len);
        }

        protected synchronized void received(SFTPPacket replyPkt) {
            try {
                if(!handle.isOpen()) {
                    /* Ignore and discard packets after close */
                    return;
                }
                checkType(replyPkt, expectType);
                handle.asyncEnd(len);
            } catch (SFTPException e) {
                handle.asyncException(e);
            }
            releasePacket(replyPkt);
        }

        protected synchronized void cancel() {
            handle.asyncException(new SFTPDisconnectException());
            this.notify();
        }

    }

    private class ReadReplyLock extends ReplyLock {

        private FileHandle handle;
        private long       fileOffset;
        private byte[]     buf;
        private int        off;
        private int        len;

        private RandomAccessFile fileTarget;
        private OutputStream     strmTarget;

        private ReadReplyLock(FileHandle handle, long fileOffset, int len) {
            super(SSH_FXP_DATA);
            this.handle     = handle;
            this.fileOffset = fileOffset;
            this.len        = len;
            handle.asyncStart(len);
        }

        protected ReadReplyLock(FileHandle handle, long fileOffset,
                                OutputStream strmTarget, int len) {
            this(handle, fileOffset, len);
            this.strmTarget = strmTarget;
        }

        protected ReadReplyLock(FileHandle handle, long fileOffset,
                                RandomAccessFile fileTarget, int len) {
            this(handle, fileOffset, len);
            this.fileTarget = fileTarget;
        }

        protected ReadReplyLock(FileHandle handle, long fileOffset,
                                byte[] buf, int off, int len) {
            this(handle, fileOffset, len);
            this.buf        = buf;
            this.off        = off;
        }

        protected synchronized void received(SFTPPacket replyPkt) {
            try {
                int n;
                if(!handle.isOpen()) {
                    /* Ignore and discard packets after close */
                    return;
                }
                checkType(replyPkt, expectType);
                if(fileTarget != null) {
                    n = replyPkt.readInt();
                    fileTarget.seek(fileOffset);
                    fileTarget.write(replyPkt.getData(), replyPkt.getRPos(), n);
                } else if(strmTarget != null) {
                    if(handle.lastOffset != fileOffset) {
                        handle.asyncException(new SFTPException(
                                                  "Out of order packets can't be handled yet!"));
                    }
                    n = replyPkt.readInt();
                    strmTarget.write(replyPkt.getData(), replyPkt.getRPos(), n);
                    handle.lastOffset = fileOffset + n;
                } else {
                    n = replyPkt.readString(buf, off);
                }
                if(n < len) {
                    resend(replyPkt, n);
                } else {
                    handle.asyncEnd(len);
                    releasePacket(replyPkt);
                }
            } catch (IOException e) {
                handle.asyncException(new SFTPException(e.getMessage()));
            } catch (SFTPEOFException e) {
                handle.asyncReadEOF();
            } catch (SFTPException e) {
                handle.asyncException(e);
            }
        }

        private void resend(SFTPPacket pkt, int n) {
            int     i  = getNextId();
            Integer id = new Integer(i);

            fileOffset += n;
            len        -= n;
            off        += n;
            pkt.reset(SSH_FXP_READ, i);
            pkt.writeString(handle.getHandle());
            pkt.writeLong(fileOffset);
            pkt.writeInt(len);
            replyLocks.put(id, this);
            txQueue.putLast(pkt);
        }

        protected synchronized void cancel() {
            handle.asyncException(new SFTPDisconnectException());
            this.notify();
        }

    }

    private final static int   POOL_SIZE = 16;

    private SSH2Connection     connection;
    private SSH2SessionChannel session;
    private Queue              txQueue;

    private int                id;
    private int                version;
    private int                maxDataSize;
    private boolean            isBlocking;
    private boolean            isOpen;
    private volatile boolean   transmitterIsRunning = false;
    private volatile boolean   receiverIsRunning = false;

    private Hashtable          replyLocks;

    private SFTPPacket[]       pktPool;
    private int                pktPoolCnt;

    /**
     * Create a new SFTP client which connects to the server.
     *
     * @param connection Connection to run over.
     * @param isBlocking True if read and write operations should be
     *                   synchronous.
     */
    public SSH2SFTPClient(SSH2Connection connection, boolean isBlocking)
        throws SFTPException {
        this(connection, isBlocking, 32768);
    }

    /**
     * Create a new SFTP client which connects to the server.
     *
     * @param connection Connection to run over.
     * @param isBlocking True if read and write operations should be
     *                   synchronous.
     * @param maxSize    Max packet size, must be 1..32768.
     */
    public SSH2SFTPClient(SSH2Connection connection, boolean isBlocking, int maxSize)
        throws SFTPException {
        if (maxSize < 0 || maxSize > 32768)
            throw new IllegalArgumentException("illegal max data size");
        this.connection = connection;
        this.id         = 0;
        this.isBlocking = isBlocking;
        this.maxDataSize = maxSize;
        this.restart();

        // INIT pkt don't have an id but version is in same place
        //
        SFTPPacket pkt = createPacket();
        pkt.reset(SSH_FXP_INIT, SSH_FILEXFER_VERSION);
        pkt.writeTo(session.getStdIn());
        pkt.reset();
        pkt.failsafeReadFrom(session.getStdOut());
        checkType(pkt, SSH_FXP_VERSION);
        version = pkt.readInt();
        releasePacket(pkt);

        if(!isBlocking) {
            startNonblocking();
        }
    }

    /**
     * Terminate the connection and abort any asynchronous calls which
     * are in progress.
     */
    public synchronized void terminate() {
        receiverIsRunning = false;
        isOpen = false;
        if(session != null) {
            session.close();
        }
        cancelAllAsync();
        if (transmitterIsRunning) {
            transmitterIsRunning = false;
            txQueue.setBlocking(false);
        }
        session = null;
        if(pktPool != null) {
            // Be nice to the GC
            for(int i = 0; i < POOL_SIZE; i++) {
                pktPool[i] = null;
            }
        }
        pktPoolCnt = 0;
    }

    /**
     * Reopens the connection to the server. Any outstanding
     * asynchronous operations are aborted.
     */
    public void restart() throws SFTPException {
        terminate();
        session = connection.newSession();
        if(!session.doSubsystem("sftp")) {
            throw new SFTPException(
                "sftp subsystem couldn't be started on server");
        }
        isOpen = true;
        pktPool = new SFTPPacket[POOL_SIZE];
        pktPoolCnt = POOL_SIZE;
        for(int i = 0; i < POOL_SIZE; i++) {
            pktPool[i] = new SFTPPacket();
        }
    }

    /**
     * Open a file on the server.
     *
     * @param name Name of file
     * @param flags Mode to open file with. Valid values are
     * <code>SSH2SFTP.SSH_FXF_*</code>.
     * @param attrs File attributes for new files.
     *
     * @return A handle identifying the file.
     */
    public FileHandle open(String name, int flags, FileAttributes attrs)
        throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_OPEN);
        pkt.writeString(name);
        pkt.writeInt(flags);
        pkt.writeAttrs(attrs);

        pkt = transmitExpectReply(pkt, SSH_FXP_HANDLE);
        FileHandle handle = new FileHandle(name, pkt.readString(), false);
        releasePacket(pkt);
        return handle;
    }

    /**
     * Close a file. This will wait for any outstanding asynchronous
     * operations.
     *
     * @param handle Handle identifying file.
     */
    public void close(FileHandle handle) throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_CLOSE, handle);
        pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
        releasePacket(pkt);
        handle.asyncClose();
    }

    /**
     * Wait for all outstanding asynchoronous operations on the given
     * file to complete.
     *
     * @param handle Handle identifying file.
     */
    public void asyncWait(FileHandle handle) throws SFTPException {
        handle.asyncWait();
    }

    /**
     * Read data from an open file on the server and stores it in a
     * local file. The data is stored at the same position in the
     * local file as it is read from in the remote file.
     * <p>
     * Note that if the client is operating in
     * asynchronous mode then the method will return before data has been
     * written to the strean. In this case the <code>asyncWait</code>
     * method must be called, or the sftp file closed, before the stream
     * can be closed.
     *
     * @param handle Handle identifying file.
     * @param fileOffset Where in the file to start to read.
     * @param fileTarget Local file to write the data into.
     * @param len Number of bytes to read. Must be less than
     *            maxDataSize, which by default is 32768 bytes.
     *
     * @return The number of read bytes.
     */
    public int read(FileHandle handle, long fileOffset,
                    RandomAccessFile fileTarget, int len)
        throws SFTPException, IOException {
        if(!handle.isOpen()) {
            throw new SFTPAsyncAbortException();
        }
        if (len <= 0 || len > maxDataSize) {
            throw new IllegalArgumentException("len must be: 0 < len <= " + maxDataSize);
        }
        SFTPPacket pkt = createPacket(SSH_FXP_READ, handle);
        pkt.writeLong(fileOffset);
        pkt.writeInt(len);

        if(isBlocking) {
            try {
                pkt = transmitExpectReply(pkt, SSH_FXP_DATA);
                len = pkt.readInt();
                fileTarget.seek(fileOffset);
                fileTarget.write(pkt.getData(), pkt.getRPos(), len);
                return len;
            } catch (SFTPEOFException e) {
                return 0;
            } finally {
                if(pkt != null)
                    releasePacket(pkt);
            }
        } else {
            Integer   id    = new Integer(pkt.getId());
            ReplyLock reply = new ReadReplyLock(handle, fileOffset, fileTarget,
                                                len);
            replyLocks.put(id, reply);
            txQueue.putLast(pkt);
            return len;
        }
    }

    /**
     * Read data from an open file on the server and stores it in a
     * local buffer. Note that if the client is operating in
     * asynchronous mode then the method will return before data has been
     * placed in the buffer. In this case the <code>asyncWait</code>
     * method must be called, or the file closed, before the data can
     * be safely accessed.
     *
     * @param handle Handle identifying file.
     * @param fileOffset Where in the file to start to read.
     * @param buf Local buffer to store data in. Must hold
     *            <code>len</code> bytes at the given offset.
     * @param off Offset in buffer to store data at.
     * @param len Number of bytes to read.  Must be less than
     *            maxDataSize, which by default is 32768 bytes.
     *
     * @return The number of read bytes.
     */
    public int read(FileHandle handle, long fileOffset,
                    byte[] buf, int off, int len)
        throws SFTPException {
        if(!handle.isOpen()) {
            throw new SFTPAsyncAbortException();
        }
        if (len <= 0 || len > maxDataSize) {
            throw new IllegalArgumentException("len must be: 0 < len <= " + maxDataSize);
        }
        SFTPPacket pkt = createPacket(SSH_FXP_READ, handle);
        pkt.writeLong(fileOffset);
        pkt.writeInt(len);

        if(isBlocking) {
            try {
                pkt = transmitExpectReply(pkt, SSH_FXP_DATA);
                return pkt.readString(buf, off);
            } catch (SFTPEOFException e) {
                return 0;
            } finally {
                if(pkt != null)
                    releasePacket(pkt);
            }
        } else {
            if(!isOpen) {
                throw new SFTPDisconnectException();
            }
            Integer   id    = new Integer(pkt.getId());
            ReplyLock reply = new ReadReplyLock(handle, fileOffset,
                                                buf, off, len);
            replyLocks.put(id, reply);
            txQueue.putLast(pkt);
            return len;
        }
    }

    /**
     * Read the entire file on the server and store in a local
     * stream. This method will be much faster if asynchronous mode is
     * used. It will always wait until the operation has completed
     * before returning, even if running in asynchronous mode.
     *
     * @param handle Handle identifying file. The handle will be
     *               closed when the transfer has completed.
     * @param out Stream to store data in.
     *
     * @return Number of bytes read.
     */
    public int readFully(FileHandle handle, OutputStream out)
        throws SFTPException, IOException {
        if(!handle.isOpen()) {
            throw new SFTPAsyncAbortException();
        }
        FileAttributes attrs = fstat(handle);

        long len   = attrs.size;
        long foffs = 0;
        int cnt   = 0;

        try {
            while(foffs < len) {
                int n = (maxDataSize < (len - foffs) ? maxDataSize :
                         (int)(len - foffs));

                SFTPPacket pkt = createPacket(SSH_FXP_READ, handle);
                pkt.writeLong(foffs);
                pkt.writeInt(n);

                if(isBlocking) {
                    try {
                        pkt = transmitExpectReply(pkt, SSH_FXP_DATA);
                        n = pkt.readInt();
                        out.write(pkt.getData(), pkt.getRPos(), n);
                    } finally {
                        if(pkt != null)
                            releasePacket(pkt);
                    }
                } else {
                    Integer   id    = new Integer(pkt.getId());
                    ReplyLock reply = new ReadReplyLock(handle, foffs, out, n);
                    replyLocks.put(id, reply);
                    txQueue.putLast(pkt);
                }

                foffs += n;

                if(!isBlocking && ++cnt == 24) {
                    cnt = 0;
                    handle.asyncWait(12);
                }
            }

            if(!isBlocking) {
                handle.asyncWait();
            }

        } finally {
            close(handle);
        }

        return (int)attrs.size;
    }

    /**
     * Internal write function.
     */
    protected void writeInternal(FileHandle handle, SFTPPacket pkt, int len)
    throws SFTPException {
        if(isBlocking) {
            pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
            releasePacket(pkt);
        } else {
            if(!isOpen) {
                throw new SFTPDisconnectException();
            }
            Integer   id    = new Integer(pkt.getId());
            ReplyLock reply = new WriteReplyLock(handle, len);
            replyLocks.put(id, reply);
            txQueue.putLast(pkt);
        }
    }

    /**
     * Write data to a remote file.
     *
     * @param handle Handle identifying file.
     * @param fileOffset Offset to store data at.
     * @param buf Buffer containing data to write.
     * @param off Offset in <code>buf</code> to read data at.
     * @param len Number of bytes to write. Must be less than
     *            maxDataSize, which by default is 32768 bytes.
     */
    public void write(FileHandle handle, long fileOffset,
                      byte[] buf, int off, int len)
    throws SFTPException {
        if(!handle.isOpen()) {
            throw new SFTPAsyncAbortException();
        }
        if (len <= 0 || len > maxDataSize) {
            throw new IllegalArgumentException("len must be: 0 < len <= " + maxDataSize);
        }
        SFTPPacket pkt = createPacket(SSH_FXP_WRITE, handle);
        pkt.writeLong(fileOffset);
        pkt.writeString(buf, off, len);

        writeInternal(handle, pkt, len);
    }

    /**
     * Write an entire stream to a file on the server.
     * This method will be much faster if asynchronous mode is
     * used. It will always wait until the operation has completed
     * before returning, even if running in asynchronous mode.
     *
     * @param handle Handle identifying file. The handle will be
     *               closed when the transfer has completed.
     * @param in Stream to read data to write from.
     */
    public long writeFully(FileHandle handle, InputStream in)
        throws SFTPException, IOException {
        return writeFully(handle, in, false);
    }

    /**
     * Write an entire stream to a file on the server.
     * This method will be much faster if asynchronous mode is
     * used. It will always wait until the operation has completed
     * before returning, even if running in asynchronous mode.
     *
     * @param handle Handle identifying file. The handle will be
     *               closed when the transfer has completed.
     * @param in Stream to read data to write from.
     * @param append if true then the data is written at the end of
     *        the opened file. In this case the handle must have been opened
     *        in append mode.
     */
    public long writeFully(FileHandle handle, InputStream in, boolean append)
    throws SFTPException, IOException {
        if(!handle.isOpen()) {
            throw new SFTPAsyncAbortException();
        }

        int len   = 0;
        long foffs = 0;
        int cnt   = 0;
        int lPos  = 0;

        if (append) 
            foffs = stat(handle.getName()).size;

        try {
            for(;;) {
                SFTPPacket pkt = createPacket(SSH_FXP_WRITE, handle);
                pkt.writeLong(foffs);
                lPos = pkt.getWPos();
                pkt.writeInt(0); // write dummy length
                int n = pkt.getMaxWriteSize();
                n = (n > maxDataSize ? maxDataSize : n);
                len = in.read(pkt.getData(), pkt.getWPos(), n);

                if(len > 0) {
                    pkt.setWPos(lPos);
                    pkt.writeInt(len); // write real length
                    pkt.setWPos(lPos + 4 + len);
                    writeInternal(handle, pkt, len);
                    foffs += len;
                    if(!isBlocking && ++cnt == 24) {
                        cnt = 0;
                        handle.asyncWait(12);
                    }

                    if((cnt % 6 == 1))
                        Thread.yield();

                } else {
                    break;
                }
            }

            if(!isBlocking) {
                handle.asyncWait();
            }

        } finally {
            close(handle);
        }

        return foffs;
    }

    /**
     * Write data buffer to a file on the server.
     * This method will be much faster if asynchronous mode is
     * used. It will always wait until the operation has completed
     * before returning, even if running in asynchronous mode.
     *
     * @param handle Handle identifying file. The handle will be
     *               closed when the transfer has completed.
     * @param in Buffer containing data to write.
     * @param append if true then the data is written at the end of
     *        the opened file. In this case the handle must have been opened
     *        in append mode.
     */
    public long writeFully(FileHandle handle, byte[] in, boolean append)
        throws SFTPException, IOException {
        return writeFully(handle, new java.io.ByteArrayInputStream(in), append);
    }


    /**
     * Get attributes of a file on the server. If the name refers to a
     * symbolic link, then this version will return information about
     * the actual link.
     *
     * @param name Name of file to get attributes of.
     *
     * @return The attributes of the given name.
     */
    public FileAttributes lstat(String name)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_LSTAT);
        pkt.writeString(name);

        return statInternal(pkt);
    }

    /**
     * Get attributes of a file on the server. If the name refers to a
     * symbolic link, then this version will return information about
     * the file the link points at.
     *
     * @param name Name of file to get attributes of.
     *
     * @return The attributes of the given name.
     */
    public FileAttributes stat(String name)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_STAT);
        pkt.writeString(name);

        return statInternal(pkt);
    }

    /**
     * Get attributes of an open file on the server.
     *
     * @param handle Handle identifying file.
     *
     * @return The attributes of the given name.
     */
    public FileAttributes fstat(FileHandle handle)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_FSTAT, handle);

        return statInternal(pkt);
    }

    private FileAttributes statInternal(SFTPPacket pkt) throws SFTPException {
        pkt = transmitExpectReply(pkt, SSH_FXP_ATTRS);
        FileAttributes attrs = pkt.readAttrs();
        releasePacket(pkt);
        return attrs;
    }

    /**
     * Set attributes on a file.
     *
     * @param name Name of file to set attributes on.
     * @param attrs Attributes to set.
     */
    public void setstat(String name, FileAttributes attrs)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_SETSTAT);
        pkt.writeString(name);
        pkt.writeAttrs(attrs);

        pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
        releasePacket(pkt);
    }

    /**
     * Set attributes on an open file.
     *
     * @param handle Handle identifying the file.
     * @param attrs Attributes to set.
     */
    public void fsetstat(FileHandle handle, FileAttributes attrs)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_FSETSTAT, handle);

        pkt.writeAttrs(attrs);

        pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
        releasePacket(pkt);
    }

    /**
     * Opens a directory on the server. This must be done before one
     * can get a list of files contained in the directory.
     *
     * @param path name of directory to open
     *
     * @return A handle to the open directory.
     */
    public FileHandle opendir(String path)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_OPENDIR);
        pkt.writeString(path);

        pkt = transmitExpectReply(pkt, SSH_FXP_HANDLE);
        FileHandle handle = new FileHandle(path, pkt.readString(), true);
        releasePacket(pkt);
        return handle;
    }

    /**
     * Gets a list of files, and other objects, in an open
     * directory. The handle used here must have been obtained with an
     * earlier call to <code>opendir</code>.
     *
     * @param handle Hande identifying the remote directory.
     *
     * @return An array of attributes with one entry per contained file.
     */
    public FileAttributes[] readdir(FileHandle handle)
    throws SFTPException {
        Vector         result = new Vector(256);
        FileAttributes item   = null;

        try {
            while(true) {
                SFTPPacket pkt = createPacket(SSH_FXP_READDIR, handle);

                pkt = transmitExpectReply(pkt, SSH_FXP_NAME);

                int count = pkt.readInt();

                if(count == 0) {
                    /* Server should send EOF but better safe than sorry... */
                    break;
                }

                for(int i = 0; i < count; i++) {
                    String name  = pkt.readJavaString();
                    String lname = pkt.readJavaString();
                    item         = pkt.readAttrs();
                    item.name    = name;
                    item.lname   = lname;
                    item.hasName = true;
                    result.addElement(item);
                }

                releasePacket(pkt);
            }
        } catch (SFTPEOFException e) {
            /* End of directory listing */
        }

        FileAttributes[] list = new FileAttributes[result.size()];
        for(int i = 0; i < list.length; i++) {
            list[i] = (FileAttributes)result.elementAt(i);
        }

        return list;
    }

    /**
     * Remove a file from the server.
     *
     * @param name Name of file to remove.
     */
    public void remove
        (String name) throws SFTPException {
            SFTPPacket pkt = createPacket(SSH_FXP_REMOVE);
            pkt.writeString(name);

            pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
            releasePacket(pkt);
        }

    /**
     * Rename a file on the server.
     *
     * @param oldName current name of file to rename.
     * @param newName desired new name of file.
     */
    public void rename(String oldName, String newName)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_RENAME);
        pkt.writeString(oldName);
        pkt.writeString(newName);

        pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
        releasePacket(pkt);
    }

    /**
     * Create a new directory on the server.
     *
     * @param name name of directory to create.
     * @param attrs Attributes to apply to the new directory.
     */
    public void mkdir(String name, FileAttributes attrs)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_MKDIR);
        pkt.writeString(name);
        pkt.writeAttrs(attrs);

        pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
        releasePacket(pkt);
    }

    /**
     * Removes a directory from the server
     *
     * @param name Name of directory to remove.
     */
    public void rmdir(String name) throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_RMDIR);
        pkt.writeString(name);

        pkt = transmitExpectReply(pkt, SSH_FXP_STATUS);
        releasePacket(pkt);
    }

    /**
     * Canonalize a given path. The canonalized path will start from
     * the root and will not contain and '<code>..</code>'.
     *
     * @param nameIn Path to canonalize.
     *
     * @return A <code>FileAttributes</code> object with the name and
     * lname fields filled in.
     */
    public FileAttributes realpath(String nameIn)
    throws SFTPException {
        SFTPPacket pkt = createPacket(SSH_FXP_REALPATH);
        pkt.writeString(nameIn);

        pkt = transmitExpectReply(pkt, SSH_FXP_NAME);
        int            cnt   = pkt.readInt(); // Always one
        String         name  = pkt.readJavaString();
        String         lname = pkt.readJavaString();
        FileAttributes attrs = pkt.readAttrs();
        attrs.name    = name;
        attrs.lname   = lname;
        attrs.hasName = true;
        releasePacket(pkt);

        return attrs;
    }

    private SFTPPacket transmitExpectReply(SFTPPacket pkt, int expectType)
    throws SFTPException {
        if(!isOpen) {
            throw new SFTPDisconnectException();
        }
        if(isBlocking) {
            synchronized(this) {
                int expectId = pkt.getId();
                pkt.writeTo(session.getStdIn());
                pkt.reset();
                pkt.readFrom(session.getStdOut());
                if(expectId != pkt.readInt()) {
                    throw new SFTPException("SFTP error, invalid packet id");
                }
                checkType(pkt, expectType);
                return pkt;
            }
        } else {
            Integer   id    = new Integer(pkt.getId());
            ReplyLock reply = new ReplyLock(expectType);
            replyLocks.put(id, reply);
            txQueue.putLast(pkt);
            return reply.expect();
        }
    }

    private void startNonblocking() {
        txQueue = new Queue(connection.getPreferences().
                            getIntPreference(SSH2Preferences.QUEUE_DEPTH),
                            connection.getPreferences().
                            getIntPreference(SSH2Preferences.QUEUE_HIWATER));
        replyLocks = new Hashtable();
        /*
         * NOTE: We could implement custom I/O streams and insert them to handle
         * the work in the tx/rx threads of the SSH2StreamChannel though the
         * gains are not huge.
         */
        Thread transmitter = new Thread(new Runnable() {
                                            public void run() {
                                                sftpTransmitLoop();
                                            }
                                        }
                                        , "SSH2SFTPTX");
        Thread receiver = new Thread(new Runnable() {
                                         public void run() {
                                             sftpReceiveLoop();
                                         }
                                     }
                                     , "SSH2SFTPRX");

        transmitter.setDaemon(true);
        receiver.setDaemon(true);
        transmitterIsRunning = true;
        transmitter.start();
        receiverIsRunning = true;
        receiver.start();
    }

    private void sftpTransmitLoop() {
        SFTPPacket pkt;
        try {
            while(transmitterIsRunning && (pkt = (SFTPPacket)txQueue.getFirst()) != null) {
                pkt.writeTo(session.getStdIn());
                releasePacket(pkt);
            }
        } catch (Throwable t) {
            connection.getLog().error("SSH2SFTPClient",
                                      "sftpTransmitLoop",
                                      "session was probably closed");
            terminate();
        }
        transmitterIsRunning = false;
    }

    private void sftpReceiveLoop() {
        SFTPPacket pkt;
        Integer    id;
        ReplyLock  reply;
        try {
            while(receiverIsRunning) {
                pkt = createPacket();
                pkt.reset();
                pkt.readFrom(session.getStdOut());

                id    = new Integer(pkt.readInt());
                reply = (ReplyLock)replyLocks.remove(id);
                if(reply == null) {
                    connection.getLog().error("SSH2SFTPClient",
                                              "sftpReceiveLoop",
                                              "received unsent id: " +
                                              id);
                    connection.getLog().debug2("SSH2SFTPClient",
                                               "sftpReceiveLoop",
                                               "sftp packet: ",
                                               pkt.getData(),
                                               0,
                                               pkt.getLength() + 5);
                    throw new SFTPException("Invalid reply");
                }

                reply.received(pkt);
            }
        } catch (SFTPDisconnectException e) {
            connection.getLog().debug("SSH2SFTPClient",
                                      "sftpReceiveLoop",
                                      "session was closed");
        } catch (SFTPException e) {
            connection.getLog().error("SSH2SFTPClient",
                                      "sftpReceiveLoop",
                                      "session was probably closed");
        } finally {
            if (receiverIsRunning) {
                terminate();
                receiverIsRunning = false;
            }
        }
    }

    private SFTPPacket createPacket(int type, FileHandle handle) {
        SFTPPacket pkt = createPacket(type);
        pkt.writeString(handle.getHandle());
        return pkt;
    }

    private SFTPPacket createPacket(int type) {
        SFTPPacket pkt = createPacket();
        pkt.reset(type, getNextId());
        return pkt;
    }

    private SFTPPacket createPacket() {
        SFTPPacket pkt;
        synchronized(pktPool) {
            if(pktPoolCnt == 0) {
                pkt = new SFTPPacket();
            } else {
                pkt = pktPool[--pktPoolCnt];
            }
        }
        return pkt;
    }

    private void releasePacket(SFTPPacket pkt) {
        synchronized(pktPool) {
            if(pktPoolCnt < POOL_SIZE) {
                pktPool[pktPoolCnt++] = pkt;
            }
        }
    }

    private void checkType(SFTPPacket pkt, int type) throws SFTPException {
        if(pkt.getType() == SSH_FXP_STATUS) {
            int error = pkt.readInt();
            if(error == SSH_FX_OK)
                return;
            if(error == SSH_FX_EOF)
                throw new SFTPEOFException();
            if(error == SSH_FX_NO_SUCH_FILE)
                throw new SFTPNoSuchFileException();
            if(error == SSH_FX_PERMISSION_DENIED)
                throw new SFTPPermissionDeniedException();
            if(error == SSH_FX_CONNECTION_LOST)
                throw new SFTPDisconnectException();
            // !!! TODO: provide error
            throw new SFTPException("Got error: " + error);
        } else if(pkt.getType() != type) {
            // !!! TODO: provide fatal error
            throw new SFTPException("Got unexpected packet: " + pkt.getType());
        }
    }

    private void cancelAllAsync() {
        if(replyLocks == null) {
            return;
        }
        Enumeration ids = replyLocks.keys();
        while(ids.hasMoreElements()) {
            ReplyLock l = (ReplyLock)replyLocks.remove(ids.nextElement());
            l.cancel();
        }
    }

    private synchronized int getNextId() {
        return id++;
    }

}
