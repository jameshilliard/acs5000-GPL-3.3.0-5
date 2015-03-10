/*
 * ====================================================================
 *
 * License for ISNetworks' MindTerm SCP modifications
 *
 * Copyright (c) 2001 ISNetworks, LLC.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution, if
 *    any, must include an acknowlegement that the software contains
 *    code based on contributions made by ISNetworks, and include
 *    a link to http://www.isnetworks.com/.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 */

/**
* AWT Panel that represents a file system.  Has buttons for basic
* file administration operations and a list of the files in a
* given directory.
* 
* This code is based on a LayoutManager tutorial on Sun's Java web site.
* http://developer.java.sun.com/developer/onlineTraining/GUI/AWTLayoutMgr/shortcourse.html
*/
package com.isnetworks.ssh;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Panel;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.Vector;

import com.mindbright.ssh.SSHMiscDialogs;
import com.mindbright.sshcommon.SSHFileTransferDialogControl;
import com.mindbright.util.ArraySort;
import com.mindbright.util.StringUtil;

/** This class represents a small pane which will list the files present
 *  on a given platform.  This pane was made into its own class to allow
 *  easy reuse as both the local and remote file displays
 *  
 *  This GUI is built up using "lazy instantiation" via method calls
 *  for each part of the component.
 */
public class FileDisplayAWT extends Panel implements 
   ActionListener, ItemListener, FileDisplayControl {

    public void actionPerformed(ActionEvent e) {
        try {
            String cmd = e.getActionCommand();
            if ("chdir".equals(cmd)) {
                String directoryName =
                    SSHMiscDialogs.textInput("Change directory", "Directory", 
                                             mOwnerFrame, getFileSystemLocationLabelText() );
                if ( directoryName != null ) {
                    mBrowser.changeDirectory( directoryName );
                }
            } else if ("mkdir".equals(cmd)) {
                String directoryName =
                    SSHMiscDialogs.textInput("Make directory relative to current path", 
                                             "Directory name", mOwnerFrame );
                if ( directoryName != null ) {
                    mBrowser.makeDirectory( directoryName );
                }
            } else if ("rename".equals(cmd)) {
                FileListItem mFileListItem = getSelectedFile();
                String newName =
                    SSHMiscDialogs.textInput("Rename file",
                                             "New file name", mOwnerFrame,
                                             mFileListItem.getName());
                if (newName != null) {
                    mBrowser.rename(mFileListItem, newName);
                }
            } else if ("delete".equals(cmd)) {
                mBrowser.delete(getSelectedFiles());
            } else if ("refresh".equals(cmd)) {
                // done below
            } else {
                FileListItem item = mFileList.getFileListItem(cmd);
                if (item != null) {
                    mBrowser.fileDoubleClicked(item);
                }
            }
            mBrowser.refresh();
        } catch (Exception ex) {
            mFileXferDialog.logError(ex);
        }
    }

    public void itemStateChanged(ItemEvent e) {
        enableButtons();
    }

    private FileBrowser mBrowser;

    private Button       mChgDirButton;
    private Button       mDeleteButton;
    private Panel        mFileButtonsInnerPanel;
    private Panel        mFileButtonsPanel;
    private Panel        mFileHeaderPanel;
    private FileListAWT  mFileList;
    private Label        mMachineDescriptionLabel;
    private String       mMachineDescriptionText;
    private Label        mFileSystemLocationLabel;
    private Button       mMkDirButton;
    private Button       mRefreshButton;
    private Button       mRenameButton;

    /**
     * Frame to own dialog boxes
     */
    private Frame mOwnerFrame;

    /**
     * Reference to SCP main dialog box to send error messages to
     */
    private SSHFileTransferDialogControl mFileXferDialog;

    /** Constructor
     *  This defines the overall GUI for this component
     *  It's a BorderLayout with a header, a set of buttons & a list
     */
    public FileDisplayAWT(Frame ownerFrame, String name,
                          SSHFileTransferDialogControl fileXferDialog) {
        super(new BorderLayout(2, 10));
        mOwnerFrame     = ownerFrame;
        mFileXferDialog = fileXferDialog;

        mMachineDescriptionLabel = new Label( name );
        mMachineDescriptionText  = name;

        add(getFileHeaderPanel(), BorderLayout.NORTH);
        add(getFileList(), BorderLayout.CENTER);
        add(getFileButtonsPanel(), BorderLayout.SOUTH);
    }

    /** The header panel -- contains labels for Remote/Local and the current directory */
    private Panel getFileHeaderPanel() {
        if (mFileHeaderPanel == null) {
            mFileHeaderPanel = new Panel(new GridLayout(2, 1));
            mFileHeaderPanel.add(getMachineDescriptionLabel());
            mFileHeaderPanel.add(getFileSystemLocationLabel());
        }
        return mFileHeaderPanel;
    }

    /** The label to show which system this file display refers to */
    private Label getMachineDescriptionLabel() {
        // Created in constructor
        return mMachineDescriptionLabel;
    }

    /** The label to show which directory this display refers to */
    private Label getFileSystemLocationLabel() {
        if (mFileSystemLocationLabel == null) {
            mFileSystemLocationLabel = new Label("");
        }
        return mFileSystemLocationLabel;
    }

    /** This is merely a wrapper to bind the set of buttons to their
     *   preferred height
     */
    private Panel getFileButtonsPanel() {
        if (mFileButtonsPanel == null) {
            mFileButtonsPanel = new Panel(new BorderLayout());
            mFileButtonsPanel.add(getFileButtonsInnerPanel(), BorderLayout.NORTH);
        }
        return mFileButtonsPanel;
    }

    /** The panel containing the buttons for the file list */
    private Panel getFileButtonsInnerPanel() {
        if (mFileButtonsInnerPanel == null) {
            mFileButtonsInnerPanel = new Panel(new GridLayout(1,5));
            mFileButtonsInnerPanel.add(getChgDirButton());
            mFileButtonsInnerPanel.add(getMkDirButton());
            mFileButtonsInnerPanel.add(getRenameButton());
            mFileButtonsInnerPanel.add(getDeleteButton());
            mFileButtonsInnerPanel.add(getRefreshButton());
        }
        return mFileButtonsInnerPanel;
    }


    //----- Buttons -----
    private Button getChgDirButton() {
        if (mChgDirButton == null) {
            mChgDirButton = new Button("ChgDir");
            mChgDirButton.setActionCommand("chdir");
            mChgDirButton.addActionListener(this);
        }
        return mChgDirButton;
    }

    private Button getMkDirButton() {
        if (mMkDirButton == null) {
            mMkDirButton = new Button("MkDir");
            mMkDirButton.setActionCommand("mkdir");
            mMkDirButton.addActionListener(this);
        }
        return mMkDirButton;
    }

    private Button getRenameButton() {
        if (mRenameButton == null) {
            mRenameButton = new Button("Rename");
            mRenameButton.setActionCommand("rename");
            mRenameButton.addActionListener(this);
        }
        return mRenameButton;
    }

    private Button getDeleteButton() {
        if (mDeleteButton == null) {
            mDeleteButton = new Button("Delete");
            mDeleteButton.setActionCommand("delete");
            mDeleteButton.addActionListener(this);
        }
        return mDeleteButton;
    }

    private Button getRefreshButton() {
        if (mRefreshButton == null) {
            mRefreshButton = new Button("Refresh");
            mRefreshButton.setActionCommand("refresh");
            mRefreshButton.addActionListener(this);
        }
        return mRefreshButton;
    }

    /** The list of files */
    private FileListAWT getFileList() {
        if (mFileList == null) {
            mFileList = new FileListAWT();
            mFileList.setMultipleMode(true);
            mFileList.addActionListener(this);
            mFileList.addItemListener(this);
        }
        return mFileList;
    }

    private void enableButtons() {
        mRenameButton.setEnabled( mFileList.getSelectionCount() == 1 );
        mDeleteButton.setEnabled( mFileList.getSelectionCount() > 0 );
    }

    //----- public methods that make the file system label a property -----

    public String getFileSystemLocationLabelText() {
        return getFileSystemLocationLabel().getText();
    }

    public void setFileSystemLocationLabelText(String arg1) {
        getFileSystemLocationLabel().setText(arg1);
    }

    public void setFileList(Vector dirs, Vector files, String directory,
                            String separator) {
        if(!directory.endsWith(separator)) {
            directory += separator;
        }
        setFileSystemLocationLabelText(directory);

        int i, dl = dirs.size(), fl = files.size();
        long totSize = 0;
        FileListItem[] list = new FileListItem[dl + fl];
        for(i = 0; i < dl; i++) {
            list[i] = (FileListItem)dirs.elementAt(i);
        }
        for(i = 0; i < fl; i++) {
            FileListItem item = (FileListItem)files.elementAt(i);
            list[i + dl] = item;
            totSize += item.getSize();
        }

        String sizeStr = "";
        if(totSize > 0) {
            sizeStr = " (" + StringUtil.nBytesToString(totSize, 4) + ")";
        }

        getMachineDescriptionLabel().setText(mMachineDescriptionText + " : " +
                                             fl + " file" + (fl > 1 ? "s" : "")
                                             + sizeStr);

        ArraySort.sort(list, 0, dl);
        ArraySort.sort(list, dl, dl + fl);

        dirs.setSize(0);
        files.setSize(0);

        mFileList.setListItems(list);
        mFileList.invalidate();

        enableButtons();
        mFileButtonsInnerPanel.invalidate();
    }

    public void setFileBrowser( FileBrowser browser ) {
        mBrowser = browser;
    }

    public FileListItem getSelectedFile() {
        return mFileList.getSelectedFileListItem();
    }

    public FileListItem[] getSelectedFiles() {
        return mFileList.getSelectedFileListItems();
    }

}
