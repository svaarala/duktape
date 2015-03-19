/*
 *  Hobject enumeration support.
 *
 *  Creates an internal enumeration state object to be used e.g. with for-in
 *  enumeration.  The state object contains a snapshot of target object keys
 *  and internal control state for enumeration.  Enumerator flags allow caller
 *  to e.g. request internal/non-enumerable properties, and to enumerate only
 *  "own" properties.
 *
 *  Also creates the result value for e.g. Object.keys() based on the same
 *  internal structure.
 *
 *  This snapshot-based enumeration approach is used to simplify enumeration:
 *  non-snapshot-based approaches are difficult to reconcile with mutating
 *  the enumeration target, running multiple long-lived enumerators at the
 *  same time, garbage collection details, etc.  The downside is that the
 *  enumerator object is memory inefficient especially for iterating arrays.
 */

#include "duk_internal.h"

/* XXX: identify enumeration target with an object index (not top of stack) */

/* must match exactly the number of internal properties inserted to enumerator */
#define DUK__ENUM_START_INDEX  2

/*
 *  Helper to sort array index keys.  The keys are in the enumeration object
 *  entry part, starting from DUK__ENUM_START_INDEX, and the entry part is dense.
 *
 *  We use insertion sort because it is simple (leading to compact code,)
 *  works nicely in-place, and minimizes operations if data is already sorted
 *  or nearly sorted (which is a very common case here).  It also minimizes
 *  the use of element comparisons in general.  This is nice because element
 *  comparisons here involve re-parsing the string keys into numbers each
 *  time, which is naturally very expensive.
 *
 *  Note that the entry part values are all "true", e.g.
 *
 *    "1" -> true, "3" -> true, "2" -> true
 *
 *  so it suffices to only work in the key part without exchanging any keys,
 *  simplifying the sort.
 *
 *  http://en.wikipedia.org/wiki/Insertion_sort
 *
 *  (Compiles to about 160 bytes now as a stand-alone function.)
 */

DUK_LOCAL void duk__sort_array_indices(duk_hthread *thr, duk_hobject *h_obj) {
	duk_hstring **keys;
	duk_hstring **p_curr, **p_insert, **p_end;
	duk_hstring *h_curr;
	duk_uarridx_t val_highest, val_curr, val_insert;

	DUK_ASSERT(h_obj != NULL);
	DUK_ASSERT(DUK_HOBJECT_GET_ENEXT(h_obj) >= 2);  /* control props */
	DUK_UNREF(thr);

	if (DUK_HOBJECT_GET_ENEXT(h_obj) <= 1 + DUK__ENUM_START_INDEX) {
		return;
	}

	keys = DUK_HOBJECT_E_GET_KEY_BASE(thr->heap, h_obj);
	p_end = keys + DUK_HOBJECT_GET_ENEXT(h_obj);
	keys += DUK__ENUM_START_INDEX;

	DUK_DDD(DUK_DDDPRINT("keys=%p, p_end=%p (after skipping enum props)",
	                     (void *) keys, (void *) p_end));

#ifdef DUK_USE_DDDPRINT
	{
		duk_uint_fast32_t i;
		for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(h_obj); i++) {
			DUK_DDD(DUK_DDDPRINT("initial: %ld %p -> %!O",
			                     (long) i,
			                     (void *) DUK_HOBJECT_E_GET_KEY_PTR(thr->heap, h_obj, i),
			                     (duk_heaphdr *) DUK_HOBJECT_E_GET_KEY(thr->heap, h_obj, i)));
		}
	}
#endif

	val_highest = DUK_HSTRING_GET_ARRIDX_SLOW(keys[0]);
	for (p_curr = keys + 1; p_curr < p_end; p_curr++) {
		DUK_ASSERT(*p_curr != NULL);
		val_curr = DUK_HSTRING_GET_ARRIDX_SLOW(*p_curr);

		if (val_curr >= val_highest) {
			DUK_DDD(DUK_DDDPRINT("p_curr=%p, p_end=%p, val_highest=%ld, val_curr=%ld -> "
			                     "already in correct order, next",
			                     (void *) p_curr, (void *) p_end, (long) val_highest, (long) val_curr));
			val_highest = val_curr;
			continue;
		}

		DUK_DDD(DUK_DDDPRINT("p_curr=%p, p_end=%p, val_highest=%ld, val_curr=%ld -> "
		                     "needs to be inserted",
		                     (void *) p_curr, (void *) p_end, (long) val_highest, (long) val_curr));

		/* Needs to be inserted; scan backwards, since we optimize
		 * for the case where elements are nearly in order.
		 */

		p_insert = p_curr - 1;
		for (;;) {
			val_insert = DUK_HSTRING_GET_ARRIDX_SLOW(*p_insert);
			if (val_insert < val_curr) {
				DUK_DDD(DUK_DDDPRINT("p_insert=%p, val_insert=%ld, val_curr=%ld -> insert after this",
				                     (void *) p_insert, (long) val_insert, (long) val_curr));
				p_insert++;
				break;
			}
			if (p_insert == keys) {
				DUK_DDD(DUK_DDDPRINT("p_insert=%p -> out of keys, insert to beginning", (void *) p_insert));
				break;
			}
			DUK_DDD(DUK_DDDPRINT("p_insert=%p, val_insert=%ld, val_curr=%ld -> search backwards",
			                     (void *) p_insert, (long) val_insert, (long) val_curr));
			p_insert--;
		}

		DUK_DDD(DUK_DDDPRINT("final p_insert=%p", (void *) p_insert));

		/*        .-- p_insert   .-- p_curr
		 *        v              v
		 *  | ... | insert | ... | curr
		 */

		h_curr = *p_curr;
		DUK_DDD(DUK_DDDPRINT("memmove: dest=%p, src=%p, size=%ld, h_curr=%p",
		                     (void *) (p_insert + 1), (void *) p_insert,
		                     (long) (p_curr - p_insert), (void *) h_curr));

		DUK_MEMMOVE((void *) (p_insert + 1),
		            (void *) p_insert,
		            (size_t) ((p_curr - p_insert) * sizeof(duk_hstring *)));
		*p_insert = h_curr;
		/* keep val_highest */
	}

#ifdef DUK_USE_DDDPRINT
	{
		duk_uint_fast32_t i;
		for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(h_obj); i++) {
			DUK_DDD(DUK_DDDPRINT("final: %ld %p -> %!O",
			                     (long) i,
			                     (void *) DUK_HOBJECT_E_GET_KEY_PTR(thr->heap, h_obj, i),
			                     (duk_heaphdr *) DUK_HOBJECT_E_GET_KEY(thr->heap, h_obj, i)));
		}
	}
#endif
}

/*
 *  Create an internal enumerator object E, which has its keys ordered
 *  to match desired enumeration ordering.  Also initialize internal control
 *  properties for enumeration.
 *
 *  Note: if an array was used to hold enumeration keys instead, an array
 *  scan would be needed to eliminate duplicates found in the prototype chain.
 */

DUK_INTERNAL void duk_hobject_enumerator_create(duk_context *ctx, duk_small_uint_t enum_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *enum_target;
	duk_hobject *curr;
	duk_hobject *res;
#if defined(DUK_USE_ES6_PROXY)
	duk_hobject *h_proxy_target;
	duk_hobject *h_proxy_handler;
	duk_hobject *h_trap_result;
#endif
	duk_uint_fast32_t i, len;  /* used for array, stack, and entry indices */

	DUK_ASSERT(ctx != NULL);

	DUK_DDD(DUK_DDDPRINT("create enumerator, stack top: %ld", (long) duk_get_top(ctx)));

	enum_target = duk_require_hobject(ctx, -1);
	DUK_ASSERT(enum_target != NULL);

	duk_push_object_internal(ctx);
	res = duk_require_hobject(ctx, -1);

	DUK_DDD(DUK_DDDPRINT("created internal object"));

	/* [enum_target res] */

	/* Target must be stored so that we can recheck whether or not
	 * keys still exist when we enumerate.  This is not done if the
	 * enumeration result comes from a proxy trap as there is no
	 * real object to check against.
	 */
	duk_push_hobject(ctx, enum_target);
	duk_put_prop_stridx(ctx, -2, DUK_STRIDX_INT_TARGET);

	/* Initialize index so that we skip internal control keys. */
	duk_push_int(ctx, DUK__ENUM_START_INDEX);
	duk_put_prop_stridx(ctx, -2, DUK_STRIDX_INT_NEXT);

	/*
	 *  Proxy object handling
	 */

#if defined(DUK_USE_ES6_PROXY)
	if (DUK_LIKELY((enum_flags & DUK_ENUM_NO_PROXY_BEHAVIOR) != 0)) {
		goto skip_proxy;
	}
	if (DUK_LIKELY(!duk_hobject_proxy_check(thr,
	                                        enum_target,
	                                        &h_proxy_target,
	                                        &h_proxy_handler))) {
		goto skip_proxy;
	}

	DUK_DDD(DUK_DDDPRINT("proxy enumeration"));
	duk_push_hobject(ctx, h_proxy_handler);
	if (!duk_get_prop_stridx(ctx, -1, DUK_STRIDX_ENUMERATE)) {
		/* No need to replace the 'enum_target' value in stack, only the
		 * enum_target reference.  This also ensures that the original
		 * enum target is reachable, which keeps the proxy and the proxy
		 * target reachable.  We do need to replace the internal _Target.
		 */
		DUK_DDD(DUK_DDDPRINT("no enumerate trap, enumerate proxy target instead"));
		DUK_DDD(DUK_DDDPRINT("h_proxy_target=%!O", (duk_heaphdr *) h_proxy_target));
		enum_target = h_proxy_target;

		duk_push_hobject(ctx, enum_target);  /* -> [ ... enum_target res handler undefined target ] */
		duk_put_prop_stridx(ctx, -4, DUK_STRIDX_INT_TARGET);

		duk_pop_2(ctx);  /* -> [ ... enum_target res ] */
		goto skip_proxy;
	}

	/* [ ... enum_target res handler trap ] */
	duk_insert(ctx, -2);
	duk_push_hobject(ctx, h_proxy_target);    /* -> [ ... enum_target res trap handler target ] */
	duk_call_method(ctx, 1 /*nargs*/);        /* -> [ ... enum_target res trap_result ] */
	h_trap_result = duk_require_hobject(ctx, -1);
	DUK_UNREF(h_trap_result);

	/* Copy trap result keys into the enumerator object. */
	len = (duk_uint_fast32_t) duk_get_length(ctx, -1);
	for (i = 0; i < len; i++) {
		/* XXX: not sure what the correct semantic details are here,
		 * e.g. handling of missing values (gaps), handling of non-array
		 * trap results, etc.
		 *
		 * For keys, we simply skip non-string keys which seems to be
		 * consistent with how e.g. Object.keys() will process proxy trap
		 * results (ES6 draft, Section 19.1.2.14).
		 */
		if (duk_get_prop_index(ctx, -1, i) && duk_is_string(ctx, -1)) {
			/* [ ... enum_target res trap_result val ] */
			duk_push_true(ctx);
			/* [ ... enum_target res trap_result val true ] */
			duk_put_prop(ctx, -4);
		} else {
			duk_pop(ctx);
		}
	}
	/* [ ... enum_target res trap_result ] */
	duk_pop(ctx);
	duk_remove(ctx, -2);

	/* [ ... res ] */

	/* The internal _Target property is kept pointing to the original
	 * enumeration target (the proxy object), so that the enumerator
	 * 'next' operation can read property values if so requested.  The
	 * fact that the _Target is a proxy disables key existence check
	 * during enumeration.
	 */
	DUK_DDD(DUK_DDDPRINT("proxy enumeration, final res: %!O", (duk_heaphdr *) res));
	goto compact_and_return;

 skip_proxy:
#endif  /* DUK_USE_ES6_PROXY */

	curr = enum_target;
	while (curr) {
		/*
		 *  Virtual properties.
		 *
		 *  String and buffer indices are virtual and always enumerable,
		 *  'length' is virtual and non-enumerable.  Array and arguments
		 *  object props have special behavior but are concrete.
		 */

		if (DUK_HOBJECT_HAS_EXOTIC_STRINGOBJ(curr) ||
		    DUK_HOBJECT_HAS_EXOTIC_BUFFEROBJ(curr)) {
			/* String and buffer enumeration behavior is identical now,
			 * so use shared handler.
			 */
			if (DUK_HOBJECT_HAS_EXOTIC_STRINGOBJ(curr)) {
				duk_hstring *h_val;
				h_val = duk_hobject_get_internal_value_string(thr->heap, curr);
				DUK_ASSERT(h_val != NULL);  /* string objects must not created without internal value */
				len = (duk_uint_fast32_t) DUK_HSTRING_GET_CHARLEN(h_val);
			} else {
				duk_hbuffer *h_val;
				DUK_ASSERT(DUK_HOBJECT_HAS_EXOTIC_BUFFEROBJ(curr));
				h_val = duk_hobject_get_internal_value_buffer(thr->heap, curr);
				DUK_ASSERT(h_val != NULL);  /* buffer objects must not created without internal value */
				len = (duk_uint_fast32_t) DUK_HBUFFER_GET_SIZE(h_val);
			}

			for (i = 0; i < len; i++) {
				duk_hstring *k;

				k = duk_heap_string_intern_u32_checked(thr, i);
				DUK_ASSERT(k);
				duk_push_hstring(ctx, k);
				duk_push_true(ctx);

				/* [enum_target res key true] */
				duk_put_prop(ctx, -3);

				/* [enum_target res] */
			}

			/* 'length' property is not enumerable, but is included if
			 * non-enumerable properties are requested.
			 */

			if (enum_flags & DUK_ENUM_INCLUDE_NONENUMERABLE) {
				duk_push_hstring_stridx(ctx, DUK_STRIDX_LENGTH);
				duk_push_true(ctx);
				duk_put_prop(ctx, -3);
			}
		} else if (DUK_HOBJECT_HAS_EXOTIC_DUKFUNC(curr)) {
			if (enum_flags & DUK_ENUM_INCLUDE_NONENUMERABLE) {
				duk_push_hstring_stridx(ctx, DUK_STRIDX_LENGTH);
				duk_push_true(ctx);
				duk_put_prop(ctx, -3);
			}
		}

		/*
		 *  Array part
		 *
		 *  Note: ordering between array and entry part must match 'abandon array'
		 *  behavior in duk_hobject_props.c: key order after an array is abandoned
		 *  must be the same.
		 */

		for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ASIZE(curr); i++) {
			duk_hstring *k;
			duk_tval *tv;

			tv = DUK_HOBJECT_A_GET_VALUE_PTR(thr->heap, curr, i);
			if (DUK_TVAL_IS_UNDEFINED_UNUSED(tv)) {
				continue;
			}
			k = duk_heap_string_intern_u32_checked(thr, i);
			DUK_ASSERT(k);

			duk_push_hstring(ctx, k);
			duk_push_true(ctx);

			/* [enum_target res key true] */
			duk_put_prop(ctx, -3);

			/* [enum_target res] */
		}

		/*
		 *  Entries part
		 */

		for (i = 0; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(curr); i++) {
			duk_hstring *k;

			k = DUK_HOBJECT_E_GET_KEY(thr->heap, curr, i);
			if (!k) {
				continue;
			}
			if (!DUK_HOBJECT_E_SLOT_IS_ENUMERABLE(thr->heap, curr, i) &&
			    !(enum_flags & DUK_ENUM_INCLUDE_NONENUMERABLE)) {
				continue;
			}
			if (DUK_HSTRING_HAS_INTERNAL(k) &&
			    !(enum_flags & DUK_ENUM_INCLUDE_INTERNAL)) {
				continue;
			}
			if ((enum_flags & DUK_ENUM_ARRAY_INDICES_ONLY) &&
			    (DUK_HSTRING_GET_ARRIDX_SLOW(k) == DUK_HSTRING_NO_ARRAY_INDEX)) {
				continue;
			}

			DUK_ASSERT(DUK_HOBJECT_E_SLOT_IS_ACCESSOR(thr->heap, curr, i) ||
			           !DUK_TVAL_IS_UNDEFINED_UNUSED(&DUK_HOBJECT_E_GET_VALUE_PTR(thr->heap, curr, i)->v));

			duk_push_hstring(ctx, k);
			duk_push_true(ctx);

			/* [enum_target res key true] */
			duk_put_prop(ctx, -3);

			/* [enum_target res] */
		}

		if (enum_flags & DUK_ENUM_OWN_PROPERTIES_ONLY) {
			break;
		}

		curr = DUK_HOBJECT_GET_PROTOTYPE(thr->heap, curr);
	}

	/* [enum_target res] */

	duk_remove(ctx, -2);

	/* [res] */

	if ((enum_flags & (DUK_ENUM_ARRAY_INDICES_ONLY | DUK_ENUM_SORT_ARRAY_INDICES)) ==
	                  (DUK_ENUM_ARRAY_INDICES_ONLY | DUK_ENUM_SORT_ARRAY_INDICES)) {
		/*
		 *  Some E5/E5.1 algorithms require that array indices are iterated
		 *  in a strictly ascending order.  This is the case for e.g.
		 *  Array.prototype.forEach() and JSON.stringify() PropertyList
		 *  handling.
		 *
		 *  To ensure this property for arrays with an array part (and
		 *  arbitrary objects too, since e.g. forEach() can be applied
		 *  to an array), the caller can request that we sort the keys
		 *  here.
		 */

		/* XXX: avoid this at least when enum_target is an Array, it has an
		 * array part, and no ancestor properties were included?  Not worth
		 * it for JSON, but maybe worth it for forEach().
		 */

		/* XXX: may need a 'length' filter for forEach()
		 */
		DUK_DDD(DUK_DDDPRINT("sort array indices by caller request"));
		duk__sort_array_indices(thr, res);
	}

#if defined(DUK_USE_ES6_PROXY)
 compact_and_return:
#endif
	/* compact; no need to seal because object is internal */
	duk_hobject_compact_props(thr, res);

	DUK_DDD(DUK_DDDPRINT("created enumerator object: %!iT", (duk_tval *) duk_get_tval(ctx, -1)));
}

/*
 *  Returns non-zero if a key and/or value was enumerated, and:
 *
 *   [enum] -> [key]        (get_value == 0)
 *   [enum] -> [key value]  (get_value == 1)
 *
 *  Returns zero without pushing anything on the stack otherwise.
 */
DUK_INTERNAL duk_bool_t duk_hobject_enumerator_next(duk_context *ctx, duk_bool_t get_value) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *e;
	duk_hobject *enum_target;
	duk_hstring *res = NULL;
	duk_uint_fast32_t idx;
	duk_bool_t check_existence;

	DUK_ASSERT(ctx != NULL);

	/* [... enum] */

	e = duk_require_hobject(ctx, -1);

	/* XXX use get tval ptr, more efficient */
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_NEXT);
	idx = (duk_uint_fast32_t) duk_require_uint(ctx, -1);
	duk_pop(ctx);
	DUK_DDD(DUK_DDDPRINT("enumeration: index is: %ld", (long) idx));

	/* Enumeration keys are checked against the enumeration target (to see
	 * that they still exist).  In the proxy enumeration case _Target will
	 * be the proxy, and checking key existence against the proxy is not
	 * required (or sensible, as the keys may be fully virtual).
	 */
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_TARGET);
	enum_target = duk_require_hobject(ctx, -1);
	DUK_ASSERT(enum_target != NULL);
#if defined(DUK_USE_ES6_PROXY)
	check_existence = (!DUK_HOBJECT_HAS_EXOTIC_PROXYOBJ(enum_target));
#else
	check_existence = 1;
#endif
	duk_pop(ctx);  /* still reachable */

	DUK_DDD(DUK_DDDPRINT("getting next enum value, enum_target=%!iO, enumerator=%!iT",
	                     (duk_heaphdr *) enum_target, (duk_tval *) duk_get_tval(ctx, -1)));

	/* no array part */
	for (;;) {
		duk_hstring *k;

		if (idx >= DUK_HOBJECT_GET_ENEXT(e)) {
			DUK_DDD(DUK_DDDPRINT("enumeration: ran out of elements"));
			break;
		}

		/* we know these because enum objects are internally created */
		k = DUK_HOBJECT_E_GET_KEY(thr->heap, e, idx);
		DUK_ASSERT(k != NULL);
		DUK_ASSERT(!DUK_HOBJECT_E_SLOT_IS_ACCESSOR(thr->heap, e, idx));
		DUK_ASSERT(!DUK_TVAL_IS_UNDEFINED_UNUSED(&DUK_HOBJECT_E_GET_VALUE(thr->heap, e, idx).v));

		idx++;

		/* recheck that the property still exists */
		if (check_existence && !duk_hobject_hasprop_raw(thr, enum_target, k)) {
			DUK_DDD(DUK_DDDPRINT("property deleted during enumeration, skip"));
			continue;
		}

		DUK_DDD(DUK_DDDPRINT("enumeration: found element, key: %!O", (duk_heaphdr *) k));
		res = k;
		break;
	}

	DUK_DDD(DUK_DDDPRINT("enumeration: updating next index to %ld", (long) idx));

	duk_push_u32(ctx, (duk_uint32_t) idx);
	duk_put_prop_stridx(ctx, -2, DUK_STRIDX_INT_NEXT);

	/* [... enum] */

	if (res) {
		duk_push_hstring(ctx, res);
		if (get_value) {
			duk_push_hobject(ctx, enum_target);
			duk_dup(ctx, -2);      /* -> [... enum key enum_target key] */
			duk_get_prop(ctx, -2); /* -> [... enum key enum_target val] */
			duk_remove(ctx, -2);   /* -> [... enum key val] */
			duk_remove(ctx, -3);   /* -> [... key val] */
		} else {
			duk_remove(ctx, -2);   /* -> [... key] */
		}
		return 1;
	} else {
		duk_pop(ctx);  /* -> [...] */
		return 0;
	}
}

/*
 *  Get enumerated keys in an Ecmascript array.  Matches Object.keys() behavior
 *  described in E5 Section 15.2.3.14.
 */

DUK_INTERNAL duk_ret_t duk_hobject_get_enumerated_keys(duk_context *ctx, duk_small_uint_t enum_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *e;
	duk_uint_fast32_t i;
	duk_uint_fast32_t idx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(duk_get_hobject(ctx, -1) != NULL);
	DUK_UNREF(thr);

	/* Create a temporary enumerator to get the (non-duplicated) key list;
	 * the enumerator state is initialized without being needed, but that
	 * has little impact.
	 */

	duk_hobject_enumerator_create(ctx, enum_flags);
	duk_push_array(ctx);

	/* [enum_target enum res] */

	e = duk_require_hobject(ctx, -2);
	DUK_ASSERT(e != NULL);

	idx = 0;
	for (i = DUK__ENUM_START_INDEX; i < (duk_uint_fast32_t) DUK_HOBJECT_GET_ENEXT(e); i++) {
		duk_hstring *k;

		k = DUK_HOBJECT_E_GET_KEY(thr->heap, e, i);
		DUK_ASSERT(k);  /* enumerator must have no keys deleted */

		/* [enum_target enum res] */
		duk_push_hstring(ctx, k);
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}

	/* [enum_target enum res] */
	duk_remove(ctx, -2);

	/* [enum_target res] */

	return 1;  /* return 1 to allow callers to tail call */
}
