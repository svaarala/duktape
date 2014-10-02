/*
 *  Very simple example program for evaluating expressions from
 *  command line
 */

#include "duktape.h"
#include <stdio.h>

static int eval_raw(duk_context *ctx) {
	duk_eval(ctx);
	return 1;
}

static int tostring_raw(duk_context *ctx) {
	duk_to_string(ctx, -1);
	return 1;
}

static void usage_exit(void) {
	fprintf(stderr, "Usage: eval <expression> [<expression>] ...\n");
	fflush(stderr);
	exit(1);
}

int main(int argc, char *argv[]) {
	duk_context *ctx;
	int i;
	const char *res;

	if (argc < 2) {
		usage_exit();
	}

	ctx = duk_create_heap_default();
	for (i = 1; i < argc; i++) {
		printf("=== eval: '%s' ===\n", argv[i]);
		duk_push_string(ctx, argv[i]);
		duk_safe_call(ctx, eval_raw, 1 /*nargs*/, 1 /*nrets*/);
		duk_safe_call(ctx, tostring_raw, 1 /*nargs*/, 1 /*nrets*/);
		res = duk_get_string(ctx, -1);
		printf("%s\n", res ? res : "null");
		duk_pop(ctx);
	}

	duk_destroy_heap(ctx);

	return 0;
}
