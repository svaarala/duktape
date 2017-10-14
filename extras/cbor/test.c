#include <stdio.h>
#include "duktape.h"
#include "duk_cbor.h"

int main(int argc, char *argv[]) {
	duk_context *ctx;
	unsigned char *buf;
	size_t len;
	duk_int_t rc;
	duk_idx_t top;

	if (argc < 2) {
		fprintf(stderr, "Usage: ./test evalstring\n");
		exit(1);
	}

	ctx = duk_create_heap_default();
	if (!ctx) {
		return 1;
	}

	duk_push_string(ctx, argv[1]);
	rc = duk_peval(ctx);
	if (rc != 0) {
		fprintf(stderr, "eval failed: %d: %s\n", (int) rc, duk_safe_to_string(ctx, -1));
		exit(1);
	}

	top = duk_get_top(ctx);
	duk_cbor_encode(ctx, -1, 0);
	if (duk_get_top(ctx) != top) {
		fprintf(stderr, "top invalid after duk_cbor_encode: %d vs. %d\n", duk_get_top(ctx), top);
		exit(1);
	}

	buf = (unsigned char *) duk_require_buffer_data(ctx, -1, &len);
	fwrite((void *) buf, 1, len, stdout);

	duk_destroy_heap(ctx);
	return 0;
}
