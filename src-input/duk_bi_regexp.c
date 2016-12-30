/*
 *  RegExp built-ins
 */

#include "duk_internal.h"

#if defined(DUK_USE_REGEXP_SUPPORT)

DUK_LOCAL void duk__get_this_regexp(duk_context *ctx) {
	duk_hobject *h;

	duk_push_this(ctx);
	h = duk_require_hobject_with_class(ctx, -1, DUK_HOBJECT_CLASS_REGEXP);
	DUK_ASSERT(h != NULL);
	DUK_UNREF(h);
	duk_insert(ctx, 0);  /* prepend regexp to valstack 0 index */
}

/* XXX: much to improve (code size) */
DUK_INTERNAL duk_ret_t duk_bi_regexp_constructor(duk_context *ctx) {
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
		/* XXX: ES2015 has a NewTarget SameValue() check which is not
		 * yet implemented.
		 */
		duk_dup_0(ctx);
		return 1;
	}

	/* Else functionality is identical for function call and constructor
	 * call.
	 */

	if (h_pattern != NULL &&
	    DUK_HOBJECT_GET_CLASS_NUMBER(h_pattern) == DUK_HOBJECT_CLASS_REGEXP) {
		duk_get_prop_stridx_short(ctx, 0, DUK_STRIDX_SOURCE);
		if (duk_is_undefined(ctx, 1)) {
			/* In ES5 one would need to read the flags individually;
			 * in ES2015 just read .flags.
			 */
			duk_get_prop_stridx(ctx, 0, DUK_STRIDX_FLAGS);
		} else {
			/* In ES2015 allowed; overrides argument RegExp flags. */
			duk_dup_1(ctx);
		}
	} else {
		if (duk_is_undefined(ctx, 0)) {
			duk_push_hstring_empty(ctx);
		} else {
			duk_dup_0(ctx);
			duk_to_string(ctx, -1);  /* Rejects Symbols. */
		}
		if (duk_is_undefined(ctx, 1)) {
			duk_push_hstring_empty(ctx);
		} else {
			duk_dup_1(ctx);
			duk_to_string(ctx, -1);  /* Rejects Symbols. */
		}

		/* [ ... pattern flags ] */
	}

	DUK_DDD(DUK_DDDPRINT("RegExp constructor/function call, pattern=%!T, flags=%!T",
	                     (duk_tval *) duk_get_tval(ctx, -2), (duk_tval *) duk_get_tval(ctx, -1)));

	/* [ ... pattern flags ] (both uncoerced) */

	duk_to_string(ctx, -2);
	duk_to_string(ctx, -1);
	duk_regexp_compile(thr);

	/* [ ... bytecode escaped_source ] */

	duk_regexp_create_instance(thr);

	/* [ ... RegExp ] */

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_regexp_prototype_exec(duk_context *ctx) {
	duk__get_this_regexp(ctx);

	/* [ regexp input ] */

	duk_regexp_match((duk_hthread *) ctx);

	/* [ result ] */

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_regexp_prototype_test(duk_context *ctx) {
	duk__get_this_regexp(ctx);

	/* [ regexp input ] */

	/* result object is created and discarded; wasteful but saves code space */
	duk_regexp_match((duk_hthread *) ctx);

	/* [ result ] */

	duk_push_boolean(ctx, (duk_is_null(ctx, -1) ? 0 : 1));

	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_regexp_prototype_tostring(duk_context *ctx) {
	/* This must be generic in ES2015 and later. */
	DUK_ASSERT_TOP(ctx, 0);
	duk_push_this(ctx);
	duk_push_string(ctx, "/");
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_SOURCE);
	duk_dup_m2(ctx);  /* another "/" */
	duk_get_prop_stridx(ctx, 0, DUK_STRIDX_FLAGS);
	duk_concat(ctx, 4);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_regexp_prototype_flags(duk_context *ctx) {
	/* .flags is ES2015 but present even when ES2015 bindings are
	 * disabled because the constructor relies on it.
	 */
	duk_uint8_t buf[8];  /* enough for all flags + NUL */
	duk_uint8_t *p = buf;

	/* .flags is generic and works on any object. */
	duk_push_this(ctx);
	(void) duk_require_hobject(ctx, -1);
	if (duk_get_prop_stridx_boolean(ctx, 0, DUK_STRIDX_GLOBAL, NULL)) {
		*p++ = DUK_ASC_LC_G;
	}
	if (duk_get_prop_stridx_boolean(ctx, 0, DUK_STRIDX_IGNORE_CASE, NULL)) {
		*p++ = DUK_ASC_LC_I;
	}
	if (duk_get_prop_stridx_boolean(ctx, 0, DUK_STRIDX_MULTILINE, NULL)) {
		*p++ = DUK_ASC_LC_M;
	}
	/* .unicode: to be added */
	/* .sticky: to be added */
	*p++ = DUK_ASC_NUL;
	DUK_ASSERT((duk_size_t) (p - buf) <= sizeof(buf));

	duk_push_string(ctx, (const char *) buf);
	return 1;
}

/* Shared helper for providing .source, .global, .multiline, etc getters. */
DUK_INTERNAL duk_ret_t duk_bi_regexp_prototype_shared_getter(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_bc;
	duk_small_int_t re_flags;
	duk_hobject *h;
	duk_int_t magic;

	DUK_ASSERT_TOP(ctx, 0);

	duk_push_this(ctx);
	h = duk_require_hobject(ctx, -1);
	magic = duk_get_current_magic(ctx);

	if (DUK_HOBJECT_GET_CLASS_NUMBER(h) == DUK_HOBJECT_CLASS_REGEXP) {
		duk_get_prop_stridx_short(ctx, 0, DUK_STRIDX_INT_SOURCE);
		duk_get_prop_stridx_short(ctx, 0, DUK_STRIDX_INT_BYTECODE);
		h_bc = duk_require_hstring(ctx, -1);
		re_flags = (duk_small_int_t) DUK_HSTRING_GET_DATA(h_bc)[0];  /* Safe even if h_bc length is 0 (= NUL) */
		duk_pop(ctx);
	} else if (h == thr->builtins[DUK_BIDX_REGEXP_PROTOTYPE]) {
		/* In ES2015 and ES2016 a TypeError would be thrown here.
		 * However, this had real world issues so ES2017 draft
		 * allows RegExp.prototype specifically, returning '(?:)'
		 * for .source and undefined for all flags.
		 */
		if (magic != 16 /* .source */) {
			return 0;
		}
		duk_push_string(ctx, "(?:)");  /* .source handled by switch-case */
		re_flags = 0;
	} else {
		DUK_DCERROR_TYPE_INVALID_ARGS(thr);
	}

	/* [ regexp source ] */

	switch (magic) {
	case 0: {  /* global */
		duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_GLOBAL));
		break;
	}
	case 1: {  /* ignoreCase */
		duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_IGNORE_CASE));
		break;
	}
	case 2: {  /* multiline */
		duk_push_boolean(ctx, (re_flags & DUK_RE_FLAG_MULTILINE));
		break;
	}
#if 0
	/* Don't provide until implemented to avoid interfering with feature
	 * detection in user code.
	 */
	case 3:    /* sticky */
	case 4: {  /* unicode */
		duk_push_false(ctx);
		break;
	}
#endif
	default: {  /* source */
		/* leave 'source' on top */
		break;
	}
	}

	return 1;
}

#endif  /* DUK_USE_REGEXP_SUPPORT */
