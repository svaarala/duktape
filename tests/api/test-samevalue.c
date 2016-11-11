/*
 *  duk_samevalue()
 */

/*===
*** test_basic (duk_safe_call)
0 0 -> 1
0 1 -> 0
0 2 -> 0
0 3 -> 0
0 4 -> 0
1 0 -> 0
1 1 -> 1
1 2 -> 0
1 3 -> 0
1 4 -> 0
2 0 -> 0
2 1 -> 0
2 2 -> 1
2 3 -> 0
2 4 -> 0
3 0 -> 0
3 1 -> 0
3 2 -> 0
3 3 -> 1
3 4 -> 0
4 0 -> 0
4 1 -> 0
4 2 -> 0
4 3 -> 0
4 4 -> 0
invalid: 0
invalid: 0
final top: 4
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, j;

	(void) udata;

	duk_eval_string(ctx, "-0");
	duk_eval_string(ctx, "+0");
	duk_eval_string(ctx, "0/0");
	duk_eval_string(ctx, "123");

	/* Overshoot index intentionally: invalid indexes compare false. */
	for (i = 0; i < 4 + 1; i++) {
		for (j = 0; j < 4 + 1; j++) {
			printf("%ld %ld -> %ld\n", (long) i, (long) j, (long) duk_samevalue(ctx, i, j));
		}
	}

	/* Invalid indices compare false. */
	printf("invalid: %ld\n", (long) duk_samevalue(ctx, -100, 100));
	printf("invalid: %ld\n", (long) duk_samevalue(ctx, DUK_INVALID_INDEX, DUK_INVALID_INDEX));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
