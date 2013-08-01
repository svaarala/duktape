/*
 *  File I/O binding example.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "duk_api.h"

int duk_fileio_readfile(duk_context *ctx) {
	const char *filename = duk_to_string(ctx, 0);
	FILE *f = NULL;
	long len;
	void *buf;
	size_t got;

	if (!filename) {
		goto error;
	}

	f = fopen(filename, "rb");
	if (!f) {
		goto error;
	}

	if (fseek(f, 0, SEEK_END) != 0) {
		goto error;
	}

	len = ftell(f);

	if (fseek(f, 0, SEEK_SET) != 0) {
		goto error;
	}

	buf = duk_push_fixed_buffer(ctx, (size_t) len);

	got = fread(buf, 1, len, f);
	if (got != len) {
		goto error;
	}

	fclose(f);
	f = NULL;

	return 1;

 error:
	if (f) {
		fclose(f);
	}

	return DUK_RET_ERROR;
}

void duk_fileio_register(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_string(ctx, "fileio");
	duk_push_object(ctx);

	duk_push_string(ctx, "readfile");
	duk_push_c_function(ctx, duk_fileio_readfile, 1);
	duk_put_prop(ctx, -3);

	duk_put_prop(ctx, -3);
	duk_pop(ctx);
}

