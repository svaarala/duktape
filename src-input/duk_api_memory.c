/*
 *  Memory calls.
 */

#include "duk_internal.h"

DUK_EXTERNAL void *duk_alloc_raw(duk_context *ctx, duk_size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	return DUK_ALLOC_RAW(thr->heap, size);
}

DUK_EXTERNAL void duk_free_raw(duk_context *ctx, void *ptr) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	DUK_FREE_RAW(thr->heap, ptr);
}

DUK_EXTERNAL void *duk_realloc_raw(duk_context *ctx, void *ptr, duk_size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	return DUK_REALLOC_RAW(thr->heap, ptr, size);
}

DUK_EXTERNAL void *duk_alloc(duk_context *ctx, duk_size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	return DUK_ALLOC(thr->heap, size);
}

DUK_EXTERNAL void duk_free(duk_context *ctx, void *ptr) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	DUK_FREE(thr->heap, ptr);
}

DUK_EXTERNAL void *duk_realloc(duk_context *ctx, void *ptr, duk_size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT_CTX_VALID(ctx);

	/*
	 *  Note: since this is an exposed API call, there should be
	 *  no way a mark-and-sweep could have a side effect on the
	 *  memory allocation behind 'ptr'; the pointer should never
	 *  be something that Duktape wants to change.
	 *
	 *  Thus, no need to use DUK_REALLOC_INDIRECT (and we don't
	 *  have the storage location here anyway).
	 */

	return DUK_REALLOC(thr->heap, ptr, size);
}

DUK_EXTERNAL void duk_get_memory_functions(duk_context *ctx, duk_memory_functions *out_funcs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;

	DUK_ASSERT_CTX_VALID(ctx);
	DUK_ASSERT(out_funcs != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	heap = thr->heap;
	out_funcs->alloc_func = heap->alloc_func;
	out_funcs->realloc_func = heap->realloc_func;
	out_funcs->free_func = heap->free_func;
	out_funcs->udata = heap->heap_udata;
}

DUK_EXTERNAL void duk_gc(duk_context *ctx, duk_uint_t flags) {
#ifdef DUK_USE_MARK_AND_SWEEP
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;

	DUK_UNREF(flags);

	/* NULL accepted */
	if (!ctx) {
		return;
	}
	DUK_ASSERT_CTX_VALID(ctx);
	heap = thr->heap;
	DUK_ASSERT(heap != NULL);

	DUK_D(DUK_DPRINT("mark-and-sweep requested by application"));
	duk_heap_mark_and_sweep(heap, 0);
#else
	DUK_D(DUK_DPRINT("mark-and-sweep requested by application but mark-and-sweep not enabled, ignoring"));
	DUK_UNREF(ctx);
	DUK_UNREF(flags);
#endif
}
