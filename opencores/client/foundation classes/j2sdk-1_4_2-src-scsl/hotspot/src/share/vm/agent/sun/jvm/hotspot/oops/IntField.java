/*
 * @(#)IntField.java	1.4 03/01/23 11:42:56
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for an int field simply provides access to the value.
public class IntField extends Field {
  public IntField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }

  public IntField(sun.jvm.hotspot.types.JIntField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true); 
  }

  public IntField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public int getValue(Oop obj) { return obj.getHandle().getJIntAt(getOffset()); }
  public void setValue(Oop obj, int value) throws MutationException {
    // Fix this: setJIntAt is missing in Address
  }
}

