/*
 *  RegExp built-ins
 */

#include "duk_internal.h"

#ifdef DUK_USE_REGEXP_SUPPORT

static void get_this_regexp(duk_context *ctx) {
	duk_hobject *h;

	duk_push_this(ctx);
	h = duk_require_hobject_with_class(ctx, -1, DUK_HOBJECT_CLASS_REGEXP);
	DUK_ASSERT(h != NULL);
	DUK_UNREF(h);
	duk_insert(ctx, 0);  /* prepend regexp to valstack 0 index */
}

/* FIXME: much to improve (code size) */
int duk_bi_regexp_constructor(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h_pattern;

	DUK_ASSERT_TOP(ctx, 2);
	h_pattern = duk_get_hobject(ctx, 0);

	if (!duk_is_constructor_call(ctx) &&
	    h_pattern != NULL &&
	    DUK_HOBJECT_GET_CLASS_NUMBER(h_pattern) == DUK_HOBJECT_CLASS_REGEXP &&
	    duk_is_undefined(ctx, 1)) {
		/* Called as a function, pattern has [[Class]] "RegExp" and
		 * flags is undefined -> return object as is.
		 */
		duk_dup(ctx, 0);
		return 1;
	}

	/* Else functionality is identical for function call and constructor
	 * call.
	 */

	if (h_pattern != NULL &&
	    DUK_HOBJECT_GET_CLASS_NUMBER(h_pattern) == DUK_HOBJECT_CLASS_REGEXP) {
		if (duk_is_undefined(ctx, 1)) {
			int flag_g, flag_i, flag_m;
			duk_get_prop_stridx(ctx, 0, DUK_STRIDX_SOURCE);

			/* FIXME: very awkward handling of flags */
			duk_get_prop_stridx(ctx, 0, DUK_STRIDX_GLOBAL);
			flag_g = duk_to_boolean(ctx, -1);
			duk_get_prop_stridx(ctx, 0, DUK_STRIDX_IGNORE_CASE);
			flag_i = duk_to_boolean(ctx, -1);
			duk_get_prop_stridx(ctx, 0, DUK_STRIDX_MULTILINE);
			flag_m = duk_to_boolean(ctx, -1);
			duk_pop_n(ctx, 3);

			duk_push_sprintf(ctx, "%s%s%s",
			                 (flag_g ? "g" : ""),
			                 (flag_i ? "i" : ""),
			                 (flag_m ? "m" : ""));

			/* [ ... pattern flags ] */
		} else {
			return DUK_RET_TYPE_ERROR;
		}
	} else {
		if (duk_is_undefined(ctx, 0)) {
			duk_push_string(ctx, "");
		} else {
			duk_dup(ctx, 0);
			duk_to_string(ctx, -1);
		}
		if (duk_is_undefined(ctx, 1)) {
			duk_push_string(ctx, "");
		} else {
			duk_dup(ctx, 1);
			duk_to_string(ctx, -1);
		}

		/* [ ... pattern flags ] */
	}

	DUK_DDDPRINT("RegExp constructor/function call, pattern=%!T, flags=%!T",
	             duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

	/* [ ... pattern flags ] */

	duk_regexp_compile(thr);

	/* [ ... bytecode escaped_source ] */

	duk_regexp_create_instance(thr);

	/* [ ... RegExp ] */

	return 1;
}

int duk_bi_regexp_prototype_exec(duk_context *ctx) {
	get_this_regexp(ctx);

	/* [ regexp input ] */

	duk_regexp_match((duk_hthread *) ctx);

	/* [ result ] */

	return 1;
}

int duk_bi_regexp_prototype_test(duk_context *ctx) {
	get_this_regexp(ctx);

	/* [ regexp input ] */

	/* result object is created and discarded; wasteful but saves code space */
	duk_regexp_match((duk_hthread *) ctx);

	/* [ result ] */

	duk_push_boolean(ctx, (duk_is_null(ctx, -1) ? 0 : 1));

	return 1;
}

int duk_bi_regexp_prototype_to_string(duk_context *ctx) {
	duk_hstring *h_bc;
	int re_flags;

	get_this_regexp(ctx);

	/* [ regexp ] */

	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_SOURCE);
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_INT_BYTECODE);
	h_bc = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_bc != NULL);
	DUK_ASSERT(DUK_HSTRING_GET_BYTELEN(h_bc) >= 1);
	DUK_ASSERT(DUK_HSTRING_GET_CHARLEN(h_bc) >= 1);
	DUK_ASSERT(DUK_HSTRING_GET_DATA(h_bc)[0] < 0x80);
	re_flags = (int) DUK_HSTRING_GET_DATA(h_bc)[0];

	/* [ regexp source bytecode ] */

	duk_push_sprintf(ctx, "/%s/%s%s%s",
	                 duk_get_string(ctx, -2),
	                 (re_flags & DUK_RE_FLAG_GLOBAL) ? "g" : "",
	                 (re_flags & DUK_RE_FLAG_IGNORE_CASE) ? "i" : "",
	                 (re_flags & DUK_RE_FLAG_MULTILINE) ? "m" : "");

	return 1;
}

#else  /* DUK_USE_REGEXP_SUPPORT */

int duk_bi_regexp_constructor(duk_context *ctx) {
	return DUK_RET_UNSUPPORTED_ERROR;
}

int duk_bi_regexp_prototype_exec(duk_context *ctx) {
	return DUK_RET_UNSUPPORTED_ERROR;
}

int duk_bi_regexp_prototype_test(duk_context *ctx) {
	return DUK_RET_UNSUPPORTED_ERROR;
}

int duk_bi_regexp_prototype_to_string(duk_context *ctx) {
	return DUK_RET_UNSUPPORTED_ERROR;
}

#endif  /* DUK_USE_REGEXP_SUPPORT */

/*

could also map flag values as follows:

"gim\0gi\0gm\0g\0"

flags	desc		offset in above string
0	(none)		3
1	g		11	
2	i		5
3	gi		4
4	m		2
5	gm		7
6	im		1
7	gim		0

*/

