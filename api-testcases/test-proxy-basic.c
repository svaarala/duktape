/*
 *  ES6 Proxy handlers can also be native Duktape/C functions.
 *
 *  Just a very basic test to ensure proxy handlers work as expected.
 */

/*===
*** test_get1 (duk_safe_call)
top: 0
top: 1
handle_get: key=getTest
get result: rc=1, value=123
final top: 0
==> rc=0, result='undefined'
*** test_get2 (duk_safe_call)
top: 0
top: 1
handle_get: key=_getTest
get result: rc=1, value=fake_value
final top: 0
==> rc=0, result='undefined'
*** test_set1 (duk_safe_call)
top: 0
top: 1
handle_set: key=setTest, val=testValue
set result: rc=1
final top: 0
==> rc=0, result='undefined'
*** test_set2 (duk_safe_call)
top: 0
top: 1
handle_set: key=_setTest, val=testValue
==> rc=1, result='TypeError: proxy rejected'
*** test_delete1 (duk_safe_call)
top: 0
top: 1
handle_delete: key=deleteTest
delete result: rc=1
final top: 0
==> rc=0, result='undefined'
*** test_delete2 (duk_safe_call)
top: 0
top: 1
handle_delete: key=_deleteTest
==> rc=1, result='TypeError: proxy rejected'
===*/

static duk_ret_t handle_get(duk_context *ctx) {
	/* 'this' binding: handler
	 * [0]: target
	 * [1]: key
	 * [2]: receiver (proxy)
	 */

	const char *key = duk_to_string(ctx, 1);

	printf("handle_get: key=%s\n", key);

	if (key != NULL && key[0] == '_') {
		/* Provide a fake value for properties beginning with an underscore. */
		duk_push_string(ctx, "fake_value");
	} else {
		/* For others, read from target. */
		duk_dup(ctx, 1);
		duk_get_prop(ctx, 0);
	}
	return 1;
}

static duk_ret_t handle_set(duk_context *ctx) {
	/* 'this' binding: handler
	 * [0]: target
	 * [1]: key
	 * [2]: val
	 * [3]: receiver (proxy)
	 */

	const char *key = duk_to_string(ctx, 1);
	const char *val = duk_to_string(ctx, 2);

	printf("handle_set: key=%s, val=%s\n", key, val);

	if (key != NULL && key[0] == '_') {
		/* Indicate set failure for properties beginning with underscore. */
		duk_push_false(ctx);
	} else {
		duk_push_true(ctx);
	}
	return 1;
}

static duk_ret_t handle_delete(duk_context *ctx) {
	/* 'this' binding: handler
	 * [0]: target
	 * [1]: key
	 */

	const char *key = duk_to_string(ctx, 1);

	printf("handle_delete: key=%s\n", key);

	if (key != NULL && key[0] == '_') {
		/* Indicate delete failure for properties beginning with underscore. */
		duk_push_false(ctx);
	} else {
		duk_push_true(ctx);
	}
	return 1;
}

static const duk_function_list_entry handler_funcs[] = {
	{ "get", handle_get, 3 },
	{ "set", handle_set, 4 },
	{ "deleteProperty", handle_delete, 2 },
        { NULL, NULL, 0 }
};

static void setup_proxy(duk_context *ctx) {
	/*
	 *  new Proxy(target, handler)
	 *
	 *  target = { getTest: 123 }
	 *  handler = { ... }
	 */

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "Proxy");
	duk_push_object(ctx);  /* target */
	duk_push_number(ctx, 123);
	duk_put_prop_string(ctx, -2, "getTest");
	duk_push_object(ctx);  /* handler */
	duk_put_function_list(ctx, -1, handler_funcs);
	duk_new(ctx, 2);  /* [ global Proxy target handler ] -> [ global proxy_object ] */
	duk_remove(ctx, -2);

	/* -> [ proxy_object ] */
}

static duk_ret_t test_get1(duk_context *ctx) {
	duk_ret_t rc;

	setup_proxy(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_get_prop_string(ctx, -1, "getTest");
	printf("get result: rc=%d, value=%s\n", (int) rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_pop(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_get2(duk_context *ctx) {
	duk_ret_t rc;

	setup_proxy(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_get_prop_string(ctx, -1, "_getTest");
	printf("get result: rc=%d, value=%s\n", (int) rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_pop(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}


static duk_ret_t test_set1(duk_context *ctx) {
	duk_ret_t rc;

	setup_proxy(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "testValue");
	rc = duk_put_prop_string(ctx, -2, "setTest");
	printf("set result: rc=%d\n", (int) rc);

	duk_pop(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_set2(duk_context *ctx) {
	duk_ret_t rc;

	setup_proxy(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "testValue");
	rc = duk_put_prop_string(ctx, -2, "_setTest");
	printf("set result: rc=%d\n", (int) rc);

	duk_pop(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_delete1(duk_context *ctx) {
	duk_ret_t rc;

	setup_proxy(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_del_prop_string(ctx, -1, "deleteTest");
	printf("delete result: rc=%d\n", (int) rc);

	duk_pop(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_delete2(duk_context *ctx) {
	duk_ret_t rc;

	setup_proxy(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_del_prop_string(ctx, -1, "_deleteTest");
	printf("delete result: rc=%d\n", (int) rc);

	duk_pop(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/* If set/delete is rejected (return false) for the property name,
	 * an error is thrown because Duktape/C contexts are always strict
	 * (in Duktape 0.12.0 and onwards).
	 */

	TEST_SAFE_CALL(test_get1);
	TEST_SAFE_CALL(test_get2);
	TEST_SAFE_CALL(test_set1);
	TEST_SAFE_CALL(test_set2);
	TEST_SAFE_CALL(test_delete1);
	TEST_SAFE_CALL(test_delete2);
}
