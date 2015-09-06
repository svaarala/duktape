/*
 *  This testcase should succeed even when Node.js and Khronos/ES6 TypedArray
 *  support has been turned off.
 */

/*===
*** test_basic (duk_safe_call)
[object Buffer]
true
|1011121314151617|
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx) {
	unsigned char *p;
	int i;

	p = (unsigned char *) duk_push_fixed_buffer(ctx, 256);
	for (i = 0; i < 256; i++) {
		p[i] = (unsigned char) i;
	}

	duk_push_buffer_object(ctx, -1, 16, 8, DUK_BUFOBJ_DUKTAPE_BUFFER);
	duk_eval_string(ctx,
		"(function (v) {\n"
		"    print(Object.prototype.toString.call(v));\n"
		"    print(v instanceof Duktape.Buffer);\n"
		"    print(Duktape.enc('jx', v));\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);
	duk_pop_2(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
