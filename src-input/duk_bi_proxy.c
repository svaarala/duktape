/*
 *  Proxy built-in (ES2015)
 */

#include "duk_internal.h"

#if defined(DUK_USE_ES6_PROXY)
DUK_INTERNAL duk_ret_t duk_bi_proxy_constructor(duk_hthread *thr) {
	DUK_ASSERT_TOP(thr, 2); /* [ target handler ] */

	duk_require_constructor_call(thr);
	duk_push_proxy(thr, 0 /*flags*/); /* [ target handler ] -> [ proxy ] */
	return 1;
}
#endif /* DUK_USE_ES6_PROXY */

#if defined(DUK_USE_ES6_PROXY)
DUK_INTERNAL duk_ret_t duk_bi_proxy_constructor_revocable(duk_hthread *thr) {
	DUK_ASSERT_TOP(thr, 2); /* [ target handler ] */

	DUK_ASSERT(!duk_is_constructor_call(thr));
	duk_push_object(thr);
	duk_push_proxy(thr, 0 /*flags*/); /* [ target handler ] -> [ proxy ] */
	duk_put_prop_literal(thr, -2, "proxy");
	duk_eval_string(thr, "(function () { throw new TypeError('unimplemented'); })");
	duk_put_prop_literal(thr, -2, "revoke");
	return 1;
}
#endif /* DUK_USE_ES6_PROXY */
