/*
 *  duk_buffer_to_string()
 */

/*===
*** test_plain (duk_safe_call)
duk_buffer_to_string: 'abcdefghijklmnop'
duk_buffer_to_string: ''
final top: 2
==> rc=0, result='undefined'
===*/

/* For plain buffers the behavior is pretty clear. */

static duk_ret_t test_plain(duk_context *ctx, void *udata) {
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
*** test_view (duk_safe_call)
duk_buffer_to_string: 'ccccddddeeee'
final top: 1
==> rc=0, result='undefined'
===*/

/* For views the active slice is interpreted as -bytes- (not elements) and
 * the bytes are used to create the string.
 */

static duk_ret_t test_view(duk_context *ctx, void *udata) {
	(void) udata;

	/* Byte order independent initializer. */
	duk_eval_string(ctx, "new Uint32Array([ 0x61616161, 0x62626262, 0x63636363, "
	                     "                  0x64646464, 0x65656565, 0x66666666 ]).subarray(2, 5)");
	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_unbacked_view (duk_safe_call)
duk_buffer_to_string: 'abcdefghijklmnop'
==> rc=1, result='TypeError: buffer required, found [object Uint32Array] (stack index 1)'
===*/

/* TypeError if view is unbacked. */

static duk_ret_t test_unbacked_view(duk_context *ctx, void *udata) {
	void *ptr;
	int i;

	(void) udata;

	ptr = duk_push_dynamic_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		((unsigned char *) ptr)[i] = (unsigned char) (0x61 + i);
	}
	duk_push_buffer_object(ctx, 0, 0, 16, DUK_BUFOBJ_UINT32ARRAY);

	duk_dup(ctx, -1);
	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));
	duk_pop(ctx);

	duk_resize_buffer(ctx, -2, 15);
	printf("duk_buffer_to_string: '%s'\n", duk_buffer_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_type (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found true (stack index 0)'
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
	TEST_SAFE_CALL(test_plain);
	TEST_SAFE_CALL(test_view);
	TEST_SAFE_CALL(test_unbacked_view);
	TEST_SAFE_CALL(test_invalid_type);
	TEST_SAFE_CALL(test_invalid_index1);
	TEST_SAFE_CALL(test_invalid_index2);
}
