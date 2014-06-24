/*
 *  Compilation and evaluation
 */

#include "duk_internal.h"

typedef struct duk__compile_raw_args duk__compile_raw_args;
struct duk__compile_raw_args {
	duk_size_t user_length;  /* should be first on 64-bit platforms */
	const char *user_buffer;
	duk_int_t flags;
};

/* Eval is just a wrapper now. */
duk_int_t duk_eval_raw(duk_context *ctx, duk_int_t flags) {
	duk_int_t comp_flags;
	duk_int_t rc;

	/* [ ... source filename ] */

	comp_flags = flags;
	comp_flags |= DUK_COMPILE_EVAL;
	if (duk_is_strict_call(ctx)) {
		comp_flags |= DUK_COMPILE_STRICT;
	}
	rc = duk_compile_raw(ctx, comp_flags);  /* may be safe, or non-safe depending on flags */

	/* [ ... closure/error ] */

	if (rc != DUK_EXEC_SUCCESS) {
		rc = DUK_EXEC_ERROR;
		goto got_rc;
	}

	if (flags & DUK_COMPILE_SAFE) {
		rc = duk_pcall(ctx, 0);
	} else {
		duk_call(ctx, 0);
		rc = DUK_EXEC_SUCCESS;
	}

	/* [ ... result/error ] */

 got_rc:
	if (flags & DUK_COMPILE_NORESULT) {
		duk_pop(ctx);
	}

	return rc;
}

/* Helper which can be called both directly and with duk_safe_call(). */
static duk_ret_t duk__do_compile(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk__compile_raw_args *comp_args;
	duk_int_t flags;
	duk_small_int_t comp_flags;
	duk_hcompiledfunction *h_templ;

	/* [ ... source filename flags ] */

	comp_args = (duk__compile_raw_args *) duk_require_pointer(ctx, -1);
	flags = comp_args->flags;
	duk_pop(ctx);

	/* [ ... source filename ] */

	/* XXX: barebones initial implementation for compiling a function:
	 * simply evaluate a parenthesis wrapped function expression.
	 */
	if (flags & DUK_COMPILE_FUNCTION) {
		duk_push_string(ctx, "(");
		duk_dup(ctx, -3);
		duk_push_string(ctx, ")");
		duk_concat(ctx, 3);
		duk_replace(ctx, -3);
	}

	/* XXX: unnecessary translation of flags */
	comp_flags = 0;
	if (flags & DUK_COMPILE_EVAL) {
		comp_flags = DUK_JS_COMPILE_FLAG_EVAL;
	}
	if (flags & DUK_COMPILE_FUNCTION) {
		comp_flags = DUK_JS_COMPILE_FLAG_EVAL;
	}
	if (flags & DUK_COMPILE_STRICT) {
		comp_flags = DUK_JS_COMPILE_FLAG_STRICT;
	}

	duk_js_compile(thr, comp_flags);

	/* [ ... func_template ] */

	h_templ = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
        duk_js_push_closure(thr,
	                   h_templ,
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV],
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV]);

	/* [ ... func_template closure ] */

	duk_remove(ctx, -2);  /* -> [ ... closure ] */

	/* [ ... closure ] */

	if (flags & DUK_COMPILE_FUNCTION) {
		/* Evaluate the function expression to get the function. */
		duk_call(ctx, 0);
	}

	return 1;
}

duk_int_t duk_compile_raw(duk_context *ctx, duk_int_t flags) {
	duk__compile_raw_args comp_args_alloc;
	duk__compile_raw_args *comp_args = &comp_args_alloc;

#if 0
	comp_args->user_buffer = user_buffer;
	comp_args->user_length = user_length;
#endif
	comp_args->flags = flags;
	duk_push_pointer(ctx, (void *) comp_args);

	if (flags & DUK_COMPILE_SAFE) {
		duk_int_t rc = duk_safe_call(ctx, duk__do_compile, 3 /*nargs*/, 1 /*nrets*/);
		return rc;
	}

	(void) duk__do_compile(ctx);
	return DUK_EXEC_SUCCESS;
}
