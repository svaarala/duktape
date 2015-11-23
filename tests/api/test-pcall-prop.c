/*===
*** test_1 (duk_safe_call)
me
rc=0, result='21'
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
object 123
rc=0, result='21'
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
number 123
rc=0, result='21'
==> rc=0, result='undefined'
*** test_4 (duk_safe_call)
rc=1, result='Error: my error'
==> rc=0, result='undefined'
*** test_5 (duk_safe_call)
rc=1, result='TypeError: cannot read property 'foo' of undefined'
==> rc=0, result='undefined'
*** test_6 (duk_safe_call)
rc=1, result='RangeError: getter error'
==> rc=0, result='undefined'
*** test_7 (duk_safe_call)
rc=1, result='Error: invalid stack index -6'
==> rc=0, result='undefined'
*** test_8 (duk_safe_call)
rc=1, result='TypeError: undefined not callable'
==> rc=0, result='undefined'
*** test_9 (duk_safe_call)
==> rc=1, result='Error: invalid call args'
final top: 0
===*/

static int test_1(duk_context *ctx) {
	duk_ret_t rc;

	/* basic success case: own property */
	duk_eval_string(ctx, "({ name: 'me', foo: function (x,y) { print(this.name); return x+y; } })");  /* idx 1 */
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_2(duk_context *ctx) {
	duk_ret_t rc;

	/* use plain number as 'this', add function to Number.prototype; non-strict handler
	 * causes this to be coerced to Number.
	 */
	duk_eval_string(ctx, "Number.prototype.func_nonstrict = function (x,y) { print(typeof this, this); return x+y; };");
	duk_pop(ctx);  /* pop result */
	duk_push_int(ctx, 123);  /* obj */
	duk_push_string(ctx, "func_nonstrict");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, -4, 2);  /* use relative index for a change */
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_3(duk_context *ctx) {
	duk_ret_t rc;

	/* use plain number as 'this', add function to Number.prototype; strict handler
	 * causes this to remain a plain number.
	 */
	duk_eval_string(ctx, "Number.prototype.func_strict = function (x,y) { 'use strict'; print(typeof this, this); return x+y; };");
	duk_pop(ctx);  /* pop result */
	duk_push_int(ctx, 123);  /* obj */
	duk_push_string(ctx, "func_strict");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_4(duk_context *ctx) {
	duk_ret_t rc;

	/* basic error case */
	duk_eval_string(ctx, "({ name: 'me', foo: function (x,y) { throw new Error('my error'); } })");  /* idx 1 */
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_5(duk_context *ctx) {
	duk_ret_t rc;

	/* property lookup fails: base value does not allow property lookup */
	duk_push_undefined(ctx);
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_6(duk_context *ctx) {
	duk_ret_t rc;

	/* property lookup fails: getter throws */
	duk_eval_string(ctx, "({ get prop() { throw new RangeError('getter error'); } })");
	duk_push_string(ctx, "prop");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_7(duk_context *ctx) {
	duk_ret_t rc;

	/* invalid object index */
	duk_eval_string(ctx, "({ foo: 1, bar: 2 })");
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, -6, 2);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_8(duk_context *ctx) {
	duk_ret_t rc;

	/* invalid arg count, causes 'key' to be identified with the object in the stack */
	duk_eval_string(ctx, "({ foo: function () { print('foo called'); } })");
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 3);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

static int test_9(duk_context *ctx) {
	duk_ret_t rc;

	/* Invalid arg count, 'key' would be below start of stack.  This
	 * results in an actual (uncaught) error at the moment, and matches
	 * the behavior of other protected call API functions.
	 */

	duk_eval_string(ctx, "({ foo: function () { print('foo called'); } })");
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	rc = duk_pcall_prop(ctx, 1, 8);
	printf("rc=%d, result='%s'\n", (int) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);  /* res */
	duk_pop(ctx);  /* obj */

	return 0;
}

void test(duk_context *ctx) {

	/* dummy just to offset the object index from 0 */
	duk_push_string(ctx, "foo");

	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
	TEST_SAFE_CALL(test_5);
	TEST_SAFE_CALL(test_6);
	TEST_SAFE_CALL(test_7);
	TEST_SAFE_CALL(test_8);
	TEST_SAFE_CALL(test_9);

	duk_pop(ctx);  /* dummy */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
}
