/*
 *  Error helpers
 */

#include "duk_internal.h"

/*
 *  Get prototype object for an integer error code.
 */

DUK_INTERNAL duk_hobject *duk_error_prototype_from_code(duk_hthread *thr, duk_errcode_t code) {
	switch (code) {
	case DUK_ERR_EVAL_ERROR:
		return thr->builtins[DUK_BIDX_EVAL_ERROR_PROTOTYPE];
	case DUK_ERR_RANGE_ERROR:
		return thr->builtins[DUK_BIDX_RANGE_ERROR_PROTOTYPE];
	case DUK_ERR_REFERENCE_ERROR:
		return thr->builtins[DUK_BIDX_REFERENCE_ERROR_PROTOTYPE];
	case DUK_ERR_SYNTAX_ERROR:
		return thr->builtins[DUK_BIDX_SYNTAX_ERROR_PROTOTYPE];
	case DUK_ERR_TYPE_ERROR:
		return thr->builtins[DUK_BIDX_TYPE_ERROR_PROTOTYPE];
	case DUK_ERR_URI_ERROR:
		return thr->builtins[DUK_BIDX_URI_ERROR_PROTOTYPE];

	/* XXX: more specific error classes? */
	case DUK_ERR_UNIMPLEMENTED_ERROR:
	case DUK_ERR_INTERNAL_ERROR:
	case DUK_ERR_ALLOC_ERROR:
	case DUK_ERR_ASSERTION_ERROR:
	case DUK_ERR_API_ERROR:
	case DUK_ERR_ERROR:
	default:
		return thr->builtins[DUK_BIDX_ERROR_PROTOTYPE];
	}
}

/*
 *  Exposed helper for setting up heap longjmp state.
 */

DUK_INTERNAL void duk_err_setup_heap_ljstate(duk_hthread *thr, duk_small_int_t lj_type) {
	duk_tval tv_tmp;

	thr->heap->lj.type = lj_type;

	DUK_ASSERT(thr->valstack_top > thr->valstack);
	DUK_TVAL_SET_TVAL(&tv_tmp, &thr->heap->lj.value1);
	DUK_TVAL_SET_TVAL(&thr->heap->lj.value1, thr->valstack_top - 1);
	DUK_TVAL_INCREF(thr, &thr->heap->lj.value1);
	DUK_TVAL_DECREF(thr, &tv_tmp);

	duk_pop((duk_context *) thr);
}
