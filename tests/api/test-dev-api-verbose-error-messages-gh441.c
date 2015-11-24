/*
 *  Fragile error test for (some) API error messages
 */

/*===
*** test_1a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -3'
*** test_1b (duk_safe_call)
==> rc=1, result='Error: invalid stack index -3'
*** test_1c (duk_safe_call)
==> rc=1, result='Error: invalid stack index -1'
*** test_2a (duk_safe_call)
TypeError: null required, found none (stack index -3)
top: 1
test__undefined ok
TypeError: null required, found undefined (stack index -3)
TypeError: boolean required, found undefined (stack index -3)
TypeError: number required, found undefined (stack index -3)
TypeError: string required, found undefined (stack index -3)
TypeError: buffer required, found undefined (stack index -3)
TypeError: pointer required, found undefined (stack index -3)
TypeError: nativefunction required, found undefined (stack index -3)
top: 1
TypeError: undefined required, found null (stack index -3)
test__null ok
TypeError: boolean required, found null (stack index -3)
TypeError: number required, found null (stack index -3)
TypeError: string required, found null (stack index -3)
TypeError: buffer required, found null (stack index -3)
TypeError: pointer required, found null (stack index -3)
TypeError: nativefunction required, found null (stack index -3)
top: 1
TypeError: undefined required, found true (stack index -3)
TypeError: null required, found true (stack index -3)
test__boolean ok
TypeError: number required, found true (stack index -3)
TypeError: string required, found true (stack index -3)
TypeError: buffer required, found true (stack index -3)
TypeError: pointer required, found true (stack index -3)
TypeError: nativefunction required, found true (stack index -3)
top: 1
TypeError: undefined required, found false (stack index -3)
TypeError: null required, found false (stack index -3)
test__boolean ok
TypeError: number required, found false (stack index -3)
TypeError: string required, found false (stack index -3)
TypeError: buffer required, found false (stack index -3)
TypeError: pointer required, found false (stack index -3)
TypeError: nativefunction required, found false (stack index -3)
top: 1
TypeError: undefined required, found 123 (stack index -3)
TypeError: null required, found 123 (stack index -3)
TypeError: boolean required, found 123 (stack index -3)
test__number ok
TypeError: string required, found 123 (stack index -3)
TypeError: buffer required, found 123 (stack index -3)
TypeError: pointer required, found 123 (stack index -3)
TypeError: nativefunction required, found 123 (stack index -3)
top: 1
TypeError: undefined required, found 'foo' (stack index -3)
TypeError: null required, found 'foo' (stack index -3)
TypeError: boolean required, found 'foo' (stack index -3)
TypeError: number required, found 'foo' (stack index -3)
test__string ok
TypeError: buffer required, found 'foo' (stack index -3)
TypeError: pointer required, found 'foo' (stack index -3)
TypeError: nativefunction required, found 'foo' (stack index -3)
top: 1
TypeError: undefined required, found [buffer:16] (stack index -3)
TypeError: null required, found [buffer:16] (stack index -3)
TypeError: boolean required, found [buffer:16] (stack index -3)
TypeError: number required, found [buffer:16] (stack index -3)
TypeError: string required, found [buffer:16] (stack index -3)
test__buffer ok
TypeError: pointer required, found [buffer:16] (stack index -3)
TypeError: nativefunction required, found [buffer:16] (stack index -3)
top: 1
TypeError: undefined required, found (null) (stack index -3)
TypeError: null required, found (null) (stack index -3)
TypeError: boolean required, found (null) (stack index -3)
TypeError: number required, found (null) (stack index -3)
TypeError: string required, found (null) (stack index -3)
TypeError: buffer required, found (null) (stack index -3)
test__pointer ok
TypeError: nativefunction required, found (null) (stack index -3)
top: 1
TypeError: undefined required, found (PTR) (stack index -3)
TypeError: null required, found (PTR) (stack index -3)
TypeError: boolean required, found (PTR) (stack index -3)
TypeError: number required, found (PTR) (stack index -3)
TypeError: string required, found (PTR) (stack index -3)
TypeError: buffer required, found (PTR) (stack index -3)
test__pointer ok
TypeError: nativefunction required, found (PTR) (stack index -3)
top: 1
TypeError: undefined required, found [object Object] (stack index -3)
TypeError: null required, found [object Object] (stack index -3)
TypeError: boolean required, found [object Object] (stack index -3)
TypeError: number required, found [object Object] (stack index -3)
TypeError: string required, found [object Object] (stack index -3)
TypeError: buffer required, found [object Object] (stack index -3)
TypeError: pointer required, found [object Object] (stack index -3)
TypeError: nativefunction required, found [object Object] (stack index -3)
top: 1
TypeError: undefined required, found [object Array] (stack index -3)
TypeError: null required, found [object Array] (stack index -3)
TypeError: boolean required, found [object Array] (stack index -3)
TypeError: number required, found [object Array] (stack index -3)
TypeError: string required, found [object Array] (stack index -3)
TypeError: buffer required, found [object Array] (stack index -3)
TypeError: pointer required, found [object Array] (stack index -3)
TypeError: nativefunction required, found [object Array] (stack index -3)
top: 1
TypeError: undefined required, found [object Function] (stack index -3)
TypeError: null required, found [object Function] (stack index -3)
TypeError: boolean required, found [object Function] (stack index -3)
TypeError: number required, found [object Function] (stack index -3)
TypeError: string required, found [object Function] (stack index -3)
TypeError: buffer required, found [object Function] (stack index -3)
TypeError: pointer required, found [object Function] (stack index -3)
test__c_function ok
top: 1
TypeError: undefined required, found function LFUNC() {/= light =/} (stack index -3)
TypeError: null required, found function LFUNC() {/= light =/} (stack index -3)
TypeError: boolean required, found function LFUNC() {/= light =/} (stack index -3)
TypeError: number required, found function LFUNC() {/= light =/} (stack index -3)
TypeError: string required, found function LFUNC() {/= light =/} (stack index -3)
TypeError: buffer required, found function LFUNC() {/= light =/} (stack index -3)
TypeError: pointer required, found function LFUNC() {/= light =/} (stack index -3)
TypeError: nativefunction required, found function LFUNC() {/= light =/} (stack index -3)
top: 1
TypeError: undefined required, found [object Function] (stack index -3)
TypeError: null required, found [object Function] (stack index -3)
TypeError: boolean required, found [object Function] (stack index -3)
TypeError: number required, found [object Function] (stack index -3)
TypeError: string required, found [object Function] (stack index -3)
TypeError: buffer required, found [object Function] (stack index -3)
TypeError: pointer required, found [object Function] (stack index -3)
TypeError: nativefunction required, found [object Function] (stack index -3)
top: 1
TypeError: undefined required, found [object Thread] (stack index -3)
TypeError: null required, found [object Thread] (stack index -3)
TypeError: boolean required, found [object Thread] (stack index -3)
TypeError: number required, found [object Thread] (stack index -3)
TypeError: string required, found [object Thread] (stack index -3)
TypeError: buffer required, found [object Thread] (stack index -3)
TypeError: pointer required, found [object Thread] (stack index -3)
TypeError: nativefunction required, found [object Thread] (stack index -3)
done
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1a(duk_context *ctx) {
	duk_require_normalize_index(ctx, -3);
	return 0;
}

static duk_ret_t test_1b(duk_context *ctx) {
	duk_require_valid_index(ctx, -3);
	return 0;
}

static duk_ret_t test_1c(duk_context *ctx) {
	duk_require_top_index(ctx);
	return 0;
}

static duk_ret_t dummy_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test__undefined(duk_context *ctx) {
	duk_require_undefined(ctx, -3);
	return 0;
}
static duk_ret_t test__null(duk_context *ctx) {
	duk_require_null(ctx, -3);
	return 0;
}
static duk_ret_t test__boolean(duk_context *ctx) {
	(void) duk_require_boolean(ctx, -3);
	return 0;
}
static duk_ret_t test__number(duk_context *ctx) {
	(void) duk_require_number(ctx, -3);
	return 0;
}
static duk_ret_t test__string(duk_context *ctx) {
	(void) duk_require_string(ctx, -3);
	return 0;
}
static duk_ret_t test__buffer(duk_context *ctx) {
	duk_size_t sz;
	(void) duk_require_buffer(ctx, -3, &sz);
	return 0;
}
static duk_ret_t test__pointer(duk_context *ctx) {
	(void) duk_require_pointer(ctx, -3);
	return 0;
}
static duk_ret_t test__c_function(duk_context *ctx) {
	(void) duk_require_c_function(ctx, -3);
	return 0;
}

#define TEST_REQUIRE(fn) do { \
		duk_set_top(ctx, 1); \
		duk_dup(ctx, 0); \
		duk_push_null(ctx); \
		duk_push_null(ctx); \
		rc = duk_safe_call(ctx, (fn), 3, 3); \
		if (rc != 0) { \
			duk_eval_string(ctx, "(function (v) { print(String(v).replace(/\\(0x.*?\\)/g, '(PTR)')" \
			                     ".replace(/light_[0-9a-fA-F_]+/g, 'LFUNC')" \
			                     ".replace(/\\*/g, '=')); })"); \
			duk_dup(ctx, -4); \
			duk_call(ctx, 1); \
		} else { \
			printf("%s ok\n", #fn); \
		} \
		duk_set_top(ctx, 1); \
	} while (0)

static void test__require_calls(duk_context *ctx) {
	duk_int_t rc;

	printf("top: %ld\n", (long) duk_get_top(ctx));

	TEST_REQUIRE(test__undefined);
	TEST_REQUIRE(test__null);
	TEST_REQUIRE(test__boolean);
	TEST_REQUIRE(test__number);
	TEST_REQUIRE(test__string);
	TEST_REQUIRE(test__buffer);
	TEST_REQUIRE(test__pointer);
	TEST_REQUIRE(test__c_function);
	/* No duk_require_object(), duk_require_array(), duk_require_c_lightfunc(), etc. */
}

static duk_ret_t test_2a(duk_context *ctx) {
	/* Test first with nothing on stack index -3. */
	duk_safe_call(ctx, test__null, 0, 1); printf("%s\n", duk_safe_to_string(ctx, 0));
	duk_pop(ctx);

	duk_set_top(ctx, 0); duk_push_undefined(ctx); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_null(ctx); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_true(ctx); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_false(ctx); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_number(ctx, 123.0); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_string(ctx, "foo\x00" "bar"); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_fixed_buffer(ctx, 16); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_pointer(ctx, NULL); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_pointer(ctx, (void *) 0xdeadbeef); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_object(ctx); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_array(ctx); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_c_function(ctx, dummy_func, 0); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_c_lightfunc(ctx, dummy_func, 0, 0, 0); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_eval_string(ctx, "(function dummy(){})"); test__require_calls(ctx);
	duk_set_top(ctx, 0); duk_push_thread(ctx); test__require_calls(ctx);

	printf("done\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1a);
	TEST_SAFE_CALL(test_1b);
	TEST_SAFE_CALL(test_1c);
	TEST_SAFE_CALL(test_2a);
}
