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

/*
 *  Create an internal enumerator object E, which has its keys ordered
 *  to match desired enumeration ordering.  Also initialize internal control
 *  properties for enumeration.
 *
 *  Note: if an array was used to hold enumeration keys instead, an array
 *  scan would be needed to eliminate duplicates found in the prototype chain.
 */

/* FIXME: identify enumeration target with an object index (not top of stack) */

/* must match exactly the number of internal properties inserted to enumerator */
#define  ENUM_START_INDEX  2

void duk_hobject_enumerator_create(duk_context *ctx, int enum_flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *target;
	duk_hobject *curr;
	duk_hobject *res;

	DUK_ASSERT(ctx != NULL);

	DUK_DPRINT("create enumerator, stack top: %d", duk_get_top(ctx));

	target = duk_require_hobject(ctx, -1);
	DUK_ASSERT(target != NULL);

	duk_push_new_object_internal(ctx);

	DUK_DPRINT("created internal object");

	/* [target res] */

	duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_INT_TARGET);
	duk_push_hobject(ctx, target);
	duk_put_prop(ctx, -3);

	/* initialize index so that we skip internal control keys */
	duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_INT_NEXT);
	duk_push_int(ctx, ENUM_START_INDEX);
	duk_put_prop(ctx, -3);

	curr = target;
	while (curr) {
		duk_u32 i;

		/*
		 *  Virtual properties.
		 *
		 *  String indices are always enumerable.
		 */

		if (DUK_HOBJECT_HAS_SPECIAL_STRINGOBJ(curr)) {
			duk_hstring *h_val;

			h_val = duk_hobject_get_internal_value_string(thr->heap, curr);
			DUK_ASSERT(h_val != NULL);  /* string objects must not created without internal value */

			/* FIXME: type for 'i' to match string max len (duk_u32) */
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

	/* [res] */

	/* compact; no need to seal because object is internal */
	res = duk_require_hobject(ctx, -1);
	duk_hobject_compact_props(thr, res);

	DUK_DDDPRINT("created enumerator object: %!iT", duk_get_tval(ctx, -1));
}

/* [enum] -> [key]  (get_value == 0)
 * [enum] -> [key value]  (get_value == 1)
 *
 * Returns non-zero if a key was enumerated.
 */
int duk_hobject_enumerator_next(duk_context *ctx, int get_value) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *e;
	duk_hobject *target;
	duk_hstring *res = NULL;
	duk_u32 idx;

	DUK_ASSERT(ctx != NULL);

	/* [enum] */

	e = duk_require_hobject(ctx, -1);

	/* FIXME: use get tval ptr, more efficient */
	duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_INT_NEXT);
	idx = (duk_u32) duk_require_number(ctx, -1);
	duk_pop(ctx);
	DUK_DPRINT("enumeration: index is: %d", idx);

	duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_INT_TARGET);
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
	duk_put_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_INT_NEXT);

	if (res) {
		duk_push_hstring(ctx, res);
	} else {
		duk_push_undefined(ctx);
	}

	/* [enum key] */
	duk_remove(ctx, -2);

	/* [key] */

	if (get_value) {
		duk_push_hobject(ctx, target);
		duk_dup(ctx, -2);      /* -> [key target key] */
		duk_get_prop(ctx, -2); /* -> [key target val] */
		duk_remove(ctx, -2);   /* -> [key val] */
	}

	return (res ? 1 : 0);
}

/*
 *  Get enumerated keys in an Ecmascript array.  Matches Object.keys() behavior
 *  described in E5 Section 15.2.3.14.
 */

void duk_hobject_get_enumerated_keys(duk_context *ctx, int enum_flags) {
	duk_hobject *e;
	int i;
	int idx;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(duk_get_hobject(ctx, -1) != NULL);

	/* Create a temporary enumerator to get the (non-duplicated) key list;
	 * the enumerator state is initialized without being needed, but that
	 * has little impact.
	 */

	duk_hobject_enumerator_create(ctx, enum_flags);
	duk_push_new_array(ctx);

	/* [target enum res] */

	e = duk_require_hobject(ctx, -2);
	DUK_ASSERT(e != NULL);

	idx = 0;
	for (i = ENUM_START_INDEX; i < e->e_used; i++) {
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
}

