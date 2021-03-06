/*
 * @(#)NumberConstantData.java	1.16 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.io.IOException;
import java.io.DataOutputStream;

/**
 * A numeric constant pool item. Can either be integer, float, long or double.
 */
final
class NumberConstantData extends ConstantPoolData {
    Number num;

    /**
     * Constructor
     */
    NumberConstantData(ConstantPool tab, Number num) {
	this.num = num;
    }

    /**
     * Write the constant to the output stream
     */
    void write(Environment env, DataOutputStream out, ConstantPool tab) throws IOException {
	if (num instanceof Integer) {
	    out.writeByte(CONSTANT_INTEGER);
	    out.writeInt(num.intValue());
	} else if (num instanceof Long) {
	    out.writeByte(CONSTANT_LONG);
	    out.writeLong(num.longValue());
	} else if (num instanceof Float) {
	    out.writeByte(CONSTANT_FLOAT);
	    out.writeFloat(num.floatValue());
	} else if (num instanceof Double) {
	    out.writeByte(CONSTANT_DOUBLE);
	    out.writeDouble(num.doubleValue());
	}
    }
    /**
     * Return the order of the constant
     */
    int order() {
	return (width() == 1) ? 0 : 3;
    }

    /**
     * Return the number of entries that it takes up in the constant pool
     */
    int width() {
	return ((num instanceof Double) || (num instanceof Long)) ? 2 : 1;
    }
}
