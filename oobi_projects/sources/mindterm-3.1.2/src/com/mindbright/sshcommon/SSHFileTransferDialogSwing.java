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
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.LayoutManager;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;

import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;

import com.isnetworks.ssh.FileBrowser;
import com.isnetworks.ssh.FileDisplayControl;
import com.isnetworks.ssh.FileDisplaySwing;
import com.isnetworks.ssh.FileListItem;
import com.mindbright.application.MindTermApp;
import com.mindbright.gui.SwingConvenience;

/**
 * Swing dialog for transferring files.
 */
public class SSHFileTransferDialogSwing extends JDialog 
    implements ActionListener, SSHFileTransferDialogControl {

    private class SpecialLayout implements LayoutManager {
        private int minWidth = 0;
        private int minHeight = 0;
        private int preferredWidth = 0;
        private int preferredHeight = 0;
        private boolean sizeUnknown = true;

        public void addLayoutComponent(String name, Component comp) {
        }
        public void removeLayoutComponent(Component comp) {
        }

        public Dimension minimumLayoutSize(Container parent) {
            return getSize(parent, minWidth, minHeight);
        }

        public Dimension preferredLayoutSize(Container parent) {
            return getSize(parent, preferredWidth, preferredHeight);
        }

        public void layoutContainer(Container parent) {
            Insets insets = parent.getInsets();
            Component c;
            int maxWidth = parent.getWidth() - (insets.left + insets.right);
            int maxHeight = parent.getHeight() - (insets.top + insets.bottom);
            int nComps = parent.getComponentCount();
            int x = insets.left;
            int y = insets.top;
            int middleWidth;
            int sideWidth;

            if (nComps < 3) {
                System.out.println("*** nComps < 3");
                return;
            }

            if (sizeUnknown) {
                setSizes(parent);
            }

            // Algorithm: After the middle components got their widths,
            // the rest of the space is devided between the first and
            // the last component. All components are assumed to be
            // visable.
            middleWidth = 0;
            for (int i = 1 ; i < nComps - 1 ; i++) {
                c = parent.getComponent(i);
                Dimension d = c.getPreferredSize();
                middleWidth += d.width;
            }
            sideWidth = (maxWidth - middleWidth) / 2;

            c = parent.getComponent(0);
            c.setBounds(x, y, sideWidth, maxHeight);
            x += sideWidth;
            for (int i = 1 ; i < nComps - 1 ; i++) {
                c = parent.getComponent(i);
                int width = c.getPreferredSize().width;
                c.setBounds(x, y, width, maxHeight);
                x += width;
            }
            c = parent.getComponent(nComps - 1);
            c.setBounds(x, y, sideWidth, maxHeight);
        }

        private Dimension getSize(Container parent, int width, int height) {
            Dimension dim = new Dimension(0, 0);

            setSizes(parent);
            Insets insets = parent.getInsets();
            dim.width = width + insets.left + insets.right;
            dim.height = height + insets.top + insets.bottom;
            sizeUnknown = false;

            return dim;
        }

        private void setSizes(Container parent) {
            int nComps = parent.getComponentCount();
            Dimension d = null;
            Dimension min = null;

            //Reset preferred/minimum width and height.
            preferredWidth = 0;
            preferredHeight = 0;
            minWidth = 0;
            minHeight = 0;

            for (int i = 0; i < nComps; i++) {
                Component c = parent.getComponent(i);
                d = c.getPreferredSize();
                min = c.getMinimumSize();

                preferredHeight = Math.max(preferredHeight, d.height);
                preferredWidth += d.width;

                minHeight = Math.max(minHeight, min.height);
                minWidth += min.height;
            }
        }
    }

    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if ("close".equals(cmd)) {
            dispose();
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
                
                if (toRemote) {
                    selectedItems = mLocalFileDisplay.getSelectedFiles();
                    fromDir = localCWD;
                    toDir   = remoteCWD;
                } else {
                    selectedItems = mRemoteFileDisplay.getSelectedFiles();
                    fromDir = remoteCWD;
                    toDir   = localCWD;
                }
                
                String[] files = new String[selectedItems.length];
                
                if (selectedItems.length == 0) {
                    throw new Exception("Please select file(s) to transfer");
                }
                
                for(int i = 0; i < selectedItems.length; i++ ) {
                    files[i] = fromDir + selectedItems[i].getName();
                }
                
                String[] files2 = new String[] {
                    toDir
                };
                
                if (!toRemote) {
                    String[] tmp = files2;
                    files2  = files;
                    files = tmp;
                }
                
                SSHFileTransfer fileXfer =
                    fileXferFactory.create(client, new File(localCWD));
                
                new SSHFileTransferGUIThreadSwing
                    (client, fileXfer, files, files2,
                     true, false, toRemote, this);
            } catch (Exception ee) {
                logError(ee);
            }
        }
    }

    private JButton       mUploadButton;
    private JButton       mDownloadButton;

    private JPanel        mFileDisplayPanel;
    private JPanel        mMainBottomSectionPanel;
    private JButton       closeButton;

    /** GUI for browsing file systems */
    private FileDisplaySwing   mLocalFileDisplay;
    private FileDisplaySwing   mRemoteFileDisplay;

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
    public SSHFileTransferDialogSwing(String title, MindTermApp client,
                                      SSHFileTransferFactory fileXferFactory) {
        super(client.getParentFrame(), title, false);

        this.client          = client;
        this.fileXferFactory = fileXferFactory;

        getContentPane().setLayout(new BorderLayout(10, 10));
        ((JComponent)getContentPane()).setBorder(new EmptyBorder(5, 5, 5, 5));

        getContentPane().add("South",  getMainBottomSectionPanel());
        getContentPane().add("Center", getFileDisplayPanel());
        addWindowListener(SwingConvenience.getWindowDisposer());
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
    private JPanel getMainBottomSectionPanel() {
        JPanel p = new JPanel(new FlowLayout(FlowLayout.RIGHT));
        closeButton = new JButton("Close");
        closeButton.setActionCommand("close");
        closeButton.addActionListener(this);
        p.add(closeButton);
        return p;
    }

    /** This is the upper section of the GUI, containing the
     *  local & remote file displays and the direction buttons
     */
    private JPanel getFileDisplayPanel() {
        JPanel p = new JPanel(new SpecialLayout());

        p.add((FileDisplaySwing)getLocalFileDisplay());

        mDownloadButton = new JButton("<--");
        mDownloadButton.setActionCommand("tolocal");
        mDownloadButton.addActionListener(this);
        
        mUploadButton  = new JButton("-->");
        mUploadButton.setActionCommand("toremote");
        mUploadButton.addActionListener(this);

        JPanel p1 = new JPanel(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.gridheight = 1;
        gbc.gridwidth  = GridBagConstraints.REMAINDER;
        gbc.insets     = new Insets(5, 5, 5, 5);
        p1.add(mDownloadButton, gbc);
        p1.add(mUploadButton, gbc);
        p.add(p1);

        p.add((FileDisplaySwing)getRemoteFileDisplay());

        return p;
    }

    /** An instance of FileDisplay for the local system */
    public FileDisplayControl getLocalFileDisplay() {
        if (mLocalFileDisplay == null)
            mLocalFileDisplay = newFileDisplay("Local System");
        return mLocalFileDisplay;
    }

    /** An instance of FileDisplay for the remote system */
    public FileDisplayControl getRemoteFileDisplay() {
        if (mRemoteFileDisplay == null)
            mRemoteFileDisplay = newFileDisplay("Remote System");
        return mRemoteFileDisplay;
    }

    private FileDisplaySwing newFileDisplay(String title) {
        return new FileDisplaySwing(client.getParentFrame(), title, this);
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
