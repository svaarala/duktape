/*===
*** test_basic (duk_safe_call)
0: 1 0
1: 1 0
2: 1 1
3: 1 1
4: 1 1
5: 1 0
6: 1 0
7: 1 1
8: 0 0
final top: 8
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

	(void) udata;

	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "\x80" "bar");
	duk_push_string(ctx, "\x81" "quux");
	duk_push_string(ctx, "\x82" "baz");
	duk_push_string(ctx, "\xc0" "quuux");
	duk_push_string(ctx, "\xfe" "quuuux");
	duk_push_string(ctx, "\xff" "quuuuux");

	for (i = 0, n = duk_get_top(ctx) + 1; i < n; i++) {
		printf("%ld: %ld %ld\n", (long) i, (long) duk_is_string(ctx, i), (long) duk_is_symbol(ctx, i));
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
