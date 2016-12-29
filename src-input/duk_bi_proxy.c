/*
 *  Proxy built-in (ES2015)
 */

#include "duk_internal.h"

#if defined(DUK_USE_ES6_PROXY)
/* Post-process a Proxy ownKeys() result at stack top.  Push a cleaned up
 * array of valid result keys (strings or symbols).  TypeError for invalid
 * values.  Flags are shared with duk_enum().
 */
DUK_INTERNAL void duk_proxy_ownkeys_postprocess(duk_context *ctx, duk_hobject *h_proxy_target, duk_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uarridx_t i, len, idx;
	duk_propdesc desc;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(h_proxy_target != NULL);

	len = (duk_uarridx_t) duk_get_length(ctx, -1);
	idx = 0;
	duk_push_array(ctx);
	/* XXX: preallocated dense array, fill in directly */
	for (i = 0; i < len; i++) {
		duk_hstring *h;

		/* [ obj trap_result res_arr ] */
		(void) duk_get_prop_index(ctx, -2, i);
		h = duk_get_hstring(ctx, -1);
		if (h == NULL) {
			DUK_ERROR_TYPE_INVALID_TRAP_RESULT(thr);
		}

		if (!(flags & DUK_ENUM_INCLUDE_NONENUMERABLE)) {
			/* No support for 'getOwnPropertyDescriptor' trap yet,
			 * so check enumerability always from target object
			 * descriptor.
			 */
			if (duk_hobject_get_own_propdesc(thr, h_proxy_target, duk_known_hstring(ctx, -1), &desc, 0 /*flags*/)) {
				if ((desc.flags & DUK_PROPDESC_FLAG_ENUMERABLE) == 0) {
					DUK_DDD(DUK_DDDPRINT("ignore non-enumerable property: %!T", duk_get_tval(ctx, -1)));
					goto skip_key;
				}
			} else {
				DUK_DDD(DUK_DDDPRINT("ignore non-existent property: %!T", duk_get_tval(ctx, -1)));
				goto skip_key;
			}
		}
		if (DUK_HSTRING_HAS_SYMBOL(h)) {
			if (!(flags & DUK_ENUM_INCLUDE_SYMBOLS)) {
				DUK_DDD(DUK_DDDPRINT("ignore symbol property: %!T", duk_get_tval(ctx, -1)));
				goto skip_key;
			}
			if (DUK_HSTRING_HAS_HIDDEN(h) && !(flags & DUK_ENUM_INCLUDE_HIDDEN)) {
				DUK_DDD(DUK_DDDPRINT("ignore hidden symbol property: %!T", duk_get_tval(ctx, -1)));
				goto skip_key;
			}
		} else {
			if (flags & DUK_ENUM_EXCLUDE_STRINGS) {
				DUK_DDD(DUK_DDDPRINT("ignore string property: %!T", duk_get_tval(ctx, -1)));
				goto skip_key;
			}
		}

		/* [ obj trap_result res_arr propname ] */
		duk_put_prop_index(ctx, -2, idx++);
		continue;

	 skip_key:
		duk_pop(ctx);
		continue;
	}

	/* XXX: Missing trap result validation for non-configurable target keys
	 * (must be present), for non-extensible target all target keys must be
	 * present and no extra keys can be present.
	 * http://www.ecma-international.org/ecma-262/6.0/#sec-proxy-object-internal-methods-and-internal-slots-ownpropertykeys
	 */

	/* XXX: The key enumerability check should trigger the "getOwnPropertyDescriptor"
	 * trap which has not yet been implemented.  In the absence of such a trap,
	 * the enumerability should be checked from the target object; this is
	 * handled above.
	 */
}
#endif  /* DUK_USE_ES6_PROXY */

#if defined(DUK_USE_ES6_PROXY)
DUK_INTERNAL duk_ret_t duk_bi_proxy_constructor(duk_context *ctx) {
	duk_hobject *h_target;
	duk_hobject *h_handler;

	duk_require_constructor_call(ctx);

	/* Reject a proxy object as the target because it would need
	 * special handler in property lookups.  (ES2015 has no such restriction)
	 */
	h_target = duk_require_hobject_promote_mask(ctx, 0, DUK_TYPE_MASK_LIGHTFUNC | DUK_TYPE_MASK_BUFFER);
	DUK_ASSERT(h_target != NULL);
	if (DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(h_target)) {
		goto fail_args;
	}

	/* Reject a proxy object as the handler because it would cause
	 * potentially unbounded recursion.  (ES2015 has no such restriction)
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

	/* XXX: the returned value is exotic in ES2015, but we use a
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
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_INT_TARGET, DUK_PROPDESC_FLAGS_NONE);

	/* Proxy handler */
	duk_dup_1(ctx);
	duk_xdef_prop_stridx_short(ctx, -2, DUK_STRIDX_INT_HANDLER, DUK_PROPDESC_FLAGS_NONE);

	return 1;  /* replacement handler */

 fail_args:
	DUK_DCERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
}
#endif  /* DUK_USE_ES6_PROXY */
