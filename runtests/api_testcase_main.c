/*
 *  Wrapper for running an API test case.
 */

#include <stdio.h>
#include <stdlib.h>
#include "duktape.h"

extern void test(duk_context *ctx);

static duk_ret_t runtests_print_alert_helper(duk_context *ctx, FILE *fh) {
	duk_idx_t nargs;
	const duk_uint8_t *buf;
	duk_size_t sz_buf;

	nargs = duk_get_top(ctx);
	if (nargs == 1 && duk_is_buffer(ctx, 0)) {
		buf = (const duk_uint8_t *) duk_get_buffer(ctx, 0, &sz_buf);
		fwrite((const void *) buf, 1, (size_t) sz_buf, fh);
	} else {
		duk_push_string(ctx, " ");
		duk_insert(ctx, 0);
		duk_join(ctx, nargs);
		fprintf(fh, "%s\n", duk_to_string(ctx, -1));
	}
	fflush(fh);
	return 0;
}
static duk_ret_t runtests_print(duk_context *ctx) {
	return runtests_print_alert_helper(ctx, stdout);
}
static duk_ret_t runtests_alert(duk_context *ctx) {
	return runtests_print_alert_helper(ctx, stderr);
}

int main(int argc, char *argv[]) {
	duk_context *ctx = NULL;

	ctx = duk_create_heap_default();
	if (!ctx) {
		fprintf(stderr, "cannot allocate heap for testcase\n");
		exit(1);
	}

	/* Minimal print() replacement; removed in Duktape 2.x. */
	duk_push_global_object(ctx);
	duk_push_string(ctx, "print");
	duk_push_c_function(ctx, runtests_print, DUK_VARARGS);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_push_string(ctx, "alert");
	duk_push_c_function(ctx, runtests_alert, DUK_VARARGS);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_pop(ctx);

	test(ctx);

	duk_destroy_heap(ctx);
}

