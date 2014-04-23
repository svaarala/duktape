/*
 *  Proxy built-in (ES6 draft)
 */

#include "duk_internal.h"

#if defined(DUK_USE_ES6_PROXY)
duk_ret_t duk_bi_proxy_constructor(duk_context *ctx) {
	if (!duk_is_constructor_call(ctx)) {
		return DUK_RET_TYPE_ERROR;
	}
	return DUK_RET_UNIMPLEMENTED_ERROR;  /*FIXME*/
}
#else  /* DUK_UES_ES6_PROXY */
duk_ret_t duk_bi_proxy_constructor(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_ES6_PROXY */

#if defined(DUK_USE_ES6_PROXY)
duk_ret_t duk_bi_proxy_constructor_revocable(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNIMPLEMENTED_ERROR;  /*FIXME*/
}
#else  /* DUK_UES_ES6_PROXY */
duk_ret_t duk_bi_proxy_constructor_revocable(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_ES6_PROXY */
