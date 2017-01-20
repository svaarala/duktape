/*
 *  Mark-and-sweep garbage collection.
 */

#include "duk_internal.h"

DUK_LOCAL_DECL void duk__mark_heaphdr(duk_heap *heap, duk_heaphdr *h);
DUK_LOCAL_DECL void duk__mark_tval(duk_heap *heap, duk_tval *tv);

/*
 *  Misc
 */

/*
 *  Marking functions for heap types: mark children recursively.
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

	/* Fast path for objects which don't have a subclass struct, or have a
	 * subclass struct but nothing that needs marking in the subclass struct.
	 */
	if (DUK_HOBJECT_HAS_FASTREFS(h)) {
		DUK_ASSERT(DUK_HOBJECT_ALLOWS_FASTREFS(h));
		return;
	}
	DUK_ASSERT(DUK_HOBJECT_PROHIBITS_FASTREFS(h));

	if (DUK_HOBJECT_IS_COMPFUNC(h)) {
		duk_hcompfunc *f = (duk_hcompfunc *) h;
		duk_tval *tv, *tv_end;
		duk_hobject **fn, **fn_end;

		/* 'data' is reachable through every compiled function which
		 * contains a reference.
		 */

		duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_HCOMPFUNC_GET_DATA(heap, f));
		duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_HCOMPFUNC_GET_LEXENV(heap, f));
		duk__mark_heaphdr(heap, (duk_heaphdr *) DUK_HCOMPFUNC_GET_VARENV(heap, f));

		if (DUK_HCOMPFUNC_GET_DATA(heap, f) != NULL) {
			tv = DUK_HCOMPFUNC_GET_CONSTS_BASE(heap, f);
			tv_end = DUK_HCOMPFUNC_GET_CONSTS_END(heap, f);
			while (tv < tv_end) {
				duk__mark_tval(heap, tv);
				tv++;
			}

			fn = DUK_HCOMPFUNC_GET_FUNCS_BASE(heap, f);
			fn_end = DUK_HCOMPFUNC_GET_FUNCS_END(heap, f);
			while (fn < fn_end) {
				duk__mark_heaphdr(heap, (duk_heaphdr *) *fn);
				fn++;
			}
		} else {
			/* May happen in some out-of-memory corner cases. */
			DUK_D(DUK_DPRINT("duk_hcompfunc 'data' is NULL, skipping marking"));
		}
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	} else if (DUK_HOBJECT_IS_BUFOBJ(h)) {
		duk_hbufobj *b = (duk_hbufobj *) h;
		duk__mark_heaphdr(heap, (duk_heaphdr *) b->buf);
		duk__mark_heaphdr(heap, (duk_heaphdr *) b->buf_prop);
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */
	} else if (DUK_HOBJECT_IS_DECENV(h)) {
		duk_hdecenv *e = (duk_hdecenv *) h;
		DUK_ASSERT_HDECENV_VALID(e);
		duk__mark_heaphdr(heap, (duk_heaphdr *) e->thread);
		duk__mark_heaphdr(heap, (duk_heaphdr *) e->varmap);
	} else if (DUK_HOBJECT_IS_OBJENV(h)) {
		duk_hobjenv *e = (duk_hobjenv *) h;
		DUK_ASSERT_HOBJENV_VALID(e);
		duk__mark_heaphdr(heap, (duk_heaphdr *) e->target);
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
#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
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
	} else {
		/* We may come here if the object should have a FASTREFS flag
		 * but it's missing for some reason.  Assert for never getting
		 * here; however, other than performance, this is harmless.
		 */
		DUK_D(DUK_DPRINT("missing FASTREFS flag for: %!iO", h));
		DUK_ASSERT(0);
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
#if defined(DUK_USE_ASSERTIONS) && defined(DUK_USE_REFERENCE_COUNTING)
	h->h_assert_refcount++;  /* Comparison refcount: bump even if already reachable. */
#endif
	if (DUK_HEAPHDR_HAS_REACHABLE(h)) {
		DUK_DDD(DUK_DDDPRINT("already marked reachable, skip"));
		return;
	}
	DUK_HEAPHDR_SET_REACHABLE(h);

	if (heap->ms_recursion_depth >= DUK_USE_MARK_AND_SWEEP_RECLIMIT) {
		/* log this with a normal debug level because this should be relatively rare */
		DUK_D(DUK_DPRINT("mark-and-sweep recursion limit reached, marking as temproot: %p", (void *) h));
		DUK_HEAP_SET_MARKANDSWEEP_RECLIMIT_REACHED(heap);
		DUK_HEAPHDR_SET_TEMPROOT(h);
		return;
	}

	heap->ms_recursion_depth++;
	DUK_ASSERT(heap->ms_recursion_depth != 0);  /* Wrap. */

	switch (DUK_HEAPHDR_GET_TYPE(h)) {
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

	DUK_ASSERT(heap->ms_recursion_depth > 0);
	heap->ms_recursion_depth--;
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
 *  Mark unreachable, finalizable objects.
 *
 *  Such objects will be moved aside and their finalizers run later.  They have
 *  to be treated as reachability roots for their properties etc to remain
 *  allocated.  This marking is only done for unreachable values which would
 *  be swept later.
 *
 *  Objects are first marked FINALIZABLE and only then marked as reachability
 *  roots; otherwise circular references might be handled inconsistently.
 */

#if defined(DUK_USE_FINALIZER_SUPPORT)
DUK_LOCAL void duk__mark_finalizable(duk_heap *heap) {
	duk_heaphdr *hdr;
	duk_size_t count_finalizable = 0;

	DUK_DD(DUK_DDPRINT("duk__mark_finalizable: %p", (void *) heap));

	DUK_ASSERT(heap->heap_thread != NULL);

	hdr = heap->heap_allocated;
	while (hdr != NULL) {
		/* A finalizer is looked up from the object and up its
		 * prototype chain (which allows inherited finalizers).
		 * The finalizer is checked for using a duk_hobject flag
		 * which is kept in sync with the presence and callability
		 * of a _Finalizer hidden symbol.
		 */

		if (!DUK_HEAPHDR_HAS_REACHABLE(hdr) &&
		    DUK_HEAPHDR_IS_OBJECT(hdr) &&
		    !DUK_HEAPHDR_HAS_FINALIZED(hdr) &&
		    DUK_HOBJECT_HAS_FINALIZER_FAST(heap, (duk_hobject *) hdr)) {
			/* heaphdr:
			 *  - is not reachable
			 *  - is an object
			 *  - is not a finalized object waiting for rescue/keep decision
			 *  - has a finalizer
			 */

			DUK_DD(DUK_DDPRINT("unreachable heap object will be "
			                   "finalized -> mark as finalizable "
			                   "and treat as a reachability root: %p",
			                   (void *) hdr));
			DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY(hdr));
			DUK_HEAPHDR_SET_FINALIZABLE(hdr);
			count_finalizable++;
		}

		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}

	if (count_finalizable == 0) {
		return;
	}

	DUK_DD(DUK_DDPRINT("marked %ld heap objects as finalizable, now mark them reachable",
	                   (long) count_finalizable));

	hdr = heap->heap_allocated;
	while (hdr != NULL) {
		if (DUK_HEAPHDR_HAS_FINALIZABLE(hdr)) {
			duk__mark_heaphdr(heap, hdr);
		}

		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}

	/* Caller will finish the marking process if we hit a recursion limit. */
}
#endif  /* DUK_USE_FINALIZER_SUPPORT */

/*
 *  Mark objects on finalize_list.
 */

#if defined(DUK_USE_FINALIZER_SUPPORT)
DUK_LOCAL void duk__mark_finalize_list(duk_heap *heap) {
	duk_heaphdr *hdr;
#if defined(DUK_USE_DEBUG)
	duk_size_t count_finalize_list = 0;
#endif

	DUK_DD(DUK_DDPRINT("duk__mark_finalize_list: %p", (void *) heap));

	hdr = heap->finalize_list;
	while (hdr != NULL) {
		duk__mark_heaphdr(heap, hdr);
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
#if defined(DUK_USE_DEBUG)
		count_finalize_list++;
#endif
	}

#if defined(DUK_USE_DEBUG)
	if (count_finalize_list > 0) {
		DUK_D(DUK_DPRINT("marked %ld objects on the finalize_list as reachable (previous finalizer run skipped)",
		                 (long) count_finalize_list));
	}
#endif
}
#endif  /* DUK_USE_FINALIZER_SUPPORT */

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

#if defined(DUK_USE_DEBUG)
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
#if defined(DUK_USE_ASSERTIONS) && defined(DUK_USE_REFERENCE_COUNTING)
	hdr->h_assert_refcount--;  /* Same node visited twice. */
#endif
	duk__mark_heaphdr(heap, hdr);

#if defined(DUK_USE_DEBUG)
	(*count)++;
#endif
}

DUK_LOCAL void duk__mark_temproots_by_heap_scan(duk_heap *heap) {
	duk_heaphdr *hdr;
#if defined(DUK_USE_DEBUG)
	duk_size_t count;
#endif

	DUK_DD(DUK_DDPRINT("duk__mark_temproots_by_heap_scan: %p", (void *) heap));

	while (DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED(heap)) {
		DUK_DD(DUK_DDPRINT("recursion limit reached, doing heap scan to continue from temproots"));

#if defined(DUK_USE_DEBUG)
		count = 0;
#endif
		DUK_HEAP_CLEAR_MARKANDSWEEP_RECLIMIT_REACHED(heap);

		hdr = heap->heap_allocated;
		while (hdr) {
#if defined(DUK_USE_DEBUG)
			duk__handle_temproot(heap, hdr, &count);
#else
			duk__handle_temproot(heap, hdr);
#endif
			hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
		}

#if defined(DUK_USE_DEBUG)
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

#if defined(DUK_USE_REFERENCE_COUNTING)
DUK_LOCAL void duk__finalize_refcounts(duk_heap *heap) {
	duk_heaphdr *hdr;

	DUK_ASSERT(heap->heap_thread != NULL);

	DUK_DD(DUK_DDPRINT("duk__finalize_refcounts: heap=%p", (void *) heap));

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

			/* Finalize using heap->heap_thread; DECREF has a
			 * suppress check for mark-and-sweep which is based
			 * on heap->ms_running.
			 */
			duk_heaphdr_refcount_finalize_norz(heap, hdr);
		}

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

#if defined(DUK_USE_FINALIZER_SUPPORT)
DUK_LOCAL void duk__clear_finalize_list_flags(duk_heap *heap) {
	duk_heaphdr *hdr;

	DUK_DD(DUK_DDPRINT("duk__clear_finalize_list_flags: %p", (void *) heap));

	hdr = heap->finalize_list;
	while (hdr) {
		DUK_HEAPHDR_CLEAR_REACHABLE(hdr);
		DUK_ASSERT(DUK_HEAPHDR_HAS_FINALIZABLE(hdr));  /* Currently true, may change if mark-and-sweep during finalization allowed. */
		/* DUK_HEAPHDR_FLAG_FINALIZED may be set. */
		DUK_ASSERT(!DUK_HEAPHDR_HAS_TEMPROOT(hdr));
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}
#endif  /* DUK_USE_FINALIZER_SUPPORT */

/*
 *  Sweep stringtable
 */

DUK_LOCAL void duk__sweep_stringtable(duk_heap *heap, duk_size_t *out_count_keep) {
	duk_hstring *h;
	duk_hstring *prev;
	duk_uint32_t i;
#if defined(DUK_USE_DEBUG)
	duk_size_t count_free = 0;
#endif
	duk_size_t count_keep = 0;

	DUK_DD(DUK_DDPRINT("duk__sweep_stringtable: %p", (void *) heap));

#if defined(DUK_USE_STRTAB_PTRCOMP)
	if (heap->strtable16 == NULL) {
#else
	if (heap->strtable == NULL) {
#endif
		goto done;
	}

	for (i = 0; i < heap->st_size; i++) {
#if defined(DUK_USE_STRTAB_PTRCOMP)
		h = DUK_USE_HEAPPTR_DEC16(heap->heap_udata, heap->strtable16[i]);
#else
		h = heap->strtable[i];
#endif
		prev = NULL;
		while (h != NULL) {
			duk_hstring *next;
			next = h->hdr.h_next;

			if (DUK_HEAPHDR_HAS_REACHABLE((duk_heaphdr *) h)) {
				DUK_HEAPHDR_CLEAR_REACHABLE((duk_heaphdr *) h);
				count_keep++;
				prev = h;
			} else {
#if defined(DUK_USE_DEBUG)
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

				/* deal with weak references first */
				duk_heap_strcache_string_remove(heap, (duk_hstring *) h);

				/* remove the string from the string table */
				duk_heap_strtable_unlink_prev(heap, (duk_hstring *) h, (duk_hstring *) prev);

				/* free inner references (these exist e.g. when external
				 * strings are enabled) and the struct itself.
				 */
				duk_free_hstring(heap, (duk_hstring *) h);

				/* don't update 'prev'; it should be last string kept */
			}

			h = next;
		}
	}

 done:
#if defined(DUK_USE_DEBUG)
	DUK_D(DUK_DPRINT("mark-and-sweep sweep stringtable: %ld freed, %ld kept",
	                 (long) count_free, (long) count_keep));
#endif
	*out_count_keep = count_keep;
}

/*
 *  Sweep heap
 */

DUK_LOCAL void duk__sweep_heap(duk_heap *heap, duk_int_t flags, duk_size_t *out_count_keep) {
	duk_heaphdr *prev;  /* last element that was left in the heap */
	duk_heaphdr *curr;
	duk_heaphdr *next;
#if defined(DUK_USE_DEBUG)
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

		if (DUK_LIKELY(DUK_HEAPHDR_HAS_REACHABLE(curr))) {
			/*
			 *  Reachable object, keep
			 */

			DUK_DDD(DUK_DDDPRINT("sweep, reachable: %p", (void *) curr));

#if defined(DUK_USE_FINALIZER_SUPPORT)
			if (DUK_UNLIKELY(DUK_HEAPHDR_HAS_FINALIZABLE(curr))) {
				/*
				 *  If object has been marked finalizable, move it to the
				 *  "to be finalized" work list.  It will be collected on
				 *  the next mark-and-sweep if it is still unreachable
				 *  after running the finalizer.
				 */

				DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(curr));
				DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(curr) == DUK_HTYPE_OBJECT);
				DUK_DDD(DUK_DDDPRINT("object has finalizer, move to finalization work list: %p", (void *) curr));

				DUK_HEAP_INSERT_INTO_FINALIZE_LIST(heap, curr);
#if defined(DUK_USE_DEBUG)
				count_finalize++;
#endif
			}
			else
#endif  /* DUK_USE_FINALIZER_SUPPORT */
			{
				/*
				 *  Object will be kept; queue object back to heap_allocated (to tail)
				 */

				if (DUK_UNLIKELY(DUK_HEAPHDR_HAS_FINALIZED(curr))) {
					/*
					 *  Object's finalizer was executed on last round, and
					 *  object has been happily rescued.
					 */

					DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(curr));
					DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(curr) == DUK_HTYPE_OBJECT);
					DUK_DD(DUK_DDPRINT("object rescued during mark-and-sweep finalization: %p", (void *) curr));
#if defined(DUK_USE_DEBUG)
					count_rescue++;
#endif
				} else {
					/*
					 *  Plain, boring reachable object.
					 */
					DUK_DD(DUK_DDPRINT("keep object: %!iO", curr));
					count_keep++;
				}

				if (prev != NULL) {
					DUK_ASSERT(heap->heap_allocated != NULL);
					DUK_HEAPHDR_SET_NEXT(heap, prev, curr);
				} else {
					DUK_ASSERT(heap->heap_allocated == NULL);
					heap->heap_allocated = curr;
				}
#if defined(DUK_USE_DOUBLE_LINKED_HEAP)
				DUK_HEAPHDR_SET_PREV(heap, curr, prev);
#endif
				DUK_ASSERT_HEAPHDR_LINKS(heap, prev);
				DUK_ASSERT_HEAPHDR_LINKS(heap, curr);
				prev = curr;
			}

			DUK_HEAPHDR_CLEAR_REACHABLE(curr);
#if defined(DUK_USE_FINALIZER_SUPPORT)
			DUK_HEAPHDR_CLEAR_FINALIZED(curr);
#endif
			/* Keep FINALIZABLE for objects on finalize_list. */

			DUK_ASSERT(!DUK_HEAPHDR_HAS_REACHABLE(curr));
			DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZED(curr));

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

#if defined(DUK_USE_DEBUG)
			if (DUK_HEAPHDR_HAS_FINALIZED(curr)) {
				DUK_DDD(DUK_DDDPRINT("finalized object not rescued: %p", (void *) curr));
			}
#endif

			/* Note: object cannot be a finalizable unreachable object, as
			 * they have been marked temporarily reachable for this round,
			 * and are handled above.
			 */

#if defined(DUK_USE_DEBUG)
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
	if (prev != NULL) {
		DUK_HEAPHDR_SET_NEXT(heap, prev, NULL);
	}
	DUK_ASSERT_HEAPHDR_LINKS(heap, prev);

#if defined(DUK_USE_DEBUG)
	DUK_D(DUK_DPRINT("mark-and-sweep sweep objects (non-string): %ld freed, %ld kept, %ld rescued, %ld queued for finalization",
	                 (long) count_free, (long) count_keep, (long) count_rescue, (long) count_finalize));
#endif
	*out_count_keep = count_keep;
}

/*
 *  Object compaction.
 *
 *  Compaction is assumed to never throw an error.
 */

DUK_LOCAL int duk__protected_compact_object(duk_context *ctx, void *udata) {
	duk_hobject *obj;
	/* XXX: for threads, compact value stack, call stack, catch stack? */

	DUK_UNREF(udata);
	obj = duk_known_hobject(ctx, -1);
	duk_hobject_compact_props((duk_hthread *) ctx, obj);
	return 0;
}

#if defined(DUK_USE_DEBUG)
DUK_LOCAL void duk__compact_object_list(duk_heap *heap, duk_hthread *thr, duk_heaphdr *start, duk_size_t *p_count_check, duk_size_t *p_count_compact, duk_size_t *p_count_bytes_saved) {
#else
DUK_LOCAL void duk__compact_object_list(duk_heap *heap, duk_hthread *thr, duk_heaphdr *start) {
#endif
	duk_heaphdr *curr;
#if defined(DUK_USE_DEBUG)
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

#if defined(DUK_USE_DEBUG)
		old_size = DUK_HOBJECT_P_COMPUTE_SIZE(DUK_HOBJECT_GET_ESIZE(obj),
		                                      DUK_HOBJECT_GET_ASIZE(obj),
		                                      DUK_HOBJECT_GET_HSIZE(obj));
#endif

		DUK_DD(DUK_DDPRINT("compact object: %p", (void *) obj));
		duk_push_hobject((duk_context *) thr, obj);
		/* XXX: disable error handlers for duration of compaction? */
		duk_safe_call((duk_context *) thr, duk__protected_compact_object, NULL, 1, 0);

#if defined(DUK_USE_DEBUG)
		new_size = DUK_HOBJECT_P_COMPUTE_SIZE(DUK_HOBJECT_GET_ESIZE(obj),
		                                      DUK_HOBJECT_GET_ASIZE(obj),
		                                      DUK_HOBJECT_GET_HSIZE(obj));
#endif

#if defined(DUK_USE_DEBUG)
		(*p_count_compact)++;
		(*p_count_bytes_saved) += (duk_size_t) (old_size - new_size);
#endif

	 next:
		curr = DUK_HEAPHDR_GET_NEXT(heap, curr);
#if defined(DUK_USE_DEBUG)
		(*p_count_check)++;
#endif
	}
}

DUK_LOCAL void duk__compact_objects(duk_heap *heap) {
	/* XXX: which lists should participate?  to be finalized? */
#if defined(DUK_USE_DEBUG)
	duk_size_t count_check = 0;
	duk_size_t count_compact = 0;
	duk_size_t count_bytes_saved = 0;
#endif

	DUK_DD(DUK_DDPRINT("duk__compact_objects: %p", (void *) heap));

	DUK_ASSERT(heap->heap_thread != NULL);

#if defined(DUK_USE_DEBUG)
	duk__compact_object_list(heap, heap->heap_thread, heap->heap_allocated, &count_check, &count_compact, &count_bytes_saved);
#if defined(DUK_USE_FINALIZER_SUPPORT)
	duk__compact_object_list(heap, heap->heap_thread, heap->finalize_list, &count_check, &count_compact, &count_bytes_saved);
#endif
#else
	duk__compact_object_list(heap, heap->heap_thread, heap->heap_allocated);
#if defined(DUK_USE_FINALIZER_SUPPORT)
	duk__compact_object_list(heap, heap->heap_thread, heap->finalize_list);
#endif
#endif
#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_ASSERT(heap->refzero_list == NULL);  /* Always handled to completion inline in DECREF. */
#endif

#if defined(DUK_USE_DEBUG)
	DUK_D(DUK_DPRINT("mark-and-sweep compact objects: %ld checked, %ld compaction attempts, %ld bytes saved by compaction",
	                 (long) count_check, (long) count_compact, (long) count_bytes_saved));
#endif
}

/*
 *  Assertion helpers.
 */

#if defined(DUK_USE_ASSERTIONS)
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

#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_ASSERT(heap->refzero_list == NULL);  /* Always handled to completion inline in DECREF. */
#endif
}

#if defined(DUK_USE_REFERENCE_COUNTING)
DUK_LOCAL void duk__assert_valid_refcounts(duk_heap *heap) {
	duk_heaphdr *hdr = heap->heap_allocated;
	while (hdr) {
		/* Cannot really assert much w.r.t. refcounts now. */

		if (DUK_HEAPHDR_GET_REFCOUNT(hdr) == 0 &&
		    DUK_HEAPHDR_HAS_FINALIZED(hdr)) {
			/* An object may be in heap_allocated list with a zero
			 * refcount if it has just been finalized and is waiting
			 * to be collected by the next cycle.
			 * (This doesn't currently happen however.)
			 */
		} else if (DUK_HEAPHDR_GET_REFCOUNT(hdr) == 0) {
			/* An object may be in heap_allocated list with a zero
			 * refcount also if it is a temporary object created
			 * during debugger paused state.  It will get collected
			 * by mark-and-sweep based on its reachability status
			 * (presumably not reachable because refcount is 0).
			 */
		}
		DUK_ASSERT_DISABLE(DUK_HEAPHDR_GET_REFCOUNT(hdr) >= 0);  /* Unsigned. */
		hdr = DUK_HEAPHDR_GET_NEXT(heap, hdr);
	}
}

DUK_LOCAL void duk__clear_assert_refcounts(duk_heap *heap) {
	duk_heaphdr *curr;
	duk_uint32_t i;

	for (curr = heap->heap_allocated; curr != NULL; curr = DUK_HEAPHDR_GET_NEXT(heap, curr)) {
		curr->h_assert_refcount = 0;
	}
#if defined(DUK_USE_FINALIZER_SUPPORT)
	for (curr = heap->finalize_list; curr != NULL; curr = DUK_HEAPHDR_GET_NEXT(heap, curr)) {
		curr->h_assert_refcount = 0;
	}
#endif
#if defined(DUK_USE_REFERENCE_COUNTING)
	for (curr = heap->refzero_list; curr != NULL; curr = DUK_HEAPHDR_GET_NEXT(heap, curr)) {
		curr->h_assert_refcount = 0;
	}
#endif

	for (i = 0; i < heap->st_size; i++) {
		duk_hstring *h;

#if defined(DUK_USE_STRTAB_PTRCOMP)
		h = DUK_USE_HEAPPTR_DEC16(heap->heap_udata, heap->strtable16[i]);
#else
		h = heap->strtable[i];
#endif
		while (h != NULL) {
			((duk_heaphdr *) h)->h_assert_refcount = 0;
			h = h->hdr.h_next;
		}
	}
}

DUK_LOCAL void duk__check_refcount_heaphdr(duk_heaphdr *hdr) {
	duk_bool_t count_ok;

	/* The refcount check only makes sense for reachable objects on
	 * heap_allocated or string table, after the sweep phase.  Prior to
	 * sweep phase refcounts will include references that are not visible
	 * via reachability roots.
	 *
	 * Because we're called after the sweep phase, all heap objects on
	 * heap_allocated are reachable.  REACHABLE flags have already been
	 * cleared so we can't check them.
	 */

	/* ROM objects have intentionally incorrect refcount (1), but we won't
	 * check them.
	 */
	DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY(hdr));

	count_ok = ((duk_size_t) DUK_HEAPHDR_GET_REFCOUNT(hdr) == hdr->h_assert_refcount);
	if (!count_ok) {
		DUK_D(DUK_DPRINT("refcount mismatch for: %p: header=%ld counted=%ld --> %!iO",
		                 (void *) hdr, (long) DUK_HEAPHDR_GET_REFCOUNT(hdr),
		                 (long) hdr->h_assert_refcount, hdr));
		DUK_ASSERT(0);
	}
}

DUK_LOCAL void duk__check_assert_refcounts(duk_heap *heap) {
	duk_heaphdr *curr;
	duk_uint32_t i;

	for (curr = heap->heap_allocated; curr != NULL; curr = DUK_HEAPHDR_GET_NEXT(heap, curr)) {
		duk__check_refcount_heaphdr(curr);
	}

	for (i = 0; i < heap->st_size; i++) {
		duk_hstring *h;

#if defined(DUK_USE_STRTAB_PTRCOMP)
		h = DUK_USE_HEAPPTR_DEC16(heap->heap_udata, heap->strtable16[i]);
#else
		h = heap->strtable[i];
#endif
		while (h != NULL) {
			duk__check_refcount_heaphdr((duk_heaphdr *) h);
			h = h->hdr.h_next;
		}
	}
}
#endif  /* DUK_USE_REFERENCE_COUNTING */
#endif  /* DUK_USE_ASSERTIONS */

/*
 *  Main mark-and-sweep function.
 *
 *  'flags' represents the features requested by the caller.  The current
 *  heap->ms_base_flags is ORed automatically into the flags; the base flags
 *  mask typically prevents certain mark-and-sweep operation to avoid trouble.
 */

DUK_INTERNAL void duk_heap_mark_and_sweep(duk_heap *heap, duk_small_uint_t flags) {
	duk_size_t count_keep_obj;
	duk_size_t count_keep_str;
#if defined(DUK_USE_VOLUNTARY_GC)
	duk_size_t tmp;
#endif

	/* If debugger is paused, garbage collection is disabled by default.
	 * This is achieved by bumping ms_prevent_count when becoming paused.
	 */
	DUK_ASSERT(!DUK_HEAP_HAS_DEBUGGER_PAUSED(heap) || heap->ms_prevent_count > 0);

	/* Prevention/recursion check as soon as possible because we may
	 * be called a number of times when voluntary mark-and-sweep is
	 * pending.
	 */
	if (heap->ms_prevent_count != 0) {
		DUK_DD(DUK_DDPRINT("reject recursive mark-and-sweep"));
		return;
	}
	DUK_ASSERT(heap->ms_running == 0);  /* ms_prevent_count is bumped when ms_running is set */

	/* Heap_thread is used during mark-and-sweep for refcount finalization
	 * (it's also used for finalizer execution once mark-and-sweep is
	 * complete).  Heap allocation code ensures heap_thread is set and
	 * properly initialized before setting ms_prevent_count to 0.
	 */
	DUK_ASSERT(heap->heap_thread != NULL);
	DUK_ASSERT(heap->heap_thread->valstack != NULL);
	DUK_ASSERT(heap->heap_thread->callstack != NULL);
	DUK_ASSERT(heap->heap_thread->catchstack != NULL);

	DUK_D(DUK_DPRINT("garbage collect (mark-and-sweep) starting, requested flags: 0x%08lx, effective flags: 0x%08lx",
	                 (unsigned long) flags, (unsigned long) (flags | heap->ms_base_flags)));

	flags |= heap->ms_base_flags;

	/*
	 *  Assertions before
	 */

#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(heap->ms_prevent_count == 0);
	DUK_ASSERT(heap->ms_running == 0);
	DUK_ASSERT(!DUK_HEAP_HAS_DEBUGGER_PAUSED(heap));
	DUK_ASSERT(!DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED(heap));
	DUK_ASSERT(heap->ms_recursion_depth == 0);
	duk__assert_heaphdr_flags(heap);
#if defined(DUK_USE_REFERENCE_COUNTING)
	/* Note: heap->refzero_free_running may be true; a refcount
	 * finalizer may trigger a mark-and-sweep.
	 */
	duk__assert_valid_refcounts(heap);
#endif  /* DUK_USE_REFERENCE_COUNTING */
#endif  /* DUK_USE_ASSERTIONS */

	/*
	 *  Begin
	 */

	DUK_ASSERT(heap->ms_prevent_count == 0);
	DUK_ASSERT(heap->ms_running == 0);
	heap->ms_prevent_count = 1;
	heap->ms_running = 1;

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

#if defined(DUK_USE_ASSERTIONS) && defined(DUK_USE_REFERENCE_COUNTING)
	duk__clear_assert_refcounts(heap);
#endif
	duk__mark_roots_heap(heap);               /* Main reachability roots. */
#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_ASSERT(heap->refzero_list == NULL);   /* Always handled to completion inline in DECREF. */
#endif
	duk__mark_temproots_by_heap_scan(heap);   /* Temproots. */

#if defined(DUK_USE_FINALIZER_SUPPORT)
	duk__mark_finalizable(heap);              /* Mark finalizable as reachability roots. */
	duk__mark_finalize_list(heap);            /* mark finalizer work list as reachability roots */
#endif
	duk__mark_temproots_by_heap_scan(heap);   /* Temproots. */

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

#if defined(DUK_USE_REFERENCE_COUNTING)
	duk__finalize_refcounts(heap);
#endif
	duk__sweep_heap(heap, flags, &count_keep_obj);
	duk__sweep_stringtable(heap, &count_keep_str);
#if defined(DUK_USE_ASSERTIONS) && defined(DUK_USE_REFERENCE_COUNTING)
	duk__check_assert_refcounts(heap);
#endif
#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_ASSERT(heap->refzero_list == NULL);   /* Always handled to completion inline in DECREF. */
#endif
#if defined(DUK_USE_FINALIZER_SUPPORT)
	duk__clear_finalize_list_flags(heap);
#endif

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
	 *  This is mainly useful in emergency GC: if the string table load
	 *  factor is really low for some reason, we can shrink the string
	 *  table to a smaller size and free some memory in the process.
	 *  Only execute in emergency GC.  String table has internal flags
	 *  to protect against recursive resizing if this mark-and-sweep pass
	 *  was triggered by a string table resize.
	 */

	if (flags & DUK_MS_FLAG_EMERGENCY) {
		DUK_D(DUK_DPRINT("stringtable resize check in emergency gc"));
		duk_heap_strtable_force_resize(heap);
	}

	/*
	 *  Finish
	 */

	DUK_ASSERT(heap->ms_prevent_count == 1);
	heap->ms_prevent_count = 0;
	DUK_ASSERT(heap->ms_running == 1);
	heap->ms_running = 0;

	/*
	 *  Assertions after
	 */

#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(heap->ms_prevent_count == 0);
	DUK_ASSERT(!DUK_HEAP_HAS_MARKANDSWEEP_RECLIMIT_REACHED(heap));
	DUK_ASSERT(heap->ms_recursion_depth == 0);
	duk__assert_heaphdr_flags(heap);
#if defined(DUK_USE_REFERENCE_COUNTING)
	/* Note: heap->refzero_free_running may be true; a refcount
	 * finalizer may trigger a mark-and-sweep.
	 */
	duk__assert_valid_refcounts(heap);
#endif  /* DUK_USE_REFERENCE_COUNTING */
#endif  /* DUK_USE_ASSERTIONS */

	/*
	 *  Reset trigger counter
	 */

#if defined(DUK_USE_VOLUNTARY_GC)
	tmp = (count_keep_obj + count_keep_str) / 256;
	heap->ms_trigger_counter = (duk_int_t) (
	    (tmp * DUK_HEAP_MARK_AND_SWEEP_TRIGGER_MULT) +
	    DUK_HEAP_MARK_AND_SWEEP_TRIGGER_ADD);
	DUK_D(DUK_DPRINT("garbage collect (mark-and-sweep) finished: %ld objects kept, %ld strings kept, trigger reset to %ld",
	                 (long) count_keep_obj, (long) count_keep_str, (long) heap->ms_trigger_counter));
#else
	DUK_D(DUK_DPRINT("garbage collect (mark-and-sweep) finished: %ld objects kept, %ld strings kept, no voluntary trigger",
	                 (long) count_keep_obj, (long) count_keep_str));
#endif

	/*
	 *  Finalize objects in the finalization work list.  Finalized
	 *  objects are queued back to heap_allocated with FINALIZED set.
	 *
	 *  Since finalizers may cause arbitrary side effects, they are
	 *  prevented e.g. during string table and object property allocation
	 *  resizing using heap->pf_prevent_count.  In this case the objects
	 *  remain in the finalization work list after mark-and-sweep exits
	 *  and they may be finalized on the next pass or any DECREF checking
	 *  for finalize_list.
	 *
	 *  As of Duktape 2.1 finalization happens outside mark-and-sweep
	 *  protection.  Even so, mark-and-sweep is prevented while finalizers
	 *  run: if mark-and-sweep runs when the finalize_list has only been
	 *  partially processed, incorrect rescue decisions are made because
	 *  finalize_list is considered a reachability root.  As a side effect:
	 *
	 *    * An out-of-memory error inside a finalizer will not
	 *      cause a mark-and-sweep and may cause the finalizer
	 *      to fail unnecessarily.
	 *
	 *  This is not optimal, but since the sweep for this phase has
	 *  already happened, this is probably good enough for now.
	 *
	 *  There are at least two main fixes to this limitation: (1) a better
	 *  notion of reachability for rescue/free decisions, and (2) skipping
	 *  rescue/free decisions when mark-and-sweep runs and finalize_list
	 *  is not empty.
	 *
	 *  XXX: avoid finalizer execution when doing emergency GC?
	 */

#if defined(DUK_USE_FINALIZER_SUPPORT)
	/* Attempt to process finalize_list, pf_prevent_count check
	 * is inside the target.
	 */
	duk_heap_process_finalize_list(heap);
#endif  /* DUK_USE_FINALIZER_SUPPORT */
}
