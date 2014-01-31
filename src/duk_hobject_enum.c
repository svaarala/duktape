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

/* FIXME: identify enumeration target with an object index (not top of stack) */

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

static void sort_array_indices(duk_hobject *h_obj) {
	duk_hstring **keys;
	duk_hstring **p_curr, **p_insert, **p_end;
	duk_hstring *h_curr;
	duk_uint32_t val_highest, val_curr, val_insert;

	DUK_ASSERT(h_obj != NULL);
	DUK_ASSERT(h_obj->e_used >= 2);  /* control props */

	if (h_obj->e_used <= 1 + DUK__ENUM_START_INDEX) {
		return;
	}

	keys = DUK_HOBJECT_E_GET_KEY_BASE(h_obj);
	p_end = keys + h_obj->e_used;
	keys += DUK__ENUM_START_INDEX;

	DUK_DDDPRINT("keys=%p, p_end=%p (after skipping enum props)",
	             (void *) keys, (void *) p_end);

#ifdef DUK_USE_DDDEBUG
	{
		int i;
		for (i = 0; i < h_obj->e_used; i++) {
			DUK_DDDPRINT("initial: %d %p -> %!O",
			             i,
			             (void *) DUK_HOBJECT_E_GET_KEY_PTR(h_obj, i),
			             (void *) DUK_HOBJECT_E_GET_KEY(h_obj, i));
		}
	}
#endif

	val_highest = DUK_HSTRING_GET_ARRIDX_SLOW(keys[0]);
	for (p_curr = keys + 1; p_curr < p_end; p_curr++) {
		DUK_ASSERT(*p_curr != NULL);
		val_curr = DUK_HSTRING_GET_ARRIDX_SLOW(*p_curr);

		if (val_curr >= val_highest) {
			DUK_DDDPRINT("p_curr=%p, p_end=%p, val_highest=%d, val_curr=%d -> "
			             "already in correct order, next",
			             (void *) p_curr, (void *) p_end, (int) val_highest, (int) val_curr);
			val_highest = val_curr;
			continue;
		}

		DUK_DDDPRINT("p_curr=%p, p_end=%p, val_highest=%d, val_curr=%d -> "
		             "needs to be inserted",
		             (void *) p_curr, (void *) p_end, (int) val_highest, (int) val_curr);
	
		/* Needs to be inserted; scan backwards, since we optimize
		 * for the case where elements are nearly in order.
		 */

		p_insert = p_curr - 1;
		for (;;) {
			val_insert = DUK_HSTRING_GET_ARRIDX_SLOW(*p_insert);
			if (val_insert < val_curr) {
				DUK_DDDPRINT("p_insert=%p, val_insert=%d, val_curr=%d -> insert after this",
				             (void *) p_insert, (int) val_insert, (int) val_curr);
				p_insert++;
				break;
			}
			if (p_insert == keys) {
				DUK_DDDPRINT("p_insert=%p -> out of keys, insert to beginning");
				break;
			}
			DUK_DDDPRINT("p_insert=%p, val_insert=%d, val_curr=%d -> search backwards",
			             (void *) p_insert, (int) val_insert, (int) val_curr);
			p_insert--;
		}

		DUK_DDDPRINT("final p_insert=%p", (void *) p_insert);

		/*        .-- p_insert   .-- p_curr
		 *        v              v
		 *  | ... | insert | ... | curr
		 */

		h_curr = *p_curr;
		DUK_DDDPRINT("memmove: dest=%p, src=%p, size=%d, h_curr=%p",
		             (void *) (p_insert + 1), (void *) p_insert,
		             (int) (p_curr - p_insert), (void *) h_curr);

		DUK_MEMMOVE((void *) (p_insert + 1),
		            (void *) p_insert,
		            (size_t) ((p_curr - p_insert) * sizeof(duk_hstring *)));
		*p_insert = h_curr;
		/* keep val_highest */
	}

#ifdef DUK_USE_DDDEBUG
	{
		int i;
		for (i = 0; i < h_obj->e_used; i++) {
			DUK_DDDPRINT("final: %d %p -> %!O",
			             i,
			             (void *) DUK_HOBJECT_E_GET_KEY_PTR(h_obj, i),
			             (void *) DUK_HOBJECT_E_GET_KEY(h_obj, i));
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

void duk_hobject_enumerator_create(duk_context *ctx, int enum_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *target;
	duk_hobject *curr;
	duk_hobject *res;

	DUK_ASSERT(ctx != NULL);

	DUK_DDDPRINT("create enumerator, stack top: %d", duk_get_top(ctx));

	target = duk_require_hobject(ctx, -1);
	DUK_ASSERT(target != NULL);

	duk_push_object_internal(ctx);

	DUK_DDDPRINT("created internal object");

	/* [target res] */

	duk_push_hstring_stridx(ctx, DUK_STRIDX_INT_TARGET);
	duk_push_hobject(ctx, target);
	duk_put_prop(ctx, -3);

	/* initialize index so that we skip internal control keys */
	duk_push_hstring_stridx(ctx, DUK_STRIDX_INT_NEXT);
	duk_push_int(ctx, DUK__ENUM_START_INDEX);
	duk_put_prop(ctx, -3);

	curr = target;
	while (curr) {
		duk_uint32_t i;

		/*
		 *  Virtual properties.
		 *
		 *  String indices are virtual and always enumerable.  String 'length'
		 *  is virtual and non-enumerable.  Array and arguments object props
		 *  have special behavior but are concrete.
		 */

		if (DUK_HOBJECT_HAS_SPECIAL_STRINGOBJ(curr)) {
			duk_hstring *h_val;

			h_val = duk_hobject_get_internal_value_string(thr->heap, curr);
			DUK_ASSERT(h_val != NULL);  /* string objects must not created without internal value */

			/* FIXME: type for 'i' to match string max len (duk_uint32_t) */
			for (i = 0; i < DUK_HSTRING_GET_CHARLEN(h_val); i++) {
				duk_hstring *k;

				k = duk_heap_string_intern_u32_checked(thr, i);
				DUK_ASSERT(k);
				duk_push_hstring(ctx, k);
				duk_push_true(ctx);

				/* [target res key true] */
				duk_put_prop(ctx, -3);

				/* [target res] */	
			}

			/* 'length' property is not enumerable, but is included if
			 * non-enumerable properties are requested.
			 */

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

		for (i = 0; i < curr->a_size; i++) {
			duk_hstring *k;
			duk_tval *tv;

			tv = DUK_HOBJECT_A_GET_VALUE_PTR(curr, i);
			if (DUK_TVAL_IS_UNDEFINED_UNUSED(tv)) {
				continue;
			}
			k = duk_heap_string_intern_u32_checked(thr, i);
			DUK_ASSERT(k);

			duk_push_hstring(ctx, k);
			duk_push_true(ctx);

			/* [target res key true] */
			duk_put_prop(ctx, -3);

			/* [target res] */
		}

		/*
		 *  Entries part
		 */

		for (i = 0; i < curr->e_used; i++) {
			duk_hstring *k;

			k = DUK_HOBJECT_E_GET_KEY(curr, i);
			if (!k) {
				continue;
			}
			if (!DUK_HOBJECT_E_SLOT_IS_ENUMERABLE(curr, i) &&
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

			DUK_ASSERT(DUK_HOBJECT_E_SLOT_IS_ACCESSOR(curr, i) ||
			           !DUK_TVAL_IS_UNDEFINED_UNUSED(&DUK_HOBJECT_E_GET_VALUE_PTR(curr, i)->v));

			duk_push_hstring(ctx, k);
			duk_push_true(ctx);

			/* [target res key true] */
			duk_put_prop(ctx, -3);

			/* [target res] */
		}

		if (enum_flags & DUK_ENUM_OWN_PROPERTIES_ONLY) {
			break;
		}

		curr = curr->prototype;
	}

	/* [target res] */

	duk_remove(ctx, -2);
	res = duk_require_hobject(ctx, -1);

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

		/* FIXME: avoid this at least when target is an Array, it has an
		 * array part, and no ancestor properties were included?  Not worth
		 * it for JSON, but maybe worth it for forEach().
		 */

		/* FIXME: may need a 'length' filter for forEach()
		 */
		DUK_DDDPRINT("sort array indices by caller request");
		sort_array_indices(res);
	}

	/* compact; no need to seal because object is internal */
	duk_hobject_compact_props(thr, res);

	DUK_DDDPRINT("created enumerator object: %!iT", duk_get_tval(ctx, -1));
}

/*
 *  Returns non-zero if a key and/or value was enumerated, and:
 *
 *   [enum] -> [key]        (get_value == 0)
 *   [enum] -> [key value]  (get_value == 1)
 *
 *  Returns zero without pushing anything on the stack otherwise.
 */
int duk_hobject_enumerator_next(duk_context *ctx, int get_value) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *e;
	duk_hobject *target;
	duk_hstring *res = NULL;
	duk_uint32_t idx;

	DUK_ASSERT(ctx != NULL);

	/* [... enum] */

	e = duk_require_hobject(ctx, -1);

	/* FIXME: use get tval ptr, more efficient */
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_NEXT);
	idx = (duk_uint32_t) duk_require_number(ctx, -1);
	duk_pop(ctx);
	DUK_DDDPRINT("enumeration: index is: %d", idx);

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_TARGET);
	target = duk_require_hobject(ctx, -1);
	DUK_ASSERT(target != NULL);
	duk_pop(ctx);  /* still reachable */

	DUK_DDDPRINT("getting next enum value, target=%!iO, enumerator=%!iT",
	             target, duk_get_tval(ctx, -1));

	/* no array part */
	for (;;) {
		duk_hstring *k;

		if (idx >= e->e_used) {
			DUK_DDDPRINT("enumeration: ran out of elements");
			break;
		}

		/* we know these because enum objects are internally created */
		k = DUK_HOBJECT_E_GET_KEY(e, idx);
		DUK_ASSERT(k != NULL);
		DUK_ASSERT(!DUK_HOBJECT_E_SLOT_IS_ACCESSOR(e, idx));
		DUK_ASSERT(!DUK_TVAL_IS_UNDEFINED_UNUSED(&DUK_HOBJECT_E_GET_VALUE(e, idx).v));

		idx++;

		/* recheck that the property still exists */
		if (!duk_hobject_hasprop_raw(thr, target, k)) {
			DUK_DDDPRINT("property deleted during enumeration, skip");
			continue;
		}

		DUK_DDDPRINT("enumeration: found element, key: %!O", k);
		res = k;
		break;
	}

	DUK_DDDPRINT("enumeration: updating next index to %d", idx);

	duk_push_number(ctx, (double) idx);
	duk_put_prop_stridx(ctx, -2, DUK_STRIDX_INT_NEXT);

	/* [... enum] */

	if (res) {
		duk_push_hstring(ctx, res);
		if (get_value) {
			duk_push_hobject(ctx, target);
			duk_dup(ctx, -2);      /* -> [... enum key target key] */
			duk_get_prop(ctx, -2); /* -> [... enum key target val] */
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

int duk_hobject_get_enumerated_keys(duk_context *ctx, int enum_flags) {
	duk_hobject *e;
	duk_uint32_t i;
	duk_uint32_t idx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(duk_get_hobject(ctx, -1) != NULL);

	/* Create a temporary enumerator to get the (non-duplicated) key list;
	 * the enumerator state is initialized without being needed, but that
	 * has little impact.
	 */

	duk_hobject_enumerator_create(ctx, enum_flags);
	duk_push_array(ctx);

	/* [target enum res] */

	e = duk_require_hobject(ctx, -2);
	DUK_ASSERT(e != NULL);

	idx = 0;
	for (i = DUK__ENUM_START_INDEX; i < e->e_used; i++) {
		duk_hstring *k;

		k = DUK_HOBJECT_E_GET_KEY(e, i);
		DUK_ASSERT(k);  /* enumerator must have no keys deleted */

		/* [target enum res] */
		duk_push_hstring(ctx, k);
		duk_put_prop_index(ctx, -2, idx);
		idx++;
	}

	/* [target enum res] */
	duk_remove(ctx, -2);

	/* [target res] */

	return 1;  /* return 1 to allow callers to tail call */
}

