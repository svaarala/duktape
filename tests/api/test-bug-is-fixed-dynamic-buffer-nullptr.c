/*
 *  Duktape 1.2.1: duk_is_dynamic_buffer() and duk_is_fixed_buffer() does a
 *  NULL pointer dereference when the index is invalid.
 */

/*===
*** test_1 (duk_safe_call)
duk_is_dynamic_buffer(-1): 0
duk_is_fixed_buffer(-1): 0
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	/* These would segfault or at least cause valgrind issues. */
	printf("duk_is_dynamic_buffer(-1): %d\n", (int) duk_is_dynamic_buffer(ctx, -1));
	printf("duk_is_fixed_buffer(-1): %d\n", (int) duk_is_fixed_buffer(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
