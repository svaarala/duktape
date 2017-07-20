/*
 *  duk_pcall_prop() allows a situation where the nargs count is such that
 *  the target object is identified as the key or even part of the argument
 *  list.
 *
 *  This behavior is not very useful but test for the current behavior.
 */

/*===
*** test_obj_is_key (duk_safe_call)
top before: 4
toString() called
myProp called
foo bar
duk_pcall_prop() rc: 0
final top: 1
==> rc=0, result='undefined'
*** test_obj_is_arg (duk_safe_call)
top before: 5
myProp called
toString() called
myProp foo
duk_pcall_prop() rc: 0
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_obj_is_key(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_eval_string(ctx,
		"({\n"
		"    toString: function () { print('toString() called'); return 'myProp' },\n"
		"    myProp: function (a, b) { print('myProp called'); print(a, b); }\n"
		"})\n");

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");

	/* [ obj 'foo' 'bar' 'quux' ]
	 *
	 *  <key> <--- args ------>
	 *
	 *   ^
	 *   `-- obj_idx
	 */

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_pcall_prop(ctx, -4, 3 /*nargs */);
	printf("duk_pcall_prop() rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_obj_is_arg(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_string(ctx, "myProp");

	duk_eval_string(ctx,
		"({\n"
		"    toString: function () { print('toString() called'); return 'myProp' },\n"
		"    myProp: function (a, b) { print('myProp called'); print(a, b); }\n"
		"})\n");

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");

	/* [ 'myProp' obj 'foo' 'bar' 'quux' ]
	 *
	 *    <key>   <------- args ------>
	 *
	 *             ^
	 *             `-- obj_idx
	 */

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_pcall_prop(ctx, -4, 4 /*nargs */);
	printf("duk_pcall_prop() rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_obj_is_key);
	TEST_SAFE_CALL(test_obj_is_arg);
}
