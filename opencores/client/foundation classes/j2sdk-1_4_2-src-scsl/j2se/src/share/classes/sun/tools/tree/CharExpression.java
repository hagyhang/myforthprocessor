/*
 * @(#)CharExpression.java	1.18 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import java.io.PrintStream;

public
class CharExpression extends IntegerExpression {
    /**
     * Constructor
     */
    public CharExpression(long where, char value) {
	super(CHARVAL, where, Type.tChar, value);
    }

    /**
     * Print
     */
    public void print(PrintStream out) {
	out.print(value + "c");
    }
}
