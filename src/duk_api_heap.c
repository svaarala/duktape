/*
 *  Heap creation and destruction
 */

#include "duk_internal.h"

typedef struct duk_internal_thread_state duk_internal_thread_state;

struct duk_internal_thread_state {
	duk_ljstate lj;
	duk_bool_t handling_error;
	duk_hthread *curr_thread;
	duk_int_t call_recursion_depth;
};

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
#if defined(DUK_USE_PROVIDE_DEFAULT_ALLOC_FUNCTIONS)
		alloc_func = duk_default_alloc_function;
		realloc_func = duk_default_realloc_function;
		free_func = duk_default_free_function;
#else
		DUK_D(DUK_DPRINT("no allocation functions given and no default providers"));
		return NULL;
#endif
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

DUK_EXTERNAL void duk_suspend(duk_context *ctx, duk_thread_state *state) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_internal_thread_state *snapshot = (duk_internal_thread_state *) state;
	duk_heap *heap;
	duk_ljstate *lj;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	heap = thr->heap;
	lj = &heap->lj;

	duk_push_tval(ctx, &lj->value1);
	duk_push_tval(ctx, &lj->value2);

	DUK_MEMMOVE((void *) &snapshot->lj, (const void *) lj, sizeof(duk_ljstate));
	snapshot->handling_error = heap->handling_error;
	snapshot->curr_thread = heap->curr_thread;
	snapshot->call_recursion_depth = heap->call_recursion_depth;

	lj->jmpbuf_ptr = NULL;
	lj->type = DUK_LJ_TYPE_UNKNOWN;
	DUK_TVAL_SET_UNDEFINED(&lj->value1);
	DUK_TVAL_SET_UNDEFINED(&lj->value2);
	heap->handling_error = 0;
	heap->curr_thread = NULL;
	heap->call_recursion_depth = 0;
}

DUK_EXTERNAL void duk_resume(duk_context *ctx, const duk_thread_state *state) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const duk_internal_thread_state *snapshot = (const duk_internal_thread_state *) state;
	duk_heap *heap;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	heap = thr->heap;

	DUK_MEMMOVE((void *) &heap->lj, (const void *) &snapshot->lj, sizeof(duk_ljstate));
	heap->handling_error = snapshot->handling_error;
	heap->curr_thread = snapshot->curr_thread;
	heap->call_recursion_depth = snapshot->call_recursion_depth;

	duk_pop_2(ctx);
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
	DUK_UNREF(h_prev_glob);
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

/* FIXME: add a duk_recheck_builtins() which would go through the thr->builtins[]
 * array and copy current values from the global object into the array.
 * For example, global.Duktape would be copied to thr->builtins[DUK_BIDX_DUKTAPE].
 * This would allow sandboxing code to reset the built-in references without
 * exposing the internal indices.
 *
 * FIXME: the duk_recheck_builtins() could also take the global object as a stack
 * argument rather than taking the current value, so that it'd be possible to
 * switch to a fresh global object without any dependency to the current global
 * object.  It would then potentially deprecate duk_set_global_object().
 */
