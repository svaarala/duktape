/*===
*** test_basic (duk_safe_call)
i=0, n=19, charcode=102
i=1, n=19, charcode=111
i=2, n=19, charcode=111
i=3, n=19, charcode=4660
i=4, n=19, charcode=98
i=5, n=19, charcode=97
i=6, n=19, charcode=114
i=7, n=19, charcode=160
i=8, n=19, charcode=113
i=9, n=19, charcode=117
i=10, n=19, charcode=117
i=11, n=19, charcode=120
i=12, n=19, charcode=0
i=13, n=19, charcode=98
i=14, n=19, charcode=97
i=15, n=19, charcode=122
i=16, n=19, charcode=0
i=17, n=19, charcode=0
i=18, n=19, charcode=0
==> rc=0, result='undefined'
*** test_invalid_arg (duk_safe_call)
==> rc=1, result='TypeError: string required, found 123 (stack index -1)'
*** test_invalid_utf8 (duk_safe_call)
index 0: 65
index 1: 65533
index 2: 66
index 3: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_size_t i, n;

	(void) udata;

	/* Simple test, intentional out-of-bounds access at the end. */

	duk_eval_string(ctx, "'foo\\u1234bar\\u00a0quux\\u0000baz'");

	n = duk_get_length(ctx, -1) + 3;  /* access 3 times out-of-bounds */
	for (i = 0; i < n; i++) {
		printf("i=%ld, n=%ld, charcode=%d\n", (long) i, (long) n, (int) duk_char_code_at(ctx, -1, i));
	}

	duk_pop(ctx);
	return 0;
}

static duk_ret_t test_invalid_arg(duk_context *ctx, void *udata) {
	(void) udata;

	/* TypeError for invalid arg type */

	duk_push_int(ctx, 123);
	(void) duk_char_code_at(ctx, -1, 10);
	duk_pop(ctx);
	return 0;
}

static duk_ret_t test_invalid_utf8(duk_context *ctx, void *udata) {
	char buf[3] = { (char) 0x41, (char) 0xff, (char) 0x42 };
	size_t i;

	(void) udata;

	duk_push_lstring(ctx, (const char *) buf, sizeof(buf));
	for (i = 0; i < sizeof(buf) + 1; i++) {  /* overshoot by one on purpose */
		printf("index %d: %d\n", (int) i, (int) duk_char_code_at(ctx, -1, i));
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_invalid_arg);
	TEST_SAFE_CALL(test_invalid_utf8);
}
