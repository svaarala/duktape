/*
 *  Compilation and evaluation
 */

#include "duk_internal.h"

/* Eval is just a wrapper now. */
int duk_eval_raw(duk_context *ctx, int flags) {
	int comp_flags;
	int rc;

	/* [ ... source filename ] */

	comp_flags = flags;
	comp_flags |= DUK_COMPILE_EVAL;
	if (duk_is_strict_call(ctx)) {
		comp_flags |= DUK_COMPILE_STRICT;
	}
	rc = duk_compile_raw(ctx, comp_flags);  /* may be safe, or non-safe depending on flags */

	/* [ ... closure/error ] */

	if (rc != DUK_EXEC_SUCCESS) {
		return DUK_EXEC_ERROR;
	}

	if (flags & DUK_COMPILE_SAFE) {
		rc = duk_pcall(ctx, 0, DUK_INVALID_INDEX);  /* FIXME: user control? */
	} else {
		duk_call(ctx, 0);
		rc = DUK_EXEC_SUCCESS;
	}

	/* [ ... result/error ] */

	return rc;
}

/* Helper which can be called both directly and with duk_safe_call(). */
static int duk__do_compile(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int flags;
	int comp_flags;
	duk_hcompiledfunction *h_templ;

	/* [ ... source filename flags ] */

	flags = duk_get_number(ctx, -1);
	duk_pop(ctx);

	/* [ ... source filename ] */

	/* FIXME: unnecessary translation of flags */
	comp_flags = 0;
	if (flags & DUK_COMPILE_EVAL) {
		comp_flags = DUK_JS_COMPILE_FLAG_EVAL;
	}
	if (flags & DUK_COMPILE_FUNCTION) {
		duk_error(ctx, DUK_ERR_UNIMPLEMENTED_ERROR, "unimplemented");
	}
	if (flags & DUK_COMPILE_STRICT) {
		comp_flags = DUK_JS_COMPILE_FLAG_STRICT;
	}

	duk_js_compile(thr, comp_flags);
	h_templ = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);

	/* [ ... func_template ] */

        duk_js_push_closure(thr,
	                   h_templ,
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV],
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV]);

	/* [ ... func_template closure ] */

	duk_remove(ctx, -2);  /* -> [ ... closure ] */

	/* [ ... closure ] */

	return 1;
}

int duk_compile_raw(duk_context *ctx, int flags) {
	duk_push_int(ctx, flags);

	if (flags & DUK_COMPILE_SAFE) {
		int rc = duk_safe_call(ctx, duk__do_compile, 3 /*nargs*/, 1 /*nrets*/, DUK_INVALID_INDEX);
		return rc;
	}

	(void) duk__do_compile(ctx);
	return DUK_EXEC_SUCCESS;
}

