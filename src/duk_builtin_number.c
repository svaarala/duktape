/*
 *  Number built-ins
 */

#include "duk_internal.h"

int duk_builtin_number_constructor(duk_context *ctx) {
	int nargs;
	duk_hobject *h_this;

	DUK_ASSERT_TOP(ctx, 1);

	/*
	 *  The Number constructor uses ToNumber(arg) for number coercion
	 *  (coercing an undefined argument to NaN).  However, if the
	 *  argument is not given at all, +0 must be used instead.  This
	 *  is a vararg function to detect whether an argument was given
	 *  or not.
	 */

	nargs = duk_get_top(ctx);
	if (nargs == 0) {
		duk_push_int(ctx, 0);
	}
	duk_to_number(ctx, 0);
	duk_set_top(ctx, 1);
	DUK_ASSERT_TOP(ctx, 1);

	if (!duk_is_constructor_call(ctx)) {
		return 1;
	}

	/*
	 *  E5 Section 15.7.2.1 requires that the constructed object
	 *  must have the original Number.prototype as its internal
	 *  prototype.  However, since Number.prototype is non-writable
	 *  and non-configurable, this doesn't have to be enforced here:
	 *  The default object (bound to 'this') is OK, though we have
	 *  to change its class.
	 *
	 *  Internal value set to ToNumber(arg) or +0; if no arg given,
	 *  ToNumber(undefined) = NaN, so special treatment is needed
	 *  (above).  String internal value is immutable.
	 */

	/* FIXME: helper */
	duk_push_this(ctx);
	h_this = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h_this != NULL);
	DUK_HOBJECT_SET_CLASS_NUMBER(h_this, DUK_HOBJECT_CLASS_NUMBER);

	DUK_ASSERT(h_this->prototype == ((duk_hthread *) ctx)->builtins[DUK_BIDX_NUMBER_PROTOTYPE]);
	DUK_ASSERT(DUK_HOBJECT_GET_CLASS_NUMBER(h_this) == DUK_HOBJECT_CLASS_NUMBER);
	DUK_ASSERT(DUK_HOBJECT_HAS_EXTENSIBLE(h_this));

	duk_dup(ctx, 0);  /* -> [ val obj val ] */
	duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);

	return 0;  /* no return value -> don't replace created value */
}

int duk_builtin_number_prototype_to_string(duk_context *ctx) {
	duk_hobject *h;

	/* FIXME: radixes etc:
	 *
	 * > (5.66).toString(36)
	 * '5.nrcyk5rcykogq2xpdb7ta9k9'
	 */

	duk_push_this(ctx);
	if (duk_is_number(ctx, -1)) {
		/* number is directly ok */
	} else {
		h = duk_get_hobject(ctx, -1);
		if (!h) {
			goto type_error;
		}
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_NUMBER) {
			goto type_error;
		}
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
		DUK_ASSERT(duk_is_number(ctx, -1));
	}  /* uneven stack on purpose */

	duk_to_string(ctx, -1);
	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

int duk_builtin_number_prototype_to_locale_string(duk_context *ctx) {
	/* FIXME */
	return duk_builtin_number_prototype_to_string(ctx);
}

int duk_builtin_number_prototype_value_of(duk_context *ctx) {
	duk_hobject *h;

	/* FIXME: shared prechecks, refactor */

	duk_push_this(ctx);
	if (duk_is_number(ctx, -1)) {
		return 1;
	}
	h = duk_get_hobject(ctx, -1);
	if (!h) {
		goto type_error;
	}
	if (DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_NUMBER) {
		goto type_error;
	}

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

int duk_builtin_number_prototype_to_fixed(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_number_prototype_to_exponential(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_number_prototype_to_precision(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

