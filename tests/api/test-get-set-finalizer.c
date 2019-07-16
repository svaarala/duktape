/*
 *  Testcase for object finalizer manipulation.
 */

/*===
*** test_basic (duk_safe_call)
top: 1
finalizer name: basic_finalizer
top: 1
before set top 0
basic_finalizer, arg: target object
after set top 0
==> rc=0, result='undefined'
*** test_recursive_finalizer (duk_safe_call)
top: 1
finalizer.bar=321
c_finalizer.quux=234
before set top 0
finalizing obj, obj.foo: 123
c_finalizer, argument bar: 321
after set top 0
before explicit gc
after explicit gc
==> rc=0, result='undefined'
*** test_get_nonobject (duk_safe_call)
read finalizer: undefined
==> rc=0, result='undefined'
*** test_set_nonobject (duk_safe_call)
==> rc=1, result='TypeError: object required, found 123 (stack index -2)'
*** test_finalizer_loop (duk_safe_call)
before pop
after pop
before forced gc
finalizer called
after forced gc
==> rc=0, result='undefined'
*** test_nonextensible (duk_safe_call)
==> rc=1, result='TypeError: not extensible'
===*/

static duk_ret_t basic_finalizer(duk_context *ctx) {
	printf("basic_finalizer, arg: %s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	/* Object to be finalized, special toString() */
	duk_push_object(ctx);
	duk_eval_string(ctx, "(function() { return 'target object'; })");
	duk_put_prop_string(ctx, -2, "toString");

	/* [ target ] */

	/* Set finalizer */
	duk_push_c_function(ctx, basic_finalizer, 1);
	duk_push_string(ctx, "basic_finalizer");
	duk_put_prop_string(ctx, -2, "myName");
	duk_set_finalizer(ctx, -2);

	/* [ target ] */

	printf("top: %ld\n", (long) duk_get_top(ctx));

	/* Get finalizer and check it is correct */
	duk_get_finalizer(ctx, -1);
	duk_get_prop_string(ctx, -1, "myName");
	printf("finalizer name: %s\n", duk_to_string(ctx, -1));
	duk_pop_2(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));

	/* [ target ] */

	printf("before set top 0\n");
	duk_set_top(ctx, 0);
	printf("after set top 0\n");

	/* [ ] */

	return 0;
}

static duk_ret_t c_finalizer(duk_context *ctx) {
	duk_get_prop_string(ctx, 0, "bar");
	printf("c_finalizer, argument bar: %s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t test_recursive_finalizer(duk_context *ctx, void *udata) {
	(void) udata;

	/* Object to be finalized */
	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "foo");

	/* ECMAScript finalizer */
	duk_eval_string(ctx, "(function (obj) { print('finalizing obj, obj.foo:', obj.foo); })");
	duk_push_int(ctx, 321);
	duk_put_prop_string(ctx, -2, "bar");

	/* [ target finalizer ] */

	/* Break the function <-> prototype reference loop so that the
	 * ECMAScript finalizer is not in a reference loop and gets
	 * collected by refcounting.
	 *
	 * (Note that 'prototype' is not configurable so we can't
	 * delete it.)
	 */
	duk_push_string(ctx, "dummy");
	duk_put_prop_string(ctx, -2, "prototype");

	/* Add a Duktape/C finalizer for the ECMAScript finalizer to
	 * exercise both Duktape/C finalizers and recursive finalization
	 */
	duk_push_c_function(ctx, c_finalizer, 1);
	duk_push_int(ctx, 234);
	duk_put_prop_string(ctx, -2, "quux");
	duk_set_finalizer(ctx, -2);

	/* [ target(foo:123) finalizer(bar:321) ] */

	/* Set ECMAScript finalizer to original object */
	duk_set_finalizer(ctx, -2);

	/* [ target(foo:123) ] */

	printf("top: %ld\n", (long) duk_get_top(ctx));

	/* Read back the finalizer and the finalizer's finalizer */
	duk_get_finalizer(ctx, -1);  /* target's finalizer */
	duk_get_finalizer(ctx, -1);  /* finalizer's finalizer */

	/* [ target(foo:123) finalizer(bar:321) c_finalizer(quux:234) ] */

	duk_get_prop_string(ctx, -2, "bar");
	printf("finalizer.bar=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	duk_get_prop_string(ctx, -1, "quux");
	printf("c_finalizer.quux=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* [ target(foo:123) finalizer(bar:321) c_finalizer(quux:234) ] */

	printf("before set top 0\n");
	duk_set_top(ctx, 0);  /* causes finalization */
	printf("after set top 0\n");

	/* [ ] */

	/* Explicit GC (just in case e.g. a reference loop prevented collection) */
	printf("before explicit gc\n");
	duk_gc(ctx, 0);
	printf("after explicit gc\n");

	return 0;
}

static duk_ret_t test_get_nonobject(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_int(ctx, 123);
	duk_get_finalizer(ctx, -1);
	printf("read finalizer: %s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

static duk_ret_t test_set_nonobject(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 321);
	duk_set_finalizer(ctx, -2);
	printf("never here\n");
	return 0;
}

static duk_ret_t test_finalizer_loop(duk_context *ctx, void *udata) {
	(void) udata;

	/* Setup a finalizer loop: the finalizer of a finalizer is the
	 * finalizer itself.  The finalizer won't be called recursively.
	 */
	duk_eval_string(ctx, "(function (obj) { print('finalizer called'); })");
	duk_dup(ctx, -1);
	duk_set_finalizer(ctx, -2);

	printf("before pop\n");
	duk_pop(ctx);
	printf("after pop\n");

	/* The finalizer participates in two circular references so it won't
	 * be collected until mark-and-sweep happens.  The first circular
	 * reference is the function<->prototype loop.  The second circular
	 * reference is the finalizer reference which points to the object
	 * itself.
	 */

	printf("before forced gc\n");
	duk_gc(ctx, 0);
	printf("after forced gc\n");

	return 0;
}

static duk_ret_t test_nonextensible(duk_context *ctx, void *udata) {
	(void) udata;

	/* Finalizer is stored as a hidden Symbol, with normal inheritance
	 * behavior.  Because it's a normal (Symbol) property, a finalizer
	 * cannot be set on a non-extensible (frozen or sealed) object.
	 */
	duk_eval_string(ctx, "(function () { var O = {}; Object.preventExtensions(O); return O; })()");
	duk_eval_string(ctx, "(function (obj) { print('finalizer called'); })");
	duk_set_finalizer(ctx, -2);

	printf("never here\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_recursive_finalizer);
	TEST_SAFE_CALL(test_get_nonobject);
	TEST_SAFE_CALL(test_set_nonobject);
	TEST_SAFE_CALL(test_finalizer_loop);
	TEST_SAFE_CALL(test_nonextensible);
}
