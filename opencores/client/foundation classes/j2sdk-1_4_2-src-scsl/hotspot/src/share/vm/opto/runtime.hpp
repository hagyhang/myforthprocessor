#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)runtime.hpp	1.160 03/01/23 12:19:10 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//------------------------------OptoRuntime------------------------------------
// Opto compiler runtime routines
//
// These are all generated from Ideal graphs.  They are called with the
// Java calling convention.  Internally they call C++.  They are made once at
// startup time and Opto compiles calls to them later.
// Things are broken up into quads: the signature they will be called with,
// the address of the generated code, the corresponding C++ code and an 
// nmethod.

// The signature (returned by "xxx_Type()") is used at startup time by the 
// Generator to make the generated code "xxx_Java".  Opto compiles calls 
// to the generated code "xxx_Java".  When the compiled code gets executed, 
// it calls the C++ code "xxx_C".  The generated nmethod is saved in the 
// CodeCache.  Exception handlers use the nmethod to get the callee-save 
// register OopMaps.
class CallInfo;

typedef const TypeFunc*(*TypeFunc_generator)();

class OptoRuntime : public AllStatic {
  friend class Matcher;  // allow access to stub names

 private:
  // resolving/lookup
  static oop          retrieve_receiver (frame caller, RegisterMap *reg_map);  
  static Handle       find_callee_info  (JavaThread* thread, Bytecodes::Code& bc, CallInfo& callinfo, TRAPS);  
  static Handle       find_callee_info_helper(JavaThread* thread, vframeStream& vfst, Bytecodes::Code& bc, CallInfo& callinfo, TRAPS);
  static methodHandle find_callee_method(JavaThread* thread, TRAPS);  
  static address      resolve_helper    (JavaThread *thread, bool is_virtual, bool is_optimized, TRAPS);  
  static void   inner_resolve_helper    (JavaThread *thread, bool is_virtual, bool is_optimized, methodHandle &callee_method, TRAPS);

  // define stubs
  static address generate_stub(ciEnv* ci_env, TypeFunc_generator gen, address C_function, const char *name, int is_fancy_jump, bool pass_tls, bool save_arguments, bool return_pc);

  // References to generated stubs
  static address _new_Java;
  static address _new_objArray_Java;
  static address _new_typeArray_Java;
  static address _multianewarray1_Java;
  static address _multianewarray2_Java;
  static address _multianewarray3_Java;
  static address _multianewarray4_Java;
  static address _multianewarray5_Java;
  static address _resolve_static_call_Java;
  static address _resolve_virtual_call_Java;
  static address _resolve_opt_virtual_call_Java;
  static address _handle_ic_miss_Java;
  static address _handle_wrong_method_Java;
  static address _throw_null_exception_Java;
  static address _throw_stack_overflow_error_Java;
  static address _throw_div0_exception_Java;
  static address _lazy_c2i_adapter_Java;  
  static address _vtable_must_compile_Java;    
  static address _complete_monitor_locking_Java;
  static address _handle_abstract_method_Java;
  static address _throw_exception_Java;
  static address _handle_exception_Java;
  static address _rethrow_Java;

  // Stubs to support JVMPI
  static address _jvmpi_method_entry_Java;
  static address _jvmpi_method_exit_Java;

# ifdef ENABLE_ZAP_DEAD_LOCALS
  static address _zap_dead_Java_locals_Java; 
  static address _zap_dead_native_locals_Java; 
# endif


  //
  // Implementation of runtime methods
  // =================================
  
  // Allocate storage for a Java instance.  
  static void new_C(klassOop klass, JavaThread *thread);
  
  // Allocate storage for a typeArray  
  static void new_typeArray_C(BasicType elem_type, int len, JavaThread *thread);
  
  // Allocate storage for a objArray  
  static void new_objArray_C(klassOop klass, int len, JavaThread *thread);
  
  // Allocate storage for a multi-dimensional arrays
  // Note: needs to be fixed for arbitrary number of dimensions  
  static void multianewarray1_C(klassOop klass, int len1, JavaThread *thread);  
  static void multianewarray2_C(klassOop klass, int len1, int len2, JavaThread *thread);  
  static void multianewarray3_C(klassOop klass, int len1, int len2, int len3, JavaThread *thread);  
  static void multianewarray4_C(klassOop klass, int len1, int len2, int len3, int len4, JavaThread *thread);  
  static void multianewarray5_C(klassOop klass, int len1, int len2, int len3, int len4, int len5, JavaThread *thread);
  
  // Resolving of calls
  static address resolve_static_call_C     (JavaThread *thread);    
  static address resolve_virtual_call_C    (JavaThread *thread);    
  static address resolve_opt_virtual_call_C(JavaThread *thread);  
  
public:
  // Slow-path Locking and Unlocking    
  static void complete_monitor_locking_C(oop obj, BasicLock* lock, JavaThread* thread);  
  static void complete_monitor_unlocking_C(oop obj, BasicLock* lock);
private:
    
  // abstract method invocation through vtables  
  static void handle_abstract_method_C(JavaThread *thread);

  // handle ic miss with caller being compiled code
  // wrong method handling (inline cache misses, zombie methods)
  static address handle_wrong_method(JavaThread* thread);
  static address handle_wrong_method_ic_miss(JavaThread* thread);

  static address lazy_c2i_adapter_generation_C(JavaThread* thread);      

  // Implicit exception support
  static void throw_null_exception_C(JavaThread* thread);
  static void throw_exception_C     (oop obj, JavaThread* thread);

  // Exception handling
  static address handle_exception_C       (JavaThread* thread);
  static address rethrow_C                (oop exception, JavaThread *thread, address return_pc );
  static void unwind_stack                (JavaThread *thread, CodeBlob *code, frame fr, RegisterMap* reg_map);
  static void deoptimize_caller_frame     (JavaThread *thread, bool doit);


  // CodeBlob support for uncommon traps, safepoints, and deoptimization
  // ===================================================================

  static DeoptimizationBlob*  _deopt_blob;
  static UncommonTrapBlob*    _uncommon_trap_blob;
  static ExceptionBlob*       _exception_blob;    
  static SafepointBlob*       _illegal_instruction_handler_blob;

  // Piece of assembly to remove a deoptimized frame and 
  // insert a number of interpreter frames.  
  static void generate_deopt_blob();

  // Runtime stub to handle an illegal instruction. Used for implementing safepoints
  // in compiled code
  static void generate_illegal_instruction_handler_blob();

  // Runtime stub for provoking an uncommon trap.
  // An uncommon trap deoptimizes the top most frame
  // (compiled frame) and continues the execution.  
  static void generate_uncommon_trap_blob();

  // Runtime stub for handling exception in compiled code. Either
  // finds exception handler in calling method, or unwind stack one level.
  // Done in two stages, due to inter-dependencies between stub-generation
  // and exception stub.
  static void setup_exception_blob();
  static void fill_in_exception_blob();

  static void pd_unwind_stack(JavaThread *thread, frame fr, RegisterMap* reg_map);

  // JVMPI support
  // TEMPORARY: following method was cloned from sharedRuntime.cpp
  static void jvmpi_method_entry_C(methodOop method, oop receiver, JavaThread* thread);
  static void jvmpi_method_exit_C(methodOop method, oop dummy, JavaThread* thread); 

  // zaping dead locals, either from Java frames or from native frames
# ifdef ENABLE_ZAP_DEAD_LOCALS
  static void zap_dead_Java_locals_C(   JavaThread* thread);
  static void zap_dead_native_locals_C( JavaThread* thread);

  static void zap_dead_java_or_native_locals( JavaThread*, bool (*)(frame*));

 public:
   static int ZapDeadCompiledLocals_count;

 private:
# endif
  

 public:

  static bool is_callee_saved_register(MachRegisterNumbers reg);

  // One time only generate runtime code stubs
  static void generate(ciEnv* env);

  // Returns the name of a stub
  static const char* stub_name(address entry);
  
  // access to runtime stubs entry points for java code
  static address new_Java()                              { return _new_Java; }
  static address new_typeArray_Java()                    { return _new_typeArray_Java; }
  static address new_objArray_Java()                     { return _new_objArray_Java; }
  static address multianewarray1_Java()                  { return _multianewarray1_Java; }
  static address multianewarray2_Java()                  { return _multianewarray2_Java; }
  static address multianewarray3_Java()                  { return _multianewarray3_Java; }
  static address multianewarray4_Java()                  { return _multianewarray4_Java; }
  static address multianewarray5_Java()                  { return _multianewarray5_Java; }
  static address resolve_virtual_call_Java()             { return _resolve_virtual_call_Java; }
  static address resolve_opt_virtual_call_Java()         { return _resolve_opt_virtual_call_Java; }
  static address resolve_static_call_Java()              { return _resolve_static_call_Java;  }    
  static address handle_ic_miss_stub()                   { return _handle_ic_miss_Java; }
  static address handle_wrong_method_stub()              { return _handle_wrong_method_Java; }
  static address lazy_c2i_adapter_stub()                 { return _lazy_c2i_adapter_Java; }    
  static address deoptimize_repack_stack()               { return _deopt_blob->unpack(); }  
  static address vtable_must_compile_stub()              { return _vtable_must_compile_Java; }  
  static address handle_abstract_method_stub()           { return _handle_wrong_method_Java; }
  static address throw_null_exception_stub()             { return _throw_null_exception_Java; }
  static address throw_stack_overflow_error_stub()       { return _throw_stack_overflow_error_Java; }
  static address throw_div0_exception_stub()             { return _throw_div0_exception_Java; }
  static address complete_monitor_locking_Java()         { return _complete_monitor_locking_Java;   }  

  // JVMPI support during execution
  static address jvmpi_method_entry_Java()               { return _jvmpi_method_entry_Java; }
  static address jvmpi_method_exit_Java()                { return _jvmpi_method_exit_Java; }

# ifdef ENABLE_ZAP_DEAD_LOCALS
  static address zap_dead_locals_stub(bool is_native)    { return is_native
                                                                  ? _zap_dead_native_locals_Java
                                                                  : _zap_dead_Java_locals_Java; }
  static MachNode* node_to_call_zap_dead_locals(Node* n, int block_num, bool is_native);
# endif


  // Returns the code blob that handles deoptimization
  static DeoptimizationBlob* deoptimization_blob()          { return _deopt_blob;         }  
  static UncommonTrapBlob* uncommon_trap_blob()             { return _uncommon_trap_blob; }  
  static ExceptionBlob*    exception_blob()                 { return _exception_blob; }
  static SafepointBlob*    illegal_exception_handler_blob() { return _illegal_instruction_handler_blob; } 

  // Leaf routines which implement arraycopy
  static void primitive_arraycopy(HeapWord* src, HeapWord* dest, intptr_t len);
  static void oop_arraycopy(HeapWord* src, HeapWord* dest, jint num);
  
  // On-stack replacement
  // Leaf routines used to fetch values from the calling interpreter frame  
  static oop fetch_monitor(int index, BasicLock *dest, address locks_addrs);

  // OSRAdapter generator
  static OSRAdapter* generate_osr_blob(int frame_size, bool returning_fp);

  // Implicit exception support  
  static void throw_div0_exception_C      (JavaThread* thread);  
  static void throw_stack_overflow_error_C(JavaThread* thread);    

  // Exception handling
  static address throw_exception_stub()     { return _throw_exception_Java; }
  static address handle_exception_stub()    { return _handle_exception_Java; }
  static address rethrow_stub()             { return _rethrow_Java; }
  

  // Type functions
  // ======================================================

  static const TypeFunc* new_Type();            // object allocation (slow case)
  static const TypeFunc* new_typeArray_Type();  // newarray (slow case)
  static const TypeFunc* new_objArray_Type ();  // anewarray (slow case)
  static const TypeFunc* multianewarray_Type(int ndim); // multianewarray
  static const TypeFunc* multianewarray1_Type(); // multianewarray
  static const TypeFunc* multianewarray2_Type(); // multianewarray
  static const TypeFunc* multianewarray3_Type(); // multianewarray
  static const TypeFunc* multianewarray4_Type(); // multianewarray
  static const TypeFunc* multianewarray5_Type(); // multianewarray
  static const TypeFunc* resolve_call_Type();  
  static const TypeFunc* complete_monitor_enter_Type();
  static const TypeFunc* complete_monitor_exit_Type();
  static const TypeFunc* uncommon_trap_Type();
  static const TypeFunc* athrow_Type();
  static const TypeFunc* handle_exception_Type();
  static const TypeFunc* rethrow_Type();
  static const TypeFunc* Math_D_D_Type();  // sin,cos & friends
  static const TypeFunc* Math_DD_D_Type(); // mod,pow & friends
  static const TypeFunc* modf_Type();

  static const TypeFunc* flush_windows_Type();

  // leaf arraycopy routine types
  static const TypeFunc* primitive_arraycopy_Type();
  static const TypeFunc* oop_arraycopy_Type();

  // leaf on stack replacement interpreter accessor types
  static const TypeFunc* fetch_int_Type();
  static const TypeFunc* fetch_long_Type();
  static const TypeFunc* fetch_float_Type();
  static const TypeFunc* fetch_double_Type();
  static const TypeFunc* fetch_oop_Type();
  static const TypeFunc* fetch_monitor_Type();

  // JVMPI support
  static const TypeFunc* jvmpi_method_entry_Type();    // ENTRY
  static const TypeFunc* jvmpi_method_exit_Type();     // ENTRY
  
# ifdef ENABLE_ZAP_DEAD_LOCALS
  static const TypeFunc* zap_dead_locals_Type();
# endif


  // Statistics code
#ifndef PRODUCT
 public:
  enum { maxICmiss_count = 100 };
 private:
  static int	 _nof_ic_misses;                 // total # of IC misses
  static int	 _ICmiss_index;                  // length of IC miss histogram
  static int	 _ICmiss_count[maxICmiss_count]; // miss counts
  static address _ICmiss_at[maxICmiss_count];    // miss addresses
  // stats for "normal" compiled calls (non-interface)
  static int	 _nof_normal_calls;              // total # of calls
  static int	 _nof_optimized_calls;           // total # of statically-bound calls
  static int	 _nof_inlined_calls;             // total # of inlined normal calls
  static int	 _nof_megamorphic_calls;         // total # of megamorphic calls (through vtable)
  static int	 _nof_static_calls;              // total # of calls to static methods or super methods (invokespecial)
  static int	 _nof_inlined_static_calls;      // total # of inlined static calls
  // stats for compiled interface calls
  static int	 _nof_interface_calls;           // total # of compiled calls
  static int	 _nof_optimized_interface_calls; // total # of statically-bound interface calls
  static int	 _nof_inlined_interface_calls;   // total # of inlined interface calls
  static int	 _nof_megamorphic_interface_calls;// total # of megamorphic interface calls
  // stats for runtime exceptions
  static int     _nof_removable_exceptions;      // total # of exceptions that could be replaced by branches due to inlining
 public: // for compiler
  static address nof_normal_calls_addr()                { return (address)&_nof_normal_calls; }
  static address nof_optimized_calls_addr()             { return (address)&_nof_optimized_calls; }
  static address nof_inlined_calls_addr()               { return (address)&_nof_inlined_calls; }
  static address nof_static_calls_addr()                { return (address)&_nof_static_calls; }
  static address nof_inlined_static_calls_addr()        { return (address)&_nof_inlined_static_calls; }
  static address nof_megamorphic_calls_addr()           { return (address)&_nof_megamorphic_calls; }
  static address nof_interface_calls_addr()             { return (address)&_nof_interface_calls; }
  static address nof_optimized_interface_calls_addr()   { return (address)&_nof_optimized_interface_calls; }
  static address nof_inlined_interface_calls_addr()     { return (address)&_nof_inlined_interface_calls; }
  static address nof_megamorphic_interface_calls_addr() { return (address)&_nof_megamorphic_interface_calls; }
  static void print_call_statistics(int comp_total);
 private:
  static int _new_ctr;                     // 'new' object requires GC
  static int _new_type_ctr;                // 'new' array requires GC
  static int _new_obj_ctr;                 // 'new' object array requires GC
  static int _multi1_ctr, _multi2_ctr, _multi3_ctr, _multi4_ctr, _multi5_ctr; 
  static int _ic_miss_ctr;                 // inline cache miss in compiled
  static int _wrong_method_ctr;            // uninitialized call site
  static int _resolve_static_ctr;          // uninitialized static call site
  static int _resolve_virtual_ctr;         // uninitialized virtual call site
  static int _resolve_opt_virtual_ctr;     // uninitialized opt virtual call
  static int _mon_enter_stub_ctr;          // monitor enter stub
  static int _mon_exit_stub_ctr;           // monitor exit stub
  static int _mon_enter_ctr;               // monitor enter slow
  static int _mon_exit_ctr;                // monitor exit slow
  static int _vtable_c2c_ctr;              // vtable compiled-to-compiled  
  static int _prim_array_copy_ctr;         // Slow-path primitive array copy
  static int _oop_array_copy_ctr;          // Slow-path oop array copy
  static int _c2i_adapter_ctr;             // c2i adapter for vtables
  static int _throw_null_ctr;              // implicit null throw
  static int _find_handler_ctr;            // find exception handler
  static int _rethrow_ctr;                 // rethrow exception
 public:
  static int _compress_i2c2i_ctr;          // Remove a paired I2C/C2I adapter
  static void trace_ic_miss(address at);
  static void print_statistics();
#endif 

};
