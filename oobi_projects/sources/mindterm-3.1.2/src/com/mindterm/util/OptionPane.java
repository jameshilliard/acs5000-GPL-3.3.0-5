package com.mindterm.util;

import java.awt.BorderLayout;
import java.awt.Button;
import java.awt.Checkbox;
import java.awt.CheckboxGroup;
import java.awt.Component;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.GridLayout;
import java.awt.Label;
import java.awt.Panel;
import java.awt.Point;
import java.awt.TextField;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.util.StringTokenizer;


/** <P>OptionPane makes it easy to pop up a standard dialog box that prompts users
  * for a value or informs them of something.</P>
  *
  * <P>While the OptionPane class may appear complex because of the large number of methods,
  * almost all uses of this class are one-line calls to one of the static showXxxDialog
  * methods shown below: </P>
  *
  * <P><UL><LI>showConfirmDialog asks a confirming question, like yes/no/cancel.</LI>
  * <LI>showInputDialog prompts for some input.</LI>
  * <LI>showMessageDialog tells the user about something that has happened.</LI></UL></P>
  *
  *
  */
public class OptionPane extends Dialog implements ActionListener, ItemListener {
    // constants
    /** This constant is returned by OptionPane.showConfirmDialog when the user clicks the Yes button. */
    public static final int YES_OPTION = 0;

    /** This constant is returned by OptionPane.showConfirmDialog when the user clicks the No button. */
    public static final int NO_OPTION = 1;

    /** This constant is returned by OptionPane.showConfirmDialog and
      * OptionPane.showInputDialog when the user clicks the Cancel button. */
    public static final int CANCEL_OPTION = -1;

    public static final int YES_NO_OPTION = 3;

    public static final int YES_NO_CANCEL_OPTION = 4;

    public static final int OK_OPTION = 5;

    public static final int OK_CANCEL_OPTION = 6;

    public static final int INFORMATION_MESSAGE = 7;

    public static final int QUESTION_MESSAGE = 8;

    public static final int ERROR_MESSAGE = 9;

    public static final int PLAIN_MESSAGE = 10;

    public static final int WARNING_MESSAGE = 11;


  // instance vars
  private static int CONFIRM_TYPE = 0;
  private static int INPUT_TYPE = 1;
  private static int MESSAGE_TYPE = 2;
  private static int OPTION_TYPE = 3;

  private Dialog my_dialog;
  private Button   my_yesButton, my_noButton, my_okButton, my_cancelButton;
  private Checkbox[] my_choiceButtons;
  private int my_type;
  private int my_selectedIndex = CANCEL_OPTION;
  private TextField my_field;
  private Object   my_choice = null;
  private Object[] my_options;


  /* Constructs a new OptionPane of the given type with the given Frame as
   * its parent, with the given window title, displaying the given message.
   *
   * If parent is non-null, the OptionPane will center itself with respect
   * to the parent.  Otherwise, the OptionPane will locate itself at (0, 0).
   *
   * Acceptable types are OptionPane.CONFIRM_TYPE, OptionPane.INPUT_TYPE, and OptionPane.MESSAGE_TYPE.
   */
  private OptionPane(Component parent, String message, String title, int type, int subtype) {
    super((parent instanceof Frame) ? (Frame)parent : new Frame(), title, true);
    init(parent, message, title, type, subtype, null, null);
  }

  private OptionPane(Component parent, String message, String title, int type, int subtype, Object[] options, Object initialOption) {
    super((parent instanceof Frame) ? (Frame)parent : new Frame(), title, true);
    init(parent, message, title, type, subtype, options, initialOption);
  }

  private void init(Component parent, String message, String title, int type, int subtype, Object[] options, Object initialOption) {
    my_type = type;
    my_options = options;

    // set initial option selected
    if (options != null) {
      for (int ii = 0;  ii < options.length;  ii++) {
        if (options[ii].equals(initialOption)) {
          my_selectedIndex = ii;
          my_choice = new Integer(ii);
        }
      }

      if (my_choice == null)
        my_choice = new Integer(CANCEL_OPTION);
    }
    setResizable(false);

    // construct components
    my_yesButton    = new Button("Yes");
    my_noButton     = new Button("No");
    my_okButton     = new Button("OK");
    my_cancelButton = new Button("Cancel");
    my_field = new TextField(10);

    // event listening
    my_yesButton.addActionListener(this);
    my_noButton.addActionListener(this);
    my_okButton.addActionListener(this);
    my_cancelButton.addActionListener(this);
    my_field.addActionListener(this);

    // layout
    Panel contentPane = new Panel(new BorderLayout());
    Panel centerPanel = new Panel();
    Panel southPanel = new Panel();

    Panel labelPanel = new Panel(new GridLayout(0, 1));
    StringTokenizer tokenizer = new StringTokenizer(message, "\n");
    while (tokenizer.hasMoreTokens())
      labelPanel.add(new Label(tokenizer.nextToken()));

    if (type == CONFIRM_TYPE) {
      centerPanel.add(labelPanel);
      southPanel.add(my_yesButton);
      southPanel.add(my_noButton);
      if (subtype == YES_NO_CANCEL_OPTION)
        southPanel.add(my_cancelButton);
    }
    else if (type == INPUT_TYPE) {
      centerPanel.setLayout(new BorderLayout());
      centerPanel.add(labelPanel, BorderLayout.CENTER);
      centerPanel.add(my_field, BorderLayout.SOUTH);
      southPanel.add(my_okButton);
      if (subtype == OK_CANCEL_OPTION)
        southPanel.add(my_cancelButton);
    }
    else if (type == MESSAGE_TYPE) {
      centerPanel.add(labelPanel);
      southPanel.add(my_okButton);
    }
    else if (type == OPTION_TYPE) {
      if (options == null)
        throw new IllegalArgumentException("null options list");

      centerPanel.setLayout(new GridLayout(0, 1));
      centerPanel.add(labelPanel);
      CheckboxGroup cbg = new CheckboxGroup();

      int numOptions = options.length;
      my_choiceButtons = new Checkbox[numOptions];
      for (int ii = 0;  ii < numOptions;  ii++) {
        my_choiceButtons[ii] = new Checkbox(options[ii].toString(), options[ii] == initialOption, cbg);
        my_choiceButtons[ii].addItemListener(this);
        centerPanel.add(my_choiceButtons[ii]);
      }

      southPanel.add(my_okButton);
      if (subtype == OK_CANCEL_OPTION)
        southPanel.add(my_cancelButton);
    }

    contentPane.add(centerPanel, BorderLayout.CENTER);
    contentPane.add(southPanel, BorderLayout.SOUTH);
    add(contentPane);
    pack();

    // set location to center of parent
    int x = 0;
    int y = 0;

    Point p = parent != null ? parent.getLocation() : new Point(0, 0);
    Dimension parentSize = parent != null ? parent.getSize() : Toolkit.getDefaultToolkit().getScreenSize();
    Dimension mySize = getSize();
    x = p.x + (parentSize.width  - mySize.width ) / 2;
    y = p.y + (parentSize.height - mySize.height) / 2;

    setLocation(x, y);
  }


  /** Processes ActionEvents in this OptionPane. */
  public void actionPerformed(ActionEvent event) {
    Object source = event.getSource();

    if (source == my_yesButton)
      my_choice = new Integer(YES_OPTION);
    if (source == my_noButton)
      my_choice = new Integer(NO_OPTION);
    else if (source == my_okButton  ||  source == my_field) {
      if (my_type == OPTION_TYPE)
        my_choice = new Integer(my_selectedIndex);
      else
        my_choice = my_field.getText();
    } else if (source == my_cancelButton)
      my_choice = null;

    hide();
    dispose();
  }

  /** Processes ItemEvents in this OptionPane. */
  public void itemStateChanged(ItemEvent event) {
    Object source = event.getSource();
    for (int ii = 0;  ii < my_choiceButtons.length;  ii++)
      if (source == my_choiceButtons[ii]) {
        my_selectedIndex = ii;
      }
  }


  /** Shows this OptionPane and returns the resulting input, if any. */
  private Object showDialog() {
    show();
    return my_choice;
  }


  /** Shows a confirmation dialog with the given message, using the given component as its parent.
    * @return an integer corresponding to the button the user pressed.  The integer may be one of
    * OptionPane.YES_OPTION, OptionPane.NO_OPTION, or OptionPane.CANCEL_OPTION.
    */
  public static int showConfirmDialog(Component parent, Object message) {
    return showConfirmDialog(parent, message, "Confirm", YES_NO_CANCEL_OPTION);
  }

  public static int showConfirmDialog(Component parent, Object message, String title, int type) {
    Integer i = (Integer)(new OptionPane(parent, (String)message, title, CONFIRM_TYPE, type).showDialog());
    return (i != null) ? i.intValue() : CANCEL_OPTION;
  }

  /** Shows a dialog asking for input, with the given message, using the given component as its parent.
    * @return the input text typed by the user; null if the user presses Cancel.
    */
  public static String showInputDialog(Component parent, Object message) {
    return showInputDialog(parent, message, "Enter Input", OK_CANCEL_OPTION);
  }

  public static String showInputDialog(Component parent, Object message, String title, int type) {
    return (String)(new OptionPane(parent, (String)message, title, INPUT_TYPE, type).showDialog());
  }

  /** Shows a dialog displaying the given message, using the given component as its parent. */
  public static void showMessageDialog(Component parent, Object message) {
    showMessageDialog(parent, message, "Message", OK_OPTION);
  }

  public static void showMessageDialog(Component parent, Object message, String title, int type) {
    new OptionPane(parent, (String)message, title, MESSAGE_TYPE, type).showDialog();
  }

  public static int showOptionDialog(Component parent, String message, String title, int type, int unused, Object unusedIcon, Object[] options, Object initialValue) {
    Integer i = (Integer)new OptionPane(parent, (String)message, title, OPTION_TYPE, type, options, initialValue).showDialog();
    return (i != null) ? i.intValue() : CANCEL_OPTION;
  }

  public static void main(String[] args) {
    String[] items = new String[] {"Fred", "Barney", "Wilma", "Dino"};
    int index = OptionPane.showOptionDialog(null, "Pick one!", "Title here", OptionPane.OK_OPTION, 0, null, items, items[1]);
    System.out.println("index: " + index);
  }
}

