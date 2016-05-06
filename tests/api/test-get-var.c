/*===
*** test_1_safecall (duk_safe_call)
strict: 1
Math.PI = 3.141592653589793
final top: 0
==> rc=0, result='undefined'
*** test_1 (duk_pcall)
strict: 1
Math.PI = 3.141592653589793
final top: 0
==> rc=0, result='undefined'
*** test_2_safecall (duk_safe_call)
strict: 1
==> rc=1, result='ReferenceError: identifier 'no-such-property' undefined'
*** test_2 (duk_pcall)
strict: 1
==> rc=1, result='ReferenceError: identifier 'no-such-property' undefined'
*** test_3_safecall (duk_safe_call)
strict: 1
==> rc=1, result='TypeError: string required, found 123 (stack index -1)'
*** test_3 (duk_pcall)
strict: 1
==> rc=1, result='TypeError: string required, found 123 (stack index -1)'
*** test_4_safecall (duk_safe_call)
strict: 1
==> rc=1, result='TypeError: string required, found 123 (stack index -1)'
*** test_4 (duk_pcall)
strict: 1
==> rc=1, result='TypeError: string required, found 123 (stack index -1)'
===*/

/* Existing property */
static duk_ret_t test_1(duk_context *ctx) {
	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "Math");
	duk_get_var(ctx);
	duk_get_prop_string(ctx, -1, "PI");
	printf("Math.PI = %s\n", duk_to_string(ctx, -1));
	duk_pop_2(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_1_safecall(duk_context *ctx, void *udata) {
	return test_1(ctx);
}

/* Nonexistent -> throw */
static duk_ret_t test_2(duk_context *ctx) {
	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "no-such-property");
	duk_get_var(ctx);
	printf("value=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_2_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_2(ctx);
}

/* Variable name is not a string */
static duk_ret_t test_3(duk_context *ctx) {
	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_get_var(ctx);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_3_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_3(ctx);
}

/* Value stack is empty (no varname at all) */
static duk_ret_t test_4(duk_context *ctx) {
	printf("strict: %d\n", (int) duk_is_strict_call(ctx));

	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_get_var(ctx);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_4_safecall(duk_context *ctx, void *udata) {
	(void) udata;
	return test_4(ctx);
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1_safecall);
	TEST_PCALL(test_1);
	TEST_SAFE_CALL(test_2_safecall);
	TEST_PCALL(test_2);
	TEST_SAFE_CALL(test_3_safecall);
	TEST_PCALL(test_3);
	TEST_SAFE_CALL(test_4_safecall);
	TEST_PCALL(test_4);
}
