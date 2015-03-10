package com.mindterm.util.ssh;

/**
 * Used to return the outcome of a process execution by the {@link SSHSession} class.
 */
public class ExecResponse
{
  /**
   * The standard output bytes.
   */  
  private byte out[];
  
  /**
   * The standard error bytes.
   */  
  private byte err[];
  
  /**
   * The return code.
   */
  private int code;
  
  /**
   * Constructs this object.
   * @param out the standard output
   * @param err the standard error
   * @param code the return code
   */
  ExecResponse(byte[] out, byte[] err, int code)
  {
    this.out = out;
    this.err = err;
    this.code = code;
  }
  
  /**
   * returns the standard output bytes.
   */
  public byte[] getOut()
  {
    return out;
  }
  
  /**
   * returns the standard error bytes.
   */
  public byte[] getErr()
  {
    return err;
  }
  
  /**
   * returns the return code.
   */
  public int getCode()
  {
    return code;
  }

    public String toString() {
        return "[" + code + " - '" + (new String(out)) + "' - '" + (new String(err)) + "']"; 
    }
}
