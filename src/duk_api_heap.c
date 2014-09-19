/*
 *  Heap creation and destruction
 */

#include "duk_internal.h"

duk_context *duk_create_heap(duk_alloc_function alloc_func,
                             duk_realloc_function realloc_func,
                             duk_free_function free_func,
                             void *alloc_udata,
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

	heap = duk_heap_alloc(alloc_func, realloc_func, free_func, alloc_udata, fatal_handler);
	if (!heap) {
		return NULL;
	}
	ctx = (duk_context *) heap->heap_thread;
	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(((duk_hthread *) ctx)->heap != NULL);
	return ctx;
}

void duk_destroy_heap(duk_context *ctx) {
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
void duk_set_global_object(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hobject *h_glob;
	duk_hobject *h_prev_glob;
	duk_hobject *h_env;
	duk_tval tv_tmp;
	duk_tval *tv;

	DUK_D(DUK_DPRINT("replace global object with: %!T", duk_get_tval(ctx, -1)));

	h_glob = duk_require_hobject(ctx, -1);
	DUK_ASSERT(h_glob != NULL);

	h_prev_glob = thr->builtins[DUK_BIDX_GLOBAL];
	thr->builtins[DUK_BIDX_GLOBAL] = h_glob;
	DUK_HOBJECT_INCREF(thr, h_glob);
	DUK_HOBJECT_DECREF(thr, h_prev_glob);  /* side effects, in theory (referenced by global env) */

	h_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	if (h_env) {
		DUK_ASSERT(DUK_HOBJECT_GET_CLASS_NUMBER(h_env) == DUK_HOBJECT_CLASS_OBJENV);
		tv = duk_hobject_find_existing_entry_tval_ptr(h_env, DUK_HTHREAD_STRING_INT_TARGET(thr));
		DUK_ASSERT(tv != NULL);
		DUK_TVAL_SET_TVAL(&tv_tmp, tv);
		DUK_TVAL_SET_OBJECT(tv, h_glob);
		DUK_HOBJECT_INCREF(thr, h_glob);
		DUK_TVAL_DECREF(thr, &tv_tmp);  /* side effects */
	}

	duk_pop(ctx);
}
