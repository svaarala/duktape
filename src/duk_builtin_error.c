/*
 *  Error built-ins
 */

#include "duk_internal.h"

static int duk_error_constructor_helper(duk_context *ctx, int bidx_prototype) {
	/* Behavior for constructor and non-constructor call is
	 * exactly the same.
	 */

	/* same for both error and each subclass like TypeError */
	int flags_and_class = DUK_HOBJECT_FLAG_EXTENSIBLE |
	                      DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_ERROR);
	
	duk_push_object_helper(ctx, flags_and_class, bidx_prototype);

	if (!duk_is_undefined(ctx, 0)) {
		duk_to_string(ctx, 0);
		duk_dup(ctx, 0);  /* [ message error message ] */
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE, DUK_PROPDESC_FLAGS_WC);
	}

	return 1;
}

int duk_builtin_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_ERROR_PROTOTYPE);
}

int duk_builtin_eval_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_EVAL_ERROR_PROTOTYPE);
}

int duk_builtin_range_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_RANGE_ERROR_PROTOTYPE);
}

int duk_builtin_reference_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_REFERENCE_ERROR_PROTOTYPE);
}

int duk_builtin_syntax_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_SYNTAX_ERROR_PROTOTYPE);
}

int duk_builtin_type_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_TYPE_ERROR_PROTOTYPE);
}

int duk_builtin_uri_error_constructor(duk_context *ctx) {
	return duk_error_constructor_helper(ctx, DUK_BIDX_URI_ERROR_PROTOTYPE);
}

int duk_builtin_error_prototype_to_string(duk_context *ctx) {
	/* FIXME: optimize with more direct internal access */

	duk_push_this(ctx);
	if (!duk_is_object(ctx, -1)) {
		goto type_error;
	}

	/* [ ... this ] */

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_NAME);
	if (duk_is_undefined(ctx, -1)) {
		duk_pop(ctx);
		duk_push_string(ctx, "Error");
	} else {
		duk_to_string(ctx, -1);
	}

	/* [ ... this name ] */

	/* FIXME: Are steps 6 and 7 in E5 Section 15.11.4.4 duplicated by
	 * accident or are they actually needed?  The first ToString()
	 * could conceivably return 'undefined'.
	 */
	duk_get_prop_stridx(ctx, -2, DUK_STRIDX_MESSAGE);
	if (duk_is_undefined(ctx, -1)) {
		duk_pop(ctx);
		duk_push_string(ctx, "");
	} else {
		duk_to_string(ctx, -1);
	}

	/* [ ... this name message ] */

	if (duk_get_length(ctx, -2) == 0) {
		/* name is empty -> return message */
		return 1;
	}
	if (duk_get_length(ctx, -1) == 0) {
		/* message is empty -> return name */
		duk_pop(ctx);
		return 1;
	}
	duk_push_string(ctx, ": ");
	duk_insert(ctx, -2);  /* ... name ': ' message */
	duk_concat(ctx, 3);

	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

