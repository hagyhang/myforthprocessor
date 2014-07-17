#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)memRegion.hpp	1.18 03/01/23 12:09:18 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A very simple data structure representing a contigous region
// region of address space.

// Note that MemRegions are passed by value, not by reference.
// The intent is that they remain very small and contain no
// objects.

class MemRegion VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
private:
  HeapWord* _start;
  size_t    _word_size;

public:
  MemRegion() : _start(NULL), _word_size(0) {};
  MemRegion(HeapWord* start, size_t word_size) :
    _start(start), _word_size(word_size) {};
  MemRegion(HeapWord* start, HeapWord* end) :
    _start(start), _word_size(pointer_delta(end, start)) {
    assert(end >= start, "incorrect constructor arguments");
  }
  
  MemRegion(const MemRegion& mr): _start(mr._start), _word_size(mr._word_size) {}
    
  MemRegion intersection(const MemRegion mr2) const;
  // regions must overlap or be adjacent
  MemRegion _union(const MemRegion mr2) const;
  // minus will fail a guarantee if mr2 is interior to this, 
  // since there's no way to return 2 disjoint regions.
  MemRegion minus(const MemRegion mr2) const;

  HeapWord* start() const { return _start; }
  HeapWord* end() const   { return _start + _word_size; }
  HeapWord* last() const  { return _start + _word_size - 1; }

  void set_start(HeapWord* start) { _start = start; }
  void set_end(HeapWord* end)     { _word_size = pointer_delta(end, _start); }
  void set_word_size(size_t word_size) {
    _word_size = word_size;
  }

  bool contains(const MemRegion mr2) const {
    return _start <= mr2._start && end() >= mr2.end();
  }
  bool contains(const void* addr) const {
    return addr >= (void*)_start && addr < (void*)end();
  }
  bool equals(const MemRegion mr2) const {
    // first disjunct since we do not have a canonical empty set
    return ((is_empty() && mr2.is_empty()) ||
            (start() == mr2.start() && end() == mr2.end()));
  }

  size_t byte_size() const { return _word_size * sizeof(HeapWord); }
  size_t word_size() const { return _word_size; }

  bool is_empty() const { return word_size() == 0; }
};

// For iteration over MemRegion's.

class MemRegionClosure : public StackObj {
public:
  virtual void do_MemRegion(MemRegion mr) = 0;
};

// A ResourceObj version of MemRegionClosure

class MemRegionClosureRO: public MemRegionClosure {
public:
  void* operator new(size_t size, ResourceObj::allocation_type type) {
	return ResourceObj::operator new(size, type);
  }
  void* operator new(size_t size, Arena *arena) {
	return ResourceObj::operator new(size, arena);
  }
  void* operator new(size_t size) {
	return ResourceObj::operator new(size);
  }

  void  operator delete(void* p) {} // nothing to do
};