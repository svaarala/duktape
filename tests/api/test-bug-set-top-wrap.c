/*===
*** test_1 (duk_safe_call)
top=0
==> rc=1, result='Error: invalid stack index 536870912'
*** test_2 (duk_safe_call)
top=0
==> rc=1, result='Error: invalid stack index 357913942'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	printf("top=%ld\n", (long) duk_get_top(ctx));

	/* duk_set_top() uses pointer arithmetic internally, and because
	 * duk_tval is a multibyte structure, it's possible to wrap the
	 * pointers and allow incorrect duk_set_top() calls to work.
	 *
	 * For instance, on a 32-bit platform and 8-byte duk_tval values,
	 * the top value (2**32 / 8) = 0x20000000 is equivalent to using
	 * the top value 0.  This should not be the case.
	 *
	 * This test case will NOT fail on a 64-bit platform.
	 */

	duk_push_string(ctx, "foo");
	duk_set_top(ctx, 0x20000000);

	printf("top=%ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	printf("top=%ld\n", (long) duk_get_top(ctx));

	/* On a 32-bit platform and 12-byte values there is no zero-
	 * equivalent value: (2**32 / 12) = 0x15555555, but 0x1555555 * 12
	 * = 0xfffffffc.  This is even more harmful than simple wrapping.
	 * Test value 0x15555556 = 0x100000008, wraps to 8.
	 */

	duk_push_string(ctx, "foo");
	duk_set_top(ctx, 0x15555556);

	printf("top=%ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
