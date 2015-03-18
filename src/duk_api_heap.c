/*
 *  Heap creation and destruction
 */

#include "duk_internal.h"

DUK_EXTERNAL
duk_context *duk_create_heap(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             void *heap_udata,
                             duk_fatal_function fatal_handler) {
	duk_heap *heap = NULL;
	duk_context *ctx;

	/* Assume that either all memory funcs are NULL or non-NULL, mixed
	 * cases will now be unsafe.
	 */

	/* XXX: just assert non-NULL values here and make caller arguments
	 * do the defaulting to the default implementations (smaller code)?
	 */

	if (!alloc_func) {
		DUK_ASSERT(realloc_func == NULL);
		DUK_ASSERT(free_func == NULL);
		alloc_func = duk_default_alloc_function;
		realloc_func = duk_default_realloc_function;
		free_func = duk_default_free_function;
	} else {
		DUK_ASSERT(realloc_func != NULL);
		DUK_ASSERT(free_func != NULL);
	}

	if (!fatal_handler) {
		fatal_handler = duk_default_fatal_handler;
	}

	DUK_ASSERT(alloc_func != NULL);
	DUK_ASSERT(realloc_func != NULL);
	DUK_ASSERT(free_func != NULL);
	DUK_ASSERT(fatal_handler != NULL);

	heap = duk_heap_alloc(alloc_func, realloc_func, free_func, heap_udata, fatal_handler);
	if (!heap) {
		return NULL;
	}
	ctx = (duk_context *) heap->heap_thread;
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(((duk_hthread *) ctx)->heap != NULL);
	return ctx;
}

DUK_EXTERNAL void duk_destroy_heap(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;

	if (!ctx) {
		return;
	}
	heap = thr->heap;
	DUK_ASSERT(heap != NULL);

	duk_heap_free(heap);
}

/* XXX: better place for this */
DUK_EXTERNAL void duk_set_global_object(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h_glob;
	duk_hobject *h_prev_glob;
	duk_hobject *h_env;
	duk_hobject *h_prev_env;

	DUK_D(DUK_DPRINT("replace global object with: %!T", duk_get_tval(ctx, -1)));

	h_glob = duk_require_hobject(ctx, -1);
	DUK_ASSERT(h_glob != NULL);

	/*
	 *  Replace global object.
	 */

	h_prev_glob = thr->builtins[DUK_BIDX_GLOBAL];
	thr->builtins[DUK_BIDX_GLOBAL] = h_glob;
	DUK_HOBJECT_INCREF(thr, h_glob);
	DUK_HOBJECT_DECREF_ALLOWNULL(thr, h_prev_glob);  /* side effects, in theory (referenced by global env) */

	/*
	 *  Replace lexical environment for global scope
	 *
	 *  Create a new object environment for the global lexical scope.
	 *  We can't just reset the _Target property of the current one,
	 *  because the lexical scope is shared by other threads with the
	 *  same (initial) built-ins.
	 */

	(void) duk_push_object_helper(ctx,
	                              DUK_HOBJECT_FLAG_EXTENSIBLE |
	                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_OBJENV),
	                              -1);  /* no prototype, updated below */

	duk_dup(ctx, -2);
	duk_dup(ctx, -3);

	/* [ ... new_glob new_env new_glob new_glob ] */

	duk_xdef_prop_stridx(thr, -3, DUK_STRIDX_INT_TARGET, DUK_PROPDESC_FLAGS_NONE);
	duk_xdef_prop_stridx(thr, -2, DUK_STRIDX_INT_THIS, DUK_PROPDESC_FLAGS_NONE);

	/* [ ... new_glob new_env ] */

	h_env = duk_get_hobject(ctx, -1);
	DUK_ASSERT(h_env != NULL);

	h_prev_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	thr->builtins[DUK_BIDX_GLOBAL_ENV] = h_env;
	DUK_HOBJECT_INCREF(thr, h_env);
	DUK_HOBJECT_DECREF_ALLOWNULL(thr, h_prev_env);  /* side effects */
	DUK_UNREF(h_env);  /* without refcounts */
	DUK_UNREF(h_prev_env);

	/* [ ... new_glob new_env ] */

	duk_pop_2(ctx);

	/* [ ... ] */
}
