/*
 * @(#)BitXorExpression.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;

public
class BitXorExpression extends BinaryBitExpression {
    /**
     * constructor
     */
    public BitXorExpression(long where, Expression left, Expression right) {
	super(BITXOR, where, left, right);
    }

    /**
     * Evaluate
     */
    Expression eval(boolean a, boolean b) {
	return new BooleanExpression(where, a ^ b);
    }
    Expression eval(int a, int b) {
	return new IntExpression(where, a ^ b);
    }
    Expression eval(long a, long b) {
	return new LongExpression(where, a ^ b);
    }


    /**
     * Simplify
     */
    Expression simplify() {
	if (left.equals(true)) {
	    return new NotExpression(where, right);
	}
	if (right.equals(true)) {
	    return new NotExpression(where, left);
	}
	if (left.equals(false) || left.equals(0)) {
	    return right;
	}
	if (right.equals(false) || right.equals(0)) {
	    return left;
	}
	return this;
    }

    /**
     * Code
     */
    void codeOperation(Environment env, Context ctx, Assembler asm) {
	asm.add(where, opc_ixor + type.getTypeCodeOffset());
    }
}
