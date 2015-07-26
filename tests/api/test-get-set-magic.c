/*
 *  Test setting and getting the Duktape/C function magic value.
 *
 *  The magic value is useful in sharing a single native helper
 *  with multiple Ecmascript bindings, with the helper's behavior
 *  being controlled by flags or other values in the magic value.
 *  The magic value is stored cheaply without needing a property
 *  slot.
 */

/*===
*** test_1 (duk_safe_call)
magic: 4660
magic: 4660
magic: 32767
magic: 32767
magic: -32768
magic: -32768
magic: -16657
magic: -16657
final top: 2
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: unexpected type'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: not nativefunction'
*** test_4 (duk_safe_call)
INFO: log line<LF>
WARN: log line<LF>
ERROR: log line<CR><LF>
FATAL: log line<LF>
final top: 4
==> rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	printf("magic: %ld\n", (long) duk_get_current_magic(ctx));
	return 0;
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_push_c_function(ctx, my_func, 0);
	duk_push_undefined(ctx);  /* dummy filler */

	duk_set_magic(ctx, -2, 0x1234);
	printf("magic: %ld\n", (long) duk_get_magic(ctx, -2));
	duk_dup(ctx, -2);
	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_set_magic(ctx, -2, 0x7fff);
	printf("magic: %ld\n", (long) duk_get_magic(ctx, -2));
	duk_dup(ctx, -2);
	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_set_magic(ctx, -2, -0x8000);
	printf("magic: %ld\n", (long) duk_get_magic(ctx, -2));
	duk_dup(ctx, -2);
	duk_call(ctx, 0);
	duk_pop(ctx);

	/* 0xdeadbeef gets truncated to 0xffffbeef == -16657 */
	duk_set_magic(ctx, -2, 0xdeadbeef);
	printf("magic: %ld\n", (long) duk_get_magic(ctx, -2));
	duk_dup(ctx, -2);
	duk_call(ctx, 0);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	/* duk_get_magic() is strict: incorrect target type throws an error.
	 * This minimizes compiled function size and magic manipulation is
	 * rare.
	 */
	duk_eval_string(ctx, "(function () {})");
	printf("magic: %ld\n", (long) duk_get_magic(ctx, -1));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	/* duk_set_magic() is similarly strict. */
	duk_eval_string(ctx, "(function () {})");
	duk_set_magic(ctx, -1, 0x4321);
	printf("magic: %ld\n", (long) duk_get_magic(ctx, -1));
	return 0;
}

/*
 *  Extended version of the guide example: illustrate how a magic value can
 *  be used to share a single native log write helper.
 */

static duk_ret_t guide_example(duk_context *ctx) {
	const char *prefix[4] = { "INFO", "WARN", "ERROR", "FATAL" };
	duk_int_t magic = duk_get_current_magic(ctx);

	printf("%s: %s", prefix[magic & 0x03], duk_safe_to_string(ctx, 0));
	if (magic & 0x04) {
		printf("<CR><LF>\n");
	} else {
		printf("<LF>\n");
	}

	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_push_c_function(ctx, guide_example, 1);
	duk_set_magic(ctx, -1, 0);  /* INFO */

	duk_push_c_function(ctx, guide_example, 1);
	duk_set_magic(ctx, -1, 1);  /* WARN */

	duk_push_c_function(ctx, guide_example, 1);
	duk_set_magic(ctx, -1, 2 | 0x04);  /* ERROR */

	duk_push_c_function(ctx, guide_example, 1);
	duk_set_magic(ctx, -1, 3);  /* FATAL */

	/* 0: func with INFO level
	 * 1: func with WARN level
	 * 2: func with ERROR level and CRLF newline
	 * 3: func with FATAL level
	 */

	duk_dup(ctx, 0);
	duk_push_string(ctx, "log line");
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_dup(ctx, 1);
	duk_push_string(ctx, "log line");
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_dup(ctx, 2);
	duk_push_string(ctx, "log line");
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_dup(ctx, 3);
	duk_push_string(ctx, "log line");
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
