/*
 *  duk_hobject property access functionality.
 *
 *  This is very central functionality for size, performance, and compliance.
 *  It is also rather intricate; see hobject-algorithms.rst for discussion on
 *  the algorithms and memory-management.rst for discussion on refcounts and
 *  side effect issues.
 *
 *  Notes:
 *
 *    - It might be tempting to assert "refcount nonzero" for objects
 *      being operated on, but that's not always correct: objects with
 *      a zero refcount may be operated on by the refcount implementation
 *      (finalization) for instance.  Hence, no refcount assertions are made.
 *
 *    - Many operations (memory allocation, identifier operations, etc)
 *      may cause arbitrary side effects (e.g. through GC and finalization).
 *      These side effects may invalidate duk_tval pointers which point to
 *      areas subject to reallocation (like value stack).  Heap objects
 *      themselves have stable pointers.  Holding heap object pointers or
 *      duk_tval copies is not problematic with respect to side effects;
 *      care must be taken when holding and using argument duk_tval pointers.
 *
 *    - If a finalizer is executed, it may operate on the the same object
 *      we're currently dealing with.  For instance, the finalizer might
 *      delete a certain property which has already been looked up and
 *      confirmed to exist.  Ideally finalizers would be disabled if GC
 *      happens during property access.  At the moment property table realloc
 *      disables finalizers, and all DECREFs may cause arbitrary changes so
 *      handle DECREF carefully.
 *
 *    - The order of operations for a DECREF matters.  When DECREF is executed,
 *      the entire object graph must be consistent; note that a refzero may
 *      lead to a mark-and-sweep through a refcount finalizer.  Use NORZ macros
 *      and an explicit DUK_REFZERO_CHECK_xxx() if achieving correct order is hard.
 */

/*
 *  XXX: array indices are mostly typed as duk_uint32_t here; duk_uarridx_t
 *  might be more appropriate.
 */

#include "duk_internal.h"

/*
 *  Helpers to resize properties allocation on specific needs.
 */

DUK_INTERNAL void duk_hobject_resize_entrypart(duk_hthread *thr, duk_hobject *obj, duk_uint32_t new_e_size) {
	duk_uint32_t old_e_size;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

	old_e_size = DUK_HOBJECT_GET_ESIZE(obj);
	if (old_e_size > new_e_size) {
		new_e_size = old_e_size;
	}

	duk_hobject_realloc_strprops(thr, obj, new_e_size);
}

/*
 *  Find an existing key from entry part either by linear scan or by
 *  using the hash index (if it exists).
 *
 *  Sets entry index (and possibly the hash index) to output variables,
 *  which allows the caller to update the entry and hash entries in-place.
 *  If entry is not found, both values are set to -1.  If entry is found
 *  but there is no hash part, h_idx is set to -1.
 */

DUK_INTERNAL duk_bool_t
duk_hobject_find_entry(duk_heap *heap, duk_hobject *obj, duk_hstring *key, duk_int_t *e_idx, duk_int_t *h_idx) {
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t *hash;
#endif

	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(e_idx != NULL);
	DUK_ASSERT(h_idx != NULL);
	DUK_UNREF(heap);

#if defined(DUK_USE_HOBJECT_HASH_PART)
	hash = DUK_HOBJECT_GET_HASH(heap, obj);
	if (hash == NULL)
#endif
	{
		/* Linear scan: more likely because most objects are small.
		 * This is an important fast path.
		 */
		duk_uint_fast32_t i;
		duk_uint_fast32_t n;
		duk_hstring **h_keys_base;
		DUK_DDD(DUK_DDDPRINT("duk_hobject_find_entry() using linear scan for lookup"));

		h_keys_base = DUK_HOBJECT_E_GET_KEY_BASE(heap, obj);
		n = DUK_HOBJECT_GET_ENEXT(obj);
		for (i = 0; i < n; i++) {
			if (h_keys_base[i] == key) {
				*e_idx = (duk_int_t) i;
				*h_idx = -1;
				return 1;
			}
		}
	}
#if defined(DUK_USE_HOBJECT_HASH_PART)
	else {
		/* Hash lookup. */
		duk_uint32_t n;
		duk_uint32_t i, step;
		duk_uint32_t mask;

		DUK_DDD(DUK_DDDPRINT("duk_hobject_find_entry() using hash part for lookup"));

		n = *hash++;
		DUK_ASSERT(DUK_IS_POWER_OF_TWO(n));
		mask = n - 1;
		i = duk_hstring_get_hash(key) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		for (;;) {
			duk_uint32_t t;

			DUK_ASSERT_DISABLE(i >= 0); /* unsigned */
			DUK_ASSERT(i < n);
			t = hash[i];
			DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_UNUSED || t == DUK_HOBJECT_HASHIDX_DELETED ||
			           (t < DUK_HOBJECT_GET_ESIZE(obj))); /* t >= 0 always true, unsigned */

			if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
				break;
			} else if (t == DUK_HOBJECT_HASHIDX_DELETED) {
				DUK_DDD(DUK_DDDPRINT("lookup miss (deleted) i=%ld, t=%ld", (long) i, (long) t));
			} else {
				DUK_ASSERT(t < DUK_HOBJECT_GET_ESIZE(obj));
				if (DUK_HOBJECT_E_GET_KEY(heap, obj, t) == key) {
					DUK_DDD(
					    DUK_DDDPRINT("lookup hit i=%ld, t=%ld -> key %p", (long) i, (long) t, (void *) key));
					*e_idx = (duk_int_t) t;
					*h_idx = (duk_int_t) i;
					return 1;
				}
				DUK_DDD(DUK_DDDPRINT("lookup miss i=%ld, t=%ld", (long) i, (long) t));
			}
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}
#endif /* DUK_USE_HOBJECT_HASH_PART */

	/* Not found, leave e_idx and h_idx unset. */
	return 0;
}

/* For internal use: get non-accessor entry value */
DUK_INTERNAL duk_tval *duk_hobject_find_entry_tval_ptr(duk_heap *heap, duk_hobject *obj, duk_hstring *key) {
	duk_int_t e_idx;
	duk_int_t h_idx;

	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_UNREF(heap);

	if (duk_hobject_find_entry(heap, obj, key, &e_idx, &h_idx)) {
		DUK_ASSERT(e_idx >= 0);
		if (!DUK_HOBJECT_E_SLOT_IS_ACCESSOR(heap, obj, e_idx)) {
			return DUK_HOBJECT_E_GET_VALUE_TVAL_PTR(heap, obj, e_idx);
		}
	}
	return NULL;
}

DUK_INTERNAL duk_tval *duk_hobject_find_entry_tval_ptr_stridx(duk_heap *heap, duk_hobject *obj, duk_small_uint_t stridx) {
	return duk_hobject_find_entry_tval_ptr(heap, obj, DUK_HEAP_GET_STRING(heap, stridx));
}

/* For internal use: get non-accessor entry value and attributes */
DUK_INTERNAL duk_tval *duk_hobject_find_entry_tval_ptr_and_attrs(duk_heap *heap,
                                                                 duk_hobject *obj,
                                                                 duk_hstring *key,
                                                                 duk_uint_t *out_attrs) {
	duk_int_t e_idx;
	duk_int_t h_idx;

	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(out_attrs != NULL);
	DUK_UNREF(heap);

	if (duk_hobject_find_entry(heap, obj, key, &e_idx, &h_idx)) {
		DUK_ASSERT(e_idx >= 0);
		if (!DUK_HOBJECT_E_SLOT_IS_ACCESSOR(heap, obj, e_idx)) {
			*out_attrs = DUK_HOBJECT_E_GET_FLAGS(heap, obj, e_idx);
			return DUK_HOBJECT_E_GET_VALUE_TVAL_PTR(heap, obj, e_idx);
		}
	}
	/* If not found, out_attrs is left unset. */
	return NULL;
}

/* For internal use: get array items value */
DUK_INTERNAL duk_tval *duk_hobject_find_array_entry_tval_ptr(duk_heap *heap, duk_hobject *obj, duk_uarridx_t i) {
	duk_tval *tv;
	duk_harray *h_arr;

	DUK_ASSERT(obj != NULL);
	DUK_UNREF(heap);

	if (!DUK_HOBJECT_IS_ARRAY(obj)) {
		return NULL;
	}
	h_arr = (duk_harray *) obj;

	if (i >= DUK_HARRAY_GET_ITEMS_LENGTH(h_arr)) {
		return NULL;
	}

	tv = DUK_HARRAY_GET_ITEMS(heap, h_arr) + i;
	return tv;
}

/*
 *  Object internal value
 *
 *  Returned value is guaranteed to be reachable / incref'd, caller does not need
 *  to incref OR decref.  No proxies or accessors are invoked, no prototype walk.
 */

DUK_INTERNAL duk_tval *duk_hobject_get_internal_value_tval_ptr(duk_heap *heap, duk_hobject *obj) {
	return duk_hobject_find_entry_tval_ptr_stridx(heap, obj, DUK_STRIDX_INT_VALUE);
}

DUK_LOCAL duk_heaphdr *duk_hobject_get_internal_value_heaphdr(duk_heap *heap, duk_hobject *obj) {
	duk_tval *tv;

	DUK_ASSERT(heap != NULL);
	DUK_ASSERT(obj != NULL);

	tv = duk_hobject_get_internal_value_tval_ptr(heap, obj);
	if (tv != NULL) {
		duk_heaphdr *h = DUK_TVAL_GET_HEAPHDR(tv);
		DUK_ASSERT(h != NULL);
		return h;
	}

	return NULL;
}

DUK_INTERNAL duk_hstring *duk_hobject_get_internal_value_string(duk_heap *heap, duk_hobject *obj) {
	duk_hstring *h;

	h = (duk_hstring *) duk_hobject_get_internal_value_heaphdr(heap, obj);
	if (h != NULL) {
		DUK_ASSERT(DUK_HEAPHDR_IS_ANY_STRING((duk_heaphdr *) h));
	}
	return h;
}

DUK_LOCAL duk_hobject *duk__hobject_get_entry_object_stridx(duk_heap *heap, duk_hobject *obj, duk_small_uint_t stridx) {
	duk_tval *tv;
	duk_hobject *h;

	tv = duk_hobject_find_entry_tval_ptr_stridx(heap, obj, stridx);
	if (tv != NULL && DUK_TVAL_IS_OBJECT(tv)) {
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		return h;
	}
	return NULL;
}

DUK_INTERNAL duk_harray *duk_hobject_get_formals(duk_hthread *thr, duk_hobject *obj) {
	duk_harray *h;

	h = (duk_harray *) duk__hobject_get_entry_object_stridx(thr->heap, obj, DUK_STRIDX_INT_FORMALS);
	if (h != NULL) {
		DUK_ASSERT(DUK_HOBJECT_IS_ARRAY((duk_hobject *) h));
		DUK_ASSERT(DUK_HARRAY_GET_LENGTH(h) <= DUK_HARRAY_GET_ITEMS_LENGTH(h));
	}
	return h;
}

DUK_INTERNAL duk_hobject *duk_hobject_get_varmap(duk_hthread *thr, duk_hobject *obj) {
	duk_hobject *h;

	h = duk__hobject_get_entry_object_stridx(thr->heap, obj, DUK_STRIDX_INT_VARMAP);
	return h;
}

/*
 *  Internal helpers for managing object 'length'
 */

DUK_INTERNAL duk_size_t duk_hobject_get_length(duk_hthread *thr, duk_hobject *obj) {
	duk_double_t val;

	DUK_CTX_ASSERT_VALID(thr);
	DUK_ASSERT(obj != NULL);

	/* Fast path for Arrays. */
	if (DUK_HOBJECT_HAS_EXOTIC_ARRAY(obj)) {
		return DUK_HARRAY_GET_LENGTH((duk_harray *) obj);
	}

	/* Slow path, .length can be e.g. accessor, obj can be a Proxy, etc. */
	duk_push_hobject(thr, obj);
	duk_push_hstring_stridx(thr, DUK_STRIDX_LENGTH);
	(void) duk_prop_getvalue_push(thr, duk_normalize_index(thr, -2), DUK_GET_TVAL_NEGIDX(thr, -1));
	val = duk_to_number_m1(thr);
	duk_pop_3_unsafe(thr);

	/* This isn't part of ECMAScript semantics; return a value within
	 * duk_size_t range, or 0 otherwise.
	 */
	if (val >= 0.0 && val <= (duk_double_t) DUK_SIZE_MAX) {
		return (duk_size_t) val;
	}
	return 0;
}

/*
 *  Fast finalizer check for an object.  Walks the prototype chain, checking
 *  for finalizer presence using DUK_HOBJECT_FLAG_HAVE_FINALIZER which is kept
 *  in sync with the actual property when setting/removing the finalizer.
 */

#if defined(DUK_USE_HEAPPTR16)
DUK_INTERNAL duk_bool_t duk_hobject_has_finalizer_fast_raw(duk_heap *heap, duk_hobject *obj) {
#else
DUK_INTERNAL duk_bool_t duk_hobject_has_finalizer_fast_raw(duk_hobject *obj) {
#endif
	duk_uint_t sanity;

	DUK_ASSERT(obj != NULL);

	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	do {
		if (DUK_UNLIKELY(DUK_HOBJECT_HAS_HAVE_FINALIZER(obj))) {
			return 1;
		}
		if (DUK_UNLIKELY(sanity-- == 0)) {
			DUK_D(DUK_DPRINT("prototype loop when checking for finalizer existence; returning false"));
			return 0;
		}
#if defined(DUK_USE_HEAPPTR16)
		DUK_ASSERT(heap != NULL);
		obj = duk_hobject_get_proto_raw(heap, obj);
#else
		obj = duk_hobject_get_proto_raw(NULL, obj); /* 'heap' arg ignored */
#endif
	} while (obj != NULL);

	return 0;
}

/*
 *  Object.seal() and Object.freeze()  (E5 Sections 15.2.3.8 and 15.2.3.9)
 *
 *  Since the algorithms are similar, a helper provides both functions.
 *  Freezing is essentially sealing + making plain properties non-writable.
 *
 *  Note: virtual (non-concrete) properties which are non-configurable but
 *  writable would pose some problems, but such properties do not currently
 *  exist (all virtual properties are non-configurable and non-writable).
 *  If they did exist, the non-configurability does NOT prevent them from
 *  becoming non-writable.  However, this change should be recorded somehow
 *  so that it would turn up (e.g. when getting the property descriptor),
 *  requiring some additional flags in the object.
 */

DUK_INTERNAL void duk_hobject_object_seal_freeze_helper(duk_hthread *thr, duk_hobject *obj, duk_bool_t is_freeze) {
	duk_uint8_t *f_base;
	duk_uint_t ownpropkeys_flags;
	duk_uarridx_t i, len;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(obj != NULL);

	DUK_ASSERT_VALSTACK_SPACE(thr, DUK_HOBJECT_PROP_VALSTACK_SPACE);

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj)) {
		DUK_DD(DUK_DDPRINT("attempt to seal/freeze a readonly object, reject"));
		DUK_ERROR_TYPE(thr, DUK_STR_NOT_CONFIGURABLE);
		DUK_WO_NORETURN(return;);
	}
#endif

	/* Object is first made non-extensible, even if we fail below.
	 * This also compacts the object.
	 */
	duk_js_preventextensions(thr, obj);

	/*
	 *  Abandon array items because all properties must become non-configurable.
	 *  Note that this is now done regardless of whether this is always the case
	 *  (skips check, but performance problem if caller would do this many times
	 *  for the same object; not likely).
	 *
	 *  XXX: Could also avoid dropping the array items but flag the Array sealed
	 *  or frozen so that default attributes for items would WE or E (but no
	 *  longer WEC) without dropping array items.  This would be more memory
	 *  efficient, but are frozen/sealed Array instances common enough?
	 */

	/* XXX: Add compaction back. */

	/* Don't coerce keys, don't require enumerable. */
	ownpropkeys_flags =
	    DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX | DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING | DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL;
	duk_prop_ownpropkeys(thr, obj, ownpropkeys_flags);
	DUK_ASSERT(duk_is_array(thr, -1));

	len = (duk_uarridx_t) duk_get_length(thr, -1);
	for (i = 0; i < len; i++) {
		(void) duk_get_prop_index(thr, -1, i);
		if (is_freeze) {
			duk_small_int_t attrs = duk_prop_getowndesc_obj_tvkey(thr, obj, duk_known_tval(thr, -1));
			duk_small_uint_t defprop_flags;

			duk_prop_pop_propdesc(thr, attrs);
			if (attrs >= 0) {
				duk_small_uint_t uattrs = (duk_small_uint_t) attrs;
				if (uattrs & DUK_PROPDESC_FLAG_ACCESSOR) {
					defprop_flags = DUK_DEFPROP_CLEAR_CONFIGURABLE | DUK_DEFPROP_THROW;
				} else {
					defprop_flags =
					    DUK_DEFPROP_CLEAR_CONFIGURABLE | DUK_DEFPROP_CLEAR_WRITABLE | DUK_DEFPROP_THROW;
				}

				(void) duk_prop_defown(thr, obj, duk_known_tval(thr, -1), 0, defprop_flags);
			}
		} else {
			duk_small_uint_t defprop_flags;

			defprop_flags = DUK_DEFPROP_CLEAR_CONFIGURABLE | DUK_DEFPROP_THROW;
			(void) duk_prop_defown(thr, obj, duk_known_tval(thr, -1), 0, defprop_flags);
		}
		duk_pop_unsafe(thr);
	}

	duk_pop_unsafe(thr);
	return;
}
