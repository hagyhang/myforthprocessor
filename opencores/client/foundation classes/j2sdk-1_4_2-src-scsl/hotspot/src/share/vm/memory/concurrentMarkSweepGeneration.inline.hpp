#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)concurrentMarkSweepGeneration.inline.hpp	1.15 03/01/23 12:07:47 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline void CMSBitMap::clearAll() {
  assert_lock_strong(&_lock);
  _bm.clear();
  return;
}

inline size_t CMSBitMap::heapWordToOffset(HeapWord* addr) const {
  return (pointer_delta(addr, _bmStartWord)) >> _shifter;
}

inline HeapWord* CMSBitMap::offsetToHeapWord(size_t offset) const {
  return _bmStartWord + (offset << _shifter);
}

inline size_t CMSBitMap::heapWordDiffToOffsetDiff(size_t diff) const {
  assert((diff & ((1 << _shifter) - 1)) == 0, "argument check");
  return diff >> _shifter;
}

inline void CMSBitMap::mark(HeapWord* addr) {
  assert_lock_strong(lock());
  assert(_bmStartWord <= addr && addr < (_bmStartWord + _bmWordSize),
         "outside underlying space?");
  _bm.set_bit(heapWordToOffset(addr));
}

inline void CMSBitMap::par_mark(HeapWord* addr) {
  assert_locked();
  assert(_bmStartWord <= addr && addr < (_bmStartWord + _bmWordSize),
         "outside underlying space?");
  _bm.par_at_put(heapWordToOffset(addr), true);
}

inline void CMSBitMap::par_clear(HeapWord* addr) {
  assert_locked();
  assert(_bmStartWord <= addr && addr < (_bmStartWord + _bmWordSize),
         "outside underlying space?");
  _bm.par_at_put(heapWordToOffset(addr), false);
}

inline void CMSBitMap::markRange(MemRegion mr) {
  assert_locked();
  mr = mr.intersection(MemRegion(_bmStartWord, _bmWordSize));
  assert(!mr.is_empty(), "unexpected empty region");
  // convert address range into offset range
  size_t start_ofs = heapWordToOffset(mr.start());
  size_t end_ofs = heapWordToOffset(mr.end());
  // Range size is usually just 1 bit.
  _bm.set_range(start_ofs, end_ofs, BitMap::small_range);
}

inline void CMSBitMap::clearRange(MemRegion mr) {
  assert_locked();
  mr = mr.intersection(MemRegion(_bmStartWord, _bmWordSize));
  assert(!mr.is_empty(), "unexpected empty region");
  // convert address range into offset range
  size_t start_ofs = heapWordToOffset(mr.start());
  size_t end_ofs = heapWordToOffset(mr.end());
  // Range size is usually just 1 bit.
  _bm.clear_range(start_ofs, end_ofs, BitMap::small_range);
}

inline void CMSBitMap::par_markRange(MemRegion mr) {
  assert_locked();
  mr = mr.intersection(MemRegion(_bmStartWord, _bmWordSize));
  assert(!mr.is_empty(), "unexpected empty region");
  // convert address range into offset range
  size_t start_ofs = heapWordToOffset(mr.start());
  size_t end_ofs = heapWordToOffset(mr.end());
  // Range size is usually just 1 bit.
  _bm.par_set_range(start_ofs, end_ofs, BitMap::small_range);
}

// Starting at "addr" (inclusive) return a memory region
// corresponding to the first maximally contiguous marked ("1") region.
inline MemRegion CMSBitMap::getAndClearMarkedRegion(HeapWord* addr) {
  return getAndClearMarkedRegion(addr, endWord());
}

// Starting at "start_addr" (inclusive) return a memory region
// corresponding to the first maximal contiguous marked ("1") region
// strictly less than end_addr.
inline MemRegion CMSBitMap::getAndClearMarkedRegion(HeapWord* start_addr,
                                                    HeapWord* end_addr) {
  HeapWord *start, *end;
  assert_locked();
  start = getNextMarkedWordAddress  (start_addr, end_addr);
  end   = getNextUnmarkedWordAddress(start,      end_addr);
  assert(start <= end, "Consistency check");
  MemRegion mr(start, end);
  if (!mr.is_empty()) {
    clearRange(mr);
  }
  return mr;
}

inline bool CMSBitMap::isMarked(HeapWord* addr) const {
  assert_locked();
  assert(_bmStartWord <= addr && addr < (_bmStartWord + _bmWordSize),
         "outside underlying space?");
  return _bm.at(heapWordToOffset(addr));
}

inline bool CMSBitMap::isUnmarked(HeapWord* addr) const {
  assert_locked();
  assert(_bmStartWord <= addr && addr < (_bmStartWord + _bmWordSize),
         "outside underlying space?");
  return !_bm.at(heapWordToOffset(addr));
}

// Return the HeapWord address corresponding to next "1" bit
// (inclusive).
inline HeapWord* CMSBitMap::getNextMarkedWordAddress(HeapWord* addr) const {
  return getNextMarkedWordAddress(addr, endWord());
}

// Return the least HeapWord address corresponding to next "1" bit
// starting at start_addr (inclusive) but strictly less than end_addr.
inline HeapWord* CMSBitMap::getNextMarkedWordAddress(
  HeapWord* start_addr, HeapWord* end_addr) const {
  assert_locked();
  size_t nextOffset = _bm.get_next_one_offset(
                        heapWordToOffset(start_addr),
                        heapWordToOffset(end_addr));
  HeapWord* nextAddr = offsetToHeapWord(nextOffset);
  assert(nextAddr >= start_addr &&
         nextAddr <= end_addr, "get_next_one postcondition");
  assert((nextAddr == end_addr) ||
         isMarked(nextAddr), "get_next_one postcondition");
  return nextAddr;
}


// Return the HeapWord address corrsponding to the next "0" bit
// (inclusive).
inline HeapWord* CMSBitMap::getNextUnmarkedWordAddress(HeapWord* addr) const {
  return getNextUnmarkedWordAddress(addr, endWord());
}

// Return the HeapWord address corrsponding to the next "0" bit
// (inclusive).
inline HeapWord* CMSBitMap::getNextUnmarkedWordAddress(
  HeapWord* start_addr, HeapWord* end_addr) const {
  assert_locked();
  size_t nextOffset = _bm.get_next_zero_offset(
                        heapWordToOffset(start_addr),
                        heapWordToOffset(end_addr));
  HeapWord* nextAddr = offsetToHeapWord(nextOffset);
  assert(nextAddr >= start_addr &&
         nextAddr <= end_addr, "get_next_zero postcondition");
  assert((nextAddr == end_addr) ||
          isUnmarked(nextAddr), "get_next_zero postcondition");
  return nextAddr;
}

inline bool CMSBitMap::isAllClear() const {
  assert_locked();
  return getNextMarkedWordAddress(startWord()) >= endWord();
}

inline void CMSBitMap::iterate(BitMapClosure* cl, HeapWord* left,
                            HeapWord* right) {
  assert_locked();
  left = MAX2(_bmStartWord, left);
  right = MIN2(_bmStartWord + _bmWordSize, right);
  if (right > left) {
    _bm.iterate(cl, heapWordToOffset(left), heapWordToOffset(right));
  }
}

inline void CMSCollector::start_icms() {
  if (CMSIncrementalMode) {
    ConcurrentMarkSweepThread::start_icms();
  }
}

inline void CMSCollector::stop_icms() {
  if (CMSIncrementalMode) {
    ConcurrentMarkSweepThread::stop_icms();
  }
}

inline void CMSCollector::disable_icms() {
  if (CMSIncrementalMode) {
    ConcurrentMarkSweepThread::disable_icms();
  }
}

inline void CMSCollector::enable_icms() {
  if (CMSIncrementalMode) {
    ConcurrentMarkSweepThread::enable_icms();
  }
}

inline void CMSCollector::icms_wait() {
  if (CMSIncrementalMode) {
    cmsThread()->icms_wait();
  }
}

inline void CMSCollector::save_sweep_limits() {
  _cmsGen->save_sweep_limit();
  _permGen->save_sweep_limit();
}

inline bool CMSCollector::is_dead_obj(oop obj) {
  HeapWord* addr = (HeapWord*)obj;
  assert( _cmsGen->cmsSpace()->contains(addr) && _cmsGen->cmsSpace()->block_is_obj(addr) ||
         _permGen->cmsSpace()->contains(addr) && _permGen->cmsSpace()->block_is_obj(addr), "must be object");
  return  CMSPermGenSweepingEnabled &&
          _collectorState == Sweeping &&
         !_markBitMap.isMarked(addr);
}

inline double
CMSStats::exp_avg(double old_avg, double cur_val, unsigned int alpha) {
  return (100.0 - alpha) * old_avg / 100.0 + alpha * cur_val / 100.0;
}

inline size_t
CMSStats::exp_avg(size_t old_avg, size_t cur_val, unsigned int alpha) {
  // Convert to double and back to avoid integer overflow.
  return (size_t)exp_avg((double)old_avg, (double)cur_val, alpha);
}
    
inline bool CMSStats::valid() const {
  return _valid_bits == _ALL_VALID;
}

inline void CMSStats::record_gc0_begin() {
  if (_gc0_begin_time.is_updated()) {
    double last_gc0_period = _gc0_begin_time.seconds();
    _gc0_period = exp_avg(_gc0_period, last_gc0_period, _gc0_alpha);
    _gc0_alpha = _saved_alpha;
    _valid_bits |= _GC0_VALID;
  }
  _cms_used_at_gc0_begin = _cms_gen->cmsSpace()->used();

  _gc0_begin_time.update();
}

inline void CMSStats::record_gc0_end(size_t cms_gen_bytes_used) {
  double last_gc0_duration = _gc0_begin_time.seconds();
  _gc0_duration = exp_avg(_gc0_duration, last_gc0_duration, _gc0_alpha);

  // Amount promoted.
  _cms_used_at_gc0_end = cms_gen_bytes_used;
  size_t promoted_bytes = _cms_used_at_gc0_end - _cms_used_at_gc0_begin;
  _gc0_promoted = exp_avg(_gc0_promoted, promoted_bytes, _gc0_alpha);

  // Amount directly allocated.
  size_t allocated_bytes = _cms_gen->direct_allocated_words() * HeapWordSize;
  _cms_gen->reset_direct_allocated_words();
  _cms_allocated = exp_avg(_cms_allocated, allocated_bytes, _gc0_alpha);
}

inline void CMSStats::record_cms_begin() {
  _cms_timer.stop();

  // This is just an approximate value, but is good enough.
  _cms_used_at_cms_begin = _cms_used_at_gc0_end;

  _cms_period = exp_avg(_cms_period, _cms_timer.seconds(), _cms_alpha);
  _cms_begin_time.update();

  _cms_timer.reset();
  _cms_timer.start();
}

inline void CMSStats::record_cms_end() {
  _cms_timer.stop();

  double cur_duration = _cms_timer.seconds();
  _cms_duration = exp_avg(_cms_duration, cur_duration, _cms_alpha);

  // Avoid division by 0.
  const size_t cms_used_mb = MAX2(_cms_used_at_cms_begin / M, (size_t)1);
  _cms_duration_per_mb = exp_avg(_cms_duration_per_mb,
				 cur_duration / cms_used_mb,
				 _cms_alpha);

  _cms_end_time.update();
  _cms_alpha = _saved_alpha;
  _allow_duty_cycle_reduction = true;
  _valid_bits |= _CMS_VALID;

  _cms_timer.start();
}

inline double CMSStats::cms_time_since_begin() const {
  return _cms_begin_time.seconds();
}

inline double CMSStats::cms_time_since_end() const {
  return _cms_end_time.seconds();
}

inline double CMSStats::promotion_rate() const {
  assert(valid(), "statistics not valid yet");
  return gc0_promoted() / gc0_period();
}

inline double CMSStats::cms_allocation_rate() const {
  assert(valid(), "statistics not valid yet");
  return cms_allocated() / gc0_period();
}

inline double CMSStats::cms_consumption_rate() const {
  assert(valid(), "statistics not valid yet");
  return (gc0_promoted() + cms_allocated()) / gc0_period();
}

inline unsigned int CMSStats::icms_update_duty_cycle() {
  // Update the duty cycle only if pacing is enabled and the stats are valid
  // (after at least one young gen gc and one cms cycle have completed).
  if (CMSIncrementalPacing && valid()) {
    return icms_update_duty_cycle_impl();
  }
  return _icms_duty_cycle;
}

inline void ConcurrentMarkSweepGeneration::save_sweep_limit() {
  cmsSpace()->save_sweep_limit();
}

inline size_t ConcurrentMarkSweepGeneration::capacity() const {
  return _cmsSpace->capacity();
}

inline size_t ConcurrentMarkSweepGeneration::used() const {
  return _cmsSpace->used();
}

inline size_t ConcurrentMarkSweepGeneration::free() const {
  return _cmsSpace->free();
}

inline MemRegion ConcurrentMarkSweepGeneration::used_region() const {
  return _cmsSpace->used_region();
}

inline MemRegion ConcurrentMarkSweepGeneration::used_region_at_save_marks() const {
  return _cmsSpace->used_region_at_save_marks();
}

inline void MarkFromRootsClosure::do_yield_check() {
  if (ConcurrentMarkSweepThread::should_yield() &&
      !_collector->foregroundGCIsActive() &&
      _yield) {
    do_yield_work();
  }
}

inline void ScanMarkedObjectsAgainCarefullyClosure::do_yield_check() {
  if (ConcurrentMarkSweepThread::should_yield() &&
      !_collector->foregroundGCIsActive() &&
      _yield) {
    do_yield_work();
  }
}

inline void SweepClosure::do_yield_check(HeapWord* addr) {
  if (ConcurrentMarkSweepThread::should_yield() &&
      !_collector->foregroundGCIsActive() &&
      _yield) {
    do_yield_work(addr);
  }
}

inline void MarkRefsIntoAndScanClosure::do_yield_check() {
  // The conditions are ordered for the remarking phase
  // when _yield is false.
  if (_yield &&
      !_collector->foregroundGCIsActive() &&
      ConcurrentMarkSweepThread::should_yield()) {
    do_yield_work();
  }
}
