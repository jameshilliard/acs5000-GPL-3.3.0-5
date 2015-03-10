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

package com.mindbright.sshcommon;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Dialog;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Label;
import java.awt.Panel;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import com.mindbright.application.MindTermApp;
import com.mindbright.gui.AWTConvenience;
import com.mindbright.gui.BorderDialog;
import com.mindbright.gui.ProgressBar;
import com.mindbright.util.StringUtil;

/**
 * Copies a bunch of files, optionally recursively, to or from the
 * remote server while giving graphical feedback. This class is meant
 * to be created from the GUI-thread. The actual file transfers will
 * take place in a separate thread.
 */
public final class SSHFileTransferGUIThreadAWT extends Thread
    implements SSHFileTransferProgress, ActionListener {
    boolean               recursive, background, toRemote;
    SSHFileTransferDialogControl xferDialog;
    MindTermApp           client;

    String[]    localFileList;
    String[]    remoteFileList;

    Dialog          copyIndicator;
    ProgressBar     progress;
    SSHFileTransfer fileXfer;
    Thread          copyThread;
    Label           srcLbl, dstLbl, sizeLbl, nameLbl, speedLbl;
    Button          cancB;
    long            startTime;
    long            lastTime;
    long            totTransSize;
    long            fileTransSize;
    long            curFileSize;
    long            lastSize;
    int             fileCnt;
    boolean         doneCopying;

    volatile boolean userCancel;

    boolean isCopyThread;

    /**
     * Create the GUI and start copying the specified files.
     *
     * @param client a connected SSH client which will be used for
     * transport
     * @param fileXfer class resposible for transferring the files
     * @param localFileList List of local files
     * @param remoteFileList List of remote files
     * @param recursive true if the transfer should include the
     * contents of directories.
     * @param background run in the background
     * @param toRemote true if the files should be copied from the
     * local machine to the remote.
     * @param xferDialog dialog causing the file transfer
     */
    public SSHFileTransferGUIThreadAWT(MindTermApp client,
                                       SSHFileTransfer fileXfer,
                                       String[] localFileList,
                                       String[] remoteFileList,
                                       boolean recursive, boolean background,
                                       boolean toRemote,
                                       SSHFileTransferDialogControl xferDialog)
        throws Exception {
        setName("SSHFileTransferGUIThreadAWT1");

        isCopyThread = false;

        this.localFileList  = localFileList;
        this.remoteFileList = remoteFileList;

        if(!toRemote) {
            if(localFileList.length > 1) {
                throw new Exception("Ambiguous local target");
            }
        } else {
            if(remoteFileList.length > 1) {
                throw new Exception("Ambiguous remote target");
            }
        }

        this.client        = client;
        this.fileXfer      = fileXfer;
        this.recursive     = recursive;
        this.background    = background;
        this.toRemote      = toRemote;
        this.fileCnt       = 0;
        this.doneCopying   = false;
        this.startTime     = 0;
        this.lastTime      = 0;
        this.totTransSize  = 0;
        this.fileTransSize = 0;
        this.lastSize      = 0;
        this.xferDialog  = xferDialog;
        this.start();
    }

    public void run() {
        if(isCopyThread) {
            copyThreadMain();
        } else {
            createGUIAndCopyThread();
        }
    }

    private void copyThreadMain() {

        try {
            nameLbl.setText("...connected");
            fileXfer.setProgress(this);
            if(toRemote) {
                fileXfer.copyToRemote(localFileList, remoteFileList[0],
                                      recursive);
            } else {
                fileXfer.copyToLocal(localFileList[0], remoteFileList,
                                     recursive);
            }
            copyThread.setPriority(Thread.NORM_PRIORITY);
        } catch (Exception e) {
            if(!userCancel) {
                client.alert("File Transfer Error: " + e.getMessage());
            }
        } finally {
            try {
                Toolkit.getDefaultToolkit().beep();
            } catch (Throwable t) {
                /* What can we do... */
            }
        }

        nameLbl.setText("Copied " + fileCnt + " file" + (fileCnt != 1 ? "s" : "") + ".");
        sizeLbl.setText(StringUtil.nBytesToString(totTransSize, 4));
        doneCopying = true;
        if(fileXfer != null)
            fileXfer.abort();
        cancB.setLabel("Done");

        copyIndicator.validate();
        xferDialog.refresh();

        AWTConvenience.setKeyListenerOfChildren
            (copyIndicator,
             new AWTConvenience.OKCancelAdapter(cancB, cancB), null);
    }

    private void createGUIAndCopyThread() {
        String sourceFile = "localhost:" + unQuote(localFileList[0]);
        String destFile   = client.getHost() + ":" + unQuote(remoteFileList[0]);

        if(!toRemote) {
            String tmp;
            tmp        = sourceFile;
            sourceFile = destFile;
            destFile   = tmp;
        }

        copyIndicator = new BorderDialog(client.getParentFrame(),
                                         "MindTerm - File Transfer", false);

        Panel p = new Panel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();

        gbc.fill      = GridBagConstraints.NONE;
        gbc.insets    = new Insets(2,4,2,4);
        gbc.gridwidth = GridBagConstraints.RELATIVE;
        gbc.anchor    = GridBagConstraints.EAST;

        p.add(new Label("Source:", Label.RIGHT), gbc);
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.anchor    = GridBagConstraints.WEST;
        srcLbl = new Label(cutName(sourceFile, 48));
        p.add(srcLbl, gbc);

        gbc.gridwidth = GridBagConstraints.RELATIVE;
        gbc.anchor    = GridBagConstraints.EAST;
        p.add(new Label("Destination:", Label.RIGHT), gbc);
        dstLbl = new Label(cutName(destFile, 48));
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.anchor    = GridBagConstraints.WEST;
        p.add(dstLbl, gbc);

        gbc.gridwidth = GridBagConstraints.RELATIVE;
        gbc.anchor    = GridBagConstraints.EAST;
        p.add(new Label("Current:", Label.RIGHT), gbc);
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.anchor    = GridBagConstraints.WEST;
        nameLbl = new Label("connecting...");
        p.add(nameLbl, gbc);

        gbc.gridwidth = GridBagConstraints.RELATIVE;
        gbc.anchor    = GridBagConstraints.EAST;
        p.add(new Label("Size:", Label.RIGHT), gbc);
        gbc.gridwidth = GridBagConstraints.REMAINDER;
        gbc.anchor    = GridBagConstraints.WEST;
        sizeLbl = new Label("");
        p.add(sizeLbl, gbc);

        gbc.weightx = 1.0;
        gbc.fill    = GridBagConstraints.HORIZONTAL;
        gbc.anchor  = GridBagConstraints.WEST;
        gbc.insets  = new Insets(12, 12, 12, 12);

        progress = new ProgressBar(512, 160, 20);
        Panel p1 = new Panel(new BorderLayout());
        p1.add(progress, BorderLayout.CENTER);
        p.add(p1, gbc);

        gbc.weightx = 0.0;
        gbc.insets  = new Insets(4, 4, 4, 4);
        gbc.fill    = GridBagConstraints.NONE;
        gbc.anchor  = GridBagConstraints.CENTER;
        speedLbl = new Label("0.0 kB/sec");
        p.add(speedLbl, gbc);

        copyIndicator.add(p, BorderLayout.CENTER);

        cancB = new Button("Cancel");
        cancB.addActionListener(this);
        copyIndicator.add(AWTConvenience.newButtonPanel
                          (new Button[] { cancB }), BorderLayout.SOUTH);

        copyIndicator.setResizable(true);
        copyIndicator.pack();
        AWTConvenience.placeDialog(copyIndicator);

        isCopyThread = true;
        copyThread = new Thread(this, "SSHFileTransferGUIThreadAWT2");

        if(background) {
            copyThread.setPriority(Thread.MIN_PRIORITY);
        }

        copyThread.start();

        copyIndicator.setVisible(true);
    }

    public void startFile(String file, long size) {
        sizeLbl.setText(StringUtil.nBytesToString(size, 4));
        nameLbl.setText(unQuote(file));
        progress.setMax(size, true);
        lastTime = System.currentTimeMillis();
        if(startTime == 0)
            startTime = lastTime;
        curFileSize   = size;
        fileTransSize = 0;
        fileCnt++;
        copyIndicator.validate();
    }

    public void startDir(String file) {
        if(startTime == 0)
            startTime = System.currentTimeMillis();
        if(toRemote) {
            srcLbl.setText(cutName("localhost:" + unQuote(file), 48));
        } else {
            dstLbl.setText(cutName("localhost:" + unQuote(file), 48));
        }
        copyIndicator.validate();
    }

    public void endFile() {
        progress.setValue(curFileSize, true);
    }

    public void endDir() {}

    public void progress(long size) {
        totTransSize  += size;
        fileTransSize += size;
        long now = System.currentTimeMillis();
        // Update display if count has changed at least 1% or 1 second
        // has passed.
        if((curFileSize > 0)
           && (((((totTransSize - lastSize) * 100) / curFileSize) >= 1)
               || (now-lastTime > 1000))) {
            progress.setValue(fileTransSize, !background);
            long elapsed = (now - startTime);
            if(elapsed > 0) {
                long curSpeed = (long)((double)totTransSize /
                                       ((double)elapsed / 1000));
                elapsed = (now - lastTime);
                if(elapsed == 0) {
                    elapsed = 1;
                }
                curSpeed += (long)((double)(totTransSize - lastSize) /
                                   ((double)elapsed / 1000));
                curSpeed >>>= 1;
                speedLbl.setText(StringUtil.nBytesToString(curSpeed, 4) + "/sec");
            }
            lastSize = totTransSize;
            lastTime = now;
            copyIndicator.validate();
        }
    }

    private static String cutName(String name, int len) {
        if(name.length() > len) {
            len -= 3;
            String pre = name.substring(0, len / 2);
            String suf = name.substring(name.length() - (len / 2));
            name = pre + "..." + suf;
        }
        return name;
    }

    private static String unQuote(String str) {
        if(str.charAt(0) == '"') {
            str = str.substring(1, str.length() - 1);
        }
        return str;
    }

    public void actionPerformed(ActionEvent e) {
        if(!doneCopying) {
            userCancel = true;
            if(fileXfer != null)
                fileXfer.abort();
            Thread.yield();
            if(copyThread != null)
                copyThread.stop();
        }
        copyIndicator.dispose();
    }

}
