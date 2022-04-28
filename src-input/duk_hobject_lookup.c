#include "duk_internal.h"

DUK_INTERNAL duk_bool_t duk_hobject_lookup_strprop_index(duk_hthread *thr,
                                                         duk_hobject *obj,
                                                         duk_hstring *key,
                                                         duk_uint_fast32_t *out_idx) {
	duk_tval *tv;
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t *hash;
#endif
	duk_propvalue *val_base;
	duk_hstring **key_base;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(out_idx != NULL);

	val_base = duk_hobject_get_props(thr->heap, obj);
	key_base = (duk_hstring **) (void *) (val_base + duk_hobject_get_esize(obj));
	/* attr_base not needed */

#if defined(DUK_USE_HOBJECT_HASH_PART)
	hash = DUK_HOBJECT_GET_HASH(heap, obj);
	if (DUK_LIKELY(hash == NULL))
#endif
	{
		duk_uint_fast32_t i;
		duk_uint_fast32_t n;

		n = DUK_HOBJECT_GET_ENEXT(obj);
		for (i = 0; i < n; i++) {
			if (key_base[i] == key) {
				*out_idx = i;
				return 1;
			}
		}
	}
#if defined(DUK_USE_HOBJECT_HASH_PART)
	else {
		duk_uint32_t n;
		duk_uint32_t i, step;
		duk_uint32_t mask;

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
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_UNUSED > DUK_HOBJECT_HASHIDX_DELETED);
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_DELETED >= 0x80000000UL);

			if (DUK_LIKELY(t < 0x80000000UL)) {
				DUK_ASSERT(t < DUK_HOBJECT_GET_ESIZE(obj));
				if (DUK_LIKELY(key_base[t] == key)) {
					*out_idx = t;
					return 1;
				}
				DUK_DDD(DUK_DDDPRINT("lookup miss i=%ld, t=%ld", (long) i, (long) t));
			} else if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
				break;
			} else {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("lookup miss (deleted) i=%ld, t=%ld", (long) i, (long) t));
			}
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}
#endif /* DUK_USE_HOBJECT_HASH_PART */

	return 0;
}

DUK_INTERNAL duk_bool_t duk_hobject_lookup_strprop_val_attrs(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_hstring *key,
                                                             duk_propvalue **out_valptr,
                                                             duk_uint8_t *out_attrs) {
	duk_tval *tv;
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t *hash;
#endif
	duk_propvalue *val_base;
	duk_hstring **key_base;
	duk_uint8_t *attr_base;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(out_valptr != NULL);
	DUK_ASSERT(out_attrs != NULL);

	val_base = duk_hobject_get_props(thr->heap, obj);
	key_base = (duk_hstring **) (void *) (val_base + duk_hobject_get_esize(obj));

#if defined(DUK_USE_HOBJECT_HASH_PART)
	hash = DUK_HOBJECT_GET_HASH(heap, obj);
	if (DUK_LIKELY(hash == NULL))
#endif
	{
		duk_uint_fast32_t i;
		duk_uint_fast32_t n;

		n = DUK_HOBJECT_GET_ENEXT(obj);
		for (i = 0; i < n; i++) {
			if (key_base[i] == key) {
				attr_base = (duk_uint8_t *) (void *) (key_base + duk_hobject_get_esize(obj));
				*out_valptr = val_base + i;
				*out_attrs = *(attr_base + i);
				return 1;
			}
		}
	}
#if defined(DUK_USE_HOBJECT_HASH_PART)
	else {
		duk_uint32_t n;
		duk_uint32_t i, step;
		duk_uint32_t mask;

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
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_UNUSED > DUK_HOBJECT_HASHIDX_DELETED);
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_DELETED >= 0x80000000UL);

			if (DUK_LIKELY(t < 0x80000000UL)) {
				DUK_ASSERT(t < DUK_HOBJECT_GET_ESIZE(obj));
				if (DUK_LIKELY(key_base[t] == key)) {
					attr_base = (duk_uint8_t *) (void *) (key_base + duk_hobject_get_esize(obj));
					*out_valptr = val_base + t;
					*out_attrs = *(attr_base + t);
					return 1;
				}
				DUK_DDD(DUK_DDDPRINT("lookup miss i=%ld, t=%ld", (long) i, (long) t));
			} else if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
				break;
			} else {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("lookup miss (deleted) i=%ld, t=%ld", (long) i, (long) t));
			}
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}
#endif /* DUK_USE_HOBJECT_HASH_PART */

	return 0;
}

/* Look up concrete strprop entry index and hash index. */
DUK_INTERNAL duk_bool_t duk_hobject_lookup_strprop_indices(duk_hthread *thr,
                                                           duk_hobject *obj,
                                                           duk_hstring *key,
                                                           duk_uint_fast32_t *out_idx,
                                                           duk_int_fast32_t *out_hashidx) {
	duk_tval *tv;
#if defined(DUK_USE_HOBJECT_HASH_PART)
	duk_uint32_t *hash;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(out_idx != NULL);

#if defined(DUK_USE_HOBJECT_HASH_PART)
	hash = DUK_HOBJECT_GET_HASH(heap, obj);
	if (DUK_LIKELY(hash == NULL))
#endif
	{
		duk_uint_fast32_t i;
		duk_uint_fast32_t n;
		duk_hstring **key_base;

		key_base = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, obj);
		n = DUK_HOBJECT_GET_ENEXT(obj);
		for (i = 0; i < n; i++) {
			if (key_base[i] == key) {
				*out_idx = i;
				*out_hashidx = -1;
				return 1;
			}
		}
	}
#if defined(DUK_USE_HOBJECT_HASH_PART)
	else {
		duk_uint32_t n;
		duk_uint32_t i, step;
		duk_uint32_t mask;
		duk_hstring **key_base;

		n = *hash++;
		DUK_ASSERT(DUK_IS_POWER_OF_TWO(n));
		mask = n - 1;
		i = duk_hstring_get_hash(key) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		key_base = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, obj);

		for (;;) {
			duk_uint32_t t;

			DUK_ASSERT_DISABLE(i >= 0); /* unsigned */
			DUK_ASSERT(i < n);
			t = hash[i];
			DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_UNUSED || t == DUK_HOBJECT_HASHIDX_DELETED ||
			           (t < DUK_HOBJECT_GET_ESIZE(obj))); /* t >= 0 always true, unsigned */
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_UNUSED > DUK_HOBJECT_HASHIDX_DELETED);
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_DELETED >= 0x80000000UL);

			if (t < 0x80000000UL) {
				DUK_ASSERT(t < DUK_HOBJECT_GET_ESIZE(obj));
				if (key_base[t] == key) {
					*out_idx = t;
					*out_hashidx = i;
					return 1;
				}
				DUK_DDD(DUK_DDDPRINT("lookup miss i=%ld, t=%ld", (long) i, (long) t));
			} else if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
				break;
			} else {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("lookup miss (deleted) i=%ld, t=%ld", (long) i, (long) t));
			}
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}
#endif /* DUK_USE_HOBJECT_HASH_PART */

	return 0;
}

DUK_INTERNAL duk_bool_t duk_hobject_lookup_idxprop_index(duk_hthread *thr,
                                                         duk_hobject *obj,
                                                         duk_uarridx_t idx,
                                                         duk_uint_fast32_t *out_idx) {
	duk_uint32_t *hash;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
	DUK_ASSERT(idx != DUK_ARRIDX_NONE);
	DUK_ASSERT(out_idx != NULL);

	if (DUK_LIKELY(obj->idx_props == NULL)) {
		return 0;
	}

	hash = (duk_uint32_t *) (void *) obj->idx_hash;
	if (obj->idx_hash == NULL) {
		/* lookup directly */
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;
		duk_uint8_t *attr_base;
		duk_uint32_t i, n;

		val_base = (duk_propvalue *) obj->idx_props;
		key_base = (duk_uarridx_t *) (val_base + obj->i_size);
		attr_base = (duk_uint8_t *) (key_base + obj->i_size);

		for (i = 0, n = obj->i_next; i < n; i++) {
			duk_uarridx_t t = key_base[i];
			if (DUK_UNLIKELY(t == idx)) {
				*out_idx = i;
				return 1;
			}
		}
	} else {
		duk_uint32_t n;
		duk_uint32_t i, step;
		duk_uint32_t mask;
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;

		n = *hash++;
		DUK_ASSERT(DUK_IS_POWER_OF_TWO(n));
		mask = n - 1;
		i = duk_hobject_compute_uarridx_hash(idx) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		val_base = (duk_propvalue *) (void *) obj->idx_props;
		key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);

		for (;;) {
			duk_uint32_t t;

			DUK_ASSERT_DISABLE(i >= 0); /* unsigned */
			DUK_ASSERT(i < n);
			t = hash[i];
			DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_UNUSED || t == DUK_HOBJECT_HASHIDX_DELETED ||
			           (t < obj->i_size)); /* t >= 0 always true, unsigned */
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_UNUSED > DUK_HOBJECT_HASHIDX_DELETED);
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_DELETED >= 0x80000000UL);

			if (t < 0x80000000UL) {
				DUK_ASSERT(t < obj->i_size);
				if (key_base[t] == idx) {
					*out_idx = t;
					return 1;
				}
				DUK_DDD(DUK_DDDPRINT("lookup miss i=%ld, t=%ld", (long) i, (long) t));
			} else if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
				break;
			} else {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("lookup miss (deleted) i=%ld, t=%ld", (long) i, (long) t));
			}
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}

	return 0;
}

DUK_INTERNAL duk_bool_t duk_hobject_lookup_idxprop_val_attrs(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_uarridx_t idx,
                                                             duk_propvalue **out_valptr,
                                                             duk_uint8_t *out_attrs) {
	duk_uint32_t *hash;
	duk_propvalue *val_base;
	duk_uarridx_t *key_base;
	duk_uint8_t *attr_base;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
	DUK_ASSERT(idx != DUK_ARRIDX_NONE);
	DUK_ASSERT(out_valptr != NULL);
	DUK_ASSERT(out_attrs != NULL);

	if (DUK_LIKELY(obj->idx_props == NULL)) {
		return 0;
	}

	val_base = (duk_propvalue *) (void *) obj->idx_props;
	key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);

	hash = (duk_uint32_t *) (void *) obj->idx_hash;
	if (obj->idx_hash == NULL) {
		/* lookup directly */
		duk_uint32_t i, n;

		for (i = 0, n = obj->i_next; i < n; i++) {
			duk_uarridx_t t = key_base[i];
			if (DUK_UNLIKELY(t == idx)) {
				attr_base = (duk_uint8_t *) (key_base + obj->i_size);
				*out_valptr = val_base + i;
				*out_attrs = *(attr_base + i);
				return 1;
			}
		}
	} else {
		duk_uint32_t n;
		duk_uint32_t i, step;
		duk_uint32_t mask;

		n = *hash++;
		DUK_ASSERT(DUK_IS_POWER_OF_TWO(n));
		mask = n - 1;
		i = duk_hobject_compute_uarridx_hash(idx) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		for (;;) {
			duk_uint32_t t;

			DUK_ASSERT_DISABLE(i >= 0); /* unsigned */
			DUK_ASSERT(i < n);
			t = hash[i];
			DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_UNUSED || t == DUK_HOBJECT_HASHIDX_DELETED ||
			           (t < obj->i_size)); /* t >= 0 always true, unsigned */
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_UNUSED > DUK_HOBJECT_HASHIDX_DELETED);
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_DELETED >= 0x80000000UL);

			if (DUK_LIKELY(t < 0x80000000UL)) {
				DUK_ASSERT(t < obj->i_size);
				if (key_base[t] == idx) {
					attr_base = (duk_uint8_t *) (key_base + obj->i_size);
					*out_valptr = val_base + t;
					*out_attrs = *(attr_base + t);
					return 1;
				}
				DUK_DDD(DUK_DDDPRINT("lookup miss i=%ld, t=%ld", (long) i, (long) t));
			} else if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
				break;
			} else {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("lookup miss (deleted) i=%ld, t=%ld", (long) i, (long) t));
			}
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}

	return 0;
}

DUK_INTERNAL duk_bool_t duk_hobject_lookup_idxprop_indices(duk_hthread *thr,
                                                           duk_hobject *obj,
                                                           duk_uarridx_t idx,
                                                           duk_uint_fast32_t *out_idx,
                                                           duk_int_fast32_t *out_hashidx) {
	duk_uint32_t *hash;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_HOBJECT_ASSERT_VALID(thr->heap, obj);
	DUK_ASSERT(idx != DUK_ARRIDX_NONE);
	DUK_ASSERT(out_idx != NULL);

	if (DUK_LIKELY(obj->idx_props == NULL)) {
		return 0;
	}

	hash = (duk_uint32_t *) (void *) obj->idx_hash;
	if (obj->idx_hash == NULL) {
		/* lookup directly */
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;
		duk_uint8_t *attr_base;
		duk_uint32_t i, n;

		val_base = (duk_propvalue *) obj->idx_props;
		key_base = (duk_uarridx_t *) (val_base + obj->i_size);
		attr_base = (duk_uint8_t *) (key_base + obj->i_size);

		for (i = 0, n = obj->i_next; i < n; i++) {
			duk_uarridx_t t = key_base[i];
			if (DUK_UNLIKELY(t == idx)) {
				*out_idx = i;
				*out_hashidx = -1;
				return 1;
			}
		}
	} else {
		duk_uint32_t n;
		duk_uint32_t i, step;
		duk_uint32_t mask;
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;

		n = *hash++;
		DUK_ASSERT(DUK_IS_POWER_OF_TWO(n));
		mask = n - 1;
		i = duk_hobject_compute_uarridx_hash(idx) & mask;
		step = 1; /* Cache friendly but clustering prone. */

		val_base = (duk_propvalue *) (void *) obj->idx_props;
		key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);

		for (;;) {
			duk_uint32_t t;

			DUK_ASSERT_DISABLE(i >= 0); /* unsigned */
			DUK_ASSERT(i < n);
			t = hash[i];
			DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_UNUSED || t == DUK_HOBJECT_HASHIDX_DELETED ||
			           (t < obj->i_size)); /* t >= 0 always true, unsigned */
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_UNUSED > DUK_HOBJECT_HASHIDX_DELETED);
			DUK_ASSERT(DUK_HOBJECT_HASHIDX_DELETED >= 0x80000000UL);

			if (t < 0x80000000UL) {
				DUK_ASSERT(t < obj->i_size);
				if (key_base[t] == idx) {
					*out_idx = t;
					*out_hashidx = i;
					return 1;
				}
				DUK_DDD(DUK_DDDPRINT("lookup miss i=%ld, t=%ld", (long) i, (long) t));
			} else if (t == DUK_HOBJECT_HASHIDX_UNUSED) {
				break;
			} else {
				DUK_ASSERT(t == DUK_HOBJECT_HASHIDX_DELETED);
				DUK_DDD(DUK_DDDPRINT("lookup miss (deleted) i=%ld, t=%ld", (long) i, (long) t));
			}
			i = (i + step) & mask;

			/* Guaranteed to finish (hash is larger than #props). */
		}
	}

	return 0;
}

DUK_INTERNAL duk_hobject *duk_hobject_lookup_strprop_known_hobject(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	duk_bool_t rc;
	duk_uint_fast32_t ent_idx;
	duk_propvalue *pv;
	duk_tval *tv_res;
	duk_hobject *res;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

	rc = duk_hobject_lookup_strprop_index(thr, obj, key, &ent_idx);
	if (!rc) {
		return NULL;
	}

	pv = DUK_HOBJECT_E_GET_VALUE_BASE(thr->heap, obj) + ent_idx;
	DUK_ASSERT((DUK_HOBJECT_E_GET_FLAGS_BASE(thr->heap, obj)[ent_idx] & DUK_PROPDESC_FLAG_ACCESSOR) == 0);
	tv_res = &pv->v;
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv_res));
	res = DUK_TVAL_GET_OBJECT(tv_res);
	DUK_ASSERT(res != NULL);
	return res;
}

DUK_INTERNAL duk_tval *duk_hobject_lookup_strprop_data_tvalptr(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	duk_uint_fast32_t ent_idx;

	if (duk_hobject_lookup_strprop_index(thr, obj, key, &ent_idx)) {
		duk_propvalue *pv = DUK_HOBJECT_E_GET_VALUE_BASE(thr->heap, obj) + ent_idx;
		duk_uint8_t attrs = *(DUK_HOBJECT_E_GET_FLAGS_BASE(thr->heap, obj) + ent_idx);
		if ((attrs & DUK_PROPDESC_FLAG_ACCESSOR) == 0) {
			return &pv->v;
		}
	}
	return NULL;
}

DUK_INTERNAL duk_hstring *duk_hobject_lookup_intvalue_hstring(duk_hthread *thr, duk_hobject *obj) {
	duk_tval *tv_int;

	tv_int = duk_hobject_lookup_strprop_data_tvalptr(thr, obj, DUK_HTHREAD_STRING_INT_VALUE(thr));
	if (tv_int != NULL && DUK_TVAL_IS_STRING(tv_int)) {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv_int);
		return h;
	}
	return NULL;
}
