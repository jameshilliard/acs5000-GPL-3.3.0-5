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
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Panel;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import com.isnetworks.ssh.FileBrowser;
import com.isnetworks.ssh.FileDisplayAWT;
import com.isnetworks.ssh.FileDisplayControl;
import com.isnetworks.ssh.FileListItem;
import com.mindbright.application.MindTermApp;
import com.mindbright.gui.AWTConvenience;

/**
 * AWT dialog for transferring files.
 */
public class SSHFileTransferDialogAWT extends Dialog 
    implements SSHFileTransferDialogControl, ActionListener {

    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if ("close".equals(cmd)) {
            setVisible(false);
            mRemoteFileBrowser.disconnect();
            
            if (chdirEventHandler != null) {
                chdirEventHandler.chdir(mLocalFileDisplay.getFileSystemLocationLabelText());
            }
        } else {
            boolean toRemote = "toremote".equals(cmd);
            try {
                FileListItem[] selectedItems;
                String         localCWD;
                String         remoteCWD;
                String         fromDir;
                String         toDir;
                
                localCWD =
                    mLocalFileDisplay.getFileSystemLocationLabelText();
                remoteCWD =
                    mRemoteFileDisplay.getFileSystemLocationLabelText();
                
                if(toRemote) {
                    selectedItems = mLocalFileDisplay.getSelectedFiles();
                    fromDir = localCWD;
                    toDir   = remoteCWD;
                } else {
                    selectedItems = mRemoteFileDisplay.getSelectedFiles();
                    fromDir = remoteCWD;
                    toDir   = localCWD;
                }
                
                String[] files = new String[selectedItems.length];
                
                if(selectedItems.length == 0) {
                    throw new Exception("Please select file(s) to transfer");
                }
                
                for(int i = 0; i < selectedItems.length; i++ ) {
                    files[i] = fromDir + selectedItems[i].getName();
                }
                
                String[] files2 = new String[] {
                    toDir
                };
                
                if(!toRemote) {
                    String[] tmp = files2;
                    files2  = files;
                    files = tmp;
                }
                
                SSHFileTransfer fileXfer =
                    fileXferFactory.create(client, new File(localCWD));
                
                new SSHFileTransferGUIThreadAWT
                    (client, fileXfer, files, files2,
                     true, false, toRemote, this);
            } catch (Exception ee) {
                logError(ee);
            }
        }
    }

    private Button        mUploadButton;
    private Button        mDownloadButton;

    private Panel         mFileDisplayPanel;
    private Panel         mMainBottomSectionPanel;
    private Button        closeButton;

    /** GUI for browsing file systems */
    private FileDisplayAWT   mLocalFileDisplay;
    private FileDisplayAWT   mRemoteFileDisplay;

    /** Back end for browsing file systems */
    private FileBrowser mRemoteFileBrowser;
    private FileBrowser mLocalFileBrowser;

    private MindTermApp            client;
    private SSHFileTransferFactory fileXferFactory;

    /** Constructor
     *  Overall, the GUI is composed of two parts:
     *    the bottom section (buttons, messages)
     *    the file-display section (two file displays & arrow buttons)
     */
    public SSHFileTransferDialogAWT(String title, MindTermApp client,
                                    SSHFileTransferFactory fileXferFactory) {
        super(client.getParentFrame(), title, false);
        this.setFont(client.getParentFrame().getFont());

        this.client          = client;
        this.fileXferFactory = fileXferFactory;

        setLayout(new BorderLayout());
        add(getFileDisplayPanel(), BorderLayout.CENTER);
        add(getMainBottomSectionPanel(), BorderLayout.SOUTH);
        addWindowListener(new AWTConvenience.CloseAdapter(closeButton));
        pack();
    }

    public void setLocalFileBrowser(FileBrowser localBrowser) {
        this.mLocalFileBrowser = localBrowser;
    }

    public void setRemoteFileBrowser(FileBrowser remoteBrowser) {
        this.mRemoteFileBrowser = remoteBrowser;
    }

    /**
     * The main bottom part of the GUI.  Now just contains the error text area
     */
    private Panel getMainBottomSectionPanel() {
        if (mMainBottomSectionPanel == null) {
            mMainBottomSectionPanel = new Panel(new FlowLayout(FlowLayout.RIGHT));
            closeButton = new Button("Close");
            closeButton.setActionCommand("close");
            closeButton.addActionListener(this);
            mMainBottomSectionPanel.add(closeButton);
        }
        return mMainBottomSectionPanel;
    }

    /** This is the upper section of the GUI, containing the
     *  local & remote file displays and the direction buttons
     *  It is a big-bad-evil GridBagLayout (tm)
     *  The general idea is that the file displays expand
     *  horizontally to fill the remaining space equally and
     *  the arrow buttons float in the center between the
     *  two file displays.
     */
    private Panel getFileDisplayPanel() {
        if (mFileDisplayPanel == null) {
            mFileDisplayPanel = new Panel(new GridBagLayout());

            GridBagConstraints gbc = new GridBagConstraints();

            gbc.gridx      = 0;
            gbc.gridy      = 0;
            gbc.gridwidth  = 1;
            gbc.gridheight = 2;
            gbc.fill       = GridBagConstraints.BOTH;
            gbc.anchor     = GridBagConstraints.CENTER;
            gbc.weightx    = 0.5;
            gbc.weighty    = 1.0;
            mFileDisplayPanel.add((FileDisplayAWT)getLocalFileDisplay(), gbc);

            gbc.gridx      = 2;
            gbc.gridy      = 0;
            gbc.gridwidth  = 1;
            gbc.gridheight = 2;
            gbc.fill       = GridBagConstraints.BOTH;
            gbc.anchor     = GridBagConstraints.CENTER;
            gbc.weightx    = 0.5;
            gbc.weighty    = 1.0;
            mFileDisplayPanel.add((FileDisplayAWT)getRemoteFileDisplay(), gbc);

            gbc.gridx      = 1;
            gbc.gridy      = 0;
            gbc.gridwidth  = 1;
            gbc.gridheight = 1;
            gbc.fill       = GridBagConstraints.NONE;
            gbc.anchor     = GridBagConstraints.SOUTH;
            gbc.weightx    = 0.0;
            gbc.weighty    = 0.5;
            gbc.insets     = new Insets(0, 4, 2, 4);
            mFileDisplayPanel.add(getDownloadButton(), gbc);

            gbc.gridx      = 1;
            gbc.gridy      = 1;
            gbc.gridwidth  = 1;
            gbc.gridheight = 1;
            gbc.fill       = GridBagConstraints.NONE;
            gbc.anchor     = GridBagConstraints.NORTH;
            gbc.weightx    = 0.0;
            gbc.weighty    = 0.5;
            gbc.insets     = new Insets(2, 4, 0, 4);
            mFileDisplayPanel.add(getUploadButton(), gbc);
        }
        return mFileDisplayPanel;
    }

    /** An instance of FileDisplay for the local system */
    public FileDisplayControl getLocalFileDisplay() {
        if(mLocalFileDisplay == null) {
            mLocalFileDisplay = newFileDisplay("Local System");
        }
        return mLocalFileDisplay;
    }

    /** An instance of FileDisplay for the remote system */
    public FileDisplayControl getRemoteFileDisplay() {
        if(mRemoteFileDisplay == null) {
            mRemoteFileDisplay = newFileDisplay("Remote System");
        }
        return mRemoteFileDisplay;
    }

    private FileDisplayAWT newFileDisplay(String title) {
        return new FileDisplayAWT(client.getParentFrame(), title, this);
    }

    /** A direction button pointing left */
    private Button getDownloadButton() {
        if (mDownloadButton == null) {
            mDownloadButton = new Button("<--");
            mDownloadButton.setActionCommand("tolocal");
            mDownloadButton.addActionListener(this);
        }
        return mDownloadButton;
    }

    /** A direction button pointing right */
    private Button getUploadButton() {
        if (mUploadButton == null) {
            mUploadButton = new Button("-->");
            mUploadButton.setActionCommand("toremote");
            mUploadButton.addActionListener(this);
        }
        return mUploadButton;
    }

    public void refresh() {
        try {
            mRemoteFileBrowser.refresh();
            mLocalFileBrowser.refresh();
        } catch (Exception e) {
            logError(e);
        }
    }

    public void doShow() {
        setVisible(true);
    }
    
    /**
     * Initialize the connection to the remote system and
     * start in the SSH home directory on the local system
     */
    public void setVisible(boolean vis) {
        if (!vis) {
            super.setVisible(false);
            return;
        }

        Dimension sDim = Toolkit.getDefaultToolkit().getScreenSize();

        int width  = (sDim.width > 600 ? 600 : sDim.width);
        int height = (sDim.height > 400 ? 400 : sDim.height);

        setSize(width, height);

        try {
            mRemoteFileBrowser.initialize();
            mLocalFileBrowser.initialize();
        } catch (Exception e) {
            logError(e);
        }
        super.setVisible(true);
    }

    /**
     * An exception happened, so show the user the message in the text area
     */
    public void logError(Exception e) {
        client.alert(e.getMessage());
    }


    private SSHChdirEventHandler chdirEventHandler = null;

    public void setLocalChdirCallback(SSHChdirEventHandler ceh) {
        chdirEventHandler = ceh;
    }
}
