/*
 *  Object built-ins
 */

#include "duk_internal.h"

/* Needed even when Object built-in disabled. */
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_to_string(duk_context *ctx) {
	duk_tval *tv;
	tv = DUK_HTHREAD_THIS_PTR((duk_hthread *) ctx);
	/* XXX: This is not entirely correct anymore; in ES2015 the
	 * default lookup should use @@toStringTag to come up with
	 * e.g. [object Symbol].
	 */
	duk_push_class_string_tval(ctx, tv);
	return 1;
}

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor(duk_context *ctx) {
	duk_uint_t arg_mask;

	arg_mask = duk_get_type_mask(ctx, 0);

	if (!duk_is_constructor_call(ctx) &&  /* not a constructor call */
	    ((arg_mask & (DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_UNDEFINED)) == 0)) {  /* and argument not null or undefined */
		duk_to_object(ctx, 0);
		return 1;
	}

	/* Pointer and buffer primitive values are treated like other
	 * primitives values which have a fully fledged object counterpart:
	 * promote to an object value.  Lightfuncs and plain buffers are
	 * coerced with ToObject() even they could also be returned as is.
	 */
	if (arg_mask & (DUK_TYPE_MASK_OBJECT |
	                DUK_TYPE_MASK_STRING |
	                DUK_TYPE_MASK_BOOLEAN |
	                DUK_TYPE_MASK_NUMBER |
	                DUK_TYPE_MASK_POINTER |
	                DUK_TYPE_MASK_BUFFER |
	                DUK_TYPE_MASK_LIGHTFUNC)) {
		/* For DUK_TYPE_OBJECT the coercion is a no-op and could
		 * be checked for explicitly, but Object(obj) calls are
		 * not very common so opt for minimal footprint.
		 */
		duk_to_object(ctx, 0);
		return 1;
	}

	(void) duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                              DUK_BIDX_OBJECT_PROTOTYPE);
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) && defined(DUK_USE_ES6)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_assign(duk_context *ctx) {
	duk_idx_t nargs;
	duk_int_t idx;

	nargs = duk_get_top_require_min(ctx, 1 /*min_top*/);

	duk_to_object(ctx, 0);
	for (idx = 1; idx < nargs; idx++) {
		/* E7 19.1.2.1 (step 4a) */
		if (duk_is_null_or_undefined(ctx, idx)) {
			continue;
		}

		/* duk_enum() respects ES2015+ [[OwnPropertyKeys]] ordering, which is
		 * convenient here.
		 */
		duk_to_object(ctx, idx);
		duk_enum(ctx, idx, DUK_ENUM_OWN_PROPERTIES_ONLY);
		while (duk_next(ctx, -1, 1 /*get_value*/)) {
			/* [ target ... enum key value ] */
			duk_put_prop(ctx, 0);
			/* [ target ... enum ] */
		}
		/* Could pop enumerator, but unnecessary because of duk_set_top()
		 * below.
		 */
	}

	duk_set_top(ctx, 1);
	return 1;
}
#endif

#if defined(DUK_USE_OBJECT_BUILTIN) && defined(DUK_USE_ES6)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 2);
	duk_push_boolean(ctx, duk_samevalue(ctx, 0, 1));
	return 1;
}
#endif

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_create(duk_context *ctx) {
	duk_hobject *proto;

	DUK_ASSERT_TOP(ctx, 2);

#if defined(DUK_USE_BUFFEROBJECT_SUPPORT)
	duk_hbufobj_promote_plain(ctx, 0);
#endif
	proto = duk_require_hobject_accept_mask(ctx, 0, DUK_TYPE_MASK_NULL);
	DUK_ASSERT(proto != NULL || duk_is_null(ctx, 0));

	(void) duk_push_object_helper_proto(ctx,
	                                    DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                    DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                                    proto);

	if (!duk_is_undefined(ctx, 1)) {
		/* [ O Properties obj ] */

		duk_replace(ctx, 0);

		/* [ obj Properties ] */

		/* Just call the "original" Object.defineProperties() to
		 * finish up.
		 */

		return duk_bi_object_constructor_define_properties(ctx);
	}

	/* [ O Properties obj ] */

	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_define_properties(duk_context *ctx) {
	duk_small_uint_t pass;
	duk_uint_t defprop_flags;
	duk_hobject *obj;
	duk_idx_t idx_value;
	duk_hobject *get;
	duk_hobject *set;

	/* Lightfunc and plain buffer handling by ToObject() coercion. */
	obj = duk_require_hobject_promote_mask(ctx, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	DUK_ASSERT(obj != NULL);

	duk_to_object(ctx, 1);        /* properties object */

	DUK_DDD(DUK_DDDPRINT("target=%!iT, properties=%!iT",
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1)));

	/*
	 *  Two pass approach to processing the property descriptors.
	 *  On first pass validate and normalize all descriptors before
	 *  any changes are made to the target object.  On second pass
	 *  make the actual modifications to the target object.
	 *
	 *  Right now we'll just use the same normalize/validate helper
	 *  on both passes, ignoring its outputs on the first pass.
	 */

	for (pass = 0; pass < 2; pass++) {
		duk_set_top(ctx, 2);  /* -> [ hobject props ] */
		duk_enum(ctx, 1, DUK_ENUM_OWN_PROPERTIES_ONLY | DUK_ENUM_INCLUDE_SYMBOLS /*enum_flags*/);

		for (;;) {
			duk_hstring *key;

			/* [ hobject props enum(props) ] */

			duk_set_top(ctx, 3);

			if (!duk_next(ctx, 2, 1 /*get_value*/)) {
				break;
			}

			DUK_DDD(DUK_DDDPRINT("-> key=%!iT, desc=%!iT",
			                     (duk_tval *) duk_get_tval(ctx, -2),
			                     (duk_tval *) duk_get_tval(ctx, -1)));

			/* [ hobject props enum(props) key desc ] */

			duk_hobject_prepare_property_descriptor(ctx,
			                                        4 /*idx_desc*/,
			                                        &defprop_flags,
			                                        &idx_value,
			                                        &get,
			                                        &set);

			/* [ hobject props enum(props) key desc [multiple values] ] */

			if (pass == 0) {
				continue;
			}

			/* This allows symbols on purpose. */
			key = duk_known_hstring(ctx, 3);
			DUK_ASSERT(key != NULL);

			duk_hobject_define_property_helper(ctx,
			                                   defprop_flags,
			                                   obj,
			                                   key,
			                                   idx_value,
			                                   get,
			                                   set,
			                                   1 /*throw_flag*/);
		}
	}

	/*
	 *  Return target object
	 */

	duk_dup_0(ctx);
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_seal_freeze_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_bool_t is_freeze;

	DUK_ASSERT_TOP(ctx, 1);

	is_freeze = (duk_bool_t) duk_get_current_magic(ctx);
	if (duk_is_buffer(ctx, 0)) {
		/* Plain buffer: already sealed, but not frozen (and can't be frozen
		 * because index properties can't be made non-writable.
		 */
		if (is_freeze) {
			goto fail_cannot_freeze;
		}
		return 1;
	} else if (duk_is_lightfunc(ctx, 0)) {
		/* Lightfunc: already sealed and frozen, success. */
		return 1;
	}
#if 0
	/* Seal/freeze are quite rare in practice so it'd be nice to get the
	 * correct behavior simply via automatic promotion (at the cost of some
	 * memory churn).  However, the promoted objects don't behave the same,
	 * e.g. promoted lightfuncs are extensible.
	 */
	h = duk_require_hobject_promote_mask(ctx, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
#endif

	h = duk_get_hobject(ctx, 0);
	if (h == NULL) {
		/* ES2015 Sections 19.1.2.5, 19.1.2.17 */
		return 1;
	}

	if (is_freeze && DUK_HOBJECT_IS_BUFOBJ(h)) {
		/* Buffer objects cannot be frozen because there's no internal
		 * support for making virtual array indices non-writable.
		 */
		DUK_DD(DUK_DDPRINT("cannot freeze a buffer object"));
		goto fail_cannot_freeze;
	}

	duk_hobject_object_seal_freeze_helper(thr, h, is_freeze);

	/* Sealed and frozen objects cannot gain any more properties,
	 * so this is a good time to compact them.
	 */
	duk_hobject_compact_props(thr, h);
	return 1;

 fail_cannot_freeze:
	DUK_DCERROR_TYPE_INVALID_ARGS(thr);  /* XXX: proper error message */
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is_sealed_frozen_shared(duk_context *ctx) {
	duk_hobject *h;
	duk_bool_t is_frozen;
	duk_uint_t mask;

	is_frozen = duk_get_current_magic(ctx);
	mask = duk_get_type_mask(ctx, 0);
	if (mask & (DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER)) {
		DUK_ASSERT(is_frozen == 0 || is_frozen == 1);
		duk_push_boolean(ctx, (mask & DUK_TYPE_MASK_LIGHTFUNC) ?
		                          1 :               /* lightfunc always frozen and sealed */
		                          (is_frozen ^ 1)); /* buffer sealed but not frozen (index props writable) */
	} else {
		/* ES2015 Sections 19.1.2.12, 19.1.2.13: anything other than an object
		 * is considered to be already sealed and frozen.
		 */
		h = duk_get_hobject(ctx, 0);
		duk_push_boolean(ctx, (h == NULL) ||
		                      duk_hobject_object_is_sealed_frozen_helper((duk_hthread *) ctx, h, is_frozen /*is_frozen*/));
	}
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_to_locale_string(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 0);
	(void) duk_push_this_coercible_to_object(ctx);
	duk_get_prop_stridx_short(ctx, 0, DUK_STRIDX_TO_STRING);
#if 0  /* This is mentioned explicitly in the E5.1 spec, but duk_call_method() checks for it in practice. */
	duk_require_callable(ctx, 1);
#endif
	duk_dup_0(ctx);  /* -> [ O toString O ] */
	duk_call_method(ctx, 0);  /* XXX: call method tail call? */
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_value_of(duk_context *ctx) {
	/* For lightfuncs and plain buffers, returns Object() coerced. */
	(void) duk_push_this_coercible_to_object(ctx);
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_is_prototype_of(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h_v;
	duk_hobject *h_obj;

	DUK_ASSERT_TOP(ctx, 1);

	h_v = duk_get_hobject(ctx, 0);
	if (!h_v) {
		duk_push_false(ctx);  /* XXX: tail call: return duk_push_false(ctx) */
		return 1;
	}

	h_obj = duk_push_this_coercible_to_object(ctx);
	DUK_ASSERT(h_obj != NULL);

	/* E5.1 Section 15.2.4.6, step 3.a, lookup proto once before compare.
	 * Prototype loops should cause an error to be thrown.
	 */
	duk_push_boolean(ctx, duk_hobject_prototype_chain_contains(thr, DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h_v), h_obj, 0 /*ignore_loop*/));
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_has_own_property(duk_context *ctx) {
	return duk_hobject_object_ownprop_helper(ctx, 0 /*required_desc_flags*/);
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_prototype_property_is_enumerable(duk_context *ctx) {
	return duk_hobject_object_ownprop_helper(ctx, DUK_PROPDESC_FLAG_ENUMERABLE /*required_desc_flags*/);
}
#endif  /* DUK_USE_OBJECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
/* Shared helper to implement Object.getPrototypeOf,
 * Object.prototype.__proto__ getter, and Reflect.getPrototypeOf.
 *
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-get-object.prototype.__proto__
 */
DUK_INTERNAL duk_ret_t duk_bi_object_getprototype_shared(duk_context *ctx) {
	/*
	 *  magic = 0: __proto__ getter
	 *  magic = 1: Object.getPrototypeOf()
	 *  magic = 2: Reflect.getPrototypeOf()
	 */

	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_hobject *proto;
	duk_tval *tv;
	duk_int_t magic;

	magic = duk_get_current_magic(ctx);

	if (magic == 0) {
		DUK_ASSERT_TOP(ctx, 0);
		duk_push_this_coercible_to_object(ctx);
	}
	DUK_ASSERT(duk_get_top(ctx) >= 1);
	if (magic < 2) {
		/* ES2015 Section 19.1.2.9, step 1 */
		duk_to_object(ctx, 0);
	}
	tv = DUK_GET_TVAL_POSIDX(ctx, 0);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_BUFFER:
		proto = thr->builtins[DUK_BIDX_UINT8ARRAY_PROTOTYPE];
		break;
	case DUK_TAG_LIGHTFUNC:
		proto = thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE];
		break;
	case DUK_TAG_OBJECT:
		h = DUK_TVAL_GET_OBJECT(tv);
		proto = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h);
		break;
	default:
		/* This implicitly handles CheckObjectCoercible() caused
		 * TypeError.
		 */
		DUK_DCERROR_TYPE_INVALID_ARGS(thr);
	}
	if (proto != NULL) {
		duk_push_hobject(ctx, proto);
	} else {
		duk_push_null(ctx);
	}
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
/* Shared helper to implement ES2015 Object.setPrototypeOf,
 * Object.prototype.__proto__ setter, and Reflect.setPrototypeOf.
 *
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-get-object.prototype.__proto__
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-object.setprototypeof
 */
DUK_INTERNAL duk_ret_t duk_bi_object_setprototype_shared(duk_context *ctx) {
	/*
	 *  magic = 0: __proto__ setter
	 *  magic = 1: Object.setPrototypeOf()
	 *  magic = 2: Reflect.setPrototypeOf()
	 */

	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h_obj;
	duk_hobject *h_new_proto;
	duk_hobject *h_curr;
	duk_ret_t ret_success = 1;  /* retval for success path */
	duk_uint_t mask;
	duk_int_t magic;

	/* Preliminaries for __proto__ and setPrototypeOf (E6 19.1.2.18 steps 1-4). */
	magic = duk_get_current_magic(ctx);
	if (magic == 0) {
		duk_push_this_check_object_coercible(ctx);
		duk_insert(ctx, 0);
		if (!duk_check_type_mask(ctx, 1, DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_OBJECT)) {
			return 0;
		}

		/* __proto__ setter returns 'undefined' on success unlike the
		 * setPrototypeOf() call which returns the target object.
		 */
		ret_success = 0;
	} else {
		if (magic == 1) {
			duk_require_object_coercible(ctx, 0);
		} else {
			duk_require_hobject_accept_mask(ctx, 0,
			                                DUK_TYPE_MASK_LIGHTFUNC |
			                                DUK_TYPE_MASK_BUFFER);
		}
		duk_require_type_mask(ctx, 1, DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_OBJECT);
	}

	h_new_proto = duk_get_hobject(ctx, 1);
	/* h_new_proto may be NULL */

	mask = duk_get_type_mask(ctx, 0);
	if (mask & (DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER)) {
		duk_hobject *curr_proto;
		curr_proto = thr->builtins[(mask & DUK_TYPE_MASK_LIGHTFUNC) ?
		                               DUK_BIDX_FUNCTION_PROTOTYPE :
		                               DUK_BIDX_UINT8ARRAY_PROTOTYPE];
		if (h_new_proto == curr_proto) {
			goto skip;
		}
		goto fail_nonextensible;
	}
	h_obj = duk_get_hobject(ctx, 0);
	if (h_obj == NULL) {
		goto skip;
	}
	DUK_ASSERT(h_obj != NULL);

	/* [[SetPrototypeOf]] standard behavior, E6 9.1.2. */
	/* TODO: implement Proxy object support here */

	if (h_new_proto == DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h_obj)) {
		goto skip;
	}
	if (!DUK_HOBJECT_HAS_EXTENSIBLE(h_obj)) {
		goto fail_nonextensible;
	}
	for (h_curr = h_new_proto; h_curr != NULL; h_curr = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h_curr)) {
		/* Loop prevention. */
		if (h_curr == h_obj) {
			goto fail_loop;
		}
	}
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h_obj, h_new_proto);
	/* fall thru */

 skip:
	duk_set_top(ctx, 1);
	if (magic == 2) {
		duk_push_true(ctx);
	}
	return ret_success;

 fail_nonextensible:
 fail_loop:
	if (magic != 2) {
		DUK_DCERROR_TYPE_INVALID_ARGS(thr);
	} else {
		duk_push_false(ctx);
		return 1;
	}
}
#endif  /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_define_property(duk_context *ctx) {
	/*
	 *  magic = 0: Object.defineProperty()
	 *  magic = 1: Reflect.defineProperty()
	 */

	duk_hobject *obj;
	duk_hstring *key;
	duk_hobject *get;
	duk_hobject *set;
	duk_idx_t idx_value;
	duk_uint_t defprop_flags;
	duk_int_t magic;
	duk_bool_t throw_flag;
	duk_bool_t ret;

	DUK_ASSERT(ctx != NULL);

	DUK_DDD(DUK_DDDPRINT("Object.defineProperty(): ctx=%p obj=%!T key=%!T desc=%!T",
	                     (void *) ctx,
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1),
	                     (duk_tval *) duk_get_tval(ctx, 2)));

	/* [ obj key desc ] */

	magic = duk_get_current_magic(ctx);

	/* Lightfuncs are currently supported by coercing to a temporary
	 * Function object; changes will be allowed (the coerced value is
	 * extensible) but will be lost.  Same for plain buffers.
	 */
	obj = duk_require_hobject_promote_mask(ctx, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	DUK_ASSERT(obj != NULL);
	key = duk_to_property_key_hstring(ctx, 1);
	(void) duk_require_hobject(ctx, 2);

	DUK_ASSERT(obj != NULL);
	DUK_ASSERT(key != NULL);
	DUK_ASSERT(duk_get_hobject(ctx, 2) != NULL);

	/*
	 *  Validate and convert argument property descriptor (an Ecmascript
	 *  object) into a set of defprop_flags and possibly property value,
	 *  getter, and/or setter values on the value stack.
	 *
	 *  Lightfunc set/get values are coerced to full Functions.
	 */

	duk_hobject_prepare_property_descriptor(ctx,
	                                        2 /*idx_desc*/,
	                                        &defprop_flags,
	                                        &idx_value,
	                                        &get,
	                                        &set);

	/*
	 *  Use Object.defineProperty() helper for the actual operation.
	 */

	DUK_ASSERT(magic == 0 || magic == 1);
	throw_flag = magic ^ 1;
	ret = duk_hobject_define_property_helper(ctx,
	                                         defprop_flags,
	                                         obj,
	                                         key,
	                                         idx_value,
	                                         get,
	                                         set,
	                                         throw_flag);

	/* Ignore the normalize/validate helper outputs on the value stack,
	 * they're popped automatically.
	 */

	if (magic == 0) {
		/* Object.defineProperty(): return target object. */
		duk_push_hobject(ctx, obj);
	} else {
		/* Reflect.defineProperty(): return success/fail. */
		duk_push_boolean(ctx, ret);
	}
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_get_own_property_descriptor(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 2);

	/* ES2015 Section 19.1.2.6, step 1 */
	if (duk_get_current_magic(ctx) == 0) {
		duk_to_object(ctx, 0);
	}

	/* [ obj key ] */

	duk_hobject_object_get_own_property_descriptor(ctx, -2);
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is_extensible(duk_context *ctx) {
	/*
	 *  magic = 0: Object.isExtensible()
	 *  magic = 1: Reflect.isExtensible()
	 */

	duk_hobject *h;

	if (duk_get_current_magic(ctx) == 0) {
		h = duk_get_hobject(ctx, 0);
	} else {
		/* Reflect.isExtensible(): throw if non-object, but we accept lightfuncs
		 * and plain buffers here because they pretend to be objects.
		 */
		h = duk_require_hobject_accept_mask(ctx, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	}

	duk_push_boolean(ctx, (h != NULL) && DUK_HOBJECT_HAS_EXTENSIBLE(h));
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
/* Shared helper for various key/symbol listings, magic:
 * 0=Object.keys()
 * 1=Object.getOwnPropertyNames(),
 * 2=Object.getOwnPropertySymbols(),
 * 3=Reflect.ownKeys()
 */
DUK_LOCAL const duk_small_uint_t duk__object_keys_enum_flags[4] = {
	/* Object.keys() */
	DUK_ENUM_OWN_PROPERTIES_ONLY |
	    DUK_ENUM_NO_PROXY_BEHAVIOR,

	/* Object.getOwnPropertyNames() */
	DUK_ENUM_INCLUDE_NONENUMERABLE |
	    DUK_ENUM_OWN_PROPERTIES_ONLY |
	    DUK_ENUM_NO_PROXY_BEHAVIOR,

	/* Object.getOwnPropertySymbols() */
	DUK_ENUM_INCLUDE_SYMBOLS |
	    DUK_ENUM_OWN_PROPERTIES_ONLY |
	    DUK_ENUM_EXCLUDE_STRINGS |
	    DUK_ENUM_INCLUDE_NONENUMERABLE |
	    DUK_ENUM_NO_PROXY_BEHAVIOR,

	/* Reflect.ownKeys() */
	DUK_ENUM_INCLUDE_SYMBOLS |
	    DUK_ENUM_OWN_PROPERTIES_ONLY |
	    DUK_ENUM_INCLUDE_NONENUMERABLE |
	    DUK_ENUM_NO_PROXY_BEHAVIOR
};

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_keys_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
#if defined(DUK_USE_ES6_PROXY)
	duk_hobject *h_proxy_target;
	duk_hobject *h_proxy_handler;
	duk_hobject *h_trap_result;
#endif
	duk_small_uint_t enum_flags;
	duk_int_t magic;

	DUK_ASSERT_TOP(ctx, 1);
	DUK_UNREF(thr);

	magic = duk_get_current_magic(ctx);
	if (magic == 3) {
		/* ES2015 Section 26.1.11 requires a TypeError for non-objects.  Lightfuncs
		 * and plain buffers pretend to be objects, so accept those too.
		 */
		obj = duk_require_hobject_promote_mask(ctx, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	} else {
		/* ES2015: ToObject coerce. */
		obj = duk_to_hobject(ctx, 0);
	}
	DUK_ASSERT(obj != NULL);
	DUK_UNREF(obj);

	/* XXX: proxy chains */

#if defined(DUK_USE_ES6_PROXY)
	/* XXX: better sharing of code between proxy target call sites */
	if (DUK_LIKELY(!duk_hobject_proxy_check(thr,
	                                        obj,
	                                        &h_proxy_target,
	                                        &h_proxy_handler))) {
		goto skip_proxy;
	}

	duk_push_hobject(ctx, h_proxy_handler);
	if (!duk_get_prop_stridx_short(ctx, -1, DUK_STRIDX_OWN_KEYS)) {
		/* Careful with reachability here: don't pop 'obj' before pushing
		 * proxy target.
		 */
		DUK_DDD(DUK_DDDPRINT("no ownKeys trap, get keys of target instead"));
		duk_pop_2(ctx);
		duk_push_hobject(ctx, h_proxy_target);
		duk_replace(ctx, 0);
		DUK_ASSERT_TOP(ctx, 1);
		goto skip_proxy;
	}

	/* [ obj handler trap ] */
	duk_insert(ctx, -2);
	duk_push_hobject(ctx, h_proxy_target);  /* -> [ obj trap handler target ] */
	duk_call_method(ctx, 1 /*nargs*/);      /* -> [ obj trap_result ] */
	h_trap_result = duk_require_hobject(ctx, -1);
	DUK_UNREF(h_trap_result);

	magic = duk_get_current_magic(ctx);
	DUK_ASSERT(magic >= 0 && magic < (duk_int_t) (sizeof(duk__object_keys_enum_flags) / sizeof(duk_small_uint_t)));
	enum_flags = duk__object_keys_enum_flags[magic];

	duk_proxy_ownkeys_postprocess(ctx, h_proxy_target, enum_flags);
	return 1;

 skip_proxy:
#endif  /* DUK_USE_ES6_PROXY */

	DUK_ASSERT_TOP(ctx, 1);
	magic = duk_get_current_magic(ctx);
	DUK_ASSERT(magic >= 0 && magic < (duk_int_t) (sizeof(duk__object_keys_enum_flags) / sizeof(duk_small_uint_t)));
	enum_flags = duk__object_keys_enum_flags[magic];
	return duk_hobject_get_enumerated_keys(ctx, enum_flags);
}
#endif  /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */

#if defined(DUK_USE_OBJECT_BUILTIN) || defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_prevent_extensions(duk_context *ctx) {
	/*
	 *  magic = 0: Object.preventExtensions()
	 *  magic = 1: Reflect.preventExtensions()
	 */

	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_uint_t mask;
	duk_int_t magic;

	magic = duk_get_current_magic(ctx);

	/* Silent success for lightfuncs and plain buffers always. */
	mask = DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER;

	/* Object.preventExtensions() silent success for non-object. */
	if (magic == 0) {
		mask |= DUK_TYPE_MASK_UNDEFINED |
		        DUK_TYPE_MASK_NULL |
		        DUK_TYPE_MASK_BOOLEAN |
		        DUK_TYPE_MASK_NUMBER |
		        DUK_TYPE_MASK_STRING |
		        DUK_TYPE_MASK_POINTER;
	}

	if (duk_check_type_mask(ctx, 0, mask)) {
		/* Not an object, already non-extensible so always success. */
		goto done;
	}
	h = duk_require_hobject(ctx, 0);
	DUK_ASSERT(h != NULL);

	DUK_HOBJECT_CLEAR_EXTENSIBLE(h);

	/* A non-extensible object cannot gain any more properties,
	 * so this is a good time to compact.
	 */
	duk_hobject_compact_props(thr, h);

 done:
	if (magic == 1) {
		duk_push_true(ctx);
	}
	return 1;
}
#endif  /* DUK_USE_OBJECT_BUILTIN || DUK_USE_REFLECT_BUILTIN */
