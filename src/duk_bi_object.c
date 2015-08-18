/*
 *  Object built-ins
 */

#include "duk_internal.h"

DUK_INTERNAL duk_ret_t duk_bi_object_constructor(duk_context *ctx) {
	if (!duk_is_constructor_call(ctx) &&
	    !duk_is_null_or_undefined(ctx, 0)) {
		duk_to_object(ctx, 0);
		return 1;
	}

	if (duk_is_object(ctx, 0)) {
		return 1;
	}

	/* Pointer and buffer primitive values are treated like other
	 * primitives values which have a fully fledged object counterpart:
	 * promote to an object value.  Lightfuncs are coerced with
	 * ToObject() even they could also be returned as is.
	 */
	if (duk_check_type_mask(ctx, 0, DUK_TYPE_MASK_STRING |
	                                DUK_TYPE_MASK_BOOLEAN |
	                                DUK_TYPE_MASK_NUMBER |
	                                DUK_TYPE_MASK_POINTER |
	                                DUK_TYPE_MASK_BUFFER |
	                                DUK_TYPE_MASK_LIGHTFUNC)) {
		duk_to_object(ctx, 0);
		return 1;
	}

	duk_push_object_helper(ctx,
	                       DUK_HOBJECT_FLAG_EXTENSIBLE |
	                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                       DUK_BIDX_OBJECT_PROTOTYPE);
	return 1;
}

/* Shared helper to implement Object.getPrototypeOf and the ES6
 * Object.prototype.__proto__ getter.
 *
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-get-object.prototype.__proto__
 */
DUK_INTERNAL duk_ret_t duk_bi_object_getprototype_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_hobject *proto;

	DUK_UNREF(thr);

	/* magic: 0=getter call, 1=Object.getPrototypeOf */
	if (duk_get_current_magic(ctx) == 0) {
		duk_push_this_coercible_to_object(ctx);
		duk_insert(ctx, 0);
	}

	h = duk_require_hobject_or_lfunc(ctx, 0);
	/* h is NULL for lightfunc */

	/* XXX: should the API call handle this directly, i.e. attempt
	 * to duk_push_hobject(ctx, null) would push a null instead?
	 * (On the other hand 'undefined' would be just as logical, but
	 * not wanted here.)
	 */

	if (h == NULL) {
		duk_push_hobject_bidx(ctx, DUK_BIDX_FUNCTION_PROTOTYPE);
	} else {
		proto = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h);
		if (proto) {
			duk_push_hobject(ctx, proto);
		} else {
			duk_push_null(ctx);
		}
	}
	return 1;
}

/* Shared helper to implement ES6 Object.setPrototypeOf and
 * Object.prototype.__proto__ setter.
 *
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-get-object.prototype.__proto__
 * http://www.ecma-international.org/ecma-262/6.0/index.html#sec-object.setprototypeof
 */
DUK_INTERNAL duk_ret_t duk_bi_object_setprototype_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h_obj;
	duk_hobject *h_new_proto;
	duk_hobject *h_curr;
	duk_ret_t ret_success = 1;  /* retval for success path */

	/* Preliminaries for __proto__ and setPrototypeOf (E6 19.1.2.18 steps 1-4);
	 * magic: 0=setter call, 1=Object.setPrototypeOf
	 */
	if (duk_get_current_magic(ctx) == 0) {
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
		duk_require_object_coercible(ctx, 0);
		duk_require_type_mask(ctx, 1, DUK_TYPE_MASK_NULL | DUK_TYPE_MASK_OBJECT);
	}

	h_new_proto = duk_get_hobject(ctx, 1);
	/* h_new_proto may be NULL */
	if (duk_is_lightfunc(ctx, 0)) {
		if (h_new_proto == thr->builtins[DUK_BIDX_FUNCTION_PROTOTYPE]) {
			goto skip;
		}
		goto fail_nonextensible;
	}
	h_obj = duk_get_hobject(ctx, 0);
	if (!h_obj) {
		goto skip;
	}
	DUK_ASSERT(h_obj != NULL);

	/* [[SetPrototypeOf]] standard behavior, E6 9.1.2 */
	/* TODO: implement Proxy object support here */

	if (h_new_proto == DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h_obj)) {
		goto skip;
	}
	if (!DUK_HOBJECT_HAS_EXTENSIBLE(h_obj)) {
		goto fail_nonextensible;
	}
	for (h_curr = h_new_proto; h_curr != NULL; h_curr = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, h_curr)) {
		/* Loop prevention */
		if (h_curr == h_obj) {
			goto fail_loop;
		}
	}
	DUK_HOBJECT_SET_PROTOTYPE_UPDREF(thr, h_obj, h_new_proto);
	/* fall thru */

 skip:
	duk_set_top(ctx, 1);
	return ret_success;

 fail_nonextensible:
 fail_loop:
	return DUK_RET_TYPE_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_get_own_property_descriptor(duk_context *ctx) {
	/* XXX: no need for indirect call */
	return duk_hobject_object_get_own_property_descriptor(ctx);
}

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_create(duk_context *ctx) {
	duk_tval *tv;
	duk_hobject *proto = NULL;

	DUK_ASSERT_TOP(ctx, 2);

	tv = duk_get_tval(ctx, 0);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_NULL(tv)) {
		;
	} else if (DUK_TVAL_IS_OBJECT(tv)) {
		proto = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(proto != NULL);
	} else {
		return DUK_RET_TYPE_ERROR;
	}

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

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_define_property(duk_context *ctx) {
	duk_hobject *obj;
	duk_hstring *key;
	duk_hobject *get;
	duk_hobject *set;
	duk_idx_t idx_value;
	duk_uint_t defprop_flags;

	DUK_ASSERT(ctx != NULL);

	DUK_DDD(DUK_DDDPRINT("Object.defineProperty(): ctx=%p obj=%!T key=%!T desc=%!T",
	                     (void *) ctx,
	                     (duk_tval *) duk_get_tval(ctx, 0),
	                     (duk_tval *) duk_get_tval(ctx, 1),
	                     (duk_tval *) duk_get_tval(ctx, 2)));

	/* [ obj key desc ] */

	/* Lightfuncs are currently supported by coercing to a temporary
	 * Function object; changes will be allowed (the coerced value is
	 * extensible) but will be lost.
	 */
	obj = duk_require_hobject_or_lfunc_coerce(ctx, 0);
	(void) duk_to_string(ctx, 1);
	key = duk_require_hstring(ctx, 1);
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

	duk_hobject_define_property_helper(ctx,
	                                   defprop_flags,
	                                   obj,
	                                   key,
	                                   idx_value,
	                                   get,
	                                   set);

	/* Ignore the normalize/validate helper outputs on the value stack,
	 * they're popped automatically.
	 */

	/*
	 *  Return target object.
	 */

	duk_push_hobject(ctx, obj);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_define_properties(duk_context *ctx) {
	duk_small_uint_t pass;
	duk_uint_t defprop_flags;
	duk_hobject *obj;
	duk_idx_t idx_value;
	duk_hobject *get;
	duk_hobject *set;

	/* Lightfunc handling by ToObject() coercion. */
	obj = duk_require_hobject_or_lfunc_coerce(ctx, 0);  /* target */
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
		duk_enum(ctx, 1, DUK_ENUM_OWN_PROPERTIES_ONLY /*enum_flags*/);

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

			/* [ hobject props enum(props) key desc value? getter? setter? ] */

			if (pass == 0) {
				continue;
			}

			key = duk_get_hstring(ctx, 3);
			DUK_ASSERT(key != NULL);

			duk_hobject_define_property_helper(ctx,
			                                   defprop_flags,
			                                   obj,
			                                   key,
			                                   idx_value,
			                                   get,
			                                   set);
		}
	}

	/*
	 *  Return target object
	 */

	duk_dup(ctx, 0);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_seal_freeze_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;
	duk_bool_t is_freeze;

	h = duk_require_hobject_or_lfunc(ctx, 0);
	if (!h) {
		/* Lightfunc, always success. */
		return 1;
	}

	is_freeze = (duk_bool_t) duk_get_current_magic(ctx);
	duk_hobject_object_seal_freeze_helper(thr, h, is_freeze);

	/* Sealed and frozen objects cannot gain any more properties,
	 * so this is a good time to compact them.
	 */
	duk_hobject_compact_props(thr, h);

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_prevent_extensions(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;

	h = duk_require_hobject_or_lfunc(ctx, 0);
	if (!h) {
		/* Lightfunc, always success. */
		return 1;
	}
	DUK_ASSERT(h != NULL);

	DUK_HOBJECT_CLEAR_EXTENSIBLE(h);

	/* A non-extensible object cannot gain any more properties,
	 * so this is a good time to compact.
	 */
	duk_hobject_compact_props(thr, h);

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is_sealed_frozen_shared(duk_context *ctx) {
	duk_hobject *h;
	duk_bool_t is_frozen;
	duk_bool_t rc;

	h = duk_require_hobject_or_lfunc(ctx, 0);
	if (!h) {
		duk_push_true(ctx);  /* frozen and sealed */
	} else {
		is_frozen = duk_get_current_magic(ctx);
		rc = duk_hobject_object_is_sealed_frozen_helper((duk_hthread *) ctx, h, is_frozen /*is_frozen*/);
		duk_push_boolean(ctx, rc);
	}
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_object_constructor_is_extensible(duk_context *ctx) {
	duk_hobject *h;

	h = duk_require_hobject_or_lfunc(ctx, 0);
	if (!h) {
		duk_push_false(ctx);
	} else {
		duk_push_boolean(ctx, DUK_HOBJECT_HAS_EXTENSIBLE(h));
	}
	return 1;
}

/* Shared helper for Object.getOwnPropertyNames() and Object.keys().
 * Magic: 0=getOwnPropertyNames, 1=Object.keys.
 */
DUK_INTERNAL duk_ret_t duk_bi_object_constructor_keys_shared(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *obj;
#if defined(DUK_USE_ES6_PROXY)
	duk_hobject *h_proxy_target;
	duk_hobject *h_proxy_handler;
	duk_hobject *h_trap_result;
	duk_uarridx_t i, len, idx;
#endif
	duk_small_uint_t enum_flags;

	DUK_ASSERT_TOP(ctx, 1);
	DUK_UNREF(thr);

	obj = duk_require_hobject_or_lfunc_coerce(ctx, 0);
	DUK_ASSERT(obj != NULL);
	DUK_UNREF(obj);

#if defined(DUK_USE_ES6_PROXY)
	if (DUK_LIKELY(!duk_hobject_proxy_check(thr,
	                                        obj,
	                                        &h_proxy_target,
	                                        &h_proxy_handler))) {
		goto skip_proxy;
	}

	duk_push_hobject(ctx, h_proxy_handler);
	if (!duk_get_prop_stridx(ctx, -1, DUK_STRIDX_OWN_KEYS)) {
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

	len = (duk_uarridx_t) duk_get_length(ctx, -1);
	idx = 0;
	duk_push_array(ctx);
	for (i = 0; i < len; i++) {
		/* [ obj trap_result res_arr ] */
		if (duk_get_prop_index(ctx, -2, i) && duk_is_string(ctx, -1)) {
			/* XXX: for Object.keys() we should check enumerability of key */
			/* [ obj trap_result res_arr propname ] */
			duk_put_prop_index(ctx, -2, idx);
			idx++;
		} else {
			duk_pop(ctx);
		}
	}

	/* XXX: for Object.keys() the [[OwnPropertyKeys]] result (trap result)
	 * should be filtered so that only enumerable keys remain.  Enumerability
	 * should be checked with [[GetOwnProperty]] on the original object
	 * (i.e., the proxy in this case).  If the proxy has a getOwnPropertyDescriptor
	 * trap, it should be triggered for every property.  If the proxy doesn't have
	 * the trap, enumerability should be checked against the target object instead.
	 * We don't do any of this now, so Object.keys() and Object.getOwnPropertyNames()
	 * return the same result now for proxy traps.  We still do clean up the trap
	 * result, so that Object.keys() and Object.getOwnPropertyNames() will return a
	 * clean array of strings without gaps.
	 */
	return 1;

 skip_proxy:
#endif  /* DUK_USE_ES6_PROXY */

	DUK_ASSERT_TOP(ctx, 1);

	if (duk_get_current_magic(ctx)) {
		/* Object.keys */
		enum_flags = DUK_ENUM_OWN_PROPERTIES_ONLY |
		             DUK_ENUM_NO_PROXY_BEHAVIOR;
	} else {
		/* Object.getOwnPropertyNames */
		enum_flags = DUK_ENUM_INCLUDE_NONENUMERABLE |
		             DUK_ENUM_OWN_PROPERTIES_ONLY |
		             DUK_ENUM_NO_PROXY_BEHAVIOR;
	}

	return duk_hobject_get_enumerated_keys(ctx, enum_flags);
}

DUK_INTERNAL duk_ret_t duk_bi_object_prototype_to_string(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	duk_push_this(ctx);
	duk_push_string(ctx, "[object ");

	if (duk_is_undefined(ctx, -2)) {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_UC_UNDEFINED);
	} else if (duk_is_null(ctx, -2)) {
		duk_push_hstring_stridx(ctx, DUK_STRIDX_UC_NULL);
	} else {
		duk_hobject *h_this;
		duk_hstring *h_classname;

		duk_to_object(ctx, -2);
		h_this = duk_get_hobject(ctx, -2);
		DUK_ASSERT(h_this != NULL);

		h_classname = DUK_HOBJECT_GET_CLASS_STRING(thr->heap, h_this);
		DUK_ASSERT(h_classname != NULL);

		duk_push_hstring(ctx, h_classname);
	}

	duk_push_string(ctx, "]");
	duk_concat(ctx, 3);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_object_prototype_to_locale_string(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 0);
	(void) duk_push_this_coercible_to_object(ctx);
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_TO_STRING);
	if (!duk_is_callable(ctx, 1)) {
		return DUK_RET_TYPE_ERROR;
	}
	duk_dup(ctx, 0);  /* -> [ O toString O ] */
	duk_call_method(ctx, 0);  /* XXX: call method tail call? */
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_object_prototype_value_of(duk_context *ctx) {
	(void) duk_push_this_coercible_to_object(ctx);
	return 1;
}

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

DUK_INTERNAL duk_ret_t duk_bi_object_prototype_has_own_property(duk_context *ctx) {
	return duk_hobject_object_ownprop_helper(ctx, 0 /*required_desc_flags*/);
}

DUK_INTERNAL duk_ret_t duk_bi_object_prototype_property_is_enumerable(duk_context *ctx) {
	return duk_hobject_object_ownprop_helper(ctx, DUK_PROPDESC_FLAG_ENUMERABLE /*required_desc_flags*/);
}
