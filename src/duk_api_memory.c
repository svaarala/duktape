/*
 *  Memory calls.
 */

#include "duk_internal.h"

void *duk_alloc_raw(duk_context *ctx, size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);

	return DUK_ALLOC_RAW(thr->heap, size);
}

void duk_free_raw(duk_context *ctx, void *ptr) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);

	DUK_FREE_RAW(thr->heap, ptr);
}

void *duk_realloc_raw(duk_context *ctx, void *ptr, size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);

	return DUK_REALLOC_RAW(thr->heap, ptr, size);
}

void *duk_alloc(duk_context *ctx, size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);

	return DUK_ALLOC(thr->heap, size);
}

void duk_free(duk_context *ctx, void *ptr) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);

	DUK_FREE(thr->heap, ptr);
}

void *duk_realloc(duk_context *ctx, void *ptr, size_t size) {
	duk_hthread *thr = (duk_hthread *) ctx;

	DUK_ASSERT(ctx != NULL);

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

void duk_get_memory_functions(duk_context *ctx, duk_memory_functions *out_funcs) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;

	DUK_ASSERT(ctx != NULL);
	DUK_ASSERT(out_funcs != NULL);
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(thr->heap != NULL);

	heap = thr->heap;
	out_funcs->alloc = heap->alloc_func;
	out_funcs->realloc = heap->realloc_func;
	out_funcs->free = heap->free_func;
	out_funcs->udata = heap->alloc_udata;
}

void duk_gc(duk_context *ctx, int flags) {
#ifdef DUK_USE_MARK_AND_SWEEP
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_heap *heap;

	DUK_UNREF(flags);

	if (!ctx) {
		return;
	}
	heap = thr->heap;
	DUK_ASSERT(heap != NULL);

	DUK_DPRINT("mark-and-sweep requested by application");
	duk_heap_mark_and_sweep(heap, 0);
#else
	DUK_DPRINT("mark-and-sweep requested by application but mark-and-sweep not enabled, ignoring");
#endif
}

