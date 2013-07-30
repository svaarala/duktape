/*
 *  Compilation and evaluation
 */

#include "duk_internal.h"

/* FIXME: placeholder */

/* Eval is just a wrapper now. */
void duk_eval(duk_context *ctx) {
	int comp_flags;

	/* [ ... source ] */

	comp_flags = DUK_COMPILE_EVAL;
	if (duk_is_strict_call(ctx)) {
		comp_flags |= DUK_COMPILE_STRICT;
	}
	duk_compile(ctx, comp_flags);

	/* [ ... closure ] */

	duk_call(ctx, 0);

	/* [ ... result ] */
}

void duk_compile(duk_context *ctx, int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int comp_flags;
	duk_hcompiledfunction *h_templ;

	/* [ ... source ] */

	/* FIXME: flags */
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
}

