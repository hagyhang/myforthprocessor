#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciObject.hpp	1.14 03/01/23 11:57:58 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciObject
//
// This class represents an oop in the HotSpot virtual machine.
// Its subclasses are structured in a hierarchy which mirrors
// an aggregate of the VM's oop and klass hierarchies (see
// oopHierarchy.hpp).  Each instance of ciObject holds a handle
// to a corresponding oop on the VM side and provides routines
// for accessing the information in its oop.  By using the ciObject
// hierarchy for accessing oops in the VM, the compiler ensures
// that it is safe with respect to garbage collection; that is,
// GC and compilation can proceed independently without
// interference.
//
// Within the VM, the oop and klass hierarchies are separate.
// The compiler interface does not preserve this separation --
// the distinction between `klassOop' and `Klass' are not
// reflected in the interface and instead the Klass hierarchy
// is directly modeled as the subclasses of ciKlass.
class ciObject : public ResourceObj {
  CI_PACKAGE_ACCESS
  friend class ciEnv;
  friend class DebugInformationRecorder;

private:
  // A JNI handle referring to an oop in the VM.  This
  // handle may, in a small set of cases, correctly be NULL.
  jobject  _handle;
  ciKlass* _klass;
  int      _ident;

protected:
  ciObject();
  ciObject(oop o);
  ciObject(Handle h);
  ciObject(ciKlass* klass);

  jobject      handle()  const { return _handle; }
  // Get the VM oop that this object holds.
  oop ciObject::get_oop() const {
    assert(_handle != NULL, "null oop");
    return JNIHandles::resolve_non_null(_handle);
  }

  // Virtual behavior of the print() method.
  virtual void print_impl() {}

  virtual const char* type_string() { return "ciObject"; }

public:
  // The klass of this ciObject.
  ciKlass* klass();

  // A number unique to this object.
  int ident();

  // Are two ciObjects equal?
  bool equals(ciObject* obj);

  // A hash value for the convenience of compilers.
  int hash();

  // Tells if this oop has an encoding.  (I.e., is it null or perm?)
  // If it does not have an encoding, the compiler is responsible for
  // making other arrangements for dealing with the oop.
  // See ciEnv::make_perm_array
  bool has_encoding();

  // The address which the compiler should embed into the
  // generated code to represent this oop.  This address
  // is not the true address of the oop -- it will get patched
  // during nmethod creation.
  //
  // Usage note: no address arithmetic allowed.  Oop must
  // be registered with the oopRecorder.
  jobject encoding();

  // What kind of ciObject is this?
  virtual bool is_null_object() const       { return false; }
  virtual bool is_instance()                { return false; }
  virtual bool is_method()                  { return false; }
  virtual bool is_method_data()             { return false; }
  virtual bool is_array()                   { return false; }
  virtual bool is_obj_array()               { return false; }
  virtual bool is_type_array()              { return false; }
  virtual bool is_symbol()                  { return false; }
  virtual bool is_type()                    { return false; }
  virtual bool is_return_address()          { return false; }
  virtual bool is_klass()                   { return false; }
  virtual bool is_instance_klass()          { return false; }
  virtual bool is_method_klass()            { return false; }
  virtual bool is_array_klass()             { return false; }
  virtual bool is_obj_array_klass()         { return false; }
  virtual bool is_type_array_klass()        { return false; }
  virtual bool is_symbol_klass()            { return false; }
  virtual bool is_klass_klass()             { return false; }
  virtual bool is_instance_klass_klass()    { return false; }
  virtual bool is_array_klass_klass()       { return false; }
  virtual bool is_obj_array_klass_klass()   { return false; }
  virtual bool is_type_array_klass_klass()  { return false; }

  // Is this a type or value which has no associated class?
  // It is true of primitive types and null objects.
  virtual bool is_classless() const         { return false; }

  // Is this ciObject a Java Language Object?  That is,
  // is the ciObject an instance or an array
  virtual bool is_java_object()             { return false; }

  // Does this ciObject represent a Java Language class?
  // That is, is the ciObject an instanceKlass or arrayKlass?
  virtual bool is_java_klass()              { return false; }

  // Is this ciObject the ciInstanceKlass representing
  // java.lang.Object()?
  virtual bool is_java_lang_Object()        { return false; }

  // Does this ciObject refer to a real oop in the VM?
  //
  // Note: some ciObjects refer to oops which have yet to be
  // created.  We refer to these as "unloaded".  Specifically,
  // there are unloaded ciMethods, ciObjArrayKlasses, and
  // ciInstanceKlasses.  By convention the ciNullObject is
  // considered loaded, and primitive types are considered loaded.
  bool is_loaded() const {
    return handle() != NULL || is_classless();
  }

  // Subclass casting with assertions.
  ciNullObject*            as_null_object() {
    assert(is_null_object(), "bad cast");
    return (ciNullObject*)this;
  }
  ciInstance*              as_instance() {
    assert(is_instance(), "bad cast");
    return (ciInstance*)this;
  }
  ciMethod*                as_method() {
    assert(is_method(), "bad cast");
    return (ciMethod*)this;
  }
  ciMethodData*            as_method_data() {
    assert(is_method_data(), "bad cast");
    return (ciMethodData*)this;
  }
  ciArray*                 as_array() {
    assert(is_array(), "bad cast");
    return (ciArray*)this;
  }
  ciObjArray*              as_obj_array() {
    assert(is_obj_array(), "bad cast");
    return (ciObjArray*)this;
  }
  ciTypeArray*             as_type_array() {
    assert(is_type_array(), "bad cast");
    return (ciTypeArray*)this;
  }
  ciSymbol*                as_symbol() {
    assert(is_symbol(), "bad cast");
    return (ciSymbol*)this;
  }
  ciType*                  as_type() {
    assert(is_type(), "bad cast");
    return (ciType*)this;
  }
  ciReturnAddress*         as_return_address() {
    assert(is_return_address(), "bad cast");
    return (ciReturnAddress*)this;
  }
  ciKlass*                 as_klass() {
    assert(is_klass(), "bad cast");
    return (ciKlass*)this;
  }
  ciInstanceKlass*         as_instance_klass() {
    assert(is_instance_klass(), "bad cast");
    return (ciInstanceKlass*)this;
  }
  ciMethodKlass*           as_method_klass() {
    assert(is_method_klass(), "bad cast");
    return (ciMethodKlass*)this;
  }
  ciArrayKlass*            as_array_klass() {
    assert(is_array_klass(), "bad cast");
    return (ciArrayKlass*)this;
  }
  ciObjArrayKlass*         as_obj_array_klass() {
    assert(is_obj_array_klass(), "bad cast");
    return (ciObjArrayKlass*)this;
  }
  ciTypeArrayKlass*        as_type_array_klass() {
    assert(is_type_array_klass(), "bad cast");
    return (ciTypeArrayKlass*)this;
  }
  ciSymbolKlass*           as_symbol_klass() {
    assert(is_symbol_klass(), "bad cast");
    return (ciSymbolKlass*)this;
  }
  ciKlassKlass*            as_klass_klass() {
    assert(is_klass_klass(), "bad cast");
    return (ciKlassKlass*)this;
  }
  ciInstanceKlassKlass*    as_instance_klass_klass() {
    assert(is_instance_klass_klass(), "bad cast");
    return (ciInstanceKlassKlass*)this;
  }
  ciArrayKlassKlass*       as_array_klass_klass() {
    assert(is_array_klass_klass(), "bad cast");
    return (ciArrayKlassKlass*)this;
  }
  ciObjArrayKlassKlass*    as_obj_array_klass_klass() {
    assert(is_obj_array_klass_klass(), "bad cast");
    return (ciObjArrayKlassKlass*)this;
  }
  ciTypeArrayKlassKlass*   as_type_array_klass_klass() {
    assert(is_type_array_klass_klass(), "bad cast");
    return (ciTypeArrayKlassKlass*)this;
  }

  // Print debugging output about this ciObject.
  void print();

  // Print debugging output about the oop this ciObject represents.
  void print_oop();
};
