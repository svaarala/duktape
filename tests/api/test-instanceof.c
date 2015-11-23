/*
 *  duk_instanceof()
 */

/*===
*** test_1 (duk_safe_call)
0 instanceof 1: 1
0 instanceof 2: 1
0 instanceof 3: 0
0 instanceof 4: 1
0 instanceof 5: 0
final top: 6
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: invalid instanceof rval'
*** test_3a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -1'
*** test_3b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 0'
*** test_3c (duk_safe_call)
==> rc=1, result='Error: invalid stack index 1'
*** test_3d (duk_safe_call)
==> rc=1, result='Error: invalid stack index 0'
*** test_3e (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

/* Basic test. */
static duk_ret_t test_1(duk_context *ctx) {
	int i;

	duk_eval_string(ctx, "new RangeError('test error')");  /* 0 */
	duk_eval_string(ctx, "Error");  /* 1 */
	duk_eval_string(ctx, "RangeError");  /* 2 */
	duk_eval_string(ctx, "TypeError");  /* 3 */
	duk_eval_string(ctx, "Object");  /* 4 */
	duk_eval_string(ctx, "Function");  /* 5 */

	for (i = 1; i <= 5; i++) {
		printf("0 instanceof %d: %d\n", i, (int) duk_instanceof(ctx, 0, i));
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_instanceof() inherits the Ecmascript type requirements for lval and rval.
 * In particular, rval must be a -callable- object.  Here, for example, trying
 * to do the equivalent of: "new Error() instanceof new Error()" is a TypeError
 * because the rval is a non-callable object.
 */
static duk_ret_t test_2(duk_context *ctx) {
	duk_eval_string(ctx, "new Error('test error')");

	duk_instanceof(ctx, 0, 0);  /* -> TypeError */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Strict behavior is also used for indices to match the strictness of
 * instanceof.  (This differs from e.g. duk_equals() on purpose.)
 */
static duk_ret_t test_3a(duk_context *ctx) {
	printf("%d\n", duk_instanceof(ctx, -1, 0));  /* -1 is out of stack */
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_3b(duk_context *ctx) {
	printf("%d\n", duk_instanceof(ctx, 0, -1));  /* -1 is out of stack */
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_3c(duk_context *ctx) {
	printf("%d\n", duk_instanceof(ctx, 1, 0));  /* 1 is out of stack */
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_3d(duk_context *ctx) {
	printf("%d\n", duk_instanceof(ctx, 0, 1));  /* 1 is out of stack */
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_3e(duk_context *ctx) {
	printf("%d\n", duk_instanceof(ctx, DUK_INVALID_INDEX, DUK_INVALID_INDEX));
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3a);
	TEST_SAFE_CALL(test_3b);
	TEST_SAFE_CALL(test_3c);
	TEST_SAFE_CALL(test_3d);
	TEST_SAFE_CALL(test_3e);
}
