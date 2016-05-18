/*---
{
    "skip": true
}
---*/

/*===
*** test_raw (duk_safe_call)
top: 0
Hello world from a file!
return value is: 123.000000
top: 0
Hello world from a file!
no result
top: 0
Hello world from a file, with exception
return value is: Error: eval error (rc=1)
top: 0
Hello world from a file, with exception
no result, rc=1
top: 0
return value is: SyntaxError: invalid object literal (line 1) (rc=1)
top: 0
no result, rc=1
top: 0
==> rc=0, result='undefined'
===*/

#define  TMPFILE  "/tmp/duk-test-eval-file-temp.js"

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
	const char *data2 = "print('Hello world from a file, with exception'); throw new Error('eval error');";
	const char *data3 = "print('Hello world from a file, with syntax error'); obj = {";
	duk_ret_t rc;

	(void) udata;

	write_file(TMPFILE, data1);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_file(ctx, TMPFILE);
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_file_noresult(ctx, TMPFILE);
	printf("no result\n");

	write_file(TMPFILE, data2);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_file(ctx, TMPFILE);
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), (int) rc);
	duk_pop(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_file_noresult(ctx, TMPFILE);
	printf("no result, rc=%d\n", (int) rc);

	write_file(TMPFILE, data3);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_file(ctx, TMPFILE);
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), (int) rc);
	duk_pop(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_file_noresult(ctx, TMPFILE);
	printf("no result, rc=%d\n", (int) rc);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_raw);
}
