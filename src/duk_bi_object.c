/*
 *  Object built-ins
 */

#include "duk_internal.h"

int duk_bi_object_constructor(duk_context *ctx) {
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
	 * promote to an object value.
	 */
	if (duk_check_type_mask(ctx, 0, DUK_TYPE_MASK_STRING |
	                                DUK_TYPE_MASK_BOOLEAN |
	                                DUK_TYPE_MASK_NUMBER |
	                                DUK_TYPE_MASK_POINTER |
	                                DUK_TYPE_MASK_BUFFER)) {
		duk_to_object(ctx, 0);
		return 1;
	}

	duk_push_object_helper(ctx,
	                       DUK_HOBJECT_FLAG_EXTENSIBLE |
	                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                       DUK_BIDX_OBJECT_PROTOTYPE);
	return 1;
}

int duk_bi_object_constructor_get_prototype_of(duk_context *ctx) {
	duk_hobject *h;

	h = duk_require_hobject(ctx, 0);
	DUK_ASSERT(h != NULL);

	/* XXX: should the API call handle this directly, i.e. attempt
	 * to duk_push_hobject(ctx, null) would push a null instead?
	 * (On the other hand 'undefined' would be just as logical, but
	 * not wanted here.)
	 */

	if (h->prototype) {
		duk_push_hobject(ctx, h->prototype);
	} else {
		duk_push_null(ctx);
	}
	return 1;
}

int duk_bi_object_constructor_get_own_property_descriptor(duk_context *ctx) {
	/* FIXME: no need for indirect call */
	return duk_hobject_object_get_own_property_descriptor(ctx);
}

int duk_bi_object_constructor_get_own_property_names(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 1);
	(void) duk_require_hobject(ctx, 0);
	return duk_hobject_get_enumerated_keys(ctx, DUK_ENUM_INCLUDE_NONENUMERABLE |
	                                            DUK_ENUM_OWN_PROPERTIES_ONLY);
}

int duk_bi_object_constructor_create(duk_context *ctx) {
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

		/* Use original function.  No need to get it explicitly,
		 * just call the helper.
		 */

		duk_replace(ctx, 0);

		/* [ obj Properties ] */

		return duk_hobject_object_define_properties(ctx);
	}

	/* [ O Properties obj ] */

	return 1;
}

int duk_bi_object_constructor_define_property(duk_context *ctx) {
	/* FIXME: no need for indirect call */
	return duk_hobject_object_define_property(ctx);
}

int duk_bi_object_constructor_define_properties(duk_context *ctx) {
	/* FIXME: no need for indirect call */
	return duk_hobject_object_define_properties(ctx);
}

static int duk__seal_freeze_helper(duk_context *ctx, int is_freeze) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;

	h = duk_require_hobject(ctx, 0);
	DUK_ASSERT(h != NULL);

	duk_hobject_object_seal_freeze_helper(thr, h, is_freeze /*freeze*/);

	/* Sealed and frozen objects cannot gain any more properties,
	 * so this is a good time to compact them.
	 */
	duk_hobject_compact_props(thr, h);

	return 1;
}

int duk_bi_object_constructor_seal(duk_context *ctx) {
	return duk__seal_freeze_helper(ctx, 0);
}

int duk_bi_object_constructor_freeze(duk_context *ctx) {
	return duk__seal_freeze_helper(ctx, 1);
}

int duk_bi_object_constructor_prevent_extensions(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h;

	h = duk_require_hobject(ctx, 0);
	DUK_ASSERT(h != NULL);

	DUK_HOBJECT_CLEAR_EXTENSIBLE(h);

	/* A non-extensible object cannot gain any more properties,
	 * so this is a good time to compact.
	 */
	duk_hobject_compact_props(thr, h);
	
	return 1;
}

int duk_bi_object_constructor_is_sealed(duk_context *ctx) {
	duk_hobject *h;
	int rc;

	h = duk_require_hobject(ctx, 0);
	DUK_ASSERT(h != NULL);

	rc = duk_hobject_object_is_sealed_frozen_helper(h, 0 /*is_frozen*/);
	duk_push_boolean(ctx ,rc);
	return 1;
}

int duk_bi_object_constructor_is_frozen(duk_context *ctx) {
	duk_hobject *h;
	int rc;

	h = duk_require_hobject(ctx, 0);
	DUK_ASSERT(h != NULL);

	rc = duk_hobject_object_is_sealed_frozen_helper(h, 1 /*is_frozen*/);
	duk_push_boolean(ctx, rc);
	return 1;
}

int duk_bi_object_constructor_is_extensible(duk_context *ctx) {
	duk_hobject *h;

	h = duk_require_hobject(ctx, 0);
	DUK_ASSERT(h != NULL);

	duk_push_boolean(ctx, DUK_HOBJECT_HAS_EXTENSIBLE(h));
	return 1;
}

int duk_bi_object_constructor_keys(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 1);
	(void) duk_require_hobject(ctx, 0);
	return duk_hobject_get_enumerated_keys(ctx, DUK_ENUM_OWN_PROPERTIES_ONLY);
}

int duk_bi_object_prototype_to_string(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	duk_push_this(ctx);
	duk_push_string(ctx, "[object ");

	if (duk_is_undefined(ctx, -2)) {
		duk_push_string(ctx, "Undefined");
	} else if (duk_is_null(ctx, -2)) {
		duk_push_string(ctx, "Null");
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

int duk_bi_object_prototype_to_locale_string(duk_context *ctx) {
	DUK_ASSERT_TOP(ctx, 0);
	(void) duk_push_this_coercible_to_object(ctx);
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_TO_STRING);
	if (!duk_is_callable(ctx, 1)) {
		return DUK_RET_TYPE_ERROR;
	}
	duk_dup(ctx, 0);  /* -> [ O toString O ] */
	duk_call_method(ctx, 0);  /* FIXME: call method tailcall? */
	return 1;
}

int duk_bi_object_prototype_value_of(duk_context *ctx) {
	(void) duk_push_this_coercible_to_object(ctx);
	return 1;
}

int duk_bi_object_prototype_is_prototype_of(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h_v;
	duk_hobject *h_obj;

	DUK_ASSERT_TOP(ctx, 1);

	h_v = duk_get_hobject(ctx, 0);
	if (!h_v) {
		duk_push_false(ctx);  /* FIXME: tail call: return duk_push_false(ctx) */
		return 1;
	}

	h_obj = duk_push_this_coercible_to_object(ctx);
	DUK_ASSERT(h_obj != NULL);

	/* E5.1 Section 15.2.4.6, step 3.a, lookup proto once before compare */
	duk_push_boolean(ctx, duk_hobject_prototype_chain_contains(thr, h_v->prototype, h_obj));
	return 1;
}

int duk_bi_object_prototype_has_own_property(duk_context *ctx) {
	return duk_hobject_object_ownprop_helper(ctx, 0 /*required_desc_flags*/);
}

int duk_bi_object_prototype_property_is_enumerable(duk_context *ctx) {
	return duk_hobject_object_ownprop_helper(ctx, DUK_PROPDESC_FLAG_ENUMERABLE /*required_desc_flags*/);
}

