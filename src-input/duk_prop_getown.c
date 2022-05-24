/*
 *  [[GetOwnProperty]]
 *
 *  Property descriptor is represented as 0-2 values on the value stack and
 *  a signed int attribute return value:
 *
 *    < 0 (-1)                                   not found, no values pushed
 *    >= 0, DUK_PROPDESC_FLAG_ACCESSOR not set   found data property, 1 value pushed
 *    >= 0, DUK_PROPDESC_FLAG_ACCESSOR set       found accessor property, 2 values (get, set) pushed
 *
 *  There's a helper to pop the result based on the result value:
 *  duk_prop_pop_propdesc().
 *
 *  The specification algorithm works on objects only.  ECMAScript bindings
 *  coerce property key and object arguments, but unfortunately not always
 *  in the same order.  For example, Object.getOwnPropertyDescriptor() first
 *  ToObject() coerces the object argument, followed by ToPropertyKey() for
 *  the key argument, while Object.prototype.propertyIsEnumerable() swaps
 *  the order.
 */

DUK_LOCAL_DECL duk_small_int_t duk__prop_getowndesc_strkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_LOCAL_DECL duk_small_int_t duk__prop_getowndesc_strkey_safe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key);
DUK_LOCAL_DECL duk_small_int_t duk__prop_getowndesc_idxkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx);
DUK_LOCAL_DECL duk_small_int_t duk__prop_getowndesc_idxkey_safe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx);

DUK_LOCAL duk_small_int_t duk__prop_plain_tail(duk_hthread *thr, duk_propvalue *pv, duk_uint8_t attrs) {
	if (DUK_LIKELY(attrs & DUK_PROPDESC_FLAG_ACCESSOR) == 0) {
		duk_push_tval(thr, &pv->v);
	} else {
		duk_push_hobject_or_undefined(thr, pv->a.get);
		duk_push_hobject_or_undefined(thr, pv->a.set);
	}
	return attrs;
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_strkey_plain(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	duk_propvalue *pv;
	duk_uint8_t attrs;

	if (duk_hobject_lookup_strprop_val_attrs(thr, obj, key, &pv, &attrs) != 0) {
		return duk__prop_plain_tail(thr, pv, attrs);
	}
	return -1;
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_idxkey_plain(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	duk_propvalue *pv;
	duk_uint8_t attrs;

	if (duk_hobject_lookup_idxprop_val_attrs(thr, obj, idx, &pv, &attrs) != 0) {
		return duk__prop_plain_tail(thr, pv, attrs);
	}
	return -1;
}

#if defined(DUK_USE_PROXY_POLICY)
DUK_LOCAL duk_small_int_t duk__prop_getown_proxy_policy(duk_hthread *thr, duk_hobject *obj) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top = duk_get_top(thr);
#endif
	duk_hobject *target;
	duk_small_int_t attrs;
	duk_small_int_t attrs_target;

	/* [ ... key result ] */

	DUK_ASSERT(duk_is_object(thr, -1) || duk_is_undefined(thr, -1));

	target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
	DUK_ASSERT(target != NULL);

	attrs_target = duk_prop_getownattr_obj_tvkey(thr, target, duk_require_tval(thr, -2));
	duk_remove_m2(thr);

	/* [ ... result ] */

	if (duk_is_undefined(thr, -1)) {
		if (attrs_target >= 0) {
			duk_small_uint_t uattrs_target = (duk_small_uint_t) attrs_target;
			if ((uattrs_target & DUK_PROPDESC_FLAG_CONFIGURABLE) == 0) {
				goto fail;
			}
			if (!duk_js_isextensible(thr, target)) {
				goto fail;
			}
		}
		attrs = -1;
		duk_pop_known(thr);
	} else {
		DUK_ASSERT(duk_is_object(thr, -1));
		attrs = (duk_small_int_t) duk_prop_topropdesc(thr); /* -> [ ... <variable> ] */
		DUK_ASSERT(attrs >= 0);
		/* XXX: Missing stuff here. */
	}

	DUK_ASSERT(duk_get_top(thr) == entry_top - 2 + duk_prop_propdesc_valcount(attrs));
	return attrs;

fail:
	DUK_ERROR_TYPE(thr, DUK_STR_INVALID_TRAP_RESULT);
	DUK_WO_NORETURN(return 0;);
}
#endif

DUK_LOCAL duk_small_int_t duk__prop_getown_proxy_tail(duk_hthread *thr, duk_hobject *obj) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top = duk_get_top(thr);
#endif
	duk_small_int_t attrs;

	/* [ ... trap handler target key ] */

	duk_dup_top(thr);
	duk_insert(thr, -5); /* Stash key for policy check. */

	/* [ ... key trap handler target key ] */

	duk_call_method(thr, 2); /* [ ... key trap handler target key ] -> [ ... key result ] */

	/* [ ... key result ] */

	if (duk_check_type_mask(thr, -1, DUK_TYPE_MASK_UNDEFINED | DUK_TYPE_MASK_OBJECT)) {
#if defined(DUK_USE_PROXY_POLICY)
		attrs = duk__prop_getown_proxy_policy(thr, obj);
#else
		DUK_DD(DUK_DDPRINT("proxy policy check for 'getOwnPropertyDescriptor' trap disabled in configuration"));
		/* XXX: Missing stuff here. */
		attrs = -1;
#endif
	} else {
		goto invalid_result;
	}

	/* [ ... <variable> ] */

	DUK_ASSERT(duk_get_top(thr) == entry_top - 4 + duk_prop_propdesc_valcount(attrs));
	return attrs;

invalid_result:
	DUK_ERROR_TYPE(thr, DUK_STR_INVALID_TRAP_RESULT);
	DUK_WO_NORETURN(return 0;);
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_strkey_helper(duk_hthread *thr,
                                                             duk_hobject *target,
                                                             duk_hstring *key,
                                                             duk_bool_t side_effect_safe) {
	duk_small_uint_t htype;
	duk_small_int_t rc;
	duk_idx_t idx_target = 0;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(target != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	if (side_effect_safe) {
		idx_target = duk_get_top(thr);
		duk_push_hobject(thr, target);
	}

retry_target:
	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) target);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	/* Object types need special handling for exotic [[GetOwnProperty]]
	 * behaviors and for virtual properties not stored in the common
	 * property table.
	 */

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			duk_harray *a = (duk_harray *) target;
			duk_push_u32(thr, DUK_HARRAY_GET_LENGTH(a));
			rc = (duk_small_int_t) (DUK_HARRAY_LENGTH_NONWRITABLE(a) ? DUK_PROPDESC_FLAGS_NONE : DUK_PROPDESC_FLAGS_W);
			goto return_rc;
		}
		break;
	case DUK_HTYPE_ARGUMENTS:
		/* Map lookup is side effect free and never finds an entry for
		 * a string key.  So here Arguments [[GetOwnProperty]]
		 * simplifies to the ordinary algorithm; 'length' is just an
		 * ordinary property.
		 */
		break;
	case DUK_HTYPE_STRING_OBJECT:
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			duk_hstring *h = duk_hobject_lookup_intvalue_hstring(thr, target);
			duk_push_u32(thr, duk_hstring_get_charlen(h));
			rc = DUK_PROPDESC_FLAGS_NONE;
			goto return_rc;
		}
		break;
	case DUK_HTYPE_PROXY:
		if (side_effect_safe) {
			duk_hobject *next;

			if (duk_proxy_trap_check_strkey(thr, (duk_hproxy *) target, key, DUK_STRIDX_GET_OWN_PROPERTY_DESCRIPTOR)) {
				duk_push_hstring(thr, key);
				rc = duk__prop_getown_proxy_tail(thr, target);
				goto return_rc;
			}

			DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(target));
			next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) target);
			DUK_ASSERT(next != NULL);
			target = duk_prop_switch_stabilized_target_top(thr, target, next);
			goto retry_target;
		} else {
			goto switch_to_safe;
		}
		break;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
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
		/* In the specification there is no own 'length' property
		 * but only an accessor.  But since we provide an own
		 * property too, provide it also here for consistency.
		 */
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			duk_hbufobj *h = (duk_hbufobj *) target;
			duk_uint_t buflen = DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h);
			duk_push_uint(thr, buflen);
			rc = DUK_PROPDESC_FLAGS_NONE;
			goto return_rc;
		}
		if (DUK_HSTRING_HAS_CANNUM(key)) {
			rc = -1;
			goto return_rc;
		}
		break;
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	rc = duk__prop_getowndesc_strkey_plain(thr, target, key);
	/* fall thru */

return_rc:
	if (side_effect_safe) {
		/* The stabilized target is not necessarily at the stack top,
		 * so can't pop the top as for e.g. prop get/set.
		 */
		DUK_ASSERT(duk_get_hobject(thr, idx_target) == target);
		duk_remove(thr, idx_target);
	}
	return rc;

switch_to_safe:
	return duk__prop_getowndesc_strkey_safe(thr, target, key);
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_strkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_getowndesc_strkey_safe(thr, obj, key);
#else
	return duk__prop_getowndesc_strkey_helper(thr, obj, key, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_strkey_safe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	return duk__prop_getowndesc_strkey_helper(thr, obj, key, 1 /*side_effect_safe*/);
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_idxkey_arguments(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	duk_hobject *map;
	duk_hobject *env;
	duk_hstring *varname;

	/* Conceptually look up ordinary own descriptor first.
	 * Then update the descriptor [[Value]] from the map.
	 * If mapped, the property must be a data descriptor
	 * because we'd remove the mapping if it was converted
	 * to an accessor.
	 */

	duk_small_int_t rc;
	duk_small_uint_t uattrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj) == DUK_HTYPE_ARGUMENTS);
	DUK_ASSERT_ARRIDX_VALID(idx);

	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
		duk_harray *a = (duk_harray *) obj;
		if (idx < DUK_HARRAY_GET_ITEMS_LENGTH(a)) {
			duk_tval *tv_val = DUK_HARRAY_GET_ITEMS(thr->heap, a) + idx;
			if (DUK_TVAL_IS_UNUSED(tv_val)) {
				rc = -1;
			} else {
				duk_push_tval(thr, tv_val);
				rc = DUK_PROPDESC_FLAGS_WEC;
			}
		} else {
			/* Comprehensiveness: cannot be in property table. */
			rc = -1;
		}
	} else {
		rc = duk__prop_getowndesc_idxkey_plain(thr, obj, idx);
	}

	if (rc < 0) {
		return rc;
	}

	uattrs = (duk_small_uint_t) rc;

	if ((uattrs & DUK_PROPDESC_FLAG_ACCESSOR) == 0) {
		varname = duk_prop_arguments_map_prep_idxkey(thr, obj, idx, &map, &env);
		if (varname != NULL) {
			/* Getvar can have arbitrary side effects, as it may be captured
			 * e.g. by a with(proxy).  So at this point the property we've
			 * looked up above may no longer be mapped, but this doesn't
			 * matter for safety.
			 */
			duk_pop_known(thr);
			(void) duk_js_getvar_envrec(thr, env, varname, 1 /*throw*/); /* -> [ ... value this_binding ] */
			duk_pop_known(thr);
		}
	}

	return rc;
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_idxkey_helper(duk_hthread *thr,
                                                             duk_hobject *target,
                                                             duk_uarridx_t idx,
                                                             duk_bool_t side_effect_safe) {
	duk_small_uint_t htype;
	duk_small_int_t rc;
	duk_idx_t idx_target = 0;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(target != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);

	if (side_effect_safe) {
		idx_target = duk_get_top(thr);
		duk_push_hobject(thr, target);
	}

retry_target:
	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) target);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		if (DUK_HOBJECT_HAS_ARRAY_ITEMS(target)) {
			duk_harray *a = (duk_harray *) target;
			if (idx < DUK_HARRAY_GET_ITEMS_LENGTH(a)) {
				duk_tval *tv_val = DUK_HARRAY_GET_ITEMS(thr->heap, a) + idx;
				if (DUK_TVAL_IS_UNUSED(tv_val)) {
					rc = -1;
					goto return_rc;
				}
				duk_push_tval(thr, tv_val);
				rc = DUK_PROPDESC_FLAGS_WEC;
				goto return_rc;
			} else {
				/* Comprehensiveness: cannot be in property table. */
				rc = -1;
				goto return_rc;
			}
		}
		break;
	case DUK_HTYPE_ARGUMENTS:
		rc = duk__prop_getowndesc_idxkey_arguments(thr, target, idx);
		goto return_rc;
	case DUK_HTYPE_STRING_OBJECT: {
		duk_hstring *h = duk_hobject_lookup_intvalue_hstring(thr, target);
		if (idx < duk_hstring_get_charlen(h)) {
			duk_prop_push_plainstr_idx(thr, h, idx);
			rc = DUK_PROPDESC_FLAGS_E;
			goto return_rc;
		}
		break;
	}
	case DUK_HTYPE_PROXY:
		if (side_effect_safe) {
			duk_hobject *next;

			if (duk_proxy_trap_check_idxkey(thr, (duk_hproxy *) target, idx, DUK_STRIDX_GET_OWN_PROPERTY_DESCRIPTOR)) {
				(void) duk_push_u32_tostring(thr, idx);
				rc = duk__prop_getown_proxy_tail(thr, target);
				goto return_rc;
			}

			DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(target));
			next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) target);
			DUK_ASSERT(next != NULL);
			target = duk_prop_switch_stabilized_target_top(thr, target, next);
			goto retry_target;
		} else {
			goto switch_to_safe;
		}
		break;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
	case DUK_HTYPE_DATAVIEW:
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
		/* All arridx are captured and don't reach OrdinaryGetOwnProperty(). */
		if (duk_hbufobj_validate_and_read_push(thr, (duk_hbufobj *) target, idx)) {
			/* NOTE: Somewhat bizarrely, typed array indices cannot be deleted
			 * but they still must report as configurable.
			 */
			rc = DUK_PROPDESC_FLAGS_WEC;
			goto return_rc;
		}
		rc = -1;
		goto return_rc;
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	rc = duk__prop_getowndesc_idxkey_plain(thr, target, idx);
	/* fall thru */

return_rc:
	if (side_effect_safe) {
		/* The stabilized target is not necessarily at the stack top,
		 * so can't pop the top as for e.g. prop get/set.
		 */
		DUK_ASSERT(duk_get_hobject(thr, idx_target) == target);
		duk_remove(thr, idx_target);
	}
	return rc;

switch_to_safe:
	return duk__prop_getowndesc_idxkey_safe(thr, target, idx);
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_idxkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_getowndesc_idxkey_safe(thr, obj, idx);
#else
	return duk__prop_getowndesc_idxkey_helper(thr, obj, idx, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL duk_small_int_t duk__prop_getowndesc_idxkey_safe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	return duk__prop_getowndesc_idxkey_helper(thr, obj, idx, 1 /*side_effect_safe*/);
}

DUK_INTERNAL duk_small_int_t duk_prop_getowndesc_obj_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
		duk_small_int_t attrs;

		attrs = duk__prop_getowndesc_idxkey_unsafe(thr, obj, duk_hstring_get_arridx_fast_known(key));
		DUK_ASSERT(duk_get_top(thr) == entry_top + duk_prop_propdesc_valcount(attrs));
		return attrs;
	} else {
		duk_small_int_t attrs;

		DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
		attrs = duk__prop_getowndesc_strkey_unsafe(thr, obj, key);
		DUK_ASSERT(duk_get_top(thr) == entry_top + duk_prop_propdesc_valcount(attrs));
		return attrs;
	}
}

DUK_INTERNAL duk_small_int_t duk_prop_getowndesc_obj_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_LIKELY(idx <= DUK_ARRIDX_MAX)) {
		duk_small_int_t attrs = duk__prop_getowndesc_idxkey_unsafe(thr, obj, idx);
		DUK_ASSERT(duk_get_top(thr) == entry_top + duk_prop_propdesc_valcount(attrs));
		return attrs;
	} else {
		duk_small_int_t attrs;
		duk_hstring *key;

		DUK_DD(DUK_DDPRINT("corner case, input idx 0xffffffff is not an arridx, must coerce to string"));

		key = duk_push_u32_tohstring(thr, idx);
		attrs = duk__prop_getowndesc_strkey_unsafe(thr, obj, key);
		duk_pop_known(thr);
		DUK_ASSERT(duk_get_top(thr) == entry_top + duk_prop_propdesc_valcount(attrs));
		return attrs;
	}
}

DUK_INTERNAL duk_small_int_t duk_prop_getowndesc_obj_tvkey(duk_hthread *thr, duk_hobject *obj, duk_tval *tv_key) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_idx_t idx_key;
	duk_small_int_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(tv_key != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	idx_key = duk_get_top(thr);
	duk_push_tval(thr, tv_key);
	attrs = duk_prop_getowndesc_obj_strkey(thr, obj, duk_to_property_key_hstring(thr, -1));
	duk_remove(thr, idx_key); /* Remove stabilized 'tv_key', keep property value or get/set. */
	DUK_ASSERT(duk_get_top(thr) == entry_top + duk_prop_propdesc_valcount(attrs));
	return attrs;
}

DUK_INTERNAL duk_small_int_t duk_prop_getownattr_obj_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_small_int_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	attrs = duk_prop_getowndesc_obj_strkey(thr, obj, key);
	duk_prop_pop_propdesc(thr, attrs);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return attrs;
}

DUK_INTERNAL duk_small_int_t duk_prop_getownattr_obj_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_small_int_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	attrs = duk_prop_getowndesc_obj_idxkey(thr, obj, idx);
	duk_prop_pop_propdesc(thr, attrs);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return attrs;
}

DUK_INTERNAL duk_small_int_t duk_prop_getownattr_obj_tvkey(duk_hthread *thr, duk_hobject *obj, duk_tval *tv_key) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_small_int_t attrs;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(tv_key != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	attrs = duk_prop_getowndesc_obj_tvkey(thr, obj, tv_key);
	duk_prop_pop_propdesc(thr, attrs);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return attrs;
}
