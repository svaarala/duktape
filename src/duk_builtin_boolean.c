/*
 *  Boolean built-ins
 */

#include "duk_internal.h"

int duk_builtin_boolean_constructor(duk_context *ctx) {
	if (duk_is_constructor_call(ctx)) {
		return DUK_RET_UNIMPLEMENTED_ERROR;  /*FIXME*/
	} else {
		return DUK_RET_UNIMPLEMENTED_ERROR;  /*FIXME*/
	}
}

int duk_builtin_boolean_prototype_to_string(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_boolean_prototype_value_of(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

