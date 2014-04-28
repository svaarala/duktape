/*
 *  Default allocation functions.
 *
 *  Assumes behavior such as malloc allowing zero size, yielding
 *  a NULL or a unique pointer which is a no-op for free.
 */

#include "duk_internal.h"

void *duk_default_alloc_function(void *udata, size_t size) {
	void *res;
	DUK_UNREF(udata);
	res = DUK_ANSI_MALLOC(size);
	DUK_DDD(DUK_DDDPRINT("default alloc function: %d -> %p",
	                     (int) size, (void *) res));
	return res;
}

void *duk_default_realloc_function(void *udata, void *ptr, size_t newsize) {
	void *res;
	DUK_UNREF(udata);
	res = DUK_ANSI_REALLOC(ptr, newsize);
	DUK_DDD(DUK_DDDPRINT("default realloc function: %p %d -> %p", 
	                     (void *) ptr, (int) newsize, (void *) res));
	return res;
}

void duk_default_free_function(void *udata, void *ptr) {
	DUK_DDD(DUK_DDDPRINT("default free function: %p", (void *) ptr));
	DUK_UNREF(udata);
	DUK_ANSI_FREE(ptr);
}
