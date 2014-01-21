/*
 *  Array built-ins
 *
 *  Note that most Array built-ins are intentionally generic and work even
 *  when the 'this' binding is not an Array instance.  To ensure this,
 *  Array algorithms do not assume "magical" Array behavior for the "length"
 *  property, for instance.
 *
 *  FIXME: the "Throw" flag should be set for (almost?) all [[Put]] and
 *  [[Delete]] operations, but it's currently false throughout.  C typing
 *  is incorrect in several places, and array lengths above 2G won't work
 *  reliably.  Further, some valid array length values may be above 2**32-1,
 *  and this is not always correctly handled.
 */

#include "duk_internal.h"

/* Perform an intermediate join when this many elements have been pushed
 * on the value stack.
 */
#define  DUK_ARRAY_MID_JOIN_LIMIT  4096

/* Shared entry code for many Array built-ins.  Note that length is left
 * on stack (it could be popped, but that's not necessary).
 */
static unsigned int push_this_obj_len_u32(duk_context *ctx) {
	unsigned int len;

	(void) duk_push_this_coercible_to_object(ctx);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_LENGTH);
	len = duk_to_uint32(ctx, -1);

	/* -> [ ... ToObject(this) ToUint32(length) ] */
	return len;
}

/*
 *  Constructor
 */

int duk_bi_array_constructor(duk_context *ctx) {
	int nargs;
	double d;
	duk_uint32_t len;
	int i;

	nargs = duk_get_top(ctx);
	duk_push_array(ctx);

	if (nargs == 1 && duk_is_number(ctx, 0)) {
		/* FIXME: expensive check */
		d = duk_get_number(ctx, 0);
		len = duk_to_uint32(ctx, 0);
		if (((double) len) != d) {
			return DUK_RET_RANGE_ERROR;
		}

		/* FIXME: if 'len' is low, may want to ensure array part is kept:
		 * the caller is likely to want a dense array.
		 */
		duk_dup(ctx, 0);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_WC);  /* [ ToUint32(len) array ToUint32(len) ] -> [ ToUint32(len) array ] */
		return 1;
	}

	/* FIXME: optimize by creating array into correct size directly, and
	 * operating on the array part directly; values can be memcpy()'d from
	 * value stack directly as long as refcounts are increased.
	 */
	for (i = 0; i < nargs; i++) {
		duk_dup(ctx, i);
		duk_def_prop_index(ctx, -2, i, DUK_PROPDESC_FLAGS_WEC);
	}

	duk_push_number(ctx, (double) nargs);  /* FIXME: push_u32 */
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_WC);
	return 1;
}

/*
 *  isArray()
 */

int duk_bi_array_constructor_is_array(duk_context *ctx) {
	duk_hobject *h;

	h = duk_get_hobject_with_class(ctx, 0, DUK_HOBJECT_CLASS_ARRAY);
	duk_push_boolean(ctx, (h != NULL));
	return 1;
}

/*
 *  toString()
 */

int duk_bi_array_prototype_to_string(duk_context *ctx) {
	(void) duk_push_this_coercible_to_object(ctx);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_JOIN);

	/* [ ... this func ] */
	if (!duk_is_callable(ctx, -1)) {
		/* Fall back to the initial (original) Object.toString().  We don't
		 * currently have pointers to the built-in functions, only the top
		 * level global objects (like "Array") so this is now done in a bit
		 * of a hacky manner.  It would be cleaner to push the (original)
		 * function and use duk_call_method().
		 */

		/* FIXME: 'this' will be ToObject() coerced twice, which is incorrect
		 * but should have no visible side effects.
		 */
		DUK_DDDPRINT("this.join is not callable, fall back to (original) Object.toString");
		duk_set_top(ctx, 0);
		return duk_bi_object_prototype_to_string(ctx);
	}

	/* [ ... this func ] */

	duk_insert(ctx, -2);

	/* [ ... func this ] */

	DUK_DDDPRINT("calling: func=%!iT, this=%!iT", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
	duk_call_method(ctx, 0);

	return 1;
}

/*
 *  concat()
 */

int duk_bi_array_prototype_concat(duk_context *ctx) {
	int i, n;
	int j, len;
	int idx, idx_last;
	duk_hobject *h;

	/* FIXME: the insert here is a bit expensive if there are a lot of items.
	 * It could also be special cased in the outermost for loop quite easily
	 * (as the element is dup()'d anyway).
	 */

	(void) duk_push_this_coercible_to_object(ctx);
	duk_insert(ctx, 0);
	n = duk_get_top(ctx);
	duk_push_array(ctx);  /* -> [ ToObject(this) item1 ... itemN arr ] */

	/* FIXME: the duk_def_prop_index() calls are currently slow as they intern
	 * the index as a string.  Further, the Array special behaviors are NOT
	 * invoked (which differs from the official algorithm).  If no error is
	 * thrown, this doesn't matter as the length is updated at the end.  However,
	 * if an error is thrown, the length will be unset.  That shouldn't matter
	 * because the caller won't get a reference to the intermediate value.
	 */

	idx = 0;
	idx_last = 0;
	for (i = 0; i < n; i++) {
		DUK_ASSERT_TOP(ctx, n + 1);

		/* [ ToObject(this) item1 ... itemN arr ] */

		duk_dup(ctx, i);
		h = duk_get_hobject_with_class(ctx, -1, DUK_HOBJECT_CLASS_ARRAY);
		if (!h) {
			duk_def_prop_index(ctx, -2, idx++, DUK_PROPDESC_FLAGS_WEC);
			idx_last = idx;
			continue;
		}

		/* [ ToObject(this) item1 ... itemN arr item(i) ] */

		/* FIXME: an array can have length higher than 32 bits; this is not handled
		 * correctly now (also len is signed so length above 2**31-1 will have trouble.
		 */
		len = duk_get_length(ctx, -1);
		for (j = 0; j < len; j++) {
			if (duk_get_prop_index(ctx, -1, j)) {
				/* [ ToObject(this) item1 ... itemN arr item(i) item(i)[j] ] */
				duk_def_prop_index(ctx, -3, idx++, DUK_PROPDESC_FLAGS_WEC);
				idx_last = idx;
			} else {
				/* XXX: according to E5.1 Section 15.4.4.4 nonexistent trailing
				 * elements do not affect 'length' but test262 disagrees.  Work
				 * as E5.1 mandates for now and don't touch idx_last.
				 */
				idx++;
				duk_pop(ctx);
			}
		}
		duk_pop(ctx);
	}

	duk_push_number(ctx, (double) idx_last);
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_LENGTH, DUK_PROPDESC_FLAGS_WC);

	DUK_ASSERT_TOP(ctx, n + 1);
	return 1;
}

/*
 *  join(), toLocaleString()
 *
 *  Note: checking valstack is necessary, but only in the per-element loop.
 *
 *  Note: the trivial approach of pushing all the elements on the value stack
 *  and then calling duk_join() fails when the array contains a large number
 *  of elements.  This problem can't be offloaded to duk_join() because the
 *  elements to join must be handled here and have special handling.  Current
 *  approach is to do intermediate joins with very large number of elements.
 *  There is no fancy handling; the prefix gets re-joined multiple times.
 */

int duk_bi_array_prototype_join_shared(duk_context *ctx) {
	duk_uint32_t len, count;
	duk_uint32_t idx;
	duk_small_int_t to_locale_string = duk_get_magic(ctx);
	duk_int_t valstack_required;

	/* For join(), nargs is 1.  For toLocaleString(), nargs is 0 and
	 * setting the top essentially pushes an undefined to the stack,
	 * thus defaulting to a comma separator.
	 */
	duk_set_top(ctx, 1);
	if (duk_is_undefined(ctx, 0)) {
		duk_pop(ctx);
		duk_push_hstring_stridx(ctx, DUK_STRIDX_COMMA);
	} else {
		duk_to_string(ctx, 0);
	}

	len = push_this_obj_len_u32(ctx);

	/* [ sep ToObject(this) len ] */

	DUK_DDDPRINT("sep=%!T, this=%!T, len=%d",
	             duk_get_tval(ctx, 0), duk_get_tval(ctx, 1), (int) len);

	valstack_required = (len >= DUK_ARRAY_MID_JOIN_LIMIT ?
	                     DUK_ARRAY_MID_JOIN_LIMIT : len);
	valstack_required++;
	duk_require_stack(ctx, valstack_required);

	duk_dup(ctx, 0);

	/* [ sep ToObject(this) len sep ] */

	count = 0;
	idx = 0;
	for (;;) {
		if (count >= DUK_ARRAY_MID_JOIN_LIMIT ||   /* intermediate join to avoid valstack overflow */
		    idx >= len) { /* end of loop (careful with len==0) */
			/* [ sep ToObject(this) len sep str0 ... str(count-1) ] */
			DUK_DDDPRINT("mid/final join, count=%d, idx=%d, len=%d",
			             (int) count, (int) idx, (int) len);
			duk_join(ctx, count);  /* -> [ sep ToObject(this) len str ] */
			duk_dup(ctx, 0);       /* -> [ sep ToObject(this) len str sep ] */
			duk_insert(ctx, -2);   /* -> [ sep ToObject(this) len sep str ] */
			count = 1;
		}
		if (idx >= len) {
			/* if true, the stack already contains the final result */
			break;
		}

		duk_get_prop_index(ctx, 1, idx);
		if (duk_is_null_or_undefined(ctx, -1)) {
			duk_pop(ctx);
			duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);
		} else {
			if (to_locale_string) {
				duk_to_object(ctx, -1);
				duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TO_LOCALE_STRING);
				duk_insert(ctx, -2);  /* -> [ ... toLocaleString ToObject(val) ] */
				duk_call_method(ctx, 0);
				duk_to_string(ctx, -1);
			} else {
				duk_to_string(ctx, -1);
			}
		}

		count++;
		idx++;
	}

	/* [ sep ToObject(this) len sep result ] */

	return 1;
}

/*
 *  pop(), push()
 */

int duk_bi_array_prototype_pop(duk_context *ctx) {
	unsigned int len;
	unsigned int idx;

	DUK_ASSERT_TOP(ctx, 0);
	len = push_this_obj_len_u32(ctx);
	if (len == 0) {
		duk_push_int(ctx, 0);
		duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LENGTH);  /* FIXME: Throw */
		return 0;
	}
	idx = len - 1;

	duk_get_prop_index(ctx, 0, idx);
	duk_del_prop_index(ctx, 0, idx);  /* FIXME: Throw */
	duk_push_int(ctx, idx);  /* FIXME: unsigned */
	duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LENGTH);  /* FIXME: Throw */
	return 1;
}

int duk_bi_array_prototype_push(duk_context *ctx) {
	/* Note: 'this' is not necessarily an Array object.  The push()
	 * algorithm is supposed to work for other kinds of objects too,
	 * so the algorithm has e.g. an explicit update for the 'length'
	 * property which is normally "magical" in arrays.
	 */

	double len;
	int i, n;

	n = duk_get_top(ctx);
	len = (double) push_this_obj_len_u32(ctx);

	/* [ arg1 ... argN obj length ] */

	/* Note: we keep track of length with a double instead of a 32-bit
	 * (unsigned) int because the length can go beyond 32 bits and the
	 * final length value is NOT wrapped to 32 bits on this call.
	 */

	for (i = 0; i < n; i++) {
		duk_push_number(ctx, len);
		duk_dup(ctx, i);
		duk_put_prop(ctx, -4);  /* FIXME: "Throw" */
		len += 1.0;
	}

	duk_push_number(ctx, len);
	duk_dup_top(ctx);
	duk_put_prop_stridx(ctx, -4, DUK_STRIDX_LENGTH);

	/* [ arg1 ... argN obj length new_length ] */
	return 1;
}

/*
 *  sort()
 *
 *  Currently qsort with random pivot.  This is now really, really slow,
 *  because there is no fast path for array parts.
 */

static int array_sort_compare(duk_context *ctx, int idx1, int idx2) {
	int have1, have2;
	int undef1, undef2;
	int ret;
	int idx_obj = 1, idx_fn = 0;  /* fixed offsets in valstack */
	duk_hstring *h1, *h2;

	/* Fast exit if indices are identical.  This is valid for a non-existent property,
	 * for an undefined value, and almost always for ToString() coerced comparison of
	 * arbitrary values (corner cases where this is not the case include e.g. a an
	 * object with varying ToString() coercion).
	 *
	 * The specification does not prohibit "caching" of values read from the array, so
	 * assuming equality for comparing an index with itself falls into the category of
	 * "caching".
	 *
	 * Also, compareFn may be inconsistent, so skipping a call to compareFn here may
	 * have an effect on the final result.  The specification does not require any
	 * specific behavior for inconsistent compare functions, so again, this fast path
	 * is OK.
	 */

	if (idx1 == idx2) {
		DUK_DDDPRINT("array_sort_compare: idx1=%d, idx2=%d -> indices identical, quick exit", idx1, idx2);
		return 0;
	}

	have1 = duk_get_prop_index(ctx, idx_obj, idx1);
	have2 = duk_get_prop_index(ctx, idx_obj, idx2);

	DUK_DDDPRINT("array_sort_compare: idx1=%d, idx2=%d, have1=%d, have2=%d, val1=%!T, val2=%!T",
	             idx1, idx2, have1, have2, duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

	if (have1) {
		if (have2) {
			;
		} else {
			ret = -1;
			goto pop_ret;
		}
	} else {
		if (have2) {
			ret = 1;
			goto pop_ret;
		} else {
			ret = 0;
			goto pop_ret;
		}
	}

	undef1 = duk_is_undefined(ctx, -2);
	undef2 = duk_is_undefined(ctx, -1);
	if (undef1) {
		if (undef2) {
			ret = 0;
			goto pop_ret;
		} else {
			ret = 1;
			goto pop_ret;
		}
	} else {
		if (undef2) {
			ret = -1;
			goto pop_ret;
		} else {
			;
		}
	}

	if (!duk_is_undefined(ctx, idx_fn)) {
		double d;

		/* no need to check callable; duk_call() will do that */
		duk_dup(ctx, idx_fn);    /* -> [ ... x y fn ] */
		duk_insert(ctx, -3);     /* -> [ ... fn x y ] */
		duk_call(ctx, 2);        /* -> [ ... res ] */

		/* The specification is a bit vague what to do if the return
		 * value is not a number.  Other implementations seem to
		 * tolerate non-numbers but e.g. V8 won't apparently do a
		 * ToNumber().
		 */

		/* FIXME: best behavior for real world compatibility? */

		d = duk_to_number(ctx, -1);
		if (d < 0.0) {
			ret = -1;
		} else if (d > 0.0) {
			ret = 1;
		} else {
			ret = 0;
		}

		duk_pop(ctx);
		DUK_DDDPRINT("-> result %d (from comparefn, after coercion)", ret);
		return ret;
	}

	/* string compare is the default (a bit oddly) */

	h1 = duk_to_hstring(ctx, -2);
	h2 = duk_to_hstring(ctx, -1);
	DUK_ASSERT(h1 != NULL);
	DUK_ASSERT(h2 != NULL);

	ret = duk_js_string_compare(h1, h2);  /* retval is directly usable */
	goto pop_ret;

 pop_ret:
	duk_pop_2(ctx);
	DUK_DDDPRINT("-> result %d", ret);
	return ret;
}

static void array_sort_swap(duk_context *ctx, int l, int r) {
	int have_l, have_r;
	int idx_obj = 1;  /* fixed offsets in valstack */

	if (l == r) {
		return;
	}

	/* swap elements; deal with non-existent elements correctly */
	have_l = duk_get_prop_index(ctx, idx_obj, l);
	have_r = duk_get_prop_index(ctx, idx_obj, r);

	if (have_r) {
		/* right exists, [[Put]] regardless whether or not left exists */
		duk_put_prop_index(ctx, idx_obj, l);
	} else {
		duk_del_prop_index(ctx, idx_obj, l);
		duk_pop(ctx);
	}

	if (have_l) {
		duk_put_prop_index(ctx, idx_obj, r);
	} else {
		duk_del_prop_index(ctx, idx_obj, r);
		duk_pop(ctx);
	}
}

#ifdef DUK_USE_DDDEBUG
/* Debug print which visualizes the qsort partitioning process. */
static void debuglog_qsort_state(duk_context *ctx, int lo, int hi, int pivot) {
	char buf[4096];
	char *ptr = buf;
	int i, n;
	n = duk_get_length(ctx, 1);
	if (n > 4000) {
		n = 4000;
	}
	*ptr++ = '[';
	for (i = 0; i < n; i++) {
		if (i == pivot) {
			*ptr++ = '|';
		} else if (i == lo) {
			*ptr++ = '<';
		} else if (i == hi) {
			*ptr++ = '>';
		} else if (i >= lo && i <= hi) {
			*ptr++ = '-';
		} else {
			*ptr++ = ' ';
		}
	}
	*ptr++ = ']';
	*ptr++ = '\0';

	DUK_DDDPRINT("%s   (lo=%d, hi=%d, pivot=%d)", buf, lo, hi, pivot);
}
#endif

static void array_qsort(duk_context *ctx, int lo, int hi) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int p, l, r;

	DUK_DDDPRINT("array_qsort: lo=%d, hi=%d, obj=%!T", lo, hi, duk_get_tval(ctx, 1));

	DUK_ASSERT_TOP(ctx, 3);

	/* In some cases it may be that lo > hi, or hi < 0; these
	 * degenerate cases happen e.g. for empty arrays, and in
	 * recursion leaves.
	 */

	/* trivial cases */
	if (hi - lo < 1) {
		DUK_DDDPRINT("degenerate case, return immediately");
		return;
	}
	DUK_ASSERT(hi > lo);
	DUK_ASSERT(hi - lo + 1 >= 2);

	/* randomized pivot selection */
	p = lo + (duk_util_tinyrandom_get_bits(thr, 30) % (hi - lo + 1));  /* rnd in [lo,hi] */
	DUK_ASSERT(p >= lo && p <= hi);
	DUK_DDDPRINT("lo=%d, hi=%d, chose pivot p=%d", lo, hi, p);

	/* move pivot out of the way */
	array_sort_swap(ctx, p, lo);
	p = lo;
	DUK_DDDPRINT("pivot moved out of the way: %!T", duk_get_tval(ctx, 1));

	l = lo + 1;
	r = hi;
	for (;;) {
		/* find elements to swap */
		for (;;) {
			DUK_DDDPRINT("left scan: l=%d, r=%d, p=%d", l, r, p);
			if (l >= hi) {
				break;
			}
			if (array_sort_compare(ctx, l, p) >= 0) {  /* !(l < p) */
				break;
			}
			l++;
		}
		for (;;) {
			DUK_DDDPRINT("right scan: l=%d, r=%d, p=%d", l, r, p);
			if (r <= lo) {
				break;
			}
			if (array_sort_compare(ctx, p, r) >= 0) {  /* !(p < r) */
				break;
			}
			r--;
		}
		if (l >= r) {
			goto done;
		}
		DUK_ASSERT(l < r);

		DUK_DDDPRINT("swap %d and %d", l, r);

		array_sort_swap(ctx, l, r);

		DUK_DDDPRINT("after swap: %!T", duk_get_tval(ctx, 1));
		l++;
		r--;
	}
 done:
	/* Note that 'l' and 'r' may cross, i.e. r < l */
	DUK_ASSERT(l >= lo && l <= hi);
	DUK_ASSERT(r >= lo && r <= hi);

	/* FIXME: there's no explicit recursion bound here now.  For the average
	 * qsort recursion depth O(log n) that's not really necessary: e.g. for
	 * 2**32 recursion depth would be about 32 which is OK.  However, qsort
	 * worst case recursion depth is O(n) which may be a problem.
	 */

	/* move pivot to its final place */
	DUK_DDDPRINT("before final pivot swap: %!T", duk_get_tval(ctx, 1));
	array_sort_swap(ctx, lo, r);	

#ifdef DUK_USE_DDDEBUG
	debuglog_qsort_state(ctx, lo, hi, r);
#endif

	DUK_DDDPRINT("recurse: pivot=%d, obj=%!T", r, duk_get_tval(ctx, 1));
	array_qsort(ctx, lo, r - 1);
	array_qsort(ctx, r + 1, hi);
}

int duk_bi_array_prototype_sort(duk_context *ctx) {
	unsigned int len;

	len = push_this_obj_len_u32(ctx);

	/* stack[0] = compareFn
	 * stack[1] = ToObject(this)
	 * stack[2] = ToUint32(length)
	 */

	array_qsort(ctx, 0, len - 1);

	DUK_ASSERT_TOP(ctx, 3);
	duk_pop(ctx);
	return 1;  /* return ToObject(this) */
}

/*
 *  splice()
 */

/* FIXME: this compiles to over 500 bytes now, even without special handling
 * for an array part.  Uses signed ints so does not handle full array range correctly.
 */

/* FIXME: can shift() / unshift() use the same helper?
 *   shift() is (close to?) <--> splice(0, 1)
 *   unshift is (close to?) <--> splice(0, 0, [items])?
 */

int duk_bi_array_prototype_splice(duk_context *ctx) {
	int nargs;
	int item_count;
	int len;
	int act_start;
	int del_count;
	int i;

	nargs = duk_get_top(ctx);
	if (nargs < 2) {
		duk_set_top(ctx, 2);
		nargs = 2;
	}

	len = push_this_obj_len_u32(ctx);

	act_start = duk_to_int_clamped(ctx, 0, -len, len);
	if (act_start < 0) {
		act_start = len + act_start;
	}
	DUK_ASSERT(act_start >= 0 && act_start <= len);

	del_count = duk_to_int_clamped(ctx, 1, 0, len - act_start);
	DUK_ASSERT(del_count >= 0 && del_count <= len - act_start);
	DUK_ASSERT(del_count + act_start <= len);

	duk_push_array(ctx);

	/* stack[0] = start
	 * stack[1] = deleteCount
	 * stack[2...nargs-1] = items
	 * stack[nargs] = ToObject(this)               -3
	 * stack[nargs+1] = ToUint32(length)           -2
	 * stack[nargs+2] = result array               -1
	 */

	DUK_ASSERT_TOP(ctx, nargs + 3);

	/* Step 9: copy elements-to-be-deleted into the result array */

	for (i = 0; i < del_count; i++) {
		if (duk_get_prop_index(ctx, -3, act_start + i)) {
			duk_put_prop_index(ctx, -2, i);  /* throw flag irrelevant (false in std alg) */
		} else {
			duk_pop(ctx);
		}
	}

	/* Steps 12 and 13: reorganize elements to make room for itemCount elements */

	DUK_ASSERT(nargs >= 2);
	item_count = nargs - 2;
	if (item_count < del_count) {
		/*    [ A B C D E F G H ]    rel_index = 2, del_count 3, item count 1
		 * -> [ A B F G H ]          (conceptual intermediate step)
		 * -> [ A B . F G H ]        (placeholder marked)
		 *    [ A B C F G H ]        (actual result at this point, C will be replaced)
		 */

		DUK_ASSERT_TOP(ctx, nargs + 3);

		for (i = act_start; i < len - del_count; i++) {
			if (duk_get_prop_index(ctx, -3, i + del_count)) {
				duk_put_prop_index(ctx, -4, i + item_count);  /* FIXME: Throw */
			} else {
				duk_pop(ctx);
				duk_del_prop_index(ctx, -3, i + item_count);  /* FIXME: Throw */
			}
		}

		DUK_ASSERT_TOP(ctx, nargs + 3);

		/* loop iterator init and limit changed from standard algorithm */
		for (i = len - 1; i >= len - del_count + item_count; i--) {
			duk_del_prop_index(ctx, -3, i);  /* FIXME: Throw */
		}

		DUK_ASSERT_TOP(ctx, nargs + 3);
	} else if (item_count > del_count) {
		/*    [ A B C D E F G H ]    rel_index = 2, del_count 3, item count 4
		 * -> [ A B F G H ]          (conceptual intermediate step)
		 * -> [ A B . . . . F G H ]  (placeholder marked)
		 *    [ A B C D E F F G H ]  (actual result at this point)
		 */

		DUK_ASSERT_TOP(ctx, nargs + 3);

		/* loop iterator init and limit changed from standard algorithm */
		for (i = len - del_count - 1; i >= act_start; i--) {
			if (duk_get_prop_index(ctx, -3, i + del_count)) {
				duk_put_prop_index(ctx, -4, i + item_count);  /* FIXME: Throw */
			} else {
				duk_pop(ctx);
				duk_del_prop_index(ctx, -3, i + item_count);  /* FIXME: Throw */
			}
		}

		DUK_ASSERT_TOP(ctx, nargs + 3);
	} else {
		/*    [ A B C D E F G H ]    rel_index = 2, del_count 3, item count 3
		 * -> [ A B F G H ]          (conceptual intermediate step)
		 * -> [ A B . . . F G H ]    (placeholder marked)
		 *    [ A B C D E F G H ]    (actual result at this point)
		 */
	}
	DUK_ASSERT_TOP(ctx, nargs + 3);

	/* Step 15: insert itemCount elements into the hole made above */

	for (i = 0; i < item_count; i++) {
		duk_dup(ctx, i + 2);  /* args start at index 2 */
		duk_put_prop_index(ctx, -4, act_start + i);  /* FIXME: Throw */
	}

	/* Step 16: update length; note that the final length may be above 32 bit range */

	duk_push_number(ctx, ((double) len) - ((double) del_count) + ((double) item_count));
	duk_put_prop_stridx(ctx, -4, DUK_STRIDX_LENGTH);

	/* result array is already at the top of stack */
	DUK_ASSERT_TOP(ctx, nargs + 3);
	return 1;
}

/*
 *  reverse()
 */

int duk_bi_array_prototype_reverse(duk_context *ctx) {
	unsigned int len;
	unsigned int middle;
	unsigned int lower, upper;
	int have_lower, have_upper;

	len = push_this_obj_len_u32(ctx);
	middle = len / 2;

	for (lower = 0; lower < middle; lower++) {
		DUK_ASSERT_TOP(ctx, 2);

		upper = len - lower - 1;

		have_lower = duk_get_prop_index(ctx, -2, lower);
		have_upper = duk_get_prop_index(ctx, -3, upper);

		/* [ ToObject(this) ToUint32(length) lowerValue upperValue ] */

		if (have_upper) {
			duk_put_prop_index(ctx, -4, lower);  /* FIXME: Throw */
		} else {
			duk_del_prop_index(ctx, -4, lower);
			duk_pop(ctx);
		}

		if (have_lower) {
			duk_put_prop_index(ctx, -3, upper);
		} else {
			duk_del_prop_index(ctx, -3, upper);
			duk_pop(ctx);
		}

		DUK_ASSERT_TOP(ctx, 2);
	}

	DUK_ASSERT_TOP(ctx, 2);
	duk_pop(ctx);  /* -> [ ToObject(this) ] */
	return 1;
}

/*
 *  slice()
 */

int duk_bi_array_prototype_slice(duk_context *ctx) {
	unsigned int len;
	int start, end;
	int idx;
	int i;

	len = push_this_obj_len_u32(ctx);
	duk_push_array(ctx);

	/* stack[0] = start
	 * stack[1] = end
	 * stack[2] = ToObject(this)
	 * stack[3] = ToUint32(length)
	 * stack[4] = result array
	 */

	start = duk_to_int_clamped(ctx, 0, -len, len);  /* FIXME: does not support full 32-bit range */
	if (start < 0) {
		start = len + start;
	}
	/* FIXME: could duk_is_undefined() provide defaulting undefined to 'len'
	 * (the upper limit)?
	 */
	if (duk_is_undefined(ctx, 1)) {
		end = len;
	} else {
		end = duk_to_int_clamped(ctx, 1, -len, len);
		if (end < 0) {
			end = len + end;
		}
	}
	DUK_ASSERT(start >= 0 && start <= len);
	DUK_ASSERT(end >= 0 && end <= len);

	idx = 0;
	for (i = start; i < end; i++) {
		DUK_ASSERT_TOP(ctx, 5);
		if (duk_get_prop_index(ctx, 2, i)) {
			duk_put_prop_index(ctx, 4, idx);
		} else {
			duk_pop(ctx);
		}
		idx++;
		DUK_ASSERT_TOP(ctx, 5);
	}

	DUK_ASSERT_TOP(ctx, 5);
	return 1;
}

/*
 *  shift()
 */

int duk_bi_array_prototype_shift(duk_context *ctx) {
	unsigned int len;
	unsigned int i;

	len = push_this_obj_len_u32(ctx);
	if (len == 0) {
		duk_push_int(ctx, 0);
		duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LENGTH);  /* FIXME: Throw */
		return 0;
	}

	duk_get_prop_index(ctx, 0, 0);

	/* stack[0] = object (this)
	 * stack[1] = ToUint32(length)
	 * stack[2] = elem at index 0 (retval)
	 */

	for (i = 1; i < len; i++) {
		DUK_ASSERT_TOP(ctx, 3);
		if (duk_get_prop_index(ctx, 0, i)) {
			/* fromPresent = true */
			duk_put_prop_index(ctx, 0, i - 1);  /* FIXME: Throw */
		} else {
			/* fromPresent = false */
			duk_del_prop_index(ctx, 0, i - 1);
			duk_pop(ctx);
		}
	}
	duk_del_prop_index(ctx, 0, len - 1);  /* FIXME: Throw */

	duk_push_number(ctx, (double) (len - 1));  /* FIXME: push uint */
	duk_put_prop_stridx(ctx, 0, DUK_STRIDX_LENGTH);

	DUK_ASSERT_TOP(ctx, 3);
	return 1;
}

/*
 *  unshift()
 */

int duk_bi_array_prototype_unshift(duk_context *ctx) {
	unsigned int nargs;
	unsigned int len;
	unsigned int i;
	double final_len;

	/* FIXME: duk_get_top return type */
	nargs = (unsigned int) duk_get_top(ctx);
	len = push_this_obj_len_u32(ctx);

	/* stack[0...nargs-1] = unshift args (vararg)
	 * stack[nargs] = ToObject(this)
	 * stack[nargs+1] = ToUint32(length)
	 */

	DUK_ASSERT_TOP(ctx, nargs + 2);

	/* Note: unshift() may operate on indices above unsigned 32-bit range
	 * and the final length may be >= 2**32.  Hence we use 'double' vars
	 * here, when appropriate.
	 */

	i = len;
	while (i > 0) {
		DUK_ASSERT_TOP(ctx, nargs + 2);
		i--;
		duk_push_number(ctx, ((double) i) + ((double) nargs));  /* k+argCount-1; note that may be above 32-bit range */
		if (duk_get_prop_index(ctx, -3, i)) {
			/* fromPresent = true */
			/* [ ... ToObject(this) ToUint32(length) to val ] */
			duk_put_prop(ctx, -4);  /* -> [ ... ToObject(this) ToUint32(length) ] */  /* FIXME: Throw */
		} else {
			/* fromPresent = false */
			/* [ ... ToObject(this) ToUint32(length) to val ] */
			duk_pop(ctx);
			duk_del_prop(ctx, -3);  /* -> [ ... ToObject(this) ToUint32(length) ] */  /* FIXME: Throw */
		}
		DUK_ASSERT_TOP(ctx, nargs + 2);
	}

	for (i = 0; i < nargs; i++) {
		DUK_ASSERT_TOP(ctx, nargs + 2);
		duk_dup(ctx, i);  /* -> [ ... ToObject(this) ToUint32(length) arg[i] ] */
		duk_put_prop_index(ctx, -3, i);  /* FIXME: Throw */
		DUK_ASSERT_TOP(ctx, nargs + 2);
	}

	DUK_ASSERT_TOP(ctx, nargs + 2);
	final_len = ((double) len) + ((double) nargs);
	duk_push_number(ctx, final_len);
	duk_dup_top(ctx);  /* -> [ ... ToObject(this) ToUint32(length) final_len final_len ] */
	duk_put_prop_stridx(ctx, -4, DUK_STRIDX_LENGTH);  /* FIXME: Throw */
	return 1;
}

/*
 *  indexOf(), lastIndexOf()
 */

int duk_bi_array_prototype_indexof_shared(duk_context *ctx) {
	/* FIXME: types, ensure loop below works when fixed (i must be able to go negative right now) */
	int nargs;
	int i, len;
	int fromIndex;
	int idx_step = duk_get_magic(ctx);  /* idx_step is +1 for indexOf, -1 for lastIndexOf */

	/* lastIndexOf() needs to be a vararg function because we must distinguish
	 * between an undefined fromIndex and a "not given" fromIndex; indexOf() is
	 * made vararg for symmetry although it doesn't strictly need to be.
	 */

	nargs = duk_get_top(ctx);
	duk_set_top(ctx, 2);

	len = push_this_obj_len_u32(ctx);
	if (len == 0) {
		goto not_found;
	}

	/* Index clamping is a bit tricky, we must ensure that we'll only iterate
	 * through elements that exist and that the specific requirements from E5.1
	 * Sections 15.4.4.14 and 15.4.4.15 are fulfilled; especially:
	 *
	 *   - indexOf: clamp to [-len,len], negative handling -> [0,len],
	 *     if clamped result is len, for-loop bails out immediately
	 *
	 *   - lastIndexOf: clamp to [-len-1, len-1], negative handling -> [-1, len-1],
	 *     if clamped result is -1, for-loop bails out immediately
	 *
	 * If fromIndex is not given, ToInteger(undefined) = 0, which is correct
	 * for indexOf() but incorrect for lastIndexOf().  Hence special handling,
	 * and why lastIndexOf() needs to be a vararg function.
	 */

	if (nargs >= 2) {
		/* indexOf: clamp fromIndex to [-len, len]
		 * (if fromIndex == len, for-loop terminates directly)
		 *
		 * lastIndexOf: clamp fromIndex to [-len - 1, len - 1]
		 * (if clamped to -len-1 -> fromIndex becomes -1, terminates for-loop directly)
		 */
		fromIndex = duk_to_int_clamped(ctx,
		                               1,
		                               (idx_step > 0 ? -len : -len - 1),
		                               (idx_step > 0 ? len : len - 1));
		if (fromIndex < 0) {
			/* for lastIndexOf, result may be -1 (mark immediate termination) */
			fromIndex = len + fromIndex;
		}
	} else {
		/* for indexOf, ToInteger(undefined) would be 0, i.e. correct, but
		 * handle both indexOf and lastIndexOf specially here.
		 */
		if (idx_step > 0) {
			fromIndex = 0;
		} else {
			fromIndex = len - 1;
		}
	}

	/* stack[0] = searchElement
	 * stack[1] = fromIndex
	 * stack[2] = object
	 * stack[3] = length (not needed, but not popped above)
	 */

	for (i = fromIndex;
	     i >= 0 && i < len;
	     i += idx_step) {
		DUK_ASSERT_TOP(ctx, 4);

		if (duk_get_prop_index(ctx, 2, i)) {
			DUK_ASSERT_TOP(ctx, 5);
			if (duk_strict_equals(ctx, 0, 4)) {
				duk_push_int(ctx, i);
				return 1;
			}
		}

		duk_pop(ctx);
	}

 not_found:
	duk_push_int(ctx, -1);
	return 1;
}

/*
 *  every(), some(), forEach(), map(), filter()
 */

#define ITER_EVERY    0
#define ITER_SOME     1
#define ITER_FOREACH  2
#define ITER_MAP      3
#define ITER_FILTER   4

/* FIXME: This helper is a bit awkward because the handling for the different iteration
 * callers is quite different.  This now compiles to a bit less than 500 bytes, so with
 * 5 callers the net result is about 100 bytes / caller.
 */

int duk_bi_array_prototype_iter_shared(duk_context *ctx) {
	int len;
	int i;
	int k;
	int bval;
	int iter_type = duk_get_magic(ctx);

	/* each call this helper serves has nargs==2 */
	DUK_ASSERT_TOP(ctx, 2);

	len = push_this_obj_len_u32(ctx);
	if (!duk_is_callable(ctx, 0)) {
		goto type_error;
	}
	/* if thisArg not supplied, behave as if undefined was supplied */

	if (iter_type == ITER_MAP || iter_type == ITER_FILTER) {
		duk_push_array(ctx);
	} else {
		duk_push_undefined(ctx);
	}

	/* stack[0] = callback
	 * stack[1] = thisArg
	 * stack[2] = object
	 * stack[3] = ToUint32(length)  (unused, but avoid unnecessary pop)
	 * stack[4] = result array (or undefined)
	 */

	k = 0;  /* result index for filter() */
	for (i = 0; i < len; i++) {
		DUK_ASSERT_TOP(ctx, 5);

		if (!duk_get_prop_index(ctx, 2, i)) {
			duk_pop(ctx);
			continue;
		}

		/* The original value needs to be preserved for filter(), hence
		 * this funny order.  We can't re-get the value because of side
		 * effects.
		 */

		duk_dup(ctx, 0);
		duk_dup(ctx, 1);
		duk_dup(ctx, -3);
		duk_push_int(ctx, i);
		duk_dup(ctx, 2);  /* [ ... val callback thisArg val i obj ] */
		duk_call_method(ctx, 3); /* -> [ ... val retval ] */

		switch (iter_type) {
		case ITER_EVERY:
			bval = duk_to_boolean(ctx, -1);
			if (!bval) {
				return 1;
			}
			break;
		case ITER_SOME:
			bval = duk_to_boolean(ctx, -1);
			if (bval) {
				return 1;
			}
			break;
		case ITER_FOREACH:
			/* nop */
			break;
		case ITER_MAP:
			duk_dup(ctx, -1);
			duk_put_prop_index(ctx, 4, i);  /* retval to result[i] */
			break;
		case ITER_FILTER:
			bval = duk_to_boolean(ctx, -1);
			if (bval) {
				duk_dup(ctx, -2);  /* orig value */
				duk_put_prop_index(ctx, 4, k);
				k++;
			}
			break;
		default:
			DUK_UNREACHABLE();
			break;
		}
		duk_pop_2(ctx);

		DUK_ASSERT_TOP(ctx, 5);
	}

	switch (iter_type) {
	case ITER_EVERY:
		duk_push_true(ctx);
		break;
	case ITER_SOME:
		duk_push_false(ctx);
		break;
	case ITER_FOREACH:
		duk_push_undefined(ctx);
		break;
	case ITER_MAP:
	case ITER_FILTER:
		DUK_ASSERT_TOP(ctx, 5);
		DUK_ASSERT(duk_is_array(ctx, -1));  /* topmost element is the result array already */
		break;
	default:
		DUK_UNREACHABLE();
		break;
	}

	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

/*
 *  reduce(), reduceRight()
 */

int duk_bi_array_prototype_reduce_shared(duk_context *ctx) {
	int nargs;
	int have_acc;
	int i, len;
	int idx_step = duk_get_magic(ctx);  /* idx_step is +1 for reduce, -1 for reduceRight */

	/* We're a varargs function because we need to detect whether
	 * initialValue was given or not.
	 */
	nargs = duk_get_top(ctx);
	DUK_DPRINT("nargs=%d", nargs);

	duk_set_top(ctx, 2);
	len = push_this_obj_len_u32(ctx);
	if (!duk_is_callable(ctx, 0)) {
		goto type_error;
	}

	/* stack[0] = callback fn
	 * stack[1] = initialValue
	 * stack[2] = object (coerced this)
	 * stack[3] = length (not needed, but not popped above)
	 * stack[4] = accumulator
	 */

	have_acc = 0;
	if (nargs >= 2) {
		duk_dup(ctx, 1);
		have_acc = 1;
	}
	DUK_DPRINT("have_acc=%d, acc=%!T", have_acc, duk_get_tval(ctx, 3));

	for (i = (idx_step >= 0 ? 0 : len - 1);
	     i >= 0 && i < len;
	     i += idx_step) {
		DUK_DPRINT("i=%d, len=%d, have_acc=%d, top=%d, acc=%!T",
		           i, len, have_acc, duk_get_top(ctx), duk_get_tval(ctx, 4));

		DUK_ASSERT((have_acc && duk_get_top(ctx) == 5) ||
		           (!have_acc && duk_get_top(ctx) == 4));

		if (!duk_has_prop_index(ctx, 2, i)) {
			continue;
		}

		if (!have_acc) {
			DUK_ASSERT_TOP(ctx, 4);
			duk_get_prop_index(ctx, 2, i);
			have_acc = 1;
			DUK_ASSERT_TOP(ctx, 5);
		} else {
			DUK_ASSERT_TOP(ctx, 5);
			duk_dup(ctx, 0);
			duk_dup(ctx, 4);
			duk_get_prop_index(ctx, 2, i);
			duk_push_int(ctx, i);  /* FIXME: type */
			duk_dup(ctx, 2);
			DUK_DPRINT("calling reduce function: func=%!T, prev=%!T, curr=%!T, idx=%!T, obj=%!T",
			           duk_get_tval(ctx, -5), duk_get_tval(ctx, -4), duk_get_tval(ctx, -3),
			           duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
			duk_call(ctx, 4);
			DUK_DPRINT("-> result: %!T", duk_get_tval(ctx, -1));
			duk_replace(ctx, 4);
			DUK_ASSERT_TOP(ctx, 5);
		}
	}

	if (!have_acc) {
		goto type_error;
	}

	DUK_ASSERT_TOP(ctx, 5);
	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

