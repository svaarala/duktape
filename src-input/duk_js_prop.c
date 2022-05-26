#include "duk_internal.h"

DUK_INTERNAL void duk_js_getprototypeof_hproxy(duk_hthread *thr, duk_hproxy *h) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ((duk_hobject *) h));
	DUK_HPROXY_ASSERT_VALID(h);
	DUK_ASSERT(h->handler != NULL);
	DUK_ASSERT(h->target != NULL);

	duk_native_stack_check(thr); /* In case handler is a Proxy too. */
	duk_require_stack(thr, DUK_HOBJECT_PROXY_VALSTACK_SPACE);
	duk_push_hobject(thr, h->handler);
	if (duk_get_prop_stridx_short(thr, -1, DUK_STRIDX_GET_PROTOTYPE_OF)) {
		duk_insert(thr, -2); /* [ -> [ ... trap handler ] */
		duk_push_hobject(thr, h->target);
		duk_call_method(thr, 1 /*nargs*/); /* [ ... trap handler target ] -> [ ... result ] */
	} else {
		duk_pop_2_known(thr);
		duk_js_getprototypeof(thr, h->target);
	}
}

/* Push result of [[GetPrototypeOf]] on the value stack.  Value stack must be
 * used because the result of a Proxy 'getPrototypeOf' trap might not be
 * reachable otherwise.
 */
DUK_INTERNAL void duk_js_getprototypeof(duk_hthread *thr, duk_hobject *obj) {
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	if (DUK_UNLIKELY(DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(obj))) {
		DUK_ASSERT(DUK_HOBJECT_GET_HTYPE(obj) == DUK_HTYPE_PROXY);
		duk_js_getprototypeof_hproxy(thr, (duk_hproxy *) obj);
	} else {
		duk_hobject *proto = duk_hobject_get_proto_raw(thr->heap, obj);
		duk_push_hobject_or_null(thr, proto);
	}
}

/* [[SetPrototypeOf]] */
DUK_LOCAL duk_bool_t duk__setprototypeof_loop_check(duk_hthread *thr, duk_hobject *obj, duk_hobject *proto) {
	duk_hobject *curr;

	curr = proto;
	while (curr != NULL) {
		if (DUK_UNLIKELY(curr == obj)) {
			return 0;
		}

		/* If [[GetPrototypeOf]] is exotic, terminate and accept.
		 * At present only Proxy has a custom algorithm.
		 */
		if (DUK_UNLIKELY(DUK_HOBJECT_GET_HTYPE(curr) == DUK_HTYPE_PROXY)) {
			break;
		} else {
			curr = duk_hobject_get_proto_raw(thr->heap, curr);
		}
	}

	return 1;
}

DUK_LOCAL duk_bool_t duk__setprototypeof_proxy(duk_hthread *thr, duk_hobject *obj, duk_hobject *proto) {
	return -1;
}

DUK_INTERNAL duk_bool_t duk_js_setprototypeof(duk_hthread *thr, duk_hobject *obj, duk_hobject *proto) {
	duk_small_uint_t htype;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);
	/* 'proto' may be NULL */

	/* [[SetPrototypeOf]] standard behavior, E6 9.1.2. */

retry_obj:
	htype = DUK_HOBJECT_GET_HTYPE(obj);

	switch (htype) {
	case DUK_HTYPE_PROXY: {
		duk_small_int_t rc_proxy;

		rc_proxy = duk__setprototypeof_proxy(thr, obj, proto);
		if (rc_proxy < 0) {
			duk_hobject *target;

			target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
			DUK_ASSERT(target != NULL);
			obj = target;
			goto retry_obj;
		} else {
			return (duk_bool_t) rc_proxy;
		}
	}
	default:
		break;
	}

	if (proto == duk_hobject_get_proto_raw(thr->heap, obj)) {
		return 1;
	}
	if (obj == thr->builtins[DUK_BIDX_OBJECT_PROTOTYPE]) {
		/* Immutable prototype, Object.prototype only for now. */
		return 0;
	}
	if (!duk_js_isextensible(thr, obj)) {
		return 0;
	}
	if (!duk__setprototypeof_loop_check(thr, obj, proto)) {
		return 0;
	}

	duk_hobject_set_proto_raw_updref(thr, obj, proto);
	return 1;
}

/* [[PreventExtensions]] */
#if defined(DUK_USE_PROXY_POLICY)
DUK_LOCAL void duk__preventextensions_proxy_policy(duk_hthread *thr, duk_hobject *obj, duk_bool_t trap_rc) {
	duk_hobject *target;

	target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
	DUK_ASSERT(target != NULL);

	if (trap_rc != 0 && duk_js_isextensible(thr, target)) {
		DUK_DD(DUK_DDPRINT("preventExtensions() trap successful, but target is still extensible"));
		DUK_ERROR_TYPE_PROXY_REJECTED(thr);
	}
}
#endif

DUK_LOCAL duk_bool_t duk__preventextensions_proxy(duk_hthread *thr, duk_hobject *obj) {
	/* 'obj' stability assumed from caller. */
	if (duk_proxy_trap_check_nokey(thr, (duk_hproxy *) obj, DUK_STRIDX_PREVENT_EXTENSIONS)) {
		duk_bool_t trap_rc;
		duk_hobject *target;

		duk_call_method(thr, 1); /* [ ... trap handler target ] -> [ ... result ] */
		trap_rc = duk_to_boolean_top_pop(thr);
#if defined(DUK_USE_PROXY_POLICY)
		duk__preventextensions_proxy_policy(thr, obj, trap_rc);
#else
		DUK_DD(DUK_DDPRINT("proxy policy check for 'preventExtensions' trap disabled in configuration"));
#endif
		return trap_rc;
	} else {
		return -1;
	}
}

DUK_INTERNAL_DECL duk_bool_t duk_js_preventextensions(duk_hthread *thr, duk_hobject *obj) {
	duk_small_uint_t htype;
	duk_bool_t rc;
	duk_bool_t was_extensible;

retry_obj:
	htype = DUK_HOBJECT_GET_HTYPE(obj);

	switch (htype) {
	case DUK_HTYPE_PROXY: {
		duk_small_int_t proxy_rc = duk__preventextensions_proxy(thr, obj);
		if (proxy_rc < 0) {
			/* Stability: assume immutable proxy chain for now
			 * (not true with revocable proxies).
			 */
			duk_hobject *target;
			target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
			DUK_ASSERT(target != NULL);
			obj = target;
			goto retry_obj;
		} else {
			return (duk_bool_t) proxy_rc;
		}
	}
	default:
		break;
	}

	was_extensible = DUK_HOBJECT_HAS_EXTENSIBLE(obj);
	DUK_HOBJECT_CLEAR_EXTENSIBLE(obj);
	rc = 1;

	if (rc != 0 && was_extensible) {
		/* A non-extensible object cannot gain any more properties,
		 * so this is a good time to compact.
		 */
		duk_hobject_compact_object(thr, obj);
	}
	return rc;
}

/* [[IsExtensible]] */
DUK_LOCAL duk_bool_t duk__isextensible_proxy(duk_hthread *thr, duk_hobject *obj) {
	/* 'obj' stability assumed from caller. */
	if (duk_proxy_trap_check_nokey(thr, (duk_hproxy *) obj, DUK_STRIDX_IS_EXTENSIBLE)) {
		duk_bool_t trap_rc;
		duk_hobject *target;
		duk_bool_t target_rc;

		duk_call_method(thr, 1); /* [ ... trap handler target ] -> [ ... result ] */
		trap_rc = duk_to_boolean(thr, -1);
#if defined(DUK_USE_PROXY_POLICY)
		target = duk_proxy_get_target_autothrow(thr, (duk_hproxy *) obj);
		DUK_ASSERT(target != NULL);
		target_rc = duk_js_isextensible(thr, target);
		if (trap_rc != target_rc) {
			DUK_ERROR_TYPE_PROXY_REJECTED(thr);
		}
#else
		DUK_DD(DUK_DDPRINT("proxy policy check for 'isExtensible' trap disabled in configuration"));
#endif
		return trap_rc;
	} else {
		return -1;
	}
}
DUK_INTERNAL_DECL duk_bool_t duk_js_isextensible(duk_hthread *thr, duk_hobject *obj) {
	duk_small_uint_t htype;

retry_obj:
	htype = DUK_HOBJECT_GET_HTYPE(obj);

	switch (htype) {
	case DUK_HTYPE_PROXY: {
		duk_small_int_t proxy_rc = duk__isextensible_proxy(thr, obj);
		if (proxy_rc < 0) {
			/* Stability: assume immutable proxy chain for now
			 * (not true with revocable proxies).
			 */
			duk_hobject *target = ((duk_hproxy *) obj)->target;
			if (target == NULL) {
				/* Deal with revoked proxies like this for now.
				 * Check required behavior.
				 */
				return 0;
			}
			obj = target;
			goto retry_obj;
		} else {
			return (duk_bool_t) proxy_rc;
		}
	}
	default:
		break;
	}

	return DUK_HOBJECT_HAS_EXTENSIBLE(obj) ? 1 : 0;
}
