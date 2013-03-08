/*
 *  Global built-ins
 */

#include "duk_internal.h"

int duk_builtin_global_object_eval(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_activation *act;
	duk_hcompiledfunction *func;
	duk_hobject *outer_lex_env;
	duk_hobject *outer_var_env;
	int this_to_global = 1;
	int comp_flags;

	DUK_ASSERT(duk_get_top(ctx) == 1);

	if (thr->callstack_top < 2) {
		/* callstack_top - 1 --> this function
		 * callstack_top - 2 --> caller
		 *
		 * If called directly from C, callstack_top might be 1.
		 * We don't support that now.
		 */
		return DUK_RET_TYPE_ERROR;
	}
	DUK_ASSERT(thr->callstack_top >= 2);  /* caller and this function */

	h = duk_get_hstring(ctx, 0);
	if (!h) {
		return 1;  /* return arg as-is */
	}

	/* FIXME: uses internal API */

	comp_flags = DUK_JS_COMPILE_FLAG_EVAL;
	act = thr->callstack + thr->callstack_top - 2;  /* caller */
	if (act->flags & DUK_ACT_FLAG_STRICT) {
		comp_flags |= DUK_JS_COMPILE_FLAG_STRICT;
	}

	duk_js_compile(thr, comp_flags);
	func = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) func));

	/* E5 Section 10.4.2 */
	DUK_ASSERT(thr->callstack_top >= 2);
	act = thr->callstack + thr->callstack_top - 1;  /* this function */
	if (act->flags & DUK_ACT_FLAG_DIRECT_EVAL) {	
		act = thr->callstack + thr->callstack_top - 2;  /* caller */
		if (act->lex_env == NULL) {
			DUK_DDDPRINT("delayed environment initialization");

			/* this may have side effects, so re-lookup act */
			duk_js_init_activation_environment_records_delayed(thr, act);
			act = thr->callstack + thr->callstack_top - 2;
		}
		DUK_ASSERT(act->lex_env != NULL);
		DUK_ASSERT(act->var_env != NULL);

		this_to_global = 0;

		if (DUK_HOBJECT_HAS_STRICT((duk_hobject *) func)) {
			duk_hobject *new_env;

			DUK_DDDPRINT("direct eval call to a strict function -> "
			             "var_env and lex_env to a fresh env, "
			             "this_binding to caller's this_binding");

			(void) duk_push_new_object_helper(ctx,
			                                  DUK_HOBJECT_FLAG_EXTENSIBLE |
			                                  DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_DECENV),
			                                  -1);  /* no prototype, updated below */
			new_env = duk_require_hobject(ctx, -1);
			DUK_ASSERT(new_env != NULL);
			DUK_DDDPRINT("new_env allocated: %!iO", new_env);

			act = thr->callstack + thr->callstack_top - 2;  /* caller */
			DUK_HOBJECT_SET_PROTOTYPE(thr, new_env, act->lex_env);  /* updates refcounts */
			act = NULL;  /* invalidated */

			outer_lex_env = new_env;
			outer_var_env = new_env;

			duk_insert(ctx, 0);  /* stash to bottom of value stack to keep new_env reachable */

			/* compiler's responsibility */
			DUK_ASSERT(DUK_HOBJECT_HAS_NEWENV((duk_hobject *) func));
		} else {
			DUK_DDDPRINT("direct eval call to a non-strict function -> "
			             "var_env and lex_env to caller's envs, "
			             "this_binding to caller's this_binding");

			outer_lex_env = act->lex_env;
			outer_var_env = act->var_env;

			/* compiler's responsibility */
			DUK_ASSERT(!DUK_HOBJECT_HAS_NEWENV((duk_hobject *) func));
		}
	} else {
		DUK_DDDPRINT("indirect eval call -> var_env and lex_env to "
		             "global object, this_binding to global object");

		this_to_global = 1;
		outer_lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
		outer_var_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	}
	act = NULL;

	duk_js_push_closure(thr, func, outer_var_env, outer_lex_env);

	if (this_to_global) {
		DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);
		duk_push_hobject(ctx, thr->builtins[DUK_BIDX_GLOBAL]);
	} else {
		duk_tval *tv;
		DUK_ASSERT(thr->callstack_top >= 2);
		act = thr->callstack + thr->callstack_top - 2;  /* caller */
		tv = thr->valstack + act->idx_bottom - 1;  /* this is just beneath bottom */
		DUK_ASSERT(tv >= thr->valstack);
		duk_push_tval(ctx, tv);
	}

	DUK_DDDPRINT("eval -> lex_env=%!iO, var_env=%!iO, this_binding=%!T",
	             outer_lex_env, outer_var_env, duk_get_tval(ctx, -1));

	duk_call_method(ctx, 0);

	return 1;
}

int duk_builtin_global_object_parse_int(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_global_object_parse_float(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_global_object_is_nan(duk_context *ctx) {
	double d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, isnan(d));
	return 1;
}

int duk_builtin_global_object_is_finite(duk_context *ctx) {
	double d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, isfinite(d));
	return 1;
}

int duk_builtin_global_object_decode_uri(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_global_object_decode_uri_component(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_global_object_encode_uri(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_global_object_encode_uri_component(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

#ifdef DUK_USE_SECTION_B

/* E5.1 Section B.2.2, step 7. */
#define  _MKBITS(a,b,c,d,e,f,g,h)  ((unsigned char) ( \
	((a) << 0) | ((b) << 1) | ((c) << 2) | ((d) << 3) | \
	((e) << 4) | ((f) << 5) | ((g) << 6) | ((h) << 7) \
	))
unsigned char duk_escape_as_is_table[16] = {
	_MKBITS(0, 0, 0, 0, 0, 0, 0, 0), _MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	_MKBITS(0, 0, 0, 0, 0, 0, 0, 0), _MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	_MKBITS(0, 0, 0, 0, 0, 0, 0, 0), _MKBITS(0, 0, 1, 1, 0, 1, 1, 1),  /* 0x20-0x2f */
	_MKBITS(1, 1, 1, 1, 1, 1, 1, 1), _MKBITS(1, 1, 0, 0, 0, 0, 0, 0),  /* 0x30-0x3f */
	_MKBITS(1, 1, 1, 1, 1, 1, 1, 1), _MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x40-0x4f */
	_MKBITS(1, 1, 1, 1, 1, 1, 1, 1), _MKBITS(1, 1, 1, 0, 0, 0, 0, 1),  /* 0x50-0x5f */
	_MKBITS(0, 1, 1, 1, 1, 1, 1, 1), _MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x60-0x6f */
	_MKBITS(1, 1, 1, 1, 1, 1, 1, 1), _MKBITS(1, 1, 1, 0, 0, 0, 0, 0)   /* 0x70-0x7f */
};

/* bit number in the byte is a bit counterintuitive, but minimizes ops */
#define  ESCAPE_AS_IS(cp)  (duk_escape_as_is_table[(cp) >> 3] & (1 << ((cp) & 0x07)))

/* FIXME: could this be implemented as a generic "transform" utility with
 * a changing callback?  What other functions could share the same helper?
 */
int duk_builtin_global_object_escape(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h_str;
	duk_hbuffer_growable *h_buf;
	duk_u8 *p_start, *p_end, *p;
	duk_u32 cp;

	h_str = duk_to_hstring(ctx, 0);
	DUK_ASSERT(h_str != NULL);

	(void) duk_push_new_growable_buffer(ctx, 0);
	h_buf = (duk_hbuffer_growable *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(h_buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(h_buf));

	p_start = DUK_HSTRING_GET_DATA(h_str);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_str);
	p = p_start;

	/* Since escape() is a legacy function, no fast path here to save space. */
	while (p < p_end) {
		duk_u8 buf[6];
		size_t len;

		cp = duk_unicode_xutf8_get_u32(thr, &p, p_start, p_end);
		if ((cp < 128) && ESCAPE_AS_IS(cp)) {
			buf[0] = (duk_u8) cp;
			len = 1;
		} else if (cp < 256) {
			buf[0] = (duk_u8) '%';
			buf[1] = (duk_u8) duk_uc_nybbles[cp >> 4];
			buf[2] = (duk_u8) duk_uc_nybbles[cp & 0x0f];
			len = 3;
		} else {
			/* FIXME: non-BMP chars will now be clipped */
			buf[0] = (duk_u8) '%';
			buf[1] = (duk_u8) 'u';
			buf[2] = (duk_u8) duk_uc_nybbles[cp >> 12];
			buf[3] = (duk_u8) duk_uc_nybbles[(cp >> 8) & 0x0f];
			buf[4] = (duk_u8) duk_uc_nybbles[(cp >> 4) & 0x0f];
			buf[5] = (duk_u8) duk_uc_nybbles[cp & 0x0f];
			len = 6;
		}
		duk_hbuffer_append_bytes(thr, h_buf, buf, len);
	}

	duk_to_string(ctx, -1);
	return 1;
}

int duk_builtin_global_object_unescape(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}
#endif

#ifdef DUK_USE_BROWSER_LIKE
static int print_alert_helper(duk_context *ctx, FILE *f_out) {
	int nargs;
	int i;
	const char *str;
	size_t len;
	char nl = '\n';

	/* If argument count is 1 and first argument is a buffer, write the buffer
	 * as raw data into the file without a newline; this allows exact control
	 * over stdout/stderr without an additional entrypoint (useful for now).
	 */

	nargs = duk_get_top(ctx);
	if (nargs == 1 && duk_is_buffer(ctx, 0)) {
		const char *buf = NULL;
		size_t sz = 0;
		buf = duk_get_buffer(ctx, 0, &sz);
		if (buf && sz > 0) {
			fwrite(buf, 1, sz, f_out);
		}
		goto flush;
	}

	/* FIXME: best semantics link?  Now apply ToString to args, join with ' ' */
	/* FIXME: ToString() coerce inplace instead? */

	if (nargs > 0) {
		for (i = 0; i < nargs; i++) {
			if (i != 0) {
				duk_push_hstring_stridx(ctx, DUK_STRIDX_SPACE);
			}
			duk_dup(ctx, i);
			duk_to_string(ctx, -1);
		}

		duk_concat(ctx, 2*nargs - 1);

		str = duk_get_lstring(ctx, -1, &len);
		if (str) {
			fwrite(str, 1, len, f_out);
		}
	}

	fwrite(&nl, 1, 1, f_out);

 flush:
	fflush(f_out);
	return 0;

}
int duk_builtin_global_object_print(duk_context *ctx) {
	return print_alert_helper(ctx, stdout);
}

int duk_builtin_global_object_alert(duk_context *ctx) {
	return print_alert_helper(ctx, stderr);
}
#endif  /* DUK_USE_BROWSER_LIKE */


