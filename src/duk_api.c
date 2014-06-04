/*
 *  API calls not falling into other categories.
 *
 *  Also contains internal functions (such as duk_get_tval()), defined
 *  in duk_api_internal.h, with semantics similar to the public API.
 */

/* XXX: repetition of stack pre-checks -> helper or macro or inline */
/* XXX: shared api error strings, and perhaps even throw code for rare cases? */

#include "duk_internal.h"

/*
 *  Global state for working around missing variadic macros
 */

#ifndef DUK_USE_VARIADIC_MACROS
const char *duk_api_global_filename = NULL;
duk_int_t duk_api_global_line = 0;
#endif

/*
 *  Helpers
 */

static int duk__api_coerce_d2i(double d) {
	int c;

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

	c = DUK_FPCLASSIFY(d);
	if (c == DUK_FP_NAN) {
		return 0;
	} else if (d < INT_MIN) {
		/* covers -Infinity */
		return INT_MIN;
	} else if (d > INT_MAX) {
		/* covers +Infinity */
		return INT_MAX;
	} else {
		/* coerce towards zero */
		return (int) d;
	}
}

/*
 *  Stack indexes and stack size management
 */

int duk_normalize_index(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	if (index < 0) {
		if (index == DUK_INVALID_INDEX) {
			goto fail;
		}
		tv = thr->valstack_top + index;
		DUK_ASSERT(tv < thr->valstack_top);
		if (tv < thr->valstack_bottom) {
			goto fail;
		}
	} else {
		tv = thr->valstack_bottom + index;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		if (tv >= thr->valstack_top) {
			goto fail;
		}
	}

	DUK_ASSERT((int) (tv - thr->valstack_bottom) >= 0);
	return (int) (tv - thr->valstack_bottom);

 fail:
	return DUK_INVALID_INDEX;
}

int duk_require_normalize_index(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int ret;

	DUK_ASSERT(ctx != NULL);

	ret = duk_normalize_index(ctx, index);
	if (ret < 0) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "invalid index");
	}
	return ret;
}

int duk_is_valid_index(duk_context *ctx, int index) {
	DUK_ASSERT(DUK_INVALID_INDEX < 0);
	return (duk_normalize_index(ctx, index) >= 0);
}

void duk_require_valid_index(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	if (duk_normalize_index(ctx, index) < 0) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "invalid index");
	}
}

int duk_get_top(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);

	return (int) (thr->valstack_top - thr->valstack_bottom);
}

/* set stack top within currently allocated range, but don't reallocate */
void duk_set_top(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_new_top;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	/* FIXME: the pointer arithmetic here is not safe on a 32-bit platform,
	 * as it may wrap.  For instance, with 8-byte values, the index 0x20000000
	 * will wrap and be equivalent to index 0; with 12-byte values, the index
	 * 0x15555556 will wrap to +8 bytes and does not even wrap evenly to a
	 * duk_tval boundary!  A correct check would first impose a min/max index
	 * which guarantees that there is only one "round" of wrapping at most,
	 * and then wrapping needs to be detected because we don't want the value
	 * stack to be wrapped around end-of-memory.
	 */

	if (index < 0) {
		if (index == DUK_INVALID_INDEX) {
			goto invalid_index;
		}
		tv_new_top = thr->valstack_top + index;
	} else {
		/* may be higher than valstack_top, but not higher than
		 * allocated stack
		 */
		tv_new_top = thr->valstack_bottom + index;
	}

	/* Check both ends: for extreme values the pointer arithmetic may wrap.
	 * The check doesn't detect wrapping so it's technically incorrect.
	 */
	if (tv_new_top < thr->valstack_bottom) {
		goto invalid_index;
	}
	if (tv_new_top > thr->valstack_end) {
		goto invalid_index;
	}

	if (tv_new_top >= thr->valstack_top) {
		/* no pointer stability issues when increasing stack size */
		while (thr->valstack_top < tv_new_top) {
			/* no need to decref previous or new value */
			DUK_ASSERT(DUK_TVAL_IS_UNDEFINED_UNUSED(thr->valstack_top));
			DUK_TVAL_SET_UNDEFINED_ACTUAL(thr->valstack_top);
			thr->valstack_top++;
		}
	} else {
		/* each DECREF potentially invalidates valstack pointers, careful */
		ptrdiff_t pdiff = ((char *) thr->valstack_top) - ((char *) tv_new_top);  /* byte diff (avoid shift/div) */

		/* XXX: inlined DECREF macro would be nice here: no NULL check,
		 * refzero queueing but no refzero algorithm run (= no pointer
		 * instability), inline code.
		 */
	
		while (pdiff > 0) {
			duk_tval tv_tmp;
			duk_tval *tv;

			thr->valstack_top--;
			tv = thr->valstack_top;
			DUK_ASSERT(tv >= thr->valstack_bottom);
			DUK_TVAL_SET_TVAL(&tv_tmp, tv);
			DUK_TVAL_SET_UNDEFINED_UNUSED(tv);
			DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */

			pdiff -= sizeof(duk_tval);
		}
	}
	return;

 invalid_index:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, "invalid index");
}

int duk_get_top_index(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int ret;

	DUK_ASSERT(ctx != NULL);

	ret = ((int) (thr->valstack_top - thr->valstack_bottom)) - 1;
	if (ret < 0) {
		/* Return invalid index; if caller uses this without checking
		 * in another API call, the index will never (practically)
		 * map to a valid stack entry.
		 */
		return DUK_INVALID_INDEX;
	}
	return ret;
}

int duk_require_top_index(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int ret;

	DUK_ASSERT(ctx != NULL);

	ret = ((int) (thr->valstack_top - thr->valstack_bottom)) - 1;
	if (ret < 0) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "invalid index");
	}
	return ret;
}

/* XXX: perhaps refactor this to allow caller to specify some parameters, or
 * at least a 'compact' flag which skips any spare or round-up .. useful for
 * emergency gc.
 */

/* Resize valstack, with careful recomputation of all pointers.
 * Must also work if ALL pointers are NULL.
 *
 * Note: this is very tricky because the valstack realloc may
 * cause a mark-and-sweep, which may run finalizers.  Running
 * finalizers may resize the valstack recursively.  So, after
 * realloc returns, we know that the valstack "top" should still
 * be the same (there should not be live values above the "top"),
 * but its underlying size may have changed.
 */
static int duk__resize_valstack(duk_context *ctx, size_t new_size) {
	duk_hthread *thr = (duk_hthread *) ctx;
	ptrdiff_t old_bottom_offset;
	ptrdiff_t old_top_offset;
	ptrdiff_t old_end_offset_post;
#ifdef DUK_USE_DEBUG
	ptrdiff_t old_end_offset_pre;
	duk_tval *old_valstack_pre;
	duk_tval *old_valstack_post;
#endif
	duk_tval *new_valstack;
	duk_tval *p;
	duk_size_t new_alloc_size;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);
	DUK_ASSERT((duk_size_t) (thr->valstack_top - thr->valstack) <= new_size);  /* can't resize below 'top' */

	/* get pointer offsets for tweaking below */
	old_bottom_offset = (((duk_uint8_t *) thr->valstack_bottom) - ((duk_uint8_t *) thr->valstack));
	old_top_offset = (((duk_uint8_t *) thr->valstack_top) - ((duk_uint8_t *) thr->valstack));
#ifdef DUK_USE_DEBUG
	old_end_offset_pre = (((duk_uint8_t *) thr->valstack_end) - ((duk_uint8_t *) thr->valstack));  /* not very useful, used for debugging */
	old_valstack_pre = thr->valstack;
#endif

	/* allocate a new valstack
	 *
	 * Note: cannot use a plain DUK_REALLOC() because a mark-and-sweep may
	 * invalidate the original thr->valstack base pointer inside the realloc
	 * process.  See doc/memory-management.txt.
	 */

	new_alloc_size = sizeof(duk_tval) * new_size;
	new_valstack = (duk_tval *) DUK_REALLOC_INDIRECT(thr->heap, duk_hthread_get_valstack_ptr, (void *) thr, new_alloc_size);
	if (!new_valstack) {
		DUK_D(DUK_DPRINT("failed to resize valstack to %d entries (%d bytes)",
		                 new_size, new_alloc_size));
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
		                 "end offset changed: %d -> %d",
		                 old_end_offset_pre,
		                 old_end_offset_post));
	}
	if (old_valstack_pre != old_valstack_post) {
		DUK_D(DUK_DPRINT("valstack pointer changed during valstack_resize(), probably by mark-and-sweep: %p -> %p",
		                 (void *) old_valstack_pre,
		                 (void *) old_valstack_post));
	}
#endif

	DUK_DD(DUK_DDPRINT("resized valstack to %d elements (%d bytes), bottom=%d, top=%d, "
	                   "new pointers: start=%p end=%p bottom=%p top=%p",
	                   (int) new_size, (int) new_alloc_size,
	                   (int) (thr->valstack_bottom - thr->valstack),
	                   (int) (thr->valstack_top - thr->valstack),
	                   (void *) thr->valstack, (void *) thr->valstack_end,
	                   (void *) thr->valstack_bottom, (void *) thr->valstack_top));

	/* init newly allocated slots (only) */
	p = (duk_tval *) ((duk_uint8_t *) thr->valstack + old_end_offset_post);
	while (p < thr->valstack_end) {
		/* never executed if new size is smaller */
		DUK_TVAL_SET_UNDEFINED_UNUSED(p);
		p++;
	}

	/* assertion check: we try to maintain elements above top in known state */
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

static int duk__check_valstack_resize_helper(duk_context *ctx,
                                             size_t min_new_size,
                                             int shrink_flag,
                                             int compact_flag,
                                             int throw_flag) {
	duk_hthread *thr = (duk_hthread *) ctx;
	size_t old_size;
	size_t new_size;
	int is_shrink = 0;

	DUK_DDD(DUK_DDDPRINT("check valstack resize: min_new_size=%d, curr_size=%d, curr_top=%d, "
	                     "curr_bottom=%d, shrink=%d, compact=%d, throw=%d",
	                     (int) min_new_size,
	                     (int) (thr->valstack_end - thr->valstack),
	                     (int) (thr->valstack_top - thr->valstack),
	                     (int) (thr->valstack_bottom - thr->valstack),
	                     shrink_flag, compact_flag, throw_flag));

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	old_size = (size_t) (thr->valstack_end - thr->valstack);

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

	DUK_DD(DUK_DDPRINT("want to %s valstack: %d -> %d elements (min_new_size %d)",
	                   (new_size > old_size ? "grow" : "shrink"),
	                   old_size, new_size, min_new_size));

	if (new_size >= thr->valstack_max) {
		/* Note: may be triggered even if minimal new_size would not reach the limit,
		 * plan limit accordingly (taking DUK_VALSTACK_GROW_STEP into account.
		 */
		if (throw_flag) {
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "valstack limit");
		} else {
			return 0;
		}
	}

	/*
	 *  When resizing the valstack, a mark-and-sweep may be triggered for
	 *  the allocation of the new valstack.  If the mark-and-sweep needs
	 *  to use our thread for something, it may cause *the same valstack*
	 *  to be resized recursively.  This happens e.g. when mark-and-sweep
	 *  finalizers are called.
	 *
	 *  This is taken into account carefully in duk__resize_valstack().
	 */

	if (!duk__resize_valstack(ctx, new_size)) {
		if (is_shrink) {
			DUK_DD(DUK_DDPRINT("valstack resize failed, but is a shrink, ignore"));
			return 1;
		}

		DUK_DD(DUK_DDPRINT("valstack resize failed"));

		if (throw_flag) {
			DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to extend valstack");
		} else {
			return 0;
		}
	}

	DUK_DDD(DUK_DDDPRINT("valstack resize successful"));
	return 1;
}

#if 0  /* XXX: unused */
int duk_check_valstack_resize(duk_context *ctx, unsigned int min_new_size, int allow_shrink) {
	return duk__check_valstack_resize_helper(ctx,
	                                         min_new_size,  /* min_new_size */
	                                         allow_shrink,  /* shrink_flag */
	                                         0,             /* compact flag */
	                                         0);            /* throw flag */
}
#endif

void duk_require_valstack_resize(duk_context *ctx, unsigned int min_new_size, int allow_shrink) {
	(void) duk__check_valstack_resize_helper(ctx,
	                                         min_new_size,  /* min_new_size */
	                                         allow_shrink,  /* shrink_flag */
	                                         0,             /* compact flag */
	                                         1);            /* throw flag */
}

int duk_check_stack(duk_context *ctx, unsigned int extra) {
	duk_hthread *thr = (duk_hthread *) ctx;
	unsigned int min_new_size;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	min_new_size = (thr->valstack_top - thr->valstack) + extra + DUK_VALSTACK_INTERNAL_EXTRA;
	return duk__check_valstack_resize_helper(ctx,
	                                         min_new_size,  /* min_new_size */
	                                         0,             /* shrink_flag */
	                                         0,             /* compact flag */
	                                         0);            /* throw flag */
}

void duk_require_stack(duk_context *ctx, unsigned int extra) {
	duk_hthread *thr = (duk_hthread *) ctx;
	unsigned int min_new_size;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	min_new_size = (thr->valstack_top - thr->valstack) + extra + DUK_VALSTACK_INTERNAL_EXTRA;
	(void) duk__check_valstack_resize_helper(ctx,
	                                         min_new_size,  /* min_new_size */
	                                         0,             /* shrink_flag */
	                                         0,             /* compact flag */
	                                         1);            /* throw flag */
}

int duk_check_stack_top(duk_context *ctx, unsigned int top) {
	unsigned int min_new_size;

	DUK_ASSERT(ctx != NULL);

	min_new_size = top + DUK_VALSTACK_INTERNAL_EXTRA;
	return duk__check_valstack_resize_helper(ctx,
	                                         min_new_size,  /* min_new_size */
	                                         0,             /* shrink_flag */
	                                         0,             /* compact flag */
	                                         0);            /* throw flag */
}

void duk_require_stack_top(duk_context *ctx, unsigned int top) {
	unsigned int min_new_size;

	DUK_ASSERT(ctx != NULL);

	min_new_size = top + DUK_VALSTACK_INTERNAL_EXTRA;
	(void) duk__check_valstack_resize_helper(ctx,
	                                         min_new_size,  /* min_new_size */
	                                         0,             /* shrink_flag */
	                                         0,             /* compact flag */
	                                         1);            /* throw flag */
}

/*
 *  Stack manipulation
 */

void duk_swap(duk_context *ctx, int index1, int index2) {
	duk_tval *tv1;
	duk_tval *tv2;
	duk_tval tv;  /* temp */

	DUK_ASSERT(ctx != NULL);

	tv1 = duk_require_tval(ctx, index1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, index2);
	DUK_ASSERT(tv2 != NULL);

	DUK_TVAL_SET_TVAL(&tv, tv1);
	DUK_TVAL_SET_TVAL(tv1, tv2);
	DUK_TVAL_SET_TVAL(tv2, &tv);	
}

void duk_swap_top(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);

	duk_swap(ctx, index, -1);
}

void duk_dup(duk_context *ctx, int from_index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, from_index);
	DUK_ASSERT(tv != NULL);

	duk_push_tval(ctx, tv);
}

void duk_dup_top(duk_context *ctx) {
	DUK_ASSERT(ctx != NULL);

	duk_dup(ctx, -1);
}

void duk_insert(duk_context *ctx, int to_index) {
	duk_tval *p;
	duk_tval *q;
	duk_tval tv;
	size_t nbytes;

	DUK_ASSERT(ctx != NULL);

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

	nbytes = (size_t) (((duk_uint8_t *) q) - ((duk_uint8_t *) p));  /* Note: 'q' is top-1 */

	DUK_DDD(DUK_DDDPRINT("duk_insert: to_index=%p, p=%p, q=%p, nbytes=%d", to_index, p, q, nbytes));
	if (nbytes > 0) {
		DUK_TVAL_SET_TVAL(&tv, q);
		DUK_MEMMOVE((void *) (p + 1), (void *) p, nbytes);
		DUK_TVAL_SET_TVAL(p, &tv);
	} else {
		/* nop: insert top to top */
		DUK_ASSERT(nbytes == 0);
		DUK_ASSERT(p == q);
	}
}

void duk_replace(duk_context *ctx, int to_index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1;
	duk_tval *tv2;
	duk_tval tv;  /* temp */

	DUK_ASSERT(ctx != NULL);

	tv1 = duk_require_tval(ctx, -1);
	DUK_ASSERT(tv1 != NULL);
	tv2 = duk_require_tval(ctx, to_index);
	DUK_ASSERT(tv2 != NULL);

	/* For tv1 == tv2, both pointing to stack top, the end result
	 * is same as duk_pop(ctx).
	 */

	DUK_TVAL_SET_TVAL(&tv, tv2);
	DUK_TVAL_SET_TVAL(tv2, tv1);
	DUK_TVAL_SET_UNDEFINED_UNUSED(tv1);
	thr->valstack_top--;
	DUK_TVAL_DECREF(thr, &tv);
}

void duk_remove(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *p;
	duk_tval *q;
#ifdef DUK_USE_REFERENCE_COUNTING
	duk_tval tv;
#endif
	size_t nbytes;

	DUK_ASSERT(ctx != NULL);

	p = duk_require_tval(ctx, index);
	DUK_ASSERT(p != NULL);
	q = duk_require_tval(ctx, -1);
	DUK_ASSERT(q != NULL);

	DUK_ASSERT(q >= p);

	/*              nbytes
	 *           <--------->
	 *    [ ... | p | x | x | q ]
	 * => [ ... | x | x | q ]
	 */

#ifdef DUK_USE_REFERENCE_COUNTING
	/* use a temp: decref only when valstack reachable values are correct */
	DUK_TVAL_SET_TVAL(&tv, p);
#endif

	nbytes = (size_t) (((duk_uint8_t *) q) - ((duk_uint8_t *) p));  /* Note: 'q' is top-1 */
	if (nbytes > 0) {
		DUK_MEMMOVE(p, p + 1, nbytes);
	}
	DUK_TVAL_SET_UNDEFINED_UNUSED(q);
	thr->valstack_top--;

#ifdef DUK_USE_REFERENCE_COUNTING
	DUK_TVAL_DECREF(thr, &tv);
#endif
}

void duk_xmove(duk_context *ctx, duk_context *from_ctx, unsigned int count) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hthread *from_thr = (duk_hthread *) from_ctx;
	void *src;
	duk_size_t nbytes;
	duk_tval *p;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(from_ctx != NULL);

	nbytes = sizeof(duk_tval) * count;
	if (nbytes == 0) {
		return;
	}
	DUK_ASSERT(thr->valstack_top <= thr->valstack_end);
	if ((duk_size_t) ((duk_uint8_t *) thr->valstack_end - (duk_uint8_t *) thr->valstack_top) < nbytes) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
	}
	src = (void *) ((duk_uint8_t *) from_thr->valstack_top - nbytes);
	if (src < (void *) from_thr->valstack_bottom) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "source stack does not contain enough elements");
	}

	/* copy values (no overlap even if ctx == from_ctx) */
	DUK_MEMCPY((void *) thr->valstack_top, src, nbytes);

	/* incref them */
	p = thr->valstack_top;
	thr->valstack_top = (duk_tval *) (((duk_uint8_t *) thr->valstack_top) + nbytes);
	while (p < thr->valstack_top) {
		DUK_TVAL_INCREF(thr, p);
		p++;
	}
}

/*
 *  Get/require
 */

/* internal */
duk_tval *duk_get_tval(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	if (index < 0) {
		if (index == DUK_INVALID_INDEX) {
			return NULL;
		}
		tv = thr->valstack_top + index;
		DUK_ASSERT(tv < thr->valstack_top);
		if (tv < thr->valstack_bottom) {
			return NULL;
		}
	} else {
		tv = thr->valstack_bottom + index;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		if (tv >= thr->valstack_top) {
			return NULL;
		}
	}
	return tv;
}

duk_tval *duk_require_tval(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(DUK_INVALID_INDEX < 0);

	if (index < 0) {
		if (index == DUK_INVALID_INDEX) {
			/* XXX: this check may not be necessary
			 * on some architectures but be careful
			 * of wrapping.
			 */
			goto fail;
		}
		tv = thr->valstack_top + index;
		DUK_ASSERT(tv < thr->valstack_top);
		if (DUK_UNLIKELY(tv < thr->valstack_bottom)) {
			goto fail;
		}
	} else {
		tv = thr->valstack_bottom + index;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		if (DUK_UNLIKELY(tv >= thr->valstack_top)) {
			goto fail;
		}
	}
	return tv;

 fail:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, "index out of bounds");
	return NULL;  /* not reachable */
}

void duk_require_undefined(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_UNDEFINED(tv)) {
		/* Note: accept both 'actual' and 'unused' undefined */
		return;
	}
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not undefined");
}

void duk_require_null(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_NULL(tv)) {
		return;
	}
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not null");
}

int duk_get_boolean(duk_context *ctx, int index) {
	int ret = 0;  /* default: false */
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_BOOLEAN(tv)) {
		ret = DUK_TVAL_GET_BOOLEAN(tv);
	}

	DUK_ASSERT(ret == 0 || ret == 1);
	return ret;
}

int duk_require_boolean(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_BOOLEAN(tv)) {
		int ret = DUK_TVAL_GET_BOOLEAN(tv);
		DUK_ASSERT(ret == 0 || ret == 1);
		return ret;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not boolean");
	return 0;  /* not reachable */
}

double duk_get_number(duk_context *ctx, int index) {
	duk_double_union ret;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	ret.d = DUK_DOUBLE_NAN;  /* default: NaN */
	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_NUMBER(tv)) {
		ret.d = DUK_TVAL_GET_NUMBER(tv);
	}

	/*
	 *  Number should already be in NaN-normalized form,
	 *  but let's normalize anyway.
	 */

	DUK_DBLUNION_NORMALIZE_NAN_CHECK(&ret);
	return ret.d;
}

double duk_require_number(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

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

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not number");
	return DUK_DOUBLE_NAN;  /* not reachable */
}

int duk_get_int(duk_context *ctx, int index) {
	/* Custom coercion for API */
	return duk__api_coerce_d2i(duk_get_number(ctx, index));
}

int duk_require_int(duk_context *ctx, int index) {
	/* Custom coercion for API */
	return duk__api_coerce_d2i(duk_require_number(ctx, index));
}

const char *duk_get_lstring(duk_context *ctx, int index, size_t *out_len) {
	const char *ret;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

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

const char *duk_require_lstring(duk_context *ctx, int index, size_t *out_len) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const char *ret;

	DUK_ASSERT(ctx != NULL);

	/* Note: this check relies on the fact that even a zero-size string
	 * has a non-NULL pointer.
	 */
	ret = duk_get_lstring(ctx, index, out_len);
	if (ret) {
		return ret;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not string");
	return NULL;  /* not reachable */
}

const char *duk_get_string(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);

	return duk_get_lstring(ctx, index, NULL);
}

const char *duk_require_string(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);

	return duk_require_lstring(ctx, index, NULL);
}

void *duk_get_pointer(duk_context *ctx, int index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_POINTER(tv)) {
		void *p = DUK_TVAL_GET_POINTER(tv);  /* may be NULL */
		return (void *) p;
	}

	return NULL;
}

void *duk_require_pointer(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	/* Note: here we must be wary of the fact that a pointer may be
	 * valid and be a NULL.
	 */
	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_POINTER(tv)) {
		void *p = DUK_TVAL_GET_POINTER(tv);  /* may be NULL */
		return (void *) p;
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not pointer");
	return NULL;  /* not reachable */
}

void *duk_get_voidptr(duk_context *ctx, int index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_IS_HEAP_ALLOCATED(tv)) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		return (void *) h;
	}

	return NULL;
}

void *duk_get_buffer(duk_context *ctx, int index, size_t *out_size) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

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
		return (void *) DUK_HBUFFER_GET_DATA_PTR(h);  /* may be NULL (but only if size is 0) */
	}

	return NULL;
}

void *duk_require_buffer(duk_context *ctx, int index, size_t *out_size) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

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
		return (void *) DUK_HBUFFER_GET_DATA_PTR(h);  /* may be NULL (but only if size is 0) */
	}

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not buffer");
	return NULL;  /* not reachable */
}

/* Raw helper for getting a value from the stack, checking its tag, and possible its object class. */
duk_heaphdr *duk_get_tagged_heaphdr_raw(duk_context *ctx, int index, duk_int_t flags_and_tag) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_int_t tag = flags_and_tag & 0xffff;  /* tags can be up to 16 bits */

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (tv && DUK_TVAL_GET_TAG(tv) == tag) {
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

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "incorrect type, expected tag %d", tag);
	return NULL;  /* not reachable */
}

/* internal */
duk_hstring *duk_get_hstring(duk_context *ctx, int index) {
	return (duk_hstring *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_STRING | DUK_GETTAGGED_FLAG_ALLOW_NULL);
}

/* internal */
duk_hstring *duk_require_hstring(duk_context *ctx, int index) {
	return (duk_hstring *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_STRING);
}

/* internal */
duk_hobject *duk_get_hobject(duk_context *ctx, int index) {
	return (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
}

/* internal */
duk_hobject *duk_require_hobject(duk_context *ctx, int index) {
	return (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
}

/* internal */
duk_hbuffer *duk_get_hbuffer(duk_context *ctx, int index) {
	return (duk_hbuffer *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_BUFFER | DUK_GETTAGGED_FLAG_ALLOW_NULL);
}

/* internal */
duk_hbuffer *duk_require_hbuffer(duk_context *ctx, int index) {
	return (duk_hbuffer *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_BUFFER);
}

/* internal */
duk_hthread *duk_get_hthread(duk_context *ctx, int index) {
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
	if (h != NULL && !DUK_HOBJECT_IS_THREAD(h)) {
		h = NULL;
	}
	return (duk_hthread *) h;
}

/* internal */
duk_hthread *duk_require_hthread(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
	DUK_ASSERT(h != NULL);
	if (!DUK_HOBJECT_IS_THREAD(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "incorrect type, expected thread");
	}
	return (duk_hthread *) h;
}

/* internal */
duk_hcompiledfunction *duk_get_hcompiledfunction(duk_context *ctx, int index) {
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
	if (h != NULL && !DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		h = NULL;
	}
	return (duk_hcompiledfunction *) h;
}

/* internal */
duk_hcompiledfunction *duk_require_hcompiledfunction(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
	DUK_ASSERT(h != NULL);
	if (!DUK_HOBJECT_IS_COMPILEDFUNCTION(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "incorrect type, expected compiledfunction");
	}
	return (duk_hcompiledfunction *) h;
}

/* internal */
duk_hnativefunction *duk_get_hnativefunction(duk_context *ctx, int index) {
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT | DUK_GETTAGGED_FLAG_ALLOW_NULL);
	if (h != NULL && !DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		h = NULL;
	}
	return (duk_hnativefunction *) h;
}

/* internal */
duk_hnativefunction *duk_require_hnativefunction(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h = (duk_hobject *) duk_get_tagged_heaphdr_raw(ctx, index, DUK_TAG_OBJECT);
	DUK_ASSERT(h != NULL);
	if (!DUK_HOBJECT_IS_NATIVEFUNCTION(h)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "incorrect type, expected nativefunction");
	}
	return (duk_hnativefunction *) h;
}

duk_c_function duk_get_c_function(duk_context *ctx, int index) {
	duk_tval *tv;
	duk_hobject *h;
	duk_hnativefunction *f;

	DUK_ASSERT(ctx != NULL);

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

duk_c_function duk_require_c_function(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_c_function ret;

	ret = duk_get_c_function(ctx, index);
	if (!ret) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "incorrect type, expected c function");
	}
	return ret;
}

duk_context *duk_get_context(duk_context *ctx, int index) {
	duk_hobject *h;

	h = duk_get_hobject(ctx, index);
	if (!h) {
		return NULL;
	}
	if (!DUK_HOBJECT_HAS_THREAD(h)) {
		return NULL;
	}
	return (duk_context *) h;
}

duk_context *duk_require_context(duk_context *ctx, int index) {
	return (duk_context *) duk_require_hthread(ctx, index);
}

size_t duk_get_length(duk_context *ctx, int index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

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
		return (size_t) DUK_HSTRING_GET_CHARLEN(h);
	}
	case DUK_TAG_OBJECT: {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		return (size_t) duk_hobject_get_length((duk_hthread *) ctx, h);
	}
	case DUK_TAG_BUFFER: {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (size_t) DUK_HBUFFER_GET_SIZE(h);
	}
	default:
		/* number */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return 0;
	}

	DUK_UNREACHABLE();
}

/*
 *  Conversions and coercions
 *
 *  The conversion/coercions are in-place operations on the value stack.
 *  Some operations are implemented here directly, while others call a
 *  helper in duk_js_ops.c after validating arguments.
 */

/* E5 Section 8.12.8 */

static int duk__defaultvalue_coerce_attempt(duk_context *ctx, int index, int func_stridx) {
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

void duk_to_defaultvalue(duk_context *ctx, int index, int hint) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	/* inline initializer for coercers[] is not allowed by old compilers like BCC */
	int coercers[2];

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	coercers[0] = DUK_STRIDX_VALUE_OF;
	coercers[1] = DUK_STRIDX_TO_STRING;

	index = duk_require_normalize_index(ctx, index);

	if (!duk_is_object(ctx, index)) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not object");
	}
	obj = duk_get_hobject(ctx, index);
	DUK_ASSERT(obj != NULL);

	if (hint == DUK_HINT_NONE) {
		if (DUK_HOBJECT_GET_CLASS_NUMBER(obj) == DUK_HOBJECT_CLASS_DATE) {
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

	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "failed to coerce with [[DefaultValue]]");
}

void duk_to_undefined(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;

	DUK_ASSERT(ctx != NULL);
	DUK_UNREF(thr);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_UNDEFINED_ACTUAL(tv);
	DUK_TVAL_DECREF(thr, &tv_temp);
}

void duk_to_null(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;

	DUK_ASSERT(ctx != NULL);
	DUK_UNREF(thr);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NULL(tv);
	DUK_TVAL_DECREF(thr, &tv_temp);
}

/* E5 Section 9.1 */
void duk_to_primitive(duk_context *ctx, int index, int hint) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(hint == DUK_HINT_NONE || hint == DUK_HINT_NUMBER || hint == DUK_HINT_STRING);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_GET_TAG(tv) != DUK_TAG_OBJECT) {
		/* everything except object stay as is */
		return;
	}
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv));

	duk_to_defaultvalue(ctx, index, hint);
}

/* E5 Section 9.2 */
int duk_to_boolean(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	int val;

	DUK_ASSERT(ctx != NULL);
	DUK_UNREF(thr);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	val = duk_js_toboolean(tv);

	/* Note: no need to re-lookup tv, conversion is side effect free */
	DUK_ASSERT(tv != NULL);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_BOOLEAN(tv, val);
	DUK_TVAL_DECREF(thr, &tv_temp);
	return val;
}

double duk_to_number(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = duk_js_tonumber(thr, tv);

	/* Note: need to re-lookup because ToNumber() may have side effects */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	return d;
}

/* XXX: combine all the integer conversions: they share everything
 * but the helper function for coercion.
 */

int duk_to_int(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = duk_js_tointeger(thr, tv);  /* E5 Section 9.4, ToInteger() */

	/* relookup in case duk_js_tointeger() ends up e.g. coercing an object */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	/* Custom coercion for API */
	return duk__api_coerce_d2i(d);
}

int duk_to_int32(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = (double) duk_js_toint32(thr, tv);

	/* must relookup */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	/* ToInt32() should already have restricted the result to
	 * an acceptable range (unless 'int' is less than 32 bits).
	 */
	return (int) d;
}

unsigned int duk_to_uint32(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = (double) duk_js_touint32(thr, tv);

	/* must relookup */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	/* ToUint32() should already have restricted the result to
	 * an acceptable range (unless 'unsigned int' is less than 32 bits).
	 */
	return (unsigned int) d;
}

unsigned int duk_to_uint16(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = (double) duk_js_touint16(thr, tv);

	/* must relookup */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	/* ToUint32() should already have restricted the result to
	 * an acceptable range (unless 'unsigned int' is less than 32 bits).
	 */
	return (unsigned int) d;
}

const char *duk_to_lstring(duk_context *ctx, int index, size_t *out_len) {
	duk_to_string(ctx, index);
	return duk_require_lstring(ctx, index, out_len);
}

static int duk__safe_to_string_raw(duk_context *ctx) {
	duk_to_string(ctx, -1);
	return 1;
}

const char *duk_safe_to_lstring(duk_context *ctx, int index, size_t *out_len) {
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
int duk_to_int_clamped_raw(duk_context *ctx, int index, int minval, int maxval, int *out_clamped) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	duk_tval tv_temp;
	double d;
	int clamped = 0;

	DUK_ASSERT(ctx != NULL);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);
	d = duk_js_tointeger(thr, tv);  /* E5 Section 9.4, ToInteger() */

	if (d < (double) minval) {
		clamped = 1;
		d = (double) minval;
	} else if (d > (double) maxval) {
		clamped = 1;
		d = (double) maxval;
	}

	/* relookup in case duk_js_tointeger() ends up e.g. coercing an object */
	tv = duk_require_tval(ctx, index);
	DUK_TVAL_SET_TVAL(&tv_temp, tv);
	DUK_TVAL_SET_NUMBER(tv, d);  /* no need to incref */
	DUK_TVAL_DECREF(thr, &tv_temp);

	if (out_clamped) {
		*out_clamped = clamped;
	} else {
		/* coerced value is updated to value stack even when RangeError thrown */
		if (clamped) {
			DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "number outside range");
		}
	}

	return (int) d;
}

int duk_to_int_clamped(duk_context *ctx, int index, int minval, int maxval) {
	int dummy;
	return duk_to_int_clamped_raw(ctx, index, minval, maxval, &dummy);
}

int duk_to_int_check_range(duk_context *ctx, int index, int minval, int maxval) {
	return duk_to_int_clamped_raw(ctx, index, minval, maxval, NULL);  /* NULL -> RangeError */
}

const char *duk_to_string(duk_context *ctx, int index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

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
		                 (const char *) DUK_HBUFFER_GET_DATA_PTR(h),
		                 (unsigned int) DUK_HBUFFER_GET_SIZE(h));
		break;
	}
	case DUK_TAG_POINTER: {
		void *ptr = DUK_TVAL_GET_POINTER(tv);
		if (ptr != NULL) {
			duk_push_sprintf(ctx, "%p", ptr);
		} else {
			/* Represent a null pointer as 'null' to be consistent with
			 * the JX format variant.  Native '%p' format for a NULL
			 * pointer may be e.g. '(nil)'.
			 */
			duk_push_hstring_stridx(ctx, DUK_STRIDX_LC_NULL);
		}
		break;
	}
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

/* internal */
duk_hstring *duk_to_hstring(duk_context *ctx, int index) {
	duk_hstring *ret;
	DUK_ASSERT(ctx != NULL);
	duk_to_string(ctx, index);
	ret = duk_get_hstring(ctx, index);
	DUK_ASSERT(ret != NULL);
	return ret;
}

void *duk_to_buffer(duk_context *ctx, int index, size_t *out_size) {
	duk_hbuffer *h_buf;

	index = duk_require_normalize_index(ctx, index);

	if (duk_is_buffer(ctx, index)) {
		/* Buffer is kept as is: note that fixed/dynamic nature of
		 * the buffer is not changed.
		 */
	} else {
		/* Non-buffer value is first ToString() coerced, then converted to
		 * a fixed size buffer.
		 */
		duk_hstring *h_str;
		void *buf;

		duk_to_string(ctx, index);
		h_str = duk_get_hstring(ctx, index);
		DUK_ASSERT(h_str != NULL);

		/* SCANBUILD: NULL pointer dereference warning, never triggered,
		 * asserted above.
		 */
		buf = duk_push_fixed_buffer(ctx, DUK_HSTRING_GET_BYTELEN(h_str));
		DUK_ASSERT(buf != NULL);
		DUK_MEMCPY(buf, DUK_HSTRING_GET_DATA(h_str), DUK_HSTRING_GET_BYTELEN(h_str));
		duk_replace(ctx, index);
	}

	h_buf = duk_get_hbuffer(ctx, index);
	DUK_ASSERT(h_buf != NULL);
	/* SCANBUILD: scan-build produces a NULL pointer dereference warning
	 * below; it never actually triggers because h_buf is actually never
	 * NULL.
	 */

	if (out_size) {
		*out_size = DUK_HBUFFER_GET_SIZE(h_buf);
	}
	return DUK_HBUFFER_GET_DATA_PTR(h_buf);
}

void *duk_to_pointer(duk_context *ctx, int index) {
	duk_tval *tv;
	void *res;

	DUK_ASSERT(ctx != NULL);

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
		/* heap allocated: return heap pointer which is NOT useful
		 * for the caller, except for debugging.
		 */
		res = (void *) DUK_TVAL_GET_HEAPHDR(tv);
		break;
	default:
		/* number */
		res = NULL;
		break;
	}

	duk_push_pointer(ctx, res);
	duk_replace(ctx, index);
	return res;
}

void duk_to_object(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;
	int shared_flags = 0;   /* shared flags for a subset of types */
	int shared_proto = 0;

	DUK_ASSERT(ctx != NULL);

	index = duk_require_normalize_index(ctx, index);

	tv = duk_require_tval(ctx, index);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL: {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "attempt to coerce incompatible value to object");
		break;
	}
	case DUK_TAG_BOOLEAN: {
		shared_flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BOOLEAN);
		shared_proto = DUK_BIDX_BOOLEAN_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_STRING: {
		shared_flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		               DUK_HOBJECT_FLAG_EXOTIC_STRINGOBJ |
		               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_STRING);
		shared_proto = DUK_BIDX_STRING_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_OBJECT: {
		/* nop */
		break;
	}
	case DUK_TAG_BUFFER: {
		shared_flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		               DUK_HOBJECT_FLAG_EXOTIC_BUFFEROBJ |
		               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BUFFER);
		shared_proto = DUK_BIDX_BUFFER_PROTOTYPE;
		goto create_object;
	}
	case DUK_TAG_POINTER: {
		shared_flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_POINTER);
		shared_proto = DUK_BIDX_POINTER_PROTOTYPE;
		goto create_object;
	}
	default: {
		shared_flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
		               DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_NUMBER);
		shared_proto = DUK_BIDX_NUMBER_PROTOTYPE;
		goto create_object;
	}
	}
	return;

 create_object:
	(void) duk_push_object_helper(ctx, shared_flags, shared_proto);

	/* Note: Boolean prototype's internal value property is not writable,
	 * but duk_def_prop_stridx() disregards the write protection.  Boolean
	 * instances are immutable.
	 *
	 * String and buffer special behaviors are already enabled which is not
	 * ideal, but a write to the internal value is not affected by them.
	 */
	duk_dup(ctx, index);
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);

	duk_replace(ctx, index);
}

/*
 *  Type checking
 */

static int duk__tag_check(duk_context *ctx, int index, int tag) {
	duk_tval *tv;

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return 0;
	}
	return (DUK_TVAL_GET_TAG(tv) == tag);
}

static int duk__obj_flag_any_default_false(duk_context *ctx, int index, int flag_mask) {
	duk_hobject *obj;

	DUK_ASSERT(ctx != NULL);

	obj = duk_get_hobject(ctx, index);
	if (obj) {
		return (DUK_HEAPHDR_CHECK_FLAG_BITS((duk_heaphdr *) obj, flag_mask) ? 1 : 0);
	}
	return 0;
}

int duk_get_type(duk_context *ctx, int index) {
	duk_tval *tv;

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
	default:
		/* Note: number has no explicit tag (in 8-byte representation) */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return DUK_TYPE_NUMBER;
	}
	DUK_UNREACHABLE();
}

int duk_check_type(duk_context *ctx, int index, int type) {
	return (duk_get_type(ctx, index) == type) ? 1 : 0;
}

int duk_get_type_mask(duk_context *ctx, int index) {
	duk_tval *tv;

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
	default:
		/* Note: number has no explicit tag (in 8-byte representation) */
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		return DUK_TYPE_MASK_NUMBER;
	}
	DUK_UNREACHABLE();
}

int duk_check_type_mask(duk_context *ctx, int index, int mask) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT(ctx != NULL);
	if (duk_get_type_mask(ctx, index) & mask) {
		return 1;
	}
	if (mask & DUK_TYPE_MASK_THROW) {
		DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "invalid type");
		DUK_UNREACHABLE();
	}
	return 0;
}

int duk_is_undefined(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);
	return duk__tag_check(ctx, index, DUK_TAG_UNDEFINED);
}

int duk_is_null(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);
	return duk__tag_check(ctx, index, DUK_TAG_NULL);
}

int duk_is_null_or_undefined(duk_context *ctx, int index) {
	duk_tval *tv;

	tv = duk_get_tval(ctx, index);
	if (!tv) {
		return 0;
	}
	return (DUK_TVAL_GET_TAG(tv) == DUK_TAG_UNDEFINED) ||
	       (DUK_TVAL_GET_TAG(tv) == DUK_TAG_NULL);
}

int duk_is_boolean(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);
	return duk__tag_check(ctx, index, DUK_TAG_BOOLEAN);
}

int duk_is_number(duk_context *ctx, int index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

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

int duk_is_nan(duk_context *ctx, int index) {
	/* XXX: This will now return false for non-numbers, even though they would
	 * coerce to NaN (as a general rule).  In particular, duk_get_number()
	 * returns a NaN for non-numbers, so should this function also return
	 * true for non-numbers?
	 */

	duk_tval *tv;

	tv = duk_get_tval(ctx, index);
	if (!tv || !DUK_TVAL_IS_NUMBER(tv)) {
		return 0;
	}
	return DUK_ISNAN(DUK_TVAL_GET_NUMBER(tv));
}

int duk_is_string(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);
	return duk__tag_check(ctx, index, DUK_TAG_STRING);
}

int duk_is_object(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);
	return duk__tag_check(ctx, index, DUK_TAG_OBJECT);
}

int duk_is_buffer(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);
	return duk__tag_check(ctx, index, DUK_TAG_BUFFER);
}

int duk_is_pointer(duk_context *ctx, int index) {
	DUK_ASSERT(ctx != NULL);
	return duk__tag_check(ctx, index, DUK_TAG_POINTER);
}

int duk_is_array(duk_context *ctx, int index) {
	duk_hobject *obj;

	DUK_ASSERT(ctx != NULL);

	obj = duk_get_hobject(ctx, index);
	if (obj) {
		return (DUK_HOBJECT_GET_CLASS_NUMBER(obj) == DUK_HOBJECT_CLASS_ARRAY ? 1 : 0);
	}
	return 0;
}

int duk_is_function(duk_context *ctx, int index) {
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_COMPILEDFUNCTION |
	                                       DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	                                       DUK_HOBJECT_FLAG_BOUND);
}

int duk_is_c_function(duk_context *ctx, int index) {
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_NATIVEFUNCTION);
}

int duk_is_ecmascript_function(duk_context *ctx, int index) {
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_COMPILEDFUNCTION);
}

int duk_is_bound_function(duk_context *ctx, int index) {
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_BOUND);
}

int duk_is_thread(duk_context *ctx, int index) {
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_THREAD);
}

int duk_is_callable(duk_context *ctx, int index) {
	/* XXX: currently same as duk_is_function() */
	return duk__obj_flag_any_default_false(ctx,
	                                       index,
	                                       DUK_HOBJECT_FLAG_COMPILEDFUNCTION |
	                                       DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	                                       DUK_HOBJECT_FLAG_BOUND);
}

int duk_is_dynamic(duk_context *ctx, int index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_HAS_DYNAMIC(h) ? 1 : 0);
	}
	return 0;
}

int duk_is_fixed(duk_context *ctx, int index) {
	duk_tval *tv;

	DUK_ASSERT(ctx != NULL);

	tv = duk_get_tval(ctx, index);
	if (DUK_TVAL_IS_BUFFER(tv)) {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);
		return (DUK_HBUFFER_HAS_DYNAMIC(h) ? 0 : 1);
	}
	return 0;
}

int duk_is_primitive(duk_context *ctx, int index) {
	return !duk_is_object(ctx, index);
}

/*
 *  Pushers
 */

/* internal */
void duk_push_tval(duk_context *ctx, duk_tval *tv) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_slot;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(tv != NULL);

	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
	}

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_TVAL(tv_slot, tv);
	DUK_TVAL_INCREF(thr, tv);
	thr->valstack_top++;
}

/* internal */
void duk_push_unused(duk_context *ctx) {
	duk_tval tv;
	DUK_ASSERT(ctx != NULL);
	DUK_TVAL_SET_UNDEFINED_ACTUAL(&tv);
	duk_push_tval(ctx, &tv);
}

void duk_push_undefined(duk_context *ctx) {
	duk_tval tv;
	DUK_ASSERT(ctx != NULL);
	DUK_TVAL_SET_UNDEFINED_ACTUAL(&tv);  /* XXX: heap constant would be nice */
	duk_push_tval(ctx, &tv);
}

void duk_push_null(duk_context *ctx) {
	duk_tval tv;
	DUK_ASSERT(ctx != NULL);
	DUK_TVAL_SET_NULL(&tv);  /* XXX: heap constant would be nice */
	duk_push_tval(ctx, &tv);
}

void duk_push_boolean(duk_context *ctx, int val) {
	duk_tval tv;
	int b = (val ? 1 : 0);
	DUK_ASSERT(ctx != NULL);
	DUK_TVAL_SET_BOOLEAN(&tv, b);
	duk_push_tval(ctx, &tv);
}

void duk_push_true(duk_context *ctx) {
	duk_push_boolean(ctx, 1);
}

void duk_push_false(duk_context *ctx) {
	duk_push_boolean(ctx, 0);
}

void duk_push_number(duk_context *ctx, double val) {
	duk_tval tv;
	duk_double_union du;
	DUK_ASSERT(ctx != NULL);

	/* normalize NaN which may not match our canonical internal NaN */
	du.d = val;
	DUK_DBLUNION_NORMALIZE_NAN_CHECK(&du);

	DUK_TVAL_SET_NUMBER(&tv, du.d);
	duk_push_tval(ctx, &tv);
}

void duk_push_int(duk_context *ctx, int val) {
	duk_push_number(ctx, (double) val);
}

void duk_push_u32(duk_context *ctx, duk_uint32_t val) {
	duk_push_number(ctx, (double) val);
}

void duk_push_nan(duk_context *ctx) {
	duk_push_number(ctx, DUK_DOUBLE_NAN);
}

const char *duk_push_lstring(duk_context *ctx, const char *str, size_t len) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_tval *tv_slot;

	DUK_ASSERT(ctx != NULL);

	/* check stack before interning (avoid hanging temp) */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
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
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "string too long");
	}

	h = duk_heap_string_intern_checked(thr, (duk_uint8_t *) str, (duk_uint32_t) len);
	DUK_ASSERT(h != NULL);

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_STRING(tv_slot, h);
	DUK_HSTRING_INCREF(thr, h);
	thr->valstack_top++;

	return (const char *) DUK_HSTRING_GET_DATA(h);
}

const char *duk_push_string(duk_context *ctx, const char *str) {
	DUK_ASSERT(ctx != NULL);

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
const char *duk_push_string_file(duk_context *ctx, const char *path) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_file *f = NULL;
	char *buf;
	long sz;

	DUK_ASSERT(ctx != NULL);
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
	buf = (char *) duk_push_fixed_buffer(ctx, (size_t) sz);
	DUK_ASSERT(buf != NULL);
	if (DUK_FREAD(buf, 1, sz, f) != (size_t) sz) {
		goto fail;
	}
	(void) DUK_FCLOSE(f);  /* ignore fclose() error */
	f = NULL;
	return duk_to_string(ctx, -1);

 fail:
	if (f) {
		DUK_FCLOSE(f);
	}
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "failed to read file");
	return NULL;
}
#else
const char *duk_push_string_file(duk_context *ctx, const char *path) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT(ctx != NULL);
	DUK_UNREF(path);
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "file I/O disabled");
	return NULL;
}
#endif  /* DUK_USE_FILE_IO */

void duk_push_pointer(duk_context *ctx, void *val) {
	duk_tval tv;
	DUK_ASSERT(ctx != NULL);

	DUK_TVAL_SET_POINTER(&tv, val);
	duk_push_tval(ctx, &tv);
}

#define DUK__PUSH_THIS_FLAG_CHECK_COERC  (1 << 0)
#define DUK__PUSH_THIS_FLAG_TO_OBJECT    (1 << 1)
#define DUK__PUSH_THIS_FLAG_TO_STRING    (1 << 2)

static void duk__push_this_helper(duk_context *ctx, int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);
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
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "not object coercible");
}

void duk_push_this(duk_context *ctx) {
	duk__push_this_helper(ctx, 0 /*flags*/);
}

void duk_push_this_check_object_coercible(duk_context *ctx) {
	duk__push_this_helper(ctx, DUK__PUSH_THIS_FLAG_CHECK_COERC /*flags*/);
}

duk_hobject *duk_push_this_coercible_to_object(duk_context *ctx) {
	duk_hobject *h;
	duk__push_this_helper(ctx, DUK__PUSH_THIS_FLAG_CHECK_COERC |
	                           DUK__PUSH_THIS_FLAG_TO_OBJECT /*flags*/);
	h = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h != NULL);
	return h;
}

duk_hstring *duk_push_this_coercible_to_string(duk_context *ctx) {
	duk_hstring *h;
	duk__push_this_helper(ctx, DUK__PUSH_THIS_FLAG_CHECK_COERC |
	                           DUK__PUSH_THIS_FLAG_TO_STRING /*flags*/);
	h = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h != NULL);
	return h;
}

void duk_push_current_function(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT_DISABLE(thr->callstack_top >= 0);
	DUK_ASSERT(thr->callstack_top <= thr->callstack_size);

	if (thr->callstack_top == 0) {
		duk_push_undefined(ctx);
	} else {
		duk_activation *act = thr->callstack + thr->callstack_top - 1;
		DUK_ASSERT(act->func != NULL);
		duk_push_hobject(ctx, act->func);
	}
}

void duk_push_current_thread(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(ctx != NULL);

	if (thr->heap->curr_thread) {
		duk_push_hobject(ctx, (duk_hobject *) thr->heap->curr_thread);
	} else {
		duk_push_undefined(ctx);
	}
}

void duk_push_global_object(duk_context *ctx) {
	DUK_ASSERT(ctx != NULL);

	duk_push_hobject_bidx(ctx, DUK_BIDX_GLOBAL);
}

/* XXX: size optimize */
static void duk__push_stash(duk_context *ctx) {
	DUK_ASSERT(ctx != NULL);
	if (!duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE)) {
		DUK_DDD(DUK_DDDPRINT("creating heap/global/thread stash on first use"));
		duk_pop(ctx);
		duk_push_object_internal(ctx);
		duk_dup_top(ctx);
		duk_def_prop_stridx(ctx, -3, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_C);  /* [ ... parent stash stash ] -> [ ... parent stash ] */
	}
	duk_remove(ctx, -2);
}

void duk_push_heap_stash(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;
	DUK_ASSERT(ctx != NULL);
	heap = thr->heap;
	DUK_ASSERT(heap->heap_object != NULL);
	duk_push_hobject(ctx, heap->heap_object);
	duk__push_stash(ctx);
}

void duk_push_global_stash(duk_context *ctx) {
	DUK_ASSERT(ctx != NULL);
	duk_push_global_object(ctx);
	duk__push_stash(ctx);
}

void duk_push_thread_stash(duk_context *ctx, duk_context *target_ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT(ctx != NULL);
	if (!target_ctx) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "invalid argument(s)");
		return;  /* not reached */
	}
	duk_push_hobject(ctx, (duk_hobject *) target_ctx);
	duk__push_stash(ctx);
}

static int duk__try_push_vsprintf(duk_context *ctx, void *buf, size_t sz, const char *fmt, va_list ap) {
	int len;

	DUK_UNREF(ctx);

	/* NUL terminator handling doesn't matter here */
	len = DUK_VSNPRINTF((char *) buf, sz, fmt, ap);
	if ((size_t) len < sz) {
		return len;
	}
	return -1;
}

const char *duk_push_vsprintf(duk_context *ctx, const char *fmt, va_list ap) {
	duk_hthread *thr = (duk_hthread *) ctx;
	size_t sz = DUK_PUSH_SPRINTF_INITIAL_SIZE;
	void *buf;
	int len;
	const char *res;

	DUK_ASSERT(ctx != NULL);

	/* special handling of fmt==NULL */
	if (!fmt) {
		duk_hstring *h_str;
		duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);
		h_str = DUK_HTHREAD_STRING_EMPTY_STRING(thr);  /* rely on interning, must be this string */
		return (const char *) DUK_HSTRING_GET_DATA(h_str);
	}

	/* initial estimate based on format string */
	sz = DUK_STRLEN(fmt) + 16;  /* XXX: plus something to avoid just missing */
	if (sz < DUK_PUSH_SPRINTF_INITIAL_SIZE) {
		sz = DUK_PUSH_SPRINTF_INITIAL_SIZE;
	}
	DUK_ASSERT(sz > 0);

	buf = duk_push_dynamic_buffer(ctx, sz);

	for (;;) {
		va_list ap_copy;  /* copied so that 'ap' can be reused */

		DUK_VA_COPY(ap_copy, ap);
		len = duk__try_push_vsprintf(ctx, buf, sz, fmt, ap_copy);
		va_end(ap_copy);
		if (len >= 0) {
			break;
		}

		/* failed, resize and try again */
		sz = sz * 2;
		if (sz >= DUK_PUSH_SPRINTF_SANITY_LIMIT) {
			DUK_ERROR(thr, DUK_ERR_API_ERROR, "cannot sprintf, required buffer insanely long");
		}

		buf = duk_resize_buffer(ctx, -1, sz);
		DUK_ASSERT(buf != NULL);
	}

	/* Cannot use duk_to_string() on the buffer because it is usually
	 * larger than 'len'.
	 */
	res = duk_push_lstring(ctx, (const char *) buf, (size_t) len);  /* [buf res] */
	duk_remove(ctx, -2);
	return res;
}

const char *duk_push_sprintf(duk_context *ctx, const char *fmt, ...) {
	va_list ap;
	const char *retval;

	/* allow fmt==NULL */
	va_start(ap, fmt);
	retval = duk_push_vsprintf(ctx, fmt, ap);
	va_end(ap);

	return retval;
}

/* internal */
int duk_push_object_helper(duk_context *ctx, int hobject_flags_and_class, int prototype_bidx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_slot;
	duk_hobject *h;
	int ret;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(prototype_bidx == -1 ||
	           (prototype_bidx >= 0 && prototype_bidx < DUK_NUM_BUILTINS));

	/* check stack before interning (avoid hanging temp) */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
	}

	h = duk_hobject_alloc(thr->heap, hobject_flags_and_class);
	if (!h) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to allocate an object");
	}

	DUK_DDD(DUK_DDDPRINT("created object with flags: 0x%08x", h->hdr.h_flags));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, h);
	DUK_HOBJECT_INCREF(thr, h);
	ret = (int) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* object is now reachable */

	if (prototype_bidx >= 0) {
		DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, thr->builtins[prototype_bidx]);
	} else {
		DUK_ASSERT(prototype_bidx == -1);
		DUK_ASSERT(h->prototype == NULL);
	}

	return ret;
}

int duk_push_object_helper_proto(duk_context *ctx, int hobject_flags_and_class, duk_hobject *proto) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int ret;
	duk_hobject *h;

	ret = duk_push_object_helper(ctx, hobject_flags_and_class, -1);
	h = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(h->prototype == NULL);
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h, proto);
	return ret;
}

int duk_push_object(duk_context *ctx) {
	return duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              DUK_BIDX_OBJECT_PROTOTYPE);
}

int duk_push_array(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
	int ret;

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
	 */

	duk_push_number(ctx, 0.0);
	duk_hobject_define_property_internal(thr,
	                                     obj,
	                                     DUK_HTHREAD_STRING_LENGTH(thr),
	                                     DUK_PROPDESC_FLAGS_W);
	DUK_HOBJECT_SET_EXOTIC_ARRAY(obj);

	return ret;
}

int duk_push_thread_raw(duk_context *ctx, int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hthread *obj;
	int ret;
	duk_tval *tv_slot;

	DUK_ASSERT(ctx != NULL);

	/* check stack before interning (avoid hanging temp) */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
	}

	obj = duk_hthread_alloc(thr->heap,
	                        DUK_HOBJECT_FLAG_EXTENSIBLE |
	                        DUK_HOBJECT_FLAG_THREAD |
	                        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_THREAD));
	if (!obj) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to allocate a thread object");
	}
	obj->state = DUK_HTHREAD_STATE_INACTIVE;
	obj->strs = thr->strs;
	DUK_DDD(DUK_DDDPRINT("created thread object with flags: 0x%08x", obj->obj.hdr.h_flags));

	/* make the new thread reachable */
	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HTHREAD_INCREF(thr, obj);
	ret = (int) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* important to do this *after* pushing, to make the thread reachable for gc */
	if (!duk_hthread_init_stacks(thr->heap, obj)) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to allocate thread");
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

int duk_push_compiledfunction(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hcompiledfunction *obj;
	int ret;
	duk_tval *tv_slot;

	DUK_ASSERT(ctx != NULL);

	/* check stack before interning (avoid hanging temp) */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
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
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to allocate a function object");
	}

	DUK_DDD(DUK_DDDPRINT("created compiled function object with flags: 0x%08x", obj->obj.hdr.h_flags));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HOBJECT_INCREF(thr, obj);
	ret = (int) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* default prototype (Note: 'obj' must be reachable) */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE]);

	return ret;
}

static int duk__push_c_function_raw(duk_context *ctx, duk_c_function func, int nargs, duk_uint32_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hnativefunction *obj;
	int ret;
	duk_tval *tv_slot;
	duk_uint16_t func_nargs;

	DUK_ASSERT(ctx != NULL);

	/* check stack before interning (avoid hanging temp) */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
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
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to allocate a function object");
	}

	obj->func = func;
	obj->nargs = func_nargs;

	DUK_DDD(DUK_DDDPRINT("created native function object with flags: 0x%08x, nargs=%d", obj->obj.hdr.h_flags, obj->nargs));

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_OBJECT(tv_slot, (duk_hobject *) obj);
	DUK_HOBJECT_INCREF(thr, obj);
	ret = (int) (thr->valstack_top - thr->valstack_bottom);
	thr->valstack_top++;

	/* default prototype (Note: 'obj' must be reachable) */
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, (duk_hobject *) obj, thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE]);

	return ret;

 api_error:
	DUK_ERROR(thr, DUK_ERR_API_ERROR, "invalid argument(s)");
	return 0;  /* not reached */
}

int duk_push_c_function(duk_context *ctx, duk_c_function func, int nargs) {
	duk_uint32_t flags;

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	        DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	        DUK_HOBJECT_FLAG_NEWENV |
	        DUK_HOBJECT_FLAG_STRICT |
	        DUK_HOBJECT_FLAG_EXOTIC_DUKFUNC |
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);
	
	return duk__push_c_function_raw(ctx, func, nargs, flags);
}

void duk_push_c_function_noexotic(duk_context *ctx, duk_c_function func, int nargs) {
	duk_uint32_t flags;

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_CONSTRUCTABLE |
	        DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	        DUK_HOBJECT_FLAG_NEWENV |
	        DUK_HOBJECT_FLAG_STRICT |
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);
	
	(void) duk__push_c_function_raw(ctx, func, nargs, flags);
}

void duk_push_c_function_noconstruct_noexotic(duk_context *ctx, duk_c_function func, int nargs) {
	duk_uint32_t flags;

	flags = DUK_HOBJECT_FLAG_EXTENSIBLE |
	        DUK_HOBJECT_FLAG_NATIVEFUNCTION |
	        DUK_HOBJECT_FLAG_NEWENV |
	        DUK_HOBJECT_FLAG_STRICT |
	        DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_FUNCTION);
	
	(void) duk__push_c_function_raw(ctx, func, nargs, flags);
}

static int duk__push_error_object_vsprintf(duk_context *ctx, int err_code, const char *filename, int line, const char *fmt, va_list ap) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int retval;
	duk_hobject *proto;
#ifdef DUK_USE_AUGMENT_ERROR_CREATE
	int noblame_fileline;
#endif

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);

	/* Error code also packs a tracedata related flag. */
#ifdef DUK_USE_AUGMENT_ERROR_CREATE
	noblame_fileline = err_code & DUK_ERRCODE_FLAG_NOBLAME_FILELINE;
#endif
	err_code = err_code & (~DUK_ERRCODE_FLAG_NOBLAME_FILELINE);

	/* error gets its 'name' from the prototype */
	proto = duk_error_prototype_from_code(thr, err_code);
	retval = duk_push_object_helper_proto(ctx,
	                                      DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                      DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ERROR),
	                                      proto);

	/* ... and its 'message' from an instance property */
	if (fmt) {
		duk_push_vsprintf(ctx, fmt, ap);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	} else {
		/* If no explicit message given, put error code into message field
		 * (as a number).  This is not fully in keeping with the Ecmascript
		 * error model because messages are supposed to be strings (Error
		 * constructors use ToString() on their argument).  However, it's
		 * probably more useful than having a separate 'code' property.
		 */
		duk_push_int(ctx, err_code);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	}

#if 0
	/* Disabled for now, not sure this is a useful property */
	duk_push_int(ctx, err_code);
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_CODE, DUK_PROPDESC_FLAGS_WC);
#endif

	/* Creation time error augmentation */
#ifdef DUK_USE_AUGMENT_ERROR_CREATE
	/* filename may be NULL in which case file/line is not recorded */
	duk_err_augment_error_create(thr, thr, filename, line, noblame_fileline);  /* may throw an error */
#endif

	return retval;
}

int duk_push_error_object_raw(duk_context *ctx, int err_code, const char *filename, int line, const char *fmt, ...) {
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = duk__push_error_object_vsprintf(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	return ret;
}

#ifndef DUK_USE_VARIADIC_MACROS
int duk_push_error_object_stash(duk_context *ctx, int err_code, const char *fmt, ...) {
	const char *filename = duk_api_global_filename;
	int line = duk_api_global_line;
	va_list ap;
	int ret;

	duk_api_global_filename = NULL;
	duk_api_global_line = 0;
	va_start(ap, fmt);
	ret = duk__push_error_object_vsprintf(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	return ret;
}
#endif

/* XXX: repetition, see duk_push_object */
void *duk_push_buffer(duk_context *ctx, size_t size, int dynamic) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv_slot;
	duk_hbuffer *h;

	DUK_ASSERT(ctx != NULL);

	/* check stack before interning (avoid hanging temp) */
	if (thr->valstack_top >= thr->valstack_end) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to push beyond currently allocated stack");
	}

	/* Check for maximum buffer length. */
	if (size > DUK_HBUFFER_MAX_BYTELEN) {
		DUK_ERROR(thr, DUK_ERR_RANGE_ERROR, "buffer too long");
	}

	h = duk_hbuffer_alloc(thr->heap, size, dynamic);
	if (!h) {
		DUK_ERROR(thr, DUK_ERR_ALLOC_ERROR, "failed to allocate buffer");
	}

	tv_slot = thr->valstack_top;
	DUK_TVAL_SET_BUFFER(tv_slot, h);
	DUK_HBUFFER_INCREF(thr, h);
	thr->valstack_top++;

	return DUK_HBUFFER_GET_DATA_PTR(h);
}

void *duk_push_fixed_buffer(duk_context *ctx, size_t size) {
	return duk_push_buffer(ctx, size, 0);
}

void *duk_push_dynamic_buffer(duk_context *ctx, size_t size) {
	return duk_push_buffer(ctx, size, 1);
}

int duk_push_object_internal(duk_context *ctx) {
	return duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              -1);  /* no prototype */
}

/* internal */
void duk_push_hstring(duk_context *ctx, duk_hstring *h) {
	duk_tval tv;
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(h != NULL);
	DUK_TVAL_SET_STRING(&tv, h);
	duk_push_tval(ctx, &tv);
}

void duk_push_hstring_stridx(duk_context *ctx, int stridx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);
	duk_push_hstring(ctx, thr->strs[stridx]);
}

/* internal */
void duk_push_hobject(duk_context *ctx, duk_hobject *h) {
	duk_tval tv;
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(h != NULL);
	DUK_TVAL_SET_OBJECT(&tv, h);
	duk_push_tval(ctx, &tv);
}

/* internal */
void duk_push_hbuffer(duk_context *ctx, duk_hbuffer *h) {
	duk_tval tv;
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(h != NULL);
	DUK_TVAL_SET_BUFFER(&tv, h);
	duk_push_tval(ctx, &tv);
}

/* internal */
void duk_push_hobject_bidx(duk_context *ctx, int builtin_idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(builtin_idx >= 0 && builtin_idx < DUK_NUM_BUILTINS);
	DUK_ASSERT(thr->builtins[builtin_idx] != NULL);  /* FIXME: this assumption may need be to relaxed! */
	duk_push_hobject(ctx, thr->builtins[builtin_idx]);
}

/*
 *  Poppers
 */

void duk_pop_n(duk_context *ctx, unsigned int count) {
	duk_hthread *thr = (duk_hthread *) ctx;
	DUK_ASSERT(ctx != NULL);

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	if ((size_t) (thr->valstack_top - thr->valstack_bottom) < (size_t) count) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "attempt to pop too many entries");
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

		thr->valstack_top--;
		tv = thr->valstack_top;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		DUK_TVAL_SET_TVAL(&tv_tmp, tv);
		DUK_TVAL_SET_UNDEFINED_UNUSED(tv);
		DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
		count--;
	}
#else
	while (count > 0) {
		duk_tval *tv;

		thr->valstack_top--;
		tv = thr->valstack_top;
		DUK_ASSERT(tv >= thr->valstack_bottom);
		DUK_TVAL_SET_UNDEFINED_UNUSED(tv);
		count--;
	}

#endif

	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
}

void duk_pop(duk_context *ctx) {
	duk_pop_n(ctx, 1);
}

void duk_pop_2(duk_context *ctx) {
	duk_pop_n(ctx, 2);
}

void duk_pop_3(duk_context *ctx) {
	duk_pop_n(ctx, 3);
}

/*
 *  Error throwing
 */

void duk_throw(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(thr->valstack_bottom >= thr->valstack);
	DUK_ASSERT(thr->valstack_top >= thr->valstack_bottom);
	DUK_ASSERT(thr->valstack_end >= thr->valstack_top);

	if (thr->valstack_top == thr->valstack_bottom) {
		DUK_ERROR(thr, DUK_ERR_API_ERROR, "no value to throw");
	}

	/* Errors are augmented when they are created, not when they are
	 * thrown or re-thrown.  The current error handler, however, runs
	 * just before an error is thrown.
	 */

#if defined(DUK_USE_AUGMENT_ERROR_THROW)
	DUK_DDD(DUK_DDDPRINT("THROW ERROR (API): %!dT (before throw augment)", duk_get_tval(ctx, -1)));
	duk_err_augment_error_throw(thr);
#endif
	DUK_DDD(DUK_DDDPRINT("THROW ERROR (API): %!dT (after throw augment)", duk_get_tval(ctx, -1)));

	duk_err_setup_heap_ljstate(thr, DUK_LJ_TYPE_THROW);

	/* thr->heap->lj.jmpbuf_ptr is checked by duk_err_longjmp() so we don't
	 * need to check that here.  If the value is NULL, a panic occurs because
	 * we can't return.
	 */

	duk_err_longjmp(thr);
	DUK_UNREACHABLE();
}

void duk_fatal(duk_context *ctx, int err_code, const char *err_msg) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(thr->heap->fatal_func != NULL);

	DUK_D(DUK_DPRINT("fatal error occurred, code %d, message %s", err_code, err_msg));

	/* fatal_func should be noreturn, but noreturn declarations on function
	 * pointers has a very spotty support apparently so it's not currently
	 * done.
	 */
	thr->heap->fatal_func(ctx, err_code, err_msg);

	DUK_PANIC(DUK_ERR_API_ERROR, "fatal handler returned");
}

void duk_error_raw(duk_context *ctx, int err_code, const char *filename, int line, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	duk__push_error_object_vsprintf(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	duk_throw(ctx);
}

#ifndef DUK_USE_VARIADIC_MACROS
void duk_error_stash(duk_context *ctx, int err_code, const char *fmt, ...) {
	const char *filename = duk_api_global_filename;
	int line = duk_api_global_line;
	va_list ap;

	duk_api_global_filename = NULL;
	duk_api_global_line = 0;

	va_start(ap, fmt);
	duk__push_error_object_vsprintf(ctx, err_code, filename, line, fmt, ap);
	va_end(ap);
	duk_throw(ctx);
}
#endif

int duk_equals(duk_context *ctx, int index1, int index2) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv1, *tv2;

	tv1 = duk_get_tval(ctx, index1);
	if (!tv1) {
		return 0;
	}
	tv2 = duk_get_tval(ctx, index2);
	if (!tv2) {
		return 0;
	}

	/* Coercion may be needed, the helper handles that by pushing the
	 * tagged values to the stack.
	 */
	return duk_js_equals(thr, tv1, tv2);

}

int duk_strict_equals(duk_context *ctx, int index1, int index2) {
	duk_tval *tv1, *tv2;

	tv1 = duk_get_tval(ctx, index1);
	if (!tv1) {
		return 0;
	}
	tv2 = duk_get_tval(ctx, index2);
	if (!tv2) {
		return 0;
	}

	/* No coercions or other side effects, so safe */
	return duk_js_strict_equals(tv1, tv2);
}

/*
 *  Heap creation
 */

duk_context *duk_create_heap(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             void *alloc_udata,
                             duk_fatal_function fatal_handler) {
	duk_heap *heap = NULL;
	duk_context *ctx;

	/* Assume that either all memory funcs are NULL or non-NULL, mixed
	 * cases will now be unsafe.
	 */

	/* XXX: just assert non-NULL values here and make caller arguments
	 * do the defaulting to the default implementations (smaller code)?
	 */

	if (!alloc_func) {
		DUK_ASSERT(realloc_func == NULL);
		DUK_ASSERT(free_func == NULL);
		alloc_func = duk_default_alloc_function;
		realloc_func = duk_default_realloc_function;
		free_func = duk_default_free_function;
	} else {
		DUK_ASSERT(realloc_func != NULL);
		DUK_ASSERT(free_func != NULL);
	}

	if (!fatal_handler) {
		fatal_handler = duk_default_fatal_handler;
	}

	DUK_ASSERT(alloc_func != NULL);
	DUK_ASSERT(realloc_func != NULL);
	DUK_ASSERT(free_func != NULL);
	DUK_ASSERT(fatal_handler != NULL);

	heap = duk_heap_alloc(alloc_func, realloc_func, free_func, alloc_udata, fatal_handler);
	if (!heap) {
		return NULL;
	}
	ctx = (duk_context *) heap->heap_thread;
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(((duk_hthread *) ctx)->heap != NULL);
	return ctx;
}

void duk_destroy_heap(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;

	if (!ctx) {
		return;
	}
	heap = thr->heap;
	DUK_ASSERT(heap != NULL);

	duk_heap_free(heap);
}

