/* 
 * @(#)SimpleTreeNode.java	1.4 03/01/23 11:50:46
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.ui.tree;

import javax.swing.*;
import javax.swing.tree.*;

/** A very simple interface for nodes in the SimpleTreeModel. */

public interface SimpleTreeNode {
  public int getChildCount();
  public SimpleTreeNode getChild(int index);
  public boolean isLeaf();
  public int getIndexOfChild(SimpleTreeNode child);

  /** TreeTable support: get name of the node */
  public String getName();
 
  /** TreeTable support: get value of the node */
  public String getValue();
}
