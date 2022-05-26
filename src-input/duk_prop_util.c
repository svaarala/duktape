/*
 *  Utilities for property operations.
 */

DUK_INTERNAL DUK_INLINE_PERF duk_bool_t duk_prop_double_idx_check(duk_double_t d, duk_uarridx_t *out_idx) {
	duk_bool_t iswhole = duk_double_equals(DUK_FLOOR(d), d);
	if (iswhole && d >= 0.0 && d <= (duk_double_t) 0xfffffffeUL) {
		/* Negative zero is allowed as arridx. */
		*out_idx = (duk_uarridx_t) d;
		return 1;
	}
	return 0;
}

DUK_INTERNAL void duk_prop_push_plainstr_idx(duk_hthread *thr, duk_hstring *h, duk_uarridx_t idx) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HEAPHDR_IS_ANY_STRING((duk_heaphdr *) h));
	DUK_ASSERT(idx < duk_hstring_get_charlen(h));

	/* Fast path for pure ASCII strings. */
#if 1
	if (duk_hstring_is_ascii(h)) {
		duk_push_lstring(thr, (const char *) duk_hstring_get_data(h) + idx, 1);
	} else
#endif
	{
		duk_push_wtf8_substring_hstring(thr, h, idx, idx + 1);
	}
}

/* FromPropertyDescriptor(): convert result of duk_prop_getown.c lookup
 * (0-2 value stack entries on stack top + attributes), considered a
 * specification Property Descriptor type, into an ECMAScript descriptor
 * object.
 *
 * No partial descriptor support.
 */
DUK_INTERNAL void duk_prop_frompropdesc_propattrs(duk_hthread *thr, duk_int_t attrs) {
	duk_uint_t uattrs;
	if (attrs < 0) {
		duk_push_undefined(thr);
		return;
	}
	uattrs = (duk_uint_t) attrs;

	duk_push_object(thr);

	if (uattrs & DUK_PROPDESC_FLAG_ACCESSOR) {
		/* [ ... get set res ] */
		duk_pull(thr, -3);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_GET);
		duk_pull(thr, -2);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_SET);
	} else {
		/* [ ... value res ] */
		duk_pull(thr, -2);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_VALUE);
		duk_push_boolean(thr, uattrs & DUK_PROPDESC_FLAG_WRITABLE);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_WRITABLE);
	}
	duk_push_boolean(thr, uattrs & DUK_PROPDESC_FLAG_ENUMERABLE);
	duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_ENUMERABLE);
	duk_push_boolean(thr, uattrs & DUK_PROPDESC_FLAG_CONFIGURABLE);
	duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_CONFIGURABLE);

	/* [ ... res ] */
}

DUK_INTERNAL void duk_prop_frompropdesc_with_idx(duk_hthread *thr, duk_idx_t idx_desc, duk_int_t defprop_flags) {
	duk_uint_t uflags;
	if (defprop_flags < 0) {
		duk_push_undefined(thr);
		return;
	}
	uflags = (duk_uint_t) defprop_flags;

	duk_push_object(thr);

	if (uflags & DUK_DEFPROP_HAVE_GETTER) {
		duk_dup(thr, idx_desc);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_GET);
	}
	if (uflags & DUK_DEFPROP_HAVE_SETTER) {
		duk_dup(thr, idx_desc + 1);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_SET);
	}
	if (uflags & DUK_DEFPROP_HAVE_VALUE) {
		if (uflags & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER)) {
			/* Invalid descriptor: clarify API what happens if you do this. */
			DUK_D(DUK_DPRINT(
			    "invalid flags passed, both value and getter/setter: 0x%08lx -> ignoring getter/setter flag(s)",
			    (unsigned long) uflags));
		}
		duk_dup(thr, idx_desc);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_VALUE);
	}
	if (uflags & DUK_DEFPROP_HAVE_WRITABLE) {
		duk_push_boolean(thr, uflags & DUK_DEFPROP_WRITABLE);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_WRITABLE);
	}
	if (uflags & DUK_DEFPROP_HAVE_ENUMERABLE) {
		duk_push_boolean(thr, uflags & DUK_DEFPROP_ENUMERABLE);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_ENUMERABLE);
	}
	if (uflags & DUK_DEFPROP_HAVE_CONFIGURABLE) {
		duk_push_boolean(thr, uflags & DUK_DEFPROP_CONFIGURABLE);
		duk_xdef_prop_stridx_short_wec(thr, -2, DUK_STRIDX_CONFIGURABLE);
	}
}

/* [[HasProperty]] followed by [[Get]] if property exists.  This specific
 * sequence is visible for e.g. Proxies.
 */
DUK_LOCAL duk_bool_t duk__prop_has_get_prop_stridx(duk_hthread *thr, duk_idx_t obj_idx, duk_small_uint_t stridx) {
	if (!duk_has_prop_stridx(thr, obj_idx, stridx)) {
		return 0;
	}
	(void) duk_get_prop_stridx(thr, obj_idx, stridx);
	return 1;
}

DUK_LOCAL duk_bool_t duk__prop_has_get_prop_stridx_toboolean(duk_hthread *thr,
                                                             duk_idx_t obj_idx,
                                                             duk_small_uint_t stridx,
                                                             duk_bool_t *out_bool) {
	if (!duk__prop_has_get_prop_stridx(thr, obj_idx, stridx)) {
		return 0;
	}
	*out_bool = duk_to_boolean(thr, -1);
	duk_pop_known(thr);
	return 1;
}

DUK_LOCAL duk_bool_t duk__prop_getset_check_and_promote(duk_hthread *thr) {
	/* NOTE: lightfuncs are coerced to full functions because
	 * lightfuncs don't fit into a property value slot.  This
	 * has some side effects, see test-dev-lightfunc-accessor.js.
	 */
	if (duk_is_callable(thr, -1)) {
		(void) duk_get_hobject_promote_lfunc(thr, -1);
	} else if (duk_is_undefined(thr, -1)) {
		;
	} else {
		return 0;
	}
	return 1;
}

/* ToPropertyDescriptor(): convert an ECMAScript descriptor object into
 * a specification Property Descriptor type (0-2 value stack entries and
 * attributes).
 *
 * Supports partial descriptors.
 *
 * [ ... desc_obj ] -> [ ... <none|value|get set ] + defprop_flags
 */
DUK_INTERNAL duk_small_uint_t duk_prop_topropdesc(duk_hthread *thr) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t entry_top = duk_get_top(thr);
	duk_idx_t val_count = 0;
#endif
	duk_idx_t obj_idx;
	duk_bool_t flag;
	duk_small_uint_t attrs = 0U;

	obj_idx = duk_require_normalize_index(thr, -1);
	duk_require_object(thr, -1);

	/* The checks and coercions may have arbitrary side effects (for
	 * example, the input value may be a Proxy) so all steps must be
	 * in the exact specification order.  Also must check for property
	 * existence ([[HasProperty]]) before reading the property value,
	 * as this side effect is visible for a Proxy.
	 */

	if (duk__prop_has_get_prop_stridx_toboolean(thr, obj_idx, DUK_STRIDX_ENUMERABLE, &flag)) {
		attrs |= (flag ? (DUK_DEFPROP_HAVE_ENUMERABLE | DUK_DEFPROP_ENUMERABLE) : (DUK_DEFPROP_HAVE_ENUMERABLE));
	}

	if (duk__prop_has_get_prop_stridx_toboolean(thr, obj_idx, DUK_STRIDX_CONFIGURABLE, &flag)) {
		attrs |= (flag ? (DUK_DEFPROP_HAVE_CONFIGURABLE | DUK_DEFPROP_CONFIGURABLE) : (DUK_DEFPROP_HAVE_CONFIGURABLE));
	}

	if (duk__prop_has_get_prop_stridx(thr, obj_idx, DUK_STRIDX_VALUE)) {
		attrs |= DUK_DEFPROP_HAVE_VALUE;
#if defined(DUK_USE_ASSERTIONS)
		val_count++;
#endif
	}

	if (duk__prop_has_get_prop_stridx_toboolean(thr, obj_idx, DUK_STRIDX_WRITABLE, &flag)) {
		attrs |= (flag ? (DUK_DEFPROP_HAVE_WRITABLE | DUK_DEFPROP_WRITABLE) : (DUK_DEFPROP_HAVE_WRITABLE));
	}

	if (duk__prop_has_get_prop_stridx(thr, obj_idx, DUK_STRIDX_GET)) {
		if (!duk__prop_getset_check_and_promote(thr)) {
			goto invalid_desc;
		}
#if defined(DUK_USE_ASSERTIONS)
		val_count++;
#endif
		attrs |= DUK_DEFPROP_HAVE_GETTER;
	}

	if (duk__prop_has_get_prop_stridx(thr, obj_idx, DUK_STRIDX_SET)) {
		if (!duk__prop_getset_check_and_promote(thr)) {
			goto invalid_desc;
		}
#if defined(DUK_USE_ASSERTIONS)
		val_count++;
#endif
		attrs |= DUK_DEFPROP_HAVE_SETTER;
	}

	if (attrs & (DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER)) {
		if (attrs & (DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_HAVE_WRITABLE)) {
			goto invalid_desc;
		}
	}

	duk_remove(thr, obj_idx);

#if defined(DUK_USE_ASSERTIONS)
	DUK_ASSERT(duk_get_top(thr) == entry_top - 1 + val_count);
#endif
	return attrs;

invalid_desc:
	DUK_ERROR_TYPE(thr, DUK_STR_INVALID_DESCRIPTOR);
	DUK_WO_NORETURN(return attrs;);
}

DUK_INTERNAL duk_bool_t duk_prop_arguments_map_prep(duk_hthread *thr,
                                                    duk_hobject *obj,
                                                    duk_hobject **out_map,
                                                    duk_hobject **out_env) {
	duk_hobject *map;
	duk_hobject *env;

	if (!DUK_HOBJECT_HAS_EXOTIC_ARGUMENTS(obj)) {
		return 0;
	}
	map = duk_hobject_lookup_strprop_known_hobject(thr, obj, DUK_HTHREAD_STRING_INT_MAP(thr));
	if (map == NULL) {
		return 0;
	}
	*out_map = map;
	env = duk_hobject_lookup_strprop_known_hobject(thr, obj, DUK_HTHREAD_STRING_INT_VARENV(thr));
	if (env == NULL) {
		return 0;
	}
	*out_env = env;
	return 1;
}

DUK_INTERNAL duk_hstring *duk_prop_arguments_map_prep_idxkey(duk_hthread *thr,
                                                             duk_hobject *obj,
                                                             duk_uarridx_t idx,
                                                             duk_hobject **out_map,
                                                             duk_hobject **out_env) {
	duk_bool_t rc;
	duk_uint_fast32_t ent_idx;
	duk_propvalue *pv;
	duk_tval *tv_varname;
	duk_hstring *varname;

	if (!duk_prop_arguments_map_prep(thr, obj, out_map, out_env)) {
		return NULL;
	}
	rc = duk_hobject_lookup_idxprop_index(thr, *out_map, idx, &ent_idx);
	if (!rc) {
		return NULL;
	}
	DUK_ASSERT((*out_map)->idx_props != NULL);
	pv = ((duk_propvalue *) (*out_map)->idx_props) + ent_idx;
	tv_varname = &pv->v;
	DUK_ASSERT(DUK_TVAL_IS_STRING(tv_varname));
	varname = DUK_TVAL_GET_STRING(tv_varname);

	return varname;
}

DUK_INTERNAL void duk_prop_pop_propdesc(duk_hthread *thr, duk_small_int_t attrs) {
	if (attrs >= 0) {
		duk_small_uint_t uattrs = (duk_small_uint_t) attrs;
		duk_pop_n(thr, (uattrs & DUK_PROPDESC_FLAG_ACCESSOR) ? 2 : 1);
	}
}

DUK_INTERNAL duk_small_uint_t duk_prop_propdesc_valcount(duk_small_int_t attrs) {
	if (attrs >= 0) {
		duk_small_uint_t uattrs = (duk_small_uint_t) attrs;
		return (uattrs & DUK_PROPDESC_FLAG_ACCESSOR) ? 2 : 1;
	}
	return 0;
}

DUK_INTERNAL duk_hobject *duk_prop_switch_stabilized_target_top(duk_hthread *thr, duk_hobject *target, duk_hobject *next) {
	duk_tval *tv_target;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(target != NULL);
	DUK_ASSERT(next != NULL);
	DUK_ASSERT(duk_get_top(thr) > 0U);

	tv_target = thr->valstack_top - 1;
	DUK_ASSERT(DUK_TVAL_IS_OBJECT(tv_target));
	DUK_ASSERT(DUK_TVAL_GET_OBJECT(tv_target) == target);
	DUK_HOBJECT_INCREF(thr, next);
	DUK_TVAL_UPDATE_OBJECT(tv_target, next);
	DUK_HOBJECT_DECREF(thr, target);
	return next;
}
