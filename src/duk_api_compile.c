/*
 *  Compilation and evaluation
 */

#include "duk_internal.h"

void duk_eval(duk_context *ctx, int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int comp_flags;
	duk_hcompiledfunction *h_templ;

	/* [ ... source ] */

	/* FIXME: other flags; strict? */
	comp_flags = DUK_JS_COMPILE_FLAG_EVAL;
	duk_js_compile(thr, comp_flags);
	h_templ = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);

	/* [ ... func_template ] */

        duk_js_push_closure(thr,
	                   h_templ,
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV],
	                   thr->builtins[DUK_BIDX_GLOBAL_ENV]);

	/* [ ... func_template closure ] */

	duk_remove(ctx, -2);  /* -> [ ... closure ] */
        duk_call(ctx, 0);     /* -> [ ... result ] */

	/* [ ... result ] */
}

void duk_compile(duk_context *ctx, int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	int comp_flags;
	duk_hcompiledfunction *h_templ;

	/* [ ... source ] */

	/* FIXME: flags */
	comp_flags = 0;
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

