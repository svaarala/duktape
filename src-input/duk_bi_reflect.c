/*
 *  'Reflect' built-in (ES7 Section 26.1)
 *  http://www.ecma-international.org/ecma-262/7.0/#sec-reflect-object
 *
 *  Many Reflect built-in functions are provided by shared helpers in
 *  duk_bi_object.c or duk_bi_function.c.
 */

#include "duk_internal.h"

#if defined(DUK_USE_REFLECT_BUILTIN)
DUK_INTERNAL duk_ret_t duk_bi_reflect_object_delete_property(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_bool_t ret;

	DUK_ASSERT_TOP(ctx, 2);
	(void) duk_require_hobject(ctx, 0);
	(void) duk_to_string(ctx, 1);

	/* [ target key ] */

	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	tv_obj = DUK_GET_TVAL_POSIDX(ctx, 0);
	tv_key = DUK_GET_TVAL_POSIDX(ctx, 1);
	ret = duk_hobject_delprop(thr, tv_obj, tv_key, 0 /*throw_flag*/);
	return duk_push_boolean(ctx, ret);
}

DUK_INTERNAL duk_ret_t duk_bi_reflect_object_get(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_idx_t nargs;

	nargs = duk_get_top(ctx);
	if (nargs < 2) {
		DUK_DCERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
	}
	(void) duk_require_hobject(ctx, 0);
	(void) duk_to_string(ctx, 1);
	if (nargs >= 3 && !duk_strict_equals(ctx, 0, 2)) {
		/* XXX: [[Get]] receiver currently unsupported */
		DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
	}

	/* [ target key receiver? ...? ] */

	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	tv_obj = DUK_GET_TVAL_POSIDX(ctx, 0);
	tv_key = DUK_GET_TVAL_POSIDX(ctx, 1);
	(void) duk_hobject_getprop(thr, tv_obj, tv_key);  /* This could also be a duk_get_prop(). */
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_reflect_object_has(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_bool_t ret;

	DUK_ASSERT_TOP(ctx, 2);
	(void) duk_require_hobject(ctx, 0);
	(void) duk_to_string(ctx, 1);

	/* [ target key ] */

	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	tv_obj = DUK_GET_TVAL_POSIDX(ctx, 0);
	tv_key = DUK_GET_TVAL_POSIDX(ctx, 1);
	ret = duk_hobject_hasprop(thr, tv_obj, tv_key);
	return duk_push_boolean(ctx, ret);
}

DUK_INTERNAL duk_ret_t duk_bi_reflect_object_set(duk_context *ctx) {
	duk_hthread *thr;
	duk_tval *tv_obj;
	duk_tval *tv_key;
	duk_tval *tv_val;
	duk_idx_t nargs;
	duk_bool_t ret;

	nargs = duk_get_top(ctx);
	if (nargs < 3) {
		DUK_DCERROR_TYPE_INVALID_ARGS((duk_hthread *) ctx);
	}
	(void) duk_require_hobject(ctx, 0);
	(void) duk_to_string(ctx, 1);
	if (nargs >= 4 && !duk_strict_equals(ctx, 0, 3)) {
		/* XXX: [[Set]] receiver currently unsupported */
		DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
	}

	/* [ target key value receiver? ...? ] */

	thr = (duk_hthread *) ctx;
	DUK_ASSERT(thr != NULL);
	tv_obj = DUK_GET_TVAL_POSIDX(ctx, 0);
	tv_key = DUK_GET_TVAL_POSIDX(ctx, 1);
	tv_val = DUK_GET_TVAL_POSIDX(ctx, 2);
	ret = duk_hobject_putprop(thr, tv_obj, tv_key, tv_val, 0 /*throw_flag*/);
	return duk_push_boolean(ctx, ret);
}
#endif  /* DUK_USE_REFLECT_BUILTIN */
