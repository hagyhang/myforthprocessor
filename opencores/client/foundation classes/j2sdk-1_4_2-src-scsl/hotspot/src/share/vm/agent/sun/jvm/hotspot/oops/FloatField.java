/*
 * @(#)FloatField.java	1.4 03/01/23 11:42:36
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import sun.jvm.hotspot.debugger.*;

// The class for a float field simply provides access to the value.
public class FloatField extends Field {
  public FloatField(FieldIdentifier id, long offset, boolean isVMField) {
    super(id, offset, isVMField);
  }

  public FloatField(sun.jvm.hotspot.types.JFloatField vmField, long startOffset) {
    super(new NamedFieldIdentifier(vmField.getName()), vmField.getOffset() + startOffset, true); 
  }

  public FloatField(InstanceKlass holder, int fieldArrayIndex) {
    super(holder, fieldArrayIndex);
  }

  public float getValue(Oop obj) { return obj.getHandle().getJFloatAt(getOffset()); }
  public void setValue(Oop obj, float value) throws MutationException {
    // Fix this: setJFloatAt is missing in Address
  }
}

