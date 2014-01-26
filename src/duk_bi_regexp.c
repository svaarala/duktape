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
duk_ret_t duk_bi_regexp_constructor(duk_context *ctx) {
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

duk_ret_t duk_bi_regexp_prototype_exec(duk_context *ctx) {
	get_this_regexp(ctx);

	/* [ regexp input ] */

	duk_regexp_match((duk_hthread *) ctx);

	/* [ result ] */

	return 1;
}

duk_ret_t duk_bi_regexp_prototype_test(duk_context *ctx) {
	get_this_regexp(ctx);

	/* [ regexp input ] */

	/* result object is created and discarded; wasteful but saves code space */
	duk_regexp_match((duk_hthread *) ctx);

	/* [ result ] */

	duk_push_boolean(ctx, (duk_is_null(ctx, -1) ? 0 : 1));

	return 1;
}

duk_ret_t duk_bi_regexp_prototype_to_string(duk_context *ctx) {
	duk_hstring *h_bc;
	duk_small_int_t re_flags;

#if 0
	/* A little tricky string approach to provide the flags string.
	 * This depends on the specific flag values in duk_regexp.h,
	 * which needs to be asserted for.  In practice this doesn't
	 * produce more compact code than the easier approach in use.
	 */

	const char *flag_strings = "gim\0gi\0gm\0g\0";
	duk_uint8_t flag_offsets[8] = {
		(duk_uint8_t) 3,   /* flags: ""    */
		(duk_uint8_t) 10,  /* flags: "g"   */
		(duk_uint8_t) 5,   /* flags: "i"   */
		(duk_uint8_t) 4,   /* flags: "gi"  */
		(duk_uint8_t) 2,   /* flags: "m"   */
		(duk_uint8_t) 7,   /* flags: "gm"  */
		(duk_uint8_t) 1,   /* flags: "im"  */
		(duk_uint8_t) 0,   /* flags: "gim" */
	};
	DUK_ASSERT(DUK_RE_FLAG_GLOBAL == 1);
	DUK_ASSERT(DUK_RE_FLAG_IGNORE_CASE == 2);
	DUK_ASSERT(DUK_RE_FLAG_MULTILINE == 4);
#endif

	get_this_regexp(ctx);

	/* [ regexp ] */

	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_SOURCE);
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_INT_BYTECODE);
	h_bc = duk_get_hstring(ctx, -1);
	DUK_ASSERT(h_bc != NULL);
	DUK_ASSERT(DUK_HSTRING_GET_BYTELEN(h_bc) >= 1);
	DUK_ASSERT(DUK_HSTRING_GET_CHARLEN(h_bc) >= 1);
	DUK_ASSERT(DUK_HSTRING_GET_DATA(h_bc)[0] < 0x80);
	re_flags = (duk_small_int_t) DUK_HSTRING_GET_DATA(h_bc)[0];

	/* [ regexp source bytecode ] */

#if 1
	/* This is a cleaner approach and also produces smaller code than
	 * the other alternative.
	 */
	duk_push_sprintf(ctx, "/%s/%s%s%s",
	                 duk_get_string(ctx, -2),
	                 (re_flags & DUK_RE_FLAG_GLOBAL) ? "g" : "",
	                 (re_flags & DUK_RE_FLAG_IGNORE_CASE) ? "i" : "",
	                 (re_flags & DUK_RE_FLAG_MULTILINE) ? "m" : "");
#else
	/* This should not be necessary because no-one should tamper with the
	 * regexp bytecode, but is prudent to avoid potential segfaults if that
	 * were to happen for some reason.
	 */
	re_flags &= 0x07;
	DUK_ASSERT(re_flags >= 0 && re_flags <= 7);  /* three flags */
	duk_push_sprintf(ctx, "/%s/%s",
	                 duk_get_string(ctx, -2),
	                 flag_strings + flag_offsets[re_flags]);
#endif

	return 1;
}

#else  /* DUK_USE_REGEXP_SUPPORT */

duk_ret_t duk_bi_regexp_constructor(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}

duk_ret_t duk_bi_regexp_prototype_exec(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}

duk_ret_t duk_bi_regexp_prototype_test(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}

duk_ret_t duk_bi_regexp_prototype_to_string(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}

#endif  /* DUK_USE_REGEXP_SUPPORT */
