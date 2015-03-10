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

package com.mindbright.terminal;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Canvas;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Panel;
import java.awt.Rectangle;
import java.awt.Scrollbar;
import java.awt.Toolkit;
import java.awt.event.ActionListener;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.util.Hashtable;

public class DisplayAWT extends Canvas
    implements DisplayView, AdjustmentListener, MouseListener,
    MouseMotionListener, ComponentListener, FocusListener {

    boolean pendingShow = true;
    boolean visTopChangeAllowed = true;

    final static int REPAINT_SLEEP = 70; // ms

    final static boolean DEBUG         = false;

    final static public int MIN_ROWS = 2;
    final static public int MIN_COLS = 8;
    final static public int MAX_COLS = 512;
    final static public int MAX_ROWS = 512;

    private Scrollbar scrollbar;
    private boolean   haveScrollbar;
    private volatile boolean updateScrollbar = false;

    private Panel     myPanel;
    private Frame     ownerFrame;

    private boolean logoDraw;
    private Image   logoImg;
    private int     logoX;
    private int     logoY;
    private int     logoW;
    private int     logoH;
    private int     centerLogoX;
    private int     centerLogoY;

    private boolean isDirty = false;
    private int     dirtyTop;
    private int     dirtyBottom;
    private int     dirtyLeft;
    private int     dirtyRight;

    private int rows; // Number of displayed rows
    private int cols; // Number of displayed columns
    private int vpixels;
    private int hpixels;
    private int borderWidth  = 2;
    private int borderHeight = 2;
    private int xPos; // Position of display windows on screen
    private int yPos; // Position of display windows on screen

    private int charWidth;
    private int charHeight;
    private int baselineIndex;
    private int lineSpaceDelta;

    private boolean cursorHollow = false;
    private boolean hasCursor = false;
    private int curRow;
    private int curCol;

    private int     lastKeyKludge     = 0;
    private boolean lastKeyWasPressed = false;

    private Color origBgColor;
    private Color origFgColor;
    private Color cursorColor;

    private int visTop;         // The buffer row number of the top displayed row
    private boolean visTopChangePending = false; // Should vis top change on resize
    private int visTopChange = 0; // New visTop value

    private Image     memImage;
    private Graphics  memGraphics;
    private Dimension memImageSize;

    private Font plainFont;
    private Font boldFont;

    private DisplayModel model;
    private DisplayController controller;

    private boolean hasSelection = false;
    private int selectionTopRow;
    private int selectionTopCol;
    private int selectionBottomRow;
    private int selectionBottomCol;


    /** This class collapses repaint requests.
     * This thread class sleeps for a couple of milli-sec, wakes up to see if
     * repainting is needed (and repaints if that is the case) and then
     * go to sleep again. The idea is that a Canvas instance is a
     * heavy-weight object and a call to it's repaint method will be
     * executed directly, and not put on an event queue.
     */
    private class Repainter extends Thread {
        protected int sleepTime;
        protected boolean repaintRequested;
        protected boolean hasSlept;

        Repainter(int sleepTime) {
            super("DisplayAWT.repainter");

            this.sleepTime = sleepTime;
            repaintRequested = false;
            hasSlept = false;

            synchronized (this) {
                start();
                try {
                    this.wait();
                } catch (InterruptedException e) {}
            }
        }

        public void run() {
            synchronized (this) {
                this.notify();
            }

            while (ownerFrame != null) {
                try {
                    synchronized (this) {
                        this.wait(sleepTime);
                        if (repaintRequested) {
                            doRepaint();
                            repaintRequested = false;
                            hasSlept = false;
                        } else {
                            hasSlept = true;
                        }
                    }
                } catch (InterruptedException e) {
//                    System.out.println("Repainter is interrupted!");
                }
            }
        }

        synchronized void repaint(boolean force) {
            repaintRequested = true;
            if (force || hasSlept) {
                synchronized (this) {
                    this.notify();
                }
            }
        }
    }

    private Repainter repainter;


    public DisplayAWT(Frame ownerFrame) {
        super();

        scrollbar     = null;
        haveScrollbar = false;
        visTop        = 0;

        isDirty = false;

        this.ownerFrame  = ownerFrame;

        // !!! We don't receive the proper component-events on the Canvas IMHO?
        //
        ownerFrame.addComponentListener(this);

        // !!! Ok, in spite of all our efforts here, we seem to need this
        // for certain situations, I give up once again...
        //
        // ownerFrame.addKeyListener(this);

        addComponentListener(this);
        addFocusListener(this);
        addMouseMotionListener(this);
        addMouseListener(this);

        try {
            /*
             * This is put in separate file to be able to use without reflect
             * package since otherwise we can't link this class in some cases.
             */
            new FocusKludge(ownerFrame);
            new FocusKludge(this);
        } catch (Throwable t) {
            // only needed/available in jdk1.4
        }

        repainter = new Repainter(REPAINT_SLEEP);
    }

    public void setModel(DisplayModel model) {
        this.model = model;
    }
    public DisplayModel getModel() {
        return model;
    }
    public void setController(DisplayController controller) {
        this.controller = controller;
    }

    public void setKeyListener(KeyListener keyListener) {
        addKeyListener(keyListener);
    }

    public void delKeyListener(KeyListener keyListener) {
        removeKeyListener(keyListener);
    }


    private boolean isInsideSelection(int row, int col) {
        if (!hasSelection) {
            return false;
        }
        if (row < selectionTopRow || row > selectionBottomRow) {
            return false;
        }
        if (row == selectionTopRow && col < selectionTopCol) {
            return false;
        }
        if (row == selectionBottomRow && col > selectionBottomCol) {
            return false;
        }
        return true;
    }

    public void setLogo(Image logoImg, int x, int y, int w, int h) {
        this.logoImg = logoImg;
        this.logoX   = x;
        this.logoY   = y;
        this.logoW   = w;
        this.logoH   = h;
    }

    public Image getLogo() {
        return logoImg;
    }
    public boolean showLogo() {
        logoDraw = (logoImg != null);
        centerLogoX = -1;
        centerLogoY = -1;

        try {
            makeAllDirty();
            repaint();
        } catch (Throwable t) {
            // Sun JRE 1.4.[01] on 24bpp Linux displays crashes when showing
            // logo. Ignore.
            logoDraw = false;
            System.out.println("Logo crash");
        }

        return logoDraw;
    }

    public void hideLogo() {
        logoDraw = false;
        try {
            makeAllDirty();
            repaint();
        } catch (Throwable t) {
            // Sun JRE 1.4.[01] on 24bpp Linux displays crashes when showing
            // logo. Ignore.
        }
    }

    public static Color getTermRGBColor(String value)
    throws NumberFormatException {
        int r, g, b, c1, c2;
        Color c;
        c1 = value.indexOf(',');
        c2 = value.lastIndexOf(',');
        if(c1 == -1 || c2 == -1)
            throw new NumberFormatException();
        r = Integer.parseInt(value.substring(0, c1).trim());
        g = Integer.parseInt(value.substring(c1 + 1, c2).trim());
        b = Integer.parseInt(value.substring(c2 + 1).trim());
        c = new Color(r, g, b);
        return c;
    }

    public static Color getTermColor(String name)
    throws IllegalArgumentException {
        int i;
        for(i = 0; i < termColors.length; i++) {
            if(termColorNames[i].equalsIgnoreCase(name))
                break;
        }
        if(i == termColors.length)
            throw new IllegalArgumentException("Unknown color: " + name);
        return termColors[i];
    }

    public void setFont(String name, int size) {
        plainFont = new Font(name, Font.PLAIN, size);
        boldFont  = new Font(name, Font.BOLD, size);

        super.setFont(plainFont);
        calculateCharSize();

        if(isShowing()) {
            setGeometry(rows, cols);
        }
    }

    public void setLineSpaceDelta(int delta) {
        lineSpaceDelta = delta;
    }

    public void setFont(Font font) {
        setFont(font.getName(), font.getSize());
    }

    public void setVisTopChangeAllowed(boolean set) {
        visTopChangeAllowed = set;
    }
    public void setVisTopDelta(int delta) {
        setVisTopDelta(delta, visTopChangeAllowed);
    }
    public void setVisTopDelta(int delta, boolean changeAllowed) {
        setVisTop(visTop + delta, changeAllowed);
    }
    public void setVisTop(int visTop) {
        setVisTop(visTop, visTopChangeAllowed);
    }
    public void setVisTop(int visTop, boolean changeAllowed) {
        if (model == null) {
            return;
        }
        visTopChangePending = false;
        visTop = fenceVisTop(visTop);
        if (this.visTop != visTop) {
            if (changeAllowed) {
                this.visTop = visTop;
                repaint();
                updateScrollbarValues();
            }
        }
    }
    public void setPendingVisTopChange(int visTop) {
        visTopChangePending = true;
        visTopChange = visTop;
    }
    private int fenceVisTop(int visTop) {
        int min = 0;
        int max = model.getBufferRows() - rows;
        if (visTop < min) {
            visTop = min;
        }
        if (visTop > max) {
            visTop = max;
        }
        return visTop;
    }

    public void updateScrollbarValues() {
        if (model == null)
            return;
        if (haveScrollbar)
            updateScrollbar = true;
    }
    
    public Container getPanelWithScrollbar(String scrollPos) {
        if(myPanel != null)
            return myPanel;

        scrollbar = new Scrollbar(Scrollbar.VERTICAL);
        updateScrollbarValues();
        scrollbar.addAdjustmentListener(this);

        myPanel = new Panel(new BorderLayout());
        myPanel.add(this, BorderLayout.CENTER);
        if(scrollPos.equals("left")) {
            myPanel.add(scrollbar, BorderLayout.WEST);
            haveScrollbar = true;
        } else if(scrollPos.equals("right")) {
            myPanel.add(scrollbar, BorderLayout.EAST);
            haveScrollbar = true;
        } else {
            haveScrollbar = false; // No scrollbar
        }

        return myPanel;
    }

    public void moveScrollbar(String scrollPos) {
        if(myPanel == null) {
            return;
        }
        if(scrollPos.equals("left") || scrollPos.equals("right")) {
            if(scrollbar != null) {
                myPanel.remove(scrollbar);
                if(scrollPos.equals("right")) {
                    myPanel.add(scrollbar, BorderLayout.EAST);
                } else {
                    myPanel.add(scrollbar, BorderLayout.WEST);
                }
                ownerFrame.pack();
                requestFocus();
                haveScrollbar = true;
            }
        } else if(scrollPos.equals("none")) {
            if(scrollbar != null) {
                myPanel.remove(scrollbar);
            }
            ownerFrame.pack();
            requestFocus();
            haveScrollbar = false;
        }
    }

    private synchronized final void makeAllDirty() {
        // Reset dirty area since it can be larger than display after
        // a resize
        dirtyTop = 0;
        dirtyLeft = 0;
        dirtyBottom = rows;
        dirtyRight = cols;
    }

    private final void makeCursorDirty() {
        makeAreaDirty(curRow, curCol, curRow+1, curCol+1);
    }

    private final void makeSelectionDirty() {
        int dirtyTop, dirtyLeft, dirtyBottom, dirtyRight;
        dirtyTop = selectionTopRow;
        dirtyBottom = selectionBottomRow;
        if (dirtyTop != dirtyBottom) {
            dirtyLeft = 0;
            dirtyRight = cols;
        } else {
            if (selectionTopCol < selectionBottomCol) {
                dirtyLeft = selectionTopCol;
                dirtyRight = selectionBottomCol;
            } else {
                dirtyRight = selectionTopCol;
                dirtyLeft = selectionBottomCol;
            }
        }
        makeAreaDirty(dirtyTop, dirtyLeft, dirtyBottom+1, dirtyRight+1);
    }

    public void updateDirtyArea(int top, int left, int bottom, int right) {
        makeAreaDirty(top, left, bottom, right);
        //System.out.println("Dirty: ("+top+","+left+","+bottom+","+right+")");
    }
    // input is buffer coordinates, dirty is screen coordinates
    private synchronized final void makeAreaDirty(int top, int left,
            int bottom, int right) {
        if (bottom < visTop || top > (visTop + rows)) {
            // Dirt outside visible area, ignore
            return;
        }

        // Translate to screen coordinates
        top = top - visTop;
        bottom = bottom - visTop;

        if (!isDirty) {
            //System.out.print("No dirt. ");
            dirtyTop = top;
            dirtyBottom = bottom;
            dirtyLeft = left;
            dirtyRight = right;
            isDirty = true;
        } else {
            //System.out.print("Old dirt=("+dirtyTop+","+dirtyLeft+")("+dirtyBottom+","+dirtyRight+"). ");
            // Grow dirty area to include all dirty spots on screen
            if(top < dirtyTop) {
                dirtyTop = top;
            }
            if(bottom > dirtyBottom) {
                dirtyBottom = bottom;
            }
            if(left < dirtyLeft) {
                dirtyLeft = left;
            }
            if(right > dirtyRight) {
                dirtyRight = right;
            }
            if (dirtyTop == dirtyBottom) {
                dirtyBottom++;
            }
            if (dirtyLeft == dirtyRight) {
                dirtyRight++;
            }
        }
        // Make sure that values are sane
        dirtyTop = (dirtyTop < 0) ? 0 : dirtyTop;
        dirtyBottom = (dirtyBottom > rows) ? rows : dirtyBottom;
        dirtyLeft  = (dirtyLeft < 0) ? 0 : dirtyLeft;
        dirtyRight = (dirtyRight > cols) ? cols : dirtyRight;

        // Make sure that the dirty area is a box, so if the new
        // dirt spans many lines, the entire screen width should
        // be repainted.
        if (dirtyBottom - dirtyTop > 1) {
            dirtyLeft = 0;
            dirtyRight = cols;
        }
        //System.out.println("New dirt=("+dirtyTop+","+dirtyLeft+")("+dirtyBottom+","+dirtyRight+"). ");
    }


    //
    // FocusListener, AdjustmentListener, MouseListener, MouseMotionListener, ComponentListener
    //
    public void focusGained(FocusEvent e) {
        setCursor(Cursor.getPredefinedCursor(Cursor.TEXT_CURSOR));
        cursorHollow = false;
        makeCursorDirty();
        repaint(true);
    }
    public void focusLost(FocusEvent e) {
        setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        cursorHollow = true;
        makeCursorDirty();
        repaint(true);
    }

    public boolean isFocusTraversable() {
        return true;
    }

    // !!! Since the realization of the window is very different on different
    // platforms (w.r.t. generated events etc.) we don't listen to
    // componentResized event until window is shown, in that instance we also
    // do the pending setGeometry.
    //
    public void componentMoved(ComponentEvent e) {
        // !!! TODO: Do we want to save absolute positions???
    }
    public void emulateComponentShown() {
        componentShown(new ComponentEvent(ownerFrame, 0));
    }
    public synchronized void componentShown(ComponentEvent e) {
        if(e.getComponent() == ownerFrame && pendingShow) {
            // !!! Ad-hoc wait to let AWT thread in, seems to prevent it from
            // sending a componentResized when getSize() returns bad value
            // AFTER we have done setSize(), this only occurs on Linux (as far
            // as I have seen) due to thread-scheduler lag ?!?
            try {
                this.wait(100);
            } catch(InterruptedException ee) {}
            pendingShow = false;
            calculateCharSize();
            setGeometry(rows, cols);
            setPosition(xPos, yPos);
        }
    }

    public void componentHidden(ComponentEvent e) {}

    public synchronized void componentResized(ComponentEvent e) {
        Dimension dim = getSize();
        int newCols = (charWidth != 0)? (dim.width  - (2 * borderWidth))  / charWidth : -1;
        int newRows = (charHeight != 0)? (dim.height - (2 * borderHeight)) / charHeight : -1;
        if(pendingShow ||
                (e != null && e.getComponent() != this) ||
                (newCols <= 0 || newRows <= 0)) {
            return;
        }

        if(newRows != rows || newCols != cols) {
            if (controller != null) {
                controller.displayDragResize(newRows, newCols);
            }
        }
    }

    public synchronized void adjustmentValueChanged(AdjustmentEvent e) {
        visTop = e.getValue();
        updateScrollbarValues();
        repaint();
    }

    private final int mouseRow(int y) {
        int mouseRow = (y - borderHeight) / charHeight;
        if(mouseRow < 0)
            mouseRow = 0;
        else if(mouseRow >= rows)
            mouseRow = rows - 1;
        return mouseRow;
    }
    private final int mouseCol(int x) {
        int mouseCol = (x - borderWidth)  / charWidth;
        if(mouseCol < 0)
            mouseCol = 0;
        else if(mouseCol >= cols)
            mouseCol = cols - 1;
        return mouseCol;
    }

    private static int getWhichButton(MouseEvent e) {
        int m = e.getModifiers();
        if ((m & MouseEvent.BUTTON1_MASK) != 0)
            return DisplayController.LEFT_BUTTON;
        else if ((m & MouseEvent.BUTTON2_MASK) != 0)
            return DisplayController.MIDDLE_BUTTON;
        else if ((m & MouseEvent.BUTTON3_MASK) != 0)
            return DisplayController.RIGHT_BUTTON;
        return DisplayController.UNKNOWN_BUTTON;
    }

    public void mouseMoved(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}
    public synchronized void mouseClicked(MouseEvent e) {
        if (e == null)
            return;

        int row = mouseRow(e.getY());
        int col = mouseCol(e.getX());
        int mod = e.getModifiers();

        if (controller != null)
            controller.mouseClicked(visTop, row, col, mod, getWhichButton(e));
    }
    public synchronized void mousePressed(MouseEvent e) {
        if (e == null)
            return;

        int row = mouseRow(e.getY());
        int col = mouseCol(e.getX());
        int mod = e.getModifiers();

        if (controller != null)
            controller.mousePressed
                (visTop, row, col, mod, getWhichButton(e), e.getX(), e.getY());
    }
    public synchronized void mouseReleased(MouseEvent e) {
        if (e == null)
            return;

        int row = mouseRow(e.getY());
        int col = mouseCol(e.getX());
        int mod = e.getModifiers();

        if (controller != null)
            controller.mouseReleased(visTop, row, col, mod, getWhichButton(e));
    }
    public synchronized void mouseDragged(MouseEvent e) {
        if (e == null)
            return;

        int row = mouseRow(e.getY());
        int col = mouseCol(e.getX());
        int mod = e.getModifiers();
        if (controller != null)
            controller.mouseDragged(visTop, row, col, mod, getWhichButton(e), 0);
    }

    //
    // Methods overridden from super-class Component + some helper functions
    //

    private void calculateCharSize() {
        int charMaxAscent;
        int charMaxDescent;
        int charLeading;
        FontMetrics fm = getFontMetrics(getFont());
        charWidth      = -1; // !!! Does not seem to work: fm.getMaxAdvance();
        charHeight     = fm.getHeight() + lineSpaceDelta;
        charMaxAscent  = fm.getMaxAscent();
        charMaxDescent = fm.getMaxDescent();
        charLeading    = fm.getLeading();
        baselineIndex  = charMaxAscent + charLeading - 1;

        if(charWidth == -1) {
            charWidth = fm.charWidth('W');
        }
    }

    public boolean isWide(char c) {
        return !DisplayUtil.isBoxOrBlockChar(c) && 
            getFontMetrics(getFont()).charWidth(c) > charWidth;
    }

    public Dimension getDimensionOfText(int rows, int cols) {
        //calculateCharSize();
        return new Dimension((cols * charWidth) + (2 * borderHeight),
                             (rows * charHeight) + (2 * borderWidth));
    }

    public Dimension getPreferredSize() {
        Dimension dim = getDimensionOfText(rows, cols);
        if(DEBUG)
            System.out.println("getPreferredSize " + cols + "x" + rows
                               + "(" + dim + ")");
        return dim;
    }

    public Dimension getMinimumSize() {
        return getDimensionOfText(MIN_ROWS, MIN_COLS);
    }

    public Dimension getMaximumSize() {
        return getDimensionOfText(MAX_ROWS, MAX_COLS);
    }

    final Rectangle getClipRect(Graphics g) {
        Rectangle clipRect = g.getClipBounds();
        if(clipRect == null) {
            Dimension winSize = getSize();
            clipRect = new Rectangle(0, 0, winSize.width, winSize.height);
        }
        return clipRect;
    }

    private void clearDirtyArea(Graphics source, Graphics dest, 
                                int left, int top, int right, int bottom) {
        boolean clearAll = (left   == 0    &&
                            right  == cols &&
                            top    == 0    &&
                            bottom == rows);
        int x, y, w, h;

        if(clearAll) {
            Dimension dim = getSize();
            x = 0;
            y = 0;
            w = dim.width;
            h = dim.height;
        } else {
            x = borderWidth + (charWidth   * left);
            y = borderHeight + (top    * charHeight);
            w = (charWidth   * (right  - left));
            h = (charHeight  * (bottom - top));
        }

        source.setColor(origBgColor);
        source.fillRect(x, y, w, h);
        source.setColor(origFgColor);
        dest.setClip(x, y, w, h);
    }

    void doRepaint() {
        super.repaint();
    }
    public void repaint() {
        repaint(false);
    }
    public void repaint(boolean force) {
        repainter.repaint(force);
    }

    public void paint(Graphics g) {
        update(g);
    }

    public void update(Graphics g) {
        if (model == null) {
            return;
        }

        // This should not happen but better safe than sorry...
        if(hpixels == 0 || vpixels == 0) {
            Dimension dim = getSize();
            vpixels = dim.height;
            hpixels = dim.width;
            if(hpixels == 0 || vpixels == 0) {
                return;
            }
        }

        Rectangle clipRect;
        int dirtyLeft, dirtyRight, dirtyTop, dirtyBottom;
        boolean isDirty;
        
        synchronized (this) {
            dirtyLeft   = this.dirtyLeft;
            dirtyRight  = this.dirtyRight;
            dirtyTop    = this.dirtyTop;
            dirtyBottom = this.dirtyBottom;
            isDirty     = this.isDirty;
            
            // Reset dirty area (i.e. we have take responsibility for it)
            this.isDirty = false;
        }        

        if((memGraphics == null) ||
                (memImageSize == null) ||
                (hpixels != memImageSize.width) ||
                (vpixels != memImageSize.height)) {
            memImageSize = new Dimension(hpixels, vpixels);
            memImage     = createImage(hpixels, vpixels);
        }
        memGraphics = memImage.getGraphics();

        if(isDirty) {
            clearDirtyArea(memGraphics, g, dirtyLeft, dirtyTop, dirtyRight, dirtyBottom);
            clipRect = getClipRect(g);
        } else {
            // If nothing is dirty, the cause for update must be
            // "destroyed" window content.
            //
            makeAllDirty();
            dirtyTop    = 0;
            dirtyBottom = rows;
            dirtyLeft   = 0;
            dirtyRight  = cols;

            clipRect    = getClipRect(g);
            memGraphics.setClip(clipRect.x, clipRect.y, clipRect.width,
                                clipRect.height);
            memGraphics.setColor(origBgColor);
            memGraphics.fillRect(clipRect.x, clipRect.y, clipRect.width,
                                 clipRect.height);
            memGraphics.setColor(origFgColor);
        }

        int x, y, curX = 0, curY = 0;
        boolean doCursor = false;
        boolean doCursorInverse = false;

        for(int i = dirtyTop; i < dirtyBottom; i++) {
            y = borderHeight + (i * charHeight);
            int[] attrRow = model.getAttribs(visTop,  i);
            char[] charRow = model.getChars(visTop, i);

            // Sanity checks to see if the model is resized between calls
            // of getAttribs() and getChars()
            if (attrRow == null || charRow == null) {
                continue;
            }
            if (attrRow.length != charRow.length) {
                continue;
            }
            if (dirtyLeft > attrRow.length || dirtyRight > attrRow.length) {
                continue;
            }

            for(int j = dirtyLeft; j < dirtyRight; j++) {
                Color bgColor = origBgColor;
                Color fgColor = origFgColor;
                int attr       = attrRow[j];
                int attrMasked = (attr & DisplayModel.MASK_ATTR);
                boolean doDraw = false;
                x = borderWidth + (charWidth * j);
                if (((attr & DisplayModel.ATTR_INVERSE) != 0) ^
                        isInsideSelection(visTop + i, j)) {
                    if ((attr & DisplayModel.ATTR_FGCOLOR) != 0) {
                        bgColor = termColors[(attr & DisplayModel.MASK_FGCOL) 
                                             >>> DisplayModel.SHIFT_FGCOL];
                    } else {
                        bgColor = origFgColor;
                    }
                    if ((attr & DisplayModel.ATTR_BGCOLOR) != 0) {
                        fgColor = (attr & DisplayModel.ATTR_INVISIBLE)!=0 ?
                        		  bgColor :
                        		  termColors[(attr & DisplayModel.MASK_BGCOL) 
                                               >>> DisplayModel.SHIFT_BGCOL];
                    } else {
                        fgColor = (attr & DisplayModel.ATTR_INVISIBLE)!=0 ?
                        		  bgColor :
                        		  origBgColor;
                    }

                    if ((attr & DisplayModel.ATTR_LOWINTENSITY) != 0 &
                    	(attr & DisplayModel.ATTR_INVISIBLE) == 0) {
                        bgColor = makeDimmerColor(bgColor);
                    }
                    doDraw = true;
                } else {
                    if((attr & DisplayModel.ATTR_BGCOLOR) != 0) {
                        bgColor = termColors[(attr & DisplayModel.MASK_BGCOL) 
                                             >>> DisplayModel.SHIFT_BGCOL];
                        doDraw = true;
                    }
                    if((attr & DisplayModel.ATTR_FGCOLOR) != 0) {
                        fgColor = (attr & DisplayModel.ATTR_INVISIBLE) != 0 ?
                        		  bgColor :
                        	      termColors[(attr & DisplayModel.MASK_FGCOL)  
                                               >>> DisplayModel.SHIFT_FGCOL];
                    }

                    if ((attr & DisplayModel.ATTR_LOWINTENSITY) != 0 &
                    	(attr & DisplayModel.ATTR_INVISIBLE) == 0) {
                        fgColor = makeDimmerColor(fgColor);
                    }
                }

                if (hasCursor && (visTop + i) == curRow && j == curCol) {
                    doCursor = true;
                    doCursorInverse = ((attr & DisplayModel.ATTR_INVERSE) != 0);
                    curX = x;
                    curY = y;
                } else {
                    // Only draw if bg is different from what we cleared area with
                    if (doDraw) {
                        memGraphics.setColor(bgColor);
                        memGraphics.fillRect(x, y, charWidth, charHeight);
                    }
                    memGraphics.setColor(fgColor);
                }

                if ((attrMasked & DisplayModel.ATTR_CHARDRAWN) != 0) {
                    if ((attr & DisplayModel.ATTR_INVISIBLE) != 0) {
                        // Don't draw anything invisible, but the
                        // underline should be drawn anyway.
                    } else if((attr & DisplayModel.ATTR_LINEDRAW) != 0) {
                        DisplayUtil.drawLineDrawChar(memGraphics, x, y,
                                                     baselineIndex,
                                                     charRow[j],
                                                     charWidth, charHeight);
                    } else if((attr & DisplayModel.ATTR_BOLD) != 0 ||
                              (attr & DisplayModel.ATTR_BLINKING) != 0) {
                        // Approximate blinking with bold font until
                        // a special update thread is implemented
                        memGraphics.setFont(boldFont);
                        memGraphics.drawChars(charRow, j, 1, x, y + baselineIndex);
                        memGraphics.setFont(plainFont);
                    } else if (DisplayUtil.isBoxOrBlockChar(charRow[j])) {
                        DisplayUtil.drawBoxOrBlockChar
                            (memGraphics, x, y, baselineIndex, 
                             charRow[j], charWidth, charHeight);
                    } else if (charRow[j] != ' ') { // no need to draw spaces
                        memGraphics.drawChars(charRow, j, 1, x, y + baselineIndex);
                    }
                    if((attr & DisplayModel.ATTR_UNDERLINE) != 0)
                        memGraphics.drawLine(x, y + baselineIndex, x + charWidth, y + baselineIndex);
                }
            }
        }

        if (doCursor) {
            memGraphics.setColor(cursorColor);
            memGraphics.setXORMode(doCursorInverse ? origFgColor : origBgColor);
            if (cursorHollow) {
                memGraphics.drawRect(curX, curY, charWidth-1, charHeight-1);
            } else {
                memGraphics.fillRect(curX, curY, charWidth, charHeight);
            }
            memGraphics.setPaintMode();
            memGraphics.setColor(origFgColor);
        }

        g.drawImage(memImage, 0, 0, this);

        if(logoDraw && logoImg != null) {
            x = logoX;
            y = logoY;
            if(x == -1) {
                if (centerLogoX == -1) {
                    centerLogoX = (hpixels / 2) - (logoW / 2);
                }
                x = centerLogoX;
            }
            if (y == -1) {
                if (centerLogoY == -1) {
                    centerLogoY = (vpixels / 2) - (logoH / 2);
                }
                y = centerLogoY;
            }
            g.drawImage(logoImg, x, y, logoW, logoH, this);
        }

        if (updateScrollbar) {
            updateScrollbar = false;
            scrollbar.setValues(visTop, rows, 0, model.getBufferRows());
            scrollbar.setBlockIncrement(rows);
        }
    }

    protected Hashtable dimmmedColors = new Hashtable();

    protected Color makeDimmerColor(Color orgColor) {
        if (orgColor == null) {
            return null;
        }

        Color dimColor = (Color) dimmmedColors.get(orgColor);
        if (dimColor != null) {
            return dimColor;
        }

        // Can't use darker() method in Color, since it don't make
        // black dimmer.

        float hsbComps[] = Color.RGBtoHSB(orgColor.getRed(),
                                          orgColor.getGreen(),
                                          orgColor.getBlue(), null);
        float hue = hsbComps[0];
        float saturation = hsbComps[1];
        float brightness = hsbComps[2];

        if ((saturation*saturation + brightness*brightness) < 0.1) {
            // orgColor is very dark, increate saturation and brightness
            // to make it appear dimmer
            saturation = (float) 0.3;
            brightness = (float) 0.3;
        } else {
            brightness *= 0.50;
        }

        dimColor = Color.getHSBColor(hue, saturation, brightness);
        dimmmedColors.put(orgColor, dimColor);

        return dimColor;
    }

    private void drawTuplet(Graphics g, int x, int y, int x2, int y2, int bi,
                            String s1, String s2) {
        Font font = g.getFont();
        g.setFont(new Font(font.getName(), font.getStyle(), font.getSize()/2));
        g.drawString(s1, x+1, y+1 + bi/2);
        g.drawString(s2, x2, y2 + bi/2);
        g.setFont(font);
    }

    final void drawLineDrawChar(Graphics g, int x, int y, int bi, char c) {
        int x2 = (x + (charWidth  / 2));
        int y2 = (y + (charHeight / 2));
        int xx = (x + charWidth);
        int yy = (y + charHeight);

        switch(c) {
        case ' ': // Blank
        case '_': // Blank
            break;
        case '`': // Diamond
            int[] polyX = new int[4];
            int[] polyY = new int[4];
            polyX[0] = x2;
            polyY[0] = y;
            polyX[1] = xx;
            polyY[1] = y2;
            polyX[2] = x2;
            polyY[2] = yy;
            polyX[3] = x;
            polyY[3] = y2;
            g.fillPolygon(polyX, polyY, 4);
            break;
        case 'a': // Checker board (stipple)
            for (int i=x; i<xx; i++) {
                for (int j=y; j<yy; j++) {
                    if ( ((i+j)%2) == 0) {
                        g.fillRect(i, j, 1, 1);
                    }
                }
            }
            break;
        case 'b': // Horizontal tab
            drawTuplet(g, x, y, x2, y2, bi, "H", "T");
            break;
        case 'c': // Form Feed
            drawTuplet(g, x, y, x2, y2, bi, "F", "F");
            break;
        case 'd': // Carriage Return
            drawTuplet(g, x, y, x2, y2, bi, "C", "R");
            break;
        case 'e': // Line Feed
            drawTuplet(g, x, y, x2, y2, bi, "L", "F");
            break;
        case 'f': { // Degrees
            char[] ca = new char[1];
            ca[0] = (char)0x00b0;
            g.drawChars(ca, 0, 1, x, y + bi);
            break;
        }
        case 'g': { // Plus/Minus
            char[] ca = new char[1];
            ca[0] = (char)0x00b1;
            g.drawChars(ca, 0, 1, x, y + bi);
            break;
        }
        case 'h': // New line
            drawTuplet(g, x, y, x2, y2, bi, "N", "L");
            break;
        case 'i': // Vertical Tab
            drawTuplet(g, x, y, x2, y2, bi, "V", "T");
            break;
        case 'j': // Lower right corner
            g.drawLine(x2, y, x2, y2);
            g.drawLine(x2, y2, x, y2);
            break;
        case 'k': // Upper right corner
            g.drawLine(x, y2, x2, y2);
            g.drawLine(x2, y2, x2, yy);
            break;
        case 'l': // Upper left corner
            g.drawLine(x2, yy, x2, y2);
            g.drawLine(x2, y2, xx, y2);
            break;
        case 'm': // Lower left corner
            g.drawLine(x2, y, x2, y2);
            g.drawLine(x2, y2, xx, y2);
            break;
        case 'n': // Cross center lines
            g.drawLine(x2, y, x2, yy);
            g.drawLine(x, y2, xx, y2);
            break;
        case 'o': // Horizontal line (top)
            g.drawLine(x, y, xx, y);
            break;
        case 'p': // Horizontal line (top-half)
            g.drawLine(x, (y+y2)/2, xx, (y+y2)/2);
            break;
        case 'q': // Horizontal line (center)
            g.drawLine(x, y2, xx, y2);
            break;
        case 'r': // Horizontal line (bottom-half)
            g.drawLine(x, (yy+y2)/2, xx, (yy+y2)/2);
            break;
        case 's': // Horizontal line (bottom)
            g.drawLine(x, yy, xx, yy);
            break;
        case 't': // Left tee
            g.drawLine(x2, y, x2, yy);
            g.drawLine(x2, y2, xx, y2);
            break;
        case 'u': // Right tee
            g.drawLine(x2, y, x2, yy);
            g.drawLine(x, y2, x2, y2);
            break;
        case 'v': // Bottom tee
            g.drawLine(x, y2, xx, y2);
            g.drawLine(x2, y2, x2, y);
            break;
        case 'w': // Top tee
            g.drawLine(x, y2, xx, y2);
            g.drawLine(x2, y2, x2, yy);
            break;
        case 'x': // Vertical line
            g.drawLine(x2, y, x2, yy);
            break;
        case 'y': { // Less than or equal
            int dx = charWidth/5;
            int dy = charHeight/5;
            g.drawLine(x+dx, y2, xx-dx, y+2*dy);
            g.drawLine(x+dx, y2, xx-dx, yy-2*dy);
            g.drawLine(x+dx, y2+dy, xx-dx, yy-dy);
            break;
        }
        case 'z': { // Greater than or equal
            int dx = charWidth/5;
            int dy = charHeight/5;
            g.drawLine(xx-dx, y2, x+dx, y+2*dy);
            g.drawLine(xx-dx, y2, x+dx, yy-2*dy);
            g.drawLine(xx-dx, y2+dy, x+dx, yy-dy);
            break;
        }
        case '{': { // Pi
            char[] ca = new char[1];
            ca[0] = (char)0x03c0;
            g.drawChars(ca, 0, 1, x, y + bi);
            break;
        }
        case '|': { // Not equal
            char[] ca = new char[1];
            ca[0] = (char)0x2260;
            g.drawChars(ca, 0, 1, x, y + bi);
            break;
        }
        case '}': { // UK pound
            char[] ca = new char[1];
            ca[0] = (char)0x00a3;
            g.drawChars(ca, 0, 1, x, y + bi);
            break;
        }
        case '~': { // Center dot
            char[] ca = new char[1];
            ca[0] = (char)0x00b7;
            g.drawChars(ca, 0, 1, x, y + bi);
            break;
        }
        default:
            if(DEBUG)
                System.out.println("Unknown line-draw-char: " + c + " (" + ((int)c) + ")");
            break;
        }
    }

    public void setPosition(int xPos, int yPos) {
        Dimension sDim  = Toolkit.getDefaultToolkit().getScreenSize();
        Dimension tDim  = getDimensionOfText(rows, cols);
        Insets    fIns  = ownerFrame.getInsets();
        int       sbSz  = (haveScrollbar?scrollbar.getSize().width:0);

        if(xPos < 0) {
            xPos += sDim.width - tDim.width - fIns.left - fIns.right - sbSz;
        }
        if(yPos < 0) {
            yPos += sDim.height - tDim.height - fIns.top - fIns.bottom;
        }
        this.xPos = xPos;
        this.yPos = yPos;

        if (isShowing()) {
            ownerFrame.setLocation(xPos, yPos);
            ownerFrame.pack();
            requestFocus();
        } else {
            pendingShow = true;
        }
        repaint(true);
    }

    public synchronized void setGeometry(int row, int col) {
        /*
        if (row == rows && col == cols) {
            return;
        }
        */

        Dimension tDim  = getDimensionOfText(row, col);
        if (vpixels == tDim.height && hpixels == tDim.width) {
            return;
        }

        vpixels = tDim.height;
        hpixels = tDim.width;
        rows = row;
        cols = col;
        setSize(tDim);

        if (visTopChangePending) {
            this.visTop = fenceVisTop(visTopChange);
            visTopChangePending = false;
        }

        if (isShowing()) {
            memGraphics = null;
            updateScrollbarValues();
            makeAllDirty();
            ownerFrame.pack();
            requestFocus();
        } else {
            pendingShow = true;
        }
        repaint();

        if (controller != null) {
            controller.displayResized(row, col, vpixels, hpixels);
        }
    }

    public synchronized void resetSelection() {
        hasSelection = false;
        makeSelectionDirty();
        repaint();
    }

    public synchronized void setSelection(int row1, int col1, int row2, int col2) {
        if (hasSelection) {
            makeSelectionDirty();
        } else {
            hasSelection = true;
        }

        if (row1 < row2) {
            selectionTopRow =    row1;
            selectionTopCol =    col1;
            selectionBottomRow = row2;
            selectionBottomCol = col2;
        } else if (row1 == row2) {
            selectionTopRow = selectionBottomRow = row1;
            if (col1 < col2) {
                selectionTopCol    = col1;
                selectionBottomCol = col2;
            } else {
                selectionTopCol    = col2;
                selectionBottomCol = col1;
            }
        } else {
            selectionTopRow =    row2;
            selectionTopCol =    col2;
            selectionBottomRow = row1;
            selectionBottomCol = col1;
        }

        makeSelectionDirty();
        repaint();
    }

    public void setNoCursor() {
        if (hasCursor) {
            hasCursor = false;
            makeCursorDirty();
            repaint();
        }
    }
    public synchronized void setCursorPosition(int row, int col) {
        makeCursorDirty();
        if (!hasCursor) {
            hasCursor = true;
        }
        curRow = row;
        curCol = col;
        makeCursorDirty();
    }

    public void setBackgroundColor(Color c) {
        origBgColor = c;
        setBackground(origBgColor);
        makeAllDirty();
        repaint();
    }

    public void setForegroundColor(Color c) {
        origFgColor = c;
        setForeground(origFgColor);
        makeAllDirty();
        repaint();
    }

    public void setCursorColor(Color c) {
        cursorColor = c;
        makeAllDirty();
        repaint();
    }

    public void reverseColors() {
        Color swap  = origBgColor;
        origBgColor = origFgColor;
        origFgColor = swap;
        makeAllDirty();
        repaint();
    }

    public void doBell() {
        doBell(false);
    }

    public void doBell(boolean visualBell) {
        if (visualBell) {
            reverseColors();
            try {
                Thread.sleep(25);
            } catch (InterruptedException e) {}
            reverseColors();
        } else {
            Toolkit toolkit = Toolkit.getDefaultToolkit();
            if(toolkit != null) {
                try {
                    toolkit.beep();
                } catch (Exception e) {
                    // Could not beep, we are probably an unpriviliged applet
                    // Automatically enable visual-bell now and "sound" it
                    // instead
                    doBell(true);
                }
            }
        }
    }

    public Component getAWTComponent() { return this; }

    public void setIgnoreClose() {
        // Do nothing
    }

    public void windowClosed() {
        removeComponentListener(this);
        removeFocusListener(this);
        removeMouseMotionListener(this);
        removeMouseListener(this);
        ownerFrame.removeComponentListener(this);
        ownerFrame = null;
        controller = null;
        model      = null;
        repainter  = null;
        
        if (myPanel != null) 
            myPanel.removeAll();
        myPanel = null;
        scrollbar = null;
    }

    public Component mkButton(String label,String cmd,ActionListener listener){
        Button button = new Button(label);
        button.setActionCommand(cmd);
        button.addActionListener(listener);
        return button;
    }
}

