#include "duk_internal.h"

/* Check and prepare for a Proxy trap call:
 *
 * - If proxy is revoked ([[ProxyHandler]] is null), throw unconditionally.
 * - If no trap, returns 0, and no changes to value stack.
 * - If trap exists, returns 1, and pushes [ trap handler target ]
 *
 * Because a trap lookup may have arbitrary side effects (for example, the
 * handler object may itself be a Proxy) the original object reference ('h')
 * is NOT guaranteed to be safe afterwards unless the caller has made
 * provisions to stabilize it e.g. via the value stack.
 */
DUK_LOCAL duk_bool_t duk__proxy_trap_check(duk_hthread *thr, duk_hproxy *h, duk_small_uint_t trap_stridx) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	DUK_HPROXY_ASSERT_VALID(h);
	DUK_ASSERT_STRIDX_VALID(trap_stridx);

	/* Specification operations detect revoked proxies by checking if
	 * O.[[ProxyHandler]] is null before a trap check, and if so, throwing
	 * unconditionally.
	 */
	if (DUK_UNLIKELY(h->handler == NULL)) {
		DUK_ASSERT(h->target == NULL);
		DUK_ERROR_TYPE_PROXY_REVOKED(thr);
	}
	DUK_ASSERT(h->handler != NULL);
	DUK_ASSERT(h->target != NULL);

	/* Once we read the trap property we no longer have any guarantees
	 * of 'h': the trap may alter the receiver or any of the objects in
	 * the inheritance / Proxy ->target path.  The whole chain may be
	 * freed.  So we must push whatever we need later and ignore 'h'
	 * after the get.  (For property reads, the receiver is safe in the
	 * value stack.)
	 */

	duk_native_stack_check(thr); /* In case handler is a Proxy too. */
	duk_require_stack(thr, DUK_HOBJECT_PROXY_VALSTACK_SPACE);
	DUK_GC_TORTURE(thr->heap);
	duk_push_hobject(thr, h->handler);
	duk_push_hobject(thr, h->target);
	h = NULL;

	(void) duk_get_prop_stridx_short(thr, -2, trap_stridx);
	DUK_GC_TORTURE(thr->heap);

	if (!duk_is_undefined(thr, -1)) {
		/* [ ... handler target trap ] */
		duk_insert(thr, -3); /* [ -> [ ... trap handler target ] */
		return 1;
	} else {
		duk_pop_3_unsafe(thr);
		return 0;
	}
}

DUK_INTERNAL_DECL duk_bool_t duk_proxy_trap_check_strkey(duk_hthread *thr,
                                                         duk_hproxy *h,
                                                         duk_hstring *key,
                                                         duk_small_uint_t trap_stridx) {
	if (DUK_UNLIKELY(DUK_HSTRING_HAS_HIDDEN(key))) {
		/* Symbol accesses must go through proxy lookup in ES2015.
		 * Hidden symbols behave like Duktape 1.x internal keys
		 * and currently won't.
		 */
		DUK_DDD(DUK_DDDPRINT("hidden key, skip proxy handler and apply to target"));
		return 0;
	}
	return duk__proxy_trap_check(thr, h, trap_stridx);
}

DUK_INTERNAL_DECL duk_bool_t duk_proxy_trap_check_nokey(duk_hthread *thr, duk_hproxy *h, duk_small_uint_t trap_stridx) {
	return duk__proxy_trap_check(thr, h, trap_stridx);
}

DUK_INTERNAL_DECL duk_bool_t duk_proxy_trap_check_idxkey(duk_hthread *thr,
                                                         duk_hproxy *h,
                                                         duk_uarridx_t idx,
                                                         duk_small_uint_t trap_stridx) {
	return duk_proxy_trap_check_nokey(thr, h, trap_stridx);
}

#if defined(DUK_USE_ES6_PROXY)
DUK_INTERNAL duk_hobject *duk_proxy_get_target_autothrow(duk_hthread *thr, duk_hproxy *h) {
	duk_hobject *target;

	DUK_CTX_ASSERT_VALID(thr);

	target = h->target;
	if (DUK_UNLIKELY(target == NULL)) {
		DUK_ERROR_TYPE_PROXY_REVOKED(thr);
	}
	return target;
}
#else
DUK_INTERNAL duk_hobject *duk_proxy_get_target_autothrow(duk_hthread *thr, duk_hproxy *h) {
	DUK_ASSERT(0); /* Never here. */
	DUK_ERROR_TYPE_PROXY_REVOKED(thr);
}
#endif /* DUK_USE_ES6_PROXY */

/* Get Proxy target object.  If the argument is not a Proxy, return it as is.
 * If a Proxy is revoked, an error is thrown.
 */
DUK_INTERNAL duk_hobject *duk_hobject_resolve_proxy_target(duk_hobject *obj) {
	DUK_ASSERT(obj != NULL);

	/* Resolve Proxy targets until Proxy chain ends.  No explicit check for
	 * a Proxy loop: user code cannot create such a loop (it would only be
	 * possible by editing duk_hproxy references directly).
	 */

#if defined(DUK_USE_ES6_PROXY)
	while (DUK_HOBJECT_IS_PROXY(obj)) {
		duk_hproxy *h_proxy;

		h_proxy = (duk_hproxy *) obj;
		DUK_HPROXY_ASSERT_VALID(h_proxy);
		obj = h_proxy->target;
		DUK_ASSERT(obj != NULL);
	}
#endif /* DUK_USE_ES6_PROXY */

	DUK_ASSERT(obj != NULL);
	return obj;
}
