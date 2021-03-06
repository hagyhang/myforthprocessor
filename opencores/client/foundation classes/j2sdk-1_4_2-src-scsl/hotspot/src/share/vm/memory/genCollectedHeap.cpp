#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)genCollectedHeap.cpp	1.106 03/01/23 12:08:17 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_genCollectedHeap.cpp.incl"

GenCollectedHeap* GenCollectedHeap::_gch;
NOT_PRODUCT(size_t GenCollectedHeap::_skip_header_HeapWords = 0;)

// The set of potentially parallel tasks in strong root scanning.
enum GCH_process_strong_roots_tasks {
  CH_PS_Universe_oops_do,
  CH_PS_JNIHandles_oops_do,
  CH_PS_ObjectSynchronizer_oops_do,
  CH_PS_FlatProfiler_oops_do,
  CH_PS_SystemDictionary_oops_do,
  CH_PS_jvmdi_oops_do,
  CH_PS_vmSymbols_oops_do,
  // Leave this one last.
  CH_PS_NumElements,
  // We probably want to parallelize both of these internally, but for now...
  GCH_PS_younger_gens,
  // Leave this one last.
  GCH_PS_NumElements
};

GenCollectedHeap::GenCollectedHeap(CollectorPolicy *policy) :
  SharedHeap(policy),
  _gen_process_strong_tasks(new SubTasksDone(GCH_PS_NumElements))
{
  if (_gen_process_strong_tasks == NULL ||
      !_gen_process_strong_tasks->valid()) {
    vm_exit_during_initialization("Failed necessary allocation.");
  }
  assert(policy != NULL, "Sanity check");
}

void GenCollectedHeap::initialize() {
  int i;
  _n_gens = collector_policy()->number_of_generations();

  // While there are no constraints in the GC code that HeapWordSize
  // be any particular value, there are multiple other areas in the
  // system which believe this to be true (e.g. oop->object_size in some
  // cases incorrectly returns the size in wordSize units rather than
  // HeapWordSize).
  guarantee(HeapWordSize == wordSize, "HeapWordSize must equal wordSize");

  // The heap must be at least as aligned as generations.  (Since train
  // cars cannot require more alignment than generations, this implies that
  // that alignement constraint is also met.)
  int alignment = Generation::GenGrain;

  _gen_specs = collector_policy()->generations();

  // Make sure the sizes are all aligned.
  for (i = 0; i < _n_gens; i++) {
    _gen_specs[i]->set_init_size(align_size_up(_gen_specs[i]->init_size(), alignment));
    _gen_specs[i]->set_max_size(align_size_up(_gen_specs[i]->max_size(), alignment));
  }

  PermanentGenerationSpec *perm_gen_spec = collector_policy()->permanent_generation();
  perm_gen_spec->set_init_size(align_size_up(perm_gen_spec->init_size(), alignment));
  perm_gen_spec->set_max_size(align_size_up(perm_gen_spec->max_size(), alignment));

  // Now figure out the total size.
  size_t total_reserved = 0;
  for (i = 0; i < _n_gens; i++) {
    total_reserved += _gen_specs[i]->max_size();
  }
  total_reserved += perm_gen_spec->max_size();

  // In MPSS, if the total heap size is not aligned to LargePageSizeInBytes 
  // then anything left over will use standard page size. 
  if (UseISM || UsePermISM) { 
    total_reserved = round_to(total_reserved, LargePageSizeInBytes); 
  } 

  ReservedSpace heap_rs(total_reserved, alignment, UseISM || UsePermISM); 
  if (!heap_rs.is_reserved()) {
    vm_exit_during_initialization("Could not reserve enough space for object heap");
  }
  _reserved = MemRegion((HeapWord*)heap_rs.base(),
			(HeapWord*)(heap_rs.base() + heap_rs.size()));

  // It is important to do this in a way such that concurrent readers can't
  // temporarily think somethings in the heap.  (Seen this happen in asserts.)
  _reserved.set_word_size(0);
  _reserved.set_start((HeapWord*)heap_rs.base());
  _reserved.set_end((HeapWord*)(heap_rs.base() + heap_rs.size()));

  _rem_set = collector_policy()->create_rem_set(_reserved, n_gens()+1);  
  set_barrier_set(rem_set()->bs());
  _gch = this;

  for (i = 0; i < _n_gens; i++) {
    ReservedSpace this_rs = heap_rs.first_part(_gen_specs[i]->max_size());
    _gens[i] = _gen_specs[i]->init(this_rs, i, rem_set());
    heap_rs = heap_rs.last_part(_gen_specs[i]->max_size());
  }
  _perm_gen = perm_gen_spec->init(heap_rs, PermSize, rem_set());

  clear_incremental_collection_will_fail();

  // If we are running CMS, create the collector responsible
  // for collecting the CMS generations.
  if (collector_policy()->is_concurrent_mark_sweep_policy()) {
    assert(_gens[1]->kind() == Generation::ConcurrentMarkSweep &&
           _perm_gen->as_gen()->kind() == Generation::ConcurrentMarkSweep,
           "Unexpected generation kinds");
    // Skip two header words in the block content verification
    NOT_PRODUCT(_skip_header_HeapWords = CMSCollector::skip_header_HeapWords();)
    CMSCollector* collector = new CMSCollector(
      (ConcurrentMarkSweepGeneration*)_gens[1],
      (ConcurrentMarkSweepGeneration*)_perm_gen->as_gen(),
      _rem_set->as_CardTableRS());
    assert(collector != NULL, "couldn't create CMS collector");
  }
}

void GenCollectedHeap::post_initialize() {
  ref_processing_init();
}

void GenCollectedHeap::ref_processing_init() {
  SharedHeap::ref_processing_init();
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->ref_processor_init();
  }
}

size_t GenCollectedHeap::capacity() const {
  size_t res = 0;
  for (int i = 0; i < _n_gens; i++) {
    res += _gens[i]->capacity();
  }
  return res;
}

size_t GenCollectedHeap::used() const {
  size_t res = 0;
  for (int i = 0; i < _n_gens; i++) {
    res += _gens[i]->used();
  }
  return res;
}

size_t GenCollectedHeap::max_capacity() const {
  size_t res = 0;
  for (int i = 0; i < _n_gens; i++) {
    res += _gens[i]->max_capacity();
  }
  return res;
}

#ifndef PRODUCT
// Override of memory state checking method in CollectedHeap:
// Some collectors (CMS for example) can't have badHeapWordVal written
// in the first two words of an object. (For instance , in the case of
// CMS these words hold state used to synchronize between certain
// (concurrent) GC steps and direct allocating mutators.)
// The skip_header_HeapWords() method below, allows us to skip
// over the requisite number of HeapWord's. Note that (for
// generational collectors) this means that those many words are
// skipped in each object, irrespective of the generation in which
// that object lives. The resultant loss of precision seems to be
// harmless and the pain of avoiding that imprecision appears somewhat
// higher than we are prepared to pay for such rudimentary debugging
// support.
void GenCollectedHeap::check_for_non_bad_heap_word_value(HeapWord* addr,
                                                         size_t size) {
  if (CheckMemoryInitialization && ZapUnusedHeapArea) {
    // We are asked to check a size in HeapWords, 
    // but the memory is mangled in juint words.
    const size_t count = size * (HeapWordSize / sizeof(juint));
    juint* start = (juint*) (addr + skip_header_HeapWords());
    for (juint* slot = start; slot < start + count; slot += 1) {
      assert(*slot == badHeapWordVal,
             "Found non badHeapWordValue in pre-allocation check");
    }
  }
}
#endif 

HeapWord* GenCollectedHeap::attempt_allocation(size_t size,
					       bool is_large_noref,
                                               bool is_tlab,
					       bool first_only) {
  HeapWord* res;
  for (int i = 0; i < _n_gens; i++) {
    if (_gens[i]->should_allocate(size, is_large_noref, is_tlab)) {
      res = _gens[i]->allocate(size, is_large_noref, is_tlab);
      if (res != NULL) return res;
      else if (first_only) break;
    }
  }
  // Otherwise...
  return NULL;
}

HeapWord* GenCollectedHeap::mem_allocate(size_t size,
                                         bool is_large_noref,
                                         bool is_tlab) {
  return collector_policy()->mem_allocate_work(size,
                                               is_large_noref,
                                               is_tlab);
}

void GenCollectedHeap::do_collection(bool  full,
                                     bool   clear_all_soft_refs,
                                     size_t size,
				     bool   is_large_noref,
                                     bool   is_tlab,
				     int    max_level,
                                     bool&  notify_ref_lock) {
  bool prepared_for_verification = false;
  notify_ref_lock = false;
  ResourceMark rm;

  assert(SafepointSynchronize::is_at_safepoint(), "should be at safepoint");
  assert(Thread::current() == VMThread::vm_thread(), "should be in vm thread");
  assert(Heap_lock->is_locked(), "the requesting thread should have the Heap_lock");
  guarantee(!is_gc_active(), "collection is not reentrant");
  assert(max_level < n_gens(), "sanity check");

  if (GC_locker::is_active()) {
    return; // GC is disabled (e.g. JNI GetXXXCritical operation)
  }

  const size_t perm_prev_used = perm_gen()->used();

  { // Call to jvmpi::post_class_unload_events must occur outside of active GC

    FlagSetting fl(_is_gc_active, true);

    // Timing
    TraceTime t(full ? "Full GC " : "GC ", PrintGCDetails, true, gclog_or_tty);

    gc_prologue(full);
    increment_total_collections();

    size_t gch_prev_used = used();

    int starting_level = 0;
    if (full) {
      // Search for the oldest generation which will collect all younger 
      // generations, and start collection loop there.
      for (int i = max_level; i >= 0; i--) {
        if (_gens[i]->full_collects_younger_generations()) {
          starting_level = i;
          break;
        }
      }
    }

    int max_level_collected = starting_level;
    for (int i = starting_level; i <= max_level; i++) {
      if (_gens[i]->should_collect(full, size, is_large_noref, is_tlab)) {
        // Timer for individual generations. Last argument is false: no CR
        TraceTime t1(_gens[i]->short_name(), PrintGCDetails, false, gclog_or_tty);
        TraceCollectorStats tcs(_gens[i]->counters());

        size_t prev_used = _gens[i]->used();
        _gens[i]->stat_record()->invocations++;
        _gens[i]->stat_record()->accumulated_time.start();

        if (PrintGC && Verbose) {
          gclog_or_tty->print("level=%d invoke=%d size=" SIZE_FORMAT,
                     i,
                     _gens[i]->stat_record()->invocations,
                     size*HeapWordSize);
        }

        if (VerifyBeforeGC && i >= VerifyGCLevel && 
            total_collections() >= VerifyGCStartAt) {
          HandleMark hm;  // Discard invalid handles created during verification
	  if (!prepared_for_verification) {
	    prepare_for_verify(); 
            prepared_for_verification = true;
	  }
	  tty->print(" VerifyBeforeGC:");
	  Universe::verify(true);
        }
        COMPILER2_ONLY(DerivedPointerTable::clear());

        // Do collection work
        {
          // Note on ref discovery: For what appear to be historical reasons,
          // GCH enables and disabled (by enqueing) refs discovery.
          // In the future this should be moved into the generation's
          // collect method so that ref discovery and enqueueing concerns
          // are local to a generation. The collect method could return
          // an appropriate indication in the case that notification on
          // the ref lock was needed. This will make the treatment of
          // weak refs more uniform (and indeed remove such concerns
          // from GCH). XXX

          HandleMark hm;  // Discard invalid handles created during gc
          save_marks();   // save marks for all gens
          // We want to discover references, but not process them yet. 
          // This mode is disabled in process_discovered_references if the
          // generation does some collection work, or in
          // enqueue_discovered_references if the generation returns
          // without doing any work.
          ReferenceProcessor* rp = _gens[i]->ref_processor();
          // If the discovery of ("weak") refs in this generation is
          // atomic wrt other collectors in this configuration, we
          // are guaranteed to have empty discovered ref lists.
          if (rp->discovery_is_atomic()) {
            rp->verify_no_references_recorded();
            rp->enable_discovery();
          } else {
            // collect() will enable discovery as appropriate
          }
          _gens[i]->collect(full, clear_all_soft_refs, size, is_large_noref, 
	    is_tlab);
          if (!rp->enqueuing_is_done()) {
            notify_ref_lock |= rp->enqueue_discovered_references();
          } else {
            notify_ref_lock |= rp->read_and_reset_notify_ref_lock();
	    rp->set_enqueuing_is_done(false);
          }
          rp->verify_no_references_recorded();
        }
	max_level_collected = i;

        // Determine if allocation request was met.
        if (size > 0) {
          if (!is_tlab || _gens[i]->supports_tlab_allocation()) {
            if (size*HeapWordSize <= _gens[i]->unsafe_max_alloc_nogc()) {
              size = 0;
            }
          }
        }

        COMPILER2_ONLY(DerivedPointerTable::update_pointers());
  
        _gens[i]->stat_record()->accumulated_time.stop();
  
        if (VerifyAfterGC && i >= VerifyGCLevel &&
            total_collections() >= VerifyGCStartAt) {
          HandleMark hm;  // Discard invalid handles created during verification
	  tty->print(" VerifyAfterGC:");
	  Universe::verify(false);
        }

        if (PrintGCDetails) {
          gclog_or_tty->print(":");
          _gens[i]->print_heap_change(prev_used);
        }
        if (PrintHeapUsageOverTime) {
	  Universe::print_heap_usage_stamp(prev_used, i);
        }
      }
    }

    if (PrintGCDetails) {
      print_heap_change(gch_prev_used);
        
      // Print perm gen info for full GC with PrintGCDetails flag.
      if (full && max_level == n_gens() - 1) {
        print_perm_heap_change(perm_prev_used);
       }
    }

    for (int j = max_level_collected; j >= 0; j -= 1) {
      // Adjust generation sizes.
      _gens[j]->compute_new_size();
    }
    if (full && max_level == n_gens() - 1) {
      // Ask the permanent generation to adjust size for full collections
      perm()->compute_new_size();
    }

    gc_epilogue(full);

  }

  if (ExitAfterGCNum > 0 && total_collections() == ExitAfterGCNum) {
    tty->print_cr("Stopping after GC #%d", ExitAfterGCNum);
    vm_exit(-1);
  }

  // Information about classes unloaded by GC may have been saved by
  // SystemDictionary::follow_roots_phase2(). We post CLASS_UNLOAD
  // events from here because we can lock GC. We always call this
  // function to make sure that the saved memory is released.
  jvmpi::post_class_unload_events();
}
    
HeapWord* GenCollectedHeap::satisfy_failed_allocation(size_t size,
                                                      bool   is_large_noref,
                                                      bool   is_tlab,
                                                      bool&  notify_ref_lock) {
  return collector_policy()->satisfy_failed_allocation(size,
                                                       is_large_noref,
                                                       is_tlab,
                                                       notify_ref_lock);
}

void GenCollectedHeap::set_par_threads(int t) {
  SharedHeap::set_par_threads(t);
  _gen_process_strong_tasks->set_par_threads(t);
}

class AssertIsPermClosure: public OopClosure {
public:
  void do_oop(oop* p) {
    assert((*p) == NULL || (*p)->is_perm(), "Referent should be perm.");
  }
};
static AssertIsPermClosure assert_is_perm_closure;

void GenCollectedHeap::process_strong_roots(int level,
					    bool younger_gens_as_roots,
					    bool collecting_perm_gen,
					    ClassScanningOption cso,
					    OopsInGenClosure* older_gens,
					    OopsInGenClosure* not_older_gens) {
  // General strong roots.
  if (n_par_threads() == 0) change_strong_roots_parity();
  if (!_gen_process_strong_tasks->is_task_claimed(CH_PS_Universe_oops_do))
    Universe::oops_do(not_older_gens);
  // Global (strong) JNI handles
  if (!_gen_process_strong_tasks->is_task_claimed(CH_PS_JNIHandles_oops_do))
    JNIHandles::oops_do(not_older_gens);
  // All threads execute this; the individual threads are task groups.
  if (ParallelGCThreads > 0) {
    Threads::possibly_parallel_oops_do(not_older_gens);
  } else {
    Threads::oops_do(not_older_gens);
  }
  if (!_gen_process_strong_tasks-> is_task_claimed(CH_PS_ObjectSynchronizer_oops_do))
    ObjectSynchronizer::oops_do(not_older_gens);
  if (!_gen_process_strong_tasks->is_task_claimed(CH_PS_FlatProfiler_oops_do))
    FlatProfiler::oops_do(not_older_gens);
  if (!_gen_process_strong_tasks->is_task_claimed(CH_PS_SystemDictionary_oops_do))
    switch (cso) {
    case CSO_AllClasses:
      SystemDictionary::oops_do(not_older_gens);
      break;
    case CSO_SystemClasses:
      SystemDictionary::always_strong_oops_do(not_older_gens);
      break;
    case CSO_None:
      break;
    }

  // Roots that should point only into permanent generation.
  {
    OopClosure* blk = NULL;
    if (collecting_perm_gen) {
      blk = not_older_gens;
    } else {
      debug_only(blk = &assert_is_perm_closure);
    }
    if (blk != NULL) {
      if (jvmdi::enabled()) { 
	if (!_gen_process_strong_tasks->is_task_claimed(CH_PS_jvmdi_oops_do))
	  jvmdi::oops_do(blk);
      }
      if (!_gen_process_strong_tasks->is_task_claimed(CH_PS_vmSymbols_oops_do))
	vmSymbols::oops_do(blk);
    }
  }

  if (!collecting_perm_gen) {
    perm_gen()->younger_refs_iterate(older_gens);
  }

  if (younger_gens_as_roots) {
    if (!_gen_process_strong_tasks->is_task_claimed(GCH_PS_younger_gens)) {
      for (int i = 0; i < level; i++) {
        not_older_gens->set_generation(_gens[i]);
        _gens[i]->oop_iterate(not_older_gens);
      }
      not_older_gens->reset_generation();
    }
  }
  // When collection is parallel, all threads get to cooperate to do
  // older-gen scanning.
  for (int i = level+1; i < _n_gens; i++) {
    older_gens->set_generation(_gens[i]);
    rem_set()->younger_refs_iterate(_gens[i], older_gens);
    older_gens->reset_generation();
  }

  _gen_process_strong_tasks->all_tasks_completed();
}

class AlwaysTrueClosure: public BoolObjectClosure {
public:
  void do_object(oop p) { ShouldNotReachHere(); }
  bool do_object_b(oop p) { return true; }
};
static AlwaysTrueClosure always_true;

void GenCollectedHeap::process_weak_roots(OopClosure* root_closure,
					  OopClosure* non_root_closure) {
  // Global (weak) JNI handles
  JNIHandles::weak_oops_do(&always_true, root_closure);

  NOT_CORE(CodeCache::oops_do(non_root_closure));
  SymbolTable::oops_do(root_closure);
  StringTable::oops_do(root_closure);
  // "Global" "weak" refs
  ReferenceProcessor::oops_do_statics(root_closure);
  // "Local" "weak" refs
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->ref_processor()->oops_do(root_closure);
  }
  perm_gen()->ref_processor()->oops_do(root_closure);
}

#define GCH_SINCE_SAVE_MARKS_ITERATE_DEFN(OopClosureType, nv_suffix)	\
void GenCollectedHeap::							\
oop_since_save_marks_iterate(int level,					\
			     OopClosureType* cur,			\
			     OopClosureType* older) {			\
  _gens[level]->oop_since_save_marks_iterate##nv_suffix(cur);		\
  for (int i = level+1; i < n_gens(); i++) {				\
    _gens[i]->oop_since_save_marks_iterate##nv_suffix(older);		\
  }									\
  perm_gen()->oop_since_save_marks_iterate##nv_suffix(older);		\
}

ALL_SINCE_SAVE_MARKS_CLOSURES(GCH_SINCE_SAVE_MARKS_ITERATE_DEFN)

#undef GCH_SINCE_SAVE_MARKS_ITERATE_DEFN

bool GenCollectedHeap::no_allocs_since_save_marks(int level) {
  for (int i = level; i < _n_gens; i++) {
    if (!_gens[i]->no_allocs_since_save_marks()) return false;
  }
  return perm_gen()->no_allocs_since_save_marks();
}

bool GenCollectedHeap::supports_inline_contig_alloc() const {
  return _gens[0]->supports_inline_contig_alloc();
}

HeapWord** GenCollectedHeap::top_addr() const {
  return _gens[0]->top_addr();
}

HeapWord** GenCollectedHeap::end_addr() const {
  return _gens[0]->end_addr();
}

size_t GenCollectedHeap::unsafe_max_alloc() {
  return _gens[0]->unsafe_max_alloc_nogc();
}

// public collection interfaces

void GenCollectedHeap::collect(GCCause::Cause cause) {
  if (collector_policy()->is_two_generation_policy()) {
    TwoGenerationCollectorPolicy *tgcp = (TwoGenerationCollectorPolicy*)collector_policy();
    // It may be faster to collect gen0 first
    if (tgcp->should_collect_gen0_first(cause)) {
      collect(cause, 0);
    }
  }

  // Collect 
  collect(cause, n_gens() - 1);
}

void GenCollectedHeap::collect(GCCause::Cause cause, int max_level) {
  // The caller doesn't have the Heap_lock
  assert(!Heap_lock->owned_by_self(), "this thread should not own the Heap_lock");
  MutexLocker ml(Heap_lock);
  collect_locked(cause, max_level);
}

void GenCollectedHeap::collect_locked(GCCause::Cause cause) {
  // The caller has the Heap_lock
  assert(Heap_lock->owned_by_self(), "this thread should own the Heap_lock");
  collect_locked(cause, n_gens() - 1);
}

// this is the private collection interface
// The Heap_lock is expected to be held on entry.

void GenCollectedHeap::collect_locked(GCCause::Cause cause, int max_level) {
  // Read the GC count while holding the Heap_lock
  int gc_count_before = total_collections();
  GCCauseSetter x(this, cause);
  {
    MutexUnlocker mu(Heap_lock);  // give up heap lock, execute gets it back
    VM_GenCollectFull op(gc_count_before, max_level);
    VMThread::execute(&op);
  }
}


void GenCollectedHeap::do_full_collection(bool clear_all_soft_refs,
					  int max_level, 
					  bool& notify_ref_lock) {
  do_collection(true                 /* full */,
                clear_all_soft_refs  /* clear_all_soft_refs */,
                0                    /* size */,
                false                /* is_large_noref */,
                false                /* is_tlab */,
                max_level            /* max_level */,
		notify_ref_lock      /* notify_ref_lock */);
}

// Returns "TRUE" iff "p" points into the allocated area of the heap.
bool GenCollectedHeap::is_in(const void* p) const {
  // This might be sped up with a cache of the last generation that
  // answered yes.
  for (int i = 0; i < _n_gens; i++) {
    if (_gens[i]->is_in(p)) return true;
  }
  if (_perm_gen->as_gen()->is_in(p)) return true;
  // Otherwise...
  return false;
}

// Returns "TRUE" iff "p" points into the allocated area of the heap.
bool GenCollectedHeap::is_in_youngest(void* p) {
  return _gens[0]->is_in(p);
}

void GenCollectedHeap::oop_iterate(OopClosure* cl) {
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->oop_iterate(cl);
  }
}

void GenCollectedHeap::oop_iterate(MemRegion mr, OopClosure* cl) {
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->oop_iterate(mr, cl);
  }
}

void GenCollectedHeap::object_iterate(ObjectClosure* cl) {
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->object_iterate(cl);
  }
  perm_gen()->object_iterate(cl);
}

void GenCollectedHeap::object_iterate_since_last_GC(ObjectClosure* cl) {
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->object_iterate_since_last_GC(cl);
  }
}

Space* GenCollectedHeap::space_containing(const void* addr) const {
  for (int i = 0; i < _n_gens; i++) {
    Space* res = _gens[i]->space_containing(addr);
    if (res != NULL) return res;
  }
  Space* res = perm_gen()->space_containing(addr);
  if (res != NULL) return res;
  // Otherwise...
  assert(false, "Could not find containing space");
  return NULL;
}


HeapWord* GenCollectedHeap::block_start(const void* addr) const {
  assert(is_in_reserved(addr), "block_start of address outside of heap");
  for (int i = 0; i < _n_gens; i++) {
    if (_gens[i]->is_in(addr)) {
      return _gens[i]->block_start(addr);
    }
  }
  if (perm_gen()->is_in(addr))
      return perm_gen()->block_start(addr);
  assert(false, "Some generation should contain the address");
  return NULL;
}

size_t GenCollectedHeap::block_size(const HeapWord* addr) const {
  assert(is_in_reserved(addr), "block_size of address outside of heap");
  for (int i = 0; i < _n_gens; i++) {
    if (_gens[i]->is_in(addr)) {
      return _gens[i]->block_size(addr);
    }
  }
  if (perm_gen()->is_in(addr))
      return perm_gen()->block_size(addr);
  assert(false, "Some generation should contain the address");
  return 0;
}

bool GenCollectedHeap::block_is_obj(const HeapWord* addr) const {
  assert(is_in_reserved(addr), "block_is_obj of address outside of heap");
  assert(block_start(addr) == addr, "addr must be a block start");
  for (int i = 0; i < _n_gens; i++) {
    if (_gens[i]->is_in(addr)) {
      return _gens[i]->block_is_obj(addr);
    }
  }
  if (perm_gen()->is_in(addr))
      return perm_gen()->block_is_obj(addr);
  assert(false, "Some generation should contain the address");
  return false;
}

bool GenCollectedHeap::supports_tlab_allocation() const {
  for (int i = 0; i < _n_gens; i += 1) {
    if (_gens[i]->supports_tlab_allocation()) {
      return true;
    }
  }
  return false;
}

size_t GenCollectedHeap::tlab_capacity() const {
  size_t result = 0;
  for (int i = 0; i < _n_gens; i += 1) {
    if (_gens[i]->supports_tlab_allocation()) {
      result += _gens[i]->tlab_capacity();
    }
  }
  return result;
}

HeapWord* GenCollectedHeap::allocate_new_tlab(size_t size) {
  HeapWord* result = mem_allocate(size   /* size */,
                                  false  /* is_large_noref */,
                                  true   /* is_tlab */);
  return result;
}

// Requires "*prev_ptr" to be non-NULL.  Deletes and a block of minimal size
// from the list headed by "*prev_ptr".
static ScratchBlock *removeSmallestScratch(ScratchBlock **prev_ptr) {
  bool first = true;
  size_t min_size = 0;   // "first" makes this conceptually infinite.
  ScratchBlock **smallest_ptr, *smallest;
  ScratchBlock  *cur = *prev_ptr;
  while (cur) {
    assert(*prev_ptr == cur, "just checking");
    if (first || cur->num_words < min_size) {
      smallest_ptr = prev_ptr;
      smallest     = cur;
      min_size     = smallest->num_words;
      first        = false;
    }
    prev_ptr = &cur->next;
    cur     =  cur->next;
  }
  smallest      = *smallest_ptr;
  *smallest_ptr = smallest->next;
  return smallest;
}

// Sort the scratch block list headed by res into decreasing size order,
// and set "res" to the result.
static void sort_scratch_list(ScratchBlock*& list) {
  ScratchBlock* sorted = NULL;
  ScratchBlock* unsorted = list;
  while (unsorted) {
    ScratchBlock *smallest = removeSmallestScratch(&unsorted);
    smallest->next  = sorted;
    sorted          = smallest;
  }
  list = sorted;
}

ScratchBlock* GenCollectedHeap::gather_scratch(Generation* requestor,
					       size_t max_alloc_words) {
  ScratchBlock* res = NULL;
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->contribute_scratch(res, requestor, max_alloc_words);
  }
  sort_scratch_list(res);
  return res;
}

size_t GenCollectedHeap::large_typearray_limit() {
  TwoGenerationCollectorPolicy *policy = 
    (TwoGenerationCollectorPolicy *)collector_policy();
  guarantee(policy->is_two_generation_policy(), "Illegal policy type");
  return policy->large_typearray_limit();
}

// Non-product code
#ifndef PRODUCT

class GenPrepareForVerifyClosure: public GenCollectedHeap::GenClosure {
  void do_generation(Generation* gen) {
    gen->prepare_for_verify();
  }
};

void GenCollectedHeap::prepare_for_verify() {
  ensure_parseability();
  GenPrepareForVerifyClosure blk;
  generation_iterate(&blk, false);
  perm_gen()->prepare_for_verify();
}

#endif

void GenCollectedHeap::generation_iterate(GenClosure* cl,
					  bool old_to_young) {
  if (old_to_young) {
    for (int i = _n_gens-1; i >= 0; i--) {
      cl->do_generation(_gens[i]);
    }
  } else {
    for (int i = 0; i < _n_gens; i++) {
      cl->do_generation(_gens[i]);
    }
  }
}

void GenCollectedHeap::space_iterate(SpaceClosure* cl) {
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->space_iterate(cl, true);
  }
  perm_gen()->space_iterate(cl, true);
}



void GenCollectedHeap::save_marks() {
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->save_marks();
  }
  perm_gen()->save_marks();
}

void GenCollectedHeap::compute_new_generation_sizes(int collectedGen) {
  for (int i = 0; i <= collectedGen; i++) {
    _gens[i]->compute_new_size();
  }
}

GenCollectedHeap* GenCollectedHeap::heap() {
  assert(_gch != NULL, "Uninitialized access to GenCollectedHeap::heap()");
  assert(_gch->kind() == CollectedHeap::GenCollectedHeap, "not a generational heap");
  return _gch;
}


void GenCollectedHeap::prepare_for_compaction() {
  Generation* scanning_gen = _gens[_n_gens-1];
  // Start by compacting into same gen.
  CompactPoint cp(scanning_gen, NULL, NULL); 
  while (scanning_gen != NULL) {
    scanning_gen->prepare_for_compaction(&cp);
    scanning_gen = prev_gen(scanning_gen);
  }
}

#ifndef PRODUCT
void GenCollectedHeap::verify(bool allow_dirty, bool silent) {
  if (!silent) tty->print("permgen ");     
  perm_gen()->verify(allow_dirty);
  for (int i = _n_gens-1; i >= 0; i--) {
    Generation* g = _gens[i];
    if (!silent) { tty->print(g->name()); tty->print(" "); }
    g->verify(allow_dirty);
  }
  if (!silent) tty->print("remset ");
  rem_set()->verify();
}

#endif

void GenCollectedHeap::print() const { print_on(tty); }
void GenCollectedHeap::print_on(outputStream* st) const {
  for (int i = 0; i < _n_gens; i++) {
    _gens[i]->print_on(st);
  }
  perm_gen()->print_on(st);
}

void GenCollectedHeap::print_heap_change(size_t prev_used) const {
  gclog_or_tty->print(" "  SIZE_FORMAT "K"
                      "->" SIZE_FORMAT "K"
                      "("  SIZE_FORMAT "K)",
                      prev_used / K, used() / K, capacity() / K);
}

//New method to print perm gen info with PrintGCDetails flag
void GenCollectedHeap::print_perm_heap_change(size_t perm_prev_used) const {
  gclog_or_tty->print(", [%s :", perm_gen()->short_name());
  perm_gen()->print_heap_change(perm_prev_used);
  gclog_or_tty->print("]");
}

int GenCollectedHeap::addr_to_arena_id(void* addr) {
  int res = 1;
  for (int i = 0; i < _n_gens; i++) {
    int loc = _gens[i]->addr_to_arena_id(addr);
    // Non-negative value means contained address; negative indicates
    // how many arena id's were in the generation.
    if (loc >= 0) return res + loc;
    else res = res - loc;
  }
  if (perm_gen()->is_in_reserved(addr)) return res;
  // Otherwise...
  return 0;
}

class GenGCPrologueClosure: public GenCollectedHeap::GenClosure {
 private:
  bool _full;
 public: 
  void do_generation(Generation* gen) { 
    gen->gc_prologue(_full);
  }
  GenGCPrologueClosure(bool full) : _full(full) {};
};

void GenCollectedHeap::gc_prologue(bool full) {
  if (PrintHeapAtGC){
    gclog_or_tty->print_cr(" {Heap before GC invocations=%d:", total_collections());
    Universe::print();
  }
  NOT_CORE(assert(InlineCacheBuffer::is_empty(), "should have cleaned up ICBuffer");)
  // JVMPI notification
  if (jvmpi::is_event_enabled(JVMPI_EVENT_GC_START)) {
    jvmpi::post_gc_start_event();
  }
  always_do_update_barrier = false;
  // Fill TLAB's and such
  ensure_parseability();
  // Call allocation profiler
  AllocationProfiler::iterate_since_last_gc();
  // Walk generations
  GenGCPrologueClosure blk(full);
  generation_iterate(&blk, false);  // not old-to-young.
  perm_gen()->gc_prologue(full);
};

class GenGCEpilogueClosure: public GenCollectedHeap::GenClosure {
 private:
  bool _full;
 public: 
  void do_generation(Generation* gen) { 
    gen->gc_epilogue(_full);
  }
  GenGCEpilogueClosure(bool full) : _full(full) {};
};

void GenCollectedHeap::gc_epilogue(bool full) {
  clear_incremental_collection_will_fail();
  if (jvmpi::is_event_enabled(JVMPI_EVENT_GC_FINISH)) {
    jvmpi::post_gc_finish_event(used(), capacity());
  }
  // I'm ignoring the "fill_newgen()" call if "alloc_event_enabled"
  // is set.
#ifdef COMPILER2
  assert(DerivedPointerTable::is_empty(), "derived pointer present");
  size_t actual_gap = pointer_delta((HeapWord*) (max_uintx-3), *(end_addr()));
  guarantee(actual_gap > (size_t)FastAllocateSizeLimit, "inline allocation wraps");
#endif

  GenGCEpilogueClosure blk(full);
  generation_iterate(&blk, false);  // not old-to-young.
  perm_gen()->gc_epilogue(full);

  always_do_update_barrier = UseConcMarkSweepGC;

  if (PrintHeapAtGC){
    gclog_or_tty->print_cr(" Heap after GC invocations=%d:", total_collections());
    Universe::print();
    gclog_or_tty->print("} ");
  }
};

class GenEnsureParseabilityClosure: public GenCollectedHeap::GenClosure {
 public:
  void do_generation(Generation* gen) {
    gen->ensure_parseability();
  }
};

void GenCollectedHeap::ensure_parseability() {
  CollectedHeap::ensure_parseability();
  GenEnsureParseabilityClosure ep_cl;
  generation_iterate(&ep_cl, false);
  perm_gen()->ensure_parseability();
}

oop GenCollectedHeap::handle_failed_promotion(Generation* gen,
					      oop obj,
					      size_t obj_size,
					      oop* ref) {
  assert(obj_size == (size_t)obj->size(), "bad obj_size passed in");
  HeapWord* result = NULL;
  bool is_large_noref = obj->is_typeArray() &&
    obj_size >= Universe::heap()->large_typearray_limit();

  // First give each higher generation a chance to allocate the promoted object.
  Generation* allocator = next_gen(gen);
  if (allocator != NULL) {
    do {
      result = allocator->allocate(obj_size, is_large_noref, false);
    } while (result == NULL && (allocator = next_gen(allocator)) != NULL);
  }

  if (result == NULL) {
    // Then give gen and higher generations a chance to expand and allocate the
    // object.
    do {
      result = gen->expand_and_allocate(obj_size, is_large_noref, false);
    } while (result == NULL && (gen = next_gen(gen)) != NULL);
  }

  if (result != NULL) {
    Memory::copy_words_aligned((HeapWord*)obj, result, obj_size);
  }
  return oop(result);
}

class GenTimeOfLastGCClosure: public GenCollectedHeap::GenClosure {
  jlong _time;   // in ms
  jlong _now;    // in ms

 public:
  GenTimeOfLastGCClosure(jlong now) : _time(now), _now(now) { }

  jlong time() { return _time; }

  void do_generation(Generation* gen) {
    _time = MIN2(_time, gen->time_of_last_gc(_now));
  }
};

jlong GenCollectedHeap::millis_since_last_gc() {
  jlong now = os::javaTimeMillis();
  GenTimeOfLastGCClosure tolgc_cl(now);
  // iterate over generations getting the oldest
  // time that a generation was collected
  generation_iterate(&tolgc_cl, false);
  tolgc_cl.do_generation(perm_gen());
  // XXX Despite the assert above, since javaTimeMillis()
  // doesnot guarantee monotonically increasing return
  // values (note, i didn't say "strictly monotonic"),
  // we need to guard against getting back a time
  // later than now. This should be fixed by basing
  // on someting like gethrtime() which guarantees
  // monotonicity. Note that cond_wait() is susceptible
  // to a similar problem, because its interface is
  // based on absolute time in the form of the
  // system time's notion of UCT. See also 4506635
  // for yet another problem of similar nature. XXX
  jlong retVal = now - tolgc_cl.time();
  if (retVal < 0) {
    NOT_PRODUCT(warning("time warp: %d", retVal);)
    return 0;
  }
  return retVal;
}
