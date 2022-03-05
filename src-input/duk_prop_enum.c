/*
 *  EnumerateObjectProperties()
 *
 *  Older "enumeration" style:
 *  - https://tc39.es/ecma262/#sec-enumerate-object-properties
 *
 *  Newer "iteration" style:
 *  - https://tc39.es/ecma262/#sec-iterable-interface
 *  - https://tc39.es/ecma262/#sec-asynciterable-interface
 *  - https://tc39.es/ecma262/#sec-operations-on-iterator-objects
 *  - https://tc39.es/ecma262/#sec-getiterator
 *
 *  The enumeration style iterators (for-in) are not directly exposed to
 *  user code so their internal implementation can be freely chosen.
 */

#include "duk_internal.h"

/* Convert from public API flags to internal enum flags. */
DUK_LOCAL duk_uint_t duk__enum_convert_public_api_flags(duk_uint_t enum_flags) {
	duk_uint_t ownpropkeys_flags = 0;

	if (enum_flags & DUK_ENUM_ARRAY_INDICES_ONLY) {
		ownpropkeys_flags |= DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX;
	} else {
		ownpropkeys_flags |= DUK_OWNPROPKEYS_FLAG_INCLUDE_ARRIDX | DUK_OWNPROPKEYS_FLAG_INCLUDE_STRING;
	}

	if (enum_flags & DUK_ENUM_INCLUDE_SYMBOLS) {
		ownpropkeys_flags |= DUK_OWNPROPKEYS_FLAG_INCLUDE_SYMBOL;
	}

	if (!(enum_flags & DUK_ENUM_INCLUDE_NONENUMERABLE)) {
		ownpropkeys_flags |= DUK_OWNPROPKEYS_FLAG_REQUIRE_ENUMERABLE;
	}

	if (enum_flags & DUK_ENUM_INCLUDE_HIDDEN) {
		ownpropkeys_flags |= DUK_OWNPROPKEYS_FLAG_INCLUDE_HIDDEN;
	}

	if (enum_flags & DUK_ENUM_NO_PROXY_BEHAVIOR) {
		ownpropkeys_flags |= DUK_OWNPROPKEYS_FLAG_NO_PROXY_BEHAVIOR;
	}

	/* DUK_ENUM_OWN_PROPERTIES_ONLY: handled in enum prototype loop. */

	/* DUK_ENUM_SORT_ARRAY_INDICES: handled after all ancestors enumerated. */

	return ownpropkeys_flags;
}

/* Enumerate a target object with specified enumeration flags and push a gapless
 * bare array with the enumerated keys.
 */
DUK_INTERNAL void duk_prop_enum_keylist(duk_hthread *thr, duk_hobject *obj, duk_uint_t enum_flags) {
	duk_idx_t idx_res;
	duk_idx_t idx_visited;
	duk_idx_t idx_obj;
	duk_idx_t idx_base_top;
	duk_uarridx_t out_idx = 0;
	duk_uint_t ownpropkeys_flags;

	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(obj != NULL);

	idx_res = duk_push_bare_array(thr);
	idx_visited = duk_push_bare_object(thr);
	duk_push_hobject(thr, obj); /* Stabilize 'obj' against becoming unreachable due to side effects. */
	idx_base_top = duk_get_top(thr);
	idx_obj = idx_base_top - 1;

	/* [ ... res visited obj(stabilized) ] */

	/* Flags are the same for every loop, so compute them only once. */
	ownpropkeys_flags = duk__enum_convert_public_api_flags(enum_flags);

	while (obj != NULL) {
		duk_harray *a;
		duk_uarridx_t i, n;
		duk_tval *val_base;

		/* [ ... res visited obj ] */

		/* Baseline implementation must use [[OwnPropertyKeys]], followed by
		 * [[GetOwnPropertyDescriptor]] for each String (not Symbol) key, as
		 * these may be observable through Proxies.  This could be optimized
		 * for non-Proxy cases where side effects are not observable.
		 *
		 * Note that while [[OwnPropertyKeys]] has a well defined ordering
		 * in ES2015+, it is NOT applied to 'ownKeys' proxy trap result
		 * which is processed as is.
		 */
		duk_prop_ownpropkeys(thr, obj, ownpropkeys_flags);

		DUK_ASSERT(duk_is_array(thr, -1));

		/* [ ... res visited obj keys ] */

		a = duk_require_harray(thr, -1);
		val_base = DUK_HARRAY_GET_ITEMS(thr->heap, a);
		DUK_ASSERT(DUK_HARRAY_GET_LENGTH(a) <= DUK_HARRAY_GET_ITEMS_LENGTH(a));
		for (i = 0, n = DUK_HARRAY_GET_LENGTH(a); i < n; i++) {
			duk_tval *tv_key;
			duk_small_int_t desc_rc;

			DUK_ASSERT(duk_get_top(thr) == idx_base_top + 1);
			DUK_ASSERT(val_base != NULL);
			DUK_ASSERT(DUK_HARRAY_GET_ITEMS(thr->heap, a) == val_base);

			tv_key = val_base + i;

			desc_rc = duk_prop_getowndesc_obj_tvkey(thr, obj, tv_key);
			if (desc_rc < 0) {
				DUK_ASSERT(duk_get_top(thr) == idx_base_top + 1);
				continue;
			}

			if (duk_prop_has(thr, duk_get_tval(thr, idx_visited), tv_key)) {
				DUK_DDD(DUK_DDDPRINT("already visited: %!T", tv_key));
			} else {
				/* Mark visited, even if property is non-enumerable and NOT included
				 * in output.  If we then find the same property in the inheritance
				 * chain, we must not enumerate it even if the inherited property is
				 * enumerable.
				 */
				duk_bool_t keep = 1;

				duk_push_true(thr);
				duk_prop_putvalue_inidx(thr, idx_visited, tv_key, duk_get_top_index(thr), 0 /*throw_flag*/);

				if (keep) {
					duk_push_tval(thr, tv_key);
					duk_to_property_key(thr, -1);
					duk_put_prop_index(thr, idx_res, out_idx++);
				}
			}
			duk_set_top(thr, idx_base_top + 1);

			/* [ ... res visited obj keys ] */
		}

		DUK_ASSERT(duk_get_top(thr) == idx_base_top + 1);
		duk_pop_unsafe(thr);
		DUK_ASSERT(duk_get_top(thr) == idx_base_top);

		if (enum_flags & DUK_ENUM_OWN_PROPERTIES_ONLY) {
			break;
		}

		DUK_ASSERT(obj != NULL);
		duk_js_getprototypeof(thr, obj);
		duk_replace(thr, idx_obj); /* Keep valstack reachability. */
		obj = duk_get_hobject(thr, idx_obj);
	}

	if (enum_flags & DUK_ENUM_SORT_ARRAY_INDICES) {
		/* ECMAScript mandates array indices to be sorted for own keys of an
		 * object but doesn't guarantee sorting for inherited indices.  This
		 * flag allows caller to request array index sort over the full output.
		 */

		/* TBD. */
	}

	DUK_ASSERT(duk_get_top(thr) == idx_base_top);

	duk_pop_2_unsafe(thr);

	/* [ ... res ] */
}

/* Create an internal enumerator object for a target object with specified enum flags. */
DUK_INTERNAL void duk_prop_enum_create_enumerator(duk_hthread *thr, duk_hobject *obj, duk_uint_t enum_flags) {
	duk_push_bare_object(thr);
	duk_prop_enum_keylist(thr, obj, enum_flags);
	duk_put_prop_literal(thr, -2, "keys");
	duk_push_hobject(thr, obj);
	duk_put_prop_literal(thr, -2, "target");
	duk_push_uint(thr, 0U);
	duk_put_prop_literal(thr, -2, "index");
}

/* Get next key (and optionally value) from an internal enumerator object and push
 * them on the value stack.
 */
DUK_INTERNAL duk_bool_t duk_prop_enum_next(duk_hthread *thr, duk_idx_t idx_enum, duk_bool_t get_value) {
	duk_uarridx_t idx_next;

	idx_enum = duk_require_normalize_index(thr, idx_enum);

	duk_get_prop_literal(thr, idx_enum, "keys");
	duk_get_prop_literal(thr, idx_enum, "target");
	duk_get_prop_literal(thr, idx_enum, "index");
	idx_next = duk_to_uint32(thr, -1);

	/* [ ... .keys .target .index ] */

	(void) duk_get_prop_index(thr, -3, idx_next);
	if (duk_is_undefined(thr, -1)) {
		duk_pop_n_unsafe(thr, 4);
		return 0;
	}

	/* [ ... .keys .target .index key ] */

	idx_next++;
	duk_push_u32(thr, (duk_uint32_t) idx_next);
	duk_put_prop_literal(thr, idx_enum, "index");

	/* [ ... .keys .target .index key ] */

	duk_insert(thr, -4);

	/* [ ... key .keys .target .index ] */

	if (get_value) {
		duk_dup_m4(thr);
		duk_get_prop(thr, -3);
		duk_insert(thr, -4);
	}

	/* [ ... key val? .keys .target .index ] */

	duk_pop_3_unsafe(thr);

	/* [ ... key val? ] */

	return 1;
}
