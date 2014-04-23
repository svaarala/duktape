/*
 *  Proxy built-in (ES6 draft)
 */

#include "duk_internal.h"

duk_ret_t duk_bi_proxy_constructor(duk_context *ctx) {
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}
	return DUK_RET_UNIMPLEMENTED_ERROR;  /*FIXME*/
}

duk_ret_t duk_bi_proxy_constructor_revocable(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;  /*FIXME*/
}
