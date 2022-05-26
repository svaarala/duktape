/*
 *  [[DefineOwnProperty]]
 *
 *  Special handling is needed when an object has exotic [[DefineOwnProperty]]
 *  or when a property is stored internally in a special way.  Special storage
 *  may also mean that only certain attribute configurations are possible.
 *
 *  Force flag is a custom feature for the C API which should enable any
 *  property attribute change to be made except when prevented by internal
 *  limitations (e.g. for Array 'length' only writability attribute can be
 *  controlled).
 *
 *  [[DefineOwnProperty]] is different from most other property operations
 *  in that it deals with potentially partial descriptors, i.e. a descriptor
 *  may contain a certain boolean or be missing the key entirely.
 *
 *  Performance matters somewhat (but not as much as for [[Get]] and [[Set]])
 *  because [[DefineOwnProperty]] is used in many internal algorithms.
 *
 *  Stabilization is required for Proxy chains (Proxy revocations can strand
 *  the current object), and for Arguments.
 */

#include "duk_internal.h"

DUK_LOCAL_DECL duk_bool_t
duk__prop_defown_strkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_desc, duk_uint_t defprop_flags);
DUK_LOCAL_DECL duk_bool_t
duk__prop_defown_strkey_safe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_desc, duk_uint_t defprop_flags);

DUK_LOCAL_DECL duk_bool_t
duk__prop_defown_idxkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_desc, duk_uint_t defprop_flags);
DUK_LOCAL_DECL duk_bool_t
duk__prop_defown_idxkey_safe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_desc, duk_uint_t defprop_flags);

#if defined(DUK_USE_PARANOID_ERRORS)
DUK_LOCAL duk_bool_t duk__prop_defown_error_shared(duk_hthread *thr, duk_uint_t defprop_flags) {
	if (defprop_flags & DUK_DEFPROP_THROW) {
		DUK_ERROR_TYPE(thr, "cannot (re)define property of object");
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_error_obj_idxkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uarridx_t idx,
                                                                duk_uint_t defprop_flags) {
	DUK_UNREF(obj);
	DUK_UNREF(idx);
	return duk__prop_defown_error_shared(thr, defprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_error_obj_strkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_hstring *key,
                                                                duk_uint_t defprop_flags) {
	DUK_UNREF(obj);
	DUK_UNREF(key);

	return duk__prop_defown_error_shared(thr, defprop_flags);
}
#elif defined(DUK_USE_VERBOSE_ERRORS)
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_error_obj_idxkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uarridx_t idx,
                                                                duk_uint_t defprop_flags) {
	if (defprop_flags & DUK_DEFPROP_THROW) {
		const char *str1 = duk_push_readable_hobject(thr, obj);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot (re)define property %lu of %s", (unsigned long) idx, str1);
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_error_obj_strkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_hstring *key,
                                                                duk_uint_t defprop_flags) {
	if (defprop_flags & DUK_DEFPROP_THROW) {
		const char *str1 = duk_push_readable_hobject(thr, obj);
		const char *str2 = duk_push_readable_hstring(thr, key);
		DUK_ERROR_FMT2(thr, DUK_ERR_TYPE_ERROR, "cannot (re)define property %s of %s", str2, str1);
	}
	return 0;
}
#else
DUK_LOCAL duk_bool_t duk__prop_defown_error_shared(duk_hthread *thr, duk_uint_t defprop_flags) {
	if (defprop_flags & DUK_DEFPROP_THROW) {
		DUK_ERROR_TYPE(thr, "cannot (re)define property of object");
	}
	return 0;
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_error_obj_idxkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_uarridx_t idx,
                                                                duk_uint_t defprop_flags) {
	DUK_UNREF(obj);
	DUK_UNREF(idx);
	return duk__prop_defown_error_shared(thr, defprop_flags);
}
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_error_obj_strkey(duk_hthread *thr,
                                                                duk_hobject *obj,
                                                                duk_hstring *key,
                                                                duk_uint_t defprop_flags) {
	DUK_UNREF(obj);
	DUK_UNREF(key);
	return duk__prop_defown_error_shared(thr, defprop_flags);
}
#endif /* error model */

DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__prop_is_accessor_descriptor(duk_uint_t defprop_flags) {
	duk_bool_t rc = ((defprop_flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER)) != 0U);
	return rc;
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_bool_t duk__prop_is_data_descriptor(duk_uint_t defprop_flags) {
	duk_bool_t rc = ((defprop_flags & (DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WRITABLE)) != 0U);
	return rc;
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_idx_t duk__prop_defown_getter_index(duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	DUK_UNREF(defprop_flags);
	return idx_desc;
}

DUK_LOCAL DUK_ALWAYS_INLINE duk_idx_t duk__prop_defown_setter_index(duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	return (defprop_flags & DUK_DEFPROP_HAVE_GETTER) ? idx_desc + 1 : idx_desc;
}

/* For a set of immutable attributes of a data property, check whether defprop_flags
 * is trying to (1) change any of the immutable attributes or (2) trying to make the
 * property an accessor.  Used to check [[DefineOwnProperty]] handling of virtualized
 * properties where full attribute control is not possible.
 */
DUK_LOCAL duk_bool_t duk__prop_validate_immutable_data_desc(duk_uint_t immutable_flags, duk_uint_t defprop_flags) {
	duk_uint_t test_mask;
	duk_uint_t t;

	DUK_ASSERT((immutable_flags & DUK_PROPDESC_FLAG_ACCESSOR) == 0U);

	DUK_ASSERT((DUK_DEFPROP_HAVE_WRITABLE >> DUK_DEFPROP_HAVE_SHIFT_COUNT) == DUK_DEFPROP_WRITABLE);
	DUK_ASSERT((DUK_DEFPROP_HAVE_ENUMERABLE >> DUK_DEFPROP_HAVE_SHIFT_COUNT) == DUK_DEFPROP_ENUMERABLE);
	DUK_ASSERT((DUK_DEFPROP_HAVE_CONFIGURABLE >> DUK_DEFPROP_HAVE_SHIFT_COUNT) == DUK_DEFPROP_CONFIGURABLE);

	test_mask = ((defprop_flags & DUK_DEFPROP_HAVE_WEC) >> DUK_DEFPROP_HAVE_SHIFT_COUNT) |
	            (DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_GETTER);
	t = (defprop_flags ^ immutable_flags) & test_mask;
	if (t != 0U) {
		/* Some WEC flag differs in the 'have flag' set, i.e. we're
		 * trying to modify the flag, or get/set is present.
		 */
		return 0;
	}

	return 1;
}

/* Create new accessor or data property.  Missing attributes default to false,
 * so we can just ignore the HAVE mask (any attribute with no HAVE bit MUST
 * be zero).
 */
DUK_LOCAL duk_bool_t duk__prop_defown_write_new_slot(duk_hthread *thr,
                                                     duk_idx_t idx_desc,
                                                     duk_uint_t defprop_flags,
                                                     duk_propvalue *pv_slot,
                                                     duk_uint8_t *attr_slot) {
	if (DUK_UNLIKELY(defprop_flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER))) {
		*attr_slot = (defprop_flags & DUK_DEFPROP_EC) | DUK_PROPDESC_FLAG_ACCESSOR;
		if (defprop_flags & DUK_DEFPROP_HAVE_GETTER) {
			pv_slot->a.get = duk_get_hobject(thr, duk__prop_defown_getter_index(idx_desc, defprop_flags));
		} else {
			pv_slot->a.get = NULL;
		}
		if (defprop_flags & DUK_DEFPROP_HAVE_SETTER) {
			pv_slot->a.set = duk_get_hobject(thr, duk__prop_defown_setter_index(idx_desc, defprop_flags));
		} else {
			pv_slot->a.set = NULL;
		}
		DUK_HOBJECT_INCREF_ALLOWNULL(thr, pv_slot->a.get);
		DUK_HOBJECT_INCREF_ALLOWNULL(thr, pv_slot->a.set);
	} else {
		/* Default attributes are 'false', overwrite based on
		 * DUK_DEFPROP_HAVE_xxx flags.
		 */
		duk_uint8_t new_attrs;

		DUK_ASSERT((DUK_DEFPROP_WRITABLE << DUK_DEFPROP_HAVE_SHIFT_COUNT) == DUK_DEFPROP_HAVE_WRITABLE);
		DUK_ASSERT((DUK_DEFPROP_ENUMERABLE << DUK_DEFPROP_HAVE_SHIFT_COUNT) == DUK_DEFPROP_HAVE_ENUMERABLE);
		DUK_ASSERT((DUK_DEFPROP_CONFIGURABLE << DUK_DEFPROP_HAVE_SHIFT_COUNT) == DUK_DEFPROP_HAVE_CONFIGURABLE);
		DUK_ASSERT((DUK_DEFPROP_WEC & 0x07U) == DUK_DEFPROP_WEC);

		new_attrs = (duk_uint8_t) ((defprop_flags & (defprop_flags >> DUK_DEFPROP_HAVE_SHIFT_COUNT)) & DUK_DEFPROP_WEC);
		*attr_slot = new_attrs;
		if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
			duk_tval *tv_src = duk_require_tval(thr, idx_desc);
			DUK_TVAL_SET_TVAL_INCREF(thr, &pv_slot->v, tv_src);
		} else {
			DUK_TVAL_SET_UNDEFINED(&pv_slot->v);
		}
	}

	return 1;
}

/* Convert an existing data property into an accessor. */
DUK_LOCAL duk_bool_t duk__prop_defown_update_convert_to_accessor(duk_hthread *thr,
                                                                 duk_idx_t idx_desc,
                                                                 duk_uint_t defprop_flags,
                                                                 duk_propvalue *pv_slot,
                                                                 duk_uint8_t *attr_slot,
                                                                 duk_uint8_t attrs,
                                                                 duk_uint8_t curr_configurable,
                                                                 duk_uint_t have_shifted) {
	duk_tval tv_old;
	duk_hobject *new_get = NULL;
	duk_hobject *new_set = NULL;

	if (defprop_flags & DUK_DEFPROP_HAVE_GETTER) {
		new_get = duk_get_hobject(thr, duk__prop_defown_getter_index(idx_desc, defprop_flags));
	}
	if (defprop_flags & DUK_DEFPROP_HAVE_SETTER) {
		new_set = duk_get_hobject(thr, duk__prop_defown_setter_index(idx_desc, defprop_flags));
	}

	if (DUK_UNLIKELY(!curr_configurable && !(defprop_flags & DUK_DEFPROP_FORCE))) {
		goto fail_not_configurable;
	}

	have_shifted &= DUK_DEFPROP_EC; /* Restrict HAVE flags to EC. */
	attrs &= DUK_PROPDESC_FLAGS_EC; /* Keep EC, zero rest. */
	attrs &= ~((duk_uint8_t) have_shifted); /* Zero anything provided. */
	attrs |= (duk_uint8_t) (defprop_flags & have_shifted); /* Set anything provided. */
	attrs |= DUK_PROPDESC_FLAG_ACCESSOR;
	*attr_slot = attrs;
	DUK_TVAL_SET_TVAL(&tv_old, &pv_slot->v);
	pv_slot->a.get = new_get;
	pv_slot->a.set = new_set;
	DUK_HOBJECT_INCREF_ALLOWNULL(thr, new_get);
	DUK_HOBJECT_INCREF_ALLOWNULL(thr, new_set);
	DUK_TVAL_DECREF_NORZ(thr, &tv_old);
	DUK_REFZERO_CHECK_SLOW(thr);
	return 1;

fail_not_configurable:
	return 0;
}

/* Convert an existing accessor property to a data property. */
DUK_LOCAL duk_bool_t duk__prop_defown_update_convert_to_data(duk_hthread *thr,
                                                             duk_idx_t idx_desc,
                                                             duk_uint_t defprop_flags,
                                                             duk_propvalue *pv_slot,
                                                             duk_uint8_t *attr_slot,
                                                             duk_uint8_t attrs,
                                                             duk_uint8_t curr_configurable,
                                                             duk_uint_t have_shifted) {
	duk_hobject *old_get;
	duk_hobject *old_set;

	if (DUK_UNLIKELY(!curr_configurable && !(defprop_flags & DUK_DEFPROP_FORCE))) {
		goto fail_not_configurable;
	}

	have_shifted &= DUK_DEFPROP_WEC; /* Restrict HAVE flags to WEC. */
	attrs &= DUK_PROPDESC_FLAGS_EC; /* Keep EC, zero rest. */
	attrs &= ~((duk_uint8_t) have_shifted); /* Zero anything provided. */
	attrs |= (duk_uint8_t) (defprop_flags & have_shifted); /* Set anything provided. */
	*attr_slot = attrs;
	old_get = pv_slot->a.get;
	old_set = pv_slot->a.set;
	if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
		duk_tval *tv_src = duk_require_tval(thr, idx_desc);
		DUK_TVAL_SET_TVAL_INCREF(thr, &pv_slot->v, tv_src);
	} else {
		DUK_TVAL_SET_UNDEFINED(&pv_slot->v);
	}
	DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, old_get);
	DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, old_set);
	DUK_REFZERO_CHECK_SLOW(thr);
	return 1;

fail_not_configurable:
	return 0;
}

/* Update an existing accessor property. */
DUK_LOCAL duk_bool_t duk__prop_defown_update_keep_accessor(duk_hthread *thr,
                                                           duk_idx_t idx_desc,
                                                           duk_uint_t defprop_flags,
                                                           duk_propvalue *pv_slot,
                                                           duk_uint8_t *attr_slot,
                                                           duk_uint8_t attrs,
                                                           duk_uint8_t curr_configurable,
                                                           duk_uint_t have_shifted) {
	duk_hobject *new_get = NULL;
	duk_hobject *new_set = NULL;

	if (defprop_flags & DUK_DEFPROP_HAVE_GETTER) {
		new_get = duk_get_hobject(thr, duk__prop_defown_getter_index(idx_desc, defprop_flags));
	}
	if (defprop_flags & DUK_DEFPROP_HAVE_SETTER) {
		new_set = duk_get_hobject(thr, duk__prop_defown_setter_index(idx_desc, defprop_flags));
	}

	have_shifted &= DUK_DEFPROP_EC; /* Restrict HAVE flags to EC. */
	if (DUK_UNLIKELY(!curr_configurable && !(defprop_flags & DUK_DEFPROP_FORCE))) {
		/* Configurable: must not change.
		 * Enumerable: must not change.
		 * Getter: must not change.
		 * Setter: must not change.
		 */
		if (((attrs ^ (duk_uint8_t) (defprop_flags & 0xffU)) & have_shifted) != 0U) {
			goto fail_not_configurable;
		}
		if ((defprop_flags & DUK_DEFPROP_HAVE_GETTER) && (new_get != pv_slot->a.get)) {
			goto fail_not_configurable;
		}
		if ((defprop_flags & DUK_DEFPROP_HAVE_SETTER) && (new_set != pv_slot->a.set)) {
			goto fail_not_configurable;
		}
	}

	attrs &= ~((duk_uint8_t) have_shifted); /* Zero anything provided. */
	attrs |= (duk_uint8_t) (defprop_flags & have_shifted); /* Set anything provided. */
	*attr_slot = attrs;
	if (defprop_flags & DUK_DEFPROP_HAVE_GETTER) {
		duk_hobject *old_get = pv_slot->a.get;
		pv_slot->a.get = duk_get_hobject(thr, duk__prop_defown_getter_index(idx_desc, defprop_flags));
		DUK_HOBJECT_INCREF_ALLOWNULL(thr, pv_slot->a.get);
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, old_get);
	}
	if (defprop_flags & DUK_DEFPROP_HAVE_SETTER) {
		duk_hobject *old_set = pv_slot->a.set;
		pv_slot->a.set = duk_get_hobject(thr, duk__prop_defown_setter_index(idx_desc, defprop_flags));
		DUK_HOBJECT_INCREF_ALLOWNULL(thr, pv_slot->a.set);
		DUK_HOBJECT_DECREF_NORZ_ALLOWNULL(thr, old_set);
	}
	DUK_REFZERO_CHECK_SLOW(thr);
	return 1;

fail_not_configurable:
	return 0;
}

/* Update an existing data property. */
DUK_LOCAL duk_bool_t duk__prop_defown_update_keep_data(duk_hthread *thr,
                                                       duk_idx_t idx_desc,
                                                       duk_uint_t defprop_flags,
                                                       duk_propvalue *pv_slot,
                                                       duk_uint8_t *attr_slot,
                                                       duk_uint8_t attrs,
                                                       duk_uint8_t curr_configurable,
                                                       duk_uint_t have_shifted) {
	have_shifted &= DUK_DEFPROP_WEC; /* Restrict HAVE flags to WEC. */
	if (DUK_UNLIKELY(!curr_configurable && !(defprop_flags & DUK_DEFPROP_FORCE))) {
		/* Configurable: must not change.
		 * Enumerable: must not change.
		 * Writable: can go from true to false, but not vice versa.
		 * Value: must not change.
		 */
		duk_uint_t have_shifted_ec = (have_shifted & DUK_DEFPROP_EC);

		if (((attrs ^ (duk_uint8_t) (defprop_flags & 0xffU)) & have_shifted_ec) != 0U) {
			DUK_DD(DUK_DDPRINT("EC flag conflict"));
			goto fail_not_configurable;
		}
		if ((attrs & DUK_PROPDESC_FLAG_WRITABLE) == 0U) {
			if ((defprop_flags & (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)) ==
			    (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)) {
				DUK_DD(DUK_DDPRINT("W flag conflict"));
				goto fail_not_configurable;
			}
			if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
				duk_tval *tv_src = duk_require_tval(thr, idx_desc);
				if (!duk_js_samevalue(&pv_slot->v, tv_src)) {
					DUK_DD(DUK_DDPRINT("[[Value]] conflict"));
					goto fail_not_configurable;
				}
			}
		}
	}

	attrs &= ~((duk_uint8_t) have_shifted); /* Zero anything provided. */
	attrs |= (duk_uint8_t) (defprop_flags & have_shifted); /* Set anything provided. */
	*attr_slot = attrs;
	if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
		duk_tval *tv_src = duk_require_tval(thr, idx_desc);
		DUK_TVAL_SET_TVAL_UPDREF_NORZ(thr, &pv_slot->v, tv_src);
	}
	DUK_REFZERO_CHECK_SLOW(thr);
	return 1;

fail_not_configurable:
	return 0;
}

/* Update a property in an existing slot, possibly converting it from a data
 * property to an accessor property or vice versa.
 */
DUK_LOCAL duk_bool_t duk__prop_defown_update_existing_slot(duk_hthread *thr,
                                                           duk_idx_t idx_desc,
                                                           duk_uint_t defprop_flags,
                                                           duk_propvalue *pv_slot,
                                                           duk_uint8_t *attr_slot) {
	duk_uint8_t attrs;
	duk_uint8_t curr_configurable;
	duk_uint_t have_shifted;

	/* In principle there is a separate validation step followed by an
	 * update step, see ValidateAndApplyPropertyDescriptor().  The
	 * algorithm here is modified in two ways:
	 *
	 *  - The validation and application step are combined.
	 *  - There is a 4-way initial branch deciding whether we're
	 *      a) updating a data property,
	 *      b) updating an accessor property,
	 *      c) converting a data property to an accessor property, or
	 *      d) converting an accessor property to a data property.
	 *    All attribute checks are inlined for each case.
	 *
	 * The general principles are:
	 *
	 * - If a property is configurable, any changes are allowed.
	 * - If a property is not configurable, no changes are allowed, except:
	 *     1. writing SameValue() to existing attribute is allowed; and
	 *     2. a writable property can be made non-writable.
	 */

	attrs = *attr_slot;
	curr_configurable = (attrs & DUK_PROPDESC_FLAG_CONFIGURABLE);
	have_shifted = (defprop_flags & DUK_DEFPROP_HAVE_WEC) >> DUK_DEFPROP_HAVE_SHIFT_COUNT;

	if (DUK_UNLIKELY(attrs & DUK_PROPDESC_FLAG_ACCESSOR)) {
		DUK_ASSERT((attrs & DUK_PROPDESC_FLAG_WRITABLE) == 0U);
		if (defprop_flags & (DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WRITABLE)) {
			/* Convert from accessor to data property. */
			return duk__prop_defown_update_convert_to_data(thr,
			                                               idx_desc,
			                                               defprop_flags,
			                                               pv_slot,
			                                               attr_slot,
			                                               attrs,
			                                               curr_configurable,
			                                               have_shifted);
		} else {
			/* Keep as an accessor. */
			return duk__prop_defown_update_keep_accessor(thr,
			                                             idx_desc,
			                                             defprop_flags,
			                                             pv_slot,
			                                             attr_slot,
			                                             attrs,
			                                             curr_configurable,
			                                             have_shifted);
		}
	} else {
		if (DUK_UNLIKELY(defprop_flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER))) {
			/* Convert from data property to accessor. */
			return duk__prop_defown_update_convert_to_accessor(thr,
			                                                   idx_desc,
			                                                   defprop_flags,
			                                                   pv_slot,
			                                                   attr_slot,
			                                                   attrs,
			                                                   curr_configurable,
			                                                   have_shifted);
		} else {
			/* Keep as a data property. */
			return duk__prop_defown_update_keep_data(thr,
			                                         idx_desc,
			                                         defprop_flags,
			                                         pv_slot,
			                                         attr_slot,
			                                         attrs,
			                                         curr_configurable,
			                                         have_shifted);
		}
	}
}

DUK_LOCAL duk_bool_t duk__prop_defown_strkey_ordinary(duk_hthread *thr,
                                                      duk_hobject *obj,
                                                      duk_hstring *key,
                                                      duk_idx_t idx_desc,
                                                      duk_uint_t defprop_flags) {
	duk_uint_fast32_t ent_idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));

	if (DUK_UNLIKELY(duk_hobject_lookup_strprop_index(thr, obj, key, &ent_idx))) {
		duk_propvalue *val_base;
		duk_hstring **key_base;
		duk_uint8_t *attr_base;
		duk_propvalue *pv_slot;
		duk_uint8_t *attr_slot;

		duk_hobject_get_strprops_key_attr(thr->heap, obj, &val_base, &key_base, &attr_base);

		DUK_ASSERT(key_base[ent_idx] == key);
		pv_slot = val_base + ent_idx;
		attr_slot = attr_base + ent_idx;

		return duk__prop_defown_update_existing_slot(thr, idx_desc, defprop_flags, pv_slot, attr_slot);
	} else {
		duk_propvalue *val_base;
		duk_hstring **key_base;
		duk_uint8_t *attr_base;
		duk_propvalue *pv_slot;
		duk_uint8_t *attr_slot;

		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj) && !(defprop_flags & DUK_DEFPROP_FORCE))) {
			goto fail_not_extensible;
		}

		ent_idx = (duk_uint_fast32_t) duk_hobject_alloc_strentry_checked(thr, obj, key);
		duk_hobject_get_strprops_key_attr(thr->heap, obj, &val_base, &key_base, &attr_base);
		DUK_ASSERT(key_base[ent_idx] == key);
		pv_slot = val_base + ent_idx;
		attr_slot = attr_base + ent_idx;

		return duk__prop_defown_write_new_slot(thr, idx_desc, defprop_flags, pv_slot, attr_slot);
	}

fail_not_extensible:
	return 0;
}

DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_strkey_stringobj_length(duk_hthread *thr,
                                                                       duk_hobject *obj,
                                                                       duk_idx_t idx_desc,
                                                                       duk_uint_t defprop_flags) {
	duk_hstring *h;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_STRING_OBJECT);

	h = duk_hobject_lookup_intvalue_hstring(thr, obj);
	if (h != NULL) {
		if (!duk__prop_validate_immutable_data_desc(DUK_PROPDESC_FLAGS_NONE, defprop_flags)) {
			goto fail_invalid_desc;
		}
		DUK_ASSERT((defprop_flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER)) == 0U);
		if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
			duk_tval tv_tmp;
			DUK_TVAL_SET_U32(&tv_tmp, duk_hstring_get_charlen(h));
			if (!duk_js_samevalue(duk_require_tval(thr, idx_desc), &tv_tmp)) {
				goto fail_invalid_desc;
			}
		}
	} else {
		goto fail_internal;
	}
	return 1;

fail_invalid_desc:
fail_internal:
	return 0;
}

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
DUK_LOCAL DUK_COLD duk_bool_t duk__prop_defown_strkey_bufobj_length(duk_hthread *thr,
                                                                    duk_hobject *obj,
                                                                    duk_idx_t idx_desc,
                                                                    duk_uint_t defprop_flags) {
	duk_hbufobj *h;

	DUK_ASSERT(DUK_HOBJECT_IS_ANY_BUFOBJ(obj));

	h = (duk_hbufobj *) obj;
	if (!duk__prop_validate_immutable_data_desc(DUK_PROPDESC_FLAGS_NONE, defprop_flags)) {
		goto invalid_desc;
	}
	if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
		duk_tval tv_tmp;
		DUK_TVAL_SET_U32(&tv_tmp, (duk_uint32_t) DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h));
		if (!duk_js_samevalue(duk_require_tval(thr, idx_desc), &tv_tmp)) {
			goto invalid_desc;
		}
	}
	return 1;

invalid_desc:
	return 0;
}
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */

DUK_LOCAL duk_bool_t duk__prop_defown_strkey_array_length(duk_hthread *thr,
                                                          duk_hobject *obj,
                                                          duk_idx_t idx_desc,
                                                          duk_uint_t defprop_flags) {
	duk_harray *a = (duk_harray *) obj;
	duk_bool_t want_write_protect;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_ARRAY);

	/* Descriptor compatibility:
	 *   - length is always non-configurable: cannot change to configurable
	 *   - length is always non-enumerable: cannot change to enumerable
	 *   - length cannot be made an accessor
	 *   - length may be writable or non-writable;
	 *     + if writable, can be changed to non-writable (but length updated first)
	 *     + if not writable, cannot change to writable
	 */

	if ((defprop_flags & (DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE)) ==
	    (DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE)) {
		DUK_DD(DUK_DDPRINT("cannot make .length configurable"));
		goto fail_invalid_desc;
	}
	if ((defprop_flags & (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE)) ==
	    (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE)) {
		DUK_DD(DUK_DDPRINT("cannot make .length enumerable"));
		goto fail_invalid_desc;
	}
	if (DUK_HARRAY_LENGTH_NONWRITABLE(a) && (defprop_flags & (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)) ==
	                                            (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)) {
		if (DUK_UNLIKELY(defprop_flags & DUK_DEFPROP_FORCE)) {
			/* XXX: Allow forcing? */
		}
		DUK_DD(DUK_DDPRINT("cannot make .length writable again"));
		goto fail_invalid_desc;
	}
	if (duk__prop_is_accessor_descriptor(defprop_flags)) {
		DUK_DD(DUK_DDPRINT("cannot make .length an accessor"));
		goto fail_invalid_desc;
	}
	want_write_protect = ((defprop_flags & (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)) == DUK_DEFPROP_HAVE_WRITABLE);

	rc = 1;

	/* The specification steps are quite different here.  Main points are:
	 * - When reducing .length: if a non-configurable element at index >= new_len
	 *   cannot be deleted, we stop at that position, and update the length to
	 *   match that element.
	 * - If a write protect is pending, it is applied EVEN if the length
	 *   update succeeds only partially (or not at all, i.e. first element
	 *   cannot be deleted).
	 */
	if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
		duk_uint32_t new_len;

		new_len = duk_harray_to_array_length_checked(thr, duk_require_tval(thr, idx_desc));
		if (DUK_LIKELY(new_len != DUK_HARRAY_GET_LENGTH(a))) {
			/* Technically a SameValue() comparison. */
			if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a) && !(defprop_flags & DUK_DEFPROP_FORCE))) {
				DUK_DD(DUK_DDPRINT("length non-writable, not SameValue()"));
				goto fail_length_nonwritable;
			}

			rc = duk_harray_put_array_length_u32(thr, obj, new_len, defprop_flags & DUK_DEFPROP_FORCE /*force_flag*/);
		}
	}

	if (want_write_protect) {
		DUK_HARRAY_SET_LENGTH_NONWRITABLE(a);
	}

	return rc;

fail_invalid_desc:
fail_length_nonwritable:
	return 0;
}

#if defined(DUK_USE_PROXY_POLICY)
DUK_LOCAL duk_bool_t duk__prop_defown_proxy_policy(duk_hthread *thr, duk_hobject *obj, duk_bool_t trap_rc) {
	duk_hobject *target;
	duk_small_int_t attrs;

	DUK_ASSERT(trap_rc == 1);
	DUK_UNREF(trap_rc);

	target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
	DUK_ASSERT(target != NULL);

	attrs = duk_prop_getowndesc_obj_tvkey(thr, target, duk_require_tval(thr, -1));
	duk_prop_pop_propdesc(thr, attrs);

	return 1;
}
#endif

DUK_LOCAL duk_bool_t duk__prop_defown_proxy_tail(duk_hthread *thr, duk_hobject *obj, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	duk_bool_t trap_rc;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);

	/* [ ... trap handler target key ] */

	duk_dup_top(thr);
	duk_insert(thr, -5); /* Stash key for policy check. */

	/* [ ... key trap handler target key ] */

	duk_prop_frompropdesc_with_idx(thr, idx_desc, (duk_int_t) defprop_flags);
	duk_call_method(thr, 3); /* [ ... key trap handler target key desc ] -> [ ... key result ] */

	trap_rc = duk_to_boolean_top_pop(thr);
	if (!trap_rc) {
		duk_pop_known(thr);
		return 0;
	}

#if defined(DUK_USE_PROXY_POLICY)
	rc = duk__prop_defown_proxy_policy(thr, obj, trap_rc);
#else
	DUK_DD(DUK_DDPRINT("proxy policy check for 'defineProperty' trap disabled in configuration"));
	rc = 1;
#endif

	duk_pop_known(thr);
	return rc;
}

DUK_LOCAL duk_small_int_t
duk__prop_defown_strkey_proxy(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_PROXY);
	DUK_ASSERT(key != NULL);

	if (duk_proxy_trap_check_strkey(thr, (duk_hproxy *) obj, key, DUK_STRIDX_DEFINE_PROPERTY)) {
		duk_push_hstring(thr, key);
		return (duk_small_int_t) duk__prop_defown_proxy_tail(thr, obj, idx_desc, defprop_flags);
	} else {
		return -1;
	}
}

DUK_LOCAL duk_bool_t duk__prop_defown_strkey_helper(duk_hthread *thr,
                                                    duk_hobject *obj,
                                                    duk_hstring *key,
                                                    duk_idx_t idx_desc,
                                                    duk_uint_t defprop_flags,
                                                    duk_bool_t side_effect_safe) {
	duk_small_uint_t htype;
	duk_hobject *target;
	duk_bool_t rc;

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
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			rc = duk__prop_defown_strkey_array_length(thr, target, idx_desc, defprop_flags);
			goto check_rc;
		}
		break;
	case DUK_HTYPE_ARGUMENTS:
		/* Arguments [[DefineOwnProperty]] simplifies to
		 * OrdinaryDefineOwnProperty() for non-index keys.
		 */
		break;
	case DUK_HTYPE_STRING_OBJECT:
		/* No exotic behavior for [[Length]] but because it is a
		 * virtual property, check for descriptor compatibility
		 * and ignore possible write.
		 */
		if (DUK_HSTRING_HAS_LENGTH(key)) {
			rc = duk__prop_defown_strkey_stringobj_length(thr, target, idx_desc, defprop_flags);
			goto check_rc;
		}
		break;
	case DUK_HTYPE_PROXY:
		if (side_effect_safe) {
			duk_small_int_t proxy_rc;
			proxy_rc = duk__prop_defown_strkey_proxy(thr, target, key, idx_desc, defprop_flags);
			if (proxy_rc < 0) {
				/* No trap, continue to target. */
				duk_hobject *next;

				next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) target);
				DUK_ASSERT(next != NULL);

				target = duk_prop_switch_stabilized_target_top(thr, target, next);
				goto retry_target;
			} else {
				rc = (duk_bool_t) proxy_rc;
				goto check_rc;
			}
		} else {
			goto switch_to_safe;
		}
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
		if (DUK_HSTRING_HAS_LENGTH_OR_CANNUM(key)) {
			if (DUK_HSTRING_HAS_LENGTH(key)) {
				/* Custom own .length property. */
				rc = duk__prop_defown_strkey_bufobj_length(thr, target, idx_desc, defprop_flags);
				goto check_rc;
			} else {
				DUK_ASSERT(DUK_HSTRING_HAS_CANNUM(key));
				/* Never accepted, short circuited as error. */
				goto fail_invalid_index;
			}
		}
		break;
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	rc = duk__prop_defown_strkey_ordinary(thr, target, key, idx_desc, defprop_flags);
	/* fall through */

check_rc:
	if (side_effect_safe) {
		duk_pop_known(thr);
	}
	if (DUK_UNLIKELY(rc == 0)) {
		return duk__prop_defown_error_obj_strkey(thr, target, key, defprop_flags);
	}
	return rc;

fail_invalid_index:
	rc = 0;
	goto check_rc;

switch_to_safe:
	return duk__prop_defown_strkey_safe(thr, obj, key, idx_desc, defprop_flags);
}

DUK_LOCAL duk_bool_t
duk__prop_defown_strkey_unsafe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_defown_strkey_safe(thr, obj, key, idx_desc, defprop_flags);
#else
	return duk__prop_defown_strkey_helper(thr, obj, key, idx_desc, defprop_flags, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL duk_bool_t
duk__prop_defown_strkey_safe(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	return duk__prop_defown_strkey_helper(thr, obj, key, idx_desc, defprop_flags, 1 /*side_effect_safe*/);
}

DUK_LOCAL duk_bool_t duk__prop_defown_bufobj_write(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_val) {
	duk_hbufobj *h = (duk_hbufobj *) obj;
	duk_bool_t our_rc;

	duk_push_hobject(thr, obj);
	duk_dup(thr, idx_val);
	(void) duk_to_number_m1(thr);

	our_rc = duk_hbufobj_validate_and_write_top(thr, h, idx);
	duk_pop_2_known(thr);
	return our_rc;
}

DUK_LOCAL duk_bool_t duk__prop_defown_idxkey_ordinary(duk_hthread *thr,
                                                      duk_hobject *obj,
                                                      duk_uarridx_t idx,
                                                      duk_idx_t idx_desc,
                                                      duk_uint_t defprop_flags) {
	duk_uint_fast32_t ent_idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

	if (DUK_UNLIKELY(duk_hobject_lookup_idxprop_index(thr, obj, idx, &ent_idx))) {
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;
		duk_uint8_t *attr_base;
		duk_propvalue *pv_slot;
		duk_uint8_t *attr_slot;

		duk_hobject_get_idxprops_key_attr(thr->heap, obj, &val_base, &key_base, &attr_base);

		DUK_ASSERT(key_base[ent_idx] == idx);
		pv_slot = val_base + ent_idx;
		attr_slot = attr_base + ent_idx;

		return duk__prop_defown_update_existing_slot(thr, idx_desc, defprop_flags, pv_slot, attr_slot);
	} else {
		duk_propvalue *val_base;
		duk_uarridx_t *key_base;
		duk_uint8_t *attr_base;
		duk_propvalue *pv_slot;
		duk_uint8_t *attr_slot;

		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj) && !(defprop_flags & DUK_DEFPROP_FORCE))) {
			goto fail_not_extensible;
		}
		ent_idx = (duk_uint_fast32_t) duk_hobject_alloc_idxentry_checked(thr, obj, idx);
		duk_hobject_get_idxprops_key_attr(thr->heap, obj, &val_base, &key_base, &attr_base);
		DUK_ASSERT(key_base[ent_idx] == idx);
		pv_slot = val_base + ent_idx;
		attr_slot = attr_base + ent_idx;

		return duk__prop_defown_write_new_slot(thr, idx_desc, defprop_flags, pv_slot, attr_slot);
	}

fail_not_extensible:
	return 0;
}

/* Attempt to write to array items part, caller handles .length. */
DUK_LOCAL duk_small_int_t duk__prop_defown_idxkey_array_items_attempt(duk_hthread *thr,
                                                                      duk_hobject *obj,
                                                                      duk_uarridx_t idx,
                                                                      duk_idx_t idx_desc,
                                                                      duk_uint_t defprop_flags) {
	duk_tval *tv_slot;
	duk_tval *tv_val;

	tv_slot = duk_hobject_obtain_arridx_slot(thr, idx, obj);

	if (DUK_UNLIKELY(tv_slot == NULL)) {
		/* Failed to extend, now abandoned. */
		DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		return -1;
	}
	DUK_ASSERT(DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));

	if (DUK_TVAL_IS_UNUSED(tv_slot)) {
		/* Writing a new entry; defaults attributes are all
		 * false, so the only allowed combination for a linear
		 * array part is that all attributes are present with
		 * true value.
		 *
		 * No side effects affecting Array .length can happen
		 * here so it should safe to update .length later.
		 */
		if (DUK_UNLIKELY(!DUK_HOBJECT_HAS_EXTENSIBLE(obj) && !(defprop_flags & DUK_DEFPROP_FORCE))) {
			goto fail_not_extensible;
		}
		if (!duk__prop_validate_immutable_data_desc(DUK_PROPDESC_FLAGS_WEC, defprop_flags)) {
			DUK_DDD(DUK_DDDPRINT("new attrs not compatible"));
			goto abandon_items;
		}
		DUK_ASSERT((defprop_flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER)) == 0U);

		/* Must write 'undefined' even without a value. */
		DUK_TVAL_SET_UNDEFINED(tv_slot);
	} else {
		/* Existing entry is already WEC, so check that we're
		 * not trying to modify any attribute to incompatible
		 * value.
		 */
		if (!duk__prop_validate_immutable_data_desc(DUK_PROPDESC_FLAGS_WEC, defprop_flags)) {
			DUK_DDD(DUK_DDDPRINT("existing attrs not compatible"));
			goto abandon_items;
		}
		DUK_ASSERT((defprop_flags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER)) == 0U);
	}

	if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
		/* When writing to an existing entry, the DECREF can cause
		 * arbitrary side effects, e.g. the Array we're working on
		 * can be modified.  But in this case we don't need to update
		 * .length.
		 */
		tv_val = DUK_GET_TVAL_POSIDX(thr, idx_desc);
		DUK_TVAL_SET_TVAL_UPDREF(thr, tv_slot, tv_val);
	}
	return 1;

abandon_items:
	duk_hobject_abandon_array_items(thr, obj);
	return -1;

fail_not_extensible:
	return 0;
}

DUK_LOCAL duk_bool_t
duk__prop_defown_idxkey_array(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	duk_harray *a = (duk_harray *) obj;
	duk_uint32_t old_len;
	duk_uint32_t new_len;
	duk_bool_t def_rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_ARRAY || DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_ARGUMENTS);

	old_len = DUK_HARRAY_GET_LENGTH(a);
	if (idx >= old_len) {
		if (DUK_UNLIKELY(DUK_HARRAY_LENGTH_NONWRITABLE(a) && !(defprop_flags & DUK_DEFPROP_FORCE))) {
			goto fail_length_not_writable;
		}
		new_len = idx + 1;
		DUK_ASSERT(new_len > idx);
	} else {
		new_len = 0U; /* Marker: no length update. */
	}

	/* If we still have linear array items, simulate ordinary
	 * [[DefineOwnProperty]], bailing out if any criterion is
	 * not met.
	 */

	if (DUK_HOBJECT_HAS_ARRAY_ITEMS(obj)) {
		duk_small_int_t items_rc = duk__prop_defown_idxkey_array_items_attempt(thr, obj, idx, idx_desc, defprop_flags);
		if (items_rc < 0) {
			/* Abandoned, fall through to ordinary idxprops. */
			DUK_DDD(DUK_DDDPRINT("abandoned, use ordinary idxprops"));
			DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
		} else if (items_rc != 0) {
			goto length_update;
		} else {
			goto fail_not_extensible;
		}
	}

	/* No array items part, run ordinary algorithm first, and update
	 * length afterwards.  Length writability already checked above
	 * so it should never fail.
	 */
	DUK_ASSERT(!DUK_HOBJECT_HAS_ARRAY_ITEMS(obj));
	def_rc = duk__prop_defown_idxkey_ordinary(thr, obj, idx, idx_desc, defprop_flags);
	if (def_rc == 0) {
		goto fail_unknown; /* Several possible reasons. */
	}
	/* pass through */

length_update:
	if (new_len > 0U) {
		DUK_ASSERT(DUK_HARRAY_GET_LENGTH(a) == old_len);
		DUK_HARRAY_SET_LENGTH(a, new_len);
	}
	return 1;

fail_length_not_writable:
fail_not_extensible:
fail_unknown:
	return 0;
}

DUK_LOCAL duk_small_int_t duk__prop_defown_idxkey_stringobj(duk_hthread *thr,
                                                            duk_hobject *obj,
                                                            duk_uarridx_t idx,
                                                            duk_idx_t idx_desc,
                                                            duk_uint_t defprop_flags) {
	duk_hstring *h;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_STRING_OBJECT);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(idx_desc >= 0);

	/* String index properties [0,len[ are individual characters
	 * with writable=false, enumerable=true, configurable=false.
	 * Test descriptor compatibility.
	 */

	h = duk_hobject_lookup_intvalue_hstring(thr, obj);
	if (DUK_LIKELY(h != NULL)) {
		if (DUK_LIKELY(!DUK_HSTRING_HAS_SYMBOL(h) && idx < duk_hstring_get_charlen(h))) {
			if (duk__prop_validate_immutable_data_desc(DUK_PROPDESC_FLAGS_E, defprop_flags) == 0) {
				goto fail_invalid_desc;
			}
			if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
				duk_bool_t match;

				duk_prop_push_plainstr_idx(thr, h, idx);
				match = duk_samevalue(thr, idx_desc, -1);
				duk_pop_known(thr);
				if (!match) {
					goto fail_invalid_desc;
				}
			}
			return 1;
		}
	} else {
		goto fail_internal;
	}
	return -1;

fail_invalid_desc:
fail_internal:
	return 0;
}

DUK_LOCAL DUK_NOINLINE duk_bool_t duk__prop_defown_idxkey_arguments(duk_hthread *thr,
                                                                    duk_hobject *obj,
                                                                    duk_uarridx_t idx,
                                                                    duk_idx_t idx_desc,
                                                                    duk_uint_t defprop_flags) {
	duk_hobject *map;
	duk_hobject *env;
	duk_hstring *varname;
	duk_bool_t is_mapped;
	duk_bool_t rc_ordinary;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_ARGUMENTS);

	/* Special behavior when property is arguments-mapped and input descriptor
	 * is a data descriptor trying to write protect the property.  If the
	 * descriptor is missing a [[Value]], read the mapped variable value and
	 * force it into the descriptor, so that the write protected value will
	 * reflect an up-to-date variable value (this is not very important because
	 * the two can then diverge anyway, but it's an explicit requirement in
	 * the specification algorithm).
	 *
	 * Use array code as a helper; it validates and updates 'length' but that
	 * doesn't matter because it's ignored for arguments.
	 *
	 * Getvar can have arbitrary side effects (it may be captured e.g. by a
	 * with(proxy) statement) so caller must stabilize 'obj'.
	 */

	varname = duk_prop_arguments_map_prep_idxkey(thr, obj, idx, &map, &env);
	is_mapped = (varname != NULL);

	if (is_mapped != 0 && duk__prop_is_data_descriptor(defprop_flags) &&
	    (defprop_flags & (DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)) ==
	        (DUK_DEFPROP_HAVE_WRITABLE)) {
		duk_uint_t modified_flags = defprop_flags | DUK_DEFPROP_HAVE_VALUE;

		/* Getvar can have arbitrary side effects, as it may be captured
		 * e.g. by a with(proxy).
		 */
		DUK_D(DUK_DPRINT("write protecting arguments mapped idxkey with no [[Value]], force current variable value"));
		(void) duk_js_getvar_envrec(thr, env, varname, 1 /*throw*/); /* -> [ ... value this_binding ] */
		rc_ordinary = duk__prop_defown_idxkey_array(thr, obj, idx, duk_get_top(thr) - 2, modified_flags);
		duk_pop_2_known(thr);
	} else {
		rc_ordinary = duk__prop_defown_idxkey_array(thr, obj, idx, idx_desc, defprop_flags);
	}

	if (rc_ordinary == 0) {
		DUK_DD(DUK_DDPRINT("ordinary defprop failed, no map update"));
		return 0;
	}

	if (is_mapped) {
		/* Specification algorithm will both update the
		 * map entry and delete it if [[Value]] is present
		 * and [[Writable]]=false.  Only the deletion
		 * should be necessary in this case.
		 */
		duk_bool_t delete_mapping = 0;

		if (duk__prop_is_accessor_descriptor(defprop_flags)) {
			DUK_D(DUK_DPRINT("delete arguments mapping for index %ld because converting to accessor", (long) idx));
			delete_mapping = 1;
		} else {
			if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
				/* Technically we're calling Set() on the map, but the map
				 * contains getter/setter functions which operate on the
				 * variables.
				 */
				DUK_DD(DUK_DDPRINT("update variable after arguments defprop, index %ld", (long) idx));
				duk_dup(thr, idx_desc);
				duk_js_putvar_envrec(thr, env, varname, DUK_GET_TVAL_NEGIDX(thr, -1), 0 /*throw_flag*/);
				duk_pop_known(thr);
			}
			if ((defprop_flags & (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE)) == DUK_DEFPROP_HAVE_WRITABLE) {
				DUK_D(DUK_DPRINT("delete arguments mapping for index %ld because setting writable=false",
				                 (long) idx));
				delete_mapping = 1;
			}
		}
		if (delete_mapping) {
			duk_push_hobject(thr, map);
			(void) duk_prop_delete_idxkey(thr, duk_get_top_index_known(thr), idx, 0 /*delprop_flags*/);
			duk_pop_known(thr);
		}
	}

	return 1;
}

DUK_LOCAL duk_small_int_t
duk__prop_defown_idxkey_proxy(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_PROXY);

	if (duk_proxy_trap_check_idxkey(thr, (duk_hproxy *) obj, idx, DUK_STRIDX_DEFINE_PROPERTY)) {
		(void) duk_push_u32_tostring(thr, idx);
		return (duk_small_int_t) duk__prop_defown_proxy_tail(thr, obj, idx_desc, defprop_flags);
	} else {
		return -1;
	}
}

DUK_LOCAL duk_bool_t duk__prop_defown_idxkey_bufobj(duk_hthread *thr,
                                                    duk_hobject *obj,
                                                    duk_uarridx_t idx,
                                                    duk_idx_t idx_desc,
                                                    duk_uint_t defprop_flags) {
	duk_hbufobj *h = (duk_hbufobj *) obj;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

	/* Index validation happens before descriptor validation in the
	 * specification.
	 */
	if (DUK_UNLIKELY(idx >= DUK_HBUFOBJ_GET_LOGICAL_LENGTH(h))) {
		goto fail_invalid_index;
	}
	if (!duk__prop_validate_immutable_data_desc(DUK_DEFPROP_WEC, defprop_flags)) {
		/* NOTE: Somewhat bizarrely, typed array indices cannot be deleted
		 * but they are still configurable.
		 */
		goto fail_invalid_desc;
	}
	if (defprop_flags & DUK_DEFPROP_HAVE_VALUE) {
		/* Value checking and coercion should happen after validation. */
		if (!duk__prop_defown_bufobj_write(thr, obj, idx, idx_desc)) {
			goto fail_invalid_index;
		}
	}
	/* Short circuited, never proceed to standard [[DefineOwnProperty]]. */
	return 1;

fail_invalid_index:
fail_invalid_desc:
	return 0;
}

DUK_LOCAL duk_bool_t duk__prop_defown_idxkey_helper(duk_hthread *thr,
                                                    duk_hobject *obj,
                                                    duk_uarridx_t idx,
                                                    duk_idx_t idx_desc,
                                                    duk_uint_t defprop_flags,
                                                    duk_bool_t side_effect_safe) {
	duk_small_uint_t htype;
	duk_hobject *target;
	duk_bool_t rc;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT_ARRIDX_VALID(idx);
	DUK_ASSERT(idx_desc >= 0);

	target = obj;
	if (side_effect_safe) {
		duk_push_hobject(thr, target);
	}

retry_target:
	htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) target);
	DUK_ASSERT(DUK_HTYPE_IS_ANY_OBJECT(htype));

	switch (htype) {
	case DUK_HTYPE_ARRAY:
		rc = duk__prop_defown_idxkey_array(thr, target, idx, idx_desc, defprop_flags);
		goto check_rc;
	case DUK_HTYPE_ARGUMENTS:
		if (side_effect_safe) {
			rc = duk__prop_defown_idxkey_arguments(thr, target, idx, idx_desc, defprop_flags);
			goto check_rc;
		} else {
			goto switch_to_safe;
		}
	case DUK_HTYPE_STRING_OBJECT: {
		duk_small_int_t str_rc;
		str_rc = duk__prop_defown_idxkey_stringobj(thr, target, idx, idx_desc, defprop_flags);
		if (str_rc < 0) {
			break;
		}
		rc = (duk_bool_t) str_rc;
		goto check_rc;
	}
	case DUK_HTYPE_PROXY:
		if (side_effect_safe) {
			duk_small_int_t proxy_rc;
			proxy_rc = duk__prop_defown_idxkey_proxy(thr, target, idx, idx_desc, defprop_flags);
			if (proxy_rc < 0) {
				/* No trap, continue to target. */
				duk_hobject *next;

				next = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) target);
				DUK_ASSERT(next != NULL);

				target = duk_prop_switch_stabilized_target_top(thr, target, next);
				goto retry_target;
			} else {
				rc = (duk_bool_t) proxy_rc;
				goto check_rc;
			}
		} else {
			goto switch_to_safe;
		}
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
		rc = duk__prop_defown_idxkey_bufobj(thr, target, idx, idx_desc, defprop_flags);
		goto check_rc;
#endif /* DUK_USE_BUFFEROBJECT_SUPPORT */
	default:
		break;
	}

	rc = duk__prop_defown_idxkey_ordinary(thr, target, idx, idx_desc, defprop_flags);
	/* fall through */

check_rc:
	if (side_effect_safe) {
		duk_pop_known(thr);
	}
	if (DUK_UNLIKELY(rc == 0)) {
		return duk__prop_defown_error_obj_idxkey(thr, target, idx, defprop_flags);
	}
	return rc;

switch_to_safe:
	return duk__prop_defown_idxkey_safe(thr, obj, idx, idx_desc, defprop_flags);
}

DUK_LOCAL duk_bool_t duk__prop_defown_idxkey_unsafe(duk_hthread *thr,
                                                    duk_hobject *obj,
                                                    duk_uarridx_t idx,
                                                    duk_idx_t idx_desc,
                                                    duk_uint_t defprop_flags) {
#if defined(DUK_USE_PREFER_SIZE)
	return duk__prop_defown_idxkey_safe(thr, obj, idx, idx_desc, defprop_flags);
#else
	return duk__prop_defown_idxkey_helper(thr, obj, idx, idx_desc, defprop_flags, 0 /*side_effect_safe*/);
#endif
}

DUK_LOCAL duk_bool_t
duk__prop_defown_idxkey_safe(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
	return duk__prop_defown_idxkey_helper(thr, obj, idx, idx_desc, defprop_flags, 1 /*side_effect_safe*/);
}

DUK_INTERNAL duk_bool_t
duk_prop_defown_strkey(duk_hthread *thr, duk_hobject *obj, duk_hstring *key, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(idx_desc >= 0);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_desc));
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_UNLIKELY(DUK_HSTRING_HAS_ARRIDX(key))) {
		duk_bool_t rc;

		rc = duk__prop_defown_idxkey_unsafe(thr, obj, duk_hstring_get_arridx_fast_known(key), idx_desc, defprop_flags);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	} else {
		duk_bool_t rc;

		DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
		rc = duk__prop_defown_strkey_unsafe(thr, obj, key, idx_desc, defprop_flags);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	}
}

DUK_INTERNAL duk_bool_t
duk_prop_defown_idxkey(duk_hthread *thr, duk_hobject *obj, duk_uarridx_t idx, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(idx_desc >= 0);
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_desc));
#if defined(DUK_USE_ASSERTIONS)
	entry_top = duk_get_top(thr);
#endif

	if (DUK_LIKELY(idx <= DUK_ARRIDX_MAX)) {
		duk_bool_t rc = duk__prop_defown_idxkey_unsafe(thr, obj, idx, idx_desc, defprop_flags);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	} else {
		duk_bool_t rc;
		duk_hstring *key;

		DUK_DD(DUK_DDPRINT("corner case, input idx 0xffffffff is not an arridx, must coerce to string"));
		key = duk_push_u32_tohstring(thr, idx);
		rc = duk__prop_defown_strkey_unsafe(thr, obj, key, idx_desc, defprop_flags);
		duk_pop_known(thr);
		DUK_ASSERT(duk_get_top(thr) == entry_top);
		return rc;
	}
}

DUK_INTERNAL duk_bool_t
duk_prop_defown(duk_hthread *thr, duk_hobject *obj, duk_tval *tv_key, duk_idx_t idx_desc, duk_uint_t defprop_flags) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top;
#endif
	duk_bool_t rc;
	duk_hstring *key;
	duk_uarridx_t idx;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL); /* Must be stable against side effects. */
	DUK_ASSERT(tv_key != NULL);
	/* tv_key may not be in value stack but it must be reachable and
	 * remain reachable despite arbitrary side effects (e.g. function
	 * constant table).
	 */
	/* 'idx_desc' must be valid, but only if defprop_flags indicates
	 * there is a value or getter/setter.
	 */
	DUK_ASSERT(duk_is_valid_posidx(thr, idx_desc));
	/* 'throw_flag' is embedded into defprop_flags. */
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
#if defined(DUK_USE_FASTINT)
		DUK_ASSERT(!DUK_TVAL_IS_FASTINT(tv_key));
#endif
		d = DUK_TVAL_GET_DOUBLE(tv_key);
		if (duk_prop_double_idx_check(d, &idx)) {
			goto use_idx;
		}
#endif
		break;
	}
	}

	duk_push_tval(thr, tv_key);
	tv_key = NULL;
	key = duk_to_property_key_hstring(thr, -1);
	DUK_ASSERT(key != NULL);
	rc = duk_prop_defown_strkey(thr, obj, key, idx_desc, defprop_flags);
	duk_pop_known(thr);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;

use_idx:
	DUK_ASSERT_ARRIDX_VALID(idx);
	rc = duk__prop_defown_idxkey_unsafe(thr, obj, idx, idx_desc, defprop_flags);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;

use_str:
	DUK_ASSERT(!DUK_HSTRING_HAS_ARRIDX(key));
	rc = duk__prop_defown_strkey_unsafe(thr, obj, key, idx_desc, defprop_flags);
	DUK_ASSERT(duk_get_top(thr) == entry_top);
	return rc;
}
