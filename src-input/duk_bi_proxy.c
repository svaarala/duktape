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
		if (DUK_UNLIKELY(DUK_HSTRING_HAS_SYMBOL(h))) {
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
	DUK_ASSERT_TOP(ctx, 2);  /* [ target handler ] */

	DUK_ASSERT(duk_is_constructor_call(ctx));
	duk_push_proxy(ctx);  /* [ target handler ] -> [ proxy ] */
	return 1;  /* replacement */
}
#endif  /* DUK_USE_ES6_PROXY */
