/*===
*** test_raw (duk_safe_call)
Hello world from a file!
return value is: 123.000000
==> rc=0, result='undefined'
===*/

#define  TMPFILE  "/tmp/duk-test-eval-file-temp.js"

#include <stdio.h>

static int test_raw(duk_context *ctx) {
	FILE *f;
	const char *data = "print('Hello world from a file!'); 123;";
	size_t ret;

	/* Write temporary data to a temp file.  This now expects to be
	 * executed on Linux, so that /tmp works.  Less than ideal, also
	 * doesn't delete the temporary file.
	 */

	f = fopen(TMPFILE, "wb");
	if (!f) {
		printf("failed to open %s\n", TMPFILE);
		return 0;
	}
	ret = fwrite((const void *) data, 1, strlen(data), f);
	fflush(f);
	fclose(f);
	f = NULL;
	if (ret != strlen(data)) {
		printf("failed to write test data fully\n");
		return 0;
	}

	duk_eval_file(ctx, TMPFILE);
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_raw);
}

