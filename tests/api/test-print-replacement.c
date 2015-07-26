/*
 *  Example of how you can replace the global print() function from C code.
 */

/*===
*** test_1 (duk_safe_call)
PRINT: foo bar
==> rc=0, result='undefined'
===*/

static duk_ret_t my_print(duk_context *ctx) {
	duk_idx_t i, n;
	fprintf(stdout, "PRINT: ");
	for (i = 0, n = duk_get_top(ctx); i < n; i++) {
		if (i > 0) {
			fprintf(stdout, " ");
		}
		fprintf(stdout, "%s", duk_safe_to_string(ctx, i));
	}
	fprintf(stdout, "\n");
	fflush(stdout);
	return 0;
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_c_function(ctx, my_print, DUK_VARARGS);
	duk_put_prop_string(ctx, -2, "print");
	duk_pop(ctx);

	duk_eval_string_noresult(ctx, "print('foo', 'bar');");

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
