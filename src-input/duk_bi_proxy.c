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
DUK_LOCAL duk_ret_t duk__bi_proxy_revoker(duk_hthread *thr) {
	duk_hobject *obj;
	duk_hobject *h;

	duk_push_current_function(thr);
	obj = duk_require_hobject(thr, -1);
	h = duk_hobject_get_internal_value_object(thr->heap, obj);
	if (h != NULL) {
		DUK_ASSERT(h != NULL);
		DUK_ASSERT(DUK_HOBJECT_IS_PROXY(h));
		duk_proxy_revoke(thr, (duk_hproxy *) h);
	}
	return 0;
}

DUK_INTERNAL duk_ret_t duk_bi_proxy_constructor_revocable(duk_hthread *thr) {
	DUK_ASSERT(!duk_is_constructor_call(thr));
	DUK_ASSERT_TOP(thr, 2); /* [ target handler ] */

	duk_push_proxy(thr, 0 /*flags*/); /* [ target handler ] -> [ proxy ] */
	duk_push_object(thr);
	duk_push_c_function(thr, duk__bi_proxy_revoker, 0);
	duk_dup(thr, 0); /* -> [ proxy retval revoker proxy ] */
	duk_xdef_prop_stridx_short(thr, -2, DUK_STRIDX_INT_VALUE, DUK_PROPDESC_FLAGS_NONE);
	duk_pull(thr, 0); /* -> [ retval revoker proxy ] */
	duk_put_prop_literal(thr, 0, "proxy");
	duk_put_prop_literal(thr, 0, "revoke"); /* -> [ retval ] */
	return 1;
}
#endif /* DUK_USE_ES6_PROXY */
