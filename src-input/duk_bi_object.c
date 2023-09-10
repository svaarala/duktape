/*
 *  Object built-ins
 */

#include "duk_internal.h"

/* Needed even when Object built-in disabled. */
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_to_string(duk_hthread *thr) {
	duk_tval *tv;

	tv = DUK_HTHREAD_THIS_PTR(thr);
	duk_push_objproto_tostring_tval(thr, tv, 0 /*avoid_side_effects*/);
	return 1;
}

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor(duk_hthread *thr) {
	duk_uint_t arg_mask;

	arg_mask = duk_get_type_mask(thr, 0);

	if (!duk_is_constructor_call(thr) && /* not a constructor call */
	    ((arg_mask & (DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_UNDEFINED)) == 0)) { /* and argument not null or undefined */
		duk_to_object(thr, 0);
		return 1;
	}

	/* Pointer and buffer primitive values are treated like other
	 * primitives values which have a fully fledged object counterpart:
	 * promote to an object value.  Lightfuncs and plain buffers are
	 * coerced with ToObject() even they could also be returned as is.
	 */
	if (arg_mask & (DUK_TYPE_MASK_OBJECT | DUK_TYPE_MASK_STRING | DUK_TYPE_MASK_BOOLEAN | DUK_TYPE_MASK_NUMBER |
	                DUK_TYPE_MASK_POINTER | DUK_TYPE_MASK_BUFFER | DUK_TYPE_MASK_LIGHTFUNC)) {
		/* For DUK_TYPE_OBJECT the coercion is a no-op and could
		 * be checked for explicitly, but Object(obj) calls are
		 * not very common so opt for minimal footprint.
		 */
		duk_to_object(thr, 0);
		return 1;
	}

	(void) duk_push_object_helper(thr,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE | DUK_HOBJECT_FLAG_FASTREFS |
	                                  DUK_HEAPHDR_HTYPE_AS_FLAGS(DUK_HTYPE_OBJECT),
	                              DUK_BIDX_OBJECT_PROTOTYPE);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) && defined(DUK_USE_ES6)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_assign(duk_hthread *thr) {
	duk_idx_t nargs;
	duk_int_t idx;

	nargs = duk_get_top_require_min(thr, 1 /*min_top*/);

	duk_to_object(thr, 0);
	for (idx = 1; idx < nargs; idx++) {
		/* E7 19.1.2.1 (step 4a) */
		if (duk_is_nullish(thr, idx)) {
			continue;
		}

		/* duk_enum() respects ES2015+ [[OwnPropertyKeys]] ordering, which is
		 * convenient here.
		 */
		duk_to_object(thr, idx);
		duk_enum(thr, idx, DUK_ENUM_OWN_PROPERTIES_ONLY);
		while (duk_next(thr, -1, 1 /*get_value*/)) {
			/* [ target ... enum key value ] */
			duk_put_prop(thr, 0);
			/* [ target ... enum ] */
		}
		/* pop the enumerator, because otherwise a large number of argumens will exhaust the valstack */
		duk_pop_known(thr);
	}

	duk_set_top(thr, 1);
	return 1;
}
#endif

#if defined(DUK_USE_OBJECT_BUILTIN) && defined(DUK_USE_ES6)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is(duk_hthread *thr) {
	DUK_ASSERT_TOP(thr, 2);
	return duk_push_boolean_return1(thr, duk_samevalue(thr, 0, 1));
}
#endif

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_create(duk_hthread *thr) {
	duk_hobject *proto;

	DUK_ASSERT_TOP(thr, 2);

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	duk_hbufobj_promote_plain(thr, 0);
#endif
	proto = duk_require_hobject_accept_mask(thr, 0, DUK_TYPE_MASK_NULL);
	DUK_ASSERT(proto != NULL || duk_is_null(thr, 0));

	(void) duk_push_object_helper_proto(thr,
	                                    DUK_HOBJECT_FLAG_EXTENSIBLE | DUK_HOBJECT_FLAG_FASTREFS |
	                                        DUK_HEAPHDR_HTYPE_AS_FLAGS(DUK_HTYPE_OBJECT),
	                                    proto);

	if (!duk_is_undefined(thr, 1)) {
		/* [ O Properties obj ] */

		duk_replace(thr, 0);

		/* [ obj Properties ] */

		/* Just call the "original" Object.defineProperties() to
		 * finish up.
		 */

		return duk_bi_object_constructor_define_properties(thr);
	}

	/* [ O Properties obj ] */

	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_define_properties(duk_hthread *thr) {
	duk_small_uint_t pass;
	duk_uint_t defprop_flags;
	duk_hobject *obj;

	DUK_ASSERT_TOP(thr, 2);

	/* Lightfunc and plain buffer handling by ToObject() coercion. */
	obj = duk_require_hobject_promote_mask(thr, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	DUK_ASSERT(obj != NULL);

	duk_to_object(thr, 1); /* properties object */

	DUK_DDD(DUK_DDDPRINT("target=%!iT, properties=%!iT", (duk_tval *) duk_get_tval(thr, 0), (duk_tval *) duk_get_tval(thr, 1)));

	/*
	 *  Two pass approach to processing the property descriptors.
	 *  On first pass validate and normalize all descriptors before
	 *  any changes are made to the target object.  On second pass
	 *  make the actual modifications to the target object.  This
	 *  is not fully compliant because the two passes are visible
	 *  as side effects (e.g. if the properties object is a Proxy).
	 *
	 *  Right now we'll just use the same normalize/validate helper
	 *  on both passes, ignoring its outputs on the first pass.
	 */

	for (pass = 0; pass < 2; pass++) {
		duk_set_top(thr, 2); /* -> [ hobject props ] */
		duk_enum(thr, 1, DUK_ENUM_OWN_PROPERTIES_ONLY | DUK_ENUM_INCLUDE_SYMBOLS /*enum_flags*/);

		DUK_ASSERT_TOP(thr, 3);

		for (;;) {
			/* [ hobject props enum(props) ] */

			duk_set_top(thr, 3);

			if (!duk_next(thr, 2, 1 /*get_value*/)) {
				break;
			}

			DUK_DDD(DUK_DDDPRINT("-> key=%!iT, desc=%!iT",
			                     (duk_tval *) duk_get_tval(thr, -2),
			                     (duk_tval *) duk_get_tval(thr, -1)));

			/* [ hobject props enum(props) key desc ] */

			defprop_flags = duk_prop_topropdesc(thr) | DUK_DEFPROP_THROW;

			/* [ hobject props enum(props) key <variable> ] */

			if (pass == 0) {
				continue;
			}

			duk_prop_defown(thr, obj, DUK_GET_TVAL_POSIDX(thr, 3), 4 /*idx_desc*/, defprop_flags);
		}
	}

	/*
	 *  Return target object
	 */

	duk_dup_0(thr);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_seal_freeze_shared(duk_hthread *thr) {
	DUK_ASSERT_TOP(thr, 1);

	duk_seal_freeze_raw(thr, 0, (duk_bool_t) duk_get_current_magic(thr) /*is_freeze*/);
	DUK_ASSERT_TOP(thr, 1);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is_sealed_frozen_shared(duk_hthread *thr) {
	duk_hobject *h;
	duk_bool_t is_frozen;
	duk_uint_t mask;

	is_frozen = (duk_bool_t) duk_get_current_magic(thr);
	mask = duk_get_type_mask(thr, 0);
	if (mask & (DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER)) {
		/* Zero-size buffers are always frozen and sealed.  Non-zero-size
		 * buffers are sealed but not frozen (index props writable).
		 */
		duk_bool_t res;

		DUK_ASSERT(is_frozen == 0 || is_frozen == 1);
		if (mask & DUK_TYPE_MASK_LIGHTFUNC) {
			/* Lightfunc always frozen and sealed. */
			res = 1;
		} else {
			duk_hbuffer *h_buf = duk_require_hbuffer(thr, 0);
			DUK_ASSERT(h_buf != NULL);
			if (DUK_HBUFFER_GET_SIZE(h_buf) == 0) {
				res = 1;
			} else {
				res = (is_frozen ^ 1); /* Buffer sealed but not frozen (index props writable). */
			}
		}
		return duk_push_boolean_return1(thr, res);
	} else {
		/* ES2015 Sections 19.1.2.12, 19.1.2.13: anything other than an object
		 * is considered to be already sealed and frozen.
		 */
		h = duk_get_hobject(thr, 0);
		return duk_push_boolean_return1(thr,
		                                (h == NULL) ||
		                                    duk_hobject_object_is_sealed_frozen_helper(thr, h, is_frozen /*is_frozen*/));
	}
	/* never here */
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_to_locale_string(duk_hthread *thr) {
	DUK_ASSERT_TOP(thr, 0);
	(void) duk_push_this_coercible_to_object(thr);
	duk_get_prop_stridx_short(thr, 0, DUK_STRIDX_TO_STRING);
#if 0 /* This is mentioned explicitly in the E5.1 spec, but duk_call_method() checks for it in practice. */
	duk_require_callable(thr, 1);
#endif
	duk_dup_0(thr); /* -> [ O toString O ] */
	duk_call_method(thr, 0); /* XXX: call method tail call? */
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_value_of(duk_hthread *thr) {
	/* For lightfuncs and plain buffers, returns Object() coerced. */
	(void) duk_push_this_coercible_to_object(thr);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_is_prototype_of(duk_hthread *thr) {
	duk_hobject *h_v;
	duk_hobject *h_obj;

	DUK_ASSERT_TOP(thr, 1);

	h_v = duk_get_hobject(thr, 0);
	if (!h_v) {
		duk_push_false(thr); /* XXX: tail call: return duk_push_false(thr) */
		return 1;
	}

	h_obj = duk_push_this_coercible_to_object(thr);
	DUK_ASSERT(h_obj != NULL);

	/* E5.1 Section 15.2.4.6, step 3.a, lookup proto once before compare.
	 * Prototype loops should cause an error to be thrown.
	 */
	return duk_push_boolean_return1(
	    thr,
	    duk_hobject_prototype_chain_contains(thr, duk_hobject_get_proto_raw(thr->heap, h_v), h_obj, 0 /*ignore_loop*/));
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_has_own_property(duk_hthread *thr) {
	duk_small_int_t rc;
	duk_hobject *obj;

	duk_to_property_key(thr, 0); /* Must happen first for correct side effect order. */
	duk_push_this(thr);
	obj = duk_to_hobject(thr, -1);
	DUK_ASSERT(obj != NULL);

	rc = duk_prop_getowndesc_obj_tvkey(thr, obj, duk_require_tval(thr, 0));
	duk_push_boolean(thr, rc >= 0);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_property_is_enumerable(duk_hthread *thr) {
	duk_small_int_t attrs;
	duk_hobject *obj;

	duk_to_property_key(thr, 0);
	duk_push_this(thr);
	obj = duk_to_hobject(thr, -1);
	attrs = duk_prop_getowndesc_obj_tvkey(thr, obj, duk_require_tval(thr, 0));
	duk_push_boolean(thr, attrs >= 0 && (attrs & DUK_PROPDESC_FLAG_ENUMERABLE));
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
/* Shared helper to implement Object.getPrototypeOf,
 * Object.prototype.__proto__ getter, and Reflect.getPrototypeOf.
 *
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-get-object.prototype.__proto__
 */
DUK_INTERNAL duk_ret_t duk_bi_object_getprototype_shared(duk_hthread *thr) {
	/*
	 *  magic = 0: __proto__ getter
	 *  magic = 1: Object.getPrototypeOf()
	 *  magic = 2: Reflect.getPrototypeOf()
	 */

	duk_hobject *h;
	duk_hobject *proto;
	duk_tval *tv;
	duk_int_t magic;

	magic = duk_get_current_magic(thr);

	if (magic == 0) {
		DUK_ASSERT_TOP(thr, 0);
		duk_push_this_coercible_to_object(thr);
	}
	DUK_ASSERT(duk_get_top(thr) >= 1);
	if (magic < 2) {
		/* ES2015 Section 19.1.2.9, step 1 */
		duk_to_object(thr, 0);
	}
	tv = DUK_GET_TVAL_POSIDX(thr, 0);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_BUFFER:
		proto = thr->builtins[DUK_BIDX_UINT8ARRAY_PROTOTYPE];
		break;
	case DUK_TAG_LIGHTFUNC:
		proto = thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE];
		break;
	case DUK_TAG_OBJECT:
		h = DUK_TVAL_GET_OBJECT(tv);
		proto = duk_hobject_get_proto_raw(thr->heap, h);
		break;
	default:
		/* This implicitly handles CheckObjectCoercible() caused
		 * TypeError.
		 */
		DUK_DCERROR_TYPE_INVALID_ARGS(thr);
	}
	duk_push_hobject_or_null(thr, proto);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
/* Shared helper to implement ES2015 Object.setPrototypeOf,
 * Object.prototype.__proto__ setter, and Reflect.setPrototypeOf.
 *
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-get-object.prototype.__proto__
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-object.setprototypeof
 */
DUK_INTERNAL duk_ret_t duk_bi_object_setprototype_shared(duk_hthread *thr) {
	/*
	 *  magic = 0: __proto__ setter
	 *  magic = 1: Object.setPrototypeOf()
	 *  magic = 2: Reflect.setPrototypeOf()
	 */

	duk_hobject *h_obj;
	duk_hobject *h_new_proto;
	duk_hobject *h_curr;
	duk_ret_t ret_success = 1; /* retval for success path */
	duk_uint_t mask;
	duk_int_t magic;

	/* Preliminaries for __proto__ and setPrototypeOf (E6 19.1.2.18 steps 1-4). */
	magic = duk_get_current_magic(thr);
	if (magic == 0) {
		duk_push_this_check_object_coercible(thr);
		duk_insert(thr, 0);
		if (!duk_check_type_mask(thr, 1, DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_OBJECT)) {
			return 0;
		}

		/* __proto__ setter returns 'undefined' on success unlike the
		 * setPrototypeOf() call which returns the target object.
		 */
		ret_success = 0;
	} else {
		if (magic == 1) {
			duk_require_object_coercible(thr, 0);
		} else {
			duk_require_hobject_accept_mask(thr, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
		}
		duk_require_type_mask(thr, 1, DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_OBJECT);
	}

	h_new_proto = duk_get_hobject(thr, 1);
	/* h_new_proto may be NULL */

	mask = duk_get_type_mask(thr, 0);
	if (mask & (DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER)) {
		duk_hobject *curr_proto;
		curr_proto =
		    thr->builtins[(mask & DUK_TYPE_MASK_LIGHTFUNC) ? DUK_BIDX_FUNCTION_PROTOTYPE : DUK_BIDX_UINT8ARRAY_PROTOTYPE];
		if (h_new_proto == curr_proto) {
			goto skip;
		}
		goto fail_nonextensible;
	}
	h_obj = duk_get_hobject(thr, 0);
	if (h_obj == NULL) {
		goto skip;
	}
	DUK_ASSERT(h_obj != NULL);

	if (!duk_js_setprototypeof(thr, h_obj, h_new_proto)) {
		goto fail_nonextensible;
	}

	/* fall through */

skip:
	duk_set_top(thr, 1);
	if (magic == 2) {
		duk_push_true(thr);
	}
	return ret_success;

fail_nonextensible:
	if (magic != 2) {
		DUK_DCERROR_TYPE_INVALID_ARGS(thr);
	} else {
		duk_push_false(thr);
		return 1;
	}
	return 0;
}
#endif /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_define_property(duk_hthread *thr) {
	/*
	 *  magic = 0: Object.defineProperty()
	 *  magic = 1: Reflect.defineProperty()
	 */

	duk_hobject *obj;
	duk_idx_t idx_value;
	duk_uint_t defprop_flags;
	duk_small_uint_t magic;
	duk_bool_t throw_flag;
	duk_bool_t ret;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT_TOP(thr, 3);

	DUK_DDD(DUK_DDDPRINT("Object.defineProperty(): ctx=%p obj=%!T key=%!T desc=%!T",
	                     (void *) thr,
	                     (duk_tval *) duk_get_tval(thr, 0),
	                     (duk_tval *) duk_get_tval(thr, 1),
	                     (duk_tval *) duk_get_tval(thr, 2)));

	/* [ obj key desc ] */

	magic = (duk_small_uint_t) duk_get_current_magic(thr);

	/* Lightfuncs are currently supported by coercing to a temporary
	 * Function object; changes will be allowed (the coerced value is
	 * extensible) but will be lost.  Same for plain buffers.
	 */
	obj = duk_require_hobject_promote_mask(thr, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(duk_get_top(thr) == 3);
	defprop_flags = duk_prop_topropdesc(thr); /* -> [ obj key <variable> */
	DUK_ASSERT(magic == 0U || magic == 1U);
	if (magic == 0U) {
		defprop_flags |= DUK_DEFPROP_THROW;
	}
	ret = duk_prop_defown(thr, obj, DUK_GET_TVAL_POSIDX(thr, 1), 2 /*idx_desc*/, defprop_flags);

	/* Ignore the property descriptor conversion outputs on the value stack,
	 * they're popped automatically.
	 */

	if (magic == 0U) {
		/* Object.defineProperty(): return target object. */
		duk_push_hobject(thr, obj);
	} else {
		/* Reflect.defineProperty(): return success/fail. */
		duk_push_boolean(thr, ret);
	}
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_get_own_property_descriptor(duk_hthread *thr) {
	duk_hobject *obj;
	duk_small_int_t attrs;

	DUK_ASSERT_TOP(thr, 2);

	/* ES2015 Section 19.1.2.6, step 1 */
	if (duk_get_current_magic(thr) == 0) {
		duk_to_object(thr, 0);
	}
	/* [ obj key ] */

	obj = duk_require_hobject(thr, 0);
	attrs = duk_prop_getowndesc_obj_tvkey(thr, obj, duk_require_tval(thr, 1));
#if 0
	if (attrs < 0) {
		return 0;
	}
#endif
	duk_prop_frompropdesc_propattrs(thr, attrs);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is_extensible(duk_hthread *thr) {
	/*
	 *  magic = 0: Object.isExtensible()
	 *  magic = 1: Reflect.isExtensible()
	 */

	duk_hobject *h;

	if (duk_get_current_magic(thr) == 0) {
		h = duk_get_hobject(thr, 0);
	} else {
		/* Reflect.isExtensible(): throw if non-object, but we accept lightfuncs
		 * and plain buffers here because they pretend to be objects.
		 */
		h = duk_require_hobject_accept_mask(thr, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	}

	return duk_push_boolean_return1(thr, (h != NULL) && duk_js_isextensible(thr, h));
}
#endif /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
/* Shared helper for various key/symbol listings, magic:
 * 0=Object.keys()
 * 1=Object.getOwnPropertyNames(),
 * 2=Object.getOwnPropertySymbols(),
 * 3=Reflect.ownKeys()
 */
DUK_LOCAL const duk_uint8_t duk__object_keys_ownpropkeys_flags[4] = {
	/* Object.keys() */
	DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX | DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING | DUK_OWNPROPKEYS_FLAG_REQUIRE_ENUMERABLE |
	    DUK_OWNPROPKEYS_FLAG_STRING_COERCE,

	/* Object.getOwnPropertyNames() */
	DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX | DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING | DUK_OWNPROPKEYS_FLAG_STRING_COERCE,

	/* Object.getOwnPropertySymbols() */
	DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL | DUK_OWNPROPKEYS_FLAG_STRING_COERCE, /* not necessary */

	/* Reflect.ownKeys() */
	DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX | DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING | DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL |
	    DUK_OWNPROPKEYS_FLAG_STRING_COERCE
};

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_keys_shared(duk_hthread *thr) {
	duk_hobject *obj;
	duk_int_t magic;
	duk_uint_t ownpropkeys_flags;

	DUK_ASSERT_TOP(thr, 1);

	magic = duk_get_current_magic(thr);
	if (magic == 3) {
		/* ES2015 Section 26.1.11 requires a TypeError for non-objects.  Lightfuncs
		 * and plain buffers pretend to be objects, so accept those too.
		 */
		obj = duk_require_hobject_promote_mask(thr, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	} else {
		/* ES2015: ToObject coerce. */
		obj = duk_to_hobject(thr, 0);
	}
	DUK_ASSERT(obj != NULL);

	DUK_ASSERT(magic >= 0 && magic < (duk_int_t) (sizeof(duk__object_keys_ownpropkeys_flags) / sizeof(duk_uint8_t)));
	ownpropkeys_flags = duk__object_keys_ownpropkeys_flags[magic];
	duk_prop_ownpropkeys(thr, obj, ownpropkeys_flags);
	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_prevent_extensions(duk_hthread *thr) {
	/*
	 *  magic = 0: Object.preventExtensions()
	 *  magic = 1: Reflect.preventExtensions()
	 */

	duk_hobject *h;
	duk_uint_t mask;
	duk_int_t magic;
	duk_bool_t rc;

	magic = duk_get_current_magic(thr);

	/* Silent success for lightfuncs and plain buffers always. */
	mask = DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER;

	/* Object.preventExtensions() silent success for non-object. */
	if (magic == 0) {
		mask |= DUK_TYPE_MASK_UNDEFINED | DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_BOOLEAN | DUK_TYPE_MASK_NUMBER |
		        DUK_TYPE_MASK_STRING | DUK_TYPE_MASK_POINTER;
	}

	if (duk_check_type_mask(thr, 0, mask)) {
		/* Not an object, already non-extensible so always success. */
		rc = 1;
	} else {
		h = duk_require_hobject(thr, 0);
		DUK_ASSERT(h != NULL);

		rc = duk_js_preventextensions(thr, h);
	}

	if (magic == 0) {
		if (rc == 0) {
			DUK_ERROR_TYPE(thr, DUK_STR_CANNOT_PREVENT_EXTENSIONS);
		}
		/* Stack top contains object for success case. */
	} else {
		duk_push_boolean(thr, rc);
	}

	return 1;
}
#endif /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

/*
 *  __defineGetter__, __defineSetter__, __lookupGetter__, __lookupSetter__
 */

#if defined(DUK_USE_ES8)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_defineaccessor(duk_hthread *thr) {
	duk_push_this(thr);
	duk_insert(thr, 0);
	duk_to_object(thr, 0);
	duk_require_callable(thr, 2);

	/* [ ToObject(this) key getter/setter ] */

	/* ToPropertyKey() coercion is not needed, duk_def_prop() does it. */
	duk_def_prop(thr,
	             0,
	             DUK_DEFPROP_SET_ENUMERABLE | DUK_DEFPROP_SET_CONFIGURABLE |
	                 (duk_get_current_magic(thr) ? DUK_DEFPROP_HAVE_SETTER : DUK_DEFPROP_HAVE_GETTER));
	return 0;
}
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_lookupaccessor(duk_hthread *thr) {
	duk_uint_t sanity;

	duk_push_this(thr);
	duk_to_object(thr, -1);

	/* XXX: Prototype walk (with sanity) should be a core property
	 * operation, could add a flag to e.g. duk_get_prop_desc().
	 */

	/* ToPropertyKey() coercion is not needed, duk_get_prop_desc() does it. */
	sanity = DUK_HOBJECT_PROTOTYPE_CHAIN_SANITY;
	while (!duk_is_undefined(thr, -1)) {
		/* [ key obj ] */
		duk_dup(thr, 0);
		duk_get_prop_desc(thr, 1, 0 /*flags*/);
		if (!duk_is_undefined(thr, -1)) {
			duk_get_prop_stridx(thr, -1, (duk_get_current_magic(thr) != 0 ? DUK_STRIDX_SET : DUK_STRIDX_GET));
			return 1;
		}
		duk_pop(thr);

		if (DUK_UNLIKELY(--sanity == 0)) {
			DUK_ERROR_RANGE_PROTO_SANITY(thr);
			DUK_WO_NORETURN(return 0;);
		}

		duk_get_prototype(thr, -1);
		duk_remove(thr, -2);
	}
	return 1;
}
#endif /* DUK_USE_ES8 */
