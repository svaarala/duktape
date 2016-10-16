/*
 *  Proxy built-in (ES6)
 */

#include "duk_internal.h"

#if defined(DUK_USE_ES6_PROXY)
DUK_INTERNAL duk_ret_t duk_bi_proxy_constructor(duk_context *ctx) {
	duk_hobject *h_target;
	duk_hobject *h_handler;

	duk_require_constructor_call(ctx);

	/* Reject a proxy object as the target because it would need
	 * special handler in property lookups.  (ES6 has no such restriction)
	 */
	h_target = duk_require_hobject_promote_mask(ctx, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	DUK_ASSERT(h_target != NULL);
	if (DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(h_target)) {
		goto fail_args;
	}

	/* Reject a proxy object as the handler because it would cause
	 * potentially unbounded recursion.  (ES6 has no such restriction)
	 *
	 * There's little practical reason to use a lightfunc or a plain
	 * buffer as the handler table: one could only provide traps via
	 * their prototype objects (Function.prototype and ArrayBuffer.prototype).
	 * Even so, as lightfuncs and plain buffers mimic their object
	 * counterparts, they're promoted and accepted here.
	 */
	h_handler = duk_require_hobject_promote_mask(ctx, 1, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	DUK_ASSERT(h_handler != NULL);
	if (DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(h_handler)) {
		goto fail_args;
	}

	/* XXX: the returned value is exotic in ES6, but we use a
	 * simple object here with no prototype.  Without a prototype,
	 * ToPrimitive() coercion fails which is a bit confusing.
	 * No callable check/handling in the current Proxy subset.
	 */
	(void) duk_push_object_helper_proto(ctx,
	                                    DUK_HOBJECT_FLAG_EXTENSIBLE |
	                                    DUK_HOBJECT_FLAG_EXOTIC_PROXYOBJ |
	                                    DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJECT),
	                                    NULL);
	DUK_ASSERT_TOP(ctx, 3);

	/* Make _Target and _Handler non-configurable and non-writable.
	 * They can still be forcibly changed by C code (both user and
	 * Duktape internal), but not by Ecmascript code.
	 */

	/* Proxy target */
	duk_dup_0(ctx);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_TARGET, DUK_PROPDESC_FLAGS_NONE);

	/* Proxy handler */
	duk_dup_1(ctx);
	duk_xdef_prop_stridx(ctx, -2, DUK_STRIDX_INT_HANDLER, DUK_PROPDESC_FLAGS_NONE);

	return 1;  /* replacement handler */

 fail_args:
	DUK_DCERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
}
#endif  /* DUK_USE_ES6_PROXY */
