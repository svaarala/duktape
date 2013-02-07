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
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_global_object_is_finite(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
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

#if 1  /* FIXME: Section B */
int duk_builtin_global_object_escape(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_global_object_unescape(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}
#endif

#if 1  /* FIXME: browser-like */
int duk_builtin_global_object_print(duk_context *ctx) {
	int nargs;
	int i;
	const char *str;
	size_t len;
	char nl = '\n';

	DUK_DDDPRINT("print arg 1: %!T", duk_get_tval(ctx, -1));

	/* FIXME: best semantics link?  Now apply ToString to args, join with ' ' */
	/* FIXME: ToString() coerce inplace instead? */

	nargs = duk_get_top(ctx);
	if (nargs > 0) {
		for (i = 0; i < nargs; i++) {
			if (i != 0) {
				duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_SPACE);
			}
			duk_dup(ctx, i);
			duk_to_string(ctx, -1);
		}

		duk_concat(ctx, 2*nargs - 1);

		str = duk_get_lstring(ctx, -1, &len);
		if (str) {
			fwrite(str, 1, len, stdout);
		}
	}

	fwrite(&nl, 1, 1, stdout);
	return 0;
}

int duk_builtin_global_object_alert(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}
#endif

