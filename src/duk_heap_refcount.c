/*
 *  Reference counting implementation.
 */

#include "duk_internal.h"

#ifdef DUK_USE_REFERENCE_COUNTING

#ifndef DUK_USE_DOUBLE_LINKED_HEAP
#error internal error, reference counting requires a double linked heap
#endif

/*
 *  Misc
 */

DUK_LOCAL void duk__queue_refzero(duk_heap *heap, duk_heaphdr *hdr) {
	/* tail insert: don't disturb head in case refzero is running */

	if (heap->refzero_list != NULL) {
		duk_heaphdr *hdr_prev;

		hdr_prev = heap->refzero_list_tail;
		DUK_ASSERT(hdr_prev != NULL);
		DUK_ASSERT(DUK_HEAPHDR_GET_NEXT(heap, hdr_prev) == NULL);

		DUK_HEAPHDR_SET_NEXT(heap, hdr, NULL);
		DUK_HEAPHDR_SET_PREV(heap, hdr, hdr_prev);
		DUK_HEAPHDR_SET_NEXT(heap, hdr_prev, hdr);
		DUK_ASSERT_HEAPHDR_LINKS(heap, hdr);
		DUK_ASSERT_HEAPHDR_LINKS(heap, hdr_prev);
		heap->refzero_list_tail = hdr;
	} else {
		DUK_ASSERT(heap->refzero_list_tail == NULL);
		DUK_HEAPHDR_SET_NEXT(heap, hdr, NULL);
		DUK_HEAPHDR_SET_PREV(heap, hdr, NULL);
		DUK_ASSERT_HEAPHDR_LINKS(heap, hdr);
		heap->refzero_list = hdr;
		heap->refzero_list_tail = hdr;
	}
}

/*
 *  Heap object refcount finalization.
 *
 *  When an object is about to be freed, all other objects it refers to must
 *  be decref'd.  Refcount finalization does NOT free the object or its inner
 *  allocations (mark-and-sweep shares these helpers), it just manipulates
 *  the refcounts.
 *
 *  Note that any of the decref's may cause a refcount to drop to zero, BUT
 *  it will not be processed inline; instead, because refzero is already
 *  running, the objects will just be queued to refzero list and processed
 *  later.  This eliminates C recursion.
 */

DUK_LOCAL void duk__refcount_finalize_hobject(duk_hthread *thr, duk_hobject *h) {
	duk_uint_fast32_t i;

	DUK_ASSERT(h);
	DUK_ASSERT(DUK_HEAPHDR_GET_TYPE((duk_heaphdr *) h) == DUK_HTYPE_OBJECT);

	/* XXX: better to get base and walk forwards? */

	for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(h); i++) {
		duk_hstring *key = DUK_HOBJECT_E_GET_KEY(thr->heap, h, i);
		if (!key) {
			continue;
		}
		duk_heaphdr_decref(thr, (duk_heaphdr *) key);
		if (DUK_HOBJECT_E_SLOT_IS_ACCESSOR(thr->heap, h, i)) {
			duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) DUK_HOBJECT_E_GET_VALUE_GETTER(thr->heap, h, i));
			duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) DUK_HOBJECT_E_GET_VALUE_SETTER(thr->heap, h, i));
		} else {
			duk_tval_decref(thr, DUK_HOBJECT_E_GET_VALUE_TVAL_PTR(thr->heap, h, i));
		}
	}

	for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ASIZE(h); i++) {
		duk_tval_decref(thr, DUK_HOBJECT_A_GET_VALUE_PTR(thr->heap, h, i));
	}

	/* hash part is a 'weak reference' and does not contribute */

	duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h));

	if (DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		duk_hcompiledfunction *f = (duk_hcompiledfunction *) h;
		duk_tval *tv, *tv_end;
		duk_hobject **funcs, **funcs_end;

		if (DUK_HCOMPILEDFUNCTION_GET_DATA(thr->heap, f) != NULL) {
			tv = DUK_HCOMPILEDFUNCTION_GET_CONSTS_BASE(thr->heap, f);
			tv_end = DUK_HCOMPILEDFUNCTION_GET_CONSTS_END(thr->heap, f);
			while (tv < tv_end) {
				duk_tval_decref(thr, tv);
				tv++;
			}

			funcs = DUK_HCOMPILEDFUNCTION_GET_FUNCS_BASE(thr->heap, f);
			funcs_end = DUK_HCOMPILEDFUNCTION_GET_FUNCS_END(thr->heap, f);
			while (funcs < funcs_end) {
				duk_heaphdr_decref(thr, (duk_heaphdr *) *funcs);
				funcs++;
			}
		} else {
			/* May happen in some out-of-memory corner cases. */
			DUK_D(DUK_DPRINT("duk_hcompiledfunction 'data' is NULL, skipping decref"));
		}

		duk_heaphdr_decref(thr, (duk_heaphdr *) DUK_HCOMPILEDFUNCTION_GET_DATA(thr->heap, f));
	} else if (DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		duk_hnativefunction *f = (duk_hnativefunction *) h;
		DUK_UNREF(f);
		/* nothing to finalize */
	} else if (DUK_HOBJECT_IS_BUFFEROBJECT(h)) {
		duk_hbufferobject *b = (duk_hbufferobject *) h;
		if (b->buf) {
			duk_heaphdr_decref(thr, (duk_heaphdr *) b->buf);
		}
	} else if (DUK_HOBJECT_IS_THREAD(h)) {
		duk_hthread *t = (duk_hthread *) h;
		duk_tval *tv;

		tv = t->valstack;
		while (tv < t->valstack_top) {
			duk_tval_decref(thr, tv);
			tv++;
		}

		for (i = 0; i < (duk_uint_fast32_t) t->callstack_top; i++) {
			duk_activation *act = t->callstack + i;
			duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) DUK_ACT_GET_FUNC(act));
			duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) act->var_env);
			duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) act->lex_env);
#ifdef DUK_USE_NONSTD_FUNC_CALLER_PROPERTY
			duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) act->prev_caller);
#endif
		}

#if 0  /* nothing now */
		for (i = 0; i < (duk_uint_fast32_t) t->catchstack_top; i++) {
			duk_catcher *cat = t->catchstack + i;
		}
#endif

		for (i = 0; i < DUK_NUM_BUILTINS; i++) {
			duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) t->builtins[i]);
		}

		duk_heaphdr_decref_allownull(thr, (duk_heaphdr *) t->resumer);
	}
}

DUK_INTERNAL void duk_heaphdr_refcount_finalize(duk_hthread *thr, duk_heaphdr *hdr) {
	DUK_ASSERT(hdr);

	switch ((int) DUK_HEAPHDR_GET_TYPE(hdr)) {
	case DUK_HTYPE_OBJECT:
		duk__refcount_finalize_hobject(thr, (duk_hobject *) hdr);
		break;
	case DUK_HTYPE_BUFFER:
		/* nothing to finalize */
		break;
	case DUK_HTYPE_STRING:
		/* cannot happen: strings are not put into refzero list (they don't even have the next/prev pointers) */
	default:
		DUK_UNREACHABLE();
	}
}

#if defined(DUK_USE_REFZERO_FINALIZER_TORTURE)
DUK_LOCAL duk_ret_t duk__refcount_fake_finalizer(duk_context *ctx) {
	DUK_UNREF(ctx);
	DUK_D(DUK_DPRINT("fake refcount torture finalizer executed"));
#if 0
	DUK_DD(DUK_DDPRINT("fake torture finalizer for: %!T", duk_get_tval(ctx, 0)));
#endif
	/* Require a lot of stack to force a value stack grow/shrink. */
	duk_require_stack(ctx, 100000);

	/* XXX: do something to force a callstack grow/shrink, perhaps
	 * just a manual forced resize?
	 */
	return 0;
}

DUK_LOCAL void duk__refcount_run_torture_finalizer(duk_hthread *thr, duk_hobject *obj) {
	duk_context *ctx;
	duk_int_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	ctx = (duk_context *) thr;

	/* Avoid fake finalization for the duk__refcount_fake_finalizer function
	 * itself, otherwise we're in infinite recursion.
	 */
	if (DUK_HOBJECT_HAS_NATIVEFUNCTION(obj)) {
		if (((duk_hnativefunction *) obj)->func == duk__refcount_fake_finalizer) {
			DUK_DD(DUK_DDPRINT("avoid fake torture finalizer for duk__refcount_fake_finalizer itself"));
			return;
		}
	}
	/* Avoid fake finalization when callstack limit has been reached.
	 * Otherwise a callstack limit error will be created, then refzero'ed,
	 * and we're in an infinite loop.
	 */
	if (thr->heap->call_recursion_depth >= thr->heap->call_recursion_limit ||
	    thr->callstack_size + 2 * DUK_CALLSTACK_GROW_STEP >= thr->callstack_max /*approximate*/) {
		DUK_D(DUK_DPRINT("call recursion depth reached, avoid fake torture finalizer"));
		return;
	}

	/* Run fake finalizer.  Avoid creating new refzero queue entries
	 * so that we are not forced into a forever loop.
	 */
	duk_push_c_function(ctx, duk__refcount_fake_finalizer, 1 /*nargs*/);
	duk_push_hobject(ctx, obj);
	rc = duk_pcall(ctx, 1);
	DUK_UNREF(rc);  /* ignored */
	duk_pop(ctx);
}
#endif  /* DUK_USE_REFZERO_FINALIZER_TORTURE */

/*
 *  Refcount memory freeing loop.
 *
 *  Frees objects in the refzero_pending list until the list becomes
 *  empty.  When an object is freed, its references get decref'd and
 *  may cause further objects to be queued for freeing.
 *
 *  This could be expanded to allow incremental freeing: just bail out
 *  early and resume at a future alloc/decref/refzero.
 */

DUK_LOCAL void duk__refzero_free_pending(duk_hthread *thr) {
	duk_heaphdr *h1, *h2;
	duk_heap *heap;
	duk_int_t count = 0;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	heap = thr->heap;
	DUK_ASSERT(heap != NULL);

	/*
	 *  Detect recursive invocation
	 */

	if (DUK_HEAP_HAS_REFZERO_FREE_RUNNING(heap)) {
		DUK_DDD(DUK_DDDPRINT("refzero free running, skip run"));
		return;
	}

	/*
	 *  Churn refzero_list until empty
	 */

	DUK_HEAP_SET_REFZERO_FREE_RUNNING(heap);
	while (heap->refzero_list) {
		duk_hobject *obj;
		duk_bool_t rescued = 0;

		/*
		 *  Pick an object from the head (don't remove yet).
		 */

		h1 = heap->refzero_list;
		obj = (duk_hobject *) h1;
		DUK_DD(DUK_DDPRINT("refzero processing %p: %!O", (void *) h1, (duk_heaphdr *) h1));
		DUK_ASSERT(DUK_HEAPHDR_GET_PREV(heap, h1) == NULL);
		DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(h1) == DUK_HTYPE_OBJECT);  /* currently, always the case */

#if defined(DUK_USE_REFZERO_FINALIZER_TORTURE)
		/* Torture option to shake out finalizer side effect issues:
		 * make a bogus function call for every finalizable object,
		 * essentially simulating the case where everything has a
		 * finalizer.
		 */
		DUK_DD(DUK_DDPRINT("refzero torture enabled, fake finalizer"));
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(h1) == 0);
		DUK_HEAPHDR_PREINC_REFCOUNT(h1);  /* bump refcount to prevent refzero during finalizer processing */
		duk__refcount_run_torture_finalizer(thr, obj);  /* must never longjmp */
		DUK_HEAPHDR_PREDEC_REFCOUNT(h1);  /* remove artificial bump */
		DUK_ASSERT_DISABLE(h1->h_refcount >= 0);  /* refcount is unsigned, so always true */
#endif

		/*
		 *  Finalizer check.
		 *
		 *  Note: running a finalizer may have arbitrary side effects, e.g.
		 *  queue more objects on refzero_list (tail), or even trigger a
		 *  mark-and-sweep.
		 *
		 *  Note: quick reject check should match vast majority of
		 *  objects and must be safe (not throw any errors, ever).
		 */

		/* An object may have FINALIZED here if it was finalized by mark-and-sweep
		 * on a previous run and refcount then decreased to zero.  We won't run the
		 * finalizer again here.
		 */

		/* A finalizer is looked up from the object and up its prototype chain
		 * (which allows inherited finalizers).
		 */
		if (duk_hobject_hasprop_raw(thr, obj, DUK_HTHREAD_STRING_INT_FINALIZER(thr))) {
			DUK_DDD(DUK_DDDPRINT("object has a finalizer, run it"));

			DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(h1) == 0);
			DUK_HEAPHDR_PREINC_REFCOUNT(h1);  /* bump refcount to prevent refzero during finalizer processing */

			duk_hobject_run_finalizer(thr, obj);  /* must never longjmp */
			DUK_ASSERT(DUK_HEAPHDR_HAS_FINALIZED(h1));  /* duk_hobject_run_finalizer() sets */

			DUK_HEAPHDR_PREDEC_REFCOUNT(h1);  /* remove artificial bump */
			DUK_ASSERT_DISABLE(h1->h_refcount >= 0);  /* refcount is unsigned, so always true */

			if (DUK_HEAPHDR_GET_REFCOUNT(h1) != 0) {
				DUK_DDD(DUK_DDDPRINT("-> object refcount after finalization non-zero, object will be rescued"));
				rescued = 1;
			} else {
				DUK_DDD(DUK_DDDPRINT("-> object refcount still zero after finalization, object will be freed"));
			}
		}

		/* Refzero head is still the same.  This is the case even if finalizer
		 * inserted more refzero objects; they are inserted to the tail.
		 */
		DUK_ASSERT(h1 == heap->refzero_list);

		/*
		 *  Remove the object from the refzero list.  This cannot be done
		 *  before a possible finalizer has been executed; the finalizer
		 *  may trigger a mark-and-sweep, and mark-and-sweep must be able
		 *  to traverse a complete refzero_list.
		 */

		h2 = DUK_HEAPHDR_GET_NEXT(heap, h1);
		if (h2) {
			DUK_HEAPHDR_SET_PREV(heap, h2, NULL);  /* not strictly necessary */
			heap->refzero_list = h2;
		} else {
			heap->refzero_list = NULL;
			heap->refzero_list_tail = NULL;
		}

		/*
		 *  Rescue or free.
		 */

		if (rescued) {
			/* yes -> move back to heap allocated */
			DUK_DD(DUK_DDPRINT("object rescued during refcount finalization: %p", (void *) h1));
			DUK_ASSERT(!DUK_HEAPHDR_HAS_FINALIZABLE(h1));
			DUK_ASSERT(DUK_HEAPHDR_HAS_FINALIZED(h1));
			DUK_HEAPHDR_CLEAR_FINALIZED(h1);
			h2 = heap->heap_allocated;
			DUK_HEAPHDR_SET_PREV(heap, h1, NULL);
			if (h2) {
				DUK_HEAPHDR_SET_PREV(heap, h2, h1);
			}
			DUK_HEAPHDR_SET_NEXT(heap, h1, h2);
			DUK_ASSERT_HEAPHDR_LINKS(heap, h1);
			DUK_ASSERT_HEAPHDR_LINKS(heap, h2);
			heap->heap_allocated = h1;
		} else {
			/* no -> decref members, then free */
			duk__refcount_finalize_hobject(thr, obj);
			duk_heap_free_heaphdr_raw(heap, h1);
		}

		count++;
	}
	DUK_HEAP_CLEAR_REFZERO_FREE_RUNNING(heap);

	DUK_DDD(DUK_DDDPRINT("refzero processed %ld objects", (long) count));

	/*
	 *  Once the whole refzero cascade has been freed, check for
	 *  a voluntary mark-and-sweep.
	 */

#if defined(DUK_USE_MARK_AND_SWEEP) && defined(DUK_USE_VOLUNTARY_GC)
	/* 'count' is more or less comparable to normal trigger counter update
	 * which happens in memory block (re)allocation.
	 */
	heap->mark_and_sweep_trigger_counter -= count;
	if (heap->mark_and_sweep_trigger_counter <= 0) {
		if (DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
			DUK_D(DUK_DPRINT("mark-and-sweep in progress -> skip voluntary mark-and-sweep now"));
		} else {
			duk_bool_t rc;
			duk_small_uint_t flags = 0;  /* not emergency */
			DUK_D(DUK_DPRINT("refcount triggering mark-and-sweep"));
			rc = duk_heap_mark_and_sweep(heap, flags);
			DUK_UNREF(rc);
			DUK_D(DUK_DPRINT("refcount triggered mark-and-sweep => rc %ld", (long) rc));
		}
	}
#endif  /* DUK_USE_MARK_AND_SWEEP && DUK_USE_VOLUNTARY_GC */
}

/*
 *  Incref and decref functions.
 *
 *  Decref may trigger immediate refzero handling, which may free and finalize
 *  an arbitrary number of objects.
 *
 */

DUK_INTERNAL void duk_heaphdr_refzero(duk_hthread *thr, duk_heaphdr *h) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);

	heap = thr->heap;
	DUK_DDD(DUK_DDDPRINT("refzero %p: %!O", (void *) h, (duk_heaphdr *) h));

	/*
	 *  Refzero handling is skipped entirely if (1) mark-and-sweep is
	 *  running or (2) execution is paused in the debugger.  The objects
	 *  are left in the heap, and will be freed by mark-and-sweep or
	 *  eventual heap destruction.
	 *
	 *  This is necessary during mark-and-sweep because refcounts are also
	 *  updated during the sweep phase (otherwise objects referenced by a
	 *  swept object would have incorrect refcounts) which then calls here.
	 *  This could be avoided by using separate decref macros in
	 *  mark-and-sweep; however, mark-and-sweep also calls finalizers which
	 *  would use the ordinary decref macros anyway and still call this
	 *  function.
	 *
	 *  This check must be enabled also when mark-and-sweep support has been
	 *  disabled: the flag is also used in heap destruction when running
	 *  finalizers for remaining objects, and the flag prevents objects from
	 *  being moved around in heap linked lists.
	 */

	/* XXX: ideally this would be just one flag (maybe a derived one) so
	 * that a single bit test is sufficient to check the condition.
	 */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	if (DUK_UNLIKELY(DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap) || DUK_HEAP_IS_PAUSED(heap))) {
#else
	if (DUK_UNLIKELY(DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap))) {
#endif
		DUK_DDD(DUK_DDDPRINT("refzero handling suppressed when mark-and-sweep running, object: %p", (void *) h));
		return;
	}

	switch ((duk_small_int_t) DUK_HEAPHDR_GET_TYPE(h)) {
	case DUK_HTYPE_STRING:
		/*
		 *  Strings have no internal references but do have "weak"
		 *  references in the string cache.  Also note that strings
		 *  are not on the heap_allocated list like other heap
		 *  elements.
		 */

		duk_heap_strcache_string_remove(heap, (duk_hstring *) h);
		duk_heap_string_remove(heap, (duk_hstring *) h);
		duk_heap_free_heaphdr_raw(heap, h);
		break;

	case DUK_HTYPE_OBJECT:
		/*
		 *  Objects have internal references.  Must finalize through
		 *  the "refzero" work list.
		 */

		duk_heap_remove_any_from_heap_allocated(heap, h);
		duk__queue_refzero(heap, h);
		duk__refzero_free_pending(thr);
		break;

	case DUK_HTYPE_BUFFER:
		/*
		 *  Buffers have no internal references.  However, a dynamic
		 *  buffer has a separate allocation for the buffer.  This is
		 *  freed by duk_heap_free_heaphdr_raw().
		 */

		duk_heap_remove_any_from_heap_allocated(heap, h);
		duk_heap_free_heaphdr_raw(heap, h);
		break;

	default:
		DUK_D(DUK_DPRINT("invalid heap type in decref: %ld", (long) DUK_HEAPHDR_GET_TYPE(h)));
		DUK_UNREACHABLE();
	}
}

#if !defined(DUK_USE_FAST_REFCOUNT_DEFAULT)
DUK_INTERNAL void duk_tval_incref(duk_tval *tv) {
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
		DUK_ASSERT_DISABLE(h->h_refcount >= 0);
		DUK_HEAPHDR_PREINC_REFCOUNT(h);
	}
}
#endif

#if 0  /* unused */
DUK_INTERNAL void duk_tval_incref_allownull(duk_tval *tv) {
	if (tv == NULL) {
		return;
	}
	if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
		DUK_ASSERT_DISABLE(h->h_refcount >= 0);
		DUK_HEAPHDR_PREINC_REFCOUNT(h);
	}
}
#endif

DUK_INTERNAL void duk_tval_decref(duk_hthread *thr, duk_tval *tv) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
		duk_heaphdr_decref(thr, h);
	}
}

#if 0  /* unused */
DUK_INTERNAL void duk_tval_decref_allownull(duk_hthread *thr, duk_tval *tv) {
	DUK_ASSERT(thr != NULL);

	if (tv == NULL) {
		return;
	}
	if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
		duk_heaphdr_decref(thr, h);
	}
}
#endif

#if !defined(DUK_USE_FAST_REFCOUNT_DEFAULT)
DUK_INTERNAL void duk_heaphdr_incref(duk_heaphdr *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
	DUK_ASSERT_DISABLE(DUK_HEAPHDR_GET_REFCOUNT(h) >= 0);

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY(h)) {
		return;
	}
#endif

	DUK_HEAPHDR_PREINC_REFCOUNT(h);
}
#endif

#if 0  /* unused */
DUK_INTERNAL void duk_heaphdr_incref_allownull(duk_heaphdr *h) {
	if (h == NULL) {
		return;
	}
	DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
	DUK_ASSERT_DISABLE(DUK_HEAPHDR_GET_REFCOUNT(h) >= 0);

	DUK_HEAPHDR_PREINC_REFCOUNT(h);
}
#endif

DUK_INTERNAL void duk_heaphdr_decref(duk_hthread *thr, duk_heaphdr *h) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
	DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(h) >= 1);

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY(h)) {
		return;
	}
#endif
	if (DUK_HEAPHDR_PREDEC_REFCOUNT(h) != 0) {
		return;
	}
	duk_heaphdr_refzero(thr, h);
}

DUK_INTERNAL void duk_heaphdr_decref_allownull(duk_hthread *thr, duk_heaphdr *h) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	if (h == NULL) {
		return;
	}
	DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY(h)) {
		return;
	}
#endif
	DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(h) >= 1);
	if (DUK_HEAPHDR_PREDEC_REFCOUNT(h) != 0) {
		return;
	}
	duk_heaphdr_refzero(thr, h);
}

#else

/* no refcounting */

#endif  /* DUK_USE_REFERENCE_COUNTING */
