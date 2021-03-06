/*
 * @(#)BreakStatement.java	1.27 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.tree;

import sun.tools.java.*;
import sun.tools.asm.Assembler;
import sun.tools.asm.Label;
import java.io.PrintStream;
import java.util.Hashtable;

public
class BreakStatement extends Statement {
    Identifier lbl;

    /**
     * Constructor
     */
    public BreakStatement(long where, Identifier lbl) {
	super(BREAK, where);
	this.lbl = lbl;
    }

    /**
     * Check statement
     */
    Vset check(Environment env, Context ctx, Vset vset, Hashtable exp) {
	reach(env, vset);
	checkLabel(env, ctx);
	CheckContext destctx = (CheckContext)new CheckContext(ctx, this).getBreakContext(lbl);
	if (destctx != null) {
	    if (destctx.frameNumber != ctx.frameNumber) {
		env.error(where, "branch.to.uplevel", lbl);
	    }
	    destctx.vsBreak = destctx.vsBreak.join(vset);
	} else {
	    if (lbl != null) {
		env.error(where, "label.not.found", lbl);
	    } else {
		env.error(where, "invalid.break");
	    }
	}
	CheckContext exitctx = ctx.getTryExitContext();
	if (exitctx != null) {
	    exitctx.vsTryExit = exitctx.vsTryExit.join(vset);
	}
	return DEAD_END;
    }

    /**
     * The cost of inlining this statement
     */
    public int costInline(int thresh, Environment env, Context ctx) {
	return 1;
    }

    /**
     * Code
     */
    public void code(Environment env, Context ctx, Assembler asm) {
	CodeContext newctx = new CodeContext(ctx, this);
	CodeContext destctx = (CodeContext)newctx.getBreakContext(lbl);
	codeFinally(env, ctx, asm, destctx, null);
	asm.add(where, opc_goto, destctx.breakLabel);
	asm.add(newctx.breakLabel);
    }

    /**
     * Print
     */
    public void print(PrintStream out, int indent) {
	super.print(out, indent);
	out.print("break");
	if (lbl != null) {
	    out.print(" " + lbl);
	}
	out.print(";");
    }
}
