#include <stdio.h>
#include "duktape.h"
#include "duk_v1_compat.h"

void duk_dump_context_stdout(duk_context *ctx) {
	duk_push_context_dump(ctx);
	fprintf(stdout, "%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
}

void duk_dump_context_stderr(duk_context *ctx) {
	duk_push_context_dump(ctx);
	fprintf(stderr, "%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
}
