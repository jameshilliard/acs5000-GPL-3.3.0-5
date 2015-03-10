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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Describes the sftp protocol. The actual implementation can then be
 * found in <code>SSH2SFTPClient</code>. These classes implement
 * version 2 of the sftp protocol.
 *
 * @see SSH2SFTPClient
 */
public class SSH2SFTP {

    private static final int DATA_BUFFER_SIZE = 34000;
    private static final int FAILSAFE_READ_BYTES = 1024;

    /**
     * Describes a file. Client applications should not access
     * the methods in this class directory.
     *
     * @see SSH2SFTPClient
     */
    public static final class FileHandle {

        private byte[]        handle;
        private String        name;
        private boolean       isDirectory;
        private boolean       isOpen;
        private boolean       asyncEOF;
        private int           asyncCnt;
        private int           reqLeft;
        private SFTPException asyncException;
        private AsyncListener listener;

        /**
         * This is used by SSH2SFTPClient to protect against out-of
         * order packets.
         */
        protected volatile long lastOffset;

        /**
         * Construct a new FileHandle with the provided data
         *
         * @param name name of file
         * @param handle handle to use in sftp protocol
         * @param isDirectory true if this is a directory
         */
        protected FileHandle(String name, byte[] handle, boolean isDirectory) {
            this.name           = name;
            this.handle         = handle;
            this.isDirectory    = isDirectory;
            this.isOpen         = true;
            this.asyncCnt       = 0;
            this.reqLeft        = 0;
            this.asyncException = null;
            this.lastOffset     = 0L;
        }

        /**
         * Check if thius handle refers to a directory. Client
         * applications should ideally use any of the stat functions
         * in SSH2SFTPClient to get this data.
         */
        public boolean isDirectory() {
            return isDirectory;
        }

        /**
         * Check if this handle is open or not
         */
        public boolean isOpen() {
            return isOpen;
        }

        /**
         * Get name of underlying file.
         */
        public String getName() {
            return name;
        }

        /**
         * Get the sftp protocol handle
         */
        protected byte[] getHandle() {
            return handle;
        }

        /**
         * Register a listener which will be notified when
         * asynchronous operations ends. The argument to the progress
         * function will be the number of bytes read.
         */
        public void addAsyncListener(AsyncListener listener) {
            this.listener = listener;
        }

        /**
         * An asynchronous operation has been started
         */
        protected synchronized void asyncStart(int len) {
            asyncCnt++;
        }

        /**
         * An asynchronous operation has completed
         */
        protected synchronized void asyncEnd(int len) {
            asyncCnt--;
            if(asyncCnt <= reqLeft) {
                this.notifyAll();
            }
            if(listener != null) {
                listener.progress((long)len);
            }
        }

        /**
         * Got an asyncronous EOF from server.
         */
        protected synchronized void asyncReadEOF() {
            asyncEOF = true;
            asyncEnd(0);
        }

        /**
         * Got an asyncronous exception
         */
        protected synchronized void asyncException(SFTPException e) {
            asyncException = e;
            this.notifyAll();
        }

        /**
         * Close an asynchronous file. No asynchronous request may be
         * outstanding.
         */
        protected synchronized void asyncClose() {
            if(asyncCnt > 0) {
                asyncException(new SFTPAsyncAbortException());
            }
            isOpen = false;
        }

        /**
         * Wait for all asynchronous operations to complete.
         *
         * @return true if eof has been received
         */
        public synchronized boolean asyncWait()
            throws SFTPException {
            return asyncWait(0);
        }

        /**
         * Wait until there is less than a certain number of
         * asynchronous operations outstanding.
         *
         * @return true if eof has been received
         */
        protected synchronized boolean asyncWait(int reqLeft)
            throws SFTPException {
            if(this.reqLeft < reqLeft) {
                this.reqLeft = reqLeft;
            }
            while(asyncCnt > reqLeft && asyncException == null && !asyncEOF) {
                try {
                    this.wait();
                } catch (InterruptedException e) {}
            }
            if(asyncException != null) {
                throw (SFTPException)asyncException.fillInStackTrace();
            }
            boolean eof = asyncEOF;
            asyncEOF    = false;
            return eof;
        }

        /**
         * Create a string representation of this object
         */
        public synchronized String toString() {
            return "FileHandle[name='" + name +
                   "',isDir=" + isDirectory +
                   ",isOpen=" + isOpen +
                   ",asyncEOF=" + asyncEOF +
                   ",asyncCnt=" + asyncCnt +
                   ",reqLeft=" + reqLeft +
                   ",asyncE=" + asyncException +
                   ",asyncL=" + listener + "]";
        }

    }

    /**
     * An interface implemented by FTP controllers which want progress reports.
     */
    public static interface AsyncListener {
        /**
         * Gets called periodically during the file transfer.
         *
         * @param size How many bytes have been transferred so far
         */
        public void progress(long size);
    }

    /**
     * Class describing the different file attributes.
     *
     * @see SSH2SFTPClient
     */
    public static final class FileAttributes {

        char[] types = { 'p', 'c', 'd', 'b', '-', 'l', 's', };

        /** Format mask, used to mask off the format flags from the mode */
        public final static int S_IFMT   = 0170000;
        /** Socket flag, set if this is a socket */
        public final static int S_IFSOCK = 0140000;
        /** Link flag, set if this is a symbolic link */
        public final static int S_IFLNK  = 0120000;
        /** Regular file flag, set if this is a regular file */
        public final static int S_IFREG  = 0100000;
        /** Block device flag, set if this is a block device */
        public final static int S_IFBLK  = 0060000;
        /** Directory flag, set if this is a directory */
        public final static int S_IFDIR  = 0040000;
        /** Character device flag, set if this is a character device */
        public final static int S_IFCHR  = 0020000;
        /** FIFO flag, set if this is a FIFO pipe */
        public final static int S_IFIFO  = 0010000;

        /** set-uid (SUID) flag */
        public final static int S_ISUID = 0004000;

        /** set-gid (SGID) flag */
        public final static int S_ISGID = 0002000;

        /** User (owner) read rights bit */
        public final static int S_IRUSR = 00400;
        /** User (owner) write rights bit */
        public final static int S_IWUSR = 00200;
        /** User (owner) execute rights bit */
        public final static int S_IXUSR = 00100;
        /** Group read rights bit */
        public final static int S_IRGRP = 00040;
        /** Group write rights bit */
        public final static int S_IWGRP = 00020;
        /** Group execute rights bit */
        public final static int S_IXGRP = 00010;
        /** Other read rights bit */
        public final static int S_IROTH = 00004;
        /** Other write rights bit */
        public final static int S_IWOTH = 00002;
        /** Other execute rights bit */
        public final static int S_IXOTH = 00001;

        /** True if the name field has been initialized */
        public boolean hasName;

        /** True if the size field has been initialized */
        public boolean hasSize;

        /** True if the user and group fields has been initialized */
        public boolean hasUserGroup;

        /** True if the permissions field has been initialized */
        public boolean hasPermissions;

        /** True if the mod time fiels has been initialized */
        public boolean hasModTime;

        /** Name of file */
        public String  name;

        /** Only valid for symbolic links, the name the link points at */
        public String  lname;

        /** Size of file */
        public long    size;

        /** uid of file */
        public int     uid;

        /** gid of file */
        public int     gid;

        /** Permissions flags */
        public int     permissions;

        /** Time of last access */
        public int     atime;

        /** Time of last modification */
        public int     mtime;

        /**
         * Return a string identifying the file. The generated string
         * looks like the output of <code>ls -l</code>.
         */
        public String toString() {
            return toString(hasName ? name : "<noname>");
        }

        /**
         * Utility function which right-justifies a string within a
         * given width.
         */
        private static String rightJustify(String s, int width) {
            String res = s;
            while (res.length() < width)
                res = " " + res;
            return res;
        }

        /**
         * Return a string identifying the file. The generated string
         * looks like the output of <code>ls -l</code>.
         *
         * @param name name of file to print
         */
        public String toString(String name) {
            StringBuffer str = new StringBuffer();
            str.append(permString());
            str.append("    1 ");
            str.append(rightJustify(Integer.toString(uid), 8));
            str.append(" ");
            str.append(rightJustify(Integer.toString(gid), 8));
            str.append(" ");
            str.append(rightJustify(Long.toString(size), 16));
            str.append(" ");
            str.append(modTimeString());
            str.append(" ");
            str.append(name);
            return str.toString();
        }

        /**
         * Generate the permissions part of the <code>ls -l</code>
         * simulated string.
         */
        public String permString() {
            StringBuffer str = new StringBuffer();
            str.append(types[(permissions & S_IFMT) >>> 13]);
            str.append(rwxString(permissions, 6));
            str.append(rwxString(permissions, 3));
            str.append(rwxString(permissions, 0));
            return str.toString();
        }

        /**
         * Print modification time in a format similar to that of
         * <code>ls -l</code>. That is for dates whiel is less than
         * six months old "MMM dd hh:mm" and for older dates "MMM dd  yyyy".
         */
        public String modTimeString() {
            SimpleDateFormat df;
            long mt  = (mtime * 1000L);
            long now = System.currentTimeMillis();
            if((now - mt) > (6 * 30 * 24 * 60 * 60 * 1000L)) {
                df = new SimpleDateFormat("MMM dd  yyyy");
            } else {
                df = new SimpleDateFormat("MMM dd hh:mm");
            }
            return df.format(new Date(mt));
        }

        /**
         * Generate part of the permissions string
         */
        private String rwxString(int v, int r) {
            v >>>= r;
            String rwx = ((((v & 0x04) != 0) ? "r" : "-") +
                          (((v & 0x02) != 0) ? "w" : "-"));
            if((r == 6 && isSUID()) ||
                    (r == 3 && isSGID())) {
                rwx += (((v & 0x01) != 0) ? "s" : "S");
            } else {
                rwx += (((v & 0x01) != 0) ? "x" : "-");
            }
            return rwx;
        }

        /**
         * Returns true if this is a socket.
         */
        public boolean isSocket() {
            return ((permissions & S_IFSOCK) == S_IFSOCK);
        }

        /**
         * Returns true if this is a symbolic link.
         */
        public boolean isLink() {
            return ((permissions & S_IFLNK) == S_IFLNK);
        }

        /**
         * Returns true if this is a regular file.
         */
        public boolean isFile() {
            return ((permissions & S_IFREG) == S_IFREG);
        }

        /**
         * Returns true if this is a block device.
         */
        public boolean isBlock() {
            return ((permissions & S_IFBLK) == S_IFBLK);
        }

        /**
         * Returns true if this is a directory.
         */
        public boolean isDirectory() {
            return ((permissions & S_IFDIR) == S_IFDIR);
        }

        /**
         * Returns true if this is a character device.
         */
        public boolean isCharacter() {
            return ((permissions & S_IFCHR) == S_IFCHR);
        }

        /**
         * Returns true if this is a FIFO pipe.
         */
        public boolean isFifo() {
            return ((permissions & S_IFIFO) == S_IFIFO);
        }

        /**
         * Returns true if this object has the setuid flag set.
         */
        public boolean isSUID() {
            return ((permissions & S_ISUID) == S_ISUID);
        }

        /**
         * Returns true if this object has the setgid flag set.
         */
        public boolean isSGID() {
            return ((permissions & S_ISGID) == S_ISGID);
        }

    }

    /**
     * An exception in the SFTP code. It only holds a string message.
     */
    public static class SFTPException extends Exception {
        public SFTPException() {}

        public SFTPException(String msg) {
            super(msg);
        }
    }

    public static class SFTPEOFException extends SFTPException {}

    public static class SFTPNoSuchFileException extends SFTPException {}

    public static class SFTPPermissionDeniedException extends SFTPException {}

    public static class SFTPDisconnectException extends SFTPException {}

    public static class SFTPAsyncAbortException extends SFTPException {}

    /**
     * Handles sftp data packets. Contains functions to read and write
     * the sftp types.
     */
    protected static final class SFTPPacket extends SSH2DataBuffer {

        private int type;
        private int id;
        private int len;

        public SFTPPacket() {
            super(DATA_BUFFER_SIZE);
        }

        public void reset(int type, int id) {
            reset();
            writeInt(0); // dummy length
            writeByte(type);
            writeInt(id);
            this.type = type;
            this.id   = id;
        }

        public int getType() {
            return type;
        }

        public int getId() {
            return id;
        }

        public int getLength() {
            return len;
        }

        public void writeAttrs(FileAttributes attrs) {
            writeInt((attrs.hasSize ? SSH_ATTR_SIZE : 0) |
                     (attrs.hasUserGroup ? SSH_ATTR_UIDGID : 0) |
                     (attrs.hasPermissions ? SSH_ATTR_PERM : 0) |
                     (attrs.hasModTime ? SSH_ATTR_MODTIME : 0));
            if(attrs.hasSize) {
                writeLong(attrs.size);
            }
            if(attrs.hasUserGroup) {
                writeInt(attrs.uid);
                writeInt(attrs.gid);
            }
            if(attrs.hasPermissions) {
                writeInt(attrs.permissions);
            }
            if(attrs.hasModTime) {
                writeInt(attrs.atime);
                writeInt(attrs.mtime);
            }
        }

        public FileAttributes readAttrs() {
            FileAttributes attrs = new FileAttributes();
            int            flags = readInt();
            attrs.hasSize        = ((flags & SSH_ATTR_SIZE) != 0);
            attrs.hasUserGroup   = ((flags & SSH_ATTR_UIDGID) != 0);
            attrs.hasPermissions = ((flags & SSH_ATTR_PERM) != 0);
            attrs.hasModTime     = ((flags & SSH_ATTR_MODTIME) != 0);
            if (attrs.hasSize) {
                attrs.size = readLong();
            }
            if (attrs.hasUserGroup) {
                attrs.uid = readInt();
                attrs.gid = readInt();
            }
            if (attrs.hasPermissions) {
                attrs.permissions = readInt();
            }
            if (attrs.hasModTime) {
                attrs.atime = readInt();
                attrs.mtime = readInt();
            }
            return attrs;
        }

        private void internalReadFrom(int cnt, InputStream in)
        throws SFTPException {
            len = 5;
            try {
                while(cnt < len) {
                    int n;
                    n = in.read(data, cnt, (len - cnt));
                    if(n == -1) {
                        throw new SFTPDisconnectException();
                    }
                    cnt += n;
                    if(cnt == 5) {
                        len  = readInt() + 4;
                        type = readByte();
                    }
                }
            } catch (IOException e) {
                throw new SFTPException(e.getMessage());
            }
            len -= 5;
        }

        /**
         * Failsafe read method. Tries to read a packet from the given stream.
         * but does ignore stuff which does not look like a packet. The
         * algorithm is that a probable packet starts with two bytes containing
         * zeros. 
         * <p>
         * This is useful when reading the first version packet which is
         * small but may be, on broken systems, preceded by some ascii
         * characters.
         *
         * @param in Stream to read data from
         */
        public void failsafeReadFrom(InputStream in)
        throws SFTPException {
            byte[] b = new byte[2];

            try {
                if (2 != in.read(b, 0, 2)) {
                    throw new SFTPDisconnectException();
                }
                for (int tries = 0; tries < FAILSAFE_READ_BYTES; tries++) {
                    if (b[0] == 0 && b[1] == 0) {
                        data[0] = b[0];
                        data[1] = b[1];
                        internalReadFrom(2, in);
                        return;
                    }
                    b[0] = b[1];
                    if (1 != in.read(b, 1, 1)) {
                        throw new SFTPDisconnectException();
                    }
                }
            } catch (IOException e) {
                throw new SFTPException(e.getMessage());
            }
            throw new SFTPException("Failed to find first packet");
        }

        public void readFrom(InputStream in)
        throws SFTPException {
            internalReadFrom(0, in);
        }

        public void writeTo(OutputStream out)
        throws SFTPException {
            len = getWPos() - 5;
            setWPos(0);
            writeInt(len + 1);
            try {
                out.write(data, 0, len + 5);
            } catch (IOException e) {
                throw new SFTPException(e.getMessage());
            }
        }
    }

    /* Version is 3 according to draft minus extension in init */
    protected final static int SSH_FILEXFER_VERSION =    2;

    /* Packet types. */
    protected final static int SSH_FXP_INIT =            1;
    protected final static int SSH_FXP_VERSION =         2;
    protected final static int SSH_FXP_OPEN =            3;
    protected final static int SSH_FXP_CLOSE =           4;
    protected final static int SSH_FXP_READ =            5;
    protected final static int SSH_FXP_WRITE =           6;
    protected final static int SSH_FXP_LSTAT =           7;
    protected final static int SSH_FXP_FSTAT =           8;
    protected final static int SSH_FXP_SETSTAT =         9;
    protected final static int SSH_FXP_FSETSTAT =       10;
    protected final static int SSH_FXP_OPENDIR =        11;
    protected final static int SSH_FXP_READDIR =        12;
    protected final static int SSH_FXP_REMOVE =         13;
    protected final static int SSH_FXP_MKDIR =          14;
    protected final static int SSH_FXP_RMDIR =          15;
    protected final static int SSH_FXP_REALPATH =       16;
    protected final static int SSH_FXP_STAT =           17;
    protected final static int SSH_FXP_RENAME =         18;

    protected final static int SSH_FXP_STATUS =         101;
    protected final static int SSH_FXP_HANDLE =         102;
    protected final static int SSH_FXP_DATA =           103;
    protected final static int SSH_FXP_NAME =           104;
    protected final static int SSH_FXP_ATTRS =          105;
    protected final static int SSH_FXP_EXTENDED =       200;
    protected final static int SSH_FXP_EXTENDED_REPLY = 201;

    /* Status/error codes. */
    public final static int SSH_FX_OK =                0;
    public final static int SSH_FX_EOF =               1;
    public final static int SSH_FX_NO_SUCH_FILE =      2;
    public final static int SSH_FX_PERMISSION_DENIED = 3;
    public final static int SSH_FX_FAILURE =           4;
    public final static int SSH_FX_BAD_MESSAGE =       5;
    public final static int SSH_FX_NO_CONNECTION =     6;
    public final static int SSH_FX_CONNECTION_LOST =   7;
    public final static int SSH_FX_OP_UNSUPPORTED =    8;

    /* Portable versions of O_RDONLY etc. */
    public final static int SSH_FXF_READ =            0x0001;
    public final static int SSH_FXF_WRITE =           0x0002;
    public final static int SSH_FXF_APPEND =          0x0004;
    public final static int SSH_FXF_CREAT =           0x0008;
    public final static int SSH_FXF_TRUNC =           0x0010;
    public final static int SSH_FXF_EXCL =            0x0020;

    /* Flags indicating presence of file attributes. */
    protected final static int SSH_ATTR_SIZE =         0x01;
    protected final static int SSH_ATTR_UIDGID =       0x02;
    protected final static int SSH_ATTR_PERM =         0x04;
    protected final static int SSH_ATTR_MODTIME =      0x08;

}
