/*===
*** test_basic (duk_safe_call)
duk_buffer_to_string: 'abcdefghijklmnop'
duk_buffer_to_string: ''
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	unsigned char *ptr;
	int i;

	(void) udata;

	ptr = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		ptr[i] = 0x61 + i;
	}

	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));

	ptr = (unsigned char *) duk_push_fixed_buffer(ctx, 0);
	(void) ptr;

	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_type (duk_safe_call)
==> rc=1, result='TypeError: invalid call args'
===*/

static duk_ret_t test_invalid_type(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_true(ctx);

	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_index1 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -1'
===*/

static duk_ret_t test_invalid_index1(duk_context *ctx, void *udata) {
	(void) udata;

	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_index2 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
===*/

static duk_ret_t test_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, DUK_INVALID_INDEX));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_invalid_type);
	TEST_SAFE_CALL(test_invalid_index1);
	TEST_SAFE_CALL(test_invalid_index2);
}
