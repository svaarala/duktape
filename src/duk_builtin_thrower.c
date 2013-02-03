/*
 *  Type error thrower, E5 Section 13.2.3.
 */

#include "duk_internal.h"

int duk_builtin_type_error_thrower(duk_context *ctx) {
	return DUK_RET_TYPE_ERROR;
}
