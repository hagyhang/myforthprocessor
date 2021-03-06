#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cardTableRS.hpp	1.16 03/01/23 12:07:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Space;
class OopsInGenClosure;
class DirtyCardToOopClosure;

// This kind of "GenRemSet" uses a card table both as shared data structure 
// for a mod ref barrier set and for the rem set information.

class CardTableRS: public GenRemSet {
  friend class VMStructs;
  // Below are private classes used in impl.
  friend class VerifyCTSpaceClosure;  
  friend class ClearNoncleanCardWrapper;

  static jbyte clean_card_val() {
    return CardTableModRefBS::clean_card;
  }

  static bool
  card_is_dirty_wrt_gen_iter(jbyte cv) {
    return CardTableModRefBS::card_is_dirty_wrt_gen_iter(cv);
  }

  CardTableModRefBSForCTRS _ct_bs;


  // Clean the bytes corresponding to "mr" (all of which must be covered.)
  void clear_MemRegion(MemRegion mr);

  virtual void younger_refs_in_space_iterate(Space* sp, OopsInGenClosure* cl);

#ifndef PRODUCT
  void verify_space(Space* s, HeapWord* gen_start);
#endif

  enum ExtendedCardValue {
    youngergen_card   = CardTableModRefBS::CT_MR_BS_last_reserved + 1,
    // These are for parallel collection.
    // There are three P (parallel) youngergen card values.  In general, this 
    // needs to be more than the number of generations (including the perm
    // gen) that might have younger_refs_do invoked on them separately.  So 
    // if we add more gens, we have to add more values.
    youngergenP1_card  = CardTableModRefBS::CT_MR_BS_last_reserved + 2,
    youngergenP2_card  = CardTableModRefBS::CT_MR_BS_last_reserved + 3,
    youngergenP3_card  = CardTableModRefBS::CT_MR_BS_last_reserved + 4,
    cur_youngergen_and_prev_nonclean_card =
      CardTableModRefBS::CT_MR_BS_last_reserved + 5
  };

  // An array that contains, for each generation, the card table value last 
  // used as the current value for a younger_refs_do iteration of that
  // portion of the table.  (The perm gen is index 0; other gens are at
  // their level plus 1.  They youngest gen is in the table, but will
  // always have the value "clean_card".)
  jbyte* _last_cur_val_in_gen;
  
  jbyte _cur_youngergen_card_val;

  jbyte cur_youngergen_card_val() {
    return _cur_youngergen_card_val;
  }
  void set_cur_youngergen_card_val(jbyte v) {
    _cur_youngergen_card_val = v;
  }
  bool is_prev_youngergen_card_val(jbyte v) {
    return
      youngergen_card <= v &&
      v < cur_youngergen_and_prev_nonclean_card &&
      v != _cur_youngergen_card_val;
  }
  // Return a youngergen_card_value that is not currently in use.
  jbyte find_unused_youngergenP_card_value();

public:
  CardTableRS(MemRegion whole_heap, int max_covered_regions);

  // *** GenRemSet functions.
  GenRemSet::Name rs_kind() { return GenRemSet::CardTable; }

  CardTableRS* as_CardTableRS() { return this; }

  CardTableModRefBS* ct_bs() { return &_ct_bs; }

  // Override.
  void prepare_for_younger_refs_iterate(bool parallel);

  // Card table entries are cleared before application; "blk" is
  // responsible for dirtying if the oop is still older-to-younger after
  // closure application.
  void younger_refs_iterate(Generation* g, OopsInGenClosure* blk);

  void inline_write_ref_field_gc(oop* field, oop new_val) {
    jbyte* byte = _ct_bs.byte_for(field);
    *byte = youngergen_card;
  }
  void write_ref_field_gc_work(oop* field, oop new_val) {
    inline_write_ref_field_gc(field, new_val);
  }

  // Override.  Might want to devirtualize this in the same fashion as
  // above.  Ensures that the value of the card for field says that it's
  // a younger card in the current collection.
  virtual void write_ref_field_gc_par(oop* field, oop new_val);

  void resize_covered_region(MemRegion new_region);

  bool is_aligned(HeapWord* addr) {
    return _ct_bs.is_card_aligned(addr);
  }

  void verify()  PRODUCT_RETURN;

  void clear(MemRegion mr);

  void clear_into_gen_and_younger(Generation* gen); 

  void invalidate(MemRegion mr) { _ct_bs.invalidate(mr); }

  static uintx ct_max_alignment_constraint() {
    return CardTableModRefBS::ct_max_alignment_constraint();
  }

  jbyte* byte_for(void* p)     { return _ct_bs.byte_for(p); }
  jbyte* byte_after(void* p)   { return _ct_bs.byte_after(p); }
  HeapWord* addr_for(jbyte* p) { return _ct_bs.addr_for(p); }

  bool is_prev_nonclean_card_val(jbyte v) {
    return
      youngergen_card <= v &&
      v <= cur_youngergen_and_prev_nonclean_card &&
      v != _cur_youngergen_card_val;
  }

  static bool youngergen_may_have_been_dirty(jbyte cv) {
    return cv == CardTableRS::cur_youngergen_and_prev_nonclean_card;
  }

};
