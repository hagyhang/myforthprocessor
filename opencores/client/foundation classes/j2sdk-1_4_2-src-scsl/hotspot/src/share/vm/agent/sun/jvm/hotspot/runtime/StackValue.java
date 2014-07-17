/*
 * @(#)StackValue.java	1.4 03/01/23 11:45:48
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.io.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.utilities.*;

public class StackValue {
  private int       type;
  private OopHandle handleValue;
  private long      integerValue;
  
  public StackValue() {
    type = BasicType.getTConflict();
  }

  public StackValue(OopHandle h) {
    handleValue = h;
    type = BasicType.getTObject();
  }

  public StackValue(long i) {
    integerValue = i;
    type = BasicType.getTInt();
  }

  /** This returns one of the "enum" values in BasicType.java */
  public int getType() {
    return type;
  }

  public OopHandle getObject() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(type == BasicType.getTObject(), "type check");
    }
    return handleValue;
  }

  public long getInteger() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(type == BasicType.getTInt(), "type check");
    }
    return integerValue;
  }

  public boolean equals(Object arg) {
    if (arg == null) {
      return false;
    }

    if (!arg.getClass().equals(getClass())) {
      return false;
    }

    StackValue sv = (StackValue) arg;
    if (type != sv.type) {
      return false;
    }
    if (type == BasicType.getTObject()) {
      return handleValue.equals(sv.handleValue);
    } else if (type == BasicType.getTInt()) {
      return (integerValue == sv.integerValue);
    } else {
      // Conflict type (not yet used)
      return true;
    }
  }

  public int hashCode() {
    if (type == BasicType.getTObject()) {
      return handleValue.hashCode();
    } else {
      // Returns 0 for conflict type
      return (int) integerValue;
    }
  }

  public void print() {
    printOn(System.out);
  }

  public void printOn(PrintStream tty) {
    if (type == BasicType.getTInt()) {
      tty.print(integerValue + " (long) " + (int) integerValue + " (int)");
    } else if (type == BasicType.getTObject()) {
      tty.print("<" + handleValue + ">");
    } else if (type == BasicType.getTConflict()) {
      tty.print("conflict");
    } else {
      throw new RuntimeException("should not reach here");
    }
  }
}