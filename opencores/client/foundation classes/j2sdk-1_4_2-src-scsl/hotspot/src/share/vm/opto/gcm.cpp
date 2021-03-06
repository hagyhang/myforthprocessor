#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gcm.cpp	1.221 03/01/23 12:16:23 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style

#include "incls/_precompiled.incl"
#include "incls/_gcm.cpp.incl"

//------------------------------set_pinned-------------------------------------
// Set the basic block for Nodes pinned into blocks
void PhaseCFG::set_pinned( VectorSet &visited, Node *n ) {
  if( visited.test_set(n->_idx) ) return;

  if( n->pinned() && !_bbs.lookup(n->_idx) ) {  // Pinned?  Nail it down!
    Node *c = n->in(0);
    assert( c, "pinned Node must have Control" );
    while( !c->is_block_start() )
      c = c->in(0);
    Block *b = _bbs[c->_idx];   // Basic block of controlling input
    b->add_inst(n);             // Add guy to basic block
    _bbs.map(n->_idx,b);        // Set basic block of pinned guy
  }
  
  uint max = n->req();          // Count of inputs
  for( uint i=0; i<max; i++ )   // For all inputs
    if( n->in(i) )
      set_pinned( visited, n->in(i) );
}

//------------------------------schedule_early---------------------------------
// Find the earliest Block any instruction can be placed in.  Some instructions
// are pinned into Blocks.  Unpinned instructions can appear in last block in 
// which all their inputs occur.
bool Node::schedule_early( VectorSet &visited, Node_List &roots, Block_Array &bbs ) {
  visited.set(_idx);            // Visited now
  uint plen = len();            // Count of inputs

  // While I am here, go ahead and look for Nodes which are taking control
  // from a is_block_proj Node.  After I inserted RegionNodes to make proper
  // blocks, the control at a is_block_proj more properly comes from the
  // Region being controlled by the block_proj Node.  
  if( in(0) ) {                // Control-dependent?
    const Node *p = in(0)->is_block_proj();
    if( p && p != this ) {      // Control from a block projection?
      // Find trailing Region
      Block *pb = bbs[in(0)->_idx];// Block-projection already has basic block
      if( pb->_num_succs == 1 ) // Only 1 successor?
        set_req(0,pb->_succs[0]->head()); // Easy case; use it
      else {                    // Else must search for successor
        uint max = pb->_nodes.size();
        assert( max > 1, "" );
        uint start = max - pb->_num_succs;
        // Find which output path belongs to projection
	uint i;
        for( i=start; i<max; i++ )
          if( pb->_nodes[i] == in(0) )
            break;
        assert( i < max, "must find" );
        // Change control to match head of successor basic block
        set_req(0,pb->_succs[i-start]->head());
      }
    }
  } else {
    if ( req() == 1 ) { // This guy is a constant with NO inputs?
      set_req(0, Compile::current()->root());
    }
  }

  // First, visit all inputs and force them to get a block.  If an
  // input is already in a block we quit following inputs (to avoid
  // cycles). Instead we put that Node on a worklist to be handled
  // later (since IT'S inputs may not have a block yet).
  uint i;
  for( i=0; i<plen; i++ ) { // For all inputs
    Node *inn = in(i);           // Get input
    if( !inn ) continue;         // Ignore NULL, missing inputs
    if( !bbs.lookup(inn->_idx)){ // Missing block selection?
      if( visited.test(inn->_idx) ) {
	// assert( !visited.test(inn->_idx), "did not schedule early" );
	return false;
      }
      if( !inn->schedule_early(visited,roots,bbs) ) {
	return false;
      }
    } else {                     // Input has block selection
      if( !visited.test_set(inn->_idx) ) { // Input not yet visited?
        roots.push(inn);         // Visit this guy later, using worklist
      }
    }
  }

  // Some instructions are pinned into a block.  These include Region,
  // Phi, Start, Return, and other control-dependent instructions and
  // any projections which depend on them.
  if( pinned() ) return true;

  // Find the last input dominated by all other inputs.
  Block *b = NULL;              // Deepest block
  uint depth = 0;               // Depth in dominator tree of deepest block
  for( i=0; i<plen; i++ ) {     // For all inputs
    Node *inn = in(i);          // Get input
    if( !inn ) continue;        // Ignore NULL, missing inputs
    Block *inb = bbs[inn->_idx];
    assert( inb, "" );
    if( depth < inb->_dom_depth ) {
#ifdef ASSERT
      { Block *tmp = inb;       // Assert that new input is dominated by all 
        while( tmp != b ) {     // previous inputs by seeing that it is 
          if( !tmp ) {          // dominated by the last deepest input.
            // Detected an unschedulable graph.  Print some nice stuff and die.
            tty->print_cr("!!! Unschedulable graph !!!");
            for( uint j=0; j<plen; j++ ) {     // For all inputs
              Node *inn = in(j);           // Get input
              if( !inn ) continue;         // Ignore NULL, missing inputs
              Block *inb = bbs[inn->_idx];
              tty->print("B%d idom=B%d depth=%2d ",inb->_pre_order, 
                inb->_idom ? inb->_idom->_pre_order : 0, inb->_dom_depth);
              inn->dump();
            }
            tty->print("Failing node: ");
            dump();
            assert( 0, "unscheduable graph" );
          }
          tmp = tmp->_idom;      
        }
      }
#endif
      b = inb;                  // Save deepest block
      depth = inb->_dom_depth;
    }
  }

  assert( b, "" );
  bbs.map(_idx,b);              // Set earliest block

  return true;
}

//------------------------------dom_lca----------------------------------------
// Find least common ancestor in dominator tree
static Block *dom_lca( Block *thsi, Block *b ) {
  if( !thsi ) return b;

  while( b->_dom_depth > thsi->_dom_depth )
    b = b->_idom;               // Walk up till b is as high as "this"

  Block *x = thsi;
  while( x->_dom_depth > b->_dom_depth )
    x = x->_idom;               // Walk up till "this" is as high as "b"

  while( x != b ) {             // Walk both up till they are the same
    x = x->_idom;
    b = b->_idom;
  }

  return x;                     // Return the LCA
}

//------------------------------use_dom_lca------------------------------------
// Account for Phis when computing the LCA
static Block *use_dom_lca( Block *thsi, Node *use, Node *def, Block_Array &bbs){
  Block *buse = bbs[use->_idx];
  if( !buse ) return thsi;      // Unused killing Projs have no use block
  const PhiNode *p = use->is_Phi(); // Handy access
  if( !p ) return dom_lca(thsi, buse);
  uint pmax = p->req();         // Number of Phi inputs
  // Why does not this loop just break after finding the matching input to
  // the Phi?  Well...its like this.  I do not have true def-use/use-def
  // chains.  Means I cannot distinguish, from the def-use direction, which
  // of many use-defs lead from the same use to the same def.  That is, this
  // Phi might have several uses of the same def.  Each use appears in a 
  // different predecessor block.  But when I enter here, I cannot distinguish
  // which use-def edge I should find the predecessor block for.  So I find
  // them all.  Means I do a little extra work if a Phi uses the same value
  // more than once.
  for( uint j=1; j<pmax; j++ )  // For all inputs
    if( p->in(j) == def )       // Found matching input?
      thsi = dom_lca( thsi, bbs[buse->pred(j)->_idx] );
  return thsi;
}

//------------------------------hoist_LCA_above_defs--------------------------
// Recursively examine parents to identify if a blocking def may occur between
// current block and 'early' block for load
// If we can show that def occurs on path from use to 'early', 
// return new 'hoisted_LCA' block 
Block *Block::hoist_LCA_above_defs( Block *early, Block *LCA, 
                                    node_idx_t index, Block_Array &bbs ) {
  // if( !this )         return NULL; // no block to process
  assert( this != NULL, "sanity check" );
  if( this == early ) return NULL; // Reached early without hitting def

  uint  pmax  = num_preds();     // parent count, duplicates possible
  bool  cover = false;
  Block *hoisted_LCA = NULL;
  // For all predecessors, one_based, while hoisted LCA hasn't reached 'early'
  for( uint j=1; ((hoisted_LCA != early) && j<pmax); j++ ) {
    Node  *pred   = this->pred( j );
    Block *parent = bbs[ pred->_idx ];
    if( parent->anti_dep_index() == index )  continue;   // Skip if visited
    parent->set_anti_dep(index);

    // Does parent interfere?
    Block *result = NULL;
    if( parent->blocker_index() == index ) {
      // hoisted_LCA is dominator of parent and LCA
      hoisted_LCA = dom_lca(LCA, parent);
      LCA = hoisted_LCA;
      result = LCA->hoist_LCA_above_defs( early, LCA, index, bbs );
    } else {
      result = parent->hoist_LCA_above_defs( early, LCA, index, bbs );
    }
    if( result == NULL ) continue;
    hoisted_LCA = result;
    LCA = hoisted_LCA;
  }

  // Check if I block is only necessary if my children haven't
  if( !hoisted_LCA && (blocker_index() == index) )  hoisted_LCA = LCA;

  return hoisted_LCA;
}


// Remove def-use edges to stores on distinct control-flow paths;
// Return 'false' if no anti-dependences were added, else
// Return updated LCA, without possible-def between it and early_block.
static bool insert_anti_dependences(Block * &LCA, Node *use, Block_Array &bbs) {
  if( !use->check_for_anti_dependence() )
    return false;

  Compile* C = Compile::current();

  // Compute the alias index.  Loads and stores with different alias indices
  // do not need anti-dependence edges.
  uint load_alias_idx = C->get_alias_index(use->adr_type());
#ifdef ASSERT
  if (load_alias_idx == Compile::AliasIdxBot && C->AliasLevel() > 0 &&
      (PrintOpto || VerifyAliases ||
       PrintMiscellaneous && (WizardMode || Verbose))) {
    // Load nodes should not consume all of memory.
    // Reporting a bottom type indicates a bug in adlc.
    // If some particular type of node validly consumes all of memory,
    // sharpen the preceding "if" to exclude it, so we can catch bugs here.
    tty->print_cr("*** Possible Anti-Dependence Bug:  Load consumes all of memory.");
    use->dump(2);
    if (VerifyAliases)  assert(load_alias_idx != Compile::AliasIdxBot, "");
  }
#endif
  assert(load_alias_idx || (use->is_Mach() && use->is_Mach()->ideal_Opcode() == Op_StrComp), "String compare is only known 'use' that does not conflict with any stores");

  if (!C->alias_type(load_alias_idx)->is_rewritable()) {
    // It is impossible to spoil this load by putting stores before it,
    // because we know that the stores will never update the value
    // which the load must witness.
    return false;
  }

  bool inserted_anti_dep = false;
  node_idx_t index = use->_idx;
  Block *early = bbs[use->_idx];

  Node_List nonlocal_stores;    // List of possible anti-deps.

  // Look at uses of the same memory state.
  // Recurse through MergeMem nodes to the stores that use them.
  Node* store = use->in(MemNode::Memory);
  // Insert anti-dependence edges from possible-def to 'use'
  // when the def is in the same block as 'use'.
  // 
  // Label as 'blockers',  any blocks containing possibly-interfering-defs
  ResourceArea *area = Thread::current()->resource_area();
  Node_List worklist_use(area);
  Node_List worklist_state(area);
  DEBUG_ONLY(VectorSet should_not_repeat(area));
  // Provide "phi_inputs" to check if every input to a PhiNode is from the 
  // original memory state.  This indicates a PhiNode for which we do not
  // need to place blockers; placing blockers may be overly conservative.
  // Mechanism: count inputs seen for each entry in worklist_use
  DEBUG_ONLY(GrowableArray<uint> phi_inputs(area, C->unique(),0,0);)
  for (DUIterator i = store->outs(); store->has_out(i); i++) {
    Node *outp = store->out(i);
    worklist_state.push(store);
    worklist_use.push(outp);
    DEBUG_ONLY(if( outp->is_Phi() ) { phi_inputs.at_put(outp->_idx, phi_inputs.at_grow(outp->_idx,0) + 1); } )
    DEBUG_ONLY(should_not_repeat <<= outp->_idx);
  }
  while (worklist_use.size() > 0) {
    Node* state= worklist_state.pop();
    Node* outp = worklist_use.pop();
    uint op = outp->Opcode();
    if (op == Op_MergeMem) {
      for (DUIterator i2 = outp->outs(); outp->has_out(i2); i2++) {
        Node* outp2 = outp->out(i2);
        worklist_state.push(outp);
        worklist_use.push(outp2);
        DEBUG_ONLY(if( outp->is_Phi() ) { phi_inputs.at_put(outp->_idx, phi_inputs.at_grow(outp->_idx,0) + 1); } )
        assert(!(outp2->is_MergeMem() && should_not_repeat.test_set(outp2->_idx)), "do not walk merges twice");
      }
      continue;
    }
    if (op == Op_MachProj || op == Op_Catch)   continue;
    if (outp->check_for_anti_dependence())     continue;  // use-use OK

    // Compute the alias index.  Loads and stores with different alias
    // indices do not need anti-dependence edges.  Wide MemBar's are
    // anti-dependent on everything (except immutable memories).
    const TypePtr *adr_type = outp->adr_type();
    if( !C->can_alias(adr_type, load_alias_idx) )
      continue;

    // Most slow-path runtime calls do NOT modify Java memory, but
    // they can block and so write Raw memory.
    MachNode *muse = outp->is_Mach();
    if( load_alias_idx != Compile::AliasIdxRaw && muse ) {
      // Check for call into the runtime using the Java calling
      // convention (and from there into a wrapper); it has no
      // _method.  Can't do this optimization for Native calls because
      // they CAN write to Java memory.
      if( muse->ideal_Opcode() == Op_CallStaticJava ) {
        assert( muse->is_MachSafePoint(), "" );
        MachSafePointNode *ms = (MachSafePointNode*)muse;
        assert( ms->is_MachCallJava(), "" );
        MachCallJavaNode *mcj = (MachCallJavaNode*)ms;
        if( mcj->_method == NULL ) {
          // These runtime calls do not write to Java visible memory
          // (other than Raw) and so do not require anti-dependence edges.
          continue;
        }
      }
      // Same for SafePoints: they read/write Raw but only read otherwise.
      // This is basically a workaround for SafePoints only defining control
      // instead of control + memory.
      if( muse->ideal_Opcode() == Op_SafePoint ) 
        continue;
    }

    // Identify a block that the current use must be above
    // Instead of finding the LCA of all inputs to a Phi that match 'store',
    // we tag each corresponding predecessor block as a "blocker" and 
    // let the data-flow analysis handle it.
    // If any predecessor of a PHI matches the 'early' block,
    // do not need a precedence edge between PHI and use
    // since use will be forced into a block preceeding the use.
    Block *block = NULL;
    Block *buse = bbs[outp->_idx];
    assert(buse != NULL, "unused killing projections 'continue'ed above");
    const PhiNode *p = outp->is_Phi();
    if (p == NULL) {          // Check for being a Phi here
      block = buse;
    } else {
      // Set blockers at each block providing input 'store' to the PHI
      // Check if this predecessor block is the same as 'early'
      // Do not assert( buse != early, "Phi merging memory after access");
      // PhiNode may be at start of block 'early' with backedge to 'early'
      uint pmax = p->req();
      for (uint j=1; j<pmax; j++) {  // For all inputs
        if (p->in(j) == state)      {// Found matching input?
          Block *pred_block = bbs[buse->pred(j)->_idx];
          pred_block->set_blocker(index);
          if (block != early)  { block = pred_block; }
        }
      }
      DEBUG_ONLY(assert( phi_inputs.at_grow(p->_idx,0) < p->req() - 1, "Expect at least one phi input will not be from original memory state");)
      // This assert asks about correct handling of PhiNodes which do not 
      // have an input edge directly from "store". See BugId 4621264
      assert(block != NULL, "Must have had at least one input to phi");
      if (block == early) { 
        // anti-dependent upon PHI pinned below 'early', no edge needed
        LCA = early;             // but can not schedule below 'early'
      }
      // Can not 'break' since may be anti-dependent upon node in
      // block 'early'
      continue;                  // have already set blocker
    }

    if (block != early) {
      // May be between early_block and current LCA
      // label as blocker if it may define memory
      block->set_blocker(index);
      nonlocal_stores.push(outp);
    } else {
      // add anti_dependence from outp to use in its own block
      outp->add_prec(use);
      LCA = early;
      inserted_anti_dep = true;
      // No need to update 'max', added def-use edge to 'use' not store
    }
  }

  // Finished if 'use' must be scheduled in its 'early_block'
  if( LCA == early ) return inserted_anti_dep;

  // Propagate up from the LCA, looking for blockers
  Block *hoisted_LCA =  LCA->hoist_LCA_above_defs(early, LCA, use->_idx, bbs );

  if( hoisted_LCA != NULL ) {
    LCA = hoisted_LCA;
    // Insert anti-dependence edges from possible-def to 'use'
    // when def is in hoisted_LCA block
    // Have already added edges to possible-defs in block 'early'
    if( LCA != early ) {
      while( nonlocal_stores.size() ) {
        Node* store = nonlocal_stores.pop();
        assert(!store->check_for_anti_dependence(), ""); //  use-use OK
        assert(store->Opcode() != Op_MachProj, "");
        Block *block = bbs[store->_idx];
        if( block != hoisted_LCA )     continue;
        // add anti_dependence from store to use in its own block
        store->add_prec(use);
        inserted_anti_dep = true;
        // No need to update 'max', added def-use edge to 'use' not use->in(1)
      }
    }
  }

  return inserted_anti_dep;
}

// This class is used to iterate backwards over the nodes in the graph.

class Node_Backward_Iterator {

private:
  Node_Backward_Iterator();

public:
  // Constructor for the iterator
  Node_Backward_Iterator(Node *root, VectorSet &visited, Node_List &stack, Block_Array &bbs);

  // Postincrement operator to iterate over the nodes
  Node *next();

private:
  VectorSet   &_visited;
  Node_List   &_stack;
  Block_Array &_bbs;
};

// Constructor for the Node_Backward_Iterator
Node_Backward_Iterator::Node_Backward_Iterator( Node *root, VectorSet &visited, Node_List &stack, Block_Array &bbs )
  : _visited(visited), _stack(stack), _bbs(bbs) {
  // The stack should contain exactly the root
  stack.clear();
  stack.push(root);

  // Clear the visited bits
  visited.Clear();
}

// Iterator for the Node_Backward_Iterator
Node *Node_Backward_Iterator::next() {

  // If the _stack is empty, then just return NULL: finished.
  if ( !_stack.size() )
    return NULL;

  // '_stack' is emulating a real _stack.  The 'visit-all-users' loop has been
  // made stateless, so I do not need to record the index 'i' on my _stack.
  // Instead I visit all users each time, scanning for unvisited users.
  // I visit unvisited not-anti-dependence users first, then anti-dependent
  // children next.
  Node *self = _stack.pop();

  // I cycle here when I am entering a deeper level of recursion.
  // The key variable 'self' was set prior to jumping here.
  while( 1 ) {

    _visited.set(self->_idx);
      
    // Now schedule all uses as late as possible.
    uint src           = self->is_Proj() ? self->in(0)->_idx : self->_idx;
    uint src_pre_order = _bbs[src]->_pre_order;
      
    // Schedule all nodes in a post-order visit
    Node *unvisited = NULL;  // Unvisited anti-dependent Node, if any

    // Scan for unvisited nodes
    for (DUIterator_Fast imax, i = self->fast_outs(imax); i < imax; i++) {
      // For all uses, schedule late
      Node* n = self->fast_out(i); // Use

      // Skip already visited children
      if ( _visited.test(n->_idx) )
        continue;

      // do not traverse backward control edges
      Node *use = n->is_Proj() ? n->in(0) : n;
      uint use_pre_order = _bbs[use->_idx]->_pre_order;

      if ( use_pre_order < src_pre_order )
        continue;

      // Phi nodes always precede uses in a basic block
      if ( use_pre_order == src_pre_order && use->is_Phi() )
        continue;

      unvisited = n;      // Found unvisited

      // Check for possible-anti-dependent 
      if( !n->check_for_anti_dependence() ) 
        break;            // Not visited, not anti-dep; schedule it NOW
    }
      
    // Did I find an unvisited not-anti-dependent Node?
    if ( !unvisited ) 
      break;                  // All done with children; post-visit 'self'

    // Visit the unvisited Node.  Contains the obvious push to 
    // indicate I'm entering a deeper level of recursion.  I push the
    // old state onto the _stack and set a new state and loop (recurse).
    _stack.push(self);
    self = unvisited;
  } // End recursion loop

  return (self);
}

//------------------------------ComputeLatenciesBackwards----------------------
// Compute the latency of all the instructions.
void PhaseCFG::ComputeLatenciesBackwards( VectorSet &visited, Node_List &stack, GrowableArray<uint> &node_latency ) {
#ifndef PRODUCT
  if (TraceOptoPipelining)
    tty->print("\n#---- ComputeLatenciesBackwards ----\n");
#endif

  Node_Backward_Iterator iter((Node *)_root, visited, stack, _bbs);
  Node *self;

  // Walk over all the nodes from last to first
  while (self = iter.next()) {
    // Set the latency for the definitions of this instruction
    self->partial_latency_of_defs(_bbs, node_latency);
  }
} // end ComputeLatenciesBackwards

//------------------------------partial_latency_of_defs------------------------
// Compute the latency impact of this instruction on all defs.  This computes
// a number that increases as we approach the beginning of the routine.
void Node::partial_latency_of_defs(Block_Array &bbs, GrowableArray<uint> &node_latency) {
  // Set the latency for this instruction
#ifndef PRODUCT
  if (TraceOptoPipelining) {
    tty->print("# Backward: latency %3d for ", node_latency.at_grow(_idx));
    dump();
    tty->print("# now scanning defs");
  }
#endif

  Node *use = is_Proj() ? in(0) : this;

  if (use->is_Root())
    return;

  uint nlen = use->len();
  uint use_latency = node_latency.at_grow(use->_idx);
  uint use_pre_order = bbs[use->_idx]->_pre_order;

  for ( uint j=0; j<nlen; j++ ) {
    Node *def = use->in(j);

    if (!def || def == use)
      continue;
      
    // Walk backwards thru projections
    Node *real_def = def->is_Proj() ? def->in(0) : def;

#ifndef PRODUCT
    if (TraceOptoPipelining) {
      tty->print("#    thru in(%2d): ", j);
      real_def->dump(); 
    }
#endif

    // If the defining block is not known, assume it is ok
    Block *def_block = bbs[real_def->_idx];
    uint def_pre_order = def_block ? def_block->_pre_order : 0;

    if ( (use_pre_order <  def_pre_order) ||
         (use_pre_order == def_pre_order && use->is_Phi()) )
      continue;

    uint delta_latency = use->latency(j);
    uint current_latency = delta_latency + use_latency;

    if (node_latency.at_grow(real_def->_idx) < current_latency)
      node_latency.at_put_grow(real_def->_idx, current_latency);

#ifndef PRODUCT
    if (TraceOptoPipelining) {
      tty->print("#      [%4d]->latency(%d) == %d (%d), node_latency.at_grow(%4d) == %d\n",
        use->_idx, j, delta_latency, current_latency, real_def->_idx, node_latency.at_grow(real_def->_idx));
    }
#endif
  }
}

//------------------------------latency_from_use-------------------------------
// Compute the latency of a specific use
int Node::latency_from_use(Block_Array &bbs, GrowableArray<uint> &node_latency, const Node *def, Node *use) const {
#ifndef PRODUCT
  if (TraceOptoPipelining) {
    tty->print("#      ");
    use->dump();
  }
#endif

  // If self-reference, return no latency
  if (use == this || use->is_Root())
    return 0;
    
  uint def_pre_order = bbs[def->_idx]->_pre_order;
  uint latency = 0;

  // If this is not a projection, then it is simple...
  if (!use->is_Proj()) {
    uint use_pre_order = bbs[use->_idx]->_pre_order;

    if (use_pre_order < def_pre_order)
      return 0;

    if (use_pre_order == def_pre_order && use->is_Phi())
      return 0;

    uint nlen = use->len();
    uint nl = node_latency.at_grow(use->_idx);

    for ( uint j=0; j<nlen; j++ ) {
      if (use->in(j) == this) {
        // Change this if we want local latencies
        uint ul = use->latency(j);
        uint  l = ul + nl;
#ifndef PRODUCT
        if (TraceOptoPipelining) {
          tty->print("#      [%4d]->latency(%d) == %d, node_latency.at_grow(%4d) == %d\n",
            use->_idx, j, ul, use->_idx, nl);
        }
#endif
        if (latency < l) latency = l;
      }
    }
  }

  // This is a projection, just grab the latency of the use(s)
  else {
    for (DUIterator_Fast jmax, j = use->fast_outs(jmax); j < jmax; j++) {
      uint l = use->latency_from_use(bbs, node_latency, def, use->fast_out(j));
      if (latency < l) latency = l;
    }
  }

  return (latency);
}

//------------------------------latency_from_uses------------------------------
// Compute the latency of this instruction relative to all of it's uses.
// This computes a number that increases as we approach the beginning of the
// routine.
int Node::latency_from_uses(Block_Array &bbs, GrowableArray<uint> &node_latency) const {
  // Set the latency for this instruction
#ifndef PRODUCT
  if (TraceOptoPipelining) {
    tty->print("# scanning uses for ");
    dump();
  }
#endif
  uint latency=0;
  const Node *def = ((Node *)this)->is_Proj() ? in(0): this;

  for (DUIterator_Fast imax, i = fast_outs(imax); i < imax; i++) {
    uint l = latency_from_use(bbs, node_latency, def, fast_out(i));

    if (latency < l) latency = l;
  }

  return (latency);
}

//------------------------------ComputeLocalLatenciesBackwards-----------------
// Compute the latency of all the instructions.
void PhaseCFG::ComputeLocalLatenciesBackwards( VectorSet &visited, Node_List &stack, GrowableArray<uint> &node_latency ) {
#ifndef PRODUCT
  if (TraceOptoPipelining)
    tty->print("\n#---- ComputeLocalLatenciesBackwards ----\n");
#endif

  Node_Backward_Iterator iter((Node *)_root, visited, stack, _bbs);
  Node *self;

  // Walk over all the nodes from last to first
  while (self = iter.next()) {

#ifndef PRODUCT
    if (TraceOptoPipelining) {
      tty->print("# Backward: latency %3d for ", node_latency.at_grow(self->_idx));
      self->dump();
    }
#endif

    // Set the partial latency for uses of this node
    self->partial_latency_of_local_defs(_bbs, node_latency);
  }
} // end ComputeLatenciesBackwards

//------------------------------partial_latency_of_local_defs-----------------------------
// Compute the latency impact of this instruction on all defs.
// This computes a number that increases as we approach the beginning of the
// block.
void Node::partial_latency_of_local_defs(Block_Array &bbs, GrowableArray<uint> &node_latency) {
  // Set the latency for this instruction
#ifndef PRODUCT
  if (TraceOptoPipelining) {
    tty->print("# scanning defs for ");
    dump();
  }
#endif

  Node *use = is_Proj() ? in(0) : this;

  if (use->is_Root())
    return;

  uint nlen = use->len();
  uint use_latency = node_latency.at_grow(use->_idx);
  const Block *use_bb = bbs[_idx];

  for ( uint j=0; j<nlen; j++ ) {
    Node *def = use->in(j);

    if (!def || def == use)
      continue;
      
    // Walk backwards thru projections
    Node *real_def = def->is_Proj() ? def->in(0) : def;

    const Block *def_bb = bbs[real_def->_idx];

    if (use_bb != def_bb || use->is_Phi())
      continue;

    uint delta_latency = use->latency(j);
    uint current_latency = delta_latency + use_latency;

    if (node_latency.at_grow(real_def->_idx) < current_latency)
      node_latency.at_put_grow(real_def->_idx, current_latency);

#ifndef PRODUCT
    if (TraceOptoPipelining) {
      tty->print("#      [%4d]->latency(%d) == %d (%d), node_latency.at_grow(%4d) == %d\n",
        use->_idx, j, delta_latency, current_latency, real_def->_idx, node_latency.at_grow(real_def->_idx));
    }
#endif
  }
}

//------------------------------ScheduleLate-----------------------------------
// Now schedule all codes as LATE as possible.  This is the LCA in the 
// dominator tree of all USES of a value.  Pick the block with the least
// loop nesting depth that is lowest in the dominator tree.
extern const char must_clone[];
void PhaseCFG::ScheduleLate(VectorSet &visited, Node_List &stack, GrowableArray<uint> &node_latency) {
  Node_Backward_Iterator iter((Node *)_root, visited, stack, _bbs);
  Node *self;
  const Block *rootBlock = _bbs[_root->_idx];

  // Walk over all the nodes from last to first
  while (self = iter.next()) {
    if (self->is_top()) {
      // Top node goes in bb #2 with other constants.
      // It must be special-cased, because it has no out edges.
      _bbs[self->_idx]->add_inst(self);
      continue;
    }

    // No uses, just terminate
    if (self->outcnt() == 0) {
      assert(self->Opcode() == Op_MachProj, "sanity");
      continue;                   // Must be a dead machine projection
    }

    // If node is pinned in the block, then no scheduling can be done.
    if( self->pinned() )          // Pinned in block?
      continue;

    MachNode *mach = self->is_Mach();

    // Don't move exception creation
    if( mach && mach->ideal_Opcode() == Op_CreateEx ) {
      _bbs[self->_idx]->add_inst(self);
      continue;
    }
    
    // Gather LCA of all uses
    Block *LCA = NULL;
    {
      for (DUIterator_Fast imax, i = self->fast_outs(imax); i < imax; i++) {
	// For all uses, find LCA
	Node* use = self->fast_out(i);
	LCA = use_dom_lca( LCA, use, self, _bbs );
      }
    }  // (Hide defs of imax, i from rest of block.)
    
    // Check if 'self' could be anti-dependent on memory
    if( self->check_for_anti_dependence() ) {
      // Hoist LCA above possible-defs and insert anti-dependences to
      // defs in new LCA block.
      Block *temp_LCA = LCA;
      insert_anti_dependences(LCA, self, _bbs);
    }
    
    // Now find the block with the least execution frequency.
    // Start at the latest schedule and work up to the earliest schedule
    // in the dominator tree.  Thus the Node will dominate all its uses.
    Block *least = LCA;           // Least execution frequency
    
    // Must clone guys stay next to use; no hoisting allowed.
    // Also cannot hoist guys that alter memory or are otherwise not
    // allocatable (hoisting can make a value live longer, leading to
    // anti and output dependency problems which are normally resolved
    // by the register allocator giving everyone a different register).
    if( !mach || !must_clone[mach->ideal_Opcode()] ) {
      Block *early = _bbs[self->_idx];   // Earliest legal placement

      // If there is no opportunity, then just stop here
      if ( LCA != early ) {
        const double delta = 1.0001;
        double least_freq  = least->_freq;
        uint target        = node_latency.at_grow(self->_idx);
        uint start_latency = node_latency.at_grow(least->_nodes[0]->_idx);
        uint end_latency   = node_latency.at_grow(LCA->_nodes[LCA->end_idx()]->_idx);
        bool in_latency    = target <= start_latency;

        // Turn off latency scheduling if scheduling is just plain off
        if( !OptoScheduling )
          in_latency = true;

	// Do not hoist (to cover latency) instructions which target a
	// single register.  Hoisting stretches the live range of the
	// single register and may force spilling.
	if( mach && mach->out_RegMask().is_bound1() && mach->out_RegMask().is_NotEmpty())
	  in_latency = true;

#ifndef PRODUCT
        if (TraceOptoPipelining) {
          tty->print("# Choose block for latency %3d: ",
            node_latency.at_grow(self->_idx));
          self->dump();
          tty->print("#  BB#%03d: start latency for [%4d]=%d, end latency for [%4d]=%d, freq=%g\n",
            LCA->_pre_order,
            LCA->_nodes[0]->_idx,
            start_latency,
            LCA->_nodes[LCA->end_idx()]->_idx,
            end_latency,
            least_freq);
        }
#endif

        // Walk up the dominator tree from LCA (Lowest common ancestor) to
        // the earliest legal location.  Capture the least execution frequency.
        while( LCA != early ) {
          LCA = LCA->_idom;         // Follow up the dominator tree

          // Don't hoist machine instructions to the root basic block
          if (mach && LCA == rootBlock)
            break;

          assert( LCA, "" );        // unscheduable graph assertion
          uint start_lat = node_latency.at_grow(LCA->_nodes[0]->_idx);
          uint end_idx   = LCA->end_idx();
          uint end_lat   = node_latency.at_grow(LCA->_nodes[end_idx]->_idx);
          double LCA_freq = LCA->_freq;
#ifndef PRODUCT
          if (TraceOptoPipelining) {
            tty->print("#  BB#%03d: start latency for [%4d]=%d, end latency for [%4d]=%d, freq=%g\n",
              LCA->_pre_order, LCA->_nodes[0]->_idx, start_lat, end_idx, end_lat, LCA_freq);
          }
#endif
          if( LCA_freq < least_freq              || // Better Frequency
              ( !in_latency                   &&    // No block containing latency
                LCA_freq < least_freq * delta &&    // No worse frequency
                target >= end_lat ) ) {             // within latency range
            least = LCA;            // Found cheaper block
            least_freq = LCA_freq;
            start_latency = start_lat;
            end_latency = end_lat;
            if (target <= start_lat)
              in_latency = true;
          }
        }

#ifndef PRODUCT
        if (TraceOptoPipelining) {
          tty->print("# Choose block BB#%03d with start latency=%3d and freq=%g\n",
            least->_pre_order, start_latency, least_freq);
        }
#endif
        // See if the latency needs to be updated
        if ( target < end_latency ) {
#ifndef PRODUCT
          if (TraceOptoPipelining) {
            tty->print("# Change latency for [%4d] from %3d to %3d\n", self->_idx, target, end_latency);
          }
#endif
          node_latency.at_put_grow(self->_idx, end_latency);
          self->partial_latency_of_defs(_bbs, node_latency);
        }
      }
    }
    
    // Schedule as late as possible with least loop nesting depth
    _bbs.map(self->_idx,least);
    least->add_inst(self);
    
    // After Matching, nearly any old Node may have projections trailing it.
    // These are usually machine-dependent flags.  In any case, they might
    // float to another block below this one.  Move them up.
    for (DUIterator_Fast imax, i = self->fast_outs(imax); i < imax; i++) {
      // For all uses
      Node* use = self->fast_out(i);
      Block *buse = _bbs[use->_idx];
      if( use->is_Proj() &&       // Projection?
          buse != least ) {       // In wrong block?
        if( buse ) buse->find_remove(use); // Remove from wrong block
        _bbs.map(use->_idx,least); // Re-insert in this block
        least->add_inst(use);
      }
    }
    
  } // Loop until all nodes have been visited

} // end ScheduleLate

//------------------------------GlobalCodeMotion-------------------------------
void PhaseCFG::GlobalCodeMotion( Matcher &matcher, uint unique, Node_List &proj_list ) {
  ResourceMark rm;

  // Set the basic block for Nodes pinned into blocks
  VectorSet visited(Thread::current()->resource_area());
  set_pinned( visited, _root );

  // Initialize the bbs.map for things on the proj_list
  uint i;
  for( i=0; i < proj_list.size(); i++ )
    _bbs.map(proj_list[i]->_idx, NULL);

  // Find the earliest Block any instruction can be placed in.  Some
  // instructions are pinned into Blocks.  Unpinned instructions can
  // appear in last block in which all their inputs occur.
  visited.Clear();
  Node_List roots;
  roots.push(_root);
  roots.push(C->top());
  while( roots.size() ) {       // While worklist is not empty
    if( !roots.pop()->schedule_early(visited,roots,_bbs) ) {
      // Bailout without retry
      C->set_result( Compile::Comp_no_retry );
      _root = NULL;
      return;
    }
  }

  // Build Def-Use edges.
  proj_list.push(_root);        // Add real root as another root
  proj_list.pop();

  // Compute the latency information (via backwards walk) for all the
  // instructions in the graph
  GrowableArray<uint> node_latency;
  Node_List stack;

  if( OptoScheduling )
    ComputeLatenciesBackwards(visited, stack, node_latency); 

  // Now schedule all codes as LATE as possible.  This is the LCA in the 
  // dominator tree of all USES of a value.  Pick the block with the least
  // loop nesting depth that is lowest in the dominator tree.  
  visited.Clear();
  ScheduleLate(visited, stack, node_latency);
  unique = C->unique();

  // Detect implicit-null-check opportunities.  Basically, find NULL checks 
  // with suitable memory ops nearby.  Use the memory op to do the NULL check.
  // I can generate a memory op if there is not one nearby.
  if( C->is_method_compilation() ) {// Don't do it for natives, adapters, or runtime stubs
    // By reversing the loop direction we get a very minor gain on mpegaudio.
    // Feel free to revert to a forward loop for clarity.
//    for( int i=0; i < (int)matcher._null_check_tests.size(); i+=2 ) {
    for( int i= matcher._null_check_tests.size()-2; i>=0; i-=2 ) {
      Node *proj = matcher._null_check_tests[i  ];
      Node *val  = matcher._null_check_tests[i+1];
      _bbs[proj->_idx]->implicit_null_check(_bbs,node_latency,proj,val);
    }
  }

  // Schedule locally.  Right now a simple topological sort.
  // Later, do a real latency aware scheduler.
  int *ready_cnt = NEW_RESOURCE_ARRAY(int,C->unique());
  memset( ready_cnt, -1, C->unique() * sizeof(int) );
  visited.Clear();
  for (i = 0; i < _num_blocks; i++) {
    if (!_blocks[i]->schedule_local(matcher, _bbs, ready_cnt, visited, node_latency)) {
      _root = NULL;
      return;
    }
  }

  // If we inserted any instructions between a Call and his CatchNode,
  // clone the instructions on all paths below the Catch.
  for( i=0; i < _num_blocks; i++ )
    _blocks[i]->call_catch_cleanup(_bbs);

#ifndef PRODUCT
  if (TraceOptoPipelining) {
    tty->print("\n---- After GlobalCodeMotion ----\n");
    for (uint i = 0; i < _num_blocks; i++) {
      tty->print("\nBB#%03d:\n", i);
      Block *bb = _blocks[i];
      for (uint j = 0; j < bb->_nodes.size(); j++)
        bb->_nodes[j]->dump();
    }
  }
#endif
}

#define MAXFREQ 1e35f
#define MINFREQ 1e-35f

//------------------------------Estimate_Block_Frequency-----------------------
// Estimate block frequencies based on IfNode probabilities.
// Two pass algorithm does a forward propagation in the first pass with some
// correction factors where static predictions are needed.  Then, the second
// pass pushes through changes caused by back edges.  This will give "exact"
// results for all dynamic frequencies, and for all staticly predicted code
// with loop nesting depth of one or less.  Static predictions with greater
// than nesting depth of one are already subject to so many static fudge
// factors that it is not worth iterating to a fixed point.
void PhaseCFG::Estimate_Block_Frequency() {
  assert( _blocks[0] == _broot, "" );
  int cnts = C->method() ? C->method()->interpreter_invocation_count() : 1;
  if( cnts == 0 ) cnts = 1;
  float f = (float)cnts/(float)FreqCountInvocations;
  _broot->_freq = f;
  _broot->_cnt  = f;
  // Do a two pass propagation of frequency information
  // PASS 1: Walk the blocks in RPO, propagating frequency info
  uint i;
  for( i = 0; i < _num_blocks; i++ ) {
    Block *b = _blocks[i];

    // Make any necessary modifications to b's frequency
    int hop = b->head()->Opcode();
    // On first trip, scale loop heads by 10 if no counts are available
    if( (hop == Op_Loop || hop == Op_CountedLoop) &&
	(b->_cnt == -1.0f) && (b->_freq < MAXFREQ) ) {
      // Try to figure out how much to scale the loop by; look for a
      // gating loop-exit test with "reasonable" back-branch
      // frequency.

      // Try and find a real loop-back controlling edge and use that
      // frequency. If we can't find it, use the old default of 10
      // otherwise use the new value. This helps loops with low
      // frequency (like allocation contention loops with -UseTLE).
      // Note special treatment below of LoopNode::EntryControl edges.      
      Block *loopprior = b;          
      Block *loopback = _bbs[b->pred(LoopNode::LoopBackControl)->_idx];
      // See if this block ends in a test (probably not) or just a
      // goto the loop head.
      if( loopback->_num_succs == 1 &&
          loopback->num_preds() == 2 ) {
        loopprior = loopback;
        // NOTE: constant 1 here isn't magic, it's just that there's exactly 1
        // predecessor (checked just above) and predecessors are 1-based, so
        // the "1" refers to the first (and only) predecessor.
        loopback = _bbs[loopprior->pred(1)->_idx];
      }
      // Call the edge frequency leading from loopback to loopprior f.
      // Then scale the loop by 1/(1-f).  Thus a loop-back edge
      // frequency of 0.9 leads to a scale factor of 10.
      float f = 0.9f;           // Default scale factor

      if( loopback->_num_succs == 2 ) {
        int eidx = loopback->end_idx();
        MachNode *mn = loopback->_nodes[eidx]->is_Mach(); // Get ending Node
        if( mn ) {
          assert( mn->is_MachIf(), "" );
          MachIfNode *mif = (MachIfNode *)mn; // MachIfNode has branch probability info
          f = mif->_prob;
          int taken = (loopback->_succs[1] == loopprior);
          assert( loopback->_succs[taken] == loopprior, "" );
          if( loopback->_nodes[eidx+1+taken]->Opcode() == Op_IfFalse ) 
            f = 1-f;              // Inverted branch sense
          if( f > 0.99f )         // Limit scale to 100
            f = 0.99f;
        }
      }
      
      // Scale loop head by this much
      b->_freq *= 1/(1-f);
      assert(b->_freq > 0.0f,"Bad frequency assignment");
    }

    // Push b's frequency to successors
    int eidx = b->end_idx();    
    Node *n = b->_nodes[eidx];  // Get ending Node
    MachNode *mach = n->is_Mach();
    int op = mach ? mach->ideal_Opcode() : n->Opcode();
    // Switch on branch type
    switch( op ) {
    // Conditionals pass on only part of their frequency and count
    case Op_CountedLoopEnd:
    case Op_If: {
      int taken  = 0;  // this is the index of the TAKEN path
      int ntaken = 1;  // this is the index of the NOT TAKEN path
      // If succ[0] is the FALSE branch, invert path info
      if( b->_nodes[eidx+1]->Opcode() == Op_IfFalse ) {
	taken  = 1;
	ntaken = 0;
      }
      float prob  = mach->is_MachIf()->_prob;
      float nprob = 1.0f - prob;
      float cnt   = mach->is_MachIf()->_fcnt;
      // If branch frequency info is available, use it
      if(cnt != -1.0) {
	float tcnt = b->_succs[taken]->_cnt;
	float ncnt = b->_succs[ntaken]->_cnt;
	// Taken Branch
	b->_succs[taken]->_freq += prob * cnt;
	b->_succs[taken]->_cnt = (tcnt == -1.0f) ? (prob * cnt) : tcnt + (prob * cnt);
	// Not Taken Branch
	b->_succs[ntaken]->_freq += nprob * cnt;
	b->_succs[ntaken]->_cnt = (ncnt == -1.0f) ? (nprob * cnt) : ncnt + (nprob * cnt);
      }
      // Otherwise, split frequency amongst children
      else {
	b->_succs[taken]->_freq  +=  prob * b->_freq;
	b->_succs[ntaken]->_freq += nprob * b->_freq;
      }
      // Special case for underflow caused by infrequent branches
      if(b->_succs[taken]->_freq < MINFREQ) b->_succs[taken]->_freq = MINFREQ;
      if(b->_succs[ntaken]->_freq < MINFREQ) b->_succs[ntaken]->_freq = MINFREQ;
      assert(b->_succs[0]->_freq > 0.0f,"Bad Branch frequency assignment");
      assert((b->_succs[0]->_cnt > 0.0f) || (b->_succs[0]->_cnt == -1.0f),"Bad count");
      assert(b->_succs[1]->_freq > 0.0f,"Bad Branch frequency assignment");
      assert((b->_succs[1]->_cnt > 0.0f) || (b->_succs[1]->_cnt == -1.0f),"Bad count");
      break;
    }
    case Op_NeverBranch:  {
      b->_succs[0]->_freq += b->_freq;
      // Special case for underflow caused by infrequent branches
      if(b->_succs[0]->_freq < MINFREQ) b->_succs[0]->_freq = MINFREQ;
      if(b->_succs[1]->_freq < MINFREQ) b->_succs[1]->_freq = MINFREQ;
      break;
    }
    // Split frequency amongst children
    case Op_Catch: {
      // Fall-thru path gets the lion's share.
      float fall = (1.0f - 0.00001*b->_num_succs)*b->_freq;
      // Exception exits get 1 in a million.
      float expt = 0.00001 * b->_freq;
      // Iterate over children pushing out frequency
      for( uint j = 0; j < b->_num_succs; j++ ) {
	const CatchProjNode *x = b->_nodes[eidx+1+j]->is_CatchProj();
	b->_succs[j]->_freq += 
	  ((x->_con == CatchProjNode::fall_through_index) ? fall : expt);
	// Special case for underflow caused by nested catches
	if(b->_succs[j]->_freq < MINFREQ) b->_succs[j]->_freq = MINFREQ;
	assert(b->_succs[j]->_freq > 0.0f,"Bad Catch Frequency Assignment");
        assert((b->_succs[j]->_cnt > 0.0f) || (b->_succs[j]->_cnt == -1.0f),"Bad Catch count assignment");
      }
      break;
    }
    // Pass frequency straight thru to target
    case Op_Root:
    case Op_Goto: {
      Block *bs = b->_succs[0];
      int hop = bs->head()->Opcode();
      bool notloop = (hop != Op_Loop && hop != Op_CountedLoop);
      // Pass count straight thru to target (except for loops)
      if( notloop && b->_cnt != -1.0f ) {
        if( bs->_cnt == -1.0f )
          bs->_cnt = 0;
        bs->_cnt += b->_cnt;
      }
      // Loops and counted loops have already had their heads scaled
      // by an amount which accounts for the backedge (but not their
      // entry).  Add frequency for normal blocks and loop entries.
      // Note special treatment above of LoopNode::LoopBackControl edges.
      if( notloop || bs->_freq <= 0 /*this is needed for irreducible loops*/||
          _bbs[bs->pred(LoopNode::EntryControl)->_idx] == b )
        bs->_freq += b->_freq;

      assert( bs->_freq > 0,"Bad Goto Frequency Assignment");
      assert( (bs->_cnt > 0.0f) || (bs->_cnt == -1.0f),"Bad goto count assignment");
      break;
    }
    // Do not push out freq to root block
    case Op_TailCall:
    case Op_TailJump:
    case Op_Return:
    case Op_Halt:
    case Op_Rethrow:
      break;
    default: 
      ShouldNotReachHere();
    } // End switch(op)
    assert(b->_freq > 0.0f,"Bad first pass frequency");
    assert((b->_cnt > 0.0f) || (b->_cnt == -1.0f),"Bad first pass count");
  } // End for all blocks


  // PASS 2: Fix up loop bodies
  for( i = 1; i < _num_blocks; i++ ) {
    Block *b = _blocks[i];
    float freq = 0.0f;
    float cnt  = -1.0f;
    // If block ends in a Halt, assume it is uncommon!
    MachNode *mach = b->end()->is_Mach();
    if( mach && mach->ideal_Opcode() == Op_Halt ) {
      if( b->_freq > 1e-6 )
	b->_freq = 1e-6f;
      continue;
    }

    // Recompute frequency based upon predecessors' frequencies
    for(uint j = 1; j < b->num_preds(); j++) {
      // Compute the frequency passed along this path
      Node *pred = b->head()->in(j);
      // Peek through projections
      if(pred->is_Proj()) pred = pred->in(0);
      // Grab the predecessor block's frequency
      Block *pblock = _bbs[pred->_idx];
      float predfreq = pblock->_freq;
      float predcnt = pblock->_cnt;
      // Properly modify the frequency for this exit path
      MachNode *mach = pred->is_Mach();
      int op = mach ? mach->ideal_Opcode() : pred->Opcode();
      // Switch on branch type
      switch(op) {
      // Conditionals pass on only part of their frequency and count
      case Op_CountedLoopEnd:
      case Op_If: {
	float prob = mach->is_MachIf()->_prob;
	float cnt  = mach->is_MachIf()->_fcnt;
	bool path  = true;
	// Is this the TRUE branch or the FALSE branch?
	if( b->head()->in(j)->Opcode() == Op_IfFalse )
	  path = false;
	// If branch frequency info is available, use it
	if(cnt != -1.0) {
	  predfreq = (path) ? (prob * cnt) : ((1.0f-prob) * cnt);
	  predcnt  = (path) ? (prob * cnt) : ((1.0f-prob) * cnt);
	}
	// Otherwise, split frequency amongst children
	else {
	  predfreq = (path) ? (prob * predfreq) : ((1.0f-prob) * predfreq);
	  predcnt  = -1.0f;
	}
        if( predfreq < MINFREQ ) predfreq = MINFREQ;

        // Raise frequency of the loop backedge block, in an effort
        // to keep it empty.  Must raise it by 10%+ because counted
        // loops normally keep a 90/10 exit ratio.
        if( op == Op_CountedLoopEnd && b->num_preds() == 2 && path == true )
          predfreq *= 1.15f;
        assert(predfreq > 0.0f,"Bad intermediate frequency");
        assert((predcnt > 0.0f) ||(predcnt == -1.0f),"Bad intermediate count");
	break;
      }
      // Catch splits frequency amongst multiple children, favoring
      // fall through
      case Op_Catch: {
	// Fall-thru path gets the lion's share.
	float fall  = (1.0f - 0.00001*pblock->_num_succs)*predfreq;
	// Exception exits get 1 in a million.
	float expt  = 0.00001 * predfreq;
	// Determine if this is fall-thru path
	const CatchProjNode *x = b->head()->in(j)->is_CatchProj();
	predfreq = (x->_con == CatchProjNode::fall_through_index) ? fall :expt;
	predcnt  = -1.0f;
        if(predfreq < MINFREQ) predfreq = MINFREQ;
        assert(predfreq > 0.0f,"Bad intermediate frequency");
        assert((predcnt > 0.0f) ||(predcnt == -1.0f),"Bad intermediate count");
	break;
      }
      // Pass frequency straight thru to target
      case Op_Root:
      case Op_Goto:
      case Op_Start:
      case Op_NeverBranch:
	break;
      // These do not push out a frequency or count
      case Op_TailCall:
      case Op_TailJump:
      case Op_Return:
      case Op_Halt:
      case Op_Rethrow:
	predfreq = 0.0f;
	predcnt = -1.0f;
	break;
      default: 
	ShouldNotReachHere();
      } // End switch(op)
      assert((predcnt > 0.0f) || (predcnt == -1.0f),"Bad intermediate count");
      // Accumulate frequency from predecessor block
      freq += predfreq;
      cnt = (predcnt == -1.0f) ? cnt : (cnt == -1.0f) ? predcnt : cnt +predcnt;
    }
    // Assign new frequency
    b->_freq = freq;
    b->_cnt = cnt;
    assert(b->_freq > 0.0f,"Bad final frequency assignment");
    assert((b->_cnt > 0.0f) ||(b->_cnt == -1.0f),"Bad final count assignment");
  } // End for all blocks
}
