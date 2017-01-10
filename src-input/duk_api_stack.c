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

#if !defined(DUK_USE_VARIADIC_MACROS)
DUK_EXTERNAL const char *duk_api_global_filename = NULL;
DUK_EXTERNAL duk_int_t duk_api_global_line = 0;
#endif

/*
 *  Misc helpers
 */

#if !defined(DUK_USE_PACKED_TVAL)
DUK_LOCAL const duk_uint_t duk__type_from_tag[] = {
	DUK_TYPE_NUMBER,
	DUK_TYPE_NUMBER,  /* fastint */
	DUK_TYPE_UNDEFINED,
	DUK_TYPE_NULL,
	DUK_TYPE_BOOLEAN,
	DUK_TYPE_POINTER,
	DUK_TYPE_LIGHTFUNC,
	DUK_TYPE_NONE,
	DUK_TYPE_STRING,
	DUK_TYPE_OBJECT,
	DUK_TYPE_BUFFER,
};
DUK_LOCAL const duk_uint_t duk__type_mask_from_tag[] = {
	DUK_TYPE_MASK_NUMBER,
	DUK_TYPE_MASK_NUMBER,  /* fastint */
	DUK_TYPE_MASK_UNDEFINED,
	DUK_TYPE_MASK_NULL,
	DUK_TYPE_MASK_BOOLEAN,
	DUK_TYPE_MASK_POINTER,
	DUK_TYPE_MASK_LIGHTFUNC,
	DUK_TYPE_MASK_NONE,
	DUK_TYPE_MASK_STRING,
	DUK_TYPE_MASK_OBJECT,
	DUK_TYPE_MASK_BUFFER,
};
#endif  /* !DUK_USE_PACKED_TVAL */

/* Check that there's room to push one value. */
#if defined(DUK_USE_VALSTACK_UNSAFE)
/* Faster but value stack overruns are memory unsafe. */
#define DUK__CHECK_SPACE() do { \
		DUK_ASSERT(!(thr->valstack_top >= thr->valstack_end)); \
	} while (0)
#else
#define DUK__CHECK_SPACE() do { \
		if (DUK_UNLIKELY(thr->valstack_top >= thr->valstack_end)) { \
			DUK_ERROR_RANGE_PUSH_BEYOND(thr); \
		} \
	} while (0)
#endif

DUK_LOCAL_DECL duk_heaphdr *duk__get_tagged_heaphdr_raw(duk_context *ctx, duk_idx_t idx, duk_uint_t tag);

DUK_LOCAL duk_int_t duk__api_coerce_d2i(duk_context *ctx, duk_idx_t idx, duk_int_t def_value, duk_bool_t require) {
	duk_hthread *thr;
	duk_tval *tv;
	duk_small_int_t c;
	duk_double_t d;

	thr = (duk_hthread *) ctx;

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

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

	if (require) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "number", DUK_STR_NOT_NUMBER);
		/* not reachable */
	}

	return def_value;
}

DUK_LOCAL duk_uint_t duk__api_coerce_d2ui(duk_context *ctx, duk_idx_t idx, duk_uint_t def_value, duk_bool_t require) {
	duk_hthread *thr;
	duk_tval *tv;
	duk_small_int_t c;
	duk_double_t d;

	/* Same as above but for unsigned int range. */

	thr = (duk_hthread *) ctx;

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

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

	if (require) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "number", DUK_STR_NOT_NUMBER);
		/* not reachable */
	}

	return def_value;
}

/*
 *  Stack index validation/normalization and getting a stack duk_tval ptr.
 *
 *  These are called by many API entrypoints so the implementations must be
 *  fast and "inlined".
 *
 *  There's some repetition because of this; keep the functions in sync.
 */

DUK_EXTERNAL duk_idx_t duk_normalize_index(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uidx_t vs_size;
	duk_uidx_t uidx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	/* Care must be taken to avoid pointer wrapping in the index
	 * validation.  For instance, on a 32-bit platform with 8-byte
	 * duk_tval the index 0x20000000UL would wrap the memory space
	 * once.
	 */

	/* Assume value stack sizes (in elements) fits into duk_idx_t. */
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	vs_size = (duk_uidx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT_DISABLE(vs_size >= 0);  /* unsigned */

	if (idx < 0) {
		uidx = vs_size + (duk_uidx_t) idx;
	} else {
		/* since index non-negative */
		DUK_ASSERT(idx != DUK_INVALID_INDEX);
		uidx = (duk_uidx_t) idx;
	}

	/* DUK_INVALID_INDEX won't be accepted as a valid index. */
	DUK_ASSERT(vs_size + (duk_uidx_t) DUK_INVALID_INDEX >= vs_size);

	if (DUK_LIKELY(uidx < vs_size)) {
		return (duk_idx_t) uidx;
	}
	return DUK_INVALID_INDEX;
}

DUK_EXTERNAL duk_idx_t duk_require_normalize_index(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uidx_t vs_size;
	duk_uidx_t uidx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	vs_size = (duk_uidx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT_DISABLE(vs_size >= 0);  /* unsigned */

	if (idx < 0) {
		uidx = vs_size + (duk_uidx_t) idx;
	} else {
		DUK_ASSERT(idx != DUK_INVALID_INDEX);
		uidx = (duk_uidx_t) idx;
	}

	/* DUK_INVALID_INDEX won't be accepted as a valid index. */
	DUK_ASSERT(vs_size + (duk_uidx_t) DUK_INVALID_INDEX >= vs_size);

	if (DUK_LIKELY(uidx < vs_size)) {
		return (duk_idx_t) uidx;
	}
	DUK_ERROR_RANGE_INDEX(thr, idx);
	return 0;  /* unreachable */
}

DUK_INTERNAL duk_tval *duk_get_tval(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uidx_t vs_size;
	duk_uidx_t uidx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	vs_size = (duk_uidx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT_DISABLE(vs_size >= 0);  /* unsigned */

	if (idx < 0) {
		uidx = vs_size + (duk_uidx_t) idx;
	} else {
		DUK_ASSERT(idx != DUK_INVALID_INDEX);
		uidx = (duk_uidx_t) idx;
	}

	/* DUK_INVALID_INDEX won't be accepted as a valid index. */
	DUK_ASSERT(vs_size + (duk_uidx_t) DUK_INVALID_INDEX >= vs_size);

	if (DUK_LIKELY(uidx < vs_size)) {
		return thr->valstack_bottom + uidx;
	}
	return NULL;
}

/* Variant of duk_get_tval() which is guaranteed to return a valid duk_tval
 * pointer.  When duk_get_tval() would return NULL, this variant returns a
 * pointer to a duk_tval with tag DUK_TAG_UNUSED.  This allows the call site
 * to avoid an unnecessary NULL check which sometimes leads to better code.
 * The return duk_tval is read only (at least for the UNUSED value).
 */
DUK_LOCAL const duk_tval_unused duk__const_tval_unused = DUK_TVAL_UNUSED_INITIALIZER();

DUK_INTERNAL duk_tval *duk_get_tval_or_unused(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;
	tv = duk_get_tval(ctx, idx);
	if (tv != NULL) {
		return tv;
	}
	return (duk_tval *) DUK_LOSE_CONST(&duk__const_tval_unused);
}

DUK_INTERNAL duk_tval *duk_require_tval(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uidx_t vs_size;
	duk_uidx_t uidx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	vs_size = (duk_uidx_t) (thr->valstack_top - thr->valstack_bottom);
	DUK_ASSERT_DISABLE(vs_size >= 0);  /* unsigned */

	/* Use unsigned arithmetic to optimize comparison. */
	if (idx < 0) {
		uidx = vs_size + (duk_uidx_t) idx;
	} else {
		DUK_ASSERT(idx != DUK_INVALID_INDEX);
		uidx = (duk_uidx_t) idx;
	}

	/* DUK_INVALID_INDEX won't be accepted as a valid index. */
	DUK_ASSERT(vs_size + (duk_uidx_t) DUK_INVALID_INDEX >= vs_size);

	if (DUK_LIKELY(uidx < vs_size)) {
		return thr->valstack_bottom + uidx;
	}
	DUK_ERROR_RANGE_INDEX(thr, idx);
	return NULL;
}

/* Non-critical. */
DUK_EXTERNAL duk_bool_t duk_is_valid_index(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	return (duk_normalize_index(ctx, idx) >= 0);
}

/* Non-critical. */
DUK_EXTERNAL void duk_require_valid_index(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	if (duk_normalize_index(ctx, idx) < 0) {
		DUK_ERROR_RANGE_INDEX(thr, idx);
		return;  /* unreachable */
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

/* Internal helper to get current top but to require a minimum top value
 * (TypeError if not met).
 */
DUK_INTERNAL duk_idx_t duk_get_top_require_min(duk_context *ctx, duk_idx_t min_top) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	if (ret < min_top) {
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
	}
	return ret;
}

/* Set stack top within currently allocated range, but don't reallocate.
 * This is performance critical especially for call handling, so whenever
 * changing, profile and look at generated code.
 */
DUK_EXTERNAL void duk_set_top(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uidx_t vs_size;
	duk_uidx_t vs_limit;
	duk_uidx_t uidx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_bottom);
	vs_size = (duk_uidx_t) (thr->valstack_top - thr->valstack_bottom);
	vs_limit = (duk_uidx_t) (thr->valstack_end - thr->valstack_bottom);

	if (idx < 0) {
		/* Negative indices are always within allocated stack but
		 * must not go below zero index.
		 */
		uidx = vs_size + (duk_uidx_t) idx;
	} else {
		/* Positive index can be higher than valstack top but must
		 * not go above allocated stack (equality is OK).
		 */
		uidx = (duk_uidx_t) idx;
	}

	/* DUK_INVALID_INDEX won't be accepted as a valid index. */
	DUK_ASSERT(vs_size + (duk_uidx_t) DUK_INVALID_INDEX >= vs_size);
	DUK_ASSERT(vs_size + (duk_uidx_t) DUK_INVALID_INDEX >= vs_limit);

#if defined(DUK_USE_VALSTACK_UNSAFE)
	DUK_ASSERT(uidx <= vs_limit);
	DUK_UNREF(vs_limit);
#else
	if (DUK_UNLIKELY(uidx > vs_limit)) {
		DUK_ERROR_RANGE_INDEX(thr, idx);
		return;  /* unreachable */
	}
#endif
	DUK_ASSERT(uidx <= vs_limit);

	/* Handle change in value stack top.  Respect value stack
	 * initialization policy: 'undefined' above top.  Note that
	 * DECREF may cause a side effect that reallocates valstack,
	 * so must relookup after DECREF.
	 */

	if (uidx >= vs_size) {
		/* Stack size increases or stays the same. */
#if defined(DUK_USE_ASSERTIONS)
		duk_uidx_t count;

		count = uidx - vs_size;
		while (count != 0) {
			count--;
			tv = thr->valstack_top + count;
			DUK_ASSERT(DUK_TVAL_IS_UNDEFINED(tv));
		}
#endif
		thr->valstack_top = thr->valstack_bottom + uidx;
	} else {
		/* Stack size decreases. */
#if defined(DUK_USE_REFERENCE_COUNTING)
		duk_uidx_t count;
		duk_tval *tv_end;

		count = vs_size - uidx;
		DUK_ASSERT(count > 0);
		tv = thr->valstack_top;
		tv_end = tv - count;
		DUK_ASSERT(tv > tv_end);  /* Because count > 0. */
		do {
			tv--;
			DUK_ASSERT(tv >= thr->valstack_bottom);
			DUK_TVAL_SET_UNDEFINED_UPDREF_NORZ(thr, tv);
		} while (tv != tv_end);
		thr->valstack_top = tv_end;
		DUK_REFZERO_CHECK_FAST(thr);
#else  /* DUK_USE_REFERENCE_COUNTING */
		duk_uidx_t count;
		duk_tval *tv_end;

		count = vs_size - uidx;
		tv = thr->valstack_top;
		tv_end = tv - count;
		DUK_ASSERT(tv > tv_end);
		do {
			tv--;
			DUK_TVAL_SET_UNDEFINED(tv);
		} while (tv != tv_end);
		thr->valstack_top = tv_end;
#endif  /* DUK_USE_REFERENCE_COUNTING */
	}
}

DUK_EXTERNAL duk_idx_t duk_get_top_index(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom) - 1;
	if (DUK_UNLIKELY(ret < 0)) {
		/* Return invalid index; if caller uses this without checking
		 * in another API call, the index won't map to a valid stack
		 * entry.
		 */
		return DUK_INVALID_INDEX;
	}
	return ret;
}

/* Internal variant: call assumes there is at least one element on the value
 * stack frame; this is only asserted for.
 */
DUK_INTERNAL duk_idx_t duk_get_top_index_unsafe(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom) - 1;
	return ret;
}

DUK_EXTERNAL duk_idx_t duk_require_top_index(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom) - 1;
	if (DUK_UNLIKELY(ret < 0)) {
		DUK_ERROR_RANGE_INDEX(thr, -1);
		return 0;  /* unreachable */
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
#if defined(DUK_USE_DEBUG)
	duk_ptrdiff_t old_end_offset_pre;
	duk_tval *old_valstack_pre;
	duk_tval *old_valstack_post;
#endif
	duk_tval *new_valstack;
	duk_size_t new_alloc_size;
	duk_tval *p;

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
#if defined(DUK_USE_DEBUG)
	old_end_offset_pre = (((duk_uint8_t *) thr->valstack_end) - ((duk_uint8_t *) thr->valstack));  /* not very useful, used for debugging */
	old_valstack_pre = thr->valstack;
#endif

	/* Allocate a new valstack.
	 *
	 * Note: cannot use a plain DUK_REALLOC() because a mark-and-sweep may
	 * invalidate the original thr->valstack base pointer inside the realloc
	 * process.  See doc/memory-management.rst.
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
	 *     to UNDEFINED, up to the post-realloc valstack_end.
	 *   - 'old_end_offset' must be computed after realloc to be correct.
	 */

	DUK_ASSERT((((duk_uint8_t *) thr->valstack_bottom) - ((duk_uint8_t *) thr->valstack)) == old_bottom_offset);
	DUK_ASSERT((((duk_uint8_t *) thr->valstack_top) - ((duk_uint8_t *) thr->valstack)) == old_top_offset);

	/* success, fixup pointers */
	old_end_offset_post = (((duk_uint8_t *) thr->valstack_end) - ((duk_uint8_t *) thr->valstack));  /* must be computed after realloc */
#if defined(DUK_USE_DEBUG)
	old_valstack_post = thr->valstack;
#endif
	thr->valstack = new_valstack;
	thr->valstack_end = new_valstack + new_size;
#if !defined(DUK_USE_PREFER_SIZE)
	thr->valstack_size = new_size;
#endif
	thr->valstack_bottom = (duk_tval *) (void *) ((duk_uint8_t *) new_valstack + old_bottom_offset);
	thr->valstack_top = (duk_tval *) (void *) ((duk_uint8_t *) new_valstack + old_top_offset);

	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	/* useful for debugging */
#if defined(DUK_USE_DEBUG)
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

	/* Init newly allocated slots (only). */
	p = (duk_tval *) (void *) ((duk_uint8_t *) thr->valstack + old_end_offset_post);
	while (p < thr->valstack_end) {
		/* Never executed if new size is smaller. */
		DUK_TVAL_SET_UNDEFINED(p);
		p++;
	}

	/* Assert for value stack initialization policy. */
#if defined(DUK_USE_ASSERTIONS)
	p = thr->valstack_top;
	while (p < thr->valstack_end) {
		DUK_ASSERT(DUK_TVAL_IS_UNDEFINED(p));
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

#if defined(DUK_USE_PREFER_SIZE)
	old_size = (duk_size_t) (thr->valstack_end - thr->valstack);
#else
	DUK_ASSERT((duk_size_t) (thr->valstack_end - thr->valstack) == thr->valstack_size);
	old_size = thr->valstack_size;
#endif

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
			DUK_ERROR_RANGE(thr, DUK_STR_VALSTACK_LIMIT);
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
			DUK_ERROR_ALLOC_FAILED(thr);
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

DUK_EXTERNAL void duk_swap(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2) {
	duk_tval *tv1;
	duk_tval *tv2;
	duk_tval tv_tmp;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_require_tval(ctx, idx1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, idx2);
	DUK_ASSERT(tv2 != NULL);

	/* If tv1==tv2 this is a NOP, no check is needed */
	DUK_TVAL_SET_TVAL(&tv_tmp, tv1);
	DUK_TVAL_SET_TVAL(tv1, tv2);
	DUK_TVAL_SET_TVAL(tv2, &tv_tmp);
}

DUK_EXTERNAL void duk_swap_top(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_swap(ctx, idx, -1);
}

DUK_EXTERNAL void duk_dup(duk_context *ctx, duk_idx_t from_idx) {
	duk_hthread *thr;
	duk_tval *tv_from;
	duk_tval *tv_to;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();

	tv_from = duk_require_tval(ctx, from_idx);
	tv_to = thr->valstack_top++;
	DUK_ASSERT(tv_from != NULL);
	DUK_ASSERT(tv_to != NULL);
	DUK_TVAL_SET_TVAL(tv_to, tv_from);
	DUK_TVAL_INCREF(thr, tv_to);  /* no side effects */
}

DUK_EXTERNAL void duk_dup_top(duk_context *ctx) {
#if defined(DUK_USE_PREFER_SIZE)
	duk_dup(ctx, -1);
#else
	duk_hthread *thr;
	duk_tval *tv_from;
	duk_tval *tv_to;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();

	if (thr->valstack_top - thr->valstack_bottom <= 0) {
		DUK_ERROR_RANGE_INDEX(thr, -1);
		return;  /* unreachable */
	}
	tv_from = thr->valstack_top - 1;
	tv_to = thr->valstack_top++;
	DUK_ASSERT(tv_from != NULL);
	DUK_ASSERT(tv_to != NULL);
	DUK_TVAL_SET_TVAL(tv_to, tv_from);
	DUK_TVAL_INCREF(thr, tv_to);  /* no side effects */
#endif
}

DUK_INTERNAL void duk_dup_0(duk_context *ctx) {
	duk_dup(ctx, 0);
}
DUK_INTERNAL void duk_dup_1(duk_context *ctx) {
	duk_dup(ctx, 1);
}
DUK_INTERNAL void duk_dup_2(duk_context *ctx) {
	duk_dup(ctx, 2);
}
DUK_INTERNAL void duk_dup_m2(duk_context *ctx) {
	duk_dup(ctx, -2);
}
DUK_INTERNAL void duk_dup_m3(duk_context *ctx) {
	duk_dup(ctx, -3);
}
DUK_INTERNAL void duk_dup_m4(duk_context *ctx) {
	duk_dup(ctx, -4);
}

DUK_EXTERNAL void duk_insert(duk_context *ctx, duk_idx_t to_idx) {
	duk_tval *p;
	duk_tval *q;
	duk_tval tv_tmp;
	duk_size_t nbytes;

	DUK_ASSERT_CTX_VALID(ctx);

	p = duk_require_tval(ctx, to_idx);
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

	DUK_DDD(DUK_DDDPRINT("duk_insert: to_idx=%ld, p=%p, q=%p, nbytes=%lu",
	                     (long) to_idx, (void *) p, (void *) q, (unsigned long) nbytes));

	/* No net refcount changes. */

	if (nbytes > 0) {
		DUK_TVAL_SET_TVAL(&tv_tmp, q);
		DUK_ASSERT(nbytes > 0);
		DUK_MEMMOVE((void *) (p + 1), (const void *) p, (size_t) nbytes);
		DUK_TVAL_SET_TVAL(p, &tv_tmp);
	} else {
		/* nop: insert top to top */
		DUK_ASSERT(nbytes == 0);
		DUK_ASSERT(p == q);
	}
}

DUK_EXTERNAL void duk_replace(duk_context *ctx, duk_idx_t to_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1;
	duk_tval *tv2;
	duk_tval tv_tmp;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_require_tval(ctx, -1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, to_idx);
	DUK_ASSERT(tv2 != NULL);

	/* For tv1 == tv2, both pointing to stack top, the end result
	 * is same as duk_pop(ctx).
	 */
	DUK_TVAL_SET_TVAL(&tv_tmp, tv2);
	DUK_TVAL_SET_TVAL(tv2, tv1);
	DUK_TVAL_SET_UNDEFINED(tv1);
	thr->valstack_top--;
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
}

DUK_EXTERNAL void duk_copy(duk_context *ctx, duk_idx_t from_idx, duk_idx_t to_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1;
	duk_tval *tv2;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);  /* w/o refcounting */

	tv1 = duk_require_tval(ctx, from_idx);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, to_idx);
	DUK_ASSERT(tv2 != NULL);

	/* For tv1 == tv2, this is a no-op (no explicit check needed). */
	DUK_TVAL_SET_TVAL_UPDREF(thr, tv2, tv1);  /* side effects */
}

DUK_EXTERNAL void duk_remove(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *p;
	duk_tval *q;
#if defined(DUK_USE_REFERENCE_COUNTING)
	duk_tval tv_tmp;
#endif
	duk_size_t nbytes;

	DUK_ASSERT_CTX_VALID(ctx);

	p = duk_require_tval(ctx, idx);
	DUK_ASSERT(p != NULL);
	q = duk_require_tval(ctx, -1);
	DUK_ASSERT(q != NULL);

	DUK_ASSERT(q >= p);

	/*              nbytes            zero size case
	 *           <--------->
	 *    [ ... | p | x | x | q ]     [ ... | p==q ]
	 * => [ ... | x | x | q ]         [ ... ]
	 */

#if defined(DUK_USE_REFERENCE_COUNTING)
	/* use a temp: decref only when valstack reachable values are correct */
	DUK_TVAL_SET_TVAL(&tv_tmp, p);
#endif

	nbytes = (duk_size_t) (((duk_uint8_t *) q) - ((duk_uint8_t *) p));  /* Note: 'q' is top-1 */
	DUK_MEMMOVE((void *) p, (const void *) (p + 1), (size_t) nbytes);  /* zero size not an issue: pointers are valid */

	DUK_TVAL_SET_UNDEFINED(q);
	thr->valstack_top--;

#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
#endif
}

DUK_INTERNAL_DECL void duk_remove_m2(duk_context *ctx) {
	duk_remove(ctx, -2);
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
		DUK_ERROR_TYPE(to_thr, DUK_STR_INVALID_CONTEXT);
		return;
	}
	if ((count < 0) ||
	    (count > (duk_idx_t) to_thr->valstack_max)) {
		/* Maximum value check ensures 'nbytes' won't wrap below. */
		DUK_ERROR_RANGE_INVALID_COUNT(to_thr);
		return;
	}

	nbytes = sizeof(duk_tval) * count;
	if (nbytes == 0) {
		return;
	}
	DUK_ASSERT(to_thr->valstack_top <= to_thr->valstack_end);
	if ((duk_size_t) ((duk_uint8_t *) to_thr->valstack_end - (duk_uint8_t *) to_thr->valstack_top) < nbytes) {
		DUK_ERROR_RANGE_PUSH_BEYOND(to_thr);
	}
	src = (void *) ((duk_uint8_t *) from_thr->valstack_top - nbytes);
	if (src < (void *) from_thr->valstack_bottom) {
		DUK_ERROR_RANGE_INVALID_COUNT(to_thr);
	}

	/* copy values (no overlap even if to_ctx == from_ctx; that's not
	 * allowed now anyway)
	 */
	DUK_ASSERT(nbytes > 0);
	DUK_MEMCPY((void *) to_thr->valstack_top, (const void *) src, (size_t) nbytes);

	p = to_thr->valstack_top;
	to_thr->valstack_top = (duk_tval *) (void *) (((duk_uint8_t *) p) + nbytes);

	if (is_copy) {
		/* Incref copies, keep originals. */
		q = to_thr->valstack_top;
		while (p < q) {
			DUK_TVAL_INCREF(to_thr, p);  /* no side effects */
			p++;
		}
	} else {
		/* No net refcount change. */
		p = from_thr->valstack_top;
		q = (duk_tval *) (void *) (((duk_uint8_t *) p) - nbytes);
		from_thr->valstack_top = q;

		while (p > q) {
			p--;
			DUK_TVAL_SET_UNDEFINED(p);
			/* XXX: fast primitive to set a bunch of values to UNDEFINED */
		}
	}
}

/*
 *  Get/opt/require
 */

DUK_EXTERNAL void duk_require_undefined(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_UNDEFINED(tv)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "undefined", DUK_STR_NOT_UNDEFINED);
	}
}

DUK_EXTERNAL void duk_require_null(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_NULL(tv)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "null", DUK_STR_NOT_NULL);
	}
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__getopt_boolean(duk_context *ctx, duk_idx_t idx, duk_bool_t def_value) {
	duk_bool_t ret;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_BOOLEAN(tv)) {
		ret = DUK_TVAL_GET_BOOLEAN(tv);
		DUK_ASSERT(ret == 0 || ret == 1);
	} else {
		ret = def_value;
		/* Not guaranteed to be 0 or 1. */
	}

	return ret;
}

DUK_EXTERNAL duk_bool_t duk_get_boolean(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk__getopt_boolean(ctx, idx, 0);  /* default: false */
}

DUK_EXTERNAL duk_bool_t duk_opt_boolean(duk_context *ctx, duk_idx_t idx, duk_bool_t def_value) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk__getopt_boolean(ctx, idx, def_value);
}

DUK_EXTERNAL duk_bool_t duk_require_boolean(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_bool_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_BOOLEAN(tv)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "boolean", DUK_STR_NOT_BOOLEAN);
	}
	ret = DUK_TVAL_GET_BOOLEAN(tv);
	DUK_ASSERT(ret == 0 || ret == 1);
	return ret;
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_double_t duk__getopt_number(duk_context *ctx, duk_idx_t idx, duk_double_t def_value) {
	duk_double_union ret;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
#if defined(DUK_USE_FASTINT)
	if (DUK_TVAL_IS_FASTINT(tv)) {
		ret.d = (duk_double_t) DUK_TVAL_GET_FASTINT(tv);  /* FIXME: cast trick */
	}
	else
#endif
	if (DUK_TVAL_IS_DOUBLE(tv)) {
		ret.d = DUK_TVAL_GET_DOUBLE(tv);
	} else {
		ret.d = def_value;
		/* FIXME: generates unnecessary code for duk_get_double()? */
		DUK_DBLUNION_NORMALIZE_NAN_CHECK(&ret);  /* FIXME: test coverage */
	}

	/* When using packed duk_tval, number must be in NaN-normalized form
	 * for it to be a duk_tval, so no need to normalize.  NOP for unpacked
	 * duk_tval.
	 */
	DUK_ASSERT(DUK_DBLUNION_IS_NORMALIZED(&ret));
	return ret.d;
}

DUK_EXTERNAL duk_double_t duk_get_number(duk_context *ctx, duk_idx_t idx) {
	return duk__getopt_number(ctx, idx, DUK_DOUBLE_NAN);  /* default: NaN */
}

DUK_EXTERNAL duk_double_t duk_opt_number(duk_context *ctx, duk_idx_t idx, duk_double_t def_value) {
	return duk__getopt_number(ctx, idx, def_value);
}

DUK_EXTERNAL duk_double_t duk_require_number(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_double_union ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_NUMBER(tv)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "number", DUK_STR_NOT_NUMBER);
	}

	ret.d = DUK_TVAL_GET_NUMBER(tv);

	/* When using packed duk_tval, number must be in NaN-normalized form
	 * for it to be a duk_tval, so no need to normalize.  NOP for unpacked
	 * duk_tval.
	 */
	DUK_ASSERT(DUK_DBLUNION_IS_NORMALIZED(&ret));
	return ret.d;
}

DUK_EXTERNAL duk_int_t duk_get_int(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_int_t) duk__api_coerce_d2i(ctx, idx, 0 /*def_value*/, 0 /*require*/);
}

DUK_EXTERNAL duk_uint_t duk_get_uint(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_uint_t) duk__api_coerce_d2ui(ctx, idx, 0 /*def_value*/, 0 /*require*/);
}

DUK_EXTERNAL duk_int_t duk_opt_int(duk_context *ctx, duk_idx_t idx, duk_int_t def_value) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_int_t) duk__api_coerce_d2i(ctx, idx, def_value, 0 /*require*/);
}

DUK_EXTERNAL duk_uint_t duk_opt_uint(duk_context *ctx, duk_idx_t idx, duk_uint_t def_value) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_uint_t) duk__api_coerce_d2ui(ctx, idx, def_value, 0 /*require*/);
}

DUK_EXTERNAL duk_int_t duk_require_int(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_int_t) duk__api_coerce_d2i(ctx, idx, 0 /*def_value*/, 1 /*require*/);
}

DUK_EXTERNAL duk_uint_t duk_require_uint(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_uint_t) duk__api_coerce_d2ui(ctx, idx, 0 /*def_value*/, 1 /*require*/);
}

DUK_EXTERNAL const char *duk_get_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len) {
	duk_hstring *h;
	const char *ret;
	duk_size_t len;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hstring(ctx, idx);
	if (h != NULL) {
		len = DUK_HSTRING_GET_BYTELEN(h);
		ret = (const char *) DUK_HSTRING_GET_DATA(h);
	} else {
		len = 0;
		ret = NULL;
	}

	if (DUK_LIKELY(out_len != NULL)) {
		*out_len = len;
	}
	return ret;
}

DUK_EXTERNAL const char *duk_require_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_require_hstring(ctx, idx);
	DUK_ASSERT(h != NULL);
	if (out_len) {
		*out_len = DUK_HSTRING_GET_BYTELEN(h);
	}
	return (const char *) DUK_HSTRING_GET_DATA(h);
}

DUK_INTERNAL const char *duk_require_lstring_notsymbol(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_require_hstring_notsymbol(ctx, idx);
	DUK_ASSERT(h != NULL);
	if (out_len) {
		*out_len = DUK_HSTRING_GET_BYTELEN(h);
	}
	return (const char *) DUK_HSTRING_GET_DATA(h);
}

DUK_EXTERNAL const char *duk_get_string(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hstring(ctx, idx);
	if (h != NULL) {
		return (const char *) DUK_HSTRING_GET_DATA(h);
	} else {
		return NULL;
	}
}

DUK_EXTERNAL const char *duk_opt_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len, const char *def_ptr, duk_size_t def_len) {
	duk_hstring *h;
	const char *ret;
	duk_size_t len;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hstring(ctx, idx);
	if (h != NULL) {
		len = DUK_HSTRING_GET_BYTELEN(h);
		ret = (const char *) DUK_HSTRING_GET_DATA(h);
	} else {
		len = def_len;
		ret = def_ptr;
	}

	if (DUK_LIKELY(out_len != NULL)) {
		*out_len = len;
	}
	return ret;
}

DUK_EXTERNAL const char *duk_opt_string(duk_context *ctx, duk_idx_t idx, const char *def_value) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hstring(ctx, idx);
	if (h != NULL) {
		return (const char *) DUK_HSTRING_GET_DATA(h);
	} else {
		return def_value;  /* FIXME: shared helper */
	}
}

DUK_INTERNAL const char *duk_get_string_notsymbol(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hstring_notsymbol(ctx, idx);
	if (h) {
		return (const char *) DUK_HSTRING_GET_DATA(h);
	} else {
		return NULL;
	}
}

DUK_EXTERNAL const char *duk_require_string(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk_require_lstring(ctx, idx, NULL);
}

DUK_INTERNAL const char *duk_require_string_notsymbol(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_require_hstring_notsymbol(ctx, idx);
	DUK_ASSERT(h != NULL);
	return (const char *) DUK_HSTRING_GET_DATA(h);
}

DUK_LOCAL void *duk__getopt_pointer(duk_context *ctx, duk_idx_t idx, void *def_value) {
	duk_tval *tv;
	void *p;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_POINTER(tv)) {
		return def_value;
	}

	p = DUK_TVAL_GET_POINTER(tv);  /* may be NULL */
	return p;
}

DUK_EXTERNAL void *duk_get_pointer(duk_context *ctx, duk_idx_t idx) {
	return duk__getopt_pointer(ctx, idx, NULL /*def_value*/);
}

DUK_EXTERNAL void *duk_opt_pointer(duk_context *ctx, duk_idx_t idx, void *def_value) {
	return duk__getopt_pointer(ctx, idx, def_value);
}

DUK_EXTERNAL void *duk_require_pointer(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	void *p;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Note: here we must be wary of the fact that a pointer may be
	 * valid and be a NULL.
	 */
	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_POINTER(tv)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "pointer", DUK_STR_NOT_POINTER);
	}
	p = DUK_TVAL_GET_POINTER(tv);  /* may be NULL */
	return p;
}

#if 0  /*unused*/
DUK_INTERNAL void *duk_get_voidptr(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;
	duk_heaphdr *h;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		return NULL;
	}

	h = DUK_TVAL_GET_HEAPHDR(tv);
	DUK_ASSERT(h != NULL);
	return (void *) h;
}
#endif

DUK_LOCAL void *duk__get_buffer_helper(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size, void *def_ptr, duk_size_t def_len, duk_bool_t throw_flag) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbuffer *h;
	void *ret;
	duk_size_t len;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	h = duk_get_hbuffer(ctx, idx);
	if (h != NULL) {
		len = DUK_HBUFFER_GET_SIZE(h);
		ret = (void *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h);  /* may be NULL (but only if size is 0) */
	} else {
		if (throw_flag) {
			DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "buffer", DUK_STR_NOT_BUFFER);
		}
		len = def_len;
		ret = def_ptr;
	}

	if (DUK_LIKELY(out_size != NULL)) {
		*out_size = len;
	}
	return ret;
}

DUK_EXTERNAL void *duk_get_buffer(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk__get_buffer_helper(ctx, idx, out_size, NULL /*def_ptr*/, 0 /*def_len*/, 0 /*throw_flag*/);
}

DUK_EXTERNAL void *duk_opt_buffer(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size, void *def_ptr, duk_size_t def_len) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk__get_buffer_helper(ctx, idx, out_size, def_ptr, def_len, 0 /*throw_flag*/);
}

DUK_EXTERNAL void *duk_require_buffer(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk__get_buffer_helper(ctx, idx, out_size, NULL /*def_ptr*/, 0 /*def_len*/, 1 /*throw_flag*/);
}

/* Get the active buffer data area for a plain buffer or a buffer object.
 * Return NULL if the the value is not a buffer.  Note that a buffer may
 * have a NULL data pointer when its size is zero, the optional 'out_isbuffer'
 * argument allows caller to detect this reliably.
 */
DUK_INTERNAL void *duk_get_buffer_data_raw(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size, void *def_ptr, duk_size_t def_len, duk_bool_t throw_flag, duk_bool_t *out_isbuffer) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	if (out_isbuffer != NULL) {
		*out_isbuffer = 0;
	}
	if (out_size != NULL) {
		*out_size = def_len;
	}

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		if (out_size != NULL) {
			*out_size = DUK_HBUFFER_GET_SIZE(h);
		}
		if (out_isbuffer != NULL) {
			*out_isbuffer = 1;
		}
		return (void *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h);  /* may be NULL (but only if size is 0) */
	}
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	else if (DUK_TVAL_IS_OBJECT(tv)) {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		if (DUK_HOBJECT_IS_BUFOBJ(h)) {
			/* XXX: this is probably a useful shared helper: for a
			 * duk_hbufobj, get a validated buffer pointer/length.
			 */
			duk_hbufobj *h_bufobj = (duk_hbufobj *) h;
			DUK_ASSERT_HBUFOBJ_VALID(h_bufobj);

			if (h_bufobj->buf != NULL &&
			    DUK_HBUFOBJ_VALID_SLICE(h_bufobj)) {
				duk_uint8_t *p;

				p = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h_bufobj->buf);
				if (out_size != NULL) {
					*out_size = (duk_size_t) h_bufobj->length;
				}
				if (out_isbuffer != NULL) {
					*out_isbuffer = 1;
				}
				return (void *) (p + h_bufobj->offset);
			}
			/* if slice not fully valid, treat as error */
		}
	}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */

	if (throw_flag) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "buffer", DUK_STR_NOT_BUFFER);
	}
	return def_ptr;
}

DUK_EXTERNAL void *duk_get_buffer_data(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size) {
	return duk_get_buffer_data_raw(ctx, idx, out_size, NULL /*def_ptr*/, 0 /*def_len*/, 0 /*throw_flag*/, NULL);
}

DUK_EXTERNAL void *duk_opt_buffer_data(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size, void *def_ptr, duk_size_t def_len) {
	return duk_get_buffer_data_raw(ctx, idx, out_size, def_ptr, def_len, 0 /*throw_flag*/, NULL);
}

DUK_EXTERNAL void *duk_require_buffer_data(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size) {
	return duk_get_buffer_data_raw(ctx, idx, out_size, NULL /*def_ptr*/, 0 /*def_len*/, 1 /*throw_flag*/, NULL);
}

/* Raw helper for getting a value from the stack, checking its tag.
 * The tag cannot be a number because numbers don't have an internal
 * tag in the packed representation.
 */

DUK_LOCAL duk_heaphdr *duk__get_tagged_heaphdr_raw(duk_context *ctx, duk_idx_t idx, duk_uint_t tag) {
	duk_tval *tv;
	duk_heaphdr *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_GET_TAG(tv) != tag) {
		return (duk_heaphdr *) NULL;
	}

	ret = DUK_TVAL_GET_HEAPHDR(tv);
	DUK_ASSERT(ret != NULL);  /* tagged null pointers should never occur */
	return ret;

}

DUK_INTERNAL duk_hstring *duk_get_hstring(duk_context *ctx, duk_idx_t idx) {
	return (duk_hstring *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_STRING);
}

DUK_INTERNAL duk_hstring *duk_get_hstring_notsymbol(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *res = (duk_hstring *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_STRING);
	if (res && DUK_HSTRING_HAS_SYMBOL(res)) {
		return NULL;
	}
	return res;
}

DUK_INTERNAL duk_hstring *duk_require_hstring(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *h;
	h = (duk_hstring *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_STRING);
	if (h == NULL) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(ctx, idx, "string", DUK_STR_NOT_STRING);
	}
	return h;
}

DUK_INTERNAL duk_hstring *duk_require_hstring_notsymbol(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *h;
	h = (duk_hstring *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_STRING);
	if (h == NULL || DUK_HSTRING_HAS_SYMBOL(h)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(ctx, idx, "string", DUK_STR_NOT_STRING);
	}
	return h;
}

DUK_INTERNAL duk_hobject *duk_get_hobject(duk_context *ctx, duk_idx_t idx) {
	return (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
}

DUK_INTERNAL duk_hobject *duk_require_hobject(duk_context *ctx, duk_idx_t idx) {
	duk_hobject *h;
	h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (h == NULL) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(ctx, idx, "object", DUK_STR_NOT_OBJECT);
	}
	return h;
}

DUK_INTERNAL duk_hbuffer *duk_get_hbuffer(duk_context *ctx, duk_idx_t idx) {
	return (duk_hbuffer *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_BUFFER);
}

DUK_INTERNAL duk_hbuffer *duk_require_hbuffer(duk_context *ctx, duk_idx_t idx) {
	duk_hbuffer *h;
	h = (duk_hbuffer *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_BUFFER);
	if (h == NULL) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(ctx, idx, "buffer", DUK_STR_NOT_BUFFER);
	}
	return h;
}

DUK_INTERNAL duk_hthread *duk_get_hthread(duk_context *ctx, duk_idx_t idx) {
	duk_hobject *h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (h != NULL && !DUK_HOBJECT_IS_THREAD(h)) {
		h = NULL;
	}
	return (duk_hthread *) h;
}

DUK_INTERNAL duk_hthread *duk_require_hthread(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (!(h != NULL && DUK_HOBJECT_IS_THREAD(h))) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "thread", DUK_STR_NOT_THREAD);
	}
	return (duk_hthread *) h;
}

DUK_INTERNAL duk_hcompfunc *duk_get_hcompfunc(duk_context *ctx, duk_idx_t idx) {
	duk_hobject *h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (h != NULL && !DUK_HOBJECT_IS_COMPFUNC(h)) {
		h = NULL;
	}
	return (duk_hcompfunc *) h;
}

DUK_INTERNAL duk_hcompfunc *duk_require_hcompfunc(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (!(h != NULL && DUK_HOBJECT_IS_COMPFUNC(h))) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "compiledfunction", DUK_STR_NOT_COMPFUNC);
	}
	return (duk_hcompfunc *) h;
}

DUK_INTERNAL duk_hnatfunc *duk_get_hnatfunc(duk_context *ctx, duk_idx_t idx) {
	duk_hobject *h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (h != NULL && !DUK_HOBJECT_IS_NATFUNC(h)) {
		h = NULL;
	}
	return (duk_hnatfunc *) h;
}

DUK_INTERNAL duk_hnatfunc *duk_require_hnatfunc(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (!(h != NULL && DUK_HOBJECT_IS_NATFUNC(h))) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "nativefunction", DUK_STR_NOT_NATFUNC);
	}
	return (duk_hnatfunc *) h;
}

DUK_EXTERNAL duk_c_function duk_get_c_function(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;
	duk_hobject *h;
	duk_hnatfunc *f;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_OBJECT(tv)) {
		return NULL;
	}
	h = DUK_TVAL_GET_OBJECT(tv);
	DUK_ASSERT(h != NULL);

	if (!DUK_HOBJECT_IS_NATFUNC(h)) {
		return NULL;
	}
	DUK_ASSERT(DUK_HOBJECT_HAS_NATFUNC(h));
	f = (duk_hnatfunc *) h;

	return f->func;
}

DUK_EXTERNAL duk_c_function duk_opt_c_function(duk_context *ctx, duk_idx_t idx, duk_c_function def_value) {
	duk_c_function ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = duk_get_c_function(ctx, idx);
	if (ret != NULL) {
		return ret;
	}

	return def_value;
}

DUK_EXTERNAL duk_c_function duk_require_c_function(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_c_function ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = duk_get_c_function(ctx, idx);
	if (!ret) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "nativefunction", DUK_STR_NOT_NATFUNC);
	}
	return ret;
}

DUK_EXTERNAL void duk_require_function(duk_context *ctx, duk_idx_t idx) {
	if (!duk_is_function(ctx, idx)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX((duk_hthread *) ctx, idx, "function", DUK_STR_NOT_FUNCTION);
	}
}

DUK_INTERNAL_DECL void duk_require_constructable(duk_context *ctx, duk_idx_t idx) {
	duk_hobject *h;

	h = duk_require_hobject_accept_mask(ctx, idx, DUK_TYPE_MASK_LIGHTFUNC);
	if (h != NULL && !DUK_HOBJECT_HAS_CONSTRUCTABLE(h)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX((duk_hthread *) ctx, idx, "constructable", DUK_STR_NOT_CONSTRUCTABLE);
	}
	/* Lightfuncs (h == NULL) are constructable. */
}

DUK_EXTERNAL duk_context *duk_get_context(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_context *) duk_get_hthread(ctx, idx);
}

DUK_EXTERNAL duk_context *duk_require_context(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_context *) duk_require_hthread(ctx, idx);
}

DUK_EXTERNAL_DECL duk_context *duk_opt_context(duk_context *ctx, duk_idx_t idx, duk_context *def_value) {
	duk_context *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = duk_get_context(ctx, idx);
	if (ret != NULL) {
		return ret;
	}

	return def_value;
}

DUK_EXTERNAL void *duk_get_heapptr(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;
	void *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		return (void *) NULL;
	}

	ret = (void *) DUK_TVAL_GET_HEAPHDR(tv);
	DUK_ASSERT(ret != NULL);
	return ret;
}

DUK_EXTERNAL_DECL void *duk_opt_heapptr(duk_context *ctx, duk_idx_t idx, void *def_value) {
	void *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	ret = duk_get_heapptr(ctx, idx);
	if (ret != NULL) {
		return ret;
	}

	return def_value;
}

DUK_EXTERNAL void *duk_require_heapptr(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	void *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (!DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, "heapobject", DUK_STR_UNEXPECTED_TYPE);
	}

	ret = (void *) DUK_TVAL_GET_HEAPHDR(tv);
	DUK_ASSERT(ret != NULL);
	return ret;
}

/* Internal helper for getting/requiring a duk_hobject with possible promotion. */
DUK_LOCAL duk_hobject *duk__get_hobject_promote_mask_raw(duk_context *ctx, duk_idx_t idx, duk_uint_t type_mask) {
	duk_uint_t val_mask;
	duk_hobject *res;

	DUK_ASSERT_CTX_VALID(ctx);

	res = duk_get_hobject(ctx, idx);  /* common case, not promoted */
	if (res != NULL) {
		DUK_ASSERT(res != NULL);
		return res;
	}

	val_mask = duk_get_type_mask(ctx, idx);
	if (val_mask & type_mask) {
		if (type_mask & DUK_TYPE_MASK_PROMOTE) {
			res = duk_to_hobject(ctx, idx);
			DUK_ASSERT(res != NULL);
			return res;
		} else {
			return NULL;  /* accept without promoting */
		}
	}

	if (type_mask & DUK_TYPE_MASK_THROW) {
		DUK_ERROR_REQUIRE_TYPE_INDEX((duk_hthread *) ctx, idx, "object", DUK_STR_NOT_OBJECT);
	}
	return NULL;
}

/* Get a duk_hobject * at 'idx'; if the value is not an object but matches the
 * supplied 'type_mask', promote it to an object and return the duk_hobject *.
 * This is useful for call sites which want an object but also accept a plain
 * buffer and/or a lightfunc which gets automatically promoted to an object.
 * Return value is NULL if value is neither an object nor a plain type allowed
 * by the mask.
 */
DUK_INTERNAL duk_hobject *duk_get_hobject_promote_mask(duk_context *ctx, duk_idx_t idx, duk_uint_t type_mask) {
	return duk__get_hobject_promote_mask_raw(ctx, idx, type_mask | DUK_TYPE_MASK_PROMOTE);
}

/* Like duk_get_hobject_promote_mask() but throw a TypeError instead of
 * returning a NULL.
 */
DUK_INTERNAL duk_hobject *duk_require_hobject_promote_mask(duk_context *ctx, duk_idx_t idx, duk_uint_t type_mask) {
	return duk__get_hobject_promote_mask_raw(ctx, idx, type_mask | DUK_TYPE_MASK_THROW | DUK_TYPE_MASK_PROMOTE);
}

/* Require a duk_hobject * at 'idx'; if the value is not an object but matches the
 * supplied 'type_mask', return a NULL instead.  Otherwise throw a TypeError.
 */
DUK_INTERNAL duk_hobject *duk_require_hobject_accept_mask(duk_context *ctx, duk_idx_t idx, duk_uint_t type_mask) {
	return duk__get_hobject_promote_mask_raw(ctx, idx, type_mask | DUK_TYPE_MASK_THROW);
}

DUK_INTERNAL duk_hobject *duk_get_hobject_with_class(duk_context *ctx, duk_idx_t idx, duk_small_uint_t classnum) {
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(classnum >= 0);  /* unsigned */
	DUK_ASSERT(classnum <= DUK_HOBJECT_CLASS_MAX);

	h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (h != NULL && DUK_HOBJECT_GET_CLASS_NUMBER(h) != classnum) {
		h = NULL;
	}
	return h;
}

DUK_INTERNAL duk_hobject *duk_require_hobject_with_class(duk_context *ctx, duk_idx_t idx, duk_small_uint_t classnum) {
	duk_hthread *thr;
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(classnum >= 0);  /* unsigned */
	DUK_ASSERT(classnum <= DUK_HOBJECT_CLASS_MAX);
	thr = (duk_hthread *) ctx;

	h = (duk_hobject *) duk__get_tagged_heaphdr_raw(ctx, idx, DUK_TAG_OBJECT);
	if (!(h != NULL && DUK_HOBJECT_GET_CLASS_NUMBER(h) == classnum)) {
		duk_hstring *h_class;
		h_class = DUK_HTHREAD_GET_STRING(thr, DUK_HOBJECT_CLASS_NUMBER_TO_STRIDX(classnum));
		DUK_UNREF(h_class);

		DUK_ERROR_REQUIRE_TYPE_INDEX(thr, idx, (const char *) DUK_HSTRING_GET_DATA(h_class), DUK_STR_UNEXPECTED_TYPE);
	}
	return h;
}

DUK_EXTERNAL duk_size_t duk_get_length(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
	case DUK_TAG_BOOLEAN:
	case DUK_TAG_POINTER:
		return 0;
#if defined(DUK_USE_PREFER_SIZE)
	/* All of these types (besides object) have a virtual, non-configurable
	 * .length property which is within size_t range so we can just look it
	 * up without specific type checks.
	 */
	case DUK_TAG_STRING:
	case DUK_TAG_BUFFER:
	case DUK_TAG_LIGHTFUNC: {
		duk_size_t ret;
		duk_get_prop_stridx(ctx, idx, DUK_STRIDX_LENGTH);
		ret = (duk_size_t) duk_to_number_m1(ctx);
		duk_pop(ctx);
		return ret;
	}
#else  /* DUK_USE_PREFER_SIZE */
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);
		if (DUK_HSTRING_HAS_SYMBOL(h)) {
			return 0;
		}
		return (duk_size_t) DUK_HSTRING_GET_CHARLEN(h);
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
#endif  /* DUK_USE_PREFER_SIZE */
	case DUK_TAG_OBJECT: {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		return (duk_size_t) duk_hobject_get_length((duk_hthread *) ctx, h);
	}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default:
		/* number or 'unused' */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv) || DUK_TVAL_IS_UNUSED(tv));
		return 0;
	}

	DUK_UNREACHABLE();
}

/*
 *  duk_known_xxx() helpers
 *
 *  Used internally when we're 100% sure that a certain index is valid and
 *  contains an object of a certain type.  For example, if we duk_push_object()
 *  we can then safely duk_known_hobject(ctx, -1).  These helpers just assert
 *  for the index and type, and if the assumptions are not valid, memory unsafe
 *  behavior happens.
 */

DUK_LOCAL duk_heaphdr *duk__known_heaphdr(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_heaphdr *h;

	DUK_ASSERT_CTX_VALID(ctx);
	if (idx < 0) {
		tv = thr->valstack_top + idx;
	} else {
		tv = thr->valstack_bottom + idx;
	}
	DUK_ASSERT(tv >= thr->valstack_bottom);
	DUK_ASSERT(tv < thr->valstack_top);
	h = DUK_TVAL_GET_HEAPHDR(tv);
	DUK_ASSERT(h != NULL);
	return h;
}

DUK_INTERNAL duk_hstring *duk_known_hstring(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT(duk_get_hstring(ctx, idx) != NULL);
	return (duk_hstring *) duk__known_heaphdr(ctx, idx);
}

DUK_INTERNAL duk_hobject *duk_known_hobject(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT(duk_get_hobject(ctx, idx) != NULL);
	return (duk_hobject *) duk__known_heaphdr(ctx, idx);
}

DUK_INTERNAL duk_hbuffer *duk_known_hbuffer(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT(duk_get_hbuffer(ctx, idx) != NULL);
	return (duk_hbuffer *) duk__known_heaphdr(ctx, idx);
}

DUK_INTERNAL duk_hcompfunc *duk_known_hcompfunc(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT(duk_get_hcompfunc(ctx, idx) != NULL);
	return (duk_hcompfunc *) duk__known_heaphdr(ctx, idx);
}

DUK_INTERNAL duk_hnatfunc *duk_known_hnatfunc(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT(duk_get_hnatfunc(ctx, idx) != NULL);
	return (duk_hnatfunc *) duk__known_heaphdr(ctx, idx);
}

DUK_EXTERNAL void duk_set_length(duk_context *ctx, duk_idx_t idx, duk_size_t len) {
	DUK_ASSERT_CTX_VALID(ctx);

	idx = duk_normalize_index(ctx, idx);
	duk_push_uint(ctx, (duk_uint_t) len);
	duk_put_prop_stridx(ctx, idx, DUK_STRIDX_LENGTH);
}

/*
 *  Conversions and coercions
 *
 *  The conversion/coercions are in-place operations on the value stack.
 *  Some operations are implemented here directly, while others call a
 *  helper in duk_js_ops.c after validating arguments.
 */

/* E5 Section 8.12.8 */

DUK_LOCAL duk_bool_t duk__defaultvalue_coerce_attempt(duk_context *ctx, duk_idx_t idx, duk_small_int_t func_stridx) {
	if (duk_get_prop_stridx(ctx, idx, func_stridx)) {
		/* [ ... func ] */
		if (duk_is_callable(ctx, -1)) {
			duk_dup(ctx, idx);         /* -> [ ... func this ] */
			duk_call_method(ctx, 0);     /* -> [ ... retval ] */
			if (duk_is_primitive(ctx, -1)) {
				duk_replace(ctx, idx);
				return 1;
			}
			/* [ ... retval ]; popped below */
		}
	}
	duk_pop(ctx);  /* [ ... func/retval ] -> [ ... ] */
	return 0;
}

DUK_EXTERNAL void duk_to_undefined(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	tv = duk_require_tval(ctx, idx);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_UNDEFINED_UPDREF(thr, tv);  /* side effects */
}

DUK_EXTERNAL void duk_to_null(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	tv = duk_require_tval(ctx, idx);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_NULL_UPDREF(thr, tv);  /* side effects */
}

/* E5 Section 9.1 */
DUK_EXTERNAL void duk_to_primitive(duk_context *ctx, duk_idx_t idx, duk_int_t hint) {
	duk_hthread *thr = (duk_hthread *) ctx;
	/* inline initializer for coercers[] is not allowed by old compilers like BCC */
	duk_small_int_t coercers[2];
	duk_small_uint_t class_number;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(hint == DUK_HINT_NONE || hint == DUK_HINT_NUMBER || hint == DUK_HINT_STRING);

	idx = duk_require_normalize_index(ctx, idx);

	if (!duk_check_type_mask(ctx, idx, DUK_TYPE_MASK_OBJECT |
	                                   DUK_TYPE_MASK_LIGHTFUNC |
	                                   DUK_TYPE_MASK_BUFFER)) {
		/* Any other values stay as is. */
		DUK_ASSERT(!duk_is_buffer(ctx, idx));  /* duk_to_string() relies on this behavior */
		return;
	}

	class_number = duk_get_class_number(ctx, idx);

	/* XXX: Symbol objects normally coerce via the ES2015-revised ToPrimitive()
	 * algorithm which consults value[@@toPrimitive] and avoids calling
	 * .valueOf() and .toString().  Before that is implemented, special
	 * case Symbol objects to behave as if they had the default @@toPrimitive
	 * algorithm of E6 Section 19.4.3.4, i.e. return the plain symbol value
	 * with no further side effects.
	 */

	if (class_number == DUK_HOBJECT_CLASS_SYMBOL) {
		duk_hobject *h_obj;
		duk_hstring *h_str;

		/* XXX: pretty awkward, index based API for internal value access? */
		h_obj = duk_known_hobject(ctx, idx);
		h_str = duk_hobject_get_internal_value_string(thr->heap, h_obj);
		if (h_str) {
			duk_push_hstring(ctx, h_str);
			duk_replace(ctx, idx);
			return;
		}
	}


	/* Objects are coerced based on E5 specification.
	 * Lightfuncs are coerced because they behave like
	 * objects even if they're internally a primitive
	 * type.  Same applies to plain buffers, which behave
	 * like ArrayBuffer objects since Duktape 2.x.
	 */

	coercers[0] = DUK_STRIDX_VALUE_OF;
	coercers[1] = DUK_STRIDX_TO_STRING;

	if (hint == DUK_HINT_NONE) {
		if (class_number == DUK_HOBJECT_CLASS_DATE) {
			hint = DUK_HINT_STRING;
		} else {
			hint = DUK_HINT_NUMBER;
		}
	}

	if (hint == DUK_HINT_STRING) {
		coercers[0] = DUK_STRIDX_TO_STRING;
		coercers[1] = DUK_STRIDX_VALUE_OF;
	}

	if (duk__defaultvalue_coerce_attempt(ctx, idx, coercers[0])) {
		DUK_ASSERT(!duk_is_buffer(ctx, idx));  /* duk_to_string() relies on this behavior */
		return;
	}

	if (duk__defaultvalue_coerce_attempt(ctx, idx, coercers[1])) {
		DUK_ASSERT(!duk_is_buffer(ctx, idx));  /* duk_to_string() relies on this behavior */
		return;
	}

	DUK_ERROR_TYPE(thr, DUK_STR_TOPRIMITIVE_FAILED);
}

/* E5 Section 9.2 */
DUK_EXTERNAL duk_bool_t duk_to_boolean(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_bool_t val;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	idx = duk_require_normalize_index(ctx, idx);
	tv = DUK_GET_TVAL_POSIDX(ctx, idx);
	DUK_ASSERT(tv != NULL);

	val = duk_js_toboolean(tv);
	DUK_ASSERT(val == 0 || val == 1);

	/* Note: no need to re-lookup tv, conversion is side effect free. */
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_BOOLEAN_UPDREF(thr, tv, val);  /* side effects */
	return val;
}

DUK_EXTERNAL duk_double_t duk_to_number(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_double_t d;

	DUK_ASSERT_CTX_VALID(ctx);

	/* XXX: No need to normalize; the whole operation could be inlined here to
	 * avoid 'tv' re-lookup.
	 */
	idx = duk_require_normalize_index(ctx, idx);
	tv = DUK_GET_TVAL_POSIDX(ctx, idx);
	DUK_ASSERT(tv != NULL);
	d = duk_js_tonumber(thr, tv);  /* XXX: fastint coercion? now result will always be a non-fastint */

	/* ToNumber() may have side effects so must relookup 'tv'. */
	tv = DUK_GET_TVAL_POSIDX(ctx, idx);
	DUK_TVAL_SET_NUMBER_UPDREF(thr, tv, d);  /* side effects */
	return d;
}

DUK_INTERNAL duk_double_t duk_to_number_m1(duk_context *ctx) {
	return duk_to_number(ctx, -1);
}
DUK_INTERNAL duk_double_t duk_to_number_m2(duk_context *ctx) {
	return duk_to_number(ctx, -2);
}

DUK_INTERNAL duk_double_t duk_to_number_tval(duk_context *ctx, duk_tval *tv) {
	duk_double_t res;

	DUK_ASSERT_CTX_VALID(ctx);

	duk_push_tval(ctx, tv);
	res = duk_to_number(ctx, -1);
	duk_pop(ctx);
	return res;
}

/* XXX: combine all the integer conversions: they share everything
 * but the helper function for coercion.
 */

typedef duk_double_t (*duk__toint_coercer)(duk_hthread *thr, duk_tval *tv);

DUK_LOCAL duk_double_t duk__to_int_uint_helper(duk_context *ctx, duk_idx_t idx, duk__toint_coercer coerce_func) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_double_t d;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, idx);
	DUK_ASSERT(tv != NULL);
	d = coerce_func(thr, tv);

	/* XXX: fastint? */

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, idx);
	DUK_TVAL_SET_NUMBER_UPDREF(thr, tv, d);  /* side effects */
	return d;
}

DUK_EXTERNAL duk_int_t duk_to_int(duk_context *ctx, duk_idx_t idx) {
	/* Value coercion (in stack): ToInteger(), E5 Section 9.4
	 * API return value coercion: custom
	 */
	DUK_ASSERT_CTX_VALID(ctx);
	(void) duk__to_int_uint_helper(ctx, idx, duk_js_tointeger);
	return (duk_int_t) duk__api_coerce_d2i(ctx, idx, 0 /*def_value*/, 0 /*require*/);
}

DUK_EXTERNAL duk_uint_t duk_to_uint(duk_context *ctx, duk_idx_t idx) {
	/* Value coercion (in stack): ToInteger(), E5 Section 9.4
	 * API return value coercion: custom
	 */
	DUK_ASSERT_CTX_VALID(ctx);
	(void) duk__to_int_uint_helper(ctx, idx, duk_js_tointeger);
	return (duk_uint_t) duk__api_coerce_d2ui(ctx, idx, 0 /*def_value*/, 0 /*require*/);
}

DUK_EXTERNAL duk_int32_t duk_to_int32(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_int32_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, idx);
	DUK_ASSERT(tv != NULL);
	ret = duk_js_toint32(thr, tv);

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, idx);
	DUK_TVAL_SET_I32_UPDREF(thr, tv, ret);  /* side effects */
	return ret;
}

DUK_EXTERNAL duk_uint32_t duk_to_uint32(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_uint32_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, idx);
	DUK_ASSERT(tv != NULL);
	ret = duk_js_touint32(thr, tv);

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, idx);
	DUK_TVAL_SET_U32_UPDREF(thr, tv, ret);  /* side effects */
	return ret;
}

DUK_EXTERNAL duk_uint16_t duk_to_uint16(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_uint16_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, idx);
	DUK_ASSERT(tv != NULL);
	ret = duk_js_touint16(thr, tv);

	/* Relookup in case coerce_func() has side effects, e.g. ends up coercing an object */
	tv = duk_require_tval(ctx, idx);
	DUK_TVAL_SET_U32_UPDREF(thr, tv, ret);  /* side effects */
	return ret;
}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
/* Special coercion for Uint8ClampedArray. */
DUK_INTERNAL duk_uint8_t duk_to_uint8clamped(duk_context *ctx, duk_idx_t idx) {
	duk_double_t d;
	duk_double_t t;
	duk_uint8_t ret;

	/* XXX: Simplify this algorithm, should be possible to come up with
	 * a shorter and faster algorithm by inspecting IEEE representation
	 * directly.
	 */

	d = duk_to_number(ctx, idx);
	if (d <= 0.0) {
		return 0;
	} else if (d >= 255) {
		return 255;
	} else if (DUK_ISNAN(d)) {
		/* Avoid NaN-to-integer coercion as it is compiler specific. */
		return 0;
	}

	t = d - DUK_FLOOR(d);
	if (t == 0.5) {
		/* Exact halfway, round to even. */
		ret = (duk_uint8_t) d;
		ret = (ret + 1) & 0xfe;  /* Example: d=3.5, t=0.5 -> ret = (3 + 1) & 0xfe = 4 & 0xfe = 4
		                          * Example: d=4.5, t=0.5 -> ret = (4 + 1) & 0xfe = 5 & 0xfe = 4
		                          */
	} else {
		/* Not halfway, round to nearest. */
		ret = (duk_uint8_t) (d + 0.5);
	}
	return ret;
}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */

DUK_EXTERNAL const char *duk_to_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len) {
	DUK_ASSERT_CTX_VALID(ctx);

	(void) duk_to_string(ctx, idx);
	DUK_ASSERT(duk_is_string(ctx, idx));
	return duk_require_lstring(ctx, idx, out_len);
}

DUK_LOCAL duk_ret_t duk__safe_to_string_raw(duk_context *ctx, void *udata) {
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(udata);

	duk_to_string(ctx, -1);
	return 1;
}

DUK_EXTERNAL const char *duk_safe_to_lstring(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len) {
	DUK_ASSERT_CTX_VALID(ctx);

	idx = duk_require_normalize_index(ctx, idx);

	/* We intentionally ignore the duk_safe_call() return value and only
	 * check the output type.  This way we don't also need to check that
	 * the returned value is indeed a string in the success case.
	 */

	duk_dup(ctx, idx);
	(void) duk_safe_call(ctx, duk__safe_to_string_raw, NULL /*udata*/, 1 /*nargs*/, 1 /*nrets*/);
	if (!duk_is_string(ctx, -1)) {
		/* Error: try coercing error to string once. */
		(void) duk_safe_call(ctx, duk__safe_to_string_raw, NULL /*udata*/, 1 /*nargs*/, 1 /*nrets*/);
		if (!duk_is_string(ctx, -1)) {
			/* Double error */
			duk_pop(ctx);
			duk_push_hstring_stridx(ctx, DUK_STRIDX_UC_ERROR);
		} else {
			;
		}
	} else {
		/* String; may be a symbol, accepted. */
		;
	}
	DUK_ASSERT(duk_is_string(ctx, -1));

	duk_replace(ctx, idx);
	DUK_ASSERT(duk_get_string(ctx, idx) != NULL);
	return duk_get_lstring(ctx, idx, out_len);
}

DUK_INTERNAL duk_hstring *duk_to_property_key_hstring(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);

	duk_to_primitive(ctx, idx, DUK_HINT_STRING);  /* needed for e.g. Symbol objects */
	h = duk_get_hstring(ctx, idx);
	if (h == NULL) {
		/* The "is string?" check may seem unnecessary, but as things
		 * are duk_to_hstring() invokes ToString() which fails for
		 * symbols.  But since symbols are already strings for Duktape
		 * C API, we check for that before doing the coercion.
		 */
		h = duk_to_hstring(ctx, idx);
	}
	DUK_ASSERT(h != NULL);
	return h;
}

#if defined(DUK_USE_DEBUGGER_SUPPORT)  /* only needed by debugger for now */
DUK_INTERNAL duk_hstring *duk_safe_to_hstring(duk_context *ctx, duk_idx_t idx) {
	(void) duk_safe_to_string(ctx, idx);
	DUK_ASSERT(duk_is_string(ctx, idx));
	DUK_ASSERT(duk_get_hstring(ctx, idx) != NULL);
	return duk_known_hstring(ctx, idx);
}
#endif

/* Push Object.prototype.toString() output for 'tv'. */
DUK_INTERNAL void duk_push_class_string_tval(duk_context *ctx, duk_tval *tv) {
	duk_hthread *thr;
	duk_small_uint_t stridx;
	duk_hstring *h_strclass;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK_UNREF(thr);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNUSED:  /* Treat like 'undefined', shouldn't happen. */
	case DUK_TAG_UNDEFINED: {
		stridx = DUK_STRIDX_UC_UNDEFINED;
		break;
	}
	case DUK_TAG_NULL: {
		stridx = DUK_STRIDX_UC_NULL;
		break;
	}
	case DUK_TAG_BOOLEAN: {
		stridx = DUK_STRIDX_UC_BOOLEAN;
		break;
	}
	case DUK_TAG_POINTER: {
		stridx = DUK_STRIDX_UC_POINTER;
		break;
	}
	case DUK_TAG_LIGHTFUNC: {
		stridx = DUK_STRIDX_UC_FUNCTION;
		break;
	}
	case DUK_TAG_STRING: {
		duk_hstring *h;
		h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);
		if (DUK_HSTRING_HAS_SYMBOL(h)) {
			stridx = DUK_STRIDX_UC_SYMBOL;
		} else {
			stridx = DUK_STRIDX_UC_STRING;
		}
		break;
	}
	case DUK_TAG_OBJECT: {
		duk_hobject *h;
		duk_small_uint_t classnum;

		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		classnum = DUK_HOBJECT_GET_CLASS_NUMBER(h);
		stridx = DUK_HOBJECT_CLASS_NUMBER_TO_STRIDX(classnum);

		/* XXX: This is not entirely correct anymore; in ES2015 the
		 * default lookup should use @@toStringTag to come up with
		 * e.g. [object Symbol], [object Uint8Array], etc.  See
		 * ES2015 Section 19.1.3.6.  The downside of implementing that
		 * directly is that the @@toStringTag lookup may have side
		 * effects, so all call sites must be checked for that.
		 * Some may need a side-effect free lookup, e.g. avoiding
		 * getters which are not typical.
		 */
		break;
	}
	case DUK_TAG_BUFFER: {
		stridx = DUK_STRIDX_UINT8_ARRAY;
		break;
	}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
		/* Fall through to generic number case. */
#endif
	default: {
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));  /* number (maybe fastint) */
		stridx = DUK_STRIDX_UC_NUMBER;
		break;
	}
	}
	h_strclass = DUK_HTHREAD_GET_STRING(thr, stridx);
	DUK_ASSERT(h_strclass != NULL);

	duk_push_sprintf(ctx, "[object %s]", (const char *) DUK_HSTRING_GET_DATA(h_strclass));
}

/* XXX: other variants like uint, u32 etc */
DUK_INTERNAL duk_int_t duk_to_int_clamped_raw(duk_context *ctx, duk_idx_t idx, duk_int_t minval, duk_int_t maxval, duk_bool_t *out_clamped) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_tmp;
	duk_double_t d, dmin, dmax;
	duk_int_t res;
	duk_bool_t clamped = 0;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_require_tval(ctx, idx);
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
	DUK_UNREF(d);  /* SCANBUILD: with suitable dmin/dmax limits 'd' is unused */
	/* 'd' and 'res' agree here */

	/* Relookup in case duk_js_tointeger() ends up e.g. coercing an object. */
	tv = duk_get_tval(ctx, idx);
	DUK_ASSERT(tv != NULL);  /* not popped by side effect */
	DUK_TVAL_SET_TVAL(&tv_tmp, tv);
#if defined(DUK_USE_FASTINT)
#if (DUK_INT_MAX <= 0x7fffffffL)
	DUK_TVAL_SET_I32(tv, res);
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
			DUK_ERROR_RANGE(thr, DUK_STR_NUMBER_OUTSIDE_RANGE);
		}
	}

	return res;
}

DUK_INTERNAL duk_int_t duk_to_int_clamped(duk_context *ctx, duk_idx_t idx, duk_idx_t minval, duk_idx_t maxval) {
	duk_bool_t dummy;
	return duk_to_int_clamped_raw(ctx, idx, minval, maxval, &dummy);
}

DUK_INTERNAL duk_int_t duk_to_int_check_range(duk_context *ctx, duk_idx_t idx, duk_int_t minval, duk_int_t maxval) {
	return duk_to_int_clamped_raw(ctx, idx, minval, maxval, NULL);  /* out_clamped==NULL -> RangeError if outside range */
}

DUK_EXTERNAL const char *duk_to_string(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	idx = duk_require_normalize_index(ctx, idx);
	tv = DUK_GET_TVAL_POSIDX(ctx, idx);
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
		/* Nop for actual strings, TypeError for Symbols.
		 * Because various internals rely on ToString() coercion of
		 * internal strings, -allow- (NOP) string coercion for hidden
		 * symbols.
		 */
#if 1
		duk_hstring *h;
		h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);
		if (DUK_HSTRING_HAS_SYMBOL(h)) {
			DUK_ERROR_TYPE((duk_hthread *) ctx, DUK_STR_CANNOT_STRING_COERCE_SYMBOL);
		} else {
			goto skip_replace;
		}
#else
		goto skip_replace;
#endif
	}
	case DUK_TAG_BUFFER: /* Go through Uint8Array.prototype.toString() for coercion. */
	case DUK_TAG_OBJECT: {
		/* Plain buffers: go through ArrayBuffer.prototype.toString()
		 * for coercion.
		 *
		 * Symbol objects: duk_to_primitive() results in a plain symbol
		 * value, and duk_to_string() then causes a TypeError.
		 */
		duk_to_primitive(ctx, idx, DUK_HINT_STRING);
		DUK_ASSERT(!duk_is_buffer(ctx, idx));  /* ToPrimitive() must guarantee */
		DUK_ASSERT(!duk_is_object(ctx, idx));
		return duk_to_string(ctx, idx);  /* Note: recursive call */
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
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv));
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		duk_push_tval(ctx, tv);
		duk_numconv_stringify(ctx,
		                      10 /*radix*/,
		                      0 /*precision:shortest*/,
		                      0 /*force_exponential*/);
		break;
	}
	}

	duk_replace(ctx, idx);

 skip_replace:
	DUK_ASSERT(duk_is_string(ctx, idx));
	return duk_require_string(ctx, idx);
}

DUK_INTERNAL duk_hstring *duk_to_hstring(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *ret;
	DUK_ASSERT_CTX_VALID(ctx);
	duk_to_string(ctx, idx);
	ret = duk_get_hstring(ctx, idx);
	DUK_ASSERT(ret != NULL);
	return ret;
}

DUK_INTERNAL duk_hstring *duk_to_hstring_m1(duk_context *ctx) {
	return duk_to_hstring(ctx, -1);
}

DUK_INTERNAL duk_hstring *duk_to_hstring_acceptsymbol(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *ret;
	DUK_ASSERT_CTX_VALID(ctx);
	ret = duk_get_hstring(ctx, idx);
	if (ret && DUK_HSTRING_HAS_SYMBOL(ret)) {
		return ret;
	}
	return duk_to_hstring(ctx, idx);
}

/* Convert a plain buffer or any buffer object into a string, using the buffer
 * bytes 1:1 in the internal string representation.  For views the active byte
 * slice (not element slice interpreted as an initializer) is used.  This is
 * necessary in Duktape 2.x because ToString(plainBuffer) no longer creates a
 * string with the same bytes as in the buffer but rather (usually)
 * '[object ArrayBuffer]'.
 */
DUK_EXTERNAL const char *duk_buffer_to_string(duk_context *ctx, duk_idx_t idx) {
	void *ptr_src;
	duk_size_t len;
	const char *res;

	idx = duk_require_normalize_index(ctx, idx);

	ptr_src = duk_require_buffer_data(ctx, idx, &len);
	DUK_ASSERT(ptr_src != NULL || len == 0);

	res = duk_push_lstring(ctx, (const char *) ptr_src, len);
	duk_replace(ctx, idx);
	return res;
}

DUK_EXTERNAL void *duk_to_buffer_raw(duk_context *ctx, duk_idx_t idx, duk_size_t *out_size, duk_uint_t mode) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbuffer *h_buf;
	const duk_uint8_t *src_data;
	duk_size_t src_size;
	duk_uint8_t *dst_data;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_UNREF(thr);

	idx = duk_require_normalize_index(ctx, idx);

	h_buf = duk_get_hbuffer(ctx, idx);
	if (h_buf != NULL) {
		/* Buffer is kept as is, with the fixed/dynamic nature of the
		 * buffer only changed if requested.  An external buffer
		 * is converted into a non-external dynamic buffer in a
		 * duk_to_dynamic_buffer() call.
		 */
		duk_uint_t tmp;
		duk_uint8_t *tmp_ptr;

		tmp_ptr = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h_buf);
		src_data = (const duk_uint8_t *) tmp_ptr;
		src_size = DUK_HBUFFER_GET_SIZE(h_buf);

		tmp = (DUK_HBUFFER_HAS_DYNAMIC(h_buf) ? DUK_BUF_MODE_DYNAMIC : DUK_BUF_MODE_FIXED);
		if ((tmp == mode && !DUK_HBUFFER_HAS_EXTERNAL(h_buf)) ||
		    mode == DUK_BUF_MODE_DONTCARE) {
			/* Note: src_data may be NULL if input is a zero-size
			 * dynamic buffer.
			 */
			dst_data = tmp_ptr;
			goto skip_copy;
		}
	} else {
		/* Non-buffer value is first ToString() coerced, then converted
		 * to a buffer (fixed buffer is used unless a dynamic buffer is
		 * explicitly requested).  Symbols are rejected with a TypeError.
		 * XXX: C API could maybe allow symbol-to-buffer coercion?
		 */
		src_data = (const duk_uint8_t *) duk_to_lstring(ctx, idx, &src_size);
	}

	dst_data = (duk_uint8_t *) duk_push_buffer(ctx, src_size, (mode == DUK_BUF_MODE_DYNAMIC) /*dynamic*/);
	if (DUK_LIKELY(src_size > 0)) {
		/* When src_size == 0, src_data may be NULL (if source
		 * buffer is dynamic), and dst_data may be NULL (if
		 * target buffer is dynamic).  Avoid zero-size memcpy()
		 * with an invalid pointer.
		 */
		DUK_MEMCPY((void *) dst_data, (const void *) src_data, (size_t) src_size);
	}
	duk_replace(ctx, idx);
 skip_copy:

	if (out_size) {
		*out_size = src_size;
	}
	return dst_data;
}

DUK_EXTERNAL void *duk_to_pointer(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;
	void *res;

	DUK_ASSERT_CTX_VALID(ctx);

	idx = duk_require_normalize_index(ctx, idx);
	tv = DUK_GET_TVAL_POSIDX(ctx, idx);
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
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv));
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		res = NULL;
		break;
	}

	duk_push_pointer(ctx, res);
	duk_replace(ctx, idx);
	return res;
}

DUK_LOCAL void duk__push_func_from_lightfunc(duk_context *ctx, duk_c_function func, duk_small_uint_t lf_flags) {
	duk_idx_t nargs;
	duk_uint_t flags = 0;   /* shared flags for a subset of types */
	duk_small_uint_t lf_len;
	duk_hnatfunc *nf;

	nargs = (duk_idx_t) DUK_LFUNC_FLAGS_GET_NARGS(lf_flags);
	if (nargs == DUK_LFUNC_NARGS_VARARGS) {
		nargs = (duk_idx_t) DUK_VARARGS;
	}

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	        DUK_HOBJECT_FLAG_NATFUNC |
	        DUK_HOBJECT_FLAG_NEWENV |
	        DUK_HOBJECT_FLAG_STRICT |
	        DUK_HOBJECT_FLAG_NOTAIL |
	        /* DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC: omitted here intentionally */
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);
	(void) duk__push_c_function_raw(ctx, func, nargs, flags);

	lf_len = DUK_LFUNC_FLAGS_GET_LENGTH(lf_flags);
	if ((duk_idx_t) lf_len != nargs) {
		/* Explicit length is only needed if it differs from 'nargs'. */
		duk_push_int(ctx, (duk_int_t) lf_len);
		duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_NONE);
	}

#if defined(DUK_USE_FUNC_NAME_PROPERTY)
	duk_push_lightfunc_name_raw(ctx, func, lf_flags);
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_C);
#endif

	nf = duk_known_hnatfunc(ctx, -1);
	nf->magic = (duk_int16_t) DUK_LFUNC_FLAGS_GET_MAGIC(lf_flags);

	/* Enable DUKFUNC exotic behavior once properties are set up. */
	DUK_HOBJECT_SET_EXOTIC_DUKFUNC((duk_hobject *) nf);
}

DUK_EXTERNAL void duk_to_object(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_uint_t flags = 0;   /* shared flags for a subset of types */
	duk_small_int_t proto = 0;

	DUK_ASSERT_CTX_VALID(ctx);

	idx = duk_require_normalize_index(ctx, idx);
	tv = DUK_GET_TVAL_POSIDX(ctx, idx);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
#if !defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_TAG_BUFFER:  /* With no bufferobject support, don't object coerce. */
#endif
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL: {
		DUK_ERROR_TYPE(thr, DUK_STR_NOT_OBJECT_COERCIBLE);
		break;
	}
	case DUK_TAG_BOOLEAN: {
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BOOLEAN);
		proto = DUK_BIDX_BOOLEAN_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_STRING: {
		duk_hstring *h;
		h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);
		if (DUK_HSTRING_HAS_SYMBOL(h)) {
			flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
			        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_SYMBOL);
			proto = DUK_BIDX_SYMBOL_PROTOTYPE;
		} else {
			flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
			        DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ |
			        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_STRING);
			proto = DUK_BIDX_STRING_PROTOTYPE;
		}
		goto create_object;
	}
	case DUK_TAG_OBJECT: {
		/* nop */
		break;
	}
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_TAG_BUFFER: {
		/* A plain buffer object coerces to a full ArrayBuffer which
		 * is not fully transparent behavior (ToObject() should be a
		 * nop for an object).  This behavior matches lightfuncs which
		 * also coerce to an equivalent Function object.  There are
		 * also downsides to defining ToObject(plainBuffer) as a no-op;
		 * for example duk_to_hobject() could result in a NULL pointer.
		 */
		duk_hbuffer *h_buf;

		h_buf = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h_buf != NULL);
		duk_hbufobj_push_uint8array_from_plain(thr, h_buf);
		goto replace_value;
	}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */
	case DUK_TAG_POINTER: {
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_POINTER);
		proto = DUK_BIDX_POINTER_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_LIGHTFUNC: {
		/* Lightfunc coerces to a Function instance with concrete
		 * properties.  Since 'length' is virtual for Duktape/C
		 * functions, don't need to define that.  The result is made
		 * extensible to mimic what happens to strings in object
		 * coercion:
		 *
		 *   > Object.isExtensible(Object('foo'))
		 *   true
		 */
		duk_small_uint_t lf_flags;
		duk_c_function func;

		DUK_TVAL_GET_LIGHTFUNC(tv, func, lf_flags);
		duk__push_func_from_lightfunc(ctx, func, lf_flags);
		goto replace_value;
	}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default: {
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv));
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_NUMBER);
		proto = DUK_BIDX_NUMBER_PROTOTYPE;
		goto create_object;
	}
	}
	DUK_ASSERT(duk_is_object(ctx, idx));
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
	duk_dup(ctx, idx);
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);

 replace_value:
	duk_replace(ctx, idx);
	DUK_ASSERT(duk_is_object(ctx, idx));
}

DUK_INTERNAL duk_hobject *duk_to_hobject(duk_context *ctx, duk_idx_t idx) {
	duk_hobject *ret;
	DUK_ASSERT_CTX_VALID(ctx);
	duk_to_object(ctx, idx);
	ret = duk_known_hobject(ctx, idx);
	return ret;
}

/*
 *  Type checking
 */

DUK_LOCAL duk_bool_t duk__tag_check(duk_context *ctx, duk_idx_t idx, duk_small_uint_t tag) {
	duk_tval *tv;

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	return (DUK_TVAL_GET_TAG(tv) == tag);
}

DUK_LOCAL duk_bool_t duk__obj_flag_any_default_false(duk_context *ctx, duk_idx_t idx, duk_uint_t flag_mask) {
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_get_hobject(ctx, idx);
	if (obj) {
		return (DUK_HEAPHDR_CHECK_FLAG_BITS((duk_heaphdr *) obj, flag_mask) ? 1 : 0);
	}
	return 0;
}

DUK_INTERNAL duk_int_t duk_get_type_tval(duk_tval *tv) {
	DUK_ASSERT(tv != NULL);

#if defined(DUK_USE_PACKED_TVAL)
	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNUSED:
		return DUK_TYPE_NONE;
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
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv));
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return DUK_TYPE_NUMBER;
	}
#else  /* DUK_USE_PACKED_TVAL */
	DUK_ASSERT(DUK_TVAL_IS_VALID_TAG(tv));
	DUK_ASSERT(sizeof(duk__type_from_tag) / sizeof(duk_uint_t) == DUK_TAG_MAX - DUK_TAG_MIN + 1);
	return (duk_int_t) duk__type_from_tag[DUK_TVAL_GET_TAG(tv) - DUK_TAG_MIN];
#endif  /* DUK_USE_PACKED_TVAL */
}

DUK_EXTERNAL duk_int_t duk_get_type(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

	return duk_get_type_tval(tv);
}

#if defined(DUK_USE_VERBOSE_ERRORS) && defined(DUK_USE_PARANOID_ERRORS)
DUK_LOCAL const char *duk__type_names[] = {
	"none",
	"undefined",
	"null",
	"boolean",
	"number",
	"string",
	"object",
	"buffer",
	"pointer",
	"lightfunc"
};

DUK_INTERNAL const char *duk_get_type_name(duk_context *ctx, duk_idx_t idx) {
	duk_int_t type_tag;

	type_tag = duk_get_type(ctx, idx);
	DUK_ASSERT(type_tag >= DUK_TYPE_MIN && type_tag <= DUK_TYPE_MAX);
	DUK_ASSERT(DUK_TYPE_MIN == 0 && sizeof(duk__type_names) / sizeof(const char *) == DUK_TYPE_MAX + 1);

	return duk__type_names[type_tag];
}
#endif

DUK_INTERNAL duk_small_uint_t duk_get_class_number(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;
	duk_hobject *obj;

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_OBJECT:
		obj = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(obj != NULL);
		return DUK_HOBJECT_GET_CLASS_NUMBER(obj);
	case DUK_TAG_BUFFER:
		/* Buffers behave like Uint8Array objects. */
		return DUK_HOBJECT_CLASS_UINT8ARRAY;
	case DUK_TAG_LIGHTFUNC:
		/* Lightfuncs behave like Function objects. */
		return DUK_HOBJECT_CLASS_FUNCTION;
	default:
		/* Primitive or UNUSED, no class number. */
		return DUK_HOBJECT_CLASS_NONE;
	}
}

DUK_EXTERNAL duk_bool_t duk_check_type(duk_context *ctx, duk_idx_t idx, duk_int_t type) {
	DUK_ASSERT_CTX_VALID(ctx);

	return (duk_get_type(ctx, idx) == type) ? 1 : 0;
}

DUK_INTERNAL duk_uint_t duk_get_type_mask_tval(duk_tval *tv) {
	DUK_ASSERT(tv != NULL);

#if defined(DUK_USE_PACKED_TVAL)
	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNUSED:
		return DUK_TYPE_MASK_NONE;
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
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv));
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return DUK_TYPE_MASK_NUMBER;
	}
#else  /* DUK_USE_PACKED_TVAL */
	DUK_ASSERT(DUK_TVAL_IS_VALID_TAG(tv));
	DUK_ASSERT(sizeof(duk__type_mask_from_tag) / sizeof(duk_uint_t) == DUK_TAG_MAX - DUK_TAG_MIN + 1);
	return (duk_int_t) duk__type_mask_from_tag[DUK_TVAL_GET_TAG(tv) - DUK_TAG_MIN];
#endif  /* DUK_USE_PACKED_TVAL */
}

DUK_EXTERNAL duk_uint_t duk_get_type_mask(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

	return duk_get_type_mask_tval(tv);
}

DUK_EXTERNAL duk_bool_t duk_check_type_mask(duk_context *ctx, duk_idx_t idx, duk_uint_t mask) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	if (duk_get_type_mask(ctx, idx) & mask) {
		return 1;
	}
	if (mask & DUK_TYPE_MASK_THROW) {
		DUK_ERROR_TYPE(thr, DUK_STR_UNEXPECTED_TYPE);
		DUK_UNREACHABLE();
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_undefined(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_UNDEFINED);
}

DUK_EXTERNAL duk_bool_t duk_is_null(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_NULL);
}

DUK_EXTERNAL duk_bool_t duk_is_boolean(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_BOOLEAN);
}

DUK_EXTERNAL duk_bool_t duk_is_number(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	/*
	 *  Number is special because it doesn't have a specific
	 *  tag in the 8-byte representation.
	 */

	/* XXX: shorter version for unpacked representation? */

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	return DUK_TVAL_IS_NUMBER(tv);
}

DUK_EXTERNAL duk_bool_t duk_is_nan(duk_context *ctx, duk_idx_t idx) {
	/* XXX: This will now return false for non-numbers, even though they would
	 * coerce to NaN (as a general rule).  In particular, duk_get_number()
	 * returns a NaN for non-numbers, so should this function also return
	 * true for non-numbers?
	 */

	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);

	/* XXX: for packed duk_tval an explicit "is number" check is unnecessary */
	if (!DUK_TVAL_IS_NUMBER(tv)) {
		return 0;
	}
	return DUK_ISNAN(DUK_TVAL_GET_NUMBER(tv));
}

DUK_EXTERNAL duk_bool_t duk_is_string(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_STRING);
}

DUK_INTERNAL duk_bool_t duk_is_string_notsymbol(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk_get_hstring_notsymbol(ctx, idx) != NULL;
}

DUK_EXTERNAL duk_bool_t duk_is_object(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_OBJECT);
}

DUK_EXTERNAL duk_bool_t duk_is_buffer(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_BUFFER);
}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_EXTERNAL duk_bool_t duk_is_buffer_data(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		return 1;
	} else if (DUK_TVAL_IS_OBJECT(tv)) {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		if (DUK_HOBJECT_IS_BUFOBJ(h)) {
			return 1;
		}
	}
	return 0;
}
#else  /* DUK_USE_BUFFEROBJECT_SUPPORT */
DUK_EXTERNAL duk_bool_t duk_is_buffer_data(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);

	return duk_is_buffer(ctx, idx);
}

#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */

DUK_EXTERNAL duk_bool_t duk_is_pointer(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_POINTER);
}

DUK_EXTERNAL duk_bool_t duk_is_lightfunc(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__tag_check(ctx, idx, DUK_TAG_LIGHTFUNC);
}

DUK_EXTERNAL duk_bool_t duk_is_symbol(duk_context *ctx, duk_idx_t idx) {
	duk_hstring *h;

	DUK_ASSERT_CTX_VALID(ctx);
	h = duk_get_hstring(ctx, idx);
	if (h != NULL && DUK_HSTRING_HAS_SYMBOL(h)) {
		return 1;
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_array(duk_context *ctx, duk_idx_t idx) {
	duk_hobject *obj;

	DUK_ASSERT_CTX_VALID(ctx);

	obj = duk_get_hobject(ctx, idx);
	if (obj) {
		return (DUK_HOBJECT_GET_CLASS_NUMBER(obj) == DUK_HOBJECT_CLASS_ARRAY ? 1 : 0);
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_function(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	if (DUK_TVAL_IS_LIGHTFUNC(tv)) {
		return 1;
	}
	return duk__obj_flag_any_default_false(ctx,
	                                       idx,
	                                       DUK_HOBJECT_FLAG_COMPFUNC |
	                                       DUK_HOBJECT_FLAG_NATFUNC |
	                                       DUK_HOBJECT_FLAG_BOUNDFUNC);
}

DUK_EXTERNAL duk_bool_t duk_is_c_function(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       idx,
	                                       DUK_HOBJECT_FLAG_NATFUNC);
}

DUK_EXTERNAL duk_bool_t duk_is_ecmascript_function(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       idx,
	                                       DUK_HOBJECT_FLAG_COMPFUNC);
}

DUK_EXTERNAL duk_bool_t duk_is_bound_function(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       idx,
	                                       DUK_HOBJECT_FLAG_BOUNDFUNC);
}

DUK_EXTERNAL duk_bool_t duk_is_thread(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__obj_flag_any_default_false(ctx,
	                                       idx,
	                                       DUK_HOBJECT_FLAG_THREAD);
}

DUK_EXTERNAL duk_bool_t duk_is_fixed_buffer(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_HAS_DYNAMIC(h) ? 0 : 1);
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_dynamic_buffer(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_HAS_DYNAMIC(h) && !DUK_HBUFFER_HAS_EXTERNAL(h) ? 1 : 0);
	}
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_is_external_buffer(duk_context *ctx, duk_idx_t idx) {
	duk_tval *tv;

	DUK_ASSERT_CTX_VALID(ctx);

	tv = duk_get_tval_or_unused(ctx, idx);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_HAS_DYNAMIC(h) && DUK_HBUFFER_HAS_EXTERNAL(h) ? 1 : 0);
	}
	return 0;
}

DUK_EXTERNAL duk_errcode_t duk_get_error_code(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_uint_t sanity;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_get_hobject(ctx, idx);

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

DUK_EXTERNAL void duk_push_undefined(duk_context *ctx) {
	duk_hthread *thr;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();

	/* Because value stack init policy is 'undefined above top',
	 * we don't need to write, just assert.
	 */
	thr->valstack_top++;
	DUK_ASSERT(DUK_TVAL_IS_UNDEFINED(thr->valstack_top - 1));
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
	DUK_TVAL_SET_I32(tv_slot, (duk_int32_t) val);
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
	DUK_TVAL_SET_U32(tv_slot, (duk_uint32_t) val);
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
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
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
		DUK_ERROR_RANGE(thr, DUK_STR_STRING_TOO_LONG);
	}

	h = duk_heap_string_intern_checked(thr, (const duk_uint8_t *) str, (duk_uint32_t) len);
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

DUK_EXTERNAL void duk_push_pointer(duk_context *ctx, void *val) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK__CHECK_SPACE();
	tv_slot = thr->valstack_top++;
	DUK_TVAL_SET_POINTER(tv_slot, val);
}

DUK_INTERNAL duk_hstring *duk_push_uint_to_hstring(duk_context *ctx, duk_uint_t i) {
	duk_hstring *h_tmp;

	/* XXX: this could be a direct DUK_SPRINTF to a buffer followed by duk_push_string() */
	duk_push_uint(ctx, (duk_uint_t) i);
	h_tmp = duk_to_hstring_m1(ctx);
	DUK_ASSERT(h_tmp != NULL);
	return h_tmp;
}

DUK_LOCAL void duk__push_this_helper(duk_context *ctx, duk_small_uint_t check_object_coercible) {
	duk_hthread *thr;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);  /* avoid warning (unsigned) */
	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr->callstack_top <= thr->callstack_size);
	DUK__CHECK_SPACE();

	DUK_ASSERT(DUK_TVAL_IS_UNDEFINED(thr->valstack_top));  /* because of valstack init policy */
	tv_slot = thr->valstack_top++;

	if (DUK_UNLIKELY(thr->callstack_top == 0)) {
		if (check_object_coercible) {
			goto type_error;
		}
		/* 'undefined' already on stack top */
	} else {
		duk_tval *tv;

		/* 'this' binding is just before current activation's bottom */
		DUK_ASSERT(thr->valstack_bottom > thr->valstack);
		tv = thr->valstack_bottom - 1;
		if (check_object_coercible &&
		    (DUK_TVAL_IS_UNDEFINED(tv) || DUK_TVAL_IS_NULL(tv))) {
			/* XXX: better macro for DUK_TVAL_IS_UNDEFINED_OR_NULL(tv) */
			goto type_error;
		}

		DUK_TVAL_SET_TVAL(tv_slot, tv);
		DUK_TVAL_INCREF(thr, tv);
	}
	return;

 type_error:
	DUK_ERROR_TYPE(thr, DUK_STR_NOT_OBJECT_COERCIBLE);
}

DUK_EXTERNAL void duk_push_this(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, 0 /*check_object_coercible*/);
}

DUK_INTERNAL void duk_push_this_check_object_coercible(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, 1 /*check_object_coercible*/);
}

DUK_INTERNAL duk_hobject *duk_push_this_coercible_to_object(duk_context *ctx) {
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, 1 /*check_object_coercible*/);
	h = duk_to_hobject(ctx, -1);
	DUK_ASSERT(h != NULL);
	return h;
}

DUK_INTERNAL duk_hstring *duk_push_this_coercible_to_string(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk__push_this_helper(ctx, 1 /*check_object_coercible*/);
	return duk_to_hstring_m1(ctx);  /* This will reject all Symbol values; accepts Symbol objects. */
}

DUK_INTERNAL duk_tval *duk_get_borrowed_this_tval(duk_context *ctx) {
	duk_hthread *thr;

	DUK_ASSERT(ctx != NULL);
	thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr->callstack_top > 0);  /* caller required to know */
	DUK_ASSERT(thr->valstack_bottom > thr->valstack);  /* consequence of above */
	DUK_ASSERT(thr->valstack_bottom - 1 >= thr->valstack);  /* 'this' binding exists */

	return thr->valstack_bottom - 1;
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
	if (!duk_get_prop_stridx_short(ctx, -1, DUK_STRIDX_INT_VALUE)) {
		DUK_DDD(DUK_DDDPRINT("creating heap/global/thread stash on first use"));
		duk_pop(ctx);
		duk_push_bare_object(ctx);
		duk_dup_top(ctx);
		duk_xdef_prop_stridx_short(ctx, -3, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_C);  /* [ ... parent stash stash ] -> [ ... parent stash ] */
	}
	duk_remove_m2(ctx);
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
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
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
		duk_push_hstring_empty(ctx);
		h_str = duk_known_hstring(ctx, -1);
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
			DUK_ERROR_RANGE(thr, DUK_STR_RESULT_TOO_LONG);
		}
	}

	/* Cannot use duk_buffer_to_string() on the buffer because it is
	 * usually larger than 'len'; 'buf' is also usually a stack buffer.
	 */
	res = duk_push_lstring(ctx, (const char *) buf, (duk_size_t) len);  /* [ buf? res ] */
	if (pushed_buf) {
		duk_remove_m2(ctx);
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

DUK_INTERNAL duk_hobject *duk_push_object_helper(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_small_int_t prototype_bidx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_slot;
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(prototype_bidx == -1 ||
	           (prototype_bidx >= 0 && prototype_bidx < DUK_NUM_BUILTINS));

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
	}

	h = duk_hobject_alloc(thr->heap, hobject_flags_and_class);
	if (!h) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}

	DUK_DDD(DUK_DDDPRINT("created object with flags: 0x%08lx", (unsigned long) h->hdr.h_flags));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, h);
	DUK_HOBJECT_INCREF(thr, h);  /* no side effects */
	thr->valstack_top++;

	/* object is now reachable */

	if (prototype_bidx >= 0) {
		DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, thr->builtins[prototype_bidx]);
	} else {
		DUK_ASSERT(prototype_bidx == -1);
		DUK_ASSERT(DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h) == NULL);
	}

	return h;
}

DUK_INTERNAL duk_hobject *duk_push_object_helper_proto(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_hobject *proto) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;

	DUK_ASSERT_CTX_VALID(ctx);

	h = duk_push_object_helper(ctx, hobject_flags_and_class, -1);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h) == NULL);
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, proto);
	return h;
}

DUK_EXTERNAL duk_idx_t duk_push_object(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	(void) duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              DUK_BIDX_OBJECT_PROTOTYPE);
	return duk_get_top_index_unsafe(ctx);
}

DUK_EXTERNAL duk_idx_t duk_push_array(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uint_t flags;
	duk_harray *obj;
	duk_idx_t ret;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_ARRAY_PART |
	        DUK_HOBJECT_FLAG_EXOTIC_ARRAY |
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ARRAY);

	obj = duk_harray_alloc(thr->heap, flags);
	if (!obj) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}

	/* XXX: since prototype is NULL, could save a check */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, thr->builtins[DUK_BIDX_ARRAY_PROTOTYPE]);

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HOBJECT_INCREF(thr, obj);  /* XXX: could preallocate with refcount = 1 */
	ret = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	DUK_ASSERT(obj->length == 0);  /* Array .length starts at zero. */
	return ret;
}

DUK_INTERNAL duk_harray *duk_push_harray(duk_context *ctx) {
	/* XXX: API call could do this directly, cast to void in API macro. */
	duk_hthread *thr;
	duk_harray *a;

	thr = (duk_hthread *) ctx;
	(void) duk_push_array(ctx);
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(thr->valstack_top - 1));
	a = (duk_harray *) DUK_TVAL_GET_OBJECT(thr->valstack_top - 1);
	DUK_ASSERT(a != NULL);
	return a;
}

/* Push a duk_harray with preallocated size (.length also set to match size).
 * Caller may then populate array part of the duk_harray directly.
 */
DUK_INTERNAL duk_harray *duk_push_harray_with_size(duk_context *ctx, duk_uint32_t size) {
	duk_harray *a;

	a = duk_push_harray(ctx);

	duk_hobject_realloc_props((duk_hthread *) ctx,
	                          (duk_hobject *) a,
	                          0,
	                          size,
	                          0,
	                          0);
	a->length = size;
	return a;
}

DUK_EXTERNAL duk_idx_t duk_push_thread_raw(duk_context *ctx, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hthread *obj;
	duk_idx_t ret;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
	}

	obj = duk_hthread_alloc(thr->heap,
	                        DUK_HOBJECT_FLAG_EXTENSIBLE |
	                        DUK_HOBJECT_FLAG_THREAD |
	                        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_THREAD));
	if (!obj) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}
	obj->state = DUK_HTHREAD_STATE_INACTIVE;
#if defined(DUK_USE_ROM_STRINGS)
	/* Nothing to initialize, strs[] is in ROM. */
#else
#if defined(DUK_USE_HEAPPTR16)
	obj->strs16 = thr->strs16;
#else
	obj->strs = thr->strs;
#endif
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
		DUK_ERROR_ALLOC_FAILED(thr);
	}

	/* initialize built-ins - either by copying or creating new ones */
	if (flags & DUK_THREAD_NEW_GLOBAL_ENV) {
		duk_hthread_create_builtin_objects(obj);
	} else {
		duk_hthread_copy_builtin_objects(thr, obj);
	}

	/* default prototype (Note: 'obj' must be reachable) */
	/* XXX: since prototype is NULL, could save a check */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, obj->builtins[DUK_BIDX_THREAD_PROTOTYPE]);

	/* Initial stack size satisfies the stack spare constraints so there
	 * is no need to require stack here.
	 */
	DUK_ASSERT(DUK_VALSTACK_INITIAL_SIZE >=
	           DUK_VALSTACK_API_ENTRY_MINIMUM + DUK_VALSTACK_INTERNAL_EXTRA);

	return ret;
}

DUK_INTERNAL duk_hcompfunc *duk_push_hcompfunc(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hcompfunc *obj;
	duk_tval *tv_slot;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
	}

	/* Template functions are not strictly constructable (they don't
	 * have a "prototype" property for instance), so leave the
	 * DUK_HOBJECT_FLAG_CONSRUCTABLE flag cleared here.
	 */

	obj = duk_hcompfunc_alloc(thr->heap,
	                          DUK_HOBJECT_FLAG_EXTENSIBLE |
	                          DUK_HOBJECT_FLAG_COMPFUNC |
	                          DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION));
	if (!obj) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}

	DUK_DDD(DUK_DDDPRINT("created compiled function object with flags: 0x%08lx", (unsigned long) obj->obj.hdr.h_flags));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HOBJECT_INCREF(thr, obj);
	thr->valstack_top++;

	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE]);

	return obj;
}

DUK_LOCAL duk_idx_t duk__push_c_function_raw(duk_context *ctx, duk_c_function func, duk_idx_t nargs, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hnatfunc *obj;
	duk_idx_t ret;
	duk_tval *tv_slot;
	duk_int16_t func_nargs;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
	}
	if (func == NULL) {
		goto api_error;
	}
	if (nargs >= 0 && nargs < DUK_HNATFUNC_NARGS_MAX) {
		func_nargs = (duk_int16_t) nargs;
	} else if (nargs == DUK_VARARGS) {
		func_nargs = DUK_HNATFUNC_NARGS_VARARGS;
	} else {
		goto api_error;
	}

	obj = duk_hnatfunc_alloc(thr->heap, flags);
	if (!obj) {
		DUK_ERROR_ALLOC_FAILED(thr);
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
	DUK_ERROR_TYPE_INVALID_ARGS(thr);
	return 0;  /* not reached */
}

DUK_EXTERNAL duk_idx_t duk_push_c_function(duk_context *ctx, duk_c_function func, duk_int_t nargs) {
	duk_uint_t flags;

	DUK_ASSERT_CTX_VALID(ctx);

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	        DUK_HOBJECT_FLAG_NATFUNC |
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
	        DUK_HOBJECT_FLAG_NATFUNC |
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
	        DUK_HOBJECT_FLAG_NATFUNC |
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
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
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
	DUK_ERROR_TYPE_INVALID_ARGS(thr);
	return 0;  /* not reached */
}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_INTERNAL duk_hbufobj *duk_push_bufobj_raw(duk_context *ctx, duk_uint_t hobject_flags_and_class, duk_small_int_t prototype_bidx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hbufobj *obj;
	duk_tval *tv_slot;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(prototype_bidx >= 0);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
	}

	obj = duk_hbufobj_alloc(thr->heap, hobject_flags_and_class);
	if (!obj) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}

	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, thr->builtins[prototype_bidx]);
	DUK_ASSERT_HBUFOBJ_VALID(obj);

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HOBJECT_INCREF(thr, obj);
	thr->valstack_top++;

	return obj;
}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */

/* XXX: There's quite a bit of overlap with buffer creation handling in
 * duk_bi_buffer.c.  Look for overlap and refactor.
 */
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
#define DUK__PACK_ARGS(classnum,protobidx,elemtype,elemshift,istypedarray) \
	(((classnum) << 24) | ((protobidx) << 16) | ((elemtype) << 8) | ((elemshift) << 4) | (istypedarray))

static const duk_uint32_t duk__bufobj_flags_lookup[] = {
	/* Node.js Buffers are Uint8Array instances which inherit from Buffer.prototype. */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_ARRAYBUFFER,       DUK_BIDX_ARRAYBUFFER_PROTOTYPE,       DUK_HBUFOBJ_ELEM_UINT8,        0, 0),  /* DUK_BUFOBJ_ARRAYBUFFER */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_UINT8ARRAY,        DUK_BIDX_NODEJS_BUFFER_PROTOTYPE,     DUK_HBUFOBJ_ELEM_UINT8,        0, 1),  /* DUK_BUFOBJ_NODEJS_BUFFER */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_DATAVIEW,          DUK_BIDX_DATAVIEW_PROTOTYPE,          DUK_HBUFOBJ_ELEM_UINT8,        0, 0),  /* DUK_BUFOBJ_DATAVIEW */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_INT8ARRAY,         DUK_BIDX_INT8ARRAY_PROTOTYPE,         DUK_HBUFOBJ_ELEM_INT8,         0, 1),  /* DUK_BUFOBJ_INT8ARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_UINT8ARRAY,        DUK_BIDX_UINT8ARRAY_PROTOTYPE,        DUK_HBUFOBJ_ELEM_UINT8,        0, 1),  /* DUK_BUFOBJ_UINT8ARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_UINT8CLAMPEDARRAY, DUK_BIDX_UINT8CLAMPEDARRAY_PROTOTYPE, DUK_HBUFOBJ_ELEM_UINT8CLAMPED, 0, 1),  /* DUK_BUFOBJ_UINT8CLAMPEDARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_INT16ARRAY,        DUK_BIDX_INT16ARRAY_PROTOTYPE,        DUK_HBUFOBJ_ELEM_INT16,        1, 1),  /* DUK_BUFOBJ_INT16ARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_UINT16ARRAY,       DUK_BIDX_UINT16ARRAY_PROTOTYPE,       DUK_HBUFOBJ_ELEM_UINT16,       1, 1),  /* DUK_BUFOBJ_UINT16ARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_INT32ARRAY,        DUK_BIDX_INT32ARRAY_PROTOTYPE,        DUK_HBUFOBJ_ELEM_INT32,        2, 1),  /* DUK_BUFOBJ_INT32ARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_UINT32ARRAY,       DUK_BIDX_UINT32ARRAY_PROTOTYPE,       DUK_HBUFOBJ_ELEM_UINT32,       2, 1),  /* DUK_BUFOBJ_UINT32ARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_FLOAT32ARRAY,      DUK_BIDX_FLOAT32ARRAY_PROTOTYPE,      DUK_HBUFOBJ_ELEM_FLOAT32,      2, 1),  /* DUK_BUFOBJ_FLOAT32ARRAY */
	DUK__PACK_ARGS(DUK_HOBJECT_CLASS_FLOAT64ARRAY,      DUK_BIDX_FLOAT64ARRAY_PROTOTYPE,      DUK_HBUFOBJ_ELEM_FLOAT64,      3, 1)   /* DUK_BUFOBJ_FLOAT64ARRAY */
};
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_EXTERNAL void duk_push_buffer_object(duk_context *ctx, duk_idx_t idx_buffer, duk_size_t byte_offset, duk_size_t byte_length, duk_uint_t flags) {
	duk_hthread *thr;
	duk_hbufobj *h_bufobj;
	duk_hbuffer *h_val;
	duk_uint32_t tmp;
	duk_uint_t classnum;
	duk_uint_t protobidx;
	duk_uint_t lookupidx;
	duk_uint_t uint_offset, uint_length, uint_added;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK_UNREF(thr);

	/* The underlying types for offset/length in duk_hbufobj is
	 * duk_uint_t; make sure argument values fit and that
	 * offset + length does not wrap.
	 */
	uint_offset = (duk_uint_t) byte_offset;
	uint_length = (duk_uint_t) byte_length;
	if (sizeof(duk_size_t) != sizeof(duk_uint_t)) {
		if ((duk_size_t) uint_offset != byte_offset || (duk_size_t) uint_length != byte_length) {
			goto range_error;
		}
	}
	uint_added = uint_offset + uint_length;
	if (uint_added < uint_offset) {
		goto range_error;
	}
	DUK_ASSERT(uint_added >= uint_offset && uint_added >= uint_length);

	DUK_ASSERT_DISABLE(flags >= 0);  /* flags is unsigned */
	lookupidx = flags;
	if (lookupidx >= sizeof(duk__bufobj_flags_lookup) / sizeof(duk_uint32_t)) {
		goto arg_error;
	}
	tmp = duk__bufobj_flags_lookup[lookupidx];
	classnum = tmp >> 24;
	protobidx = (tmp >> 16) & 0xff;

	h_val = duk_require_hbuffer(ctx, idx_buffer);
	DUK_ASSERT(h_val != NULL);

	h_bufobj = duk_push_bufobj_raw(ctx,
	                               DUK_HOBJECT_FLAG_EXTENSIBLE |
	                               DUK_HOBJECT_FLAG_BUFOBJ |
	                               DUK_HOBJECT_CLASS_AS_FLAGS(classnum),
	                               protobidx);
	DUK_ASSERT(h_bufobj != NULL);

	h_bufobj->buf = h_val;
	DUK_HBUFFER_INCREF(thr, h_val);
	h_bufobj->offset = uint_offset;
	h_bufobj->length = uint_length;
	h_bufobj->shift = (tmp >> 4) & 0x0f;
	h_bufobj->elem_type = (tmp >> 8) & 0xff;
	h_bufobj->is_typedarray = tmp & 0x0f;
	DUK_ASSERT_HBUFOBJ_VALID(h_bufobj);

	/* TypedArray views need an automatic ArrayBuffer which must be
	 * provided as .buffer property of the view.  The ArrayBuffer is
	 * referenced via duk_hbufobj->buf_prop and an inherited .buffer
	 * accessor returns it.  The ArrayBuffer is created lazily on first
	 * access so we don't need to do anything more here.
	 */
	return;

 range_error:
	DUK_ERROR_RANGE(thr, DUK_STR_INVALID_ARGS);
	return;  /* not reached */

 arg_error:
	DUK_ERROR_TYPE(thr, DUK_STR_INVALID_ARGS);
	return;  /* not reached */
}
#else  /* DUK_USE_BUFFEROBJECT_SUPPORT */
DUK_EXTERNAL void duk_push_buffer_object(duk_context *ctx, duk_idx_t idx_buffer, duk_size_t byte_offset, duk_size_t byte_length, duk_uint_t flags) {
	DUK_UNREF(idx_buffer);
	DUK_UNREF(byte_offset);
	DUK_UNREF(byte_length);
	DUK_UNREF(flags);
	DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
}
#endif  /* DUK_USE_BUFFEROBJECT_SUPPORT */

DUK_EXTERNAL duk_idx_t duk_push_error_object_va_raw(duk_context *ctx, duk_errcode_t err_code, const char *filename, duk_int_t line, const char *fmt, va_list ap) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *proto;
#if defined(DUK_USE_AUGMENT_ERROR_CREATE)
	duk_bool_t noblame_fileline;
#endif

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_UNREF(filename);
	DUK_UNREF(line);

	/* Error code also packs a tracedata related flag. */
#if defined(DUK_USE_AUGMENT_ERROR_CREATE)
	noblame_fileline = err_code & DUK_ERRCODE_FLAG_NOBLAME_FILELINE;
#endif
	err_code = err_code & (~DUK_ERRCODE_FLAG_NOBLAME_FILELINE);

	/* error gets its 'name' from the prototype */
	proto = duk_error_prototype_from_code(thr, err_code);
	(void) duk_push_object_helper_proto(ctx,
	                                    DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                    DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ERROR),
	                                    proto);

	/* ... and its 'message' from an instance property */
	if (fmt) {
		duk_push_vsprintf(ctx, fmt, ap);
		duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	} else {
		/* If no explicit message given, put error code into message field
		 * (as a number).  This is not fully in keeping with the Ecmascript
		 * error model because messages are supposed to be strings (Error
		 * constructors use ToString() on their argument).  However, it's
		 * probably more useful than having a separate 'code' property.
		 */
		duk_push_int(ctx, err_code);
		duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	}

	/* XXX: .code = err_code disabled, not sure if useful */

	/* Creation time error augmentation */
#if defined(DUK_USE_AUGMENT_ERROR_CREATE)
	/* filename may be NULL in which case file/line is not recorded */
	duk_err_augment_error_create(thr, thr, filename, line, noblame_fileline);  /* may throw an error */
#endif

	return duk_get_top_index_unsafe(ctx);
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
	void *buf_data;

	DUK_ASSERT_CTX_VALID(ctx);

	/* check stack first */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR_RANGE_PUSH_BEYOND(thr);
	}

	/* Check for maximum buffer length. */
	if (size > DUK_HBUFFER_MAX_BYTELEN) {
		DUK_ERROR_RANGE(thr, DUK_STR_BUFFER_TOO_LONG);
	}

	h = duk_hbuffer_alloc(thr->heap, size, flags, &buf_data);
	if (!h) {
		DUK_ERROR_ALLOC_FAILED(thr);
	}

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_BUFFER(tv_slot, h);
	DUK_HBUFFER_INCREF(thr, h);
	thr->valstack_top++;

	return (void *) buf_data;
}

DUK_INTERNAL void *duk_push_fixed_buffer_nozero(duk_context *ctx, duk_size_t len) {
	return duk_push_buffer_raw(ctx, len, DUK_BUF_FLAG_NOZERO);
}

DUK_INTERNAL void *duk_push_fixed_buffer_zero(duk_context *ctx, duk_size_t len) {
	void *ptr;
	ptr = duk_push_buffer_raw(ctx, len, 0);
#if !defined(DUK_USE_ZERO_BUFFER_DATA)
	/* ES2015 requires zeroing even when DUK_USE_ZERO_BUFFER_DATA
	 * is not set.
	 */
	DUK_MEMZERO((void *) ptr, (size_t) len);
#endif
	return ptr;
}

DUK_EXTERNAL duk_idx_t duk_push_heapptr(duk_context *ctx, void *ptr) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_idx_t ret;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Reviving an object using a heap pointer is a dangerous API
	 * operation: if the application doesn't guarantee that the
	 * pointer target is always reachable, difficult-to-diagnose
	 * problems may ensue.  Try to validate the 'ptr' argument to
	 * the extent possible.
	 */

#if defined(DUK_USE_ASSERTIONS)
	{
		/* One particular problem case is where an object has been
		 * queued for finalization but the finalizer hasn't been
		 * executed.
		 */
		duk_heaphdr *curr;
		for (curr = thr->heap->finalize_list;
		     curr != NULL;
		     curr = DUK_HEAPHDR_GET_NEXT(thr->heap, curr)) {
			DUK_ASSERT(curr != (duk_heaphdr *) ptr);
		}
	}
#endif

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

/* Push object with no prototype, i.e. a "bare" object. */
DUK_EXTERNAL duk_idx_t duk_push_bare_object(duk_context *ctx) {
	(void) duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              -1);  /* no prototype */
	return duk_get_top_index_unsafe(ctx);
}

DUK_INTERNAL void duk_push_hstring(duk_context *ctx, duk_hstring *h) {
	duk_tval tv;
	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(h != NULL);
	DUK_TVAL_SET_STRING(&tv, h);
	duk_push_tval(ctx, &tv);
}

DUK_INTERNAL void duk_push_hstring_stridx(duk_context *ctx, duk_small_uint_t stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_UNREF(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, stridx));
}

DUK_INTERNAL void duk_push_hstring_empty(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_UNREF(thr);
	duk_push_hstring(ctx, DUK_HTHREAD_GET_STRING(thr, DUK_STRIDX_EMPTY_STRING));
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
	duk_tval *tv;
#if defined(DUK_USE_REFERENCE_COUNTING)
	duk_tval *tv_end;
#endif

	DUK_ASSERT_CTX_VALID(ctx);

	if (DUK_UNLIKELY(count < 0)) {
		DUK_ERROR_RANGE_INVALID_COUNT(thr);
		return;
	}

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	if (DUK_UNLIKELY((duk_size_t) (thr->valstack_top - thr->valstack_bottom) < (duk_size_t) count)) {
		DUK_ERROR_RANGE_INVALID_COUNT(thr);
	}

	/*
	 *  Must be very careful here, every DECREF may cause reallocation
	 *  of our valstack.
	 */

	/* XXX: inlined DECREF macro would be nice here: no NULL check,
	 * refzero queueing but no refzero algorithm run (= no pointer
	 * instability), inline code.
	 */

	/* XXX: optimize loops */

#if defined(DUK_USE_REFERENCE_COUNTING)
	tv = thr->valstack_top;
	tv_end = tv - count;
	while (tv != tv_end) {
		tv--;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		DUK_TVAL_SET_UNDEFINED_UPDREF_NORZ(thr, tv);
	}
	thr->valstack_top = tv;
	DUK_REFZERO_CHECK_FAST(thr);
#else
	tv = thr->valstack_top;
	while (count > 0) {
		count--;
		tv--;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		DUK_TVAL_SET_UNDEFINED(tv);
	}
	thr->valstack_top = tv;
#endif

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
}

/* Popping one element is called so often that when footprint is not an issue,
 * compile a specialized function for it.
 */
#if defined(DUK_USE_PREFER_SIZE)
DUK_EXTERNAL void duk_pop(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_pop_n(ctx, 1);
}
#else
DUK_EXTERNAL void duk_pop(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	DUK_ASSERT_CTX_VALID(ctx);

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	if (DUK_UNLIKELY(thr->valstack_top == thr->valstack_bottom)) {
		DUK_ERROR_RANGE_INVALID_COUNT(thr);
	}

	tv = --thr->valstack_top;  /* tv points to element just below prev top */
	DUK_ASSERT(tv >= thr->valstack_bottom);
#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_TVAL_SET_UNDEFINED_UPDREF(thr, tv);  /* side effects */
#else
	DUK_TVAL_SET_UNDEFINED(tv);
#endif
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
}
#endif  /* !DUK_USE_PREFER_SIZE */

/* Unsafe internal variant which assumes there are enough values on the value
 * stack so that a top check can be skipped safely.
 */
#if defined(DUK_USE_PREFER_SIZE)
DUK_INTERNAL void duk_pop_unsafe(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_pop_n(ctx, 1);
}
#else
DUK_INTERNAL void duk_pop_unsafe(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	DUK_ASSERT_CTX_VALID(ctx);

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_top != thr->valstack_bottom);

	tv = --thr->valstack_top;  /* tv points to element just below prev top */
	DUK_ASSERT(tv >= thr->valstack_bottom);
#if defined(DUK_USE_REFERENCE_COUNTING)
	DUK_TVAL_SET_UNDEFINED_UPDREF(thr, tv);  /* side effects */
#else
	DUK_TVAL_SET_UNDEFINED(tv);
#endif
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
}
#endif  /* !DUK_USE_PREFER_SIZE */

DUK_EXTERNAL void duk_pop_2(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_pop_n(ctx, 2);
}

DUK_EXTERNAL void duk_pop_3(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);
	duk_pop_n(ctx, 3);
}

/*
 *  Pack and unpack (pack value stack entries into an array and vice versa)
 */

/* XXX: pack index range? array index offset? */
DUK_INTERNAL void duk_pack(duk_context *ctx, duk_idx_t count) {
	duk_hthread *thr;
	duk_harray *a;
	duk_tval *tv_src;
	duk_tval *tv_dst;
	duk_tval *tv_curr;
	duk_tval *tv_limit;
	duk_idx_t top;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;

	top = (duk_idx_t) (thr->valstack_top - thr->valstack_bottom);
	if (count < 0 || count > top) {
		DUK_ERROR_RANGE_INVALID_COUNT(thr);
		return;
	}

	/* Wrapping is controlled by the check above: value stack top can be
	 * at most thr->valstack_max which is low enough so that multiplying
	 * with sizeof(duk_tval) won't wrap.
	 */
	DUK_ASSERT(count >= 0 && count <= (duk_idx_t) thr->valstack_max);
	DUK_ASSERT((duk_size_t) count <= DUK_SIZE_MAX / sizeof(duk_tval));  /* no wrapping */

	a = duk_push_harray_with_size(ctx, (duk_uint32_t) count);  /* XXX: uninitialized would be OK */
	DUK_ASSERT(a != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_ASIZE((duk_hobject *) a) == (duk_uint32_t) count);
	DUK_ASSERT(count == 0 || DUK_HOBJECT_A_GET_BASE(thr->heap, (duk_hobject *) a) != NULL);
	DUK_ASSERT((duk_idx_t) a->length == count);

	/* Copy value stack values directly to the array part without
	 * any refcount updates: net refcount changes are zero.
	 */

	tv_src = thr->valstack_top - count - 1;
	tv_dst = DUK_HOBJECT_A_GET_BASE(thr->heap, (duk_hobject *) a);
	DUK_MEMCPY((void *) tv_dst, (const void *) tv_src, (size_t) count * sizeof(duk_tval));

	/* Overwrite result array to final value stack location and wipe
	 * the rest; no refcount operations needed.
	 */

	tv_dst = tv_src;  /* when count == 0, same as tv_src (OK) */
	tv_src = thr->valstack_top - 1;
	DUK_TVAL_SET_TVAL(tv_dst, tv_src);

	/* XXX: internal helper to wipe a value stack segment? */
	tv_curr = tv_dst + 1;
	tv_limit = thr->valstack_top;
	while (tv_curr != tv_limit) {
		/* Wipe policy: keep as 'undefined'. */
		DUK_TVAL_SET_UNDEFINED(tv_curr);
		tv_curr++;
	}
	thr->valstack_top = tv_dst + 1;
}

#if 0
/* XXX: unpack to position? */
DUK_INTERNAL void duk_unpack(duk_context *ctx) {
	/* - dense with length <= a_part
	 * - dense with length > a_part
	 * - sparse
	 * - array-like but not actually an array?
	 * - how to deal with 'unused' values (gaps); inherit or ignore?
	 */
}
#endif

/*
 *  Error throwing
 */

DUK_EXTERNAL void duk_throw_raw(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	if (thr->valstack_top == thr->valstack_bottom) {
		DUK_ERROR_TYPE_INVALID_ARGS(thr);
	}

	/* Errors are augmented when they are created, not when they are
	 * thrown or re-thrown.  The current error handler, however, runs
	 * just before an error is thrown.
	 */

	/* Sync so that augmentation sees up-to-date activations, NULL
	 * thr->ptr_curr_pc so that it's not used if side effects occur
	 * in augmentation or longjmp handling.
	 */
	duk_hthread_sync_and_null_currpc(thr);

#if defined(DUK_USE_AUGMENT_ERROR_THROW)
	DUK_DDD(DUK_DDDPRINT("THROW ERROR (API): %!dT (before throw augment)", (duk_tval *) duk_get_tval(ctx, -1)));
	duk_err_augment_error_throw(thr);
#endif
	DUK_DDD(DUK_DDDPRINT("THROW ERROR (API): %!dT (after throw augment)", (duk_tval *) duk_get_tval(ctx, -1)));

	duk_err_setup_heap_ljstate(thr, DUK_LJ_TYPE_THROW);

	/* thr->heap->lj.jmpbuf_ptr is checked by duk_err_longjmp() so we don't
	 * need to check that here.  If the value is NULL, a fatal error occurs
	 * because we can't return.
	 */

	duk_err_longjmp(thr);
	DUK_UNREACHABLE();
}

DUK_EXTERNAL void duk_fatal_raw(duk_context *ctx, const char *err_msg) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(thr->heap->fatal_func != NULL);

	DUK_D(DUK_DPRINT("fatal error occurred: %s", err_msg ? err_msg : "NULL"));

	/* fatal_func should be noreturn, but noreturn declarations on function
	 * pointers has a very spotty support apparently so it's not currently
	 * done.
	 */
	thr->heap->fatal_func(thr->heap->heap_udata, err_msg);

	/* If the fatal handler returns, all bets are off.  It'd be nice to
	 * print something here but since we don't want to depend on stdio,
	 * there's no way to do so portably.
	 */
	DUK_D(DUK_DPRINT("fatal error handler returned, all bets are off!"));
	for (;;) {
		/* loop forever, don't return (function marked noreturn) */
	}
}

DUK_EXTERNAL void duk_error_va_raw(duk_context *ctx, duk_errcode_t err_code, const char *filename, duk_int_t line, const char *fmt, va_list ap) {
	DUK_ASSERT_CTX_VALID(ctx);

	duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	(void) duk_throw(ctx);
}

DUK_EXTERNAL void duk_error_raw(duk_context *ctx, duk_errcode_t err_code, const char *filename, duk_int_t line, const char *fmt, ...) {
	va_list ap;

	DUK_ASSERT_CTX_VALID(ctx);

	va_start(ap, fmt);
	duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	(void) duk_throw(ctx);
}

#if !defined(DUK_USE_VARIADIC_MACROS)
DUK_NORETURN(DUK_LOCAL_DECL void duk__throw_error_from_stash(duk_context *ctx, duk_errcode_t err_code, const char *fmt, va_list ap));

DUK_LOCAL void duk__throw_error_from_stash(duk_context *ctx, duk_errcode_t err_code, const char *fmt, va_list ap) {
	const char *filename;
	duk_int_t line;

	DUK_ASSERT_CTX_VALID(ctx);

	filename = duk_api_global_filename;
	line = duk_api_global_line;
	duk_api_global_filename = NULL;
	duk_api_global_line = 0;

	duk_push_error_object_va_raw(ctx, err_code, filename, line, fmt, ap);
	(void) duk_throw(ctx);
}

#define DUK__ERROR_STASH_SHARED(code) do { \
		va_list ap; \
		va_start(ap, fmt); \
		duk__throw_error_from_stash(ctx, (code), fmt, ap); \
		va_end(ap); \
		/* Never reached; if return 0 here, gcc/clang will complain. */ \
	} while (0)

DUK_EXTERNAL duk_ret_t duk_error_stash(duk_context *ctx, duk_errcode_t err_code, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(err_code);
}
DUK_EXTERNAL duk_ret_t duk_generic_error_stash(duk_context *ctx, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(DUK_ERR_ERROR);
}
DUK_EXTERNAL duk_ret_t duk_eval_error_stash(duk_context *ctx, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(DUK_ERR_EVAL_ERROR);
}
DUK_EXTERNAL duk_ret_t duk_range_error_stash(duk_context *ctx, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(DUK_ERR_RANGE_ERROR);
}
DUK_EXTERNAL duk_ret_t duk_reference_error_stash(duk_context *ctx, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(DUK_ERR_REFERENCE_ERROR);
}
DUK_EXTERNAL duk_ret_t duk_syntax_error_stash(duk_context *ctx, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(DUK_ERR_SYNTAX_ERROR);
}
DUK_EXTERNAL duk_ret_t duk_type_error_stash(duk_context *ctx, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(DUK_ERR_TYPE_ERROR);
}
DUK_EXTERNAL duk_ret_t duk_uri_error_stash(duk_context *ctx, const char *fmt, ...) {
	DUK__ERROR_STASH_SHARED(DUK_ERR_URI_ERROR);
}
#endif  /* DUK_USE_VARIADIC_MACROS */

/*
 *  Comparison
 */

DUK_EXTERNAL duk_bool_t duk_equals(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1, *tv2;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_get_tval(ctx, idx1);
	tv2 = duk_get_tval(ctx, idx2);
	if ((tv1 == NULL) || (tv2 == NULL)) {
		return 0;
	}

	/* Coercion may be needed, the helper handles that by pushing the
	 * tagged values to the stack.
	 */
	return duk_js_equals(thr, tv1, tv2);
}

DUK_EXTERNAL duk_bool_t duk_strict_equals(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2) {
	duk_tval *tv1, *tv2;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_get_tval(ctx, idx1);
	tv2 = duk_get_tval(ctx, idx2);
	if ((tv1 == NULL) || (tv2 == NULL)) {
		return 0;
	}

	/* No coercions or other side effects, so safe */
	return duk_js_strict_equals(tv1, tv2);
}

DUK_EXTERNAL_DECL duk_bool_t duk_samevalue(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2) {
	duk_tval *tv1, *tv2;

	DUK_ASSERT_CTX_VALID(ctx);

	tv1 = duk_get_tval(ctx, idx1);
	tv2 = duk_get_tval(ctx, idx2);
	if ((tv1 == NULL) || (tv2 == NULL)) {
		return 0;
	}

	/* No coercions or other side effects, so safe */
	return duk_js_samevalue(tv1, tv2);
}

/*
 *  instanceof
 */

DUK_EXTERNAL duk_bool_t duk_instanceof(duk_context *ctx, duk_idx_t idx1, duk_idx_t idx2) {
	duk_tval *tv1, *tv2;

	DUK_ASSERT_CTX_VALID(ctx);

	/* Index validation is strict, which differs from duk_equals().
	 * The strict behavior mimics how instanceof itself works, e.g.
	 * it is a TypeError if rval is not a -callable- object.  It would
	 * be somewhat inconsistent if rval would be allowed to be
	 * non-existent without a TypeError.
	 */
	tv1 = duk_require_tval(ctx, idx1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, idx2);
	DUK_ASSERT(tv2 != NULL);

	return duk_js_instanceof((duk_hthread *) ctx, tv1, tv2);
}

/*
 *  Lightfunc
 */

DUK_INTERNAL void duk_push_lightfunc_name_raw(duk_context *ctx, duk_c_function func, duk_small_uint_t lf_flags) {
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

	duk_push_sprintf(ctx, "light_");
	duk_push_string_funcptr(ctx, (duk_uint8_t *) &func, sizeof(func));
	duk_push_sprintf(ctx, "_%04x", (unsigned int) lf_flags);
	duk_concat(ctx, 3);
}

DUK_INTERNAL void duk_push_lightfunc_name(duk_context *ctx, duk_tval *tv) {
	duk_c_function func;
	duk_small_uint_t lf_flags;

	DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv));
	DUK_TVAL_GET_LIGHTFUNC(tv, func, lf_flags);
	duk_push_lightfunc_name_raw(ctx, func, lf_flags);
}

DUK_INTERNAL void duk_push_lightfunc_tostring(duk_context *ctx, duk_tval *tv) {
	duk_c_function func;
	duk_small_uint_t lf_flags;

	DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv));
	DUK_TVAL_GET_LIGHTFUNC(tv, func, lf_flags);  /* read before 'tv' potentially invalidated */

	duk_push_string(ctx, "function ");
	duk_push_lightfunc_name_raw(ctx, func, lf_flags);
	duk_push_string(ctx, "() { [lightfunc code] }");
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

/*
 *  Push readable string summarizing duk_tval.  The operation is side effect
 *  free and will only throw from internal errors (e.g. out of memory).
 *  This is used by e.g. property access code to summarize a key/base safely,
 *  and is not intended to be fast (but small and safe).
 */

#define DUK__READABLE_STRING_MAXCHARS 32

/* String sanitizer which escapes ASCII control characters and a few other
 * ASCII characters, passes Unicode as is, and replaces invalid UTF-8 with
 * question marks.  No errors are thrown for any input string, except in out
 * of memory situations.
 */
DUK_LOCAL void duk__push_hstring_readable_unicode(duk_context *ctx, duk_hstring *h_input) {
	duk_hthread *thr;
	const duk_uint8_t *p, *p_start, *p_end;
	duk_uint8_t buf[DUK_UNICODE_MAX_XUTF8_LENGTH * DUK__READABLE_STRING_MAXCHARS +
	                2 /*quotes*/ + 3 /*periods*/];
	duk_uint8_t *q;
	duk_ucodepoint_t cp;
	duk_small_uint_t nchars;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(h_input != NULL);
	thr = (duk_hthread *) ctx;

	p_start = (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h_input);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
	p = p_start;
	q = buf;

	nchars = 0;
	*q++ = (duk_uint8_t) DUK_ASC_SINGLEQUOTE;
	for (;;) {
		if (p >= p_end) {
			break;
		}
		if (nchars == DUK__READABLE_STRING_MAXCHARS) {
			*q++ = (duk_uint8_t) DUK_ASC_PERIOD;
			*q++ = (duk_uint8_t) DUK_ASC_PERIOD;
			*q++ = (duk_uint8_t) DUK_ASC_PERIOD;
			break;
		}
		if (duk_unicode_decode_xutf8(thr, &p, p_start, p_end, &cp)) {
			if (cp < 0x20 || cp == 0x7f || cp == DUK_ASC_SINGLEQUOTE || cp == DUK_ASC_BACKSLASH) {
				DUK_ASSERT(DUK_UNICODE_MAX_XUTF8_LENGTH >= 4);  /* estimate is valid */
				DUK_ASSERT((cp >> 4) <= 0x0f);
				*q++ = (duk_uint8_t) DUK_ASC_BACKSLASH;
				*q++ = (duk_uint8_t) DUK_ASC_LC_X;
				*q++ = (duk_uint8_t) duk_lc_digits[cp >> 4];
				*q++ = (duk_uint8_t) duk_lc_digits[cp & 0x0f];
			} else {
				q += duk_unicode_encode_xutf8(cp, q);
			}
		} else {
			p++;  /* advance manually */
			*q++ = (duk_uint8_t) DUK_ASC_QUESTION;
		}
		nchars++;
	}
	*q++ = (duk_uint8_t) DUK_ASC_SINGLEQUOTE;

	duk_push_lstring(ctx, (const char *) buf, (duk_size_t) (q - buf));
}

DUK_LOCAL const char *duk__push_string_tval_readable(duk_context *ctx, duk_tval *tv, duk_bool_t error_aware) {
	duk_hthread *thr;

	DUK_ASSERT_CTX_VALID(ctx);
	thr = (duk_hthread *) ctx;
	DUK_UNREF(thr);
	/* 'tv' may be NULL */

	if (tv == NULL) {
		duk_push_string(ctx, "none");
	} else {
		switch (DUK_TVAL_GET_TAG(tv)) {
		case DUK_TAG_STRING: {
			/* XXX: symbol support (maybe in summary rework branch) */
			duk__push_hstring_readable_unicode(ctx, DUK_TVAL_GET_STRING(tv));
			break;
		}
		case DUK_TAG_OBJECT: {
			duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
			DUK_ASSERT(h != NULL);

			if (error_aware &&
			    duk_hobject_prototype_chain_contains(thr, h, thr->builtins[DUK_BIDX_ERROR_PROTOTYPE], 1 /*ignore_loop*/)) {
				/* Get error message in a side effect free way if
				 * possible; if not, summarize as a generic object.
				 * Error message currently gets quoted.
				 */
				/* XXX: better internal getprop call; get without side effects
				 * but traverse inheritance chain.
				 */
				duk_tval *tv_msg;
				tv_msg = duk_hobject_find_existing_entry_tval_ptr(thr->heap, h, DUK_HTHREAD_STRING_MESSAGE(thr));
				if (tv_msg) {
					/* It's important this summarization is
					 * not error aware to avoid unlimited
					 * recursion when the .message property
					 * is e.g. another error.
					 */
					return duk_push_string_tval_readable(ctx, tv_msg);
				}
			}
			duk_push_class_string_tval(ctx, tv);
			break;
		}
		case DUK_TAG_BUFFER: {
			/* While plain buffers mimic Uint8Arrays, they summarize differently.
			 * This is useful so that the summarized string accurately reflects the
			 * internal type which may matter for figuring out bugs etc.
			 */
			/* XXX: Hex encoded, length limited buffer summary here? */
			duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
			DUK_ASSERT(h != NULL);
			duk_push_sprintf(ctx, "[buffer:%ld]", (long) DUK_HBUFFER_GET_SIZE(h));
			break;
		}
		case DUK_TAG_POINTER: {
			/* Surround with parentheses like in JX, ensures NULL pointer
			 * is distinguishable from null value ("(null)" vs "null").
			 */
			duk_push_tval(ctx, tv);
			duk_push_sprintf(ctx, "(%s)", duk_to_string(ctx, -1));
			duk_remove_m2(ctx);
			break;
		}
		default: {
			duk_push_tval(ctx, tv);
			break;
		}
		}
	}

	return duk_to_string(ctx, -1);
}
DUK_INTERNAL const char *duk_push_string_tval_readable(duk_context *ctx, duk_tval *tv) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__push_string_tval_readable(ctx, tv, 0 /*error_aware*/);
}

DUK_INTERNAL const char *duk_push_string_readable(duk_context *ctx, duk_idx_t idx) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk_push_string_tval_readable(ctx, duk_get_tval(ctx, idx));
}

DUK_INTERNAL const char *duk_push_string_tval_readable_error(duk_context *ctx, duk_tval *tv) {
	DUK_ASSERT_CTX_VALID(ctx);
	return duk__push_string_tval_readable(ctx, tv, 1 /*error_aware*/);
}

DUK_INTERNAL void duk_push_symbol_descriptive_string(duk_context *ctx, duk_hstring *h) {
	const duk_uint8_t *p;
	const duk_uint8_t *p_end;
	const duk_uint8_t *q;

	/* .toString() */
	duk_push_string(ctx, "Symbol(");
	p = (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h);
	p_end = p + DUK_HSTRING_GET_BYTELEN(h);
	DUK_ASSERT(p[0] == 0xff || (p[0] & 0xc0) == 0x80);
	p++;
	for (q = p; q < p_end; q++) {
		if (*q == 0xffU) {
			/* Terminate either at end-of-string (but NUL MUST
			 * be accepted without terminating description) or
			 * 0xFF, which is used to mark start of unique trailer
			 * (and cannot occur in CESU-8 / extended UTF-8).
			 */
			break;
		}
	}
	duk_push_lstring(ctx, (const char *) p, (duk_size_t) (q - p));
	duk_push_string(ctx, ")");
	duk_concat(ctx, 3);
}
