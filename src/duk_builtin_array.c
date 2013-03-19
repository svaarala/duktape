/*
 *  Array built-ins
 *
 *  Note that most Array built-ins are intentionally generic and work even
 *  when the 'this' binding is not an Array instance.  To ensure this,
 *  Array algorithms do not assume "magical" Array behavior for the "length"
 *  property, for instance.
 */

#include "duk_internal.h"

int duk_builtin_array_constructor(duk_context *ctx) {
	if (duk_is_constructor_call(ctx)) {
		return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
	} else {
		return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
	}
}

int duk_builtin_array_constructor_is_array(duk_context *ctx) {
	duk_hobject *h;

	h = duk_get_hobject_with_class(ctx, 0, DUK_HOBJECT_CLASS_ARRAY);
	duk_push_boolean(ctx, (h != NULL));
	return 1;
}

int duk_builtin_array_prototype_to_string(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;

	duk_push_this(ctx);
	DUK_DDDPRINT("this: %!T", duk_get_tval(ctx, -1));

	duk_to_object(ctx, -1);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_JOIN);

	/* FIXME: uneven stack would reduce code size */

	/* [ ... this func ] */
	if (!duk_is_callable(ctx, -1)) {
		/* FIXME: this is incorrect, must use the built-in toString() */
		DUK_DDDPRINT("this.join is not callable, fall back to Object.toString");
		duk_pop(ctx);
		duk_push_hobject(ctx, thr->builtins[DUK_BIDX_OBJECT_PROTOTYPE]);
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TO_STRING);
		duk_remove(ctx, -2);
	}

	/* [ ... this func ] */

	duk_insert(ctx, -2);

	/* [ ... func this ] */

	DUK_DDDPRINT("calling: func=%!iT, this=%!iT", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
	duk_call_method(ctx, 0);

	return 1;
}

int duk_builtin_array_prototype_to_locale_string(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_concat(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

/* Note: checking valstack is necessary, but only in the per-element loop */

/* FIXME: This placeholder does not work for a large number of elements.
 * Provide proper hierarchical concat/join primitives in the API and use
 * them here.
 */

int duk_builtin_array_prototype_join(duk_context *ctx) {
	duk_u32 len;
	duk_u32 i;

	DUK_ASSERT_TOP(ctx, 1);  /* nargs is 1 */
	if (duk_is_undefined(ctx, 0)) {
		duk_pop(ctx);
		duk_push_hstring_stridx(ctx, DUK_STRIDX_COMMA);
	} else {
		duk_to_string(ctx, 0);
	}

	duk_push_this(ctx);
	duk_to_object(ctx, -1);  /* FIXME: common enough to warrant an internal helper?  push_this_to_object */

	/* [ sep ToObject(this) ] */

	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_LENGTH);
	len = duk_to_uint32(ctx, -1);

	DUK_DDDPRINT("sep=%!T, this=%!T, len=%d", duk_get_tval(ctx, 0), duk_get_tval(ctx, 1), len);

	duk_require_stack(ctx, len + 1);
	duk_dup(ctx, 0);

	/* [ sep ToObject(this) sep ] */

	for (i = 0; i < len; i++) {
		duk_get_prop_index(ctx, 1, i);
		if (duk_is_null_or_undefined(ctx, -1)) {
			duk_pop(ctx);
			duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);
		} else {
			duk_to_string(ctx, -1);
		}
	}

	/* [ sep ToObject(this) sep str0 ... str(len-1) ] */

	duk_join(ctx, len);
	return 1;
}

int duk_builtin_array_prototype_pop(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_push(duk_context *ctx) {
	/* Note: 'this' is not necessarily an Array object.  The push()
	 * algorithm is supposed to work for other kinds of objects too,
	 * so the algorithm has e.g. an explicit update for the 'length'
	 * property which is normally "magical" in arrays.
	 */
	duk_u32 len;
	int i, n;

	n = duk_get_top(ctx);

	duk_push_this(ctx);
	duk_to_object(ctx, -1);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_LENGTH);
	len = duk_to_uint32(ctx, -1);

	/* [ arg1 ... argN obj length ] */

	for (i = 0; i < n; i++) {
		duk_dup(ctx, i);
		duk_put_prop_index(ctx, -3, len);  /* FIXME: "Throw" is true for this [[Put]] call, needs API support */
		len++;
	}

	duk_push_number(ctx, (double) len);  /* FIXME: duk_push_u32 */
	duk_dup_top(ctx);
	duk_put_prop_stridx(ctx, -3, DUK_STRIDX_LENGTH);

	/* [ arg1 ... argN obj length new_length ] */
	return 1;
}

int duk_builtin_array_prototype_reverse(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_shift(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_slice(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_sort(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_splice(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_unshift(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_index_of(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_last_index_of(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_every(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_some(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_for_each(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_map(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_array_prototype_filter(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

static int reduce_helper(duk_context *ctx, int idx_step) {
	int nargs;
	int have_acc;
	int i, len;

	/* idx_step is +1 for reduce, -1 for reduceRight */

	/* We're a varargs function because we need to detect whether
	 * initialValue was given or not.
	 */
	nargs = duk_get_top(ctx);
	DUK_DPRINT("nargs=%d", nargs);

	duk_set_top(ctx, 2);
	duk_push_this_coercible_to_object(ctx);
	len = duk_get_length(ctx, -1);
	if (!duk_is_callable(ctx, 0)) {
		goto type_error;
	}

	/* stack[0] = callback fn
	 * stack[1] = initialValue
	 * stack[2] = object (coerced this)
	 * stack[3] = accumulator
	 */

	have_acc = 0;
	if (nargs >= 2) {
		duk_dup(ctx, 1);
		have_acc = 1;
	}
	DUK_DPRINT("have_acc=%d, acc=%!T", have_acc, duk_get_tval(ctx, 3));

	for (i = (idx_step >= 0 ? 0 : len - 1);
	     i >= 0 && i < len;
	     i += idx_step) {
		DUK_DPRINT("i=%d, len=%d, have_acc=%d, top=%d, acc=%!T",
		           i, len, have_acc, duk_get_top(ctx), duk_get_tval(ctx, 3));

		DUK_ASSERT((have_acc && duk_get_top(ctx) == 4) ||
		           (!have_acc && duk_get_top(ctx) == 3));

		if (!duk_has_prop_index(ctx, 2, i)) {
			continue;
		}

		if (!have_acc) {
			DUK_ASSERT_TOP(ctx, 3);
			duk_get_prop_index(ctx, 2, i);
			have_acc = 1;
			DUK_ASSERT_TOP(ctx, 4);
		} else {
			DUK_ASSERT_TOP(ctx, 4);
			duk_dup(ctx, 0);
			duk_dup(ctx, 3);
			duk_get_prop_index(ctx, 2, i);
			duk_push_int(ctx, i);  /* FIXME: type */
			duk_dup(ctx, 2);
			DUK_DPRINT("calling reduce function: func=%!T, prev=%!T, curr=%!T, idx=%!T, obj=%!T",
			           duk_get_tval(ctx, -5), duk_get_tval(ctx, -4), duk_get_tval(ctx, -3),
			           duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
			duk_call(ctx, 4);
			DUK_DPRINT("-> result: %!T", duk_get_tval(ctx, -1));
			duk_replace(ctx, 3);
			DUK_ASSERT_TOP(ctx, 4);
		}
	}

	if (!have_acc) {
		goto type_error;
	}

	DUK_ASSERT_TOP(ctx, 4);
	return 1;

 type_error:
	return DUK_RET_TYPE_ERROR;
}

int duk_builtin_array_prototype_reduce(duk_context *ctx) {
	return reduce_helper(ctx, 1 /*idx_step*/);
}

int duk_builtin_array_prototype_reduce_right(duk_context *ctx) {
	return reduce_helper(ctx, -1 /*idx_step*/);
}

