#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)functionAtStart.cpp	1.6 03/01/23 12:22:29 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_functionAtStart.cpp.incl"

// An empty function, exported from the DLL, which is put first on the
// link line to wind up as the first function in the DLL/DSO. Needed
// to check whether the PC, upon a crash, is within the VM or not.

address JVM_FunctionAtStart() {
  return CAST_FROM_FN_PTR(address, JVM_FunctionAtStart);
}
