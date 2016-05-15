/*---
{
    "skip": true
}
---*/

/*===
*** test_raw (duk_safe_call)
Hello world from a file!
return value is: 123
compile rc=1 -> SyntaxError: invalid object literal (line 1)
top: 0
==> rc=0, result='undefined'
===*/

#define  TMPFILE  "/tmp/duk-test-compile-file-temp.js"

#include <stdio.h>

static void write_file(const char *filename, const char *data) {
	FILE *f;
	size_t ret;

	/* Write temporary data to a temp file.  This now expects to be
	 * executed on Linux, so that /tmp works.  Less than ideal, also
	 * doesn't delete the temporary file.
	 */

	f = fopen(filename, "wb");
	if (!f) {
		printf("failed to open %s\n", TMPFILE);
		return;
	}
	ret = fwrite((const void *) data, 1, strlen(data), f);
	fflush(f);
	fclose(f);
	f = NULL;
	if (ret != strlen(data)) {
		printf("failed to write test data fully\n");
		return;
	}
}

static duk_ret_t test_raw(duk_context *ctx, void *udata) {
	const char *data1 = "print('Hello world from a file!'); 123;";
	const char *data2 = "print('Hello world from a file, with syntax error'); obj = {";
	duk_ret_t rc;

	(void) udata;

	write_file(TMPFILE, data1);
	duk_compile_file(ctx, 0, TMPFILE);
	duk_call(ctx, 0);
	printf("return value is: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	write_file(TMPFILE, data2);
	rc = duk_pcompile_file(ctx, 0, TMPFILE);
	printf("compile rc=%d -> %s\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_raw);
}
