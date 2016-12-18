/*
 *  Reference counting implementation.
 */

#include "duk_internal.h"

#if defined(DUK_USE_REFERENCE_COUNTING)

#if !defined(DUK_USE_DOUBLE_LINKED_HEAP)
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
 *  it will not be processed inline.  If refcount finalization is triggered
 *  by refzero processing, the objects will be just queued to the refzero
 *  list and processed later which eliminates C recursion.  If refcount
 *  finalization is triggered by mark-and-sweep, any refzero situations are
 *  ignored because mark-and-sweep will deal with them.  NORZ variants can
 *  be used here in both cases.
 */

DUK_LOCAL void duk__refcount_finalize_hobject(duk_hthread *thr, duk_hobject *h) {
	duk_uint_fast32_t i;
	duk_uint_fast32_t n;
	duk_propvalue *p_val;
	duk_tval *p_tv;
	duk_hstring **p_key;
	duk_uint8_t *p_flag;

	DUK_ASSERT(h);
	DUK_ASSERT(DUK_HEAPHDR_GET_TYPE((duk_heaphdr *) h) == DUK_HTYPE_OBJECT);

	/* XXX: better to get base and walk forwards? */

	p_key = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, h);
	p_val = DUK_HOBJECT_E_GET_VALUE_BASE(thr->heap, h);
	p_flag = DUK_HOBJECT_E_GET_FLAGS_BASE(thr->heap, h);
	n = DUK_HOBJECT_GET_ENEXT(h);
	while (n-- > 0) {
		duk_hstring *key;

		key = p_key[n];
		if (!key) {
			continue;
		}
		DUK_HSTRING_DECREF_NORZ(thr, key);
		if (p_flag[n] & DUK_PROPDESC_FLAG_ACCESSOR) {
			duk_hobject *h_getset;
			h_getset = p_val[n].a.get;
			DUK_ASSERT(h_getset == NULL || DUK_HEAPHDR_IS_OBJECT((duk_heaphdr *) h_getset));
			DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, h_getset);
			h_getset = p_val[n].a.set;
			DUK_ASSERT(h_getset == NULL || DUK_HEAPHDR_IS_OBJECT((duk_heaphdr *) h_getset));
			DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, h_getset);
		} else {
			duk_tval *tv_val;
			tv_val = &p_val[n].v;
			DUK_TVAL_DECREF_NORZ(thr, tv_val);
		}
	}

	p_tv = DUK_HOBJECT_A_GET_BASE(thr->heap, h);
	n = DUK_HOBJECT_GET_ASIZE(h);
	while (n-- > 0) {
		duk_tval *tv_val;
		tv_val = p_tv + n;
		DUK_TVAL_DECREF_NORZ(thr, tv_val);
	}

	/* hash part is a 'weak reference' and does not contribute */

	{
		duk_hobject *h_proto;
		h_proto = (duk_hobject *) DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h);
		DUK_ASSERT(h_proto == NULL || DUK_HEAPHDR_IS_OBJECT((duk_heaphdr *) h_proto));
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, h_proto);
	}

	/* XXX: rearrange bits to allow a switch case to be used here? */
	/* XXX: add a fast path for objects (and arrays)? */

	/* DUK_HOBJECT_IS_ARRAY(h): needs no special handling now as there are
	 * no extra fields in need of decref.
	 */
	if (DUK_HOBJECT_IS_COMPFUNC(h)) {
		duk_hcompfunc *f = (duk_hcompfunc *) h;
		duk_tval *tv, *tv_end;
		duk_hobject **funcs, **funcs_end;

		if (DUK_HCOMPFUNC_GET_DATA(thr->heap, f) != NULL) {
			tv = DUK_HCOMPFUNC_GET_CONSTS_BASE(thr->heap, f);
			tv_end = DUK_HCOMPFUNC_GET_CONSTS_END(thr->heap, f);
			while (tv < tv_end) {
				DUK_TVAL_DECREF_NORZ(thr, tv);
				tv++;
			}

			funcs = DUK_HCOMPFUNC_GET_FUNCS_BASE(thr->heap, f);
			funcs_end = DUK_HCOMPFUNC_GET_FUNCS_END(thr->heap, f);
			while (funcs < funcs_end) {
				duk_hobject *h_func;
				h_func = *funcs;
				DUK_ASSERT(DUK_HEAPHDR_IS_OBJECT((duk_heaphdr *) h_func));
				DUK_HCOMPFUNC_DECREF_NORZ(thr, (duk_hcompfunc *) h_func);
				funcs++;
			}
		} else {
			/* May happen in some out-of-memory corner cases. */
			DUK_D(DUK_DPRINT("duk_hcompfunc 'data' is NULL, skipping decref"));
		}

		DUK_HEAPHDR_DECREF_ALLOWNULL(thr, (duk_heaphdr *) DUK_HCOMPFUNC_GET_LEXENV(thr->heap, f));
		DUK_HEAPHDR_DECREF_ALLOWNULL(thr, (duk_heaphdr *) DUK_HCOMPFUNC_GET_VARENV(thr->heap, f));
		DUK_HEAPHDR_DECREF_ALLOWNULL(thr, (duk_hbuffer *) DUK_HCOMPFUNC_GET_DATA(thr->heap, f));
	} else if (DUK_HOBJECT_IS_NATFUNC(h)) {
		duk_hnatfunc *f = (duk_hnatfunc *) h;
		DUK_UNREF(f);
		/* nothing to finalize */
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	} else if (DUK_HOBJECT_IS_BUFOBJ(h)) {
		duk_hbufobj *b = (duk_hbufobj *) h;
		DUK_HBUFFER_DECREF_NORZ_ALLOWNULL(thr, (duk_hbuffer *) b->buf);
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, (duk_hobject *) b->buf_prop);
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */
	} else if (DUK_HOBJECT_IS_THREAD(h)) {
		duk_hthread *t = (duk_hthread *) h;
		duk_tval *tv;

		tv = t->valstack;
		while (tv < t->valstack_top) {
			DUK_TVAL_DECREF_NORZ(thr, tv);
			tv++;
		}

		for (i = 0; i < (duk_uint_fast32_t) t->callstack_top; i++) {
			duk_activation *act = t->callstack + i;
			DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, (duk_hobject *) DUK_ACT_GET_FUNC(act));
			DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, (duk_hobject *) act->var_env);
			DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, (duk_hobject *) act->lex_env);
#if defined(DUK_USE_NONSTD_FUNC_CALLER_PROPERTY)
			DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, (duk_hobject *) act->prev_caller);
#endif
		}

#if 0  /* nothing now */
		for (i = 0; i < (duk_uint_fast32_t) t->catchstack_top; i++) {
			duk_catcher *cat = t->catchstack + i;
		}
#endif

		for (i = 0; i < DUK_NUM_BUILTINS; i++) {
			DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, (duk_hobject *) t->builtins[i]);
		}

		DUK_HTHREAD_DECREF_NORZ_ALLOWNULL(thr, (duk_hthread *) t->resumer);
	}
}

DUK_INTERNAL void duk_heaphdr_refcount_finalize(duk_hthread *thr, duk_heaphdr *hdr) {
	DUK_ASSERT(hdr);

	if (DUK_HEAPHDR_GET_TYPE(hdr) == DUK_HTYPE_OBJECT) {
		duk__refcount_finalize_hobject(thr, (duk_hobject *) hdr);
	}
	/* DUK_HTYPE_BUFFER: nothing to finalize */
	/* DUK_HTYPE_STRING: nothing to finalize */
}

#if defined(DUK_USE_FINALIZER_SUPPORT)
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
	if (DUK_HOBJECT_HAS_NATFUNC(obj)) {
		if (((duk_hnatfunc *) obj)->func == duk__refcount_fake_finalizer) {
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
#endif  /* DUK_USE_FINALIZER_SUPPORT */

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

DUK_INTERNAL void duk_refzero_free_pending(duk_hthread *thr) {
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
#if defined(DUK_USE_FINALIZER_SUPPORT)
		duk_bool_t rescued = 0;
#endif  /* DUK_USE_FINALIZER_SUPPORT */

		/*
		 *  Pick an object from the head (don't remove yet).
		 */

		h1 = heap->refzero_list;
		obj = (duk_hobject *) h1;
		DUK_DD(DUK_DDPRINT("refzero processing %p: %!O", (void *) h1, (duk_heaphdr *) h1));
		DUK_ASSERT(DUK_HEAPHDR_GET_PREV(heap, h1) == NULL);
		DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(h1) == DUK_HTYPE_OBJECT);  /* currently, always the case */

#if defined(DUK_USE_FINALIZER_SUPPORT)
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
#endif  /* DUK_USE_REFZERO_FINALIZER_TORTURE */
#endif  /* DUK_USE_FINALIZER_SUPPORT */

		/*
		 *  Finalizer check.
		 *
		 *  Note: running a finalizer may have arbitrary side effects, e.g.
		 *  queue more objects on refzero_list (tail), or even trigger a
		 *  mark-and-sweep.
		 *
		 *  Note: quick reject check should match vast majority of
		 *  objects and must be safe (not throw any errors, ever).
		 *
		 *  An object may have FINALIZED here if it was finalized by mark-and-sweep
		 *  on a previous run and refcount then decreased to zero.  We won't run the
		 *  finalizer again here.
		 *
		 *  A finalizer is looked up from the object and up its prototype chain
		 *  (which allows inherited finalizers).
		 */

#if defined(DUK_USE_FINALIZER_SUPPORT)
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
#endif  /* DUK_USE_FINALIZER_SUPPORT */

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

#if defined(DUK_USE_FINALIZER_SUPPORT)
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
		} else
#endif  /* DUK_USE_FINALIZER_SUPPORT */
		{
			/* no -> decref members, then free */
			duk__refcount_finalize_hobject(thr, obj);
			DUK_ASSERT(DUK_HEAPHDR_GET_TYPE(h1) == DUK_HTYPE_OBJECT);  /* currently, always the case */
			duk_free_hobject(heap, (duk_hobject *) h1);
		}

		count++;
	}
	DUK_HEAP_CLEAR_REFZERO_FREE_RUNNING(heap);

	DUK_DDD(DUK_DDDPRINT("refzero processed %ld objects", (long) count));

	/*
	 *  Once the whole refzero cascade has been freed, check for
	 *  a voluntary mark-and-sweep.
	 */

#if defined(DUK_USE_VOLUNTARY_GC)
	/* 'count' is more or less comparable to normal trigger counter update
	 * which happens in memory block (re)allocation.
	 */
	heap->mark_and_sweep_trigger_counter -= count;
	if (heap->mark_and_sweep_trigger_counter <= 0) {
		duk_bool_t rc;
		duk_small_uint_t flags = 0;  /* not emergency */
		DUK_D(DUK_DPRINT("refcount triggering mark-and-sweep"));
		rc = duk_heap_mark_and_sweep(heap, flags);
		DUK_UNREF(rc);
		DUK_D(DUK_DPRINT("refcount triggered mark-and-sweep => rc %ld", (long) rc));
	}
#endif  /* DUK_USE_VOLUNTARY_GC */
}

/*
 *  Incref and decref functions.
 *
 *  Decref may trigger immediate refzero handling, which may free and finalize
 *  an arbitrary number of objects.
 *
 *  Refzero handling is skipped entirely if (1) mark-and-sweep is running or
 *  (2) execution is paused in the debugger.  The objects are left in the heap,
 *  and will be freed by mark-and-sweep or eventual heap destruction.
 *
 *  This is necessary during mark-and-sweep because refcounts are also updated
 *  during the sweep phase (otherwise objects referenced by a swept object
 *  would have incorrect refcounts) which then calls here.  This could be
 *  avoided by using separate decref macros in mark-and-sweep; however,
 *  mark-and-sweep also calls finalizers which would use the ordinary decref
 *  macros anyway.
 *
 *  The DUK__RZ_SUPPRESS_CHECK() must be enabled also when mark-and-sweep
 *  support has been disabled: the flag is also used in heap destruction when
 *  running finalizers for remaining objects, and the flag prevents objects
 *  from being moved around in heap linked lists.
 */

/* The suppress condition is important to performance.  The flags being tested
 * are in the same duk_heap field so a single TEST instruction (on x86) tests
 * for them.
 */
#if defined(DUK_USE_DEBUGGER_SUPPORT)
#define DUK__RZ_SUPPRESS_COND() \
	(DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap) || DUK_HEAP_IS_PAUSED(heap))
#else
#define DUK__RZ_SUPPRESS_COND() \
	(DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap))
#endif
#define DUK__RZ_SUPPRESS_CHECK() do { \
		if (DUK_UNLIKELY(DUK__RZ_SUPPRESS_COND())) { \
			DUK_DDD(DUK_DDDPRINT("refzero handling suppressed when mark-and-sweep running, object: %p", (void *) h)); \
			return; \
		} \
	} while (0)

#define DUK__RZ_STRING() do { \
		duk_heap_strcache_string_remove(thr->heap, (duk_hstring *) h); \
		duk_heap_string_remove(heap, (duk_hstring *) h); \
		duk_free_hstring(heap, (duk_hstring *) h); \
	} while (0)
#define DUK__RZ_BUFFER() do { \
		duk_heap_remove_any_from_heap_allocated(heap, (duk_heaphdr *) h); \
		duk_free_hbuffer(heap, (duk_hbuffer *) h); \
	} while (0)
#define DUK__RZ_OBJECT() do { \
		duk_heap_remove_any_from_heap_allocated(heap, (duk_heaphdr *) h); \
		duk__queue_refzero(heap, (duk_heaphdr *) h); \
		if (!skip_free_pending) { \
			duk_refzero_free_pending(thr); \
		} \
	} while (0)
#if defined(DUK_USE_FAST_REFCOUNT_DEFAULT)
#define DUK__RZ_INLINE DUK_ALWAYS_INLINE
#else
#define DUK__RZ_INLINE /*nop*/
#endif

DUK_LOCAL DUK__RZ_INLINE void duk__hstring_refzero_helper(duk_hthread *thr, duk_hstring *h) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	heap = thr->heap;

	DUK__RZ_SUPPRESS_CHECK();
	DUK__RZ_STRING();
}

DUK_LOCAL DUK__RZ_INLINE void duk__hbuffer_refzero_helper(duk_hthread *thr, duk_hbuffer *h) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	heap = thr->heap;

	DUK__RZ_SUPPRESS_CHECK();
	DUK__RZ_BUFFER();
}

DUK_LOCAL DUK__RZ_INLINE void duk__hobject_refzero_helper(duk_hthread *thr, duk_hobject *h, duk_bool_t skip_free_pending) {
	duk_heap *heap;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	heap = thr->heap;

	DUK__RZ_SUPPRESS_CHECK();
	DUK__RZ_OBJECT();
}

DUK_LOCAL DUK__RZ_INLINE void duk__heaphdr_refzero_helper(duk_hthread *thr, duk_heaphdr *h, duk_bool_t skip_free_pending) {
	duk_heap *heap;
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	heap = thr->heap;

	htype = (duk_small_uint_t) DUK_HEAPHDR_GET_TYPE(h);
	DUK__RZ_SUPPRESS_CHECK();

	switch (htype) {
	case DUK_HTYPE_STRING:
		/* Strings have no internal references but do have "weak"
		 * references in the string cache.  Also note that strings
		 * are not on the heap_allocated list like other heap
		 * elements.
		 */

		DUK__RZ_STRING();
		break;

	case DUK_HTYPE_OBJECT:
		/* Objects have internal references.  Must finalize through
		 * the "refzero" work list.
		 */

		DUK__RZ_OBJECT();
		break;

	case DUK_HTYPE_BUFFER:
		/* Buffers have no internal references.  However, a dynamic
		 * buffer has a separate allocation for the buffer.  This is
		 * freed by duk_heap_free_heaphdr_raw().
		 */

		DUK__RZ_BUFFER();
		break;

	default:
		DUK_D(DUK_DPRINT("invalid heap type in decref: %ld", (long) DUK_HEAPHDR_GET_TYPE(h)));
		DUK_UNREACHABLE();
	}
}

DUK_INTERNAL void duk_heaphdr_refzero(duk_hthread *thr, duk_heaphdr *h) {
	duk__heaphdr_refzero_helper(thr, h, 0 /*skip_free_pending*/);
}

DUK_INTERNAL void duk_heaphdr_refzero_norz(duk_hthread *thr, duk_heaphdr *h) {
	duk__heaphdr_refzero_helper(thr, h, 1 /*skip_free_pending*/);
}

DUK_INTERNAL void duk_hstring_refzero(duk_hthread *thr, duk_hstring *h) {
	duk__hstring_refzero_helper(thr, h);
}

DUK_INTERNAL void duk_hbuffer_refzero(duk_hthread *thr, duk_hbuffer *h) {
	duk__hbuffer_refzero_helper(thr, h);
}

DUK_INTERNAL void duk_hobject_refzero(duk_hthread *thr, duk_hobject *h) {
	duk__hobject_refzero_helper(thr, h, 0 /*skip_free_pending*/);
}

DUK_INTERNAL void duk_hobject_refzero_norz(duk_hthread *thr, duk_hobject *h) {
	duk__hobject_refzero_helper(thr, h, 1 /*skip_free_pending*/);
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

DUK_INTERNAL void duk_tval_decref(duk_hthread *thr, duk_tval *tv) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(h) >= 1);
#if 0
		if (DUK_HEAPHDR_PREDEC_REFCOUNT(h) != 0) {
			return;
		}
		duk_heaphdr_refzero(thr, h);
#else
		duk_heaphdr_decref(thr, h);
#endif
	}
}

DUK_INTERNAL void duk_tval_decref_norz(duk_hthread *thr, duk_tval *tv) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_NEEDS_REFCOUNT_UPDATE(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT(h) >= 1);
#if 0
		if (DUK_HEAPHDR_PREDEC_REFCOUNT(h) != 0) {
			return;
		}
		duk_heaphdr_refzero_norz(thr, h);
#else
		duk_heaphdr_decref(thr, h);
#endif
	}
}
#endif  /* !DUK_USE_FAST_REFCOUNT_DEFAULT */

#define DUK__DECREF_ASSERTS() do { \
		DUK_ASSERT(thr != NULL); \
		DUK_ASSERT(thr->heap != NULL); \
		DUK_ASSERT(h != NULL); \
		DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID((duk_heaphdr *) h)); \
		DUK_ASSERT(DUK_HEAPHDR_GET_REFCOUNT((duk_heaphdr *) h) >= 1); \
	} while (0)
#if defined(DUK_USE_ROM_OBJECTS)
#define DUK__DECREF_SHARED() do { \
		if (DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) h)) { \
			return; \
		} \
		if (DUK_HEAPHDR_PREDEC_REFCOUNT((duk_heaphdr *) h) != 0) { \
			return; \
		} \
	} while (0)
#else
#define DUK__DECREF_SHARED() do { \
		if (DUK_HEAPHDR_PREDEC_REFCOUNT((duk_heaphdr *) h) != 0) { \
			return; \
		} \
	} while (0)
#endif

#if !defined(DUK_USE_FAST_REFCOUNT_DEFAULT)
/* This will in practice be inlined because it's just an INC instructions
 * and a bit test + INC when ROM objects are enabled.
 */
DUK_INTERNAL void duk_heaphdr_incref(duk_heaphdr *h) {
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HEAPHDR_HTYPE_VALID(h));
	DUK_ASSERT_DISABLE(DUK_HEAPHDR_GET_REFCOUNT(h) >= 0);

	DUK_HEAPHDR_PREINC_REFCOUNT(h);
}

DUK_INTERNAL void duk_heaphdr_decref(duk_hthread *thr, duk_heaphdr *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_heaphdr_refzero(thr, h);
}
DUK_INTERNAL void duk_heaphdr_decref_norz(duk_hthread *thr, duk_heaphdr *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_heaphdr_refzero_norz(thr, h);
}
#endif  /* !DUK_USE_FAST_REFCOUNT_DEFAULT */

#if 0  /* Not needed. */
DUK_INTERNAL void duk_hstring_decref(duk_hthread *thr, duk_hstring *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_hstring_refzero(thr, h);
}
DUK_INTERNAL void duk_hstring_decref_norz(duk_hthread *thr, duk_hstring *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_hstring_refzero_norz(thr, h);
}
DUK_INTERNAL void duk_hbuffer_decref(duk_hthread *thr, duk_hbuffer *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_hbuffer_refzero(thr, h);
}
DUK_INTERNAL void duk_hbuffer_decref_norz(duk_hthread *thr, duk_hbuffer *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_hbuffer_refzero_norz(thr, h);
}
DUK_INTERNAL void duk_hobject_decref(duk_hthread *thr, duk_hobject *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_hobject_refzero(thr, h);
}
DUK_INTERNAL void duk_hobject_decref_norz(duk_hthread *thr, duk_hobject *h) {
	DUK__DECREF_ASSERTS();
	DUK__DECREF_SHARED();
	duk_hobject_refzero_norz(thr, h);
}
#endif

#else  /* DUK_USE_REFERENCE_COUNTING */

/* no refcounting */

#endif  /* DUK_USE_REFERENCE_COUNTING */
