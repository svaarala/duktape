/*
 *  Misc support functions
 */

#include "duk_internal.h"

DUK_INTERNAL duk_bool_t duk_hobject_prototype_chain_contains(duk_hthread *thr,
                                                             duk_hobject *h,
                                                             duk_hobject *p,
                                                             duk_bool_t ignore_loop) {
	duk_uint_t sanity;

	DUK_ASSERT(thr != NULL);

	/* False if the object is NULL or the prototype 'p' is NULL.
	 * In particular, false if both are NULL (don't compare equal).
	 */
	if (h == NULL || p == NULL) {
		return 0;
	}

	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	do {
		if (h == p) {
			return 1;
		}

		if (DUK_UNLIKELY(--sanity == 0)) {
			if (ignore_loop) {
				break;
			} else {
				DUK_ERROR_RANGE_PROTO_SANITY(thr);
				DUK_WO_NORETURN(return 0;);
			}
		}
		h = duk_hobject_get_proto_raw(thr->heap, h);
	} while (h);

	return 0;
}

DUK_INTERNAL duk_hobject *duk_hobject_get_proto_raw(duk_heap *heap, duk_hobject *h) {
	DUK_UNREF(heap);
	DUK_ASSERT(h != NULL);
	return DUK_HOBJECT_GET_PROTOTYPE(heap, h);
}

DUK_INTERNAL void duk_hobject_set_proto_raw_updref(duk_hthread *thr, duk_hobject *h, duk_hobject *p) {
#if defined(DUK_USE_REFERENCE_COUNTING)
	duk_hobject *tmp;

	DUK_ASSERT(h);
	tmp = duk_hobject_get_proto_raw(thr->heap, h);
	DUK_HOBJECT_SET_PROTOTYPE(thr->heap, h, p);
	DUK_HOBJECT_INCREF_ALLOWNULL(thr, p); /* avoid problems if p == h->prototype */
	DUK_HOBJECT_DECREF_ALLOWNULL(thr, tmp);
#else
	DUK_ASSERT(h);
	DUK_UNREF(thr);
	DUK_HOBJECT_SET_PROTOTYPE(thr->heap, h, p);
#endif
}

DUK_INTERNAL void duk_hobject_get_props_key_attr(duk_heap *heap,
                                                 duk_hobject *obj,
                                                 duk_propvalue **out_val_base,
                                                 duk_hstring ***out_key_base,
                                                 duk_uint8_t **out_attr_base) {
	duk_propvalue *val_base;
	duk_hstring **key_base;
	duk_uint8_t *attr_base;

	val_base = duk_hobject_get_props(heap, obj);
	key_base = (duk_hstring **) (void *) (val_base + duk_hobject_get_esize(obj));
	attr_base = (duk_uint8_t *) (void *) (key_base + duk_hobject_get_esize(obj));

	*out_val_base = val_base;
	*out_key_base = key_base;
	*out_attr_base = attr_base;
}

DUK_INTERNAL duk_propvalue *duk_hobject_get_props(duk_heap *heap, duk_hobject *obj) {
#if defined(DUK_USE_HEAPPTR16)
	return (duk_propvalue *) DUK_USE_HEAPPTR_DEC16(thr->heap->heap_udata, obj->hdr.h_extra16);
#else
	return (duk_propvalue *) (void *) obj->props;
#endif
}

DUK_INTERNAL duk_uint32_t duk_hobject_get_esize(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	return DUK_HOBJECT_GET_ESIZE(h);
}

DUK_INTERNAL duk_uint32_t duk_hobject_get_enext(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	return DUK_HOBJECT_GET_ENEXT(h);
}

DUK_INTERNAL duk_size_t duk_hobject_get_ebytes(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	return DUK_HOBJECT_P_COMPUTE_SIZE(DUK_HOBJECT_GET_ESIZE(h));
}

DUK_INTERNAL duk_uint32_t duk_hobject_get_isize(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	return h->i_size;
}

DUK_INTERNAL duk_uint32_t duk_hobject_get_inext(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	return h->i_next;
}

DUK_INTERNAL duk_size_t duk_hobject_get_ibytes(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	return sizeof(duk_propvalue) * (h->i_size);
}

DUK_INTERNAL duk_uint32_t duk_hobject_get_hsize(duk_heap *heap, duk_hobject *h) {
	duk_uint32_t *hash;

	DUK_ASSERT(h != NULL);
	DUK_UNREF(heap);
#if defined(DUK_USE_HOBJECT_HASH_PART)
	hash = DUK_HOBJECT_GET_HASH(heap, h);
	if (hash != NULL) {
		return hash[0];
	} else {
		return 0;
	}
#else
	DUK_UNREF(hash);
	return 0;
#endif
}

DUK_INTERNAL_DECL size_t duk_hobject_get_hbytes(duk_heap *heap, duk_hobject *h) {
	duk_uint32_t *hash;

	DUK_ASSERT(h != NULL);
	DUK_UNREF(heap);
#if defined(DUK_USE_HOBJECT_HASH_PART)
	hash = DUK_HOBJECT_GET_HASH(heap, h);
	if (hash != NULL) {
		DUK_ASSERT(hash[0] + 1U >= hash[0]);
		return (hash[0] + 1U) * sizeof(duk_uint32_t);
	} else {
		return 0;
	}
#else
	DUK_UNREF(hash);
	return 0;
#endif
}

DUK_INTERNAL duk_uint32_t duk_harray_get_active_items_length(duk_harray *a) {
	/* For now use items_length as is.  Could use 'length' for arrays
	 * at least.
	 */
	DUK_ASSERT(a != NULL);
	return DUK_HARRAY_GET_ITEMS_LENGTH(a);
}

DUK_INTERNAL duk_uint32_t duk_hobject_get_asize(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(h)) {
		DUK_ASSERT(DUK_HOBJECT_IS_HARRAY(h));
		return DUK_HARRAY_GET_ITEMS_LENGTH((duk_harray *) h);
	} else {
		return 0;
	}
}

DUK_INTERNAL duk_size_t duk_hobject_get_abytes(duk_hobject *h) {
	DUK_ASSERT(h != NULL);
	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(h)) {
		DUK_ASSERT(DUK_HOBJECT_IS_HARRAY(h));
		return sizeof(duk_tval) * (duk_size_t) DUK_HARRAY_GET_ITEMS_LENGTH((duk_harray *) h);
	} else {
		return 0;
	}
}

DUK_INTERNAL duk_uint32_t duk_hobject_compute_uarridx_hash(duk_uarridx_t idx) {
	return (duk_uint32_t) (idx * 3);
}

/*
 *  Object.isSealed() and Object.isFrozen()  (E5 Sections 15.2.3.11, 15.2.3.13)
 *
 *  Since the algorithms are similar, a helper provides both functions.
 *  Freezing is essentially sealing + making plain properties non-writable.
 */

DUK_INTERNAL duk_bool_t duk_hobject_object_is_sealed_frozen_helper(duk_hthread *thr, duk_hobject *obj, duk_bool_t is_frozen) {
	duk_uint_t ownpropkeys_flags;
	duk_uarridx_t i, len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(obj != NULL);

	DUK_ASSERT_VALSTACK_SPACE(thr, DUK_HOBJECT_PROP_VALSTACK_SPACE);

	if (duk_js_isextensible(thr, obj)) {
		return 0;
	}

	/* Don't coerce keys, don't require enumerable. */
	ownpropkeys_flags =
	    DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX | DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING | DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL;
	duk_prop_ownpropkeys(thr, obj, ownpropkeys_flags);
	DUK_ASSERT(duk_is_array(thr, -1));

	len = (duk_uarridx_t) duk_get_length(thr, -1);
	for (i = 0; i < len; i++) {
		duk_small_int_t attrs;

		(void) duk_get_prop_index(thr, -1, i);
		attrs = duk_prop_getowndesc_obj_tvkey(thr, obj, DUK_GET_TVAL_NEGIDX(thr, -1));
		duk_prop_pop_propdesc(thr, attrs);
		duk_pop_unsafe(thr);
		if (attrs >= 0) {
			if (attrs & DUK_PROPDESC_FLAG_CONFIGURABLE) {
				return 0;
			}
			if (is_frozen && (attrs & DUK_PROPDESC_FLAG_WRITABLE)) {
				return 0;
			}
		}
	}

	duk_pop_unsafe(thr);
	return 1;
}

DUK_INTERNAL void duk_hobject_start_critical(duk_hthread *thr,
                                             duk_small_uint_t *prev_ms_base_flags,
                                             duk_small_uint_t flags_to_set,
                                             duk_bool_t *prev_error_not_allowed) {
	/* Call sites triggering property table resizes, such as [[Set]],
	 * assume no side effects due to the resize.  If this were not the
	 * case, call sites would run into various difficulties like checking
	 * that a write is allowed, but a side effect actually preventing the
	 * write leading to inconsistent behavior.
	 *
	 * It would be nice to relax these protections as much as possible
	 * because we'd like the property table resize to succeed if at all
	 * possible.  For example compaction only really needs to be prevented
	 * for the object being operated on.
	 */

#if defined(DUK_USE_ASSERTIONS)
	/* Whole path must be error throw free, but we may be called from
	 * within error handling so can't assert for error_not_allowed == 0.
	 */
	*prev_error_not_allowed = thr->heap->error_not_allowed;
	thr->heap->error_not_allowed = 1;
#else
	DUK_UNREF(prev_error_not_allowed);
#endif
	*prev_ms_base_flags = thr->heap->ms_base_flags;
	thr->heap->ms_base_flags |= flags_to_set;
	thr->heap->pf_prevent_count++; /* Avoid finalizers. */
	DUK_ASSERT(thr->heap->pf_prevent_count != 0); /* Wrap. */
}

DUK_INTERNAL void duk_hobject_end_critical(duk_hthread *thr,
                                           duk_small_uint_t *prev_ms_base_flags,
                                           duk_bool_t *prev_error_not_allowed) {
	DUK_ASSERT(thr->heap->pf_prevent_count > 0);
	thr->heap->pf_prevent_count--;
	thr->heap->ms_base_flags = *prev_ms_base_flags;
#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(thr->heap->error_not_allowed == 1);
	thr->heap->error_not_allowed = *prev_error_not_allowed;
#else
	DUK_UNREF(prev_error_not_allowed);
#endif
}
