/*
 *  Buffer built-ins
 */

#include "duk_internal.h"

/*
 *  Constructor
 */

int duk_builtin_buffer_constructor(duk_context *ctx) {
	if (duk_get_top(ctx) == 0) {
		(void) duk_push_fixed_buffer(ctx, 0);
	} else {
		duk_to_buffer(ctx, 0, NULL);
	}
	DUK_ASSERT(duk_is_buffer(ctx, 0));
	duk_set_top(ctx, 1);

	if (duk_is_constructor_call(ctx)) {
		duk_push_object_helper(ctx,
		                       DUK_HOBJECT_FLAG_EXTENSIBLE |
		                       DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_BUFFER),
		                       DUK_BIDX_BUFFER_PROTOTYPE);

		/* Buffer object internal value is immutable */
		duk_dup(ctx, 0);
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);
	}
	/* Note: unbalanced stack on purpose */

	return 1;
}

/*
 *  toString(), valueOf()
 */

static int pointer_tostring_valueof_helper(duk_context *ctx, int to_string) {
	duk_tval *tv;

	duk_push_this(ctx);
	tv = duk_require_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	if (DUK_TVAL_IS_BUFFER(tv)) {
		/* nop */
	} else if (DUK_TVAL_IS_OBJECT(tv)) {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		/* Must be a "buffer object", i.e. class "Buffer" */
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h) != DUK_HOBJECT_CLASS_BUFFER) {
			goto type_error;
		}

		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
	} else {
		goto type_error;
	}

	if (to_string) {
		duk_to_string(ctx, -1);
	}
	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

int duk_builtin_buffer_prototype_to_string(duk_context *ctx) {
	return pointer_tostring_valueof_helper(ctx, 1);
}

int duk_builtin_buffer_prototype_value_of(duk_context *ctx) {
	return pointer_tostring_valueof_helper(ctx, 0);
}

