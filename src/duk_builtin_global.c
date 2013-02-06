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

	DUK_ASSERT(duk_get_top(ctx) == 1);
	DUK_ASSERT(thr->callstack_top >= 1);

	h = duk_get_hstring(ctx, 0);
	if (!h) {
		return 1;  /* return arg as-is */
	}

	/* FIXME: uses internal API */
	duk_js_compile_program(thr, 1 /*is_eval*/);
	func = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) func));

	DUK_ASSERT(thr->callstack_top >= 1);
	act = thr->callstack + thr->callstack_top - 1;
	if (act->flags & DUK_ACT_FLAG_DIRECT_EVAL) {
		is_direct_eval = 1;
		if (DUK_HOBJECT_HAS_STRICT((duk_hobject *) func)) {
			DUK_DDDPRINT("direct eval call to a strict function");
		} else {
			DUK_DDDPRINT("direct eval call to a non-strict function");
		}

		/* FIXME: env handling is incorrect, fix */

		DUK_DDDPRINT("FIXME: no direct eval call handling now - behave like indirect eval");
		outer_lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
		outer_var_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	} else {
		DUK_DDDPRINT("indirect eval call");
		outer_lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
		outer_var_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	}
	act = NULL;

	duk_js_push_closure(thr,
	                   (duk_hcompiledfunction *) duk_get_hobject(ctx, -1),
	                   outer_var_env,
	                   outer_lex_env);

	duk_call(ctx, 0);

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

