#include "duk_internal.h"

/* Coerce a new .length candidate to a number and check that it's a valid
 * .length.
 */
DUK_INTERNAL duk_uint32_t duk_harray_to_array_length_checked(duk_hthread *thr, duk_tval *tv) {
	duk_uint32_t res;
	duk_double_t d;

#if !defined(DUK_USE_PREFER_SIZE)
#if defined(DUK_USE_FASTINT)
	/* When fastints are enabled, the most interesting case is assigning
	 * a fastint to .length (e.g. arr.length = 0).
	 */
	if (DUK_TVAL_IS_FASTINT(tv)) {
		/* Very common case. */
		duk_int64_t fi;
		fi = DUK_TVAL_GET_FASTINT(tv);
		if (fi < 0 || fi > DUK_I64_CONSTANT(0xffffffff)) {
			goto fail_range;
		}
		return (duk_uint32_t) fi;
	}
#else /* DUK_USE_FASTINT */
	/* When fastints are not enabled, the most interesting case is any
	 * number.
	 */
	if (DUK_TVAL_IS_DOUBLE(tv)) {
		d = DUK_TVAL_GET_NUMBER(tv);
	}
#endif /* DUK_USE_FASTINT */
	else
#endif /* !DUK_USE_PREFER_SIZE */
	{
		/* In all other cases, and when doing a size optimized build,
		 * fall back to the comprehensive handler.
		 */
		d = duk_js_tonumber(thr, tv);
	}

	/* Refuse to update an Array's 'length' to a value outside the
	 * 32-bit range.  Negative zero is accepted as zero.
	 */
	res = duk_double_to_uint32_t(d);
	if (!duk_double_equals((duk_double_t) res, d)) {
		goto fail_range;
	}

	return res;

fail_range:
	DUK_ERROR_RANGE(thr, DUK_STR_INVALID_ARRAY_LENGTH);
	DUK_WO_NORETURN(return 0;);
}

/* Delete elements required by a smaller length, taking into account
 * potentially non-configurable elements.  Returns non-zero if all
 * elements could be deleted, and zero if all or some elements could
 * not be deleted.
 */
DUK_INTERNAL duk_bool_t duk_harray_put_array_length_u32_smaller(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uint32_t old_len,
                                                                duk_uint32_t new_len,
                                                                duk_bool_t force_flag) {
	duk_uint32_t target_len;
	duk_uint_fast32_t i, n;
	duk_tval *tv;
	duk_bool_t rc;
	duk_harray *h_arr;

	DUK_DDD(DUK_DDDPRINT("new array length smaller than old (%ld -> %ld), "
	                     "probably need to remove elements",
	                     (long) old_len,
	                     (long) new_len));

	/*
	 *  New length is smaller than old length, need to delete properties above
	 *  the new length.
	 *
	 *  If array items exists, this is straightforward: array entries cannot
	 *  be non-configurable so this is guaranteed to work.
	 *
	 *  If array items does not exist, array-indexed values are scattered
	 *  in the idxprops part, and some may not be configurable (preventing length
	 *  from becoming lower than their index + 1).  To handle the algorithm
	 *  in E5 Section 15.4.5.1, step l correctly, we scan the entire property
	 *  set twice.
	 */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(new_len < old_len);
	DUK_ASSERT_VALSTACK_SPACE(thr, DUK_HOBJECT_PROP_VALSTACK_SPACE);

	DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_ARRAY(obj));
	DUK_ASSERT(DUK_HOBJECT_IS_ARRAY(obj));

	h_arr = (duk_harray *) obj;

	if (DUK_LIKELY(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj))) {
		/*
		 *  All defined array-indexed properties are in the array items
		 *  (we assume the array items is comprehensive), and all array
		 *  entries are writable, configurable, and enumerable.  Thus,
		 *  nothing can prevent array entries from being deleted.
		 */

		DUK_DDD(DUK_DDDPRINT("have array items, easy case"));

		if (old_len < DUK_HARRAY_GET_ITEMS_LENGTH(h_arr)) {
			/* XXX: assertion that entries >= old_len are already unused */
			i = old_len;
		} else {
			i = DUK_HARRAY_GET_ITEMS_LENGTH(h_arr);
		}
		DUK_ASSERT(i <= DUK_HARRAY_GET_ITEMS_LENGTH(h_arr));

		while (i > new_len) {
			i--;
			tv = DUK_HARRAY_GET_ITEMS(thr->heap, h_arr) + i;
			DUK_TVAL_DECREF_NORZ(thr, tv);
			DUK_TVAL_SET_UNUSED(tv);
		}
		DUK_HARRAY_SET_LENGTH(h_arr, new_len);
		DUK_HARRAY_ASSERT_VALID(thr->heap, h_arr);
		DUK_REFZERO_CHECK_FAST(thr);
		DUK_HARRAY_ASSERT_VALID(thr->heap, h_arr);
		return 1;
	} else {
		/*
		 *  Entries part is a bit more complex.
		 */

		/* The approach here works when the length reduction is large
		 * in proportion to the array size, as a full scan is done.
		 * For small reductions just iterating the indices one-by-one
		 * and updating the length for each would be better.
		 */

		duk_propvalue *val_base;
		duk_uarridx_t *key_base;
		duk_uint8_t *attr_base;

		/* Stage 1: find highest preventing non-configurable entry (if any).
		 * When forcing, ignore non-configurability.
		 */

		val_base = (duk_propvalue *) (void *) obj->idx_props;
		key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);
		attr_base = (duk_uint8_t *) (void *) (key_base + obj->i_size);

		target_len = new_len;
		if (force_flag) {
			goto skip_stage1;
		}
		for (i = 0, n = obj->i_next; i < n; i++) {
			duk_uarridx_t key;
			duk_uint8_t attrs;

			key = key_base[i];
			if (key < (duk_uarridx_t) new_len) {
				continue;
			}
			if (key == DUK_ARRIDX_NONE) {
				continue;
			}
			attrs = attr_base[i];
			if (attrs & DUK_PROPDESC_FLAG_CONFIGURABLE) {
				continue;
			}

			/* Relevant array index is non-configurable, blocks write.
			 * Update length target (= length we can achieve).
			 */
			if (key >= target_len) {
				target_len = key + 1;
			}
		}
	skip_stage1:

		/* Stage 2: delete configurable entries above target length. */

		for (i = 0, n = obj->i_next; i < n; i++) {
			duk_uarridx_t key;
			duk_uint8_t attrs;
			duk_propvalue *pv;

			key = key_base[i];
			if (key < target_len) {
				continue;
			}
			if (key == DUK_ARRIDX_NONE) {
				continue;
			}

			key_base[i] = DUK_ARRIDX_NONE;
			attrs = attr_base[i];
			pv = val_base + i;
			if (attrs & DUK_PROPDESC_FLAG_ACCESSOR) {
				duk_hobject *tmp;

				tmp = pv->a.get;
				DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, tmp);
				tmp = pv->a.set;
				DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, tmp);
			} else {
				duk_tval *tmp;

				tmp = &pv->v;
				DUK_TVAL_DECREF_NORZ(thr, tmp);
			}
		}

		DUK_HARRAY_ASSERT_VALID(thr->heap, h_arr);
		DUK_HARRAY_SET_LENGTH(h_arr, target_len);
		DUK_HARRAY_ASSERT_VALID(thr->heap, h_arr);

		DUK_REFZERO_CHECK_SLOW(thr);

		if (target_len == new_len) {
			return 1;
		}
	}

	return 0;
}

DUK_INTERNAL duk_bool_t duk_harray_put_array_length_u32(duk_hthread *thr,
                                                        duk_hobject *obj,
                                                        duk_uint32_t new_len,
                                                        duk_bool_t force_flag) {
	duk_harray *a;
	duk_uint32_t old_len;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_VALSTACK_SPACE(thr, DUK_HOBJECT_PROP_VALSTACK_SPACE);
	DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_ARRAY(obj));
	DUK_ASSERT(DUK_HOBJECT_IS_ARRAY(obj));

	a = (duk_harray *) obj;
	DUK_HARRAY_ASSERT_VALID(thr->heap, a);

	old_len = DUK_HARRAY_GET_LENGTH(a);

	if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a) && !force_flag)) {
		DUK_DDD(DUK_DDDPRINT("length is not writable, fail"));
		return 0;
	}

	if (new_len >= old_len) {
		DUK_DDD(DUK_DDDPRINT("new length is same or higher than old length, just update length, no deletions"));
		DUK_HARRAY_SET_LENGTH(a, new_len);
		return 1;
	}

	DUK_DDD(DUK_DDDPRINT("new length is lower than old length, probably must delete entries"));

	/*
	 *  New length lower than old length => delete elements, then
	 *  update length.
	 *
	 *  Note: even though a bunch of elements have been deleted, the 'desc' is
	 *  still valid as properties haven't been resized (and entries compacted).
	 */

	rc = duk_harray_put_array_length_u32_smaller(thr, obj, old_len, new_len, force_flag);

	/* XXX: shrink array allocation or entries compaction here? */

	return rc;
}

DUK_INTERNAL duk_bool_t duk_harray_put_array_length_top(duk_hthread *thr, duk_hobject *obj, duk_bool_t force_flag) {
	duk_uint32_t new_len;

	DUK_ASSERT(duk_is_valid_index(thr, -1));
	new_len = duk_harray_to_array_length_checked(thr, DUK_GET_TVAL_NEGIDX(thr, -1));
	return duk_harray_put_array_length_u32(thr, obj, new_len, force_flag);
}
