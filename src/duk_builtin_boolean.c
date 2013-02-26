/*
 *  Boolean built-ins
 */

#include "duk_internal.h"

int duk_builtin_boolean_constructor(duk_context *ctx) {
	duk_hobject *this;

	duk_to_boolean(ctx, 0);

	if (duk_is_constructor_call(ctx)) {
		/* FIXME: helper; rely on Boolean.prototype as being non-writable, non-configurable */
		duk_push_this(ctx);
		this = duk_get_hobject(ctx, -1);
		DUK_ASSERT(this != NULL);
		DUK_ASSERT(this->prototype == ((duk_hthread *) ctx)->builtins[DUK_BIDX_BOOLEAN_PROTOTYPE]);

		DUK_HOBJECT_SET_CLASS_NUMBER(this, DUK_HOBJECT_CLASS_BOOLEAN);

		duk_dup(ctx, 0);  /* -> [ val obj val ] */
		duk_def_prop_stridx(ctx, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);  /* FIXME: proper flags? */
	}  /* unbalanced stack */

	return 1;
}

int duk_builtin_boolean_prototype_to_string(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_boolean_prototype_value_of(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

