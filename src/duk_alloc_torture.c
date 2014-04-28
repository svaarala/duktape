/*
 *  Torture allocation functions.
 *
 *  Provides various debugging features:
 *
 *    - Wraps allocations with "buffer zones" which are checked on free
 *    - Overwrites freed memory with garbage (not zero)
 *    - Debug prints memory usage info after every alloc/realloc/free
 *
 *  Can be left out of a standard compilation.
 */

#include "duk_internal.h"

/* FIXME: unimplemented */

void *duk_torture_alloc_function(void *udata, size_t size) {
	void *res;
	DUK_UNREF(udata);
	res = DUK_ANSI_MALLOC(size);
	DUK_DDD(DUK_DDDPRINT("torture alloc function: %d -> %p",
	                     (int) size, (void *) res));
	return res;
}

void *duk_torture_realloc_function(void *udata, void *ptr, size_t newsize) {
	void *res;
	DUK_UNREF(udata);
	res = DUK_ANSI_REALLOC(ptr, newsize);
	DUK_DDD(DUK_DDDPRINT("torture realloc function: %p %d -> %p",
	                     (void *) ptr, (int) newsize, (void *) res));
	return res;
}

void duk_torture_free_function(void *udata, void *ptr) {
	DUK_DDD(DUK_DDDPRINT("torture free function: %p", (void *) ptr));
	DUK_UNREF(udata);
	DUK_ANSI_FREE(ptr);
}
