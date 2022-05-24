/*
 *  [[Delete]] and 'delete' operator for properties (not super references).
 *
 *  The [[Delete]] algorithm returns true (success) or false (failure) but
 *  doesn't throw explicitly.  Errors may also be thrown explicitly by e.g.
 *  a Proxy trap.  In non-strict mode the 'delete' operator returns true/false
 *  and in strict mode returns true or throws.  In practice the throwing must
 *  be inlined into the [[Delete]] algorithms to provide good error messages.
 *
 *  Stabilization is needed for Proxies because side effects may revoke some
 *  Proxies in a Proxy chain, stranding the current target object, see
 *  discussion in duk_prop_get.c.
 */

#include "duk_internal.h"

DUK_LOCAL_DECL duk_bool_t duk__prop_delete_obj_strkey_ordinary(duk_hthread *thr,
                                                               duk_hobject *obj,
                                                               duk_hstring *key,
                                                               duk_small_uint_t delprop_flags);
DUK_LOCAL_DECL duk_bool_t duk__prop_delete_obj_idxkey_ordinary(duk_hthread *thr,
                                                               duk_hobject *obj,
                                                               duk_uarridx_t idx,
                                                               duk_small_uint_t delprop_flags);

DUK_LOCAL_DECL duk_bool_t duk__prop_delete_obj_strkey_unsafe(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_hstring *key,
                                                             duk_small_uint_t delprop_flags);
DUK_LOCAL_DECL duk_bool_t duk__prop_delete_obj_strkey_safe(duk_hthread *thr,
                                                           duk_hobject *obj,
                                                           duk_hstring *key,
                                                           duk_small_uint_t delprop_flags);
DUK_LOCAL_DECL duk_bool_t duk__prop_delete_obj_idxkey_unsafe(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_uarridx_t idx,
                                                             duk_small_uint_t delprop_flags);
DUK_LOCAL_DECL duk_bool_t duk__prop_delete_obj_idxkey_safe(duk_hthread *thr,
                                                           duk_hobject *obj,
                                                           duk_uarridx_t idx,
                                                           duk_small_uint_t delprop_flags);

#if defined(DUK_USE_PARANOID_ERRORS)
DUK_LOCAL duk_bool_t duk__prop_delete_error_shared_obj(duk_hthread *thr, duk_hobject *obj, duk_small_uint_t delprop_flags) {
	DUK_UNREF(obj);
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		DUK_ERROR_TYPE(thr, "cannot delete property of object");
	}
	return 0;
}
DUK_LOCAL duk_bool_t duk__prop_delete_error_shared_objidx(duk_hthread *thr, duk_idx_t idx_obj, duk_small_uint_t delprop_flags) {
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		const char *str1 = duk_get_type_name(thr, idx_obj);
		DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "cannot delete property of %s", str1);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_obj_strkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_hstring *key,
                                                                duk_small_uint_t delprop_flags) {
	return duk__prop_delete_error_shared_obj(thr, obj, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_strkey(duk_hthread *thr,
                                                                   duk_idx_t idx_obj,
                                                                   duk_hstring *key,
                                                                   duk_small_uint_t delprop_flags) {
	return duk__prop_delete_error_shared_objidx(thr, idx_obj, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_obj_idxkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uarridx_t idx,
                                                                duk_small_uint_t delprop_flags) {
	return duk__prop_delete_error_shared_obj(thr, obj, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_idxkey(duk_hthread *thr,
                                                                   duk_idx_t idx_obj,
                                                                   duk_uarridx_t idx,
                                                                   duk_small_uint_t delprop_flags) {
	return duk__prop_delete_error_shared_objidx(thr, idx_obj, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_tvkey(duk_hthread *thr,
                                                                  duk_idx_t idx_obj,
                                                                  duk_tval *tv_key,
                                                                  duk_small_uint_t delprop_flags) {
	return duk__prop_delete_error_shared_objidx(thr, idx_obj, delprop_flags);
}
#elif defined(DUK_USE_VERBOSE_ERRORS)
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_obj_strkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_hstring *key,
                                                                duk_small_uint_t delprop_flags) {
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		const char *str1 = duk_push_readable_hobject(thr, obj);
		const char *str2 = duk_push_readable_hstring(thr, key);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot delete property %s of %s", str2, str1);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_strkey(duk_hthread *thr,
                                                                   duk_idx_t idx_obj,
                                                                   duk_hstring *key,
                                                                   duk_small_uint_t delprop_flags) {
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		const char *str1 = duk_push_readable_idx(thr, idx_obj);
		const char *str2 = duk_push_readable_hstring(thr, key);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot delete property %s of %s", str2, str1);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_obj_idxkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uarridx_t idx,
                                                                duk_small_uint_t delprop_flags) {
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		const char *str1 = duk_push_readable_hobject(thr, obj);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot delete property %lu of %s", (unsigned long) idx, str1);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_idxkey(duk_hthread *thr,
                                                                   duk_idx_t idx_obj,
                                                                   duk_uarridx_t idx,
                                                                   duk_small_uint_t delprop_flags) {
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		const char *str1 = duk_push_readable_idx(thr, idx_obj);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot delete property %lu of %s", (unsigned long) idx, str1);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_tvkey(duk_hthread *thr,
                                                                  duk_idx_t idx_obj,
                                                                  duk_tval *tv_key,
                                                                  duk_small_uint_t delprop_flags) {
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		const char *str1 = duk_push_readable_idx(thr, idx_obj);
		const char *str2 = duk_push_readable_tval(thr, tv_key);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot delete property %s of %s", str2, str1);
	}
	return 0;
}
#else
DUK_LOCAL duk_bool_t duk__prop_delete_error_shared(duk_hthread *thr, duk_small_uint_t delprop_flags) {
	if (delprop_flags & DUK_DELPROP_FLAG_THROW) {
		DUK_ERROR_TYPE(thr, "cannot delete property");
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_obj_strkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_hstring *key,
                                                                duk_small_uint_t delprop_flags) {
	DUK_UNREF(obj);
	DUK_UNREF(key);
	return duk__prop_delete_error_shared(thr, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_strkey(duk_hthread *thr,
                                                                   duk_idx_t idx_obj,
                                                                   duk_hstring *key,
                                                                   duk_small_uint_t delprop_flags) {
	DUK_UNREF(idx_obj);
	DUK_UNREF(key);
	return duk__prop_delete_error_shared(thr, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_obj_idxkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uarridx_t idx,
                                                                duk_small_uint_t delprop_flags) {
	DUK_UNREF(obj);
	DUK_UNREF(idx);
	return duk__prop_delete_error_shared(thr, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_idxkey(duk_hthread *thr,
                                                                   duk_idx_t idx_obj,
                                                                   duk_uarridx_t idx,
                                                                   duk_small_uint_t delprop_flags) {
	DUK_UNREF(idx_obj);
	DUK_UNREF(idx);
	return duk__prop_delete_error_shared(thr, delprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_delete_error_objidx_tvkey(duk_hthread *thr,
                                                                  duk_idx_t idx_obj,
                                                                  duk_tval *tv_key,
                                                                  duk_small_uint_t delprop_flags) {
	DUK_UNREF(idx_obj);
	DUK_UNREF(tv_key);
	return duk__prop_delete_error_shared(thr, delprop_flags);
}
#endif /* error model */

DUK_LOCAL void duk__prop_delete_ent_shared(duk_hthread *thr, duk_propvalue *pv_slot, duk_uint8_t attrs) {
	if (DUK_UNLIKELY(attrs & DUK_PROPDESC_FLAG_ACCESSOR)) {
		duk_hobject *tmp;

		tmp = pv_slot->a.get;
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, tmp);
		tmp = pv_slot->a.set;
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, tmp);
	} else {
		duk_tval *tmp;

		tmp = &pv_slot->v;
		DUK_TVAL_DECREF_NORZ(thr, tmp);
	}

	DUK_REFZERO_CHECK_SLOW(thr);

	/* Slot is left as garbage. */
}

DUK_LOCAL void duk__prop_delete_proxy_policy(duk_hthread *thr, duk_hobject *obj, duk_bool_t trap_rc) {
	duk_hobject *target;
	duk_small_int_t attrs;

	if (trap_rc == 0) {
		return;
	}

	/* [ ... key ] */

	target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
	DUK_ASSERT(target != NULL);

	attrs = duk_prop_getowndesc_obj_tvkey(thr, target, duk_get_tval(thr, -1 /*trap key*/));
	target = NULL; /* Potentially invalidated. */

	/* [ ... key value ] OR [ ... key get set ] */

	if (attrs >= 0) {
		duk_small_uint_t uattrs = (duk_small_uint_t) attrs;
		if ((uattrs & DUK_PROPDESC_FLAG_CONFIGURABLE) == 0) {
			/* Non-configurable property in target: reject deletion.  Trap still
			 * gets called (and may remove the property prior to this policy check).
			 */
			goto reject;
		}

		/* Re-get 'target', as previous [[GetOwnProperty]] may have invalidated it,
		 * or even revoked the Proxy.
		 */

		target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
		DUK_ASSERT(target != NULL);

		if (!duk_js_isextensible(thr, target)) {
			/* If property exists in target: reject delection.  Trap still
			 * gets called.
			 */
			goto reject;
		}
	} else {
		/* If target has no 'key', then no restrictions are applied. */
	}

	duk_prop_pop_propdesc(thr, attrs);
	return;

reject:
	DUK_ERROR_TYPE(thr, DUK_STR_PROXY_REJECTED);
}

DUK_LOCAL duk_bool_t duk__prop_delete_proxy_tail(duk_hthread *thr, duk_hobject *obj) {
	duk_bool_t trap_rc;

	DUK_ASSERT(thr != NULL);

	/* [ ... trap handler target key ] */

	duk_dup_top(thr);
	duk_insert(thr, -5); /* Stash key for policy check. */

	/* [ ... key trap handler target key ] */

	duk_call_method(thr, 2); /* [ ... key trap handler target key ] -> [ ... key result ] */

	trap_rc = duk_to_boolean_top_pop(thr);

#if defined(DUK_USE_PROXY_POLICY)
	duk__prop_delete_proxy_policy(thr, obj, trap_rc);
#else
	DUK_DD(DUK_DDPRINT("proxy policy check for 'deleteProperty' trap disabled in configuration"));
#endif

	duk_pop_known(thr);

	return trap_rc;
}

DUK_LOCAL duk_small_int_t duk__prop_delete_obj_strkey_proxy(duk_hthread *thr, duk_hobject *obj, duk_hstring *key) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_PROXY);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	if (duk_proxy_trap_check_strkey(thr, (duk_hproxy *) obj, key, DUK_STRIDX_DELETE_PROPERTY)) {
		duk_push_hstring(thr, key);
		return (duk_small_int_t) duk__prop_delete_proxy_tail(thr, obj);
	} else {
		return -1;
	}
}

DUK_LOCAL duk_small_int_t duk__prop_delete_obj_idxkey_proxy(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_PROXY);
	DUK_ASSERT_ARRIDX_VALID(idx);

	if (duk_proxy_trap_check_idxkey(thr, (duk_hproxy *) obj, idx, DUK_STRIDX_DELETE_PROPERTY)) {
		(void) duk_push_u32_tostring(thr, idx);
		return (duk_small_int_t) duk__prop_delete_proxy_tail(thr, obj);
	} else {
		return -1;
	}
}

DUK_LOCAL duk_bool_t duk__prop_delete_obj_strkey_ordinary(duk_hthread *thr,
                                                          duk_hobject *obj,
                                                          duk_hstring *key,
                                                          duk_small_uint_t delprop_flags) {
	duk_uint_fast32_t ent_idx;
	duk_int_fast32_t hash_idx;

	if (duk_hobject_lookup_strprop_indices(thr, obj, key, &ent_idx, &hash_idx) != 0) {
		duk_propvalue *val_base;
		duk_hstring **key_base;
		duk_uint8_t *attr_base;
		duk_hstring **key_slot;
		duk_propvalue *pv_slot;
		duk_uint8_t attrs;
		duk_uint32_t *hash_base;
		duk_uint32_t *hash_slot;

		attr_base = DUK_HOBJECT_E_GET_FLAGS_BASE(thr->heap, obj);
		attrs = attr_base[ent_idx];

		if (DUK_UNLIKELY(!(attrs & DUK_PROPDESC_FLAG_CONFIGURABLE) && !(delprop_flags & DUK_DELPROP_FLAG_FORCE))) {
			goto fail_not_configurable;
		}

		val_base = DUK_HOBJECT_E_GET_VALUE_BASE(thr->heap, obj);
		pv_slot = val_base + ent_idx;

		key_base = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, obj);
		key_slot = key_base + ent_idx;
		DUK_ASSERT(*key_slot != NULL);
		DUK_ASSERT(*key_slot == key);

		DUK_HSTRING_DECREF_NORZ(thr, key);
		*key_slot = NULL;

#if defined(DUK_USE_HOBJECT_HASH_PART)
		if (hash_idx >= 0) {
			hash_base = DUK_HOBJECT_GET_HASH(thr->heap, obj);
			DUK_ASSERT(hash_base != NULL);
			DUK_ASSERT((duk_uint_fast32_t) hash_idx < (duk_uint_fast32_t) hash_base[0]);
			hash_slot = hash_base + 1 + hash_idx;
			*hash_slot = DUK_HOBJECT_HASHIDX_DELETED;
		}
#else
		DUK_UNREF(hash_idx);
#endif

		/* Attrs are left as garbage. */

		duk__prop_delete_ent_shared(thr, pv_slot, attrs);
	}
	return 1;

fail_not_configurable:
	return duk__prop_delete_error_obj_strkey(thr, obj, key, delprop_flags);
}

/* [[Delete]] for duk_hobject, with string key. */
DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__prop_delete_obj_strkey_helper(duk_hthread *thr,
                                                                          duk_hobject *obj,
                                                                          duk_hstring *key,
                                                                          duk_small_uint_t delprop_flags,
                                                                          duk_bool_t side_effect_safe) {
	duk_small_uint_t htype;
	duk_hobject *target;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	target = obj;
	if (side_effect_safe) {
		duk_push_hobject(thr, target);
	}

retry_target:
	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) target);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		/* Array has no special [[Delete]] operation, but we must
		 * handle the 'length' property here because it is stored
		 * outside of strprops.
		 */
		if (DUK_UNLIKELY(DUK_HSTRING_HAS_LENGTH(key))) {
			goto fail_not_configurable;
		}
		break;
	case DUK_HTYPE_ARGUMENTS:
		/* For arguments objects, 'length' is tracked as an ordinary
		 * property (and it is, by default, configurable) and no mapped
		 * indices come here.  The Arguments [[Delete]] simplifies to
		 * OrdinaryDelete() here.
		 */
		break;
	case DUK_HTYPE_PROXY: {
		/* Unlike get/set, delete doesn't follow the inheritance chain.
		 * There may be a Proxy chain however, and Proxy trap lookups
		 * can have arbitrary side effects, including stranding the
		 * current 'target' by revoking Proxies in the chain.  Current
		 * 'target' is stabilized on the value stack.
		 *
		 *   P1 --> P2 --> P3 --> target
		 *
		 * Above P1 is the original 'obj'.  Suppose we've progressed
		 * to P3, and a side effect of looking up the P3 proxy trap
		 * revokes proxy P1, i.e. NULLs the 'target' pointer of P1.
		 *
		 *   P1 -X> P2 --> P3 --> target
		 *          `------------------'
		 *           potentially unreachable without stabilization
		 *
		 * This may cause P2, P3, and target to be garbage collected.
		 */
		if (side_effect_safe) {
			duk_small_int_t rc = duk__prop_delete_obj_strkey_proxy(thr, target, key);
			DUK_ASSERT(rc == 0 || rc == 1 || rc == -1);
			if (rc >= 0) {
				if (rc) {
					goto success;
				}
				goto fail_proxy;
			} else {
				duk_hobject *next;

				next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) target);
				DUK_ASSERT(next != NULL);

				target = duk_prop_switch_stabilized_target_top(thr, target, next);
				goto retry_target;
			}
		} else {
			goto switch_to_safe;
		}
	}
	case DUK_HTYPE_STRING_OBJECT:
		if (DUK_UNLIKELY(DUK_HSTRING_HAS_LENGTH(key))) {
			goto fail_not_configurable;
		}
		break;
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
		break;
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
		if (DUK_HSTRING_HAS_LENGTH_OR_CANNUM(key)) {
			if (DUK_HSTRING_HAS_LENGTH(key)) {
				/* Non-standard behavior: because we present a virtual
				 * non-configurable .length property, fail the delete.
				 */
				goto fail_not_configurable;
			} else {
				/* CanonicalNumericIndexString (not arridx); no exotic
				 * behavior but standard OrdinaryDelete() calls
				 * [[GetOwnProperty]] which is special and short
				 * circuits all CanonicalNumericIndexStrings.  Here
				 * the CanonicalNumericIndexString is always out of
				 * bounds, so [[GetOwnProperty]] returns undefined
				 * (no descriptor), which causes OrdinaryDelete() to
				 * return true (= success) with no action.
				 */
				DUK_ASSERT(DUK_HSTRING_HAS_CANNUM(key));
				goto success;
			}
		}
		break;
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	/* No virtual properties preventing delete, look up concrete
	 * property table.
	 */
	if (side_effect_safe) {
		duk_bool_t del_rc = duk__prop_delete_obj_strkey_ordinary(thr, target, key, delprop_flags);
		duk_pop_known(thr);
		return del_rc;
	} else {
		return duk__prop_delete_obj_strkey_ordinary(thr, target, key, delprop_flags);
	}

success:
	if (side_effect_safe) {
		duk_pop_known(thr);
	}
	return 1;

fail_not_configurable:
fail_proxy:
	if (side_effect_safe) {
		duk_pop_known(thr);
	}
	return duk__prop_delete_error_obj_strkey(thr, target, key, delprop_flags);

switch_to_safe:
	return duk__prop_delete_obj_strkey_safe(thr, obj, key, delprop_flags);
}

DUK_LOCAL duk_bool_t duk__prop_delete_obj_strkey_unsafe(duk_hthread *thr,
                                                        duk_hobject *obj,
                                                        duk_hstring *key,
                                                        duk_small_uint_t delprop_flags) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_delete_obj_strkey_safe(thr, obj, key, delprop_flags);
#else
	return duk__prop_delete_obj_strkey_helper(thr, obj, key, delprop_flags, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL DUK_NOINLINE duk_bool_t duk__prop_delete_obj_strkey_safe(duk_hthread *thr,
                                                                   duk_hobject *obj,
                                                                   duk_hstring *key,
                                                                   duk_small_uint_t delprop_flags) {
	return duk__prop_delete_obj_strkey_helper(thr, obj, key, delprop_flags, 1 /*side_effect_safe*/);
}

DUK_LOCAL duk_bool_t duk__prop_delete_obj_idxkey_array(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx) {
	duk_harray *a = (duk_harray *) obj;

	/* Delete from array items part never causes it to be abandoned
	 * at present, which allows one to craft an Array which is very
	 * sparse but keeps its array items.
	 */
	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
		if (idx < DUK_HARRAY_GET_LENGTH(a)) {
			if (idx < DUK_HARRAY_GET_ITEMS_LENGTH(a)) {
				duk_tval *tv = DUK_HARRAY_GET_ITEMS(thr->heap, a) + idx;
				DUK_TVAL_SET_UNUSED_UPDREF(thr, tv);
				return 1;
			} else {
				/* Technically within .length, but no
				 * concrete allocation, so non-existent.
				 */
				return 1;
			}
		} else {
			/* No index keys should be elsewhere. */
			return 1;
		}
	}

	DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	return 0;
}

DUK_LOCAL duk_bool_t duk__prop_delete_obj_idxkey_arguments(duk_hthread *thr,
                                                           duk_hobject *obj,
                                                           duk_uarridx_t idx,
                                                           duk_small_uint_t delprop_flags) {
	duk_harray *a = (duk_harray *) obj;
	duk_hobject *map;
	duk_hobject *env;
	duk_hstring *varname;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) obj) == DUK_HTYPE_ARGUMENTS);

	/* Conceptual algorithm:
	 *   1. Check if mapped.
	 *   2. Ordinary delete (arbitrary side effects).
	 *   3. If was mapped in 1, and ordinary delete succeeds, delete from map.
	 */

	varname = duk_prop_arguments_map_prep_idxkey(thr, obj, idx, &map, &env);

	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
		if (idx < DUK_HARRAY_GET_ITEMS_LENGTH(a)) {
			duk_tval *tv = DUK_HARRAY_GET_ITEMS(thr->heap, a) + idx;
			DUK_TVAL_SET_UNUSED_UPDREF(thr, tv);
		} else {
			/* No index keys should be elsewhere. */
		}
		/* Always succeeds.  As for arrays, never abandoned here. */
	} else {
		duk_bool_t del_rc = duk__prop_delete_obj_idxkey_ordinary(thr, obj, idx, delprop_flags);
		if (del_rc == 0) {
			return 0;
		}
	}
	DUK_GC_TORTURE(thr->heap);

	/* Ordinary delete successful, delete from map if was mapped
	 * in the beginning.  Map delete should always succeed; its
	 * retval is ignored.
	 *
	 * NOTE: 'varname' may be a dangling reference at this point
	 * so any dereference may be invalid.  But we only check its
	 * previous value for NULL here ("varname was non-NULL earlier").
	 */
	DUK_GC_TORTURE(thr->heap);
	if (varname != NULL) {
		(void) duk__prop_delete_obj_idxkey_ordinary(thr, map, idx, 0 /*delprop_flags*/);
	}
	return 1;
}

DUK_LOCAL duk_bool_t duk__prop_delete_obj_idxkey_ordinary(duk_hthread *thr,
                                                          duk_hobject *obj,
                                                          duk_uarridx_t idx,
                                                          duk_small_uint_t delprop_flags) {
	duk_uint_fast32_t ent_idx;
	duk_int_fast32_t hash_idx;

	if (duk_hobject_lookup_idxprop_indices(thr, obj, idx, &ent_idx, &hash_idx) != 0) {
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;
		duk_uint8_t *attr_base;
		duk_uarridx_t *key_slot;
		duk_propvalue *pv_slot;
		duk_uint8_t attrs;
		duk_uint32_t *hash_base;
		duk_uint32_t *hash_slot;

		val_base = (duk_propvalue *) (void *) obj->idx_props;
		key_base = (duk_uarridx_t *) (void *) (val_base + obj->i_size);
		attr_base = (duk_uint8_t *) (void *) (key_base + obj->i_size);

		attrs = attr_base[ent_idx];
		if (DUK_UNLIKELY(!(attrs & DUK_PROPDESC_FLAG_CONFIGURABLE) && !(delprop_flags & DUK_DELPROP_FLAG_FORCE))) {
			goto fail_not_configurable;
		}

		key_slot = key_base + ent_idx;
		DUK_ASSERT(*key_slot == idx);
		*key_slot = DUK_ARRIDX_NONE;
		/* Attrs are left as garbage. */

		pv_slot = val_base + ent_idx;

		if (hash_idx >= 0) {
			hash_base = obj->idx_hash;
			DUK_ASSERT(hash_base != NULL);
			DUK_ASSERT((duk_uint_fast32_t) hash_idx < (duk_uint_fast32_t) hash_base[0]);
			hash_slot = hash_base + 1 + hash_idx;
			*hash_slot = DUK_HOBJECT_HASHIDX_DELETED;
		}

		duk__prop_delete_ent_shared(thr, pv_slot, attrs);
	}

	return 1;

fail_not_configurable:
	return duk__prop_delete_error_obj_idxkey(thr, obj, idx, delprop_flags);
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__prop_delete_obj_idxkey_helper(duk_hthread *thr,
                                                                          duk_hobject *obj,
                                                                          duk_uarridx_t idx,
                                                                          duk_small_uint_t delprop_flags,
                                                                          duk_bool_t side_effect_safe) {
	duk_small_uint_t htype;
	duk_hobject *target;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(idx != DUK_ARRIDX_NONE);

	target = obj;
	if (side_effect_safe) {
		duk_push_hobject(thr, target);
	}

retry_target:
	/* Check HTYPE specific properties first.  Some special properties
	 * cannot be deleted, even with force flag.
	 */
	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) target);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		if (duk__prop_delete_obj_idxkey_array(thr, target, idx)) {
			goto success;
		}
		/* Continue to index properties if abandoned. */
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(target));
		break;
	case DUK_HTYPE_ARGUMENTS:
		/* Argument object indices are stored either in array items
		 * or index part.  There's an exotic [[Delete]] algorithm
		 * which first runs OrdinaryDelete(); if it succeeds, the
		 * arguments map is checked and updated.
		 */
		return duk__prop_delete_obj_idxkey_arguments(thr, target, idx, delprop_flags);
	case DUK_HTYPE_PROXY: {
		/* Proxy chains may be revoked by side effects so we
		 * must stabilize the current 'target' once side effects
		 * become possible.
		 */
		if (side_effect_safe) {
			duk_small_int_t rc = duk__prop_delete_obj_idxkey_proxy(thr, target, idx);
			DUK_ASSERT(rc == 0 || rc == 1 || rc == -1);
			if (rc >= 0) {
				if (rc) {
					goto success;
				}
				goto fail_proxy;
			} else {
				duk_hobject *next;

				next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) target);
				DUK_ASSERT(next != NULL);

				target = duk_prop_switch_stabilized_target_top(thr, target, next);
				goto retry_target;
			}
		} else {
			goto switch_to_safe;
		}
	}
	case DUK_HTYPE_STRING_OBJECT: {
		duk_hstring *h;

		h = duk_hobject_lookup_intvalue_hstring(thr, target);
		if (DUK_LIKELY(h != NULL)) {
			if (DUK_LIKELY(!DUK_HSTRING_HAS_SYMBOL(h) && idx < duk_hstring_get_charlen(h))) {
				goto fail_not_configurable;
			}
		}
		break;
	}
#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	case DUK_HTYPE_ARRAYBUFFER:
		break;
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
	case DUK_HTYPE_FLOAT64ARRAY: {
		duk_hbufobj *h = (duk_hbufobj *) target;
		duk_uint8_t *data;

		data = duk_hbufobj_get_validated_data_ptr(thr, h, idx);
		if (data != NULL) {
			goto fail_not_configurable;
		} else {
			/* Any canonical numeric index string outside of
			 * length is treated as missing ([[GetOwnProperty]]
			 * returns undefined), with no concrete lookup.
			 */
			goto success;
		}
		break;
	}
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	/* No virtual properties preventing delete, look up concrete
	 * property table.
	 */

	if (side_effect_safe) {
		duk_bool_t del_rc = duk__prop_delete_obj_idxkey_ordinary(thr, target, idx, delprop_flags);
		duk_pop_known(thr);
		return del_rc;
	} else {
		return duk__prop_delete_obj_idxkey_ordinary(thr, target, idx, delprop_flags);
	}

success:
	if (side_effect_safe) {
		duk_pop_known(thr);
	}
	return 1;

fail_not_configurable:
fail_proxy:
	if (side_effect_safe) {
		duk_pop_known(thr);
	}
	return duk__prop_delete_error_obj_idxkey(thr, target, idx, delprop_flags);

switch_to_safe:
	return duk__prop_delete_obj_idxkey_safe(thr, obj, idx, delprop_flags);
}

DUK_LOCAL duk_bool_t duk__prop_delete_obj_idxkey_unsafe(duk_hthread *thr,
                                                        duk_hobject *obj,
                                                        duk_uarridx_t idx,
                                                        duk_small_uint_t delprop_flags) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_delete_obj_idxkey_safe(thr, obj, idx, delprop_flags);
#else
	return duk__prop_delete_obj_idxkey_helper(thr, obj, idx, delprop_flags, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL DUK_NOINLINE duk_bool_t duk__prop_delete_obj_idxkey_safe(duk_hthread *thr,
                                                                   duk_hobject *obj,
                                                                   duk_uarridx_t idx,
                                                                   duk_small_uint_t delprop_flags) {
	return duk__prop_delete_obj_idxkey_helper(thr, obj, idx, delprop_flags, 1 /*side_effect_safe*/);
}

DUK_INTERNAL duk_bool_t duk_prop_delete_obj_strkey(duk_hthread *thr,
                                                   duk_hobject *obj,
                                                   duk_hstring *key,
                                                   duk_small_uint_t delprop_flags) {
	if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
		return duk__prop_delete_obj_idxkey_unsafe(thr, obj, duk_hstring_get_arridx_fast_known(key), delprop_flags);
	} else {
		return duk__prop_delete_obj_strkey_unsafe(thr, obj, key, delprop_flags);
	}
}

DUK_INTERNAL duk_bool_t duk_prop_delete_obj_idxkey(duk_hthread *thr,
                                                   duk_hobject *obj,
                                                   duk_uarridx_t idx,
                                                   duk_small_uint_t delprop_flags) {
	if (DUK_LIKELY(idx <= DUK_ARRIDX_MAX)) {
		return duk__prop_delete_obj_idxkey_unsafe(thr, obj, idx, delprop_flags);
	} else {
		duk_bool_t rc;
		duk_hstring *key;

		DUK_DD(DUK_DDPRINT("corner case, input idx 0xffffffff is not an arridx, must coerce to string"));
		key = duk_push_u32_tohstring(thr, idx);
		rc = duk__prop_delete_obj_strkey_unsafe(thr, obj, key, delprop_flags);
		duk_pop_known(thr);
		return rc;
	}
}

DUK_LOCAL duk_bool_t duk__prop_delete_strkey(duk_hthread *thr,
                                             duk_idx_t idx_obj,
                                             duk_hstring *key,
                                             duk_small_uint_t delprop_flags) {
	duk_tval *tv_obj;

	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	tv_obj = thr->valstack_bottom + idx_obj;

	/* The 'delete' operator conceptually performs a ToObject() and then
	 * runs the normal [[Delete]].  No actual property is deleted if the
	 * delete argument is a primitive value, but false/throw may be needed.
	 */

	switch (DUK_TVAL_GET_TAG(tv_obj)) {
	case DUK_TAG_UNUSED:
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
		/* Conceptually this error happens in ToObject() coercion of
		 * the delete operator algorithm, so it's unconditional.
		 */
		goto fail_invalid_base_uncond;
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv_obj);

		if (!DUK_HSTRING_HAS_SYMBOL(h) && DUK_HSTRING_HAS_LENGTH(key)) {
			goto fail_not_configurable;
		}
		break;
	}
	case DUK_TAG_OBJECT:
		/* Typical case. */
		return duk__prop_delete_obj_strkey_unsafe(thr, DUK_TVAL_GET_OBJECT(tv_obj), key, delprop_flags);
	case DUK_TAG_BUFFER:
		/* Mimic an actual Uint8Array object, for which we present a
		 * non-standard own .length property.
		 */
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			goto fail_not_configurable;
		}
		break;
	case DUK_TAG_BOOLEAN:
		break;
	case DUK_TAG_POINTER:
		break;
	case DUK_TAG_LIGHTFUNC:
		break;
	default:
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_obj));
	}

	/* No property found, success. */
	return 1;

fail_not_configurable:
	return duk__prop_delete_error_objidx_strkey(thr, idx_obj, key, delprop_flags);
fail_invalid_base_uncond:
	return duk__prop_delete_error_objidx_strkey(thr, idx_obj, key, DUK_DELPROP_FLAG_THROW);
}

DUK_LOCAL duk_bool_t duk__prop_delete_idxkey(duk_hthread *thr,
                                             duk_idx_t idx_obj,
                                             duk_uarridx_t idx,
                                             duk_small_uint_t delprop_flags) {
	duk_tval *tv_obj;

	tv_obj = thr->valstack_bottom + idx_obj;

	switch (DUK_TVAL_GET_TAG(tv_obj)) {
	case DUK_TAG_UNUSED:
	case DUK_TAG_UNDEFINED:
	case DUK_TAG_NULL:
		/* Conceptually this error happens in ToObject() coercion of
		 * the delete operator algorithm, so it's unconditional.
		 */
		goto fail_invalid_base_uncond;
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv_obj);

		if (!DUK_HSTRING_HAS_SYMBOL(h) && idx < duk_hstring_get_charlen(h)) {
			goto fail_not_configurable;
		}
		break;
	}
	case DUK_TAG_OBJECT:
		/* Typical case. */
		return duk__prop_delete_obj_idxkey_unsafe(thr, DUK_TVAL_GET_OBJECT(tv_obj), idx, delprop_flags);
	case DUK_TAG_BUFFER: {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv_obj);
		if (idx < DUK_HBUFFER_GET_SIZE(h)) {
			goto fail_not_configurable;
		}
		break;
	}
	case DUK_TAG_BOOLEAN:
		break;
	case DUK_TAG_POINTER:
		break;
	case DUK_TAG_LIGHTFUNC:
		break;
	default:
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_obj));
	}

	/* No property found, success. */
	return 1;

fail_not_configurable:
	return duk__prop_delete_error_objidx_idxkey(thr, idx_obj, idx, delprop_flags);
fail_invalid_base_uncond:
	return duk__prop_delete_error_objidx_idxkey(thr, idx_obj, idx, DUK_DELPROP_FLAG_THROW);
}

DUK_INTERNAL duk_bool_t duk_prop_delete_strkey(duk_hthread *thr,
                                               duk_idx_t idx_obj,
                                               duk_hstring *key,
                                               duk_small_uint_t delprop_flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_obj));
	DUK_ASSERT(key != NULL);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
		duk_bool_t rc;

		rc = duk__prop_delete_idxkey(thr, idx_obj, duk_hstring_get_arridx_fast_known(key), delprop_flags);
		DUK_ASSERT(duk_get_top(thr) == rc);
		return rc;
	} else {
		duk_bool_t rc;

		DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
		rc = duk__prop_delete_strkey(thr, idx_obj, key, delprop_flags);
		DUK_ASSERT(duk_get_top(thr) == rc);
		return rc;
	}
}

DUK_INTERNAL duk_bool_t duk_prop_delete_idxkey(duk_hthread *thr,
                                               duk_idx_t idx_obj,
                                               duk_uarridx_t idx,
                                               duk_small_uint_t delprop_flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_obj));
	DUK_ASSERT_ARRIDX_VALID(idx);
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_LIKELY(idx <= DUK_ARRIDX_MAX)) {
		duk_bool_t rc = duk__prop_delete_idxkey(thr, idx_obj, idx, delprop_flags);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	} else {
		duk_bool_t rc;
		duk_hstring *key;

		DUK_DD(DUK_DDPRINT("corner case, input idx 0xffffffff is not an arridx, must coerce to string"));
		key = duk_push_u32_tohstring(thr, idx);
		rc = duk__prop_delete_strkey(thr, idx_obj, key, delprop_flags);
		duk_pop_known(thr);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	}
}

DUK_INTERNAL duk_bool_t duk_prop_deleteoper(duk_hthread *thr, duk_idx_t idx_obj, duk_tval *tv_key, duk_small_uint_t delprop_flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_bool_t rc;
	duk_hstring *key;
	duk_uarridx_t idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_obj));
	DUK_ASSERT(tv_key != NULL);
	/* 'tv_key' is not necessarily in value stack (may be a const).  It
	 * must remain reachable despite side effects, but the 'tv_key' pointer
	 * itself may be unstable (e.g. in value stack).
	 */
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

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
		duk_double_t d;
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv_key));
		d = DUK_TVAL_GET_DOUBLE(tv_key);
		if (duk_prop_double_idx_check(d, &idx)) {
			goto use_idx;
		}
#endif
		break;
	}
	}

	/* For undefined and null, 'delete' operator first does a ToObject()
	 * which causes a TypeError.  This must happen before any side effects
	 * caused by key coercion.
	 */

	if (DUK_UNLIKELY(duk_is_nullish(thr, idx_obj))) {
		return duk__prop_delete_error_objidx_tvkey(thr, idx_obj, tv_key, DUK_DELPROP_FLAG_THROW);
	}

	duk_push_tval(thr, tv_key);
	tv_key = NULL;
	key = duk_to_property_key_hstring(thr, -1);
	DUK_ASSERT(key != NULL);
	rc = duk_prop_delete_strkey(thr, idx_obj, key, delprop_flags);
	duk_pop_known(thr);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;

use_idx:
	DUK_ASSERT_ARRIDX_VALID(idx);
	rc = duk__prop_delete_idxkey(thr, idx_obj, idx, delprop_flags);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;

use_str:
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	rc = duk__prop_delete_strkey(thr, idx_obj, key, delprop_flags);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;
}
