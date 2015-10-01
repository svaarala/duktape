/*
 *  Error helpers
 */

#include "duk_internal.h"

/*
 *  Helper to walk the thread chain and see if there is an active error
 *  catcher.  Protected calls or finally blocks aren't considered catching.
 */

#if defined(DUK_USE_DEBUGGER_SUPPORT) && \
    (defined(DUK_USE_DEBUGGER_THROW_NOTIFY) || defined(DUK_USE_DEBUGGER_PAUSE_UNCAUGHT))
DUK_LOCAL duk_bool_t duk__have_active_catcher(duk_hthread *thr) {
	/*
	 * XXX: As noted above, a protected API call won't be counted as a
	 * catcher. This is usually convenient, e.g. in the case of a top-
	 * level duk_pcall(), but may not always be desirable. Perhaps add an
	 * argument to treat them as catchers?
	 */

	duk_size_t i;

	DUK_ASSERT(thr != NULL);

	while (thr != NULL) {
		for (i = 0; i < thr->catchstack_top; i++) {
			duk_catcher *cat = thr->catchstack + i;
			if (DUK_CAT_HAS_CATCH_ENABLED(cat)) {
				return 1;  /* all we need to know */
			}
		}
		thr = thr->resumer;
	}
	return 0;
}
#endif  /* DUK_USE_DEBUGGER_SUPPORT && (DUK_USE_DEBUGGER_THROW_NOTIFY || DUK_USE_DEBUGGER_PAUSE_UNCAUGHT) */

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
#if defined(DUK_USE_DEBUGGER_SUPPORT)
	/* If something is thrown with the debugger attached and nobody will
	 * catch it, execution is paused before the longjmp, turning over
	 * control to the debug client.  This allows local state to be examined
	 * before the stack is unwound.  Errors are not intercepted when debug
	 * message loop is active (e.g. for Eval).
	 */

	/* XXX: Allow customizing the pause and notify behavior at runtime
	 * using debugger runtime flags.  For now the behavior is fixed using
	 * config options.
	 */
#if defined(DUK_USE_DEBUGGER_THROW_NOTIFY) || defined(DUK_USE_DEBUGGER_PAUSE_UNCAUGHT)
	if (DUK_HEAP_IS_DEBUGGER_ATTACHED(thr->heap) &&
	    !thr->heap->dbg_processing &&
	    lj_type == DUK_LJ_TYPE_THROW) {
		duk_context *ctx = (duk_context *) thr;
		duk_bool_t fatal;
		duk_hobject *h_obj;

		/* Don't intercept a DoubleError, we may have caused the initial double
		 * fault and attempting to intercept it will cause us to be called
		 * recursively and exhaust the C stack.
		 */
		h_obj = duk_get_hobject(ctx, -1);
		if (h_obj == thr->builtins[DUK_BIDX_DOUBLE_ERROR]) {
			DUK_D(DUK_DPRINT("built-in DoubleError instance thrown, not intercepting"));
			goto skip_throw_intercept;
		}

		DUK_D(DUK_DPRINT("throw with debugger attached, report to client"));

		fatal = !duk__have_active_catcher(thr);

#if defined(DUK_USE_DEBUGGER_THROW_NOTIFY)
		/* Report it to the debug client */
		duk_debug_send_throw(thr, fatal);
#endif

#if defined(DUK_USE_DEBUGGER_PAUSE_UNCAUGHT)
		if (fatal) {
			DUK_D(DUK_DPRINT("throw will be fatal, halt before longjmp"));
			duk_debug_halt_execution(thr, 1 /*use_prev_pc*/);
		}
#endif
	}

 skip_throw_intercept:
#endif  /* DUK_USE_DEBUGGER_THROW_NOTIFY || DUK_USE_DEBUGGER_PAUSE_UNCAUGHT */
#endif  /* DUK_USE_DEBUGGER_SUPPORT */

	thr->heap->lj.type = lj_type;

	DUK_ASSERT(thr->valstack_top > thr->valstack);
	DUK_TVAL_SET_TVAL_UPDREF(thr, &thr->heap->lj.value1, thr->valstack_top - 1);  /* side effects */

	duk_pop((duk_context *) thr);
}
