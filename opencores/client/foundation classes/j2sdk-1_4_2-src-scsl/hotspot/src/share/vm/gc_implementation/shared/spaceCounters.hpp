#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)spaceCounters.hpp	1.2 03/01/23 12:04:15 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A SpaceCounter is a holder class for performance counters
// that track a space;

class SpaceCounters: public CHeapObj {
  friend class VMStructs;

 private:
  PerfVariable*      _capacity;
  PerfVariable*      _used;

  // Constant PerfData types don't need to retain a reference.
  // However, it's a good idea to document them here.
  // PerfConstant*     _size;

  MutableSpace*     _object_space;
  const char*       _name_space;

 public:

  SpaceCounters(const char* name_space, const char* name,
                int ordinal, size_t max_size, MutableSpace* m);
  
  inline void update_capacity() {
    _capacity->set_value(_object_space->capacity_in_bytes());
  }

  inline void update_used() {
    _used->set_value(_object_space->used_in_bytes());
  }

  inline void update_all() {
    update_used();
    update_capacity();
  }

  const char* name_space() const        { return _name_space; }
};

class MutableSpaceUsedHelper: public PerfLongSampleHelper {
  private:
    MutableSpace* _m;

  public:
    MutableSpaceUsedHelper(MutableSpace* m) : _m(m) { }

    inline jlong take_sample() {
      return _m->used_in_bytes();
    }
};
