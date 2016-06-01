/*
 *  duk_debugger_notify()
 */

/*===
*** test_notify_not_attached (duk_safe_call)
top: 0
top: 0
top: 1
string: dummy below nvalues
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_notify_not_attached(duk_context *ctx, void *udata) {
	(void) udata;

	/* Not attached, ignored; no arguments. */
	duk_debugger_notify(ctx, 0);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	/* Not attached, ignored, two arguments. */
	duk_push_string(ctx, "DummyNotify");
	duk_push_int(ctx, 123);
	duk_debugger_notify(ctx, 2);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	/* Check that argument below 'nvalues' is untouched. */
	duk_push_string(ctx, "dummy below nvalues");
	duk_push_string(ctx, "DummyNotify");
	duk_push_int(ctx, 123);
	duk_debugger_notify(ctx, 2);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("string: %s\n", duk_safe_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_notify_invalid_count1 (duk_safe_call)
==> rc=1, result='RangeError: not enough stack values for notify'
===*/

static duk_ret_t test_notify_invalid_count1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "DummyNotify");
	duk_debugger_notify(ctx, 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_notify_invalid_count2 (duk_safe_call)
==> rc=1, result='RangeError: invalid count'
===*/

static duk_ret_t test_notify_invalid_count2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "DummyNotify");
	duk_debugger_notify(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_notify_not_attached);
	TEST_SAFE_CALL(test_notify_invalid_count1);
	TEST_SAFE_CALL(test_notify_invalid_count2);
}
