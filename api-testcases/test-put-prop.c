/*===
*** test_ex_writable (duk_safe_call)
strict: 0
put rc=1
result: {"foo":"bar"}
final top: 1
==> rc=0, result='undefined'
*** test_ex_writable (duk_pcall)
strict: 1
put rc=1
result: {"foo":"bar"}
final top: 1
==> rc=0, result='undefined'
*** test_ex_nonwritable (duk_safe_call)
strict: 0
get Math -> rc=1
Math.PI=3.141592653589793
put rc=0
Math.PI=3.141592653589793
final top: 2
==> rc=0, result='undefined'
*** test_ex_nonwritable (duk_pcall)
strict: 1
get Math -> rc=1
Math.PI=3.141592653589793
==> rc=1, result='TypeError: property not writable'
*** test_ex_accessor_wo_setter (duk_safe_call)
strict: 0
eval:
(function () {
    var o = {};
    Object.defineProperty(o, 'foo', {
        configurable: true,
        extensible: true,
        get: function() { print('getter'); }
        // no setter
    });
    return o;
})()
top after eval: 1
put rc=0
result: {}
final top: 1
==> rc=0, result='undefined'
*** test_ex_accessor_wo_setter (duk_pcall)
strict: 1
eval:
(function () {
    var o = {};
    Object.defineProperty(o, 'foo', {
        configurable: true,
        extensible: true,
        get: function() { print('getter'); }
        // no setter
    });
    return o;
})()
top after eval: 1
==> rc=1, result='TypeError: undefined setter for accessor'
*** test_ex_setter_throws (duk_safe_call)
strict: 0
eval:
(function () {
    var o = {};
    Object.defineProperty(o, 'foo', {
        configurable: true,
        extensible: true,
        get: function() { print('getter'); },
        set: function() { print('setter, throw error'); throw 'setter error' }
    });
    return o;
})()
top after eval: 1
setter, throw error
==> rc=1, result='setter error'
*** test_ex_setter_throws (duk_pcall)
strict: 1
eval:
(function () {
    var o = {};
    Object.defineProperty(o, 'foo', {
        configurable: true,
        extensible: true,
        get: function() { print('getter'); },
        set: function() { print('setter, throw error'); throw 'setter error' }
    });
    return o;
})()
top after eval: 1
setter, throw error
==> rc=1, result='setter error'
*** test_new_extensible (duk_safe_call)
strict: 0
put rc=1
result: {"foo":1,"bar":"quux"}
final top: 1
==> rc=0, result='undefined'
*** test_new_extensible (duk_pcall)
strict: 1
put rc=1
result: {"foo":1,"bar":"quux"}
final top: 1
==> rc=0, result='undefined'
*** test_new_not_extensible (duk_safe_call)
strict: 0
eval:
(function () { var o = { foo: 1 }; Object.preventExtensions(o); return o; })()
top after eval: 1
put rc=0
result: {"foo":1}
final top: 1
==> rc=0, result='undefined'
*** test_new_not_extensible (duk_pcall)
strict: 1
eval:
(function () { var o = { foo: 1 }; Object.preventExtensions(o); return o; })()
top after eval: 1
==> rc=1, result='TypeError: object not extensible'
===*/

/* Test property writing API call.
 *
 * Property write behavior is quite complex in Ecmascript and there
 * are many Ecmascript testcases to cover the behavior.  The purpose
 * of this testcase is to ensure the exposed API behavior is as
 * expected without covering every specification case.  In particular,
 * different throwing / return code combinations need to be covered,
 * in both strict and non-strict mode.
 *
 * The test case functions are called both with duk_safe_call()
 * and duk_pcall().  These two establish different execution
 * contexts: one is called outside a Duktape/C activation and
 * is non-strict, while the other is called inside a Duktape/C
 * activation and is strict.  A single test function is called
 * from both contexts.
 */

/*
 *  Not covered cases include:
 *
 *  - new property, ancestor has writable data property
 *  - new property, ancestor has write-protected data property
 *  - new property, ancestor has accessor with setter
 *    + setter succeeds
 *    + setter fails
 *  - new property, ancestor has accessor without setter
 */

/* success */
int test_ex_writable(duk_context *ctx) {
	int rc;

	printf("strict: %d\n", duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);
	duk_push_string(ctx, "{ \"foo\": 1 }");
	duk_json_decode(ctx, 0);

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* strict: error
 * non-strict: return 0
 */
int test_ex_nonwritable(duk_context *ctx) {
	int rc;

	printf("strict: %d\n", duk_is_strict_call(ctx));

	/* Math.PI is not writable */

	duk_set_top(ctx, 0);
	duk_push_global_object(ctx);
	rc = duk_get_prop_string(ctx, -1, "Math");  /* -> [ global Math ] */
	printf("get Math -> rc=%d\n", rc);

	rc = duk_get_prop_string(ctx, -1, "PI");
	printf("Math.PI=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_push_string(ctx, "PI");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", rc);

	rc = duk_get_prop_string(ctx, -1, "PI");
	printf("Math.PI=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* strict: error
 * non-strict: return 0
 */
int test_ex_accessor_wo_setter(duk_context *ctx) {
	const char *src;
	int rc;

	printf("strict: %d\n", duk_is_strict_call(ctx));

	src = "(function () {\n"
	      "    var o = {};\n"
	      "    Object.defineProperty(o, 'foo', {\n"
	      "        configurable: true,\n"
	      "        extensible: true,\n"
	      "        get: function() { print('getter'); }\n"
	      "        // no setter\n"
	      "    });\n"
	      "    return o;\n"
	      "})()";

	duk_set_top(ctx, 0);
	duk_push_string(ctx, src);
	printf("eval:\n%s\n", duk_get_string(ctx, -1));
	duk_eval(ctx);
	printf("top after eval: %d\n", duk_get_top(ctx));

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* strict: setter error propagates
 * non-strict: same
 */
int test_ex_setter_throws(duk_context *ctx) {
	const char *src;
	int rc;

	printf("strict: %d\n", duk_is_strict_call(ctx));

	src = "(function () {\n"
	      "    var o = {};\n"
	      "    Object.defineProperty(o, 'foo', {\n"
	      "        configurable: true,\n"
	      "        extensible: true,\n"
	      "        get: function() { print('getter'); },\n"
	      "        set: function() { print('setter, throw error'); throw 'setter error' }\n"
	      "    });\n"
	      "    return o;\n"
	      "})()";

	duk_set_top(ctx, 0);
	duk_push_string(ctx, src);
	printf("eval:\n%s\n", duk_get_string(ctx, -1));
	duk_eval(ctx);
	printf("top after eval: %d\n", duk_get_top(ctx));

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;

	printf("strict: %d\n", duk_is_strict_call(ctx));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* success */
int test_new_extensible(duk_context *ctx) {
	int rc;

	printf("strict: %d\n", duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);
	duk_push_string(ctx, "{ \"foo\": 1 }");
	duk_json_decode(ctx, 0);

	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* strict: error
 * non-strict: return 0
 */
int test_new_not_extensible(duk_context *ctx) {
	const char *src;
	int rc;

	printf("strict: %d\n", duk_is_strict_call(ctx));

	src = "(function () { var o = { foo: 1 }; Object.preventExtensions(o); return o; })()";

	duk_set_top(ctx, 0);
	duk_push_string(ctx, src);
	printf("eval:\n%s\n", duk_get_string(ctx, -1));
	duk_eval(ctx);
	printf("top after eval: %d\n", duk_get_top(ctx));

	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

#define  TEST(func)  do { \
		TEST_SAFE_CALL(func); \
		TEST_PCALL(func); \
	} while (0)

void test(duk_context *ctx) {
	/*
	 *  Cases where own property already exists
	 */

	TEST(test_ex_writable);
	TEST(test_ex_nonwritable);
	TEST(test_ex_accessor_wo_setter);
	TEST(test_ex_setter_throws);

	/*
	 *  Cases where no own property, possibly ancestor
	 *  property of same name
	 */

	TEST(test_new_extensible);
	TEST(test_new_not_extensible);
}

