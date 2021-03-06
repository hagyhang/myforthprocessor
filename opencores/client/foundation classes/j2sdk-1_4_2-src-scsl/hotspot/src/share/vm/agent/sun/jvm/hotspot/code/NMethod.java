/*
 * @(#)NMethod.java	1.14 03/01/23 11:24:15
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

public class NMethod extends CodeBlob {
  private static long          pcDescSize;
  private static CIntegerField zombieInstructionSizeField;
  private static sun.jvm.hotspot.types.OopField methodField;
  /** Incremented for each nmethod invocation if CountExecution == true */
  private static CIntegerField invocationCountField;
  /** != InvocationEntryBci if this nmethod is an on-stack replacement method */
  private static CIntegerField entryBCIField;
  /** To support simple linked-list chaining of nmethods */
  private static AddressField  linkField;
  /** Offsets for different nmethod parts */
  private static CIntegerField exceptionOffsetField;
  private static CIntegerField stubOffsetField;
  private static CIntegerField scopesDataOffsetField;
  private static CIntegerField scopesPCsOffsetField;
  private static CIntegerField handlerTableOffsetField;
  private static CIntegerField nulChkTableOffsetField;
  private static CIntegerField nmethodEndOffsetField;

  private static CIntegerField firstDependentField;
  private static CIntegerField numberOfDependentsField;

  private static CIntegerField maxNofMonitorsField;
  private static CIntegerField maxNofLocalsField;
  
  /** Offsets for entry points */
  /** Entry point with class check */
  private static AddressField  entryPointField;
  /** Entry point without class check */
  private static AddressField  verifiedEntryPointField;
  /** Entry point when coming from interpreter */
  private static AddressField  interpreterEntryPointField;
  /** Entry point for on stack replacement */
  private static AddressField  osrEntryPointField;
  private static CIntegerField frameStartOffsetField;

  // FIXME: add access to flags (how?)

  /** NMethod Flushing lock (if non-zero, then the nmethod is not removed) */
  private static JIntField     lockCountField;

  /** not_entrant method removal. Each mark_sweep pass will update 
      this mark to current sweep invocation count if it is seen on the
      stack.  An not_entrant method can be removed when there is no
      more activations, i.e., when the _stack_traversal_mark is less than
      current sweep traversal index. */
  private static CIntegerField stackTraversalMarkField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("nmethod");

    zombieInstructionSizeField  = type.getCIntegerField("_zombie_instruction_size");
    methodField                 = type.getOopField("_method");
    invocationCountField        = type.getCIntegerField("_invocation_count");
    entryBCIField               = type.getCIntegerField("_entry_bci");
    linkField                   = type.getAddressField("_link");
    exceptionOffsetField        = type.getCIntegerField("_exception_offset");
    stubOffsetField             = type.getCIntegerField("_stub_offset");
    scopesDataOffsetField       = type.getCIntegerField("_scopes_data_offset");
    scopesPCsOffsetField        = type.getCIntegerField("_scopes_pcs_offset");
    handlerTableOffsetField     = type.getCIntegerField("_handler_table_offset");
    nulChkTableOffsetField      = type.getCIntegerField("_nul_chk_table_offset");
    nmethodEndOffsetField       = type.getCIntegerField("_nmethod_end_offset");
    firstDependentField         = type.getCIntegerField("_first_dependent");
    numberOfDependentsField     = type.getCIntegerField("_number_of_dependents");
    maxNofMonitorsField         = type.getCIntegerField("_max_nof_monitors");
    maxNofLocalsField           = type.getCIntegerField("_max_nof_locals");
    entryPointField             = type.getAddressField("_entry_point");
    verifiedEntryPointField     = type.getAddressField("_verified_entry_point");
    interpreterEntryPointField  = type.getAddressField("_interpreter_entry_point");
    osrEntryPointField          = type.getAddressField("_osr_entry_point");
    frameStartOffsetField       = type.getCIntegerField("_frame_start_offset");
    lockCountField              = type.getJIntField("_lock_count");
    stackTraversalMarkField     = type.getCIntegerField("_stack_traversal_mark");

    pcDescSize = db.lookupType("PcDesc").getSize();
  }

  public NMethod(Address addr) {
    super(addr);
  }


  // Accessors
  public Address getAddress() {
    return addr;
  }

  public Method getMethod() {
    return (Method) VM.getVM().getObjectHeap().newOop(methodField.getValue(addr));
  }

  // Type info
  public boolean isNMethod()      { return true;                    }
  public boolean isJavaMethod()   { return !getMethod().isNative(); }
  public boolean isNativeMethod() { return getMethod().isNative();  }
  public boolean isOSRMethod()    { return getEntryBCI() != VM.getVM().getInvocationEntryBCI(); }

  /** Boundaries for different parts */
  public Address constantsBegin()       { return instructionsBegin();                                }
  public Address constantsEnd()         { return getEntryPoint();                                    }
  public Address codeBegin()            { return getEntryPoint();                                    }
  public Address codeEnd()              { return headerBegin().addOffsetTo(getExceptionOffset());    }
  public Address exceptionBegin()       { return headerBegin().addOffsetTo(getExceptionOffset());    }
  public Address exceptionEnd()         { return headerBegin().addOffsetTo(getStubOffset());         }
  public Address stubBegin()            { return headerBegin().addOffsetTo(getStubOffset());         }
  public Address stubEnd()              { return headerBegin().addOffsetTo(getScopesDataOffset());   }
  public Address scopesDataBegin()      { return headerBegin().addOffsetTo(getScopesDataOffset());   }
  public Address scopesDataEnd()        { return headerBegin().addOffsetTo(getScopesPCsOffset());    }
  public Address scopesPCsBegin()       { return headerBegin().addOffsetTo(getScopesPCsOffset());    }
  public Address scopesPCsEnd()         { return headerBegin().addOffsetTo(getHandlerTableOffset()); }
  public Address handlerTableBegin()    { return headerBegin().addOffsetTo(getHandlerTableOffset()); }
  public Address handlerTableEnd()      { return headerBegin().addOffsetTo(getNulChkTableOffset());  }
  public Address nulChkTableBegin()     { return headerBegin().addOffsetTo(getNulChkTableOffset());  }
  public Address nulChkTableEnd()       { return headerBegin().addOffsetTo(getNMethodEndOffset());   }

  public int constantsSize()            { return (int) constantsEnd()   .minus(constantsBegin());    }
  public int codeSize()                 { return (int) codeEnd()        .minus(codeBegin());         }
  public int exceptionSize()            { return (int) exceptionEnd()   .minus(exceptionBegin());    }
  public int stubSize()                 { return (int) stubEnd()        .minus(stubBegin());         }
  public int scopesDataSize()           { return (int) scopesDataEnd()  .minus(scopesDataBegin());   }
  public int scopesPCsSize()            { return (int) scopesPCsEnd()   .minus(scopesPCsBegin());    }
  public int handlerTableSize()         { return (int) handlerTableEnd().minus(handlerTableBegin()); }
  public int nulChkTableSize()          { return (int) nulChkTableEnd() .minus(nulChkTableBegin());  }

  public int totalSize() {
    return
      constantsSize()    +
      codeSize()         +
      exceptionSize()    +
      stubSize()         +
      scopesDataSize()   +
      scopesPCsSize()    +
      handlerTableSize() +
      nulChkTableSize();
  }

  public boolean constantsContains   (Address addr) { return constantsBegin()   .lessThanOrEqual(addr) && constantsEnd()   .greaterThan(addr); }
  public boolean codeContains        (Address addr) { return codeBegin()        .lessThanOrEqual(addr) && codeEnd()        .greaterThan(addr); }
  public boolean exceptionContains   (Address addr) { return exceptionBegin()   .lessThanOrEqual(addr) && exceptionEnd()   .greaterThan(addr); }
  public boolean stubContains        (Address addr) { return stubBegin()        .lessThanOrEqual(addr) && stubEnd()        .greaterThan(addr); }
  public boolean scopesDataContains  (Address addr) { return scopesDataBegin()  .lessThanOrEqual(addr) && scopesDataEnd()  .greaterThan(addr); }
  public boolean scopesPCsContains   (Address addr) { return scopesPCsBegin()   .lessThanOrEqual(addr) && scopesPCsEnd()   .greaterThan(addr); }
  public boolean handlerTableContains(Address addr) { return handlerTableBegin().lessThanOrEqual(addr) && handlerTableEnd().greaterThan(addr); }
  public boolean nulChkTableContains (Address addr) { return nulChkTableBegin() .lessThanOrEqual(addr) && nulChkTableEnd() .greaterThan(addr); }

  public int getFirstDependent()     { return (int) firstDependentField.getValue(addr);      }
  public int getNumberOfDependents() { return (int) numberOfDependentsField.getValue(addr);  }
  public int getDependentLimit()     { return getFirstDependent() + getNumberOfDependents(); }

  /** Entry points */
  public Address getEntryPoint()         { return entryPointField.getValue(addr);         }
  public Address getVerifiedEntryPoint() { return verifiedEntryPointField.getValue(addr); }
  
  /** Return the address of the interpreter_entry_point, null if there is not one. */
  public Address getInterpreterEntryPointOrNull() { return interpreterEntryPointField.getValue(addr); }
  // FIXME: add interpreter_entry_point()
  // FIXME: add lazy_interpreter_entry_point() for C2

  // **********
  // * FIXME: * ADD ACCESS TO FLAGS!!!!
  // **********
  // public boolean isInUse();
  // public boolean isAlive();
  // public boolean isNotEntrant();
  // public boolean isZombie();

  // ********************************
  // * MAJOR FIXME: MAJOR HACK HERE *
  // ********************************
  public boolean isZombie() { return false; }

  // public boolean isUnloaded();
  // public boolean isYoung();
  // public boolean isOld();
  // public int     age();
  // public boolean isMarkedForDeoptimization();
  // public boolean isMarkedForUnloading();
  // public boolean isMarkedForReclamation();
  // public int     level();
  // public int     version();

  // FIXME: add mutators for above
  // FIXME: add exception cache access?

  /** On-stack replacement support */
  // FIXME: add mutators
  public int getOSREntryBCI() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(getEntryBCI() != VM.getVM().getInvocationEntryBCI(), "wrong kind of nmethod");
    }
    return getEntryBCI();
  }
  
  public NMethod getLink() {
    return (NMethod) VMObjectFactory.newObject(NMethod.class, linkField.getValue(addr));
  }
  
  /** Tells whether frames described by this nmethod can be
      deoptimized. Note: native wrappers cannot be deoptimized. */
  public boolean canBeDeoptimized() { return isJavaMethod(); }
  
  // FIXME: add inline cache support
  // FIXME: add flush()
  
  public boolean isLockedByVM() { return lockCountField.getValue(addr) > 0; }

  // FIXME: add mark_as_seen_on_stack
  // FIXME: add can_not_entrant_be_converted

  // FIXME: add GC support
  //  void follow_roots_or_mark_for_unloading(bool unloading_occurred, bool& marked_for_unloading);
  //  void follow_root_or_mark_for_unloading(oop* root, bool unloading_occurred, bool& marked_for_unloading);
  //  void preserve_callee_argument_oops(frame fr, const RegisterMap *reg_map, void f(oop*));
  //  void adjust_pointers();
  
  /** Finds a PCDesc with real-pc equal to "pc" */
  public PCDesc getPCDescAt(Address pc, boolean atCall) {
    // FIXME: consider adding cache like the one down in the VM
    for (Address p = scopesPCsBegin(); p.lessThan(scopesPCsEnd()); p = p.addOffsetTo(pcDescSize)) {
      PCDesc pcDesc = new PCDesc(p);
      if (pcDesc.getRealPC(this).equals(pc) && pcDesc.isAtCall() == atCall) {
        return pcDesc;
      }
    }
    return null;
  }

  /** ScopeDesc for an instruction */
  public ScopeDesc getScopeDescAt(Address pc, boolean atCall) {
    PCDesc pd = getPCDescAt(pc, atCall);
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(pd != null, "scope must be present");
    }
    return new ScopeDesc(this, pd.getScopeDecodeOffset());
  }

  /** This is only for use by the debugging system, and is only
      intended for use in the topmost frame, where we are not
      guaranteed to be at a PC for which we have a PCDesc. It finds
      the PCDesc with realPC closest to the current PC. */
  public PCDesc getPCDescNearDbg(Address pc) {
    PCDesc bestGuessPCDesc = null;
    long bestDistance = 0;
    for (Address p = scopesPCsBegin(); p.lessThan(scopesPCsEnd()); p = p.addOffsetTo(pcDescSize)) {
      PCDesc pcDesc = new PCDesc(p);
      // In case pc is null
      long distance = -pcDesc.getRealPC(this).minus(pc);
      if ((bestGuessPCDesc == null) ||
          ((distance >= 0) && (distance < bestDistance))) {
        bestGuessPCDesc = pcDesc;
        bestDistance    = distance;
      }
    }
    return bestGuessPCDesc;
  }

  /** This is only for use by the debugging system, and is only
      intended for use in the topmost frame, where we are not
      guaranteed to be at a PC for which we have a PCDesc. It finds
      the ScopeDesc closest to the current PC. NOTE that this may
      return NULL for compiled methods which don't have any
      ScopeDescs! */
  public ScopeDesc getScopeDescNearDbg(Address pc) {
    PCDesc pd = getPCDescNearDbg(pc);
    if (pd == null) return null;
    return new ScopeDesc(this, pd.getScopeDecodeOffset());
  }

  // FIXME: add getPCOffsetForBCI()

  /** Space reserved on frame for BasicLock structures used in synchronizations */
  public int getMaxNofMonitors() { return (int) maxNofMonitorsField.getValue(addr); }
  
  /** Maximum number of locals includes space allocated for spills */
  public int getMaxNofLocals() { return (int) maxNofLocalsField.getValue(addr); }

  // FIXME: add embeddedOopAt()
  // FIXME: add isDependentOn()
  // FIXME: add isPatchableAt()
  
  /** Support for code generation. Only here for proof-of-concept. */
  public static int getEntryPointOffset()            { return (int) entryPointField.getOffset();            }
  public static int getVerifiedEntryPointOffset()    { return (int) verifiedEntryPointField.getOffset();    }
  public static int getOSREntryPointOffset()         { return (int) osrEntryPointField.getOffset();         }
  public static int getInterpreterEntryPointOffset() { return (int) interpreterEntryPointField.getOffset(); }
  public static int getEntryBCIOffset()              { return (int) entryBCIField.getOffset();              }
  /** NOTE: renamed from "method_offset_in_bytes" */
  public static int getMethodOffset()                { return (int) methodField.getOffset();                }

  public void print() {
    printOn(System.out);
  }

  public String toString() {
    Method method = getMethod();
    return "NMethod for " +
            method.getMethodHolder().getName().asString() + "." + 
            method.getName().asString() + method.getSignature().asString() + "==>n" +
	    super.toString();
  }

  public String flagsToString() {
    // FIXME need access to flags... 
    return "";
  }


  public void printOn(PrintStream tty) {
    Method method = getMethod();
    tty.print("NMethod for " +
              method.getMethodHolder().getName().asString() + "." +
              method.getName().asString() + method.getSignature().asString());
    printComponentsOn(tty);
  }

  //--------------------------------------------------------------------------------
  // Internals only below this point
  //

  private int getEntryBCI()           { return (int) entryBCIField          .getValue(addr); }
  private int getExceptionOffset()    { return (int) exceptionOffsetField   .getValue(addr); }
  private int getStubOffset()         { return (int) stubOffsetField        .getValue(addr); }
  private int getScopesDataOffset()   { return (int) scopesDataOffsetField  .getValue(addr); }
  private int getScopesPCsOffset()    { return (int) scopesPCsOffsetField   .getValue(addr); }
  private int getHandlerTableOffset() { return (int) handlerTableOffsetField.getValue(addr); }
  private int getNulChkTableOffset()  { return (int) nulChkTableOffsetField .getValue(addr); }
  private int getNMethodEndOffset()   { return (int) nmethodEndOffsetField  .getValue(addr); }
}
