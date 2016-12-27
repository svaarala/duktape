/*
 *  Test duk_push_buffer_object() behavior with DUK_USE_BUFFEROBJECT_SUPPORT
 *  disabled.  Object coercion of a plain buffer fails.  Also ArrayBuffer,
 *  ArrayBuffer.prototype, and all other buffer object bindings are
 *  non-functional.  This behavior is likely to change so don't rely on it.
 *
 *  This testcase is skipped by default; run manually.
 */

/*---
{
    "skip": true
}
---*/

/*===
*** test_arraybuffer (duk_safe_call)
push plain buffer
push buffer object
==> rc=1, result='Error: unsupported'
*** test_uint8array (duk_safe_call)
push plain buffer
push buffer object
==> rc=1, result='Error: unsupported'
*** test_nodejsbuffer (duk_safe_call)
push plain buffer
push buffer object
==> rc=1, result='Error: unsupported'
*** test_object_coerce (duk_safe_call)
==> rc=1, result='TypeError: not object coercible'
*** test_hex_decode (duk_safe_call)
|deadbeef|
==> rc=1, result='TypeError: not object coercible'
===*/

static duk_ret_t test__helper(duk_context *ctx, duk_uint_t flags) {
	unsigned char *ptr;
	int i;

	/* Even with bufferobject support disabled it's possible to work
	 * with plain buffers.  However, you can't create actual ArrayBuffers
	 * or typed arrays.
	 */

	printf("push plain buffer\n");
	ptr = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	for (i = 0; i < 16; i++) {
		ptr[i] = 0x61 + i;
	}

	printf("push buffer object\n");
	duk_push_buffer_object(ctx, -1, 4, 10, flags);

	duk_dup_top(ctx);
	printf("--> '%s'\n", duk_buffer_to_string(ctx, -1));
	duk_pop(ctx);

	duk_eval_string(ctx,
		"(function (v) {\n"
		"    print(Duktape.enc('jx', v));\n"
		"    print(Duktape.enc('jx', ArrayBuffer.prototype.slice(v, 1, 4)));\n"
		"})\n");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_arraybuffer(duk_context *ctx, void *udata) {
	(void) udata;
	return test__helper(ctx, DUK_BUFOBJ_ARRAYBUFFER);
}

static duk_ret_t test_uint8array(duk_context *ctx, void *udata) {
	(void) udata;
	return test__helper(ctx, DUK_BUFOBJ_UINT8ARRAY);
}

static duk_ret_t test_nodejsbuffer(duk_context *ctx, void *udata) {
	(void) udata;
	return test__helper(ctx, DUK_BUFOBJ_NODEJS_BUFFER);
}

static duk_ret_t test_object_coerce(duk_context *ctx, void *udata) {
	(void) udata;

	(void) duk_push_fixed_buffer(ctx, 16);
	duk_to_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_hex_decode(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string_noresult(ctx,
                "(function () {\n"
		"    var buf = Duktape.dec('hex', 'deadbeef');\n"
		"    print(Duktape.enc('jx', buf));\n"
		"})()\n");
	(void) duk_push_fixed_buffer(ctx, 16);
	duk_to_object(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_arraybuffer);
	TEST_SAFE_CALL(test_uint8array);
	TEST_SAFE_CALL(test_nodejsbuffer);
	TEST_SAFE_CALL(test_object_coerce);
	TEST_SAFE_CALL(test_hex_decode);
}
