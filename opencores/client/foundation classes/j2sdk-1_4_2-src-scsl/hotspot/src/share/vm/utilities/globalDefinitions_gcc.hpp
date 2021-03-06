#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globalDefinitions_gcc.hpp	1.29 03/01/23 12:28:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds compiler-dependent includes,
// globally used constants & types, class (forward)
// declarations and a few frequently used utility functions.

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#ifdef SOLARIS
  #include <ieeefp.h>
#endif // SOLARIS
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>
#ifdef SOLARIS
  #include <thread.h>
#endif // SOLARIS
#include <limits.h>
#include <errno.h>
#ifdef SOLARIS
#include <sys/trap.h>
#include <sys/regset.h>
#include <ucontext.h>
#include <setjmp.h>
#endif // SOLARIS
# ifdef SOLARIS_MUTATOR_LIBTHREAD
# include <sys/procfs.h>
# endif

#ifdef LINUX
  #include <inttypes.h>
  #include <signal.h>
  #include <ucontext.h>
  #include <sys/time.h>
#endif // LINUX

#ifndef	LINUX
// Compiler-specific primitive types
typedef unsigned short     uint16_t;
#ifndef _UINT32_T
#define _UINT32_T
typedef unsigned int       uint32_t;
#endif // _UINT32_T

#if !defined(_SYS_INT_TYPES_H)
#ifndef _UINT64_T
#define _UINT64_T
typedef unsigned long long uint64_t;
#endif // _UINT64_T
// %%%% how to access definition of intptr_t portably in 5.5 onward?
typedef int			intptr_t;
typedef unsigned int		uintptr_t;
// If this gets an error, figure out a symbol XXX that implies the
// prior definition of intptr_t, and add "&& !defined(XXX)" above.
#endif // _SYS_INT_TYPES_H

#endif // !LINUX


// <sys/trap.h> promises that the system will not use traps 16-31
#define ST_RESERVED_FOR_USER_0 0x10

// Additional Java basic types

typedef unsigned char      jubyte;
typedef unsigned short     jushort;
typedef unsigned int       juint;
typedef unsigned long long julong;

//----------------------------------------------------------------------------------------------------
// Special (possibly not-portable) casts
// Cast floats into same-size integers and vice-versa w/o changing bit-pattern
// %%%%%% These seem like standard C++ to me--how about factoring them out? - Ungar

inline jint    jint_cast   (jfloat  x)           { return *(jint*   )&x; }
inline jlong   jlong_cast  (jdouble x)           { return *(jlong*  )&x; }

inline jfloat  jfloat_cast (jint    x)           { return *(jfloat* )&x; }
inline jdouble jdouble_cast(jlong   x)           { return *(jdouble*)&x; }

//----------------------------------------------------------------------------------------------------
// Constant for jlong (specifying an long long canstant is C++ compiler specific)

// Build a 64bit integer constant
#define CONST64(x)  (x ## LL)

const jlong min_jlong = CONST64(0x8000000000000000);
const jlong max_jlong = CONST64(0x7fffffffffffffff);


#ifdef SOLARIS
//----------------------------------------------------------------------------------------------------
// ANSI C++ fixes
// NOTE:In the ANSI committee's continuing attempt to make each version
// of C++ incompatible with the previous version, you can no longer cast
// pointers to functions without specifying linkage unless you want to get
// warnings.
//
// This also means that pointers to functions can no longer be "hidden"
// in opaque types like void * because at the invokation point warnings
// will be generated. While this makes perfect sense from a type safety 
// point of view it causes a lot of warnings on old code using C header
// files. Here are some typedefs to make the job of silencing warnings
// a bit easier.
// 
// The final kick in the teeth is that you can only have extern "C" linkage
// specified at file scope. So these typedefs are here rather than in the
// .hpp for the class (os:Solaris usually) that needs them.

extern "C" {
   typedef int (*int_fnP_thread_t_iP_uP_stack_tP_gregset_t)(thread_t, int*, unsigned *, stack_t*, gregset_t);
   typedef int (*int_fnP_thread_t_i_gregset_t)(thread_t, int, gregset_t);
   typedef int (*int_fnP_thread_t_i)(thread_t, int);
   typedef int (*int_fnP_thread_t)(thread_t);

   typedef int (*int_fnP_cond_tP_mutex_tP_timestruc_tP)(cond_t *cv, mutex_t *mx, timestruc_t *abst);
   typedef int (*int_fnP_cond_tP_mutex_tP)(cond_t *cv, mutex_t *mx);

   // typedef for missing API in libc
   typedef int (*int_fnP_mutex_tP_i_vP)(mutex_t *, int, void *);
   typedef int (*int_fnP_mutex_tP)(mutex_t *);
   typedef int (*int_fnP_cond_tP_i_vP)(cond_t *cv, int scope, void *arg);
   typedef int (*int_fnP_cond_tP)(cond_t *cv);
};
#endif // SOLARIS

//----------------------------------------------------------------------------------------------------
// Debugging

#define DEBUG_EXCEPTION ::abort();

extern "C" void breakpoint();
#define BREAKPOINT ::breakpoint()

// checking for nanness
#ifdef SOLARIS
#ifdef SPARC
inline int g_isnan(float  f) { return isnanf(f); }
#else
// isnanf() broken on Intel Solaris use isnand()
inline int g_isnan(float  f) { return isnand(f); }
#endif
inline int g_isnan(double f) { return isnand(f); }
#elif LINUX
inline int g_isnan(float  f) { return isnanf(f); }
inline int g_isnan(double f) { return isnan(f); }
#else
#error "missing platform-specific definition here"
#endif

// Checking for finiteness

inline int g_isfinite(jfloat  f)                 { return finite(f); }
inline int g_isfinite(jdouble f)                 { return finite(f); }


// Wide characters

inline int wcslen(const jchar* x) { return wcslen((const wchar_t*)x); }


// [RGV] Need to check for accurate sizes 
#ifdef PRODUCT
const int Interpreter_Code_Size = 164 * 1024;
#else
const int Interpreter_Code_Size = 164 * 1024;
#endif // PRODUCT

// Portability macros
#define PRAGMA_INTERFACE             #pragma interface
#define PRAGMA_IMPLEMENTATION        #pragma implementation
#define VALUE_OBJ_CLASS_SPEC

#if defined(__GNUC_MINOR__) && (__GNUC_MINOR__ < 95)
#define TEMPLATE_TABLE_BUG
#endif
#if defined(__GNUC_MINOR__) && (__GNUC_MINOR__ >= 95)
#define CONST_SDM_BUG
#endif

// Formatting.
#ifdef _LP64
#define FORMAT64_MODIFIER "l"
#else // !_LP64
#define FORMAT64_MODIFIER "ll"
#endif // _LP64
