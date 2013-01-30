/*
 *  Default allocation functions.
 *
 *  Assumes behavior such as malloc allowing zero size, yielding
 *  a NULL or a unique pointer which is a no-op for free.
 */

#include "duk_internal.h"

void *duk_default_alloc_function(void *udata, size_t size) {
	void *res;
	res = malloc(size);
	res = res;  /* suppress warning */
	DUK_DDDPRINT("default alloc function: %d -> %p", size, res);
	return res;
}

void *duk_default_realloc_function(void *udata, void *ptr, size_t newsize) {
	void *res;
	res = realloc(ptr, newsize);
	res = res;  /* suppress warning */
	DUK_DDDPRINT("default realloc function: %p %d -> %p", ptr, newsize, res);
	return res;
}

void duk_default_free_function(void *udata, void *ptr) {
	DUK_DDDPRINT("default free function: %p", ptr);
	free(ptr);
}


