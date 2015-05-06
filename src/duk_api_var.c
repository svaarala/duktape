/*
 *  Variable access
 */

#include "duk_internal.h"

DUK_EXTERNAL void duk_get_var(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;
	duk_hstring *h_varname;
	duk_small_int_t throw_flag = 1;  /* always throw ReferenceError for unresolvable */

	DUK_ASSERT_CTX_VALID(ctx);

	h_varname = duk_require_hstring(ctx, -1);  /* XXX: tostring? */
	DUK_ASSERT(h_varname != NULL);

	act = duk_hthread_get_current_activation(thr);
	if (act) {
		(void) duk_js_getvar_activation(thr, act, h_varname, throw_flag);  /* -> [ ... varname val this ] */
	} else {
		/* Outside any activation -> look up from global. */
		DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL_ENV] != NULL);
		(void) duk_js_getvar_envrec(thr, thr->builtins[DUK_BIDX_GLOBAL_ENV], h_varname, throw_flag);
	}

	/* [ ... varname val this ]  (because throw_flag == 1, always resolved) */

	duk_pop(ctx);
	duk_remove(ctx, -2);

	/* [ ... val ] */

	/* Return value would be pointless: because throw_flag==1, we always
	 * throw if the identifier doesn't resolve.
	 */
	return;
}

DUK_EXTERNAL void duk_put_var(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_activation *act;
	duk_hstring *h_varname;
	duk_tval *tv_val;
	duk_small_int_t throw_flag;

	DUK_ASSERT_CTX_VALID(ctx);

	h_varname = duk_require_hstring(ctx, -2);  /* XXX: tostring? */
	DUK_ASSERT(h_varname != NULL);

	tv_val = duk_require_tval(ctx, -1);

	throw_flag = duk_is_strict_call(ctx);

	act = duk_hthread_get_current_activation(thr);
	if (act) {
		duk_js_putvar_activation(thr, act, h_varname, tv_val, throw_flag);  /* -> [ ... varname val this ] */
	} else {
		/* Outside any activation -> put to global. */
		DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL_ENV] != NULL);
		duk_js_putvar_envrec(thr, thr->builtins[DUK_BIDX_GLOBAL_ENV], h_varname, tv_val, throw_flag);
	}

	/* [ ... varname val ] */

	duk_pop_2(ctx);

	/* [ ... ] */

	return;
}

DUK_EXTERNAL duk_bool_t duk_del_var(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, DUK_STR_UNIMPLEMENTED);
	return 0;
}

DUK_EXTERNAL duk_bool_t duk_has_var(duk_context *ctx) {
	DUK_ASSERT_CTX_VALID(ctx);

	DUK_ERROR((duk_hthread *) ctx, DUK_ERR_UNIMPLEMENTED_ERROR, DUK_STR_UNIMPLEMENTED);
	return 0;
}
