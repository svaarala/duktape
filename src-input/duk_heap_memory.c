/*
 *  Memory allocation handling.
 */

#include "duk_internal.h"

/*
 *  Helpers
 *
 *  The fast path checks are done within a macro to ensure "inlining"
 *  while the slow path actions use a helper (which won't typically be
 *  inlined in size optimized builds).
 */

#if defined(DUK_USE_MARK_AND_SWEEP) && defined(DUK_USE_VOLUNTARY_GC)
#define DUK__VOLUNTARY_PERIODIC_GC(heap)  do { \
		(heap)->mark_and_sweep_trigger_counter--; \
		if ((heap)->mark_and_sweep_trigger_counter <= 0) { \
			duk__run_voluntary_gc(heap); \
		} \
	} while (0)

DUK_LOCAL void duk__run_voluntary_gc(duk_heap *heap) {
	if (DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
		DUK_DD(DUK_DDPRINT("mark-and-sweep in progress -> skip voluntary mark-and-sweep now"));
	} else {
		duk_small_uint_t flags;
		duk_bool_t rc;

		DUK_D(DUK_DPRINT("triggering voluntary mark-and-sweep"));
		flags = 0;
		rc = duk_heap_mark_and_sweep(heap, flags);
		DUK_UNREF(rc);
	}
}
#else
#define DUK__VOLUNTARY_PERIODIC_GC(heap)  /* no voluntary gc */
#endif  /* DUK_USE_MARK_AND_SWEEP && DUK_USE_VOLUNTARY_GC */

/*
 *  Allocate memory with garbage collection
 */

#ifdef DUK_USE_MARK_AND_SWEEP
DUK_INTERNAL void *duk_heap_mem_alloc(duk_heap *heap, duk_size_t size) {
	void *res;
	duk_bool_t rc;
	duk_small_int_t i;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT_DISABLE(size >= 0);

	/*
	 *  Voluntary periodic GC (if enabled)
	 */

	DUK__VOLUNTARY_PERIODIC_GC(heap);

	/*
	 *  First attempt
	 */

#ifdef DUK_USE_GC_TORTURE
	/* simulate alloc failure on every alloc (except when mark-and-sweep is running) */
	if (!DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
		DUK_DDD(DUK_DDDPRINT("gc torture enabled, pretend that first alloc attempt fails"));
		res = NULL;
		DUK_UNREF(res);
		goto skip_attempt;
	}
#endif
	res = heap->alloc_func(heap->heap_udata, size);
	if (res || size == 0) {
		/* for zero size allocations NULL is allowed */
		return res;
	}
#ifdef DUK_USE_GC_TORTURE
 skip_attempt:
#endif

	DUK_D(DUK_DPRINT("first alloc attempt failed, attempt to gc and retry"));

	/*
	 *  Avoid a GC if GC is already running.  This can happen at a late
	 *  stage in a GC when we try to e.g. resize the stringtable
	 *  or compact objects.
	 */

	if (DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
		DUK_D(DUK_DPRINT("duk_heap_mem_alloc() failed, gc in progress (gc skipped), alloc size %ld", (long) size));
		return NULL;
	}

	/*
	 *  Retry with several GC attempts.  Initial attempts are made without
	 *  emergency mode; later attempts use emergency mode which minimizes
	 *  memory allocations forcibly.
	 */

	for (i = 0; i < DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_LIMIT; i++) {
		duk_small_uint_t flags;

		flags = 0;
		if (i >= DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_EMERGENCY_LIMIT - 1) {
			flags |= DUK_MS_FLAG_EMERGENCY;
		}

		rc = duk_heap_mark_and_sweep(heap, flags);
		DUK_UNREF(rc);

		res = heap->alloc_func(heap->heap_udata, size);
		if (res) {
			DUK_D(DUK_DPRINT("duk_heap_mem_alloc() succeeded after gc (pass %ld), alloc size %ld",
			                 (long) (i + 1), (long) size));
			return res;
		}
	}

	DUK_D(DUK_DPRINT("duk_heap_mem_alloc() failed even after gc, alloc size %ld", (long) size));
	return NULL;
}
#else  /* DUK_USE_MARK_AND_SWEEP */
/*
 *  Compared to a direct macro expansion this wrapper saves a few
 *  instructions because no heap dereferencing is required.
 */
DUK_INTERNAL void *duk_heap_mem_alloc(duk_heap *heap, duk_size_t size) {
	DUK_ASSERT(heap != NULL);
	DUK_ASSERT_DISABLE(size >= 0);

	return heap->alloc_func(heap->heap_udata, size);
}
#endif  /* DUK_USE_MARK_AND_SWEEP */

DUK_INTERNAL void *duk_heap_mem_alloc_zeroed(duk_heap *heap, duk_size_t size) {
	void *res;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT_DISABLE(size >= 0);

	res = DUK_ALLOC(heap, size);
	if (res) {
		/* assume memset with zero size is OK */
		DUK_MEMZERO(res, size);
	}
	return res;
}

/*
 *  Reallocate memory with garbage collection
 */

#ifdef DUK_USE_MARK_AND_SWEEP
DUK_INTERNAL void *duk_heap_mem_realloc(duk_heap *heap, void *ptr, duk_size_t newsize) {
	void *res;
	duk_bool_t rc;
	duk_small_int_t i;

	DUK_ASSERT(heap != NULL);
	/* ptr may be NULL */
	DUK_ASSERT_DISABLE(newsize >= 0);

	/*
	 *  Voluntary periodic GC (if enabled)
	 */

	DUK__VOLUNTARY_PERIODIC_GC(heap);

	/*
	 *  First attempt
	 */

#ifdef DUK_USE_GC_TORTURE
	/* simulate alloc failure on every realloc (except when mark-and-sweep is running) */
	if (!DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
		DUK_DDD(DUK_DDDPRINT("gc torture enabled, pretend that first realloc attempt fails"));
		res = NULL;
		DUK_UNREF(res);
		goto skip_attempt;
	}
#endif
	res = heap->realloc_func(heap->heap_udata, ptr, newsize);
	if (res || newsize == 0) {
		/* for zero size allocations NULL is allowed */
		return res;
	}
#ifdef DUK_USE_GC_TORTURE
 skip_attempt:
#endif

	DUK_D(DUK_DPRINT("first realloc attempt failed, attempt to gc and retry"));

	/*
	 *  Avoid a GC if GC is already running.  See duk_heap_mem_alloc().
	 */

	if (DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
		DUK_D(DUK_DPRINT("duk_heap_mem_realloc() failed, gc in progress (gc skipped), alloc size %ld", (long) newsize));
		return NULL;
	}

	/*
	 *  Retry with several GC attempts.  Initial attempts are made without
	 *  emergency mode; later attempts use emergency mode which minimizes
	 *  memory allocations forcibly.
	 */

	for (i = 0; i < DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_LIMIT; i++) {
		duk_small_uint_t flags;

		flags = 0;
		if (i >= DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_EMERGENCY_LIMIT - 1) {
			flags |= DUK_MS_FLAG_EMERGENCY;
		}

		rc = duk_heap_mark_and_sweep(heap, flags);
		DUK_UNREF(rc);

		res = heap->realloc_func(heap->heap_udata, ptr, newsize);
		if (res || newsize == 0) {
			DUK_D(DUK_DPRINT("duk_heap_mem_realloc() succeeded after gc (pass %ld), alloc size %ld",
			                 (long) (i + 1), (long) newsize));
			return res;
		}
	}

	DUK_D(DUK_DPRINT("duk_heap_mem_realloc() failed even after gc, alloc size %ld", (long) newsize));
	return NULL;
}
#else  /* DUK_USE_MARK_AND_SWEEP */
/* saves a few instructions to have this wrapper (see comment on duk_heap_mem_alloc) */
DUK_INTERNAL void *duk_heap_mem_realloc(duk_heap *heap, void *ptr, duk_size_t newsize) {
	DUK_ASSERT(heap != NULL);
	/* ptr may be NULL */
	DUK_ASSERT_DISABLE(newsize >= 0);

	return heap->realloc_func(heap->heap_udata, ptr, newsize);
}
#endif  /* DUK_USE_MARK_AND_SWEEP */

/*
 *  Reallocate memory with garbage collection, using a callback to provide
 *  the current allocated pointer.  This variant is used when a mark-and-sweep
 *  (e.g. finalizers) might change the original pointer.
 */

#ifdef DUK_USE_MARK_AND_SWEEP
DUK_INTERNAL void *duk_heap_mem_realloc_indirect(duk_heap *heap, duk_mem_getptr cb, void *ud, duk_size_t newsize) {
	void *res;
	duk_bool_t rc;
	duk_small_int_t i;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT_DISABLE(newsize >= 0);

	/*
	 *  Voluntary periodic GC (if enabled)
	 */

	DUK__VOLUNTARY_PERIODIC_GC(heap);

	/*
	 *  First attempt
	 */

#ifdef DUK_USE_GC_TORTURE
	/* simulate alloc failure on every realloc (except when mark-and-sweep is running) */
	if (!DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
		DUK_DDD(DUK_DDDPRINT("gc torture enabled, pretend that first indirect realloc attempt fails"));
		res = NULL;
		DUK_UNREF(res);
		goto skip_attempt;
	}
#endif
	res = heap->realloc_func(heap->heap_udata, cb(heap, ud), newsize);
	if (res || newsize == 0) {
		/* for zero size allocations NULL is allowed */
		return res;
	}
#ifdef DUK_USE_GC_TORTURE
 skip_attempt:
#endif

	DUK_D(DUK_DPRINT("first indirect realloc attempt failed, attempt to gc and retry"));

	/*
	 *  Avoid a GC if GC is already running.  See duk_heap_mem_alloc().
	 */

	if (DUK_HEAP_HAS_MARKANDSWEEP_RUNNING(heap)) {
		DUK_D(DUK_DPRINT("duk_heap_mem_realloc_indirect() failed, gc in progress (gc skipped), alloc size %ld", (long) newsize));
		return NULL;
	}

	/*
	 *  Retry with several GC attempts.  Initial attempts are made without
	 *  emergency mode; later attempts use emergency mode which minimizes
	 *  memory allocations forcibly.
	 */

	for (i = 0; i < DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_LIMIT; i++) {
		duk_small_uint_t flags;

#ifdef DUK_USE_ASSERTIONS
		void *ptr_pre;  /* ptr before mark-and-sweep */
		void *ptr_post;
#endif

#ifdef DUK_USE_ASSERTIONS
		ptr_pre = cb(heap, ud);
#endif
		flags = 0;
		if (i >= DUK_HEAP_ALLOC_FAIL_MARKANDSWEEP_EMERGENCY_LIMIT - 1) {
			flags |= DUK_MS_FLAG_EMERGENCY;
		}

		rc = duk_heap_mark_and_sweep(heap, flags);
		DUK_UNREF(rc);
#ifdef DUK_USE_ASSERTIONS
		ptr_post = cb(heap, ud);
		if (ptr_pre != ptr_post) {
			/* useful for debugging */
			DUK_DD(DUK_DDPRINT("note: base pointer changed by mark-and-sweep: %p -> %p",
			                   (void *) ptr_pre, (void *) ptr_post));
		}
#endif

		/* Note: key issue here is to re-lookup the base pointer on every attempt.
		 * The pointer being reallocated may change after every mark-and-sweep.
		 */

		res = heap->realloc_func(heap->heap_udata, cb(heap, ud), newsize);
		if (res || newsize == 0) {
			DUK_D(DUK_DPRINT("duk_heap_mem_realloc_indirect() succeeded after gc (pass %ld), alloc size %ld",
			                 (long) (i + 1), (long) newsize));
			return res;
		}
	}

	DUK_D(DUK_DPRINT("duk_heap_mem_realloc_indirect() failed even after gc, alloc size %ld", (long) newsize));
	return NULL;
}
#else  /* DUK_USE_MARK_AND_SWEEP */
/* saves a few instructions to have this wrapper (see comment on duk_heap_mem_alloc) */
DUK_INTERNAL void *duk_heap_mem_realloc_indirect(duk_heap *heap, duk_mem_getptr cb, void *ud, duk_size_t newsize) {
	return heap->realloc_func(heap->heap_udata, cb(heap, ud), newsize);
}
#endif  /* DUK_USE_MARK_AND_SWEEP */

/*
 *  Free memory
 */

#ifdef DUK_USE_MARK_AND_SWEEP
DUK_INTERNAL void duk_heap_mem_free(duk_heap *heap, void *ptr) {
	DUK_ASSERT(heap != NULL);
	/* ptr may be NULL */

	/* Must behave like a no-op with NULL and any pointer returned from
	 * malloc/realloc with zero size.
	 */
	heap->free_func(heap->heap_udata, ptr);

	/* Count free operations toward triggering a GC but never actually trigger
	 * a GC from a free.  Otherwise code which frees internal structures would
	 * need to put in NULLs at every turn to ensure the object is always in
	 * consistent state for a mark-and-sweep.
	 */
#ifdef DUK_USE_VOLUNTARY_GC
	heap->mark_and_sweep_trigger_counter--;
#endif
}
#else
/* saves a few instructions to have this wrapper (see comment on duk_heap_mem_alloc) */
DUK_INTERNAL void duk_heap_mem_free(duk_heap *heap, void *ptr) {
	DUK_ASSERT(heap != NULL);
	/* ptr may be NULL */

	/* Note: must behave like a no-op with NULL and any pointer
	 * returned from malloc/realloc with zero size.
	 */
	heap->free_func(heap->heap_udata, ptr);
}
#endif
