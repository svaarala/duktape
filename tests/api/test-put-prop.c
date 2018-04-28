/*===
*** test_ex_writable_safecall (duk_safe_call)
strict: 1
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
*** test_ex_nonwritable_safecall (duk_safe_call)
strict: 1
get Math -> rc=1
Math.PI=3.141592653589793
==> rc=1, result='TypeError: not writable'
*** test_ex_nonwritable (duk_pcall)
strict: 1
get Math -> rc=1
Math.PI=3.141592653589793
==> rc=1, result='TypeError: not writable'
*** test_ex_accessor_wo_setter_safecall (duk_safe_call)
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
==> rc=1, result='TypeError: setter undefined'
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
==> rc=1, result='TypeError: setter undefined'
*** test_ex_setter_throws_safecall (duk_safe_call)
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
*** test_new_extensible_safecall (duk_safe_call)
strict: 1
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
*** test_new_not_extensible_safecall (duk_safe_call)
strict: 1
eval:
(function () { var o = { foo: 1 }; Object.preventExtensions(o); return o; })()
top after eval: 1
==> rc=1, result='TypeError: not extensible'
*** test_new_not_extensible (duk_pcall)
strict: 1
eval:
(function () { var o = { foo: 1 }; Object.preventExtensions(o); return o; })()
top after eval: 1
==> rc=1, result='TypeError: not extensible'
*** test_putprop_shorthand_a_safecall (duk_safe_call)
{"2001":234,"foo":123,"bar":123,"nul\u0000key":345,"heapptr":456,"stringCoerced":567,"undefined":678,"litkey":789}
final top: 1
==> rc=0, result='undefined'
*** test_putprop_shorthand_a (duk_pcall)
{"2001":234,"foo":123,"bar":123,"nul\u0000key":345,"heapptr":456,"stringCoerced":567,"undefined":678,"litkey":789}
final top: 1
==> rc=0, result='undefined'
*** test_putprop_invalid_index1 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 234'
*** test_putprop_invalid_index2 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_string_invalid_index1 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_string_invalid_index2 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_lstring_invalid_index1 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_lstring_invalid_index2 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_literal_invalid_index1 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_literal_invalid_index2 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_index_invalid_index1 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 345'
*** test_putprop_index_invalid_index2 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
*** test_putprop_heapptr_invalid_index1 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index 456'
*** test_putprop_heapptr_invalid_index2 (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -2147483648'
===*/

/* Test property writing API call.
 *
 * Property write behavior is quite complex in ECMAScript and there
 * are many ECMAScript testcases to cover the behavior.  The purpose
 * of this testcase is to ensure the exposed API behavior is as
 * expected without covering every specification case.  In particular,
 * different throwing / return code combinations need to be covered.
 * Duktape/C contexts are now (since Duktape 0.12.0) always strict,
 * so the non-strict case doesn't happen when using Duktape/C API.
 *
 * The test case functions are called both with duk_safe_call() and
 * duk_pcall().  These two establish different execution contexts:
 * one is called outside a Duktape/C activation and the other is called
 * inside a Duktape/C activation.  Both cases are now 'strict' (since
 * Duktape 0.12.0).  A single test function is called from both contexts.
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
static duk_ret_t test_ex_writable(duk_context *ctx) {
	duk_ret_t rc;

	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);
	duk_push_string(ctx, "{ \"foo\": 1 }");
	duk_json_decode(ctx, 0);

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_ex_writable_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_ex_writable(ctx);
}

/* strict: error
 * (non-strict: return 0)
 */
static duk_ret_t test_ex_nonwritable(duk_context *ctx) {
	duk_ret_t rc;

	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	/* Math.PI is not writable */

	duk_set_top(ctx, 0);
	duk_push_global_object(ctx);
	rc = duk_get_prop_string(ctx, -1, "Math");  /* -> [ global Math ] */
	printf("get Math -> rc=%d\n", (int) rc);

	rc = duk_get_prop_string(ctx, -1, "PI");
	printf("Math.PI=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_push_string(ctx, "PI");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", (int) rc);

	rc = duk_get_prop_string(ctx, -1, "PI");
	printf("Math.PI=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_ex_nonwritable_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_ex_nonwritable(ctx);
}

/* strict: error
 * (non-strict: return 0)
 */
static duk_ret_t test_ex_accessor_wo_setter(duk_context *ctx) {
	const char *src;
	duk_ret_t rc;

	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

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
	printf("top after eval: %ld\n", (long) duk_get_top(ctx));

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_ex_accessor_wo_setter_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_ex_accessor_wo_setter(ctx);
}

/* strict: setter error propagates
 * (non-strict: same)
 */
static duk_ret_t test_ex_setter_throws(duk_context *ctx) {
	const char *src;
	duk_ret_t rc;

	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

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
	printf("top after eval: %ld\n", (long) duk_get_top(ctx));

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;

	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_ex_setter_throws_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_ex_setter_throws(ctx);
}

/* success */
static duk_ret_t test_new_extensible(duk_context *ctx) {
	duk_ret_t rc;

	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);
	duk_push_string(ctx, "{ \"foo\": 1 }");
	duk_json_decode(ctx, 0);

	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_new_extensible_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_new_extensible(ctx);
}

/* strict: error
 * (non-strict: return 0)
 */
static duk_ret_t test_new_not_extensible(duk_context *ctx) {
	const char *src;
	duk_ret_t rc;

	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	src = "(function () { var o = { foo: 1 }; Object.preventExtensions(o); return o; })()";

	duk_set_top(ctx, 0);
	duk_push_string(ctx, src);
	printf("eval:\n%s\n", duk_get_string(ctx, -1));
	duk_eval(ctx);
	printf("top after eval: %ld\n", (long) duk_get_top(ctx));

	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");
	rc = duk_put_prop(ctx, -3);
	printf("put rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("result: %s\n", duk_to_string(ctx, 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_new_not_extensible_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_new_not_extensible(ctx);
}

static duk_ret_t test_putprop_shorthand_a(duk_context *ctx) {
	void *ptr;

	duk_eval_string(ctx, "({ foo: 123 })");

	duk_push_uint(ctx, 123);
	duk_put_prop_string(ctx, -2, "bar" "\x00" "quux");

	duk_push_uint(ctx, 234);
	duk_put_prop_index(ctx, -2, 2001);

	duk_push_uint(ctx, 345);
	duk_put_prop_lstring(ctx, -2, "nul" "\x00" "keyx", 7);

	duk_push_string(ctx, "heapptr");
	ptr = duk_require_heapptr(ctx, -1);
	duk_push_string(ctx, "DUMMY");  /* just to ensure 'heapptr' is not used based on position */
	duk_push_uint(ctx, 456);
	duk_put_prop_heapptr(ctx, -4, ptr);
	duk_pop_2(ctx);

	duk_eval_string(ctx, "({ toString: function () { return 'stringCoerced'; } })");
	ptr = duk_require_heapptr(ctx, -1);
	duk_push_string(ctx, "DUMMY");  /* just to ensure 'heapptr' is not used based on position */
	duk_push_uint(ctx, 567);
	duk_put_prop_heapptr(ctx, -4, ptr);
	duk_pop_2(ctx);

	duk_push_uint(ctx, 678);
	duk_put_prop_heapptr(ctx, -2, NULL);

	duk_push_uint(ctx, 789);
	duk_put_prop_literal(ctx, -2, "litkey");

	duk_json_encode(ctx, -1);
	printf("%s\n", duk_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_putprop_shorthand_a_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_putprop_shorthand_a(ctx);
}

static duk_ret_t test_putprop_invalid_index1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "foo");
	duk_push_uint(ctx, 123);
	duk_put_prop(ctx, 234);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "foo");
	duk_push_uint(ctx, 123);
	duk_put_prop(ctx, DUK_INVALID_INDEX);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_string_invalid_index1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_string(ctx, -2, "foo");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_string_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_string(ctx, DUK_INVALID_INDEX, "foo");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_lstring_invalid_index1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_lstring(ctx, 123, "foox", 3);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_lstring_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_lstring(ctx, DUK_INVALID_INDEX, "foox", 3);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_literal_invalid_index1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_literal(ctx, 123, "foo");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_literal_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_literal(ctx, DUK_INVALID_INDEX, "foo");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_index_invalid_index1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_index(ctx, 345, 1001U);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_index_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_index(ctx, DUK_INVALID_INDEX, 1001U);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_heapptr_invalid_index1(duk_context *ctx, void *udata) {
	void *ptr;

	(void) udata;

	duk_push_string(ctx, "foo");
	ptr = duk_get_heapptr(ctx, -1);

	duk_push_uint(ctx, 123);
	duk_put_prop_heapptr(ctx, 456, ptr);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_putprop_heapptr_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_uint(ctx, 123);
	duk_put_prop_heapptr(ctx, DUK_INVALID_INDEX, NULL);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

#define  TEST(func)  do { \
		TEST_SAFE_CALL(func ## _safecall); \
		TEST_PCALL(func); \
	} while (0)

void test(duk_context *ctx) {
	/* Cases where own property already exists. */
	TEST(test_ex_writable);
	TEST(test_ex_nonwritable);
	TEST(test_ex_accessor_wo_setter);
	TEST(test_ex_setter_throws);

	/* Cases where no own property, possibly ancestor
	 * property of same name.
	 */
	TEST(test_new_extensible);
	TEST(test_new_not_extensible);

	/* Shorthands. */
	TEST(test_putprop_shorthand_a);

	/* Invalid index. */
	TEST_SAFE_CALL(test_putprop_invalid_index1);
	TEST_SAFE_CALL(test_putprop_invalid_index2);
	TEST_SAFE_CALL(test_putprop_string_invalid_index1);
	TEST_SAFE_CALL(test_putprop_string_invalid_index2);
	TEST_SAFE_CALL(test_putprop_lstring_invalid_index1);
	TEST_SAFE_CALL(test_putprop_lstring_invalid_index2);
	TEST_SAFE_CALL(test_putprop_literal_invalid_index1);
	TEST_SAFE_CALL(test_putprop_literal_invalid_index2);
	TEST_SAFE_CALL(test_putprop_index_invalid_index1);
	TEST_SAFE_CALL(test_putprop_index_invalid_index2);
	TEST_SAFE_CALL(test_putprop_heapptr_invalid_index1);
	TEST_SAFE_CALL(test_putprop_heapptr_invalid_index2);
}
