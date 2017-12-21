/*
 *  duk_instanceof() with rhs having @@hasInstance
 */

/*===
*** test_1 (duk_safe_call)
hasinst called
instanceof: 1
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* Function.prototype[@@hasInstance] is not writable or configurable.
	 * To set it, use duk_def_prop() or Object.defineProperty() to avoid
	 * the ancestor blocking the write.
	 */
	duk_eval_string(ctx, "123");
	duk_eval_string(ctx, "(function foo() {})");
	duk_push_string(ctx, DUK_WELLKNOWN_SYMBOL("Symbol.hasInstance"));
	duk_eval_string(ctx, "(function hasinst() { print('hasinst called'); return true; })");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);

	/* [ lhs rhs ] */

	printf("instanceof: %d\n", (int) duk_instanceof(ctx, 0, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
