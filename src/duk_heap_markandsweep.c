/*
 *  Mark-and-sweep garbage collection.
 */

#include "duk_internal.h"

#ifdef DUK_USE_MARK_AND_SWEEP

DUK_LOCAL_DECL void duk__mark_heaphdr(duk_heap *heap, duk_heaphdr *h);
DUK_LOCAL_DECL void duk__mark_tval(duk_heap *heap, duk_tval *tv);

/*
 *  Misc
 */

/* Select a thread for mark-and-sweep use.
 *
 * XXX: This needs to change later.
 */
DUK_LOCAL duk_hthread *duk__get_temp_hthread(duk_heap *heap) {
	if (heap->curr_thread) {
		return heap->curr_thread;
	}
	return heap->heap_thread;  /* may be NULL, too */
}

/*
 *  Marking functions for heap types: mark children recursively
 */

DUK_LOCAL void duk__mark_hstring(duk_heap *heap, duk_hstring *h) {
	DUK_UNREF(heap);
	DUK_UNREF(h);

	DUK_DDD(DUK_DDDPRINT("duk__mark_hstring: %p", (void *) h));
	DUK_ASSERT(h);

	/* nothing to process */
}

DUK_LOCAL void duk__mark_hobject(duk_heap *heap, duk_hobject *h) {
	duk_uint_fast32_t i;

	DUK_DDD(DUK_DDDPRINT("duk__mark_hobject: %p", (void *) h));

	DUK_ASSERT(h);

	/* XXX: use advancing pointers instead of index macros -> faster and smaller? */

	for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(h); i++) {
		duk_hstring *key = DUK_HOBJECT_E_GET_KEY(heap, h, i);
		if (!key) {
			continue;
		}
		duk__mark_heaphdr(heap, (duk_heaphdr *) key);
		if (DUK_HOBJECT_E_SLOT_IS_ACCESSOR(heap, h, i)) {
			duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_HOBJECT_E_GET_VALUE_PTR(heap, h, i)->a.get);
			duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_HOBJECT_E_GET_VALUE_PTR(heap, h, i)->a.set);
		} else {
			duk__mark_tval(heap, &DUK_HOBJECT_E_GET_VALUE_PTR(heap, h, i)->v);
		}
	}

	for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ASIZE(h); i++) {
		duk__mark_tval(heap, DUK_HOBJECT_A_GET_VALUE_PTR(heap, h, i));
	}

	/* hash part is a 'weak reference' and does not contribute */

	duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_HOBJECT_GET_PROTOTYPE(heap, h));

	if (DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		duk_hcompiledfunction *f = (duk_hcompiledfunction *) h;
		duk_tval *tv, *tv_end;
		duk_hobject **fn, **fn_end;

		/* 'data' is reachable through every compiled function which
		 * contains a reference.
		 */

		duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_HCOMPILEDFUNCTION_GET_DATA(heap, f));

		if (DUK_HCOMPILEDFUNCTION_GET_DATA(heap, f) != NULL) {
			tv = DUK_HCOMPILEDFUNCTION_GET_CONSTS_BASE(heap, f);
			tv_end = DUK_HCOMPILEDFUNCTION_GET_CONSTS_END(heap, f);
			while (tv < tv_end) {
				duk__mark_tval(heap, tv);
				tv++;
			}

			fn = DUK_HCOMPILEDFUNCTION_GET_FUNCS_BASE(heap, f);
			fn_end = DUK_HCOMPILEDFUNCTION_GET_FUNCS_END(heap, f);
			while (fn < fn_end) {
				duk__mark_heaphdr(heap, (duk_heaphdr *) *fn);
				fn++;
			}
		} else {
			/* May happen in some out-of-memory corner cases. */
			DUK_D(DUK_DPRINT("duk_hcompiledfunction 'data' is NULL, skipping marking"));
		}
	} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		duk_hnativefunction *f = (duk_hnativefunction *) h;
		DUK_UNREF(f);
		/* nothing to mark */
	} else if (DUK_HOBJECT_IS_BUFFEROBJECT(h)) {
		duk_hbufferobject *b = (duk_hbufferobject *) h;
		duk__mark_heaphdr(heap, (duk_heaphdr *) b->buf);
	} else if (DUK_HOBJECT_IS_THREAD(h)) {
		duk_hthread *t = (duk_hthread *) h;
		duk_tval *tv;

		tv = t->valstack;
		while (tv < t->valstack_top) {
			duk__mark_tval(heap, tv);
			tv++;
		}

		for (i = 0; i < (duk_uint_fast32_t) t->callstack_top; i++) {
			duk_activation *act = t->callstack + i;
			duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_ACT_GET_FUNC(act));
			duk__mark_heaphdr(heap, (duk_heaphdr *) act->var_env);
			duk__mark_heaphdr(heap, (duk_heaphdr *) act->lex_env);
#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
			duk__mark_heaphdr(heap, (duk_heaphdr *) act->prev_caller);
#endif
		}

#if 0  /* nothing now */
		for (i = 0; i < (duk_uint_fast32_t) t->catchstack_top; i++) {
			duk_catcher *cat = t->catchstack + i;
		}
#endif

		duk__mark_heaphdr(heap, (duk_heaphdr *) t->resumer);

		/* XXX: duk_small_uint_t would be enough for this loop */
		for (i = 0; i < DUK_NUM_BUILTINS; i++) {
			duk__mark_heaphdr(heap, (duk_heaphdr *) t->builtins[i]);
		}
	}
}

/* recursion tracking happens here only */
DUK_LOCAL void duk__mark_heaphdr(duk_heap *heap, duk_heaphdr *h) {
	DUK_DDD(DUK_DDDPRINT("duk__mark_heaphdr %p, type %ld",
	                     (void *) h,
	                     (h != NULL ? (long) DUK_HEAPHDR_GET_TYPE(h) : (long) -1)));
	if (!h) {
		return;
	}
#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY(h)) {
		DUK_DDD(DUK_DDDPRINT("readonly object %p, skip", (void *) h));
		return;
	}
#endif
	if (DUK_HEAPHDR_HAS_REACHABLE(h)) {
		DUK_DDD(DUK_DDDPRINT("already marked reachable, skip"));
		return;
	}
	DUK_HEAPHDR_SET_REACHABLE(h);

	if (heap->mark_and_sweep_recursion_depth >= DUK_USE_MARK_AND_SWEEP_RECLIMIT) {
		/* log this with a normal debug level because this should be relatively rare */
		DUK_D(DUK_DPRINT("mark-and-sweep recursion limit reached, marking as temproot: %p", (void *) h));
		DUK_HEAP_SET_MARKANDSWEEP_RECLIMIT_REACHED(heap);
		DUK_HEAPHDR_SET_TEMPROOT(h);
		return;
	}

	heap->mark_and_sweep_recursion_depth++;

	switch ((int) DUK_HEAPHDR_GET_TYPE(h)) {
	case DUK_HTYPE_STRING:
		duk__mark_hstring(heap, (duk_hstring *) h);
		break;
	case DUK_HTYPE_OBJECT:
		duk__mark_hobject(heap, (duk_hobject *) h);
		break;
	case DUK_HTYPE_BUFFER:
		/* nothing to mark */
		break;
	default:
		DUK_D(DUK_DPRINT("attempt to mark heaphdr %p with invalid htype %ld", (void *) h, (long) DUK_HEAPHDR_GET_TYPE(h)));
		DUK_UNREACHABLE();
	}

	heap->mark_and_sweep_recursion_depth--;
}

DUK_LOCAL void duk__mark_tval(duk_heap *heap, duk_tval *tv) {
	DUK_DDD(DUK_DDDPRINT("duk__mark_tval %p", (void *) tv));
	if (!tv) {
		return;
	}
	if (DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		duk__mark_heaphdr(heap, DUK_TVAL_GET_HEAPHDR(tv));
	}
}

/*
 *  Mark the heap.
 */

DUK_LOCAL void duk__mark_roots_heap(duk_heap *heap) {
	duk_small_uint_t i;

	DUK_DD(DUK_DDPRINT("duk__mark_roots_heap: %p", (void *) heap));

	duk__mark_heaphdr(heap, (duk_heaphdr *) heap->heap_thread);
	duk__mark_heaphdr(heap, (duk_heaphdr *) heap->heap_object);

	for (i = 0; i < DUK_HEAP_NUM_STRINGS; i++) {
		duk_hstring *h = DUK_HEAP_GET_STRING(heap, i);
		duk__mark_heaphdr(heap, (duk_heaphdr *) h);
	}

	duk__mark_tval(heap, &heap->lj.value1);
	duk__mark_tval(heap, &heap->lj.value2);

#if defined(DUK_USE_DEBUGGER_SUPPORT)
	for (i = 0; i < heap->dbg_breakpoint_count; i++) {
		duk__mark_heaphdr(heap, (duk_heaphdr *) heap->dbg_breakpoints[i].filename);
	}
#endif
}

/*
 *  Mark refzero_list objects.
 *
 *  Objects on the refzero_list have no inbound references.  They might have
 *  outbound references to objects that we might free, which would invalidate
 *  any references held by the refzero objects.  A refzero object might also
 *  be rescued by refcount finalization.  Refzero objects are treated as
 *  reachability roots to ensure they (or anything they point to) are not
 *  freed in mark-and-sweep.
 */

#ifdef DUK_USE_REFERENCE_COUNTING
DUK_LOCAL void duk__mark_refzero_list(duk_heap *heap) {
	duk_heaphdr *hdr;

	DUK_DD(DUK_DDPRINT("duk__mark_refzero_list: %p", (void *) heap));

	hdr = heap->refzero_list;
	while (hdr) {
		duk__mark_heaphdr(heap, hdr);
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}
#endif

/*
 *  Mark unreachable, finalizable objects.
 *
 *  Such objects will be moved aside and their finalizers run later.  They have
 *  to be treated as reachability roots for their properties etc to remain
 *  allocated.  This marking is only done for unreachable values which would
 *  be swept later (refzero_list is thus excluded).
 *
 *  Objects are first marked FINALIZABLE and only then marked as reachability
 *  roots; otherwise circular references might be handled inconsistently.
 */

DUK_LOCAL void duk__mark_finalizable(duk_heap *heap) {
	duk_hthread *thr;
	duk_heaphdr *hdr;
	duk_size_t count_finalizable = 0;

	DUK_DD(DUK_DDPRINT("duk__mark_finalizable: %p", (void *) heap));

	thr = duk__get_temp_hthread(heap);
	DUK_ASSERT(thr != NULL);

	hdr = heap->heap_allocated;
	while (hdr) {
		/* A finalizer is looked up from the object and up its prototype chain
		 * (which allows inherited finalizers).  A prototype loop must not cause
		 * an error to be thrown here; duk_hobject_hasprop_raw() will ignore a
		 * prototype loop silently and indicate that the property doesn't exist.
		 */

		if (!DUK_HEAPHDR_HAS_REACHABLE(hdr) &&
		    DUK_HEAPHDR_GET_TYPE(hdr) == DUK_HTYPE_OBJECT &&
		    !DUK_HEAPHDR_HAS_FINALIZED(hdr) &&
		    duk_hobject_hasprop_raw(thr, (duk_hobject *) hdr, DUK_HTHREAD_STRING_INT_FINALIZER(thr))) {

			/* heaphdr:
			 *  - is not reachable
			 *  - is an object
			 *  - is not a finalized object
			 *  - has a finalizer
			 */

			DUK_DD(DUK_DDPRINT("unreachable heap object will be "
			                   "finalized -> mark as finalizable "
			                   "and treat as a reachability root: %p",
			                   (void *) hdr));
			DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY(hdr));
			DUK_HEAPHDR_SET_FINALIZABLE(hdr);
			count_finalizable ++;
		}

		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}

	if (count_finalizable == 0) {
		return;
	}

	DUK_DD(DUK_DDPRINT("marked %ld heap objects as finalizable, now mark them reachable",
	                   (long) count_finalizable));

	hdr = heap->heap_allocated;
	while (hdr) {
		if (DUK_HEAPHDR_HAS_FINALIZABLE(hdr)) {
			duk__mark_heaphdr(heap, hdr);
		}

		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}

	/* Caller will finish the marking process if we hit a recursion limit. */
}

/*
 *  Mark objects on finalize_list.
 *
 */

DUK_LOCAL void duk__mark_finalize_list(duk_heap *heap) {
	duk_heaphdr *hdr;
#ifdef DUK_USE_DEBUG
	duk_size_t count_finalize_list = 0;
#endif

	DUK_DD(DUK_DDPRINT("duk__mark_finalize_list: %p", (void *) heap));

	hdr = heap->finalize_list;
	while (hdr) {
		duk__mark_heaphdr(heap, hdr);
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
#ifdef DUK_USE_DEBUG
		count_finalize_list++;
#endif
	}

#ifdef DUK_USE_DEBUG
	if (count_finalize_list > 0) {
		DUK_D(DUK_DPRINT("marked %ld objects on the finalize_list as reachable (previous finalizer run skipped)",
		                 (long) count_finalize_list));
	}
#endif
}

/*
 *  Fallback marking handler if recursion limit is reached.
 *
 *  Iterates 'temproots' until recursion limit is no longer hit.  Note
 *  that temproots may reside either in heap allocated list or the
 *  refzero work list.  This is a slow scan, but guarantees that we
 *  finish with a bounded C stack.
 *
 *  Note that nodes may have been marked as temproots before this
 *  scan begun, OR they may have been marked during the scan (as
 *  we process nodes recursively also during the scan).  This is
 *  intended behavior.
 */

#ifdef DUK_USE_DEBUG
DUK_LOCAL void duk__handle_temproot(duk_heap *heap, duk_heaphdr *hdr, duk_size_t *count) {
#else
DUK_LOCAL void duk__handle_temproot(duk_heap *heap, duk_heaphdr *hdr) {
#endif
	if (!DUK_HEAPHDR_HAS_TEMPROOT(hdr)) {
		DUK_DDD(DUK_DDDPRINT("not a temp root: %p", (void *) hdr));
		return;
	}

	DUK_DDD(DUK_DDDPRINT("found a temp root: %p", (void *) hdr));
	DUK_HEAPHDR_CLEAR_TEMPROOT(hdr);
	DUK_HEAPHDR_CLEAR_REACHABLE(hdr);  /* done so that duk__mark_heaphdr() works correctly */
	duk__mark_heaphdr(heap, hdr);

#ifdef DUK_USE_DEBUG
	(*count)++;
#endif
}

DUK_LOCAL void duk__mark_temproots_by_heap_scan(duk_heap *heap) {
	duk_heaphdr *hdr;
#ifdef DUK_USE_DEBUG
	duk_size_t count;
#endif

	DUK_DD(DUK_DDPRINT("duk__mark_temproots_by_heap_scan: %p", (void *) heap));

	while (DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED(heap)) {
		DUK_DD(DUK_DDPRINT("recursion limit reached, doing heap scan to continue from temproots"));

#ifdef DUK_USE_DEBUG
		count = 0;
#endif
		DUK_HEAP_CLEAR_MARKANDSWEEP_RECLIMIT_REACHED(heap);

		hdr = heap->heap_allocated;
		while (hdr) {
#ifdef DUK_USE_DEBUG
			duk__handle_temproot(heap, hdr, &count);
#else
			duk__handle_temproot(heap, hdr);
#endif
			hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
		}

		/* must also check refzero_list */
#ifdef DUK_USE_REFERENCE_COUNTING
		hdr = heap->refzero_list;
		while (hdr) {
#ifdef DUK_USE_DEBUG
			duk__handle_temproot(heap, hdr, &count);
#else
			duk__handle_temproot(heap, hdr);
#endif
			hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
		}
#endif  /* DUK_USE_REFERENCE_COUNTING */

#ifdef DUK_USE_DEBUG
		DUK_DD(DUK_DDPRINT("temproot mark heap scan processed %ld temp roots", (long) count));
#endif
	}
}

/*
 *  Finalize refcounts for heap elements just about to be freed.
 *  This must be done for all objects before freeing to avoid any
 *  stale pointer dereferences.
 *
 *  Note that this must deduce the set of objects to be freed
 *  identically to duk__sweep_heap().
 */

#ifdef DUK_USE_REFERENCE_COUNTING
DUK_LOCAL void duk__finalize_refcounts(duk_heap *heap) {
	duk_hthread *thr;
	duk_heaphdr *hdr;

	thr = duk__get_temp_hthread(heap);
	DUK_ASSERT(thr != NULL);

	DUK_DD(DUK_DDPRINT("duk__finalize_refcounts: heap=%p, hthread=%p",
	                   (void *) heap, (void *) thr));

	hdr = heap->heap_allocated;
	while (hdr) {
		if (!DUK_HEAPHDR_HAS_REACHABLE(hdr)) {
			/*
			 *  Unreachable object about to be swept.  Finalize target refcounts
			 *  (objects which the unreachable object points to) without doing
			 *  refzero processing.  Recursive decrefs are also prevented when
			 *  refzero processing is disabled.
			 *
			 *  Value cannot be a finalizable object, as they have been made
			 *  temporarily reachable for this round.
			 */

			DUK_DDD(DUK_DDDPRINT("unreachable object, refcount finalize before sweeping: %p", (void *) hdr));
			duk_heaphdr_refcount_finalize(thr, hdr);
		}

		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}
#endif  /* DUK_USE_REFERENCE_COUNTING */

/*
 *  Clear (reachable) flags of refzero work list.
 */

#ifdef DUK_USE_REFERENCE_COUNTING
DUK_LOCAL void duk__clear_refzero_list_flags(duk_heap *heap) {
	duk_heaphdr *hdr;

	DUK_DD(DUK_DDPRINT("duk__clear_refzero_list_flags: %p", (void *) heap));

	hdr = heap->refzero_list;
	while (hdr) {
		DUK_HEAPHDR_CLEAR_REACHABLE(hdr);
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_TEMPROOT(hdr));
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}
#endif  /* DUK_USE_REFERENCE_COUNTING */

/*
 *  Clear (reachable) flags of finalize_list
 *
 *  We could mostly do in the sweep phase when we move objects from the
 *  heap into the finalize_list.  However, if a finalizer run is skipped
 *  during a mark-and-sweep, the objects on the finalize_list will be marked
 *  reachable during the next mark-and-sweep.  Since they're already on the
 *  finalize_list, no-one will be clearing their REACHABLE flag so we do it
 *  here.  (This now overlaps with the sweep handling in a harmless way.)
 */

DUK_LOCAL void duk__clear_finalize_list_flags(duk_heap *heap) {
	duk_heaphdr *hdr;

	DUK_DD(DUK_DDPRINT("duk__clear_finalize_list_flags: %p", (void *) heap));

	hdr = heap->finalize_list;
	while (hdr) {
		DUK_HEAPHDR_CLEAR_REACHABLE(hdr);
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_TEMPROOT(hdr));
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}

/*
 *  Sweep stringtable
 */

#if defined(DUK_USE_STRTAB_CHAIN)

/* XXX: skip count_free w/o debug? */
#if defined(DUK_USE_HEAPPTR16)
DUK_LOCAL void duk__sweep_string_chain16(duk_heap *heap, duk_uint16_t *slot, duk_size_t *count_keep, duk_size_t *count_free) {
	duk_uint16_t h16 = *slot;
	duk_hstring *h;
	duk_uint16_t null16 = heap->heapptr_null16;

	if (h16 == null16) {
		/* nop */
		return;
	}
	h = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, h16);
	DUK_ASSERT(h != NULL);

	if (DUK_HEAPHDR_HAS_REACHABLE((duk_heaphdr *) h)) {
		DUK_HEAPHDR_CLEAR_REACHABLE((duk_heaphdr *) h);
		(*count_keep)++;
	} else {
#if defined(DUK_USE_REFERENCE_COUNTING)
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) h) == 0);
#endif
		/* deal with weak references first */
		duk_heap_strcache_string_remove(heap, (duk_hstring *) h);
		*slot = null16;

		/* free inner references (these exist e.g. when external
		 * strings are enabled)
		 */
		duk_free_hstring_inner(heap, h);
		DUK_FREE(heap, h);
		(*count_free)++;
	}
}
#else  /* DUK_USE_HEAPPTR16 */
DUK_LOCAL void duk__sweep_string_chain(duk_heap *heap, duk_hstring **slot, duk_size_t *count_keep, duk_size_t *count_free) {
	duk_hstring *h = *slot;

	if (h == NULL) {
		/* nop */
		return;
	}

	if (DUK_HEAPHDR_HAS_REACHABLE((duk_heaphdr *) h)) {
		DUK_HEAPHDR_CLEAR_REACHABLE((duk_heaphdr *) h);
		(*count_keep)++;
	} else {
#if defined(DUK_USE_REFERENCE_COUNTING)
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) h) == 0);
#endif
		/* deal with weak references first */
		duk_heap_strcache_string_remove(heap, (duk_hstring *) h);
		*slot = NULL;

		/* free inner references (these exist e.g. when external
		 * strings are enabled)
		 */
		duk_free_hstring_inner(heap, h);
		DUK_FREE(heap, h);
		(*count_free)++;
	}
}
#endif  /* DUK_USE_HEAPPTR16 */

DUK_LOCAL void duk__sweep_stringtable_chain(duk_heap *heap, duk_size_t *out_count_keep) {
	duk_strtab_entry *e;
	duk_uint_fast32_t i;
	duk_size_t count_free = 0;
	duk_size_t count_keep = 0;
	duk_size_t j, n;
#if defined(DUK_USE_HEAPPTR16)
	duk_uint16_t *lst;
#else
	duk_hstring **lst;
#endif

	DUK_DD(DUK_DDPRINT("duk__sweep_stringtable: %p", (void *) heap));

	/* Non-zero refcounts should not happen for unreachable strings,
	 * because we refcount finalize all unreachable objects which
	 * should have decreased unreachable string refcounts to zero
	 * (even for cycles).
	 */

	for (i = 0; i < DUK_STRTAB_CHAIN_SIZE; i++) {
		e = heap->strtable + i;
		if (e->listlen == 0) {
#if defined(DUK_USE_HEAPPTR16)
			duk__sweep_string_chain16(heap, &e->u.str16, &count_keep, &count_free);
#else
			duk__sweep_string_chain(heap, &e->u.str, &count_keep, &count_free);
#endif
		} else {
#if defined(DUK_USE_HEAPPTR16)
			lst = (duk_uint16_t *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, e->u.strlist16);
#else
			lst = e->u.strlist;
#endif
			for (j = 0, n = e->listlen; j < n; j++) {
#if defined(DUK_USE_HEAPPTR16)
				duk__sweep_string_chain16(heap, lst + j, &count_keep, &count_free);
#else
				duk__sweep_string_chain(heap, lst + j, &count_keep, &count_free);
#endif
			}
		}
	}

	DUK_D(DUK_DPRINT("mark-and-sweep sweep stringtable: %ld freed, %ld kept",
	                 (long) count_free, (long) count_keep));
	*out_count_keep = count_keep;
}
#endif  /* DUK_USE_STRTAB_CHAIN */

#if defined(DUK_USE_STRTAB_PROBE)
DUK_LOCAL void duk__sweep_stringtable_probe(duk_heap *heap, duk_size_t *out_count_keep) {
	duk_hstring *h;
	duk_uint_fast32_t i;
#ifdef DUK_USE_DEBUG
	duk_size_t count_free = 0;
#endif
	duk_size_t count_keep = 0;

	DUK_DD(DUK_DDPRINT("duk__sweep_stringtable: %p", (void *) heap));

	for (i = 0; i < heap->st_size; i++) {
#if defined(DUK_USE_HEAPPTR16)
		h = (duk_hstring *) DUK_USE_HEAPPTR_DEC16(heap->heap_udata, heap->strtable16[i]);
#else
		h = heap->strtable[i];
#endif
		if (h == NULL || h == DUK_STRTAB_DELETED_MARKER(heap)) {
			continue;
		} else if (DUK_HEAPHDR_HAS_REACHABLE((duk_heaphdr *) h)) {
			DUK_HEAPHDR_CLEAR_REACHABLE((duk_heaphdr *) h);
			count_keep++;
			continue;
		}

#ifdef DUK_USE_DEBUG
		count_free++;
#endif

#if defined(DUK_USE_REFERENCE_COUNTING)
		/* Non-zero refcounts should not happen for unreachable strings,
		 * because we refcount finalize all unreachable objects which
		 * should have decreased unreachable string refcounts to zero
		 * (even for cycles).
		 */
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) h) == 0);
#endif

		DUK_DDD(DUK_DDDPRINT("sweep string, not reachable: %p", (void *) h));

		/* deal with weak references first */
		duk_heap_strcache_string_remove(heap, (duk_hstring *) h);

		/* remove the string (mark DELETED), could also call
		 * duk_heap_string_remove() but that would be slow and
		 * pointless because we already know the slot.
		 */
#if defined(DUK_USE_HEAPPTR16)
		heap->strtable16[i] = heap->heapptr_deleted16;
#else
		heap->strtable[i] = DUK_STRTAB_DELETED_MARKER(heap);
#endif

		/* free inner references (these exist e.g. when external
		 * strings are enabled)
		 */
		duk_free_hstring_inner(heap, (duk_hstring *) h);

		/* finally free the struct itself */
		DUK_FREE(heap, h);
	}

#ifdef DUK_USE_DEBUG
	DUK_D(DUK_DPRINT("mark-and-sweep sweep stringtable: %ld freed, %ld kept",
	                 (long) count_free, (long) count_keep));
#endif
	*out_count_keep = count_keep;
}
#endif  /* DUK_USE_STRTAB_PROBE */

/*
 *  Sweep heap
 */

DUK_LOCAL void duk__sweep_heap(duk_heap *heap, duk_int_t flags, duk_size_t *out_count_keep) {
	duk_heaphdr *prev;  /* last element that was left in the heap */
	duk_heaphdr *curr;
	duk_heaphdr *next;
#ifdef DUK_USE_DEBUG
	duk_size_t count_free = 0;
	duk_size_t count_finalize = 0;
	duk_size_t count_rescue = 0;
#endif
	duk_size_t count_keep = 0;

	DUK_UNREF(flags);
	DUK_DD(DUK_DDPRINT("duk__sweep_heap: %p", (void *) heap));

	prev = NULL;
	curr = heap->heap_allocated;
	heap->heap_allocated = NULL;
	while (curr) {
		/* Strings and ROM objects are never placed on the heap allocated list. */
		DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(curr) != DUK_HTYPE_STRING);
		DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY(curr));

		next = DUK_HEAPHDR_GET_NEXT(heap, curr);

		if (DUK_HEAPHDR_HAS_REACHABLE(curr)) {
			/*
			 *  Reachable object, keep
			 */

			DUK_DDD(DUK_DDDPRINT("sweep, reachable: %p", (void *) curr));

			if (DUK_HEAPHDR_HAS_FINALIZABLE(curr)) {
				/*
				 *  If object has been marked finalizable, move it to the
				 *  "to be finalized" work list.  It will be collected on
				 *  the next mark-and-sweep if it is still unreachable
				 *  after running the finalizer.
				 */

				DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(curr));
				DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(curr) == DUK_HTYPE_OBJECT);
				DUK_DDD(DUK_DDDPRINT("object has finalizer, move to finalization work list: %p", (void *) curr));

#ifdef DUK_USE_DOUBLE_LINKED_HEAP
				if (heap->finalize_list) {
					DUK_HEAPHDR_SET_PREV(heap, heap->finalize_list, curr);
				}
				DUK_HEAPHDR_SET_PREV(heap, curr, NULL);
#endif
				DUK_HEAPHDR_SET_NEXT(heap, curr, heap->finalize_list);
				DUK_ASSERT_HEAPHDR_LINKS(heap, curr);
				heap->finalize_list = curr;
#ifdef DUK_USE_DEBUG
				count_finalize++;
#endif
			} else {
				/*
				 *  Object will be kept; queue object back to heap_allocated (to tail)
				 */

				if (DUK_HEAPHDR_HAS_FINALIZED(curr)) {
					/*
					 *  Object's finalizer was executed on last round, and
					 *  object has been happily rescued.
					 */

					DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(curr));
					DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(curr) == DUK_HTYPE_OBJECT);
					DUK_DD(DUK_DDPRINT("object rescued during mark-and-sweep finalization: %p", (void *) curr));
#ifdef DUK_USE_DEBUG
					count_rescue++;
#endif
				} else {
					/*
					 *  Plain, boring reachable object.
					 */
					DUK_DD(DUK_DDPRINT("keep object: %!iO", curr));
					count_keep++;
				}

				if (!heap->heap_allocated) {
					heap->heap_allocated = curr;
				}
				if (prev) {
					DUK_HEAPHDR_SET_NEXT(heap, prev, curr);
				}
#ifdef DUK_USE_DOUBLE_LINKED_HEAP
				DUK_HEAPHDR_SET_PREV(heap, curr, prev);
#endif
				DUK_ASSERT_HEAPHDR_LINKS(heap, prev);
				DUK_ASSERT_HEAPHDR_LINKS(heap, curr);
				prev = curr;
			}

			DUK_HEAPHDR_CLEAR_REACHABLE(curr);
			DUK_HEAPHDR_CLEAR_FINALIZED(curr);
			DUK_HEAPHDR_CLEAR_FINALIZABLE(curr);

			DUK_ASSERT(!DUK_HEAPHDR_HAS_REACHABLE(curr));
			DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(curr));
			DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(curr));

			curr = next;
		} else {
			/*
			 *  Unreachable object, free
			 */

			DUK_DDD(DUK_DDDPRINT("sweep, not reachable: %p", (void *) curr));

#if defined(DUK_USE_REFERENCE_COUNTING)
			/* Non-zero refcounts should not happen because we refcount
			 * finalize all unreachable objects which should cancel out
			 * refcounts (even for cycles).
			 */
			DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(curr) == 0);
#endif
			DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(curr));

			if (DUK_HEAPHDR_HAS_FINALIZED(curr)) {
				DUK_DDD(DUK_DDDPRINT("finalized object not rescued: %p", (void *) curr));
			}

			/* Note: object cannot be a finalizable unreachable object, as
			 * they have been marked temporarily reachable for this round,
			 * and are handled above.
			 */

#ifdef DUK_USE_DEBUG
			count_free++;
#endif

			/* weak refs should be handled here, but no weak refs for
			 * any non-string objects exist right now.
			 */

			/* free object and all auxiliary (non-heap) allocs */
			duk_heap_free_heaphdr_raw(heap, curr);

			curr = next;
		}
	}
	if (prev) {
		DUK_HEAPHDR_SET_NEXT(heap, prev, NULL);
	}
	DUK_ASSERT_HEAPHDR_LINKS(heap, prev);

#ifdef DUK_USE_DEBUG
	DUK_D(DUK_DPRINT("mark-and-sweep sweep objects (non-string): %ld freed, %ld kept, %ld rescued, %ld queued for finalization",
	                 (long) count_free, (long) count_keep, (long) count_rescue, (long) count_finalize));
#endif
	*out_count_keep = count_keep;
}

/*
 *  Run (object) finalizers in the "to be finalized" work list.
 */

DUK_LOCAL void duk__run_object_finalizers(duk_heap *heap, duk_small_uint_t flags) {
	duk_heaphdr *curr;
	duk_heaphdr *next;
#ifdef DUK_USE_DEBUG
	duk_size_t count = 0;
#endif
	duk_hthread *thr;

	DUK_DD(DUK_DDPRINT("duk__run_object_finalizers: %p", (void *) heap));

	thr = duk__get_temp_hthread(heap);
	DUK_ASSERT(thr != NULL);

	curr = heap->finalize_list;
	while (curr) {
		DUK_DDD(DUK_DDDPRINT("mark-and-sweep finalize: %p", (void *) curr));

		DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(curr) == DUK_HTYPE_OBJECT);  /* only objects have finalizers */
		DUK_ASSERT(!DUK_HEAPHDR_HAS_REACHABLE(curr));                /* flags have been already cleared */
		DUK_ASSERT(!DUK_HEAPHDR_HAS_TEMPROOT(curr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(curr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(curr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY(curr));  /* No finalizers for ROM objects */

		if (DUK_LIKELY((flags & DUK_MS_FLAG_SKIP_FINALIZERS) == 0)) {
			/* Run the finalizer, duk_hobject_run_finalizer() sets FINALIZED.
			 * Next mark-and-sweep will collect the object unless it has
			 * become reachable (i.e. rescued).  FINALIZED prevents the
			 * finalizer from being executed again before that.
			 */
			duk_hobject_run_finalizer(thr, (duk_hobject *) curr);  /* must never longjmp */
			DUK_ASSERT(DUK_HEAPHDR_HAS_FINALIZED(curr));
		} else {
			/* Used during heap destruction: don't actually run finalizers
			 * because we're heading into forced finalization.  Instead,
			 * queue finalizable objects back to the heap_allocated list.
			 */
			DUK_D(DUK_DPRINT("skip finalizers flag set, queue object to heap_allocated without finalizing"));
			DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(curr));
		}

		/* queue back to heap_allocated */
		next = DUK_HEAPHDR_GET_NEXT(heap, curr);
		DUK_HEAP_INSERT_INTO_HEAP_ALLOCATED(heap, curr);

		curr = next;
#ifdef DUK_USE_DEBUG
		count++;
#endif
	}

	/* finalize_list will always be processed completely */
	heap->finalize_list = NULL;

#ifdef DUK_USE_DEBUG
	DUK_D(DUK_DPRINT("mark-and-sweep finalize objects: %ld finalizers called", (long) count));
#endif
}

/*
 *  Object compaction.
 *
 *  Compaction is assumed to never throw an error.
 */

DUK_LOCAL int duk__protected_compact_object(duk_context *ctx) {
	/* XXX: for threads, compact value stack, call stack, catch stack? */

	duk_hobject *obj = duk_get_hobject(ctx, -1);
	DUK_ASSERT(obj != NULL);
	duk_hobject_compact_props((duk_hthread *) ctx, obj);
	return 0;
}

#ifdef DUK_USE_DEBUG
DUK_LOCAL void duk__compact_object_list(duk_heap *heap, duk_hthread *thr, duk_heaphdr *start, duk_size_t *p_count_check, duk_size_t *p_count_compact, duk_size_t *p_count_bytes_saved) {
#else
DUK_LOCAL void duk__compact_object_list(duk_heap *heap, duk_hthread *thr, duk_heaphdr *start) {
#endif
	duk_heaphdr *curr;
#ifdef DUK_USE_DEBUG
	duk_size_t old_size, new_size;
#endif
	duk_hobject *obj;

	DUK_UNREF(heap);

	curr = start;
	while (curr) {
		DUK_DDD(DUK_DDDPRINT("mark-and-sweep compact: %p", (void *) curr));

		if (DUK_HEAPHDR_GET_TYPE(curr) != DUK_HTYPE_OBJECT) {
			goto next;
		}
		obj = (duk_hobject *) curr;

#ifdef DUK_USE_DEBUG
		old_size = DUK_HOBJECT_P_COMPUTE_SIZE(DUK_HOBJECT_GET_ESIZE(obj),
		                                      DUK_HOBJECT_GET_ASIZE(obj),
		                                      DUK_HOBJECT_GET_HSIZE(obj));
#endif

		DUK_DD(DUK_DDPRINT("compact object: %p", (void *) obj));
		duk_push_hobject((duk_context *) thr, obj);
		/* XXX: disable error handlers for duration of compaction? */
		duk_safe_call((duk_context *) thr, duk__protected_compact_object, 1, 0);

#ifdef DUK_USE_DEBUG
		new_size = DUK_HOBJECT_P_COMPUTE_SIZE(DUK_HOBJECT_GET_ESIZE(obj),
		                                      DUK_HOBJECT_GET_ASIZE(obj),
		                                      DUK_HOBJECT_GET_HSIZE(obj));
#endif

#ifdef DUK_USE_DEBUG
		(*p_count_compact)++;
		(*p_count_bytes_saved) += (duk_size_t) (old_size - new_size);
#endif

	 next:
		curr = DUK_HEAPHDR_GET_NEXT(heap, curr);
#ifdef DUK_USE_DEBUG
		(*p_count_check)++;
#endif
	}
}

DUK_LOCAL void duk__compact_objects(duk_heap *heap) {
	/* XXX: which lists should participate?  to be finalized? */
#ifdef DUK_USE_DEBUG
	duk_size_t count_check = 0;
	duk_size_t count_compact = 0;
	duk_size_t count_bytes_saved = 0;
#endif
	duk_hthread *thr;

	DUK_DD(DUK_DDPRINT("duk__compact_objects: %p", (void *) heap));

	thr = duk__get_temp_hthread(heap);
	DUK_ASSERT(thr != NULL);

#ifdef DUK_USE_DEBUG
	duk__compact_object_list(heap, thr, heap->heap_allocated, &count_check, &count_compact, &count_bytes_saved);
	duk__compact_object_list(heap, thr, heap->finalize_list, &count_check, &count_compact, &count_bytes_saved);
#ifdef DUK_USE_REFERENCE_COUNTING
	duk__compact_object_list(heap, thr, heap->refzero_list, &count_check, &count_compact, &count_bytes_saved);
#endif
#else
	duk__compact_object_list(heap, thr, heap->heap_allocated);
	duk__compact_object_list(heap, thr, heap->finalize_list);
#ifdef DUK_USE_REFERENCE_COUNTING
	duk__compact_object_list(heap, thr, heap->refzero_list);
#endif
#endif

#ifdef DUK_USE_DEBUG
	DUK_D(DUK_DPRINT("mark-and-sweep compact objects: %ld checked, %ld compaction attempts, %ld bytes saved by compaction",
	                 (long) count_check, (long) count_compact, (long) count_bytes_saved));
#endif
}

/*
 *  Assertion helpers.
 */

#ifdef DUK_USE_ASSERTIONS
DUK_LOCAL void duk__assert_heaphdr_flags(duk_heap *heap) {
	duk_heaphdr *hdr;

	hdr = heap->heap_allocated;
	while (hdr) {
		DUK_ASSERT(!DUK_HEAPHDR_HAS_REACHABLE(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_TEMPROOT(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(hdr));
		/* may have FINALIZED */
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}

#ifdef DUK_USE_REFERENCE_COUNTING
	hdr = heap->refzero_list;
	while (hdr) {
		DUK_ASSERT(!DUK_HEAPHDR_HAS_REACHABLE(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_TEMPROOT(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(hdr));
		DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(hdr));
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
#endif  /* DUK_USE_REFERENCE_COUNTING */
}

#ifdef DUK_USE_REFERENCE_COUNTING
DUK_LOCAL void duk__assert_valid_refcounts(duk_heap *heap) {
	duk_heaphdr *hdr = heap->heap_allocated;
	while (hdr) {
		if (DUK_HEAPHDR_GET_REFCOUNT(hdr) == 0 &&
		    DUK_HEAPHDR_HAS_FINALIZED(hdr)) {
			/* An object may be in heap_allocated list with a zero
			 * refcount if it has just been finalized and is waiting
			 * to be collected by the next cycle.
			 */
		} else if (DUK_HEAPHDR_GET_REFCOUNT(hdr) == 0) {
			/* An object may be in heap_allocated list with a zero
			 * refcount also if it is a temporary object created by
			 * a finalizer; because finalization now runs inside
			 * mark-and-sweep, such objects will not be queued to
			 * refzero_list and will thus appear here with refcount
			 * zero.
			 */
#if 0  /* this case can no longer occur because refcount is unsigned */
		} else if (DUK_HEAPHDR_GET_REFCOUNT(hdr) < 0) {
			DUK_D(DUK_DPRINT("invalid refcount: %ld, %p -> %!O",
			                 (hdr != NULL ? (long) DUK_HEAPHDR_GET_REFCOUNT(hdr) : (long) 0),
			                 (void *) hdr, (duk_heaphdr *) hdr));
			DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(hdr) > 0);
#endif
		}
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}
#endif  /* DUK_USE_REFERENCE_COUNTING */
#endif  /* DUK_USE_ASSERTIONS */

/*
 *  Finalizer torture.  Do one fake finalizer call which causes side effects
 *  similar to one or more finalizers on actual objects.
 */

#if defined(DUK_USE_MARKANDSWEEP_FINALIZER_TORTURE)
DUK_LOCAL duk_ret_t duk__markandsweep_fake_finalizer(duk_context *ctx) {
	DUK_D(DUK_DPRINT("fake mark-and-sweep torture finalizer executed"));

	/* Require a lot of stack to force a value stack grow/shrink.
	 * Recursive mark-and-sweep is prevented by allocation macros
	 * so this won't trigger another mark-and-sweep.
	 */
	duk_require_stack(ctx, 100000);

	/* XXX: do something to force a callstack grow/shrink, perhaps
	 * just a manual forced resize or a forced relocating realloc?
	 */

	return 0;
}

DUK_LOCAL void duk__markandsweep_torture_finalizer(duk_hthread *thr) {
	duk_context *ctx;
	duk_int_t rc;

	DUK_ASSERT(thr != NULL);
	ctx = (duk_context *) thr;

	/* Avoid fake finalization when callstack limit has been reached.
	 * Otherwise a callstack limit error will be created, then refzero'ed.
	 */
	if (thr->heap->call_recursion_depth >= thr->heap->call_recursion_limit ||
	    thr->callstack_size + 2 * DUK_CALLSTACK_GROW_STEP >= thr->callstack_max /*approximate*/) {
		DUK_D(DUK_DPRINT("call recursion depth reached, avoid fake mark-and-sweep torture finalizer"));
		return;
	}

	/* Run fake finalizer.  Avoid creating unnecessary garbage. */
	duk_push_c_function(ctx, duk__markandsweep_fake_finalizer, 0 /*nargs*/);
	rc = duk_pcall(ctx, 0 /*nargs*/);
	DUK_UNREF(rc);  /* ignored */
	duk_pop(ctx);
}
#endif  /* DUK_USE_MARKANDSWEEP_FINALIZER_TORTURE */

/*
 *  Main mark-and-sweep function.
 *
 *  'flags' represents the features requested by the caller.  The current
 *  heap->mark_and_sweep_base_flags is ORed automatically into the flags;
 *  the base flags mask typically prevents certain mark-and-sweep operations
 *  to avoid trouble.
 */

DUK_INTERNAL duk_bool_t duk_heap_mark_and_sweep(duk_heap *heap, duk_small_uint_t flags) {
	duk_hthread *thr;
	duk_size_t count_keep_obj;
	duk_size_t count_keep_str;
#ifdef DUK_USE_VOLUNTARY_GC
	duk_size_t tmp;
#endif

	/* XXX: thread selection for mark-and-sweep is currently a hack.
	 * If we don't have a thread, the entire mark-and-sweep is now
	 * skipped (although we could just skip finalizations).
	 */

	/* If thr != NULL, the thr may still be in the middle of
	 * initialization.
	 * XXX: Improve the thread viability test.
	 */
	thr = duk__get_temp_hthread(heap);
	if (thr == NULL) {
		DUK_D(DUK_DPRINT("gc skipped because we don't have a temp thread"));

		/* reset voluntary gc trigger count */
#ifdef DUK_USE_VOLUNTARY_GC
		heap->mark_and_sweep_trigger_counter = DUK_HEAP_MARK_AND_SWEEP_TRIGGER_SKIP;
#endif
		return 0;  /* OK */
	}

	/* If debugger is paused, garbage collection is disabled by default. */
	/* XXX: will need a force flag if garbage collection is triggered
	 * explicitly during paused state.
	 */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	if (DUK_HEAP_IS_PAUSED(heap)) {
		/* Checking this here rather that in memory alloc primitives
		 * reduces checking code there but means a failed allocation
		 * will go through a few retries before giving up.  That's
		 * fine because this only happens during debugging.
		 */
		DUK_D(DUK_DPRINT("gc skipped because debugger is paused"));
		return 0;
	}
#endif

	DUK_D(DUK_DPRINT("garbage collect (mark-and-sweep) starting, requested flags: 0x%08lx, effective flags: 0x%08lx",
	                 (unsigned long) flags, (unsigned long) (flags | heap->mark_and_sweep_base_flags)));

	flags |= heap->mark_and_sweep_base_flags;

	/*
	 *  Assertions before
	 */

#ifdef DUK_USE_ASSERTIONS
	DUK_ASSERT(!DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap));
	DUK_ASSERT(!DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED(heap));
	DUK_ASSERT(heap->mark_and_sweep_recursion_depth == 0);
	duk__assert_heaphdr_flags(heap);
#ifdef DUK_USE_REFERENCE_COUNTING
	/* Note: DUK_HEAP_HAS_REFZERO_FREE_RUNNING(heap) may be true; a refcount
	 * finalizer may trigger a mark-and-sweep.
	 */
	duk__assert_valid_refcounts(heap);
#endif  /* DUK_USE_REFERENCE_COUNTING */
#endif  /* DUK_USE_ASSERTIONS */

	/*
	 *  Begin
	 */

	DUK_HEAP_SET_MARKANDSWEEP_RUNNING(heap);

	/*
	 *  Mark roots, hoping that recursion limit is not normally hit.
	 *  If recursion limit is hit, run additional reachability rounds
	 *  starting from "temproots" until marking is complete.
	 *
	 *  Marking happens in two phases: first we mark actual reachability
	 *  roots (and run "temproots" to complete the process).  Then we
	 *  check which objects are unreachable and are finalizable; such
	 *  objects are marked as FINALIZABLE and marked as reachability
	 *  (and "temproots" is run again to complete the process).
	 *
	 *  The heap finalize_list must also be marked as a reachability root.
	 *  There may be objects on the list from a previous round if the
	 *  previous run had finalizer skip flag.
	 */

	duk__mark_roots_heap(heap);               /* main reachability roots */
#ifdef DUK_USE_REFERENCE_COUNTING
	duk__mark_refzero_list(heap);             /* refzero_list treated as reachability roots */
#endif
	duk__mark_temproots_by_heap_scan(heap);   /* temproots */

	duk__mark_finalizable(heap);              /* mark finalizable as reachability roots */
	duk__mark_finalize_list(heap);            /* mark finalizer work list as reachability roots */
	duk__mark_temproots_by_heap_scan(heap);   /* temproots */

	/*
	 *  Sweep garbage and remove marking flags, and move objects with
	 *  finalizers to the finalizer work list.
	 *
	 *  Objects to be swept need to get their refcounts finalized before
	 *  they are swept.  In other words, their target object refcounts
	 *  need to be decreased.  This has to be done before freeing any
	 *  objects to avoid decref'ing dangling pointers (which may happen
	 *  even without bugs, e.g. with reference loops)
	 *
	 *  Because strings don't point to other heap objects, similar
	 *  finalization is not necessary for strings.
	 */

	/* XXX: more emergency behavior, e.g. find smaller hash sizes etc */

#ifdef DUK_USE_REFERENCE_COUNTING
	duk__finalize_refcounts(heap);
#endif
	duk__sweep_heap(heap, flags, &count_keep_obj);
#if defined(DUK_USE_STRTAB_CHAIN)
	duk__sweep_stringtable_chain(heap, &count_keep_str);
#elif defined(DUK_USE_STRTAB_PROBE)
	duk__sweep_stringtable_probe(heap, &count_keep_str);
#else
#error internal error, invalid strtab options
#endif
#ifdef DUK_USE_REFERENCE_COUNTING
	duk__clear_refzero_list_flags(heap);
#endif
	duk__clear_finalize_list_flags(heap);

	/*
	 *  Object compaction (emergency only).
	 *
	 *  Object compaction is a separate step after sweeping, as there is
	 *  more free memory for it to work with.  Also, currently compaction
	 *  may insert new objects into the heap allocated list and the string
	 *  table which we don't want to do during a sweep (the reachability
	 *  flags of such objects would be incorrect).  The objects inserted
	 *  are currently:
	 *
	 *    - a temporary duk_hbuffer for a new properties allocation
	 *    - if array part is abandoned, string keys are interned
	 *
	 *  The object insertions go to the front of the list, so they do not
	 *  cause an infinite loop (they are not compacted).
	 */

	if ((flags & DUK_MS_FLAG_EMERGENCY) &&
	    !(flags & DUK_MS_FLAG_NO_OBJECT_COMPACTION)) {
		duk__compact_objects(heap);
	}

	/*
	 *  String table resize check.
	 *
	 *  Note: this may silently (and safely) fail if GC is caused by an
	 *  allocation call in stringtable resize_hash().  Resize_hash()
	 *  will prevent a recursive call to itself by setting the
	 *  DUK_MS_FLAG_NO_STRINGTABLE_RESIZE in heap->mark_and_sweep_base_flags.
	 */

	/* XXX: stringtable emergency compaction? */

	/* XXX: remove this feature entirely? it would only matter for
	 * emergency GC.  Disable for lowest memory builds.
	 */
#if defined(DUK_USE_MS_STRINGTABLE_RESIZE)
	if (!(flags & DUK_MS_FLAG_NO_STRINGTABLE_RESIZE)) {
		DUK_DD(DUK_DDPRINT("resize stringtable: %p", (void *) heap));
		duk_heap_force_strtab_resize(heap);
	} else {
		DUK_D(DUK_DPRINT("stringtable resize skipped because DUK_MS_FLAG_NO_STRINGTABLE_RESIZE is set"));
	}
#endif

	/*
	 *  Finalize objects in the finalization work list.  Finalized
	 *  objects are queued back to heap_allocated with FINALIZED set.
	 *
	 *  Since finalizers may cause arbitrary side effects, they are
	 *  prevented during string table and object property allocation
	 *  resizing using the DUK_MS_FLAG_NO_FINALIZERS flag in
	 *  heap->mark_and_sweep_base_flags.  In this case the objects
	 *  remain in the finalization work list after mark-and-sweep
	 *  exits and they may be finalized on the next pass.
	 *
	 *  Finalization currently happens inside "MARKANDSWEEP_RUNNING"
	 *  protection (no mark-and-sweep may be triggered by the
	 *  finalizers).  As a side effect:
	 *
	 *    1) an out-of-memory error inside a finalizer will not
	 *       cause a mark-and-sweep and may cause the finalizer
	 *       to fail unnecessarily
	 *
	 *    2) any temporary objects whose refcount decreases to zero
	 *       during finalization will not be put into refzero_list;
	 *       they can only be collected by another mark-and-sweep
	 *
	 *  This is not optimal, but since the sweep for this phase has
	 *  already happened, this is probably good enough for now.
	 */

#if defined(DUK_USE_MARKANDSWEEP_FINALIZER_TORTURE)
	/* Cannot simulate individual finalizers because finalize_list only
	 * contains objects with actual finalizers.  But simulate side effects
	 * from finalization by doing a bogus function call and resizing the
	 * stacks.
	 */
	if (flags & DUK_MS_FLAG_NO_FINALIZERS) {
		DUK_D(DUK_DPRINT("skip mark-and-sweep torture finalizer, DUK_MS_FLAG_NO_FINALIZERS is set"));
	} else if (!(thr->valstack != NULL && thr->callstack != NULL && thr->catchstack != NULL)) {
		DUK_D(DUK_DPRINT("skip mark-and-sweep torture finalizer, thread not yet viable"));
	} else {
		DUK_D(DUK_DPRINT("run mark-and-sweep torture finalizer"));
		duk__markandsweep_torture_finalizer(thr);
	}
#endif  /* DUK_USE_MARKANDSWEEP_FINALIZER_TORTURE */

	if (flags & DUK_MS_FLAG_NO_FINALIZERS) {
		DUK_D(DUK_DPRINT("finalizer run skipped because DUK_MS_FLAG_NO_FINALIZERS is set"));
	} else {
		duk__run_object_finalizers(heap, flags);
	}

	/*
	 *  Finish
	 */

	DUK_HEAP_CLEAR_MARKANDSWEEP_RUNNING(heap);

	/*
	 *  Assertions after
	 */

#ifdef DUK_USE_ASSERTIONS
	DUK_ASSERT(!DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap));
	DUK_ASSERT(!DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED(heap));
	DUK_ASSERT(heap->mark_and_sweep_recursion_depth == 0);
	duk__assert_heaphdr_flags(heap);
#ifdef DUK_USE_REFERENCE_COUNTING
	/* Note: DUK_HEAP_HAS_REFZERO_FREE_RUNNING(heap) may be true; a refcount
	 * finalizer may trigger a mark-and-sweep.
	 */
	duk__assert_valid_refcounts(heap);
#endif  /* DUK_USE_REFERENCE_COUNTING */
#endif  /* DUK_USE_ASSERTIONS */

	/*
	 *  Reset trigger counter
	 */

#ifdef DUK_USE_VOLUNTARY_GC
	tmp = (count_keep_obj + count_keep_str) / 256;
	heap->mark_and_sweep_trigger_counter = (duk_int_t) (
	    (tmp * DUK_HEAP_MARK_AND_SWEEP_TRIGGER_MULT) +
	    DUK_HEAP_MARK_AND_SWEEP_TRIGGER_ADD);
	DUK_D(DUK_DPRINT("garbage collect (mark-and-sweep) finished: %ld objects kept, %ld strings kept, trigger reset to %ld",
	                 (long) count_keep_obj, (long) count_keep_str, (long) heap->mark_and_sweep_trigger_counter));
#else
	DUK_D(DUK_DPRINT("garbage collect (mark-and-sweep) finished: %ld objects kept, %ld strings kept, no voluntary trigger",
	                 (long) count_keep_obj, (long) count_keep_str));
#endif

	return 0;  /* OK */
}

#else  /* DUK_USE_MARK_AND_SWEEP */

/* no mark-and-sweep gc */

#endif  /* DUK_USE_MARK_AND_SWEEP */
