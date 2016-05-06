/*
 *  Some tests for using ROM strings and builtins, but a writable global
 *  object.
 *
 *  Run manually.
 */

/*---
{
    "skip": true
}
---*/

/*===
*** test_set_prototype (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
===*/

/* Prevent setting prototype of a built-in object from C code.  This is
 * different from setPrototypeOf() because the C API does not care if
 * the object is non-extensible.
 */
static duk_ret_t test_set_prototype(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "Math");
	duk_push_object(ctx);  /* dummy */
	duk_set_prototype(ctx, -2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_set_prototype);
}
