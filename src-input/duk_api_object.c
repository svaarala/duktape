/*
 *  Object handling: property access and other support functions.
 */

#include "duk_internal.h"

/*
 *  Property handling
 *
 *  The API exposes only the most common property handling functions.
 *  The caller can invoke ECMAScript built-ins for full control (e.g.
 *  defineProperty, getOwnPropertyDescriptor).
 */

DUK_EXTERNAL duk_bool_t duk_get_prop(duk_hthread *thr, duk_idx_t obj_idx) {
	duk_idx_t idx_key;
	duk_tval *tv_key;
	duk_bool_t rc;

	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	idx_key = duk_require_top_index(thr);
	tv_key = DUK_GET_TVAL_NEGIDX(thr, -1);
	rc = duk_prop_getvalue_outidx(thr, obj_idx, tv_key, idx_key);
	DUK_ASSERT(rc == 0 || rc == 1);
	/* a value is left on stack regardless of rc */

	/* XXX: The return value is a bit problematic.  For normal objects a
	 * found/not found distinction is clear, but for Proxies there's no
	 * such distinction because a 'get' trap cannot distinguish between
	 * an 'undefined' and a missing value.  So either this return value
	 * should be removed, or be true when the property is missing or
	 * 'undefined'.
	 */
	DUK_ASSERT(duk_is_undefined(thr, -1) || rc == 1);
	return rc; /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_get_prop_string(duk_hthread *thr, duk_idx_t obj_idx, const char *key) {
	duk_idx_t idx_key;
	duk_hstring *h_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_string(thr, key);
	h_key = duk_known_hstring(thr, -1);
	idx_key = duk_get_top_index_unsafe(thr);
	return duk_prop_getvalue_strkey_outidx(thr, obj_idx, h_key, idx_key);
}

DUK_EXTERNAL duk_bool_t duk_get_prop_lstring(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	duk_idx_t idx_key;
	duk_hstring *h_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_lstring(thr, key, key_len);
	h_key = duk_known_hstring(thr, -1);
	idx_key = duk_get_top_index_unsafe(thr);
	return duk_prop_getvalue_strkey_outidx(thr, obj_idx, h_key, idx_key);
}

#if !defined(DUK_USE_PREFER_SIZE)
DUK_EXTERNAL duk_bool_t duk_get_prop_literal_raw(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	duk_idx_t idx_key;
	duk_hstring *h_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(key[key_len] == (char) 0);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_literal_raw(thr, key, key_len);
	h_key = duk_known_hstring(thr, -1);
	idx_key = duk_get_top_index_unsafe(thr);
	return duk_prop_getvalue_strkey_outidx(thr, obj_idx, h_key, idx_key);
}
#endif

DUK_EXTERNAL duk_bool_t duk_get_prop_index(duk_hthread *thr, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	duk_idx_t idx_out;

	DUK_ASSERT_API_ENTRY(thr);

	/* The case of index > 0xfffffffe is handled by the property code. */
	obj_idx = duk_require_normalize_index(thr, obj_idx);
	duk_push_undefined(thr);
	idx_out = duk_get_top_index_unsafe(thr);
	return duk_prop_getvalue_idxkey_outidx(thr, obj_idx, arr_idx, idx_out);
}

DUK_EXTERNAL duk_bool_t duk_get_prop_heapptr(duk_hthread *thr, duk_idx_t obj_idx, void *ptr) {
	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_heapptr(thr, ptr); /* NULL -> 'undefined' */
	return duk_get_prop(thr, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	duk_hstring *h_str;
	duk_idx_t idx_out;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	h_str = DUK_HTHREAD_GET_STRING(thr, stridx);
	duk_push_undefined(thr);
	idx_out = duk_get_top_index_unsafe(thr);
	return duk_prop_getvalue_strkey_outidx(thr, obj_idx, h_str, idx_out);
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx_short_raw(duk_hthread *thr, duk_uint_t packed_args) {
	return duk_get_prop_stridx(thr, (duk_idx_t) (duk_int16_t) (packed_args >> 16), (duk_small_uint_t) (packed_args & 0xffffUL));
}

DUK_INTERNAL duk_bool_t duk_get_prop_stridx_boolean(duk_hthread *thr,
                                                    duk_idx_t obj_idx,
                                                    duk_small_uint_t stridx,
                                                    duk_bool_t *out_has_prop) {
	duk_bool_t rc;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);

	rc = duk_get_prop_stridx(thr, obj_idx, stridx);
	if (out_has_prop) {
		*out_has_prop = rc;
	}
	return duk_to_boolean_top_pop(thr);
}

/* This get variant is for internal use, it differs from standard
 * duk_get_prop() in that:
 *   - Object argument must be an object (primitive values not supported).
 *   - Key argument must be a string (no coercion).
 *   - Only own properties are checked (no inheritance).  Only "entry part"
 *     properties are checked (not array index properties).
 *   - Property must be a plain data property, not a getter.
 *   - Proxy traps are not triggered.
 */
DUK_INTERNAL duk_bool_t duk_xget_owndataprop(duk_hthread *thr, duk_idx_t obj_idx) {
	duk_hobject *h_obj;
	duk_hstring *h_key;
	duk_tval *tv_val;

	DUK_ASSERT_API_ENTRY(thr);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property get right now.
	 */

	h_obj = duk_get_hobject(thr, obj_idx);
	if (h_obj == NULL) {
		return 0;
	}
	h_key = duk_require_hstring(thr, -1);

	tv_val = duk_hobject_find_entry_tval_ptr(thr->heap, h_obj, h_key);
	if (tv_val == NULL) {
		return 0;
	}

	duk_push_tval(thr, tv_val);
	duk_remove_m2(thr); /* remove key */

	return 1;
}

DUK_INTERNAL duk_bool_t duk_xget_owndataprop_stridx(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_hstring(thr, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_xget_owndataprop(thr, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_xget_owndataprop_stridx_short_raw(duk_hthread *thr, duk_uint_t packed_args) {
	return duk_xget_owndataprop_stridx(thr,
	                                   (duk_idx_t) (duk_int16_t) (packed_args >> 16),
	                                   (duk_small_uint_t) (packed_args & 0xffffUL));
}

DUK_EXTERNAL duk_bool_t duk_put_prop(duk_hthread *thr, duk_idx_t obj_idx) {
	duk_bool_t throw_flag;
	duk_bool_t rc;
	duk_tval *tv_key;
	duk_idx_t val_idx;

	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	tv_key = duk_require_tval(thr, -2); /* Also ensures -1 exists. */
	DUK_ASSERT(duk_is_valid_index(thr, -1));
	val_idx = duk_get_top_index_unsafe(thr);
	throw_flag = duk_is_strict_call(thr);

	rc = duk_prop_putvalue_inidx(thr, obj_idx, tv_key, val_idx, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_2_unsafe(thr);
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_put_prop_string(duk_hthread *thr, duk_idx_t obj_idx, const char *key) {
	duk_bool_t throw_flag;
	duk_bool_t rc;
	duk_idx_t val_idx;
	duk_hstring *h_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	/* Careful here and with other duk_put_prop_xxx() helpers: the
	 * target object and the property value may be in the same value
	 * stack slot (unusual, but still conceptually clear).
	 */
	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_string(thr, key);
	val_idx = duk_require_normalize_index(thr, -2);
	h_key = duk_known_hstring(thr, -1);
	throw_flag = duk_is_strict_call(thr);

	rc = duk_prop_putvalue_strkey_inidx(thr, obj_idx, h_key, val_idx, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_2_unsafe(thr);
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_put_prop_lstring(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	duk_bool_t throw_flag;
	duk_bool_t rc;
	duk_idx_t val_idx;
	duk_hstring *h_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_lstring(thr, key, key_len);
	val_idx = duk_require_normalize_index(thr, -2);
	h_key = duk_known_hstring(thr, -1);
	throw_flag = duk_is_strict_call(thr);

	rc = duk_prop_putvalue_strkey_inidx(thr, obj_idx, h_key, val_idx, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_2_unsafe(thr);
	return rc;
}

#if !defined(DUK_USE_PREFER_SIZE)
DUK_EXTERNAL duk_bool_t duk_put_prop_literal_raw(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	duk_bool_t throw_flag;
	duk_bool_t rc;
	duk_idx_t val_idx;
	duk_hstring *h_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(key[key_len] == (char) 0);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_literal_raw(thr, key, key_len);
	val_idx = duk_require_normalize_index(thr, -2);
	h_key = duk_known_hstring(thr, -1);
	throw_flag = duk_is_strict_call(thr);

	rc = duk_prop_putvalue_strkey_inidx(thr, obj_idx, h_key, val_idx, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_2_unsafe(thr);
	return rc;
}
#endif

DUK_EXTERNAL duk_bool_t duk_put_prop_index(duk_hthread *thr, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	duk_bool_t throw_flag;
	duk_bool_t rc;
	duk_idx_t val_idx;

	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	val_idx = duk_require_normalize_index(thr, -1);
	throw_flag = duk_is_strict_call(thr);

	rc = duk_prop_putvalue_idxkey_inidx(thr, obj_idx, arr_idx, val_idx, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_unsafe(thr);
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_put_prop_heapptr(duk_hthread *thr, duk_idx_t obj_idx, void *ptr) {
	duk_bool_t throw_flag;
	duk_bool_t rc;
	duk_tval *tv_key;
	duk_idx_t val_idx;

	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_heapptr(thr, ptr); /* NULL -> 'undefined' */
	val_idx = duk_require_normalize_index(thr, -2); /* Also ensures -1 exists. */
	tv_key = DUK_GET_TVAL_NEGIDX(thr, -1);
	throw_flag = duk_is_strict_call(thr);

	rc = duk_prop_putvalue_inidx(thr, obj_idx, tv_key, val_idx, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_2_unsafe(thr);
	return rc;
}

DUK_INTERNAL duk_bool_t duk_put_prop_stridx(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	duk_bool_t throw_flag;
	duk_bool_t rc;
	duk_idx_t val_idx;
	duk_hstring *h_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	val_idx = duk_require_normalize_index(thr, -1);
	h_key = DUK_HTHREAD_GET_STRING(thr, stridx);
	DUK_ASSERT(h_key != NULL);
	throw_flag = duk_is_strict_call(thr);

	rc = duk_prop_putvalue_strkey_inidx(thr, obj_idx, h_key, val_idx, throw_flag);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_unsafe(thr);
	return rc;
}

DUK_INTERNAL duk_bool_t duk_put_prop_stridx_short_raw(duk_hthread *thr, duk_uint_t packed_args) {
	return duk_put_prop_stridx(thr, (duk_idx_t) (duk_int16_t) (packed_args >> 16), (duk_small_uint_t) (packed_args & 0xffffUL));
}

DUK_EXTERNAL duk_bool_t duk_del_prop(duk_hthread *thr, duk_idx_t obj_idx) {
	duk_tval *tv_key;
	duk_small_uint_t delprop_flags;
	duk_bool_t rc;

	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	tv_key = duk_require_tval(thr, -1);
	delprop_flags = (duk_is_strict_call(thr) ? DUK_DELPROP_FLAG_THROW : 0U);

	rc = duk_prop_deleteoper(thr, obj_idx, tv_key, delprop_flags);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_unsafe(thr); /* remove key */
	return rc;
}

DUK_EXTERNAL duk_bool_t duk_del_prop_string(duk_hthread *thr, duk_idx_t obj_idx, const char *key) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_string(thr, key);
	return duk_del_prop(thr, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_del_prop_lstring(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_lstring(thr, key, key_len);
	return duk_del_prop(thr, obj_idx);
}

#if !defined(DUK_USE_PREFER_SIZE)
DUK_EXTERNAL duk_bool_t duk_del_prop_literal_raw(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(key[key_len] == (char) 0);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_literal_raw(thr, key, key_len);
	return duk_del_prop(thr, obj_idx);
}
#endif

DUK_EXTERNAL duk_bool_t duk_del_prop_index(duk_hthread *thr, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	duk_push_uarridx(thr, arr_idx);
	return duk_del_prop(thr, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_del_prop_heapptr(duk_hthread *thr, duk_idx_t obj_idx, void *ptr) {
	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_heapptr(thr, ptr); /* NULL -> 'undefined' */
	return duk_del_prop(thr, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_del_prop_stridx(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	duk_push_hstring(thr, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_del_prop(thr, obj_idx);
}

#if 0
DUK_INTERNAL duk_bool_t duk_del_prop_stridx_short_raw(duk_hthread *thr, duk_uint_t packed_args) {
	return duk_del_prop_stridx(thr, (duk_idx_t) (duk_int16_t) (packed_args >> 16),
	                                (duk_small_uint_t) (packed_args & 0xffffUL));
}
#endif

DUK_EXTERNAL duk_bool_t duk_has_prop(duk_hthread *thr, duk_idx_t obj_idx) {
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_bool_t rc;

	DUK_ASSERT_API_ENTRY(thr);

	/* Note: copying tv_obj and tv_key to locals to shield against a valstack
	 * resize is not necessary for a property existence check right now.
	 */

	tv_obj = duk_require_tval(thr, obj_idx);
	tv_key = duk_require_tval(thr, -1);

	rc = duk_prop_has(thr, tv_obj, tv_key);
	DUK_ASSERT(rc == 0 || rc == 1);

	duk_pop_unsafe(thr); /* remove key */
	return rc; /* 1 if property found, 0 otherwise */
}

DUK_EXTERNAL duk_bool_t duk_has_prop_string(duk_hthread *thr, duk_idx_t obj_idx, const char *key) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_string(thr, key);
	return duk_has_prop(thr, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_has_prop_lstring(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_lstring(thr, key, key_len);
	return duk_has_prop(thr, obj_idx);
}

#if !defined(DUK_USE_PREFER_SIZE)
DUK_EXTERNAL duk_bool_t duk_has_prop_literal_raw(duk_hthread *thr, duk_idx_t obj_idx, const char *key, duk_size_t key_len) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(key[key_len] == (char) 0);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_literal_raw(thr, key, key_len);
	return duk_has_prop(thr, obj_idx);
}
#endif

DUK_EXTERNAL duk_bool_t duk_has_prop_index(duk_hthread *thr, duk_idx_t obj_idx, duk_uarridx_t arr_idx) {
	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	duk_push_uarridx(thr, arr_idx);
	return duk_has_prop(thr, obj_idx);
}

DUK_EXTERNAL duk_bool_t duk_has_prop_heapptr(duk_hthread *thr, duk_idx_t obj_idx, void *ptr) {
	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	(void) duk_push_heapptr(thr, ptr); /* NULL -> 'undefined' */
	return duk_has_prop(thr, obj_idx);
}

DUK_INTERNAL duk_bool_t duk_has_prop_stridx(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	duk_push_hstring(thr, DUK_HTHREAD_GET_STRING(thr, stridx));
	return duk_has_prop(thr, obj_idx);
}

#if 0
DUK_INTERNAL duk_bool_t duk_has_prop_stridx_short_raw(duk_hthread *thr, duk_uint_t packed_args) {
	return duk_has_prop_stridx(thr, (duk_idx_t) (duk_int16_t) (packed_args >> 16),
	                                (duk_small_uint_t) (packed_args & 0xffffUL));
}
#endif

DUK_INTERNAL void duk_xdef_prop(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t desc_flags) {
	duk_hobject *obj;
	duk_tval *tv_key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT((desc_flags & DUK_PROPDESC_FLAGS_WEC) == desc_flags);

	obj = duk_require_hobject(thr, obj_idx);
	DUK_ASSERT(obj != NULL);
	tv_key = duk_require_tval(thr, -2);
	DUK_ASSERT(duk_require_tval(thr, -1) != NULL);

	desc_flags |= DUK_DEFPROP_FORCE;
	desc_flags |= DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_HAVE_VALUE;
	(void) duk_prop_defown(thr, obj, tv_key, duk_get_top_index_unsafe(thr), desc_flags);

	duk_pop_2_unsafe(thr); /* pop key and value */
}

DUK_INTERNAL void duk_xdef_prop_index(duk_hthread *thr, duk_idx_t obj_idx, duk_uarridx_t arr_idx, duk_small_uint_t desc_flags) {
	duk_hobject *obj;

	DUK_ASSERT_API_ENTRY(thr);

	obj = duk_require_hobject(thr, obj_idx);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(duk_is_valid_index(thr, -1)); /* valid because object exists */

	desc_flags |= DUK_DEFPROP_FORCE;
	desc_flags |= DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_HAVE_VALUE;
	(void) duk_prop_defown_idxkey(thr, obj, arr_idx, duk_get_top_index_unsafe(thr), desc_flags);
	duk_pop_unsafe(thr);
}

DUK_INTERNAL void duk_xdef_prop_stridx(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx, duk_small_uint_t desc_flags) {
	duk_hobject *obj;
	duk_hstring *key;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT_STRIDX_VALID(stridx);

	obj = duk_require_hobject(thr, obj_idx);
	DUK_ASSERT(obj != NULL);
	key = DUK_HTHREAD_GET_STRING(thr, stridx);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_is_valid_index(thr, -1)); /* valid because object exists */

	desc_flags |= DUK_DEFPROP_FORCE;
	desc_flags |= DUK_DEFPROP_HAVE_WEC | DUK_DEFPROP_HAVE_VALUE;
	(void) duk_prop_defown_strkey(thr, obj, key, duk_get_top_index_unsafe(thr), desc_flags);
	duk_pop_unsafe(thr);
}

DUK_INTERNAL void duk_xdef_prop_stridx_short_raw(duk_hthread *thr, duk_uint_t packed_args) {
	duk_xdef_prop_stridx(thr,
	                     (duk_idx_t) (duk_int8_t) (packed_args >> 24),
	                     (duk_small_uint_t) (packed_args >> 8) & 0xffffUL,
	                     (duk_small_uint_t) (packed_args & 0xffL));
}

/* This is a rare property helper; it sets the global thrower (E5 Section 13.2.3)
 * setter/getter into an object property.  This is needed by the 'arguments'
 * object creation code, function instance creation code, and Function.prototype.bind().
 */
DUK_INTERNAL void duk_xdef_prop_stridx_thrower(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	duk_push_hstring_stridx(thr, stridx);
	duk_push_hobject_bidx(thr, DUK_BIDX_TYPE_ERROR_THROWER);
	duk_dup_top_unsafe(thr);
	duk_def_prop(thr, obj_idx, DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_FORCE); /* attributes always 0 */
}

/* Object.getOwnPropertyDescriptor() equivalent C binding. */
DUK_EXTERNAL void duk_get_prop_desc(duk_hthread *thr, duk_idx_t obj_idx, duk_uint_t flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top = duk_get_top(thr);
#endif
	duk_hobject *obj;
	duk_int_t attrs;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_UNREF(flags); /* no flags defined yet */

	obj = duk_require_hobject(thr, obj_idx);
	attrs = duk_prop_getowndesc_obj_tvkey(thr, obj, duk_require_tval(thr, -1)); /* -> [ ... key <desc specific> ] */
	duk_prop_frompropdesc_propattrs(thr, attrs); /* -> [ ... key desc ] */
	duk_remove_m2(thr); /* -> [ ... desc ] */
	DUK_ASSERT(duk_get_top(thr) == entry_top); /* key replaced with desc */
}

/* Object.defineProperty() equivalent C binding. */
DUK_EXTERNAL void duk_def_prop(duk_hthread *thr, duk_idx_t obj_idx, duk_uint_t flags) {
	duk_idx_t idx_base;
	duk_hobject *obj;
	duk_uint_t is_data_desc;
	duk_uint_t is_acc_desc;

	DUK_ASSERT_API_ENTRY(thr);

	obj = duk_require_hobject(thr, obj_idx);

	is_data_desc = flags & (DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WRITABLE);
	is_acc_desc = flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER);
	if (is_data_desc && is_acc_desc) {
		/* "Have" flags must not be conflicting so that they would
		 * apply to both a plain property and an accessor at the same
		 * time.
		 */
		goto fail_invalid_desc;
	}

	idx_base = duk_get_top_index(thr);
	if (flags & DUK_DEFPROP_HAVE_SETTER) {
		duk_hobject *set;
		duk_require_type_mask(thr, idx_base, DUK_TYPE_MASK_UNDEFINED | DUK_TYPE_MASK_OBJECT | DUK_TYPE_MASK_LIGHTFUNC);
		set = duk_get_hobject_promote_lfunc(thr, idx_base);
		if (set != NULL && !DUK_HOBJECT_IS_CALLABLE(set)) {
			goto fail_not_callable;
		}
		idx_base--;
	}
	if (flags & DUK_DEFPROP_HAVE_GETTER) {
		duk_hobject *get;
		duk_require_type_mask(thr, idx_base, DUK_TYPE_MASK_UNDEFINED | DUK_TYPE_MASK_OBJECT | DUK_TYPE_MASK_LIGHTFUNC);
		get = duk_get_hobject_promote_lfunc(thr, idx_base);
		if (get != NULL && !DUK_HOBJECT_IS_CALLABLE(get)) {
			goto fail_not_callable;
		}
		idx_base--;
	}
	if (flags & DUK_DEFPROP_HAVE_VALUE) {
		idx_base--;
	}

	duk_require_valid_index(thr, idx_base);

#if 0
	if (duk_is_strict_call(thr)) {
		flags |= DUK_DEFPROP_THROW;
	}
#endif
	flags |= DUK_DEFPROP_THROW;

	duk_prop_defown(thr, obj, DUK_GET_TVAL_POSIDX(thr, idx_base), idx_base + 1, flags /*defprop_flags*/);

	/* Clean up stack */

	duk_set_top(thr, idx_base);

	/* [ ... obj ... ] */

	return;

fail_invalid_desc:
	DUK_ERROR_TYPE(thr, DUK_STR_INVALID_DESCRIPTOR);
	DUK_WO_NORETURN(return;);

fail_not_callable:
	DUK_ERROR_TYPE(thr, DUK_STR_NOT_CALLABLE);
	DUK_WO_NORETURN(return;);
}

/*
 *  Object related
 */

DUK_EXTERNAL void duk_compact(duk_hthread *thr, duk_idx_t obj_idx) {
	duk_hobject *obj;

	DUK_ASSERT_API_ENTRY(thr);

	obj = duk_get_hobject(thr, obj_idx);
	if (obj) {
		/* Note: this may fail, caller should protect the call if necessary */
		duk_hobject_compact_object(thr, obj);
	}
}

DUK_INTERNAL void duk_compact_m1(duk_hthread *thr) {
	DUK_ASSERT_API_ENTRY(thr);

	duk_compact(thr, -1);
}

DUK_EXTERNAL void duk_enum(duk_hthread *thr, duk_idx_t obj_idx, duk_uint_t enum_flags) {
	DUK_ASSERT_API_ENTRY(thr);

	duk_dup(thr, obj_idx);
	duk_require_hobject_promote_mask(thr, -1, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	duk_prop_enum_create_enumerator(thr, duk_require_hobject(thr, -1), enum_flags);
	duk_remove_m2(thr);
}

DUK_EXTERNAL duk_bool_t duk_next(duk_hthread *thr, duk_idx_t enum_index, duk_bool_t get_value) {
	DUK_ASSERT_API_ENTRY(thr);

	duk_require_hobject(thr, enum_index);
	return duk_prop_enum_next(thr, enum_index, get_value);
}

DUK_INTERNAL void duk_seal_freeze_raw(duk_hthread *thr, duk_idx_t obj_idx, duk_bool_t is_freeze) {
	duk_tval *tv;

	DUK_ASSERT_API_ENTRY(thr);

	tv = duk_require_tval(thr, obj_idx);
	DUK_ASSERT(tv != NULL);

	/* Seal/freeze are quite rare in practice so it'd be nice to get the
	 * correct behavior simply via automatic promotion (at the cost of some
	 * memory churn).  However, the promoted objects don't behave the same,
	 * e.g. promoted lightfuncs are extensible.
	 */

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_BUFFER:
		/* Plain buffer: already sealed, but not frozen.  Cannot be
		 * made frozen because index properties can't be made
		 * non-writable.  However, if length is zero, there are no
		 * offending indices and freezing is possible (same applies
		 * to standard Uint8Array).
		 */
#if 1
		if (is_freeze) {
			duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
			if (DUK_HBUFFER_GET_SIZE(h) != 0) {
				goto fail_cannot_freeze;
			}
		}
#endif
		break;
	case DUK_TAG_LIGHTFUNC:
		/* Lightfunc: already sealed and frozen, success. */
		break;
	case DUK_TAG_OBJECT: {
		/* Buffer objects cannot be frozen because there's no internal
		 * support for making virtual array indices non-writable.  Zero
		 * size buffers are special and can sometimes be frozen.
		 */
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		if (is_freeze && DUK_HOBJECT_IS_BUFOBJ(h)) {
			/* If we're trying to freeze() and buffer has length > 0,
			 * reject early.  Otherwise, for zero size buffers, continue
			 * to the generic helper.
			 */
			if (DUK_HBUFFER_GET_SIZE((duk_hbuffer *) h) > 0) {
				DUK_DD(DUK_DDPRINT("cannot freeze a buffer object with length >0"));
				goto fail_cannot_freeze;
			}
		}

		duk_hobject_object_seal_freeze_helper(thr, h, is_freeze);

		/* Sealed and frozen objects cannot gain any more properties,
		 * so this would be a good time to compact them.  At present
		 * the seal/freeze already does that however.
		 */
		DUK_HOBJECT_ASSERT_COMPACT(thr->heap, h);
		break;
	}
	default:
		/* ES2015 Sections 19.1.2.5, 19.1.2.17 */
		break;
	}
	return;

fail_cannot_freeze:
	DUK_ERROR_TYPE_INVALID_ARGS(thr); /* XXX: proper error message */
	DUK_WO_NORETURN(return;);
}

DUK_EXTERNAL void duk_seal(duk_hthread *thr, duk_idx_t obj_idx) {
	DUK_ASSERT_API_ENTRY(thr);

	duk_seal_freeze_raw(thr, obj_idx, 0 /*is_freeze*/);
}

DUK_EXTERNAL void duk_freeze(duk_hthread *thr, duk_idx_t obj_idx) {
	DUK_ASSERT_API_ENTRY(thr);

	duk_seal_freeze_raw(thr, obj_idx, 1 /*is_freeze*/);
}

/*
 *  Helpers for writing multiple properties
 */

DUK_EXTERNAL void duk_put_function_list(duk_hthread *thr, duk_idx_t obj_idx, const duk_function_list_entry *funcs) {
	const duk_function_list_entry *ent = funcs;

	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	if (ent != NULL) {
		while (ent->key != NULL) {
			duk_push_c_function(thr, ent->value, ent->nargs);
			duk_put_prop_string(thr, obj_idx, ent->key);
			ent++;
		}
	}
}

DUK_EXTERNAL void duk_put_number_list(duk_hthread *thr, duk_idx_t obj_idx, const duk_number_list_entry *numbers) {
	const duk_number_list_entry *ent = numbers;
	duk_tval *tv;

	DUK_ASSERT_API_ENTRY(thr);

	obj_idx = duk_require_normalize_index(thr, obj_idx);
	if (ent != NULL) {
		while (ent->key != NULL) {
			tv = thr->valstack_top++;
			DUK_ASSERT(DUK_TVAL_IS_UNDEFINED(tv)); /* value stack init policy */
			DUK_TVAL_SET_NUMBER_CHKFAST_SLOW(tv, ent->value); /* no need for decref/incref */
			duk_put_prop_string(thr, obj_idx, ent->key);
			ent++;
		}
	}
}

/*
 *  Shortcut for accessing global object properties
 */

DUK_EXTERNAL duk_bool_t duk_get_global_string(duk_hthread *thr, const char *key) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	ret = duk_get_prop_string(thr, -1, key);
	duk_remove_m2(thr);
	return ret;
}

DUK_EXTERNAL duk_bool_t duk_get_global_lstring(duk_hthread *thr, const char *key, duk_size_t key_len) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	ret = duk_get_prop_lstring(thr, -1, key, key_len);
	duk_remove_m2(thr);
	return ret;
}

#if !defined(DUK_USE_PREFER_SIZE)
DUK_EXTERNAL duk_bool_t duk_get_global_literal_raw(duk_hthread *thr, const char *key, duk_size_t key_len) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);
	DUK_ASSERT(key[key_len] == (char) 0);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	ret = duk_get_prop_literal_raw(thr, -1, key, key_len);
	duk_remove_m2(thr);
	return ret;
}
#endif

DUK_EXTERNAL duk_bool_t duk_get_global_heapptr(duk_hthread *thr, void *ptr) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	ret = duk_get_prop_heapptr(thr, -1, ptr);
	duk_remove_m2(thr);
	return ret;
}

DUK_EXTERNAL duk_bool_t duk_put_global_string(duk_hthread *thr, const char *key) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	duk_insert(thr, -2);
	ret = duk_put_prop_string(thr, -2, key); /* [ ... global val ] -> [ ... global ] */
	duk_pop(thr);
	return ret;
}

DUK_EXTERNAL duk_bool_t duk_put_global_lstring(duk_hthread *thr, const char *key, duk_size_t key_len) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	duk_insert(thr, -2);
	ret = duk_put_prop_lstring(thr, -2, key, key_len); /* [ ... global val ] -> [ ... global ] */
	duk_pop(thr);
	return ret;
}

#if !defined(DUK_USE_PREFER_SIZE)
DUK_EXTERNAL duk_bool_t duk_put_global_literal_raw(duk_hthread *thr, const char *key, duk_size_t key_len) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);
	DUK_ASSERT(key[key_len] == (char) 0);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	duk_insert(thr, -2);
	ret = duk_put_prop_literal_raw(thr, -2, key, key_len); /* [ ... global val ] -> [ ... global ] */
	duk_pop(thr);
	return ret;
}
#endif

DUK_EXTERNAL duk_bool_t duk_put_global_heapptr(duk_hthread *thr, void *ptr) {
	duk_bool_t ret;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);

	/* XXX: direct implementation */

	duk_push_hobject(thr, thr->builtins[DUK_BIDX_GLOBAL]);
	duk_insert(thr, -2);
	ret = duk_put_prop_heapptr(thr, -2, ptr); /* [ ... global val ] -> [ ... global ] */
	duk_pop(thr);
	return ret;
}

/*
 *  ES2015 GetMethod()
 */

DUK_INTERNAL duk_bool_t duk_get_method_stridx(duk_hthread *thr, duk_idx_t idx, duk_small_uint_t stridx) {
	(void) duk_get_prop_stridx(thr, idx, stridx);
	if (duk_is_nullish(thr, -1)) {
		duk_pop_nodecref_unsafe(thr);
		return 0;
	}
	if (!duk_is_callable(thr, -1)) {
		DUK_ERROR_TYPE(thr, DUK_STR_NOT_CALLABLE);
		DUK_WO_NORETURN(return 0;);
	}
	return 1;
}

/*
 *  Object prototype
 */

DUK_EXTERNAL void duk_get_prototype(duk_hthread *thr, duk_idx_t idx) {
	duk_hobject *obj;
	duk_hobject *proto;

	DUK_ASSERT_API_ENTRY(thr);

	obj = duk_require_hobject(thr, idx);
	DUK_ASSERT(obj != NULL);

	proto = duk_hobject_get_proto_raw(thr->heap, obj);
	duk_push_hobject_or_undefined(thr, proto);
}

DUK_EXTERNAL void duk_set_prototype(duk_hthread *thr, duk_idx_t idx) {
	duk_hobject *obj;
	duk_hobject *proto;

	DUK_ASSERT_API_ENTRY(thr);

	obj = duk_require_hobject(thr, idx);
	DUK_ASSERT(obj != NULL);
	duk_require_type_mask(thr, -1, DUK_TYPE_MASK_UNDEFINED | DUK_TYPE_MASK_OBJECT);
	proto = duk_get_hobject(thr, -1);
	/* proto can also be NULL here (allowed explicitly) */

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj)) {
		DUK_ERROR_TYPE(thr, DUK_STR_NOT_CONFIGURABLE); /* XXX: "read only object"? */
		DUK_WO_NORETURN(return;);
	}
#endif

	duk_hobject_set_proto_raw_updref(thr, obj, proto);

	duk_pop(thr);
}

DUK_INTERNAL void duk_clear_prototype(duk_hthread *thr, duk_idx_t idx) {
	duk_hobject *obj;

	DUK_ASSERT_API_ENTRY(thr);

	obj = duk_require_hobject(thr, idx);
	DUK_ASSERT(obj != NULL);

#if defined(DUK_USE_ROM_OBJECTS)
	if (DUK_HEAPHDR_HAS_READONLY((duk_heaphdr *) obj)) {
		DUK_ERROR_TYPE(thr, DUK_STR_NOT_CONFIGURABLE); /* XXX: "read only object"? */
		DUK_WO_NORETURN(return;);
	}
#endif

	duk_hobject_set_proto_raw_updref(thr, obj, NULL);
}

DUK_INTERNAL duk_bool_t duk_is_bare_object(duk_hthread *thr, duk_idx_t idx) {
	duk_hobject *obj;
	duk_hobject *proto;

	DUK_ASSERT_API_ENTRY(thr);

	obj = duk_require_hobject(thr, idx);
	DUK_ASSERT(obj != NULL);

	proto = duk_hobject_get_proto_raw(thr->heap, obj);
	return (proto == NULL);
}

/*
 *  Object finalizer
 */

#if defined(DUK_USE_FINALIZER_SUPPORT)
/* XXX: these could be implemented as macros calling an internal function
 * directly.
 * XXX: same issue as with Duktape.fin: there's no way to delete the property
 * now (just set it to undefined).
 */
DUK_EXTERNAL void duk_get_finalizer(duk_hthread *thr, duk_idx_t idx) {
	DUK_ASSERT_API_ENTRY(thr);

	/* This get intentionally walks the inheritance chain at present,
	 * which matches how the effective finalizer property is also
	 * looked up in GC.
	 */
	duk_get_prop_stridx(thr, idx, DUK_STRIDX_INT_FINALIZER);
}

DUK_EXTERNAL void duk_set_finalizer(duk_hthread *thr, duk_idx_t idx) {
	duk_hobject *h;
	duk_bool_t callable;

	DUK_ASSERT_API_ENTRY(thr);

	h = duk_require_hobject(thr, idx); /* Get before 'put' so that 'idx' is correct. */
	callable = duk_is_callable(thr, -1);

	/* At present finalizer is stored as a hidden Symbol, with normal
	 * inheritance and access control.  As a result, finalizer cannot
	 * currently be set on a non-extensible (sealed or frozen) object.
	 * It might be useful to allow it.
	 */
	duk_put_prop_stridx(thr, idx, DUK_STRIDX_INT_FINALIZER);

	/* In addition to setting the finalizer property, keep a "have
	 * finalizer" flag in duk_hobject in sync so that refzero can do
	 * a very quick finalizer check by walking the prototype chain
	 * and checking the flag alone.  (Note that this means that just
	 * setting _Finalizer on an object won't affect finalizer checks.)
	 *
	 * NOTE: if the argument is a Proxy object, this flag will be set
	 * on the Proxy, not the target.  As a result, the target won't get
	 * a finalizer flag and the Proxy also won't be finalized as there's
	 * an explicit Proxy check in finalization now.
	 */

	if (callable) {
		DUK_HOBJECT_SET_HAVE_FINALIZER(h);
	} else {
		DUK_HOBJECT_CLEAR_HAVE_FINALIZER(h);
	}
}
#else /* DUK_USE_FINALIZER_SUPPORT */
DUK_EXTERNAL void duk_get_finalizer(duk_hthread *thr, duk_idx_t idx) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_UNREF(idx);
	DUK_ERROR_UNSUPPORTED(thr);
	DUK_WO_NORETURN(return;);
}

DUK_EXTERNAL void duk_set_finalizer(duk_hthread *thr, duk_idx_t idx) {
	DUK_ASSERT_API_ENTRY(thr);
	DUK_UNREF(idx);
	DUK_ERROR_UNSUPPORTED(thr);
	DUK_WO_NORETURN(return;);
}
#endif /* DUK_USE_FINALIZER_SUPPORT */
