#include "duk_internal.h"

/* Count actually used entry part entries (non-NULL keys). */
DUK_LOCAL duk_uint32_t duk__count_used_e_keys(duk_hthread *thr, duk_hobject *obj) {
	duk_uint_fast32_t i;
	duk_uint_fast32_t n = 0;
	duk_hstring **e;

	DUK_ASSERT(obj != NULL);
	DUK_UNREF(thr);

	e = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, obj);
	for (i = 0; i < DUK_HOBJECT_GET_ENEXT(obj); i++) {
		if (DUK_LIKELY(*e++ != NULL)) {
			n++;
		}
	}
	return (duk_uint32_t) n;
}

DUK_LOCAL duk_uint32_t duk__count_used_i_keys(duk_hthread *thr, duk_hobject *obj) {
	duk_uint_fast32_t i;
	duk_uint_fast32_t n = 0;
	duk_uint32_t *e;

	DUK_ASSERT(obj != NULL);
	DUK_UNREF(thr);

	e = (duk_uint32_t *) (((duk_propvalue *) (void *) obj->idx_props) + obj->i_size);
	for (i = 0; i < obj->i_next; i++) {
		if (DUK_LIKELY(*e++ != DUK_ARRIDX_NONE)) {
			n++;
		}
	}
	return (duk_uint32_t) n;
}

/* Get minimum entry part growth for a certain size. */
DUK_INTERNAL duk_uint32_t duk_hobject_get_min_grow_e(duk_uint32_t e_size) {
	duk_uint32_t res;

	res = (e_size + DUK_USE_HOBJECT_ENTRY_MINGROW_ADD) / DUK_USE_HOBJECT_ENTRY_MINGROW_DIVISOR;
	DUK_ASSERT(res >= 1); /* important for callers */
	return res;
}

DUK_INTERNAL duk_uint32_t duk_hobject_get_min_grow_i(duk_uint32_t i_size) {
	duk_uint32_t res;

	res = (i_size + DUK_USE_HOBJECT_ENTRY_MINGROW_ADD) / DUK_USE_HOBJECT_ENTRY_MINGROW_DIVISOR;
	DUK_ASSERT(res >= 1); /* important for callers */
	return res;
}

/* Get minimum array items growth for a certain size. */
DUK_INTERNAL duk_uint32_t duk_hobject_get_min_grow_a(duk_uint32_t a_size) {
	duk_uint32_t res;

	res = (a_size + DUK_USE_HOBJECT_ARRAY_MINGROW_ADD) / DUK_USE_HOBJECT_ARRAY_MINGROW_DIVISOR;
	DUK_ASSERT(res >= 1); /* important for callers */
	return res;
}

/* Compute hash part size as a function of entry part size. */
DUK_LOCAL duk_uint32_t duk__compute_hash_size(duk_uint32_t e_size) {
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t res;
	duk_uint32_t tmp;

	DUK_ASSERT(e_size <= DUK_HOBJECT_MAX_PROPERTIES);
	DUK_ASSERT((DUK_HOBJECT_MAX_PROPERTIES << 2U) > DUK_HOBJECT_MAX_PROPERTIES); /* Won't wrap, even shifted by shifted by 2. */

	if (e_size < DUK_USE_HOBJECT_HASH_PROP_LIMIT) {
		return 0;
	}

	/* Hash size should be 2^N where N is chosen so that 2^N is
	 * larger than e_size.  Extra shifting is used to ensure hash
	 * is relatively sparse.
	 */
	tmp = e_size;
	res = 2; /* Result will be 2 ** (N + 1). */
	while (tmp >= 0x40) {
		tmp >>= 6;
		res <<= 6;
	}
	while (tmp != 0) {
		tmp >>= 1;
		res <<= 1;
	}
	DUK_ASSERT(res > e_size); /* >= e_size required for hashing to work */
	return res;
#else
	DUK_UNREF(e_size);
	return 0;
#endif /* USE_PROP_HASH_PART */
}

DUK_INTERNAL duk_uint32_t duk_harray_count_used_items(duk_heap *heap, duk_harray *a) {
	duk_uint32_t i, n;
	duk_tval *tv_base;
	duk_uint32_t count;

	tv_base = DUK_HARRAY_GET_ITEMS(heap, a);
	n = duk_harray_get_active_items_length(a);
	count = 0;
	for (i = 0; i < n; i++) {
		duk_tval *tv = tv_base + i;
		if (DUK_LIKELY(!DUK_TVAL_IS_UNUSED(tv))) {
			count++;
		}
	}

	return count;
}

/* Compact array items. */
DUK_LOCAL void duk__harray_compact_items(duk_hthread *thr, duk_harray *obj) {
	duk_tval *items;
	duk_uint32_t i;

	if (!DUK_HOBJECT_HAS_ARRAY_ITEMS((duk_hobject *) obj)) {
		return;
	}

	items = DUK_HARRAY_GET_ITEMS(thr->heap, obj);
	i = DUK_HARRAY_GET_ITEMS_LENGTH(obj);

	while (i > 0U) {
		duk_tval *tv = items + (i - 1);
		if (!DUK_TVAL_IS_UNUSED(tv)) {
			break; /* => 'i' is active length */
		}
		i--;
	}
	if (i != DUK_HARRAY_GET_ITEMS_LENGTH(obj)) {
		duk_small_uint_t prev_ms_base_flags;
		duk_bool_t prev_error_not_allowed;
		duk_tval *new_items;
		duk_size_t new_alloc_size;

		DUK_DD(DUK_DDPRINT("compacting items, items_length=%ld, active length=%ld",
		                   (long) DUK_HARRAY_GET_ITEMS_LENGTH(obj),
		                   (long) i));

		DUK_ASSERT(i < DUK_HARRAY_GET_ITEMS_LENGTH(obj));

		duk_hobject_start_critical(thr, &prev_ms_base_flags, DUK_MS_FLAG_NO_OBJECT_COMPACTION, &prev_error_not_allowed);

		/* No need for alloc size wrap check because we're always reallocating
		 * to smaller than previous size.
		 */
		new_alloc_size = sizeof(duk_tval) * (duk_size_t) i;
		DUK_ASSERT(new_alloc_size / sizeof(duk_tval) == (duk_size_t) i);
		new_items =
		    (duk_tval *) DUK_REALLOC(thr->heap, (duk_uint8_t *) DUK_HARRAY_GET_ITEMS(thr->heap, obj), new_alloc_size);

		duk_hobject_end_critical(thr, &prev_ms_base_flags, &prev_error_not_allowed);

		if (DUK_UNLIKELY(new_items == NULL && new_alloc_size > 0)) {
			/* This should actually never happen because we're realloc()'ing to a
			 * smaller size.
			 */
			DUK_D(DUK_DPRINT("realloc to smaller non-zero size failed, realloc bug"));
			DUK_ERROR_ALLOC_FAILED(thr);
		}
		DUK_HARRAY_SET_ITEMS(thr->heap, obj, new_items);
		DUK_HARRAY_SET_ITEMS_LENGTH(obj, i);
	} else {
		DUK_DD(DUK_DDPRINT("items already compacted, active length=%ld", (long) i));
	}
}

/*
 *  Reallocate property allocation, moving properties to the new allocation.
 *
 *  Includes key compaction, rehashing, and can also optionally abandon
 *  the array items, migrating array entries into the beginning of the
 *  new index key part.
 *
 *  There is no support for in-place reallocation or just compacting keys
 *  without resizing the property allocation.  This is intentional to keep
 *  code size minimal, but would be useful future work.
 *
 *  A GC triggered during this reallocation process must not interfere with
 *  the object being resized.  This is currently controlled by preventing
 *  finalizers (as they may affect ANY object) and object compaction in
 *  mark-and-sweep.  It would suffice to protect only this particular object
 *  from compaction, however.  DECREF refzero cascades are side effect free
 *  and OK.
 */

DUK_LOCAL DUK_ALWAYS_INLINE void duk__hobject_realloc_strprops_rehash(duk_uint32_t new_h_size,
                                                                      duk_uint32_t *new_h,
                                                                      duk_hstring **new_e_k,
                                                                      duk_uint32_t new_e_next) {
	duk_uint32_t mask;
	duk_uint32_t i;

	if (new_h_size == 0) {
		DUK_DDD(DUK_DDDPRINT("no hash part, no rehash"));
		return;
	}

	DUK_ASSERT(new_h != NULL);
	DUK_ASSERT(new_e_next <= new_h_size); /* equality not actually possible */

	/* Fill new_h with u32 0xff = UNUSED. */
	new_h++; /* Skip size. */
	DUK_ASSERT(new_h_size > 0);
	duk_memset(new_h, 0xff, sizeof(duk_uint32_t) * new_h_size);

	DUK_ASSERT(DUK_IS_POWER_OF_TWO(new_h_size));
	mask = new_h_size - 1; /* New size assumed to be 2^N. */
	for (i = 0; i < new_e_next; i++) {
		duk_hstring *key = new_e_k[i];
		duk_uint32_t j, step;

		DUK_ASSERT(key != NULL);
		j = duk_hstring_get_hash(key) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		for (;;) {
			DUK_ASSERT(new_h[j] != DUK_HOBJECT_HASHIDX_DELETED); /* should never happen */
			if (new_h[j] == DUK_HOBJECT_HASHIDX_UNUSED) {
				DUK_DDD(DUK_DDDPRINT("rebuild hit %ld -> %ld", (long) j, (long) i));
				new_h[j] = (duk_uint32_t) i;
				break;
			}
			DUK_DDD(DUK_DDDPRINT("rebuild miss %ld, step %ld", (long) j, (long) step));
			j = (j + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_uint32_t duk__hobject_realloc_strprops_copykeys(duk_hthread *thr,
                                                                                duk_hobject *obj,
                                                                                duk_hstring **new_e_k,
                                                                                duk_propvalue *new_e_pv,
                                                                                duk_uint8_t *new_e_f,
                                                                                duk_uint32_t new_e_next) {
	duk_uint32_t i;
	duk_hstring **key_base;
	duk_propvalue *val_base;
	duk_uint8_t *attr_base;

	DUK_UNREF(thr);

	key_base = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, obj);
	val_base = DUK_HOBJECT_E_GET_VALUE_BASE(thr->heap, obj);
	attr_base = DUK_HOBJECT_E_GET_FLAGS_BASE(thr->heap, obj);

	for (i = 0; i < DUK_HOBJECT_GET_ENEXT(obj); i++) {
		duk_hstring *key;

		key = key_base[i];
		if (key == NULL) {
			continue;
		}

		new_e_k[new_e_next] = key;
		new_e_pv[new_e_next] = val_base[i];
		new_e_f[new_e_next] = attr_base[i];
		new_e_next++;
	}
	/* the entries [new_e_next, new_e_size[ are left uninitialized on purpose (ok, not gc reachable) */

	return new_e_next;
}

#if defined(DUK_USE_ASSERTIONS)
DUK_LOCAL void duk__hobject_realloc_strprops_pre_assert(duk_hthread *thr, duk_hobject *obj) {
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
}
DUK_LOCAL void duk__hobject_realloc_strprops_post_assert(duk_hthread *thr, duk_hobject *obj) {
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
}
#endif /* DUK_USE_ASSERTIONS */

DUK_INTERNAL void duk_hobject_realloc_strprops(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_e_size) {
	duk_small_uint_t prev_ms_base_flags;
	duk_bool_t prev_error_not_allowed;
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t new_h_alloc_size;
	duk_uint32_t new_h_size;
	duk_uint32_t *new_h;
#endif
	duk_uint32_t new_p_alloc_size;
	duk_uint8_t *new_p;
	duk_hstring **new_e_k;
	duk_propvalue *new_e_pv;
	duk_uint8_t *new_e_f;
	duk_uint32_t new_e_next;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_PROPS(thr->heap, obj) != NULL || DUK_HOBJECT_GET_ESIZE(obj) == 0);
	DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj));
	DUK_ASSERT_VALSTACK_SPACE(thr, DUK_HOBJECT_PROP_VALSTACK_SPACE);

	DUK_STATS_INC(thr->heap, stats_object_realloc_strprops);

	/* Pre-resize assertions. */
#if defined(DUK_USE_ASSERTIONS)
	duk__hobject_realloc_strprops_pre_assert(thr, obj);
#endif

#if defined(DUK_USE_HOBJECT_HASH_PART)
	new_h_size = duk__compute_hash_size(new_e_size);
	DUK_ASSERT(new_h_size == 0 || new_h_size >= new_e_size); /* Required to guarantee success of rehashing. */
#endif

	/* Pre realloc debug log. */
	DUK_DDD(DUK_DDDPRINT("attempt to resize hobject %p: entries (%ld -> %ld bytes), hash (%ld -> %ld bytes), "
	                     "from {p=%p,e_size=%ld,e_next=%ld,h_size=%ld} to "
	                     "{e_size=%ld,h_size=%ld}",
	                     (void *) obj,
	                     (long) DUK_HOBJECT_P_COMPUTE_SIZE(duk_hobject_get_esize(obj)),
	                     (long) DUK_HOBJECT_P_COMPUTE_SIZE(new_e_size),
	                     (long) DUK_HOBJECT_H_COMPUTE_SIZE(duk_hobject_get_hsize(thr->heap, obj)),
	                     (long) DUK_HOBJECT_H_COMPUTE_SIZE(new_h_size),
	                     (void *) DUK_HOBJECT_GET_PROPS(thr->heap, obj),
	                     (long) duk_hobject_get_esize(obj),
	                     (long) duk_hobject_get_enext(obj),
	                     (long) duk_hobject_get_hsize(thr->heap, obj),
	                     (long) new_e_size,
	                     (long) new_h_size));

	/* Property count limit check.  This is the only point where we ensure
	 * we don't get more allocated space we can handle.  This works on
	 * allocation size, which grows in chunks, so the limit is a bit
	 * approximate but good enough.
	 */
	if (new_e_size > DUK_HOBJECT_MAX_PROPERTIES) {
		DUK_ERROR_ALLOC_FAILED(thr);
		DUK_WO_NORETURN(return;);
	}
#if defined(DUK_USE_OBJSIZES16)
	if (new_e_size > DUK_UINT16_MAX) {
		/* If caller gave us sizes larger than what we can store,
		 * fail memory safely with an internal error rather than
		 * truncating the sizes.
		 */
		DUK_ERROR_INTERNAL(thr);
		DUK_WO_NORETURN(return;);
	}
#endif

	/* Start critical section, protect against side effects.
	 * The new areas are not tracked in the Duktape heap at all, so
	 * it's critical we get to free/keep them in a controlled manner.
	 */

	duk_hobject_start_critical(thr, &prev_ms_base_flags, DUK_MS_FLAG_NO_OBJECT_COMPACTION, &prev_error_not_allowed);

	new_p = NULL;
#if defined(DUK_USE_HOBJECT_HASH_PART)
	new_h = NULL;
#endif

	if (new_e_size == 0) {
		new_p_alloc_size = 0;
	} else {
		/* Alloc may trigger mark-and-sweep but no compaction, and
		 * cannot throw.
		 *
		 * Alloc size wrapping prevented by maximum property count.
		 */
		new_p_alloc_size = DUK_HOBJECT_P_COMPUTE_SIZE(new_e_size);
		DUK_ASSERT(new_p_alloc_size > 0U);
		new_p = (duk_uint8_t *) DUK_ALLOC(thr->heap, new_p_alloc_size);
		if (new_p == NULL) {
			/* NULL always indicates alloc failure because
			 * new_p_alloc_size > 0.
			 */
			goto alloc_failed;
		}
	}

#if defined(DUK_USE_HOBJECT_HASH_PART)
	if (new_h_size == 0) {
		new_h_alloc_size = 0;
	} else {
		/* Alloc size wrapping prevented by maximum property count. */
		new_h_alloc_size = DUK_HOBJECT_H_COMPUTE_SIZE(new_h_size);
		DUK_ASSERT(new_h_alloc_size > 0U);
		new_h = (duk_uint32_t *) DUK_ALLOC(thr->heap, new_h_alloc_size);
		if (new_h == NULL) {
			/* NULL always indicates alloc failure because
			 * new_h_alloc_size > 0.
			 */
			goto alloc_failed;
		}
		new_h[0] = new_h_size;
	}
#endif

	/* Set up pointers to the new property area. */
	new_e_pv = (duk_propvalue *) (void *) new_p;
	new_e_k = (duk_hstring **) (void *) (new_e_pv + new_e_size);
	new_e_f = (duk_uint8_t *) (void *) (new_e_k + new_e_size);
	new_e_next = 0;
	DUK_ASSERT((new_p != NULL) || (new_e_k == NULL && new_e_pv == NULL && new_e_f == NULL));

	/* Copy and compact keys and values in the entry part. */
	new_e_next = duk__hobject_realloc_strprops_copykeys(thr, obj, new_e_k, new_e_pv, new_e_f, new_e_next);

	/* Rebuild the hash part always from scratch (guaranteed to finish
	 * as long as caller gave consistent parameters).  Rehashing is
	 * required after entry compaction or hash resize.  In addition,
	 * rehashing gets rid of elements marked deleted (DUK_HOBJECT_HASHIDX_DELETED)
	 * which is critical to ensuring the hash part never fills up.
	 */
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk__hobject_realloc_strprops_rehash(new_h_size, new_h, new_e_k, new_e_next);
#endif /* DUK_USE_HOBJECT_HASH_PART */

	/* Post realloc debug log. */
	DUK_DDD(DUK_DDDPRINT("resized hobject %p successfully", (void *) obj));

	/* All done, switch props and hash allocation to new one.  Free old
	 * allocations, including duk_harray .items if abandoned array.
	 */
	DUK_FREE_CHECKED(thr, DUK_HOBJECT_GET_PROPS(thr->heap, obj));
#if defined(DUK_USE_HOBJECT_HASH_PART)
	DUK_FREE_CHECKED(thr, DUK_HOBJECT_GET_HASH(thr->heap, obj));
#endif
	DUK_HOBJECT_SET_PROPS(thr->heap, obj, new_p);
#if defined(DUK_USE_HOBJECT_HASH_PART)
	DUK_HOBJECT_SET_HASH(thr->heap, obj, (duk_uint32_t *) (void *) new_h);
#endif
	DUK_HOBJECT_SET_ESIZE(obj, new_e_size);
	DUK_HOBJECT_SET_ENEXT(obj, new_e_next);

	DUK_DDD(DUK_DDDPRINT("resize result: %!O", (duk_heaphdr *) obj));

	/* End critical section, remove side effect protections. */
	duk_hobject_end_critical(thr, &prev_ms_base_flags, &prev_error_not_allowed);

	/* Post-resize assertions. */
#if defined(DUK_USE_ASSERTIONS)
	duk__hobject_realloc_strprops_post_assert(thr, obj);
#endif
	return;

alloc_failed:
	DUK_D(DUK_DPRINT("object entry/hash resize failed"));

#if defined(DUK_USE_HOBJECT_HASH_PART)
	DUK_FREE_CHECKED(thr, new_h); /* OK for NULL. */
#endif
	DUK_FREE_CHECKED(thr, new_p); /* OK for NULL. */

	duk_hobject_end_critical(thr, &prev_ms_base_flags, &prev_error_not_allowed);

	DUK_ERROR_ALLOC_FAILED(thr);
	DUK_WO_NORETURN(return;);
}

/* Abandon array items, moving array entries into entries part.  This requires a
 * props resize, which is a heavy operation.  We also compact the entries part
 * while we're at it, although this is not strictly required.  With a separate
 * index key part, we no longer need to string intern keys which simplifies the
 * operation.
 */
DUK_INTERNAL void duk_hobject_abandon_array_items(duk_hthread *thr, duk_hobject *obj) {
	duk_harray *a;
	duk_uint32_t new_i_size_minimum;
	duk_uint32_t new_i_size;
	duk_propvalue *val_base;
	duk_uarridx_t *key_base;
	duk_uint8_t *attr_base;
	duk_uint32_t i, n;
	duk_uint32_t out_i;
	duk_tval *tv_base;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT(DUK_HOBJECT_IS_HARRAY(obj));

	/* When a linear array items part exists, the index part must
	 * be missing, which makes abandoning easier.
	 */
	DUK_ASSERT(obj->idx_props == NULL);
	DUK_ASSERT(obj->i_size == 0);

	a = (duk_harray *) obj;
	new_i_size_minimum = duk_harray_count_used_items(thr->heap, a);
	new_i_size = new_i_size_minimum + duk_hobject_get_min_grow_i(new_i_size_minimum);

#if defined(DUK_USE_OBJSIZES16)
	if (new_i_size > DUK_UINT16_MAX) {
		new_i_size = DUK_UINT16_MAX;
	}
#endif
	if (!(new_i_size >= new_i_size_minimum)) {
		DUK_ERROR_ALLOC_FAILED(thr);
		DUK_WO_NORETURN(return;);
	}

	duk_hobject_realloc_idxprops(thr, obj, new_i_size);

	/* The index part is now large enough to host all array items so we
	 * can insert them all without side effects.  Steal refcounts.
	 */

	val_base = (duk_propvalue *) (void *) obj->idx_props;
	key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);
	attr_base = (duk_uint8_t *) (void *) (key_base + obj->i_size);
	tv_base = DUK_HARRAY_GET_ITEMS(thr->heap, a);

	out_i = 0;
	for (i = 0, n = duk_harray_get_active_items_length(a); i < n; i++) {
		duk_tval *tv_src = tv_base + i;
		duk_tval *tv_dst;
		duk_int_t ent_idx;

		if (DUK_UNLIKELY(DUK_TVAL_IS_UNUSED(tv_src))) {
			continue;
		}
		DUK_ASSERT(out_i < new_i_size_minimum);

		/* It's crucial this is side effect free.  The helper has a grow
		 * check but we've ensured there is enough space so no side
		 * effects should occur.  We don't take advantage of the fact
		 * that ent_idx grows by one on each round at present.
		 */
		ent_idx = duk_hobject_alloc_idxentry_checked(thr, obj, i);
		DUK_ASSERT(ent_idx == out_i);

		tv_dst = &val_base[ent_idx].v;
		DUK_TVAL_SET_TVAL(tv_dst, tv_src);
		key_base[ent_idx] = i;
		attr_base[ent_idx] = DUK_PROPDESC_FLAGS_WEC;
		out_i++;
	}

	/* Switch and finalize. */
	obj->i_next = out_i;
	obj->i_size = new_i_size;
	DUK_HOBJECT_CLEAR_ARRAY_ITEMS(obj);
	DUK_FREE_CHECKED(thr, DUK_HARRAY_GET_ITEMS(thr->heap, a));
	DUK_HARRAY_SET_ITEMS(thr->heap, a, NULL);
	DUK_HARRAY_SET_ITEMS_LENGTH(a, 0);

	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
}

DUK_LOCAL DUK_ALWAYS_INLINE void duk__hobject_realloc_idxprops_rehash(duk_uint32_t new_h_size,
                                                                      duk_uint32_t *new_h,
                                                                      duk_uarridx_t *new_i_k,
                                                                      duk_uint32_t new_i_next) {
	duk_uint32_t mask;
	duk_uint32_t i;

	if (new_h_size == 0) {
		DUK_DDD(DUK_DDDPRINT("no hash part, no rehash"));
		return;
	}

	DUK_ASSERT(new_h != NULL);
	DUK_ASSERT(new_i_next <= new_h_size); /* equality not actually possible */

	/* Fill new_h with u32 0xff = UNUSED. */
	new_h++; /* Skip size. */
	DUK_ASSERT(new_h_size > 0);
	duk_memset(new_h, 0xff, sizeof(duk_uint32_t) * new_h_size);

	DUK_ASSERT(DUK_IS_POWER_OF_TWO(new_h_size));
	mask = new_h_size - 1; /* New size assumed to be 2^N. */
	for (i = 0; i < new_i_next; i++) {
		duk_uarridx_t key = new_i_k[i];
		duk_uint32_t j, step;

		DUK_ASSERT(key != DUK_ARRIDX_NONE);
		j = duk_hobject_compute_uarridx_hash(key) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		for (;;) {
			DUK_ASSERT(new_h[j] != DUK_HOBJECT_HASHIDX_DELETED); /* should never happen */
			if (new_h[j] == DUK_HOBJECT_HASHIDX_UNUSED) {
				DUK_DDD(DUK_DDDPRINT("rebuild hit %ld -> %ld", (long) j, (long) i));
				new_h[j] = (duk_uint32_t) i;
				break;
			}
			DUK_DDD(DUK_DDDPRINT("rebuild miss %ld, step %ld", (long) j, (long) step));
			j = (j + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_uint32_t duk__hobject_realloc_idxprops_copykeys(duk_hthread *thr,
                                                                                duk_hobject *obj,
                                                                                duk_uint32_t *new_i_k,
                                                                                duk_propvalue *new_i_pv,
                                                                                duk_uint8_t *new_i_f,
                                                                                duk_uint32_t new_i_next) {
	duk_uint32_t i;
	duk_uarridx_t *key_base;
	duk_propvalue *val_base;
	duk_uint8_t *attr_base;

	DUK_UNREF(thr);

	val_base = (duk_propvalue *) (void *) obj->idx_props;
	key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);
	attr_base = (duk_uint8_t *) (void *) (key_base + obj->i_size);

	for (i = 0; i < obj->i_next; i++) {
		duk_uint32_t key;

		key = key_base[i];
		if (key == DUK_ARRIDX_NONE) {
			continue;
		}

		new_i_k[new_i_next] = key;
		new_i_pv[new_i_next] = val_base[i];
		new_i_f[new_i_next] = attr_base[i];
		new_i_next++;
	}
	/* the entries [new_i_next, new_i_size[ are left uninitialized on purpose (ok, not gc reachable) */

	return new_i_next;
}

#if defined(DUK_USE_ASSERTIONS)
DUK_LOCAL void duk__hobject_realloc_idxprops_pre_assert(duk_hthread *thr, duk_hobject *obj) {
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
}
DUK_LOCAL void duk__hobject_realloc_idxprops_post_assert(duk_hthread *thr, duk_hobject *obj) {
#if 0
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
#endif
}

#endif /* DUK_USE_ASSERTIONS */

DUK_INTERNAL void duk_hobject_realloc_idxprops(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_i_size) {
	duk_small_uint_t prev_ms_base_flags;
	duk_bool_t prev_error_not_allowed;
	duk_uint32_t new_h_alloc_size;
	duk_uint32_t new_h_size;
	duk_uint32_t *new_h;
	duk_uint32_t new_p_alloc_size;
	duk_uint8_t *new_p;
	duk_uarridx_t *new_i_k;
	duk_propvalue *new_i_pv;
	duk_uint8_t *new_i_f;
	duk_uint32_t new_i_next;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(!DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj));
	DUK_ASSERT_VALSTACK_SPACE(thr, DUK_HOBJECT_PROP_VALSTACK_SPACE);

	DUK_STATS_INC(thr->heap, stats_object_realloc_idxprops);

	/* Pre-resize assertions. */
#if defined(DUK_USE_ASSERTIONS)
	duk__hobject_realloc_idxprops_pre_assert(thr, obj);
#endif

	new_h_size = duk__compute_hash_size(new_i_size);
	DUK_ASSERT(new_h_size == 0 || new_h_size >= new_i_size); /* Required to guarantee success of rehashing. */

	/* Property count limit check.  This is the only point where we ensure
	 * we don't get more allocated space we can handle.  This works on
	 * allocation size, which grows in chunks, so the limit is a bit
	 * approximate but good enough.
	 */
	if (new_i_size > DUK_HOBJECT_MAX_PROPERTIES) {
		DUK_ERROR_ALLOC_FAILED(thr);
		DUK_WO_NORETURN(return;);
	}
#if defined(DUK_USE_OBJSIZES16)
	if (new_i_size > DUK_UINT16_MAX) {
		/* If caller gave us sizes larger than what we can store,
		 * fail memory safely with an internal error rather than
		 * truncating the sizes.
		 */
		DUK_ERROR_INTERNAL(thr);
		DUK_WO_NORETURN(return;);
	}
#endif

	/* Start critical section, protect against side effects.
	 * The new areas are not tracked in the Duktape heap at all, so
	 * it's critical we get to free/keep them in a controlled manner.
	 */

	duk_hobject_start_critical(thr, &prev_ms_base_flags, DUK_MS_FLAG_NO_OBJECT_COMPACTION, &prev_error_not_allowed);

	new_p = NULL;
	new_h = NULL;

	if (new_i_size == 0) {
		new_p_alloc_size = 0;
	} else {
		/* Alloc may trigger mark-and-sweep but no compaction, and
		 * cannot throw.
		 *
		 * Alloc size wrapping prevented by maximum property count.
		 */
		new_p_alloc_size = (sizeof(duk_propvalue) + sizeof(duk_uint32_t) + sizeof(duk_uint8_t)) * new_i_size;
		DUK_ASSERT(new_p_alloc_size > 0U);
		new_p = (duk_uint8_t *) DUK_ALLOC(thr->heap, new_p_alloc_size);
		if (new_p == NULL) {
			/* NULL always indicates alloc failure because
			 * new_p_alloc_size > 0.
			 */
			goto alloc_failed;
		}
	}

	if (new_h_size == 0) {
		new_h_alloc_size = 0;
	} else {
		/* Alloc size wrapping prevented by maximum property count. */
		new_h_alloc_size = sizeof(duk_uint32_t) * (new_h_size + 1);
		DUK_ASSERT(new_h_alloc_size > 0U);
		new_h = (duk_uint32_t *) DUK_ALLOC(thr->heap, new_h_alloc_size);
		if (new_h == NULL) {
			/* NULL always indicates alloc failure because
			 * new_h_alloc_size > 0.
			 */
			goto alloc_failed;
		}
		new_h[0] = new_h_size;
	}

	/* Set up pointers to the new property area. */
	new_i_pv = (duk_propvalue *) (void *) new_p;
	new_i_k = (duk_uarridx_t *) (void *) (new_i_pv + new_i_size);
	new_i_f = (duk_uint8_t *) (void *) (new_i_k + new_i_size);
	new_i_next = 0;
	DUK_ASSERT((new_p != NULL) || (new_i_k == NULL && new_i_pv == NULL && new_i_f == NULL));

	/* Copy and compact keys and values in the entry part. */
	new_i_next = duk__hobject_realloc_idxprops_copykeys(thr, obj, new_i_k, new_i_pv, new_i_f, new_i_next);

	/* Rebuild the hash part always from scratch (guaranteed to finish
	 * as long as caller gave consistent parameters).  Rehashing is
	 * required after entry compaction or hash resize.  In addition,
	 * rehashing gets rid of elements marked deleted (DUK_HOBJECT_HASHIDX_DELETED)
	 * which is critical to ensuring the hash part never fills up.
	 */
	duk__hobject_realloc_idxprops_rehash(new_h_size, new_h, new_i_k, new_i_next);

	/* Post realloc debug log. */
	DUK_DDD(DUK_DDDPRINT("resized hobject %p successfully", (void *) obj));

	/* All done, switch props and hash allocation to new one.  Free old
	 * allocations, including duk_harray .items if abandoned array.
	 */
	DUK_FREE_CHECKED(thr, obj->idx_props);
	DUK_FREE_CHECKED(thr, obj->idx_hash);
	obj->idx_props = new_p;
	obj->idx_hash = new_h;
	obj->i_size = new_i_size;
	obj->i_next = new_i_next;

	DUK_DDD(DUK_DDDPRINT("resize result: %!O", (duk_heaphdr *) obj));

	/* End critical section, remove side effect protections. */
	duk_hobject_end_critical(thr, &prev_ms_base_flags, &prev_error_not_allowed);

	/* Post-resize assertions. */
#if defined(DUK_USE_ASSERTIONS)
	duk__hobject_realloc_idxprops_post_assert(thr, obj);
#endif
	return;

alloc_failed:
	DUK_D(DUK_DPRINT("object idxprops/hash resize failed"));

	DUK_FREE_CHECKED(thr, new_h); /* OK for NULL. */
	DUK_FREE_CHECKED(thr, new_p); /* OK for NULL. */

	duk_hobject_end_critical(thr, &prev_ms_base_flags, &prev_error_not_allowed);

	DUK_ERROR_ALLOC_FAILED(thr);
	DUK_WO_NORETURN(return;);
}

/*
 *  Allocate and initialize a new entry, resizing the properties allocation
 *  if necessary.  Returns entry index (e_idx) or throws an error if alloc fails.
 *
 *  Sets the key of the entry (increasing the key's refcount), and updates
 *  the hash part if it exists.  Caller must set value and flags, and update
 *  the entry value refcount.  A decref for the previous value is not necessary.
 */

DUK_LOCAL DUK_NOINLINE void duk__grow_strprops_for_new_entry_item(duk_hthread *thr, duk_hobject *obj) {
	duk_uint32_t old_e_used; /* actually used, non-NULL entries */
	duk_uint32_t new_e_size_minimum;
	duk_uint32_t new_e_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

	/* Duktape 0.11.0 and prior tried to optimize the resize by not
	 * counting the number of actually used keys prior to the resize.
	 * This worked mostly well but also caused weird leak-like behavior
	 * as in: test-bug-object-prop-alloc-unbounded.js.  So, now we count
	 * the keys explicitly to compute the new entry part size.
	 */

	old_e_used = duk__count_used_e_keys(thr, obj);
	new_e_size_minimum = old_e_used + 1;
	new_e_size = old_e_used + duk_hobject_get_min_grow_e(old_e_used);

#if defined(DUK_USE_OBJSIZES16)
	if (new_e_size > DUK_UINT16_MAX) {
		new_e_size = DUK_UINT16_MAX;
	}
#endif

	if (!(new_e_size >= new_e_size_minimum)) {
		DUK_ERROR_ALLOC_FAILED(thr);
		DUK_WO_NORETURN(return;);
	}

	duk_hobject_realloc_strprops(thr, obj, new_e_size);
}

DUK_INTERNAL duk_int_t duk_hobject_alloc_strentry_checked(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t *h_base;
#endif
	duk_uint32_t idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_ENEXT(obj) <= DUK_HOBJECT_GET_ESIZE(obj));
	DUK_HOBJECT_ASSERT_KEY_ABSENT(thr->heap, obj, key);

	if (DUK_HOBJECT_GET_ENEXT(obj) >= DUK_HOBJECT_GET_ESIZE(obj)) {
		/* only need to guarantee 1 more slot, but allocation growth is in chunks */
		DUK_DDD(DUK_DDDPRINT("strprops full, allocate space for one more entry"));
		duk__grow_strprops_for_new_entry_item(thr, obj);
	}
	DUK_ASSERT(DUK_HOBJECT_GET_ENEXT(obj) < DUK_HOBJECT_GET_ESIZE(obj));
	idx = DUK_HOBJECT_POSTINC_ENEXT(obj);

	/* previous value is assumed to be garbage, so don't touch it */
	DUK_HOBJECT_E_SET_KEY(thr->heap, obj, idx, key);
	DUK_HSTRING_INCREF(thr, key);

#if defined(DUK_USE_HOBJECT_HASH_PART)
	h_base = DUK_HOBJECT_GET_HASH(heap, obj);
	if (DUK_UNLIKELY(h_base != NULL)) {
		duk_uint32_t n, mask;
		duk_uint32_t i, step;

		n = *h_base++;
		DUK_ASSERT(DUK_IS_POWER_OF_TWO(n));
		mask = n - 1;
		i = duk_hstring_get_hash(key) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		for (;;) {
			duk_uint32_t t = h_base[i];
			if (t >= 0x80000000UL) {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_UNUSED || t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("duk__hobject_alloc_entry_checked() inserted key into hash part, %ld -> %ld",
				                     (long) i,
				                     (long) idx));
				DUK_ASSERT_DISABLE(i >= 0); /* unsigned */
				DUK_ASSERT(i < n);
				DUK_ASSERT_DISABLE(idx >= 0);
				DUK_ASSERT(idx < DUK_HOBJECT_GET_ESIZE(obj));
				h_base[i] = idx;
				break;
			}
			DUK_DDD(DUK_DDDPRINT("duk__hobject_alloc_entry_checked() miss %ld", (long) i));
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}
#endif /* DUK_USE_HOBJECT_HASH_PART */

	/* Note: we could return the hash index here too, but it's not
	 * needed right now.
	 */

	DUK_ASSERT_DISABLE(idx >= 0);
	DUK_ASSERT(idx < DUK_HOBJECT_GET_ESIZE(obj));
	DUK_ASSERT(idx < DUK_HOBJECT_GET_ENEXT(obj));
	return (duk_int_t) idx;
}

DUK_LOCAL DUK_NOINLINE void duk__grow_idxprops_for_new_entry_item(duk_hthread *thr, duk_hobject *obj) {
	duk_uint32_t old_i_used; /* actually used, non-NULL entries */
	duk_uint32_t new_i_size_minimum;
	duk_uint32_t new_i_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

	old_i_used = duk__count_used_i_keys(thr, obj);
	new_i_size_minimum = old_i_used + 1;
	new_i_size = old_i_used + duk_hobject_get_min_grow_i(old_i_used);

#if defined(DUK_USE_OBJSIZES16)
	if (new_i_size > DUK_UINT16_MAX) {
		new_i_size = DUK_UINT16_MAX;
	}
#endif

	if (!(new_i_size >= new_i_size_minimum)) {
		DUK_ERROR_ALLOC_FAILED(thr);
		DUK_WO_NORETURN(return;);
	}

	duk_hobject_realloc_idxprops(thr, obj, new_i_size);
}

DUK_INTERNAL duk_int_t duk_hobject_alloc_idxentry_checked(duk_hthread *thr, duk_hobject *obj, duk_uint32_t key) {
	duk_uint32_t *h_base;
	duk_uint32_t idx;
	duk_uint32_t *key_base;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != DUK_ARRIDX_NONE);
	DUK_ASSERT(obj->i_next <= obj->i_size);

	if (obj->i_next >= obj->i_size) {
		DUK_DDD(DUK_DDDPRINT("idxprops full, allocate space for one more entry"));
		duk__grow_idxprops_for_new_entry_item(thr, obj);
	}
	DUK_ASSERT(obj->i_next < obj->i_size);
	DUK_ASSERT(obj->idx_props != NULL);
	idx = obj->i_next++;

	/* previous value is assumed to be garbage, so don't touch it */

	key_base = (duk_uint32_t *) (((duk_propvalue *) obj->idx_props) + obj->i_size);
	key_base[idx] = key;

	h_base = obj->idx_hash;
	if (DUK_UNLIKELY(h_base != NULL)) {
		duk_uint32_t n, mask;
		duk_uint32_t i, step;

		n = *h_base++;
		DUK_ASSERT(DUK_IS_POWER_OF_TWO(n));
		mask = n - 1;
		i = duk_hobject_compute_uarridx_hash(key) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		for (;;) {
			duk_uint32_t t = h_base[i];
			if (t >= 0x80000000UL) {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_UNUSED || t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("duk__hobject_alloc_entry_checked() inserted key into hash part, %ld -> %ld",
				                     (long) i,
				                     (long) idx));
				DUK_ASSERT_DISABLE(i >= 0); /* unsigned */
				DUK_ASSERT(i < n);
				DUK_ASSERT_DISABLE(idx >= 0);
				DUK_ASSERT(idx < obj->i_size);
				h_base[i] = idx;
				break;
			}
			DUK_DDD(DUK_DDDPRINT("duk__hobject_alloc_entry_checked() miss %ld", (long) i));
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}

	DUK_ASSERT_DISABLE(idx >= 0);
	DUK_ASSERT(idx < obj->i_size);
	DUK_ASSERT(idx < obj->i_next);
	return (duk_int_t) idx;
}

/* Count actually used array items entries and array minimum size.
 * NOTE: 'out_min_size' can be computed much faster by starting from the
 * end and breaking out early when finding first used entry, but this is
 * not needed now.
 */
DUK_LOCAL void duk__compute_a_stats(duk_hthread *thr, duk_hobject *obj, duk_uint32_t *out_used, duk_uint32_t *out_min_size) {
	duk_uint_fast32_t i;
	duk_uint_fast32_t limit;
	duk_uint_fast32_t used = 0;
	duk_uint_fast32_t highest_idx = (duk_uint_fast32_t) -1; /* see below */
	duk_tval *tv;
	duk_harray *a;

	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(out_used != NULL);
	DUK_ASSERT(out_min_size != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_HARRAY(obj));
	DUK_UNREF(thr);

	a = (duk_harray *) obj;
	tv = DUK_HARRAY_GET_ITEMS(thr->heap, a);
	limit = duk_harray_get_active_items_length(a);
	for (i = 0; i < limit; i++) {
		if (!DUK_TVAL_IS_UNUSED(tv)) {
			used++;
			highest_idx = i;
		}
		tv++;
	}

	/* Initial value for highest_idx is -1 coerced to unsigned.  This
	 * is a bit odd, but (highest_idx + 1) will then wrap to 0 below
	 * for out_min_size as intended.
	 */

	*out_used = (duk_uint32_t) used;
	*out_min_size = (duk_uint32_t) (highest_idx + 1); /* 0 if no used entries */
}

/* Check array density and indicate whether or not the array items should be abandoned. */
DUK_LOCAL duk_bool_t duk__abandon_array_density_check(duk_uint32_t a_used, duk_uint32_t a_size) {
	/*
	 *  Array abandon check; abandon if:
	 *
	 *    new_used / new_size < limit
	 *    new_used < limit * new_size        || limit is 3 bits fixed point
	 *    new_used < limit' / 8 * new_size   || *8
	 *    8*new_used < limit' * new_size     || :8
	 *    new_used < limit' * (new_size / 8)
	 *
	 *  Here, new_used = a_used, new_size = a_size.
	 *
	 *  Note: some callers use approximate values for a_used and/or a_size
	 *  (e.g. dropping a '+1' term).  This doesn't affect the usefulness
	 *  of the check, but may confuse debugging.
	 */

	return (a_size >= 256) && (a_used < DUK_USE_HOBJECT_ARRAY_ABANDON_LIMIT * (a_size >> 3));
}

/* Fast check for extending array: check whether or not a slow density check is required. */
DUK_LOCAL duk_bool_t duk__abandon_array_slow_check_required(duk_uint32_t arr_idx, duk_uint32_t old_size) {
	duk_uint32_t new_size_min;

	/*
	 *  In a fast check we assume old_size equals old_used (i.e., existing
	 *  array is fully dense).
	 *
	 *  Slow check if:
	 *
	 *    (new_size - old_size) / old_size > limit
	 *    new_size - old_size > limit * old_size
	 *    new_size > (1 + limit) * old_size        || limit' is 3 bits fixed point
	 *    new_size > (1 + (limit' / 8)) * old_size || * 8
	 *    8 * new_size > (8 + limit') * old_size   || : 8
	 *    new_size > (8 + limit') * (old_size / 8)
	 *    new_size > limit'' * (old_size / 8)      || limit'' = 9 -> max 25% increase
	 *    arr_idx + 1 > limit'' * (old_size / 8)
	 *
	 *  This check doesn't work well for small values, so old_size is rounded
	 *  up for the check (and the '+ 1' of arr_idx can be ignored in practice):
	 *
	 *    arr_idx > limit'' * ((old_size + 7) / 8)
	 */

	new_size_min = arr_idx + 1;
	return (new_size_min >= DUK_USE_HOBJECT_ARRAY_ABANDON_MINSIZE) &&
	       (arr_idx > DUK_USE_HOBJECT_ARRAY_FAST_RESIZE_LIMIT * ((old_size + 7) >> 3));
}

DUK_LOCAL duk_bool_t duk__abandon_array_check(duk_hthread *thr, duk_uint32_t arr_idx, duk_hobject *obj) {
	duk_uint32_t min_size;
	duk_uint32_t old_used;
	duk_uint32_t old_size;

	DUK_ASSERT(DUK_HOBJECT_IS_HARRAY(obj));
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	if (!duk__abandon_array_slow_check_required(arr_idx, DUK_HARRAY_GET_ITEMS_LENGTH((duk_harray *) obj))) {
		DUK_DDD(DUK_DDDPRINT("=> fast resize is OK"));
		return 0;
	}

	duk__compute_a_stats(thr, obj, &old_used, &old_size);

	DUK_DDD(DUK_DDDPRINT("abandon check, array stats: old_used=%ld, old_size=%ld, arr_idx=%ld",
	                     (long) old_used,
	                     (long) old_size,
	                     (long) arr_idx));

	min_size = arr_idx + 1;
#if defined(DUK_USE_OBJSIZES16)
	if (min_size > DUK_UINT16_MAX) {
		goto do_abandon;
	}
#endif
	DUK_UNREF(min_size);

	/* Note: intentionally use approximations to shave a few instructions:
	 *   a_used = old_used  (accurate: old_used + 1)
	 *   a_size = arr_idx   (accurate: arr_idx + 1)
	 */
	if (duk__abandon_array_density_check(old_used, arr_idx)) {
		DUK_DD(DUK_DDPRINT("write to new array entry beyond current length, "
		                   "decided to abandon array items (would become too sparse)"));

		goto do_abandon;
	}

	DUK_DDD(DUK_DDDPRINT("=> decided to keep array items"));
	return 0;

do_abandon:
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	duk_hobject_abandon_array_items(thr, obj);
	DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	return 1;
}

DUK_INTERNAL duk_tval *duk_hobject_obtain_arridx_slot_slowpath(duk_hthread *thr, duk_uint32_t arr_idx, duk_hobject *obj) {
	/*
	 *  Array needs to grow, but we don't want it becoming too sparse.
	 *  If it were to become sparse, abandon array items, moving all
	 *  array entries into the entries part (for good).
	 *
	 *  Since we don't keep track of actual density (used vs. size) of
	 *  the array items, we need to estimate somehow.  The check is made
	 *  in two parts:
	 *
	 *    - Check whether the resize need is small compared to the
	 *      current size (relatively); if so, resize without further
	 *      checking (essentially we assume that the original part is
	 *      "dense" so that the result would be dense enough).
	 *
	 *    - Otherwise, compute the resize using an actual density
	 *      measurement based on counting the used array entries.
	 */

	DUK_ASSERT(DUK_HOBJECT_IS_HARRAY(obj));
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));

	DUK_DDD(DUK_DDDPRINT("write to new array requires array resize, decide whether to do a "
	                     "fast resize without abandon check (arr_idx=%ld, old_size=%ld)",
	                     (long) arr_idx,
	                     (long) duk_hobject_get_asize(obj)));

	if (DUK_UNLIKELY(duk__abandon_array_check(thr, arr_idx, obj) != 0)) {
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		return NULL;
	}

	DUK_DD(DUK_DDPRINT("write to new array entry beyond current length, "
	                   "decided to extend current allocation"));

	/* In principle it's possible to run out of memory extending the
	 * array but with the allocation going through if we were to abandon
	 * the array items and try again.  In practice this should be rare
	 * because abandoned arrays have a higher per-entry footprint.
	 */

	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	duk_harray_grow_items_for_size(thr, obj, arr_idx + 1);

	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT(arr_idx < DUK_HARRAY_GET_ITEMS_LENGTH((duk_harray *) obj));
	return DUK_HARRAY_GET_ITEMS(thr->heap, (duk_harray *) obj) + arr_idx;
}

DUK_INTERNAL DUK_INLINE duk_tval *duk_hobject_obtain_arridx_slot(duk_hthread *thr, duk_uint32_t arr_idx, duk_hobject *obj) {
	DUK_ASSERT(DUK_HOBJECT_IS_HARRAY(obj));
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	if (arr_idx < DUK_HARRAY_GET_ITEMS_LENGTH((duk_harray *) obj)) {
		return DUK_HARRAY_GET_ITEMS(thr->heap, (duk_harray *) obj) + arr_idx;
	} else {
		return duk_hobject_obtain_arridx_slot_slowpath(thr, arr_idx, obj);
	}
}

/*
 *  Compact an object.  Minimizes allocation size for objects which are
 *  not likely to be extended or when we're low on memory.  This is useful
 *  for internal and non-extensible objects, but can also be called for
 *  extensible objects.  May abandon the array items if it is computed to
 *  be too sparse.
 *
 *  This call is relatively expensive, as it needs to scan both the
 *  entries and the array items.
 *
 *  The call may fail due to allocation error.
 */

DUK_INTERNAL void duk_hobject_compact_object(duk_hthread *thr, duk_hobject *obj) {
	duk_uint32_t e_size = 0; /* currently used -> new size */
	duk_bool_t abandon_array = 0;
	duk_uint32_t a_size = 0; /* currently required */
	duk_uint32_t a_used = 0; /* actually used */

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj)) {
		DUK_DD(DUK_DDPRINT("ignore attempt to compact a rom object"));
		return;
	}
#endif

	e_size = duk__count_used_e_keys(thr, obj);

	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
		duk__compute_a_stats(thr, obj, &a_used, &a_size);

		if (duk__abandon_array_density_check(a_used, a_size)) {
			DUK_DD(DUK_DDPRINT("decided to abandon array during compaction, a_used=%ld, a_size=%ld",
			                   (long) a_used,
			                   (long) a_size));
			abandon_array = 1;
			e_size += a_used;
		} else {
			DUK_DD(DUK_DDPRINT("decided to keep array during compaction"));
			DUK_ASSERT(abandon_array == 0);
		}
	}

	DUK_DD(DUK_DDPRINT("compacting hobject, used e keys %ld, used a keys %ld, min a size %ld, "
	                   "resized array density would be: %ld/%ld = %lf",
	                   (long) e_size,
	                   (long) a_used,
	                   (long) a_size,
	                   (long) a_used,
	                   (long) a_size,
	                   (double) a_used / (double) a_size));

	DUK_DD(DUK_DDPRINT("compacting hobject -> new e_size %ld, new a_size=%ld, abandon_array=%ld",
	                   (long) e_size,
	                   (long) a_size,
	                   (long) abandon_array));

	duk_hobject_realloc_strprops(thr, obj, e_size);

	/* XXX: Compact idxprops and array items. */
}

/* Grow array items for a new highest array index. */
DUK_INTERNAL void duk_harray_grow_items_for_size(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_items_min_length) {
	duk_uint32_t new_items_length;
	duk_harray *h_arr = (duk_harray *) obj;
	duk_small_uint_t prev_ms_base_flags;
	duk_bool_t prev_error_not_allowed;
	duk_tval *new_items = NULL;
	duk_size_t old_alloc_size, new_alloc_size;
	duk_tval *tv, *tv_end;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_ARRAY(obj) || DUK_HOBJECT_IS_ARGUMENTS(obj));
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	DUK_ASSERT(new_items_min_length >= DUK_HARRAY_GET_ITEMS_LENGTH((duk_harray *) obj));
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);

	new_items_length = new_items_min_length + duk_hobject_get_min_grow_a(new_items_min_length);
	DUK_ASSERT(new_items_length > new_items_min_length); /* duk_hobject_get_min_grow_a() is always >= 1 */

#if defined(DUK_USE_OBJSIZES16)
	if (new_items_length > DUK_UINT16_MAX) {
		new_items_length = DUK_UINT16_MAX;
	}
#endif
	if (!(new_items_length >= new_items_min_length)) {
		goto alloc_fail;
	}
	DUK_ASSERT_DISABLE(new_items_length <= 0xffffffffUL);

	/* Start critical section, protect against side effects.
	 * XXX: Could allow compaction, by using indirect alloc.
	 */
	duk_hobject_start_critical(thr, &prev_ms_base_flags, DUK_MS_FLAG_NO_OBJECT_COMPACTION, &prev_error_not_allowed);

	/* Realloc, safe from side effects. */
	old_alloc_size = sizeof(duk_tval) * DUK_HARRAY_GET_ITEMS_LENGTH(h_arr);
	new_alloc_size = sizeof(duk_tval) * new_items_length;
	if (sizeof(duk_size_t) <= sizeof(duk_uint32_t)) {
		if (new_alloc_size / sizeof(duk_tval) != new_items_length) {
			goto alloc_fail;
		}
	}

	DUK_DD(DUK_DDPRINT("grow array: %ld -> %ld entries, %ld -> %ld bytes",
	                   (long) DUK_HARRAY_GET_ITEMS_LENGTH(h_arr),
	                   (long) new_items_length,
	                   old_alloc_size,
	                   new_alloc_size));
	DUK_ASSERT(new_alloc_size >= old_alloc_size);
	new_items = (duk_tval *) DUK_REALLOC(thr->heap, (duk_uint8_t *) DUK_HARRAY_GET_ITEMS(thr->heap, h_arr), new_alloc_size);

	/* End critical section, remove side effect protections. */
	duk_hobject_end_critical(thr, &prev_ms_base_flags, &prev_error_not_allowed);

	if (DUK_UNLIKELY(new_items == NULL)) {
		DUK_ERROR_ALLOC_FAILED(thr);
		DUK_WO_NORETURN(return;);
	}

	/* Init new values to UNUSED, per array items contract. */
	tv = (duk_tval *) (void *) ((duk_uint8_t *) new_items + old_alloc_size);
	tv_end = (duk_tval *) (void *) ((duk_uint8_t *) new_items + new_alloc_size);
	while (tv != tv_end) {
		DUK_TVAL_SET_UNUSED(tv);
		tv++;
	}

	DUK_HARRAY_SET_ITEMS(thr->heap, h_arr, new_items);
	DUK_HARRAY_SET_ITEMS_LENGTH(h_arr, new_items_length);
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
	return;

alloc_fail:
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
	DUK_ERROR_ALLOC_FAILED(thr);
	DUK_WO_NORETURN(return;);
}

DUK_INTERNAL duk_tval *duk_harray_append_reserve_items(duk_hthread *thr,
                                                       duk_harray *a,
                                                       duk_uarridx_t start_idx,
                                                       duk_uint32_t count) {
	duk_uint32_t min_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS((duk_hobject *) a));

	min_size = start_idx + count;
	if (DUK_UNLIKELY(min_size < start_idx)) {
		DUK_ERROR_RANGE_INVALID_LENGTH(thr);
		DUK_WO_NORETURN(return NULL;); /* Wrap. */
	}
	if (DUK_HARRAY_GET_ITEMS_LENGTH(a) < min_size) {
		duk_harray_grow_items_for_size(thr, (duk_hobject *) a, min_size);
	}
	DUK_ASSERT(DUK_HARRAY_GET_ITEMS_LENGTH(a) >= min_size);

	DUK_ASSERT(DUK_HARRAY_GET_ITEMS(thr->heap, a) != NULL || (start_idx == 0 && count == 0));
	return DUK_HARRAY_GET_ITEMS(thr->heap, a) + start_idx;
}
