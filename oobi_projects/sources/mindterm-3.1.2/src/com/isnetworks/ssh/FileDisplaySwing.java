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
* Swing Panel that represents a file system.  Has buttons for basic
* file administration operations and a list of the files in a
* given directory.
* 
* This code is based on a LayoutManager tutorial on Sun's Java web site.
* http://developer.java.sun.com/developer/onlineTraining/GUI/AWTLayoutMgr/shortcourse.html
*/
package com.isnetworks.ssh;

import java.awt.BorderLayout;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.ScrollPaneConstants;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;

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
public class FileDisplaySwing extends JPanel implements 
   ActionListener, ListSelectionListener, MouseListener, FileDisplayControl {

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
            } 
            mBrowser.refresh();
        } catch (Exception ex) {
            mFileXferDialog.logError(ex);
        }
    }

    public void valueChanged(ListSelectionEvent e) {
        enableButtons();
        mFileList.ensureIndexIsVisible(mFileList.getLeadSelectionIndex());
    }

    public void mouseClicked(MouseEvent e)  {
        if (e.getClickCount() >= 2) {
            int idx = mFileList.locationToIndex(e.getPoint());    
            mFileList.ensureIndexIsVisible(idx);
            FileListItem item = mFileList.getFileListItem
                ((String)mFileList.getModel().getElementAt(idx));
            if (item != null) {
                try {
                    mBrowser.fileDoubleClicked(item);
                    mBrowser.refresh();
                } catch (Exception ex) {
                    mFileXferDialog.logError(ex);
                }
            }
        }
    }
    public void mouseEntered(MouseEvent e)  {}
    public void mouseExited(MouseEvent e)   {}
    public void mousePressed(MouseEvent e)  {}
    public void mouseReleased(MouseEvent e) {}

    private FileBrowser mBrowser;

    private JButton     mChgDirButton;
    private JButton     mDeleteButton;
    private JPanel      mFileHeaderPanel;
    private FileListSwing mFileList;
    private JLabel      mMachineDescriptionLabel;
    private String      mMachineDescriptionText;
    private JLabel      mFileSystemLocationLabel;
    private JButton     mMkDirButton;
    private JButton     mRefreshButton;
    private JButton     mRenameButton;

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
    public FileDisplaySwing(Frame ownerFrame, String name,
                            SSHFileTransferDialogControl fileXferDialog) {
        super(new BorderLayout());

        mOwnerFrame     = ownerFrame;
        mFileXferDialog = fileXferDialog;

        mMachineDescriptionLabel = new JLabel( name );
        mMachineDescriptionText  = name;

        add("North",  getFileHeaderPanel());
        JComponent c = getFileList();
        JScrollPane sp = new JScrollPane
            (c, ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
             ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);        
        add("Center", sp);
        add("South",  getFileButtonsPanel());
    }

    /** The header panel -- contains labels for Remote/Local and the current directory */
    private JPanel getFileHeaderPanel() {
        if (mFileHeaderPanel == null) {
            mFileHeaderPanel = new JPanel(new GridLayout(2, 1));
            mFileHeaderPanel.add(getMachineDescriptionLabel());
            mFileHeaderPanel.add(getFileSystemLocationLabel());
        }
        return mFileHeaderPanel;
    }

    /** The label to show which system this file display refers to */
    private JLabel getMachineDescriptionLabel() {
        // Created in constructor
        return mMachineDescriptionLabel;
    }

    /** The label to show which directory this display refers to */
    private JLabel getFileSystemLocationLabel() {
        if (mFileSystemLocationLabel == null) {
            mFileSystemLocationLabel = new JLabel("");
        }
        return mFileSystemLocationLabel;
    }

    /** The panel containing the buttons for the file list */
    private JPanel getFileButtonsPanel() {
        JPanel p = new JPanel(new GridLayout(2, 3, 5, 5));
        p.add(getChgDirButton());
        p.add(getMkDirButton());
        p.add(getRenameButton());
        p.add(getDeleteButton());
        p.add(getRefreshButton());

        JPanel p1 = new JPanel(new BorderLayout());
        p1.add(new JLabel(" "), BorderLayout.NORTH);
        p1.add(p, BorderLayout.WEST);
        p1.add(new JLabel(""), BorderLayout.CENTER);
        return p1;
    }


    //----- Buttons -----
    private JButton makeButton(String label, String acmd) {
        JButton b = new JButton(label);
        b.setActionCommand(acmd);
        b.addActionListener(this);
        b.setMargin(new Insets(2,2,2,2));
        return b;
    }

    private JButton getChgDirButton() {
        if (mChgDirButton == null)
            mChgDirButton = makeButton("ChDir", "chdir");
        return mChgDirButton;
    }

    private JButton getMkDirButton() {
        if (mMkDirButton == null)
            mMkDirButton = makeButton("MkDir", "mkdir");
        return mMkDirButton;
    }

    private JButton getRenameButton() {
        if (mRenameButton == null)
            mRenameButton = makeButton("Rename", "rename");
        return mRenameButton;
    }

    private JButton getDeleteButton() {
        if (mDeleteButton == null)
            mDeleteButton = makeButton("Delete", "delete");
        return mDeleteButton;
    }

    private JButton getRefreshButton() {
        if (mRefreshButton == null)
            mRefreshButton = makeButton("Refresh", "refresh");
        return mRefreshButton;
    }

    /** The list of files */
    private FileListSwing getFileList() {
        if (mFileList == null) {
            mFileList = new FileListSwing();
            mFileList.addListSelectionListener(this);
            mFileList.addMouseListener(this);
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

        enableButtons();
    }

    public void setFileBrowser(FileBrowser browser) {
        mBrowser = browser;
    }

    public FileListItem getSelectedFile() {
        return mFileList.getSelectedFileListItem();
    }

    public FileListItem[] getSelectedFiles() {
        return mFileList.getSelectedFileListItems();
    }

}
