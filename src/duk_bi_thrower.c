/*
 *  Type error thrower, E5 Section 13.2.3.
 */

#include "duk_internal.h"

DUK_INTERNAL duk_ret_t duk_bi_type_error_thrower(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_TYPE_ERROR;
}
