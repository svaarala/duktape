/*
 *  API calls related to general value stack manipulation: resizing the value
 *  stack, pushing and popping values, type checking and reading values,
 *  coercing values, etc.
 *
 *  Also contains internal functions (such as duk_get_tval()), defined
 *  in duk_api_internal.h, with semantics similar to the public API.
 */

/* XXX: repetition of stack pre-checks -> helper or macro or inline */
/* XXX: shared api error strings, and perhaps even throw code for rare cases? */

#include "duk_internal.h"

/*
 *  Forward declarations
 */

DUK_LOCAL_DECL duk_idx_t duk__push_c_function_raw(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_uint_t flags);

/*
 *  Global state for working around missing variadic macros
 */

#ifndef DUK_USE_VARIADIC_MACROS
DUK_EXTERNAL const char *duk_api_global_filename = NULL;
DUK_EXTERNAL duk_int_t duk_api_global_line = 0;
#endif

/*
 *  Helpers
 */

#if defined(DUK_USE_VALSTACK_UNSAFE)
/* Faster but value stack overruns are memory unsafe. */
#define  DUK__CHECK_SPACE() do { \
		DUK_ASSERT(!(thr->valstack_top >= thr->valstack_end)); \
	} while (0)
#else
#define  DUK__CHECK_SPACE() do { \
		if (thr->valstack_top >= thr->valstack_end) { \
			DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK); \
		} \
	} while (0)
#endif

DUK_LOCAL duk_int_t duk__api_coerce_d2i(duk_context *ctx, duk_idx_t index, duk_bool_t require) {
	duk_hthread *thr;
	duk_tval *tv;
	duk_small_int_t c;
	duk_double_t d;

	thr = (duk_hthread *) ctx;

	tv = duk_get_tval(ctx, index);
	if (tv == NULL) {
		goto error_notnumber;
	}

	/*
	 *  Special cases like NaN and +/- Infinity are handled explicitly
	 *  because a plain C coercion from double to int handles these cases
	 *  in undesirable ways.  For instance, NaN may coerce to INT_MIN
	 *  (not zero), and INT_MAX + 1 may coerce to INT_MIN (not INT_MAX).
	 *
	 *  This double-to-int coercion differs from ToInteger() because it
	 *  has a finite range (ToInteger() allows e.g. +/- Infinity).  It
	 *  also differs from ToInt32() because the INT_MIN/INT_MAX clamping
	 *  depends on the size of the int type on the platform.  In particular,
	 *  on platforms with a 64-bit int type, the full range is allowed.
	 */

#if defined(DUK_USE_FASTINT)
	if (DUK_TVAL_IS_FASTINT(tv)) {
		duk_int64_t t = DUK_TVAL_GET_FASTINT(tv);
#if (DUK_INT_MAX <= 0x7fffffffL)
		/* Clamping only necessary for 32-bit ints. */
		if (t < DUK_INT_MIN) {
			t = DUK_INT_MIN;
		} else if (t > DUK_INT_MAX) {
			t = DUK_INT_MAX;
		}
#endif
		return (duk_int_t) t;
	}
#endif

	if (DUK_TVAL_IS_NUMBER(tv)) {
		d = DUK_TVAL_GET_NUMBER(tv);
		c = (duk_small_int_t) DUK_FPCLASSIFY(d);
		if (c == DUK_FP_NAN) {
			return 0;
		} else if (d < (duk_double_t) DUK_INT_MIN) {
			/* covers -Infinity */
			return DUK_INT_MIN;
		} else if (d > (duk_double_t) DUK_INT_MAX) {
			/* covers +Infinity */
			return DUK_INT_MAX;
		} else {
			/* coerce towards zero */
			return (duk_int_t) d;
		}
	}

 error_notnumber:

	if (require) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_NUMBER);
	} else {
		return 0;
	}
}

DUK_LOCAL duk_uint_t duk__api_coerce_d2ui(duk_context *ctx, duk_idx_t index, duk_bool_t require) {
	duk_hthread *thr;
	duk_tval *tv;
	duk_small_int_t c;
	duk_double_t d;

	/* Same as above but for unsigned int range. */

	thr = (duk_hthread *) ctx;

	tv = duk_get_tval(ctx, index);
	if (tv == NULL) {
		goto error_notnumber;
	}

#if defined(DUK_USE_FASTINT)
	if (DUK_TVAL_IS_FASTINT(tv)) {
		duk_int64_t t = DUK_TVAL_GET_FASTINT(tv);
		if (t < 0) {
			t = 0;
		}
#if (DUK_UINT_MAX <= 0xffffffffUL)
		/* Clamping only necessary for 32-bit ints. */
		else if (t > DUK_UINT_MAX) {
			t = DUK_UINT_MAX;
		}
#endif
		return (duk_uint_t) t;
	}
#endif

	if (DUK_TVAL_IS_NUMBER(tv)) {
		d = DUK_TVAL_GET_NUMBER(tv);
		c = (duk_small_int_t) DUK_FPCLASSIFY(d);
		if (c == DUK_FP_NAN) {
			return 0;
		} else if (d < 0.0) {
			/* covers -Infinity */
			return (duk_uint_t) 0;
		} else if (d > (duk_double_t) DUK_UINT_MAX) {
			/* covers +Infinity */
			return (duk_uint_t) DUK_UINT_MAX;
		} else {
			/* coerce towards zero */
			return (duk_uint_t) d;
		}
	}

 error_notnumber:

	if (require) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_NUMBER);
	} else {
		return 0;
	}
}

/*
 *  Stack index validation/normalization and getting a stack duk_tval ptr.
 *
 *  These are called by many API entrypoints so the implementations must be
 *  fast and "inlined".
 *
 *  There's some repetition because of this; keep the functions in sync.
 */

DUK_EXTERNAL duk_idx_t duk_normalize_index(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t vs_size;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	/* Care must be taken to avoid pointer wrapping in the index
	 * validation.  For instance, on a 32-bit platform with 8-byte
	 * duk_tval the index 0x20000000UL would wrap the memory space
	 * once.
	 */

	/* Assume value stack sizes (in elements) fits into duk_idx_t. */
	vs_size = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT(vs_size >= 0);

	if (index < 0) {
		index = vs_size + index;
		if (DUK_UNLIKELY(index < 0)) {
			/* Also catches index == DUK_INVALID_INDEX: vs_size >= 0
			 * so that vs_size + DUK_INVALID_INDEX cannot underflow
			 * and will always be negative.
			 */
			return DUK_INVALID_INDEX;
		}
	} else {
		/* since index non-negative */
		DUK_ASSERT(index != DUK_INVALID_INDEX);

		if (DUK_UNLIKELY(index >= vs_size)) {
			return DUK_INVALID_INDEX;
		}
	}

	DUK_ASSERT(index >= 0);
	DUK_ASSERT(index < vs_size);
	return index;
}

DUK_EXTERNAL duk_idx_t duk_require_normalize_index(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t vs_size;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	vs_size = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT(vs_size >= 0);

	if (index < 0) {
		index = vs_size + index;
		if (DUK_UNLIKELY(index < 0)) {
			goto invalid_index;
		}
	} else {
		DUK_ASSERT(index != DUK_INVALID_INDEX);
		if (DUK_UNLIKELY(index >= vs_size)) {
			goto invalid_index;
		}
	}

	DUK_ASSERT(index >= 0);
	DUK_ASSERT(index < vs_size);
	return index;

 invalid_index:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_INDEX);
	return 0;  /* unreachable */
}

DUK_INTERNAL duk_tval *duk_get_tval(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t vs_size;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	vs_size = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT(vs_size >= 0);

	if (index < 0) {
		index = vs_size + index;
		if (DUK_UNLIKELY(index < 0)) {
			return NULL;
		}
	} else {
		DUK_ASSERT(index != DUK_INVALID_INDEX);
		if (DUK_UNLIKELY(index >= vs_size)) {
			return NULL;
		}
	}

	DUK_ASSERT(index >= 0);
	DUK_ASSERT(index < vs_size);
	return thr->valstack_bottom + index;
}

DUK_INTERNAL duk_tval *duk_require_tval(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t vs_size;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	vs_size = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT(vs_size >= 0);

	if (index < 0) {
		index = vs_size + index;
		if (DUK_UNLIKELY(index < 0)) {
			goto invalid_index;
		}
	} else {
		DUK_ASSERT(index != DUK_INVALID_INDEX);
		if (DUK_UNLIKELY(index >= vs_size)) {
			goto invalid_index;
		}
	}

	DUK_ASSERT(index >= 0);
	DUK_ASSERT(index < vs_size);
	return thr->valstack_bottom + index;

 invalid_index:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_INDEX);
	return NULL;
}

/* Non-critical. */
DUK_EXTERNAL duk_bool_t duk_is_valid_index(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	return (duk_normalize_index(ctx, index) >= 0);
}

/* Non-critical. */
DUK_EXTERNAL void duk_require_valid_index(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	if (duk_normalize_index(ctx, index) < 0) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_INDEX);
	}
}

/*
 *  Value stack top handling
 */

DUK_EXTERNAL duk_idx_t duk_get_top(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
}

/* set stack top within currently allocated range, but don't reallocate */
DUK_EXTERNAL void duk_set_top(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t vs_size;
	duk_idx_t vs_limit;
	duk_idx_t count;
	duk_tval tv_tmp;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	vs_size = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	vs_limit = (duk_idx_t) (thr->valstack_end - thr->valstack_bottom);

	if (index < 0) {
		/* Negative indices are always within allocated stack but
		 * must not go below zero index.
		 */
		index = vs_size + index;
		if (index < 0) {
			/* Also catches index == DUK_INVALID_INDEX. */
			goto invalid_index;
		}
	} else {
		/* Positive index can be higher than valstack top but must
		 * not go above allocated stack (equality is OK).
		 */
		if (index > vs_limit) {
			goto invalid_index;
		}
	}
	DUK_ASSERT(index >= 0);
	DUK_ASSERT(index <= vs_limit);

	if (index >= vs_size) {
		/* Stack size increases or stays the same.  Fill the new
		 * entries (if any) with undefined.  No pointer stability
		 * issues here so we can use a running pointer.
		 */

		tv = thr->valstack_top;
		count = index - vs_size;
		DUK_ASSERT(count >= 0);
		while (count > 0) {
			/* no need to decref previous or new value */
			count--;
			DUK_ASSERT(DUK_TVAL_IS_UNDEFINED_UNUSED(tv));
			DUK_TVAL_SET_UNDEFINED_ACTUAL(tv);
			tv++;
		}
		thr->valstack_top = tv;
	} else {
		/* Stack size decreases, DECREF entries which are above the
		 * new top.  Each DECREF potentially invalidates valstack
		 * pointers, so don't hold on to pointers.  The valstack top
		 * must also be updated on every loop in case a GC is triggered.
		 */

		/* XXX: Here it would be useful to have a DECREF macro which
		 * doesn't need a NULL check, and does refzero queueing without
		 * running the refzero algorithm.  There would be no pointer
		 * instability in this case, and code could be inlined.  After
		 * the loop, one call to refzero would be needed.
		 */

		count = vs_size - index;
		DUK_ASSERT(count > 0);

		while (count > 0) {
			count--;
			tv = --thr->valstack_top;  /* tv -> value just before prev top value */
			DUK_ASSERT(tv >= thr->valstack_bottom);
			DUK_TVAL_SET_TVAL(&tv_tmp, tv);
			DUK_TVAL_SET_UNDEFINED_UNUSED(tv);
			DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */

			/* XXX: fast primitive to set a bunch of values to UNDEFINED_UNUSED */

		}
	}
	return;

 invalid_index:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_INDEX);
}

DUK_EXTERNAL duk_idx_t duk_get_top_index(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = ((duk_idx_t) (thr->valstack_top - thr->valstack_bottom)) - 1;
	if (DUK_UNLIKELY(ret < 0)) {
		/* Return invalid index; if caller uses this without checking
		 * in another API call, the index won't map to a valid stack
		 * entry.
		 */
		return DUK_INVALID_INDEX;
	}
	return ret;
}

DUK_EXTERNAL duk_idx_t duk_require_top_index(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = ((duk_idx_t) (thr->valstack_top - thr->valstack_bottom)) - 1;
	if (DUK_UNLIKELY(ret < 0)) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_INDEX);
	}
	return ret;
}

/*
 *  Value stack resizing.
 *
 *  This resizing happens above the current "top": the value stack can be
 *  grown or shrunk, but the "top" is not affected.  The value stack cannot
 *  be resized to a size below the current "top".
 *
 *  The low level reallocation primitive must carefully recompute all value
 *  stack pointers, and must also work if ALL pointers are NULL.  The resize
 *  is quite tricky because the valstack realloc may cause a mark-and-sweep,
 *  which may run finalizers.  Running finalizers may resize the valstack
 *  recursively (the same value stack we're working on).  So, after realloc
 *  returns, we know that the valstack "top" should still be the same (there
 *  should not be live values above the "top"), but its underlying size and
 *  pointer may have changed.
 */

/* XXX: perhaps refactor this to allow caller to specify some parameters, or
 * at least a 'compact' flag which skips any spare or round-up .. useful for
 * emergency gc.
 */

DUK_LOCAL duk_bool_t duk__resize_valstack(duk_context *ctx, duk_size_t new_size) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_ptrdiff_t old_bottom_offset;
	duk_ptrdiff_t old_top_offset;
	duk_ptrdiff_t old_end_offset_post;
#ifdef DUK_USE_DEBUG
	duk_ptrdiff_t old_end_offset_pre;
	duk_tval *old_valstack_pre;
	duk_tval *old_valstack_post;
#endif
	duk_tval *new_valstack;
	duk_tval *p;
	duk_size_t new_alloc_size;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
	DUK_ASSERT((duk_size_t) (thr->valstack_top - thr->valstack) <= new_size);  /* can't resize below 'top' */
	DUK_ASSERT(new_size <= thr->valstack_max);  /* valstack limit caller has check, prevents wrapping */
	DUK_ASSERT(new_size <= DUK_SIZE_MAX / sizeof(duk_tval));  /* specific assert for wrapping */

	/* get pointer offsets for tweaking below */
	old_bottom_offset = (((duk_uint8_t *) thr->valstack_bottom) - ((duk_uint8_t *) thr->valstack));
	old_top_offset = (((duk_uint8_t *) thr->valstack_top) - ((duk_uint8_t *) thr->valstack));
#ifdef DUK_USE_DEBUG
	old_end_offset_pre = (((duk_uint8_t *) thr->valstack_end) - ((duk_uint8_t *) thr->valstack));  /* not very useful, used for debugging */
	old_valstack_pre = thr->valstack;
#endif

	/* Allocate a new valstack.
	 *
	 * Note: cannot use a plain DUK_REALLOC() because a mark-and-sweep may
	 * invalidate the original thr->valstack base pointer inside the realloc
	 * process.  See doc/memory-management.txt.
	 */

	new_alloc_size = sizeof(duk_tval) * new_size;
	new_valstack = (duk_tval *) DUK_REALLOC_INDIRECT(thr->heap, duk_hthread_get_valstack_ptr, (void *) thr, new_alloc_size);
	if (!new_valstack) {
		/* Because new_size != 0, if condition doesn't need to be
		 * (new_valstack != NULL || new_size == 0).
		 */
		DUK_ASSERT(new_size != 0);
		DUK_D(DUK_DPRINT("failed to resize valstack to %lu entries (%lu bytes)",
		                 (unsigned long) new_size, (unsigned long) new_alloc_size));
		return 0;
	}

	/* Note: the realloc may have triggered a mark-and-sweep which may
	 * have resized our valstack internally.  However, the mark-and-sweep
	 * MUST NOT leave the stack bottom/top in a different state.  Particular
	 * assumptions and facts:
	 *
	 *   - The thr->valstack pointer may be different after realloc,
	 *     and the offset between thr->valstack_end <-> thr->valstack
	 *     may have changed.
	 *   - The offset between thr->valstack_bottom <-> thr->valstack
	 *     and thr->valstack_top <-> thr->valstack MUST NOT have changed,
	 *     because mark-and-sweep must adhere to a strict stack policy.
	 *     In other words, logical bottom and top MUST NOT have changed.
	 *   - All values above the top are unreachable but are initialized
	 *     to UNDEFINED_UNUSED, up to the post-realloc valstack_end.
	 *   - 'old_end_offset' must be computed after realloc to be correct.
	 */

	DUK_ASSERT((((duk_uint8_t *) thr->valstack_bottom) - ((duk_uint8_t *) thr->valstack)) == old_bottom_offset);
	DUK_ASSERT((((duk_uint8_t *) thr->valstack_top) - ((duk_uint8_t *) thr->valstack)) == old_top_offset);

	/* success, fixup pointers */
	old_end_offset_post = (((duk_uint8_t *) thr->valstack_end) - ((duk_uint8_t *) thr->valstack));  /* must be computed after realloc */
#ifdef DUK_USE_DEBUG
	old_valstack_post = thr->valstack;
#endif
	thr->valstack = new_valstack;
	thr->valstack_end = new_valstack + new_size;
	thr->valstack_bottom = (duk_tval *) ((duk_uint8_t *) new_valstack + old_bottom_offset);
	thr->valstack_top = (duk_tval *) ((duk_uint8_t *) new_valstack + old_top_offset);

	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	/* useful for debugging */
#ifdef DUK_USE_DEBUG
	if (old_end_offset_pre != old_end_offset_post) {
		DUK_D(DUK_DPRINT("valstack was resized during valstack_resize(), probably by mark-and-sweep; "
		                 "end offset changed: %lu -> %lu",
		                 (unsigned long) old_end_offset_pre,
		                 (unsigned long) old_end_offset_post));
	}
	if (old_valstack_pre != old_valstack_post) {
		DUK_D(DUK_DPRINT("valstack pointer changed during valstack_resize(), probably by mark-and-sweep: %p -> %p",
		                 (void *) old_valstack_pre,
		                 (void *) old_valstack_post));
	}
#endif

	DUK_DD(DUK_DDPRINT("resized valstack to %lu elements (%lu bytes), bottom=%ld, top=%ld, "
	                   "new pointers: start=%p end=%p bottom=%p top=%p",
	                   (unsigned long) new_size, (unsigned long) new_alloc_size,
	                   (long) (thr->valstack_bottom - thr->valstack),
	                   (long) (thr->valstack_top - thr->valstack),
	                   (void *) thr->valstack, (void *) thr->valstack_end,
	                   (void *) thr->valstack_bottom, (void *) thr->valstack_top));

	/* init newly allocated slots (only) */
	p = (duk_tval *) ((duk_uint8_t *) thr->valstack + old_end_offset_post);
	while (p < thr->valstack_end) {
		/* never executed if new size is smaller */
		DUK_TVAL_SET_UNDEFINED_UNUSED(p);
		p++;
	}

	/* assertion check: we maintain elements above top in known state */
#ifdef DUK_USE_ASSERTIONS
	p = thr->valstack_top;
	while (p < thr->valstack_end) {
		/* everything above old valstack top should be preinitialized now */
		DUK_ASSERT(DUK_TVAL_IS_UNDEFINED_UNUSED(p));
		p++;
	}
#endif
	return 1;
}

DUK_INTERNAL
duk_bool_t duk_valstack_resize_raw(duk_context *ctx,
                                   duk_size_t min_new_size,
                                   duk_small_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_size_t old_size;
	duk_size_t new_size;
	duk_bool_t is_shrink = 0;
	duk_small_uint_t shrink_flag = (flags & DUK_VSRESIZE_FLAG_SHRINK);
	duk_small_uint_t compact_flag = (flags & DUK_VSRESIZE_FLAG_COMPACT);
	duk_small_uint_t throw_flag = (flags & DUK_VSRESIZE_FLAG_THROW);

	DUK_DDD(DUK_DDDPRINT("check valstack resize: min_new_size=%lu, curr_size=%ld, curr_top=%ld, "
	                     "curr_bottom=%ld, shrink=%d, compact=%d, throw=%d",
	                     (unsigned long) min_new_size,
	                     (long) (thr->valstack_end - thr->valstack),
	                     (long) (thr->valstack_top - thr->valstack),
	                     (long) (thr->valstack_bottom - thr->valstack),
	                     (int) shrink_flag, (int) compact_flag, (int) throw_flag));

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	old_size = (duk_size_t) (thr->valstack_end - thr->valstack);

	if (min_new_size <= old_size) {
		is_shrink = 1;
		if (!shrink_flag ||
		    old_size - min_new_size < DUK_VALSTACK_SHRINK_THRESHOLD) {
			DUK_DDD(DUK_DDDPRINT("no need to grow or shrink valstack"));
			return 1;
		}
	}

	new_size = min_new_size;
	if (!compact_flag) {
		if (is_shrink) {
			/* shrink case; leave some spare */
			new_size += DUK_VALSTACK_SHRINK_SPARE;
		}

		/* round up roughly to next 'grow step' */
		new_size = (new_size / DUK_VALSTACK_GROW_STEP + 1) * DUK_VALSTACK_GROW_STEP;
	}

	DUK_DD(DUK_DDPRINT("want to %s valstack: %lu -> %lu elements (min_new_size %lu)",
	                   (const char *) (new_size > old_size ? "grow" : "shrink"),
	                   (unsigned long) old_size, (unsigned long) new_size,
	                   (unsigned long) min_new_size));

	if (new_size > thr->valstack_max) {
		/* Note: may be triggered even if minimal new_size would not reach the limit,
		 * plan limit accordingly (taking DUK_VALSTACK_GROW_STEP into account).
		 */
		if (throw_flag) {
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_VALSTACK_LIMIT);
		} else {
			return 0;
		}
	}

	/*
	 *  When resizing the valstack, a mark-and-sweep may be triggered for
	 *  the allocation of the new valstack.  If the mark-and-sweep needs
	 *  to use our thread for something, it may cause *the same valstack*
	 *  to be resized recursively.  This happens e.g. when mark-and-sweep
	 *  finalizers are called.  This is taken into account carefully in
	 *  duk__resize_valstack().
	 *
	 *  'new_size' is known to be <= valstack_max, which ensures that
	 *  size_t and pointer arithmetic won't wrap in duk__resize_valstack().
	 */

	if (!duk__resize_valstack(ctx, new_size)) {
		if (is_shrink) {
			DUK_DD(DUK_DDPRINT("valstack resize failed, but is a shrink, ignore"));
			return 1;
		}

		DUK_DD(DUK_DDPRINT("valstack resize failed"));

		if (throw_flag) {
			DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_FAILED_TO_EXTEND_VALSTACK);
		} else {
			return 0;
		}
	}

	DUK_DDD(DUK_DDDPRINT("valstack resize successful"));
	return 1;
}

DUK_EXTERNAL duk_bool_t duk_check_stack(duk_context *ctx, duk_idx_t extra) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_size_t min_new_size;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	if (DUK_UNLIKELY(extra < 0)) {
		/* Clamping to zero makes the API more robust to calling code
		 * calculation errors.
		 */
		extra = 0;
	}

	min_new_size = (thr->valstack_top - thr->valstack) + extra + DUK_VALSTACK_INTERNAL_EXTRA;
	return duk_valstack_resize_raw(ctx,
	                               min_new_size,         /* min_new_size */
	                               0 /* no shrink */ |   /* flags */
	                               0 /* no compact */ |
	                               0 /* no throw */);
}

DUK_EXTERNAL void duk_require_stack(duk_context *ctx, duk_idx_t extra) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_size_t min_new_size;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	if (DUK_UNLIKELY(extra < 0)) {
		/* Clamping to zero makes the API more robust to calling code
		 * calculation errors.
		 */
		extra = 0;
	}

	min_new_size = (thr->valstack_top - thr->valstack) + extra + DUK_VALSTACK_INTERNAL_EXTRA;
	(void) duk_valstack_resize_raw(ctx,
	                               min_new_size,  /* min_new_size */
	                               0 /* no shrink */ |   /* flags */
	                               0 /* no compact */ |
	                               DUK_VSRESIZE_FLAG_THROW);
}

DUK_EXTERNAL duk_bool_t duk_check_stack_top(duk_context *ctx, duk_idx_t top) {
	duk_size_t min_new_size;

	DUK_ASSERT_CTX_VALID(ctx);

	if (DUK_UNLIKELY(top < 0)) {
		/* Clamping to zero makes the API more robust to calling code
		 * calculation errors.
		 */
		top = 0;
	}

	min_new_size = top + DUK_VALSTACK_INTERNAL_EXTRA;
	return duk_valstack_resize_raw(ctx,
	                               min_new_size,  /* min_new_size */
	                               0 /* no shrink */ |   /* flags */
	                               0 /* no compact */ |
	                               0 /* no throw */);
}

DUK_EXTERNAL void duk_require_stack_top(duk_context *ctx, duk_idx_t top) {
	duk_size_t min_new_size;

	DUK_ASSERT_CTX_VALID(ctx);

	if (DUK_UNLIKELY(top < 0)) {
		/* Clamping to zero makes the API more robust to calling code
		 * calculation errors.
		 */
		top = 0;
	}

	min_new_size = top + DUK_VALSTACK_INTERNAL_EXTRA;
	(void) duk_valstack_resize_raw(ctx,
	                               min_new_size,  /* min_new_size */
	                               0 /* no shrink */ |   /* flags */
	                               0 /* no compact */ |
	                               DUK_VSRESIZE_FLAG_THROW);
}

/*
 *  Basic stack manipulation: swap, dup, insert, replace, etc
 */

DUK_EXTERNAL void duk_swap(duk_context *ctx, duk_idx_t index1, duk_idx_t index2) {
	duk_tval *tv1;
	duk_tval *tv2;
	duk_tval tv_tmp;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_require_tval(ctx, index1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, index2);
	DUK_ASSERT(tv2 != NULL);

	/* If tv1==tv2 this is a NOP, no check is needed */
	DUK_TVAL_SET_TVAL(&tv_tmp, tv1);
	DUK_TVAL_SET_TVAL(tv1, tv2);
	DUK_TVAL_SET_TVAL(tv2, &tv_tmp);
}

DUK_EXTERNAL void duk_swap_top(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_swap(ctx, index, -1);
}

DUK_EXTERNAL void duk_dup(duk_context *ctx, duk_idx_t from_index) {
	duk_hthread *thr;
	duk_tval *tv_from;
	duk_tval *tv_to;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();

	tv_from = duk_require_tval(ctx, from_index);
	tv_to = thr->valstack_top++;
	DUK_ASSERT(tv_from != NULL);
	DUK_ASSERT(tv_to != NULL);
	DUK_TVAL_SET_TVAL(tv_to, tv_from);
	DUK_TVAL_INCREF(thr, tv_to);  /* no side effects */
}

DUK_EXTERNAL void duk_dup_top(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_from;
	duk_tval *tv_to;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();

	if (thr->valstack_top - thr->valstack_bottom <= 0) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_INDEX);
	}
	tv_from = thr->valstack_top - 1;
	tv_to = thr->valstack_top++;
	DUK_ASSERT(tv_from != NULL);
	DUK_ASSERT(tv_to != NULL);
	DUK_TVAL_SET_TVAL(tv_to, tv_from);
	DUK_TVAL_INCREF(thr, tv_to);  /* no side effects */
}

DUK_EXTERNAL void duk_insert(duk_context *ctx, duk_idx_t to_index) {
	duk_tval *p;
	duk_tval *q;
	duk_tval tv_tmp;
	duk_size_t nbytes;

	DUK_ASSERT_CTX_VALID(ctx);

	p = duk_require_tval(ctx, to_index);
	DUK_ASSERT(p != NULL);
	q = duk_require_tval(ctx, -1);
	DUK_ASSERT(q != NULL);

	DUK_ASSERT(q >= p);

	/*              nbytes
	 *           <--------->
	 *    [ ... | p | x | x | q ]
	 * => [ ... | q | p | x | x ]
	 */

	nbytes = (duk_size_t) (((duk_uint8_t *) q) - ((duk_uint8_t *) p));  /* Note: 'q' is top-1 */

	DUK_DDD(DUK_DDDPRINT("duk_insert: to_index=%ld, p=%p, q=%p, nbytes=%lu",
	                     (long) to_index, (void *) p, (void *) q, (unsigned long) nbytes));

	/* No net refcount changes. */

	if (nbytes > 0) {
		DUK_TVAL_SET_TVAL(&tv_tmp, q);
		DUK_ASSERT(nbytes > 0);
		DUK_MEMMOVE((void *) (p + 1), (void *) p, nbytes);
		DUK_TVAL_SET_TVAL(p, &tv_tmp);
	} else {
		/* nop: insert top to top */
		DUK_ASSERT(nbytes == 0);
		DUK_ASSERT(p == q);
	}
}

DUK_EXTERNAL void duk_replace(duk_context *ctx, duk_idx_t to_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1;
	duk_tval *tv2;
	duk_tval tv_tmp;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_require_tval(ctx, -1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, to_index);
	DUK_ASSERT(tv2 != NULL);

	/* For tv1 == tv2, both pointing to stack top, the end result
	 * is same as duk_pop(ctx).
	 */

	DUK_TVAL_SET_TVAL(&tv_tmp, tv2);
	DUK_TVAL_SET_TVAL(tv2, tv1);
	DUK_TVAL_SET_UNDEFINED_UNUSED(tv1);
	thr->valstack_top--;
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
}

DUK_EXTERNAL void duk_copy(duk_context *ctx, duk_idx_t from_index, duk_idx_t to_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1;
	duk_tval *tv2;
	duk_tval tv_tmp;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);  /* w/o refcounting */

	tv1 = duk_require_tval(ctx, from_index);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, to_index);
	DUK_ASSERT(tv2 != NULL);

	/* For tv1 == tv2, this is a no-op (no explicit check needed). */

	DUK_TVAL_SET_TVAL(&tv_tmp, tv2);
	DUK_TVAL_SET_TVAL(tv2, tv1);
	DUK_TVAL_INCREF(thr, tv2);  /* no side effects */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
}

DUK_EXTERNAL void duk_remove(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *p;
	duk_tval *q;
#ifdef DUK_USE_REFERENCE_COUNTING
	duk_tval tv_tmp;
#endif
	duk_size_t nbytes;

	DUK_ASSERT_CTX_VALID(ctx);

	p = duk_require_tval(ctx, index);
	DUK_ASSERT(p != NULL);
	q = duk_require_tval(ctx, -1);
	DUK_ASSERT(q != NULL);

	DUK_ASSERT(q >= p);

	/*              nbytes            zero size case
	 *           <--------->
	 *    [ ... | p | x | x | q ]     [ ... | p==q ]
	 * => [ ... | x | x | q ]         [ ... ]
	 */

#ifdef DUK_USE_REFERENCE_COUNTING
	/* use a temp: decref only when valstack reachable values are correct */
	DUK_TVAL_SET_TVAL(&tv_tmp, p);
#endif

	nbytes = (duk_size_t) (((duk_uint8_t *) q) - ((duk_uint8_t *) p));  /* Note: 'q' is top-1 */
	DUK_MEMMOVE(p, p + 1, nbytes);  /* zero size not an issue: pointers are valid */

	DUK_TVAL_SET_UNDEFINED_UNUSED(q);
	thr->valstack_top--;

#ifdef DUK_USE_REFERENCE_COUNTING
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
#endif
}

/*
 *  Stack slice primitives
 */

DUK_EXTERNAL void duk_xcopymove_raw(duk_context *to_ctx, duk_context *from_ctx, duk_idx_t count, duk_bool_t is_copy) {
	duk_hthread *to_thr = (duk_hthread *) to_ctx;
	duk_hthread *from_thr = (duk_hthread *) from_ctx;
	void *src;
	duk_size_t nbytes;
	duk_tval *p;
	duk_tval *q;

	/* XXX: several pointer comparison issues here */

	DUK_ASSERT_CTX_VALID(to_ctx);
	DUK_ASSERT_CTX_VALID(from_ctx);
	DUK_ASSERT(to_ctx != NULL);
	DUK_ASSERT(from_ctx != NULL);

	if (to_ctx == from_ctx) {
		DUK_ERROR(to_thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CONTEXT);
		return;
	}
	if ((count < 0) ||
	    (count > (duk_idx_t) to_thr->valstack_max)) {
		/* Maximum value check ensures 'nbytes' won't wrap below. */
		DUK_ERROR(to_thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_COUNT);
		return;
	}

	nbytes = sizeof(duk_tval) * count;
	if (nbytes == 0) {
		return;
	}
	DUK_ASSERT(to_thr->valstack_top <= to_thr->valstack_end);
	if ((duk_size_t) ((duk_uint8_t *) to_thr->valstack_end - (duk_uint8_t *) to_thr->valstack_top) < nbytes) {
		DUK_ERROR(to_thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}
	src = (void *) ((duk_uint8_t *) from_thr->valstack_top - nbytes);
	if (src < (void *) from_thr->valstack_bottom) {
		DUK_ERROR(to_thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_COUNT);
	}

	/* copy values (no overlap even if to_ctx == from_ctx; that's not
	 * allowed now anyway)
	 */
	DUK_ASSERT(nbytes > 0);
	DUK_MEMCPY((void *) to_thr->valstack_top, src, nbytes);

	p = to_thr->valstack_top;
	to_thr->valstack_top = (duk_tval *) (((duk_uint8_t *) p) + nbytes);

	if (is_copy) {
		/* incref copies, keep originals */
		q = to_thr->valstack_top;
		while (p < q) {
			DUK_TVAL_INCREF(to_thr, p);  /* no side effects */
			p++;
		}
	} else {
		/* no net refcount change */
		p = from_thr->valstack_top;
		q = (duk_tval *) (((duk_uint8_t *) p) - nbytes);
		from_thr->valstack_top = q;

		/* elements above stack top are kept UNUSED */
		while (p > q) {
			p--;
			DUK_TVAL_SET_UNDEFINED_UNUSED(p);

			/* XXX: fast primitive to set a bunch of values to UNDEFINED_UNUSED */
		}
	}
}

/*
 *  Get/require
 */

DUK_EXTERNAL void duk_require_undefined(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_UNDEFINED(tv)) {
		/* Note: accept both 'actual' and 'unused' undefined */
		return;
	}
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_UNDEFINED);
}

DUK_EXTERNAL void duk_require_null(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_NULL(tv)) {
		return;
	}
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_NULL);
	return;  /* not reachable */
}

DUK_EXTERNAL duk_bool_t duk_get_boolean(duk_context *ctx, duk_idx_t index) {
	duk_bool_t ret = 0;  /* default: false */
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_BOOLEAN(tv)) {
		ret = DUK_TVAL_GET_BOOLEAN(tv);
	}

	DUK_ASSERT(ret == 0 || ret == 1);
	return ret;
}

DUK_EXTERNAL duk_bool_t duk_require_boolean(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_BOOLEAN(tv)) {
		duk_bool_t ret = DUK_TVAL_GET_BOOLEAN(tv);
		DUK_ASSERT(ret == 0 || ret == 1);
		return ret;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_BOOLEAN);
	return 0;  /* not reachable */
}

DUK_EXTERNAL duk_double_t duk_get_number(duk_context *ctx, duk_idx_t index) {
	duk_double_union ret;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	ret.d = DUK_DOUBLE_NAN;  /* default: NaN */
	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_NUMBER(tv)) {
		ret.d = DUK_TVAL_GET_NUMBER(tv);
	}

	/*
	 *  Number should already be in NaN-normalized form, but let's
	 *  normalize anyway.
	 */

	DUK_DBLUNION_NORMALIZE_NAN_CHECK(&ret);
	return ret.d;
}

DUK_EXTERNAL duk_double_t duk_require_number(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_NUMBER(tv)) {
		duk_double_union ret;
		ret.d = DUK_TVAL_GET_NUMBER(tv);

		/*
		 *  Number should already be in NaN-normalized form,
		 *  but let's normalize anyway.
		 */

		DUK_DBLUNION_NORMALIZE_NAN_CHECK(&ret);
		return ret.d;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_NUMBER);
	return DUK_DOUBLE_NAN;  /* not reachable */
}

DUK_EXTERNAL duk_int_t duk_get_int(duk_context *ctx, duk_idx_t index) {
	/* Custom coercion for API */
	DUK_ASSERT_CTX_VALID(ctx);
	return (duk_int_t) duk__api_coerce_d2i(ctx, index, 0 /*require*/);
}

DUK_EXTERNAL duk_uint_t duk_get_uint(duk_context *ctx, duk_idx_t index) {
	/* Custom coercion for API */
	DUK_ASSERT_CTX_VALID(ctx);
	return (duk_uint_t) duk__api_coerce_d2ui(ctx, index, 0 /*require*/);
}

DUK_EXTERNAL duk_int_t duk_require_int(duk_context *ctx, duk_idx_t index) {
	/* Custom coercion for API */
	DUK_ASSERT_CTX_VALID(ctx);
	return (duk_int_t) duk__api_coerce_d2i(ctx, index, 1 /*require*/);
}

DUK_EXTERNAL duk_uint_t duk_require_uint(duk_context *ctx, duk_idx_t index) {
	/* Custom coercion for API */
	DUK_ASSERT_CTX_VALID(ctx);
	return (duk_uint_t) duk__api_coerce_d2ui(ctx, index, 1 /*require*/);
}

DUK_EXTERNAL const char *duk_get_lstring(duk_context *ctx, duk_idx_t index, duk_size_t *out_len) {
	const char *ret;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	/* default: NULL, length 0 */
	ret = NULL;
	if (out_len) {
		*out_len = 0;
	}

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_STRING(tv)) {
		/* Here we rely on duk_hstring instances always being zero
		 * terminated even if the actual string is not.
		 */
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);
		ret = (const char *) DUK_HSTRING_GET_DATA(h);
		if (out_len) {
			*out_len = DUK_HSTRING_GET_BYTELEN(h);
		}
	}

	return ret;
}

DUK_EXTERNAL const char *duk_require_lstring(duk_context *ctx, duk_idx_t index, duk_size_t *out_len) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const char *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: this check relies on the fact that even a zero-size string
	 * has a non-NULL pointer.
	 */
	ret = duk_get_lstring(ctx, index, out_len);
	if (ret) {
		return ret;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_STRING);
	return NULL;  /* not reachable */
}

DUK_EXTERNAL const char *duk_get_string(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk_get_lstring(ctx, index, NULL);
}

DUK_EXTERNAL const char *duk_require_string(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk_require_lstring(ctx, index, NULL);
}

DUK_EXTERNAL void *duk_get_pointer(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_POINTER(tv)) {
		void *p = DUK_TVAL_GET_POINTER(tv);  /* may be NULL */
		return (void *) p;
	}

	return NULL;
}

DUK_EXTERNAL void *duk_require_pointer(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: here we must be wary of the fact that a pointer may be
	 * valid and be a NULL.
	 */
	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_POINTER(tv)) {
		void *p = DUK_TVAL_GET_POINTER(tv);  /* may be NULL */
		return (void *) p;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_POINTER);
	return NULL;  /* not reachable */
}

#if 0  /*unused*/
DUK_INTERNAL void *duk_get_voidptr(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		return (void *) h;
	}

	return NULL;
}
#endif

DUK_EXTERNAL void *duk_get_buffer(duk_context *ctx, duk_idx_t index, duk_size_t *out_size) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	if (out_size != NULL) {
		*out_size = 0;
	}

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		if (out_size) {
			*out_size = DUK_HBUFFER_GET_SIZE(h);
		}
		return (void *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h);  /* may be NULL (but only if size is 0) */
	}

	return NULL;
}

DUK_EXTERNAL void *duk_require_buffer(duk_context *ctx, duk_idx_t index, duk_size_t *out_size) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	if (out_size != NULL) {
		*out_size = 0;
	}

	/* Note: here we must be wary of the fact that a data pointer may
	 * be a NULL for a zero-size buffer.
	 */

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		if (out_size) {
			*out_size = DUK_HBUFFER_GET_SIZE(h);
		}
		return (void *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h);  /* may be NULL (but only if size is 0) */
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_BUFFER);
	return NULL;  /* not reachable */
}

/* Raw helper for getting a value from the stack, checking its tag, and possible its object class.
 * The tag cannot be a number because numbers don't have an internal tag in the packed representation.
 */
DUK_INTERNAL duk_heaphdr *duk_get_tagged_heaphdr_raw(duk_context *ctx, duk_idx_t index, duk_uint_t flags_and_tag) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_small_uint_t tag = flags_and_tag & 0xffffU;  /* tags can be up to 16 bits */

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && (DUK_TVAL_GET_TAG(tv) == tag)) {
		duk_heaphdr *ret;

		/* Note: tag comparison in general doesn't work for numbers,
		 * but it does work for everything else (heap objects here).
		 */
		ret = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(ret != NULL);  /* tagged null pointers should never occur */

		/* If class check has been requested, tag must also be DUK_TAG_OBJECT.
		 * This allows us to just check the class check flag without checking
		 * the tag also.
		 */
		DUK_ASSERT((flags_and_tag & DUK_GETTAGGED_FLAG_CHECK_CLASS) == 0 ||
		           tag == DUK_TAG_OBJECT);

		if ((flags_and_tag & DUK_GETTAGGED_FLAG_CHECK_CLASS) == 0 ||  /* no class check */
		    (duk_int_t) DUK_HOBJECT_GET_CLASS_NUMBER((duk_hobject *) ret) ==  /* or class check matches */
		        (duk_int_t) ((flags_and_tag >> DUK_GETTAGGED_CLASS_SHIFT) & 0xff)) {
			return ret;
		}
	}

	if (flags_and_tag & DUK_GETTAGGED_FLAG_ALLOW_NULL) {
		return (duk_heaphdr *) NULL;
	}

	/* Formatting the tag number here is not very useful: the tag value
	 * is Duktape internal (not the same as DUK_TYPE_xxx) and even depends
	 * on the duk_tval layout.  If anything, add a human readable type here.
	 */
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_UNEXPECTED_TYPE);
	return NULL;  /* not reachable */
}

DUK_INTERNAL duk_hstring *duk_get_hstring(duk_context *ctx, duk_idx_t index) {
	return (duk_hstring *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_STRING | DUK_GETTAGGED_FLAG_ALLOW_NULL);
}

DUK_INTERNAL duk_hstring *duk_require_hstring(duk_context *ctx, duk_idx_t index) {
	return (duk_hstring *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_STRING);
}

DUK_INTERNAL duk_hobject *duk_get_hobject(duk_context *ctx, duk_idx_t index) {
	return (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
}

DUK_INTERNAL duk_hobject *duk_require_hobject(duk_context *ctx, duk_idx_t index) {
	return (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
}

DUK_INTERNAL duk_hbuffer *duk_get_hbuffer(duk_context *ctx, duk_idx_t index) {
	return (duk_hbuffer *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_BUFFER | DUK_GETTAGGED_FLAG_ALLOW_NULL);
}

DUK_INTERNAL duk_hbuffer *duk_require_hbuffer(duk_context *ctx, duk_idx_t index) {
	return (duk_hbuffer *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_BUFFER);
}

DUK_INTERNAL duk_hthread *duk_get_hthread(duk_context *ctx, duk_idx_t index) {
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
	if (h != NULL && !DUK_HOBJECT_IS_THREAD(h)) {
		h = NULL;
	}
	return (duk_hthread *) h;
}

DUK_INTERNAL duk_hthread *duk_require_hthread(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
	DUK_ASSERT(h != NULL);
	if (!DUK_HOBJECT_IS_THREAD(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_THREAD);
	}
	return (duk_hthread *) h;
}

DUK_INTERNAL duk_hcompiledfunction *duk_get_hcompiledfunction(duk_context *ctx, duk_idx_t index) {
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
	if (h != NULL && !DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		h = NULL;
	}
	return (duk_hcompiledfunction *) h;
}

#if 0  /*unused*/
DUK_INTERNAL duk_hcompiledfunction *duk_require_hcompiledfunction(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
	DUK_ASSERT(h != NULL);
	if (!DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_COMPILEDFUNCTION);
	}
	return (duk_hcompiledfunction *) h;
}
#endif

DUK_INTERNAL duk_hnativefunction *duk_get_hnativefunction(duk_context *ctx, duk_idx_t index) {
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
	if (h != NULL && !DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		h = NULL;
	}
	return (duk_hnativefunction *) h;
}

DUK_INTERNAL duk_hnativefunction *duk_require_hnativefunction(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
	DUK_ASSERT(h != NULL);
	if (!DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_NATIVEFUNCTION);
	}
	return (duk_hnativefunction *) h;
}

DUK_EXTERNAL duk_c_function duk_get_c_function(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;
	duk_hobject *h;
	duk_hnativefunction *f;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return NULL;
	}
	if (!DUK_TVAL_IS_OBJECT(tv)) {
		return NULL;
	}
	h = DUK_TVAL_GET_OBJECT(tv);
	DUK_ASSERT(h != NULL);

	if (!DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		return NULL;
	}
	DUK_ASSERT(DUK_HOBJECT_HAS_NATIVEFUNCTION(h));
	f = (duk_hnativefunction *) h;

	return f->func;
}

DUK_EXTERNAL duk_c_function duk_require_c_function(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_c_function ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = duk_get_c_function(ctx, index);
	if (!ret) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_C_FUNCTION);
	}
	return ret;
}

DUK_EXTERNAL duk_context *duk_get_context(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_context *) duk_get_hthread(ctx, index);
}

DUK_EXTERNAL duk_context *duk_require_context(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_context *) duk_require_hthread(ctx, index);
}

DUK_EXTERNAL void *duk_get_heapptr(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;
	void *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		ret = (void *) DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(ret != NULL);
		return ret;
	}

	return (void *) NULL;
}

DUK_EXTERNAL void *duk_require_heapptr(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	void *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		ret = (void *) DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(ret != NULL);
		return ret;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_UNEXPECTED_TYPE);
	return (void *) NULL;  /* not reachable */
}

#if 0
/* This would be pointless: we'd return NULL for both lightfuncs and
 * unexpected types.
 */
duk_hobject *duk_get_hobject_or_lfunc(duk_context *ctx, duk_idx_t index) {
}
#endif

/* Useful for internal call sites where we either expect an object (function)
 * or a lightfunc.  Accepts an object (returned as is) or a lightfunc (coerced
 * to an object).  Return value is NULL if value is neither an object nor a
 * lightfunc.
 */
duk_hobject *duk_get_hobject_or_lfunc_coerce(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_OBJECT(tv)) {
		return DUK_TVAL_GET_OBJECT(tv);
	} else if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
		duk_to_object(ctx, index);
		return duk_require_hobject(ctx, index);
	}

	return NULL;
}

/* Useful for internal call sites where we either expect an object (function)
 * or a lightfunc.  Returns NULL for a lightfunc.
 */
DUK_INTERNAL duk_hobject *duk_require_hobject_or_lfunc(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_OBJECT(tv)) {
		return DUK_TVAL_GET_OBJECT(tv);
	} else if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
		return NULL;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_UNEXPECTED_TYPE);
	return NULL;  /* not reachable */
}

/* Useful for internal call sites where we either expect an object (function)
 * or a lightfunc.  Accepts an object (returned as is) or a lightfunc (coerced
 * to an object).  Return value is never NULL.
 */
DUK_INTERNAL duk_hobject *duk_require_hobject_or_lfunc_coerce(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	if (DUK_TVAL_IS_OBJECT(tv)) {
		return DUK_TVAL_GET_OBJECT(tv);
	} else if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
		duk_to_object(ctx, index);
		return duk_require_hobject(ctx, index);
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_UNEXPECTED_TYPE);
	return NULL;  /* not reachable */
}

DUK_EXTERNAL duk_size_t duk_get_length(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return 0;
	}

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
	case DUK_TAG_BOOLEAN:
	case DUK_TAG_POINTER:
		return 0;
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);
		return (duk_size_t) DUK_HSTRING_GET_CHARLEN(h);
	}
	case DUK_TAG_OBJECT: {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		return (duk_size_t) duk_hobject_get_length((duk_hthread *) ctx, h);
	}
	case DUK_TAG_BUFFER: {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (duk_size_t) DUK_HBUFFER_GET_SIZE(h);
	}
	case DUK_TAG_LIGHTFUNC: {
		duk_small_uint_t lf_flags;
		lf_flags = DUK_TVAL_GET_LIGHTFUNC_FLAGS(tv);
		return (duk_size_t) DUK_LFUNC_FLAGS_GET_LENGTH(lf_flags);
	}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default:
		/* number */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return 0;
	}

	DUK_UNREACHABLE();
}

DUK_INTERNAL void duk_set_length(duk_context *ctx, duk_idx_t index, duk_size_t length) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hobject(ctx, index);
	if (!h) {
		return;
	}

	duk_hobject_set_length(thr, h, (duk_uint32_t) length);  /* XXX: typing */
}

/*
 *  Conversions and coercions
 *
 *  The conversion/coercions are in-place operations on the value stack.
 *  Some operations are implemented here directly, while others call a
 *  helper in duk_js_ops.c after validating arguments.
 */

/* E5 Section 8.12.8 */

DUK_LOCAL duk_bool_t duk__defaultvalue_coerce_attempt(duk_context *ctx, duk_idx_t index, duk_small_int_t func_stridx) {
	if (duk_get_prop_stridx(ctx, index, func_stridx)) {
		/* [ ... func ] */
		if (duk_is_callable(ctx, -1)) {
			duk_dup(ctx, index);         /* -> [ ... func this ] */
			duk_call_method(ctx, 0);     /* -> [ ... retval ] */
			if (duk_is_primitive(ctx, -1)) {
				duk_replace(ctx, index);
				return 1;
			}
			/* [ ... retval ]; popped below */
		}
	}
	duk_pop(ctx);  /* [ ... func/retval ] -> [ ... ] */
	return 0;
}

DUK_EXTERNAL void duk_to_defaultvalue(duk_context *ctx, duk_idx_t index, duk_int_t hint) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	/* inline initializer for coercers[] is not allowed by old compilers like BCC */
	duk_small_int_t coercers[2];

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	coercers[0] = DUK_STRIDX_VALUE_OF;
	coercers[1] = DUK_STRIDX_TO_STRING;

	index = duk_require_normalize_index(ctx, index);
	obj = duk_require_hobject_or_lfunc(ctx, index);

	if (hint == DUK_HINT_NONE) {
		if (obj != NULL && DUK_HOBJECT_GET_CLASS_NUMBER(obj) == DUK_HOBJECT_CLASS_DATE) {
			hint = DUK_HINT_STRING;
		} else {
			hint = DUK_HINT_NUMBER;
		}
	}

	if (hint == DUK_HINT_STRING) {
		coercers[0] = DUK_STRIDX_TO_STRING;
		coercers[1] = DUK_STRIDX_VALUE_OF;
	}

	if (duk__defaultvalue_coerce_attempt(ctx, index, coercers[0])) {
		return;
	}

	if (duk__defaultvalue_coerce_attempt(ctx, index, coercers[1])) {
		return;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_DEFAULTVALUE_COERCE_FAILED);
}

DUK_EXTERNAL void duk_to_undefined(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_UNDEFINED_ACTUAL(tv);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
}

DUK_EXTERNAL void duk_to_null(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_NULL(tv);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
}

/* E5 Section 9.1 */
DUK_EXTERNAL void duk_to_primitive(duk_context *ctx, duk_idx_t index, duk_int_t hint) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(hint == DUK_HINT_NONE || hint == DUK_HINT_NUMBER || hint == DUK_HINT_STRING);

	index = duk_require_normalize_index(ctx, index);

	if (!duk_check_type_mask(ctx, index, DUK_TYPE_MASK_OBJECT |
	                                     DUK_TYPE_MASK_LIGHTFUNC)) {
		/* everything except object stay as is */
		return;
	}
	duk_to_defaultvalue(ctx, index, hint);
}

/* E5 Section 9.2 */
DUK_EXTERNAL duk_bool_t duk_to_boolean(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_bool_t val;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	val = duk_js_toboolean(tv);
	DUK_ASSERT(val == 0 || val == 1);

	/* Note: no need to re-lookup tv, conversion is side effect free */
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_BOOLEAN(tv, val);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	return val;
}

DUK_EXTERNAL duk_double_t duk_to_number(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_double_t d;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	/* XXX: fastint? */
	d = duk_js_tonumber(thr, tv);

	/* Note: need to re-lookup because ToNumber() may have side effects */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	return d;
}

/* XXX: combine all the integer conversions: they share everything
 * but the helper function for coercion.
 */

typedef duk_double_t (*duk__toint_coercer)(duk_hthread *thr, duk_tval *tv);

DUK_LOCAL duk_double_t duk__to_int_uint_helper(duk_context *ctx, duk_idx_t index, duk__toint_coercer coerce_func) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_double_t d;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = coerce_func(thr, tv);

	/* XXX: fastint? */

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	return d;
}

DUK_EXTERNAL duk_int_t duk_to_int(duk_context *ctx, duk_idx_t index) {
	/* Value coercion (in stack): ToInteger(), E5 Section 9.4
	 * API return value coercion: custom
	 */
	DUK_ASSERT_CTX_VALID(ctx);
	(void) duk__to_int_uint_helper(ctx, index, duk_js_tointeger);
	return (duk_int_t) duk__api_coerce_d2i(ctx, index, 0 /*require*/);
}

DUK_EXTERNAL duk_uint_t duk_to_uint(duk_context *ctx, duk_idx_t index) {
	/* Value coercion (in stack): ToInteger(), E5 Section 9.4
	 * API return value coercion: custom
	 */
	DUK_ASSERT_CTX_VALID(ctx);
	(void) duk__to_int_uint_helper(ctx, index, duk_js_tointeger);
	return (duk_uint_t) duk__api_coerce_d2ui(ctx, index, 0 /*require*/);
}

DUK_EXTERNAL duk_int32_t duk_to_int32(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_int32_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	ret = duk_js_toint32(thr, tv);

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, index);
#if defined(DUK_USE_FASTINT)
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_FASTINT_I32(tv, ret);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	return ret;
#else
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_NUMBER(tv, (duk_double_t) ret);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	return ret;
#endif
}

DUK_EXTERNAL duk_uint32_t duk_to_uint32(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_uint32_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	ret = duk_js_touint32(thr, tv);

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, index);
#if defined(DUK_USE_FASTINT)
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_FASTINT_U32(tv, ret);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	return ret;
#else
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_NUMBER(tv, (duk_double_t) ret);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
#endif
	return ret;
}

DUK_EXTERNAL duk_uint16_t duk_to_uint16(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_uint16_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	ret = duk_js_touint16(thr, tv);

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, index);
#if defined(DUK_USE_FASTINT)
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_FASTINT_U32(tv, ret);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	return ret;
#else
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
	DUK_TVAL_SET_NUMBER(tv, (duk_double_t) ret);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
#endif
	return ret;
}

DUK_EXTERNAL const char *duk_to_lstring(duk_context *ctx, duk_idx_t index, duk_size_t *out_len) {
	DUK_ASSERT_CTX_VALID(ctx);

	(void) duk_to_string(ctx, index);
	return duk_require_lstring(ctx, index, out_len);
}

DUK_LOCAL duk_ret_t duk__safe_to_string_raw(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_to_string(ctx, -1);
	return 1;
}

DUK_EXTERNAL const char *duk_safe_to_lstring(duk_context *ctx, duk_idx_t index, duk_size_t *out_len) {
	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_require_normalize_index(ctx, index);

	/* We intentionally ignore the duk_safe_call() return value and only
	 * check the output type.  This way we don't also need to check that
	 * the returned value is indeed a string in the success case.
	 */

	duk_dup(ctx, index);
	(void) duk_safe_call(ctx, duk__safe_to_string_raw, 1 /*nargs*/, 1 /*nrets*/);
	if (!duk_is_string(ctx, -1)) {
		/* Error: try coercing error to string once. */
		(void) duk_safe_call(ctx, duk__safe_to_string_raw, 1 /*nargs*/, 1 /*nrets*/);
		if (!duk_is_string(ctx, -1)) {
			/* Double error */
			duk_pop(ctx);
			duk_push_hstring_stridx(ctx, DUK_STRIDX_UC_ERROR);
		} else {
			;
		}
	} else {
		;
	}
	DUK_ASSERT(duk_is_string(ctx, -1));

	duk_replace(ctx, index);
	return duk_require_lstring(ctx, index, out_len);
}

/* XXX: other variants like uint, u32 etc */
DUK_INTERNAL duk_int_t duk_to_int_clamped_raw(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval, duk_bool_t *out_clamped) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_double_t d, dmin, dmax;
	duk_int_t res;
	duk_bool_t clamped = 0;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = duk_js_tointeger(thr, tv);  /* E5 Section 9.4, ToInteger() */

	dmin = (duk_double_t) minval;
	dmax = (duk_double_t) maxval;

	if (d < dmin) {
		clamped = 1;
		res = minval;
		d = dmin;
	} else if (d > dmax) {
		clamped = 1;
		res = maxval;
		d = dmax;
	} else {
		res = (duk_int_t) d;
	}
	/* 'd' and 'res' agree here */

	/* Relookup in case duk_js_tointeger() ends up e.g. coercing an object. */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
#if defined(DUK_USE_FASTINT)
#if (DUK_INT_MAX <= 0x7fffffffL)
	DUK_TVAL_SET_FASTINT_I32(tv, res);
#else
	/* Clamping needed if duk_int_t is 64 bits. */
	if (res >= DUK_FASTINT_MIN && res <= DUK_FASTINT_MAX) {
		DUK_TVAL_SET_FASTINT(tv, res);
	} else {
		DUK_TVAL_SET_NUMBER(tv, d);
	}
#endif
#else
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
#endif
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */

	if (out_clamped) {
		*out_clamped = clamped;
	} else {
		/* coerced value is updated to value stack even when RangeError thrown */
		if (clamped) {
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_NUMBER_OUTSIDE_RANGE);
		}
	}

	return res;
}

DUK_INTERNAL duk_int_t duk_to_int_clamped(duk_context *ctx, duk_idx_t index, duk_idx_t minval, duk_idx_t maxval) {
	duk_bool_t dummy;
	return duk_to_int_clamped_raw(ctx, index, minval, maxval, &dummy);
}

DUK_INTERNAL duk_int_t duk_to_int_check_range(duk_context *ctx, duk_idx_t index, duk_int_t minval, duk_int_t maxval) {
	return duk_to_int_clamped_raw(ctx, index, minval, maxval, NULL);  /* out_clamped==NULL -> RangeError if outside range */
}

DUK_EXTERNAL const char *duk_to_string(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED: {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_LC_UNDEFINED);
		break;
	}
	case DUK_TAG_NULL: {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_LC_NULL);
		break;
	}
	case DUK_TAG_BOOLEAN: {
		if (DUK_TVAL_GET_BOOLEAN(tv)) {
			duk_push_hstring_stridx(ctx, DUK_STRIDX_TRUE);
		} else {
			duk_push_hstring_stridx(ctx, DUK_STRIDX_FALSE);
		}
		break;
	}
	case DUK_TAG_STRING: {
		/* nop */
		goto skip_replace;
	}
	case DUK_TAG_OBJECT: {
		duk_to_primitive(ctx, index, DUK_HINT_STRING);
		return duk_to_string(ctx, index);  /* Note: recursive call */
	}
	case DUK_TAG_BUFFER: {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);

		/* Note: this allows creation of internal strings. */

		DUK_ASSERT(h != NULL);
		duk_push_lstring(ctx,
		                 (const char *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h),
		                 (duk_size_t) DUK_HBUFFER_GET_SIZE(h));
		break;
	}
	case DUK_TAG_POINTER: {
		void *ptr = DUK_TVAL_GET_POINTER(tv);
		if (ptr != NULL) {
			duk_push_sprintf(ctx, DUK_STR_FMT_PTR, (void *) ptr);
		} else {
			/* Represent a null pointer as 'null' to be consistent with
			 * the JX format variant.  Native '%p' format for a NULL
			 * pointer may be e.g. '(nil)'.
			 */
			duk_push_hstring_stridx(ctx, DUK_STRIDX_LC_NULL);
		}
		break;
	}
	case DUK_TAG_LIGHTFUNC: {
		/* Should match Function.prototype.toString() */
		duk_push_lightfunc_tostring(ctx, tv);
		break;
	}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default: {
		/* number */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		duk_push_tval(ctx, tv);
		duk_numconv_stringify(ctx,
		                      10 /*radix*/,
		                      0 /*precision:shortest*/,
		                      0 /*force_exponential*/);
		break;
	}
	}

	duk_replace(ctx, index);

 skip_replace:
	return duk_require_string(ctx, index);
}

DUK_INTERNAL duk_hstring *duk_to_hstring(duk_context *ctx, duk_idx_t index) {
	duk_hstring *ret;
	DUK_ASSERT_CTX_VALID(ctx);
	duk_to_string(ctx, index);
	ret = duk_get_hstring(ctx, index);
	DUK_ASSERT(ret != NULL);
	return ret;
}

DUK_EXTERNAL void *duk_to_buffer_raw(duk_context *ctx, duk_idx_t index, duk_size_t *out_size, duk_uint_t mode) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbuffer *h_buf;
	const duk_uint8_t *src_data;
	duk_size_t src_size;
	duk_uint8_t *dst_data;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	index = duk_require_normalize_index(ctx, index);

	h_buf = duk_get_hbuffer(ctx, index);
	if (h_buf != NULL) {
		/* Buffer is kept as is, with the fixed/dynamic nature of the
		 * buffer only changed if requested.
		 */
		duk_uint_t tmp;

		src_data = (const duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h_buf);
		src_size = DUK_HBUFFER_GET_SIZE(h_buf);

		tmp = (DUK_HBUFFER_HAS_DYNAMIC(h_buf) ? DUK_BUF_MODE_DYNAMIC : DUK_BUF_MODE_FIXED);
		if (tmp == mode || mode == DUK_BUF_MODE_DONTCARE) {
			/* Note: src_data may be NULL if input is a zero-size
			 * dynamic buffer.
			 */
			dst_data = (duk_uint8_t *) src_data;
			goto skip_copy;
		}
	} else {
		/* Non-buffer value is first ToString() coerced, then converted
		 * to a buffer (fixed buffer is used unless a dynamic buffer is
		 * explicitly requested).
		 */

		src_data = (const duk_uint8_t *) duk_to_lstring(ctx, index, &src_size);
	}

	dst_data = (duk_uint8_t *) duk_push_buffer(ctx, src_size, (mode == DUK_BUF_MODE_DYNAMIC) /*dynamic*/);
	if (DUK_LIKELY(src_size > 0)) {
		/* When src_size == 0, src_data may be NULL (if source
		 * buffer is dynamic), and dst_data may be NULL (if
		 * target buffer is dynamic).  Avoid zero-size memcpy()
		 * with an invalid pointer.
		 */
		DUK_MEMCPY(dst_data, src_data, src_size);
	}
	duk_replace(ctx, index);
 skip_copy:

	if (out_size) {
		*out_size = src_size;
	}
	return dst_data;
}

DUK_EXTERNAL void *duk_to_pointer(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;
	void *res;

	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
	case DUK_TAG_BOOLEAN:
		res = NULL;
		break;
	case DUK_TAG_POINTER:
		res = DUK_TVAL_GET_POINTER(tv);
		break;
	case DUK_TAG_STRING:
	case DUK_TAG_OBJECT:
	case DUK_TAG_BUFFER:
		/* Heap allocated: return heap pointer which is NOT useful
		 * for the caller, except for debugging.
		 */
		res = (void *) DUK_TVAL_GET_HEAPHDR(tv);
		break;
	case DUK_TAG_LIGHTFUNC:
		/* Function pointers do not always cast correctly to void *
		 * (depends on memory and segmentation model for instance),
		 * so they coerce to NULL.
		 */
		res = NULL;
		break;
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default:
		/* number */
		res = NULL;
		break;
	}

	duk_push_pointer(ctx, res);
	duk_replace(ctx, index);
	return res;
}

DUK_EXTERNAL void duk_to_object(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_uint_t flags = 0;   /* shared flags for a subset of types */
	duk_small_int_t proto = 0;

	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL: {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_OBJECT_COERCIBLE);
		break;
	}
	case DUK_TAG_BOOLEAN: {
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BOOLEAN);
		proto = DUK_BIDX_BOOLEAN_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_STRING: {
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		        DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ |
		        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_STRING);
		proto = DUK_BIDX_STRING_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_OBJECT: {
		/* nop */
		break;
	}
	case DUK_TAG_BUFFER: {
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		        DUK_HOBJECT_FLAG_EXOTIC_BUFFEROBJ |
		        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BUFFER);
		proto = DUK_BIDX_BUFFER_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_POINTER: {
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_POINTER);
		proto = DUK_BIDX_POINTER_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_LIGHTFUNC: {
		/* Lightfunc coerces to a Function instance with concrete
		 * properties.  Since 'length' is virtual for Duktape/C
		 * functions, don't need to define that.
		 *
		 * The result is made extensible to mimic what happens to
		 * strings:
		 *   > Object.isExtensible(Object('foo'))
		 *   true
		 */
		duk_small_uint_t lf_flags;
		duk_small_uint_t nargs;
		duk_small_uint_t lf_len;
		duk_c_function func;
		duk_hnativefunction *nf;

		DUK_TVAL_GET_LIGHTFUNC(tv, func, lf_flags);

		nargs = DUK_LFUNC_FLAGS_GET_NARGS(lf_flags);
		if (nargs == DUK_LFUNC_NARGS_VARARGS) {
			nargs = DUK_VARARGS;
		}
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		        DUK_HOBJECT_FLAG_CONSTRUCTABLE |
		        DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	                DUK_HOBJECT_FLAG_NEWENV |
	                DUK_HOBJECT_FLAG_STRICT |
	                DUK_HOBJECT_FLAG_NOTAIL |
			/* DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC: omitted here intentionally */
	                DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);
		(void) duk__push_c_function_raw(ctx, func, (duk_idx_t) nargs, flags);

		lf_len = DUK_LFUNC_FLAGS_GET_LENGTH(lf_flags);
		if (lf_len != nargs) {
			/* Explicit length is only needed if it differs from 'nargs'. */
			duk_push_int(ctx, (duk_int_t) lf_len);
			duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_NONE);
		}
		duk_push_lightfunc_name(ctx, tv);
		duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_NONE);

		nf = duk_get_hnativefunction(ctx, -1);
		DUK_ASSERT(nf != NULL);
		nf->magic = (duk_int16_t) DUK_LFUNC_FLAGS_GET_MAGIC(lf_flags);

		/* Enable DUKFUNC exotic behavior once properties are set up. */
		DUK_HOBJECT_SET_EXOTIC_DUKFUNC((duk_hobject *) nf);
		goto replace_value;
	}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default: {
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_NUMBER);
		proto = DUK_BIDX_NUMBER_PROTOTYPE;
		goto create_object;
	}
	}
	return;

 create_object:
	(void) duk_push_object_helper(ctx, flags, proto);

	/* Note: Boolean prototype's internal value property is not writable,
	 * but duk_xdef_prop_stridx() disregards the write protection.  Boolean
	 * instances are immutable.
	 *
	 * String and buffer special behaviors are already enabled which is not
	 * ideal, but a write to the internal value is not affected by them.
	 */
	duk_dup(ctx, index);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);

 replace_value:
	duk_replace(ctx, index);
}

/*
 *  Type checking
 */

DUK_LOCAL duk_bool_t duk__tag_check(duk_context *ctx, duk_idx_t index, duk_small_uint_t tag) {
	duk_tval *tv;

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return 0;
	}
	return (DUK_TVAL_GET_TAG(tv) == tag);
}

DUK_LOCAL duk_bool_t duk__obj_flag_any_default_false(duk_context *ctx, duk_idx_t index, duk_uint_t flag_mask) {
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_get_hobject(ctx, index);
	if (obj) {
		return (DUK_HEAPHDR_CHECK_FLAG_BITS((duk_heaphdr *) obj, flag_mask) ? 1 : 0);
	}
	return 0;
}

DUK_EXTERNAL duk_int_t duk_get_type(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return DUK_TYPE_NONE;
	}
	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
		return DUK_TYPE_UNDEFINED;
	case DUK_TAG_NULL:
		return DUK_TYPE_NULL;
	case DUK_TAG_BOOLEAN:
		return DUK_TYPE_BOOLEAN;
	case DUK_TAG_STRING:
		return DUK_TYPE_STRING;
	case DUK_TAG_OBJECT:
		return DUK_TYPE_OBJECT;
	case DUK_TAG_BUFFER:
		return DUK_TYPE_BUFFER;
	case DUK_TAG_POINTER:
		return DUK_TYPE_POINTER;
	case DUK_TAG_LIGHTFUNC:
		return DUK_TYPE_LIGHTFUNC;
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default:
		/* Note: number has no explicit tag (in 8-byte representation) */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return DUK_TYPE_NUMBER;
	}
	DUK_UNREACHABLE();
}

DUK_EXTERNAL duk_bool_t duk_check_type(duk_context *ctx, duk_idx_t index, duk_int_t type) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_get_type(ctx, index) == type) ? 1 : 0;
}

DUK_EXTERNAL duk_uint_t duk_get_type_mask(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return DUK_TYPE_MASK_NONE;
	}
	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
		return DUK_TYPE_MASK_UNDEFINED;
	case DUK_TAG_NULL:
		return DUK_TYPE_MASK_NULL;
	case DUK_TAG_BOOLEAN:
		return DUK_TYPE_MASK_BOOLEAN;
	case DUK_TAG_STRING:
		return DUK_TYPE_MASK_STRING;
	case DUK_TAG_OBJECT:
		return DUK_TYPE_MASK_OBJECT;
	case DUK_TAG_BUFFER:
		return DUK_TYPE_MASK_BUFFER;
	case DUK_TAG_POINTER:
		return DUK_TYPE_MASK_POINTER;
	case DUK_TAG_LIGHTFUNC:
		return DUK_TYPE_MASK_LIGHTFUNC;
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default:
		/* Note: number has no explicit tag (in 8-byte representation) */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return DUK_TYPE_MASK_NUMBER;
	}
	DUK_UNREACHABLE();
}

DUK_EXTERNAL duk_bool_t duk_check_type_mask(duk_context *ctx, duk_idx_t index, duk_uint_t mask) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	if (duk_get_type_mask(ctx, index) & mask) {
		return 1;
	}
	if (mask & DUK_TYPE_MASK_THROW) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_UNEXPECTED_TYPE);
		DUK_UNREACHABLE();
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_undefined(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_UNDEFINED);
}

DUK_EXTERNAL duk_bool_t duk_is_null(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_NULL);
}

DUK_EXTERNAL duk_bool_t duk_is_null_or_undefined(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;
	duk_small_uint_t tag;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return 0;
	}
	tag = DUK_TVAL_GET_TAG(tv);
	return (tag == DUK_TAG_UNDEFINED) || (tag == DUK_TAG_NULL);
}

DUK_EXTERNAL duk_bool_t duk_is_boolean(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_BOOLEAN);
}

DUK_EXTERNAL duk_bool_t duk_is_number(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	/*
	 *  Number is special because it doesn't have a specific
	 *  tag in the 8-byte representation.
	 */

	/* XXX: shorter version for 12-byte representation? */

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return 0;
	}
	return DUK_TVAL_IS_NUMBER(tv);
}

DUK_EXTERNAL duk_bool_t duk_is_nan(duk_context *ctx, duk_idx_t index) {
	/* XXX: This will now return false for non-numbers, even though they would
	 * coerce to NaN (as a general rule).  In particular, duk_get_number()
	 * returns a NaN for non-numbers, so should this function also return
	 * true for non-numbers?
	 */

	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (!tv || !DUK_TVAL_IS_NUMBER(tv)) {
		return 0;
	}
	return DUK_ISNAN(DUK_TVAL_GET_NUMBER(tv));
}

DUK_EXTERNAL duk_bool_t duk_is_string(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_STRING);
}

DUK_EXTERNAL duk_bool_t duk_is_object(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_OBJECT);
}

DUK_EXTERNAL duk_bool_t duk_is_buffer(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_BUFFER);
}

DUK_EXTERNAL duk_bool_t duk_is_pointer(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_POINTER);
}

DUK_EXTERNAL duk_bool_t duk_is_lightfunc(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, index, DUK_TAG_LIGHTFUNC);
}

DUK_EXTERNAL duk_bool_t duk_is_array(duk_context *ctx, duk_idx_t index) {
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_get_hobject(ctx, index);
	if (obj) {
		return (DUK_HOBJECT_GET_CLASS_NUMBER(obj) == DUK_HOBJECT_CLASS_ARRAY ? 1 : 0);
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_function(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_LIGHTFUNC(tv)) {
		return 1;
	}
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_COMPILEDFUNCTION |
	                                       DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	                                       DUK_HOBJECT_FLAG_BOUND);
}

DUK_EXTERNAL duk_bool_t duk_is_c_function(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_NATIVEFUNCTION);
}

DUK_EXTERNAL duk_bool_t duk_is_ecmascript_function(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_COMPILEDFUNCTION);
}

DUK_EXTERNAL duk_bool_t duk_is_bound_function(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_BOUND);
}

DUK_EXTERNAL duk_bool_t duk_is_thread(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_THREAD);
}

DUK_EXTERNAL duk_bool_t duk_is_callable(duk_context *ctx, duk_idx_t index) {
	/* XXX: currently same as duk_is_function() */
	DUK_ASSERT_CTX_VALID(ctx);
	return duk_is_function(ctx, index);
}

DUK_EXTERNAL duk_bool_t duk_is_dynamic_buffer(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_HAS_DYNAMIC(h) ? 1 : 0);
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_fixed_buffer(duk_context *ctx, duk_idx_t index) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval(ctx, index);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_HAS_DYNAMIC(h) ? 0 : 1);
	}
	return 0;
}

/* XXX: make macro in API */
DUK_EXTERNAL duk_bool_t duk_is_primitive(duk_context *ctx, duk_idx_t index) {
	DUK_ASSERT_CTX_VALID(ctx);

	return !duk_is_object(ctx, index);
}

DUK_EXTERNAL duk_errcode_t duk_get_error_code(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_uint_t sanity;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hobject(ctx, index);

	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	do {
		if (!h) {
			return DUK_ERR_NONE;
		}
		if (h == thr->builtins[DUK_BIDX_EVAL_ERROR_PROTOTYPE]) {
			return DUK_ERR_EVAL_ERROR;
		}
		if (h == thr->builtins[DUK_BIDX_RANGE_ERROR_PROTOTYPE]) {
			return DUK_ERR_RANGE_ERROR;
		}
		if (h == thr->builtins[DUK_BIDX_REFERENCE_ERROR_PROTOTYPE]) {
			return DUK_ERR_REFERENCE_ERROR;
		}
		if (h == thr->builtins[DUK_BIDX_SYNTAX_ERROR_PROTOTYPE]) {
			return DUK_ERR_SYNTAX_ERROR;
		}
		if (h == thr->builtins[DUK_BIDX_TYPE_ERROR_PROTOTYPE]) {
			return DUK_ERR_TYPE_ERROR;
		}
		if (h == thr->builtins[DUK_BIDX_URI_ERROR_PROTOTYPE]) {
			return DUK_ERR_URI_ERROR;
		}
		if (h == thr->builtins[DUK_BIDX_ERROR_PROTOTYPE]) {
			return DUK_ERR_ERROR;
		}

		h = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h);
	} while (--sanity > 0);

	return DUK_ERR_NONE;
}

/*
 *  Pushers
 */

DUK_INTERNAL void duk_push_tval(duk_context *ctx, duk_tval *tv) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(tv != NULL);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_TVAL(tv_slot, tv);
	DUK_TVAL_INCREF(thr, tv);  /* no side effects */
}

#if defined(DUK_USE_DEBUGGER_SUPPORT)
/* Right now only needed by the debugger. */
DUK_INTERNAL void duk_push_unused(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_UNDEFINED_UNUSED(tv_slot);
}
#endif

DUK_EXTERNAL void duk_push_undefined(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_UNDEFINED_ACTUAL(tv_slot);
}

DUK_EXTERNAL void duk_push_null(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_NULL(tv_slot);
}

DUK_EXTERNAL void duk_push_boolean(duk_context *ctx, duk_bool_t val) {
	duk_hthread *thr;
	duk_tval *tv_slot;
	duk_small_int_t b;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	b = (val ? 1 : 0);  /* ensure value is 1 or 0 (not other non-zero) */
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_BOOLEAN(tv_slot, b);
}

DUK_EXTERNAL void duk_push_true(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_BOOLEAN_TRUE(tv_slot);
}

DUK_EXTERNAL void duk_push_false(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_BOOLEAN_FALSE(tv_slot);
}

/* normalize NaN which may not match our canonical internal NaN */
DUK_EXTERNAL void duk_push_number(duk_context *ctx, duk_double_t val) {
	duk_hthread *thr;
	duk_tval *tv_slot;
	duk_double_union du;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	du.d = val;
	DUK_DBLUNION_NORMALIZE_NAN_CHECK(&du);
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_NUMBER(tv_slot, du.d);
}

DUK_EXTERNAL void duk_push_int(duk_context *ctx, duk_int_t val) {
#if defined(DUK_USE_FASTINT)
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
#if DUK_INT_MAX <= 0x7fffffffL
	DUK_TVAL_SET_FASTINT_I32(tv_slot, (duk_int32_t) val);
#else
	if (val >= DUK_FASTINT_MIN && val <= DUK_FASTINT_MAX) {
		DUK_TVAL_SET_FASTINT(tv_slot, (duk_int64_t) val);
	} else {
		duk_double_t = (duk_double_t) val;
		DUK_TVAL_SET_NUMBER(tv_slot, d);
	}
#endif
#else  /* DUK_USE_FASTINT */
	duk_hthread *thr;
	duk_tval *tv_slot;
	duk_double_t d;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	d = (duk_double_t) val;
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_NUMBER(tv_slot, d);
#endif  /* DUK_USE_FASTINT */
}

DUK_EXTERNAL void duk_push_uint(duk_context *ctx, duk_uint_t val) {
#if defined(DUK_USE_FASTINT)
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
#if DUK_UINT_MAX <= 0xffffffffUL
	DUK_TVAL_SET_FASTINT_U32(tv_slot, (duk_uint32_t) val);
#else
	if (val <= DUK_FASTINT_MAX) {  /* val is unsigned so >= 0 */
		/* XXX: take advantage of val being unsigned, no need to mask */
		DUK_TVAL_SET_FASTINT(tv_slot, (duk_int64_t) val);
	} else {
		duk_double_t = (duk_double_t) val;
		DUK_TVAL_SET_NUMBER(tv_slot, d);
	}
#endif
#else  /* DUK_USE_FASTINT */
	duk_hthread *thr;
	duk_tval *tv_slot;
	duk_double_t d;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	d = (duk_double_t) val;
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_NUMBER(tv_slot, d);
#endif  /* DUK_USE_FASTINT */
}

DUK_EXTERNAL void duk_push_nan(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_slot;
	duk_double_union du;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	DUK_DBLUNION_SET_NAN(&du);
	DUK_ASSERT(DUK_DBLUNION_IS_NORMALIZED(&du));
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_NUMBER(tv_slot, du.d);
}

DUK_EXTERNAL const char *duk_push_lstring(duk_context *ctx, const char *str, duk_size_t len) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack before interning (avoid hanging temp) */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}

	/* NULL with zero length represents an empty string; NULL with higher
	 * length is also now trated like an empty string although it is
	 * a bit dubious.  This is unlike duk_push_string() which pushes a
	 * 'null' if the input string is a NULL.
	 */
	if (!str) {
		len = 0;
	}

	/* Check for maximum string length */
	if (len > DUK_HSTRING_MAX_BYTELEN) {
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_STRING_TOO_LONG);
	}

	h = duk_heap_string_intern_checked(thr, (duk_uint8_t *) str, (duk_uint32_t) len);
	DUK_ASSERT(h != NULL);

	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_STRING(tv_slot, h);
	DUK_HSTRING_INCREF(thr, h);  /* no side effects */

	return (const char *) DUK_HSTRING_GET_DATA(h);
}

DUK_EXTERNAL const char *duk_push_string(duk_context *ctx, const char *str) {
	DUK_ASSERT_CTX_VALID(ctx);

	if (str) {
		return duk_push_lstring(ctx, str, DUK_STRLEN(str));
	} else {
		duk_push_null(ctx);
		return NULL;
	}
}

#ifdef DUK_USE_FILE_IO
/* This is a bit clunky because it is ANSI C portable.  Should perhaps
 * relocate to another file because this is potentially platform
 * dependent.
 */
DUK_EXTERNAL const char *duk_push_string_file_raw(duk_context *ctx, const char *path, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_file *f = NULL;
	char *buf;
	long sz;  /* ANSI C typing */

	DUK_ASSERT_CTX_VALID(ctx);

	if (!path) {
		goto fail;
	}
	f = DUK_FOPEN(path, "rb");
	if (!f) {
		goto fail;
	}
	if (DUK_FSEEK(f, 0, SEEK_END) < 0) {
		goto fail;
	}
	sz = DUK_FTELL(f);
	if (sz < 0) {
		goto fail;
	}
	if (DUK_FSEEK(f, 0, SEEK_SET) < 0) {
		goto fail;
	}
	buf = (char *) duk_push_fixed_buffer(ctx, (duk_size_t) sz);
	DUK_ASSERT(buf != NULL);
	if ((duk_size_t) DUK_FREAD(buf, 1, (size_t) sz, f) != (duk_size_t) sz) {
		goto fail;
	}
	(void) DUK_FCLOSE(f);  /* ignore fclose() error */
	f = NULL;
	return duk_to_string(ctx, -1);

 fail:
	if (f) {
		DUK_FCLOSE(f);
	}

	if (flags != 0) {
		DUK_ASSERT(flags == DUK_STRING_PUSH_SAFE);  /* only flag now */
		duk_push_undefined(ctx);
	} else {
		/* XXX: string not shared because it is conditional */
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "read file error");
	}
	return NULL;
}
#else
DUK_EXTERNAL const char *duk_push_string_file_raw(duk_context *ctx, const char *path, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(path);

	if (flags != 0) {
		DUK_ASSERT(flags == DUK_STRING_PUSH_SAFE);  /* only flag now */
		duk_push_undefined(ctx);
	} else {
		/* XXX: string not shared because it is conditional */
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "file I/O disabled");
	}
	return NULL;
}
#endif  /* DUK_USE_FILE_IO */

DUK_EXTERNAL void duk_push_pointer(duk_context *ctx, void *val) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_POINTER(tv_slot, val);
}

#define DUK__PUSH_THIS_FLAG_CHECK_COERC  (1 << 0)
#define DUK__PUSH_THIS_FLAG_TO_OBJECT    (1 << 1)
#define DUK__PUSH_THIS_FLAG_TO_STRING    (1 << 2)

DUK_LOCAL void duk__push_this_helper(duk_context *ctx, duk_small_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);  /* avoid warning (unsigned) */
	DUK_ASSERT(thr->callstack_top <= thr->callstack_size);

	if (thr->callstack_top == 0) {
		if (flags & DUK__PUSH_THIS_FLAG_CHECK_COERC) {
			goto type_error;
		}
		duk_push_undefined(ctx);
	} else {
		duk_tval tv_tmp;
		duk_tval *tv;

		/* 'this' binding is just before current activation's bottom */
		DUK_ASSERT(thr->valstack_bottom > thr->valstack);
		tv = thr->valstack_bottom - 1;
		if (flags & DUK__PUSH_THIS_FLAG_CHECK_COERC) {
			if (DUK_TVAL_IS_UNDEFINED(tv) || DUK_TVAL_IS_NULL(tv)) {
				goto type_error;
			}
		}

		DUK_TVAL_SET_TVAL(&tv_tmp, tv);
		duk_push_tval(ctx, &tv_tmp);
	}

	if (flags & DUK__PUSH_THIS_FLAG_TO_OBJECT) {
		duk_to_object(ctx, -1);
	} else if (flags & DUK__PUSH_THIS_FLAG_TO_STRING) {
		duk_to_string(ctx, -1);
	}

	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_NOT_OBJECT_COERCIBLE);
}

DUK_EXTERNAL void duk_push_this(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, 0 /*flags*/);
}

DUK_INTERNAL void duk_push_this_check_object_coercible(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, DUK__PUSH_THIS_FLAG_CHECK_COERC /*flags*/);
}

DUK_INTERNAL duk_hobject *duk_push_this_coercible_to_object(duk_context *ctx) {
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, DUK__PUSH_THIS_FLAG_CHECK_COERC |
	                           DUK__PUSH_THIS_FLAG_TO_OBJECT /*flags*/);
	h = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h != NULL);
	return h;
}

DUK_INTERNAL duk_hstring *duk_push_this_coercible_to_string(duk_context *ctx) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, DUK__PUSH_THIS_FLAG_CHECK_COERC |
	                           DUK__PUSH_THIS_FLAG_TO_STRING /*flags*/);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);
	return h;
}

DUK_EXTERNAL void duk_push_current_function(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);
	DUK_ASSERT(thr->callstack_top <= thr->callstack_size);

	act = duk_hthread_get_current_activation(thr);
	if (act) {
		duk_push_tval(ctx, &act->tv_func);
	} else {
		duk_push_undefined(ctx);
	}
}

DUK_EXTERNAL void duk_push_current_thread(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);

	if (thr->heap->curr_thread) {
		duk_push_hobject(ctx, (duk_hobject *) thr->heap->curr_thread);
	} else {
		duk_push_undefined(ctx);
	}
}

DUK_EXTERNAL void duk_push_global_object(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_push_hobject_bidx(ctx, DUK_BIDX_GLOBAL);
}

/* XXX: size optimize */
DUK_LOCAL void duk__push_stash(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	if (!duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE)) {
		DUK_DDD(DUK_DDDPRINT("creating heap/global/thread stash on first use"));
		duk_pop(ctx);
		duk_push_object_internal(ctx);
		duk_dup_top(ctx);
		duk_xdef_prop_stridx(ctx, -3, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_C);  /* [ ... parent stash stash ] -> [ ... parent stash ] */
	}
	duk_remove(ctx, -2);
}

DUK_EXTERNAL void duk_push_heap_stash(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;
	DUK_ASSERT_CTX_VALID(ctx);
	heap = thr->heap;
	DUK_ASSERT(heap->heap_object != NULL);
	duk_push_hobject(ctx, heap->heap_object);
	duk__push_stash(ctx);
}

DUK_EXTERNAL void duk_push_global_stash(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_push_global_object(ctx);
	duk__push_stash(ctx);
}

DUK_EXTERNAL void duk_push_thread_stash(duk_context *ctx, duk_context *target_ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT_CTX_VALID(ctx);
	if (!target_ctx) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CALL_ARGS);
		return;  /* not reached */
	}
	duk_push_hobject(ctx, (duk_hobject *) target_ctx);
	duk__push_stash(ctx);
}

/* XXX: duk_ssize_t would be useful here */
DUK_LOCAL duk_int_t duk__try_push_vsprintf(duk_context *ctx, void *buf, duk_size_t sz, const char *fmt, va_list ap) {
	duk_int_t len;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(ctx);

	/* NUL terminator handling doesn't matter here */
	len = DUK_VSNPRINTF((char *) buf, sz, fmt, ap);
	if (len < (duk_int_t) sz) {
		/* Return value of 'sz' or more indicates output was (potentially)
		 * truncated.
		 */
		return (duk_int_t) len;
	}
	return -1;
}

DUK_EXTERNAL const char *duk_push_vsprintf(duk_context *ctx, const char *fmt, va_list ap) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uint8_t stack_buf[DUK_PUSH_SPRINTF_INITIAL_SIZE];
	duk_size_t sz = DUK_PUSH_SPRINTF_INITIAL_SIZE;
	duk_bool_t pushed_buf = 0;
	void *buf;
	duk_int_t len;  /* XXX: duk_ssize_t */
	const char *res;

	DUK_ASSERT_CTX_VALID(ctx);

	/* special handling of fmt==NULL */
	if (!fmt) {
		duk_hstring *h_str;
		duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);
		h_str = DUK_HTHREAD_STRING_EMPTY_STRING(thr);  /* rely on interning, must be this string */
		return (const char *) DUK_HSTRING_GET_DATA(h_str);
	}

	/* initial estimate based on format string */
	sz = DUK_STRLEN(fmt) + 16;  /* format plus something to avoid just missing */
	if (sz < DUK_PUSH_SPRINTF_INITIAL_SIZE) {
		sz = DUK_PUSH_SPRINTF_INITIAL_SIZE;
	}
	DUK_ASSERT(sz > 0);

	/* Try to make do with a stack buffer to avoid allocating a temporary buffer.
	 * This works 99% of the time which is quite nice.
	 */
	for (;;) {
		va_list ap_copy;  /* copied so that 'ap' can be reused */

		if (sz <= sizeof(stack_buf)) {
			buf = stack_buf;
		} else if (!pushed_buf) {
			pushed_buf = 1;
			buf = duk_push_dynamic_buffer(ctx, sz);
		} else {
			buf = duk_resize_buffer(ctx, -1, sz);
		}
		DUK_ASSERT(buf != NULL);

		DUK_VA_COPY(ap_copy, ap);
		len = duk__try_push_vsprintf(ctx, buf, sz, fmt, ap_copy);
		va_end(ap_copy);
		if (len >= 0) {
			break;
		}

		/* failed, resize and try again */
		sz = sz * 2;
		if (sz >= DUK_PUSH_SPRINTF_SANITY_LIMIT) {
			DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_SPRINTF_TOO_LONG);
		}
	}

	/* Cannot use duk_to_string() on the buffer because it is usually
	 * larger than 'len'.  Also, 'buf' is usually a stack buffer.
	 */
	res = duk_push_lstring(ctx, (const char *) buf, (duk_size_t) len);  /* [ buf? res ] */
	if (pushed_buf) {
		duk_remove(ctx, -2);
	}
	return res;
}

DUK_EXTERNAL const char *duk_push_sprintf(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	const char *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	/* allow fmt==NULL */
	va_start(ap, fmt);
	ret = duk_push_vsprintf(ctx, fmt, ap);
	va_end(ap);

	return ret;
}

DUK_INTERNAL duk_idx_t duk_push_object_helper(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_small_int_t prototype_bidx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_slot;
	duk_hobject *h;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(prototype_bidx == -1 ||
	           (prototype_bidx >= 0 && prototype_bidx < DUK_NUM_BUILTINS));

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}

	h = duk_hobject_alloc(thr->heap, hobject_flags_and_class);
	if (!h) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_OBJECT_ALLOC_FAILED);
	}

	DUK_DDD(DUK_DDDPRINT("created object with flags: 0x%08lx", (unsigned long) h->hdr.h_flags));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, h);
	DUK_HOBJECT_INCREF(thr, h);  /* no side effects */
	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* object is now reachable */

	if (prototype_bidx >= 0) {
		DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, thr->builtins[prototype_bidx]);
	} else {
		DUK_ASSERT(prototype_bidx == -1);
		DUK_ASSERT(DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h) == NULL);
	}

	return ret;
}

DUK_INTERNAL duk_idx_t duk_push_object_helper_proto(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_hobject *proto) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = duk_push_object_helper(ctx, hobject_flags_and_class, -1);
	h = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h) == NULL);
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, proto);
	return ret;
}

DUK_EXTERNAL duk_idx_t duk_push_object(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              DUK_BIDX_OBJECT_PROTOTYPE);
}

DUK_EXTERNAL duk_idx_t duk_push_array(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = duk_push_object_helper(ctx,
	                             DUK_HOBJECT_FLAG_EXTENSIBLE |
	                             DUK_HOBJECT_FLAG_ARRAY_PART |
	                             DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ARRAY),
	                             DUK_BIDX_ARRAY_PROTOTYPE);

	obj = duk_require_hobject(ctx, ret);

	/*
	 *  An array must have a 'length' property (E5 Section 15.4.5.2).
	 *  The special array behavior flag must only be enabled once the
	 *  length property has been added.
	 *
	 *  The internal property must be a number (and preferably a
	 *  fastint if fastint support is enabled).
	 */

	duk_push_int(ctx, 0);
#if defined(DUK_USE_FASTINT)
	DUK_ASSERT(DUK_TVAL_IS_FASTINT(duk_require_tval(ctx, -1)));
#endif

	duk_hobject_define_property_internal(thr,
	                                     obj,
	                                     DUK_HTHREAD_STRING_LENGTH(thr),
	                                     DUK_PROPDESC_FLAGS_W);
	DUK_HOBJECT_SET_EXOTIC_ARRAY(obj);

	return ret;
}

DUK_EXTERNAL duk_idx_t duk_push_thread_raw(duk_context *ctx, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hthread *obj;
	duk_idx_t ret;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}

	obj = duk_hthread_alloc(thr->heap,
	                        DUK_HOBJECT_FLAG_EXTENSIBLE |
	                        DUK_HOBJECT_FLAG_THREAD |
	                        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_THREAD));
	if (!obj) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_THREAD_ALLOC_FAILED);
	}
	obj->state = DUK_HTHREAD_STATE_INACTIVE;
#if defined(DUK_USE_HEAPPTR16)
	obj->strs16 = thr->strs16;
#else
	obj->strs = thr->strs;
#endif
	DUK_DDD(DUK_DDDPRINT("created thread object with flags: 0x%08lx", (unsigned long) obj->obj.hdr.h_flags));

	/* make the new thread reachable */
	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HTHREAD_INCREF(thr, obj);
	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* important to do this *after* pushing, to make the thread reachable for gc */
	if (!duk_hthread_init_stacks(thr->heap, obj)) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_THREAD_ALLOC_FAILED);
	}

	/* initialize built-ins - either by copying or creating new ones */
	if (flags & DUK_THREAD_NEW_GLOBAL_ENV) {
		duk_hthread_create_builtin_objects(obj);
	} else {
		duk_hthread_copy_builtin_objects(thr, obj);
	}

	/* default prototype (Note: 'obj' must be reachable) */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, obj->builtins[DUK_BIDX_THREAD_PROTOTYPE]);

	/* Initial stack size satisfies the stack spare constraints so there
	 * is no need to require stack here.
	 */
	DUK_ASSERT(DUK_VALSTACK_INITIAL_SIZE >=
	           DUK_VALSTACK_API_ENTRY_MINIMUM + DUK_VALSTACK_INTERNAL_EXTRA);

	return ret;
}

DUK_INTERNAL duk_idx_t duk_push_compiledfunction(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hcompiledfunction *obj;
	duk_idx_t ret;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}

	/* Template functions are not strictly constructable (they don't
	 * have a "prototype" property for instance), so leave the
	 * DUK_HOBJECT_FLAG_CONSRUCTABLE flag cleared here.
	 */

	obj = duk_hcompiledfunction_alloc(thr->heap,
	                                  DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                  DUK_HOBJECT_FLAG_COMPILEDFUNCTION |
	                                  DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION));
	if (!obj) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_FUNC_ALLOC_FAILED);
	}

	DUK_DDD(DUK_DDDPRINT("created compiled function object with flags: 0x%08lx", (unsigned long) obj->obj.hdr.h_flags));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HOBJECT_INCREF(thr, obj);
	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* default prototype (Note: 'obj' must be reachable) */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE]);

	return ret;
}

DUK_LOCAL duk_idx_t duk__push_c_function_raw(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hnativefunction *obj;
	duk_idx_t ret;
	duk_tval *tv_slot;
	duk_uint16_t func_nargs;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}
	if (func == NULL) {
		goto api_error;
	}
	if (nargs >= 0 && nargs < DUK_HNATIVEFUNCTION_NARGS_MAX) {
		func_nargs = (duk_uint16_t) nargs;
	} else if (nargs == DUK_VARARGS) {
		func_nargs = DUK_HNATIVEFUNCTION_NARGS_VARARGS;
	} else {
		goto api_error;
	}

	obj = duk_hnativefunction_alloc(thr->heap, flags);
	if (!obj) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_FUNC_ALLOC_FAILED);
	}

	obj->func = func;
	obj->nargs = func_nargs;

	DUK_DDD(DUK_DDDPRINT("created native function object with flags: 0x%08lx, nargs=%ld",
	                     (unsigned long) obj->obj.hdr.h_flags, (long) obj->nargs));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HOBJECT_INCREF(thr, obj);
	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* default prototype (Note: 'obj' must be reachable) */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE]);

	return ret;

 api_error:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CALL_ARGS);
	return 0;  /* not reached */
}

DUK_EXTERNAL duk_idx_t duk_push_c_function(duk_context *ctx, duk_c_function func, duk_int_t nargs) {
	duk_uint_t flags;

	DUK_ASSERT_CTX_VALID(ctx);

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	        DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	        DUK_HOBJECT_FLAG_NEWENV |
	        DUK_HOBJECT_FLAG_STRICT |
	        DUK_HOBJECT_FLAG_NOTAIL |
	        DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC |
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);

	return duk__push_c_function_raw(ctx, func, nargs, flags);
}

DUK_INTERNAL void duk_push_c_function_noexotic(duk_context *ctx, duk_c_function func, duk_int_t nargs) {
	duk_uint_t flags;

	DUK_ASSERT_CTX_VALID(ctx);

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	        DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	        DUK_HOBJECT_FLAG_NEWENV |
	        DUK_HOBJECT_FLAG_STRICT |
	        DUK_HOBJECT_FLAG_NOTAIL |
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);

	(void) duk__push_c_function_raw(ctx, func, nargs, flags);
}

DUK_INTERNAL void duk_push_c_function_noconstruct_noexotic(duk_context *ctx, duk_c_function func, duk_int_t nargs) {
	duk_uint_t flags;

	DUK_ASSERT_CTX_VALID(ctx);

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	        DUK_HOBJECT_FLAG_NEWENV |
	        DUK_HOBJECT_FLAG_STRICT |
	        DUK_HOBJECT_FLAG_NOTAIL |
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);

	(void) duk__push_c_function_raw(ctx, func, nargs, flags);
}

DUK_EXTERNAL duk_idx_t duk_push_c_lightfunc(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_idx_t length, duk_int_t magic) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval tv_tmp;
	duk_small_uint_t lf_flags;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}

	if (nargs >= DUK_LFUNC_NARGS_MIN && nargs <= DUK_LFUNC_NARGS_MAX) {
		/* as is */
	} else if (nargs == DUK_VARARGS) {
		nargs = DUK_LFUNC_NARGS_VARARGS;
	} else {
		goto api_error;
	}
	if (!(length >= DUK_LFUNC_LENGTH_MIN && length <= DUK_LFUNC_LENGTH_MAX)) {
		goto api_error;
	}
	if (!(magic >= DUK_LFUNC_MAGIC_MIN && magic <= DUK_LFUNC_MAGIC_MAX)) {
		goto api_error;
	}

	lf_flags = DUK_LFUNC_FLAGS_PACK(magic, length, nargs);
	DUK_TVAL_SET_LIGHTFUNC(&tv_tmp, func, lf_flags);
	duk_push_tval(ctx, &tv_tmp);  /* XXX: direct valstack write */
	DUK_ASSERT(thr->valstack_top != thr->valstack_bottom);
	return ((duk_idx_t) (thr->valstack_top - thr->valstack_bottom)) - 1;

 api_error:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CALL_ARGS);
	return 0;  /* not reached */
}

DUK_EXTERNAL duk_idx_t duk_push_error_object_va_raw(duk_context *ctx, duk_errcode_t err_code, const char *filename, duk_int_t line, const char *fmt, va_list ap) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;
	duk_hobject *proto;
#ifdef DUK_USE_AUGMENT_ERROR_CREATE
	duk_bool_t noblame_fileline;
#endif

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_UNREF(filename);
	DUK_UNREF(line);

	/* Error code also packs a tracedata related flag. */
#ifdef DUK_USE_AUGMENT_ERROR_CREATE
	noblame_fileline = err_code & DUK_ERRCODE_FLAG_NOBLAME_FILELINE;
#endif
	err_code = err_code & (~DUK_ERRCODE_FLAG_NOBLAME_FILELINE);

	/* error gets its 'name' from the prototype */
	proto = duk_error_prototype_from_code(thr, err_code);
	ret = duk_push_object_helper_proto(ctx,
	                                   DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                   DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ERROR),
	                                   proto);

	/* ... and its 'message' from an instance property */
	if (fmt) {
		duk_push_vsprintf(ctx, fmt, ap);
		duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	} else {
		/* If no explicit message given, put error code into message field
		 * (as a number).  This is not fully in keeping with the Ecmascript
		 * error model because messages are supposed to be strings (Error
		 * constructors use ToString() on their argument).  However, it's
		 * probably more useful than having a separate 'code' property.
		 */
		duk_push_int(ctx, err_code);
		duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	}

#if 0
	/* Disabled for now, not sure this is a useful property */
	duk_push_int(ctx, err_code);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_CODE, DUK_PROPDESC_FLAGS_WC);
#endif

	/* Creation time error augmentation */
#ifdef DUK_USE_AUGMENT_ERROR_CREATE
	/* filename may be NULL in which case file/line is not recorded */
	duk_err_augment_error_create(thr, thr, filename, line, noblame_fileline);  /* may throw an error */
#endif

	return ret;
}

DUK_EXTERNAL duk_idx_t duk_push_error_object_raw(duk_context *ctx, duk_errcode_t err_code, const char *filename, duk_int_t line, const char *fmt, ...) {
	va_list ap;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	va_start(ap, fmt);
	ret = duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	return ret;
}

#if !defined(DUK_USE_VARIADIC_MACROS)
DUK_EXTERNAL duk_idx_t duk_push_error_object_stash(duk_context *ctx, duk_errcode_t err_code, const char *fmt, ...) {
	const char *filename = duk_api_global_filename;
	duk_int_t line = duk_api_global_line;
	va_list ap;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	duk_api_global_filename = NULL;
	duk_api_global_line = 0;
	va_start(ap, fmt);
	ret = duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	return ret;
}
#endif  /* DUK_USE_VARIADIC_MACROS */

DUK_EXTERNAL void *duk_push_buffer_raw(duk_context *ctx, duk_size_t size, duk_small_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_slot;
	duk_hbuffer *h;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_PUSH_BEYOND_ALLOC_STACK);
	}

	/* Check for maximum buffer length. */
	if (size > DUK_HBUFFER_MAX_BYTELEN) {
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, DUK_STR_BUFFER_TOO_LONG);
	}

	h = duk_hbuffer_alloc(thr->heap, size, flags);
	if (!h) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, DUK_STR_BUFFER_ALLOC_FAILED);
	}

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_BUFFER(tv_slot, h);
	DUK_HBUFFER_INCREF(thr, h);
	thr->valstack_top++;

	return DUK_HBUFFER_GET_DATA_PTR(thr->heap, h);
}

DUK_EXTERNAL duk_idx_t duk_push_heapptr(duk_context *ctx, void *ptr) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);

	if (ptr == NULL) {
		goto push_undefined;
	}

	switch (DUK_HEAPHDR_GET_TYPE((duk_heaphdr *) ptr)) {
	case DUK_HTYPE_STRING:
		duk_push_hstring(ctx, (duk_hstring *) ptr);
		break;
	case DUK_HTYPE_OBJECT:
		duk_push_hobject(ctx, (duk_hobject *) ptr);
		break;
	case DUK_HTYPE_BUFFER:
		duk_push_hbuffer(ctx, (duk_hbuffer *) ptr);
		break;
	default:
		goto push_undefined;
	}
	return ret;

 push_undefined:
	duk_push_undefined(ctx);
	return ret;
}

DUK_INTERNAL duk_idx_t duk_push_object_internal(duk_context *ctx) {
	return duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              -1);  /* no prototype */
}

DUK_INTERNAL void duk_push_hstring(duk_context *ctx, duk_hstring *h) {
	duk_tval tv;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(h != NULL);
	DUK_TVAL_SET_STRING(&tv, h);
	duk_push_tval(ctx, &tv);
}

DUK_INTERNAL void duk_push_hstring_stridx(duk_context *ctx, duk_small_int_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
}

DUK_INTERNAL void duk_push_hobject(duk_context *ctx, duk_hobject *h) {
	duk_tval tv;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(h != NULL);
	DUK_TVAL_SET_OBJECT(&tv, h);
	duk_push_tval(ctx, &tv);
}

DUK_INTERNAL void duk_push_hbuffer(duk_context *ctx, duk_hbuffer *h) {
	duk_tval tv;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(h != NULL);
	DUK_TVAL_SET_BUFFER(&tv, h);
	duk_push_tval(ctx, &tv);
}

DUK_INTERNAL void duk_push_hobject_bidx(duk_context *ctx, duk_small_int_t builtin_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(builtin_idx >= 0 && builtin_idx < DUK_NUM_BUILTINS);
	DUK_ASSERT(thr->builtins[builtin_idx] != NULL);
	duk_push_hobject(ctx, thr->builtins[builtin_idx]);
}

/*
 *  Poppers
 */

DUK_EXTERNAL void duk_pop_n(duk_context *ctx, duk_idx_t count) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	if (count < 0) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_COUNT);
		return;
	}

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	if ((duk_size_t) (thr->valstack_top - thr->valstack_bottom) < (duk_size_t) count) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_POP_TOO_MANY);
	}

	/*
	 *  Must be very careful here, every DECREF may cause reallocation
	 *  of our valstack.
	 */

	/* XXX: inlined DECREF macro would be nice here: no NULL check,
	 * refzero queueing but no refzero algorithm run (= no pointer
	 * instability), inline code.
	 */

#ifdef DUK_USE_REFERENCE_COUNTING
	while (count > 0) {
		duk_tval tv_tmp;
		duk_tval *tv;

		tv = --thr->valstack_top;  /* tv points to element just below prev top */
		DUK_ASSERT(tv >= thr->valstack_bottom);
		DUK_TVAL_SET_TVAL(&tv_tmp, tv);
		DUK_TVAL_SET_UNDEFINED_UNUSED(tv);
		DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
		count--;
	}
#else
	while (count > 0) {
		duk_tval *tv;

		tv = --thr->valstack_top;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		DUK_TVAL_SET_UNDEFINED_UNUSED(tv);
		count--;
	}
#endif

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
}

DUK_EXTERNAL void duk_pop(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_pop_n(ctx, 1);
}

DUK_EXTERNAL void duk_pop_2(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_pop_n(ctx, 2);
}

DUK_EXTERNAL void duk_pop_3(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_pop_n(ctx, 3);
}

/*
 *  Error throwing
 */

DUK_EXTERNAL void duk_throw(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	if (thr->valstack_top == thr->valstack_bottom) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, DUK_STR_INVALID_CALL_ARGS);
	}

	/* Errors are augmented when they are created, not when they are
	 * thrown or re-thrown.  The current error handler, however, runs
	 * just before an error is thrown.
	 */

#if defined(DUK_USE_AUGMENT_ERROR_THROW)
	DUK_DDD(DUK_DDDPRINT("THROW ERROR (API): %!dT (before throw augment)", (duk_tval *) duk_get_tval(ctx, -1)));
	duk_err_augment_error_throw(thr);
#endif
	DUK_DDD(DUK_DDDPRINT("THROW ERROR (API): %!dT (after throw augment)", (duk_tval *) duk_get_tval(ctx, -1)));

	duk_err_setup_heap_ljstate(thr, DUK_LJ_TYPE_THROW);

	/* thr->heap->lj.jmpbuf_ptr is checked by duk_err_longjmp() so we don't
	 * need to check that here.  If the value is NULL, a panic occurs because
	 * we can't return.
	 */

	duk_err_longjmp(thr);
	DUK_UNREACHABLE();
}

DUK_EXTERNAL void duk_fatal(duk_context *ctx, duk_errcode_t err_code, const char *err_msg) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(thr->heap->fatal_func != NULL);

	DUK_D(DUK_DPRINT("fatal error occurred, code %ld, message %s",
	                 (long) err_code, (const char *) err_msg));

	/* fatal_func should be noreturn, but noreturn declarations on function
	 * pointers has a very spotty support apparently so it's not currently
	 * done.
	 */
	thr->heap->fatal_func(ctx, err_code, err_msg);

	DUK_PANIC(DUK_ERR_API_ERROR, "fatal handler returned");
}

DUK_EXTERNAL void duk_error_va_raw(duk_context *ctx, duk_errcode_t err_code, const char *filename, duk_int_t line, const char *fmt, va_list ap) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	duk_throw(ctx);
}

DUK_EXTERNAL void duk_error_raw(duk_context *ctx, duk_errcode_t err_code, const char *filename, duk_int_t line, const char *fmt, ...) {
	va_list ap;

	DUK_ASSERT_CTX_VALID(ctx);

	va_start(ap, fmt);
	duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	duk_throw(ctx);
}

#if !defined(DUK_USE_VARIADIC_MACROS)
DUK_EXTERNAL void duk_error_stash(duk_context *ctx, duk_errcode_t err_code, const char *fmt, ...) {
	const char *filename;
	duk_int_t line;
	va_list ap;

	DUK_ASSERT_CTX_VALID(ctx);

	filename = duk_api_global_filename;
	line = duk_api_global_line;
	duk_api_global_filename = NULL;
	duk_api_global_line = 0;

	va_start(ap, fmt);
	duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	duk_throw(ctx);
}
#endif  /* DUK_USE_VARIADIC_MACROS */

/*
 *  Comparison
 */

DUK_EXTERNAL duk_bool_t duk_equals(duk_context *ctx, duk_idx_t index1, duk_idx_t index2) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1, *tv2;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_get_tval(ctx, index1);
	tv2 = duk_get_tval(ctx, index2);
	if ((tv1 == NULL) || (tv2 == NULL)) {
		return 0;
	}

	/* Coercion may be needed, the helper handles that by pushing the
	 * tagged values to the stack.
	 */
	return duk_js_equals(thr, tv1, tv2);
}

DUK_EXTERNAL duk_bool_t duk_strict_equals(duk_context *ctx, duk_idx_t index1, duk_idx_t index2) {
	duk_tval *tv1, *tv2;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_get_tval(ctx, index1);
	tv2 = duk_get_tval(ctx, index2);
	if ((tv1 == NULL) || (tv2 == NULL)) {
		return 0;
	}

	/* No coercions or other side effects, so safe */
	return duk_js_strict_equals(tv1, tv2);
}

/*
 *  instanceof
 */

DUK_EXTERNAL duk_bool_t duk_instanceof(duk_context *ctx, duk_idx_t index1, duk_idx_t index2) {
	duk_tval *tv1, *tv2;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Index validation is strict, which differs from duk_equals().
	 * The strict behavior mimics how instanceof itself works, e.g.
	 * it is a TypeError if rval is not a -callable- object.  It would
	 * be somewhat inconsistent if rval would be allowed to be
	 * non-existent without a TypeError.
	 */
	tv1 = duk_require_tval(ctx, index1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, index2);
	DUK_ASSERT(tv2 != NULL);

	return duk_js_instanceof((duk_hthread *) ctx, tv1, tv2);
}

/*
 *  Lightfunc
 */

DUK_INTERNAL void duk_push_lightfunc_name(duk_context *ctx, duk_tval *tv) {
	duk_c_function func;

	DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv));

	/* Lightfunc name, includes Duktape/C native function pointer, which
	 * can often be used to locate the function from a symbol table.
	 * The name also includes the 16-bit duk_tval flags field because it
	 * includes the magic value.  Because a single native function often
	 * provides different functionality depending on the magic value, it
	 * seems reasonably to include it in the name.
	 *
	 * On the other hand, a complicated name increases string table
	 * pressure in low memory environments (but only when function name
	 * is accessed).
	 */

	func = DUK_TVAL_GET_LIGHTFUNC_FUNCPTR(tv);
	duk_push_sprintf(ctx, "light_");
	duk_push_string_funcptr(ctx, (duk_uint8_t *) &func, sizeof(func));
	duk_push_sprintf(ctx, "_%04x", (unsigned int) DUK_TVAL_GET_LIGHTFUNC_FLAGS(tv));
	duk_concat(ctx, 3);
}

DUK_INTERNAL void duk_push_lightfunc_tostring(duk_context *ctx, duk_tval *tv) {
	DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv));

	duk_push_string(ctx, "function ");
	duk_push_lightfunc_name(ctx, tv);
	duk_push_string(ctx, "() {/* light */}");
	duk_concat(ctx, 3);
}

/*
 *  Function pointers
 *
 *  Printing function pointers is non-portable, so we do that by hex printing
 *  bytes from memory.
 */

DUK_INTERNAL void duk_push_string_funcptr(duk_context *ctx, duk_uint8_t *ptr, duk_size_t sz) {
	duk_uint8_t buf[32 * 2];
	duk_uint8_t *p, *q;
	duk_small_uint_t i;
	duk_small_uint_t t;

	DUK_ASSERT(sz <= 32);  /* sanity limit for function pointer size */

	p = buf;
#if defined(DUK_USE_INTEGER_LE)
	q = ptr + sz;
#else
	q = ptr;
#endif
	for (i = 0; i < sz; i++) {
#if defined(DUK_USE_INTEGER_LE)
		t = *(--q);
#else
		t = *(q++);
#endif
		*p++ = duk_lc_digits[t >> 4];
		*p++ = duk_lc_digits[t & 0x0f];
	}

	duk_push_lstring(ctx, (const char *) buf, sz * 2);
}

#undef DUK__CHECK_SPACE
