package com.mindbright.application;

import java.awt.event.KeyEvent;
import javax.swing.JTextField;

/**
 *
 * @author ivan.gardino
 */
public class CustomJTextField extends JTextField {

    private CustomJTextField next;
    
    private boolean isProcesses = true;
    
    public CustomJTextField(){
        super();
    }

    protected void processKeyEvent(KeyEvent ke){
        if( ke.getKeyCode() == KeyEvent.VK_ENTER){
            if(next != null){
                if(!next.isProcesses()){
                    next.grabFocus();
                    next.setIsProcesses(true);
                    return;
                }else{
                    next.setIsProcesses(false);
                }
            }
        }

        super.processKeyEvent(ke);
    }
    
    
    public CustomJTextField getNext() {
        return next;
    }

    public void setNext(CustomJTextField next) {
        this.next = next;
    }

    public boolean isProcesses() {
        return isProcesses;
    }

    public void setIsProcesses(boolean isProcesses) {
        this.isProcesses = isProcesses;
    }
}