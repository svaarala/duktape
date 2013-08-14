/*
 *  Wrapper for running an API test case.
 */

#include <stdio.h>
#include <stdlib.h>
#include "duktape.h"

extern void test(duk_context *ctx);

int main(int argc, char *argv[]) {
	duk_context *ctx = NULL;

	/* FIXME: resource limits */

	ctx = duk_create_heap_default();
	if (!ctx) {
		fprintf(stderr, "cannot allocate heap for testcase\n");
		exit(1);
	}

	test(ctx);

	duk_destroy_heap(ctx);
}

