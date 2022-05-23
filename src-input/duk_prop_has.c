/*
 *  [[HasProperty]], 'in' operator
 */

#include "duk_internal.h"

/* Outcome for existence check. */
#define DUK__HASOWN_NOTFOUND      0 /* not found, continue to parent */
#define DUK__HASOWN_FOUND         1 /* found, stop walk */
#define DUK__HASOWN_DONE_NOTFOUND 2 /* not found, stop walk */

DUK_LOCAL_DECL duk_bool_t duk__prop_has_obj_strkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_LOCAL_DECL duk_bool_t duk__prop_has_obj_strkey_safe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_LOCAL_DECL duk_bool_t duk__prop_has_obj_idxkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx);
DUK_LOCAL_DECL duk_bool_t duk__prop_has_obj_idxkey_safe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx);

DUK_LOCAL duk_small_int_t duk__prop_hasown_strkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	duk_uint_fast32_t ent_idx;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	rc = duk_hobject_lookup_strprop_index(thr, obj, key, &ent_idx);
	DUK_UNREF(ent_idx);
	return (duk_small_int_t) rc;
}

DUK_LOCAL duk_small_int_t duk__prop_hasown_idxkey_ordinary(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	duk_uint_fast32_t ent_idx;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);

	rc = duk_hobject_lookup_idxprop_index(thr, obj, idx, &ent_idx);
	DUK_UNREF(ent_idx);
	return (duk_small_int_t) rc;
}

DUK_LOCAL duk_small_int_t duk__prop_hasown_idxkey_stringobj(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	duk_hstring *h;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_STRING_OBJECT);
	DUK_ASSERT_ARRIDX_VALID(idx);

	h = duk_hobject_lookup_intvalue_hstring(thr, obj);
	if (DUK_LIKELY(h != NULL)) {
		if (DUK_LIKELY(!DUK_HSTRING_HAS_SYMBOL(h) && idx < duk_hstring_get_charlen(h))) {
			return DUK__HASOWN_FOUND;
		}
	}

	return DUK__HASOWN_NOTFOUND;
}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_LOCAL duk_small_int_t duk__prop_hasown_idxkey_typedarray(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	duk_hbufobj *h;
	duk_uint8_t *data;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HEAPHDR_IS_ANY_BUFOBJ((duk_heaphdr *) obj));
	DUK_ASSERT_ARRIDX_VALID(idx);

	h = (duk_hbufobj *) obj;

	data = duk_hbufobj_get_validated_data_ptr(thr, h, idx);
	if (DUK_LIKELY(data != NULL)) {
		return DUK__HASOWN_FOUND;
	} else {
		/* Out-of-bounds, detached, uncovered: treat like out-of-bounds. */
		return DUK__HASOWN_DONE_NOTFOUND;
	}
}
#else
DUK_LOCAL duk_small_int_t duk__prop_hasown_idxkey_typedarray(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	return duk__prop_hasown_idxkey_error(thr, obj, idx);
}
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */

DUK_LOCAL duk_small_int_t duk__prop_hasown_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	/* Here we need either [[HasProperty]] (if exotic) or [[GetOwnProperty]]
	 * for each object variant, but we don't need the associated value,
	 * just existence.
	 */
	switch (htype) {
	case DUK_HTYPE_ARRAY:
		/* Special because 'length' is stored specially. */
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			return DUK__HASOWN_FOUND;
		}
		break;
	case DUK_HTYPE_ARGUMENTS:
		/* For string keys, reduces to OrdinaryHasProperty(). */
		break;
	case DUK_HTYPE_STRING_OBJECT:
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			return DUK__HASOWN_FOUND;
		}
		break;
	case DUK_HTYPE_PROXY:
		/* Handled by caller to ensure stabilization. */
		return DUK__HASOWN_NOTFOUND;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
		/* Nothing special. */
		break;
	case DUK_HTYPE_INT8ARRAY:
	case DUK_HTYPE_UINT8ARRAY:
	case DUK_HTYPE_UINT8CLAMPEDARRAY:
	case DUK_HTYPE_INT16ARRAY:
	case DUK_HTYPE_UINT16ARRAY:
	case DUK_HTYPE_INT32ARRAY:
	case DUK_HTYPE_UINT32ARRAY:
	case DUK_HTYPE_FLOAT32ARRAY:
	case DUK_HTYPE_FLOAT64ARRAY:
		/* In specification no special string keys, but we present
		 * a virtual, own 'length'.
		 *
		 * All CanonicalNumericIndexStrings (not covered by index
		 * path) are captured and are considered absent with no
		 * further inheritance lookup.
		 */
		if (DUK_HSTRING_HAS_LENGTH_OR_CANNUM(key)) {
			if (DUK_HSTRING_HAS_LENGTH(key)) {
				return DUK__HASOWN_FOUND;
			} else {
				DUK_ASSERT(DUK_HSTRING_HAS_CANNUM(key));
				return DUK__HASOWN_DONE_NOTFOUND;
			}
		}
		break;
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	}

	return duk__prop_hasown_strkey_ordinary(thr, obj, key);
}

DUK_LOCAL duk_small_int_t duk__prop_hasown_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);

	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		/* Special because keys are potentially stored specially. */
		if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
			duk_harray *a = (duk_harray *) obj;
			if (idx < DUK_HARRAY_GET_ITEMS_LENGTH(a)) {
				duk_tval *tv = DUK_HARRAY_GET_ITEMS(thr->heap, a) + idx;
				if (!DUK_TVAL_IS_UNUSED(tv)) {
					return DUK__HASOWN_FOUND;
				} else {
					return DUK__HASOWN_NOTFOUND;
				}
			}
			return DUK__HASOWN_NOTFOUND; /* Comprehensiveness. */
		}
		break;
	case DUK_HTYPE_ARGUMENTS:
		/* No exotic [[HasProperty]]; exotic [[GetOwnProperty]] uses
		 * OrdinaryGetOwnProperty() and fudges the result, but this
		 * has no effect on existence and no side effects, so just use
		 * the ordinary algorithm.
		 */
		break;
	case DUK_HTYPE_STRING_OBJECT:
		return duk__prop_hasown_idxkey_stringobj(thr, obj, idx);
	case DUK_HTYPE_PROXY:
		/* Proxy trap lookup may have arbitrary side effects (even the
		 * handler object may be a Proxy itself) so handle in caller.
		 */
		return DUK__HASOWN_NOTFOUND;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
		/* Nothing special. */
		break;
	case DUK_HTYPE_INT8ARRAY:
	case DUK_HTYPE_UINT8ARRAY:
	case DUK_HTYPE_UINT8CLAMPEDARRAY:
	case DUK_HTYPE_INT16ARRAY:
	case DUK_HTYPE_UINT16ARRAY:
	case DUK_HTYPE_INT32ARRAY:
	case DUK_HTYPE_UINT32ARRAY:
	case DUK_HTYPE_FLOAT32ARRAY:
	case DUK_HTYPE_FLOAT64ARRAY:
		/* Exotic [[HasProperty]], short circuit all
		 * CanonicalNumericIndexString properties.
		 */
		return duk__prop_hasown_idxkey_typedarray(thr, obj, idx);
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	}

	return duk__prop_hasown_idxkey_ordinary(thr, obj, idx);
}

DUK_LOCAL duk_bool_t duk__prop_has_proxy_tail(duk_hthread *thr) {
	duk_bool_t rc;

	/* [ ... trap handler target key ] */
	duk_dup_top(thr);
	duk_insert(thr, -5); /* [ ... key trap handler target key ] */
	duk_dup_m2(thr);
	duk_insert(thr, -6); /* [ ... target key trap handler target key ] */

	duk_call_method(thr, 2); /* [ ... target key trap handler target key ] -> [ ... target key result ] */
	rc = duk_to_boolean(thr, -1);

	if (!rc) {
		duk_small_int_t attrs;

		attrs = duk_prop_getowndesc_obj_tvkey(thr, duk_require_hobject(thr, -3), duk_require_tval(thr, -2));
		duk_prop_pop_propdesc(thr, attrs);

		if (attrs >= 0) {
			duk_small_uint_t uattrs = (duk_small_uint_t) attrs;

			if (!(uattrs & DUK_PROPDESC_FLAG_CONFIGURABLE)) {
				goto invalid_result;
			}
			if (!duk_js_isextensible(thr, duk_require_hobject(thr, -3))) {
				goto invalid_result;
			}
		}
	}

	duk_pop_3_unsafe(thr);
	DUK_ASSERT(DUK__HASOWN_NOTFOUND == 0 && DUK__HASOWN_FOUND == 1);
	DUK_ASSERT(rc == DUK__HASOWN_NOTFOUND || rc == DUK__HASOWN_FOUND);
	return rc;

invalid_result:
	DUK_ERROR_TYPE(thr, DUK_STR_INVALID_TRAP_RESULT);
	DUK_WO_NORETURN(return 0;);
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__prop_has_obj_stroridx_helper(duk_hthread *thr,
                                                                         duk_hobject *obj,
                                                                         duk_hstring *key,
                                                                         duk_uarridx_t idx,
                                                                         duk_bool_t use_key,
                                                                         duk_bool_t side_effect_safe) {
	duk_bool_t rc;
	duk_small_uint_t sanity;

	/* Ordinary [[HasProperty]] uses [[GetOwnProperty]] and if property
	 * is not found, looks up next object using [[GetPrototypeOf]], and
	 * calls the next object's [[HasProperty]].
	 *
	 * [[GetPrototypeOf]] is not invoked for Proxies as they have a
	 * custom [[HasProperty]].
	 */

	if (side_effect_safe) {
		duk_push_hobject(thr, obj);
	}

	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	do {
		duk_hobject *next;
		duk_small_int_t rc_hasown;

		if (use_key) {
			rc_hasown = duk__prop_hasown_strkey(thr, obj, key);
		} else {
			rc_hasown = duk__prop_hasown_idxkey(thr, obj, idx);
		}

		if (rc_hasown >= 1) {
			DUK_ASSERT(DUK__HASOWN_DONE_NOTFOUND == 2);
			DUK_ASSERT((DUK__HASOWN_DONE_NOTFOUND & 0x01) == 0);
			rc = ((duk_bool_t) rc_hasown) & 0x01U; /* convert 'done, not found' (= 2) to 0 (not found) */
			goto done;
		}
		DUK_ASSERT(rc_hasown == DUK__HASOWN_NOTFOUND);

		next = duk_hobject_get_proto_raw(thr->heap, obj);
		if (next == NULL) {
			/* Proxy needs special handling which we can deal with
			 * here as proxy internal prototype is always NULL.
			 */
			if (DUK_UNLIKELY(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(obj))) {
				if (side_effect_safe) {
					if (use_key) {
						if (duk_proxy_trap_check_strkey(thr, (duk_hproxy *) obj, key, DUK_STRIDX_HAS)) {
							duk_push_hstring(thr, key);
							rc = duk__prop_has_proxy_tail(thr);
							goto done;
						}
					} else {
						if (duk_proxy_trap_check_idxkey(thr, (duk_hproxy *) obj, idx, DUK_STRIDX_HAS)) {
							(void) duk_push_u32_tostring(thr, idx);
							rc = duk__prop_has_proxy_tail(thr);
							goto done;
						}
					}

					DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(obj));
					next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
					DUK_ASSERT(next != NULL);
					goto go_next;
				} else {
					goto switch_to_safe;
				}
			}

			rc = 0;
			goto done;
		}

	go_next:
		DUK_ASSERT(next != NULL);
		if (side_effect_safe) {
			obj = duk_prop_switch_stabilized_target_top(thr, obj, next);
		} else {
			obj = next;
		}
	} while (--sanity > 0);

	DUK_ERROR_RANGE_PROTO_SANITY(thr);
	DUK_WO_NORETURN(return 0;);

done:
	if (side_effect_safe) {
		duk_pop_unsafe(thr);
	}

	return rc;

switch_to_safe:
	if (use_key) {
		return duk__prop_has_obj_strkey_safe(thr, obj, key);
	} else {
		return duk__prop_has_obj_idxkey_safe(thr, obj, idx);
	}
}

DUK_LOCAL duk_bool_t duk__prop_has_obj_strkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_has_obj_strkey_safe(thr, obj, key);
#else
	return duk__prop_has_obj_stroridx_helper(thr, obj, key, 0, 1 /*use_key*/, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL duk_bool_t duk__prop_has_obj_strkey_safe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	return duk__prop_has_obj_stroridx_helper(thr, obj, key, 0, 1 /*use_key*/, 1 /*side_effect_safe*/);
}

DUK_LOCAL duk_bool_t duk__prop_has_obj_idxkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	DUK_ASSERT_ARRIDX_VALID(idx);
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_has_obj_idxkey_safe(thr, obj, idx);
#else
	return duk__prop_has_obj_stroridx_helper(thr, obj, NULL, idx, 0 /*use_key*/, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL duk_bool_t duk__prop_has_obj_idxkey_safe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	DUK_ASSERT_ARRIDX_VALID(idx);
	return duk__prop_has_obj_stroridx_helper(thr, obj, NULL, idx, 0 /*use_key*/, 1 /*side_effect_safe*/);
}

DUK_LOCAL duk_bool_t duk__prop_has_strkey(duk_hthread *thr, duk_tval *tv_obj, duk_hstring *key) {
	duk_hobject *next;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv_obj) || DUK_TVAL_IS_BUFFER(tv_obj) || DUK_TVAL_IS_LIGHTFUNC(tv_obj));
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	if (DUK_LIKELY(DUK_TVAL_IS_OBJECT(tv_obj))) {
		next = DUK_TVAL_GET_OBJECT(tv_obj);
	} else if (DUK_TVAL_IS_BUFFER(tv_obj)) {
		/* All CanonicalNumericIndexStrings (including -0 here) must
		 * be captured and treated as absent (to mimic Uint8Array).
		 * Present a non-standard .length property.
		 */
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			return 1;
		}
		if (DUK_HSTRING_HAS_CANNUM(key)) {
			return 0;
		}
		next = thr->builtins[DUK_BIDX_UINT8ARRAY_PROTOTYPE];
	} else {
		DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv_obj));
		next = thr->builtins[DUK_BIDX_NATIVE_FUNCTION_PROTOTYPE];
	}

	return duk__prop_has_obj_strkey_unsafe(thr, next, key);
}

DUK_LOCAL duk_bool_t duk__prop_has_idxkey(duk_hthread *thr, duk_tval *tv_obj, duk_uarridx_t idx) {
	duk_hobject *next;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv_obj != NULL);
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv_obj) || DUK_TVAL_IS_BUFFER(tv_obj) || DUK_TVAL_IS_LIGHTFUNC(tv_obj));
	DUK_ASSERT_ARRIDX_VALID(idx);

	if (DUK_LIKELY(DUK_TVAL_IS_OBJECT(tv_obj))) {
		next = DUK_TVAL_GET_OBJECT(tv_obj);
	} else if (DUK_TVAL_IS_BUFFER(tv_obj)) {
		/* All arridx properties are CanonicalNumericIndexStrings, and plain
		 * buffer captures any [[HasProperty]] check (like Uint8Array).
		 */
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv_obj);
		if (idx < DUK_HBUFFER_GET_SIZE(h)) {
			return 1;
		} else {
			return 0;
		}
	} else {
		/* No index valued own properties. */
		DUK_ASSERT(DUK_TVAL_IS_LIGHTFUNC(tv_obj));
		next = thr->builtins[DUK_BIDX_NATIVE_FUNCTION_PROTOTYPE];
	}

	return duk__prop_has_obj_idxkey_unsafe(thr, next, idx);
}

DUK_INTERNAL duk_bool_t duk_prop_has_strkey(duk_hthread *thr, duk_tval *tv_obj, duk_hstring *key) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv_obj != NULL);
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv_obj) || DUK_TVAL_IS_BUFFER(tv_obj) || DUK_TVAL_IS_LIGHTFUNC(tv_obj));
	DUK_ASSERT(key != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
		duk_bool_t rc;

		rc = duk__prop_has_idxkey(thr, tv_obj, duk_hstring_get_arridx_fast_known(key));
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	} else {
		duk_bool_t rc;

		DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
		rc = duk__prop_has_strkey(thr, tv_obj, key);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	}
}

DUK_INTERNAL duk_bool_t duk_prop_has_idxkey(duk_hthread *thr, duk_tval *tv_obj, duk_uarridx_t idx) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv_obj != NULL);
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv_obj) || DUK_TVAL_IS_BUFFER(tv_obj) || DUK_TVAL_IS_LIGHTFUNC(tv_obj));
	DUK_ASSERT_ARRIDX_VALID(idx);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_LIKELY(idx <= DUK_ARRIDX_MAX)) {
		duk_bool_t rc = duk__prop_has_idxkey(thr, tv_obj, idx);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	} else {
		duk_bool_t rc;
		duk_hstring *key;

		DUK_DD(DUK_DDPRINT("corner case, input idx 0xffffffff is not an arridx, must coerce to string"));
		key = duk_push_u32_tohstring(thr, idx);
		rc = duk__prop_has_strkey(thr, tv_obj, key);
		duk_pop_unsafe(thr);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	}
}

DUK_INTERNAL duk_bool_t duk_prop_has(duk_hthread *thr, duk_tval *tv_obj, duk_tval *tv_key) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_bool_t rc;
	duk_hstring *key;
	duk_uarridx_t idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(tv_obj != NULL);
	DUK_ASSERT(tv_key != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	/* Behavior for non-object rvalue differs from other property
	 * operations: primitive values are rejected, and rvalue (base)
	 * is considered before the key.  This is visible from coercion
	 * side effects.
	 *
	 * Lightfuncs and plain buffers behave like objects so we allow
	 * them as rvalues.
	 */

	switch (DUK_TVAL_GET_TAG(tv_obj)) {
	case DUK_TAG_OBJECT:
	case DUK_TAG_LIGHTFUNC:
	case DUK_TAG_BUFFER:
		break;
	default:
		DUK_ERROR_TYPE_INVALID_RVALUE(thr);
		DUK_WO_NORETURN(return 0;);
	}

	switch (DUK_TVAL_GET_TAG(tv_key)) {
	case DUK_TAG_STRING:
		key = DUK_TVAL_GET_STRING(tv_key);
		if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
			idx = duk_hstring_get_arridx_fast_known(key);
			goto use_idx;
		} else {
			DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
			goto use_str;
		}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT: {
		duk_int64_t fi = DUK_TVAL_GET_FASTINT(tv_key);
		if (fi >= 0 && fi <= (duk_int64_t) DUK_ARRIDX_MAX) {
			idx = (duk_uarridx_t) fi;
			goto use_idx;
		}
		break;
	}
#endif
#if !defined(DUK_USE_PACKED_TVAL)
	case DUK_TAG_NUMBER: {
		duk_double_t d = DUK_TVAL_GET_DOUBLE(tv_key);
		if (duk_prop_double_idx_check(d, &idx)) {
			goto use_idx;
		}
		break;
	}
#endif
	case DUK_TAG_UNUSED:
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
	case DUK_TAG_BOOLEAN:
	case DUK_TAG_POINTER:
	case DUK_TAG_LIGHTFUNC:
	case DUK_TAG_OBJECT:
	case DUK_TAG_BUFFER:
	default: {
#if defined(DUK_USE_PACKED_TVAL)
		if (DUK_TVAL_IS_NUMBER(tv_key)) {
			duk_double_t d = DUK_TVAL_GET_DOUBLE(tv_key);
			if (duk_prop_double_idx_check(d, &idx)) {
				goto use_idx;
			}
		}
#endif
		break;
	}
	}

	/* We need to coerce the key, and we need temporary value
	 * stack space to do it.  Do it on stack top and recurse
	 * (with no risk of further recursion).  All pointers need
	 * to be stabilized to do this.
	 */
	duk_push_tval(thr, tv_obj);
	duk_push_tval(thr, tv_key);
	(void) duk_to_property_key_hstring(thr, -1);
	rc = duk_prop_has(thr, DUK_GET_TVAL_NEGIDX(thr, -2), DUK_GET_TVAL_NEGIDX(thr, -1));
	duk_pop_2_unsafe(thr);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;

use_idx:
	DUK_ASSERT_ARRIDX_VALID(idx);
	rc = duk__prop_has_idxkey(thr, tv_obj, idx);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;

use_str:
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	rc = duk__prop_has_strkey(thr, tv_obj, key);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;
}
